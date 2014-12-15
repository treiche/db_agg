/*
 * TableToDom.cpp
 *
 *  Created on: Dec 8, 2014
 *      Author: arnd
 */

#include "TableToDom.h"
#include <iostream>
#include "utils/xml.h"
#include "utils/logging.h"

using namespace std;

namespace db_agg {

DECLARE_LOGGER("TableToDom");

void TableToDom::setStylesheet(xmlNodePtr stylesheetFragment) {
    LOG_DEBUG("TableToDom " << as_string(stylesheetFragment));
    xmlDocPtr doc = xmlNewDoc((xmlChar*)"1.0");
    xmlNodePtr stylesheet = xmlNewDocNode(doc,nullptr,(xmlChar*)"stylesheet",(xmlChar*)nullptr);
    xmlDocSetRootElement(doc,stylesheet);
    xmlNsPtr ns = xmlNewNs(stylesheet,(xmlChar*)XSLT_NAMESPACE,(xmlChar*)"xsl");
    xmlSetNs(stylesheet,ns);
    xmlSetNsProp(stylesheet,nullptr,(xmlChar*)"version",(xmlChar*)"1.1");
    xmlNodePtr tmpl = xmlNewChild(stylesheet,ns,(xmlChar*)"template",(xmlChar*)nullptr);
    xmlSetNsProp(tmpl,nullptr,(xmlChar*)"match",(xmlChar*)"/");

    xmlAddChild(tmpl,stylesheetFragment);


    LOG_DEBUG("parse stylesheet " << as_string((xmlNodePtr)doc));
    xslDoc = xsltParseStylesheetDoc(doc);
    LOG_DEBUG("constr done");
}

xmlNodePtr TableToDom::transform(std::shared_ptr<TableData> table) {
    LOG_DEBUG("transform " << table);
    xmlDocPtr doc = xmlNewDoc((xmlChar*)"1.0");
    xmlNodePtr tab = xmlNewNode(nullptr,(xmlChar*)"table");
    xmlDocSetRootElement(doc,tab);

    auto cols = table->getColumns();
    for (auto col:cols) {
        xmlNodePtr header = xmlNewNode(nullptr,(xmlChar*)"header");
        xmlSetProp(header,(xmlChar*)"name",(xmlChar*)col.first.c_str());
        xmlSetProp(header,(xmlChar*)"type",(xmlChar*)to_string(col.second).c_str());
        xmlAddChild(tab,header);
    }

    for (uint64_t row = 0; row < table->getRowCount(); row++) {
        xmlNodePtr rowElement = xmlNewNode(nullptr,(xmlChar*)"row");
        xmlAddChild(tab,rowElement);
        for (uint32_t col = 0; col < table->getColCount(); col++) {
            xmlNodePtr valueElement = xmlNewNode(nullptr,(xmlChar*)"value");
            xmlAddChild(rowElement,valueElement);
            string value = table->getValue(row,col);
            xmlNodePtr ve = xmlNewText((xmlChar*)value.c_str());
            xmlAddChild(valueElement,ve);
        }
    }

    xmlBufferPtr buf = xmlBufferCreate();
    xmlNodeDump(buf,tab->doc,tab,0,1);
        LOG_DEBUG("content = " << buf->content);
    xmlBufferFree(buf);

    const char **params = nullptr;
    xmlDocPtr res = xsltApplyStylesheet(xslDoc,doc,params);

    return (xmlNodePtr)res;
}


}


