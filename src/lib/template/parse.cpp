// Generated by Bisonc++ V4.05.00 on Sun, 16 Nov 2014 11:22:15 +0100

// $insert class.ih
#include "Parser.ih"

// The FIRST element of SR arrays shown below uses `d_type', defining the
// state's type, and `d_lastIdx' containing the last element's index. If
// d_lastIdx contains the REQ_TOKEN bitflag (see below) then the state needs
// a token: if in this state d_token__ is _UNDETERMINED_, nextToken() will be
// called

// The LAST element of SR arrays uses `d_token' containing the last retrieved
// token to speed up the (linear) seach.  Except for the first element of SR
// arrays, the field `d_action' is used to determine what to do next. If
// positive, it represents the next state (used with SHIFT); if zero, it
// indicates `ACCEPT', if negative, -d_action represents the number of the
// rule to reduce to.

// `lookup()' tries to find d_token__ in the current SR array. If it fails, and
// there is no default reduction UNEXPECTED_TOKEN__ is thrown, which is then
// caught by the error-recovery function.

// The error-recovery function will pop elements off the stack until a state
// having bit flag ERR_ITEM is found. This state has a transition on _error_
// which is applied. In this _error_ state, while the current token is not a
// proper continuation, new tokens are obtained by nextToken(). If such a
// token is found, error recovery is successful and the token is
// handled according to the error state's SR table and parsing continues.
// During error recovery semantic actions are ignored.

// A state flagged with the DEF_RED flag will perform a default
// reduction if no other continuations are available for the current token.

// The ACCEPT STATE never shows a default reduction: when it is reached the
// parser returns ACCEPT(). During the grammar
// analysis phase a default reduction may have been defined, but it is
// removed during the state-definition phase.

// So:
//      s_x[] = 
//      {
//                  [_field_1_]         [_field_2_]
//
// First element:   {state-type,        idx of last element},
// Other elements:  {required token,    action to perform},
//                                      ( < 0: reduce, 
//                                          0: ACCEPT,
//                                        > 0: next state)
// Last element:    {set to d_token__,    action to perform}
//      }

// When the --thread-safe option is specified, all static data are defined as
// const. If --thread-safe is not provided, the state-tables are not defined
// as const, since the lookup() function below will modify them


namespace // anonymous
{
    char const author[] = "Frank B. Brokken (f.b.brokken@rug.nl)";

    enum 
    {
        STACK_EXPANSION = 5     // size to expand the state-stack with when
                                // full
    };

    enum ReservedTokens
    {
        PARSE_ACCEPT     = 0,   // `ACCEPT' TRANSITION
        _UNDETERMINED_   = -2,
        _EOF_            = -1,
        _error_          = 256
    };
    enum StateType       // modify statetype/data.cc when this enum changes
    {
        NORMAL,
        ERR_ITEM,
        REQ_TOKEN,
        ERR_REQ,    // ERR_ITEM | REQ_TOKEN
        DEF_RED,    // state having default reduction
        ERR_DEF,    // ERR_ITEM | DEF_RED
        REQ_DEF,    // REQ_TOKEN | DEF_RED
        ERR_REQ_DEF // ERR_ITEM | REQ_TOKEN | DEF_RED
    };    
    struct PI__     // Production Info
    {
        size_t d_nonTerm; // identification number of this production's
                            // non-terminal 
        size_t d_size;    // number of elements in this production 
    };

    struct SR__     // Shift Reduce info, see its description above
    {
        union
        {
            int _field_1_;      // initializer, allowing initializations 
                                // of the SR s_[] arrays
            int d_type;
            int d_token;
        };
        union
        {
            int _field_2_;

            int d_lastIdx;          // if negative, the state uses SHIFT
            int d_action;           // may be negative (reduce), 
                                    // postive (shift), or 0 (accept)
            size_t d_errorState;    // used with Error states
        };
    };

