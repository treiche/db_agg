#include "cache/CacheRegistry.h"

#include <jansson.h>
#include <log4cplus/logger.h>
#include <fstream>
#include <stdexcept>
#include <utility>

#include "utils/File.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
    static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("CacheRegistry"));

    CacheRegistry::CacheRegistry(string cacheDir, string cacheRegistryFile):
        cacheDir(cacheDir),
        cacheRegistryFile(cacheRegistryFile) {
        load();
    }

    void CacheRegistry::registerItem(std::string id, Time lastExecuted, size_t lastDuration, string path, string format, uint64_t rowCount) {
        if (items.find(id)==items.end()) {
            items[id] = CacheItem();
        }
        items[id].id = id;
        items[id].lastExecuted = lastExecuted;
        items[id].lastDuration = lastDuration;
        items[id].path = cacheDir + "/" + id + "." + format;
        items[id].format = format;
        items[id].rowCount = rowCount;
        items[id].links.insert(path);
    }

    void CacheRegistry::save() {
        json_t *root = json_object();
        for (auto& item:items) {
            json_t *jo = json_object();
            json_object_set_new(jo, "lastExecuted", json_string(item.second.lastExecuted.to_string().c_str()));
            json_object_set_new(jo, "lastDuration", json_integer(item.second.lastDuration));
            json_object_set_new(jo, "rowCount", json_integer(item.second.rowCount));
            json_object_set_new(jo, "path", json_string(item.second.path.c_str()));
            json_object_set_new(jo, "format", json_string(item.second.format.c_str()));
            json_object_set_new(root,item.first.c_str(),jo);
            json_t *jsonLinks = json_array();
            for (string link:item.second.links) {
                json_array_append_new(jsonLinks, json_string(link.c_str()));
            }
            json_object_set_new(jo, "links", jsonLinks);
            //json_decref(jo);
        }
        if (json_dump_file(root,cacheRegistryFile.c_str(),JSON_INDENT(2)) == -1) {
            throw runtime_error("saving cache registry file failed");
        }
        json_decref(root);
    }

    void CacheRegistry::load() {
        json_t *json;
        json_error_t error;
        json = json_load_file(cacheRegistryFile.c_str(), 0, &error);
        if (!json) {
            LOG4CPLUS_DEBUG(LOG, "loading cache registry failed");
            return;
        }
        void *iter = json_object_iter(json);
        while(iter) {
            const char *key = json_object_iter_key(iter);
            json_t *value = json_object_iter_value(iter);
            CacheItem ci;
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
            items[key]=ci;
            iter = json_object_iter_next(json, iter);
        }
        json_decref(json);
        removeOrphans();
    }

    bool CacheRegistry::exists(string id) {
        if (items.find(id) != items.end()) {
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
                LOG4CPLUS_INFO(LOG, "remove orphan "+item.second.path);
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

}




