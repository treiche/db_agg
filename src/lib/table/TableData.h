#ifndef TABLEDATA_H_
#define TABLEDATA_H_

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "DataChunk.h"


namespace db_agg {

using ColDef = std::pair<std::string,uint32_t>;

class TableData {
private:
    uint64_t rowCount = 0;
    uint32_t colCount = 0;
    std::vector<ColDef> columns;
protected:
    void setRowCount(uint64_t rowCount);
    void setColumns(std::vector<ColDef> columns);
    TableData();
public:
    virtual ~TableData();
    virtual uint64_t getRowCount() const;
    virtual uint32_t getColCount() const;
    virtual const std::vector<ColDef>& getColumns() const;
    virtual void appendRaw(void *data, uint64_t size) = 0;
    virtual void getRows(uint64_t startRow, uint64_t rows, std::vector<DataChunk>& chunks) const = 0;
    virtual void save(std::string filePath);
    virtual std::string calculateMD5Sum() const = 0;
    virtual std::string toSqlValues() const;
    virtual std::string toColumnDefinitions() const;
    virtual DataChunk getColumn(uint64_t row, uint32_t col) const = 0;
    virtual void addRow(std::vector<std::string> row);
    uint32_t getColumnIndex(std::string colName) const;
    ColDef getColumn(std::string colName) const;
    bool hasColumn(std::string colName) const;
    std::string getValue(uint64_t row, uint32_t col) const;
};
}

#endif /* TABLEDATA_H_ */
