/*
 * ColRef.cpp
 *
 *  Created on: Jul 3, 2014
 *      Author: arnd
 */

#include "ColRef.h"

using namespace std;

namespace db_agg {

ColRef::ColRef(std::shared_ptr<TableData> table, uint32_t colIdx):
        table(table),
        colIdx(colIdx) {}


shared_ptr<TableData> ColRef::getTable() const {
    return table;
}

uint32_t ColRef::getColIdx() const {
    return colIdx;
}


}


