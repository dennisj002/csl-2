
#include "../../include/csl.h"

const int64 MEM_FREE = 0 ;
const int64 MEM_ALLOC = 1 ;
int64 mmap_TotalMemAllocated = 0, mmap_TotalMemFreed = 0 ;

byte*
_mmap_AllocMem ( int64 size )
{
    mmap_TotalMemAllocated += size ;
    return ( byte* ) mmap ( NULL, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, - 1, 0 ) ;
}

void
mmap_FreeMem ( byte * chunk, int64 size )
{
    mmap_TotalMemFreed += size ;
    munmap ( chunk, size ) ;
}

byte *
mmap_AllocMem ( int64 size )
{
    byte * mem = _mmap_AllocMem ( size ) ;
    if ( ( mem == MAP_FAILED ) )
    {
        perror ( "_Mem_Mmap" ) ;
        OVT_ShowMemoryAllocated ( ) ;
        //OVT_Exit ( ) ;
        CSL_FullRestart ( ) ;
    }
    //Kbhit_Pause ( ) ;
    memset ( mem, 0, size ) ; // ?? : is this necessary??
    return mem ;
}

void
MemChunk_Show ( MemChunk * mchunk )
{
    Printf ( ( byte* ) "\nMemChunk : address : 0x%08x : allocType = %8lu : size = %8d", ( uint64 ) mchunk, ( uint64 ) mchunk->S_WAllocType, ( int64 ) mchunk->S_ChunkSize ) ;
}

void
_MemChunk_WithSymbol_Show ( MemChunk * mchunk, int64 flag )
{
    Symbol * sym = ( Symbol * ) ( mchunk + 1 ) ;
    Printf ( ( byte* ) "\n%s : %s : 0x%lld : %d, ", ( flag == MEM_ALLOC ) ? "Alloc" : "Free",
        ( ( int64 ) ( sym->S_Name ) > 0x80000000 ) ? ( char* ) sym->S_Name : "(null)", mchunk->S_WAllocType, mchunk->S_ChunkSize ) ;
}

void
_MemChunk_Account ( MemChunk * mchunk, int64 flag )
{
    if ( _O_ )
    {
        if ( flag == MEM_ALLOC )
        {
            _O_->TotalMemAllocated += mchunk->S_ChunkSize ;
            _O_->Mmap_RemainingMemoryAllocated += mchunk->S_ChunkSize ;
        }
        else
        {
            _O_->TotalMemFreed += mchunk->S_ChunkSize ;
            _O_->Mmap_RemainingMemoryAllocated -= mchunk->S_ChunkSize ;
        }
    }
}

void
_Mem_ChunkFree ( MemChunk * mchunk )
{
    _MemChunk_Account ( mchunk, MEM_FREE ) ;
    dlnode_Remove ( ( dlnode* ) mchunk ) ;
    mmap_FreeMem ( mchunk->S_unmap, mchunk->S_ChunkSize ) ;
}

byte *
_Mem_ChunkAllocate ( int64 size, uint64 allocType )
{
    // a MemChunk is already a part of the size : cf. types.h
    MemChunk * mchunk = ( MemChunk * ) mmap_AllocMem ( size ) ;
    mchunk->S_unmap = ( byte* ) mchunk ;
    mchunk->S_ChunkSize = size ; // S_ChunkSize is the total size of the chunk including any prepended accounting structure in that total
    mchunk->S_WAllocType = allocType ;
    _MemChunk_Account ( ( MemChunk* ) mchunk, MEM_ALLOC ) ;
    dllist_AddNodeToHead ( &_O_->PermanentMemList, ( dlnode* ) mchunk ) ;
    return ( byte* ) mchunk ;
}

