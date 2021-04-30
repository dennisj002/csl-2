
#include "../../include/csl.h"

void
_Compile_C_Call_1_Arg ( byte* function, int64 arg )
{
    //_Compile_RspReg_Push ( arg ) ;
    _Compile_PushReg ( RDI ) ;
    Compile_MoveImm_To_Reg ( RDI, arg, CELL ) ;
    Compile_Call ( function ) ;
    _Compile_PopToReg ( RDI ) ;
    //_Compile_RspReg_Drop ( ) ;
}

void
_CompileN ( byte * data, int64 size )
{
    ByteArray_AppendCopy ( _O_CodeByteArray, size, data ) ; // size in bytes
}

// nb : can be used by an compiler on
// a code body that has only one RET point

void
_CompileFromUptoRET ( byte * data )
{
    ByteArray_AppendCopyUpToRET ( _O_CodeByteArray, data ) ; // size in bytes
}

// copy block from 'address' to Here

void
_Compile_WordInline ( Word * word ) // , byte * dstAddress )
{
    BI_Block_Copy (0, 0, ( byte* ) word->Definition, word->S_CodeSize, 0 ) ;
}

void
_CompileFromName ( byte * wordName )
{
    Word * word = Finder_Word_FindUsing (_Context_->Finder0, wordName, 0) ;
    // ?? Exception : error message here
    if ( ! word ) _Throw ( QUIT ) ;
    _Word_Compile ( word ) ;
}

void
_CompileFromName_Inline ( byte * wordName )
{
    Word * word = Finder_Word_FindUsing (_Context_->Finder0, wordName, 0) ;
    if ( ! word ) _Throw ( QUIT ) ;
    _Compile_WordInline ( word ) ;
}

void
_AdjustGotoInfo ( dlnode * node, int64 originalAddress )
{
    GotoInfo * gotoInfo = ( GotoInfo* ) node ;
    if ( ( gotoInfo->CompileAtAddress == ( byte* ) originalAddress ) ) //&& (gotoInfo->GI_CAttribute != GI_LABEL )) 
    {
        gotoInfo->CompileAtAddress = Here ;
        gotoInfo->pb_JmpOffsetPointer = Here + 1 ;
        //_CSL_AdjustDbgSourceCodeAddress ( (byte*) originalAddress, Here + 1 ) ;
    }
}

void
AdjustGotoJmpOffsetPointer ( dlnode * node, int64 address )
{
    GotoInfo * gi = ( GotoInfo * ) node ;
    if ( ( gi->pb_JmpOffsetPointer == ( byte* ) address ) ) //&& ( gi->GI_CAttribute != GI_GOTO ) )
    {
        gi->pb_JmpOffsetPointer = Here + 1 ;
        //_CSL_AdjustDbgSourceCodeAddress ( (byte*) address, Here + 1 ) ;
    }
}

void
AdjustLabel ( dlnode * node, int64 address )
{
    GotoInfo * gi = ( GotoInfo * ) node ;
    if ( gi->LabeledAddress == ( byte* ) address )
        gi->LabeledAddress = Here ;
}

void
_GotoInfo_SetAndRemove ( GotoInfo * gotoInfo, byte * address, Boolean removeFlag )
{
    _SetOffsetForCallOrJump (gotoInfo->pb_JmpOffsetPointer, address , JMPI32) ;
    if ( removeFlag ) GotoInfo_Remove ( ( dlnode* ) gotoInfo ) ;
}

