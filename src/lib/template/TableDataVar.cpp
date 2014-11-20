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
	if (path == "columns") {
		return table->getColCount();
	}
	if (path == "rows") {
		return table->getRowCount();
	}
	return 0;
}

vector<string> TableDataVar::keys(string path) {
	return vector<string>();
}

VarType TableDataVar::type(string path) {
	vector<string> items;
	split(path,'.',items);
	if (items.size() > 0) {
		if (items[0] == "columns") {
			if (items.size() == 1) {
				return VarType::LIST;
			} else if (items.size() == 2) {
				return VarType::MAP;
			} else if (items.size() == 3) {
				if (items[2] == "name") {
					return VarType::STRING;
				} else if (items[2] == "type") {
					return VarType::INTEGER;
				}
			}
		}
	}
}

string TableDataVar::get_string(string path) {
	if (type(path) == VarType::INTEGER) {
		return to_string(get_integer(path));
	}
	vector<string> items;
	split(path,'.',items);
	if (items.size() == 3) {
		if (items[0] == "columns" && items[2] == "name") {
			size_t idx = stoi(items[1]);
			return table->getColumns()[idx].first;
		}
	}
	return "";
}

bool TableDataVar::get_bool(string path) {

}

int TableDataVar::get_integer(string path) {
	vector<string> items;
	split(path,'.',items);
	if (items.size() == 3) {
		if (items[0] == "columns" && items[2] == "type") {
			size_t idx = stoi(items[1]);
			return table->getColumns()[idx].second;
		}
	}
}

double TableDataVar::get_double(string path) {

}



}
