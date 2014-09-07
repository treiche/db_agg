/*
 * QueryProcessor.cpp
 *
 *  Created on: Jan 10, 2014
 *      Author: arnd
 */

#include "core/QueryProcessor.h"

#include "utils/logging.h"

#include "utils/utility.h"

#include "utils/md5.h"
#include <fstream>

#include "utils/File.h"

#include "injection/DependencyInjector.h"

#include "utils/SignalHandler.h"

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
    map<string,string> externalSources,
    size_t statementTimeout,
    map<string,string> queryParameter,
    bool dontExecute,
    size_t maxParallelExecutions):
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
    queryParameter(queryParameter),
    dontExecute(dontExecute),
    maxParallelExecutions(maxParallelExecutions) {
}

QueryProcessor::~QueryProcessor() {
}

void QueryProcessor::stop() {
    stopped = true;
    for (auto exec:executionGraph.getQueryExecutions()) {
        exec->stop();
    }
}

void QueryProcessor::process(string query, string environment) {
    LOG_DEBUG("process query = " << query << " environment = " << environment);
    Locator::setDefaultEnvironment(environment);

    vector<string> functions = extensionLoader.getAvailableExecutorNames();
    LOG_DEBUG("parse query with " << queryParameter.size() << " parameters");
    vector<Query*> queries = queryParser.parse(query,externalSources,queryParameter,functions);
    for (auto query:queries) {
        executionGraph.addQuery(query);
    }
    LOG_DEBUG("parse query done");
    populateUrls(environment);
    populateTransitions();
    loadExternalSources();
    calculateExecutionIds();
    shared_ptr<Event> event(new Event(EventType::APPLICATION_INITIALIZED,""));
    fireEvent(event);
    loadFromCache();
    executionGraph.dumpExecutionPlan(outputDir);
    executionGraph.dumpGraph(outputDir);
    if (dontExecute) {
        shared_ptr<Event> fe(new Event(EventType::APPLICATION_FINISHED,""));
        fireEvent(fe);
        return;
    }
    checkConnections();

    set<QueryExecution*> runningQueries;
    bool done = false;
    do {
        done = true;
        for (auto exec:executionGraph.getQueryExecutions()) {
            try {
                if (!exec->isDone() && exec->isComplete()) {
                    if (runningQueries.size() < maxParallelExecutions || runningQueries.find(exec) != runningQueries.end()) {
                        runningQueries.insert(exec);
                        bool execDone = exec->process();
                        if (execDone) {
                            runningQueries.erase(exec);
                        }
                        done &= execDone;
                    } else {
                        LOG_DEBUG("exec " << exec->getName() << " skipped as maxExecution limit (" << maxParallelExecutions << ") is reached");
                    }
                }
                if (stopped) {
                    throw CancelException("got cancellation request");
                }
            } catch(CancelException& ce) {
                cleanUp();
                throw ce;
            }
        }
    } while(!done);
}

void QueryProcessor::cleanUp() {
    for (auto exec:executionGraph.getQueryExecutions()) {
        exec->cleanUp();
    }
}

void QueryProcessor::loadExternalSources() {
    LOG_INFO("load externals");
    /*
    for (auto query:executionGraph.getQueries()) {
        if (query->getType() == "external") {
            LOG_DEBUG("load external source " << query->getName());
            shared_ptr<TableData> td = externalSources[query->getName()];
            for (auto& qe:executionGraph.getQueryExecutions(query)) {
                qe->setResult(td);
                qe->setDone();
                for (auto channel:qe->getChannels()) {
                    channel->open();
                    channel->send(td);
                    channel->close();
                }
            }
        }
    }
    */

}


