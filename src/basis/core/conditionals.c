#include "../../include/csl.h"

CaseNode *
_CaseNode_New ( uint64 type, block block, int64 value )
{
    CaseNode * cnode = ( CaseNode* ) Mem_Allocate ( sizeof ( CaseNode ), type ) ;
    cnode->CN_CaseBlock = ( uint64 ) block ;
    cnode->CN_CaseUint64Value = value ; //nb. CN_CaseUint64Value is member of a union 
    return cnode ;
}

// ( q n -- )

void
_CSL_Case ( uint64 allocType )
{
    block caseBlock = ( block ) TOS ;
    int64 caseValue = ( uint64 ) String_New ( ( byte* ) NOS, STRING_MEM ) ;
    DataStack_DropN ( 2 ) ;
    if ( ! _Compiler_->CurrentMatchList ) _Compiler_->CurrentMatchList = _dllist_New ( allocType ) ;
    CaseNode * cnode = _CaseNode_New ( allocType, caseBlock, caseValue ) ;
    dllist_AddNodeToTail ( _Compiler_->CurrentMatchList, ( dlnode* ) cnode ) ;
}

void
CSL_Case ( )
{
    _CSL_Case ( DICTIONARY ) ;
}

void
Switch_MapFunction ( dlnode * node, uint64 switchValue )
{
    CaseNode * cnode = ( CaseNode* ) node ;
    if ( String_Equal ( cnode->CN_CaseBytePtrValue, ( byte* ) switchValue ) )
        ( ( block ) ( cnode->CN_CaseBlock ) ) ( ) ;
}

void
MatchAccessFunction ( )
{
    dllist_Map1 ( ( dllist* ) TOS, ( MapFunction1 ) Switch_MapFunction, _Dsp_ [ - 1 ] ) ;
    DataStack_DropN ( 2 ) ;
}

void
CSL_Match ( )
{
    if ( CompileMode )
    {
        _Do_LiteralValue ( ( int64 ) _Compiler_->CurrentMatchList ) ;
        Compile_Call_TestRSP ( ( byte* ) MatchAccessFunction ) ;
    }
    else
    {
        dllist_Map1 ( _Compiler_->CurrentMatchList, ( MapFunction1 ) Switch_MapFunction, TOS ) ;
        DataStack_DropN ( 1 ) ;
    }
    _Compiler_->CurrentMatchList = 0 ; // this allows no further "case"s to be added to this "switch" list a new list will be started with the next "case"
}

