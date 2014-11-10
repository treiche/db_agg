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

namespace db_agg {
class Template2 {
private:
    Var var{"root",std::map<std::string,any>()};
    void printASTNode(ASTNode *node, int level);
public:
    Template2();
    void set(std::string name, std::string value);
    std::string render(std::string tmpl);
};
}



#endif /* TEMPLATE2_H_ */
