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
DECLARE_LOGGER("Event");


void EventProducer::addEventListener(EventListener *listener) {
    listeners.push_back(listener);
}
void EventProducer::fireEvent(shared_ptr<Event> event) {
    LOG_DEBUG("fireEvent to " << listeners.size() << " listeners");
    for (auto& listener:listeners) {
        LOG_DEBUG("fireEvent");
        listener->handleEvent(event);
    }
}

ostream& operator<<(std::ostream& cout,const EventType type) {
    switch(type) {
        case EventType::APPLICATION_INITIALIZED:
            cout << "APPLICATION_INITIALIZED";
            break;
        case EventType::APPLICATION_FINISHED:
            cout << "APPLICATION_FINISHED";
            break;
        case EventType::INITIALIZE:
            cout << "INITIALIZE";
            break;
        case EventType::PROCESSED:
            cout << "PROCESSED";
            break;
        case EventType::EXECUTION_STATE_CHANGE:
            cout << "EXECUTION_STATE_CHANGE";
            break;
        case EventType::APPLICATION_FAILED:
            cout << "APPLICATION_FAILED";
            break;
        case EventType::APPLICATION_CANCELED:
            cout << "APPLICATION_CANCELED";
            break;
        case EventType::RECEIVE_DATA:
            cout << "RECEIVE_DATA";
            break;
        case EventType::SENT_DATA:
            cout << "SENT_DATA";
            break;
        case EventType::CACHE_LOADED:
            cout << "CACHE_LOADED";
            break;
        case EventType::QUERY_PREPARED:
            cout << "QUERY_PREPARED";
            break;
        default:
            cout << "UNKNOWN";
    }
    return cout;

}

}
