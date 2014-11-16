/*
 * TableDataVar.h
 *
 *  Created on: Nov 12, 2014
 *      Author: arnd
 */

#ifndef TABLEDATAVAR_H_
#define TABLEDATAVAR_H_

#include <memory>
#include "Var.h"
#include "table/TableData.h"

namespace db_agg {
class TableDataVar: public Var {
private:
    std::shared_ptr<TableData> table;
public:
    TableDataVar(std::shared_ptr<TableData> tableData);
    virtual ~TableDataVar();
    virtual any& get(std::string path) override;
    virtual size_t size(std::string path) override;
};
}




#endif /* TABLEDATAVAR_H_ */
