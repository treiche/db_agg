/*
 * DomToTable.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: arnd
 */

#include "utils/logging.h"
#include "table/TableDataFactory.h"
#include "DomToTable.h"
#include "utils/xml.h"


using namespace std;

namespace db_agg {

DECLARE_LOGGER("DomToTable")

static string getAttribute(xmlElementPtr element, string attrName) {
    xmlAttribute *attr = element->attributes;
    while(attrName.compare((const char *)attr->name) != 0) {
        attr = (xmlAttribute*)attr->next;
    }
    return string((const char *)attr->children->content);
}


void DomToTable::setStylesheet(xmlNodePtr stylesheetFragment) {
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


shared_ptr<TableData> DomToTable::transform(string source) {
    shared_ptr<TableData> tableData;
    xmlDocPtr doc = xmlReadMemory(source.c_str(), source.size(), nullptr, NULL, 0);
    const char **params = nullptr;
    xmlDocPtr res = xsltApplyStylesheet(xslDoc, doc, params);
    vector<string> header;
    vector<string> row;
    recurse((xmlNodePtr)res,tableData,header,row);
    return tableData;
}

void DomToTable::recurse(xmlNodePtr node, std::shared_ptr<TableData>& tableData, vector<string>& header, vector<string>& row) {
    LOG_DEBUG("recurse " << node->name);
    if (node->type == XML_ELEMENT_NODE) {
        xmlElementPtr element = (xmlElementPtr)node;
        string elementName((char *)element->name);
        if (elementName == "header") {
            string name = getAttribute(element,"name");
            string type = getAttribute(element,"type");
            header.push_back(name + ":" + type);
        }
        if (elementName == "row" && !tableData) {
            tableData = TableDataFactory::getInstance().create("text",header);
        }
        if (elementName == "value") {
            string content = (char*)node->children->content;
            LOG_DEBUG("content = " << content);
            row.push_back(content);
        }
    }
    xmlNodePtr tmp = node->children;
    while(tmp) {
        recurse(tmp,tableData,header,row);
        tmp = tmp->next;
    }
    if (node->type == XML_ELEMENT_NODE) {
        xmlElementPtr element = (xmlElementPtr)node;
        string elementName((char *)element->name);
        if (elementName == "row" && !row.empty()) {
            tableData->addRow(row);
            row.clear();
        }
    }
}


}


