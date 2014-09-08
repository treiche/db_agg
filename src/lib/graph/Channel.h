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
#include "DataSender.h"

namespace db_agg {

enum class ChannelState {
    READY,
    OPEN,
    CLOSED
};

class Channel {
private:
    std::string targetPort;
    DataReceiver *target;
    std::string sourcePort{""};
    DataSender *source{nullptr};
    ChannelState state{ChannelState::READY};
public:
    //Channel(std::string targetPort, DataReceiver *target);
    Channel(DataSender *source, std::string sourcePort, DataReceiver *target, std::string targetPort);
    void open();
    void send(std::shared_ptr<TableData> data);
    void close();
    ChannelState getState();
    std::string getTargetPort();
    friend class ExecutionGraph;
    friend class ExecutionGraph2;
};
}



#endif /* CHANNEL_H_ */
