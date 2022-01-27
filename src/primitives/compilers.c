#include "../include/csl.h"

void
CSL_Here ( )
{
    DataStack_Push ( ( int64 ) Here ) ;
}

void
CSL_Code ( )
{
    DataStack_Push ( ( int64 ) _O_CodeByteArray ) ;
}

void
CompileCall ( )
{
    Compile_Call_TestRSP ( ( byte* ) DataStack_Pop ( ) ) ;
}

void
Compile_A_CSLWord ( )
{
    _Word_Compile ( ( Word* ) DataStack_Pop ( ) ) ;
}

void
CompileInt64 ( )
{
    _Compile_Int64 ( DataStack_Pop ( ) ) ;
}

void
Compile_Int32 ( )
{
    int64 l = DataStack_Pop ( ) ;
    _Compile_Int32 ( l ) ;
}

void
Compile_Int16 ( )
{
    int64 w = DataStack_Pop ( ) ;
    _Compile_Int16 ( ( short ) w ) ;
}

void
Compile_Int8 ( )
{
    int64 b = DataStack_Pop ( ) ;
    _Compile_Int8 ( b ) ;
}

void
Compile_N_Bytes ( )
{
    int64 size = DataStack_Pop ( ) ;
    byte * data = ( byte* ) DataStack_Pop ( ) ;
    _CompileN ( data, size ) ;
}

GotoInfo *
_GotoInfo_Allocate ( )
{
    GotoInfo * gi = ( GotoInfo * ) Mem_Allocate ( sizeof ( GotoInfo ), COMPILER_TEMP ) ;
    return gi ;
}

void
GotoInfo_Remove ( dlnode * node )
{
    GotoInfo * gi = ( GotoInfo* ) node ;
    dlnode_Remove ( ( dlnode * ) gi ) ;
}

GotoInfo *
GotoInfo_Init ( GotoInfo * gotoInfo, byte * lname, uint64 type )
{
    gotoInfo->pb_LabelName = lname ;
    gotoInfo->pb_JmpOffsetPointer = Here - INT32_SIZE ; // after the jmp/call opcode
    gotoInfo->GI_CAttribute = type ;
    gotoInfo->CompileAtAddress = Here - 5 ; // 5 : size of jmp/call insn
    return gotoInfo ;
}

GotoInfo *
GotoInfo_New ( byte * lname, uint64 type )
{
    GotoInfo * gotoInfo = ( GotoInfo * ) _GotoInfo_Allocate ( ) ;
    GotoInfo_Init ( gotoInfo, lname, type ) ;
    dllist_AddNodeToHead ( _Context_->Compiler0->GotoList, ( dlnode* ) gotoInfo ) ;
    return gotoInfo ;
}

void
_CSL_CompileCallGoto ( byte * name, uint64 type )
{
    if ( ( type == GI_RECURSE ) ) // && (! GetState ( _Context_, C_SYNTAX | PREFIX_MODE | INFIX_MODE ) ) ) // tail call recursion??
    {
        _Compile_UninitializedCall ( ) ;
    }
    else Compile_UninitializedJump ( ) ;
    GotoInfo_New ( name, type ) ;
}

void
_CSL_Goto ( byte * name )
{
    _CSL_CompileCallGoto ( name, GI_GOTO ) ;
}

void
_CSL_GotoLabel ( byte * name )
{
    _CSL_CompileCallGoto ( name, GI_GOTO_LABEL ) ;
}

void
CSL_Goto ( )
{
    _CSL_Goto ( ( byte * ) DataStack_Pop ( ) ) ;
}

void
CSL_Goto_Prefix ( )
{
    byte * gotoToken = Lexer_ReadToken ( _Context_->Lexer0 ) ;
    _CSL_Goto ( gotoToken ) ;
}

void
CSL_Label ( )
{
    _CSL_Label ( ( byte* ) DataStack_Pop ( ) ) ;
}

void
CSL_Label_Prefix ( )
{
    byte * labelToken = Lexer_ReadToken ( _Context_->Lexer0 ) ;
    _CSL_Label ( labelToken ) ;
}

// 'return' is a prefix word now C_SYNTAX or not
// not satisfied yet with how 'return' works with blocks and locals ???

