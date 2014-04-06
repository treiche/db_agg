/*
 * TableIndex.cpp
 *
 *  Created on: Mar 31, 2014
 *      Author: arnd
 */

#include "TableIndex.h"

#include <fstream>

#include <log4cplus/logger.h>


using namespace std;
using namespace log4cplus;

namespace db_agg {

static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("TableIndex"));


void TableIndex::addOffset(uint64_t offset) {
    assert(offsets.empty() || offset > offsets.back());
    offsets.push_back(offset);
}

void TableIndex::save(std::string fileName) {
    assert(offsets.size() % rowCount == 0);
    ofstream os{fileName};
    os.write((char*)&rowCount,sizeof(uint64_t));
    os.write((char*)&colCount,sizeof(uint32_t));
    for (auto offset:offsets) {
        os.write((char*)&offset,sizeof(uint64_t));
    }
    os.close();
}

void TableIndex::load(std::string fileName) {
    ifstream is{fileName};
    is.read((char*)&rowCount,sizeof(uint64_t));
    is.read((char*)&colCount,sizeof(uint32_t));
    for (size_t idx=0; idx < (rowCount * colCount); idx++) {
        uint64_t offset = 0;
        is.read((char*)&offset,sizeof(uint64_t));
        offsets.push_back(offset);
    }
    is.close();
}

uint64_t TableIndex::getOffset(uint64_t row, uint32_t col) {
    assert(row < rowCount);
    assert(col < colCount);
    return offsets[(row * colCount) + col];
}


}



