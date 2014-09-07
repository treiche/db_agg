/*
 * ManyToOne.cpp
 *
 *  Created on: Sep 7, 2014
 *      Author: arnd
 */

#include "ManyToOne.h"
#include "table/TableDataFactory.h"

using namespace std;


namespace db_agg {
bool ManyToOne::process() {
	vector<shared_ptr<TableData>> sources;
	for (auto& dep:getDependencies()) {
		sources.push_back(dep.second);
	}
	auto result = TableDataFactory::getInstance().join(sources);
	setResult("",result);
	setDone();
	return true;
}
}


