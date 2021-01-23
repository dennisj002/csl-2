
#include "../include/csl.h"

byte *
Interpret_C_Until_NotIncluding_Token4 ( Interpreter * interp, byte * end1, byte * end2, byte* end3, byte* end4, byte * delimiters, Boolean newlineBreakFlag )
{
    byte * token ;
    int64 inChar ;
    Lexer * lexer = _Lexer_ ;
    do
    {
        if ( newlineBreakFlag && ReadLine_AreWeAtNewlineAfterSpaces ( _ReadLiner_ ) )
        {
            token = 0 ;
            break ;
        }
        token = _Lexer_ReadToken ( lexer, delimiters ) ;
        List_CheckInterpretLists_OnVariable ( _Compiler_->PostfixLists, token ) ;
        if ( String_Equal ( token, end1 ) || String_Equal ( token, end2 ) || String_Equal ( token, end3 ) || String_Equal ( token, end4 ) ) break ;
        else if ( GetState ( _Compiler_, DOING_A_PREFIX_WORD ) && _String_EqualSingleCharString ( token, ')' ) )
        {
            Interpreter_InterpretAToken ( interp, token, lexer->TokenStart_ReadLineIndex, lexer->SC_Index ) ;
            token = 0 ;
            break ;
        }
#if 1        
        else if ( GetState ( _Context_, C_SYNTAX ) && ( String_Equal ( token, "," ) || _String_EqualSingleCharString ( token, ';' ) ) )
        {
            CSL_ArrayModeOff_OptimizeOn ( ) ;
            break ;
        }
#endif        
        else Interpreter_InterpretAToken ( interp, token, lexer->TokenStart_ReadLineIndex, lexer->SC_Index ) ;
        inChar = ReadLine_PeekNextChar ( _Context_->ReadLiner0 ) ;
        if ( ( inChar == 0 ) || ( inChar == - 1 ) || ( inChar == eof ) ) token = 0 ;
    }
    while ( token ) ;
    if ( token ) CSL_PushToken_OnTokenList ( token ) ;
    return token ;
}

byte *
Interpret_Until_Token ( Interpreter * interp, byte * end, byte * delimiters )
{
    byte * token ;
    int64 inChar ;
    Lexer * lexer = _Lexer_ ;
    do
    {
        token = _Lexer_ReadToken ( lexer, delimiters ) ;
        if ( String_Equal ( token, end ) )
        {
            if ( GetState ( _Compiler_, C_COMBINATOR_LPAREN ) && ( _String_EqualSingleCharString ( token, ';' ) ) ) CSL_PushToken_OnTokenList ( token ) ;
            break ;
        }
        if ( _String_EqualSingleCharString ( token, ';' ) && GetState ( _Context_, C_SYNTAX ) && GetState ( _Compiler_, C_COMBINATOR_PARSING ) )
        {
            CSL_PushToken_OnTokenList ( token ) ;
            CSL_ArrayModeOff_OptimizeOn ( ) ;
            break ;
        }
        else Interpreter_InterpretAToken ( interp, token, lexer->TokenStart_ReadLineIndex, lexer->SC_Index ) ;
        inChar = ReadLine_PeekNextChar ( _Context_->ReadLiner0 ) ;
        if ( ( inChar == 0 ) || ( inChar == - 1 ) || ( inChar == eof ) ) token = 0 ;
    }
    while ( token ) ;
    return token ;
}

