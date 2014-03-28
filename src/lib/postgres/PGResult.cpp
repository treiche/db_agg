/*
 * PGResult.cpp
 *
 *  Created on: Feb 1, 2014
 *      Author: arnd
 */


#include "PGResult.h"
#include <log4cplus/logger.h>

using namespace std;
using namespace log4cplus;

namespace db_agg {
static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("PGResult"));


void PGResult::initialize(PGresult *result) {
    _result = result;
}

PGResult::~PGResult() {
    PQclear(_result);
    _result = nullptr;
}

ExecStatusType PGResult::getStatusType() {
    assert(_result!=nullptr);
    return PQresultStatus(_result);
}

string PGResult::getStatusTypeAsString(ExecStatusType status) {
    char *s = PQresStatus(status);
    return string(s);
}

std::string PGResult::getStatus() {
    return getStatusTypeAsString(getStatusType());
}

size_t PGResult::getRowCount() {
    assert(_result!=nullptr);
    return PQntuples(_result);
}

size_t PGResult::getColCount() {
    assert(_result!=nullptr);
    return PQnfields(_result);
}

string PGResult::getValue(size_t row, size_t col) {
    assert(_result!=nullptr);
    char *c = PQgetvalue(_result,0,0);
    return string(c);
}

std::string PGResult::getColumnName(size_t col) {
    assert(_result!=nullptr);
    char *c = PQfname(_result, col);
    return string(c);
}

uint32_t PGResult::getColumnType(size_t col) {
    assert(_result!=nullptr);
    return PQftype(_result,col);
}

string PGResult::getCommandStatus() {
    assert(_result!=nullptr);
    char *status = PQcmdStatus(_result);
    return string(status);
}

PGResult::operator bool() {
    return _result != nullptr;
}

vector<pair<string,uint32_t>> PGResult::getColumns() {
    vector<pair<string,uint32_t>> columns;
    size_t colCount = getColCount();
    for (int idx = 0; idx < colCount; idx++) {
        //char *name = PQfname(res, idx);
        string name = getColumnName(idx);
        //uint32_t type = PQftype(res,idx);
        uint32_t type = getColumnType(idx);
        columns.push_back(pair<string,uint32_t>(name,type));
    }
    return columns;
}


}
