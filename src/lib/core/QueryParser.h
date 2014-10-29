#ifndef QUERYPARSER_H_
#define QUERYPARSER_H_

#include <map>
#include "core/Query.h"

namespace db_agg {
class QueryParser {
protected:
    Query *getSourceQuery(Dependency dep,std::vector<Query*>& queries);
    std::set<std::string> extractUsedNamespaces(std::string query);
    void detectDependencies(std::vector<Query*>& queries);
    void detectScriptQueries(std::vector<Query*>& queries, std::vector<std::string>& functions);
    void extractMetaData(std::vector<Query*>& queries);
    std::string normalizeQuery(std::string query);
public:
    virtual ~QueryParser() {}
    virtual std::vector<Query*> parse(
            std::string query,
            std::string url,
            std::map<std::string,std::string>& externalSources,
            std::map<std::string,std::string>& queryParameter,
            std::vector<std::string> functions) = 0;
};
}

#endif /* QUERYPARSER_H_ */
