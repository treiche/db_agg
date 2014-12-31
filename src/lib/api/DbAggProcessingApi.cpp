/*
 * DbAggProcessingApi.cpp
 *
 *  Created on: Dec 18, 2014
 *      Author: arnd
 */

#include <string>
#include <iostream>
#include <db_agg.h>
#include <thread>
#include <log4cplus/configurator.h>

#include "event/AsyncEvent.h"

extern "C" {
#include <jansson.h>
}

#include "DbAggProcessingApi.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {

DECLARE_LOGGER("DbAggProcessingApi");

class DbAggProcessingApi::XImpl: public AsyncEventListener {
public:
    Configuration config;
    Application *app;
    std::thread *t;
    virtual void handleEvent(std::shared_ptr<Event> event) override;
};

void DbAggProcessingApi::XImpl::handleEvent(shared_ptr<Event> event) {
    cout << "GOT EVENT" << endl;
}


DbAggProcessingApi::DbAggProcessingApi() {
    this->pImpl = new XImpl();
}

DbAggProcessingApi::~DbAggProcessingApi() {
    cout << "called ~DbAggProcessingApi" << endl;
    if (pImpl->t) {
        delete pImpl->t;
    }
    if (pImpl->app) {
        delete pImpl->app;
    }
    delete pImpl;
}


void DbAggProcessingApi::configure(string jsConfig) {
    PropertyConfigurator::doConfigure("/home/arnd/.db_agg/etc/log4cplus.properties");
    LOG_DEBUG("configure('" << jsConfig << "')");
    pImpl->config.fromJson(jsConfig);
    LOG_DEBUG("queryFile = " << pImpl->config.getQueryFile());
    LOG_DEBUG("disableCache = " << pImpl->config.getDisableCache());
    LOG_DEBUG("statementTimeout = " << pImpl->config.getStatementTimeout());
    map<string,string> externalSources = pImpl->config.getExternalSources();
    for (auto es:externalSources) {
        LOG_DEBUG("external source: " << es.first << " = " << es.second);
    }
    vector<string> externalExcelSources = pImpl->config.getExternalExcelSources();
    for (auto ees:externalExcelSources) {
        LOG_DEBUG("excel: " << ees);
    }
    LOG_DEBUG("database registry at " << pImpl->config.getDatabaseRegistryFile());
    LOG_DEBUG("configure logging: " << pImpl->config.getLogConf());
    cout << "disable cache: " << pImpl->config.getDisableCache() << endl;
    pImpl->app = new Application();
    pImpl->app->addAsyncListener(pImpl);
    LOG_DEBUG("bootstrap");
    pImpl->app->bootstrap(pImpl->config);
    LOG_DEBUG("run");
}

/*
bool DbAggProcessingApi::step() {
    bool done = pImpl->app->step();
    return done;
}
*/

bool DbAggProcessingApi::run(bool async) {
    if (async) {
        // std::thread t(&Application::run,pImpl->app);
        pImpl->t = new std::thread(&Application::run,pImpl->app);
        pImpl->t->detach();
    } else {
        bool done = pImpl->app->run();
    }
    return true;
}

