/*
 * ExecutionGraph.cpp
 *
 *  Created on: Jun 1, 2014
 *      Author: arnd
 */

#include "ExecutionGraph.h"

#include "utils/logging.h"
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
        THROW_EXC("duplicate result id. execution with id " << queryExecution->getId() << " exists already");
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

vector<QueryExecution*> ExecutionGraph::getDependencies(QueryExecution *exec) {
    vector<Transition*> transitions = getIncomingTransitions(exec);
    vector<QueryExecution*> dependencies;
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
    return dependencies;
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

vector<QueryExecution*> ExecutionGraph::getTargets(Channel* sourceChannel) {
    vector<QueryExecution*> targets;
    Transition *t = dynamic_cast<Transition*>(sourceChannel->receiver);
    for (auto targetChannel:transToExec[t]) {
        QueryExecution *target = dynamic_cast<QueryExecution*>(targetChannel->receiver);
        targets.push_back(target);
    }
    return targets;
}

void ExecutionGraph::dumpGraph(string outputDir) {
    ofstream out{outputDir + "/executionPlan.graphml"};
    out << R"(
<graphml xmlns="http://graphml.graphdrawing.org/xmlns"  
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xsi:schemaLocation="http://graphml.graphdrawing.org/xmlns
     http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd">
   <graph id="executionPlan" edgedefault="directed">
)";
    for (auto& query:queries) {
        out << "<node id='" << query->getId() << "'/>\n";
        for (auto exec:executionsByQuery[query]) {
            out << "<node id='" << exec << "'/>\n";
        }
    }
    for (auto& t:transitions) {
        out << "<node id='" << t << "'/>\n";
    }

    for (auto& query:queries) {
        for (auto exec:executionsByQuery[query]) {
            out << "<edge source='" << query->getId() << "' target='" << exec << "'/>\n";
        }
    }

    for (auto& exec:execToTrans) {
        for (auto channel:exec.second) {
            DataReceiver *receiver = channel->receiver;
            Transition *transition = dynamic_cast<Transition*>(receiver);
            out << "<edge source='" << exec.first << "' target='" << transition << "'/>\n";
        }
    }

    for (auto& trans:transToExec) {
        for (auto channel:trans.second) {
            DataReceiver *receiver = channel->receiver;
            QueryExecution *exec = dynamic_cast<QueryExecution*>(receiver);
            out << "<edge source='" << trans.first << "' target='" << exec << "'/>\n";
        }
    }

    out << "</graph></graphml>";
    out.close();
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

    //int transitionNo = 0;
    for (auto& t:transitions) {
        out << "  \"" << t << "\" [color=blue, shape=circle, width=0.1, fixedsize=true, label=\"" << t->getName() << "\"]" << endl;
    }

    //transitionNo = 0;
    for (auto& exec:execToTrans) {
        //++transitionNo;
        for (auto channel:exec.second) {
            DataReceiver *receiver = channel->receiver;
            Transition *transition = dynamic_cast<Transition*>(receiver);
            string style = "solid";
            if (channel->getState() == ChannelState::CLOSED) {
                style = "dashed";
            }
            out << "  \"" << exec.first << "\" -> \"" << transition << "\" [style=" << style << ", fontsize=6.5]" << endl;
        }
    }

    for (auto& trans:transToExec) {
        //++transitionNo;
        for (auto channel:trans.second) {
            DataReceiver *receiver = channel->receiver;
            QueryExecution *exec = dynamic_cast<QueryExecution*>(receiver);
            string style = "solid";
            if (channel->getState() == ChannelState::CLOSED) {
                style = "dashed";
            }
            out << "  \"" << trans.first << "\" -> \"" << exec << "\" [style=" << style << ", fontsize=6.5]" << endl;
        }
    }

    out << "}" << endl;
    out.close();
    string cmd = "dot -q1 -Tsvg -o " + outputDir+"/executionPlan.svg "+ outputDir+"/executionPlan.dot";
    int exitCode = system(cmd.c_str());
    if (exitCode != 0) {
        LOG_WARN("failed to create image from executionPlan.dot");
    }
    LOG_DEBUG("dump execution plan done");
}


}


