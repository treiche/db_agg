/*
 * Event.cpp
 *
 *  Created on: Jan 24, 2014
 *      Author: arnd
 */


#include "event/Event.h"

#include "utils/logging.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("Event"));


void EventProducer::addEventListener(EventListener *listener) {
    listeners.push_back(listener);
}
void EventProducer::fireEvent(Event& event) {
    LOG_DEBUG("fireEvent to " << listeners.size() << " listeners");
    for (auto& listener:listeners) {
        LOG_DEBUG("fireEvent");
        listener->handleEvent(event);
    }
}

}
