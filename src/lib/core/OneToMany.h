/*
 * OneToMany.h
 *
 *  Created on: Sep 7, 2014
 *      Author: arnd
 */

#ifndef ONETOMANY_H_
#define ONETOMANY_H_

#include "QueryExecution.h"
#include "sharding/ShardingStrategy.h"
#include <memory>

namespace db_agg {
class OneToMany: public QueryExecution {
private:
	std::vector<std::shared_ptr<ShardingStrategy>> sharders;
	short noShards;
	std::string shardColSearchExpr;
	std::pair<std::shared_ptr<ShardingStrategy>,int> findShardColIndex(std::vector<std::pair<std::string,uint32_t>> columns);
public:
	OneToMany(std::vector<std::shared_ptr<ShardingStrategy>> sharders, short noShards);
    virtual bool process() override;
    virtual bool isTransition();

};
}



#endif /* ONETOMANY_H_ */
