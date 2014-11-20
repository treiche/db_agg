/*
 * ManyToMany.h
 *
 *  Created on: Sep 7, 2014
 *      Author: arnd
 */

#ifndef MANYTOMANY_H_
#define MANYTOMANY_H_

#include "QueryExecution.h"
#include "sharding/ShardingStrategy.h"
#include <memory>
#include <vector>

namespace db_agg {
class ManyToMany: public QueryExecution {
private:
	short noShards;
	std::vector<std::shared_ptr<ShardingStrategy>> sharders;
	std::string shardColSearchExpr;
	std::pair<std::shared_ptr<ShardingStrategy>,int> findShardColIndex(std::vector<std::pair<std::string,uint32_t>> columns);
public:
    ManyToMany(std::vector<std::shared_ptr<ShardingStrategy>> sharders, short noShards);
    virtual bool process() override;
    virtual bool isTransition() override;
};
}




#endif /* MANYTOMANY_H_ */
