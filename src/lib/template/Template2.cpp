/*
 * Template2.cpp
 *
 *  Created on: Nov 8, 2014
 *      Author: arnd
 */

#include "Template2.h"

#include <iostream>

using namespace std;

db_agg::ASTNode *parse(string tmpl);

namespace db_agg {

void Template2::printASTNode(ASTNode *node, int level) {
	cout << string(level*2,' ') << node->getType();
	if (node->getType() == "text" || node->getType() == "var") {
		 cout << ": '" << node->getValue() << "'";
	}
	cout  << endl;
	for (auto& child:node->getChilds()) {
		printASTNode(child,level+1);
	}
}


Template2::Template2() {
}


void Template2::set(string name, string value) {
	var.set(var.getName() + "." + name, value);
	//var.set("root.xx",12);
}

string Template2::render(std::string tmpl) {
	ASTNode *root = parse(tmpl);
	cout << endl << "AST:" << endl;
	printASTNode(root,0);
	return "";
}

}


