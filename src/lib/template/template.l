%{
    #include <iostream>
    
    extern "C" {
        #include "template.tab.h"
    }
    
    using namespace std;

    extern "C" {
          int yylex();
          void yyerror(char *s);
          int yyparse();
    }
%}


%x stmt expr

%%

    // int stmt_level = 0;


"{%" { cout << "[BLOCK_START]"; BEGIN(stmt); return BLOCK_START; }


"{{" { cout << "[VAR_START]"; BEGIN(expr); return VAR_START; }


<expr>{

    "}}" { BEGIN(INITIAL); cout << "[VAR_END]"; return VAR_END; }
    
    [a-z\.]+ { 
       cout << "[VAR]" << yytext; yylval.s = strdup(yytext); return VAR; 
    }
}


<stmt>{

    "%}" {
        BEGIN(INITIAL);
        cout << "[BLOCK_END]";
        return BLOCK_END;
    }

    "in" { 
       cout << "[IN]"; return IN; 
    }
    
    "endfor" { 
       cout << "[ENDFOR]"; return ENDFOR; 
    }

    "for" { 
       cout << "[FOR]"; return FOR; 
    }

    [a-z\.]+ { 
       cout << "[VAR]" << yytext; yylval.s = strdup(yytext); return VAR; 
    }

}

[^{%]+  {
    yylval.s = strdup(yytext);
    cout << "[RAW_TEXT=" << yylval.s << "]";
    return RAW_TEXT;
}



%%

