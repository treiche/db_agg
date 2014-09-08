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
        return results[shardId];
    }

    void QueryExecution::setResult(string shardId, shared_ptr<TableData> result) {
        LOG_DEBUG("set result to " << result);
        results[shardId] = result;
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

    bool QueryExecution::isTransition() {
    	return false;
    }

    void QueryExecution::cleanUp() {}

    bool QueryExecution::isResourceAvailable() {
        return true;
    }

    void QueryExecution::stop() {}

}

