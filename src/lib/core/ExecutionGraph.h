/*
 * ExecutionGraph.h
 *
 *  Created on: Jun 1, 2014
 *      Author: arnd
 */

#ifndef EXECUTIONGRAPH_H_
#define EXECUTIONGRAPH_H_

#include <map>
#include <vector>
#include "QueryExecution.h"
#include "Transition.h"
#include "Query.h"
#include "graph/Channel.h"

namespace db_agg {
class ExecutionGraph {
private:
    std::map<std::string,QueryExecution*> queryExecutionsById;
    std::vector<QueryExecution*> queryExecutions;
    std::vector<Transition*> transitions;
    std::vector<Query*> queries;
    std::vector<Channel*> channels;
    std::map<Query*,std::vector<QueryExecution*>> executionsByQuery;
    std::map<QueryExecution*,std::vector<Channel*>> execToTrans;
    std::map<Transition*,std::vector<Channel*>> transToExec;
public:
    ~ExecutionGraph();
    void addTransition(Transition *transition);
    std::vector<Transition*>& getTransitions();
    void addQueryExecution(QueryExecution *queryExecution);
    void addQueryExecution(Query*,QueryExecution*);
    QueryExecution& getQueryExecution(std::string id);
    QueryExecution& getQueryExecution(Query *query,int shardId);
    std::vector<QueryExecution*>& getQueryExecutions();
    std::vector<QueryExecution*>& getQueryExecutions(Query *query);
    void addQuery(Query *query);
    std::vector<Query*>& getQueries();
    void createChannel(QueryExecution *exec, Transition *transition);
    void createChannel(Transition *transition, QueryExecution *exec);
    std::vector<Channel*>& getOutputChannels(QueryExecution *exec);
    std::vector<QueryExecution*> getSources(Transition *transition);
    std::vector<QueryExecution*> getTargets(Channel* source);
    std::vector<Transition*> getIncomingTransitions(QueryExecution *exec);
    std::vector<QueryExecution*> getDependencies(QueryExecution *exec);
    void dumpExecutionPlan(std::string outputDir);
    void dumpGraph(std::string outputDir);
};
}



#endif /* EXECUTIONGRAPH_H_ */