    // $insert staticdata
    
// Productions Info Records:
PI__ const s_productionInfo[] = 
{
     {0, 0}, // not used: reduction values are negative
     {274, 0}, // 1: unit ->  <empty>
     {274, 2}, // 2: unit ->  template unit
     {275, 1}, // 3: template ->  block
     {275, 1}, // 4: template ->  raw_text
     {275, 1}, // 5: template ->  subst_expr
     {276, 3}, // 6: subst_expr (VAR_START) ->  VAR_START var VAR_END
     {277, 1}, // 7: template_list ->  template
     {277, 2}, // 8: template_list ->  template_list template
     {278, 1}, // 9: block ->  for_block
     {279, 10}, // 10: for_block (BLOCK_START) ->  BLOCK_START FOR var IN var BLOCK_END template_list BLOCK_START ENDFOR BLOCK_END
     {280, 1}, // 11: var (VAR) ->  VAR
     {281, 1}, // 12: raw_text (RAW_TEXT) ->  RAW_TEXT
     {282, 1}, // 13: unit_$ ->  unit
};

// State info and SR__ transitions for each state.


SR__ const s_0[] =
{
    { { REQ_DEF}, { 10} },               
    { {     274}, {  1} }, // unit       
    { {     275}, {  2} }, // template   
    { {     278}, {  3} }, // block      
    { {     281}, {  4} }, // raw_text   
    { {     276}, {  5} }, // subst_expr 
    { {     279}, {  6} }, // for_block  
    { {     265}, {  7} }, // RAW_TEXT   
    { {     259}, {  8} }, // VAR_START  
    { {     257}, {  9} }, // BLOCK_START
    { {       0}, { -1} },               
};

SR__ const s_1[] =
{
    { { REQ_TOKEN}, {            2} }, 
    { {     _EOF_}, { PARSE_ACCEPT} }, 
    { {         0}, {            0} }, 
};

SR__ const s_2[] =
{
    { { REQ_DEF}, { 10} },               
    { {     274}, { 10} }, // unit       
    { {     275}, {  2} }, // template   
    { {     278}, {  3} }, // block      
    { {     281}, {  4} }, // raw_text   
    { {     276}, {  5} }, // subst_expr 
    { {     279}, {  6} }, // for_block  
    { {     265}, {  7} }, // RAW_TEXT   
    { {     259}, {  8} }, // VAR_START  
    { {     257}, {  9} }, // BLOCK_START
    { {       0}, { -1} },               
};

SR__ const s_3[] =
{
    { { DEF_RED}, {  1} }, 
    { {       0}, { -3} }, 
};

SR__ const s_4[] =
{
    { { DEF_RED}, {  1} }, 
    { {       0}, { -4} }, 
};

SR__ const s_5[] =
{
    { { DEF_RED}, {  1} }, 
    { {       0}, { -5} }, 
};

SR__ const s_6[] =
{
    { { DEF_RED}, {  1} }, 
    { {       0}, { -9} }, 
};

SR__ const s_7[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -12} }, 
};

SR__ const s_8[] =
{
    { { REQ_TOKEN}, {  3} },       
    { {       280}, { 11} }, // var
    { {       262}, { 12} }, // VAR
    { {         0}, {  0} },       
};

SR__ const s_9[] =
{
    { { REQ_TOKEN}, {  2} },       
    { {       261}, { 13} }, // FOR
    { {         0}, {  0} },       
};

SR__ const s_10[] =
{
    { { DEF_RED}, {  1} }, 
    { {       0}, { -2} }, 
};

SR__ const s_11[] =
{
    { { REQ_TOKEN}, {  2} },           
    { {       260}, { 14} }, // VAR_END
    { {         0}, {  0} },           
};

SR__ const s_12[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -11} }, 
};

SR__ const s_13[] =
{
    { { REQ_TOKEN}, {  3} },       
    { {       280}, { 15} }, // var
    { {       262}, { 12} }, // VAR
    { {         0}, {  0} },       
};

SR__ const s_14[] =
{
    { { DEF_RED}, {  1} }, 
    { {       0}, { -6} }, 
};

SR__ const s_15[] =
{
    { { REQ_TOKEN}, {  2} },      
    { {       263}, { 16} }, // IN
    { {         0}, {  0} },      
};

SR__ const s_16[] =
{
    { { REQ_TOKEN}, {  3} },       
    { {       280}, { 17} }, // var
    { {       262}, { 12} }, // VAR
    { {         0}, {  0} },       
};

SR__ const s_17[] =
{
    { { REQ_TOKEN}, {  2} },             
    { {       258}, { 18} }, // BLOCK_END
    { {         0}, {  0} },             
};

SR__ const s_18[] =
{
    { { REQ_TOKEN}, { 10} },                 
    { {       277}, { 19} }, // template_list
    { {       275}, { 20} }, // template     
    { {       278}, {  3} }, // block        
    { {       281}, {  4} }, // raw_text     
    { {       276}, {  5} }, // subst_expr   
    { {       279}, {  6} }, // for_block    
    { {       265}, {  7} }, // RAW_TEXT     
    { {       259}, {  8} }, // VAR_START    
    { {       257}, {  9} }, // BLOCK_START  
    { {         0}, {  0} },                 
};

