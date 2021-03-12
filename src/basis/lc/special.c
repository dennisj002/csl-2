#include "../../include/csl.h"
//===================================================================================================================
//| LO_SpecialFunction(s) 
//===================================================================================================================
#define TEST_NEW 0
#define DEFINE_DBG _Is_DebugOn
#if TEST_NEW 
//#define DEFINE_DBG true
#define NEW_DEFINE true
#else
//#define DEFINE_DBG 0
#define NEW_DEFINE 0
#endif

ListObject *
_LO_Define ( ListObject * l0, ListObject * locals0 )
{
    LambdaCalculus * lc = _LC_ ;
    ListObject *value, *l1, *locals1 = 0, *value1, *l2, *lnext ;
    Word * word, *idLo ;
    SetState ( _CSL_, _DEBUG_SHOW_, DEFINE_DBG ) ;
    if ( _Is_DebugOn ) _LO_PrintWithValue ( l0, "\n_LO_Define : l0 = ", "" ) ;
    if ( ( l0->W_LispAttributes & ( LIST | LIST_NODE ) ) ) // scheme 'define : ( define ( func args) funcBody )
    {
        idLo = _LO_First ( l0 ) ;
#if NEW_DEFINE        
        ListObject * value0, *lambda ;
        value = LO_List_New ( LISP ) ;
        value0 = _LO_Next ( l0 ) ;
        LO_AddToHead ( value, value0 ) ; // body
#endif
        value1 = LO_List_New ( LISP ) ;
        for ( l2 = _LO_Next ( idLo ) ; l2 ; l2 = lnext )
        {
            lnext = _LO_Next ( l2 ) ;
            LO_AddToTail ( value1, l2 ) ;
            l2->W_ObjectAttributes |= LOCAL_VARIABLE ;
        }
        value1->W_LispAttributes |= ( LIST | LIST_NODE ) ;
        if ( _Is_DebugOn ) _LO_PrintWithValue ( locals1, "\n_LO_Define : locals1 = ", "" ) ;
        locals1 = value1 ;
#if NEW_DEFINE        
        LO_AddToHead ( value, value1 ) ;
        lambda = _LO_Read_DoToken ( _LC_, "lambda", 0, - 1, - 1 ) ;
        LO_AddToHead ( value, lambda ) ;
        if ( _Is_DebugOn ) _LO_PrintWithValue ( idLo, "\n_LO_Define : idLo = ", "" ) ;
        if ( _Is_DebugOn ) _LO_PrintWithValue ( locals1, "\n_LO_Define : locals1 = ", "" ) ;
        if ( _Is_DebugOn ) _LO_PrintWithValue ( value0, "\n_LO_Define : value0 = ", "" ) ;
        if ( _Is_DebugOn ) _LO_PrintWithValue ( value, "\n_LO_Define : value = ", "" ) ;
        //value->W_LispAttributes |= T_LAMBDA ;
        locals1 = 0 ;
#endif        
    }
#if NEW_DEFINE
    else idLo = l0, value = _LO_Next ( l0 ) ;
#else    
    else idLo = l0 ;
    value = _LO_Next ( l0 ) ;
#endif    
    idLo = idLo->Lo_CSLWord ;
    word = DataObject_New ( T_LC_DEFINE, 0, ( byte* ) idLo->Name, 0, NAMESPACE_VARIABLE, 0, 0, 0, 0, LISP, l0->W_RL_Index, l0->W_SC_Index ) ;
    CSL_WordList_Init ( word ) ;
    word->Definition = 0 ; // reset the definition from LO_Read
    _Context_->CurrentWordBeingCompiled = word ;
    word->Lo_CSLWord = word ;
    SetState ( lc, ( LC_DEFINE_MODE ), true ) ;
    Namespace_DoAddWord ( locals0 ? locals0 : lc->LispDefinesNamespace, word ) ; // put it at the beginning of the list to be found first
    if ( _Is_DebugOn ) _LO_PrintWithValue ( idLo, "\n_LO_Define : idLo = ", "" ) ;
    if ( _Is_DebugOn ) _LO_PrintWithValue ( value, "\n_LO_Define : value = ", "" ) ;
    if ( locals1 )
    {
        word->Lo_LambdaFunctionParameters = _LO_Copy ( ( ListObject* ) locals1, LISP ) ;
        word->Lo_LambdaFunctionBody = _LO_Copy ( value, LISP ) ;
        word->W_LispAttributes |= T_LAMBDA ;
        if ( _Is_DebugOn ) _LO_PrintWithValue ( word->Lo_LambdaFunctionParameters, "\n_LO_Define : word->Lo_LambdaFunctionParameters = ", "" ) ;
        if ( _Is_DebugOn ) _LO_PrintWithValue ( word->Lo_LambdaFunctionBody, "\n_LO_Define : word->Lo_LambdaFunctionBody = ", "" ) ; //, Pause ( ) ;
    }
    else
    {
        value = _LO_Eval ( lc, value, 0, 0 ) ; // 0 : don't apply
        if ( ( value && ( value->W_LispAttributes & T_LAMBDA ) ) )
        {
            value->Lo_LambdaFunctionParameters = _LO_Copy ( value->Lo_LambdaFunctionParameters, LISP ) ;
            value->Lo_LambdaFunctionBody = _LO_Copy ( value->Lo_LambdaFunctionBody, LISP ) ;
            if ( _Is_DebugOn ) _LO_PrintWithValue ( value->Lo_LambdaFunctionParameters, "\n_LO_Define : value->Lo_LambdaFunctionParameters = ", "\n" ) ;
            if ( _Is_DebugOn ) _LO_PrintWithValue ( value->Lo_LambdaFunctionBody, "\n_LO_Define : value->Lo_LambdaFunctionBody = ", "\n" ) ;
        }
        else value = _LO_Copy ( value, LISP ) ; // this value object should now become part of LISP non temporary memory
    }
    SetState ( _CSL_, _DEBUG_SHOW_, false ) ;
    word->Lo_Value = ( uint64 ) value ; // used by eval
    word->W_LispAttributes |= ( T_LC_DEFINE | T_LISP_SYMBOL ) ;
    word->State |= LC_DEFINED ;

    // the value was entered into the LISP memory, now we need a temporary carrier for LO_Print : yes, apparently, but why?
    l1 = DataObject_New ( T_LC_NEW, 0, word->Name, word->W_MorphismAttributes,
        word->W_ObjectAttributes, word->W_LispAttributes, 0, ( int64 ) value, 0, LISP, - 1, - 1 ) ; // all words are symbols
    l1->W_LispAttributes |= ( T_LC_DEFINE | T_LISP_SYMBOL ) ;
    l1->W_SourceCode = word->W_SourceCode = lc->LC_SourceCode ;
    if ( GetState ( _LC_, LC_COMPILE_MODE ) ) l1->W_SC_WordList = word->W_SC_WordList = _LC_->Lambda_SC_WordList ;
    SetState ( lc, ( LC_COMPILE_MODE | LC_DEFINE_MODE ), false ) ;
    _CSL_FinishWordDebugInfo ( l1 ) ;
    _Word_Finish ( l1 ) ;
    return l1 ;
}