string DbAggProcessingApi::receive() {
    LOG_DEBUG("receive()");
    shared_ptr<Event> event = pImpl->getEvent();
    if (event) {
        json_t *js = json_object();
        stringstream type;
        type << event->type;
        json_object_set(js,"type",json_string(type.str().c_str()));
        json_object_set(js,"exec",json_string(event->resultId.c_str()));
        if (event->type == EventType::EXECUTION_STATE_CHANGE) {
            ExecutionStateChangeEvent *e = (ExecutionStateChangeEvent*)event.get();
            stringstream state;
            state << e->state;
            json_object_set(js,"state",json_string(state.str().c_str()));
        } else if (event->type == EventType::APPLICATION_FAILED) {
            ApplicationFailedEvent *e = (ApplicationFailedEvent*)event.get();
            json_object_set(js,"reason",json_string(e->reason.c_str()));
        } else if (event->type == EventType::QUERY_PREPARED) {
            QueryPreparedEvent *e = (QueryPreparedEvent*)event.get();
            json_object_set(js,"graph",json_string(e->executionPlanFile.c_str()));
        } else if (event->type == EventType::RECEIVE_DATA) {
            ReceiveDataEvent *e = (ReceiveDataEvent*)event.get();
            json_object_set(js,"rows",json_integer(e->rowsReceived));
        } else if (event->type == EventType::SENT_DATA) {
            SentDataEvent *e = (SentDataEvent*)event.get();
            json_object_set(js,"rowsSent",json_integer(e->rowsSent));
        } else if (event->type == EventType::CACHE_LOADED) {
            CacheLoadEvent *e = (CacheLoadEvent*)event.get();
            json_object_set(js,"lastDuration",json_integer(e->lastDuration));
            json_object_set(js,"lastExecuted",json_string(e->lastExecuted.to_string().c_str()));
            json_object_set(js,"lastRowsReceived",json_integer(e->lastRowsReceived));
        } else if (event->type == EventType::APPLICATION_INITIALIZED) {
            ExecutionGraph& g = pImpl->app->getExecutionGraph();
            vector<Query*> queries = g.getQueries();
            json_t *jsQueries = json_array();
            json_object_set(js,"queries",jsQueries);
            //json << ", \"queries\": [";
            for (size_t idx = 0; idx < queries.size(); idx++) {
                json_t *jsQuery = json_object();
                json_array_append(jsQueries,jsQuery);
                json_object_set(jsQuery,"name",json_string(queries[idx]->getName().c_str()));
                json_object_set(jsQuery,"id",json_string(queries[idx]->getId().c_str()));
                json_t *jsExecs = json_array();
                json_object_set(jsQuery,"executions",jsExecs);
                auto execs = g.getQueryExecutions(queries[idx]);
                for (size_t idx2 = 0; idx2 < execs.size(); idx2++) {
                    auto exec = execs[idx2];
                    json_t *jsExec = json_object();
                    json_array_append(jsExecs,jsExec);
                    json_object_set(jsExec,"name",json_string(exec->getName().c_str()));
                    json_object_set(jsExec,"id",json_string(exec->getId().c_str()));
                    string url = "no url";
                    if (exec->getUrl() != nullptr) {
                        url = exec->getUrl()->getUrl(false,false,false);
                    }
                    json_object_set(jsExec,"url",json_string(url.c_str()));
                }
            }
        }
        return json_dumps(js,0);
    }
    return "";
}

void DbAggProcessingApi::stop() {
    pImpl->app->stop();
}

/*
string DbAggProcessingApi::receive() {
    LOG_DEBUG("receive()");
    shared_ptr<Event> event = pImpl->getEvent();
    if (event) {
        stringstream json;
        json << "{\"type\":\"" << event->type << "\"";
        json << ", \"exec\":\"" << event->resultId << "\"";

        if (event->type == EventType::EXECUTION_STATE_CHANGE) {
            ExecutionStateChangeEvent *esce = (ExecutionStateChangeEvent*)event.get();
            json << ", \"state\":\"" << esce->state << "\"";
        } else if (event->type == EventType::APPLICATION_FAILED) {
            ApplicationFailedEvent *efe = (ApplicationFailedEvent*)event.get();
            json << ", \"reason\":\"" << efe->reason << "\"";
        } else if (event->type == EventType::QUERY_PREPARED) {
            QueryPreparedEvent *esce = (QueryPreparedEvent*)event.get();
            json << ", \"graph\":\"" << esce->executionPlanFile << "\"";
        } else if (event->type == EventType::RECEIVE_DATA) {
            ReceiveDataEvent *esce = (ReceiveDataEvent*)event.get();
            json << ", \"rows\":" << esce->rowsReceived << "";
        } else if (event->type == EventType::SENT_DATA) {
            SentDataEvent *esce = (SentDataEvent*)event.get();
            json << ", \"rows\":\"" << esce->rowsSent << "\"";
        } else if (event->type == EventType::CACHE_LOADED) {
            CacheLoadEvent *esce = (CacheLoadEvent*)event.get();
            json << ", \"lastDuration\":" << esce->lastDuration << "";
            json << ", \"lastExecuted\":\"" << esce->lastExecuted.to_string() << "\"";
            json << ", \"lastRowsReceived\":" << esce->lastRowsReceived << "";
        } else if (event->type == EventType::APPLICATION_INITIALIZED) {
            ExecutionGraph& g = pImpl->app->getExecutionGraph();
            vector<Query*> queries = g.getQueries();
            json << ", \"queries\": [";
            for (size_t idx = 0; idx < queries.size(); idx++) {
                json << "{ \"name\":\"" << queries[idx]->getName() << "\"";
                json << ", \"id\":\"" << queries[idx]->getId() << "\"";
                json << ", \"executions\":[";
                auto execs = g.getQueryExecutions(queries[idx]);
                for (size_t idx2 = 0; idx2 < execs.size(); idx2++) {
                    auto exec = execs[idx2];
                    json << "{ \"name\":\"" << exec->getName() << "\"";
                    json << ", \"id\":\"" << exec->getId() << "\"";
                    json << "}";
                    if (idx2 < execs.size() -1) {
                        json << ",";
                    }
                }
                json << "]";
                json << "}";
                if (idx < queries.size() -1) {
                    json << ",";
                }
            }
            json << "]";
        }
        json << "}";
        return json.str();
    }
    return "";
}

*/
}
