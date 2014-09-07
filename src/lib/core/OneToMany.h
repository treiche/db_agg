/*
 * OneToMany.h
 *
 *  Created on: Sep 7, 2014
 *      Author: arnd
 */

#ifndef ONETOMANY_H_
#define ONETOMANY_H_

#include "QueryExecution.h"

namespace db_agg {
class OneToMany: public QueryExecution {
private:
	short noShards;
	ShardingStrategy *sharder;
	std::string shardColSearchExpr;
	int findShardColIndex(std::vector<std::pair<std::string,uint32_t>> columns, std::string searchExpr);
public:
	OneToMany(ShardingStrategy *sharder, std::string shardColSearchExpr, short noShards);
    virtual bool process() override;
};
}



#endif /* ONETOMANY_H_ */
