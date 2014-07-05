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
        for (auto& depName:depNames) {
            dependencies[depName] = nullptr;
        }
    }

    QueryExecution::~QueryExecution() {
        LOG_TRACE("delete query execution");
        if (data!=nullptr) {
            data.reset();
        }
    }

    void QueryExecution::release() {
        LOG_TRACE("use count before release = " << data.use_count());
        data.reset();
        LOG_TRACE("use count after release = " << data.use_count());
    }


    shared_ptr<TableData> QueryExecution::getResult() {
        return data;
    }

    void QueryExecution::setResult(shared_ptr<TableData> data) {
        LOG_DEBUG("set result to " << data);
        this->data = data;
    }

    bool QueryExecution::isComplete() {
        bool complete = true;
        for (auto& dep:dependencies) {
            complete &= (dep.second!=nullptr);
        }
        return complete;
    }

    string QueryExecution::inject(string query, size_t copyThreshold) {
            assert(dependencyInjector != nullptr);
            LOG_DEBUG("called inject " << query << " di = " << dependencyInjector);
            return dependencyInjector->inject(query,dependencies,copyThreshold);
    }

    void QueryExecution::receive(string name, shared_ptr<TableData> data) {
        LOG_DEBUG("receive data " << data);
        if (dependencies.find(name)==dependencies.end()) {
            throw runtime_error("no dependency '" + name + "' declared");
        }
        dependencies[name] = data;
    }

    void QueryExecution::addChannel(Channel* channel) {
        this->channels.push_back(channel);
    }

    std::ostream& operator<<(std::ostream& cout,const QueryExecution& qe) {
        cout << "QueryExecution[sql=" << qe.sql << "]";
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

    shared_ptr<TableData> QueryExecution::getData() {
        return data;
    }

    void QueryExecution::setData(std::shared_ptr<TableData> data) {
        this->data = data;
    }

    void QueryExecution::setScheduled() {
        scheduled = true;
        startTime = std::chrono::system_clock::now();
    }

    bool QueryExecution::isScheduled() {
        return scheduled;
    }

    void QueryExecution::setDone() {
        endTime = std::chrono::system_clock::now();
        done = true;
    }

    bool QueryExecution::isDone() {
        return done;
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

    void QueryExecution::cleanUp() {}

    bool QueryExecution::isResourceAvailable() {
        return true;
    }

    void QueryExecution::stop() {}

}

