/*
 * MD5ShardingStrategy.cpp
 *
 *  Created on: Mar 30, 2014
 *      Author: arnd
 */

#include "MD5ShardingStrategy.h"

#include <log4cplus/logger.h>
#include "utils/md5.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("MD5ShardingStrategy"));

int MD5ShardingStrategy::getShardId(std::string shardKey) {
    MD5 md5 = MD5(shardKey);
    const unsigned char *md5sum = md5.rawDigest();
    LOG4CPLUS_TRACE(LOG, "md5 of " << shardKey << " is " << md5sum );
    int virtualShardId = (md5sum[15] & 0xff) + ((md5sum[14] & 0xff) << 8) + ((md5sum[13] & 0xff) << 16);
    LOG4CPLUS_TRACE(LOG, "virtual shard id " << virtualShardId);
    int concreteShardId = virtualShardId & (shardCount - 1);
    return concreteShardId + 1;
}

void MD5ShardingStrategy::setShardCount(int shardCount) {
    if (shardCount % 2 != 0) {
        throw runtime_error("shard count must be an event number");
    }
    this->shardCount = shardCount;
}

}