Word *
Interpret_DoPrefixFunction_OrUntil_RParen ( Interpreter * interp, Word * prefixFunction )
{
    Word * word = 0 ;
    if ( prefixFunction )
    {
        byte * token ;
        int64 i, flag = 0 ; 
        Compiler * compiler = interp->Compiler0 ;
        while ( 1 )
        {
            token = Lexer_ReadToken ( interp->Lexer0 ) ; // skip the opening left paren
            if ( token && ( ! _String_EqualSingleCharString ( token, '(' ) ) )
            {
                if ( word = Finder_Word_FindUsing (interp->Finder0, token, 1) )
                {
                    if ( word->W_MorphismAttributes & DEBUG_WORD ) continue ;
                    else flag = 1 ;
                    break ;
                }
            }
            else break ;
        }
        d0 ( if ( Is_DebugModeOn ) _CSL_SC_WordList_Show ( "\n_Interpret_PrefixFunction_Until_RParen", 0, 0 ) ) ;
        SetState ( compiler, PREFIX_ARG_PARSING, true ) ;
        if ( prefixFunction->W_NumberOfPrefixedArgs )
        {
            Interpreter_InterpretAToken ( interp, token, - 1, - 1 ) ;
            for ( i = 0 ; i < (prefixFunction->W_NumberOfPrefixedArgs - 1) ; i ++ ) // -1 : we already did one above
            {
                Interpreter_InterpretNextToken ( interp ) ;
            }
        }
        else
        {
            if ( flag ) Interpreter_InterpretAToken ( interp, token, - 1, - 1 ) ;
            else Interpret_Until_Token ( interp, ( byte* ) ")", ( byte* ) " ,\n\r\t" ) ;
        }
        SetState ( compiler, ( DOING_BEFORE_A_PREFIX_WORD ), false ) ;
        if ( ! GetState ( _Debugger_, DBG_INFIX_PREFIX ) ) word = Interpreter_DoWord_Default ( interp, prefixFunction, prefixFunction->W_RL_Index, prefixFunction->W_SC_Index ) ;
        SetState ( compiler, ( PREFIX_ARG_PARSING | DOING_A_PREFIX_WORD ), false ) ;
        SetState ( _Debugger_, DBG_INFIX_PREFIX, false ) ;
    }
    return word ;
}

void
_Interpret_Until_Including_Token ( Interpreter * interp, byte * end, byte * delimiters )
{
    byte * token ;
    while ( token = _Lexer_ReadToken ( interp->Lexer0, delimiters ) )
    {
        Interpreter_InterpretAToken ( interp, token, - 1, - 1 ) ;
        if ( String_Equal ( ( char* ) token, end ) ) break ;
    }
}

byte *
_Interpret_Until_NotIncluding_Token ( Interpreter * interp, byte * end, byte * delimiters )
{
    byte * token ;
    while ( token = _Lexer_ReadToken ( interp->Lexer0, delimiters ) )
    {
        if ( String_Equal ( ( char* ) token, end ) ) return token ;
        Interpreter_InterpretAToken ( interp, token, - 1, - 1 ) ;
    }
}

void
Interpret_PrefixFunction_Until_Token ( Interpreter * interp, Word * prefixFunction, byte * end, byte * delimiters )
{
    int64 svscwi = _CSL_->SC_Index ;
    Interpret_Until_Token ( interp, end, delimiters ) ;
    SetState ( _Context_->Compiler0, PREFIX_ARG_PARSING, false ) ;
    if ( prefixFunction ) Interpreter_DoWord_Default ( interp, prefixFunction, - 1, svscwi ) ;
}

void
Interpret_UntilFlagged ( Interpreter * interp, int64 doneFlags )
{
    do Interpreter_InterpretNextToken ( interp ) ;
    while ( ( ! Interpreter_IsDone ( interp, doneFlags ) ) ) ;
}

void
Interpret_ToEndOfLine ( Interpreter * interp )
{
    ReadLiner * rl = interp->ReadLiner0 ;
    do
    {
        Interpreter_InterpretNextToken ( interp ) ;
        if ( GetState ( interp->Lexer0, LEXER_END_OF_LINE ) ) break ; // either the lexer with get a newline or the readLiner
        if ( ReadLine_AreWeAtNewlineAfterSpaces ( rl ) ) break ;
    }
    while ( ( ! Interpreter_IsDone ( interp, END_OF_FILE | END_OF_STRING ) ) ) ;
}

void
Interpret_UntilFlaggedWithInit ( Interpreter * interp, int64 doneFlags )
{
    Interpreter_Init ( interp ) ;
    Interpret_UntilFlagged ( interp, doneFlags ) ;
}

void
Interpret_String ( byte *str )
{
    _CSL_ContextNew_InterpretString ( _CSL_, str ) ;
}

void
_CSL_Interpret ( CSL * csl )
{
    do
    {
        _CSL_Init_SessionCore ( csl, 1, 1 ) ;
        Context_Interpret ( csl->Context0 ) ;
    }
    while ( GetState ( csl, CSL_RUN ) ) ;
}

void
CSL_InterpreterRun ( )
{
    _CSL_Interpret ( _CSL_ ) ;
}

