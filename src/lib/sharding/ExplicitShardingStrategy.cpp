/*
 * ExplicitShardingStrategy.cpp
 *
 *  Created on: Oct 21, 2014
 *      Author: arnd
 */

#include "ExplicitShardingStrategy.h"

#include "utils/logging.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
DECLARE_LOGGER("ExplicitShardingStrategy");

int ExplicitShardingStrategy::getShardId(std::string shardKey) {
	return stoi(shardKey);
}

void ExplicitShardingStrategy::setShardCount(int shardCount) {
    if (shardCount % 2 != 0) {
        throw runtime_error("shard count must be an event number");
    }
    this->shardCount = shardCount;
}

}