ListObject *
_LO_MakeLambda ( ListObject * l0 )
{
    ListObject *args, *body, *lambda, *lnew, *body0 ;
    // allow args to be optionally an actual parenthesized list or just vars after the lambda
    if ( GetState ( _LC_, LC_DEFINE_MODE ) ) lambda = _Context_->CurrentWordBeingCompiled ;
    else lambda = _Word_New ( ( byte* ) "<lambda>", WORD_CREATE, 0, 0, 0, 0, DICTIONARY ) ; // don't _Word_Add : must *not* be "lambda" else it will wrongly replace the lambda T_SPECIAL_FUNCTION word in LO_Find
    args = l0 ;
    body0 = _LO_Next ( l0 ) ;
    if ( args->W_LispAttributes & ( LIST | LIST_NODE ) ) args = _LO_Copy ( args, LISP_TEMP ) ; // syntactically the args can be enclosed in parenthesis or not
    else
    {
        lnew = LO_New ( LIST, 0 ) ;
        do
        {
            LO_AddToTail ( lnew, _LO_CopyOne ( args, LISP_TEMP ) ) ;
        }
        while ( ( args = _LO_Next ( args ) ) != body0 ) ;
        args = lnew ;
    }
    if ( ( body0->W_LispAttributes & ( LIST | LIST_NODE ) ) ) body = _LO_Copy ( body0, LISP_TEMP ) ;
    else
    {
        lnew = LO_New ( LIST, 0 ) ;
        LO_AddToTail ( lnew, _LO_CopyOne ( body0, LISP_TEMP ) ) ;
        body = lnew ;
    }
    if ( GetState ( _LC_, LC_COMPILE_MODE ) )
    {
        SetState ( _LC_, LC_LAMBDA_MODE, true ) ;
        block codeBlk = CompileLispBlock ( args, body ) ;
        SetState ( _LC_, LC_LAMBDA_MODE, false ) ;
        *lambda->W_PtrToValue = ( uint64 ) codeBlk ;
    }
    if ( ! GetState ( _LC_, LC_COMPILE_MODE ) ) // nb! this needs to be 'if' not 'else' or else if' because the state is sometimes changed by CompileLispBlock, eg. for function parameters
    {
        lambda->Lo_CSLWord = lambda ;
        lambda->Lo_LambdaFunctionParameters = args ;
        lambda->Lo_LambdaFunctionBody = body ;
        lambda->W_LispAttributes |= T_LAMBDA | T_LISP_SYMBOL ;
        //lambda->CAttribute = 0 ;
    }
    return lambda ;
}

