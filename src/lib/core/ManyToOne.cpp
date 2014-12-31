/*
 * ManyToOne.cpp
 *
 *  Created on: Sep 7, 2014
 *      Author: arnd
 */

#include "ManyToOne.h"
#include "utils/logging.h"
#include "table/TableDataFactory.h"
#include <iostream>

using namespace std;

DECLARE_LOGGER("ManyToOne");

namespace db_agg {
bool ManyToOne::process() {
	LOG_DEBUG("process many to one state = " << getState());
	vector<shared_ptr<TableData>> sources;
	for (auto& dep:getInPorts()) {
	    LOG_DEBUG("dependency = " << dep->getName());
		sources.push_back(dep->getResult());
	}
	auto result = TableDataFactory::getInstance().join(sources);
	LOG_INFO("set result to default port " << getName() << " address=" << this);
	setResult("",result);
	//setState(QueryExecutionState::DONE);
	return true;
}

bool ManyToOne::isTransition() {
	return true;
}
}


