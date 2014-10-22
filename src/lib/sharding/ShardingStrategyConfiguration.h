/*
 * ShardingStrategyConfiguration.h
 *
 *  Created on: Oct 22, 2014
 *      Author: arnd
 */

#ifndef SHARDINGSTRATEGYCONFIGURATION_H_
#define SHARDINGSTRATEGYCONFIGURATION_H_

#include <string>

namespace db_agg {

class ShardingStrategyConfiguration {
private:
    std::string name;
    std::string shardCol;
public:
    ShardingStrategyConfiguration(std::string name, std::string shardCol);
    std::string getName();
    std::string getShardCol();
};

}


#endif /* SHARDINGSTRATEGYCONFIGURATION_H_ */
