/*
 * ExecutionGraph2.h
 *
 *  Created on: Sep 7, 2014
 *      Author: arnd
 */

#ifndef EXECUTIONGRAPH2_H_
#define EXECUTIONGRAPH2_H_

#include <vector>
#include <set>
#include "Channel.h"
#include "core/QueryExecution.h"
#include "core/Query.h"

namespace db_agg {
class ExecutionGraph2 {
private:
	std::vector<Query*> queries;
	std::set<QueryExecution*> executions;
	std::vector<Channel*> channels;
	std::map<Query*,std::vector<QueryExecution*>> executionsByQuery;
    std::map<std::string,QueryExecution*> executionById;
public:
	void addQuery(Query *query);
    void addQueryExecution(Query*,QueryExecution*);
    void addQueryExecution(QueryExecution*);
    std::vector<Query*>& getQueries();
    std::set<QueryExecution*>& getQueryExecutions();
    std::vector<QueryExecution*>& getQueryExecutions(Query *query);
    QueryExecution& getQueryExecution(Query *query,int shardId);
    QueryExecution& getQueryExecution(std::string id);
    // std::vector<QueryExecution*> getTargets(Channel* source);
    void createChannel(QueryExecution *source, std::string sourcePort, QueryExecution *target, std::string targetPort);
    // std::vector<Channel*>& getOutputChannels(QueryExecution *exec);
    std::vector<QueryExecution*> getDependencies(QueryExecution *exec);
    void dumpExecutionPlan(std::string outputDir);
};
}



#endif /* EXECUTIONGRAPH2_H_ */
