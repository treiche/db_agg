
%scanner Scanner.h
%scanner-token-function d_scanner.lex()

%namespace db_agg

%token BLOCK_START
%token BLOCK_END
%token VAR_START
%token VAR_END
%token FOR
%token VAR
%token IN
%token ENDFOR
%token RAW_TEXT

%type <TEXT> var raw_text
%type <NODE> unit template template_list block subst_expr for_block

%polymorphic NODE: ASTNode* ; TEXT: std::string;

%start unit

/*
%union {
    const char *s;
    struct ASTNode *node;
}
*/


%%

unit: { $$=rootNode; }
    | template unit { rootNode->prependChild($1); }
;

template: block { $$ = $1; }
    | raw_text  { $$ = new ASTNode("text",$1); }
    | subst_expr { $$ = new ASTNode("subst", $1); }
;


subst_expr: VAR_START var VAR_END { cout << endl << "$2 = " << $2 << endl;  $$ = new ASTNode("var",$2); };

template_list: template {
        $$ = new ASTNode("template", $1);
        // $$->prependChild($1);
    }
    | template_list template { 
        $$ = $1;
        $$->appendChild($2);
    }
;

block: for_block { $$=$1; };

for_block:  BLOCK_START FOR var IN var BLOCK_END template_list BLOCK_START ENDFOR BLOCK_END {
    $$ = new ASTNode("for");
    $$->prependChild($7);
    $$->prependChild(new ASTNode("var",$3)); 
    $$->prependChild(new ASTNode("loop_expr",$5));
};


var:
    VAR {
        cout << endl << "match var " << d_scanner.matched().c_str() << endl;
        // $$ = strdup(d_scanner.matched().c_str());
        $$ = d_scanner.matched();
    };

raw_text:
    RAW_TEXT {
        $$ = d_scanner.matched();
    };
