/*
 * QueryProcessor.cpp
 *
 *  Created on: Jan 10, 2014
 *      Author: arnd
 */

#include "core/QueryProcessor.h"
#include "utils/logging.h"
#include "utils/utility.h"
#include "utils/string.h"
#include "utils/md5.h"
#include <fstream>
#include "utils/File.h"
#include "injection/DependencyInjector.h"
#include "utils/SignalHandler.h"
#include "OneToMany.h"
#include "ManyToOne.h"
#include "ManyToMany.h"


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
    calculateExecutionIds();
    shared_ptr<Event> event(new Event(EventType::APPLICATION_INITIALIZED,""));
    fireEvent(event);
    loadFromCache();
    executionGraph.dumpExecutionPlan(outputDir);
    LOG_DEBUG("got " << executionGraph.getQueryExecutions().size() << " executions");
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
        for (auto exec:executionGraph.getQueryExecutions()) {
        	if (!exec->isDone() && exec->isComplete()) {
        		// cout << "exec " << exec->getName() << ": done = " << exec->isDone() << " complete = "  << exec->isComplete() << endl;
        		done = false;
        	}
        }
    } while(!done);
    for (auto kk:executionGraph.getQueryExecutions()) {
    	if (!kk->isDone()) {
    		LOG_ERROR("not done = " << kk->getName() << " complete: " << kk->isComplete());
    	}
    }
}

void QueryProcessor::cleanUp() {
    for (auto exec:executionGraph.getQueryExecutions()) {
        exec->cleanUp();
    }
}