void QueryProcessor::loadFromCache() {
    LOG_INFO("load items from cache");
    for (auto& qr:executionGraph.getQueryExecutions()) {
        if (cacheRegistry.exists(qr->getId())) {
            string resultId = qr->getId();
            File path(cacheRegistry.getPath(resultId));
            if (path.exists()) {
                LOG_INFO("cache item for " << qr->getName() << "[" << resultId << "] exists");
                if (!disableCache) {
                    shared_ptr<TableData> data = cacheRegistry.getData(resultId);
                    qr->setResult("", data);
                    qr->setDone();
                    // close all incoming channels here
                    // executionGraph.getSources(qr);
                    //qr.second->doTransitions();
                    File linkPath(outputDir + "/" + qr->getName() + ".csv");
                    if (!linkPath.exists()) {
                        linkPath.linkTo(path);
                    }
                    shared_ptr<Event> event(new ExecutionStateChangeEvent(qr->getId(),"CACHED"));
                    fireEvent(event);
                    shared_ptr<Event> re(new ReceiveDataEvent(resultId,cacheRegistry.getRowCount(resultId)));
                    fireEvent(re);
                }
                CacheItem& ci = cacheRegistry.get(resultId);
                shared_ptr<Event> ce(new CacheLoadEvent(resultId,ci.lastExecuted,ci.lastDuration,ci.rowCount));
                fireEvent(ce);
            }
        } else {
            LOG_INFO("cache item for " << qr->getName() << "[" << qr->getId() << "] does not exist");
        }
    }
    LOG_DEBUG("load items from cache done");
    for (auto& qr:executionGraph.getQueryExecutions()) {
        if (qr->isDone()) {
            for (auto channel:qr->getChannels()) {
                vector<QueryExecution*> targets = executionGraph.getTargets(channel);
                bool allDone = true;
                for (auto target:targets) {
                    allDone &= target->isDone();
                }
                if (allDone) {
                    continue;
                }
                LOG_DEBUG("open channel " << channel->getTargetPort());
                if (channel->getState() != ChannelState::READY) {
                    continue;
                }
                channel->open();
                LOG_INFO("send data");
                channel->send(qr->getResult(""));
                LOG_INFO("send data done");
                channel->close();
            }
            qr->release();
        }
    }
}

void QueryProcessor::populateUrls(string environment) {
    LOG_TRACE("populate urls");
    for (auto query:executionGraph.getQueries()) {
        LOG_DEBUG("    [" << query->getName() << "] = " << query->toString());
        vector<shared_ptr<Url>> urls;

        if (query->getType() == "script") {
            shared_ptr<Url> url(new Url("file","","","script.sh"));
            urls.push_back(url);
        } else

        if (query->getType() == "resource") {
            shared_ptr<Url> url(new Url(query->getQuery()));
            urls.push_back(url);
        } else if (query->getType() == "memcached") {
            string env = query->getEnvironment();
            if (env.compare("") == 0) {
                env = environment;
            }
            urls = databaseRegistry.getUrls(env,query->getType());
            // cout << "urls = " << urls.size() << endl;
            // urls.push_back(urls[0]);
        } else
        if (query->getUsedNamespaces().empty()) {
            // get worker url
            urls.push_back(databaseRegistry.getWorker());
        } else {
            string dbId = databaseRegistry.getDatabaseByNamespace(query->getUsedNamespaces());
            query->setDatabaseId(dbId);
            string env = query->getEnvironment();
            if (env.compare("") == 0) {
                env = environment;
            }
            urls = databaseRegistry.getUrls(dbId,env,query->getShardId());
            if (LOG.isEnabledFor(DEBUG_LOG_LEVEL)) {
                LOG_DEBUG(
                  "query " << query->getLocator().getQName() << " resolves to urls:" << endl <<
                  "[environment = " << query->getEnvironment() << "]"
                );
                for (auto& url:urls) {
                    LOG_DEBUG("    " << url->getUrl());
                }
            }
        }
        if (urls.empty()) {
            THROW_EXC("no url found for " <<  query->getName());
        }

        for (auto& url:urls) {
            url->setParameter("statementTimeout", to_string(statementTimeout));
            string searchPath = query->getMetaData("search_path","");
            if (searchPath != "") {
                LOG_DEBUG("set search_path to " << searchPath)
                url->setParameter("search_path", searchPath);
            }
        }

        vector<string> deps;
        for (auto& dep:query->getDependencies()) {
            deps.push_back(dep.locator.getQName());
        }

        // get credentials
        for (size_t idx=0; idx<urls.size(); idx++) {
            shared_ptr<Url> url = urls[idx];
            string resultId = md5hex(url->getUrl(false,false,false) + query->getQuery());
            if (query->getType() == "postgres") {
                pair<string,string> c = passwordManager.getCredential(url.get());
                url->setUser(c.first);
                url->setPassword(c.second);
            }
            // calculate link path
            string linkPath = query->getLocator().getQName();
            if (urls.size() > 1) {
                linkPath += "_" + to_string(idx + 1);
            }
            string injectorName = query->getMetaData("injector","default");
            LOG_DEBUG("injector for query " << query->getName() << ":" << injectorName);
            shared_ptr<DependencyInjector> di = extensionLoader.getDependencyInjector(injectorName);
            LOG_DEBUG("create execution for " << query->getName() << " type = " << query->getType());
            QueryExecution *exec = extensionLoader.getQueryExecution(query->getType());
            if (exec == nullptr) {
                throw runtime_error("unhandled query execution type '" + query->getType() + "'");
            }
            exec->init(linkPath, resultId, url ,query->getNormalizedQuery(),deps,di, query->getArguments());
            exec->addEventListener(this);
            executionGraph.addQueryExecution(query,exec);
        }
    }
    for (auto& query:executionGraph.getQueries()) {
        if (executionGraph.getQueryExecutions(query).size() == 0) {
            throw runtime_error("empty query executions");
        }
    }

    LOG_DEBUG("populate urls done");
}

