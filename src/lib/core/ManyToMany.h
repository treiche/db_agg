/*
 * ManyToMany.h
 *
 *  Created on: Sep 7, 2014
 *      Author: arnd
 */

#ifndef MANYTOMANY_H_
#define MANYTOMANY_H_

#include "QueryExecution.h"

namespace db_agg {
class ManyToMany: public QueryExecution {
private:
	short noShards;
	ShardingStrategy *sharder;
	std::string shardColSearchExpr;
	int findShardColIndex(std::vector<std::pair<std::string,uint32_t>> columns, std::string searchExpr);
public:
	ManyToMany(ShardingStrategy *sharder, std::string shardColSearchExpr, short noShards);
    virtual bool process() override;
};
}




#endif /* MANYTOMANY_H_ */