void
_InstallGotoPoint_Key ( dlnode * node, int64 blockInfo, int64 key )
{
    Word * word ;
    GotoInfo * gotoInfo = ( GotoInfo* ) node ;
    BlockInfo * bi = ( BlockInfo * ) blockInfo ;
    int64 address = * ( int32* ) gotoInfo->pb_JmpOffsetPointer ;
    if ( ( address == 0 ) || ( key & ( GI_BREAK | GI_RETURN | GI_GOTO | GI_LABEL ) ) ) // if we move a block its recurse offset remains, check if this looks like at real offset pointer
    {
        if ( ( ( gotoInfo->GI_CAttribute & ( GI_GOTO ) ) && ( key & ( GI_GOTO ) ) ) )
            // || ( ( gotoInfo->GI_CAttribute & ( GI_LABEL ) ) && ( key & ( GI_LABEL )) ) )
        {
            Namespace * ns = _Namespace_Find ( ( byte* ) "__labels__", _CSL_->Namespaces, 0 ) ;
            if ( ns && ( word = _Finder_FindWord_InOneNamespace ( _Finder_, ns, gotoInfo->pb_LabelName ) ) )
            {
                GotoInfo * giw = ( GotoInfo * ) word->W_Value ;
                _GotoInfo_SetAndRemove ( gotoInfo, ( byte* ) giw->LabeledAddress, 0 ) ;
            }
        }
        else if ( ( gotoInfo->GI_CAttribute & GI_RETURN ) && ( key & GI_RETURN ) )
        {
            _GotoInfo_SetAndRemove ( gotoInfo, Here, 0 ) ;
        }
        else if ( ( gotoInfo->GI_CAttribute & GI_BREAK ) && ( key & GI_BREAK ) )
        {
            if ( _Context_->Compiler0->BreakPoint )
            {
                _GotoInfo_SetAndRemove ( gotoInfo, _Context_->Compiler0->BreakPoint, 0 ) ;
            }
        }
        else if ( ( gotoInfo->GI_CAttribute & GI_CONTINUE ) && ( key & GI_CONTINUE ) )
        {
            if ( _Context_->Compiler0->ContinuePoint )
            {
                _GotoInfo_SetAndRemove ( gotoInfo, _Context_->Compiler0->ContinuePoint, 0 ) ;
            }
        }
        else if ( ( gotoInfo->GI_CAttribute & GI_RECURSE ) && ( key & GI_RECURSE ) )
        {
            _GotoInfo_SetAndRemove ( gotoInfo, bi->bp_First, 0 ) ;
            //_GotoInfo_SetAndRemove ( gotoInfo, bi->LocalFrameStart, 0 ) ; //can we just make it a jmp to LocalFrameStart if there is only one in a function (tail-call)
        }
        else if ( ( gotoInfo->GI_CAttribute & GI_TAIL_CALL ) && ( key & GI_TAIL_CALL ) )
        {
            _GotoInfo_SetAndRemove ( gotoInfo, bi->AfterLocalFrame, 0 ) ;
        }
    }
}

void
_CheckForGotoPoint ( dlnode * node, int64 key, int64 * status )
{
    GotoInfo * gotoInfo = ( GotoInfo* ) node ;
    if ( gotoInfo->GI_CAttribute & key )
    {
        *status = DONE ;
    }
}

void
_RemoveGotoPoint ( dlnode * node, int64 key, int64 * status )
{
    GotoInfo * gotoInfo = ( GotoInfo* ) node ;
    if ( gotoInfo->GI_CAttribute & key )
    {
        dlnode_Remove ( ( dlnode* ) gotoInfo ) ;
        *status = DONE ;
    }
}

void
_CSL_InstallGotoCallPoints_Keyed ( BlockInfo * bi, int64 key )
{
    dllist_Map2 ( _Context_->Compiler0->GotoList, ( MapFunction2 ) _InstallGotoPoint_Key, ( int64 ) bi, key ) ;
}

#if 0

int64
_CSL_AdjustGotoPoint ( int64 originalAddress )
{
    dllist_Map1 ( _Context_->Compiler0->GotoList, ( MapFunction1 ) _AdjustGotoInfo, originalAddress ) ;
}
#endif

void
CSL_AdjustLabels ( byte * srcAddress )
{
    dllist_Map1 ( _Context_->Compiler0->GotoList, ( MapFunction1 ) AdjustLabel, ( int64 ) ( srcAddress ) ) ;
}

int64
CSL_CheckForGotoPoints ( int64 key ) // compile time
{
    int64 status = 0 ;
    dllist_Map_OnePlusStatus ( _Context_->Compiler0->GotoList, ( MapFunction2 ) _CheckForGotoPoint, key, &status ) ;
    return status ;
}

int64
CSL_RemoveGotoPoints ( int64 key ) // compile time
{
    int64 status = 0 ;
    dllist_Map_OnePlusStatus ( _Context_->Compiler0->GotoList, ( MapFunction2 ) _RemoveGotoPoint, key, &status ) ;
    return status ;
}

