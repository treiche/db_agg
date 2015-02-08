/*
 * TableIndex.h
 *
 *  Created on: Mar 31, 2014
 *      Author: arnd
 */

#ifndef TABLEINDEX_H_
#define TABLEINDEX_H_

#include <cstdint>
#include <vector>
#include <string>

namespace db_agg {
class TableIndex {
private:
    uint64_t rowCount;
    uint16_t colCount;
    std::vector<uint64_t> offsets;
public:
    void addOffset(uint64_t offset);
    void setRowCount(uint64_t rowCount) {
        this->rowCount = rowCount;
    }
    uint64_t getRowCount() const {
        return rowCount;
    }
    void setColCount(uint32_t colCount) {
        this->colCount = colCount;
    }
    uint64_t getColCount() const {
        return colCount;
    }
    void save(std::string fileName);
    void load(std::string fileName);
    void clear() {
        offsets.clear();
        rowCount = 0;
        colCount = 0;
    }
    uint64_t getOffset(uint64_t row, uint32_t col) const;
};
}



#endif /* TABLEINDEX_H_ */
