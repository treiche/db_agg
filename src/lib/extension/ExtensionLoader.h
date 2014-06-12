/*
 * ExtensionLoader.h
 *
 *  Created on: Dec 21, 2013
 *      Author: arnd
 */

#ifndef EXTENSIONLOADER_H_
#define EXTENSIONLOADER_H_

#include <map>
#include <string>

#include "extension/Extension.h"
#include "sharding/ShardingStrategy.h"

namespace db_agg {

class ExtensionLoader {
    std::map<std::string,Extension*> extensions;
    std::map<std::string,void*> libraries;
public:
    ~ExtensionLoader();
    void loadExtensions(std::string extensionDir);
    std::shared_ptr<ShardingStrategy> getShardingStrategy(std::string name);
    std::shared_ptr<DependencyInjector> getDependencyInjector(std::string name);
    QueryExecution *getQueryExecution(std::string name);
    Extension& getExtension(ComponentType component, std::string name);
    std::vector<std::string> getAvailableExecutorNames();
};


}
#endif /* EXTENSIONLOADER_H_ */
