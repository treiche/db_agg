/*
 * SplittedTableData.h
 *
 *  Created on: Apr 17, 2014
 *      Author: arnd
 */

#ifndef SPLITTEDTABLEDATA_H_
#define SPLITTEDTABLEDATA_H_

#include <memory>
#include <vector>

#include "TableData.h"

namespace db_agg {

class SplittedTableData: public TableData {
private:
    uint64_t rowCount;
    uint32_t colCount;
    std::shared_ptr<TableData> source;
    std::vector<uint64_t> rows;
public:
    SplittedTableData(std::shared_ptr<TableData> source, std::vector<uint64_t> offsets);
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
};

}




#endif /* SPLITTEDTABLEDATA_H_ */