ListObject *
LO_SpecialFunction ( LambdaCalculus * lc, ListObject * l0, ListObject * locals )
{
    ListObject * lfirst, *macro = 0 ;
    if ( lfirst = _LO_First ( l0 ) )
    {
        while ( lfirst && ( lfirst->W_LispAttributes & T_LISP_MACRO ) )
        {
            //if ( Is_DebugModeOn ) LO_Debug_ExtraShow ( 0, 0, 0, ( byte* ) "\nLO_SpecialFunction : macro eval before : l0 = %s : locals = %s", c_gd ( _LO_PRINT_TO_STRING ( l0 ) ), locals ? _LO_PRINT_TO_STRING ( locals ) : ( byte* ) "" ) ;
            macro = lfirst ;
            macro->W_LispAttributes &= ~ T_LISP_MACRO ; // prevent short recursive loop calling of this function thru LO_Eval below
            l0 = _LO_Eval ( lc, l0, locals, 1 ) ;
            macro->W_LispAttributes |= T_LISP_MACRO ; // restore to its true type
            lfirst = _LO_First ( l0 ) ;
            macro = 0 ;
            //if ( Is_DebugModeOn ) LO_Debug_ExtraShow ( 0, 0, 0, ( byte* ) "\nLO_SpecialFunction : macro eval after : l0 = %s : locals = %s", c_gd ( _LO_PRINT_TO_STRING ( l0 ) ), locals ? _LO_PRINT_TO_STRING ( locals ) : ( byte* ) "" ) ;
        }
        if ( lfirst && lfirst->Lo_CSLWord && IS_MORPHISM_TYPE ( lfirst->Lo_CSLWord ) )
        {
            int64 tlf = (int64) (lfirst->Lo_CSLWord->W_LispAttributes & T_LISP_IF) ;
            //if ( tlf ) 
            l0 = ( ( LispFunction3 ) ( lfirst->Lo_CSLWord->Definition ) ) ( lfirst, locals, tlf ) ;  // non macro special functions here
            //else l0 = ( ( LispFunction3 ) ( lfirst->Lo_CSLWord->Definition ) ) ( lfirst, locals ) ;  // non macro special functions here
        }
        else
        {
            //if ( Is_DebugModeOn ) LO_Debug_ExtraShow ( 0, 0, 0, ( byte* ) "\nLO_SpecialFunction : final eval before : l0 = %s : locals = %s", c_gd ( _LO_PRINT_TO_STRING ( l0 ) ), locals ? _LO_PRINT_TO_STRING ( locals ) : ( byte* ) "nil" ) ;
            l0 = _LO_Eval ( lc, l0, locals, 1 ) ;
            //if ( Is_DebugModeOn ) LO_Debug_ExtraShow ( 0, 0, 0, ( byte* ) "\nLO_SpecialFunction : final eval after : l0 = %s : locals = %s", c_gd ( _LO_PRINT_TO_STRING ( l0 ) ), locals ? _LO_PRINT_TO_STRING ( locals ) : ( byte* ) "nil" ) ;
        }
    }
    return l0 ;
}

ListObject *
LO_Lambda ( ListObject * l0 )
{
    // lambda signature is "lambda" or an alias like "/\", /.", etc.
    //ListObject *lambdaSignature = _LO_First ( l0 ) ;
    Word * lambda = _LO_MakeLambda ( _LO_Next ( l0 ) ) ;
    lambda->W_LispAttributes |= T_LAMBDA ;
    return lambda ;
}

// (define macro (lambda (id (args) (args1)) ( 'define id ( lambda (args)  (args1) ) ) ) )

ListObject *
_LO_Macro ( ListObject * l0, ListObject * locals )
{
    ListObject *idNode = _LO_Next ( l0 ) ;
    //l0 = _LO_Define ( ( byte* ) "macro", idNode, locals ) ;
    l0 = _LO_Define ( idNode, 0 ) ;
    l0->W_LispAttributes |= T_LISP_MACRO ;
    if ( l0->Lo_CSLWord ) l0->Lo_CSLWord->W_LispAttributes |= T_LISP_MACRO ;
    if ( GetState ( _CSL_, DEBUG_MODE ) ) LO_Print ( l0 ) ;
    return l0 ;
}

ListObject *
LO_CompileDefine ( ListObject * l0 )
{
    SetState ( _Context_->Compiler0, RETURN_TOS, true ) ;
    SetState ( _LC_, LC_COMPILE_MODE, true ) ;
    ListObject * idNode = _LO_Next ( l0 ) ;
    l0 = _LO_Define ( idNode, 0 ) ;
    SetState ( _LC_, LC_COMPILE_MODE, false ) ;
    return l0 ;
}

ListObject *
LO_Define ( ListObject * l0 )
{
    l0 = LO_CompileDefine ( l0 ) ;
    return l0 ;
}

// setq

ListObject *
LO_Set ( ListObject * lfirst )
{
    ListObject *l0, * lsymbol, *value, *lset ;
    // lfirst is the 'set' signature
    for ( l0 = lfirst ; lsymbol = _LO_Next ( l0 ) ; l0 = value )
    {
        value = _LO_Next ( lsymbol ) ;
        if ( value )
        {
            lset = _LO_Define ( lsymbol, 0 ) ;
            LO_Print ( lset ) ;
        }
        else break ;
    }
    return 0 ;
}

ListObject *
LO_Let ( ListObject * lfirst, ListObject * locals )
{
    return LO_Set ( lfirst ) ;
}

ListObject *
_LO_Cons ( ListObject *first, ListObject * second ) //, uint64 allocType )
{
    ListObject * l0 = LO_New ( LIST, 0 ) ;
    LO_AddToTail ( l0->Lo_List, first ) ;
    LO_AddToTail ( l0->Lo_List, second ) ;

    return l0 ;
}

#if 0

