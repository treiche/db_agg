/*
 * Template2.h
 *
 *  Created on: Nov 8, 2014
 *      Author: arnd
 */

#ifndef TEMPLATE2_H_
#define TEMPLATE2_H_

#include "Var.h"
#include "ASTNode.h"
#include <sstream>

namespace db_agg {
class Template2 {
private:
    Var var{"root",std::map<std::string,any>()};
    void printASTNode(ASTNode *node, int level);
    void render(ASTNode *node, std::stringstream& output, std::map<std::string,std::string>& context);
    std::string translateVar(std::string varName, std::map<std::string,std::string>& context);
public:
    Template2();
    void set(std::string name, std::string value);
    std::string render(std::string tmpl);
    Var& getVar() {
    	return var;
    }
};
}



#endif /* TEMPLATE2_H_ */
