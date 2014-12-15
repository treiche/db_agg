/*
 * TableDataFactory.cpp
 *
 *  Created on: Jun 11, 2014
 *      Author: arnd
 */

#include "TableDataFactory.h"
#include "CsvTableData.h"
#include "JoinedTableData.h"
#include "SplittedTableData.h"
#include "ExtendedTableData.h"
#include "TableDataRange.h"

using namespace std;

namespace db_agg {

TableDataFactory TableDataFactory::instance;

TableDataFactory& TableDataFactory::getInstance() {
    return instance;
}

shared_ptr<TableData> TableDataFactory::create(string format, vector<pair<string,uint32_t>> columns) {
    if (format == "text") {
        return shared_ptr<TableData>(new CsvTableData(columns));
    }
    throw runtime_error("unsupported format '" + format + "'");
}

shared_ptr<TableData> TableDataFactory::create(string format, vector<string> columns) {
    if (format == "text") {
        return shared_ptr<TableData>(new CsvTableData(columns));
    }
    throw runtime_error("unsupported format '" + format + "'");
}


shared_ptr<TableData> TableDataFactory::load(std::string path) {
    return shared_ptr<TableData>(new CsvTableData(path));
}

shared_ptr<TableData> TableDataFactory::join(vector<shared_ptr<TableData>> sources) {
    return shared_ptr<TableData>(new JoinedTableData(sources));
}

shared_ptr<TableData> TableDataFactory::split(shared_ptr<TableData> source, vector<uint64_t> offsets) {
    return shared_ptr<TableData>(new SplittedTableData(source,offsets));
}

shared_ptr<TableData> TableDataFactory::extend(vector<shared_ptr<TableData>> tables) {
    return shared_ptr<TableData>(new ExtendedTableData(tables));
}

shared_ptr<TableData> TableDataFactory::extend(vector<ColRef> colRefs) {
    return shared_ptr<TableData>(new ExtendedTableData(colRefs));
}

shared_ptr<TableData> TableDataFactory::range(shared_ptr<TableData> source, uint64_t startRow, uint64_t rowCount) {
    return shared_ptr<TableData>(new TableDataRange(source,startRow,rowCount));
}

}