ListObject *
LO_If ( ListObject * l0, ListObject * locals )
{
    LambdaCalculus * lc = _LC_ ;
    ListObject * test, *testResult, *sequence, *result ;
    // 'cond' is first node ; skip it.
    l0 = _LO_First ( l0 ) ; // the 'cond' (maybe aliased)
    test = _LO_Next ( l0 ) ;
    if ( Is_DebugOn ) _LO_PrintWithValue ( l0, "\nLO_Cond : l0 = ", "" ) ;
    while ( ( sequence = _LO_Next ( test ) ) )
    {
        if ( Is_DebugOn ) _LO_PrintWithValue ( test, "\nLO_Cond : test = ", "" ) ;
        if ( String_Equal ( test->Name, "else" ) )
        {
            //if ( Is_DebugOn ) _LO_PrintWithValue ( sequence, "\nLO_Cond : sequence = ", "" ) ;
            result = _LO_Eval ( lc, sequence, locals, 1 ) ;
            break ;
        }
        testResult = _LO_Eval ( lc, test, locals, 1 ) ;
        if ( ( testResult->Lo_Value ) || ( String_Equal ( testResult->Name, "else" ) ) )
        {
            //if ( Is_DebugOn ) _LO_PrintWithValue ( sequence, "\nLO_Cond : sequence = ", "" ) ;
            result = _LO_Eval ( lc, sequence, locals, 1 ) ;
            break ;
        }
        else test = _LO_Next ( sequence ) ;
    }
    if ( ! sequence )
    {
        if ( Is_DebugOn ) _LO_PrintWithValue ( test, "\nLO_Cond : test = ", "" ) ;
        result = _LO_Eval ( lc, test, locals, 1 ) ;
    }
    if ( Is_DebugOn ) _LO_PrintWithValue ( result, "\nLO_Cond : result = ", "" ) ;
    return result ;
}
#else

ListObject *
LO_If ( ListObject * l0, ListObject * locals )
{
    return LO_Cond ( l0, locals, 1 ) ;

}
#endif

/* not exactly but ...
LIST:
'(' ( LIST | ATOM ) + ')'
COND : LIST | ATOM
IFTRUE : LIST | ATOM
CONDITIONAL : ( cond ( <cond clause>+ )
COND_FUNC : '(' 'cond' ( CONDITIONAL )+ ( IFTRUE | 'else' IFTRUE ) ? ')'
 * r7r2 : formal syntax
The following extensions to BNF are used to make the de-
scription more concise: <thing>* means zero or more occur-
rences of <thing>; and <thing>+ means at least one <thing>.
<expression> : <identifier>
    | <literal>
    | <procedure call>
    | <lambda expression>
    | <conditional>
    | <assignment>
    | <derived expression>
    | <macro use>
    |<macro block>
    | <includer>
<conditional> : (if <test> <consequent> <alternate>)
<test> : <expression>
<test> : <expression> 
<command> : <expression>
<sequence> : <command>* <expression> 
<cond clause> : (<test> <sequence>) | (<test>)
<derived expression> : (cond <cond clause>+ ) | (cond <cond clause>* (else <sequence>)) | ...
 */
#if 1
#define IS_COND_MORPHISM_TYPE(word) ( word->W_MorphismAttributes & ( CATEGORY_OP|KEYWORD|ADDRESS_OF_OP|BLOCK|T_LAMBDA ) || ( word->W_LispAttributes & ( T_LAMBDA ) ) )
ListObject *
LO_Cond ( ListObject * l0, ListObject * locals, int64 ifFlag )
{
    LambdaCalculus * lc = _LC_ ;
    ListObject *condClause, * test, *testForElse = 0, *sequence, * resultNode = nil, * result, *testResult, *seqNext = 0 ;
    Boolean timt = false, tilt = false ; // timt : test is morphism type ; tilt : test is list type
    // 'cond' is first node ; skip it.
    if ( DEFINE_DBG ) _LO_PrintWithValue ( l0, "\nLO_Cond : l0 = ", "" ) ;
    if ( condClause = _LO_Next ( l0 ) )
    {
        do
        {
            if ( DEFINE_DBG ) _LO_PrintWithValue ( condClause, "\nLO_Cond : condClause = ", "" ) ;
            test = _LO_First ( condClause ) ;
            if ( DEFINE_DBG ) _LO_PrintWithValue ( test, "\nLO_Cond : test = ", "" ) ;
            if ( test->W_LispAttributes & ( LIST | LIST_NODE ) )
            {
                tilt = true ;
                testForElse = _LO_First ( test ) ;
                if ( String_Equal ( testForElse->Name, "else" ) )
                {
                    resultNode = _LO_Next ( test ) ;
                    break ;
                }
            }
            else if ( timt = IS_MORPHISM_TYPE ( test ) ) test = condClause, tilt = true ;
            if ( DEFINE_DBG ) _LO_PrintWithValue ( test, "\nLO_Cond : test = ", "" ) ;
            sequence = _LO_Next ( test ) ;
            if ( ! sequence )
            {
                if ( DEFINE_DBG ) _LO_PrintWithValue ( test, "\nLO_Cond : test = ", "" ) ;
                resultNode = test ; // where there is just a test with a sequence => a finally : the test is the finally
                break ;
            }
            testResult = _LO_Eval ( lc, test, locals, 1 ) ;
            if ( testResult->Lo_Value || ( String_Equal ( test->Name, "else" ) ) )
            {
                resultNode = sequence ;
                break ;
            }
            else
            {
                if ( ifFlag )
                {
                    seqNext = _LO_Next ( sequence ) ;
                    if ( seqNext ) condClause = seqNext ;
                    else condClause = _LO_Next ( condClause ) ;
                    resultNode = condClause ;
                    break ;
                }
                Boolean silt = ( sequence->W_LispAttributes & ( LIST | LIST_NODE ) ) ;
                if ( ( ! timt ) && ( ! tilt ) && ( ! silt ) )
                {
                    seqNext = _LO_Next ( sequence ) ;
                    if ( seqNext ) condClause = seqNext ;
                    else condClause = _LO_Next ( condClause ) ;
                }
                else condClause = _LO_Next ( condClause ) ;
                if ( timt ) condClause = _LO_Next ( condClause ) ;
            }
            timt = false ;
            tilt = false ;
        }
        while ( condClause ) ;
        if ( ! resultNode )
            resultNode = test ; // where there is just a test with a sequence => a finally : the test is the finally
        if ( DEFINE_DBG ) _LO_PrintWithValue ( resultNode, "\nLO_Cond : resultNode = ", "" ) ;
        result = _LO_Eval ( lc, resultNode, locals, 1 ) ; // last one, no need to copy
        if ( DEFINE_DBG ) _LO_PrintWithValue ( result, "\nLO_Cond : result = ", "" ) ;
        return result ;
    }
    return nil ;
}
#elif 1 // backup working copy 0.915.920

