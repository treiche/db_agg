#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <string>

namespace db_agg {
class Connection {
    std::string host;
    int port;
    std::string environment;
    std::string database;
    int shardId;
    std::string suffix;
    size_t statementTimeout = 0;
public:
    Connection(std::string host, int port, std::string environment, std::string database, int shardId, std::string suffix);
    std::string getUrl();
    std::string getDatabase() {
        return database;
    }
    std::string getHost() {
        return host;
    }
    int getPort() {
        return port;
    }
    size_t getStatementTimeout() {
        return statementTimeout;
    }
    void setStatementTimeout(size_t statementTimeout) {
    	this->statementTimeout = statementTimeout;
    }
};
}


#endif /* CONNECTION_H_ */
