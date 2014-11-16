
%namespace db_agg


%x stmt expr

%%

    // int stmt_level = 0;
    // using namespace std;


"{%" { cout << "[BLOCK_START]"; begin(StartCondition__::stmt); return ParserBase::BLOCK_START; }


"{{" { cout << "[VAR_START]"; begin(StartCondition__::expr); return ParserBase::VAR_START; }


<expr>{

    "}}" { begin(StartCondition__::INITIAL); cout << "[VAR_END]"; return ParserBase::VAR_END; }
    
    [a-z\.]+ { 
       cout << "[VAR]" << matched(); return ParserBase::VAR; 
    }
}


<stmt>{

    "%}" {
        begin(StartCondition__::INITIAL);
        cout << "[BLOCK_END]";
        return ParserBase::BLOCK_END;
    }

    "in" { 
       cout << "[IN]"; return ParserBase::IN; 
    }
    
    "endfor" { 
       cout << "[ENDFOR]"; return ParserBase::ENDFOR; 
    }

    "for" { 
       cout << "[FOR]"; return ParserBase::FOR; 
    }

    [a-z\.]+ { 
       cout << "[VAR]" << matched(); return ParserBase::VAR; 
    }

}

[^{%]+  {
    // yylval.s = strdup(yytext);
    cout << "[RAW_TEXT=" << matched() << "]";
    return ParserBase::RAW_TEXT;
}


