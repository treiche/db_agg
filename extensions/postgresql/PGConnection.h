/*
 * PGConnection.h
 *
 *  Created on: Jan 19, 2014
 *      Author: arnd
 */

#ifndef PGCONNECTION_H_
#define PGCONNECTION_H_

#include <libpq-fe.h>
#include <stddef.h>
#include <string>

namespace db_agg {
class PGConnection {
private:
    PGconn *_conn = nullptr;
public:
    void initialize(PGconn *_conn);
    void finish();
    int setnonblocking(int v);
    int isnonblocking();
    PostgresPollingStatusType connectPoll();
    std::string errorMessage();
    int sendQuery(std::string query);
    int flush();
    int consumeInput();
    PGresult *getResult();
    int isBusy();
    int putCopyData(std::string data);
    int putCopyEnd(std::string message);
    int getCopyData(char *&data,bool async);
    PGcancel *getCancel();
    PGconn * operator*();
    operator bool();
    bool connected();
    static PGConnection connectDb(std::string conninfo);
    static PGConnection connectStart(std::string conninfo);
    static bool ping(std::string conninfo);
};
}



#endif /* PGCONNECTION_H_ */
