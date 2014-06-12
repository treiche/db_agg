/*
 * ExecutionGraph.cpp
 *
 *  Created on: Jun 1, 2014
 *      Author: arnd
 */

#include "ExecutionGraph.h"

#include <log4cplus/logger.h>
#include <fstream>

#include "graph/Channel.h"

using namespace log4cplus;
using namespace std;

namespace db_agg {

static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("ExecutionGraph"));

ExecutionGraph::~ExecutionGraph() {
    for (auto query:queries) {
        delete query;
    }
    for (auto exec:queryExecutions) {
        delete exec;
    }
    for (auto transition:transitions) {
        delete transition;
    }
    for (auto channel:channels) {
        delete channel;
    }
}

void ExecutionGraph::addQueryExecution(QueryExecution *queryExecution) {
    if (queryExecutionsById.find(queryExecution->getId())!=queryExecutionsById.end()) {
        LOG4CPLUS_ERROR(LOG, "execution with id " << queryExecution->getId() << " exists already");
        throw runtime_error("duplicate result id");
    }
    queryExecutionsById[queryExecution->getId()] = queryExecution;
    queryExecutions.push_back(queryExecution);
}

QueryExecution& ExecutionGraph::getQueryExecution(std::string id) {
    if (queryExecutionsById.find(id)==queryExecutionsById.end()) {
        throw runtime_error("query execution with id " + id + " does not exist.");
    }
    return *queryExecutionsById[id];
}

QueryExecution& ExecutionGraph::getQueryExecution(Query *query,int shardId) {
    return *executionsByQuery[query][shardId];
}


vector<QueryExecution*>& ExecutionGraph::getQueryExecutions() {
    return queryExecutions;
}

vector<QueryExecution*>& ExecutionGraph::getQueryExecutions(Query *query) {
    return executionsByQuery[query];
}

void ExecutionGraph::addTransition(Transition *transition) {
    transitions.push_back(transition);
}

vector<Transition*>& ExecutionGraph::getTransitions() {
    return transitions;
}

void ExecutionGraph::addQueryExecution(Query *query,QueryExecution *queryExecution) {
    executionsByQuery[query].push_back(queryExecution);
}

void ExecutionGraph::addQuery(Query *query) {
    queries.push_back(query);
}

vector<Query*>& ExecutionGraph::getQueries() {
    return queries;
}

void ExecutionGraph::createChannel(Transition *transition, QueryExecution *exec) {
    Channel *channel = new Channel(transition->getName(),exec);
    transition->addChannel(channel);
    channels.push_back(channel);
    transToExec[transition].push_back(channel);
}

void ExecutionGraph::createChannel(QueryExecution *exec, Transition *transition) {
    Channel *channel = new Channel(exec->getName(), transition);
    exec->addChannel(channel);
    channels.push_back(channel);
    execToTrans[exec].push_back(channel);
}

vector<Channel*>& ExecutionGraph::getOutputChannels(QueryExecution *exec) {
    return execToTrans[exec];
}

vector<QueryExecution*> ExecutionGraph::getSources(Transition *transition) {
    vector<QueryExecution*> sources;
    for (auto exec:execToTrans) {
        for (auto channel:exec.second) {
            Transition *t = dynamic_cast<Transition*>(channel->receiver);
            if (t==transition) {
                sources.push_back(exec.first);
            }
        }
    }
    return sources;
}

vector<Transition*> ExecutionGraph::getIncomingTransitions(QueryExecution *exec) {
    vector<Transition*> trans;
    for (auto transition:transToExec) {
        for (auto channel:transition.second) {
            QueryExecution *qe = dynamic_cast<QueryExecution*>(channel->receiver);
            if (qe==exec) {
                trans.push_back(transition.first);
            }
        }
    }
    return trans;
}

void ExecutionGraph::dumpExecutionPlan(string outputDir) {
    LOG4CPLUS_INFO(LOG, "dump execution plan");
    ofstream out{outputDir + "/executionPlan.dot"};
    out << "digraph plan {" << endl;
    out << "    graph [rankdir=LR, splines=true]" << endl;
    out << "    node  [shape=box, style=filled, labelloc=t]" << endl;
    // dump nodes
    int clusterNo = 0;
    for (auto& query:queries) {
        out << "    subgraph cluster_" << ++clusterNo <<  " {" << endl;
        out << "        label = \"" << query->getLocator().getQName() << "\""<< endl;
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
        out << "        \"" + label + "\" [fontsize=7.0, fontname=\"Courier new\", shape=note]" << endl;
        // out << "  \"" << query.first << "\" [label=\"" << query.second.getLocator().getQName() << "\", fillcolor=red]" << endl;
        for (auto exec:executionsByQuery[query]) {
            out << "        \"" << exec << "\" [label=\"" << exec->getName() << "\", fillcolor=green, height=0.2, fontsize=8.5]" << endl;
        }
        out << "    }" << endl;
    }

    int transitionNo = 0;
    for (auto& t:transitions) {
        out << "  \"" << t << "\" [color=blue, shape=circle, width=0.1, fixedsize=true, label=\"" << t->getName() << "\"]" << endl;
    }
    // dump dependencies
    /*
    for (auto& query:queryParser.getQueries()) {
        for (auto& dep:query.second.getDependencies()) {
            out << "  \"" << query.first << "\" -> \"" << dep.sourceQuery->getId() << "\" [xlabel=\"" << dep.locator.getQName() << "\", arrowhead=none, style=dashed, color=gray]" << endl;
        }
        for (auto& exec:query.second.getQueryExecutions()) {
            out << "  \"" << query.first << "\" -> \"" << exec.getId() << "\" [arrowhead=none, style=dashed, penwidth=0.1]" << endl;
        }
    }
    */

    transitionNo = 0;
    /*
    for (auto& t:transitions) {
        ++transitionNo;
        for (auto& source:t->getSources()) {
            out << "  \"" << source->getId() << "\" -> \"transition_" << transitionNo << "\" [style=solid, fontsize=6.5]" << endl;
        }
        for (auto& target:t->getTargets()) {
            out << "  \"transition_" << transitionNo << "\" -> \"" << target->getId() << "\" [style=solid, fontsize=6.5]" << endl;
        }
    }
    */

    transitionNo = 0;
    for (auto& exec:execToTrans) {
        ++transitionNo;
        for (auto channel:exec.second) {
            DataReceiver *receiver = channel->receiver;
            Transition *transition = dynamic_cast<Transition*>(receiver);
            out << "  \"" << exec.first << "\" -> \"" << transition << "\" [style=solid, fontsize=6.5]" << endl;
        }
    }

    for (auto& trans:transToExec) {
        ++transitionNo;
        for (auto channel:trans.second) {
            DataReceiver *receiver = channel->receiver;
            QueryExecution *exec = dynamic_cast<QueryExecution*>(receiver);
            out << "  \"" << trans.first << "\" -> \"" << exec << "\" [style=solid, fontsize=6.5]" << endl;
        }
    }

    out << "}" << endl;
    out.close();
    string cmd = "dot -q1 -Tpng -o " + outputDir+"/executionPlan.png "+ outputDir+"/executionPlan.dot";
    int exitCode = system(cmd.c_str());
    if (exitCode != 0) {
        LOG4CPLUS_ERROR(LOG, "failed to create image from executionPlan.dot");
    }
    LOG4CPLUS_DEBUG(LOG, "dump execution plan done");
}


}


