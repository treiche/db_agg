/*
 * ShardingStrategy.cpp
 *
 *  Created on: Oct 22, 2014
 *      Author: arnd
 */

#include "ShardingStrategy.h"
#include "utils/logging.h"
#include "utils/RegExp.h"

using namespace std;

namespace db_agg {

DECLARE_LOGGER("ShardingStrategy")

void ShardingStrategy::setShardCount(int shardCount) {
    this->shardCount = shardCount;
}

void ShardingStrategy::setShardColExpr(string shardColExpr) {
    this->shardColExpr = shardColExpr;
}

int ShardingStrategy::findShardColIndex(vector<pair<string,uint32_t>> columns) {
    RegExp re(shardColExpr);
    for (size_t idx=0;idx<columns.size();idx++) {
        string colName = columns[idx].first;
        vector<RegExp::match> matches = re.exec(colName);
        if (matches.size()>0) {
            LOG_ERROR("found shard col index: " << colName << " matches '" << shardColExpr << "'");
            return idx;
        }
    }
    return -1;
}


}


