/*
 * QueryProcessor.cpp
 *
 *  Created on: Jan 10, 2014
 *      Author: arnd
 */

#include "core/QueryProcessor.h"

#include <log4cplus/logger.h>

#include "utils/utility.h"

#include "utils/md5.h"
#include <fstream>

#include "utils/File.h"
#include "table/CsvTableData.h"

#include "postgres/PGConnection.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {

static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("QueryProcessor"));


QueryProcessor::QueryProcessor(
    QueryParser& queryParser,
    DatabaseRegistry& registry,
    ExtensionLoader& extensionLoader,
    PasswordManager& passwordManager,
    CacheRegistry& cacheRegistry,
    string outputDir,
    bool disableCache,
    size_t copyThreshold,
    std::map<std::string,TableData*> externalSources,
    size_t statementTimeout,
    map<string,string> queryParameter):
    queryParser(queryParser),
    databaseRegistry(registry),
    extensionLoader(extensionLoader),
    passwordManager(passwordManager),
    cacheRegistry(cacheRegistry),
    outputDir(outputDir),
    disableCache(disableCache),
    copyThreshold(copyThreshold),
    externalSources(externalSources),
    statementTimeout(statementTimeout),
    queryParameter(queryParameter) {
}

QueryProcessor::~QueryProcessor() {
    LOG4CPLUS_TRACE(LOG, "delete query processor");
    if (queryExecutor!=nullptr) {
        delete queryExecutor;
        queryExecutor = nullptr;
    }
    LOG4CPLUS_TRACE(LOG, "delete query processor done");
}

void QueryProcessor::stop() {
    if (queryExecutor!=nullptr) {
        queryExecutor->stop();
    }
}

void QueryProcessor::process(string query, string environment) {
    LOG4CPLUS_DEBUG(LOG,"process query = " << query << " environment = " << environment);
    Locator::setDefaultEnvironment(environment);
    map<string,string> externalQueries;
    for (auto& externalSource:externalSources) {
    	externalQueries[externalSource.first] = "select * from " + externalSource.second->calculateMD5Sum();
    }
    LOG4CPLUS_DEBUG(LOG,"parse query with " << queryParameter.size() << " parameters");
    queryParser.parse(query,externalQueries,queryParameter);
    LOG4CPLUS_DEBUG(LOG,"parse query done");
    populateUrls(environment);
    populateTransitions();
    loadExternalSources();
    calculateExecutionIds();
    Event event{EventType::APPLICATION_INITIALIZED,""};
    fireEvent(event);
    loadFromCache();
    dumpExecutionPlan();
    checkConnections();
    queryExecutor = new AsyncQueryExecutor();
    queryExecutor->addEventListener(this);
    queryExecutor->process();
    LOG4CPLUS_DEBUG(LOG, "process complete");
}

void QueryProcessor::loadExternalSources() {
    LOG4CPLUS_INFO(LOG, "load externals");
    for (auto& query:queryParser.getQueries()) {
        if (query.isExternal()) {
            LOG4CPLUS_DEBUG(LOG, "load external source " << query.getName());
            TableData *td = externalSources[query.getName()];
            for (auto& qe:query.getQueryExecutions()) {
                qe.setResult(td);
                qe.setDone();
                qe.doTransitions();
            }
        }
    }

}


void QueryProcessor::loadFromCache() {
    LOG4CPLUS_INFO(LOG, "load items from cache");
    for (auto& qr:idToResult) {
        if (cacheRegistry.exists(qr.first)) {
            string resultId = qr.first;
            File path(cacheRegistry.getPath(resultId));
            if (path.exists()) {
                LOG4CPLUS_INFO(LOG, "cache item for " << resultId << " exists");
                TableData *data = new CsvTableData(cacheRegistry.getPath(resultId));
                if (!disableCache) {
                    qr.second->setResult(data);
                    qr.second->setDone();
                    //qr.second->doTransitions();
                    File linkPath(outputDir + "/" + qr.second->getName() + ".csv");
                    if (!linkPath.exists()) {
                        linkPath.linkTo(path);
                    }
                    ExecutionStateChangeEvent event{qr.first,"CACHED"};
                    fireEvent(event);
                    ReceiveDataEvent re{resultId,cacheRegistry.getRowCount(resultId)};
                    fireEvent(re);
                }
                CacheItem& ci = cacheRegistry.get(resultId);
                CacheLoadEvent ce{resultId,ci.lastExecuted,ci.lastDuration,ci.rowCount};
                fireEvent(ce);
            }
        } else {
            LOG4CPLUS_INFO(LOG, "cache item for " << qr.first << " does not exist");
        }
    }
    LOG4CPLUS_DEBUG(LOG, "load items from cache done");
    for (auto& qr:idToResult) {
        if (qr.second->isDone()) {
            qr.second->doTransitions();
        }
    }
}

