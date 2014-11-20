#ifndef CACHEREGISTRY_H_
#define CACHEREGISTRY_H_

#include <map>
#include <set>
#include <string>
#include <memory>
#include "table/TableDataFactory.h"

#include "utils/Time.h"

namespace db_agg {

struct CacheItem {
    std::string id;
    std::string execId;
    Time lastExecuted;
    size_t lastDuration;
    std::string path;
    std::string format;
    std::set<std::string> links;
    uint64_t rowCount;
};

class CacheRegistry {
    std::map<std::string,CacheItem> items;
    std::string cacheDir;
    void removeOrphans();
public:
    CacheRegistry(std::string cacheDir);
    void registerItem(
    		std::string resultId,
    		std::string execId,
    		Time lastExecuted,
    		size_t lastDuration,
    		std::string path,
    		std::string format,
    		uint64_t rowCount
    );
    bool exists(std::string id);
    void save(std::string id);
    void load(std::string id);
    std::string getPath(std::string id);
    std::shared_ptr<TableData> getData(std::string id);
    uint64_t getRowCount(std::string id);
    CacheItem& get(std::string id);
};
}

#endif /* CACHEREGISTRY_H_ */
