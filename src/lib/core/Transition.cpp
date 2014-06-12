#include "Transition.h"

#include <log4cplus/logger.h>
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
    LOG4CPLUS_DEBUG(LOG, "split(" << src << "," << dstSize << "," << sharder << ")");
    sharder->setShardCount(dstSize);
    vector<shared_ptr<TableData>> splitted(dstSize);
    vector<vector<uint64_t>> offsets(dstSize);
    uint64_t rows = src->getRowCount();
    uint32_t cols = src->getColCount();
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
            offsets[shardId-1].push_back(row);
        } catch(InvalidShardKeyException& ise) {
            continue;
        }
    }

    for (size_t idx=0;idx<dstSize;idx++) {
        splitted[idx] = TableDataFactory::getInstance().split(src,offsets[idx]);
    }

    LOG4CPLUS_DEBUG(LOG, "return splitted result");
    return splitted;
}

Transition::~Transition() {
    LOG4CPLUS_TRACE(LOG,"delete transition " << this);
    /*
    for (auto data:createdData) {
        data.reset();
    }
    */
    LOG4CPLUS_TRACE(LOG,"delete transition done");
}

/*
void Transition::doTransition(string resultId, shared_ptr<TableData> data) {
    sourceData[resultId] = data;
    doTransition();
}
*/

void Transition::receive(string name, shared_ptr<TableData> data) {
    LOG4CPLUS_DEBUG(LOG, "receive data " << data);
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
        LOG4CPLUS_DEBUG(LOG, "channel '" << channel->getName() << "' is closed already");
    }
    LOG4CPLUS_DEBUG(LOG, "channelsDone = " << channelsDone);
    if (channelsDone) {
        string message = "transition '" + name +"' already done !";
        LOG4CPLUS_WARN(LOG, message);
        done = true;
        return;
    }
    if (done) {
        string message = "transition '" + name +"' already done !";
        LOG4CPLUS_WARN(LOG, message);
        return;
    }
    LOG4CPLUS_DEBUG(LOG, "do transition '" << name << "' with src = " << srcSize << " dst = " << dstSize);
    bool complete = true;
    if (receivedData.size() != srcSize) {
        LOG4CPLUS_DEBUG(LOG, "transitions sources are not complete... skip");
        return;
    }
    if (dstSize == 1 && srcSize > 1) {
        LOG4CPLUS_DEBUG(LOG, "many to one. join ...");
        shared_ptr<TableData> td = TableDataFactory::getInstance().join(receivedData);
        //createdData.push_back(td);
        LOG4CPLUS_DEBUG(LOG, "add dependency '" << name << "'");
        for (Channel *channel:channels) {
            channel->open();
            channel->send(td);
            channel->close();
        }
        done=true;
        return;
    } else if (dstSize > 1 && srcSize==1) {
        // split result
        shared_ptr<TableData> src = receivedData[0];
        LOG4CPLUS_DEBUG(LOG, "start split action sharder=" << sharder << " src=" << src);
        if (sharder == nullptr) {
            // no splitting
            LOG4CPLUS_DEBUG(LOG, "inject targets");
            for (auto channel:channels) {
                channel->open();
                channel->send(src);
                channel->close();
            }
            LOG4CPLUS_DEBUG(LOG, "add dependency done");
            done = true;
            return;
        } else {
            // split
            LOG4CPLUS_DEBUG(LOG, "start splitting " << src);
            vector<shared_ptr<TableData>> splitted = split(src,dstSize, sharder,shardColSearchExpr);
            /*
            for (auto data:splitted) {
                createdData.push_back(data);
            }
            */
            LOG4CPLUS_DEBUG(LOG, "splitting done");
            for (int cnt=0;cnt<dstSize;cnt++) {
                channels[cnt]->open();
                channels[cnt]->send(splitted[cnt]);
                channels[cnt]->close();
            }
            LOG4CPLUS_DEBUG(LOG, "add dependency done");
            done = true;
            return;
        }
    } else if (srcSize>1 && dstSize>1) {
        LOG4CPLUS_DEBUG(LOG, "start many to many");
        shared_ptr<TableData> joined = TableDataFactory::getInstance().join(receivedData);
        vector<shared_ptr<TableData>> splitted = split(joined,dstSize,sharder,shardColSearchExpr);
        for (int cnt=0;cnt<dstSize;cnt++) {
            channels[cnt]->open();
            channels[cnt]->send(splitted[cnt]);
            channels[cnt]->close();
        }
        LOG4CPLUS_DEBUG(LOG, "add dependency done");
        done = true;
        joined.reset();
        return;
    } else if (srcSize==1 && dstSize==1) {
        LOG4CPLUS_DEBUG(LOG, "start one to one");
        channels[0]->open();
        channels[0]->send(receivedData[0]);
        channels[0]->close();
        done = true;
        return;
    }

    throw runtime_error("transition type " + to_string(srcSize) + " -> " + to_string(dstSize) + " not implemented yet");
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