SR__ const s_19[] =
{
    { { REQ_TOKEN}, {  9} },               
    { {       257}, { 21} }, // BLOCK_START
    { {       275}, { 22} }, // template   
    { {       278}, {  3} }, // block      
    { {       281}, {  4} }, // raw_text   
    { {       276}, {  5} }, // subst_expr 
    { {       279}, {  6} }, // for_block  
    { {       265}, {  7} }, // RAW_TEXT   
    { {       259}, {  8} }, // VAR_START  
    { {         0}, {  0} },               
};

SR__ const s_20[] =
{
    { { DEF_RED}, {  1} }, 
    { {       0}, { -7} }, 
};

SR__ const s_21[] =
{
    { { REQ_TOKEN}, {  3} },          
    { {       264}, { 23} }, // ENDFOR
    { {       261}, { 13} }, // FOR   
    { {         0}, {  0} },          
};

SR__ const s_22[] =
{
    { { DEF_RED}, {  1} }, 
    { {       0}, { -8} }, 
};

SR__ const s_23[] =
{
    { { REQ_TOKEN}, {  2} },             
    { {       258}, { 24} }, // BLOCK_END
    { {         0}, {  0} },             
};

SR__ const s_24[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -10} }, 
};


// State array:
SR__ const *s_state[] =
{
  s_0,  s_1,  s_2,  s_3,  s_4,  s_5,  s_6,  s_7,  s_8,  s_9,
  s_10,  s_11,  s_12,  s_13,  s_14,  s_15,  s_16,  s_17,  s_18,  s_19,
  s_20,  s_21,  s_22,  s_23,  s_24,
};

} // anonymous namespace ends