void QueryProcessor::populateTransitions() {
    LOG_TRACE("populate transitions");
    for (auto query:executionGraph.getQueries()) {
        for (auto& dep:query->getDependencies()) {
            Query& sourceQuery = *dep.sourceQuery;
            Query& targetQuery = *query;
            vector<QueryExecution*> sourceExecutions;
            for (int shardId=0; shardId < (int)executionGraph.getQueryExecutions(&sourceQuery).size(); shardId++) {
                if (dep.locator.getShardId()==-1 || dep.locator.getShardId()-1 == shardId) {
                    sourceExecutions.push_back(&executionGraph.getQueryExecution(&sourceQuery,shardId));
                }
            }
            size_t dstSize = executionGraph.getQueryExecutions(&targetQuery).size();
            size_t srcSize = sourceExecutions.size();
            if (dstSize == 1 && srcSize == 1) {
                // one to one
                string name = dep.locator.getQName();
                Transition *t = new Transition(dep.locator.getQName(),1,1);
                executionGraph.addTransition(t);
                QueryExecution& targetExecution = executionGraph.getQueryExecution(&targetQuery,0);
                executionGraph.createChannel(sourceExecutions[0],t);
                executionGraph.createChannel(t,&targetExecution);
            } else if (dstSize > 1 && dstSize == srcSize) {
                // many to many without sharding
                if (sourceQuery.getDatabaseId() == targetQuery.getDatabaseId()) {
                    for (size_t cnt=0; cnt< dstSize; cnt++) {
                        Transition *t = new Transition(dep.locator.getQName(),1,dstSize);
                        QueryExecution *sourceExecution = &executionGraph.getQueryExecution(&sourceQuery,cnt);
                        QueryExecution *targetExecution = &executionGraph.getQueryExecution(&targetQuery,cnt);
                        executionGraph.addTransition(t);
                        executionGraph.createChannel(sourceExecution,t);
                        executionGraph.createChannel(t,targetExecution);
                    }
                } else {
                    LOG_TRACE("build many-to-many sharded");
                    Transition *t = new Transition(dep.locator.getQName(), srcSize, dstSize);
                    string shardingStrategyName = databaseRegistry.getShardingStrategyName(targetQuery.getDatabaseId());
                    string shardColSearchExpr = databaseRegistry.getShardColumn(targetQuery.getDatabaseId());
                    shared_ptr<ShardingStrategy> sharder = extensionLoader.getShardingStrategy(shardingStrategyName);
                    LOG_TRACE("set sharder to " << sharder);
                    t->setSharder(sharder);
                    t->setShardColSearchExpr(shardColSearchExpr);
                    for (size_t cnt=0; cnt< srcSize; cnt++) {
                        QueryExecution& sourceExecution = executionGraph.getQueryExecution(&sourceQuery,cnt);
                        executionGraph.createChannel(&sourceExecution,t);
                    }
                    for (size_t cnt=0; cnt< dstSize; cnt++) {
                        QueryExecution& targetExecution = executionGraph.getQueryExecution(&targetQuery,cnt);
                        executionGraph.createChannel(t,&targetExecution);
                    }
                    executionGraph.addTransition(t);
                }
            } else if (dstSize == 1 && srcSize > 1) {
                // many to one
                Transition *t = new Transition(dep.locator.getQName(),srcSize, dstSize);
                for (size_t cnt=0;cnt<srcSize; cnt++) {
                    QueryExecution *sourceExecution = &executionGraph.getQueryExecution(&sourceQuery,cnt);
                    executionGraph.createChannel(sourceExecution,t);
                }
                executionGraph.addTransition(t);
                QueryExecution *targetExecution = &executionGraph.getQueryExecution(&targetQuery,0);
                executionGraph.createChannel(t,targetExecution);
            } else if (dstSize > 1 && srcSize == 1) {
                LOG_DEBUG("prepare one-to-many transition");
                Transition *t = new Transition(dep.locator.getQName(),srcSize,dstSize);
                string shardingStrategyName = databaseRegistry.getShardingStrategyName(targetQuery.getDatabaseId());
                if (shardingStrategyName.empty()) {
                    LOG_WARN("no sharding strategy for database " << targetQuery.getDatabaseId());
                } else {
                    LOG_DEBUG("sharder name " << shardingStrategyName);
                    shared_ptr<ShardingStrategy> sharder = extensionLoader.getShardingStrategy(shardingStrategyName);
                    LOG_TRACE("set sharder to " << sharder);
                    t->setSharder(sharder);
                }
                QueryExecution *sourceExecution = &executionGraph.getQueryExecution(&sourceQuery,0);
                executionGraph.createChannel(sourceExecution,t);
                for (size_t cnt=0;cnt<dstSize; cnt++) {
                    QueryExecution *targetExecution = &executionGraph.getQueryExecution(&targetQuery,cnt);
                    executionGraph.createChannel(t,targetExecution);
                }
                executionGraph.addTransition(t);
            }
        }
    }
    LOG_TRACE("populate transitions done");
}

