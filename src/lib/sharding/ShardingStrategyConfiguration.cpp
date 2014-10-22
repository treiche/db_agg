/*
 * ShardingStrategyConfiguration.cpp
 *
 *  Created on: Oct 22, 2014
 *      Author: arnd
 */

#include "ShardingStrategyConfiguration.h"

using namespace std;

namespace db_agg {

ShardingStrategyConfiguration::ShardingStrategyConfiguration(std::string name, std::string shardCol):
    name(name),
    shardCol(shardCol) {}

string ShardingStrategyConfiguration::getName() {
    return name;
}
string ShardingStrategyConfiguration::getShardCol() {
    return shardCol;
}

}
