/*
 * AsyncEventProducer.cpp
 *
 *  Created on: Aug 11, 2014
 *      Author: arnd
 */


#include "AsyncEvent.h"
#include "utils/logging.h"

using namespace std;

namespace db_agg {
DECLARE_LOGGER("AsyncEvent")

shared_ptr<Event> AsyncEventListener::getEvent() {
    assert(producer != nullptr);
    return producer->getEvent(this);
}

void AsyncEventListener::setProducer(AsyncEventProducer *producer) {
    assert(producer != nullptr);
    this->producer = producer;
}

void AsyncEventListener::processEvent() {
    shared_ptr<Event> event = getEvent();
    if (event != nullptr) {
        LOG_ERROR("handle event " << (int)event->type);
        handleEvent(event);
    }
}

shared_ptr<Event> AsyncEventProducer::getEvent(AsyncEventListener *listener) {
    if (queues.find(listener) == queues.end()) {
        return nullptr;
    }
    if (queues[listener].empty()) {
        return nullptr;
    }
    std::shared_ptr<Event> event  = queues[listener].front();
    LOG_DEBUG("got event for listener event = " << (int)event->type << " result id = " << event->resultId);
    queues[listener].pop_front();
    return event;
}

void AsyncEventProducer::fireEvent(shared_ptr<Event> event) {
    EventProducer::fireEvent(event);
    for (auto listener:listeners) {
        queues[listener].push_back(event);
    }
}

void AsyncEventProducer::addAsyncListener(AsyncEventListener *listener) {
    listener->setProducer(this);
    listeners.push_back(listener);
}
}
