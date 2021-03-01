
#include "../../include/csl.h"

// could be integrated with Lexer_ParseToken_ToWord ??

Word *
_Interpreter_TokenToWord ( Interpreter * interp, byte * token, int64 tsrli, int64 scwi )
{
    Word * word ;
    Context * cntx = _Context_ ;
    cntx->CurrentTokenWord = 0 ;
    if ( token )
    {
        interp->Token = token ;
        cntx->CurrentToken = token ;
        word = Lexer_ParseToken_ToWord ( interp->Lexer0, token, tsrli, scwi ) ;
        Word_SetTsrliScwi ( word, tsrli, scwi ) ;
        DEBUG_SETUP ( word, 1 ) ;
        cntx->CurrentTokenWord = word ; // dbg flag
        cntx->TokenDebugSetupWord = word ;
    }
    return cntx->CurrentTokenWord ; // allow DEBUG_SETUP to set this to 0 to skip interpreting it when it is 'stepped'
}

Word *
Interpreter_InterpretAToken ( Interpreter * interp, byte * token, int64 tsrli, int64 scwi )
{
    Word * word = 0 ;
    if ( token )
    {
        interp->Token = token ;
        word = _Interpreter_TokenToWord ( interp, token, tsrli, scwi ) ;
        word = Interpreter_DoWord ( interp, word, tsrli, scwi ) ;
    }
    return word ;
}

void
Interpreter_InterpretNextToken ( Interpreter * interp )
{
    byte * token = Lexer_ReadToken ( interp->Lexer0 ) ;
    //Printf ( (byte*) "\nInterpreter_InterpretNextToken : token = %s", token ) ;
    Interpreter_InterpretAToken ( interp, token, _Lexer_->TokenStart_ReadLineIndex, _Lexer_->SC_Index ) ;
}

void
Interpreter_InterpretSelectedTokens ( Interpreter * interp )
{
    ReadLiner * rl = _ReadLiner_ ;
    byte * token = Lexer_ReadToken ( _Lexer_ ) ;
    //Printf ( (byte*) "\nInterpreter_InterpretNextToken : token = %s", token ) ;
    if ( String_Equal ( "#", token ) && ( rl->ReadIndex == 1 ) )
    {
        Interpreter_InterpretAToken ( interp, token, _Lexer_->TokenStart_ReadLineIndex, _Lexer_->SC_Index ) ;
    }
}

Word *
Interpreter_DoWord_Default ( Interpreter * interp, Word * word0, int64 tsrli, int64 scwi )
{
    Word * word = Compiler_CopyDuplicatesAndPush ( word0, tsrli, scwi ) ;
    interp->w_Word = word ;
    Word_SetTsrliScwi ( word, tsrli, scwi ) ;
    Word_Eval ( word ) ;
    return word ; //let callee know about actual word evaled here after Compiler_CopyDuplicatesAndPush
}

Word *
_Interpreter_DoInfixWord ( Interpreter * interp, Word * word )
{
    byte * token = 0 ;
    Compiler * compiler = _Compiler_ ;
    SetState ( compiler, ( DOING_AN_INFIX_WORD | DOING_BEFORE_AN_INFIX_WORD ), true ) ;
    if ( GetState ( _Context_, C_SYNTAX ) && ( word->W_MorphismAttributes & ( CATEGORY_OP_EQUAL | CATEGORY_OP_OPEQUAL ) ) )
    {
        if ( ( word->W_MorphismAttributes & ( CATEGORY_OP_EQUAL | CATEGORY_OP_OPEQUAL ) ) ) SetState ( compiler, C_INFIX_EQUAL, true ) ;
        token = Interpret_C_Until_NotIncluding_Token5 ( interp, ( byte* ) ";", ( byte* ) ",", ( byte* ) ")", ( byte* ) "]", "#", ( byte* ) " \n\r\t", 0 ) ; // nb : delimiters parameter is necessary
        //token = Interpret_C_Until_NotIncluding_Token5 ( interp, ( byte* ) ";", ( byte* ) ",", ( byte* ) ")", ( byte* ) "]", 0, 0 ) ;
    }
    else Interpreter_InterpretNextToken ( interp ) ;
    // then continue and interpret this 'word' - just one out of lexical order
    SetState ( compiler, DOING_BEFORE_AN_INFIX_WORD, false ) ; //svState ) ;
    word = Interpreter_DoWord_Default ( interp, word, word->W_RL_Index, word->W_SC_Index ) ;
    SetState ( compiler, ( DOING_AN_INFIX_WORD | C_INFIX_EQUAL ), false ) ;
    return word ;
}
#if NEW_INTERPRET
#if 0

