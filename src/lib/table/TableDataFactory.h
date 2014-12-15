/*
 * TableDataFactory.h
 *
 *  Created on: Jun 11, 2014
 *      Author: arnd
 */

#ifndef TABLEDATAFACTORY_H_
#define TABLEDATAFACTORY_H_

#include <memory>

#include "TableData.h"
#include "ColRef.h"


namespace db_agg {
class TableDataFactory {
private:
    static TableDataFactory instance;
    TableDataFactory() {};
public:
    static TableDataFactory& getInstance();
    std::shared_ptr<TableData> create(std::string format, std::vector<std::pair<std::string,uint32_t>> columns);
    std::shared_ptr<TableData> create(std::string format, std::vector<std::string> columns);
    std::shared_ptr<TableData> load(std::string path);
    std::shared_ptr<TableData> join(std::vector<std::shared_ptr<TableData>> sources);
    std::shared_ptr<TableData> split(std::shared_ptr<TableData> source, std::vector<uint64_t> offsets);
    std::shared_ptr<TableData> extend(std::vector<std::shared_ptr<TableData>> tables);
    std::shared_ptr<TableData> extend(std::vector<ColRef> colRefs);
    std::shared_ptr<TableData> range(std::shared_ptr<TableData> source, uint64_t startRow, uint64_t rowCount);
};
}



#endif /* TABLEDATAFACTORY_H_ */
