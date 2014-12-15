/*
 * TableDataRange.h
 *
 *  Created on: Dec 9, 2014
 *      Author: arnd
 */

#ifndef TABLEDATARANGE_H_
#define TABLEDATARANGE_H_

#include <memory>
#include <vector>

#include "TableData.h"

namespace db_agg {

class TableDataRange: public TableData {
private:
    std::shared_ptr<TableData> source;
    uint64_t startRow;
    uint64_t rowCount;
    TableDataRange(std::shared_ptr<TableData> source, uint64_t startRow, uint64_t rowCount);
public:
    virtual void appendRaw(void *data, uint64_t size) override;
    virtual void getRows(uint64_t startRow, uint64_t rows, std::vector<DataChunk>& chunks) override;
    virtual void save(std::string filePath) override;
    virtual std::string calculateMD5Sum() override;
    virtual DataChunk getColumn(uint64_t row, uint32_t col) override;
    friend class TableDataFactory;
};

}

#endif /* TABLEDATARANGE_H_ */
