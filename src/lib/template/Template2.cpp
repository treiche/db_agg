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
	if (node->getType() == "text" || node->getType() == "var" || node->getType() == "loop_expr") {
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
}

string Template2::translateVar(string varName, map<string,string>& context) {
	string translated = varName;
	auto idx = varName.find(".");
	if (idx != string::npos) {
		string firstItem = varName.substr(0,idx);
		if (context.find(firstItem) != context.end()) {
			translated = context[firstItem] + "." + varName.substr(idx+1);
		}
	} else {
		if (context.find(varName) != context.end()) {
			translated = context[varName];
		}
	}
	return translated;
}

void Template2::render(ASTNode *node, stringstream& output, map<string,string>& context) {
	if (node->getType() == "root" || node->getType() == "template") {
		for (auto& child:node->getChilds()) {
			render(child,output,context);
		}
	} else if (node->getType() == "text") {
		output << node->getValue();
	} else if (node->getType() == "subst") {
		string varName = node->getChild("var")->getValue();
		auto idx = varName.find(".");
		output << var.get("root." + translateVar(varName,context));
	} else if (node->getType() == "for") {
		string placeholder = node->getChild("var")->getValue();
		string loopExpr = node->getChild("loop_expr")->getValue();
		string translated = translateVar(loopExpr,context);
		ASTNode *subTemplate = node->getChild("template");
		size_t len = var.size("root." + translated);
		cout << "len = " << len << endl;
		for (size_t idx = 0; idx < len; idx++) {
			context[placeholder] = translated + "." + to_string(idx);
			render(subTemplate,output,context);
		}
	}
}

string Template2::render(std::string tmpl) {
	ASTNode *root = parse(tmpl);
	cout << endl << "AST:" << endl;
	printASTNode(root,0);
	stringstream ss;
	map<string,string> context;
	render(root,ss,context);
	return ss.str();
}

}


