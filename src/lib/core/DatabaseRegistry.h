#ifndef DATABASEREGISTRY_H_
#define DATABASEREGISTRY_H_

#include <set>
#include <string>
#include <vector>

extern "C" {
    #include <libxml/tree.h>
    #include <libxml/parser.h>
    #include <libxml/xpath.h>
    #include <libxml/xpathInternals.h>
}

#include "core/Connection.h"

namespace db_agg {

class DatabaseRegistry {
private:
    xmlXPathContextPtr xpathCtx;
    xmlDocPtr doc;
    std::string databaseNamingStrategy;
    Connection getUrl(xmlElementPtr databaseNode);
    std::string evaluateXPath(std::string expr);
public:
    DatabaseRegistry(std::string regfile);
    ~DatabaseRegistry();
    std::vector<Connection> getUrls(std::string database, std::string environment, short shardId);
    std::string getDatabaseByNamespace(std::set<std::string> namespaces);
    std::string getShardingStrategyName(std::string databaseId);
    std::string getShardColumn(std::string databaseId);
    Connection getWorker();
    std::vector<std::string> getSystems();
    std::string getDatabaseNamingStrategy();
};

}

#endif /* DATABASEREGISTRY_H_ */
