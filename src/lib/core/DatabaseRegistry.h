#ifndef DATABASEREGISTRY_H_
#define DATABASEREGISTRY_H_

#include <set>
#include <string>
#include <vector>

#include "core/Connection.h"

namespace db_agg {

class DatabaseRegistry {
private:
    struct XImpl;
    XImpl *pImpl = 0;
public:
    DatabaseRegistry();
    ~DatabaseRegistry();
    DatabaseRegistry(std::string regfile);
    std::vector<Connection> getUrls(std::string database, std::string environment, short shardId);
    std::string getDatabaseByNamespace(std::set<std::string> namespaces);
    std::string getShardingStrategyName(std::string databaseId);
    Connection getWorker();
    std::vector<std::string> getSystems();
    std::string getDatabaseNamingStrategy();
};

}

#endif /* DATABASEREGISTRY_H_ */
