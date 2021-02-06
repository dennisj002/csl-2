
#include "../../include/csl.h"

void
CSL_PreProcessor ( )
{
    Lexer * lexer = _Lexer_ ;
    Interpreter * interp = _Context_->Interpreter0 ;
    int64 svState = GetState ( lexer, ( ADD_TOKEN_TO_SOURCE | ADD_CHAR_TO_SOURCE ) ) ;
    Lexer_SourceCodeOff ( lexer ) ;
    _CSL_UnAppendFromSourceCode_NChars ( _CSL_, 1 ) ; // 1 : '#'
    SetState ( interp, PREPROCESSOR_MODE, true ) ;
    uint64 *svStackPointer = GetDataStackPointer ( ) ;
    byte * token = Lexer_ReadToken ( lexer ) ;
    Word * ppword = _Finder_FindWord_InOneNamespace ( _Finder_, Namespace_Find ( ( byte* ) "PreProcessor" ), token ) ;
    Interpreter_DoWord ( interp, ppword, - 1, - 1 ) ;
    SetState ( interp, PREPROCESSOR_MODE, false ) ;
    if ( Compiling ) SetState ( lexer, ( ADD_TOKEN_TO_SOURCE | ADD_CHAR_TO_SOURCE ), svState ) ;
    SetDataStackPointer ( svStackPointer ) ;
}

Boolean
_GetCondStatus ( )
{
    Context * cntx = _Context_ ;
    Boolean status, svcm = GetState ( cntx->Compiler0, COMPILE_MODE ), svcs = GetState ( cntx, C_SYNTAX ) ;
    SetState ( cntx->Compiler0, COMPILE_MODE, false ) ;
    SetState ( cntx, C_SYNTAX, false ) ;
    SetState ( cntx, CONTEXT_PREPROCESSOR_MODE, true ) ;
    uint64 *svStackPointer = GetDataStackPointer ( ) ;
    Interpret_ToEndOfLine ( cntx->Interpreter0 ) ;
    SetState ( cntx, CONTEXT_PREPROCESSOR_MODE, false ) ;
    SetState ( cntx->Compiler0, COMPILE_MODE, svcm ) ;
    SetState ( cntx, C_SYNTAX, svcs ) ;
    status = DataStack_Pop ( ) ;
    if ( status > 0 ) status = true ;
    SetDataStackPointer ( svStackPointer ) ;
    return status ;
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
    Printf ( "\n\nInputLineString = %s", _ReadLiner_->InputLineString ) ;
    Printf ( "\n%s : Ppibs = 0x%016lx : depth = %d : Filename = %s : Line = %d", prefix, ppibs, depth, ppibs->Filename, ppibs->LineNumber ) ;
    Printf ( "\nIfBlockStatus = %d", ppibs->IfBlockStatus ) ;
    Printf ( "\nElifStatus = %d", ppibs->ElifStatus ) ;
    Printf ( "\nElseStatus = %d", ppibs->ElseStatus ) ;
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
    if ( llen ) //|| GetState ( _Context_, PREPROCESSOR_IF_MODE ))
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
    else _SyntaxError ( ( byte* ) "#elxx without #if", 1 ) ;
    return 0 ;
}

