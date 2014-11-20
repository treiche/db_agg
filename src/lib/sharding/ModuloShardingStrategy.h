/*
 * ModuloShardingStrategy.h
 *
 *  Created on: Oct 15, 2014
 *      Author: arnd
 */

#ifndef MODULOSHARDINGSTRATEGY_H_
#define MODULOSHARDINGSTRATEGY_H_

#include "ShardingStrategy.h"

namespace db_agg {
class ModuloShardingStrategy: public ShardingStrategy {
private:
    int shardCount = 8;
public:
    virtual ~ModuloShardingStrategy() = default;
    virtual int getShardId(std::string shardKey) override;
    virtual void setShardCount(int shardCount) override;

};

}

#endif /* MODULOSHARDINGSTRATEGY_H_ */
