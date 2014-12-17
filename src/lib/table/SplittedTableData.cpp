/*
 * SplittedTableData.cpp
 *
 *  Created on: Apr 17, 2014
 *      Author: arnd
 */


#include "SplittedTableData.h"
#include "utils/logging.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
DECLARE_LOGGER("SplittedTableData");


SplittedTableData::SplittedTableData(shared_ptr<TableData> source, vector<uint64_t> rows) {
    this->source = source;
    setColumns(source->getColumns());
    this->rows = rows;
    setRowCount(rows.size());
}

void SplittedTableData::appendRaw(void *data, uint64_t size) {
    throw runtime_error("appendRaw not supported");
}

void SplittedTableData::getRows(uint64_t startRow, uint64_t count, std::vector<DataChunk>& chunks) {
    for (uint64_t row = startRow; row < startRow + count; row++) {
        source->getRows(rows[row],1,chunks);
    }
}

void SplittedTableData::save(string filePath) {
    // throw runtime_error("save not supported");
    TableData::save(filePath);
}

string SplittedTableData::calculateMD5Sum() {
    throw runtime_error("calculateMD5Sum not supported");
}

DataChunk SplittedTableData::getColumn(uint64_t row, uint32_t col) {
    return source->getColumn(rows[row], col);
}


}


