/*
 * Url.cpp
 *
 *  Created on: Jun 10, 2014
 *      Author: arnd
 */

#include <iostream>
#include <sstream>
#include "Url.h"
#include "utils/string.h"
#include "utils/RegExp.h"
#include "utils/utility.h"
#include "utils/logging.h"

using namespace std;


namespace db_agg {

DECLARE_LOGGER("Url")

Url::Url() {}

Url::~Url() {
    if (uri) {
        xmlFreeURI(uri);
    }
}

Url::Url(string url) {
    if (true) {
        uri = xmlParseURIRaw(url.c_str(),0);
        if (!uri) {
            THROW_EXC("invalid url: '" << url << "'");
        }
        if (uri->scheme) {
            this->protocol = uri->scheme;
        }
        if (uri->server) {
            this->host = uri->server;
        }
        if (uri->port) {
            this->port = to_string(uri->port);
        }
        if (uri->path) {
            split(uri->path, '/', this->path);
        }
        if (uri->fragment) {
            this->fragment = uri->fragment;
        }
        if (uri->query) {
            vector<string> items;
            split(uri->query, '&', items);
            for (auto item:items) {
                vector<string> kv;
                split(item, '=', kv);
                if (kv.size()==1) {
                    parameters[kv[0]] = "";
                } else if (kv.size()==2) {
                    parameters[kv[0]] = kv[1];
                } else {
                    THROW_EXC("invalid parameter '" << item << "'");
                }
            }
        }
    } else {
        RegExp re("([a-z]+)?://([a-z\\.]+)?(:([0-9]+))?/([a-zA-Z0-9-\\./_]+)?(#([a-zA-Z_0-9]+))?");
        vector<RegExp::match> matches = re.exec(url);
        if (matches.empty()) {
            throw runtime_error("invalid url");
        }
        this->protocol = matches[1].substr.empty() ? "file" : matches[1].substr;
        this->host = matches[2].substr;
        this->port = matches[4].substr;;
        string pathItems = matches[5].substr;
        split(pathItems, '/', this->path);
        if (matches.size() == 8) {
            this->fragment = matches[7].substr;
        }
    }
}

Url::Url(string protocol, string host, string port, string path, string fragment):
        protocol(protocol),
        host(host),
        port(port),
        fragment(fragment)
    {
        split(path, '/', this->path);

    }

string Url::getProtocol() {
    return protocol;
}

void Url::setProtocol(string protocol) {
    this->protocol = protocol;
}

string Url::getHost() {
    return host;
}

void Url::setHost(string host) {
    this->host = host;
}

string Url::getPort() {
    return port;
}

void Url::setPort(string port) {
    this->port = port;
}

string Url::getPath() {
    return "/" + join(path,"/");
}

void Url::setPath(string path) {
    this->path.clear();
    split(path, '/', this->path);
}


void Url::addPathItem(string pathItem) {
    this->path.push_back(pathItem);
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
    string lastItem = path.back();
    size_t idx = lastItem.find_last_of(".");
    return lastItem.substr(idx+1);
}

string Url::getUrl() {
    return getUrl(true,true,true);
}

string Url::getUrl(bool includeParameters, bool includeCredentials, bool maskPassword) {
    string url = protocol + "://" + host;
    if (!port.empty()) {
        url += ":" + port;
    }
    url +=  join(path,"/");
    if (includeParameters && !parameters.empty()) {
        url += "?";
        int len = parameters.size();
        int idx = 0;
        for (auto& parameter:parameters) {
            url += encode(parameter.first) + "=" + encode(parameter.second);
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

bool Url::matches(std::shared_ptr<Url> wildcardUrl) {
    if (wildcardUrl->protocol != "*" && wildcardUrl->protocol != protocol) {
        return false;
    }
    if (wildcardUrl->host != "*" && wildcardUrl->host != host) {
        return false;
    }
    if (wildcardUrl->port != "*" && wildcardUrl->port != port) {
        return false;
    }

    size_t len = this->path.size();
    if (wildcardUrl->path.size() < len) {
        len = wildcardUrl->path.size();
    }

    for (size_t idx = 0; idx < len; idx++) {
        if (wildcardUrl->path[idx] != "*" && wildcardUrl->path[idx] != this->path[idx]) {
            return false;
        }
    }
    return true;
}

string Url::encode(string data) {
    stringstream encoded;
    string reserved = "!#$%&'()*+,/:;=?@[]";
    for (size_t idx = 0; idx < data.size(); idx++) {
        char c = data[idx];
        if (reserved.find(c) != string::npos) {
            encoded << "%" << std::hex << (int)c;
        } else {
            encoded << c;
        }

    }
    return encoded.str();
}

string Url::decode(std::string data) {
    stringstream decoded;
    for (size_t idx = 0; idx < data.size(); idx++) {
        char c = data[idx];
        if (c=='%') {
        }
    }
    return decoded.str();
}


shared_ptr<Url> Url::getWildcardUrl() {
    return shared_ptr<Url>(new Url("*","*","*","*","*"));
}

}


