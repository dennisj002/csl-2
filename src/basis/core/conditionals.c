#include "../../include/csl.h"

void
Match_MapFunction ( dlnode * node, uint64 switchValue )
{
    CaseNode * cnode = ( CaseNode* ) node ;
    //if ( Is_DebugOn ) Printf ( (byte*) "\nswitchValue = 0x%lx : cnode->CN_CaseUint64Value = 0x%lx", switchValue, cnode->CN_CaseUint64Value ) ;
    //if (( Is_DebugOn ) && ( IsString ( ( byte* ) switchValue ) && IsString ( ( byte* ) cnode->CN_CaseBytePtrValue ) ))
    //    Printf ( (byte*) "\nswitchValue = %s : cnode->CN_CaseBytePtrValue = %s", switchValue, cnode->CN_CaseBytePtrValue ) ;
    if ( IsString ( ( byte* ) switchValue ) && IsString ( ( byte* ) cnode->CN_CaseBytePtrValue )
        && String_Equal ( cnode->CN_CaseBytePtrValue, ( byte* ) switchValue ) )
    {
        ( ( block ) ( cnode->CN_CaseBlock ) ) ( ) ;
    }
    else if ( switchValue == cnode->CN_CaseUint64Value ) ( ( block ) ( cnode->CN_CaseBlock ) ) ( ) ;
}

void
MatchAccessFunction ( )
{
    dllist_Map1 ( ( dllist* ) TOS, ( MapFunction1 ) Match_MapFunction, _Dsp_ [ - 1 ] ) ;
    DataStack_DropN ( 2 ) ;
}

CaseNode *
_CaseNode_New ( uint64 type, block block, int64 value )
{
    CaseNode * cnode = ( CaseNode* ) Mem_Allocate ( sizeof ( CaseNode ), type ) ;
    cnode->CN_CaseBlock = ( uint64 ) block ;
    cnode->CN_CaseUint64Value = value ; //nb. CN_CaseUint64Value is member of a union 
    return cnode ;
}

void
_CSL_Switch ( uint64 allocType )
{
    if ( ! _Compiler_->CurrentMatchList ) _Compiler_->CurrentMatchList = _dllist_New ( allocType ) ;
    if ( CompileMode )
    {
        _Do_LiteralValue ( ( int64 ) _Compiler_->CurrentMatchList ) ;
        Compile_Call_TestRSP ( ( byte* ) MatchAccessFunction ) ;
    }
    else
    {
        dllist_Map1 ( _Compiler_->CurrentMatchList, ( MapFunction1 ) Match_MapFunction, TOS ) ;
        DataStack_DropN ( 1 ) ;
    }
    _Compiler_->CurrentMatchList = 0 ; // this allows no further "case"s to be added to this "switch" list a new list will be started with the next "case"
}

void
_CS_Case ( uint64 allocType )
{
    Interpreter * interp = _Interpreter_ ;
    int64 caseValue ;
    byte * token = Lexer_ReadToken ( _Lexer_ ) ;
    Word * word = _Interpreter_TokenToWord ( interp, token, - 1, - 1 ) ;
    SetState_TrueFalse ( _Compiler_, DOING_CASE, COMPILE_MODE ) ;
    if ( ( token[0] == '\'' ) || ( token[0] == '"' ) )
    {
        Interpreter_DoWord ( interp, word, - 1, - 1 ) ;
        caseValue = ( uint64 ) String_New ( ( byte* ) DataStack_Pop ( ), STRING_MEM ) ;
    }
    else caseValue = word->S_Value ;
    SetState ( _Compiler_, ( COMPILE_MODE ), true ) ;
    CSL_Interpret_C_Blocks ( 1, 0, 0 ) ;
    block caseBlock = ( block ) DataStack_Pop ( ) ; //TOS ;
    if ( ! _Compiler_->CurrentMatchList ) _Compiler_->CurrentMatchList = _dllist_New ( allocType ) ;
    CaseNode * cnode = _CaseNode_New ( allocType, caseBlock, caseValue ) ;
    dllist_AddNodeToTail ( _Compiler_->CurrentMatchList, ( dlnode* ) cnode ) ;
    SetState ( _Compiler_, DOING_CASE, false ) ;
}

void
_CS_Match ( uint64 allocType )
{
    Interpreter * interp = _Interpreter_ ;
    Word * word ;
    byte * token ;
    if ( ! _Compiler_->CurrentMatchList ) _Compiler_->CurrentMatchList = _dllist_New ( allocType ) ;
    token = Lexer_ReadToken ( interp->Lexer0 ) ;
    word = _Interpreter_TokenToWord ( interp, token, - 1, - 1 ) ;
    if ( GetState ( _Context_, C_SYNTAX ) ) CSL_Interpret_C_Blocks ( 1, 0, 0 ) ;
    Interpreter_DoWord ( interp, word, - 1, - 1 ) ;
    _Do_LiteralValue ( ( int64 ) _Compiler_->CurrentMatchList ) ;
    Compile_Call_TestRSP ( ( byte* ) MatchAccessFunction ) ;
    if ( GetState ( _Context_, C_SYNTAX ) ) DataStack_DropN ( 1 ) ;
    _Compiler_->CurrentMatchList = 0 ; // this allows no further "case"s to be added to this "switch" list a new list will be started with the next "case"
}

void
CSL_Switch ( )
{
    _CSL_Switch ( DICTIONARY ) ;
}

void
CS_Case ( )
{
    _CS_Case ( DICTIONARY ) ;
}

void
CS_Match ( )
{
    _CS_Match ( DICTIONARY ) ;
}

#if 0
void
_CSL_Case ( uint64 allocType )
{
    block caseBlock = ( block ) DataStack_Pop ( ) ;
    int64 caseValue = ( uint64 ) String_New ( ( byte* ) DataStack_Pop ( ), STRING_MEM ) ;
    if ( ! _Compiler_->CurrentMatchList ) _Compiler_->CurrentMatchList = _dllist_New ( allocType ) ;
    CaseNode * cnode = _CaseNode_New ( allocType, caseBlock, caseValue ) ;
    dllist_AddNodeToTail ( _Compiler_->CurrentMatchList, ( dlnode* ) cnode ) ;
}

void
CSL_Case ( )
{
    _CSL_Case ( DICTIONARY ) ;
}
#endif




