#include "Transition.h"

#include "utils/logging.h"
#include <iostream>
#include <stdexcept>
#include "table/TableDataFactory.h"
#include "core/ExecutionHandler.h"
#include "utils/RegExp.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("Transition"));

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

vector<shared_ptr<TableData>> split(shared_ptr<TableData> src, int dstSize, shared_ptr<ShardingStrategy> sharder, string shardColSearchExpr) {
    assert(sharder != nullptr);
    assert(src.get() != nullptr);
    LOG_DEBUG("split(" << src << "," << dstSize << "," << sharder << ")");
    sharder->setShardCount(dstSize);
    vector<shared_ptr<TableData>> splitted(dstSize);
    vector<vector<uint64_t>> offsets(dstSize);
    uint64_t rows = src->getRowCount();
    uint32_t cols = src->getColCount();
    LOG_DEBUG("get shard key index");
    size_t shardKeyIndex = findShardColIndex(src->getColumns(),shardColSearchExpr); // sharder->getShardKeyIndex(src->getColumns());
    if (shardKeyIndex >= cols) {
        LOG_ERROR("no shard key index found:\n  available columns");
        for (auto& col:src->getColumns()) {
            LOG_ERROR("    " << col.first);
        }
        THROW_EXC("no shard key index found");
    }
    LOG_DEBUG("shardKeyIndex is " << shardKeyIndex);
    for (uint64_t row = 0; row < rows; row++) {
        LOG_TRACE("split row " << row);
        string shardKey = src->getValue(row,shardKeyIndex);
        try {
            int shardId = sharder->getShardId(shardKey);
            offsets[shardId-1].push_back(row);
        } catch(InvalidShardKeyException& ise) {
            continue;
        }
    }

    for (size_t idx=0;idx<dstSize;idx++) {
        splitted[idx] = TableDataFactory::getInstance().split(src,offsets[idx]);
    }

    LOG_DEBUG("return splitted result");
    return splitted;
}

Transition::~Transition() {
    LOG_TRACE("delete transition " << this);
    /*
    for (auto data:createdData) {
        data.reset();
    }
    */
    LOG_TRACE("delete transition done");
}

/*
void Transition::doTransition(string resultId, shared_ptr<TableData> data) {
    sourceData[resultId] = data;
    doTransition();
}
*/

void Transition::receive(string name, shared_ptr<TableData> data) {
    assert(data.get() != nullptr);
    LOG_DEBUG("receive data " << data);
    receivedData.push_back(data);
    if (receivedData.size() == srcSize) {
        doTransition();
    }
}

void Transition::addChannel(Channel* channel) {
    this->channels.push_back(channel);
}

void Transition::doTransition() {
    bool channelsDone = true;
    for (auto channel:channels) {
        channelsDone &= channel->getState() == ChannelState::CLOSED;
        LOG_DEBUG("channel '" << channel->getTargetPort() << "' is closed already");
    }
    LOG_DEBUG("channelsDone = " << channelsDone);
    if (channelsDone) {
        string message = "transition '" + name +"' already done !";
        LOG_WARN(message);
        done = true;
        release();
        return;
    }
    if (done) {
        string message = "transition '" + name +"' already done !";
        LOG_WARN(message);
        release();
        return;
    }
    LOG_DEBUG("do transition '" << name << "' with src = " << srcSize << " dst = " << dstSize);
    bool complete = true;
    if (receivedData.size() != srcSize) {
        LOG_DEBUG("transitions sources are not complete... skip");
        return;
    }
    if (dstSize == 1 && srcSize > 1) {
        LOG_DEBUG("many to one. join ...");
        shared_ptr<TableData> td = TableDataFactory::getInstance().join(receivedData);
        //createdData.push_back(td);
        LOG_DEBUG("add dependency '" << name << "'");
        for (Channel *channel:channels) {
            channel->open();
            channel->send(td);
            channel->close();
        }
        done=true;
        release();
        return;
    } else if (dstSize > 1 && srcSize==1) {
        // split result
        shared_ptr<TableData> src = receivedData[0];
        LOG_DEBUG("start split action sharder=" << sharder << " src=" << src);
        if (sharder == nullptr) {
            // no splitting
            LOG_DEBUG("inject targets");
            for (auto channel:channels) {
                channel->open();
                channel->send(src);
                channel->close();
            }
            LOG_DEBUG("add dependency done");
            done = true;
            release();
            return;
        } else {
            // split
            LOG_DEBUG("start splitting " << src);
            vector<shared_ptr<TableData>> splitted = split(src,dstSize, sharder,shardColSearchExpr);
            /*
            for (auto data:splitted) {
                createdData.push_back(data);
            }
            */
            LOG_DEBUG("splitting done");
            for (int cnt=0;cnt<dstSize;cnt++) {
                channels[cnt]->open();
                channels[cnt]->send(splitted[cnt]);
                channels[cnt]->close();
            }
            LOG_DEBUG("add dependency done");
            done = true;
            release();
            return;
        }
    } else if (srcSize>1 && dstSize>1) {
        LOG_DEBUG("start many to many");
        shared_ptr<TableData> joined = TableDataFactory::getInstance().join(receivedData);
        vector<shared_ptr<TableData>> splitted = split(joined,dstSize,sharder,shardColSearchExpr);
        for (int cnt=0;cnt<dstSize;cnt++) {
            channels[cnt]->open();
            channels[cnt]->send(splitted[cnt]);
            channels[cnt]->close();
        }
        LOG_DEBUG("add dependency done");
        done = true;
        joined.reset();
        release();
        return;
    } else if (srcSize==1 && dstSize==1) {
        LOG_DEBUG("start one to one");
        channels[0]->open();
        channels[0]->send(receivedData[0]);
        channels[0]->close();
        done = true;
        release();
        return;
    }

    throw runtime_error("transition type " + to_string(srcSize) + " -> " + to_string(dstSize) + " not implemented yet");
}

void Transition::release() {
    for (auto& data:receivedData) {
        data.reset();
    }
}

std::ostream& operator<<(std::ostream& cout,const Transition& t) {
    cout << "{\"name\": \"" << t.name << "\""
         << ", sources = " << t.srcSize
         << ", targets = " << t.dstSize
        //  << ", targets = " << *(t.targets[0])
    ;
    return cout;
}

}
