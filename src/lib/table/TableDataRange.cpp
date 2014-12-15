/*
 * TableDataRange.cpp
 *
 *  Created on: Dec 9, 2014
 *      Author: arnd
 */


#include "TableDataRange.h"
#include "utils/logging.h"

using namespace std;


namespace db_agg {

DECLARE_LOGGER("TableDataRange");

TableDataRange::TableDataRange(shared_ptr<TableData> source, uint64_t startRow, uint64_t rowCount):
    source(source),
    startRow(startRow),
    rowCount(rowCount) {

    setColumns(source->getColumns());
    setRowCount(rowCount);
}

void TableDataRange::appendRaw(void *data, uint64_t size) {
    throw runtime_error("appendRaw not supported");
}

void TableDataRange::getRows(uint64_t startRow, uint64_t count, std::vector<DataChunk>& chunks) {
    for (uint64_t row = startRow; row < startRow + count; row++) {
        LOG_ERROR("getRows(" << row << ")");
        source->getRows(this->startRow + startRow,1,chunks);
    }
}

void TableDataRange::save(string filePath) {
    // throw runtime_error("save not supported");
    TableData::save(filePath);
}

string TableDataRange::calculateMD5Sum() {
    throw runtime_error("calculateMD5Sum not supported");
}

DataChunk TableDataRange::getColumn(uint64_t row, uint32_t col) {
    return source->getColumn(this->startRow + row, col);
}


}
