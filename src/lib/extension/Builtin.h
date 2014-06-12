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
#include "injection/DefaultDependencyInjector.h"
#include "injection/NoResultInjector.h"

namespace db_agg {
class Builtin: public Extension {
private:
    static std::map<ComponentType,std::vector<std::string>> components;
public:
    virtual std::map<ComponentType,std::vector<std::string>> getProvidedComponents() override;
    virtual QueryExecution *getQueryExecution(std::string name) override;
    virtual std::shared_ptr<ShardingStrategy> getShardingStrategy(std::string name) override;
    virtual std::shared_ptr<DependencyInjector> getDependencyInjector(std::string name) override;
    virtual ~Builtin();
};
}



#endif /* BUILTIN_H_ */
