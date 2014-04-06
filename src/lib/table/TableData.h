#ifndef TABLEDATA_H_
#define TABLEDATA_H_

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "type/TypedValue.h"

namespace db_agg {

class TableData {
public:
    virtual ~TableData() {};
    virtual uint64_t getRowCount() = 0;
    virtual uint32_t getColCount() = 0;
    virtual std::vector<std::pair<std::string,uint32_t>> getColumns() = 0;
    virtual void setPointer(uint64_t row, uint32_t col) = 0;
    virtual void readValue(TypedValue& value) = 0;
    virtual void readValue(uint32_t row, uint32_t col, TypedValue& value) = 0;
    virtual void * getRaw() = 0;
    virtual uint64_t getSize() = 0;
    virtual void setRaw(void *data, uint64_t size) = 0;
    virtual void appendRaw(void *data, uint64_t size) = 0;
    virtual void *getRawRow(uint32_t row, uint32_t& size) = 0;
    virtual void save(std::string filePath) = 0;
    virtual std::string calculateMD5Sum() = 0;
    virtual std::string toSqlValues();
    virtual std::string toColumnDefinitions();
    virtual std::string getValue(uint64_t row, uint32_t col) = 0;
};
}

#endif /* TABLEDATA_H_ */
