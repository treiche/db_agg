// Generated by Bisonc++ V4.05.00 on Sat, 15 Nov 2014 15:53:52 +0100

#ifndef db_aggParser_h_included
#define db_aggParser_h_included

// $insert baseclass

#include "ASTNode.h"
#include "Parserbase.h"
// $insert scanner.h
#include "Scanner.h"
#include <sstream>

// $insert namespace-open
namespace db_agg
{

#undef Parser
class Parser: public ParserBase
{
    // $insert scannerobject
    ASTNode *rootNode = new ASTNode("root");
    Scanner d_scanner;
        
    public:
    /*
    	Parser(std::istream& in, std::ostream& out):
    		d_scanner(in,out) {}
*/
    	int parse();
    	
    	int parse(std::string text) {
    		std::stringstream ss(text);
    		d_scanner.switchStreams(ss,std::cout);
    		return parse();
    	}
    	
        ASTNode *getAST() {
        	return rootNode;
        }

    private:
        void error(char const *msg);    // called on (syntax) errors
        int lex();                      // returns the next token from the
                                        // lexical scanner. 
        void print();                   // use, e.g., d_token, d_loc

    // support functions for parse():
        void executeAction(int ruleNr);
        void errorRecovery();
        int lookup(bool recovery);
        void nextToken();
        void print__();
        void exceptionHandler__(std::exception const &exc);
};

// $insert namespace-close
}

#endif
