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

#include "core/ExecutionHandler.h"
#include "table/TableData.h"
#include "core/Transition.h"
#include "event/Event.h"
#include "core/Url.h"
#include "injection/DependencyInjector.h"
#include "graph/DataSender.h"
#include "graph/Channel.h"

namespace db_agg {
class Transition;

class QueryExecution: public DataReceiver, public DataSender, public EventProducer {
    private:
        std::shared_ptr<TableData> data;
        std::string id;
        std::shared_ptr<Url> url;
        std::string sql;
        std::string name;
        bool scheduled = false;
        bool done = false;
        std::map<std::string,std::shared_ptr<TableData>> dependencies;
        std::shared_ptr<DependencyInjector> dependencyInjector;
        std::chrono::system_clock::time_point startTime;
        std::chrono::system_clock::time_point endTime;
        std::vector<Channel*> channels;
        std::vector<std::string> arguments;
    protected:
        void setData(std::shared_ptr<TableData> data);
        std::vector<std::string> getArguments();
        std::map<std::string,std::shared_ptr<TableData>>& getDependencies();
        std::shared_ptr<DependencyInjector> getInjector();
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
        std::shared_ptr<TableData> getData();
        void setScheduled();
        bool isScheduled();
        void setDone();
        bool isDone();
        void release();
        size_t getDuration();
        std::shared_ptr<Url> getUrl();
        std::string getSql();
        std::shared_ptr<TableData> getResult();
        void setResult(std::shared_ptr<TableData> data);
        std::string inject(std::string query, size_t copyThreshold);
        bool isComplete();
        std::vector<Channel*> getChannels();

        virtual void stop() = 0;
        virtual void schedule() = 0;
        virtual bool process() = 0;
        virtual void cleanUp() = 0;
        virtual bool isResourceAvailable() = 0;
        // Interfaces:
        // DataReceiver
        virtual void receive(std::string name, std::shared_ptr<TableData> data) override;
        // DataSender
        virtual void addChannel(Channel* channel) override;

        friend std::ostream& operator<<(std::ostream& cout,const QueryExecution& t);
};

std::ostream& operator<<(std::ostream& cout,const QueryExecution& t);

}

#endif /* QUERYEXECUTION_H_ */
