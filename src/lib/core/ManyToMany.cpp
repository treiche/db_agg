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


ManyToMany::ManyToMany(vector<shared_ptr<ShardingStrategy>> sharders, short noShards):
    sharders(sharders),
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

    auto s = findShardColIndex(sourceTable->getColumns());
    auto sharder = s.first;
    auto shardKeyIdx = s.second;

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

bool ManyToMany::isTransition() {
    return true;
}

pair<shared_ptr<ShardingStrategy>,int> ManyToMany::findShardColIndex(vector<pair<std::string,uint32_t>> columns) {
    for (auto sharder:sharders) {
        int shardColIdx = sharder->findShardColIndex(columns);
        if (shardColIdx != -1) {
            return make_pair(sharder,shardColIdx);
        }
    }
    return make_pair(nullptr,-1);
}

}

