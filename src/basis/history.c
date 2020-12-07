
#include "../include/csl.h"

HistoryStringNode *
HistoryStringNode_New ( byte * hstring )
{
    HistoryStringNode * hsn = ( HistoryStringNode * ) Mem_Allocate ( sizeof ( HistoryStringNode ), STATIC ) ;
    _Symbol_Init_AllocName ( ( Symbol* ) hsn, hstring, STATIC ) ; // use Name for history string
    //hsn->S_CAttribute = HISTORY_NODE ;
    return hsn ;
}

HistoryStringNode *
HistorySymbolList_Find ( byte * hstring )
{
    HistoryStringNode * hsn = 0 ;
    dlnode * node, * nextNode ;
#if 1   
    for ( node = dllist_First ( ( dllist* ) _O_->HistorySpace_StringList ) ; node ; node = nextNode ) // index = dlnode_NextNode ( &_Q->HistoryList, (dlnode *) index ) )
    {
        nextNode = dlnode_Next ( node ) ;
        hsn = ( HistoryStringNode* ) node ;
        if ( ( hsn->S_Name ) && ( String_Equal ( ( char* ) hsn->S_Name, ( char* ) hstring ) ) )
        {
            return hsn ;
        }
    }
#else // some work towards eliminating the StringList and just using the MemList
    for ( node = dllist_First ( ( dllist* ) _O_->HistorySpace_StringList.MemList ) ; node ; node = nextNode ) // index = dlnode_NextNode ( &_Q->HistoryList, (dlnode *) index ) )
    {
        nextNode = dlnode_Next ( node ) ;
        hsn = ( HistoryStringNode* ) ( ( MemChunk * ) node + 1 ) ;
        if ( ( hsn->S_Name ) && ( String_Equal ( ( char* ) hsn->S_Name, ( char* ) hstring ) ) )
        {
            return hsn ;
        }
    }
#endif    
    return 0 ;
}

void
ReadLine_ShowHistoryNode ( ReadLiner * rl )
{
    rl->EscapeModeFlag = 0 ;
    if ( rl->HistoryNode && rl->HistoryNode->S_Name )
    {
        byte * dst = Buffer_Data ( _CSL_->ScratchB1 ) ;
        dst = _String_ConvertStringToBackSlash ( dst, rl->HistoryNode->S_Name, - 1 ) ;
        _ReadLine_PrintfClearTerminalLine ( ) ;
        __ReadLine_DoStringInput ( rl, String_FilterMultipleSpaces ( dst, TEMPORARY ), rl->AltPrompt ) ;
        ReadLine_SetCursorPosition ( rl, rl->EndPosition ) ;
    }
    else
    {
        ReadLine_ClearCurrentTerminalLine ( rl, rl->EndPosition ) ; // nb : this is also part of ShowString
        ReadLine_ShowNormalPrompt ( rl ) ;
    }
    _ReadLine_CursorToEnd ( rl ) ;
}

void
_OpenVmTil_AddStringToHistoryList ( byte * istring )
{
    HistoryStringNode * hsn ;
    if ( istring && strcmp ( ( char* ) istring, "" ) ) // don't add blank lines to history
    {
        //Buffer * buffer = Buffer_New ( BUFFER_SIZE ) ;
        byte * nstring = Buffer_Data ( _CSL_->ScratchB1 ) ;
        nstring = _String_ConvertStringToBackSlash ( nstring, istring, - 1 ) ;

        hsn = HistorySymbolList_Find ( nstring ) ;
        if ( ! hsn )
        {
            hsn = HistoryStringNode_New ( nstring ) ;
        }
        else dlnode_Remove ( ( dlnode* ) hsn ) ; // make it last with dllist_AddNodeToTail
        dllist_AddNodeToTail ( _O_->HistorySpace_StringList, ( dlnode* ) hsn ) ; //
        d0 ( int64 ll = List_Length ( _O_->HistorySpace_StringList ) ) ;
        dllist_SetCurrentNode_After ( _O_->HistorySpace_StringList ) ; // ! properly set Object.dln_Node
        //Buffer_SetAsUnused ( buffer ) ;
    }
}

void
OpenVmTil_AddStringToHistory ( )
{
    byte * string = ( byte* ) DataStack_Pop ( ) ;
    _OpenVmTil_AddStringToHistoryList ( string ) ;
}

void
OpenVmTil_AddStringToHistoryOn ( )
{
    SetState ( _Context_->ReadLiner0, ADD_TO_HISTORY, true ) ;
}

void
OpenVmTil_AddStringToHistoryOff ( )
{
    SetState ( _Context_->ReadLiner0, ADD_TO_HISTORY, false ) ;
}

#if 0

void
HistorySpace_Delete ( )
{
    //MemList_FreeExactType ( HISTORY ) ;
    OVT_MemListFree_HistorySpace ( ) ;
}

void
_HistorySpace_Init ( OpenVmTil * ovt )
{
    if ( ovt )
    {
        if ( ! ovt->HistorySpace_StringList )
        {
#if 0        
            MemorySpace * ms = ovt->MemorySpace0 ;
            ms->HistorySpace = MemorySpace_NBA_New ( ms, ( byte* ) "HistorySpace", HISTORY_SIZE, HISTORY ) ;
#endif        
            //ovt->HistorySpace_StringList.StringList = & ovt->HistorySpace_StringList._StringList ;
            //dllist_Init ( &ovt->HistorySpace_StringList.StringList, ovt->HistorySpace_StringList._StringList_HeadNode, ovt->HistorySpace_StringList._StringList_TailNode ) ;
            ovt->HistorySpace_StringList = _dllist_New ( HISTORY ) ;
            //ovt->HistorySpace_StringList.HistorySpaceNBA = ovt->HistorySpace ;
            //if ( reset ) _NamedByteArray_Init ( _O_->HistorySpace_StringList.HistorySpaceNBA, ( byte* ) "HistorySpace", HISTORY_SIZE, HISTORY ) ;
        }
    }
}

void
_HistorySpace_New ( OpenVmTil * ovt, int64 resetFlag )
{
    if ( resetFlag )
    {
        HistorySpace_Delete ( ) ;
    }
    _HistorySpace_Init ( ovt ) ;
}

void
HistorySpace_Reset ( void )
{
    _HistorySpace_New ( _O_, 1 ) ;
}
#endif

void
History_Init ( )
{
    dllist_Init ( &HistoryMemChunkList, &hml_Head, &hml_Tail ) ;
    _O_->HistorySpace = MemorySpace_NBA_OvtNew ( ( byte* ) "HistorySpace", HISTORY_SIZE, STATIC ) ;
    _O_->HistorySpace_StringList = _dllist_New ( STATIC ) ;
}