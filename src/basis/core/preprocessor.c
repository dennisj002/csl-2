
#include "../../include/csl.h"

#define PP_ELIF 2
#define PP_ELSE 1
void
CSL_PreProcessor ( )
{
    Lexer * lexer = _Lexer_ ;
    Interpreter * interp = _Context_->Interpreter0 ;
    int64 svState = GetState ( lexer, ( ADD_TOKEN_TO_SOURCE | ADD_CHAR_TO_SOURCE ) ) ;
    Lexer_SourceCodeOff ( lexer ) ;
    _CSL_UnAppendFromSourceCode_NChars ( _CSL_, 1 ) ; // 1 : '#'
    //Finder_SetNamedQualifyingNamespace ( _Finder_, ( byte* ) "PreProcessor" ) ;
    SetState ( interp, PREPROCESSOR_MODE, true ) ;
    byte * token = Lexer_ReadToken ( lexer ) ;
    Word * ppword = _Finder_FindWord_InOneNamespace ( _Finder_, Namespace_Find ( ( byte* ) "PreProcessor" ), token ) ;
    Interpreter_DoWord ( interp, ppword, -1, -1 ) ;
    //Interpreter_InterpretNextToken ( interp ) ;
    SetState ( interp, PREPROCESSOR_MODE, false ) ;
    if ( Compiling ) SetState ( lexer, ( ADD_TOKEN_TO_SOURCE | ADD_CHAR_TO_SOURCE ), svState ) ;
}

Ppibs *
Ppibs_Init ( Ppibs * ppibs )
{
    memset ( ppibs, 0, sizeof (Ppibs ) ) ;
    return ppibs ;
}

Ppibs *
Ppibs_New ( )
{
    Ppibs * ppibs = ( Ppibs * ) Mem_Allocate ( sizeof (Ppibs ), CONTEXT ) ;
    //Ppibs_Init ( ppibs ) ;
    return ppibs ;
}

#define DEBUG_SAVE_DONT_DELETE 0
#if DEBUG_SAVE_DONT_DELETE
#define dbg( x ) x

void
Ppibs_Print ( Ppibs * ppibs, byte * prefix )
{
    int64 depth = List_Length ( _Context_->PreprocessorStackList ) ;
    Printf ( ( byte* ) "\n\nInputLineString = %s", _ReadLiner_->InputLineString ) ;
    Printf ( ( byte* ) "\n%s : Ppibs = 0x%016lx : depth = %d : Filename = %s : Line = %d", prefix, ppibs, depth, ppibs->Filename, ppibs->LineNumber ) ;
    Printf ( ( byte* ) "\nIfBlockStatus = %d", ppibs->IfBlockStatus ) ;
    Printf ( ( byte* ) "\nElifStatus = %d", ppibs->ElifStatus ) ;
    Printf ( ( byte* ) "\nElseStatus = %d", ppibs->ElseStatus ) ;
    //Pause () ;
}
#else 
#define dbg( x )
#endif
/* preprocessor BNF :
 *  ppBlock      =:=     #if (elifBlock)* (elseBlock)? #endif
 *  elifBlock    =:=     #elif (ppBlock)*
 *  elseBlock    =:=     #else (ppBlock)*
 */
// "#if" stack pop is 'true' interpret until "#else" and this does nothing ; if stack pop 'false' skip to "#else" token skip those tokens and continue interpreting

Boolean
GetAccumulatedBlockStatus ( int listStartIndex )
{
    int64 status = 0, i, llen = List_Length ( _Context_->PreprocessorStackList ) ;
    if ( llen )
    {
        Ppibs *ppibSstatus ;
        for ( i = listStartIndex ; i <= ( llen - 1 ) ; i ++ ) // -1: 0 based list
        {
            ppibSstatus = ( Ppibs * ) List_Pick_Value ( _Context_->PreprocessorStackList, i ) ;
            status = status || ( ppibSstatus->IfBlockStatus || ppibSstatus->ElifStatus || ppibSstatus->ElseStatus ) ;
            if ( ! status ) break ;
        }
    }
    return llen ? status : 1 ; // 1 : default status
}

