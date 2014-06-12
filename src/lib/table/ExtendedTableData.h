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
class ExtendedTableData: public TableData {
private:
    std::vector<std::shared_ptr<TableData>> sources;
    ExtendedTableData(std::vector<std::shared_ptr<TableData>> tables);
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
