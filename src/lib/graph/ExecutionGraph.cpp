/*
 * ExecutionGraph.cpp
 *
 *  Created on: Sep 7, 2014
 *      Author: arnd
 */

#include "ExecutionGraph.h"
#include "utils/logging.h"
#include <fstream>

using namespace std;


namespace db_agg {

DECLARE_LOGGER("ExecutionGraph")

void ExecutionGraph::addQuery(Query *query) {
	queries.push_back(query);
}

void ExecutionGraph::addQueryExecution(Query *query,QueryExecution *queryExecution) {
    executionsByQuery[query].push_back(queryExecution);
    executions.insert(queryExecution);
    if (executionById.find(queryExecution->getId()) != executionById.end()) {
    	THROW_EXC("execution with id " << queryExecution->getId() << " already added");
    }
	executionById[queryExecution->getId()] = queryExecution;
}

void ExecutionGraph::addQueryExecution(QueryExecution *exec) {
    if (executionById.find(exec->getId()) != executionById.end()) {
    	THROW_EXC("execution with id " << exec->getId() << " already added");
    }
	executions.insert(exec);
	executionById[exec->getId()] = exec;
}

vector<Query*>& ExecutionGraph::getQueries() {
    return queries;
}

set<QueryExecution*>& ExecutionGraph::getQueryExecutions() {
    return executions;
}

/*
vector<QueryExecution*> ExecutionGraph::getTargets(Channel* sourceChannel) {
    vector<QueryExecution*> targets;
    for (auto channel:channels) {
    	if (channel->)
    }
    return targets;
}
*/

QueryExecution& ExecutionGraph::getQueryExecution(Query *query,int shardId) {
	if (executionsByQuery.find(query) == executionsByQuery.end()) {
		THROW_EXC("query " << query << " not found.");
	}
	vector<QueryExecution*>& executions = executionsByQuery[query];
    //return *executionsByQuery[query][shardId];
	if (shardId >= (int)executions.size()) {
		THROW_EXC("invalid array index " << shardId);
	}
	return *executions.at(shardId);
}

vector<QueryExecution*>& ExecutionGraph::getQueryExecutions(Query *query) {
    return executionsByQuery[query];
}


void ExecutionGraph::createChannel(QueryExecution *source, std::string sourcePort, QueryExecution *target, std::string targetPort) {
	DataSender *sender = dynamic_cast<DataSender*>(source);
	DataReceiver *receiver = dynamic_cast<DataReceiver*>(target);
	Channel *channel = new Channel(sender, sourcePort, receiver, targetPort);
    source->addChannel(channel);
    channels.push_back(channel);
}

bool ExecutionGraph::exists(std::string id) {
	return executionById.find(id) != executionById.end();
}

QueryExecution& ExecutionGraph::getQueryExecution(std::string id) {
    if (executionById.find(id)==executionById.end()) {
        THROW_EXC("query execution with id " + id + " does not exist.");
    }
    return *executionById[id];
}

/*
vector<Channel*>& ExecutionGraph::getOutputChannels(QueryExecution *exec) {
    return execToTrans[exec];
}
*/


vector<QueryExecution*> ExecutionGraph::getDependencies(QueryExecution *exec) {
    vector<QueryExecution*> dependencies;
    for (auto channel:channels) {
    	QueryExecution *tgt = dynamic_cast<QueryExecution*>(channel->target);
    	if (tgt == exec) {
    		QueryExecution *src = dynamic_cast<QueryExecution*>(channel->source);
    		dependencies.push_back(src);
    		for (auto transitive:getDependencies(src)) {
    			dependencies.push_back(transitive);
    		}
    	}
    }
    /*
    vector<Transition*> transitions = getIncomingTransitions(exec);
    for (auto transition:transitions) {
        vector<QueryExecution*> sources = getSources(transition);
        for (QueryExecution *source:sources) {
            dependencies.push_back(source);
            vector<QueryExecution*> transientDependencies = getDependencies(source);
            for (auto transientDependency:transientDependencies) {
                dependencies.push_back(transientDependency);
            }
        }
    }
    */
    return dependencies;
}


void ExecutionGraph::dumpExecutionPlan(string outputDir) {
    LOG_INFO("dump execution plan");
    ofstream out{outputDir + "/executionPlan.dot"};
    out << "digraph plan {" << endl;
    out << "    graph [rankdir=LR, splines=true]" << endl;
    out << "    node  [shape=box, style=filled, labelloc=t]" << endl;

    // dump nodes
    int clusterNo = 0;
    for (auto& query:queries) {
        out << "    subgraph cluster_" << ++clusterNo <<  " {" << endl;
        out << "        label = \"" << query->getLocator().getQName() << " [" << query->getType() << "]\""<< endl;
        if (true) {
            string label;
            string formattedQuery = query->getFormattedQuery();
            for (auto c:formattedQuery) {
                if (c=='\n') {
                    label += "\\l";
                } else if (c==' ') {
                    label += "\xC2\xA0";
                } else if (c=='"') {
                    label += "\\\"";
                } else {
                    label += c;
                }
            }
            label += "\\l";
            out << "        \"" + label + "\"";
        } else {
            out << "\" ";
        }
        out << " [fontsize=7.0, fontname=\"Courier new\", shape=note]" << endl;
        // out << "  \"" << query.first << "\" [label=\"" << query.second.getLocator().getQName() << "\", fillcolor=red]" << endl;
        for (auto exec:executionsByQuery[query]) {
            string fillcolor = "yellow";
            if (exec->isDone()) {
                fillcolor = "green";
            }
            out << "        \"" << exec << "\" [label=\"" << exec->getName() << "\", fillcolor=" << fillcolor << ", height=0.2, fontsize=8.5]" << endl;
        }
        out << "    }" << endl;
    }

    for (auto exec:executions) {
        string fillcolor = "yellow";
        if (exec->isDone()) {
            fillcolor = "green";
        }
    	if (exec->isTransition()) {
    		out << "        \"" << exec << "\" [label=\"" << exec->getName() << "\", fillcolor=" << fillcolor << ", height=0.2, fontsize=8.5]" << endl;
    	}
    }

    for (auto& channel:channels) {
    	QueryExecution *source = dynamic_cast<QueryExecution*>(channel->source);
    	QueryExecution *target = dynamic_cast<QueryExecution*>(channel->target);
    	out << "  \"" << source << "\" -> \"" << target << "\" [taillabel=\"" << channel->sourcePort << "\", headlabel=\"" << channel->targetPort << "\"]" << endl;
    }

    out << "\n}";
    out.close();
    string cmd = "dot -q1 -Tsvg -o " + outputDir+"/executionPlan.svg "+ outputDir+"/executionPlan.dot";
    int exitCode = system(cmd.c_str());
    if (exitCode != 0) {
        LOG_WARN("failed to create image from executionPlan.dot");
    }
    LOG_INFO("dump execution plan done");
}


}