ByteArray *
_ByteArray_AppendSpace_MakeSure ( ByteArray * ba, int64 size ) // size in bytes
{
    NamedByteArray * nba = ba->OurNBA ;
    if ( nba )
    {
        while ( ba->MemRemaining < size )
        {
            int64 largestRemaining = 0 ;
            // check the other bas in the nba list to see if any have enough remaining
            {
                dlnode * node, *nodeNext ;
                for ( node = dllist_First ( ( dllist* ) & nba->NBA_BaList ) ; node ; node = nodeNext )
                {
                    if ( node == nodeNext ) break ; // ?? TODO : should need this
                    nodeNext = dlnode_Next ( node ) ;
                    ba = Get_BA_Symbol_To_BA ( node ) ;
                    if ( ba )
                    {
                        if ( ba->MemRemaining > largestRemaining ) largestRemaining = ba->MemRemaining ;
                        if ( ba->MemRemaining >= size ) goto done ;
                    }
                }
            }
            _O_->AllocationRequestLacks ++ ;
            //nba->NBA_DataSize = ( ( ( ++ nba->ReAllocations ) * nba->NBA_DataSize ) + size ) ;
            nba->NBA_DataSize += ( ( ( ++ nba->ReAllocations ) * ( 10 * K ) ) + size ) ;
            //nba->NBA_DataSize = ( ( ( ++ nba->ReAllocations ) * nba->NBA_DataSize ) + size ) ;
            if ( _O_->Verbosity > 3 ) NBA_PrintInfo ( nba ) ;
#if 0            
            if ( ( _O_CodeByteArray == ba ) && Compiling )
            {
                Printf ( ( byte* ) "\nOnly %d code space remaining : \n", nba->LargestRemaining ) ;
                byte * compiledAtAddress = Compile_UninitializedJumpEqualZero ( ) ;
                Stack_Push_PointerToJmpOffset ( compiledAtAddress ) ;
                Pause ( ) ;
            }
#endif            
            ba = _NamedByteArray_AddNewByteArray ( nba, nba->NBA_DataSize ) ;
#if 0            
            if ( ( _O_CodeByteArray == ba ) && Compiling )
            {
                _ByteArray_SetHere ( ba, ba->EndIndex ) ;
                CSL_CalculateAndSetPreviousJmpOffset_ToHere ( ) ;
            }
#endif            
        }
    }
    else Error_Abort ( "_ByteArray_AppendSpace_MakeSure", ( byte* ) "\n_ByteArray_AppendSpace_MakeSure : no nba?!\n" ) ;
done:
    nba->ba_CurrentByteArray = ba ;
    return ba ;
}

//macros.h :  _Allocate( size, nba ) _ByteArray_AppendSpace ( nba->ba_CurrentByteArray, size ) 

byte *
_ByteArray_AppendSpace ( ByteArray * ba, int64 size ) // size in bytes
{
    while ( ba->MemRemaining < size )
    {
        ba = _ByteArray_AppendSpace_MakeSure ( ba, size ) ;
    }
    ba->StartIndex = ba->EndIndex ; // move index to end of the last append
    ba->EndIndex += size ;
    if ( ba->OurNBA ) ba->OurNBA->MemRemaining -= size ; //nb. debugger->StepInstructionBA doesn't have an nba
    ba->MemRemaining -= size ;
    return ba->StartIndex ;
}

byte *
Mem_Allocate ( int64 size, uint64 allocType )
{
    MemorySpace * ms = _O_->MemorySpace0 ;
    switch ( allocType )
    {
        case OPENVMTIL: return _Allocate ( size, ms->OpenVmTilSpace ) ;
        case OBJECT_MEM: return _Allocate ( size, ms->ObjectSpace ) ;
        case INTERNAL_OBJECT_MEM: return _Allocate ( size, ms->InternalObjectSpace ) ;
        case LISP: return _Allocate ( size, ms->LispSpace ) ;
        case TEMPORARY: return _Allocate ( size, ms->TempObjectSpace ) ; // used for SourceCode
        case DICTIONARY: return _Allocate ( size, ms->DictionarySpace ) ;
        case SESSION: return _Allocate ( size, ms->SessionObjectsSpace ) ;
        case CODE: return _Allocate ( size, ms->CodeSpace ) ;
        case BUFFER: return _Allocate ( size, ms->BufferSpace ) ;
        case HISTORY: return _Allocate ( size, ms->HistorySpace ) ;
        case LISP_TEMP: return _Allocate ( size, ms->LispTempSpace ) ;
        case CONTEXT: return _Allocate ( size, ms->ContextSpace ) ;
        case COMPILER_TEMP: return _Allocate ( size, ms->CompilerTempObjectSpace ) ;
        case WORD_RECYCLING: return _Allocate ( size, ms->WordRecylingSpace ) ;
        case T_CSL: case DATA_STACK: return _Allocate ( size, ms->CSLInternalSpace ) ;
        case STRING_MEMORY: return _Allocate ( size, ms->StringSpace ) ;
        case RUNTIME: // not used??
        {
            _O_->RunTimeAllocation += size ;
            return mmap_AllocMem ( size ) ;
        }
        default: CSL_Exception ( MEMORY_ALLOCATION_ERROR, 0, QUIT ) ;
    }
    return 0 ;
}

void
Mem_FreeItem ( dllist * mList, byte * item )
{
    dlnode * node, *nodeNext ;
    for ( node = dllist_First ( ( dllist* ) mList ) ; node ; node = nodeNext )
    {
        MemChunk * mchunk = ( MemChunk* ) node ;
        nodeNext = dlnode_Next ( node ) ;
        if ( ( byte* ) mchunk->S_pb_Data2 == item )
        {
            _Mem_ChunkFree ( mchunk ) ;
            return ;
        }
    }
}

