/*
 * ExtensionLoader.cpp
 *
 *  Created on: Dec 21, 2013
 *      Author: arnd
 */

#include "extension/ExtensionLoader.h"

#include <dlfcn.h>
#include <log4cplus/logger.h>
#include <iostream>
#include <utility>
#include <vector>

#include "utils/File.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
    static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("ExtensionLoader"));

    ExtensionLoader::~ExtensionLoader() {
        LOG4CPLUS_TRACE(LOG,"delete extension loader " << this);
        for (auto &ext:extensions) {
            // delete ext.second;
        }
        for (auto &lib:libraries) {
            dlclose(lib.second);
        }
        LOG4CPLUS_TRACE(LOG,"delete extension loader done");
    }

    void ExtensionLoader::loadExtensions(std::string extensionDir) {
        LOG4CPLUS_DEBUG(LOG,"load extensions from " << extensionDir);
        File extDir(extensionDir);
        vector<string> childs;
        extDir.getChilds(childs);
        for (auto& child:childs) {
            LOG4CPLUS_DEBUG(LOG,"load extension " << child);
            string path = extensionDir + "/" + child;
            void *handle = dlopen(path.c_str(),RTLD_NOW);
            if (!handle) {
                LOG4CPLUS_WARN(LOG,"loading extension " << child << " failed. error: " << dlerror());
                continue;
            }
            Extension *(*getExtension)() = (Extension *(*)())dlsym(handle,"getExtension");
            if (!getExtension) {
                LOG4CPLUS_WARN(LOG,"no method getExtension found: " << dlerror());
            }
            extensions[child] = getExtension();
            libraries[child] = handle;
        }
    }

    ShardingStrategy *ExtensionLoader::getShardingStrategy(string name) {
        for (auto ext:extensions) {
            LOG4CPLUS_INFO(LOG, "search sharding strategy '" << name << "' in extensions " << ext.first);
            ShardingStrategy *ss = ext.second->getShardingStrategy(name);
            if (ss != nullptr) {
                LOG4CPLUS_INFO(LOG,"found sharding strategy '" << name << "' in module "<< ext.first);
                return ss;
            }
        }
        return nullptr;
    }

}
