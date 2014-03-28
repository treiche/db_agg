#ifndef QUERY_H_
#define QUERY_H_

#include <set>
#include <string>
#include <vector>
#include <deque>

#include "core/Locator.h"
#include "core/QueryExecution.h"

namespace db_agg {

class Query;

class Dependency {
public:
    Locator locator;
    std::string alias;
    Query *sourceQuery;
};

class Query {
    std::string id;
    Locator locator;
    std::string query;
    std::string formattedQuery;
    std::set<std::string> usedNamespaces;
    std::deque<Dependency> dependencies;
    std::deque<QueryExecution> executions;
    std::string databaseId;
    bool external = false;
public:
    Query() {}
    Query(std::string id, Locator locator, std::string query, std::string formattedQuery, std::set<std::string> usedNamespaces, bool external = false);
    void addDependency(Locator locator, std::string alias);
    std::deque<Dependency>& getDependencies() {
        return dependencies;
    }
    void setDatabaseId(std::string databaseId) {
        this->databaseId = databaseId;
    }
    std::string getDatabaseId() {
        return databaseId;
    }
    Locator& getLocator() {
        return locator;
    }
    std::string getQuery();
    std::string getFormattedQuery();
    std::string toString();
    const std::set<std::string>& getUsedNamespaces() const {
        return usedNamespaces;
    }

    short getShardId() const {
        return locator.getShardId();
    }

    std::string getEnvironment() const {
        return locator.getEnvironment();
    }
/*
    void setEnvironment(std::string e) {
        environment=e;
    }
*/
    std::string getId() {
        return id;
    }

    std::string getName() {
        return locator.getName();
    }

    bool isExternal() {
        return external;
    }

    void addQueryExecution(QueryExecution qe);
    QueryExecution *getQueryExecution(size_t shardId);
    std::deque<QueryExecution>& getQueryExecutions();
};


}


#endif /* QUERY_H_ */
