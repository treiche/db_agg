#ifndef REGEXPQUERYPARSER_H_
#define REGEXPQUERYPARSER_H_

#include <map>
#include <set>
#include <string>

#include "core/QueryParser.h"

namespace db_agg {
class RegExpQueryParser : public QueryParser {
    std::set<std::string> extractUsedNamespaces(std::string query);
    void detectDependencies(std::vector<Query*>& queries);
    Query *getSourceQuery(Dependency dep,std::vector<Query*>& queries);
    void detectScriptQueries(std::vector<Query*>& queries, std::vector<std::string>& functions);
    void extractMetaData(std::vector<Query*>& queries);
public:
    RegExpQueryParser();
    virtual ~RegExpQueryParser();
    virtual std::vector<Query*> parse(
            std::string query,
            std::map<std::string,std::string>& externalSources,
            std::map<std::string,
            std::string>& queryParameter,
            std::vector<std::string> functions) override;

};

}

#endif /* REGEXPQUERYPARSER_H_ */
