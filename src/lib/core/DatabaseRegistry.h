#ifndef DATABASEREGISTRY_H_
#define DATABASEREGISTRY_H_

#include <set>
#include <string>
#include <vector>
#include <memory>

extern "C" {
    #include <libxml/tree.h>
    #include <libxml/parser.h>
    #include <libxml/xpath.h>
    #include <libxml/xpathInternals.h>
}

#include "core/Url.h"
#include "sharding/ShardingStrategyConfiguration.h"

namespace db_agg {

class DatabaseRegistry {
private:
    xmlXPathContextPtr xpathCtx;
    xmlDocPtr doc;
    std::string databaseNamingStrategy;
    std::shared_ptr<Url> getUrl(xmlElementPtr databaseNode);
    std::shared_ptr<Url> getUrlFromServerNode(xmlElementPtr databaseNode);
    std::string evaluateXPath(std::string expr);
public:
    DatabaseRegistry(std::string regfile);
    ~DatabaseRegistry();
    std::vector<std::shared_ptr<Url>> getUrls(std::string database, std::string environment, short shardId);
    std::vector<std::shared_ptr<Url>> getUrls(std::string environment, std::string type);
    std::string getDatabaseByNamespace(std::set<std::string> namespaces);
    std::vector<ShardingStrategyConfiguration> getShardingStrategies(std::string databaseId);
    std::shared_ptr<Url> getWorker();
    std::vector<std::string> getSystems();
    std::string getDatabaseNamingStrategy();
};

}

#endif /* DATABASEREGISTRY_H_ */
