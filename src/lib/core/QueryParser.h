#ifndef QUERYPARSER_H_
#define QUERYPARSER_H_

#include <map>
#include "core/Query.h"

namespace db_agg {
class QueryParser {
public:
    virtual ~QueryParser() {}
    virtual std::vector<Query*> parse(
            std::string query,
            std::map<std::string,std::string>& externalSources,
            std::map<std::string,std::string>& queryParameter,
            std::vector<std::string> functions) = 0;
};
}

#endif /* QUERYPARSER_H_ */
