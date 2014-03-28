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
#include "core/ShardingStrategy.h"

namespace db_agg {

class ExtensionLoader {
    std::map<std::string,Extension*> extensions;
    std::map<std::string,void*> libraries;
public:
    ~ExtensionLoader();
    void loadExtensions(std::string extensionDir);
    ShardingStrategy *getShardingStrategy(std::string name);
};


}
#endif /* EXTENSIONLOADER_H_ */
