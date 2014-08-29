/*
 * XmlQueryParser.h
 *
 *  Created on: Aug 28, 2014
 *      Author: arnd
 */

#ifndef XMLQUERYPARSER_H_
#define XMLQUERYPARSER_H_

#include "QueryParser.h"
#include <map>
#include <string>

extern "C" {
    #include <libxml/tree.h>
    #include <libxml/xinclude.h>
}

namespace db_agg {
class XmlQueryParser: public QueryParser {
private:
    Query *parseQuery(xmlElementPtr executionNode);
    std::map<std::string,std::string> getProperties(xmlElementPtr element);
public:
    ~XmlQueryParser();
    virtual std::vector<Query*> parse(
            std::string query,
            std::map<std::string,std::string>& externalSources,
            std::map<std::string,std::string>& queryParameter,
            std::vector<std::string> functions) override;
};
}



#endif /* XMLQUERYPARSER_H_ */
