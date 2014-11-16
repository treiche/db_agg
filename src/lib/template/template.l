
%namespace db_agg


%x stmt expr

%%

    // int stmt_level = 0;
    // using namespace std;


"{%" { begin(StartCondition__::stmt); return ParserBase::BLOCK_START; }


"{{" { begin(StartCondition__::expr); return ParserBase::VAR_START; }


<expr>{

    "}}" { begin(StartCondition__::INITIAL); return ParserBase::VAR_END; }
    
    [a-z\.]+ { 
       return ParserBase::VAR; 
    }
}


<stmt>{

    "%}" {
        begin(StartCondition__::INITIAL);
        return ParserBase::BLOCK_END;
    }

    "in" { 
       return ParserBase::IN; 
    }
    
    "endfor" { 
       return ParserBase::ENDFOR; 
    }

    "for" { 
       return ParserBase::FOR; 
    }

    [a-z\.]+ { 
       return ParserBase::VAR; 
    }

}

[^{%]+  {
    return ParserBase::RAW_TEXT;
}

. {
    return ParserBase::RAW_TEXT;
}
