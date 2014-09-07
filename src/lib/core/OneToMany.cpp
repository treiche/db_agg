/*
 * OneToMany.cpp
 *
 *  Created on: Sep 7, 2014
 *      Author: arnd
 */

#include "OneToMany.h"
#include <assert.h>
#include "utils/logging.h"
#include "utils/RegExp.h"
#include "table/TableDataFactory.h"

using namespace std;

namespace db_agg {

DECLARE_LOGGER("OneToMany");


OneToMany::OneToMany(ShardingStrategy *sharder, string shardColSearchExpr, short noShards):
	sharder(sharder),
	shardColSearchExpr(shardColSearchExpr),
	noShards(noShards) {

}

bool OneToMany::process() {
	assert(getDependencies().size() == 1);
	shared_ptr<TableData> sourceTable = (*getDependencies().begin()).second;
	uint64_t rows = sourceTable->getRowCount();

	int shardKeyIdx = findShardColIndex(sourceTable->getColumns(), shardColSearchExpr);

	vector<vector<uint64_t>> offsets(noShards);

	map<size_t,shared_ptr<TableData>> results;
	for (uint64_t row = 0; row < rows; row++) {
		int shardId = sharder->getShardId(sourceTable->getValue(row,shardKeyIdx));
		offsets[shardId].push_back(row);
	}

	for (size_t idx=0;idx<noShards;idx++) {
        shared_ptr<TableData> shardedData = TableDataFactory::getInstance().split(sourceTable,offsets[idx]);
        setResult(to_string(idx),shardedData);
    }

	setDone();
	return true;
}

int OneToMany::findShardColIndex(vector<pair<string,uint32_t>> columns, string searchExpr) {
    RegExp re(searchExpr);
    for (size_t idx=0;idx<columns.size();idx++) {
        string colName = columns[idx].first;
        vector<RegExp::match> matches = re.exec(colName);
        if (matches.size()>0) {
            return idx;
        }
    }
    throw runtime_error("unable to find shard key column");
}


}

