#include "AsyncQueryExecutor.h"

#include "postgres/PGConnection.h"
#include "postgres/PGResult.h"

#include "utils/logging.h"
#include <log4cplus/loggingmacros.h>
#include <log4cplus/tstring.h>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>
#include <deque>
#include <thread>

#include "utils/utility.h"
#include "utils/SignalHandler.h"

extern "C" {
    #include <libpq-fe.h>
}

using namespace std;
using namespace log4cplus;

namespace db_agg {
    static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("AsyncQueryExecutor"));

    enum class PGQueryExecutionState {
        INITIAL,
        CONNECTING,
        CONNECTED,
        QUERY_SENDING,
        QUERY_SENT,
        DONE,
        FAILED
    };

    struct AsyncQueryExecutor::QueryTask {
        string connectionUrl;
        string query;
        string id;
        int queryNo;
        PGConnection conn;
        PGResult *result;
        string data;
        bool done;
        PGQueryExecutionState state;
        ExecutionHandler *handler;
        uint64_t rowsReceived = 0;
        int64_t lastRowSent = -1;
    };

    struct AsyncQueryExecutor::XImpl
    {
       deque<QueryTask*> tasks;
       bool cancelRequest = false;
    };

    AsyncQueryExecutor::AsyncQueryExecutor() {
        this->pImpl = new XImpl();
    }

    AsyncQueryExecutor::~AsyncQueryExecutor() {
        LOG_DEBUG("delete async query executor");
        for (unsigned int taskNo = 0;taskNo < this->pImpl->tasks.size();taskNo++) {
            QueryTask *task = this->pImpl->tasks[taskNo];
            LOG_DEBUG("delete connection at " << *task->conn);
            if (task->conn.connected()) {
                LOG_DEBUG("release connection " << task->connectionUrl);
                task->conn.finish();
            }
            delete task;
        }
        delete this->pImpl;
    }

    void AsyncQueryExecutor::stop() {
        pImpl->cancelRequest = true;
    }

    void AsyncQueryExecutor::addQuery(string id, string connectionUrl, string query, ExecutionHandler *handler) {
        QueryTask *qt = new QueryTask();
        qt->id = id;
        qt->connectionUrl = connectionUrl;
        qt->query = query;
        qt->queryNo = 0;
        qt->done = false;
        qt->state = PGQueryExecutionState::INITIAL;
        qt->handler = handler;
        this->pImpl->tasks.push_back(qt);
    }

    bool AsyncQueryExecutor::process() {
        LOG_DEBUG("start processing with " << this->pImpl->tasks.size() << " queries");
        // PQinitOpenSSL(1,1);
        bool done = false;
        fireEvent(EventType::INITIALIZE,-1);
        try {
            done = this->loop();
        } catch(CancelException& ce) {
            LOG_ERROR("query execution canceled:" << endl);
            cleanUp("CANCEL");
            throw ce;
        } catch(AsyncQueryExecutorException& aqee) {
            LOG_ERROR("query execution failed: " << aqee.what());
            LOG_ERROR("query:\n" << aqee.getQuery());
            cleanUp("");
            throw aqee;
        } catch(runtime_error& re) {
            LOG_ERROR("query execution failed: " << re.what());
            cleanUp("");
            throw re;
        } catch(exception& re) {
            LOG_ERROR("query execution failed: " << re.what());
            cleanUp("");
            throw re;
        } catch(...) {
            // TODO: find out how this works
            exception_ptr e = current_exception();
            LOG_ERROR("query execution failed:");
            cleanUp("");
            THROW_EXC("caught exception and stopped all tasks");
        }
        return done;
    }

    void AsyncQueryExecutor::cleanUp(string reason) {
        for (size_t idx = 0; idx < this->pImpl->tasks.size(); idx++) {
            cleanUp(idx, reason);
        }
    }

    bool AsyncQueryExecutor::loop() {
        LOG_DEBUG("called loop with " << this->pImpl->tasks.size() << " tasks scheduled");
        bool tasksDone = true;
        for (size_t taskNo = 0; taskNo < this->pImpl->tasks.size(); taskNo++) {
            LOG_TRACE("process task " << taskNo);
            bool taskDone = false;
            if (pImpl->cancelRequest) {
                throw CancelException("cancellation requested");
            }
            try {
                taskDone = this->processTask(taskNo);
            } catch(AsyncQueryExecutorException& e) {
                QueryTask *task = pImpl->tasks[taskNo];
                task->state = PGQueryExecutionState::FAILED;
                fireStateChangeEvent(taskNo, "FAILED");
                throw runtime_error(e.what());
            }
            tasksDone &= taskDone;
        }
        return tasksDone;
    }