void QueryProcessor::loadFromCache() {
    LOG_INFO("load items from cache");
    for (auto& qr:executionGraph.getQueryExecutions()) {
    	for (auto portName:qr->getPortNames()) {
    		string resultId = qr->getPortId(portName);
			if (cacheRegistry.exists(resultId)) {
				File path(cacheRegistry.getPath(resultId));
				if (path.exists()) {
					LOG_INFO("cache item for " << qr->getName() << "[" << resultId << "] exists");
					if (!disableCache) {
						shared_ptr<TableData> data = cacheRegistry.getData(resultId);
						qr->setResult(portName, data);
						qr->setDone();
						// close all incoming channels here
						// executionGraph.getSources(qr);
						//qr.second->doTransitions();
						File linkPath(outputDir + "/" + qr->getName() + portName + ".csv");
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
    }
    LOG_DEBUG("load items from cache done");
    for (auto& qr:executionGraph.getQueryExecutions()) {
        if (qr->isDone()) {
            for (auto channel:qr->getChannels()) {
            	QueryExecution *target = channel->getTarget();
            	if (!target->isDone()) {
                    if (channel->getState() == ChannelState::READY) {
                    	channel->open();
                		LOG_DEBUG("send data to target " << target->getName() << " port=" << channel->getTargetPort());
                    	channel->send(qr->getResult(channel->getSourcePort()));
                    	channel->close();
                    }
            	}
            	/*
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
                */
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
                  "[environment = " << query->getEnvironment() << ", dbId=" << dbId << "]"
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
                QueryExecution& targetExecution = executionGraph.getQueryExecution(&targetQuery,0);
                assert(sourceExecutions.size() == 1);
                executionGraph.createChannel(sourceExecutions[0], "", &targetExecution, sourceExecutions[0]->getName());
            } else if (dstSize > 1 && dstSize == srcSize) {
                // many to many without sharding
                if (sourceQuery.getDatabaseId() == targetQuery.getDatabaseId()) {
                    for (size_t cnt=0; cnt< dstSize; cnt++) {
                        QueryExecution *sourceExecution = &executionGraph.getQueryExecution(&sourceQuery,cnt);
                        QueryExecution *targetExecution = &executionGraph.getQueryExecution(&targetQuery,cnt);
                        executionGraph.createChannel(sourceExecution,"",targetExecution,sourceQuery.getName());
                    }
                } else {
                    LOG_TRACE("build many-to-many sharded");
                    string shardingStrategyName = databaseRegistry.getShardingStrategyName(targetQuery.getDatabaseId());
                    string shardColSearchExpr = databaseRegistry.getShardColumn(targetQuery.getDatabaseId());
                    LOG_DEBUG("shardColSearchExpr = " << shardColSearchExpr);
                    if (shardColSearchExpr.empty()) {
                    	THROW_EXC("empty shard col expr");
                    }
                    shared_ptr<ShardingStrategy> sharder = extensionLoader.getShardingStrategy(shardingStrategyName);
                    LOG_TRACE("set sharder to " << sharder);
                    ManyToMany *m2m = new ManyToMany(sharder, shardColSearchExpr,dstSize);
                    executionGraph.addQueryExecution(m2m);
                    for (size_t cnt=0; cnt< srcSize; cnt++) {
                        QueryExecution& sourceExecution = executionGraph.getQueryExecution(&sourceQuery,cnt);
                        executionGraph.createChannel(&sourceExecution, to_string(cnt+1),m2m,to_string(cnt+1));
                    }
                    for (size_t cnt=0; cnt< dstSize; cnt++) {
                        QueryExecution& targetExecution = executionGraph.getQueryExecution(&targetQuery,cnt);
                        executionGraph.createChannel(m2m, to_string(cnt+1),&targetExecution,to_string(cnt+1));
                    }
                }
            } else if (dstSize == 1 && srcSize > 1) {
                // many to one
                QueryExecution *targetExecution = &executionGraph.getQueryExecution(&targetQuery,0);
				string md5;
                for (size_t cnt=0;cnt<srcSize; cnt++) {
                    QueryExecution *sourceExecution = &executionGraph.getQueryExecution(&sourceQuery,cnt);
                    md5 += sourceExecution->getUrl()->getUrl(false,false,false) + sourceExecution->getSql();
                }
				string id = md5hex(md5 + ":joined");
				bool execExists = executionGraph.exists(id);
                ManyToOne *m2o = nullptr;
                if (execExists) {
                	m2o = dynamic_cast<ManyToOne*>(&executionGraph.getQueryExecution(id));
                } else {
                	m2o = new ManyToOne();
                }
				vector<string> args;
				vector<string> depNames;
				if (!execExists) {
					for (size_t cnt=0;cnt<srcSize; cnt++) {
						QueryExecution *sourceExecution = &executionGraph.getQueryExecution(&sourceQuery,cnt);
						depNames.push_back(to_string(cnt+1));
						executionGraph.createChannel(sourceExecution,"",m2o,to_string(cnt+1));
					}
					shared_ptr<DependencyInjector> di = extensionLoader.getDependencyInjector("default");
					m2o->init(sourceQuery.getName() + ":joined", id,targetExecution->getUrl(),"query",depNames,di,args);
					m2o->addEventListener(this);
					executionGraph.addQueryExecution(m2o);
                }
                executionGraph.createChannel(m2o,"",targetExecution,sourceQuery.getName());
            } else if (dstSize > 1 && srcSize == 1) {
                if (dep.sourceQuery->getShardId() != -1) {
                	LOG_DEBUG("prepare one to many unsharded transition");
					QueryExecution *sourceExecution = &executionGraph.getQueryExecution(&sourceQuery,0);
					for (size_t cnt=0;cnt<dstSize; cnt++) {
						QueryExecution *targetExecution = &executionGraph.getQueryExecution(&targetQuery,cnt);
						executionGraph.createChannel(sourceExecution,"",targetExecution,sourceExecution->getName());
					}
                } else {
					LOG_DEBUG("prepare one-to-many transition");
					string shardColSearchExpr = databaseRegistry.getShardColumn(targetQuery.getDatabaseId());
					string shardingStrategyName = databaseRegistry.getShardingStrategyName(targetQuery.getDatabaseId());
					shared_ptr<ShardingStrategy> sharder;
					if (shardingStrategyName.empty()) {
						LOG_WARN("no sharding strategy for database " << targetQuery.getDatabaseId());
					} else {
						LOG_DEBUG("sharder name " << shardingStrategyName);
						sharder = extensionLoader.getShardingStrategy(shardingStrategyName);
						LOG_TRACE("set sharder to " << sharder);
					}
					QueryExecution *sourceExecution = &executionGraph.getQueryExecution(&sourceQuery,0);
					string id = md5hex(sourceExecution->getUrl()->getUrl(false,false,false) + sourceExecution->getSql() + ":splitted");
					OneToMany *o2m = nullptr;
					bool execExists = executionGraph.exists(id);
					if (execExists) {
						o2m = dynamic_cast<OneToMany*>(&executionGraph.getQueryExecution(id));
					} else {
						o2m = new OneToMany(sharder, shardColSearchExpr,(size_t) dstSize);
					}
					vector<string> depNames;
					depNames.push_back(sourceExecution->getName());
					vector<string> args;
					shared_ptr<DependencyInjector> di = extensionLoader.getDependencyInjector("default");
					o2m->init(sourceExecution->getName() + ":splitted",id,sourceExecution->getUrl(),"query",depNames,di,args);
					if (!execExists) {
						o2m->addEventListener(this);
						executionGraph.addQueryExecution(o2m);
						executionGraph.createChannel(sourceExecution,"",o2m,sourceExecution->getName());
					}
					//vector<string> portNames;
					for (size_t cnt=0;cnt<dstSize; cnt++) {
						QueryExecution *targetExecution = &executionGraph.getQueryExecution(&targetQuery,cnt);
						executionGraph.createChannel(o2m, to_string(cnt+1), targetExecution, sourceExecution->getName());
						//portNames.push_back(to_string(cnt+1));
					}
					//LOG_DEBUG("set one-to-many portNames = " << join(portNames,","))
					//o2m->setPortNames(portNames);
                }
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
                exec->setPortId("",resultId);
            } else {
                string md5data;
                calculateExecutionId(*exec,md5data);
                string resultId(md5hex(md5data));
                exec->setId(resultId);
                LOG_DEBUG("md5 of query " << query->getName() << " -> " << exec->getId());
                dump_md5_sources(query->getName(), exec->getId(), md5data);
                executionGraph.addQueryExecution(exec);
                for (auto portName:exec->getPortNames()) {
                	string resultId(md5hex(md5data + portName));
                	exec->setPortId(portName,resultId);
                }
            }
        }
    }

    for (auto exec:executionGraph.getQueryExecutions()) {
    	if (exec->isTransition()) {
    		string md5data;
    		calculateExecutionId(*exec,md5data);
            string execId(md5hex(md5data));
            exec->setId(execId);
            dump_md5_sources(exec->getName(), exec->getId(), md5data);
            executionGraph.addQueryExecution(exec);
            for (auto portName:exec->getPortNames()) {
            	string resultId(md5hex(md5data+portName));
            	exec->setPortId(portName,resultId);
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

void QueryProcessor::cacheItem(string execId) {
    LOG_DEBUG("save cache item "+execId);
    QueryExecution& exec = executionGraph.getQueryExecution(execId);
    LOG_DEBUG("execution: " << exec.getName());
    for (auto portName:exec.getPortNames()) {
    	LOG_DEBUG("save port '" << portName << "' with id " << exec.getPortId(portName));
        File linkPath{outputDir + "/" + exec.getName() + portName + ".csv"};
    	string portId = exec.getPortId(portName);
		uint64_t rowCount = exec.getResult(portName)->getRowCount();
		cacheRegistry.registerItem(portId, execId, Time(),exec.getDuration(),linkPath.abspath(),"csv", rowCount);
		exec.getResult(portName)->save(cacheRegistry.getPath(portId));
	    cacheRegistry.save(portId);
	    if (linkPath.exists()) {
	        linkPath.remove();
	    }
	    linkPath.linkTo(cacheRegistry.getPath(portId));
    }
    LOG_TRACE("save data done");
}

void QueryProcessor::handleEvent(shared_ptr<Event> event) {
    if (event->type==EventType::PROCESSED) {
        QueryExecution& result = executionGraph.getQueryExecution(event->resultId);
        LOG_DEBUG("PROCESSED: " << result.getSql() << " name =" << result.getName() << " address=" << &result);
        result.setDone();
        LOG_DEBUG("set execution " << result.getName() << " done.");
        bool allReady = true;
        for (auto channel:result.getChannels()) {
        	bool ready = result.getResult(channel->getSourcePort()) != nullptr;
        	LOG_DEBUG("channel source port = " << channel->getSourcePort() << " ready = " << ready);
        	allReady &= ready;
        }
        LOG_DEBUG("allReady = " << allReady);
        //if (result.getResult("")==nullptr) {
        if (!allReady) {
            THROW_EXC("result not ready");
        }
        LOG_DEBUG("doTransitions");
        //vector<Channel*> outputChannels = executionGraph.getOutputChannels(&result);
        for (auto channel:result.getChannels()) {
            channel->open();
            channel->send(result.getResult(channel->getSourcePort()));
            channel->close();
        }
        LOG_DEBUG("cacheItem " << event->resultId);
        //if (!result.isTransition()) {
        	cacheItem(event->resultId);
        //}
        LOG_DEBUG("cacheItem done");
        result.release();
    }
    vector<QueryExecution*> exec = findExecutables();
    if (!exec.empty()) {
        LOG_DEBUG("found " << exec.size() << " executables");
        for (auto result:exec) {
            LOG_DEBUG("schedule executable  " << result->getSql());
            string sql = result->inject(result->getSql(), copyThreshold);
            ExecutionHandler *eh = dynamic_cast<ExecutionHandler*>(result);
            //queryExecutor->addQuery(result->getId(), result->getConnectionUrl().getUrl(true,false,true), sql, eh);
            result->schedule();
        }
    }
    fireEvent(event);
}

ExecutionGraph& QueryProcessor::getExecutionGraph() {
    return executionGraph;
}

}


