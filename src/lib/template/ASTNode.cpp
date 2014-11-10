/*
 * ASTNode.cpp
 *
 *  Created on: Nov 8, 2014
 *      Author: arnd
 */

#include "ASTNode.h"

using namespace std;

namespace db_agg {

ASTNode::ASTNode(string type):
	type(type) {}

ASTNode::ASTNode(string type, string value):
	type(type),
	value(value) {}

void ASTNode::addChild(ASTNode *child) {
	childs.push_front(child);
}

void ASTNode::appendChild(ASTNode *child) {
	childs.push_back(child);
}


std::list<ASTNode*>& ASTNode::getChilds() {
	return childs;
}

string ASTNode::getType() {
	return type;
}

string ASTNode::getValue() {
	return value;
}


}