void
FreeChunkList ( dllist * list )
{
    dlnode * node, *nodeNext ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nodeNext )
    {
        nodeNext = dlnode_Next ( node ) ;
        _Mem_ChunkFree ( ( MemChunk* ) node ) ;
    }
}

void
FreeNba_BaNode ( NamedByteArray * nba, dlnode * node )
{
    dlnode_Remove ( node ) ; // remove BA_Symbol from nba->NBA_BaList cf. _NamedByteArray_AddNewByteArray
    MemChunk* mchunk = ( MemChunk* ) ( ( Symbol * ) node )->S_Value ;
    nba->TotalAllocSize -= mchunk->S_ChunkSize ;
    _Mem_ChunkFree ( mchunk ) ;
}

void
FreeNba_BaList ( NamedByteArray * nba )
{
    dllist * list = & nba->NBA_BaList ;
    dlnode * node, *nodeNext ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nodeNext )
    {
        nodeNext = dlnode_Next ( node ) ;
        FreeNba_BaNode ( nba, node ) ;
    }
}

void
NBA_FreeChunkType ( Symbol * s, uint64 allocType, int64 exactFlag )
{
    NamedByteArray * nba = Get_NBA_Symbol_To_NBA ( s ) ;
    if ( exactFlag )
    {
        if ( nba->NBA_AAttribute != allocType ) return ;
    }
    else if ( ! ( nba->NBA_AAttribute & allocType ) ) return ;
    FreeNba_BaList ( nba ) ;
    nba->MemRemaining = 0 ;
    nba->MemAllocated = 0 ;
    _NamedByteArray_AddNewByteArray ( nba, nba->NBA_DataSize ) ;
}

NamedByteArray *
MemorySpace_NBA_New ( MemorySpace * memSpace, byte * name, int64 size, int64 allocType )
{
    NamedByteArray *nba = NamedByteArray_New ( name, size, allocType ) ;
    dllist_AddNodeToHead ( &memSpace->NBAs, ( dlnode* ) & nba->NBA_Symbol ) ;
    return nba ;
}

void
OVT_FreeTempMem ( )
{
    OVT_MemListFree_CompilerTempObjects ( ) ;
    OVT_MemListFree_TempObjects ( ) ;
}

void
MemorySpace_Init ( MemorySpace * ms )
{
    OpenVmTil * ovt = _O_ ;

    ms->OpenVmTilSpace = MemorySpace_NBA_New ( ms, ( byte* ) "OpenVmTilSpace", ovt->OpenVmTilSize, OPENVMTIL ) ;
    ms->CSLInternalSpace = MemorySpace_NBA_New ( ms, ( byte* ) "CSLInternalSpace", ovt->CSLSize, T_CSL ) ;
    ms->ObjectSpace = MemorySpace_NBA_New ( ms, ( byte* ) "ObjectSpace", ovt->ObjectsSize, OBJECT_MEM ) ;
    ms->InternalObjectSpace = MemorySpace_NBA_New ( ms, ( byte* ) "InternalObjectSpace", ovt->InternalObjectsSize, INTERNAL_OBJECT_MEM ) ;
    ms->LispSpace = MemorySpace_NBA_New ( ms, ( byte* ) "LispSpace", ovt->LispSize, LISP ) ;
    ms->TempObjectSpace = MemorySpace_NBA_New ( ms, ( byte* ) "TempObjectSpace", ovt->TempObjectsSize, TEMPORARY ) ;
    ms->CompilerTempObjectSpace = MemorySpace_NBA_New ( ms, ( byte* ) "CompilerTempObjectSpace", ovt->CompilerTempObjectsSize, COMPILER_TEMP ) ;
    ms->WordRecylingSpace = MemorySpace_NBA_New ( ms, ( byte* ) "WordRecylingSpace", ovt->WordRecylingSize, WORD_RECYCLING ) ;
    ms->CodeSpace = MemorySpace_NBA_New ( ms, ( byte* ) "CodeSpace", ovt->MachineCodeSize, CODE ) ;
    ms->SessionObjectsSpace = MemorySpace_NBA_New ( ms, ( byte* ) "SessionObjectsSpace", ovt->SessionObjectsSize, SESSION ) ;
    ms->DictionarySpace = MemorySpace_NBA_New ( ms, ( byte* ) "DictionarySpace", ovt->DictionarySize, DICTIONARY ) ;
    ms->LispTempSpace = MemorySpace_NBA_New ( ms, ( byte* ) "LispTempSpace", ovt->LispTempSize, LISP_TEMP ) ;
    ms->BufferSpace = MemorySpace_NBA_New ( ms, ( byte* ) "BufferSpace", ovt->BufferSpaceSize, BUFFER ) ;
    ms->HistorySpace = MemorySpace_NBA_New ( ms, ( byte* ) "HistorySpace", HISTORY_SIZE, HISTORY ) ;
    ms->StringSpace = MemorySpace_NBA_New ( ms, ( byte* ) "StringSpace", ovt->StringSpaceSize, STRING_MEMORY ) ;

    ms->BufferList = _dllist_New ( OPENVMTIL ) ; // put it here to minimize allocating chunks for each node and the list
    ms->RecycledWordList = _dllist_New ( OPENVMTIL ) ; // put it here to minimize allocating chunks for each node and the list
    ms->RecycledOptInfoList = _dllist_New ( OPENVMTIL ) ; // put it here to minimize allocating chunks for each node and the list

    _O_CodeByteArray = ms->CodeSpace->ba_CurrentByteArray ; //init CompilerSpace ptr

    if ( _O_->Verbosity > 2 ) Printf ( ( byte* ) "\nSystem Memory has been initialized.  " ) ;
}

