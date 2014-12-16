/*
 * Dependency.h
 *
 *  Created on: Dec 16, 2014
 *      Author: arnd
 */

#ifndef DEPENDENCY_H_
#define DEPENDENCY_H_

#include <string>
#include "core/Locator.h"

namespace db_agg {

class Query;

class Dependency {
private:
    Locator locator;
    std::string alias;
    Query *sourceQuery{nullptr};
public:
    Dependency(Locator locator, std::string alias);
    Locator& getLocator();
    std::string getAlias();
    Query* getSourceQuery();
    void setSourceQuery(Query *sourceQuery);
};

}




#endif /* DEPENDENCY_H_ */
