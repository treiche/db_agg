/*
 * Event.h
 *
 *  Created on: Jan 19, 2014
 *      Author: arnd
 */

#ifndef EVENT_H_
#define EVENT_H_

#include <string>
#include <vector>

#include "utils/Time.h"

namespace db_agg {
enum class EventType {
    APPLICATION_INITIALIZED=1,
    APPLICATION_FINISHED=2,
    INITIALIZE=3,
    PROCESSED=4,
    EXECUTION_STATE_CHANGE=5,
    APPLICATION_FAILED=6,
    APPLICATION_CANCELED=7,
    RECEIVE_DATA=8,
    CACHE_LOADED=9
};

class Event {
public:
    EventType type;
    std::string resultId;
    Event(EventType type):
        Event(type,"") {}
    Event(EventType type, std::string resultId):
        type(type),
        resultId(resultId) {}
};

class CacheLoadEvent: public Event {
public:
    Time lastExecuted;
    size_t lastDuration;
    size_t lastRowsReceived;
    CacheLoadEvent(std::string resultId, Time lastExecuted, size_t lastDuration, size_t lastRowsReceived):
        Event(EventType::CACHE_LOADED, resultId),
        lastExecuted(lastExecuted),
        lastDuration(lastDuration),
        lastRowsReceived(lastRowsReceived) {
    }
};

class ExecutionStateChangeEvent: public Event {
public:
    ExecutionStateChangeEvent(std::string resultId, std::string state):
        Event(EventType::EXECUTION_STATE_CHANGE, resultId),
        state(state) { }
    std::string state;
};

class ApplicationFailedEvent: public Event {
public:
    ApplicationFailedEvent(std::string reason):
        Event(EventType::APPLICATION_FAILED, ""),
        reason(reason) {
    }
    std::string reason;
};

class ReceiveDataEvent: public Event {
public:
    size_t rowsReceived = 0;
    ReceiveDataEvent(std::string resultId, size_t rowsReceived):
        Event(EventType::RECEIVE_DATA, resultId),
        rowsReceived(rowsReceived) {}
};

class EventListener {
public:
    virtual void handleEvent(Event& event) {}
    virtual ~EventListener() {}
};

class EventProducer {
private:
    std::vector<EventListener*> listeners;
public:
    void addEventListener(EventListener *listener);
    void fireEvent(Event& event);
};

}



#endif /* EVENT_H_ */