ListObject *
LO_Cond ( ListObject * l0, ListObject * locals, 0 )
{
    LambdaCalculus * lc = _LC_ ;
    ListObject *condClause, * test, *testForElse = 0, *sequence, * resultNode = nil, * result, *testResult ;
    Boolean mt = false ;
    // 'cond' is first node ; skip it.
    if ( DEFINE_DBG ) _LO_PrintWithValue ( l0, "\nLO_Cond : l0 = ", "" ) ;
    if ( condClause = _LO_Next ( l0 ) )
    {
        do
        {
            if ( DEFINE_DBG ) _LO_PrintWithValue ( condClause, "\nLO_Cond : condClause = ", "" ) ;
            test = _LO_First ( condClause ) ;
            if ( test->W_LispAttributes & ( LIST | LIST_NODE ) )
            {
                testForElse = _LO_First ( test ) ;
                if ( String_Equal ( testForElse->Name, "else" ) )
                {
                    resultNode = _LO_Next ( test ) ;
                    break ;
                }
            }
            else if ( mt = IS_MORPHISM_TYPE ( test ) ) test = condClause ;
            if ( DEFINE_DBG ) _LO_PrintWithValue ( test, "\nLO_Cond : test = ", "" ) ;
            testResult = _LO_Eval ( lc, test, locals, 1 ) ;
            if ( ( ( testResult != nil ) && ( testResult->Lo_Value ) ) )
            {
                sequence = _LO_Next ( test ) ;
                if ( sequence ) resultNode = sequence ;
                else resultNode = test ;
                break ;
            }
            else
            {
                condClause = _LO_Next ( condClause ) ;
                if ( mt ) condClause = _LO_Next ( condClause ) ;
                mt = false ;
            }
        }
        while ( condClause ) ;
        if ( ! resultNode ) resultNode = test ;
        if ( DEFINE_DBG ) _LO_PrintWithValue ( resultNode, "\nLO_Cond : resultNode = ", "" ) ;
        result = _LO_Eval ( lc, resultNode, locals, 1 ) ; // last one, no need to copy
        if ( DEFINE_DBG ) _LO_PrintWithValue ( result, "\nLO_Cond : result = ", "" ) ;
        return result ;
    }
    return nil ;
}
#elif 0

ListObject *
LO_ElseTest ( ListObject * elseNode )
{
    ListObject * resultNode = 0 ;
    if ( elseNode )
    {
        while ( elseNode->W_LispAttributes & ( LIST | LIST_NODE ) )
        {
            elseNode = _LO_First ( elseNode ) ;
        }
        if ( String_Equal ( elseNode->Name, "else" ) ) resultNode = _LO_Next ( elseNode ) ;
    }
    return resultNode ;
}

