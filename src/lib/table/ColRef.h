/*
 * ColRef.h
 *
 *  Created on: Jul 3, 2014
 *      Author: arnd
 */

#ifndef COLREF_H_
#define COLREF_H_

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


}



#endif /* COLREF_H_ */
