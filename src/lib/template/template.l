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


%x stmt

%%

    int stmt_level = 0;


"{%" { cout << "[BLOCK_START]"; BEGIN(stmt); return BLOCK_START; }


<stmt>{

    "%}" {
        BEGIN(INITIAL);
        cout << "[BLOCK_END]";
        return BLOCK_END;
    }

    "in" { 
       cout << "[IN]"; return IN; 
    }
    
    "for" { 
       cout << "[FOR]"; return FOR; 
    }

    "endfor" { 
       cout << "[ENDFOR]"; return ENDFOR; 
    }
    
    
    [a-z]+ { 
       cout << "[VAR]" << yytext; yylval.s = strdup(yytext); return VAR; 
    }

}

[^{%]+  {
    yylval.s = strdup(yytext);
    cout << "[RAW_TEXT=" << yylval.s << "]";
    return RAW_TEXT;
}



%%

