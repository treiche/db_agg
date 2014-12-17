/*
 * ExtensionLoader.cpp
 *
 *  Created on: Dec 21, 2013
 *      Author: arnd
 */

#include "extension/ExtensionLoader.h"

#include <dlfcn.h>
#include "utils/logging.h"
#include <iostream>
#include <utility>
#include <vector>

#include "utils/File.h"
#include "Builtin.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
    DECLARE_LOGGER("ExtensionLoader");

    ExtensionLoader::~ExtensionLoader() {
        LOG_TRACE("delete extension loader " << this);
        for (auto &ext:extensions) {
            delete ext.second;
        }
        for (auto &lib:libraries) {
            dlclose(lib.second);
        }
        LOG_TRACE("delete extension loader done");
    }

    void ExtensionLoader::loadExtensions(std::string extensionDir) {
        LOG_DEBUG("load extensions from " << extensionDir);
        File extDir(extensionDir);
        vector<string> childs;
        extDir.getChilds(childs);
        for (auto& child:childs) {
            LOG_DEBUG("load extension " << child);
            string path = extensionDir + "/" + child;
            void *handle = dlopen(path.c_str(),RTLD_NOW);
            if (!handle) {
                LOG_WARN("loading extension " << child << " failed. error: " << dlerror());
                continue;
            }
            Extension *(*getExtension)() = (Extension *(*)())dlsym(handle,"getExtension");
            if (!getExtension) {
                LOG_WARN("no method getExtension found: " << dlerror());
                continue;
            }
            extensions[child] = getExtension();
            libraries[child] = handle;
        }
        extensions["builtin"] = new Builtin();
    }

    shared_ptr<ShardingStrategy> ExtensionLoader::getShardingStrategy(string name) {
        return getExtension(ComponentType::SHARDING_STRATEGY, name).getShardingStrategy(name);
    }

    QueryExecution *ExtensionLoader::getQueryExecution(std::string name) {
        return getExtension(ComponentType::QUERY_EXECUTION, name).getQueryExecution(name);
    }

    shared_ptr<DependencyInjector> ExtensionLoader::getDependencyInjector(std::string name) {
        return getExtension(ComponentType::DEPENDENCY_INJECTOR, name).getDependencyInjector(name);
    }

    Extension& ExtensionLoader::getExtension(ComponentType componentType, std::string name) {
        vector<Extension*> candidates;
        for (auto extension:extensions) {
            map<ComponentType,vector<string>> components = extension.second->getProvidedComponents();
            for (auto& component:components) {
                if (component.first == componentType) {
                    for (auto cn:component.second) {
                        if (cn == name) {
                            candidates.push_back(extension.second);
                        }
                    }
                }
            }
        }
        if (candidates.size() == 0) {
            throw runtime_error("no extension '" + name + "' supported");
        }
        if (candidates.size() > 1) {
            throw runtime_error("found more than one extension");
        }
        return *candidates[0];
    }

    vector<string> ExtensionLoader::getAvailableExecutorNames() {
        vector<string> executors;
        for (auto extension:extensions) {
            map<ComponentType,vector<string>> components = extension.second->getProvidedComponents();
            for (auto& component:components) {
                if (component.first == ComponentType::QUERY_EXECUTION) {
                    for (auto cn:component.second) {
                        executors.push_back(cn);
                    }
                }
            }
        }
        return executors;
    }
}
