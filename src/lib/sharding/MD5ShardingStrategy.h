/*
 * MD5ShardingStrategy.h
 *
 *  Created on: Mar 30, 2014
 *      Author: arnd
 */

#ifndef MD5SHARDINGSTRATEGY_H_
#define MD5SHARDINGSTRATEGY_H_

#include "ShardingStrategy.h"

namespace db_agg {
class MD5ShardingStrategy: public ShardingStrategy {
private:
    int shardCount = 8;
public:
    virtual ~MD5ShardingStrategy() = default;
    virtual int getShardId(std::string shardKey) override;
    virtual void setShardCount(int shardCount) override;

};


}



#endif /* MD5SHARDINGSTRATEGY_H_ */
