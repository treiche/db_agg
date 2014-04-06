/*
 * Tablecpp
 *
 *  Created on: Mar 29, 2014
 *      Author: arnd
 */

#include "TableData.h"

#include <log4cplus/logger.h>
#include "type/oids.h"
#include "type/TypeRegistry.h"


using namespace std;
using namespace log4cplus;

namespace db_agg {

static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("TableData"));


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
        TypedValue tv;
        for (uint32_t col = 0;col<cols;col++) {
            readValue(tv);
            string colValue = string(tv.value.stringVal,tv.getSize());
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
                    throw runtime_error("invalid boolean value '" + colValue + "'");
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
    int len = cols.size();
    int idx = 0;
    for (auto col:cols) {
        string colName = col.first;
        uint32_t colType = col.second;
        LOG4CPLUS_DEBUG(LOG, "process column " << colName);
        TypeInfo *ti = TypeRegistry::getInstance().getTypeInfo(colType);
        if (ti == nullptr) {
            LOG4CPLUS_WARN(LOG, "unknown type id '" << colType << "' assuming text");
            ti =  TypeRegistry::getInstance().getTypeInfo(TEXT);
        }
        LOG4CPLUS_DEBUG(LOG, "type name is  " << ti->name);
        sql += "  " + colName + " " + ti->name;
        if (idx<len-1) {
            sql += ",";
        }
        sql += "\n";
        idx++;
    }
    return sql;
}

}