MemorySpace *
MemorySpace_New ( )
{
    MemorySpace *memSpace = ( MemorySpace* ) mmap_AllocMem ( sizeof ( MemorySpace ) ) ;
    _O_->MemorySpace0 = memSpace ;
    _O_->OVT_InitialUnAccountedMemory += sizeof ( MemorySpace ) ; // needed here because '_O_' was not initialized yet for MemChunk accounting
    dllist_Init ( &memSpace->NBAs, &memSpace->NBAsHeadNode, &memSpace->NBAsTailNode ) ; //= _dllist_New ( OPENVMTIL ) ;
    MemorySpace_Init ( memSpace ) ; // can't be initialized until after it is hooked into it's System

    return memSpace ;
}

NamedByteArray *
_OVT_Find_NBA ( byte * name )
{
    // needs a Word_Find that can be called before everything is initialized
    Symbol * s = DLList_FindName_InOneNamespaceList ( &_O_->MemorySpace0->NBAs, ( byte * ) name ) ;
    if ( s ) return Get_NBA_Symbol_To_NBA ( s ) ;
    else return 0 ;
}

// fuzzy still but haven't yet needed to adjust this one

void
_OVT_MemList_FreeNBAMemory ( NamedByteArray *nba, uint64 moreThan, int64 always )
{
    if ( nba && ( always || ( nba->MemAllocated > ( nba->MemInitial + moreThan ) ) ) ) // this logic is fuzzy ?? what is wanted is a way to fine tune mem allocation 
    {
        dlnode * node, *nodeNext ;
        int64 size, flag ;
        for ( flag = 0, node = dllist_First ( ( dllist* ) & nba->NBA_BaList ) ; node ; node = nodeNext )
        {
            nodeNext = dlnode_Next ( node ) ;
            ByteArray * ba = Get_BA_Symbol_To_BA ( node ) ;
            if ( ba )
            {
                if ( ! flag ++ ) // keep one ba initialized
                {
                    _ByteArray_Init ( ba ) ;
                    nba->ba_CurrentByteArray = ba ;
                    size = ba->BA_DataSize ;
                    nba->MemAllocated = size ;
                    nba->MemRemaining = size ;
                }
                else
                {
                    FreeNba_BaNode ( nba, node ) ;
                    nba->NumberOfByteArrays -- ;
                }
            }
        }
        nba->InitFreedRandMarker = rand ( ) ;
    }
}

void
NamedByteArray_Delete ( NamedByteArray * nba, Boolean reinitFlag )
{
    ByteArray * ba ;
    dlnode * node, *nodeNext ;
    if ( nba )
    {
        dlnode_Remove ( ( dlnode* ) & nba->NBA_Symbol ) ;
        for ( node = dllist_First ( ( dllist* ) & nba->NBA_BaList ) ; node ; node = nodeNext )
        {
            nodeNext = dlnode_Next ( node ) ;
            ba = Get_BA_Symbol_To_BA ( node ) ;
            _Mem_ChunkFree ( ( MemChunk * ) ba ) ;
        }
        if ( ! reinitFlag ) _Mem_ChunkFree ( ( MemChunk * ) nba ) ; // mchunk )

        else _NamedByteArray_Init ( nba, nba->NBA_MemChunk.Name, nba->NBA_DataSize, nba->NBA_AAttribute ) ;
    }
}

void
OVT_MemList_DeleteNBAMemory ( byte * name, Boolean reinitFlag )
{
    NamedByteArray *nba = _OVT_Find_NBA ( name ) ;
    NamedByteArray_Delete ( nba, reinitFlag ) ;
}

void
OVT_MemList_FreeNBAMemory ( byte * name, uint64 moreThan, int64 always )
{
    NamedByteArray *nba = _OVT_Find_NBA ( name ) ;
    _OVT_MemList_FreeNBAMemory ( nba, moreThan, always ) ;
}

