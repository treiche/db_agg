/*
 * ExecutionGraph2.cpp
 *
 *  Created on: Sep 7, 2014
 *      Author: arnd
 */

#include "ExecutionGraph2.h"
#include "utils/logging.h"
#include <fstream>

using namespace std;


namespace db_agg {

DECLARE_LOGGER("ExecutionGraph2")

void ExecutionGraph2::addQuery(Query *query) {
	queries.push_back(query);
}

void ExecutionGraph2::addQueryExecution(Query *query,QueryExecution *queryExecution) {
    executionsByQuery[query].push_back(queryExecution);
    executions.insert(queryExecution);
	executionById[queryExecution->getId()] = queryExecution;
}

void ExecutionGraph2::addQueryExecution(QueryExecution *exec) {
	executions.insert(exec);
	executionById[exec->getId()] = exec;
}

vector<Query*>& ExecutionGraph2::getQueries() {
    return queries;
}

set<QueryExecution*>& ExecutionGraph2::getQueryExecutions() {
    return executions;
}

/*
vector<QueryExecution*> ExecutionGraph2::getTargets(Channel* sourceChannel) {
    vector<QueryExecution*> targets;
    for (auto channel:channels) {
    	if (channel->)
    }
    return targets;
}
*/

QueryExecution& ExecutionGraph2::getQueryExecution(Query *query,int shardId) {
    return *executionsByQuery[query][shardId];
}

vector<QueryExecution*>& ExecutionGraph2::getQueryExecutions(Query *query) {
    return executionsByQuery[query];
}


void ExecutionGraph2::createChannel(QueryExecution *source, std::string sourcePort, QueryExecution *target, std::string targetPort) {
	DataSender *sender = dynamic_cast<DataSender*>(source);
	DataReceiver *receiver = dynamic_cast<DataReceiver*>(target);
	Channel *channel = new Channel(sender, sourcePort, receiver, targetPort);
    source->addChannel(channel);
    channels.push_back(channel);
}

QueryExecution& ExecutionGraph2::getQueryExecution(std::string id) {
    if (executionById.find(id)==executionById.end()) {
        THROW_EXC("query execution with id " + id + " does not exist.");
    }
    return *executionById[id];
}

/*
vector<Channel*>& ExecutionGraph2::getOutputChannels(QueryExecution *exec) {
    return execToTrans[exec];
}
*/


vector<QueryExecution*> ExecutionGraph2::getDependencies(QueryExecution *exec) {
    vector<QueryExecution*> dependencies;
    for (auto channel:channels) {
    	if (channel->target == exec) {
    		dependencies.push_back((QueryExecution*)channel->source);
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


void ExecutionGraph2::dumpExecutionPlan(string outputDir) {
    LOG_INFO("dump execution plan");
    ofstream out{outputDir + "/executionPlan2.dot"};
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
    string cmd = "dot -q1 -Tsvg -o " + outputDir+"/executionPlan2.svg "+ outputDir+"/executionPlan2.dot";
    int exitCode = system(cmd.c_str());
    if (exitCode != 0) {
        LOG_WARN("failed to create image from executionPlan.dot");
    }
    LOG_DEBUG("dump execution plan done");
}


}


