/*
 * ManyToMany.h
 *
 *  Created on: Sep 7, 2014
 *      Author: arnd
 */

#ifndef MANYTOMANY_H_
#define MANYTOMANY_H_

#include "QueryExecution.h"
#include <memory>

namespace db_agg {
class ManyToMany: public QueryExecution {
private:
	short noShards;
	std::shared_ptr<ShardingStrategy> sharder;
	std::string shardColSearchExpr;
	int findShardColIndex(std::vector<std::pair<std::string,uint32_t>> columns, std::string searchExpr);
public:
	ManyToMany(std::shared_ptr<ShardingStrategy> sharder, std::string shardColSearchExpr, short noShards);
    virtual bool process() override;
    virtual bool isTransition() override;
};
}




#endif /* MANYTOMANY_H_ */
