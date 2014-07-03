/*
 * DataChunk.cpp
 *
 *  Created on: Jul 3, 2014
 *      Author: arnd
 */

#include "DataChunk.h"

using namespace std;

namespace db_agg {
DataChunk::DataChunk(char *ptr, uint64_t size):
        ptr(ptr),
        size(size) {}

const char *DataChunk::getPtr() {
    return ptr;
}

uint64_t DataChunk::getSize() {
    return size;
}

string DataChunk::contiguous(std::vector<DataChunk> chunks) {
    string result;
    for (auto& chunk:chunks) {
        result.append(chunk.getPtr(),chunk.getSize());
    }
    return result;
}

}



