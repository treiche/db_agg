/*
 * SplittedTableData.cpp
 *
 *  Created on: Apr 17, 2014
 *      Author: arnd
 */


#include "SplittedTableData.h"
#include <log4cplus/logger.h>

using namespace std;
using namespace log4cplus;

namespace db_agg {
static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("SplittedTableData"));


SplittedTableData::SplittedTableData(shared_ptr<TableData> source, vector<uint64_t> rows) {
    this->source = source;
    //this->colCount = source->getColCount();
    setColumns(source->getColumns());
    this->rows = rows;
    setRowCount(rows.size());
}

void SplittedTableData::appendRaw(void *data, uint64_t size) {
    throw runtime_error("appendRaw not supported");
}

void *SplittedTableData::getRawRow(uint32_t row, uint32_t& size) {
    return source->getRawRow(rows[row], size);
}
void SplittedTableData::save(string filePath) {
    throw runtime_error("save not supported");
}

string SplittedTableData::calculateMD5Sum() {
    throw runtime_error("calculateMD5Sum not supported");
}

string SplittedTableData::getValue(uint64_t row, uint32_t col) {
    return source->getValue(rows[row], col);
}


}


