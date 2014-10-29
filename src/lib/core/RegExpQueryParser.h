#ifndef REGEXPQUERYPARSER_H_
#define REGEXPQUERYPARSER_H_

#include <map>
#include <set>
#include <string>

#include "core/QueryParser.h"

namespace db_agg {
class RegExpQueryParser : public QueryParser {
public:
    RegExpQueryParser();
    virtual ~RegExpQueryParser();
    virtual std::vector<Query*> parse(
            std::string query,
            std::string url,
            std::map<std::string,std::string>& externalSources,
            std::map<std::string,
            std::string>& queryParameter,
            std::vector<std::string> functions) override;

};

}

#endif /* REGEXPQUERYPARSER_H_ */
