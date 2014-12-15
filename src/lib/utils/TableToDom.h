/*
 * TableToDom.h
 *
 *  Created on: Dec 8, 2014
 *      Author: arnd
 */

#ifndef TABLETODOM_H_
#define TABLETODOM_H_

#include <string>
#include <memory>

extern "C" {
    #include <libxml/tree.h>
    #include <libxslt/documents.h>
    #include <libxslt/transform.h>
    #include <libxslt/xsltutils.h>
}

#include "table/TableData.h"

namespace db_agg {
class TableToDom {
private:
    xsltStylesheetPtr xslDoc;
public:
    void setStylesheet(xmlNodePtr stylesheetFragment);
    xmlNodePtr transform(std::shared_ptr<TableData> table);
};
}




#endif /* TABLETODOM_H_ */
