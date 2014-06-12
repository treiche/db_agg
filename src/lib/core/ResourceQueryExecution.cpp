/*
 * ResourceQueryExecution.cpp
 *
 *  Created on: Jun 11, 2014
 *      Author: arnd
 */

#include "ResourceQueryExecution.h"
#include <log4cplus/logger.h>

#include "table/TableDataFactory.h"
#include "excel/ExcelToTextFormat.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {

static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("ResourceQueryExecution"));

/*
ResourceQueryExecution::ResourceQueryExecution(string name, string id, shared_ptr<Url> url, string sql, vector<string> depName, DependencyInjector *dependencyInjector):
        QueryExecution(name,id,url,sql,depName,dependencyInjector) {
}
*/


void ResourceQueryExecution::stop() {
    LOG4CPLUS_DEBUG(LOG,"stop");
}

void ResourceQueryExecution::schedule() {
    LOG4CPLUS_DEBUG(LOG,"schedule");
    setScheduled();
}

bool ResourceQueryExecution::process() {
    LOG4CPLUS_DEBUG(LOG,"load resource from " << getUrl()->getPath() << " extension = " << getUrl()->getExtension());
    ExecutionStateChangeEvent ec{getId(),"CONNECTED"};
    fireEvent(ec);
    shared_ptr<TableData> data;
    if (getUrl()->getExtension() == "xlsx") {
        ExcelToTextFormat ett;
        map<string,shared_ptr<TableData>> sheets = ett.transform("/"+getUrl()->getPath());
        data = sheets[getUrl()->getFragment()];
    } else {
        data = TableDataFactory::getInstance().load("/" + getUrl()->getPath());
    }
    setResult(data);
    Event event{EventType::PROCESSED,getId()};
    fireEvent(event);
    ReceiveDataEvent rde{getId(),data->getRowCount()};
    fireEvent(rde);
    ExecutionStateChangeEvent e{getId(),"DONE"};
    EventProducer::fireEvent(e);
    setDone();
    return true;
}

void ResourceQueryExecution::cleanUp() {
    LOG4CPLUS_DEBUG(LOG,"cleanUp");
}

bool ResourceQueryExecution::isResourceAvailable() {
    return true;
}
}





