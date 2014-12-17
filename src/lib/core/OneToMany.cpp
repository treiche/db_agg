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


OneToMany::OneToMany(vector<shared_ptr<ShardingStrategy>> sharders, short noShards):
	sharders(sharders),
	noShards(noShards) {

	vector<string> portNames;
	for (short idx = 0; idx < noShards; idx++) {
		portNames.push_back(to_string(idx+1));
	}
	setPortNames(portNames);
}

bool OneToMany::process() {
	assert(getDependencies().size() == 1);
	shared_ptr<TableData> sourceTable = (*getDependencies().begin()).second;
	uint64_t rows = sourceTable->getRowCount();

	auto s = findShardColIndex(sourceTable->getColumns());
	auto sharder = s.first;
	auto shardKeyIdx = s.second;

	if (shardKeyIdx == -1) {
		LOG_WARN("no shard key column found. falling back to unsharded one-to-many transition.")
		for (short idx=0;idx<noShards;idx++) {
	        setResult(to_string(idx+1),sourceTable);
	    }
	} else {

		vector<vector<uint64_t>> offsets(noShards);
		map<size_t,shared_ptr<TableData>> results;
		for (uint64_t row = 0; row < rows; row++) {
			try {
				int shardId = sharder->getShardId(sourceTable->getValue(row,shardKeyIdx));
				LOG_DEBUG("shardId of '" << sourceTable->getValue(row,shardKeyIdx) << "' = " << shardId);
				offsets[shardId-1].push_back(row);
			} catch(InvalidShardKeyException& iske) {
				LOG_WARN("invalid shard key '" << sourceTable->getValue(row,shardKeyIdx) << "'");
			}
		}

		for (short idx=0;idx<noShards;idx++) {
			shared_ptr<TableData> shardedData = TableDataFactory::getInstance().split(sourceTable,offsets[idx]);
			setResult(to_string(idx+1),shardedData);
		}
	}

	setState(QueryExecutionState::DONE);
	return true;
}

pair<shared_ptr<ShardingStrategy>,int> OneToMany::findShardColIndex(vector<pair<std::string,uint32_t>> columns) {
    for (auto sharder:sharders) {
        int shardColIdx = sharder->findShardColIndex(columns);
        if (shardColIdx != -1) {
            return make_pair(sharder,shardColIdx);
        }
    }
    return make_pair(nullptr,-1);
}

bool OneToMany::isTransition() {
	return true;
}

}

