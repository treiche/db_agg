/*
 * Builtin.h
 *
 *  Created on: Mar 30, 2014
 *      Author: arnd
 */

#ifndef BUILTIN_H_
#define BUILTIN_H_

#include <string>
#include "Extension.h"
#include "sharding/ShardingStrategy.h"

namespace db_agg {
class Builtin: public Extension {
    virtual ShardingStrategy *getShardingStrategy(std::string name) override;
    virtual ~Builtin();
};
}



#endif /* BUILTIN_H_ */
