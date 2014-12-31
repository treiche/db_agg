/*
 * PostgresqlExecution.h
 *
 *  Created on: Dec 29, 2014
 *      Author: arnd
 */

#ifndef POSTGRESQLEXECUTION_H_
#define POSTGRESQLEXECUTION_H_

#include "core/QueryExecution.h"

extern "C" {
    #include <libpq-fe.h>
}

namespace db_agg {

enum class ConnectionState {
    INITIAL,
    CONNECTED,
    QUERY_SENDING,
    QUERY_SENT,
    FAILED
};

enum class ProcessingState {
    INITIAL,
    BUSY
};

std::ostream& operator<<(std::ostream& cout,const ConnectionState state);


class PostgresqlExecution: public QueryExecution {
private:
    PGconn *connection{nullptr};
    PGresult *result{nullptr};
    ConnectionState connectionState{ConnectionState::INITIAL};
    ProcessingState processingState{ProcessingState::INITIAL};
    std::string toPostgresUrl(std::shared_ptr<Url> url);
    void connect();
    void sendQuery();
    void flush();
    void communicate();
public:
    virtual bool process() override;
};
}



#endif /* POSTGRESQLEXECUTION_H_ */
