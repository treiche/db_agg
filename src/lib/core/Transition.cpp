#include "Transition.h"

#include <log4cplus/logger.h>
#include <iostream>
#include <stdexcept>
#include "table/CsvTableData.h"
#include "core/ExecutionHandler.h"
#include "utils/RegExp.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("Transition"));


static shared_ptr<TableData> join(vector<QueryExecution*> sources) {
    LOG4CPLUS_DEBUG(LOG, "join");
    vector<shared_ptr<TableData>> tds;
    for (auto source:sources) {
        td->appendRaw(source->getData()->getRaw(), source->getData()->getSize());
    }
    shared_ptr<TableData> td(new JoinedTableData(tds));
    LOG4CPLUS_DEBUG(LOG, "join done rowCount = " << td->getRowCount());
    return td;
}

int findShardColIndex(vector<pair<string,uint32_t>> columns, string searchExpr) {
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

vector<shared_ptr<TableData>> split(shared_ptr<TableData> src, int dstSize, ShardingStrategy *sharder, string shardColSearchExpr) {
    assert(sharder != nullptr);
    LOG4CPLUS_DEBUG(LOG, "split(" << src << "," << dstSize << "," << sharder << ")");
    sharder->setShardCount(dstSize);
    vector<shared_ptr<TableData>> splitted(dstSize);
    vector<vector<uint64_t>> offsets(dstSize);
    uint64_t rows = src->getRowCount();
    uint32_t cols = src->getColCount();
    size_t reserveSize = src->getSize() / dstSize;
    LOG4CPLUS_DEBUG(LOG, "reserve " << reserveSize << " bytes for each shard ");
    for (int cnt=0;cnt<dstSize;cnt++) {
        splitted[cnt] = new CsvTableData(src->getColumns());
        splittedData[cnt].reserve(reserveSize);
    }
    LOG4CPLUS_DEBUG(LOG, "get shard key index");
    size_t shardKeyIndex = findShardColIndex(src->getColumns(),shardColSearchExpr); // sharder->getShardKeyIndex(src->getColumns());
    if (shardKeyIndex >= cols) {
        LOG4CPLUS_ERROR(LOG, "no shard key index found:\n  available columns");
        for (auto& col:src->getColumns()) {
            LOG4CPLUS_ERROR(LOG, "    " << col.first);
        }
        throw runtime_error("no shard key index found");
    }
    LOG4CPLUS_DEBUG(LOG,"shardKeyIndex is " << shardKeyIndex);
    for (uint64_t row = 0; row < rows; row++) {
        LOG4CPLUS_TRACE(LOG, "split row " << row);
        string shardKey = src->getValue(row,shardKeyIndex);
        try {
            int shardId = sharder->getShardId(shardKey);
            // cout << "shardId of '" << shardKey << "' is " << shardId << endl;
            uint32_t rowSize;
            void *rowData = src->getRawRow(row,rowSize);
            splittedData[shardId-1].append(string((const char*)rowData,rowSize));
        } catch(InvalidShardKeyException& ise) {
            continue;
        }
    }

    for (size_t idx=0;idx<splittedData.size();idx++) {
        string data = splittedData[idx];
        splitted[idx]->appendRaw((void*)data.c_str(),data.size());
    }

    LOG4CPLUS_DEBUG(LOG, "return splitted result");
    return splitted;
}


Transition::Transition(string name, vector<QueryExecution*> sources, vector<QueryExecution*> targets, ShardingStrategy *sharder) :
    name(name),
    sources(sources),
    targets(targets),
    sharder(sharder) {
    LOG4CPLUS_DEBUG(LOG, "create transition '" << name << "'");
    assert((targets.size() == 1 && sharder == nullptr) || (targets.size() > 1 && sharder != nullptr));
}

Transition::~Transition() {
    LOG4CPLUS_TRACE(LOG,"delete transition " << this);
    for (auto data:createdData) {
        data.reset();
    }
    LOG4CPLUS_TRACE(LOG,"delete transition done");
}

void Transition::doTransition() {
    if (done) {
        string message = "transition '" + name +"' already done !";
        LOG4CPLUS_WARN(LOG, message);
        return;
    }
    int srcSize = sources.size();
    int dstSize = targets.size();
    LOG4CPLUS_DEBUG(LOG, "do transition '" << name << "' with src = " << srcSize << " dst = " << dstSize);
    bool allTargetsDone = true;
    for (auto& target:targets) {
        allTargetsDone &= target->isDone();
    }
    if (allTargetsDone) {
        LOG4CPLUS_DEBUG(LOG, "all targets done. skip transition ...");
        return;
    }

    bool complete = true;
    for (auto& source:sources) {
        complete &= source->isDone();
    }
    if (!complete) {
        LOG4CPLUS_DEBUG(LOG, "transitions sources are not complete... skip");
        return;
    }
    if (dstSize == 1 && srcSize > 1) {
        LOG4CPLUS_DEBUG(LOG, "many to one. join ...");
        shared_ptr<TableData> td = join(sources);
        createdData.push_back(td);
        LOG4CPLUS_DEBUG(LOG, "add dependency '" << name << "'");
        targets[0]->addDependency(name, td);
        done=true;
        return;
    } else if (dstSize > 1 && srcSize==1) {
        // split result
        shared_ptr<TableData> src = sources[0]->getData();
        LOG4CPLUS_DEBUG(LOG, "start split action sharder=" << sharder << " src=" << src);
        //uint32_t rows = src->getRowCount();
        //uint32_t cols = src->getColCount();
        if (sharder == nullptr) {
            // no splitting
            LOG4CPLUS_DEBUG(LOG, "inject targets");
            for (auto& target:targets) {
                LOG4CPLUS_DEBUG(LOG, "add dependency " + name);
                target->addDependency(name,src);
            }
            LOG4CPLUS_DEBUG(LOG, "add dependency done");
            done = true;
            return;
        } else {
            // split
            LOG4CPLUS_DEBUG(LOG, "start splitting " << src);
            vector<shared_ptr<TableData>> splitted = split(src,dstSize, sharder,shardColSearchExpr);
            for (auto data:splitted) {
                createdData.push_back(data);
            }
            LOG4CPLUS_DEBUG(LOG, "splitting done");
            for (int cnt=0;cnt<dstSize;cnt++) {
                targets[cnt]->addDependency(name, splitted[cnt]);
            }
            LOG4CPLUS_DEBUG(LOG, "add dependency done");
            done = true;
            return;
        }
    } else if (srcSize>1 && dstSize>1) {
        LOG4CPLUS_DEBUG(LOG, "start many to many");
        shared_ptr<TableData> joined = join(sources);
        vector<shared_ptr<TableData>> splitted = split(joined,dstSize,sharder,shardColSearchExpr);
        for (int cnt=0;cnt<dstSize;cnt++) {
            targets[cnt]->addDependency(name, splitted[cnt]);
        }
        LOG4CPLUS_DEBUG(LOG, "add dependency done");
        done = true;
        joined.reset();
        return;
    } else if (srcSize==1 && dstSize==1) {
        LOG4CPLUS_DEBUG(LOG, "start one to one");
        targets[0]->addDependency(name,sources[0]->getData());
        done = true;
        return;
    }

    throw runtime_error("transition type " + to_string(srcSize) + " -> " + to_string(dstSize) + " not implemented yet");
}

std::ostream& operator<<(std::ostream& cout,const Transition& t) {
    cout << "{\"name\": \"" << t.name << "\""
         << ", sources = " << t.sources.size()
         << ", targets = " << t.targets.size()
         << ", targets = " << *(t.targets[0])
    ;
    return cout;
}

}
