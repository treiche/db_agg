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
#include "ColRef.h"

namespace db_agg {

class ExtendedTableData: public TableData {
private:
    std::vector<ColRef> colRefs;
    std::vector<size_t> tableOffsets;
    std::vector<uint32_t> columnOffsets;
    ExtendedTableData(std::vector<std::shared_ptr<TableData>> tables);
    ExtendedTableData(std::vector<ColRef> columns);
    void init(std::vector<ColRef> columns);
public:
    virtual ~ExtendedTableData();
    virtual void appendRaw(void *data, uint64_t size) override;
    virtual void getRows(uint64_t startRow, uint64_t rows, std::vector<DataChunk>& chunks) const override;
    virtual void save(std::string filePath) override;
    virtual std::string calculateMD5Sum() const override;
    virtual DataChunk getColumn(uint64_t row, uint32_t col) const override;
    friend class TableDataFactory;
};
}



#endif /* EXTENDEDTABLEDATA_H_ */
