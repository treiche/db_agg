#ifndef TRANSITION_H_
#define TRANSITION_H_

#include <cstdint>
#include <string>
#include <vector>

#include "core/QueryExecution.h"
#include "sharding/ShardingStrategy.h"
#include "graph/Channel.h"
#include "graph/DataReceiver.h"
#include "graph/DataSender.h"

namespace db_agg {
class QueryExecution;

std::vector<TableData*> split(TableData *src, int dstSize, ShardingStrategy *sharder);

class Transition : public DataReceiver, public DataSender {
    std::string name;
    int srcSize = 0;
    int dstSize = 0;
    std::vector<std::shared_ptr<TableData>> receivedData;
    // std::map<std::string,std::shared_ptr<TableData>> sourceData;
    bool done = false;
    std::shared_ptr<ShardingStrategy> sharder;
    std::string shardColSearchExpr;
    std::vector<Channel*> channels;
public:
    Transition() {}
    Transition(std::string name, int srcSize, int dstSize): name(name), srcSize(srcSize), dstSize(dstSize) {
        sharder = nullptr;
    }

    void receive(std::string name, std::shared_ptr<TableData> data) override;
    virtual void addChannel(Channel* channel) override;
    virtual void doTransition();
    virtual std::string getId() {
        return name;
    }
    virtual ~Transition();
    void setSharder(std::shared_ptr<ShardingStrategy> sharder) {
        this->sharder = sharder;
    }
    void setShardColSearchExpr(std::string shardColSearchExpr) {
        this->shardColSearchExpr = shardColSearchExpr;
    }
    void setName(std::string name) {
        this->name = name;
    }
    std::string getName() {
        return name;
    }

    bool isDone() {
        return done;
    }
    friend std::ostream& operator<<(std::ostream& cout,const Transition& t);
};

std::ostream& operator<<(std::ostream& cout,const Transition& t);


}

#endif /* TRANSITION_H_ */
