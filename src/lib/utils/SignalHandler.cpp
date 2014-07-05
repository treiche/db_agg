/*
 * SignalHandler.cpp
 *
 *  Created on: Jan 26, 2014
 *      Author: arnd
 */

#include "SignalHandler.h"

#include "utils/logging.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("SignalHandler"));

void GlobalSignalHandler::handleSignal(int signal) {
    //cout << "GlobalSignalHandler::handle signal" << endl;
    LOG_TRACE("received signal " << signal);
    for (auto& child:childs) {
        //cout << "GlobalSignalHandler::handle forward " << signal << endl;
        LOG_TRACE("forward signal " << signal);
        child->handleSignal(signal);
    }
}

void GlobalSignalHandler::addHandler(SignalHandler *handler) {
    childs.push_back(handler);
}

GlobalSignalHandler GlobalSignalHandler::instance;

GlobalSignalHandler& GlobalSignalHandler::getInstance() {
    return instance;
}


GlobalSignalHandler::~GlobalSignalHandler() {

}

}


