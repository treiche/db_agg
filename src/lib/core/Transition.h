#ifndef TRANSITION_H_
#define TRANSITION_H_

#include <cstdint>
#include <string>
#include <vector>

#include "core/QueryExecution.h"
#include "core/ShardingStrategy.h"

namespace db_agg {
class QueryExecution;

std::vector<TableData*> split(TableData *src, int dstSize, ShardingStrategy *sharder);

class Transition {
    std::string name;
    std::vector<QueryExecution*> sources;
    std::vector<QueryExecution*> targets;
    bool done = false;
    ShardingStrategy *sharder = nullptr;
    std::vector<TableData*> createdData;
public:
    Transition() {}
    Transition(std::string name): name(name) {
        sharder = nullptr;
    }
    Transition(std::string name, std::vector<QueryExecution*> sources, std::vector<QueryExecution*> targets, ShardingStrategy *sharder);
    virtual void doTransition();
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
    void setName(std::string name) {
        this->name = name;
    }
    std::string getName() {
        return name;
    }
    friend std::ostream& operator<<(std::ostream& cout,const Transition& t);
};

std::ostream& operator<<(std::ostream& cout,const Transition& t);


}

#endif /* TRANSITION_H_ */
