#ifndef QUERY_H_
#define QUERY_H_

#include <set>
#include <string>
#include <vector>
#include <deque>

#include "core/Locator.h"
#include "core/QueryExecution.h"
#include "core/Dependency.h"

namespace db_agg {

class Query {
private:
    std::string id;
    Locator locator;
    std::string query;
    std::string type;
    std::string formattedQuery;
    std::string normalizedQuery;
    std::set<std::string> usedNamespaces;
    std::deque<Dependency> dependencies;
    std::string databaseId;
    std::vector<std::string> arguments;
    std::map<std::string, std::string> metaData;
    std::shared_ptr<Url> url;
public:
    Query() {
    }
    Query(std::string id, std::string type, Locator locator, std::string query,
            std::string formattedQuery, std::string normalizedQuery,
            std::set<std::string> usedNamespaces);
    void addDependency(Locator locator, std::string alias);
    std::deque<Dependency>& getDependencies();
    void setDatabaseId(std::string databaseId);
    std::string getDatabaseId();
    void setUrl(std::shared_ptr<Url>);
    std::shared_ptr<Url> getUrl();
    Locator& getLocator();
    std::string getQuery();
    std::string getFormattedQuery();
    std::string getNormalizedQuery();
    std::string toString();
    std::set<std::string>& getUsedNamespaces();
    short getShardId();
    std::string getEnvironment();
    std::string getId();
    std::string getName();
    std::string getType();
    void setType(std::string queryType);
    void setArguments(std::vector<std::string> arguments);
    std::vector<std::string>& getArguments();
    void setMetaData(std::map<std::string, std::string> metaData);
    std::string getMetaData(std::string name, std::string fallback);
};

}

#endif /* QUERY_H_ */