void
OVT_MemListFree_ContextMemory ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "ContextSpace", 1 * M, 1 ) ;
}

void
OVT_MemListFree_TempObjects ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "TempObjectSpace", 1 * M, 1 ) ;
}

void
OVT_MemListFree_LispSpace ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "LispSpace", 1 * M, 1 ) ;
}

void
_OVT_MemListFree_WordRecyclingSpace ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "WordRecylingSpace", 0, 1 ) ;
}

void
_OVT_MemListFree_CompilerTempObjectSpace ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "CompilerTempObjectSpace", 0, 1 ) ;
}

void
OVT_MemListFree_CompilerTempObjects ( )
{
    _OVT_MemListFree_CompilerTempObjectSpace ( ) ;
}

void
OVT_MemListFree_LispTemp ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "LispTempSpace", 2 * M, 1 ) ;
}

void
OVT_MemListFree_Session ( )
{

    OVT_MemList_FreeNBAMemory ( ( byte* ) "SessionObjectsSpace", 2 * M, 1 ) ;
}

void
OVT_MemListFree_Buffers ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "BufferSpace", 2 * M, 0 ) ;
}

void
OVT_MemListFree_HistorySpace ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "HistorySpace", 1 * M, 0 ) ;
}

void
_OVT_MemListFree_CSLInternal ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "CSLInternalSpace", 0, 1 ) ;
}

void
_NBAsMemList_FreeTypes ( int64 allocType, int64 exactFlag )
{
    dllist_Map2 ( &_O_->MemorySpace0->NBAs, ( MapFunction2 ) NBA_FreeChunkType, allocType, exactFlag ) ;
}

void
NBAsMemList_FreeExactType ( int64 allocType )
{
    _NBAsMemList_FreeTypes ( allocType, 1 ) ;
}

void
NBAsMemList_FreeVariousTypes ( int64 allocType )
{
    _NBAsMemList_FreeTypes ( allocType, 0 ) ;
}

void
NBA_Show ( NamedByteArray * nba, int64 flag )
{
    byte * name = nba->NBA_Symbol.S_Name ;
    NBA_PrintInfo ( nba ) ;
    if ( flag && ( _O_->Verbosity > 1 ) )
    {
        dlnode * node, *nodeNext ;
        for ( node = dllist_First ( ( dllist* ) & nba->NBA_BaList ) ; node ; node = nodeNext )
        {
            nodeNext = dlnode_Next ( node ) ;
            ByteArray * ba = Get_BA_Symbol_To_BA ( node ) ;
            MemChunk_Show ( &ba->BA_MemChunk ) ;
        }
    }
}

void
NBA_PrintInfo ( NamedByteArray * nba )
{
    byte * name = nba->NBA_Symbol.S_Name ;
    if ( _O_->Verbosity > 1 )
    {
        Printf ( ( byte* ) "\n%-43s InUse = " INT_FRMT_9 " : Unused = " INT_FRMT_9 " : ReAllocations = %4d : Largest = %8d : Smallest = %8d : alloctype = %8lu ",
            name, nba->MemAllocated - nba->MemRemaining, nba->MemRemaining, nba->ReAllocations, nba->LargestRemaining,
            nba->SmallestRemaining, ( uint64 ) nba->NBA_AAttribute ) ;
    }
    else
    {
        Printf ( ( byte* ) "\n%-43s InUse = " INT_FRMT_9 " : Unused = " INT_FRMT_9 " : ReAllocations = %4d : Largest = %8d : Smallest = %8d",
            name, nba->MemAllocated - nba->MemRemaining, nba->MemRemaining, nba->ReAllocations, nba->LargestRemaining, nba->SmallestRemaining ) ;
    }
}

void
NBA_AccountRemainingAndShow ( NamedByteArray * nba, Boolean flag )
{
    byte * name = nba->NBA_Symbol.S_Name ;
    int64 largest = 0, smallest = 0 ;
    nba->MemRemaining = 0 ;
    dlnode * node, *nodeNext ;
    for ( node = dllist_First ( ( dllist* ) & nba->NBA_BaList ) ; node ; node = nodeNext )
    {
        nodeNext = dlnode_Next ( node ) ;
        ByteArray * ba = Get_BA_Symbol_To_BA ( node ) ;
        nba->MemRemaining += ba->MemRemaining ;
        if ( _O_->Verbosity > 1 ) Printf ( ( byte* ) "\n%s : ba = " UINT_FRMT_09 " ba->MemRemaining = " INT_FRMT_9, name, ba, ba->MemRemaining ) ;
        if ( ! largest ) largest = ba->MemRemaining ;
        else if ( ba->MemRemaining > largest ) largest = ba->MemRemaining ;
        if ( ! smallest ) smallest = ba->MemRemaining ;
        else if ( ba->MemRemaining < smallest ) smallest = ba->MemRemaining ;
    }
    nba->LargestRemaining = largest ;
    nba->SmallestRemaining = smallest ;
    if ( flag ) NBA_PrintInfo ( nba ) ;
}