int64
GetElxxStatus ( int64 cond, int64 type )
{
    int64 llen = List_Length ( _Context_->PreprocessorStackList ) ;
    if ( llen )
    {
        Ppibs *top = ( Ppibs * ) List_Top_Value ( _Context_->PreprocessorStackList ) ;
        Boolean status = false, accStatus = GetAccumulatedBlockStatus ( 1 ) ; // 1 ??
        if ( type == PP_ELIF )
        {
            if ( ( top->ElseStatus || top->ElifStatus ) ) status = 0 ; // no 'elif 1' after an 'elif 1' or and 'else' in the same block
            else
            {
                if ( llen > 1 )
                {
                    if ( accStatus ) status = ( ! ( top->IfBlockStatus || top->ElseStatus ) ) && cond ;
                    else status = 0 ;
                }
                else status = ( ! ( top->IfBlockStatus || top->ElseStatus ) ) && cond ;
            }
            top->ElifStatus = status ;
            top->ElseStatus = 0 ; // normally elif can't come after else but we make reasonable (?) sense of it here
        }
        else if ( type == PP_ELSE )
        {
            if ( llen > 1 )
            {
                if ( accStatus ) status = ( ! ( top->IfBlockStatus || top->ElifStatus ) ) ;
                else status = 0 ;
            }
            else status = ( ! ( top->IfBlockStatus || top->ElifStatus ) ) ;
            top->ElseStatus = status ;
            top->ElifStatus = 0 ; //status ; // so total block status will be the 'else' status
        }
        List_SetTop_Value ( _Context_->PreprocessorStackList, ( int64 ) top ) ;
        dbg ( Ppibs_Print ( top, ( byte* ) ( ( type == PP_ELSE ) ? "Else : ElxxStatus: top of PreprocessorStackList" : "Elif : ElxxStatus" ) ) ) ;
        return status ;
    }
    else _SyntaxError ( ( byte* ) "#Elxx without #if", 1 ) ; 
    return 0 ;
}

Boolean
_GetCondStatus ( )
{
    Context * cntx = _Context_ ;
    Boolean status, svcm = GetState ( cntx->Compiler0, COMPILE_MODE ), svcs = GetState ( cntx, C_SYNTAX ) ;
    SetState ( cntx->Compiler0, COMPILE_MODE, false ) ;
    SetState ( cntx, C_SYNTAX, false ) ;
    SetState ( cntx, CONTEXT_PREPROCESSOR_MODE, true ) ;
    Interpret_ToEndOfLine ( cntx->Interpreter0 ) ;
    SetState ( cntx, CONTEXT_PREPROCESSOR_MODE, false ) ;
    SetState ( cntx->Compiler0, COMPILE_MODE, svcm ) ;
    SetState ( cntx, C_SYNTAX, svcs ) ;
    status = DataStack_Pop ( ) ;
    if ( status > 0 ) status = true ;
    return status ;
}

Boolean
GetIfStatus ( )
{
    Ppibs * cstatus = Ppibs_New ( ) ;
    Boolean accStatus, cond ;
    accStatus = GetAccumulatedBlockStatus ( 0 ) ;
    cond = _GetCondStatus ( ) ;
    cstatus->IfBlockStatus = cond && accStatus ;
    cstatus->Filename = _ReadLiner_->Filename ;
    cstatus->LineNumber = _ReadLiner_->LineNumber ;
    //_List_PushNew_1Value ( dllist *list, int64 type, int64 value, int64 allocType )
    _List_PushNew_1Value (_Context_->PreprocessorStackList, CONTEXT , T_PREPROCESSOR, ( int64 ) cstatus) ; //SESSION ) ;
    dbg ( Ppibs_Print ( cstatus, ( byte* ) "IfStatus" ) ) ;
    return cstatus->IfBlockStatus ;
}

Boolean
GetElifStatus ( )
{
    int64 cond = _GetCondStatus ( ) ;
    return GetElxxStatus ( cond, PP_ELIF ) ;
}

Boolean
GetElseStatus ( )
{
    return GetElxxStatus ( 0, PP_ELSE ) ;
}

