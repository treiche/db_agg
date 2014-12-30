/*
 * ExecutionGraph.h
 *
 *  Created on: Sep 7, 2014
 *      Author: arnd
 */

#ifndef EXECUTIONGRAPH_H_
#define EXECUTIONGRAPH_H_

#include <vector>
#include <set>
#include "Channel.h"
#include "core/QueryExecution.h"
#include "core/Query.h"
#include "graph/Graph.h"
#include "Port.h"

namespace db_agg {

class ExecutionGraph: public Graph<Query*,QueryExecution*,Channel*,Port*> {
public:
    void addQuery(Query *query);
    void addQueryExecution(Query*,QueryExecution*);
    void addQueryExecution(QueryExecution*);
    std::vector<Query*> getQueries();
    std::set<QueryExecution*> getQueryExecutions();
    std::vector<QueryExecution*> getQueryExecutions(Query *query);
    QueryExecution& getQueryExecution(Query *query,int shardId);
    QueryExecution& getQueryExecution(std::string id);
    void createChannel(QueryExecution *source, std::string sourcePort, QueryExecution *target, std::string targetPort);
    std::vector<QueryExecution*> getDependencies(QueryExecution *exec);
    void dumpExecutionPlan(std::string outputDir);
    bool exists(std::string id);
};
}



#endif /* EXECUTIONGRAPH_H_ */