Word *
Interpreter_DoInfixOpStackWord ( )
{
    Interpreter * interp = _Interpreter_ ;
    Compiler * compiler = _Compiler_ ;
    Word * rtn ;
    if ( GetState ( compiler, ( DOING_AN_INFIX_WORD | DOING_BEFORE_AN_INFIX_WORD ) ) || GetState ( compiler, C_INFIX_EQUAL ) )
    {
        rtn = 0 ;
        goto done ;
    }
    Word * iopWord = ( Word * ) Stack_Top ( interp->InfixOpStack ) ;
    if ( iopWord )
    {
        SetState ( compiler, DOING_BEFORE_AN_INFIX_WORD, false ) ;
        if ( ( iopWord->W_MorphismAttributes & C_INFIX_OP_EQUAL ) ) SetState ( compiler, C_INFIX_EQUAL, true ) ;
        iopWord = Interpreter_DoWord_Default ( interp, iopWord, iopWord->W_RL_Index, iopWord->W_SC_Index ) ;
        _Stack_Pop ( interp->InfixOpStack ) ;
    }
    rtn = iopWord ;
done:
    SetState ( compiler, ( DOING_AN_INFIX_WORD | C_INFIX_EQUAL ), false ) ;
    return rtn ;
}

Word *
Interpreter_DoInfixWord ( Interpreter * interp, Word * word )
{
    DEBUG_SETUP ( word, 0 ) ;
    //if ( ( word->W_MorphismAttributes & (CATEGORY_OP_OPEQUAL | CATEGORY_OP_EQUAL | CATEGORY_PLUS_PLUS_MINUS_MINUS) ) ) word = _Interpreter_DoInfixWord ( interp, word ) ;
    //else
    {
        // we assume infix word takes only one following 'arg'
        if ( word->W_TypeAttributes == WT_INFIXABLE )
            Stack_Push ( interp->InfixOpStack, ( int64 ) word ) ;
        else
        {
            Compiler * compiler = _Compiler_ ;
            SetState ( compiler, ( DOING_AN_INFIX_WORD | DOING_BEFORE_AN_INFIX_WORD ), true ) ;
            word = Interpreter_DoWord_Default ( interp, word, word->W_RL_Index, word->W_SC_Index ) ;
            Word * iopWord = ( Word * ) Stack_Top ( interp->InfixOpStack ) ;
            if ( iopWord && ( iopWord->W_MorphismAttributes & C_INFIX_OP_EQUAL ) ) word = Interpreter_DoInfixOpStackWord ( ) ;
        }
    }
    return word ;
}
#else

Word *
Interpreter_DoInfixOpStackWord ( )
{
    Interpreter * interp = _Interpreter_ ;
    Compiler * compiler = _Compiler_ ;
    Word * iopWord = ( Word * ) Stack_Top ( interp->InfixOpStack ) ;
    if ( iopWord )
    {
        iopWord = Interpreter_DoWord_Default ( interp, iopWord, iopWord->W_RL_Index, iopWord->W_SC_Index ) ;
        Stack_Pop_WithZeroOnEmpty ( interp->InfixOpStack ) ;
    }
    return iopWord ;
}

Word *
Interpreter_DoInfixWord ( Interpreter * interp, Word * word )
{
    DEBUG_SETUP ( word, 0 ) ;
    Compiler * compiler = _Compiler_ ;
    Word * iopWord ;
    if ( word->W_MorphismAttributes & ( COMBINATOR | BLOCK_DELIMITER | KEYWORD ) ) return 0 ;
    if ( word->W_MorphismAttributes & ( COMBINATOR | BLOCK_DELIMITER ) ) return 0 ;
    if ( word->W_MorphismAttributes & LEFT_PAREN ) Stack_Push ( interp->InfixOpStack, 0 ) ;
    else if ( word->W_MorphismAttributes & RIGHT_PAREN )
    {
        iopWord = ( Word * ) Stack_Top ( interp->InfixOpStack ) ;
        word = Interpreter_DoInfixOpStackWord ( ) ;
    }
    else if ( IS_OBJECT_TYPE ( word ) )
    {
        if ( interp->InfixInterpState & ( IMS_EQUAL | IMS_OP_EQUAL ) ) word = Interpreter_DoWord_Default ( interp, word, word->W_RL_Index, word->W_SC_Index ) ;
        else if ( interp->InfixInterpState & ( IMS_INIT | IMS_OP ) )
        {
            word = Interpreter_DoWord_Default ( interp, word, word->W_RL_Index, word->W_SC_Index ) ;
            iopWord = ( Word * ) Stack_Top ( interp->InfixOpStack ) ;
            word = Interpreter_DoInfixOpStackWord ( ) ;
        }
        else if ( interp->InfixInterpState & ( IMS_OBJECT ) )
        {
            iopWord = ( Word * ) Stack_Top ( interp->InfixOpStack ) ;
            word = Interpreter_DoInfixOpStackWord ( ) ;
        }
        interp->InfixInterpState = IMS_OBJECT ;
    }
    else // IMS_OP | IMS_EQUAL | IMS_OP_EQUAL
    {
        if ( word->W_MorphismAttributes & ( CATEGORY_OP_EQUAL ) )
        {
            Stack_Push ( interp->InfixOpStack, ( int64 ) word ) ;
            interp->InfixInterpState = IMS_EQUAL ;
        }
        else if ( word->W_MorphismAttributes & ( CATEGORY_OP_OPEQUAL ) )
        {
            Stack_Push ( interp->InfixOpStack, ( int64 ) word ) ;
            interp->InfixInterpState = IMS_OP_EQUAL ;
        }
        if ( word->W_TypeAttributes == WT_INFIXABLE )
        {
            Stack_Push ( interp->InfixOpStack, ( int64 ) word ) ;
            interp->InfixInterpState |= IMS_OP ;
        }
        else
        {
            word = _Interpreter_DoPrefixWord ( _Context_, interp, word ) ;
            interp->InfixInterpState = IMS_OBJECT ;
        }
        //return 0 ;
    }
    return word ;
}
#endif

