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
#include <vector>
#include <memory>

namespace db_agg {
class Url {
private:
    std::string protocol;
    std::string host;
    std::string port;
    std::vector<std::string> path;
    std::string user;
    std::string password;
    std::map<std::string,std::string> parameters;
    std::string fragment;
    std::string encode(std::string);
    std::string decode(std::string);
public:
    Url();
    Url(std::string url);
    Url(std::string protocol, std::string host, std::string port, std::string path, std::string fragment = "");
    std::string getUrl();
    std::string getUrl(bool includeParameter, bool includeCredentials, bool maskPassword);
    std::string getProtocol();
    void setProtocol(std::string protocol);
    std::string getHost();
    void setHost(std::string host);
    std::string getPort();
    void setPort(std::string port);
    std::string getPath();
    void setPath(std::string path);
    void addPathItem(std::string pathItem);
    std::string getUser();
    void setUser(std::string user);
    std::string getPassword();
    void setPassword(std::string password);
    bool hasParameter(std::string name);
    std::string getParameter(std::string name);
    void setParameter(std::string name, std::string value);
    std::string getFragment();
    std::string getExtension();
    bool matches(std::shared_ptr<Url> wildcardUrl);
    static std::shared_ptr<Url> getWildcardUrl();
};
}



#endif /* URL_H_ */
