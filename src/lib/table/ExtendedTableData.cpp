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

void ExtendedTableData::getRows(uint64_t startRow, uint64_t rows, std::vector<DataChunk>& chunks) {
    DataChunk delimiter((char*)"\t",1);
    DataChunk eol((char*)"\n",1);
    for (uint64_t row = startRow; row < startRow + rows; row++) {
        for (uint32_t col = 0; col < getColCount(); col++) {
            chunks.push_back(getColumn(row,col));
            if (col < getColCount() - 1) {
                chunks.push_back(delimiter);
            }
        }
        chunks.push_back(eol);
    }
}


void ExtendedTableData::save(string filePath) {
    TableData::save(filePath);
}

string ExtendedTableData::calculateMD5Sum() {
    throw runtime_error("calculateMD5Sum not supported");
}

DataChunk ExtendedTableData::getColumn(uint64_t row, uint32_t col) {
    return colRefs[col].getTable()->getColumn(row,colRefs[col].getColIdx());
}



}


