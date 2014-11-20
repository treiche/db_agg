#include "cache/CacheRegistry.h"

#include <jansson.h>
#include "utils/logging.h"
#include <fstream>
#include <stdexcept>
#include <utility>

#include "utils/File.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
    static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("CacheRegistry"));

    CacheRegistry::CacheRegistry(string cacheDir):
        cacheDir(cacheDir) {
    }

    void CacheRegistry::registerItem(std::string resultId, std::string execId, Time lastExecuted, size_t lastDuration, string path, string format, uint64_t rowCount) {
        if (items.find(resultId)==items.end()) {
            items[resultId] = CacheItem();
        }
        items[resultId].id = resultId;
        items[resultId].execId = execId;
        items[resultId].lastExecuted = lastExecuted;
        items[resultId].lastDuration = lastDuration;
        items[resultId].path = cacheDir + "/" + resultId + "." + format;
        items[resultId].format = format;
        items[resultId].rowCount = rowCount;
        items[resultId].links.insert(path);
    }

    void CacheRegistry::save(string id) {
        if (items.find(id) == items.end()) {
            THROW_EXC("cache item with id '" << id << "' does not exist");
        }
        CacheItem& item = items[id];
        json_t *jo = json_object();
        json_object_set_new(jo, "execId", json_string(item.execId.c_str()));
        json_object_set_new(jo, "lastExecuted", json_string(item.lastExecuted.to_string().c_str()));
        json_object_set_new(jo, "lastDuration", json_integer(item.lastDuration));
        json_object_set_new(jo, "rowCount", json_integer(item.rowCount));
        json_object_set_new(jo, "path", json_string(item.path.c_str()));
        json_object_set_new(jo, "format", json_string(item.format.c_str()));
        json_t *jsonLinks = json_array();
        for (string link:item.links) {
            json_array_append_new(jsonLinks, json_string(link.c_str()));
        }
        json_object_set_new(jo, "links", jsonLinks);
        string cacheItemFile = cacheDir + "/" + id + ".cache";
        if (json_dump_file(jo, cacheItemFile.c_str(), JSON_INDENT(2)) == -1) {
            throw runtime_error("saving cache registry file failed");
        }
        json_decref(jo);
    }

    void CacheRegistry::load(string id) {
        json_t *value;
        json_error_t error;
        string cacheItemFile = cacheDir + "/" + id + ".cache";
        value = json_load_file(cacheItemFile.c_str(), 0, &error);
        if (!value) {
            LOG_DEBUG("loading cache item '" << id << "' failed");
            return;
        }
        CacheItem ci;
        json_t *jsExecId = json_object_get(value,"execId");
        if (jsExecId) {
        	ci.execId = json_string_value(jsExecId);
        }
        Time lastExecuted(json_string_value(json_object_get(value,"lastExecuted")));
        ci.lastExecuted = lastExecuted;
        ci.lastDuration = json_integer_value(json_object_get(value,"lastDuration"));
        ci.rowCount = json_integer_value(json_object_get(value,"rowCount"));
        ci.path = json_string_value(json_object_get(value,"path"));
        ci.format = json_string_value(json_object_get(value,"format"));
        json_t *jsonLinks = json_object_get(value,"links");
        size_t linkCount = json_array_size(jsonLinks);
        for (size_t idx = 0; idx < linkCount; idx++) {
            ci.links.insert(json_string_value(json_array_get(jsonLinks, idx)));
        }
        items[id] = ci;
        json_decref(value);
    }

    bool CacheRegistry::exists(string id) {
        if (items.find(id) != items.end()) {
            return true;
        }
        File cacheItemFile(cacheDir + "/" + id + ".cache");
        if (cacheItemFile.exists()) {
        	load(id);
        	return true;
        }
        return false;
    }

    string CacheRegistry::getPath(std::string id) {
        return items[id].path;
    }

    uint64_t CacheRegistry::getRowCount(std::string id) {
        return items[id].rowCount;
    }

    void CacheRegistry::removeOrphans() {
        set<string> toBeDeleted;
        for (auto& item:items) {
            File cacheFile(item.second.path);
            if (!cacheFile.exists()) {
                LOG_INFO("remove orphan "+item.second.path);
                for (string link:item.second.links) {
                    File linkPath(link);
                    linkPath.remove();
                }
                toBeDeleted.insert(item.first);
            }
        }
        for (auto& resultId:toBeDeleted) {
            items.erase(resultId);
        }
    }

    CacheItem& CacheRegistry::get(std::string id) {
        assert(items.find(id) != items.end());
        return items[id];
    }

    shared_ptr<TableData> CacheRegistry::getData(string id) {
        return TableDataFactory::getInstance().load(getPath(id));
    }
}




