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
          void yyerror(const char *s);
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
%token VAR_START
%token VAR_END
%token FOR
%token <s> VAR
%token IN
%token ENDFOR
%token <s> RAW_TEXT


%start unit


%union {
    char *s;
    struct ASTNode *node;
}

%type <node> for_block template_list template block subst_expr

%%

unit:
    | template unit { rootNode->prependChild($1); }

template: block { $$=$1; }
    | RAW_TEXT  { $$ = new ASTNode("text",$1); }
    | subst_expr { $$ = new ASTNode("subst"); $$->prependChild($1); }


subst_expr: VAR_START VAR VAR_END { $$=new ASTNode("var",$2); }

template_list: template {
        $$ = new ASTNode("template");
        $$->prependChild($1);
    }
    | template_list template { 
        $$ = $1;
        $$->appendChild($2);
    }

block: for_block { $$=$1; }

for_block:  BLOCK_START FOR VAR IN VAR BLOCK_END template_list BLOCK_START ENDFOR BLOCK_END {
    $$ = new ASTNode("for");
    $$->prependChild($7);
    $$->prependChild(new ASTNode("var",$3));
    $$->prependChild(new ASTNode("loop_expr",$5));
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

void yyerror(const char *s) {
    cerr << endl << s << endl;
}