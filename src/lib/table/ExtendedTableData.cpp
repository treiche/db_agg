/*
 * ExtendedTableData.cpp
 *
 *  Created on: Jun 11, 2014
 *      Author: arnd
 */

#include <assert.h>
#include "ExtendedTableData.h"
#include "Data.h"

using namespace std;

namespace db_agg {

ColRef::ColRef(std::shared_ptr<TableData> table, uint32_t colIdx):
        table(table),
        colIdx(colIdx) {}


shared_ptr<TableData> ColRef::getTable() {
    return table;
}

uint32_t ColRef::getColIdx() {
    return colIdx;
}


ExtendedTableData::ExtendedTableData(vector<shared_ptr<TableData>> tables) {
    vector<ColRef> refs;
    for (auto& table:tables) {
        for (uint32_t colNo = 0; colNo < table->getColCount(); colNo++) {
            refs.push_back(ColRef{table,colNo});
        }
    }
    init(refs);
}

ExtendedTableData::ExtendedTableData(vector<ColRef> columns) {
    init(columns);
}

void ExtendedTableData::init(vector<ColRef> colRefs) {
    assert(colRefs.size() > 0);
    this->colRefs = colRefs;
    setRowCount(colRefs[0].getTable()->getRowCount());
    vector<ColDef> colDefs;
    for (auto& colRef:colRefs) {
        if (colRef.getTable()->getRowCount() != getRowCount()) {
            throw runtime_error("tables need same row count to be extendable");
        }
        colDefs.push_back(colRef.getTable()->getColumns()[colRef.getColIdx()]);
    }
    setColumns(colDefs);
}


void ExtendedTableData::appendRaw(void *data, uint64_t size) {
    throw runtime_error("appendRaw not supported");
}

void *ExtendedTableData::getRawRow(uint32_t row, uint32_t& size) {
    // throw runtime_error("getRawRow not supported");
    /*
    string raw;
    Data chunks;
    for (auto& source:sources) {
        uint32_t len;
        char *p = (char*)source->getRawRow(row,len);
        chunks.addChunk(p,len);
        size += len;
    }
    Data normalized;
    chunks.range(0,chunks.size(),normalized);
    return (void*)normalized.getPtr();
    */
}

void ExtendedTableData::save(string filePath) {
    throw runtime_error("save not supported");
}

string ExtendedTableData::calculateMD5Sum() {
    throw runtime_error("calculateMD5Sum not supported");
}

string ExtendedTableData::getValue(uint64_t row, uint32_t col) {
    return colRefs[col].getTable()->getValue(row,colRefs[col].getColIdx());
}



}