// $insert namespace-open
namespace db_agg
{

// If the parsing function call uses arguments, then provide an overloaded
// function.  The code below doesn't rely on parameters, so no arguments are
// required.  Furthermore, parse uses a function try block to allow us to do
// ACCEPT and ABORT from anywhere, even from within members called by actions,
// simply throwing the appropriate exceptions.

ParserBase::ParserBase()
:
    d_stackIdx__(-1),
    // $insert debuginit
    d_debug__(false),
    d_nErrors__(0),
    // $insert requiredtokens
    d_requiredTokens__(0),
    d_acceptedTokens__(d_requiredTokens__),
    d_token__(_UNDETERMINED_),
    d_nextToken__(_UNDETERMINED_)
{}


void Parser::print__()
{
// $insert print
}

void ParserBase::clearin()
{
    d_token__ = d_nextToken__ = _UNDETERMINED_;
}

void ParserBase::push__(size_t state)
{
    if (static_cast<size_t>(d_stackIdx__ + 1) == d_stateStack__.size())
    {
        size_t newSize = d_stackIdx__ + STACK_EXPANSION;
        d_stateStack__.resize(newSize);
        d_valueStack__.resize(newSize);
    }
    ++d_stackIdx__;
    d_stateStack__[d_stackIdx__] = d_state__ = state;
    *(d_vsp__ = &d_valueStack__[d_stackIdx__]) = d_val__;
}

void ParserBase::popToken__()
{
    d_token__ = d_nextToken__;

    d_val__ = d_nextVal__;
    d_nextVal__ = STYPE__();

    d_nextToken__ = _UNDETERMINED_;
}
     
void ParserBase::pushToken__(int token)
{
    d_nextToken__ = d_token__;
    d_nextVal__ = d_val__;
    d_token__ = token;
}
     
void ParserBase::pop__(size_t count)
{
    if (d_stackIdx__ < static_cast<int>(count))
    {
        ABORT();
    }

    d_stackIdx__ -= count;
    d_state__ = d_stateStack__[d_stackIdx__];
    d_vsp__ = &d_valueStack__[d_stackIdx__];
}

inline size_t ParserBase::top__() const
{
    return d_stateStack__[d_stackIdx__];
}

void Parser::executeAction(int production)
try
{
    if (d_token__ != _UNDETERMINED_)
        pushToken__(d_token__);     // save an already available token

                                    // save default non-nested block $$
    if (int size = s_productionInfo[production].d_size)
        d_val__ = d_vsp__[1 - size];

    switch (production)
    {
        // $insert actioncases
        
        case 1:
#line 34 "src/lib/template/template.y"
        { d_val__.get<Tag__::NODE>()=rootNode; }
        break;

        case 2:
#line 35 "src/lib/template/template.y"
        { rootNode->prependChild(d_vsp__[-1].data<Tag__::NODE>()); }
        break;

        case 3:
#line 38 "src/lib/template/template.y"
        { d_val__.get<Tag__::NODE>() = d_vsp__[0].data<Tag__::NODE>(); }
        break;

        case 4:
#line 39 "src/lib/template/template.y"
        { d_val__.get<Tag__::NODE>() = new ASTNode("text",d_vsp__[0].data<Tag__::TEXT>()); }
        break;

        case 5:
#line 40 "src/lib/template/template.y"
        { d_val__.get<Tag__::NODE>() = new ASTNode("subst", d_vsp__[0].data<Tag__::NODE>()); }
        break;

        case 6:
#line 44 "src/lib/template/template.y"
        { d_val__.get<Tag__::NODE>() = new ASTNode("var",d_vsp__[-1].data<Tag__::TEXT>()); }
        break;

        case 7:
#line 46 "src/lib/template/template.y"
        {
         d_val__.get<Tag__::NODE>() = new ASTNode("template", d_vsp__[0].data<Tag__::NODE>());
         }
        break;

        case 8:
#line 49 "src/lib/template/template.y"
        { 
         d_val__.get<Tag__::NODE>() = d_vsp__[-1].data<Tag__::NODE>();
         d_val__.get<Tag__::NODE>()->appendChild(d_vsp__[0].data<Tag__::NODE>());
         }
        break;

        case 9:
#line 55 "src/lib/template/template.y"
        { d_val__.get<Tag__::NODE>()=d_vsp__[0].data<Tag__::NODE>(); }
        break;

        case 10:
#line 57 "src/lib/template/template.y"
        {
         d_val__.get<Tag__::NODE>() = new ASTNode("for");
         d_val__.get<Tag__::NODE>()->prependChild(d_vsp__[-3].data<Tag__::NODE>());
         d_val__.get<Tag__::NODE>()->prependChild(new ASTNode("var",d_vsp__[-7].data<Tag__::TEXT>())); 
         d_val__.get<Tag__::NODE>()->prependChild(new ASTNode("loop_expr",d_vsp__[-5].data<Tag__::TEXT>()));
        }
        break;

        case 11:
#line 66 "src/lib/template/template.y"
        {
         d_val__.get<Tag__::TEXT>() = d_scanner.matched();
         }
        break;

        case 12:
#line 71 "src/lib/template/template.y"
        {
         d_val__.get<Tag__::TEXT>() = d_scanner.matched();
         }
        break;

    }
}
catch (std::exception const &exc)
{
    exceptionHandler__(exc);
}

inline void ParserBase::reduce__(PI__ const &pi)
{
    d_token__ = pi.d_nonTerm;
    pop__(pi.d_size);

}

// If d_token__ is _UNDETERMINED_ then if d_nextToken__ is _UNDETERMINED_ another
// token is obtained from lex(). Then d_nextToken__ is assigned to d_token__.
void Parser::nextToken()
{
    if (d_token__ != _UNDETERMINED_)        // no need for a token: got one
        return;                             // already

    if (d_nextToken__ != _UNDETERMINED_)
    {
        popToken__();                       // consume pending token
    }
    else
    {
        ++d_acceptedTokens__;               // accept another token (see
                                            // errorRecover())
        d_token__ = lex();
        if (d_token__ <= 0)
            d_token__ = _EOF_;
    }
    print();
}

// if the final transition is negative, then we should reduce by the rule
// given by its positive value. Note that the `recovery' parameter is only
// used with the --debug option
int Parser::lookup(bool recovery)
{
    // $insert threading
    SR__ const *sr = s_state[d_state__];  // get the appropriate state-table
    int lastIdx = sr->d_lastIdx;        // sentinel-index in the SR_ array

    SR__ const *lastElementPtr = sr + lastIdx;
    SR__ const *elementPtr = sr + 1;      // start the search at s_xx[1]

    while (elementPtr != lastElementPtr && elementPtr->d_token != d_token__)
        ++elementPtr;

    if (elementPtr == lastElementPtr)   // reached the last element
    {
        if (elementPtr->d_action < 0)   // default reduction
        {
            return elementPtr->d_action;                
        }

        // No default reduction, so token not found, so error.
        throw UNEXPECTED_TOKEN__;
    }

    // not at the last element: inspect the nature of the action
    // (< 0: reduce, 0: ACCEPT, > 0: shift)

    int action = elementPtr->d_action;


    return action;
}

