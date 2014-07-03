#ifndef EXECUTIONHANDLER_H_
#define EXECUTIONHANDLER_H_

#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include <memory>

#include "table/TableData.h"

namespace db_agg {


class ExecutionHandler {
public:
    virtual ~ExecutionHandler() {};
    virtual void handleCopyIn(size_t step, uint64_t startRows, uint64_t rows, std::vector<DataChunk>& chunks, uint64_t& rowsRead) = 0;
    virtual uint64_t getRowCount(size_t step) = 0;
    virtual void handleCopyOut(size_t step, std::string data) = 0;
    virtual void handleTuples(size_t step, std::vector<std::pair<std::string, uint32_t>>& columns) = 0;
};
}

#endif /* EXECUTIONHANDLER_H_ */
