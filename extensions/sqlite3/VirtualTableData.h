/*
 * VirtualTableData.h
 *
 *  Created on: Nov 29, 2014
 *      Author: arnd
 */

#ifndef VIRTUALTABLEDATA_H_
#define VIRTUALTABLEDATA_H_

#include <map>
#include <memory>
#include "table/TableData.h"

extern "C" {
#include <sqlite3.h>
}

int sq3_register_virtual_tabledata( sqlite3 * db );

void registerTableData(std::string name,
        std::shared_ptr<db_agg::TableData> tabledata);

#endif /* VIRTUALTABLEDATA_H_ */
