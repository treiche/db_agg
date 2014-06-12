/*
 * Url.h
 *
 *  Created on: Jun 10, 2014
 *      Author: arnd
 */

#ifndef URL_H_
#define URL_H_

#include <string>
#include <map>

namespace db_agg {
class Url {
private:
    std::string protocol;
    std::string host;
    int port = 5432;
    std::string path;
    std::string user;
    std::string password;
    std::map<std::string,std::string> parameters;
    std::string fragment;
public:
    Url(std::string url);
    Url(std::string protocol, std::string host, int port, std::string path, std::string fragment = "");
    std::string getUrl();
    std::string getUrl(bool includeParameter, bool includeCredentials, bool maskPassword);
    std::string getProtocol();
    std::string getHost();
    int getPort();
    std::string getPath();
    std::string getUser();
    void setUser(std::string user);
    std::string getPassword();
    void setPassword(std::string password);
    bool hasParameter(std::string name);
    std::string getParameter(std::string name);
    void setParameter(std::string name, std::string value);
    std::string getFragment();
    std::string getExtension();
};
}



#endif /* URL_H_ */
