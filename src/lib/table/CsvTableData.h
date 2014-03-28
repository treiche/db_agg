#ifndef CSVTABLEDATA_H_
#define CSVTABLEDATA_H_

#include <cstdint>
#include <string>
#include <vector>

#include "type/TypedValue.h"
#include "TableData.h"

namespace db_agg {
class CsvTableData : public TableData {
private:
    struct XImpl;
    XImpl *pImpl;
    void readData();
    void loadColumns();
    void readColumns(std::string firstLine);
    void calculateRowCount();
    void buildIndex();
    void loadOnDemand(std::string reason);
public:
    CsvTableData(std::string csvFile);
    CsvTableData(std::string csvFile, std::vector<std::pair<std::string,uint32_t>> columns);
    CsvTableData(std::vector<std::string> columns);
    CsvTableData(std::vector<std::pair<std::string,uint32_t>> columns);
    CsvTableData(void *data, uint64_t size);
    virtual ~CsvTableData() override;
    virtual uint64_t getRowCount() override;
    virtual uint32_t getColCount() override;
    virtual std::vector<std::pair<std::string,uint32_t>> getColumns() override;
    virtual void setPointer(uint64_t row, uint32_t col) override;
    virtual void readValue(TypedValue& value) override;
    virtual void readValue(uint32_t row, uint32_t col, TypedValue& value) override;
    virtual void * getRaw() override;
    virtual uint64_t getSize() override;
    virtual void setRaw(void *data, uint64_t size) override;
    virtual void appendRaw(void *data, uint64_t size) override;
    virtual void *getRawRow(uint32_t row, uint32_t& size) override;
    virtual void save(std::string filePath) override;
    virtual std::string calculateMD5Sum() override;
};
}

#endif /* CSVTABLEDATA_H_ */
