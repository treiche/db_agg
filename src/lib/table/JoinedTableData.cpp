/*
 * JoinedTableData.cpp
 *
 *  Created on: Apr 17, 2014
 *      Author: arnd
 */


#include "JoinedTableData.h"
#include <log4cplus/logger.h>

using namespace std;
using namespace log4cplus;

namespace db_agg {
static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("JoinedTableData"));


JoinedTableData::JoinedTableData(std::vector<std::shared_ptr<TableData>> sources) {
    assert(!sources.empty());
    uint64_t rowCount = 0;
    uint32_t colCount = sources[0]->getColCount();
    offsets.push_back(0);
    for (size_t idx = 0; idx < sources.size(); idx++) {
        auto& source = sources[idx];
        rowCount += source->getRowCount();
        offsets.push_back(rowCount);
        assert(source->getColCount()==colCount);
    }
    this->sources = sources;
    setColumns(sources[0]->getColumns());
    setRowCount(rowCount);
}

void JoinedTableData::appendRaw(void *data, uint64_t size) {
    throw runtime_error("not supported");
}

void JoinedTableData::getRows(uint64_t startRow, uint64_t rows, std::vector<DataChunk>& chunks) {
   for (uint64_t row = startRow; row < startRow + rows; row++) {
       int sourceIdx;
       uint64_t relRow;
       calculateRelativeRow(row, sourceIdx, relRow);
       sources[sourceIdx]->getRows(relRow,1,chunks);
   }
}

void JoinedTableData::save(string filePath) {
    throw runtime_error("not supported");
}

string JoinedTableData::calculateMD5Sum() {
    throw runtime_error("not supported");
}

void JoinedTableData::calculateRelativeRow(uint64_t row,int& sourceIdx,uint64_t& relativeRow) {
    for (int idx=0;idx<offsets.size()-1;idx++) {
        uint64_t offset = offsets[idx];
        if (row >= offset && row < offsets[idx+1]) {
            //cout << "source " << idx << endl;
            uint64_t relRow = row - offset;
            //cout << "offset = " << offset << " relOfs = " << relRow << endl;
            sourceIdx = idx;
            relativeRow = relRow;
            return;
        }
    }
    throw runtime_error("row out of bounds");
}

DataChunk JoinedTableData::getColumn(uint64_t row, uint32_t col) {
    int sourceIdx;
    uint64_t relRow;
    calculateRelativeRow(row, sourceIdx, relRow);
    return sources[sourceIdx]->getColumn(relRow,col);
}


}

