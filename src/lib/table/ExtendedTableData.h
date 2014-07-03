/*
 * ExtendedTableData.h
 *
 *  Created on: Jun 11, 2014
 *      Author: arnd
 */

#ifndef EXTENDEDTABLEDATA_H_
#define EXTENDEDTABLEDATA_H_

#include <memory>
#include "TableData.h"

namespace db_agg {

class ColRef {
private:
    std::shared_ptr<TableData> table;
    uint32_t colIdx;
public:
    ColRef(std::shared_ptr<TableData> table, uint32_t colIdx);
    std::shared_ptr<TableData> getTable();
    uint32_t getColIdx();
};

class ExtendedTableData: public TableData {
private:
    std::vector<std::pair<std::string,uint32_t>> columns;
    uint32_t colCount;
    uint64_t rowCount;
    std::vector<ColRef> colRefs;
    std::vector<size_t> tableOffsets;
    std::vector<uint32_t> columnOffsets;
    ExtendedTableData(std::vector<std::shared_ptr<TableData>> tables);
    ExtendedTableData(std::vector<ColRef> columns);
    void init(std::vector<ColRef> columns);
public:
    virtual uint64_t getRowCount() override;
    virtual uint32_t getColCount() override;
    virtual std::vector<std::pair<std::string,uint32_t>> getColumns() override;
    virtual void * getRaw() override;
    virtual uint64_t getSize() override;
    virtual void setRaw(void *data, uint64_t size) override;
    virtual void appendRaw(void *data, uint64_t size) override;
    virtual void *getRawRow(uint32_t row, uint32_t& size) override;
    virtual void save(std::string filePath) override;
    virtual std::string calculateMD5Sum() override;
    virtual std::string getValue(uint64_t row, uint32_t col) override;
    friend class TableDataFactory;
};
}



#endif /* EXTENDEDTABLEDATA_H_ */