Boolean
GetIfStatus ( )
{
    Ppibs * cstatus = Ppibs_New ( ) ;
    Boolean accStatus, cond ;
    //SetState ( _Context_, PREPROCESSOR_IF_MODE, true ) ;
    accStatus = GetAccumulatedBlockStatus ( 0 ) ;
    cond = _GetCondStatus ( ) ;
    cstatus->IfBlockStatus = cond && accStatus ;
    cstatus->Filename = _ReadLiner_->Filename ;
    cstatus->LineNumber = _ReadLiner_->LineNumber ;
    _List_PushNew_1Value ( _Context_->PreprocessorStackList, CONTEXT, T_PREPROCESSOR, ( int64 ) cstatus ) ;
    dbg ( Ppibs_Print ( cstatus, ( byte* ) "IfStatus" ) ) ;
    //SetState ( _Context_, PREPROCESSOR_IF_MODE, false ) ;
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
                        if ( skipControl == PP_IFDEF ) continue ;
                        if ( skipControl == PP_ELSE ) ifLevel ++ ;
                        else if ( GetIfStatus ( ) ) goto done ; // PP_INTERP
                    }
                    else if ( String_Equal ( token1, "else" ) )
                    {
                        if ( skipControl == PP_ELSE ) continue ;
                        else if ( GetElseStatus ( ) ) goto done ;
                    }
                    else if ( String_Equal ( token1, "elif" ) )
                    {
                        if ( skipControl == PP_ELSE ) continue ;
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
#if 1                    
                    else if ( String_Equal ( token1, "define" ) ) continue ;
                    else if ( String_Equal ( token1, "defined" ) ) continue ;
                    else if ( String_Equal ( token1, "undef" ) ) continue ;
                    else if ( String_Equal ( token1, "ifdef" ) ) continue ;
                    else if ( String_Equal ( token1, "ifndef" ) ) continue ;
                    else if ( String_Equal ( token1, "include" ) ) continue ;
                    else if ( String_Equal ( token1, "error" ) ) continue ;
                    else if ( String_Equal ( token1, "warning" ) ) continue ;
                    else _SyntaxError ( ( byte* ) "Stray '#' in code!", 1 ) ;
#else                    
                    else continue ;
#endif                    
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

void
CSL_IfDef_Preprocessor ( )
{
    if ( ( ! _CSL_Defined ( ) ) && ( ! GetIfStatus ( ) ) ) SkipPreprocessorCode ( PP_IFDEF ) ;
}

void
CSL_Ifndef_Preprocessor ( )
{
    if ( _CSL_Defined ( ) ) SkipPreprocessorCode ( PP_SKIP ) ;
}

void
AddDirectoriesToPreprocessor_IncludeDirectory_List ( )
{
    Namespace * ns = _CSL_->C_Preprocessor_IncludeDirectory_List_Namespace ;
    if ( ! ns ) ns = _CSL_->C_Preprocessor_IncludeDirectory_List_Namespace = _Namespace_New ( "cpidln", 0 ) ;
    _Word_New ( "/usr/local/include/", 0, 0, 0, 0, ns, DICTIONARY ) ;
    _Word_New ( "/usr/include/", 0, 0, 0, 0, ns, DICTIONARY ) ;
}

byte *
CheckFilename ( Symbol * symbol, byte * fname )
{
    byte * fn = ( char* ) Buffer_Data_Cleared ( _CSL_->ScratchB4 ) ;
    strncpy ( fn, symbol->Name, BUFFER_SIZE ) ;
    strncat ( fn, fname, BUFFER_SIZE ) ;
    FILE * file = fopen ( ( char* ) fn, "r" ) ;
    if ( file ) return fn ;
    else return 0 ;
}

int64
_CSL_Defined ( )
{
    byte * token = Lexer_ReadToken ( _Lexer_ ) ;
    Word * w = Finder_Word_FindUsing ( _Finder_, token, 0 ) ;
    d1 ( DataStack_Check ( ) ) ;
    return ( int64 ) w ;
}

void
CSL_Undef ( )
{
    Word * w = ( Word* ) _CSL_Defined ( ) ;
    if ( w ) dlnode_Remove ( ( dlnode* ) w ) ;
}

int64
CSL_Defined ( )
{
    int64 d = _CSL_Defined ( ) ;
    DataStack_Push ( d ) ;
}

void
CSL_Include_PreProcessor ( )
{
    FILE * file ;
    char * _filename = ( char* ) _CSL_FilenameToken ( ), *fn = ( char* ) Buffer_Data_Cleared ( _CSL_->ScratchB5 ), *filename, *afn ;
    strncpy ( fn, _filename, BUFFER_SIZE ) ;
    CSL_CommentToEndOfLine ( ) ; // shouldn't be anything after the filename but if there is ignore it
    if ( fn [0] == '<' )
    {
        if ( ! _CSL_->C_Preprocessor_IncludeDirectory_List_Namespace ) AddDirectoriesToPreprocessor_IncludeDirectory_List ( ) ;
        fn [strlen ( ( char* ) fn ) - 1] = 0 ;
        filename = & fn[1] ;
        dllist * dirl = _CSL_->C_Preprocessor_IncludeDirectory_List_Namespace->W_List ;
        afn = ( byte* ) dllist_Map1_WReturn ( dirl, ( MapFunction1 ) CheckFilename, ( int64 ) filename ) ;
    }
    else afn = filename ;
    CSL_C_Syntax_On ( ) ;
    if ( VERBOSITY > 1 ) Printf ( ( byte* ) "\nEntering : %s at %s", afn, Context_Location ( ) ) ;
    _CSL_ContextNew_IncludeFile ( afn ) ;
}

