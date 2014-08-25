/*
 * Tablecpp
 *
 *  Created on: Mar 29, 2014
 *      Author: arnd
 */

#include "TableData.h"

#include <fstream>


#include "utils/logging.h"
#include "utils/utility.h"
#include "utils/string.h"
#include "type/oids.h"
#include "type/TypeRegistry.h"


using namespace std;
using namespace log4cplus;

namespace db_agg {

static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("TableData"));

TableData::TableData() {

}

TableData::~TableData() {
    LOG_INFO("destroy table data (use_count)");
}

void TableData::setRowCount(uint64_t rowCount) {
    this->rowCount = rowCount;
}

uint64_t TableData::getRowCount() {
    return rowCount;
}

uint32_t TableData::getColCount()  {
    return colCount;
}

void TableData::setColumns(std::vector<ColDef> columns) {
    this->columns = columns;
    this->colCount = columns.size();
}


vector<ColDef>& TableData::getColumns() {
    return columns;
}

string TableData::getValue(uint64_t row, uint32_t col) {
    DataChunk chunk = getColumn(row,col);
    return string(chunk.getPtr(),chunk.getSize());
}

void TableData::save(std::string filePath) {
    vector<DataChunk> chunks;
    getRows(0,getRowCount(),chunks);
    LOG_DEBUG("save data in file " + filePath);
    ofstream os{filePath};
    for (size_t idx = 0; idx < getColCount(); idx++) {
        pair<string,uint32_t> p = getColumns()[idx];
        TypeInfo *ti =  TypeRegistry::getInstance().getTypeInfo((long int)p.second);
        if (ti == nullptr) {
            LOG_WARN("unknown type id '" << p.second << "' for columns '" << p.first << "'. assuming text");
            ti =  TypeRegistry::getInstance().getTypeInfo(TEXT);
        }
        os << p.first << ":" << ti->name;
        if (idx < getColCount() - 1) {
            os << "\t";
        }
    }
    os << endl;
    for (auto chunk:chunks) {
        os.write(chunk.getPtr(),chunk.getSize());
    }
    os.close();
}

string TableData::toSqlValues() {
    string values;
    size_t rows = getRowCount();
    size_t cols = getColCount();
    vector<TypeInfo*> typeInfos(cols);
    for (uint32_t col = 0;col<cols;col++) {
        auto colDef = getColumns()[col];
        TypeInfo *ti = TypeRegistry::getInstance().getTypeInfo(colDef.second);
        typeInfos[col] = ti;
    }
    for (size_t row=0; row<getRowCount(); row++) {
        values.append("(");
        for (uint32_t col = 0;col<cols;col++) {
            string colValue = getValue(row,col);
            bool isNull = false;
            // TODO: should be done in TypedValue to string
            if (colValue == "\\N") {
                isNull = true;
                colValue = "null";
            } else if (typeInfos[col]->category == 'B') {
                if (colValue == "t") {
                    colValue="true";
                } else if (colValue == "f") {
                    colValue="false";
                } else {
                    throw runtime_error("2invalid boolean value '" + colValue + "'");
                }
            }
            if (!isNull && typeInfos[col]->needsQuoting()) {
                values.append("E'");
            }
            values.append(colValue);
            if (!isNull && typeInfos[col]->needsQuoting()) {
                values.append("'");
            }
            if (row==0) {
                values.append("::");
                values.append(typeInfos[col]->name);
            }
            if (col<cols-1) {
                values.append(",");
            }
        }
        values.append(")");
        if (row<rows-1) {
            values.append(",");
        }
    }
    return values;
}

std::string TableData::toColumnDefinitions() {
    string sql;
    auto cols = getColumns();
    size_t len = cols.size();
    size_t idx = 0;
    for (auto col:cols) {
        string colName = col.first;
        uint32_t colType = col.second;
        LOG_DEBUG("process column " << colName);
        TypeInfo *ti = TypeRegistry::getInstance().getTypeInfo(colType);
        if (ti == nullptr) {
            LOG_WARN("unknown type id '" << colType << "' assuming text");
            ti =  TypeRegistry::getInstance().getTypeInfo(TEXT);
        }
        LOG_DEBUG("type name is  " << ti->name);
        sql += "  \"" + colName + "\" " + ti->name;
        if (idx<len-1) {
            sql += ",";
        }
        sql += "\n";
        idx++;
    }
    return sql;
}

uint32_t TableData::getColumnIndex(string colName) {
    for (uint32_t idx = 0; idx < columns.size(); idx++) {
        if (columns[idx].first == colName) {
            return idx;
        }
    }
    vector<string> availableColumns;
    for (uint32_t idx = 0; idx < columns.size(); idx++) {
        availableColumns.push_back(columns[idx].first);
    }
    THROW_EXC("column with name '" << colName << "' not found. available columns: " << join(availableColumns,","));
}

ColDef TableData::getColumn(string colName) {
    return getColumns()[getColumnIndex(colName)];
}

void TableData::addRow(std::vector<std::string> row) {
    throw runtime_error("not supported");
}

bool TableData::hasColumn(std::string colName) {
    for (uint32_t idx = 0; idx < columns.size(); idx++) {
        if (columns[idx].first == colName) {
            return true;
        }
    }
    return false;
}

}