void
OVT_ShowNBAs ( OpenVmTil * ovt, int64 flag )
{
    if ( ovt )
    {
        dlnode * node, *nodeNext ;
        if ( ovt->MemorySpace0 && ( node = dllist_First ( ( dllist* ) & ovt->MemorySpace0->NBAs ) ) )
        {
            for ( ; node ; node = nodeNext )
            {

                nodeNext = dlnode_Next ( node ) ;
                NamedByteArray * nba = Get_NBA_Symbol_To_NBA ( node ) ;
                NBA_Show ( nba, flag ) ;
            }
        }
        printf ( "\n" ) ;
        fflush ( stdout ) ;
    }
}

int64
_OVT_ShowPermanentMemList ( OpenVmTil * ovt )
{
    int64 size ;
    if ( ovt )
    {
        int64 diff ;
        dlnode * node, *nodeNext ;
        if ( ovt->Verbosity > 2 ) printf ( "\nMemChunk List :: " ) ;
        for ( size = 0, node = dllist_First ( ( dllist* ) & ovt->PermanentMemList ) ; node ; node = nodeNext )
        {
            nodeNext = dlnode_Next ( node ) ;
            if ( ovt->Verbosity > 2 ) MemChunk_Show ( ( MemChunk * ) node ) ;
            size += ( ( MemChunk * ) node )->S_ChunkSize ;
        }
        diff = ovt->Mmap_RemainingMemoryAllocated - size ;
        if ( diff ) //|| ovt->Verbosity > 2 )
        {
            printf ( "\nTotal Size = %9ld : ovt->Mmap_RemainingMemoryAllocated = %9ld :: diff = %6ld", size, ovt->Mmap_RemainingMemoryAllocated, diff ) ;
        }
        printf ( "\nMmap_RemainingMemoryAllocated                     = %9ld : <=: ovt->Mmap_RemainingMemoryAllocated", ovt->Mmap_RemainingMemoryAllocated ) ;
        fflush ( stdout ) ;
    }
    ovt->PermanentMemListRemainingAccounted = size ;

    return size ;
}

void
_Calculate_TotalNbaAccountedMemAllocated ( OpenVmTil * ovt, Boolean showFlag )
{
    if ( ovt )
    {
        dlnode * node, * nextNode ;
        NamedByteArray * nba ;
        ovt->TotalNbaAccountedMemRemaining = 0 ;
        ovt->TotalNbaAccountedMemAllocated = 0 ;
        if ( ovt && ovt->MemorySpace0 )
        {
            for ( node = dllist_First ( ( dllist* ) & ovt->MemorySpace0->NBAs ) ; node ; node = nextNode )
            {

                nextNode = dlnode_Next ( node ) ;
                nba = Get_NBA_Node_To_NBA ( node ) ;
                NBA_AccountRemainingAndShow ( nba, showFlag ) ;
                ovt->TotalNbaAccountedMemAllocated += nba->TotalAllocSize ;
                ovt->TotalNbaAccountedMemRemaining += nba->MemRemaining ;
            }
        }
    }
}

void
Calculate_TotalNbaAccountedMemAllocated ( OpenVmTil * ovt, int64 flag )
{
    _Calculate_TotalNbaAccountedMemAllocated ( ovt, flag ) ;
    if ( _CSL_ && _DataStack_ ) // so we can use this function anywhere
    {

        int64 dsu = DataStack_Depth ( ) * sizeof (int64 ) ;
        int64 dsa = ( STACK_SIZE * sizeof (int64 ) ) - dsu ;
        Printf ( ( byte* ) "\nData Stack                                  InUse = %9d : Unused = %9d", dsu, dsa ) ;
    }
    printf ( "\nTotal Accounted Mem                         InUse = %9ld : Unused = %9ld",
        ovt->TotalNbaAccountedMemAllocated - ovt->TotalNbaAccountedMemRemaining, ovt->TotalNbaAccountedMemRemaining ) ;
    fflush ( stdout ) ;
}

