/*
 * Builtin.cpp
 *
 *  Created on: Mar 30, 2014
 *      Author: arnd
 */

#include "Builtin.h"
#include "sharding/MD5ShardingStrategy.h"

#include <log4cplus/logger.h>

using namespace std;
using namespace log4cplus;

namespace db_agg {
static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("Builtin"));

ShardingStrategy *Builtin::getShardingStrategy(std::string name) {
    if (name == "md5") {
        return new MD5ShardingStrategy();
    }
    return nullptr;
}

Builtin::~Builtin() {};


}