void QueryProcessor::populateUrls(string environment) {
    LOG4CPLUS_TRACE(LOG, "populate urls");
    for (auto& query:queryParser.getQueries()) {
        LOG4CPLUS_DEBUG(LOG, "    [" << query.getName() << "] = " << query.toString());
        vector<Connection> urls;
        if (query.getUsedNamespaces().empty()) {
            // get worker url
            urls.push_back(databaseRegistry.getWorker());
        } else {
            string dbId = databaseRegistry.getDatabaseByNamespace(query.getUsedNamespaces());
            query.setDatabaseId(dbId);
            string env = query.getEnvironment();
            if (env.compare("") == 0) {
                env = environment;
            }
            urls = databaseRegistry.getUrls(dbId,env,query.getShardId());
            if (LOG.isEnabledFor(DEBUG_LOG_LEVEL)) {
                LOG4CPLUS_DEBUG(LOG,
                  "query " << query.getLocator().getQName() << " resolves to urls:" << endl <<
                  "[environment = " << query.getEnvironment() << "]"
                );
                for (auto& url:urls) {
                    LOG4CPLUS_DEBUG(LOG, "    " << url.getUrl());
                }
            }
        }
        if (urls.empty()) {
            throw runtime_error("no url found for " +  query.getName());
        }

        for (auto& url:urls) {
        	url.setStatementTimeout(statementTimeout);
        }

        vector<string> deps;
        for (auto& dep:query.getDependencies()) {
            deps.push_back(dep.locator.getQName());
        }

        for (size_t idx=0; idx<urls.size(); idx++) {
            Connection url = urls[idx];
            string resultId = md5hex(url.getUrl() + query.getQuery());
            pair<string,string> c = passwordManager.getCredential(url);
            // calculate link path
            string linkPath = query.getLocator().getQName();
            if (urls.size() > 1) {
                linkPath += "_" + to_string(idx + 1);
            }
            query.addQueryExecution(QueryExecution(linkPath, resultId, url.getUrl() + " user="+c.first+" password="+c.second,query.getQuery(),deps));
        }
    }
    for (auto& query:queryParser.getQueries()) {
        if (query.getQueryExecutions().size() == 0) {
            throw runtime_error("empty query executions");
        }
    }
    LOG4CPLUS_TRACE(LOG, "populate urls done");
}

