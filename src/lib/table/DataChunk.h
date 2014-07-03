/*
 * DataChunk.h
 *
 *  Created on: Jul 3, 2014
 *      Author: arnd
 */

#ifndef DATACHUNK_H_
#define DATACHUNK_H_

#include <cstdint>
#include <string>
#include <vector>

namespace db_agg {
class DataChunk {
private:
    const char *ptr;
    uint64_t size;
public:
    DataChunk(char *ptr, uint64_t size);
    const char *getPtr();
    uint64_t getSize();
    static std::string contiguous(std::vector<DataChunk> chunks);
};

}



#endif /* DATACHUNK_H_ */
