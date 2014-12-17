/*
 * ModuloShardingStrategy.cpp
 *
 *  Created on: Mar 30, 2014
 *      Author: arnd
 */

#include "ModuloShardingStrategy.h"

#include "utils/logging.h"
#include "utils/md5.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
DECLARE_LOGGER("ModuloShardingStrategy");

int ModuloShardingStrategy::getShardId(std::string shardKey) {
	int iKey = stoi(shardKey);
	cout << "shardCount = " << shardCount << endl;
	cout << "ikey = " << iKey << " = " << ((iKey % shardCount) + 1) << endl;
    return iKey % shardCount + 1;
}

void ModuloShardingStrategy::setShardCount(int shardCount) {
    this->shardCount = shardCount;
}

}