void
_OVT_ShowMemoryAllocated ( OpenVmTil * ovt )
{
    Boolean vf = ( ovt->Verbosity > 1 ) ;
    if ( ! vf ) Printf ( ( byte* ) c_gu ( "\nIncrease the verbosity setting to 2 or more for more info here. ( Eg. : verbosity 2 = )" ) ) ;
    int64 leak = ( mmap_TotalMemAllocated - mmap_TotalMemFreed ) - ( ovt->TotalMemAllocated - ovt->TotalMemFreed ) - ovt->OVT_InitialUnAccountedMemory ;
    Calculate_TotalNbaAccountedMemAllocated ( ovt, 1 ) ; //leak || vf ) ;
    _OVT_ShowPermanentMemList ( ovt ) ;
    int64 memDiff2 = ovt->Mmap_RemainingMemoryAllocated - ovt->PermanentMemListRemainingAccounted ;
    byte * memDiff2s = ( byte* ) "\nCurrent Unaccounted Diff (leak?)                 = %9d : <=: ovt->Mmap_RemainingMemoryAllocated - ovt->PermanentMemListRemainingAccounted" ;
    byte * leaks = ( byte* ) "\nleak?                                            = %9d : <=  (mmap_TotalMemAllocated - mmap_TotalMemFreed) - (ovt->TotalMemAllocated - ovt->TotalMemFreed) "
        "\n                                                                    - ovt->OVT_InitialUnAccountedMemory" ;
    if ( memDiff2 ) Printf ( ( byte* ) c_ad ( memDiff2s ), memDiff2 ) ;
    else if ( vf ) Printf ( ( byte* ) c_ud ( memDiff2s ), memDiff2 ) ;
    if ( leak ) Printf ( ( byte* ) c_ad ( leaks ), leak ) ;
    else if ( vf ) Printf ( ( byte* ) c_ud ( leaks ), leak ) ;
    if ( memDiff2 || leak || vf )
    {
        Printf ( ( byte* ) "\nTotalNbaAccountedMemAllocated                    = %9d : <=: ovt->TotalNbaAccountedMemAllocated", ovt->TotalNbaAccountedMemAllocated ) ;
        Printf ( ( byte* ) "\nMem Used - Categorized                           = %9d : <=: ovt->TotalNbaAccountedMemAllocated - ovt->TotalNbaAccountedMemRemaining", ovt->TotalNbaAccountedMemAllocated - ovt->TotalNbaAccountedMemRemaining ) ; //+ ovt->UnaccountedMem ) ) ;
        Printf ( ( byte* ) "\nTotalNbaAccountedMemRemaining                    = %9d : <=: ovt->TotalNbaAccountedMemRemaining", ovt->TotalNbaAccountedMemRemaining ) ;
        Printf ( ( byte* ) "\nMmap_RemainingMemoryAllocated                    = %9d : <=: ovt->Mmap_RemainingMemoryAllocated", ovt->Mmap_RemainingMemoryAllocated ) ;
        Printf ( ( byte* ) "\nPermanentMemListRemainingAccounted               = %9d : <=: ovt->PermanentMemListRemainingAccounted", ovt->PermanentMemListRemainingAccounted ) ; //+ ovt->UnaccountedMem ) ) ;
        Printf ( ( byte* ) "\nTotal Mem Remaining                              = %9d : <=: ovt->TotalMemAllocated - ovt->TotalMemFreed", ovt->TotalMemAllocated - ovt->TotalMemFreed ) ; //+ ovt->UnaccountedMem ) ) ;
        Printf ( ( byte* ) "\nmmap_TotalMemAllocated                           = %9d : <=: mmap_TotalMemAllocated", mmap_TotalMemAllocated ) ; //+ ovt->UnaccountedMem ) ) ;
        Printf ( ( byte* ) "\nmmap_TotalMemFreed                               = %9d : <=: mmap_TotalMemFreed", mmap_TotalMemFreed ) ; //+ ovt->UnaccountedMem ) ) ;
        Printf ( ( byte* ) "\nmmap Total Mem Remaining                         = %9d : <=: mmap_TotalMemAllocated - mmap_TotalMemFreed", mmap_TotalMemAllocated - mmap_TotalMemFreed ) ; //+ ovt->UnaccountedMem ) ) ;
        Printf ( ( byte* ) "\nTotal Mem Allocated                              = %9d : <=: ovt->TotalMemAllocated", ovt->TotalMemAllocated ) ; //+ ovt->UnaccountedMem ) ) ;
        Printf ( ( byte* ) "\nTotal Mem Freed                                  = %9d : <=: ovt->TotalMemFreed", ovt->TotalMemFreed ) ; //+ ovt->UnaccountedMem ) ) ;
        Printf ( ( byte* ) "\nTotal Mem Remaining                              = %9d : <=: ovt->TotalMemAllocated - ovt->TotalMemFreed", ovt->TotalMemAllocated - ovt->TotalMemFreed ) ; //+ ovt->UnaccountedMem ) ) ;
        Printf ( ( byte* ) "\nOVT_InitialUnAccountedMemory                     = %9d : <=: ovt->OVT_InitialUnAccountedMemory", ovt->OVT_InitialUnAccountedMemory ) ; //+ ovt->UnaccountedMem ) ) ;
        Printf ( ( byte* ) "\nTotalMemSizeTarget                               = %9d : <=: ovt->TotalMemSizeTarget", ovt->TotalMemSizeTarget ) ;
    }
    Printf ( ( byte* ) "\nTotal Memory Allocated                            = %9d"
        "\nTotal Memory leaks                                = %9d", ovt->TotalNbaAccountedMemAllocated, leak ) ;
    int64 wordSize = ( sizeof ( Word ) + sizeof ( WordData ) ) ;
    Printf ( ( byte* ) "\nRecycledWordCount :: %5d x %3d bytes : Recycled = %9d", _O_->MemorySpace0->RecycledWordCount,
        wordSize, _O_->MemorySpace0->RecycledWordCount * wordSize ) ;
    Printf ( ( byte* ) "\nWrdInRecycling :: %5d x %3d bytes : InRecycling = %9d", _O_->MemorySpace0->WordsInRecycling,
        wordSize, _O_->MemorySpace0->WordsInRecycling * wordSize ) ;
    Buffer_PrintBuffers ( ) ;
}

