/*
 * Channel.h
 *
 *  Created on: Jun 2, 2014
 *      Author: arnd
 */

#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <string>
#include <memory>
#include "DataReceiver.h"

namespace db_agg {

enum class ChannelState {
    READY,
    OPEN,
    CLOSED
};

class Channel {
private:
    std::string name;
    DataReceiver *receiver;
    ChannelState state{ChannelState::READY};
public:
    Channel(std::string name, DataReceiver *receiver);
    void open();
    void send(std::shared_ptr<TableData> data);
    void close();
    ChannelState getState();
    std::string getName();
    friend class ExecutionGraph;
};
}



#endif /* CHANNEL_H_ */
