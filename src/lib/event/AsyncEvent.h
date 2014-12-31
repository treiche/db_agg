/*
 * AsyncEventProducer.h
 *
 *  Created on: Aug 11, 2014
 *      Author: arnd
 */

#ifndef ASYNCEVENTPRODUCER_H_
#define ASYNCEVENTPRODUCER_H_

#include <map>
#include <deque>
#include <vector>
#include <memory>
#include "Event.h"

namespace db_agg {

class AsyncEventProducer;

class AsyncEventListener: public EventListener {
private:
    AsyncEventProducer *producer;
public:
    void setProducer(AsyncEventProducer *producer);
    std::shared_ptr<Event> getEvent();
    void processEvent();
};

class AsyncEventProducer: public EventProducer {
private:
    std::vector<AsyncEventListener*> listeners;
    std::map<AsyncEventListener*,std::deque<std::shared_ptr<Event>>> queues;
public:
    void addAsyncListener(AsyncEventListener *listener);
    std::shared_ptr<Event> getEvent(AsyncEventListener *listener);
    void fireEvent(std::shared_ptr<Event> event);
};
}



#endif /* ASYNCEVENTPRODUCER_H_ */
