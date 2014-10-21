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


OneToMany::OneToMany(shared_ptr<ShardingStrategy> sharder, string shardColSearchExpr, short noShards):
	sharder(sharder),
	shardColSearchExpr(shardColSearchExpr),
	noShards(noShards) {

	vector<string> portNames;
	for (size_t idx = 0; idx < noShards; idx++) {
		portNames.push_back(to_string(idx+1));
	}
	setPortNames(portNames);
}

bool OneToMany::process() {
	assert(getDependencies().size() == 1);
	shared_ptr<TableData> sourceTable = (*getDependencies().begin()).second;
	uint64_t rows = sourceTable->getRowCount();

	int shardKeyIdx = findShardColIndex(sourceTable->getColumns(), shardColSearchExpr);

	if (shardKeyIdx == -1) {
		LOG_WARN("no shard key column found. falling back to unsharded one-to-many transition.")
		for (size_t idx=0;idx<noShards;idx++) {
	        setResult(to_string(idx+1),sourceTable);
	    }
	} else {

		vector<vector<uint64_t>> offsets(noShards);
		map<size_t,shared_ptr<TableData>> results;
		for (uint64_t row = 0; row < rows; row++) {
			try {
				int shardId = sharder->getShardId(sourceTable->getValue(row,shardKeyIdx));
				LOG_TRACE("shardId of '" << sourceTable->getValue(row,shardKeyIdx) << "' = " << shardId);
				offsets[shardId-1].push_back(row);
			} catch(InvalidShardKeyException& iske) {
				LOG_WARN("invalid shard key '" << sourceTable->getValue(row,shardKeyIdx) << "'");
			}
		}

		for (size_t idx=0;idx<noShards;idx++) {
			shared_ptr<TableData> shardedData = TableDataFactory::getInstance().split(sourceTable,offsets[idx]);
			setResult(to_string(idx+1),shardedData);
		}
	}

	shared_ptr<Event> event(new Event(EventType::PROCESSED,getId()));
	LOG_DEBUG("send PROCESSED event");
    fireEvent(event);

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
    LOG_WARN("unable to find shard key column. falling back to unsharded one-to-many");
    return -1;
}

bool OneToMany::isTransition() {
	return true;
}

}

