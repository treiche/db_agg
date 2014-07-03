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
    std::shared_ptr<TableData> source;
    std::vector<uint64_t> rows;
    SplittedTableData(std::shared_ptr<TableData> source, std::vector<uint64_t> offsets);
public:
    virtual void appendRaw(void *data, uint64_t size) override;
    virtual void getRows(uint64_t startRow, uint64_t rows, std::vector<DataChunk>& chunks) override;
    virtual void save(std::string filePath) override;
    virtual std::string calculateMD5Sum() override;
    virtual DataChunk getColumn(uint64_t row, uint32_t col) override;
    friend class TableDataFactory;
};

}




#endif /* SPLITTEDTABLEDATA_H_ */
