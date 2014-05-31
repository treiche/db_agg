#ifndef TRANSITION_H_
#define TRANSITION_H_

#include <cstdint>
#include <string>
#include <vector>

#include "core/QueryExecution.h"
#include "sharding/ShardingStrategy.h"

namespace db_agg {
class QueryExecution;

std::vector<TableData*> split(TableData *src, int dstSize, ShardingStrategy *sharder);

class Transition {
    std::string name;
    std::vector<QueryExecution*> sources;
    std::vector<QueryExecution*> targets;
    std::map<std::string,std::shared_ptr<TableData>> sourceData;
    bool done = false;
    ShardingStrategy *sharder = nullptr;
    std::string shardColSearchExpr;
    std::vector<std::shared_ptr<TableData>> createdData;
public:
    Transition() {}
    Transition(std::string name): name(name) {
        sharder = nullptr;
    }
    Transition(std::string name, std::vector<QueryExecution*> sources, std::vector<QueryExecution*> targets, ShardingStrategy *sharder);
    virtual void doTransition();
    virtual std::string getId() {
        return name;
    }
    virtual void doTransition(std::string resultId, std::shared_ptr<TableData> data);
    virtual ~Transition();
    void addSource(QueryExecution *source) {
        sources.push_back(source);
    }
    std::vector<QueryExecution*>& getSources() {
        return sources;
    }
    void addTarget(QueryExecution *target) {
        targets.push_back(target);
    }
    std::vector<QueryExecution*>& getTargets() {
        return targets;
    }
    void setSharder(ShardingStrategy *sharder) {
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
