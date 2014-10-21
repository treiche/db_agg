/*
 * ExplicitShardingStratgegy.h
 *
 *  Created on: Oct 21, 2014
 *      Author: arnd
 */

#ifndef EXPLICITSHARDINGSTRATEGY_H_
#define EXPLICITSHARDINGSTRATEGY_H_

#include "ShardingStrategy.h"

namespace db_agg {
class ExplicitShardingStrategy: public ShardingStrategy {
private:
    int shardCount = 8;
public:
    virtual ~ExplicitShardingStrategy() = default;
    virtual int getShardId(std::string shardKey) override;
    virtual void setShardCount(int shardCount) override;

};


}

#endif /* EXPLICITSHARDINGSTRATEGY_H_ */
