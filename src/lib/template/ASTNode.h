/*
 * ASTNode.h
 *
 *  Created on: Nov 8, 2014
 *      Author: arnd
 */

#ifndef ASTNODE_H_
#define ASTNODE_H_

#include <string>
#include <list>

namespace db_agg {
class ASTNode {
private:
	std::string type;
	std::string value;
	std::list<ASTNode*> childs;
public:
	ASTNode(std::string type);
	ASTNode(std::string type, std::string value);
	std::string getType();
	std::string getValue();
	void prependChild(ASTNode *child);
	void appendChild(ASTNode *child);
	std::list<ASTNode*>& getChilds();
	ASTNode *getChild(std::string name);
};


}



#endif /* ASTNODE_H_ */
