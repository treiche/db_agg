/*
 * QueryExecution.cpp
 *
 *  Created on: Dec 29, 2013
 *      Author: arnd
 */

#include "QueryExecution.h"

#include "utils/logging.h"

#include "type/oids.h"
#include "type/TypeRegistry.h"
#include "utils/RegExp.h"
#include "utils/string.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
    static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("QueryExecution"));

    QueryExecution::QueryExecution() {}

    void QueryExecution::init(
            string name,
            string id,
            shared_ptr<Url> url,
            string sql,
            vector<string> depNames,
            shared_ptr<DependencyInjector> dependencyInjector,
            vector<string> arguments) {
        this->id = id;
        this->url = url;
        this->sql = sql;
        this->name = name;
        this->dependencyInjector = dependencyInjector;
        this->arguments = arguments;
        if (depNames.empty()) {
            setState(QueryExecutionState::COMPLETE);
        }
        for (auto& depName:depNames) {
            dependencies[depName] = nullptr;
        }
    }

    QueryExecution::~QueryExecution() {
        LOG_TRACE("delete query execution");
        for (auto& result:results) {
            if (result.second.get() != nullptr) {
                result.second.reset();
            }
        }
    }

    void QueryExecution::release() {
        LOG_INFO("use_count for result " << getName());
        for (auto& result:results) {
            LOG_INFO("result use_count before release = " << result.second.use_count() << " unique = " << result.second.unique());
            result.second.reset();
        }
        for (auto& dependency:dependencies) {
            LOG_INFO("dependency " << dependency.first << " use_count before release = " << dependency.second.use_count());
            dependency.second.reset();
        }
        LOG_INFO("use_count injector = " << dependencyInjector.use_count());
        dependencyInjector.reset();
    }


    shared_ptr<TableData> QueryExecution::getResult(string shardId) {
        if (results.find(shardId) == results.end()) {
            for (auto result:results) {
                LOG_INFO("result = " << result.first);
            }
            THROW_EXC("execution '" << getName() << "' does not have a result port '" << shardId << "' has " << results.size() << "candidates");
        }
        return results[shardId];
    }

    void QueryExecution::setResult(string shardId, shared_ptr<TableData> result) {
        LOG_DEBUG("set result to " << result);
        results[shardId] = result;
    }

    string QueryExecution::inject(string query, size_t copyThreshold) {
            assert(dependencyInjector != nullptr);
            LOG_DEBUG("called inject " << query << " di = " << dependencyInjector);
            return dependencyInjector->inject(query,dependencies,copyThreshold);
    }

    void QueryExecution::receive(string name, shared_ptr<TableData> data) {
        LOG_DEBUG("receive data " << name << " [depsize=" << dependencies.size() << "]");
        if (dependencies.find(name) == dependencies.end()) {
            for (auto dep:dependencies) {
                LOG_INFO("declared " << dep.first);
            }
            THROW_EXC("no dependency '" + name + "' declared");
        }
        dependencies[name] = data;
        // check if complete
        bool complete = true;
        for (auto& dep:dependencies) {
            complete &= (dep.second!=nullptr);
        }
        if (complete) {
            LOG_ERROR("set state of '" << name << "' to complete as all dependencies are there");
            setState(QueryExecutionState::COMPLETE);
        }
    }

    void QueryExecution::addChannel(Channel* channel) {
        this->channels.push_back(channel);
    }

    std::ostream& operator<<(std::ostream& cout,const QueryExecution& qe) {
        cout << "QueryExecution[sql=" << qe.sql << "]";
        return cout;
    }

    std::ostream& operator<<(std::ostream& cout,const QueryExecutionState state) {
        cout << "State[";
        switch(state) {
            case QueryExecutionState::INITIAL:
                cout << "INITIAL";
                break;
            case QueryExecutionState::COMPLETE:
                cout << "COMPLETE";
                break;
            case QueryExecutionState::SCHEDULED:
                cout << "SCHEDULED";
                break;
            case QueryExecutionState::RUNNING:
                cout << "RUNNING";
                break;
            case QueryExecutionState::STOPPED:
                cout << "STOPPED";
                break;
            case QueryExecutionState::DONE:
                cout << "DONE";
                break;
            default:
                cout << "UNKNOWN";
        }
        cout << "]";
        return cout;
    }

    void QueryExecution::setId(string id) {
        this->id = id;
    }

    string QueryExecution::getId() {
        return id;
    }

    string QueryExecution::getName() {
        return name;
    }

    void QueryExecution::setState(QueryExecutionState state) {
        if (this->state == state) {
            THROW_EXC("state " << state << " is already set !");
            return;
        }

        if (state == QueryExecutionState::SCHEDULED && this->state != QueryExecutionState::COMPLETE) {
            THROW_EXC("cannot set SCHEDULED state as execution is not COMPLETE: current state = " << state);
        }

        if (this->state == QueryExecutionState::DONE) {
            THROW_EXC("execution is already in final state DONE");
        }

        if (this->state == QueryExecutionState::STOPPED) {
            THROW_EXC("execution is already in final state STOPPED");
        }

        if (state == QueryExecutionState::SCHEDULED) {
            startTime = std::chrono::system_clock::now();
        } else if (state == QueryExecutionState::DONE) {
            endTime = std::chrono::system_clock::now();
        }
        this->state = state;
    }

    QueryExecutionState QueryExecution::getState() {
        return state;
    }

    bool QueryExecution::isComplete() {
        return state == QueryExecutionState::COMPLETE;
    }

    void QueryExecution::setScheduled() {
        setState(QueryExecutionState::SCHEDULED);
    }

    bool QueryExecution::isScheduled() {
        return state == QueryExecutionState::SCHEDULED;
    }

    void QueryExecution::setDone() {
        setState(QueryExecutionState::DONE);
    }

    bool QueryExecution::isDone() {
        return state == QueryExecutionState::DONE;
    }

    size_t QueryExecution::getDuration() {
        std::chrono::system_clock::duration duration = endTime - startTime;
        return duration.count();
    }

    shared_ptr<Url> QueryExecution::getUrl() {
        return url;
    }

    string QueryExecution::getSql() {
        return sql;
    }

    vector<Channel*> QueryExecution::getChannels() {
        return channels;
    }

    vector<string>& QueryExecution::getArguments() {
        return arguments;
    }

    map<string,shared_ptr<TableData>>& QueryExecution::getDependencies() {
        return dependencies;
    }

    shared_ptr<DependencyInjector> QueryExecution::getInjector() {
        return dependencyInjector;
    }

    void QueryExecution::schedule() {
        setScheduled();
    }

    bool QueryExecution::isTransition() {
        return false;
    }

    void QueryExecution::cleanUp() {}

    bool QueryExecution::isResourceAvailable() {
        return true;
    }

    void QueryExecution::stop() {}

    void QueryExecution::setPortNames(vector<std::string> portNames) {
        portIds.clear();
        for (auto& portName:portNames) {
            portIds[portName] = "";
        }
    }

    vector<string> QueryExecution::getPortNames() {
        vector<string> portNames;
        for (auto port:portIds) {
            portNames.push_back(port.first);
        }
        return portNames;
    }

    void QueryExecution::setPortId(string portName, string portId) {
        if (portIds.find(portName) == portIds.end()) {
            THROW_EXC("unknown port '" << portName << "'");
        }
        portIds[portName] = portId;
    }

    string QueryExecution::getPortId(string portName) {
        if (portIds.find(portName) == portIds.end()) {
            THROW_EXC("unknown port '" << portName << "'");
        }
        return portIds[portName];
    }

}