// 'mem'

void
OVT_ShowMemoryAllocated ( )
{
    _OVT_ShowMemoryAllocated ( _O_ ) ;
}

#if 0
// 'memAllocated'

void
OVT_Mem_ShowAllocated ( )
{
    _OVT_ShowMemoryAllocated ( _O_ ) ;
    _OVT_ShowPermanentMemList ( _O_ ) ;
    //OVT_ShowNBAs ( _O_, 1 ) ;
}
#endif

void
_OVT_CheckCodeSpaceForRoom ( int64 memDesired )
{
    _O_CodeByteArray = _ByteArray_AppendSpace_MakeSure ( _O_CodeByteArray, memDesired ) ;
}

void
OVT_CheckCodeSpaceForRoom ( )
{
    _OVT_CheckCodeSpaceForRoom ( 4 * K ) ;
}

// only check a first node, if no first node the list is empty

byte *
DLList_CheckRecycledForAllocation ( dllist * list, int64 size )
{
    DLNode * node = 0 ;
    if ( list ) node = ( DLNode* ) dllist_First ( ( dllist* ) list ) ;
    if ( node )
    {
        dlnode_Remove ( ( dlnode* ) node ) ; // necessary else we destroy the list!
        Mem_Clear ( ( byte* ) node, size ) ;
        node->n_InUseFlag = N_IN_USE ;
        return ( byte* ) node ;
    }

    else return 0 ;
}

void
OVT_RecyclingAccounting ( int64 flag )
{
    if ( flag == OVT_RA_RECYCLED )
    {
        _O_->MemorySpace0->RecycledWordCount ++ ;
        _O_->MemorySpace0->WordsInRecycling -- ;
    }
    else

        if ( flag == OVT_RA_ADDED ) _O_->MemorySpace0->WordsInRecycling ++ ;

}

void
OVT_Recycle ( dlnode * anode )
{

    if ( anode ) dllist_AddNodeToTail ( _O_->MemorySpace0->RecycledWordList, anode ) ;
    //_O_->MemorySpace0->WordsInRecycling ++ ;
    OVT_RecyclingAccounting ( OVT_RA_ADDED ) ;
}

// put a word on the recycling list

void
Word_Recycle ( Word * w )
{

    OVT_Recycle ( ( dlnode * ) w ) ;
    w->W_ObjectAttributes &= ~ ( RECYCLABLE_COPY | RECYCLABLE_LOCAL ) ;
}

void
_CheckRecycleWord ( Word * w )
{
    if ( w && ( w->W_ObjectAttributes & ( RECYCLABLE_COPY | RECYCLABLE_LOCAL ) ) )
    {
        //if ( Is_DebugOn ) 

        if ( _O_->Verbosity > 2 ) _Printf ( ( byte* ) "\n_CheckRecycleWord : recycling : %s", w->Name ) ; //, Pause () ;
        Word_Recycle ( w ) ;
    }
}

void
CheckRecycleNamespaceWord ( Node * node )
{

    _CheckRecycleWord ( ( Word * ) node ) ;
}

// check a compiler word list for recycleable words and add them to the recycled word list : _O_->MemorySpace0->RecycledWordList

void
DLList_Recycle_NamespaceList ( dllist * list )
{

    dllist_Map ( list, ( MapFunction0 ) CheckRecycleNamespaceWord ) ;
}

// check a compiler word list for recycleable words and add them to the recycled word list : _O_->MemorySpace0->RecycledWordList

void
DLList_RemoveWords ( dllist * list )
{
    dllist_Map ( list, ( MapFunction0 ) dlnode_Remove ) ;
}

