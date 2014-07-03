#ifndef CSVTABLEDATA_H_
#define CSVTABLEDATA_H_

#include <cstdint>
#include <string>
#include <vector>

#include "TableData.h"
#include "TableIndex.h"

namespace db_agg {
class CsvTableData : public TableData {
private:
    char *data = nullptr;
    uint64_t size = 0;
    uint32_t currentColumn = 0;
    uint64_t currentRow = 0;
    uint64_t ptr = 0;
    TableIndex index;
    std::string fileName;
    void readData();
    void loadColumns();
    void readColumns(std::string firstLine);
    void calculateRowCount();
    void buildIndex();
    void loadOnDemand(std::string reason);
    CsvTableData(std::string csvFile);
    CsvTableData(std::string csvFile, std::vector<std::pair<std::string,uint32_t>> columns);
    CsvTableData(std::vector<std::string> columns);
    CsvTableData(std::vector<std::pair<std::string,uint32_t>> columns);
public:
    virtual ~CsvTableData() override;
    virtual uint64_t getRowCount() override;
    virtual uint32_t getColCount() override;
    virtual std::vector<ColDef>& getColumns() override;
    virtual void appendRaw(void *data, uint64_t size) override;
    virtual void getRows(uint64_t startRow, uint64_t rows, std::vector<DataChunk>& chunks) override;
    virtual void save(std::string filePath) override;
    virtual std::string calculateMD5Sum() override;
    virtual DataChunk getColumn(uint64_t row, uint32_t col) override;
    virtual void addRow(std::vector<std::string> row) override;
    friend class TableDataFactory;
};
}

#endif /* CSVTABLEDATA_H_ */