Boolean
_GetEndifStatus ( )
{
    Boolean status = GetAccumulatedBlockStatus ( 0 ) ;
    return status ;
}

Boolean
GetEndifStatus ( )
{
    List_Pop ( _Context_->PreprocessorStackList ) ;
    Boolean status = _GetEndifStatus ( ) ;
    return status ;
}

void
SkipPreprocessorCode ( Boolean skipControl )
{
    Context * cntx = _Context_ ;
    Lexer * lexer = cntx->Lexer0 ;
    byte * token ;
    int64 ifLevel = 0 ;
    int64 svState = GetState ( lexer, ( ADD_TOKEN_TO_SOURCE | ADD_CHAR_TO_SOURCE ) ) ;
    Lexer_SourceCodeOff ( lexer ) ;
    do
    {
        int64 inChar = ReadLine_PeekNextChar ( cntx->ReadLiner0 ) ;
        if ( ( inChar == - 1 ) || ( inChar == eof ) )
        {
            SetState ( lexer, LEXER_END_OF_LINE, true ) ;
            goto done ;
        }
        token = Lexer_ReadToken ( lexer ) ;
        if ( token )
        {
            if ( String_Equal ( token, "//" ) )
            {
                CSL_CommentToEndOfLine ( ) ;
                Lexer_SourceCodeOff ( lexer ) ;
            }
            else if ( String_Equal ( token, "/*" ) )
            {
                CSL_ParenthesisComment ( ) ;
                Lexer_SourceCodeOff ( lexer ) ;
            }
            else if ( String_Equal ( token, "#" ) )
            {
                byte * token1 = Lexer_ReadToken ( lexer ) ;
                if ( token1 )
                {
                    if ( String_Equal ( token1, "if" ) )
                    {
                        if ( skipControl == PP_ELSE ) ifLevel ++ ;
                        else if ( GetIfStatus ( ) ) goto done ; // PP_INTERP
                    }
                    else if ( String_Equal ( token1, "else" ) )
                    {
                        if ( skipControl == PP_ELSE ) continue ;
                            //else if ( ( skipControl == 2 ) && ( ! ifLevel ) ) goto done ;
                        else if ( GetElseStatus ( ) ) goto done ;
                    }
                    else if ( String_Equal ( token1, "elif" ) )
                    {
                        if ( skipControl == PP_ELSE ) continue ;
                            //else if ( ( skipControl == 2 ) && ( ! ifLevel ) ) goto done ;
                        else if ( GetElifStatus ( ) ) goto done ;
                    }
                    else if ( String_Equal ( token1, "endif" ) )
                    {
                        if ( skipControl == PP_ELSE )
                        {
                            if ( -- ifLevel < 0 )
                            {
                                List_Pop ( _Context_->PreprocessorStackList ) ;
                                goto done ;
                            }
                        }
                        else if ( GetEndifStatus ( ) ) goto done ;
                    }
                    else if ( String_Equal ( token1, "define" ) ) continue ;
                    else _SyntaxError ( ( byte* ) "Stray '#' in code!", 1 ) ;
                }
                else goto done ;
            }
        }
    }
    while ( token ) ;
done:
    if ( Compiling ) SetState ( lexer, ( ADD_TOKEN_TO_SOURCE | ADD_CHAR_TO_SOURCE ), svState ) ;
}

void
CSL_If_ConditionalInterpret ( )
{
    if ( ! GetIfStatus ( ) ) SkipPreprocessorCode ( PP_SKIP ) ;
}

void
CSL_Elif_ConditionalInterpret ( )
{
    if ( ! GetElifStatus ( ) ) SkipPreprocessorCode ( PP_ELIF ) ;
}

void
CSL_Else_ConditionalInterpret ( )
{
    if ( ! GetElseStatus ( ) ) SkipPreprocessorCode ( PP_ELSE ) ;
}

void
CSL_Endif_ConditionalInterpret ( )
{
    if ( ! GetEndifStatus ( ) ) SkipPreprocessorCode ( PP_SKIP ) ;
}

