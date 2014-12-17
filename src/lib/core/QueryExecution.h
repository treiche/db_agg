/*
 * QueryExecution.h
 *
 *  Created on: Dec 29, 2013
 *      Author: arnd
 */

#ifndef QUERYEXECUTION_H_
#define QUERYEXECUTION_H_

#include <map>
#include <string>
#include <vector>
#include <deque>
#include <chrono>
#include <memory>

// #include "core/ExecutionHandler.h"
#include "table/TableData.h"
#include "event/Event.h"
#include "core/Url.h"
#include "injection/DependencyInjector.h"
#include "graph/DataSender.h"
#include "graph/Channel.h"

namespace db_agg {


enum class QueryExecutionState {
    INITIAL,
    COMPLETE,
    SCHEDULED,
    RUNNING,
    STOPPED,
    DONE
};

class QueryExecution: public DataReceiver, public DataSender, public EventProducer {
    private:
        std::map<std::string,std::shared_ptr<TableData>> results;
        std::string id;
        std::shared_ptr<Url> url;
        std::string sql;
        std::string name;
        QueryExecutionState state{QueryExecutionState::INITIAL};
        std::map<std::string,std::shared_ptr<TableData>> dependencies;
        std::map<std::string,std::string> portIds{{"",""}};
        std::shared_ptr<DependencyInjector> dependencyInjector;
        std::chrono::system_clock::time_point startTime;
        std::chrono::system_clock::time_point endTime;
        std::vector<Channel*> channels;
        std::vector<std::string> arguments;
    protected:
        std::vector<std::string>& getArguments();
        std::map<std::string,std::shared_ptr<TableData>>& getDependencies();
        std::shared_ptr<DependencyInjector> getInjector();
        void setPortNames(std::vector<std::string> portNames);
    public:
        QueryExecution();
        void init(std::string name,
                  std::string id,
                  std::shared_ptr<Url> url,
                  std::string sql,
                  std::vector<std::string> depName,
                  std::shared_ptr<DependencyInjector> dependencyInjector,
                  std::vector<std::string> arguments);
        virtual ~QueryExecution() override;
        std::string getId();
        void setId(std::string id);
        std::string getName();
        void setState(QueryExecutionState state);
        QueryExecutionState getState();
        void release();
        size_t getDuration();
        std::shared_ptr<Url> getUrl();
        std::string getSql();
        std::shared_ptr<TableData> getResult(std::string shardId);
        void setResult(std::string shardId, std::shared_ptr<TableData> data);
        std::string inject(std::string query, size_t copyThreshold);
        std::vector<Channel*> getChannels();

        std::vector<std::string> getPortNames();
        void setPortId(std::string portName, std::string portId);
        std::string getPortId(std::string portName);

        virtual bool isTransition();
        virtual void stop();
        virtual void schedule();
        virtual bool process() = 0;
        virtual void cleanUp();
        virtual bool isResourceAvailable();
        // Interfaces:
        // DataReceiver
        virtual void receive(std::string name, std::shared_ptr<TableData> data) override;
        // DataSender
        virtual void addChannel(Channel* channel) override;

        friend std::ostream& operator<<(std::ostream& cout,const QueryExecution& t);
};

std::ostream& operator<<(std::ostream& cout,const QueryExecution& t);

std::ostream& operator<<(std::ostream& cout,const QueryExecutionState t);

}

#endif /* QUERYEXECUTION_H_ */
