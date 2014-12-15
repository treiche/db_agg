/*
 * Memcached.h
 *
 *  Created on: Jun 18, 2014
 *      Author: arnd
 */

#ifndef MEMCACHED_H_
#define MEMCACHED_H_

#include <vector>
#include <string>

namespace db_agg {
class Memcached {
private:
    struct XImpl;
    XImpl *pImpl;
public:
    Memcached(std::vector<std::string> servers);
    ~Memcached();
    std::vector<std::string> getMulti(std::vector<std::string> keys);
};
}



#endif /* MEMCACHED_H_ */
