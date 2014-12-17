/*
 * ResourceQueryExecution.cpp
 *
 *  Created on: Jun 11, 2014
 *      Author: arnd
 */

#include "ResourceQueryExecution.h"
#include "utils/logging.h"

#include "table/TableDataFactory.h"
#include "excel/ExcelToTextFormat.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {

DECLARE_LOGGER("ResourceQueryExecution");

/*
ResourceQueryExecution::ResourceQueryExecution(string name, string id, shared_ptr<Url> url, string sql, vector<string> depName, DependencyInjector *dependencyInjector):
        QueryExecution(name,id,url,sql,depName,dependencyInjector) {
}
*/


void ResourceQueryExecution::stop() {
    LOG_DEBUG("stop");
}

void ResourceQueryExecution::schedule() {
    LOG_DEBUG("schedule");
    setState(QueryExecutionState::SCHEDULED);
}

bool ResourceQueryExecution::process() {
    LOG_DEBUG("load resource from " << getUrl()->getPath() << " extension = " << getUrl()->getExtension());
    shared_ptr<Event> ec(new ExecutionStateChangeEvent(getId(),"CONNECTED"));
    fireEvent(ec);
    shared_ptr<TableData> data;
    if (getUrl()->getExtension() == "xlsx") {
        ExcelToTextFormat ett;
        map<string,shared_ptr<TableData>> sheets = ett.transform("/"+getUrl()->getPath());
        data = sheets[getUrl()->getFragment()];
    } else {
        data = TableDataFactory::getInstance().load("/" + getUrl()->getPath());
    }
    setResult("", data);
    setState(QueryExecutionState::DONE);
    return true;
}

void ResourceQueryExecution::cleanUp() {
    LOG_DEBUG("cleanUp");
}

bool ResourceQueryExecution::isResourceAvailable() {
    return true;
}
}





