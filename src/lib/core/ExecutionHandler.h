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
    virtual std::string handleCopyIn(size_t step, uint32_t row) = 0;
    virtual uint64_t getRowCount(size_t step) = 0;
    virtual void handleCopyOut(size_t step, std::string data) = 0;
    virtual void handleTuples(size_t step, std::vector<std::pair<std::string, uint32_t>>& columns) = 0;

    virtual std::shared_ptr<TableData> getResult() = 0;
    virtual void setResult(std::shared_ptr<TableData> data) = 0;
    virtual void addDependency(std::string name, std::shared_ptr<TableData> data) = 0;
    virtual std::string inject(std::string query, size_t copyThreshold) = 0;
    virtual bool isComplete() = 0;
    virtual ~ExecutionHandler() {};
};
}

#endif /* EXECUTIONHANDLER_H_ */
