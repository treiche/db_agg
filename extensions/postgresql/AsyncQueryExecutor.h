#ifndef ASYNCQUERYEXECUTOR_H_
#define ASYNCQUERYEXECUTOR_H_

#include <stdexcept>
#include <string>

#include "ExecutionHandler.h"
#include "event/Event.h"

namespace db_agg {

class AsyncQueryExecutor: public EventProducer {
private:
    struct XImpl;
    struct QueryTask;
    XImpl* pImpl;
    bool processTask(int taskNo);
    void fireEvent(std::shared_ptr<Event> event, int taskNo);
    void fireEvent(EventType type, int taskNo);
    void fireStateChangeEvent(int taskNo, std::string state);
    void cleanUp(int taskNo, std::string reason);
    bool loop();
public:
    AsyncQueryExecutor();
    ~AsyncQueryExecutor();
    void addQuery(std::string id, std::string connectionUrl, std::string query, ExecutionHandler *handler);
    bool process();
    void stop();
    void cleanUp(std::string reason);
};

class AsyncQueryExecutorException : public std::runtime_error {
private:
    std::string query;
public:
    AsyncQueryExecutorException(std::string what, std::string query);
    std::string getQuery() {
        return query;
    }
};

}

#endif /* ASYNCQUERYEXECUTOR_H_ */
