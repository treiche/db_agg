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
    // Var API
	virtual size_t size(std::string path) override;
	virtual std::vector<std::string> keys(std::string path) override;
	virtual VarType type(std::string path) override;
	virtual std::string get_string(std::string path) override;
    virtual bool get_bool(std::string path) override;
    virtual int get_integer(std::string path) override;
    virtual double get_double(std::string path) override;
};
}




#endif /* TABLEDATAVAR_H_ */
