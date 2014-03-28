#include "core/Connection.h"

#include <string>

using namespace std;

namespace db_agg {
    Connection::Connection(string host, int port, string environment, string database, int shardId, string suffix) {
        this->host = host;
        this->port = port;
        this->environment = environment;
        this->database = database;
        this->shardId = shardId;
        this->suffix = suffix;
    }

    string Connection::getUrl() {
        return "host=" + host + " port=" + to_string(port) + " dbname=" + database + " options='--statement-timeout=" + to_string(statementTimeout)+"'";
    }

}
