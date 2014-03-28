/*
 * QueryExecution.cpp
 *
 *  Created on: Dec 29, 2013
 *      Author: arnd
 */

#include "QueryExecution.h"

#include <log4cplus/logger.h>

#include "type/oids.h"
#include "type/TypeRegistry.h"
#include "table/CsvTableData.h"
#include "utils/RegExp.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
    static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("QueryExecution"));

    QueryExecution::QueryExecution() {}

    QueryExecution::QueryExecution(std::string name, std::string id, std::string connectionUrl, std::string sql, std::vector<std::string> depNames):
            id(id),
            connectionUrl(connectionUrl),
            sql(sql),
            name(name) {
        for (auto& depName:depNames) {
            dependencies[depName] = nullptr;
        }
    }

    QueryExecution::~QueryExecution() {
        LOG4CPLUS_TRACE(LOG, "delete query execution");
        if (data!=nullptr) {
            delete data;
        }
    }

    void QueryExecution::addTransition(Transition *t) {
        transitions.push_back(t);
    }

    void QueryExecution::addIncomingTransition(Transition *t) {
        incomingTransitions.push_back(t);
    }

    void QueryExecution::addDependency(string name, TableData *data) {
        if (dependencies.find(name)==dependencies.end()) {
            throw runtime_error("no dependency '" + name + "' declared");
        }
        dependencies[name] = data;
    }

    string QueryExecution::inject(string query, size_t copyThreshold) {
        LOG4CPLUS_DEBUG(LOG, "called inject " << query);
        for (auto dep:dependencies) {
            if (dep.second == nullptr) {
                throw runtime_error("can't inject sql because of missing dependency '" + dep.first + "'");
            }
        }
        string sql;
        string step = "";
        //step += "SET SESSION statement_timeout TO 1;\n select pg_sleep(2); \n";
        //sql += step;
        //steps.push_back(ExecutionStep{step,nullptr});
        // 1. Step
        for (pair<string,TableData*> dependency:dependencies) {
            LOG4CPLUS_DEBUG(LOG, "process dependency " << dependency.first);
            step = "CREATE TEMPORARY TABLE " + dependency.first + "(\n";
            auto cols = dependency.second->getColumns();
            int len = cols.size();
            int idx = 0;
            for (auto col:cols) {
                string colName = col.first;
                uint32_t colType = col.second;
                LOG4CPLUS_DEBUG(LOG, "process column " << colName);
                TypeInfo *ti = TypeRegistry::getInstance().getTypeInfo(colType);
                if (ti == nullptr) {
                    LOG4CPLUS_WARN(LOG, "unknown type id '" << colType << "' assuming text");
                    ti =  TypeRegistry::getInstance().getTypeInfo(TEXT);
                }
                LOG4CPLUS_DEBUG(LOG, "type name is  " << ti->name);
                step += "  " + colName + " " + ti->name;
                if (idx<len-1) {
                    step += ",";
                }
                step += "\n";
                idx++;
            }
            step += ") ON COMMIT DROP;\n";
            sql += step;
            steps.push_back(ExecutionStep{step,nullptr});
            if (dependency.second->getRowCount() > 0) {
                if (dependency.second->getRowCount() < copyThreshold) {
                    step = "INSERT INTO " + dependency.first + " VALUES " + toSqlValues(*dependency.second) + ";\n";
                    steps.push_back(ExecutionStep{step,dependency.second});
                } else {
                    step = "COPY " + dependency.first + " FROM STDIN;\n";
                    steps.push_back(ExecutionStep{step,dependency.second});
                }
                sql += step;
            }
        }
        // 2. step: get meta information
        RegExp re{R"(limit ([1-9]+[0-9]*))"};
        vector<RegExp::match> matches;
        step = query;
        bool foundLimit = false;
        do {
             matches = re.exec(step);
             if (!matches.empty()) {
                 step = step.substr(0,matches[1].start) + "0" + step.substr(matches[1].end);
                 foundLimit = true;
             }
        } while(!matches.empty());
        if (!foundLimit) {
            step += " limit 0";
        }
        step += ";\n";
        steps.push_back(ExecutionStep{step,nullptr});
        sql += step;
        // 3. step: get binary result
        step = "COPY (" + query + ") TO STDOUT;\n";
        steps.push_back(ExecutionStep{step,nullptr});
        sql += step;
        LOG4CPLUS_DEBUG(LOG, "query = " << sql);
        this->dependencies = dependencies;
        return sql;
    }

    string QueryExecution::toSqlValues(TableData& data) {
        string values;
        size_t rows = data.getRowCount();
        size_t cols = data.getColCount();
        vector<TypeInfo*> typeInfos(cols);
        for (uint32_t col = 0;col<cols;col++) {
            auto colDef = data.getColumns()[col];
            TypeInfo *ti = TypeRegistry::getInstance().getTypeInfo(colDef.second);
            typeInfos[col] = ti;
        }
        for (size_t row=0; row<data.getRowCount(); row++) {
            values.append("(");
            TypedValue tv;
            for (uint32_t col = 0;col<cols;col++) {
                data.readValue(tv);
                string colValue = string(tv.value.stringVal,tv.getSize());
                bool isNull = false;
                // TODO: should be done in TypedValue to string
                if (colValue == "\\N") {
                    isNull = true;
                    colValue = "null";
                } else if (typeInfos[col]->category == 'B') {
                    if (colValue == "t") {
                        colValue="true";
                    } else if (colValue == "f") {
                        colValue="false";
                    } else {
                        throw runtime_error("invalid boolean value '" + colValue + "'");
                    }
                }
                if (!isNull && typeInfos[col]->needsQuoting()) {
                    values.append("E'");
                }
                values.append(colValue);
                if (!isNull && typeInfos[col]->needsQuoting()) {
                    values.append("'");
                }
                if (row==0) {
                    values.append("::");
                    values.append(typeInfos[col]->name);
                }
                if (col<cols-1) {
                    values.append(",");
                }
            }
            values.append(")");
            if (row<rows-1) {
                values.append(",");
            }
        }
        return values;
    }

    uint64_t QueryExecution::getRowCount(size_t stepNo) {
        ExecutionStep& step = steps[stepNo];
        return step.getDependency()->getRowCount();
    }


    string QueryExecution::handleCopyIn(size_t stepNo, uint32_t row) {
        ExecutionStep& step = steps[stepNo];
        uint32_t size;
        void *rawRow = step.getDependency()->getRawRow(row,size);
        return string((const char*)rawRow,size);
    }

    void QueryExecution::handleCopyOut(size_t stepNo,string _data) {
         data->appendRaw((void*)_data.c_str(),_data.size());
    }

    void QueryExecution::handleTuples(size_t step, vector<pair<string, uint32_t> >& columns) {
        vector<pair<string, uint32_t>> cleaned;
        for (size_t idx = 0; idx < columns.size(); idx++) {
            auto column = columns[idx];
            if (column.first == "?column?") {
                LOG4CPLUS_DEBUG(LOG,"column " << idx << "has unknown name/type. assume type TEXT");
                cleaned.push_back(pair<string,uint32_t>(string("argument_") + to_string(idx),TEXT));
            } else {
                cleaned.push_back(column);
            }
        }
        data = new CsvTableData(cleaned);
    }

    TableData * QueryExecution::getResult() {
        return data;
    }

    void QueryExecution::setResult(TableData *data) {
        LOG4CPLUS_DEBUG(LOG,"set result to " << data);
        this->data = data;
    }

    bool QueryExecution::isComplete() {
        bool complete = true;
        for (auto& dep:dependencies) {
            complete &= (dep.second!=nullptr);
        }
        return complete;
    }

    void QueryExecution::doTransitions() {
        if (!transitions.empty()) {
            LOG4CPLUS_DEBUG(LOG, "do transitions for query " << sql);
        }
        for (Transition *t:transitions) {
            t->doTransition();
        }
    }

    std::ostream& operator<<(std::ostream& cout,const QueryExecution& qe) {
        cout << "QueryExecution[sql=" << qe.sql << "]";
        return cout;
    }

}