#elif 0

Word *
Interpreter_DoInfixWord ( Interpreter * interp, Word * word )
{
    Context * cntx = _Context_ ;
    if ( word->W_TypeAttributes == WT_INFIXABLE )
        Stack_Push ( interp->InfixOpStack, ( int64 ) word ) ;
    else
    {
        Compiler * compiler = _Compiler_ ;
        Word * iopWord ; //Boolean flag = false ;
        SetState ( compiler, ( DOING_AN_INFIX_WORD | DOING_BEFORE_AN_INFIX_WORD ), true ) ;
#if 0        
        if ( ( word->Name[0] == ';' ) ) //&& ( GetState ( cntx->Compiler0, ( DOING_A_PREFIX_WORD | DOING_BEFORE_A_PREFIX_WORD ) ) ) )
        {
            iopWord = ( Word * ) Stack_Top ( interp->InfixOpStack ) ;
            flag = true ;
        }
#endif        
        word = Interpreter_DoWord_Default ( interp, word, word->W_RL_Index, word->W_SC_Index ) ;
        //if ( flag == false )
        iopWord = ( Word * ) Stack_Top ( interp->InfixOpStack ) ;
        if ( iopWord && ( ! GetState ( cntx->Compiler0, ( DOING_A_PREFIX_WORD | DOING_BEFORE_A_PREFIX_WORD ) ) ) )
        {
            if ( ( iopWord->W_MorphismAttributes & C_INFIX_OP_EQUAL ) ) SetState ( compiler, C_INFIX_EQUAL, true ) ;
            word = iopWord ;
            word = Interpreter_DoWord_Default ( interp, word, word->W_RL_Index, word->W_SC_Index ) ;
            _Stack_Pop ( _Interpreter_->InfixOpStack ) ;
        }
        SetState ( compiler, ( DOING_AN_INFIX_WORD | DOING_BEFORE_AN_INFIX_WORD | C_INFIX_EQUAL ), false ) ;
    }
}
#endif

Word *
_Interpreter_DoPrefixWord ( Context * cntx, Interpreter * interp, Word * word )
{
    SetState ( cntx->Compiler0, ( DOING_A_PREFIX_WORD | DOING_BEFORE_A_PREFIX_WORD ), true ) ;
    word = Interpret_DoPrefixFunction_OrUntil_RParen ( interp, word ) ;
    SetState ( cntx->Compiler0, DOING_A_PREFIX_WORD, false ) ;
    return word ;
}

Word *
Interpreter_DoPrefixWord ( Context * cntx, Interpreter * interp, Word * word )
{
    if ( Lexer_IsNextWordLeftParen ( interp->Lexer0 ) ) word = _Interpreter_DoPrefixWord ( cntx, interp, word ) ;
    else if ( word->W_MorphismAttributes & CATEGORY_OP_1_ARG ) word = _Interpreter_DoInfixWord ( interp, word ) ; //goto doInfix ;
    else _SyntaxError ( ( byte* ) "Attempting to call a prefix function without following parenthesized args", 1 ) ;
    return word ;
}

Word *
Interpreter_C_PREFIX_RTL_ARGS_Word ( Word * word )
{
    word = LC_CompileRun_C_ArgList ( word ) ;
    return word ;
}

#if NEW_INTERPRET

