#ifndef TABLEDATA_H_
#define TABLEDATA_H_

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

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
    virtual ~TableData() {};
    virtual uint64_t getRowCount();
    virtual uint32_t getColCount();
    virtual std::vector<ColDef>& getColumns();
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
    virtual void addRow(std::vector<std::string> row);
    uint32_t getColumnIndex(std::string colName);
};
}

#endif /* TABLEDATA_H_ */