void dump_md5_sources(string queryName, string md5, string source) {
    File md5dir{"md5"};
    if (!md5dir.exists()) {
        md5dir.mkdir();
    }
    string fileName = "md5/" + queryName + "_" + md5 + ".txt";
    ofstream os{fileName};
    os.write(source.c_str(),source.size());
    os.close();
}


void QueryProcessor::calculateExecutionIds() {
    LOG_TRACE("calculate execution ids");
    for (auto query:executionGraph.getQueries()) {
        for (auto exec:executionGraph.getQueryExecutions(query)) {
            if (query->getType() == "external") {
                string resultId = exec->getResult("")->calculateMD5Sum();
                LOG_DEBUG("md5 of external " << query->getName() << " -> " << resultId);
                exec->setId(resultId);
                executionGraph.addQueryExecution(exec);
            } else {
                string md5data;
                calculateExecutionId(*exec,md5data);
                string resultId(md5hex(md5data));
                exec->setId(resultId);
                LOG_DEBUG("md5 of query " << query->getName() << " -> " << exec->getId());
                dump_md5_sources(query->getName(), exec->getId(), md5data);
                 executionGraph.addQueryExecution(exec);
            }
        }
    }
    LOG_TRACE("calculate execution ids done");
}

void QueryProcessor::checkConnections() {
    for (auto query:executionGraph.getQueries()) {
        for (auto exec:executionGraph.getQueryExecutions(query)) {
            if (!exec->isDone()) {
                shared_ptr<Event> event(new ExecutionStateChangeEvent(exec->getId(),"PING"));
                fireEvent(event);
                if (exec->isResourceAvailable()) {
                    shared_ptr<Event> event(new ExecutionStateChangeEvent(exec->getId(),"OK"));
                    fireEvent(event);
                } else {
                    throw runtime_error("resource " + exec->getUrl()->getUrl() + " is not available.");
                }
            }
        }
    }
}

