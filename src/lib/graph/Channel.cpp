/*
 * Channel.cpp
 *
 *  Created on: Jun 2, 2014
 *      Author: arnd
 */

#include "Channel.h"

#include <log4cplus/logger.h>


using namespace std;
using namespace log4cplus;

namespace db_agg {

static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("Channel"));


Channel::Channel(string name, DataReceiver *receiver): name(name), receiver(receiver) {}

void Channel::open() {
    if (state != ChannelState::READY) {
        string msg = "channel '" + name + "' is not ready. [state=" + to_string((int)state) + "]";
        LOG4CPLUS_ERROR(LOG,msg);
        throw runtime_error(msg);
    }
    state = ChannelState::OPEN;
}

void Channel::send(std::shared_ptr<TableData> data) {
    if (state != ChannelState::OPEN) {
        throw runtime_error("channel is not open.");
    }
    receiver->receive(name, data);
}

void Channel::close() {
    if (state != ChannelState::OPEN) {
        throw runtime_error("channel is not open.");
    }
    state = ChannelState::CLOSED;
}

ChannelState Channel::getState() {
    return state;
}

string Channel::getName() {
    return name;
}

}
