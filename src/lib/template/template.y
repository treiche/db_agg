%{
    #include <iostream>
    #include <string>
    #include <typeinfo>
    
    #include <stdio.h>
    
    using namespace std;

    #include "ASTNode.h"
    using namespace db_agg;

    extern "C" {
          int yylex();
          int yyparse();
          void yyerror(char *s);
    }
          struct yy_buffer_state;
          typedef size_t yy_size_t;
          yy_buffer_state *yy_scan_string(const char * base);
          void yy_delete_buffer(yy_buffer_state*);

    #include "ASTNode.h"
    
    static ASTNode *rootNode = new ASTNode("root");
    
%}


%token BLOCK_START
%token BLOCK_END
%token FOR
%token <s> VAR
%token IN
%token ENDFOR
%token <s> RAW_TEXT

%type <node> expr block

%union {
    char *s;
    struct ASTNode *node;
}

%%

unit: /* no action */
    | RAW_TEXT unit { rootNode->addChild(new ASTNode("text",$1)); }
    | block unit { rootNode->addChild($1); }

block: BLOCK_START expr BLOCK_END { 
           $$=new ASTNode("block"); 
           $$->addChild($2);
       }

expr: FOR VAR IN VAR { 
          $$=new ASTNode("for"); 
          $$->addChild(new ASTNode("var",$2)); 
          $$->addChild(new ASTNode("var",$4)); 
      }
    | ENDFOR { 
          $$=new ASTNode("endfor");
      }

%%

ASTNode *parse(string tmpl) {
  yy_buffer_state *buffer = yy_scan_string(tmpl.c_str());
  yyparse();
  yy_delete_buffer(buffer);
  return rootNode;
}


int main(int argc, char **argv) {
  ASTNode *root = parse("jhkjhs {% for k in nn %} s");
  cout << "childs = " << root->getChilds().size() << endl;
  return 0;
}

void yyerror(char *s) {
    cerr << s << endl;
}