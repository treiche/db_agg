/*
 * ManyToMany.cpp
 *
 *  Created on: Sep 7, 2014
 *      Author: arnd
 */

#include "ManyToMany.h"
#include <assert.h>
#include "utils/logging.h"
#include "utils/RegExp.h"
#include "table/TableDataFactory.h"

using namespace std;

namespace db_agg {

DECLARE_LOGGER("ManyToMany");


ManyToMany::ManyToMany(ShardingStrategy *sharder, string shardColSearchExpr, short noShards):
	sharder(sharder),
	shardColSearchExpr(shardColSearchExpr),
	noShards(noShards) {

}

bool ManyToMany::process() {
	assert(getDependencies().size() == noShards);

	vector<shared_ptr<TableData>> sources;
	for (auto& dep:getDependencies()) {
		sources.push_back(dep.second);
	}
	auto sourceTable = TableDataFactory::getInstance().join(sources);


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

int ManyToMany::findShardColIndex(vector<pair<string,uint32_t>> columns, string searchExpr) {
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

