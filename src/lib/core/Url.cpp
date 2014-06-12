/*
 * Url.cpp
 *
 *  Created on: Jun 10, 2014
 *      Author: arnd
 */

#include <iostream>
#include "Url.h"
#include "utils/RegExp.h"

using namespace std;

namespace db_agg {

Url::Url(string url) {
    RegExp re("([a-z]+)?://([a-z\\.]+)?(:([0-9]+))?/([a-zA-Z0-9-\\./_]+)?(#([a-zA-Z_]+))?");
    vector<RegExp::match> matches = re.exec(url);
    if (matches.empty()) {
        throw runtime_error("invalid url");
    }
    this->protocol = matches[1].substr.empty() ? "file" : matches[1].substr;
    this->host = matches[2].substr;
    this->port = matches[4].substr.empty() ? 0 : stoi(matches[4].substr);
    this->path = matches[5].substr;
    if (matches.size() == 8) {
        this->fragment = matches[7].substr;
    }
}

Url::Url(string protocol, string host, int port, string path, string fragment):
        protocol(protocol),
        host(host),
        port(port),
        path(path),
        fragment(fragment)
    { }

string Url::getProtocol() {
    return protocol;
}

string Url::getHost() {
    return host;
}

int Url::getPort() {
    return port;
}

string Url::getPath() {
    return path;
}

string Url::getUser() {
    return user;
}

void Url::setUser(std::string user) {
    this->user = user;
}

string Url::getPassword() {
    return password;
}

void Url::setPassword(std::string password) {
    this->password = password;
}

bool Url::hasParameter(std::string name) {
    return parameters.find(name) != parameters.end();
}

string Url::getParameter(string name) {
    return parameters[name];
}

void Url::setParameter(string name, string value) {
    parameters[name] = value;
}

string Url::getFragment() {
    return fragment;
}

string Url::getExtension() {
    size_t idx = path.find_last_of(".");
    return path.substr(idx+1);
}

string Url::getUrl() {
    return getUrl(true,true,true);
}

string Url::getUrl(bool includeParameters, bool includeCredentials, bool maskPassword) {
    string url = protocol + "://" + host + "/" + path;
    if (includeParameters && !parameters.empty()) {
        url += "?";
        int len = parameters.size();
        int idx = 0;
        for (auto& parameter:parameters) {
            url += parameter.first + "=" + parameter.second;
            idx++;
            if (idx!=len || includeCredentials) {
                url += "&";
            }
        }
        if (includeCredentials) {
            url += "user=" + user + "&password=";
            if (maskPassword) {
                url += "xxxxxxxx";
            } else {
                url += password;
            }
        }
    }
    if (!fragment.empty()) {
        url += "#" + fragment;
    }
    return url;
}

}


