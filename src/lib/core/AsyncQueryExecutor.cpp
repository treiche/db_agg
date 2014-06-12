#include "AsyncQueryExecutor.h"

#include "postgres/PGConnection.h"
#include "postgres/PGResult.h"

#include <log4cplus/logger.h>
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

    enum class QueryExecutionState {
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
        QueryExecutionState state;
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
        LOG4CPLUS_DEBUG(LOG, "delete async query executor");
        for (unsigned int taskNo = 0;taskNo < this->pImpl->tasks.size();taskNo++) {
            QueryTask *task = this->pImpl->tasks[taskNo];
            LOG4CPLUS_DEBUG(LOG, "delete connection at " << *task->conn);
            if (task->conn.connected()) {
                LOG4CPLUS_DEBUG(LOG, "release connection " << task->connectionUrl);
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
        qt->state = QueryExecutionState::INITIAL;
        qt->handler = handler;
        this->pImpl->tasks.push_back(qt);
    }

    bool AsyncQueryExecutor::process() {
        LOG4CPLUS_DEBUG(LOG, "start processing with " << this->pImpl->tasks.size() << " queries");
        // PQinitOpenSSL(1,1);
        bool done = false;
        fireEvent(EventType::INITIALIZE,-1);
        try {
            done = this->loop();
        } catch(CancelException& ce) {
            LOG4CPLUS_ERROR(LOG, "query execution canceled:" << endl);
            cleanUp("CANCEL");
            throw ce;
        } catch(AsyncQueryExecutorException& aqee) {
            LOG4CPLUS_ERROR(LOG, "query execution failed: " << aqee.what());
            LOG4CPLUS_ERROR(LOG, "query:\n" << aqee.getQuery());
            cleanUp("");
            throw aqee;
        } catch(runtime_error& re) {
            LOG4CPLUS_ERROR(LOG, "query execution failed: " << re.what());
            cleanUp("");
            throw re;
        } catch(exception& re) {
            LOG4CPLUS_ERROR(LOG, "query execution failed: " << re.what());
            cleanUp("");
            throw re;
        } catch(...) {
            // TODO: find out how this works
            exception_ptr e = current_exception();
            LOG4CPLUS_ERROR(LOG, "query execution failed:");
            cleanUp("");
            throw runtime_error("caught exception and stopped all tasks");
        }
        return done;
    }

    void AsyncQueryExecutor::cleanUp(string reason) {
        for (size_t idx = 0; idx < this->pImpl->tasks.size(); idx++) {
            cleanUp(idx, reason);
        }
    }

    bool AsyncQueryExecutor::loop() {
        LOG4CPLUS_DEBUG(LOG, "called loop with " << this->pImpl->tasks.size() << " tasks scheduled");
        bool tasksDone = true;
        for (size_t taskNo = 0; taskNo < this->pImpl->tasks.size(); taskNo++) {
            LOG4CPLUS_TRACE(LOG, "process task " << taskNo);
            bool taskDone = false;
            if (pImpl->cancelRequest) {
                throw CancelException("cancellation requested");
            }
            try {
                taskDone = this->processTask(taskNo);
            } catch(AsyncQueryExecutorException& e) {
                QueryTask *task = pImpl->tasks[taskNo];
                task->state = QueryExecutionState::FAILED;
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
        if (task->state == QueryExecutionState::INITIAL) {
            bool connectAsync = false;
            string connectionUrl = task->connectionUrl; // + " options='--statement-timeout=30000 --client-min-messages=debug1'";
            if (!connectAsync) {
                task->conn = PGConnection::connectDb(connectionUrl.c_str());
                LOG4CPLUS_INFO(LOG,"connecting to " << maskPassword(task->connectionUrl));
                if (task->conn.connected()) {
                    task->state = QueryExecutionState::CONNECTED;
                    fireStateChangeEvent(taskNo, "CONNECTED");
                } else {
                    LOG4CPLUS_ERROR(LOG,"connecting to " << task->connectionUrl << " failed");
                    throw runtime_error("connecting to " + task->connectionUrl + " failed");
                }
            } else {
                task->conn = PGConnection::connectStart(connectionUrl.c_str());
                if (task->conn.setnonblocking(1) == -1) {
                    LOG4CPLUS_ERROR(LOG, "unable to set non blocking mode");
                }
                if (task->conn.isnonblocking() != 1) {
                    LOG4CPLUS_ERROR(LOG, "set non blocking mode failed");
                }
                task->state = QueryExecutionState::CONNECTING;
                fireStateChangeEvent(taskNo, "CONNECTING");
            }
        } else if (task->state == QueryExecutionState::CONNECTING) {
            PostgresPollingStatusType ppst = conn.connectPoll();
            if (ppst == PGRES_POLLING_FAILED) {
                LOG4CPLUS_ERROR(LOG, "failed to connect to " << task->connectionUrl);
                LOG4CPLUS_ERROR(LOG, "postgres message " << conn.errorMessage());
                throw runtime_error("connection failed");
            }
            if (ppst == PGRES_POLLING_OK) {
                task->state = QueryExecutionState::CONNECTED;
                fireStateChangeEvent(taskNo, "CONNECTED");
            }
        } else if (task->state == QueryExecutionState::CONNECTED) {
            LOG4CPLUS_DEBUG(LOG, "about to send query");
            if (!task->conn.sendQuery(task->query)) {
                string message = task->conn.errorMessage();
                LOG4CPLUS_ERROR(LOG, "sending query  " << task->query << " failed. message = " << message);
                throw runtime_error("sending query  " + task->query + " failed. message = " + message + " connection = " + task->connectionUrl);
            }
            task->state = QueryExecutionState::QUERY_SENDING;
            fireStateChangeEvent(taskNo, "QUERY_SENDING");
        } else if (task->state == QueryExecutionState::QUERY_SENDING) {
            int ret = conn.flush();
            LOG4CPLUS_DEBUG(LOG, "flush returned " << ret);
            if (ret==1) {
                task->state = QueryExecutionState::QUERY_SENDING;
            } else {
                task->state = QueryExecutionState::QUERY_SENT;
                fireStateChangeEvent(taskNo, "QUERY_SENT");
            }
        } else if (task->state == QueryExecutionState::QUERY_SENT) {
            int cir = conn.consumeInput();
            LOG4CPLUS_TRACE(LOG, "consume input returned " << cir);
            if (!cir) {
                string message = conn.errorMessage();
                LOG4CPLUS_ERROR(LOG, "consume input " << task->query << " failed. message = " << message);
                throw string("query failure ") + message;
            }
            if (conn.isBusy()==0) {
                LOG4CPLUS_TRACE(LOG, "is not busy");

                PGResult *res = task->result;
                if (res==nullptr) {
                    task->result = new PGResult();
                    task->result->initialize(conn.getResult());
                    res = task->result;
                }

                LOG4CPLUS_DEBUG(LOG, "RESULT:" << res);
                if (!*res) {
                    task->done = true;
                    task->state = QueryExecutionState::DONE;
                    fireStateChangeEvent(taskNo, "DONE");
                    LOG4CPLUS_DEBUG(LOG, "RESULT IS NULL");
                    fireEvent(EventType::PROCESSED, taskNo);
                    return true;
                }
                ExecStatusType status = res->getStatusType();
                LOG4CPLUS_DEBUG(LOG, "result status is " << status);
                string ss = res->getStatusTypeAsString(status);
                LOG4CPLUS_DEBUG(LOG, "result status is " << ss);
                if (status == PGRES_FATAL_ERROR) {
                    LOG4CPLUS_ERROR(LOG, "get result returned fatal error");
                    string em = conn.errorMessage();
                    LOG4CPLUS_ERROR(LOG, "message = " << em);
                    LOG4CPLUS_DEBUG(LOG, "query =\n" << task->query);
                    throw AsyncQueryExecutorException(em, task->query);
                } else if (status == PGRES_COPY_IN) {
                    fireStateChangeEvent(taskNo, "COPY_IN");
                    LOG4CPLUS_DEBUG(LOG, "got copy in res=" << res);
                    int copyDataResult = 0;
                    uint64_t rowCount = task->handler->getRowCount(task->queryNo);
                    LOG4CPLUS_DEBUG(LOG, "rowCount = " << rowCount);
                    uint64_t rowsPerChunk = 100;
                    if (rowCount>0) {
                        do {
                            if (pImpl->cancelRequest) {
                                throw CancelException("cancled while copy in");
                            }
                            LOG4CPLUS_TRACE(LOG, "get row " << (task->lastRowSent+1));
                            uint64_t rowsRead = 0;
                            string data = task->handler->handleCopyIn(task->queryNo, task->lastRowSent + 1, rowsPerChunk, rowsRead);
                            LOG4CPLUS_TRACE(LOG, "data = " << data);
                            LOG4CPLUS_TRACE(LOG, "rowsRead = " << rowsRead);
                            LOG4CPLUS_TRACE(LOG, "lastRowSent = " << (task->lastRowSent+1));
                            copyDataResult = conn.putCopyData(data);
                            if (copyDataResult == 1) {
                                task->lastRowSent += rowsRead;
                                SentDataEvent rde{task->id,task->lastRowSent + 1};
                                EventProducer::fireEvent(rde);
                                if ((task->lastRowSent+1)<rowCount) {
                                    return false;
                                }
                            } else if (copyDataResult == 0) {
                                LOG4CPLUS_ERROR(LOG, "copy in would block. skip ...");
                                return false;
                            } else if (copyDataResult==-1) {
                                LOG4CPLUS_ERROR(LOG, "copy data failed" << conn.errorMessage());
                                throw runtime_error("copy data failed:" + conn.errorMessage());
                            }
                        } while((task->lastRowSent+1)<rowCount);
                        int copyEndResult = conn.putCopyEnd("");
                        if (copyEndResult == 0) {
                            LOG4CPLUS_ERROR(LOG, "putCopyEnd would block.");
                            return false;
                        }
                        task->lastRowSent = -1;
                    } else {
                        int copyEndResult = conn.putCopyEnd("");
                        if (copyEndResult == 0) {
                            LOG4CPLUS_ERROR(LOG, "putCopyEnd would block.");
                        }
                    }
                } else if (status == PGRES_COPY_OUT) {
                    fireStateChangeEvent(taskNo, "COPY_OUT");
                    LOG4CPLUS_TRACE(LOG, "got copy out " << res);
                    char *data = NULL;
                    int size=0;
                    do {
                        size = conn.getCopyData(data,true);
                        if (size == 0) {
                            LOG4CPLUS_TRACE(LOG, "copy out would block . skip further processing!!! [received " << task->data.size() << " bytes]");
                            ReceiveDataEvent rde{task->id,task->rowsReceived};
                            EventProducer::fireEvent(rde);
                            return false;
                        } else if (size==-2) {
                            LOG4CPLUS_ERROR(LOG, "an error occured !!! ");
                            throw runtime_error("error occurred when reading COPY OUT data");
                        } else if (size!=-1) {
                            LOG4CPLUS_TRACE(LOG, "got data " << data);
                            task->data += string(data,size);
                            task->rowsReceived++;
                        }
                        if (data) {
                            PQfreemem(data);
                        }
                    } while(size!=-1);

                    LOG4CPLUS_TRACE(LOG, "received " << task->data.size() << " bytes");
                    if (!task->data.empty()) {
                        ReceiveDataEvent rde{task->id,task->rowsReceived};
                        EventProducer::fireEvent(rde);
                        LOG4CPLUS_TRACE(LOG, "save in handler " << task->data.size() << " bytes");
                        task->handler->handleCopyOut(task->queryNo,task->data);
                        LOG4CPLUS_TRACE(LOG, "save in handler " << task->data.size() << " bytes done");
                    }
                    task->data.clear();

                } else if (status == PGRES_TUPLES_OK) {
                    fireStateChangeEvent(taskNo, "TUPLES");
                    LOG4CPLUS_DEBUG(LOG, "get result ");
                    vector<pair<string,uint32_t>> columns = res->getColumns();
                    string status = res->getCommandStatus();
                    LOG4CPLUS_DEBUG(LOG, "status is " << status);
                    task->handler->handleTuples(task->queryNo, columns);
                    task->queryNo++;
                } else if (status == PGRES_COMMAND_OK) {
                    fireStateChangeEvent(taskNo, "COMMAND");
                    string status = res->getCommandStatus();
                    LOG4CPLUS_DEBUG(LOG, "command status is " << status);
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
        ExecutionStateChangeEvent event{task->id,state};
        EventProducer::fireEvent(event);
    }

    void AsyncQueryExecutor::fireEvent(EventType type, int taskNo) {
        LOG4CPLUS_TRACE(LOG, "fire event " << taskNo);
        Event event{type};
        if (taskNo!=-1) {
            event.resultId = pImpl->tasks[taskNo]->id;
        }
        EventProducer::fireEvent(event);
    }

    void AsyncQueryExecutor::fireEvent(Event& event, int taskNo) {
        EventProducer::fireEvent(event);
    }

    void AsyncQueryExecutor::cleanUp(int taskNo, string reason) {
        LOG4CPLUS_INFO(LOG, "clean up task " << taskNo << "/" << pImpl->tasks.size());
        QueryTask *task = pImpl->tasks[taskNo];
        if (!reason.empty()) {
            fireStateChangeEvent(taskNo, reason);
        }
        PGConnection& conn = task->conn;
        PGcancel *pgCancel = conn.getCancel();
        if (pgCancel) {
            char buf[256];
            if (PQcancel(pgCancel,buf,256) == 0) {
                LOG4CPLUS_ERROR(LOG, "cancelling failed " << buf);
            }
            PQfreeCancel(pgCancel);
        }
        PGresult *result = nullptr;
        do {
            LOG4CPLUS_TRACE(LOG, "get result");
            result = conn.getResult();
            LOG4CPLUS_TRACE(LOG, "get result = " << result);
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
