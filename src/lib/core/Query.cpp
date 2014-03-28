#include "core/Query.h"

#include <log4cplus/logger.h>
#include <map>
#include <sstream>

using namespace std;
using namespace log4cplus;
namespace db_agg {
    static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("Query"));
/*
    Query::Query(string name, string query, set<string> usedNamespaces)
      : Query(name, query, usedNamespaces, -1, "") {}
*/
    /*!
     @todo test todo annotation
     */
    Query::Query(string id, Locator locator, string query, string formattedQuery, set<string> usedNamespaces, bool external) {
        this->id = id;
        LOG4CPLUS_DEBUG(LOG, "query '" << locator.getQName() << "' id = " << id);
        this->locator = locator;
        this->query = query;
        this->formattedQuery = formattedQuery;
        this->usedNamespaces = usedNamespaces;
        this->external = external;
    }

    void Query::addDependency(Locator locator, string alias) {
        Dependency dep{Locator(locator),alias};
        dependencies.push_back(dep);
    }

    string Query::getQuery() {
        return query;
    }

    string Query::getFormattedQuery() {
        return formattedQuery;
    }

    string Query::toString() {
        stringstream ss;
        ss << "Query[";
        ss << "id=" << id << ",";
        ss << "locator = " << locator.getQName() << ",";
        ss << "query=" << query << ",";
        ss << "usedNamespaces=";
        unsigned int cnt=0;
        for (set<string>::iterator it=usedNamespaces.begin(); it!=usedNamespaces.end(); ++it) {
            ss << (*it);
            if (cnt<usedNamespaces.size()-1) {
                ss << ",";
            }
            cnt++;
        }
        ss << ",dependencies = [";
        for (auto &dep:dependencies) {
            ss << dep.locator.getQName();
            ss << " ";
        }
        ss << "]";
        return ss.str();
    }

    void Query::addQueryExecution(QueryExecution queryExecution) {
        executions.push_back(queryExecution);
    }

    QueryExecution* Query::getQueryExecution(size_t shardId) {
        if (shardId >= executions.size()) {
            throw runtime_error("out of range");
        }
        return &executions[shardId];
    }

    std::deque<QueryExecution>& Query::getQueryExecutions() {
        return executions;
    }
}
