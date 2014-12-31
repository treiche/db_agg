/*
 * PostgresqlExecution.cpp
 *
 *  Created on: Dec 29, 2014
 *      Author: arnd
 */

#include "PostgresqlExecution.h"
#include "utils/logging.h"

using namespace std;


namespace db_agg {

DECLARE_LOGGER("PostgresqlExecution")

bool PostgresqlExecution::process() {
    //LOG_ERROR("process connectionState " << connectionState << " processingState " << processingState);
    switch(connectionState) {
        case ConnectionState::INITIAL:
            connect();
            break;
        case ConnectionState::CONNECTED:
            sendQuery();
            break;
        case ConnectionState::QUERY_SENDING:
            flush();
            break;
        case ConnectionState::QUERY_SENT:
            communicate();
            return true;
            break;
    }
    return false;
}

void PostgresqlExecution::connect() {
    // LOG_ERROR("connect")
    connection = PQconnectdb(toPostgresUrl(getUrl()).c_str());
    if (connection) {
        connectionState = ConnectionState::CONNECTED;
    } else {
        THROW_EXC("connecting to '" << getUrl()->getUrl(false,false,false) << "' failed")
    }
}

void PostgresqlExecution::sendQuery() {
    LOG_ERROR("sendQuery")
    if (!PQsendQuery(connection, getSql().c_str())) {
        THROW_EXC("sending query failed: " << PQerrorMessage(connection));
    }
    connectionState = ConnectionState::QUERY_SENDING;
}

void PostgresqlExecution::flush() {
    LOG_ERROR("flush")
    int ret = PQflush(connection);
    if (ret==1) {
        connectionState = ConnectionState::QUERY_SENDING;
    } else {
        connectionState = ConnectionState::QUERY_SENT;
    }
}

void PostgresqlExecution::communicate() {
    LOG_ERROR("communicate")
    int cir = PQconsumeInput(connection);
    if (!cir) {
        THROW_EXC("consume input " << getSql() << " failed. message = " << PQerrorMessage(connection));
    }
    bool busy = PQisBusy(connection);
    if (busy) {
        processingState = ProcessingState::BUSY;
        return;
    }
    if (result == nullptr) {
        result = PQgetResult(connection);
    }
}


string PostgresqlExecution::toPostgresUrl(shared_ptr<Url> url) {
    string pgurl = "host=" + url->getHost() + " port=" + url->getPort() + " dbname=" + url->getPath().substr(1);
    pgurl += " user=" + url->getUser() + " password=" + url->getPassword();
    vector<string> options;
    if (url->hasParameter("statementTimeout")) {
        options.push_back("statementTimeout");
    }
    if (url->hasParameter("search_path")) {
        options.push_back("search_path");
    }
    /*
    if (!options.empty()) {
        pgurl += " options='";
        for (size_t idx = 0; idx < options.size(); idx++) {

        }
        pgurl += "'";
    }
    */
    if (url->hasParameter("statementTimeout") || url->hasParameter("search_path")) {
        pgurl += " options='";
        if (url->hasParameter("statementTimeout")) {
            pgurl += "--statement-timeout=" + url->getParameter("statementTimeout");
        }
        if (url->hasParameter("search_path")) {
            pgurl += " --search-path=" + url->getParameter("search_path");
        }
        pgurl += "'";
        // --pgurl += " options='--statement-timeout=" + url->getParameter("statementTimeout")+"'";
    }
    return pgurl;
}

ostream& operator<<(ostream& cout,const ConnectionState state) {
    switch(state) {
        case ConnectionState::INITIAL:
            cout << "INITIAL";
            break;
        case ConnectionState::CONNECTED:
            cout << "CONNECTED";
            break;
        case ConnectionState::QUERY_SENDING:
            cout << "QUERY_SENDING";
            break;
        case ConnectionState::QUERY_SENT:
            cout << "QUERY_SENT";
            break;
        case ConnectionState::FAILED:
            cout << "FAILED";
            break;
    }
    return cout;
}



}

