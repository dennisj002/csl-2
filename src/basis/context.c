
#include "../include/csl.h"

void
_Context_Prompt ( int64 control )
{
    if ( ( control && ( ! IS_INCLUDING_FILES ) ) || ( GetState ( _Debugger_, DBG_ACTIVE ) ) ) CSL_DoPrompt ( ) ;
}

byte *
_Context_Location ( Context * cntx )
{
    byte * str = 0 ;
    if ( cntx && cntx->ReadLiner0 )
    {
        byte * buffer = Buffer_Data ( _CSL_->StringB ) ;
        snprintf ( ( char* ) buffer, BUF_IX_SIZE, "%s : %ld.%ld", ( char* ) cntx->ReadLiner0->Filename ? ( char* ) cntx->ReadLiner0->Filename : "<command line>", cntx->ReadLiner0->LineNumber, cntx->Lexer0->CurrentReadIndex ) ;
        str = cntx->Location = String_New ( buffer, TEMPORARY ) ;
    }
    return str ;
}

byte *
Context_Location ( )
{
    return _Context_Location ( _Context_ ) ;
}

Word *
_Context_CurrentWord ( Context * cntx )
{
    return cntx->CurrentlyRunningWord ? cntx->CurrentlyRunningWord : _Context_->CurrentEvalWord ? _Context_->CurrentEvalWord : _Context_->LastRanWord ? _Context_->LastRanWord : cntx->CurrentTokenWord ;
}

Word *
Context_CurrentWord ( )
{
    return _Context_CurrentWord ( _Context_ ) ;
}

Context *
_Context_Allocate ( )
{
    NBA * nba = MemorySpace_NBA_New ( _O_->MemorySpace0, ( byte* ) String_New ( ( byte* ) "ContextSpace", STRING_MEM ), 10 * K, OPENVMTIL ) ;
    _O_->MemorySpace0->ContextSpace = nba ;
    Context * cntx = ( Context* ) Mem_Allocate ( sizeof ( Context ), OPENVMTIL ) ;
    cntx->ContextNba = nba ;
    return cntx ;
}

void
Context_SetDefaultTokenDelimiters ( Context * cntx, byte * delimiters, uint64 allocType )
{
    cntx->DefaultDelimiterCharSet = CharSet_New ( delimiters, allocType ) ;
    cntx->DefaultTokenDelimiters = delimiters ;
}

Context *
_Context_Init ( Context * cntx0, Context * cntx )
{
    if ( cntx0 && cntx0->System0 ) cntx->System0 = System_Copy ( cntx0->System0, CONTEXT ) ; // nb : in this case System is copied -- DataStack is shared
    else cntx->System0 = System_New ( CONTEXT ) ;
    List_Init ( _CSL_->Compiler_N_M_Node_WordList ) ;
    Context_SetDefaultTokenDelimiters ( cntx, ( byte* ) " \n\r\t", CONTEXT ) ;
    cntx->Interpreter0 = Interpreter_New ( CONTEXT ) ;
    cntx->Lexer0 = cntx->Interpreter0->Lexer0 ;
    cntx->ReadLiner0 = cntx->Interpreter0->ReadLiner0 ;
    //cntx->Lexer0->OurInterpreter = cntx->Interpreter0 ;
    cntx->Finder0 = cntx->Interpreter0->Finder0 ;
    cntx->Compiler0 = cntx->Interpreter0->Compiler0 ;
    cntx->PreprocessorStackList = _dllist_New ( CONTEXT ) ;
    return cntx ;
}

Context *
_Context_New ( CSL * csl )
{
    Context * cntx = _Context_Allocate ( ), *cntx0 = csl->Context0 ;
    _Context_ = csl->Context0 = cntx ;
    _Context_Init ( cntx0, cntx ) ;
    cntx->ContextDataStack = csl->DataStack ; // nb. using the same one and only DataStack
    return cntx ;
}

void
_Context_Run_1 ( Context * cntx, ContextFunction_1 contextFunction, byte * arg )
{
    contextFunction ( cntx, arg ) ;
}