    // When an error has occurred, pop elements off the stack until the top
    // state has an error-item. If none is found, the default recovery
    // mode (which is to abort) is activated. 
    //
    // If EOF is encountered without being appropriate for the current state,
    // then the error recovery will fall back to the default recovery mode.
    // (i.e., parsing terminates)
void Parser::errorRecovery()
try
{
    if (d_acceptedTokens__ >= d_requiredTokens__)// only generate an error-
    {                                           // message if enough tokens 
        ++d_nErrors__;                          // were accepted. Otherwise
        error("Syntax error");                  // simply skip input

    }


    // get the error state
    while (not (s_state[top__()][0].d_type & ERR_ITEM))
    {
        pop__();
    }

    // In the error state, lookup a token allowing us to proceed.
    // Continuation may be possible following multiple reductions,
    // but eventuall a shift will be used, requiring the retrieval of
    // a terminal token. If a retrieved token doesn't match, the catch below 
    // will ensure the next token is requested in the while(true) block
    // implemented below:

    int lastToken = d_token__;                  // give the unexpected token a
                                                // chance to be processed
                                                // again.

    pushToken__(_error_);                       // specify _error_ as next token
    push__(lookup(true));                       // push the error state

    d_token__ = lastToken;                      // reactivate the unexpected
                                                // token (we're now in an
                                                // ERROR state).

    bool gotToken = true;                       // the next token is a terminal

    while (true)
    {
        try
        {
            if (s_state[d_state__]->d_type & REQ_TOKEN)
            {
                gotToken = d_token__ == _UNDETERMINED_;
                nextToken();                    // obtain next token
            }
            
            int action = lookup(true);

            if (action > 0)                 // push a new state
            {
                push__(action);
                popToken__();

                if (gotToken)
                {

                    d_acceptedTokens__ = 0;
                    return;
                }
            }
            else if (action < 0)
            {
                // no actions executed on recovery but save an already 
                // available token:
                if (d_token__ != _UNDETERMINED_)
                    pushToken__(d_token__);
 
                                            // next token is the rule's LHS
                reduce__(s_productionInfo[-action]); 
            }
            else
                ABORT();                    // abort when accepting during
                                            // error recovery
        }
        catch (...)
        {
            if (d_token__ == _EOF_)
                ABORT();                    // saw inappropriate _EOF_
                      
            popToken__();                   // failing token now skipped
        }
    }
}
catch (ErrorRecovery__)       // This is: DEFAULT_RECOVERY_MODE
{
    ABORT();
}

    // The parsing algorithm:
    // Initially, state 0 is pushed on the stack, and d_token__ as well as
    // d_nextToken__ are initialized to _UNDETERMINED_. 
    //
    // Then, in an eternal loop:
    //
    //  1. If a state does not have REQ_TOKEN no token is assigned to
    //     d_token__. If the state has REQ_TOKEN, nextToken() is called to
    //      determine d_nextToken__ and d_token__ is set to
    //     d_nextToken__. nextToken() will not call lex() unless d_nextToken__ is 
    //     _UNDETERMINED_. 
    //
    //  2. lookup() is called: 
    //     d_token__ is stored in the final element's d_token field of the
    //     state's SR_ array. 
    //
    //  3. The current token is looked up in the state's SR_ array
    //
    //  4. Depending on the result of the lookup() function the next state is
    //     shifted on the parser's stack, a reduction by some rule is applied,
    //     or the parsing function returns ACCEPT(). When a reduction is
    //     called for, any action that may have been defined for that
    //     reduction is executed.
    //
    //  5. An error occurs if d_token__ is not found, and the state has no
    //     default reduction. Error handling was described at the top of this
    //     file.

int Parser::parse()
try 
{
    push__(0);                              // initial state
    clearin();                              // clear the tokens.

    while (true)
    {
        try
        {
            if (s_state[d_state__]->d_type & REQ_TOKEN)
                nextToken();                // obtain next token


            int action = lookup(false);     // lookup d_token__ in d_state__

            if (action > 0)                 // SHIFT: push a new state
            {
                push__(action);
                popToken__();               // token processed
            }
            else if (action < 0)            // REDUCE: execute and pop.
            {
                executeAction(-action);
                                            // next token is the rule's LHS
                reduce__(s_productionInfo[-action]); 
            }
            else 
                ACCEPT();
        }
        catch (ErrorRecovery__)
        {
            errorRecovery();
        }
    }
}
catch (Return__ retValue)
{
    return retValue;
}

// $insert polymorphicImpl
namespace Meta__
{
   Base::~Base()
   {}
}


// $insert namespace-close
}


