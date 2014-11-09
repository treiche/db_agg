/*
 * Template2.cpp
 *
 *  Created on: Nov 8, 2014
 *      Author: arnd
 */

#include "Template2.h"
#include "ASTNode.h"

#include <iostream>

using namespace std;

db_agg::ASTNode *parse(string tmpl);

namespace db_agg {

void printASTNode(ASTNode *node, int level) {
	cout << string(level*2,' ') << node->getType() << ":" << node->getValue() << endl;
	for (auto& child:node->getChilds()) {
		printASTNode(child,level+1);
	}
}


Template2::Template2() {
	string tmpl = "raw text at start {% for k in nn %} middle {% endfor %} text at end";
	//string tmpl = "blabla ablblba";
	ASTNode *root = parse(tmpl);
	cout << endl << "AST:" << endl;
	printASTNode(root,0);
}
}


