/*
 * Template2.h
 *
 *  Created on: Nov 8, 2014
 *      Author: arnd
 */

#ifndef TEMPLATE2_H_
#define TEMPLATE2_H_

#include "JsonVar.h"
#include "ASTNode.h"
#include <sstream>
#include <map>

namespace db_agg {
class Template2 {
private:
    JsonVar var{"root",std::map<std::string,any>()};
    void printASTNode(ASTNode *node, int level);
    void render(ASTNode *node, std::stringstream& output, std::map<std::string,std::string>& context);
    std::string translateVar(std::string varName, std::map<std::string,std::string>& context);
public:
    Template2();
    void set(std::string name, std::string value);
    std::string render(std::string tmpl);
    JsonVar& getVar() {
    	return var;
    }
};
}



#endif /* TEMPLATE2_H_ */
