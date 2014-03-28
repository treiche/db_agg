/*
 * PGResult.h
 *
 *  Created on: Feb 1, 2014
 *      Author: arnd
 */

#ifndef PGRESULT_H_
#define PGRESULT_H_

#include <libpq-fe.h>
#include <string>
#include <vector>

namespace db_agg {

class PGResult {
private:
    PGresult *_result = nullptr;
public:
    ~PGResult();
    void initialize(PGresult *result);
    ExecStatusType getStatusType();
    std::string getStatusTypeAsString(ExecStatusType status);
    std::string getStatus();
    size_t getRowCount();
    size_t getColCount();
    std::string getValue(size_t row, size_t col);
    std::string getColumnName(size_t col);
    uint32_t getColumnType(size_t col);
    std::string getCommandStatus();
    operator bool();
    std::vector<std::pair<std::string,std::uint32_t>> getColumns();
};

}

#endif /* PGRESULT_H_ */
