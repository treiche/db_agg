#ifndef DB_AGG_PARSER_H_
#define DB_AGG_PARSER_H_

#include <string>
#include <map>
#include <vector>

#include "cli/CommandLineParser.h"
#include "core/Configuration.h"

using namespace std;

namespace db_agg {

class db_agg_parser: public CommandLineParser {
public:
    db_agg_parser();
    void parse(int argc, char **argv, Configuration& config);
    void dumpConfiguration(Configuration&);


};

}

#endif /* DB_AGG_PARSER_H_ */