ListObject *
LO_Cond ( ListObject * l0, ListObject * locals, 0 )
{
    LambdaCalculus * lc = _LC_ ;
    ListObject *condClause, *condClause1, * test, *elseNode = 0, *sequence, * resultNode = nil, * result, *testTorF ;
    // 'cond' is first node ; skip it.
    if ( Is_DebugOn ) _LO_PrintWithValue ( l0, "\nLO_Cond : l0 = ", "" ) ;
    //if ( Is_DebugOn ) if ( locals ) _LO_PrintWithValue ( locals, "\nLO_Cond : locals = ", "" ) ;
    if ( condClause = _LO_Next ( l0 ) )
    {
        do
        {
#if 0            
            //if ( Is_DebugOn ) _LO_PrintWithValue ( condClause, "\nLO_Cond : condClause = ", "" ) ;
            test = _LO_First ( condClause ) ;
            if ( test->W_LispAttributes & ( LIST | LIST_NODE ) )
            {
                testForElse = _LO_First ( test ) ;
                //if ( Is_DebugOn && String_Equal ( testForElse->Name, "else" )) Printf ("\ngot : else\n") ;
            }
            else testForElse = test ;
            //if ( Is_DebugOn ) _LO_PrintWithValue ( test, "\nLO_Cond : test = ", "" ) ;
#endif      
            // what is test? what is sequence?
            //test = condClause ;
            do
            {
                if ( IS_NON_MORPHISM_TYPE ( condClause ) )
                {
                    test = condClause ;
                    break ;
                }
                else if ( condClause->W_LispAttributes & ( LIST | LIST_NODE ) )
                {
                    condClause = _LO_First ( condClause ) ;
                    if ( sequence = _LO_Next ( condClause1 ) )
                    {
                        if ( sequence->W_LispAttributes & ( LIST | LIST_NODE ) ) ;
                    }
                    //if ( Is_DebugOn && String_Equal ( testForElse->Name, "else" )) Printf ("\ngot : else\n") ;
                }
                else ;
            }
            while ( 1 ) ;
            if ( Is_DebugOn ) _LO_PrintWithValue ( test, "\nLO_Cond : test = ", "" ) ;
            sequence = _LO_Next ( test ) ;
            if ( resultNode = LO_ElseTest ( test ) ) break ;
            if ( Is_DebugOn ) _LO_PrintWithValue ( sequence, "\nLO_Cond : sequence = ", "" ) ;
            testTorF = _LO_Eval ( lc, test, locals, 1 ) ;
            //if ( Is_DebugOn ) _LO_PrintWithValue ( testTorF, "\nLO_Cond : testTorF = ", "" ) ;
            if ( ( ( testTorF != nil ) && ( testTorF->Lo_Value ) ) )
            {
                if ( sequence ) resultNode = sequence ;
                else resultNode = test ;
                //if ( Is_DebugOn && String_Equal ( testForElse->Name, "else" ) ) Printf ("\nLO_Cond : test = else ") ;
                break ;
            }
            else if ( resultNode = LO_ElseTest ( sequence ) ) break ;
            else condClause = _LO_Next ( sequence ) ;
        }
        while ( condClause ) ;
        if ( ! resultNode ) resultNode = test ; //sequence ? sequence : test ;
        if ( Is_DebugOn ) _LO_PrintWithValue ( resultNode, "\nLO_Cond : resultNode = ", "" ) ;
        result = _LO_Eval ( lc, resultNode, locals, 1 ) ; // last one, no need to copy
        //if ( Is_DebugOn ) _LO_PrintWithValue ( result, "\nLO_Cond : result = ", "" ) ;
        return result ;
    }
    return nil ;
}
#elif 0

ListObject *
LO_Cond ( ListObject * l0, ListObject * locals, 0 )
{
    LambdaCalculus * lc = _LC_ ;
    ListObject *condClause, *testForElse, *resultNode = 0, * result, *testTorF ;
    // 'cond' is first node ; skip it.
    condClause = _LO_Next ( l0 ) ;
    while ( condClause )
    {
        testForElse = _LO_First ( condClause ) ;
        if ( testForElse->W_LispAttributes & ( LIST | LIST_NODE ) ) testForElse = _LO_First ( testForElse ) ;
        resultNode = _LO_Next ( testForElse ) ;
        if ( resultNode->W_LispAttributes & ( LIST | LIST_NODE ) ) resultNode = _LO_First ( resultNode ) ;
        testTorF = _LO_Eval ( lc, testForElse, locals, 1 ) ;
        if ( ( ( testTorF != nil ) && ( testTorF->Lo_Value ) ) || ( String_Equal ( testForElse->Name, "else" ) ) || ( String_Equal ( testTorF->Name, "else" ) ) ) break ;
        else condClause = _LO_Next ( condClause ) ;
    }
    result = _LO_Eval ( lc, resultNode, locals, 1 ) ; // last one, no need to copy
    return result ;
}
#endif
// lisp 'list' function
// lfirst must be the first element of the list

ListObject *
_LO_List ( LambdaCalculus * lc, ListObject * lfirst )
{
    ListObject * lnew = LO_New ( LIST, 0 ), *l0, *lnext, *l1 ;
    for ( l0 = lfirst ; l0 ; l0 = lnext )
    {
        lnext = _LO_Next ( l0 ) ;
        if ( l0->W_LispAttributes & ( LIST | LIST_NODE ) ) l1 = _LO_List ( lc, _LO_First ( l0 ) ) ;
        else l1 = LO_Eval ( LO_CopyOne ( l0 ) ) ;
        LO_AddToTail ( lnew, l1 ) ;
    }
    return lnew ;
}

ListObject *
LO_List ( ListObject * lfirst )
{
    LambdaCalculus * lc = _LC_ ;
    // 'list' is first node ; skip it.
    ListObject * l0 = _LO_List ( lc, _LO_Next ( lfirst ) ) ;
    return l0 ;
}

ListObject *
LO_Begin ( ListObject * l0, ListObject * locals )
{
    LambdaCalculus * lc = _LC_ ;
    ListObject * leval ;
    // 'begin' is first node ; skip it.
    SetState ( _LC_, LC_BEGIN_MODE, true ) ;
    if ( l0 )
    {
        for ( l0 = _LO_Next ( l0 ) ; l0 ; l0 = _LO_Next ( l0 ) )
        {
            leval = _LO_Eval ( lc, l0, locals, 1 ) ;
        }
    }
    else leval = 0 ;
    SetState ( _LC_, LC_BEGIN_MODE, false ) ;
    return leval ;
}

