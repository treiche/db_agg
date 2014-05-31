#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <string>

namespace db_agg {
class Connection {
    std::string host;
    int port = 5432;
    std::string environment;
    std::string database;
    int shardId = -1;
    std::string user;
    std::string password;
    std::string suffix;
    size_t statementTimeout = 0;
public:
    Connection() {}
    Connection(std::string host, int port, std::string environment, std::string database, int shardId, std::string suffix);
    std::string getUrl(bool includeOptions, bool maskPassword, bool includeCredentials);
    std::string getDatabase() {
        return database;
    }
    std::string getHost() {
        return host;
    }
    int getPort() {
        return port;
    }
    void setUser(std::string user) {
        this->user = user;
    }
    void setPassword(std::string password) {
        this->password = password;
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
