/*
 * PGConnection.cpp
 *
 *  Created on: Jan 19, 2014
 *      Author: arnd
 */


#include "PGConnection.h"
#include <log4cplus/logger.h>
#include "utils/utility.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("PGConnection"));


void PGConnection::initialize(PGconn *_conn) {
    this->_conn = _conn;
}
void PGConnection::finish() {
    PQfinish(_conn);
    _conn = nullptr;
}
int PGConnection::setnonblocking(int v) {
    return PQsetnonblocking(_conn,v);
}
int PGConnection::isnonblocking() {
    return PQisnonblocking(_conn);
}
PostgresPollingStatusType PGConnection::connectPoll() {
    return PQconnectPoll(_conn);
}
std::string PGConnection::errorMessage() {
    return PQerrorMessage(_conn);
}
int PGConnection::sendQuery(std::string query) {
    return PQsendQuery(_conn, query.c_str());
}
int PGConnection::flush() {
    return PQflush(_conn);
}
int PGConnection::consumeInput() {
    return PQconsumeInput(_conn);
}
PGresult *PGConnection::getResult() {
    return PQgetResult(_conn);
}
int PGConnection::isBusy() {
    return PQisBusy(_conn);
}
int PGConnection::putCopyData(std::string data) {
    return PQputCopyData(_conn, data.c_str(), data.size());
}
int PGConnection::putCopyEnd(std::string message) {
    const char *msg = message.c_str();
    if (message.empty()) {
        msg=NULL;
    }
    return PQputCopyEnd(_conn,msg);
}
int PGConnection::getCopyData(char *&data,bool async) {
    return PQgetCopyData(_conn,&data,async);
}
PGcancel *PGConnection::getCancel() {
    return PQgetCancel(_conn);
}
PGconn * PGConnection::operator*() {
    return _conn;
}
bool PGConnection::connected() {
   return _conn != nullptr;
}

PGConnection::operator bool() {
    return _conn != nullptr;
}
PGConnection PGConnection::connectDb(std::string conninfo) {
    PGConnection conn;
    conn.initialize(PQconnectdb(conninfo.c_str()));
    return conn;
}
PGConnection PGConnection::connectStart(std::string conninfo) {
    PGConnection conn;
    conn.initialize(PQconnectStart(conninfo.c_str()));
    return conn;
}

void PGConnection::ping(std::string conninfo) {
    PGPing ping = PQping(conninfo.c_str());
    if (ping != PQPING_OK) {
        throw runtime_error("ping " + maskPassword(conninfo) + " failed.");
    }
}

}

