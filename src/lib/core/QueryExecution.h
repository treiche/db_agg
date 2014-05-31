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
#include "Connection.h"
#include "injection/DependencyInjector.h"

namespace db_agg {
class Transition;

class QueryExecution: public ExecutionHandler {
    private:
        std::shared_ptr<TableData> data;
        std::string id;
        Connection connectionUrl;
        std::string sql;
        std::string name;
        std::vector<Transition*> transitions;
        std::vector<Transition*> incomingTransitions;
        bool scheduled = false;
        bool done = false;
        std::map<std::string,std::shared_ptr<TableData>> dependencies;
        DependencyInjector *dependencyInjector = nullptr;
        std::chrono::system_clock::time_point startTime;
        std::chrono::system_clock::time_point endTime;
        public:
        QueryExecution();
        QueryExecution(std::string name, std::string id, Connection connectionUrl, std::string sql, std::vector<std::string> depName, DependencyInjector *dependencyInjector);
        virtual ~QueryExecution() override;

        bool allTransitionsDone();

        std::string getId() { return id; }
        void setId(std::string id) {
            this->id = id;
        }
        std::string getName() { return name; }
        std::shared_ptr<TableData> getData() { return data; }
        void setScheduled() {
            scheduled = true;
            startTime = std::chrono::system_clock::now();
        }
        bool isScheduled() { return scheduled; }
        void setDone() {
            endTime = std::chrono::system_clock::now();
            done = true;
        }
        void release();
        size_t getDuration() {
            std::chrono::system_clock::duration duration = endTime - startTime;
            return duration.count();
        }
        bool isDone() { return done; }
        Connection getConnectionUrl() { return connectionUrl; }
        std::string getSql() { return sql; }

        void addTransition(Transition *t);
        void addIncomingTransition(Transition *t);

        virtual std::string handleCopyIn(size_t step, uint64_t startRows, uint64_t rows, uint64_t& rowsRead) override;
        virtual uint64_t getRowCount(size_t step) override;
        virtual void handleCopyOut(size_t step, std::string data) override;
        virtual void handleTuples(size_t step, std::vector<std::pair<std::string, uint32_t>>& columns) override;

        virtual std::shared_ptr<TableData> getResult() override;
        virtual void setResult(std::shared_ptr<TableData> data) override;
        virtual void addDependency(std::string name, std::shared_ptr<TableData> data) override;
        virtual std::string inject(std::string query, size_t copyThreshold) override;
        virtual bool isComplete() override;

        std::vector<Transition*>& getIncomingTransitions() {
            return incomingTransitions;
        }
        std::vector<Transition*>& getTransitions() {
            return transitions;
        }
        void doTransitions();
        friend std::ostream& operator<<(std::ostream& cout,const QueryExecution& t);
};

std::ostream& operator<<(std::ostream& cout,const QueryExecution& t);

}

#endif /* QUERYEXECUTION_H_ */
