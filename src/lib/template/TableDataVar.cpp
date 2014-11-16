/*
 * TableDataVar.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: arnd
 */


#include "TableDataVar.h"
#include "utils/string.h"

using namespace std;


namespace db_agg {

TableDataVar::TableDataVar(shared_ptr<TableData> table): table(table) {}

TableDataVar::~TableDataVar() {
	table.reset();
}

size_t TableDataVar::size(string path) {
	if (path == "root.columns") {
		return table->getColCount();
	}
	if (path == "root.rows") {
		return table->getRowCount();
	}
	return 0;
}

any& TableDataVar::get(string path) {
	vector<string> items;
	split(path,'.',items);
	if (items[0]=="columns") {
		if (items.size() > 1) {
			size_t idx = stoi(items[1]);
			if (items.size()>2 && items[2] == "name") {
				any& a = *new any(table->getColumns()[idx].first);
				return a;
			}
			if (items.size()>2 && items[2] == "type") {
				any& a = *new any(table->getColumns()[idx].second);
				return a;
			}
		}
	}
}

}

