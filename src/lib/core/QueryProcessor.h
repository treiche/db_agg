/*
 * QueryProcessor2.h
 *
 *  Created on: Jan 10, 2014
 *      Author: arnd
 */

#ifndef QUERYPROCESSOR_H_
#define QUERYPROCESSOR_H_

#include <string>
#include <list>

#include "core/DatabaseRegistry.h"
#include "utils/PasswordManager.h"
#include "core/QueryParser.h"
#include "extension/ExtensionLoader.h"
#include "cache/CacheRegistry.h"
#include "core/AsyncQueryExecutor.h"
#include "table/TableData.h"
#include "ExecutionGraph.h"
#include "graph/ExecutionGraph2.h"

namespace db_agg {
class QueryProcessor : public EventListener, public EventProducer {
    QueryParser& queryParser;
    DatabaseRegistry& databaseRegistry;
    ExtensionLoader& extensionLoader;
    PasswordManager& passwordManager;
    CacheRegistry& cacheRegistry;
    std::string outputDir;
    bool disableCache;
    size_t copyThreshold;
    std::map<std::string,std::string> externalSources;
    ExecutionGraph executionGraph;
    ExecutionGraph2 executionGraph2;
    size_t statementTimeout;
    std::map<std::string,std::string> queryParameter;
    void populateUrls(std::string environment);
    void populateTransitions();
    void loadFromCache();
    void calculateExecutionIds();
    void calculateExecutionId(QueryExecution& exec,std::string& md5data);
    void checkConnections();
    std::vector<QueryExecution*> findExecutables();
    void cacheItem(std::string resultId);
    void loadExternalSources();
    void cleanUp();
    bool dontExecute;
    bool stopped = false;
    size_t maxParallelExecutions = 1000;
public:
    QueryProcessor(
            QueryParser& queryParser,
            DatabaseRegistry& databaseRegistry,
            ExtensionLoader& extensionLoader,
            PasswordManager& passwordManager,
            CacheRegistry& cacheRegistry,
            std::string outputDir,
            bool disableCache,
            size_t copyThreshold,
            std::map<std::string,std::string> externalSources,
            size_t statementTimeout,
            std::map<std::string,std::string> queryParameter,
            bool dontExecute,
            size_t maxParallelExecutions
    );
    ~QueryProcessor();
    void process(std::string query, std::string environment);
    void handleEvent(std::shared_ptr<Event> event) override;
    void stop();
    ExecutionGraph& getExecutionGraph();
};
}


#endif /* QUERYPROCESSOR_H_ */