Word *
Interpreter_DoInfixOrPrefixWord ( Interpreter * interp, Word * word )
{
    if ( word )
    {
        Context * cntx = _Context_ ;
        if ( word->W_TypeAttributes == WT_C_PREFIX_RTL_ARGS ) word = Interpreter_C_PREFIX_RTL_ARGS_Word ( word ) ;
            //else if ( ( word->W_TypeAttributes == WT_INFIXABLE ) && ( GetState ( cntx, ( INFIX_MODE | C_SYNTAX ) ) ) ) word = Interpreter_DoInfixWord ( interp, word ) ;
            // nb. Interpreter must be in INFIX_MODE because it is effective for more than one word
        else if ( GetState ( cntx, ( INFIX_MODE | C_SYNTAX ) ) && ( ! ( word->W_MorphismAttributes & COMBINATOR ) ) ) word = Interpreter_DoInfixWord ( interp, word ) ;
        else if ( ( word->W_TypeAttributes == WT_PREFIX ) || Lexer_IsWordPrefixing ( interp->Lexer0, word ) ) word = _Interpreter_DoPrefixWord ( cntx, interp, word ) ;
        else return 0 ;
    }
    return word ;
}
#else

Word *
Interpreter_DoInfixOrPrefixWord ( Interpreter * interp, Word * word )
{
    if ( word )
    {
        Context * cntx = _Context_ ;
        if ( word->W_TypeAttributes == WT_C_PREFIX_RTL_ARGS ) word = Interpreter_C_PREFIX_RTL_ARGS_Word ( word ) ;
        else if ( ( word->W_TypeAttributes == WT_INFIXABLE ) && ( GetState ( cntx, ( INFIX_MODE | C_SYNTAX ) ) ) ) word = _Interpreter_DoInfixWord ( interp, word ) ;
            // nb. Interpreter must be in INFIX_MODE because it is effective for more than one word
        else if ( ( word->W_TypeAttributes == WT_PREFIX ) || Lexer_IsWordPrefixing ( interp->Lexer0, word ) )
        {
            // with Lexer_IsWordPrefixing any postfix word that is not a keyword or a c_rtl arg word can now be used as a prefix function with parentheses (in PREFIX_MODE) - some 'syntactic sugar'
            // nb! : for this to work you must turn prefix mode on - 'prefixOn'
            word = _Interpreter_DoPrefixWord ( cntx, interp, word ) ;
        }
        else return 0 ;
    }
    return word ;
}

#endif
// four types of words related to syntax
// 1. regular rpn - reverse polish notation
// 2. regular prefix : polish, prefix notation where the function precedes the arguments - as in lisp
// 3. infix which takes one (or more) following 'args' and then becomes regular rpn : here only one arg is currently accepted
// 4. C arg lists which are left to right but are evaluated right to left, ie. the rightmost operand goes on the stack first then the next rightmost and so on such that topmost is the left operand
// we just rearrange the functions and args such that they all become regular rpn - forth like

Word *
Interpreter_DoWord ( Interpreter * interp, Word * word, int64 tsrli, int64 scwi )
{
    Word * word1 ;
    if ( word && word->Name )
    {
        Word_SetTsrliScwi ( word, tsrli, scwi ) ; // some of this maybe too much
        interp->w_Word = word ;
        if ( ! ( word1 = Interpreter_DoInfixOrPrefixWord ( interp, word ) ) ) word1 = Interpreter_DoWord_Default ( interp, word, tsrli, scwi ) ;
        if ( word1 && ( ! ( word1->W_MorphismAttributes & DEBUG_WORD ) ) ) word = word1, interp->LastWord = word1 ;
    }
    return word ;
}
// interpret with find after parse for known objects
// !! this version eliminates the possiblity of numbers being used as words !!
// objects and morphisms - terms from category theory

Word *
Interpreter_ReadNextTokenToWord ( Interpreter * interp )
{
    Word * word = 0 ;
    byte * token ;
    if ( token = Lexer_ReadToken ( interp->Lexer0 ) ) word = _Interpreter_TokenToWord ( interp, token, - 1, - 1 ) ;
    else SetState ( _Context_->Lexer0, LEXER_END_OF_LINE, true ) ;
    return word ;
}

Boolean
Word_IsSyntactic ( Word * word )
{
    if ( ( ! GetState ( _Debugger_, DBG_INFIX_PREFIX ) )
        && ( ( word->W_TypeAttributes & ( WT_PREFIX | WT_C_PREFIX_RTL_ARGS ) ) || ( Lexer_IsWordPrefixing ( _Lexer_, word )
        || ( ( word->W_TypeAttributes == WT_INFIXABLE ) && ( GetState ( _Context_, INFIX_MODE ) ) ) ) ) )
    {
        return true ;
    }
    else return false ;
}

void
Interpreter_SetLexState ( Interpreter * interp )
{
    byte llc = _Lexer_->LastLexedChar ;
    if ( llc == 0 ) SetState ( interp, END_OF_STRING, true ) ;
    else if ( llc == eof ) SetState ( interp, END_OF_FILE, true ) ;
    else if ( llc == '\n' ) SetState ( interp, END_OF_LINE, true ) ;
}

