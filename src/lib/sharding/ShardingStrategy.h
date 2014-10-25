#ifndef SHARDINGSTRATEGY_H_
#define SHARDINGSTRATEGY_H_

#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include <stdexcept>

namespace db_agg {

class ShardingStrategy {
protected:
    int shardCount;
    std::string shardColExpr;
public:
    virtual ~ShardingStrategy() = default;
    virtual int getShardId(std::string shardKey) = 0;
    virtual void setShardCount(int shardCount);
    virtual void setShardColExpr(std::string shardColExpr);
    int findShardColIndex(std::vector<std::pair<std::string,uint32_t>> columns);
};

class InvalidShardKeyException : public std::runtime_error {
public:
    InvalidShardKeyException(std::string what): runtime_error(what) {}
};

}

#endif /* SHARDINGSTRATEGY_H_ */