void QueryProcessor::calculateExecutionId(QueryExecution& exec, string& md5data) {
    auto dependencies = executionGraph.getDependencies(&exec);
    sort(dependencies.begin(), dependencies.end(), [] (QueryExecution *qe1, QueryExecution *qe2) {
        string key1 = qe1->getUrl()->getUrl(false,false,false) + qe1->getSql();
        string key2 = qe2->getUrl()->getUrl(false,false,false) + qe2->getSql();
        return key1 > key2;
    });
    string url = exec.getUrl()->getUrl(false,false,true);
    md5data.append(url + '\n' + exec.getSql());
    for (auto dependency:dependencies) {
        md5data.append("\n");
        md5data.append(dependency->getUrl()->getUrl(false,false,false) + "\n" + dependency->getSql());
    }
}

vector<QueryExecution*> QueryProcessor::findExecutables() {
    vector<QueryExecution*> executables;
    for (auto& exec:executionGraph.getQueryExecutions()) {
        if (!exec->isDone() && exec->isComplete() && !exec->isScheduled()) {
            executables.push_back(exec);
        }
    }
    return executables;
}

void QueryProcessor::cacheItem(string resultId) {
    LOG_DEBUG("save cache item "+resultId);
    QueryExecution& exec = executionGraph.getQueryExecution(resultId);
    File linkPath{outputDir + "/" + exec.getName() + ".csv"};
    uint64_t rowCount = exec.getResult("")->getRowCount();
    cacheRegistry.registerItem(resultId,Time(),exec.getDuration(),linkPath.abspath(),"csv", rowCount);
    exec.getResult("")->save(cacheRegistry.getPath(resultId));
    LOG_TRACE("save data done");
    cacheRegistry.save(resultId);
    if (linkPath.exists()) {
        linkPath.remove();
    }
    linkPath.linkTo(cacheRegistry.getPath(resultId));
}

void QueryProcessor::handleEvent(shared_ptr<Event> event) {
    if (event->type==EventType::PROCESSED) {
        QueryExecution& result = executionGraph.getQueryExecution(event->resultId);
        LOG_DEBUG("PROCESSED: " << result.getSql() << " id=" << result.getName());
        result.setDone();
        if (result.getResult("")==nullptr) {
            THROW_EXC("result not ready");
        }
        LOG_DEBUG("doTransitions");
        vector<Channel*> outputChannels = executionGraph.getOutputChannels(&result);
        for (auto channel:outputChannels) {
            channel->open();
            channel->send(result.getResult(""));
            channel->close();
        }
        cacheItem(event->resultId);
        result.release();
    }
    vector<QueryExecution*> exec = findExecutables();
    if (!exec.empty()) {
        LOG_DEBUG("found " << exec.size() << " executables");
        for (auto result:exec) {
            LOG_DEBUG("schedule executable  " << result->getSql());
            string sql = result->inject(result->getSql(), copyThreshold);
            ExecutionHandler *eh = dynamic_cast<ExecutionHandler*>(result);
            // queryExecutor->addQuery(result->getId(), result->getConnectionUrl().getUrl(true,false,true), sql, eh);
            result->schedule();
        }
    }
    fireEvent(event);
}

ExecutionGraph& QueryProcessor::getExecutionGraph() {
    return executionGraph;
}

}


