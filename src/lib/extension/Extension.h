/*
 * extension.h
 *
 *  Created on: Dec 21, 2013
 *      Author: arnd
 */

#ifndef EXTENSION_H_
#define EXTENSION_H_

#include <string>

#include "sharding/ShardingStrategy.h"

namespace db_agg {

class Extension {
public:
    virtual ShardingStrategy *getShardingStrategy(std::string name) = 0;
    virtual ~Extension() {};
};

}


#endif /* EXTENSION_H_ */