bool AsyncQueryExecutor::processTask(int taskNo) {
        QueryTask *task = this->pImpl->tasks[taskNo];
        if (task->done) {
            return true;
        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        PGConnection& conn = task->conn;
        if (task->state == PGQueryExecutionState::INITIAL) {
            bool connectAsync = false;
            string connectionUrl = task->connectionUrl; // + " options='--statement-timeout=30000 --client-min-messages=debug1'";
            if (!connectAsync) {
                task->conn = PGConnection::connectDb(connectionUrl.c_str());
                LOG_INFO("connecting to " << maskPassword(task->connectionUrl));
                if (task->conn.connected()) {
                    task->state = PGQueryExecutionState::CONNECTED;
                    fireStateChangeEvent(taskNo, "CONNECTED");
                } else {
                    THROW_EXC("connecting to " << task->connectionUrl << " failed");
                }
            } else {
                task->conn = PGConnection::connectStart(connectionUrl.c_str());
                if (task->conn.setnonblocking(1) == -1) {
                    LOG_WARN("unable to set non blocking mode");
                }
                if (task->conn.isnonblocking() != 1) {
                    LOG_WARN("set non blocking mode failed");
                }
                task->state = PGQueryExecutionState::CONNECTING;
                fireStateChangeEvent(taskNo, "CONNECTING");
            }
        } else if (task->state == PGQueryExecutionState::CONNECTING) {
            PostgresPollingStatusType ppst = conn.connectPoll();
            if (ppst == PGRES_POLLING_FAILED) {
                THROW_EXC("failed to connect to " << task->connectionUrl << ". postgres message: " << conn.errorMessage());
            }
            if (ppst == PGRES_POLLING_OK) {
                task->state = PGQueryExecutionState::CONNECTED;
                fireStateChangeEvent(taskNo, "CONNECTED");
            }
        } else if (task->state == PGQueryExecutionState::CONNECTED) {
            LOG_DEBUG("about to send query");
            if (!task->conn.sendQuery(task->query)) {
                THROW_EXC("sending query  " << task->query << " failed. message = " << task->conn.errorMessage() << " connection = " << task->connectionUrl);
            }
            task->state = PGQueryExecutionState::QUERY_SENDING;
            fireStateChangeEvent(taskNo, "QUERY_SENDING");
        } else if (task->state == PGQueryExecutionState::QUERY_SENDING) {
            int ret = conn.flush();
            LOG_DEBUG("flush returned " << ret);
            if (ret==1) {
                task->state = PGQueryExecutionState::QUERY_SENDING;
            } else {
                task->state = PGQueryExecutionState::QUERY_SENT;
                fireStateChangeEvent(taskNo, "QUERY_SENT");
            }
        } else if (task->state == PGQueryExecutionState::QUERY_SENT) {
            int cir = conn.consumeInput();
            LOG_TRACE("consume input returned " << cir);
            if (!cir) {
                string message = conn.errorMessage();
                THROW_EXC("consume input " << task->query << " failed. message = " << conn.errorMessage());
            }
            if (conn.isBusy()==0) {
                LOG_TRACE("is not busy");

                PGResult *res = task->result;
                if (res==nullptr) {
                    task->result = new PGResult();
                    task->result->initialize(conn.getResult());
                    res = task->result;
                }

                LOG_DEBUG("RESULT:" << res);
                if (!*res) {
                    task->done = true;
                    task->state = PGQueryExecutionState::DONE;
                    task->conn.finish();
                    LOG_DEBUG("released connection " << task->connectionUrl);
                    fireStateChangeEvent(taskNo, "DONE");
                    LOG_DEBUG("RESULT IS NULL");
                    fireEvent(EventType::PROCESSED, taskNo);
                    return true;
                }
                ExecStatusType status = res->getStatusType();
                LOG_DEBUG("result status is " << status);
                string ss = res->getStatusTypeAsString(status);
                LOG_DEBUG("result status is " << ss);
                if (status == PGRES_FATAL_ERROR) {
                    LOG_WARN("get result returned fatal error");
                    string em = conn.errorMessage();
                    LOG_WARN("message = " << em);
                    LOG_DEBUG("query =\n" << task->query);
                    throw AsyncQueryExecutorException(em, task->query);
                } else if (status == PGRES_COPY_IN) {
                    fireStateChangeEvent(taskNo, "COPY_IN");
                    LOG_DEBUG("got copy in res=" << res);
                    int copyDataResult = 0;
                    uint64_t rowCount = task->handler->getRowCount(task->queryNo);
                    LOG_DEBUG("rowCount = " << rowCount);
                    uint64_t rowsPerChunk = 100;
                    if (rowCount>0) {
                        do {
                            if (pImpl->cancelRequest) {
                                throw CancelException("cancled while copy in");
                            }
                            LOG_TRACE("get row " << (task->lastRowSent+1));
                            uint64_t rowsRead = 0;
                            vector<DataChunk> chunks;
                            task->handler->handleCopyIn(task->queryNo, task->lastRowSent + 1, rowsPerChunk, chunks, rowsRead);
                            string data = DataChunk::contiguous(chunks);
                            LOG_TRACE("data = " << data);
                            LOG_TRACE("rowsRead = " << rowsRead);
                            LOG_TRACE("lastRowSent = " << (task->lastRowSent+1));
                            copyDataResult = conn.putCopyData(data);
                            if (copyDataResult == 1) {
                                task->lastRowSent += rowsRead;
                                shared_ptr<Event> rde(new SentDataEvent(task->id,task->lastRowSent + 1));
                                EventProducer::fireEvent(rde);
                                if ((task->lastRowSent+1)<(int64_t)rowCount) {
                                    return false;
                                }
                            } else if (copyDataResult == 0) {
                                LOG_DEBUG("copy in would block. skip ...");
                                return false;
                            } else if (copyDataResult==-1) {
                                THROW_EXC("copy data failed: " << conn.errorMessage());
                            }
                        } while((task->lastRowSent+1)<(int64_t)rowCount);
                        int copyEndResult = conn.putCopyEnd("");
                        if (copyEndResult == 0) {
                            LOG_DEBUG("putCopyEnd would block.");
                            return false;
                        }
                        task->lastRowSent = -1;
                    } else {
                        int copyEndResult = conn.putCopyEnd("");
                        if (copyEndResult == 0) {
                            LOG_DEBUG("putCopyEnd would block.");
                        } else if (copyEndResult == -1) {
                            THROW_EXC("putCopyEnd failed.");
                        } else {
                            fireStateChangeEvent(taskNo, "COPY_DONE");
                        }
                    }
                } else if (status == PGRES_COPY_OUT) {
                    fireStateChangeEvent(taskNo, "COPY_OUT");
                    LOG_TRACE("got copy out " << res);
                    char *data = NULL;
                    int size=0;
                    do {
                        size = conn.getCopyData(data,true);
                        if (size == 0) {
                            LOG_TRACE("copy out would block . skip further processing!!! [received " << task->data.size() << " bytes]");
                            shared_ptr<Event> rde(new ReceiveDataEvent(task->id,task->rowsReceived));
                            EventProducer::fireEvent(rde);
                            return false;
                        } else if (size==-2) {
                            THROW_EXC("error occurred when reading COPY OUT data");
                        } else if (size!=-1) {
                            LOG_TRACE("got data " << data);
                            task->data += string(data,size);
                            task->rowsReceived++;
                        }
                        if (data) {
                            PQfreemem(data);
                        }
                    } while(size!=-1);

                    LOG_TRACE("received " << task->data.size() << " bytes");
                    if (!task->data.empty()) {
                        shared_ptr<Event> rde(new ReceiveDataEvent(task->id,task->rowsReceived));
                        EventProducer::fireEvent(rde);
                        LOG_TRACE("save in handler " << task->data.size() << " bytes");
                        task->handler->handleCopyOut(task->queryNo,task->data);
                        LOG_TRACE("save in handler " << task->data.size() << " bytes done");
                    }
                    task->data.clear();

                } else if (status == PGRES_TUPLES_OK) {
                    fireStateChangeEvent(taskNo, "TUPLES");
                    LOG_DEBUG("get result ");
                    vector<pair<string,uint32_t>> columns = res->getColumns();
                    string status = res->getCommandStatus();
                    LOG_DEBUG("status is " << status);
                    task->handler->handleTuples(task->queryNo, columns);
                    task->queryNo++;
                } else if (status == PGRES_COMMAND_OK) {
                    fireStateChangeEvent(taskNo, "COMMAND");
                    string status = res->getCommandStatus();
                    LOG_DEBUG("command status is " << status);
                    task->queryNo++;
                }
                if (res != nullptr) {
                    delete task->result;
                    task->result = nullptr;
                }
            }
        }
        return false;
    }

    void AsyncQueryExecutor::fireStateChangeEvent(int taskNo, std::string state) {
        QueryTask *task = pImpl->tasks[taskNo];
        shared_ptr<Event> event(new ExecutionStateChangeEvent(task->id,state));
        EventProducer::fireEvent(event);
    }

    void AsyncQueryExecutor::fireEvent(EventType type, int taskNo) {
        LOG_TRACE("fire event " << taskNo);
        shared_ptr<Event> event(new Event(type));
        if (taskNo!=-1) {
            event->resultId = pImpl->tasks[taskNo]->id;
        }
        EventProducer::fireEvent(event);
    }

    void AsyncQueryExecutor::fireEvent(shared_ptr<Event> event, int taskNo) {
        EventProducer::fireEvent(event);
    }

    void AsyncQueryExecutor::cleanUp(int taskNo, string reason) {
        LOG_INFO("clean up task " << taskNo << "/" << pImpl->tasks.size());
        QueryTask *task = pImpl->tasks[taskNo];
        if (!reason.empty()) {
            fireStateChangeEvent(taskNo, reason);
        }
        PGConnection& conn = task->conn;
        PGcancel *pgCancel = conn.getCancel();
        if (pgCancel) {
            char buf[256];
            if (PQcancel(pgCancel,buf,256) == 0) {
                LOG_ERROR("cancelling failed " << buf);
            }
            PQfreeCancel(pgCancel);
        }
        PGresult *result = nullptr;
        do {
            LOG_TRACE("get result");
            result = conn.getResult();
            LOG_TRACE("get result = " << result);
            if (result) {
                PQclear(result);
                result = nullptr;
            }
        } while (result);
        //PQfinish(task->conn);
        conn.finish();
        //task->conn = nullptr;
    }

    AsyncQueryExecutorException::AsyncQueryExecutorException(string what, string sql): runtime_error(what) {
       query=sql;
    }
}
