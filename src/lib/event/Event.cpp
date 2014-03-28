/*
 * Event.cpp
 *
 *  Created on: Jan 24, 2014
 *      Author: arnd
 */


#include "event/Event.h"

#include <log4cplus/logger.h>

using namespace std;
using namespace log4cplus;

namespace db_agg {
static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("Event"));


void EventProducer::addEventListener(EventListener *listener) {
    listeners.push_back(listener);
}
void EventProducer::fireEvent(Event& event) {
    LOG4CPLUS_DEBUG(LOG,"fireEvent to " << listeners.size() << " listeners");
    for (auto& listener:listeners) {
        LOG4CPLUS_DEBUG(LOG,"fireEvent");
        listener->handleEvent(event);
    }
}

}