void
CSL_Return ( )
{
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    Word* wordr = CSL_WordList ( 0 ) ; // 'return
    compiler->ReturnWord = wordr ;
    byte * token = Lexer_Peek_Next_NonDebugTokenWord (_Lexer_, 0) ;
    Word * word = Finder_Word_FindUsing ( _Finder_, token, 0 ) ; 
    SetState ( _Compiler_, DOING_RETURN, true ) ;
    CSL_DoReturnWord ( word ) ;
    SetState ( _Compiler_, DOING_RETURN, false ) ;
}

void
CSL_Continue ( )
{
    _CSL_CompileCallGoto ( 0, GI_CONTINUE ) ;
}

void
CSL_Break ( )
{
    _CSL_CompileCallGoto ( 0, GI_BREAK ) ;
}

void
CSL_SetupRecursiveCall ( )
{
    _CSL_CompileCallGoto ( 0, GI_RECURSE ) ;
}

Word *
_CSL_Literal ( )
{
    int64 value = DataStack_Pop ( ) ;
    Word * word = DataObject_New ( LITERAL, 0, ( byte* ) value, 0, LITERAL | CONSTANT, 0, 0, value, 0, STRING_MEMORY, - 1, - 1 ) ;
    word->S_Value = ( int64 ) word->Name ;
    return word ;
}

void
CSL_Literal ( )
{
    Word * word = _CSL_Literal ( ) ;
    Interpreter_DoWord ( _Context_->Interpreter0, word, - 1, - 1 ) ;
    //DataStack_Push ( (int64) word ) ;
}

void
CSL_Constant ( )
{
    Word *tword = 0, *cword ;
    int64 value = DataStack_Pop ( ) ;
    tword = CSL_TypeStack_Pop ( ) ;
    byte * name = ( byte* ) DataStack_Pop ( ) ;
    cword = DataObject_New ( CONSTANT, 0, name, 0, CONSTANT, 0, 0, value, 0, 0, - 1, - 1 ) ;
    if ( tword ) cword->W_ObjectAttributes |= tword->W_ObjectAttributes ;
    CSL_Finish_WordSourceCode ( _CSL_, cword, 0 ) ;
}

void
CSL_Variable ( )
{
    byte * name = ( byte* ) DataStack_Pop ( ) ;
    Word * word = DataObject_New ( NAMESPACE_VARIABLE, 0, name, 0, NAMESPACE_VARIABLE, 0, 0, 0, 0, 0, - 1, - 1 ) ;
    if ( ! Compiling ) CSL_Finish_WordSourceCode ( _CSL_, word, 0 ) ;
}

// "{|" - exit the Compiler start interpreting
// named after the forth word '[' 

void
CSL_LeftBracket ( )
{
    Compiler * compiler = _Compiler_ ;
    SetState ( compiler, COMPILE_MODE, false ) ;
    if ( compiler->SaveOptimizeState ) CSL_OptimizeOn ( ) ;
}

// "|}" - enter the Compiler
// named following the forth word ']'

void
_CSL_RightBracket ( )
{
    Compiler * compiler = _Compiler_ ;
    SetState ( compiler, COMPILE_MODE, true ) ;
    compiler->SaveOptimizeState = GetState ( _CSL_, OPTIMIZE_ON ) ;
}

void
CSL_RightBracket ( )
{
    if ( ! Compiling ) Compiler_Init ( _Compiler_, 0 ) ;
    _CSL_RightBracket ( ) ;
}

void
CSL_AsmModeOn ( )
{
    SetState ( _Context_->Compiler0, ASM_MODE, true ) ;
    SetState ( _Context_, ASM_SYNTAX, true ) ;
}

void
CSL_AsmModeOff ( )
{
    SetState ( _Context_->Compiler0, ASM_MODE, false ) ;
    SetState ( _Context_, ASM_SYNTAX, false ) ;
}

void
CSL_CompileMode ( )
{
    DataStack_Push ( GetState ( _Context_->Compiler0, COMPILE_MODE ) ) ;
}

void
CSL_FinishWordDebugInfo ( )
{
    _CSL_FinishWordDebugInfo ( 0 ) ;
}