void
_Context_Run_2 ( Context * cntx, ContextFunction_2 contextFunction, byte * arg, int64 arg2 )
{
    contextFunction ( cntx, arg, arg2 ) ;
}

void
_Context_Run ( Context * cntx, ContextFunction contextFunction )
{
    contextFunction ( cntx ) ;
}

Context *
CSL_Context_PushNew ( CSL * csl )
{
    _Stack_Push ( csl->ContextDataStack, ( int64 ) csl->Context0 ) ;
    Context * cntx = _Context_New ( csl ) ;
    return cntx ;
}

void
CSL_Context_PopDelete ( CSL * csl )
{
    NBA * cnba = csl->Context0->ContextNba ;
    Context * cntx = ( Context* ) _Stack_Pop ( csl->ContextDataStack ) ;
    //Compiler_DeleteDebugInfo ( cntx->Compiler0 ) ;
    _Context_ = csl->Context0 = cntx ;
    _O_->MemorySpace0->ContextSpace = cntx->ContextNba ;
    NamedByteArray_Delete ( cnba, 0 ) ;
}

void
_CSL_Contex_NewRun_1 ( CSL * csl, ContextFunction_1 contextFunction, byte *arg )
{
    Context * cntx = CSL_Context_PushNew ( csl ) ;
    _Context_Run_1 ( cntx, contextFunction, arg ) ;
    CSL_Context_PopDelete ( csl ) ; // this could be coming back from wherever so the stack variables are gone
}

void
_CSL_Contex_NewRun_2 ( CSL * csl, ContextFunction_2 contextFunction, byte *arg, int64 arg2 )
{
    Context * cntx = CSL_Context_PushNew ( csl ) ;
    _Context_Run_2 ( cntx, contextFunction, arg, arg2 ) ;
    CSL_Context_PopDelete ( csl ) ; // this could be coming back from wherever so the stack variables are gone
}

void
_CSL_Contex_NewRun_Void ( CSL * csl, Word * word )
{
    if ( word )
    {
        CSL_Context_PushNew ( csl ) ;
        Block_Eval ( word->Definition ) ;
        CSL_Context_PopDelete ( csl ) ; // this could be coming back from wherever so the stack variables are gone
    }
}

void
_Context_InterpretString ( Context * cntx, byte *str )
{
    Interpreter * interp = cntx->Interpreter0 ;
    ReadLiner * rl = cntx->ReadLiner0 ;
    _SetEcho ( 0 ) ;
    int64 interpState = interp->State ;
    int64 lexerState = interp->Lexer0->State ;
    Readline_Setup_OneStringInterpret ( rl, str ) ;
    Interpret_UntilFlaggedWithInit ( cntx->Interpreter0, END_OF_STRING ) ;
    Readline_Restore_InputLine_State ( rl ) ;
    interp->Lexer0->State = lexerState ;
    interp->State = interpState ;
}

void
_CSL_ContextNew_InterpretString ( CSL * csl, byte * str )
{
    if ( str ) _CSL_Contex_NewRun_1 ( csl, _Context_InterpretString, str ) ;
}

void
_Context_InterpretFile ( Context * cntx )
{
    if ( GetState ( _Debugger_, DBG_AUTO_MODE ) )
    {
        _CSL_DebugContinue ( 0 ) ;
    }
    else Interpret_UntilFlaggedWithInit ( cntx->Interpreter0, END_OF_FILE | END_OF_STRING ) ;
}