ListObject *
LO_Car ( ListObject * l0 )
{
    ListObject * lfirst = _LO_Next ( l0 ) ;
    if ( lfirst->W_LispAttributes & ( LIST_NODE | LIST ) ) return LO_CopyOne ( _LO_First ( lfirst ) ) ; //( ListObject * ) lfirst ;
    else return LO_CopyOne ( lfirst ) ;
}

ListObject *
LO_Cdr ( ListObject * l0 )
{
    ListObject * lfirst = _LO_Next ( l0 ) ;
    if ( lfirst->W_LispAttributes & ( LIST_NODE | LIST ) ) return _LO_Next ( _LO_First ( lfirst ) ) ; //( ListObject * ) lfirst ;
    return ( ListObject * ) _LO_Next ( lfirst ) ;
}

ListObject *
_LC_Eval ( ListObject * l0 )
{
    LambdaCalculus * lc = _LC_ ;
    ListObject * lfirst = _LO_Next ( l0 ) ;
    return LO_Eval ( lfirst ) ;
}

void
_LO_Semi ( Word * word )
{
    if ( word )
    {
        CSL_EndBlock ( ) ;
        block blk = ( block ) DataStack_Pop ( ) ;
        Word_InitFinal ( word, ( byte* ) blk ) ;
        word->W_LispAttributes |= T_LISP_CSL_COMPILED ;
        word->W_SourceCode = _LC_->LC_SourceCode ;
        word->W_SC_WordList = _LC_->Lambda_SC_WordList ;
        _LC_->Lambda_SC_WordList = 0 ;
    }
}

Word *
_LO_Colon ( ListObject * lfirst )
{
    Context * cntx = _Context_ ;
    ListObject *lcolon = lfirst, *lname, *ldata ;
    lname = _LO_Next ( lcolon ) ;
    ldata = _LO_Next ( lname ) ;
    _CSL_Namespace_NotUsing ( ( byte* ) "Lisp" ) ; // nb. don't use Lisp words when compiling csl
    CSL_RightBracket ( ) ;
    Word * word = Word_New ( lname->Name ) ;
    SetState ( cntx->Compiler0, COMPILE_MODE, true ) ;
    CSL_InitSourceCode_WithName ( _CSL_, lname->Name, 1 ) ;
    cntx->CurrentWordBeingCompiled = word ;
    _CSL_RecycleInit_CSL_N_M_Node_WordList ( _CSL_->CSL_N_M_Node_WordList, 1 ) ;
    CSL_WordList_PushWord ( _LO_CopyOne ( lcolon, DICTIONARY ) ) ;
    CSL_BeginBlock ( ) ;

    return word ;
}

// compile csl code in Lisp/Scheme

ListObject *
_LO_CSL ( ListObject * lfirst )
{
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    LambdaCalculus * lc = 0 ;
    ListObject *ldata, *word = 0, *word1 ; //, *lcolon ;
    if ( _LC_ )
    {
        if ( GetState ( _LC_, LC_READ ) )
        {
            SetState ( _LC_, LC_READ_MACRO_OFF, true ) ;
            return 0 ;
        }
        SetState ( _LC_, LC_INTERP_MODE, true ) ;
        lc = _LC_ ;
    }
    _CSL_Namespace_NotUsing ( ( byte * ) "Lisp" ) ; // nb. don't use Lisp words when compiling csl
    SetState ( cntx, LC_CSL, true ) ;
    SetState ( compiler, LISP_MODE, false ) ;
    _CSL_RecycleInit_CSL_N_M_Node_WordList ( _CSL_->CSL_N_M_Node_WordList, 1 ) ;
    CSL_WordList_PushWord ( _LO_CopyOne ( lfirst, DICTIONARY ) ) ;
    for ( ldata = _LO_Next ( lfirst ) ; ldata ; ldata = _LO_Next ( ldata ) )
    {
        if ( ldata->W_LispAttributes & ( LIST | LIST_NODE ) )
        {
            _CSL_Parse_LocalsAndStackVariables ( 1, 1, ldata, compiler->LocalsCompilingNamespacesStack, 0 ) ;
        }
        else if ( String_Equal ( ldata->Name, ( byte * ) "tick" ) || String_Equal ( ldata->Name, ( byte * ) "'" ) )
        {
            ldata = _LO_Next ( ldata ) ;
            Lexer_ParseObject ( _Lexer_, ldata->Name ) ;
            DataStack_Push ( ( int64 ) _Lexer_->Literal ) ;
        }
        else if ( String_Equal ( ldata->Name, ( byte * ) "s:" ) )
        {
            //lcolon = ldata ;
            CSL_DbgSourceCodeOn ( ) ;
            word = _LO_Colon ( ldata ) ;
            ldata = _LO_Next ( ldata ) ; // bump ldata to account for name - skip name
        }
        else if ( _String_EqualSingleCharString ( ldata->Name, ':' ) )
        {
            //lcolon = ldata ;
            word = _LO_Colon ( ldata ) ;
            ldata = _LO_Next ( ldata ) ; // bump ldata to account for name - skip name
        }
        else if ( String_Equal ( ldata->Name, ( byte * ) "return" ) )
        {
            ldata = _LO_Next ( ldata ) ;
            CSL_DoReturnWord ( ldata ) ;
        }
        else if ( String_Equal ( ldata->Name, ( byte * ) ";s" ) && ( ! GetState ( cntx, C_SYNTAX ) ) )
        {
            CSL_DbgSourceCodeOff ( ) ;
            _LO_Semi ( word ) ;
        }
        else if ( _String_EqualSingleCharString ( ldata->Name, ';' ) && ( ! GetState ( cntx, C_SYNTAX ) ) )
        {
#if 0            
            // in case we have more than one ":" on our list ...
            ListObject *ldata1 = _LO_Next ( ldata ) ; // bump ldata to account for name
            word->W_SourceCode = String_New_SourceCode ( _CSL_->SC_Buffer ) ;
            if ( ldata1 && String_Equal ( ldata1->Name, ( byte * ) ":" ) )
            {
                CSL_InitSourceCode_WithName ( _CSL_, ( byte* ) "(", 1 ) ;
            }
#endif            
            _LO_Semi ( word ) ;
        }
        else //if ( ldata )
        {
            word1 = _Interpreter_TokenToWord ( cntx->Interpreter0, ldata->Name, ldata->W_RL_Index, ldata->W_SC_Index ) ;
            Interpreter_DoWord ( cntx->Interpreter0, word1, ldata->W_RL_Index, ldata->W_SC_Index ) ;
        }
    }
    SetState ( cntx, LC_CSL, false ) ;
    if ( lc )
    {
        _LC_ = lc ;
        SetState ( _LC_, LC_INTERP_DONE, true ) ;
        SetState ( _LC_, LC_READ_MACRO_OFF, false ) ;
        //LC_RestoreStack ( ) ;
    }
    Namespace_DoNamespace_Name ( ( byte * ) "Lisp" ) ;
    if ( ! CompileMode )
    {
        Compiler_Init ( compiler, 0 ) ;
        CSL_AfterWordReset ( ) ;
    }
    return nil ;
}

