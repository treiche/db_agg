/*
 * Data.h
 *
 *  Created on: Jun 20, 2014
 *      Author: arnd
 */

#ifndef DATA_H_
#define DATA_H_

#include <vector>
#include <string>

namespace db_agg {
class Data {
private:
    struct Chunk {
        const char *ptr;
        uint64_t size;
    };
    bool owned = false;
    std::vector<Chunk> chunks;
    Data(const char *ptr, uint64_t size, bool owned);
public:
    Data();
    Data(Data& src);
    ~Data();
    void addChunk(const char *ptr, uint64_t size);
    uint64_t size();
    void range(uint64_t offset, uint64_t size, Data& data);
    std::string str();
    const char *getPtr();
};
}



#endif /* DATA_H_ */
