/*
 * Template.h
 *
 *  Created on: Mar 12, 2014
 *      Author: arnd
 */

#ifndef TEMPLATE_H_
#define TEMPLATE_H_

#include <string>
#include <map>

#include "utils/RegExp.h"

namespace db_agg {
class Template {
private:
    RegExp regexp;
    std::map<std::string,std::string> variables;
    std::string startDelimiter;
    std::string endDelimiter;
    std::string escapeDelimiter(std::string);
public:
    Template();
    Template(std::string startDelimiter, std::string endDelimiter);
    void set(std::map<std::string,std::string> variables);
    void set(std::string name, std::string value);
    std::string render(std::string tmpl);
};
}



#endif /* TEMPLATE_H_ */
