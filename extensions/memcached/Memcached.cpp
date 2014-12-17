/*
 * Memcached.cpp
 *
 *  Created on: Jun 18, 2014
 *      Author: arnd
 */

#include "Memcached.h"
#include "utils/logging.h"
#include <log4cplus/logger.h>

extern "C" {

#include "libmemcached/memcached.h"

}

using namespace std;
using namespace log4cplus;

namespace db_agg {

DECLARE_LOGGER("Memcached");

struct Memcached::XImpl {
    memcached_st *memc;
};

Memcached::Memcached(vector<string> servers) {
    pImpl = new XImpl();
    string config_string;
    for (size_t idx = 0; idx < servers.size(); idx++) {
        config_string += "--SERVER=" + servers[idx];
        if (idx < servers.size() -1) {
            config_string += " ";
        }
    }
    pImpl->memc = memcached(config_string.c_str(), config_string.size());
    if (pImpl->memc == nullptr) {
        LOG_ERROR("memc = " << pImpl->memc);
        THROW_EXC("error: " << memcached_last_error_message(pImpl->memc));
    }
}

Memcached::~Memcached() {
    memcached_free(pImpl->memc);
    delete pImpl;
}

vector<string> Memcached::getMulti(vector<string> keys) {
    const char **keyArray = new const char*[keys.size()];
    size_t *key_length = new size_t[keys.size()];
    for (size_t idx = 0; idx < keys.size(); idx++) {
        keyArray[idx] = keys[idx].c_str();
        key_length[idx] = keys[idx].size();
    }

    memcached_return_t rc = memcached_mget(pImpl->memc, keyArray, key_length, keys.size());

    if (rc != MEMCACHED_SUCCESS) {
        THROW_EXC("error: " << memcached_last_error_message(pImpl->memc));
    }

    char *return_value;
    char return_key[MEMCACHED_MAX_KEY];
    size_t return_key_length;
    size_t return_value_length;
    uint32_t flags = 0;

    map<string,string> resultMap;

    while ((return_value = memcached_fetch(pImpl->memc, return_key, &return_key_length, &return_value_length, &flags, &rc))) {
        LOG_TRACE("return_key = " << string(return_key, return_key_length));
        string key(return_key, return_key_length);
        string value(return_value, return_value_length);
        resultMap[key] = value;
        free(return_value);
    }

    delete [] keyArray;
    delete [] key_length;

    vector<string> result;
    for (auto key:keys) {
        if (resultMap.find(key) == resultMap.end()) {
            result.push_back("");
        } else {
            result.push_back(resultMap[key]);
        }

    }

    return result;
}

}


int main(int args, char **argv) {
    db_agg::Memcached mc{{"localhost:11211","localhost:11211"}};
    vector<string> results = mc.getMulti({"key","2NA21J000-O00000M000:1"});
    cout << "returned " << results.size() << " results" << endl;
    for (string result:results) {
        cout << "result = " << result << endl;
    }
}
