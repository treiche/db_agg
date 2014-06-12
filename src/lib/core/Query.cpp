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
    Query::Query(string id, string type, Locator locator, string query, string formattedQuery, set<string> usedNamespaces) {
        this->id = id;
        LOG4CPLUS_DEBUG(LOG, "query '" << locator.getQName() << "' id = " << id);
        this->locator = locator;
        this->query = query;
        this->formattedQuery = formattedQuery;
        this->usedNamespaces = usedNamespaces;
        this->type = type;
    }

    void Query::addDependency(Locator locator, string alias) {
        Dependency dep{Locator(locator),alias};
        dependencies.push_back(dep);
    }

    deque<Dependency>& Query::getDependencies() {
        return dependencies;
    }

    void Query::setDatabaseId(string databaseId) {
        this->databaseId = databaseId;
    }

    string Query::getDatabaseId() {
        return databaseId;
    }

    Locator& Query::getLocator() {
        return locator;
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

    std::set<std::string>& Query::getUsedNamespaces() {
       return usedNamespaces;
    }

    short Query::getShardId() {
        return locator.getShardId();
    }

    string Query::getEnvironment() {
        return locator.getEnvironment();
    }

    string Query::getId() {
        return id;
    }

    string Query::getName() {
        return locator.getName();
    }

    string Query::getType() {
        return type;
    }

    void Query::setType(string queryType) {
        this->type = queryType;
    }

    void Query::setArguments(vector<string> arguments) {
        this->arguments = arguments;
    }

    vector<string>& Query::getArguments() {
        return arguments;
    }

    void Query::setMetaData(map<string,string> metaData) {
        this->metaData = metaData;
    }

    string Query::getMetaData(string name, string fallback) {
        if (metaData.find(name) == metaData.end()) {
            return fallback;
        }
        return metaData[name];
    }

}
