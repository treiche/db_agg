/*
 * DomToTable.h
 *
 *  Created on: Dec 7, 2014
 *      Author: arnd
 */

#ifndef DOMTOTABLE_H_
#define DOMTOTABLE_H_

#include <string>
#include <memory>
#include "table/TableData.h"

extern "C" {
    #include <libxslt/documents.h>
    #include <libxslt/transform.h>
    #include <libxslt/xsltutils.h>
}


namespace db_agg {
class DomToTable {
private:
    xsltStylesheetPtr xslDoc;
    void recurse(xmlNodePtr node, std::shared_ptr<TableData>& tableData, std::vector<std::string>& header, std::vector<std::string>& row);
public:
    void setStylesheet(xmlNodePtr stylesheetFragment);
    std::shared_ptr<TableData> transform(std::string source);
};
}



#endif /* DOMTOTABLE_H_ */