void QueryProcessor::populateTransitions() {
    LOG4CPLUS_TRACE(LOG, "populate transitions");
    for (auto& query:queryParser.getQueries()) {
        for (auto& dep:query.getDependencies()) {
            Query& sourceQuery = *dep.sourceQuery;
            Query& targetQuery = query;
            vector<QueryExecution*> sourceExecutions;
            for (int shardId=0; shardId < (int)sourceQuery.getQueryExecutions().size(); shardId++) {
                if (dep.locator.getShardId()==-1 || dep.locator.getShardId()-1 == shardId) {
                    sourceExecutions.push_back(sourceQuery.getQueryExecution(shardId));
                }
            }
            size_t dstSize = targetQuery.getQueryExecutions().size();
            size_t srcSize = sourceExecutions.size();
            if (dstSize == 1 && srcSize == 1) {
                // one to one
                string name = dep.locator.getQName();
                Transition t(dep.locator.getQName());
                t.addSource(sourceExecutions[0]);
                t.addTarget(targetQuery.getQueryExecution(0));
                transitions.push_back(t);
                sourceExecutions.at(0)->addTransition(&transitions.back());
                targetQuery.getQueryExecution(0)->addIncomingTransition(&transitions.back());
            } else if (dstSize > 1 && dstSize == srcSize) {
                // many to many without sharding
                if (sourceQuery.getDatabaseId() == targetQuery.getDatabaseId()) {
                    for (size_t cnt=0; cnt< dstSize; cnt++) {
                        Transition t(dep.locator.getQName());
                        t.addSource(sourceQuery.getQueryExecution(cnt));
                        t.addTarget(targetQuery.getQueryExecution(cnt));
                        transitions.push_back(t);
                        sourceExecutions[cnt]->addTransition(&transitions.back());
                        targetQuery.getQueryExecution(cnt)->addIncomingTransition(&transitions.back());
                    }
                } else {
                    LOG4CPLUS_TRACE(LOG, "build many-to-many sharded");
                    Transition t(dep.locator.getQName());
                    string shardingStrategyName = databaseRegistry.getShardingStrategyName(targetQuery.getDatabaseId());
                    ShardingStrategy *sharder = extensionLoader.getShardingStrategy(shardingStrategyName);
                    LOG4CPLUS_TRACE(LOG,"set sharder to " << sharder);
                    t.setSharder(sharder);
                    for (size_t cnt=0; cnt< srcSize; cnt++) {
                        t.addSource(sourceQuery.getQueryExecution(cnt));
                    }
                    for (size_t cnt=0; cnt< dstSize; cnt++) {
                        t.addTarget(targetQuery.getQueryExecution(cnt));
                    }
                    transitions.push_back(t);
                    for (size_t cnt=0; cnt< srcSize; cnt++) {
                        sourceExecutions[cnt]->addTransition(&transitions.back());
                    }
                    for (size_t cnt=0; cnt< dstSize; cnt++) {
                        targetQuery.getQueryExecution(cnt)->addIncomingTransition(&transitions.back());
                    }
                    //throw runtime_error("many-to-many sharded not implemented yet");
                    //exit(0);
                }
            } else if (dstSize == 1 && srcSize > 1) {
                // many to one
                Transition t(dep.locator.getQName());
                t.addTarget(targetQuery.getQueryExecution(0));
                for (size_t cnt=0;cnt<srcSize; cnt++) {
                    t.addSource(sourceQuery.getQueryExecution(cnt));
                }
                transitions.push_back(t);
                for (size_t cnt=0;cnt<srcSize; cnt++) {
                    sourceExecutions[cnt]->addTransition(&transitions.back());
                }
                targetQuery.getQueryExecution(0)->addIncomingTransition(&transitions.back());
            } else if (dstSize > 1 && srcSize == 1) {
                // one to many
                Transition t(dep.locator.getQName());
                string shardingStrategyName = databaseRegistry.getShardingStrategyName(targetQuery.getDatabaseId());
                ShardingStrategy *sharder = extensionLoader.getShardingStrategy(shardingStrategyName);
                LOG4CPLUS_TRACE(LOG,"set sharder to " << sharder);
                t.setSharder(sharder);
                t.addSource(sourceQuery.getQueryExecution(0));
                for (size_t cnt=0;cnt<dstSize; cnt++) {
                    t.addTarget(targetQuery.getQueryExecution(cnt));
                }
                transitions.push_back(t);
                sourceExecutions[0]->addTransition(&transitions.back());
                for (size_t cnt=0;cnt<dstSize; cnt++) {
                    targetQuery.getQueryExecution(cnt)->addIncomingTransition(&transitions.back());
                }
            }
        }
    }
    LOG4CPLUS_TRACE(LOG, "populate transitions done");
}

void QueryProcessor::calculateExecutionIds() {
    LOG4CPLUS_TRACE(LOG, "calculate execution ids");
    for (auto& query:queryParser.getQueries()) {
        for (QueryExecution& exec:query.getQueryExecutions()) {
        	if (query.isExternal()) {
        		string resultId = exec.getResult()->calculateMD5Sum();
        	    LOG4CPLUS_DEBUG(LOG, "md5 of external " << query.getName() << " -> " << resultId);
				idToResult[resultId] = &exec;
        	} else {
				string md5data;
				calculateExecutionId(exec,md5data);
				string resultId(md5hex(md5data));
				if (resultId.compare(exec.getId())!=0) {
					//cout << "move result id " << exec.getId() << " -> " << resultId << endl;
				}
				exec.setId(resultId);
				if (idToResult.find(resultId)!=idToResult.end()) {
					LOG4CPLUS_ERROR(LOG, "found duplicate result id for query "+query.getName());
					throw runtime_error("duplicate result id");
				}
				idToResult[resultId] = &exec;
        	}
        }
    }
    LOG4CPLUS_TRACE(LOG, "calculate execution ids done");
}

void QueryProcessor::checkConnections() {
    for (auto& query:queryParser.getQueries()) {
        if (!query.isExternal()) {
            for (QueryExecution& exec:query.getQueryExecutions()) {
                if (!exec.isDone()) {
                    LOG4CPLUS_INFO(LOG, "check connection "+maskPassword(exec.getConnectionUrl()));
                    ExecutionStateChangeEvent event{exec.getId(),"PING"};
                    fireEvent(event);
                    PGConnection::ping(exec.getConnectionUrl());
                    event.state = "OK";
                    fireEvent(event);
                }
            }
        }
    }
}

