/*
 * ExecutionGraph2.h
 *
 *  Created on: Sep 7, 2014
 *      Author: arnd
 */

#ifndef EXECUTIONGRAPH2_H_
#define EXECUTIONGRAPH2_H_

#include <vector>
#include "Channel.h"
#include "QueryExecution.h"
#include "Query.h"

namespace db_agg {
class ExecutionGraph2 {
private:
	std::vector<Query*> queries;
	std::vector<QueryExecution*> executions;
	std::vector<Channel*> channels;
	std::map<Query*,std::vector<QueryExecution*>> executionsByQuery;
public:
	void addQuery(Query *query);
    void addQueryExecution(Query*,QueryExecution*);
    void addQueryExecution(QueryExecution*);
    std::vector<Query*>& getQueries();
    void createChannel(QueryExecution *source, std::string sourcePort, QueryExecution *target, std::string targetPort);
    void dumpExecutionPlan(std::string outputDir);
};
}



#endif /* EXECUTIONGRAPH2_H_ */
