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

class QueryExecution;

enum class ChannelState {
    READY,
    OPEN,
    CLOSED
};

class Channel {
private:
    QueryExecution *source{nullptr};
    std::string sourcePort{""};
    QueryExecution *target;
    std::string targetPort;
    ChannelState state{ChannelState::READY};
public:
    Channel(QueryExecution *source, std::string sourcePort, QueryExecution *target, std::string targetPort);
    void open();
    void send(std::shared_ptr<TableData> data);
    void close();
    ChannelState getState();
    QueryExecution* getTarget();
    QueryExecution* getSource();
    std::string getTargetPort();
    std::string getSourcePort();
    friend class ExecutionGraph;
};
}



#endif /* CHANNEL_H_ */
