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
    DECLARE_LOGGER("QueryExecution");

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
            Port *inPort = new Port("",depName);
            inPorts.push_back(inPort);
            //dependencies[depName] = nullptr;
        }
        Port *defaultPort = new Port("","");
        outPorts.push_back(defaultPort);
    }

    QueryExecution::~QueryExecution() {
        LOG_TRACE("delete query execution");
        for (auto& port:outPorts) {
            port->release();
        }
    }

    void QueryExecution::release() {
        LOG_INFO("use_count for result " << getName());
        for (auto& port:outPorts) {
            port->release();
        }
        for (auto& inPort:inPorts) {
            //LOG_INFO("dependency " << inPort->getName() << " use_count before release = " << dependency.second.use_count());
            //dependency.second.reset();
            inPort->release();
        }
        LOG_INFO("use_count injector = " << dependencyInjector.use_count());
        dependencyInjector.reset();
    }


    shared_ptr<TableData> QueryExecution::getResult(string shardId) {
        for (auto port:outPorts) {
            if (port->getName() == shardId) {
                return port->getResult();
            }
        }
        THROW_EXC("execution '" << getName() << "' does not have a result port '" << shardId << "'");
        /*
        if (results.find(shardId) == results.end()) {
            for (auto result:results) {
                LOG_INFO("result = " << result.first);
            }
            THROW_EXC("execution '" << getName() << "' does not have a result port '" << shardId << "' has " << results.size() << "candidates");
        }
        return results[shardId];
        */
    }

    void QueryExecution::setResult(string shardId, shared_ptr<TableData> result) {
        LOG_ERROR("set port '" << shardId << "' of " << getName() << " to " << result.get());
        bool found = false;
        for (auto port:outPorts) {
            if (port->getName() == shardId) {
                port->setResult(result);
                found = true;
                break;
            }
        }
        if (!found) {
            vector<string> portNames;
            for (auto port:outPorts) {
                portNames.push_back(port->getName());
            }
            THROW_EXC("no port '" << shardId << "' found in exec '" << getName() << "'. candidates: " << join(portNames,","));
        }
        bool done = true;
        for (auto port:outPorts) {
            done &= port->getResult().get() != nullptr;
        }
        if (done && state != QueryExecutionState::DONE) {
            LOG_DEBUG("all done. set state of " << getName() << " for port " << shardId)
            setState(QueryExecutionState::DONE);
        }
    }

    string QueryExecution::inject(string query, size_t copyThreshold) {
            assert(dependencyInjector != nullptr);
            LOG_DEBUG("called inject " << query << " di = " << dependencyInjector);
            return dependencyInjector->inject(query,inPorts,copyThreshold);
    }

    void QueryExecution::receive(string name, shared_ptr<TableData> data) {
        LOG_DEBUG("receive data " << name << " [depsize=" << inPorts.size() << "]");
        Port *dep = nullptr;
        for (auto inPort:inPorts) {
            if (inPort->getName() == name) {
                dep = inPort;
            }
        }
        if (dep == nullptr) {
            for (auto inPort:inPorts) {
                LOG_INFO("declared " << inPort->getName());
            }
            THROW_EXC("no dependency '" + name + "' declared");
        }
        dep->setResult(data);
        // check if complete
        bool complete = true;
        for (auto dep:inPorts) {
            complete &= (dep->getResult().get() != nullptr);
        }
        if (complete) {
            LOG_DEBUG("set state of '" << name << "' to complete as all dependencies are there");
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
            //THROW_EXC("state " << state << " is already set !");
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
            shared_ptr<Event> event(new Event(EventType::PROCESSED, getId()));
            fireEvent(event);
            shared_ptr<Event> e(new ExecutionStateChangeEvent(getId(), "DONE"));
            EventProducer::fireEvent(e);
        }
        this->state = state;
    }

    QueryExecutionState QueryExecution::getState() {
        return state;
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

    /*
    map<string,shared_ptr<TableData>>& QueryExecution::getDependencies() {
        return dependencies;
    }
    */

    shared_ptr<DependencyInjector> QueryExecution::getInjector() {
        return dependencyInjector;
    }

    void QueryExecution::schedule() {
        setState(QueryExecutionState::SCHEDULED);
    }

    bool QueryExecution::isTransition() {
        return false;
    }

    void QueryExecution::cleanUp() {}

    bool QueryExecution::isResourceAvailable() {
        return true;
    }

    void QueryExecution::stop() {}

    /*
    void QueryExecution::setPortNames(vector<std::string> portNames) {
        portIds.clear();
        for (auto& portName:portNames) {
            portIds[portName] = "";
        }
    }

    vector<string> QueryExecution::getPortNames() {
        vector<string> portNames;
        for (auto port:ports) {
            portNames.push_back(port->getName());
        }
        return portNames;
    }
    */

    deque<Port*>& QueryExecution::getOutPorts() {
        return outPorts;
    }

    deque<Port*>& QueryExecution::getInPorts() {
        return inPorts;
    }

    Port *QueryExecution::getOutPort(string name) {
        for (auto port:outPorts) {
            if (port->getName() == name) {
                return port;
            }
        }
        THROW_EXC("no port with name '" << name << "'");
    }

    void QueryExecution::addOutPort(Port *port) {
        outPorts.push_back(port);
    }

/*
    void QueryExecution::setPortId(string portName, string portId) {
        for (auto port:ports) {
            if (port->getName() == portName) {
                port->setId(portId);
                return;
            }
        }
        THROW_EXC("unknown port '" << portName << "'");
    }

    string QueryExecution::getPortId(string portName) {
        for (auto port:ports) {
            if (port->getName() == portName) {
                return port->getId();
            }
        }
        THROW_EXC("unknown port '" << portName << "'");
    }
*/
}

