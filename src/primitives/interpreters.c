#include "../include/csl.h"

void
CSL_DoWord ( )
{
    Word * word = ( Word* ) DataStack_Pop ( ) ;
    Interpreter_DoWord ( _Context_->Interpreter0, word, - 1, - 1 ) ;
}

void
CSL_CommentToEndOfLine ( )
{
    Lexer * lexer = _Lexer_ ;
    _CSL_UnAppendTokenFromSourceCode ( _CSL_, lexer->OriginalToken ) ;
    int64 svState = GetState ( lexer, ( ADD_TOKEN_TO_SOURCE | ADD_CHAR_TO_SOURCE ) ) ;
    Lexer_SourceCodeOff ( _Lexer_ ) ;
    ReadLiner_CommentToEndOfLine ( _Context_->ReadLiner0 ) ;
    String_RemoveEndWhitespace ( _CSL_->SC_Buffer ) ;
    _CSL_SC_ScratchPadIndex_Init ( _CSL_ ) ;
    SetState ( lexer, LEXER_END_OF_LINE, true ) ;
    //Lexer_SourceCodeOn ( _Lexer_ ) ;
    //if ( Compiling ) 
    SetState ( lexer, ( ADD_TOKEN_TO_SOURCE | ADD_CHAR_TO_SOURCE ), svState ) ;
}

void
CSL_ParenthesisComment ( )
{
    Lexer * lexer = _Lexer_ ;
    _CSL_UnAppendTokenFromSourceCode ( _CSL_, lexer->OriginalToken ) ;
    int64 svState = GetState ( lexer, ( ADD_TOKEN_TO_SOURCE | ADD_CHAR_TO_SOURCE ) ) ;
    Lexer_SourceCodeOff ( lexer ) ;
    while ( 1 )
    {
        int64 inChar = ReadLine_PeekNextChar ( lexer->ReadLiner0 ) ;
        if ( (!inChar) || ( inChar == - 1 ) || ( inChar == eof ) ) break ;
        char * token = ( char* ) Lexer_ReadToken ( lexer ) ;
        if ( token && ( strcmp ( token, "*/" ) == 0) ) return ;
    }
    if ( Compiling ) SetState ( lexer, ( ADD_TOKEN_TO_SOURCE | ADD_CHAR_TO_SOURCE ), svState ) ;
}

void
CSL_Define ( )
{
    Context * cntx = _Context_ ;
    Interpreter * interp = cntx->Interpreter0 ;
    SetState ( interp, PREPROCESSOR_DEFINE, true ) ;
    CSL_Colon ( ) ;
    Interpret_ToEndOfLine ( interp ) ;
    int64 locals = Compiler_IsFrameNecessary ( _Compiler_ ) ;
    SetState ( interp, PREPROCESSOR_DEFINE, false ) ;
    CSL_SemiColon ( ) ;
    if ( locals ) CSL_Prefix ( ) ; // if we have local variables make it a prefix word ; maybe : if ( GetState ( _Context_, C_SYNTAX ) ) 
    else
    {
        Word * word = _CSL_->LastFinished_Word ;
        if ( word ) word->W_ObjectAttributes |= (LITERAL|CONSTANT) ;
    }
    CSL_Inline ( ) ;
    CSL_SourceCode_Init ( ) ; //don't leave the define in sc
}

void
CSL_Interpreter_IsDone ( )
{
    DataStack_Push ( GetState ( _Context_->Interpreter0, END_OF_FILE | END_OF_STRING | INTERPRETER_DONE ) ) ;
}

void
CSL_Interpreter_Done ( )
{
    SetState ( _Context_->Interpreter0, INTERPRETER_DONE, true ) ;
}

void
CSL_Interpreter_Init ( )
{
    Interpreter_Init ( _Context_->Interpreter0 ) ;
}

void
CSL_InterpretNextToken ( )
{
    Interpreter_InterpretNextToken ( _Context_->Interpreter0 ) ;
}

void
CSL_Interpret ( )
{
    _Context_InterpretFile ( _Context_ ) ;
}

void
CSL_InterpretPromptedLine ( )
{
    CSL_DoPrompt ( ) ;
    //Context_Interpret ( CSL->Context0 ) ;
    Interpret_ToEndOfLine ( _Interpreter_ ) ;
}

void
CSL_InterpretString ( )
{
    Interpret_String ( ( byte* ) DataStack_Pop ( ) ) ;
}

void
CSL_Interpreter_EvalWord ( )
{

    Interpreter_DoWord ( _Context_->Interpreter0, ( Word* ) DataStack_Pop ( ), - 1, - 1 ) ;
}

void
CSL_TokenToWord ( )
{
    byte * token = ( byte* ) DataStack_Pop ( ) ;
    DataStack_Push ( ( int64 ) _Interpreter_TokenToWord ( _Context_->Interpreter0, token, - 1, - 1 ) ) ;
}

void
CSL_InterpreterStop ( )
{
    SetState ( _Context_->Interpreter0, INTERPRETER_DONE, true ) ;
    SetState ( _CSL_, CSL_RUN, false ) ;
}

dllist *
_CSL_Interpret_ReadToList ( )
{
    byte * token ;
    Interpreter * interp = _Context_->Interpreter0 ;
    interp->InterpList = List_New ( SESSION ) ;
    while ( token = Lexer_ReadToken ( _Lexer_ ) )
    {
        if ( String_Equal ( token, ";l" ) ) break ;
        Word * word = _Interpreter_TokenToWord ( interp, token, - 1, - 1 ) ;
        if ( word )
        {

            _Word_Interpret ( word ) ;
            //List_Push_A_1Value_Node ( interp->InterpList, word ) ;
            List_PushNew_T_WORD ( interp->InterpList, ( int64 ) word, COMPILER_TEMP ) ;
        }
    }
    return interp->InterpList ;
}

void
CSL_Interpret_ReadToList ( )
{

    dllist * interpList = _CSL_Interpret_ReadToList ( ) ;
    DataStack_Push ( ( int64 ) interpList ) ;
}

void
CSL_Interpret_List ( )
{
    dllist * interpList = ( dllist* ) DataStack_Pop ( ) ;
    List_Interpret ( interpList ) ;
}

#if 0
#include "/usr/local/include/python3.7m/Python.h"

void
CSL_Interpret_Python ( )
{
    byte * pstring = ( byte* ) DataStack_Pop ( ) ;
    wchar_t *program = Py_DecodeLocale ( "python3.7", NULL ) ;
    Py_SetProgramName ( program ) ; /* optional but recommended */
    Py_Initialize ( ) ;
    PyRun_SimpleString ( pstring ) ;
    PyMem_RawFree ( program ) ;
}
#endif