void QueryProcessor::calculateExecutionId(QueryExecution& exec, string& md5data) {
    md5data.append(exec.getConnectionUrl() + exec.getSql());
    vector<Transition*> inc = exec.getIncomingTransitions();
    for (Transition *t:inc) {
        for (QueryExecution *src:t->getSources()) {
            md5data.append(" || ");
            this->calculateExecutionId(*src,md5data);
        }
    }
}

vector<QueryExecution*> QueryProcessor::findExecutables() {
    vector<QueryExecution*> executables;
    for (auto& exec:idToResult) {
        if (!exec.second->isDone() && exec.second->isComplete() && !exec.second->isScheduled()) {
            executables.push_back(exec.second);
        }
    }
    return executables;
}

void QueryProcessor::cacheItem(string resultId) {
    LOG4CPLUS_DEBUG(LOG, "save cache item "+resultId);
    File linkPath{outputDir + "/" + idToResult[resultId]->getName() + ".csv"};
    uint64_t rowCount = idToResult[resultId]->getData()->getRowCount();
    cacheRegistry.registerItem(resultId,Time(),idToResult[resultId]->getDuration(),linkPath.abspath(),"csv", rowCount);
    idToResult[resultId]->getData()->save(cacheRegistry.getPath(resultId));
    LOG4CPLUS_TRACE(LOG, "save data done");
    // TODO: has to be done on exit
    cacheRegistry.save();
    if (linkPath.exists()) {
        linkPath.remove();
    }
    linkPath.linkTo(cacheRegistry.getPath(resultId));
}

void QueryProcessor::handleEvent(Event& event) {
    if (event.type==EventType::PROCESSED) {
        QueryExecution& result = *idToResult[event.resultId];
        LOG4CPLUS_DEBUG(LOG, "PROCESSED: " << result.getSql());
        result.setDone();
        if (result.getData()==nullptr) {
            throw runtime_error("result not ready");
        }
        result.doTransitions();
        cacheItem(event.resultId);
    }
    vector<QueryExecution*> exec = findExecutables();
    if (!exec.empty()) {
        LOG4CPLUS_DEBUG(LOG, "found " << exec.size() << " executables");
        for (auto result:exec) {
            LOG4CPLUS_DEBUG(LOG, "schedule executable  " << result->getSql());
            string sql = result->inject(result->getSql(), copyThreshold);
            queryExecutor->addQuery(result->getId(), result->getConnectionUrl(), sql, result);
            result->setScheduled();
        }
    }
    fireEvent(event);
}


void QueryProcessor::dumpExecutionPlan() {
    LOG4CPLUS_INFO(LOG, "dump execution plan");
    ofstream out{outputDir + "/executionPlan.dot"};
    out << "digraph plan {" << endl;
    out << "    graph [rankdir=LR, splines=true]" << endl;
    out << "    node  [shape=box, style=filled, labelloc=t]" << endl;
    // dump nodes
    int clusterNo = 0;
    for (auto& query:queryParser.getQueries()) {
        out << "    subgraph cluster_" << ++clusterNo <<  " {" << endl;
        out << "        label = \"" << query.getLocator().getQName() << "\""<< endl;
        string label;
        string formattedQuery = query.getFormattedQuery();
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
        for (auto& exec:query.getQueryExecutions()) {
            out << "        \"" << exec.getId() << "\" [label=\"" << exec.getName() << "\", fillcolor=green, height=0.2, fontsize=8.5]" << endl;
        }
        out << "    }" << endl;
    }

    int transitionNo = 0;
    for (auto& t:transitions) {
        out << "  \"transition_" << ++transitionNo << "\" [color=blue, shape=circle, width=0.1, fixedsize=true, label=\"" << t.getName() << "\"]" << endl;
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
    for (auto& t:transitions) {
        ++transitionNo;
        for (auto& source:t.getSources()) {
            out << "  \"" << source->getId() << "\" -> \"transition_" << transitionNo << "\" [style=solid, fontsize=6.5]" << endl;
        }
        for (auto& target:t.getTargets()) {
            out << "  \"transition_" << transitionNo << "\" -> \"" << target->getId() << "\" [style=solid, fontsize=6.5]" << endl;
        }
    }

    out << "}" << endl;
    out.close();
    string cmd = "dot -q1 -Tpng -o " + outputDir+"/executionPlan.png "+ outputDir+"/executionPlan.dot";
    system(cmd.c_str());
    LOG4CPLUS_DEBUG(LOG, "dump execution plan done");
}


}


