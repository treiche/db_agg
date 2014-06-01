/*
 * Template.cpp
 *
 *  Created on: Mar 12, 2014
 *      Author: arnd
 */


#include "utils/Template.h"

#include <log4cplus/logger.h>

#include <vector>

using namespace std;
using namespace log4cplus;

namespace db_agg {
static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("Template"));

Template::Template(): Template("{","}") {}

Template::Template(string startDelimiter, string endDelimiter) {
    this->startDelimiter = startDelimiter;
    this->endDelimiter = endDelimiter;
    string re = escapeDelimiter(startDelimiter) + "([a-zA-Z0-9_]+)" + escapeDelimiter(endDelimiter);
    LOG4CPLUS_DEBUG(LOG, "regex = " << re);
    regexp.setExpr(re);
}

string Template::escapeDelimiter(string delimiter) {
    string escaped = "";
    for (const char c:delimiter) {
        escaped.append("\\");
        escaped += c;
    }
    return escaped;
}


void Template::set(std::string name, std::string value) {
    LOG4CPLUS_TRACE(LOG, "set var '" << name << "' to '" << value << "'");
    variables[name] = value;
}

void Template::set(map<string,string> variables) {
    for (auto& entry:variables) {
        this->variables[entry.first] = entry.second;
    }
}

std::string Template::render(std::string tmpl) {
    string result;
    int offset = 0;
    int lastOffset = 0;
    vector<string> matches;
    while(regexp.find(tmpl,matches,offset)) {
        LOG4CPLUS_DEBUG(LOG, "found " << matches[1] << " offset = " << offset);
        if (variables.find(matches[1]) == variables.end()) {
            string message = "unresolved variable '" + matches[1] + "'";
            LOG4CPLUS_ERROR(LOG, message);
            throw runtime_error(message);
        }
        result += tmpl.substr(lastOffset,offset-lastOffset-matches[1].size()-startDelimiter.size()) + variables[matches[1]];
        lastOffset = offset + endDelimiter.size();
    }
    if (lastOffset < tmpl.size()) {
        result += tmpl.substr(lastOffset);
    }
    return result;
}


}
