/*
 * Dependency.cpp
 *
 *  Created on: Dec 16, 2014
 *      Author: arnd
 */

#include "Dependency.h"

using namespace std;


namespace db_agg {

Dependency::Dependency(Locator locator, std::string alias) {
    this->locator = locator;
    this->alias = alias;
}

Locator& Dependency::getLocator() {
    return locator;
}

string Dependency::getAlias() {
    return alias;
}

Query* Dependency::getSourceQuery() {
    return sourceQuery;
}

void Dependency::setSourceQuery(Query* sourceQuery) {
    this->sourceQuery = sourceQuery;
}

}

