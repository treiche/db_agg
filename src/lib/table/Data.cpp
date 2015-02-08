/*
 * Data.cpp
 *
 *  Created on: Jun 20, 2014
 *      Author: arnd
 */

#include "Data.h"
#include <iostream>
#include <cassert>
#include <cstring>

using namespace std;

namespace db_agg {

Data::Data() {}


Data::Data(Data& src) {
    cout << "called copy constructor" << endl;
    this->chunks = src.chunks;
    this->owned = src.owned;
    src.owned = false;
}

Data::Data(const char *ptr, uint64_t size, bool owned) {
    chunks.push_back(Chunk{ptr,size});
    this->owned = owned;
}

Data::~Data() {
    if (this->owned) {
        cout << "delete ptr " << (void*)chunks[0].ptr << endl;
        // delete chunks[0].ptr;
    }
}

void Data::addChunk(const char *ptr, uint64_t size) {
    chunks.push_back(Chunk{ptr,size});
}

const char * Data::getPtr() {
    assert(chunks.size() == 1);
    return chunks[0].ptr;
}

uint64_t Data::size() {
    uint64_t sum = 0;
    for (auto& chunk:chunks) {
        sum += chunk.size;
    }
    return sum;
}

std::string Data::str() {
    //cout << "called str with " << chunks.size() << " chunks" << endl;
    if (chunks.size() == 1) {
        //cout << "str() returns "  << (uint64_t)chunks[0].ptr << endl;
        return string(chunks[0].ptr,chunks[0].size);
    }
    Data cont;
    this->range(0,size(),cont);
    return cont.str();
}

void Data::range(uint64_t offset, uint64_t len, Data& data) {
    uint64_t sum = size();
    assert(offset + len <= sum);
    if (chunks.size() == 1) {
        data.addChunk(chunks[0].ptr, chunks[0].size);
        data.owned = false;
        return;
    }
    // check if range is within a single chunk and return ref data
    // detect startChunk and relOffset
    uint64_t ofs = 0;
    uint64_t relOfs = offset;
    size_t startChunk = 0;
    uint64_t startOffset = 0;
    size_t endChunk = 0;
    uint64_t endOffset = 0;
    cout << "chunk size " << chunks.size() << " this len = " << this->size() << endl;
    for (size_t idx = 0; idx < chunks.size(); idx++) {
        Chunk& chunk = chunks[idx];
        if (offset >= ofs && (offset + len) <= (ofs+chunk.size)) {
            //cout << "within a single chunk" << endl;
            //Data single(chunk.ptr + relOfs, len, false);
            data.chunks.push_back(Chunk{chunk.ptr + relOfs, len});
            data.owned = false;
            //cout << "create chunk on " << (uint64_t)chunk.ptr << " len=" << len << endl;
            return;
        } else if (offset >= ofs) {
            //cout << "chunk start" << endl;
            startChunk = idx;
            startOffset = relOfs;
        } else if (offset + len <= ofs + chunk.size) {
            // cout << "chunk end" << endl;
            endChunk = idx;
            endOffset = relOfs;
            break;
        }
        ofs += chunk.size;
        relOfs -= chunk.size;
    }
    // cout << "calculate new chunk start = " << startChunk << " startOffset = " << startOffset << endl;
    char *newPtr = new char[len];
    size_t tmp = 0;
    for (size_t idx = startChunk; idx < chunks.size(); idx++) {
        Chunk& chunk = chunks[idx];
        if (idx==startChunk) {
            memcpy(newPtr + tmp, chunk.ptr + startOffset, chunks[idx].size - startOffset);
            tmp += chunks[idx].size - startOffset;
        } else if (idx==endChunk) {
            memcpy(newPtr + tmp, chunk.ptr, chunks[idx].size - endOffset);
            break;
        } else {
            memcpy(newPtr + tmp, chunk.ptr, chunks[idx].size);
            tmp += chunks[idx].size;
        }
    }
    cout << "return alloc ptr " << (void*)newPtr << endl;
    data.chunks.push_back(Chunk{newPtr,len});
    data.owned = true;
    // Data ret(newPtr,len,true);
    return;
}

}


