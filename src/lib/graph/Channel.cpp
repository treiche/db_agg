/*
 * Channel.cpp
 *
 *  Created on: Jun 2, 2014
 *      Author: arnd
 */

#include "Channel.h"

#include "utils/logging.h"


using namespace std;
using namespace log4cplus;

namespace db_agg {

static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("Channel"));


// Channel::Channel(string targetPort, DataReceiver *target): targetPort(targetPort), target(target) {}

Channel::Channel(DataSender *source, std::string sourcePort, DataReceiver *target, std::string targetPort):
	source(source),
	sourcePort(sourcePort),
	target(target),
	targetPort(targetPort) {

}

void Channel::open() {
    if (state != ChannelState::READY) {
        THROW_EXC("channel '" << targetPort << "' is not ready. [state=" << to_string((int)state) << "]");
    }
    state = ChannelState::OPEN;
}

void Channel::send(std::shared_ptr<TableData> data) {
    if (state != ChannelState::OPEN) {
        THROW_EXC("channel is not open.");
    }
    target->receive(targetPort, data);
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

string Channel::getTargetPort() {
    return targetPort;
}

string Channel::getSourcePort() {
    return sourcePort;
}

QueryExecution* Channel::getTarget() {
	return (QueryExecution*)(target);
}


}
