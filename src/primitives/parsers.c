
#include "../include/csl.h"

void
CSL_Token ( )
{
    byte * token = Lexer_ReadToken ( _Lexer_ ) ;
    DataStack_Push ( ( int64 ) token ) ;
}

void
CSL_TokenQID ( )
{
    byte * token ;
    Word * word ;
    while ( 1 )
    {
        token = Lexer_ReadToken ( _Lexer_ ) ;
        word = _Interpreter_TokenToWord ( _Interpreter_, token, - 1, - 1 ) ;
        if ( word )
        {
            Boolean isForwardDotted = ReadLiner_IsTokenForwardDotted ( _ReadLiner_, word->W_RL_Index ) ;
            if ( ( isForwardDotted ) || ( token[0] == '.' ) ) Word_Eval ( word ) ;
            else break ;
        }
    }
    if ( GetState ( _Lexer_, LEXER_END_OF_LINE ) ) SetState ( _Interpreter_, END_OF_LINE, true ) ; //necessary to update interpreter state since we are pushing the last token
    DataStack_Push ( ( int64 ) token ) ;
}

void
CSL_FilenameToken ( )
{
    byte * token = _Lexer_LexNextToken_WithDelimiters ( _Lexer_, 0, 1, 0, 1, LEXER_ALLOW_DOT ) ;
    DataStack_Push ( ( int64 ) token ) ;
}

void
CSL_SingleQuote ( )
{
    _CSL_SingleQuote ( ) ;
}

void
CSL_Tick ( )
{
    _CSL_SingleQuote ( ) ;
}

void
Parse_SkipUntil_EitherToken_OrNewline ( byte * end1, byte* end2 )
{
    byte * token ;
    int64 inChar ;
    ReadLiner * rl = _Context_->ReadLiner0 ;
    do
    {
        if ( ( token = Lexer_ReadToken ( _Context_->Lexer0 ) ) )
        {
            if ( ( end1 && String_Equal ( token, end1 ) ) || ( end2 && String_Equal ( token, end2 ) ) ) break ;
        }
        inChar = ReadLine_PeekNextChar ( rl ) ;
        if ( ( inChar == 0 ) || ( inChar == - 1 ) || ( inChar == eof ) || ReadLine_AreWeAtNewlineAfterSpaces ( rl ) ) break ;
    }
    while ( token ) ;
}

void
CSL_ParseObject ( )
{
    Lexer * lexer = _Context_->Lexer0 ;
    byte * token = ( byte* ) DataStack_Pop ( ) ;
    Lexer_ParseObject ( lexer, token ) ;
    DataStack_Push ( lexer->Literal ) ;
}

void
CSL_DoubleQuoteMacro ( )
{
    _Lexer_ParseTerminatingMacro ( _Lexer_, '\"', 1 ) ;
}

void
_CSL_Word_ClassStructure_PrintData ( Word * typedefWord, Word * word )
{
    typedefWord = Word_UnAlias ( typedefWord ) ;
    CSL_NewLine () ;
    if ( typedefWord && word ) Word_ClassStructure_PrintData (0, word, typedefWord->W_SourceCode ) ;
}


void
CSL_Word_Name_ClassStructure_PrintData ( )
{
    byte * token = ( byte* ) DataStack_Pop ( ) ;
    Word * typedefWord = Finder_Word_FindUsing (_Finder_, token, 0) ;
    byte * token1 = ( byte* ) DataStack_Pop ( ) ;
    Word * word = Finder_Word_FindUsing (_Finder_, token1, 0) ;
    _CSL_Word_ClassStructure_PrintData ( typedefWord, word ) ;
}


void
CSL_Word_ClassStructure_PrintData ( )
{
    Word * typedefWord = ( Word* ) DataStack_Pop ( ) ;
    Word * word = ( Word* ) DataStack_Pop ( ) ;
    _CSL_Word_ClassStructure_PrintData ( typedefWord, word ) ;
}


