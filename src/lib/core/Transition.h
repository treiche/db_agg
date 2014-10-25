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
private:
    std::string name;
    int srcSize = 0;
    int dstSize = 0;
    std::vector<std::shared_ptr<TableData>> receivedData;
    bool done = false;
    std::vector<std::shared_ptr<ShardingStrategy>> sharders;
    std::vector<Channel*> channels;
    std::vector<std::shared_ptr<TableData>> split(std::shared_ptr<TableData> src);
    std::pair<std::shared_ptr<ShardingStrategy>,int> findShardColIndex(std::vector<std::pair<std::string,uint32_t>> columns);
public:
    Transition();
    Transition(std::string name, int srcSize, int dstSize);
    void setShardingStrategies(std::vector<std::shared_ptr<ShardingStrategy>> sharders);
    void receive(std::string name, std::shared_ptr<TableData> data) override;
    virtual void addChannel(Channel* channel) override;
    virtual void doTransition();
    virtual std::string getId();
    virtual ~Transition();
    void setName(std::string name);
    std::string getName();
    bool isDone();
    void release();
    friend std::ostream& operator<<(std::ostream& cout,const Transition& t);
};

std::ostream& operator<<(std::ostream& cout,const Transition& t);


}

#endif /* TRANSITION_H_ */
