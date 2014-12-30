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
	addCluster(query);
}

void ExecutionGraph::addQueryExecution(Query *query,QueryExecution *queryExecution) {
    if (hasNode(queryExecution)) {
        THROW_EXC("execution with id " << queryExecution->getName() << " already added.");
    }
    addNode(query,queryExecution);
}

void ExecutionGraph::addQueryExecution(QueryExecution *queryExecution) {
    if (hasNode(queryExecution)) {
        LOG_WARN("execution with id " << queryExecution->getName() << " already added.");
        return;
    }
    addNode(queryExecution);
}

vector<Query*> ExecutionGraph::getQueries() {
    vector<Query*> queries;
    for (auto cluster:clusters) {
        queries.push_back(cluster);
    }
    return queries;
}

set<QueryExecution*> ExecutionGraph::getQueryExecutions() {
    set<QueryExecution*> execs;
    for (auto exec:nodes) {
        execs.insert(exec);
    }
    return execs;
}

QueryExecution& ExecutionGraph::getQueryExecution(Query *query,int shardId) {
    if (!hasCluster(query)) {
        THROW_EXC("query " << query << " not found.");
    }
    std::list<QueryExecution*> execs = cluster2nodes[query];
    int idx = 0;
    for (auto exec:execs) {
        if (idx == shardId) {
            return *exec;
        }
        idx++;
    }
    THROW_EXC("invalid array index " << shardId);
}

vector<QueryExecution*> ExecutionGraph::getQueryExecutions(Query *query) {
    vector<QueryExecution*> execs;
    for (auto exec:cluster2nodes[query]) {
        execs.push_back(exec);
    }
    return execs;
}


void ExecutionGraph::createChannel(QueryExecution *source, std::string sourcePort, QueryExecution *target, std::string targetPort) {
    LOG_DEBUG("create channel");

    /*
    Port *sp = new Port("",sourcePort);
    source->addPort(sp);
    addPort(source,sp);

    Port *tp = new Port("",targetPort);
    target->addPort(tp);
    addPort(target,tp);
    */
    Channel *channel = new Channel(source, sourcePort, target, targetPort);
    source->addChannel(channel);
    addEdge(channel);
}

bool ExecutionGraph::exists(std::string id) {
    for (auto exec:nodes) {
        if (exec->getId() == id) {
            return true;
        }
    }
    return false;
}

QueryExecution& ExecutionGraph::getQueryExecution(std::string id) {
    for (auto exec:nodes) {
        if (exec->getId() == id) {
            return *exec;
        }
    }
    THROW_EXC("query execution with id " + id + " does not exist.");
}

vector<QueryExecution*> ExecutionGraph::getDependencies(QueryExecution *exec) {
    vector<QueryExecution*> dependencies;
    for (auto channel:edges) {
    	QueryExecution *tgt = channel->target;
    	if (tgt == exec) {
    		QueryExecution *src = channel->source;
    		dependencies.push_back(src);
    		for (auto transitive:getDependencies(src)) {
    			dependencies.push_back(transitive);
    		}
    	}
    }
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
    for (auto& query:clusters) {
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
        for (auto exec:cluster2nodes[query]) {
            string fillcolor = "yellow";
            if (exec->getState() == QueryExecutionState::DONE) {
                fillcolor = "green";
            }
            out << "        \"" << exec->getId() << "\" [label=\"" << exec->getName() << "\", fillcolor=" << fillcolor << ", height=0.2, fontsize=8.5]" << endl;
        }
        out << "    }" << endl;
    }

    for (auto exec:nodes) {
        string fillcolor = "yellow";
        if (exec->getState() == QueryExecutionState::DONE) {
            fillcolor = "green";
        }
    	if (exec->isTransition()) {
    		out << "        \"" << exec->getId() << "\" [label=\"" << exec->getName() << "\", fillcolor=" << fillcolor << ", height=0.2, fontsize=8.5]" << endl;
    	}
    }

    for (auto& channel:edges) {
    	QueryExecution *source =  channel->source;
    	QueryExecution *target = channel->target;
    	out << "  \"" << source->getId() << "\" -> \"" << target->getId() << "\" [taillabel=\"" << channel->sourcePort << "\", headlabel=\"" << channel->targetPort << "\"]" << endl;
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