void
_Context_IncludeFile ( Context * cntx, byte *filename, int64 interpretFlag )
{
    if ( filename )
    {
        FILE * file = fopen ( ( char* ) filename, "r" ) ;
        if ( file )
        {
            ReadLiner * rl = cntx->ReadLiner0 ;
            rl->Filename = String_New ( filename, STRING_MEM ) ;
            if ( _O_->Verbosity > 2 ) Printf ( ( byte* ) "\nincluding %s ...\n", filename ) ;
            cntx->ReadLiner0->InputFile = file ;
            ReadLine_SetRawInputFunction ( rl, ReadLine_GetNextCharFromString ) ;
            SetState ( cntx->System0, ADD_READLINE_TO_HISTORY, false ) ;
            cntx->System0->IncludeFileStackNumber ++ ;
            _SetEcho ( 0 ) ;

            ReadLine_ReadFileIntoAString ( rl, file ) ;
            fclose ( file ) ;

            if ( interpretFlag ) Interpret_UntilFlaggedWithInit ( cntx->Interpreter0, END_OF_FILE | END_OF_STRING ) ;

            cntx->System0->IncludeFileStackNumber -- ;
            if ( _O_->Verbosity > 2 ) Printf ( ( byte* ) "\n%s included\n", filename ) ;
            OVT_MemList_FreeNBAMemory ( ( byte* ) "ObjectSpace", 1 * M, 1 ) ; // not able to do this yet ??
        }
        else
        {
            Printf ( ( byte* ) "\nError : _CSL_IncludeFile : \"%s\" : not found! :: %s\n", filename,
                _Context_Location ( ( Context* ) _CSL_->ContextDataStack->StackPointer [0] ) ) ;
        }
    }
}

void
_CSL_ContextNew_IncludeFile ( byte * filename )
{
    _CSL_Contex_NewRun_2 ( _CSL_, _Context_IncludeFile, filename, 1 ) ;
}

int64
_Context_StringEqual_PeekNextToken ( Context * cntx, byte * check, Boolean evalFlag )
{
    byte *token = Lexer_Peek_Next_NonDebugTokenWord ( cntx->Lexer0, evalFlag, 0 ) ;
    if ( token ) return String_Equal ( ( char* ) token, ( char* ) check ) ;
    else return 0 ;
}

void
Context_Interpret ( Context * cntx )
{
    Interpret_UntilFlaggedWithInit ( cntx->Interpreter0, END_OF_LINE | END_OF_FILE | END_OF_STRING ) ;
}

byte *
Context_IsInFile ( Context * cntx )
{
    return cntx->ReadLiner0->Filename ;
}

void
Qid_Save_Set_InNamespace ( Namespace * ns )
{
    Compiler_Save_Qid_BackgroundNamespace ( _Compiler_ ) ;
    _CSL_Namespace_InNamespaceSet ( ns ) ; //Word_UnAlias ( ns ) ) ;
}

void
CSL_Set_QidInNamespace ( Namespace * ns )
{
    //_CSL_Namespace_QidInNamespaceSet ( ns ) ; //Word_UnAlias ( ns ) ) ;
    _CSL_Namespace_QidInNamespaceSet ( ns ) ; //Word_UnAlias ( ns ) ) ;
}

void
Context_DoDotted_Pre ( Context * cntx, Word * ns )
{
    int64 isForwardDotted, isReverseDotted ;
    isReverseDotted = Lexer_IsTokenReverseDotted ( cntx->Lexer0 ) ;
    isForwardDotted = ReadLiner_IsTokenForwardDotted ( _ReadLiner_, cntx->Lexer0->TokenEnd_ReadLineIndex - 1 ) ; //word->W_RL_Index ) ;
    if ( ( isForwardDotted ) ) //&& Is_NamespaceType ( ns ) )
    {
        CSL_Set_QidInNamespace ( ns ) ;
        Finder_SetQualifyingNamespace ( cntx->Finder0, ns ) ;
        if ( ( ! isReverseDotted ) || ( ! cntx->BaseObject ) ) cntx->BaseObject = ns ;
    }
    SetState ( cntx, IS_FORWARD_DOTTED, isForwardDotted ) ;
    SetState ( cntx, IS_REVERSE_DOTTED, isReverseDotted ) ;
}

void
Context_DoDotted_Post ( Context * cntx )
{
    if ( ! GetState ( cntx, IS_FORWARD_DOTTED ) )
    {
        if ( ! GetState ( cntx->Compiler0, ( LC_ARG_PARSING | ARRAY_MODE ) ) ) cntx->BaseObject = 0 ; // nb! very important !! 
        if ( GetState ( cntx, IS_REVERSE_DOTTED ) ) 
        {
            Context_ClearQualifyingNamespace () ;
            Context_ClearQidInNamespace () ;
            SetState ( _Context_, (IS_REVERSE_DOTTED|IS_FORWARD_DOTTED), false ) ;
        }
    }
}



