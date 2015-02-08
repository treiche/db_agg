/*
 * TableReader.h
 *
 *  Created on: Jun 12, 2014
 *      Author: arnd
 */

#ifndef TABLEREADER_H_
#define TABLEREADER_H_

namespace db_agg {
class TableReader {
public:
    virtual ~TableReader();
    virtual uint64_t getRowCount() = 0;
    virtual uint32_t getColCount() = 0;
    virtual std::vector<std::pair<std::string,uint32_t>> getColumns() = 0;
    template <class T>
    virtual void getValue(uint64_t row, uint32_t col, T& value) = 0;
};
}

#endif /* TABLEREADER_H_ */
