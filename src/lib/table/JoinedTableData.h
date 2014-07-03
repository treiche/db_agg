/*
 * JoinedTableData.h
 *
 *  Created on: Apr 17, 2014
 *      Author: arnd
 */

#ifndef JOINEDTABLEDATA_H_
#define JOINEDTABLEDATA_H_

#include <memory>
#include <vector>

#include "TableData.h"

namespace db_agg {

class JoinedTableData: public TableData {
private:
    std::vector<std::shared_ptr<TableData>> sources;
    std::vector<uint64_t> offsets;
    void calculateRelativeRow(uint64_t row, int& sourceIdx, uint64_t& relRow);
    JoinedTableData(std::vector<std::shared_ptr<TableData>> sources);
public:
    virtual void appendRaw(void *data, uint64_t size) override;
    virtual void *getRawRow(uint32_t row, uint32_t& size) override;
    virtual void save(std::string filePath) override;
    virtual std::string calculateMD5Sum() override;
    virtual std::string getValue(uint64_t row, uint32_t col) override;
    friend class TableDataFactory;
};

}



#endif /* JOINEDTABLEDATA_H_ */
