/*
 * Port.cpp
 *
 *  Created on: Dec 30, 2014
 *      Author: arnd
 */

#include "Port.h"
#include "utils/logging.h"

using namespace std;

namespace db_agg {

DECLARE_LOGGER("Port")

Port::Port(string id, string name): id(id), name(name) {}

string Port::getId() {
    return id;
}

void Port::setId(string portId) {
    this->id = portId;
}

string Port::getName() {
    return name;
}

void Port::setResult(std::shared_ptr<TableData> result) {
    this->result = result;
}

std::shared_ptr<TableData> Port::getResult() {
    return result;
}

void Port::release() {
    result.reset();
}


}


