/*
 * Memcached.h
 *
 *  Created on: Jun 18, 2014
 *      Author: arnd
 */

#ifndef MEMCACHEDQUERY_H_
#define MEMCACHEDQUERY_H_

#include "table/TableData.h"
#include <memory>

namespace memcached {
class MemcachedQuery: public db_agg::QueryExecution {
private:
    uint64_t lastOffset = 0;
    uint64_t chunkSize = 1000;
    std::shared_ptr<db_agg::TableData> sourceTable;
    std::shared_ptr<db_agg::TableData> memcachedResultTable;
    uint32_t keyColIdx = 0;
public:
    virtual bool process() override;
};
}




#endif /* MEMCACHEDQUERY_H_ */
