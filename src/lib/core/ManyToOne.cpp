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
	LOG_DEBUG("process many to one");
	vector<shared_ptr<TableData>> sources;
	for (auto& dep:getDependencies()) {
		sources.push_back(dep.second);
	}
	auto result = TableDataFactory::getInstance().join(sources);
	setResult("",result);
	setDone();
	shared_ptr<Event> event(new Event(EventType::PROCESSED,getId()));
    fireEvent(event);
	return true;
}

bool ManyToOne::isTransition() {
	return true;
}
}