#if 0

ListObject *
LO_Logic_Equals ( ListObject * l0, ListObject * locals )
{
    ListObject * lfirst = _LO_Next ( l0 ), *lsecond, *lf, *ls, * lresult ;
    lsecond = _LO_Next ( lfirst ) ;
    lf = _LO_Eval ( _LC_, lfirst, locals, 1 ) ;
    ls = _LO_Eval ( _LC_, lsecond, locals, 1 ) ;
    DataStack_Push ( ( int64 ) ( lf->Lo_Value == ls->Lo_Value ) ) ;
    lresult = LO_PrepareReturnObject ( ) ;
    return lresult ;
}

ListObject *
LO_Plus ( ListObject * l0, ListObject * locals )
{
    ListObject * lfirst = _LO_Next ( l0 ), *lsecond, *lf, *ls, * lresult ;
    lsecond = _LO_Next ( lfirst ) ;
    //lf = _LO_Eval ( _LC_, LO_CopyOne (lfirst), locals, 1 ) ;
    //ls = _LO_Eval ( _LC_, LO_CopyOne (lsecond), locals, 1 ) ;
    lf = _LO_Eval ( _LC_, lfirst, locals, 1 ) ;
    ls = _LO_Eval ( _LC_, lsecond, locals, 1 ) ;
    DataStack_Push ( ( int64 ) ( lf->Lo_Value + ls->Lo_Value ) ) ;
    lresult = LO_PrepareReturnObject ( ) ;
    return lresult ;
}

ListObject *
LO_Minus ( ListObject * l0, ListObject * locals )
{
    ListObject * lfirst = _LO_Next ( l0 ), *lsecond, *lf, *ls, * lresult ;
    lsecond = _LO_Next ( lfirst ) ;
    lf = _LO_Eval ( _LC_, lfirst, locals, 1 ) ;
    ls = _LO_Eval ( _LC_, lsecond, locals, 1 ) ;
    DataStack_Push ( ( int64 ) ( lf->Lo_Value - ls->Lo_Value ) ) ;
    lresult = LO_PrepareReturnObject ( ) ;
    return lresult ;
}

ListObject *
LO_LessThan ( ListObject * l0, ListObject * locals )
{
    ListObject * lfirst = _LO_Next ( l0 ), *lsecond, *lf, *ls, * lresult ;
    lsecond = _LO_Next ( lfirst ) ;
    lf = _LO_Eval ( _LC_, lfirst, locals, 1 ) ;
    ls = _LO_Eval ( _LC_, lsecond, locals, 1 ) ;
    DataStack_Push ( ( int64 ) ( lf->Lo_Value < ls->Lo_Value ) ) ;
    lresult = LO_PrepareReturnObject ( ) ;
    return lresult ;
}

ListObject *
LO_Else ( ListObject * l0 )
{
    ListObject * lfirst = _LO_Next ( l0 ) ;
    //if ( lfirst->W_LispAttributes & ( LIST_NODE | LIST ) ) return LO_CopyOne ( _LO_First ( lfirst ) ) ; //( ListObject * ) lfirst ;
    return LO_Eval ( LO_CopyOne ( lfirst ) ) ;
}

#endif

