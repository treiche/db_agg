#ifndef QUERYPARSER_H_
#define QUERYPARSER_H_

#include <map>
#include "core/Query.h"

namespace db_agg {
class QueryParser {
public:
    virtual ~QueryParser() {}
    virtual std::deque<Query>& parse(std::string query, std::map<std::string,std::string>& externalSources,std::map<std::string,std::string>& queryParameter) = 0;
    virtual std::deque<Query>& getQueries() = 0;
};
}

#endif /* QUERYPARSER_H_ */
