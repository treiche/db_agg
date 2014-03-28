#ifndef REGEXPQUERYPARSER_H_
#define REGEXPQUERYPARSER_H_

#include <map>
#include <set>
#include <string>

#include "core/QueryParser.h"

namespace db_agg {
class RegExpQueryParser : public QueryParser {
    struct XImpl;
    XImpl *pImpl;
    std::set<std::string> extractUsedNamespaces(std::string query);
    void detectDependencies();
    Query *getSourceQuery(Dependency dep);
public:
    RegExpQueryParser();
    virtual ~RegExpQueryParser();
    virtual std::deque<Query>& parse(std::string query, std::map<std::string,std::string>& externalSources,std::map<std::string,std::string>& queryParameter) override;
    virtual std::deque<Query>& getQueries() override;

};

}

#endif /* REGEXPQUERYPARSER_H_ */
