/*
 * ExtendedTableData.cpp
 *
 *  Created on: Jun 11, 2014
 *      Author: arnd
 */

#include <assert.h>
#include "ExtendedTableData.h"

using namespace std;

namespace db_agg {

ExtendedTableData::ExtendedTableData(vector<shared_ptr<TableData>> tables) {
    assert(tables.size() > 0);
    auto rowCount = tables[0]->getRowCount();
    for (auto table:tables) {
        if (table->getRowCount() != rowCount) {
            throw runtime_error("tables need same row count to be extendable");
        }
    }
}

uint64_t ExtendedTableData::getRowCount() {
    return sources[0]->getRowCount();
}

uint32_t ExtendedTableData::getColCount() {
    throw runtime_error("getColCount not supported");
}

vector<pair<string,uint32_t>> ExtendedTableData::getColumns() {
    throw runtime_error("getColumns not supported");
}

void * ExtendedTableData::getRaw() {
    throw runtime_error("deprecated");
}
uint64_t ExtendedTableData::getSize() {
    throw runtime_error("getSize not supported");
}
void ExtendedTableData::setRaw(void *data, uint64_t size) {
    throw runtime_error("setRaw not supported");

}
void ExtendedTableData::appendRaw(void *data, uint64_t size) {
    throw runtime_error("appendRaw not supported");
}

void *ExtendedTableData::getRawRow(uint32_t row, uint32_t& size) {
    throw runtime_error("getRawRow not supported");
}
void ExtendedTableData::save(string filePath) {
    throw runtime_error("save not supported");
}

string ExtendedTableData::calculateMD5Sum() {
    throw runtime_error("calculateMD5Sum not supported");
}

string ExtendedTableData::getValue(uint64_t row, uint32_t col) {
    throw runtime_error("getValue not supported");
}



}


