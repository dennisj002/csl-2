
#include "../include/csl.h"
#define VERSION ((byte*) "0.910.610" ) 

// inspired by :: Foundations of Mathematical Logic [Foml] by Haskell Curry, 
// CT/Oop (Category Theory, Object Oriented Programming, Type Theory), 
// Formal Language Theory (Panini, Chomsky) 
// C/C++/C#, Lisp, RPN/Lag : Reverse Polish Notation, (Left Associative Grammar), 
// State Machines, Push Down Automata (PDA), Turing Machines :: 
// Also Laws of Form, by G.S. Brown, Kurt Goedel, Quantum Field Theory, ACIM, etc.
// csl : context sensitive language
// til : a toolkit for implementing languages (maybe even a compiler compiler) based on these ideas,

OpenVmTil * _O_ ;

int
main ( int argc, char * argv [ ] )
{
    openvmtil ( argc, argv ) ;
}

void
openvmtil ( int64 argc, char * argv [ ] )
{
    LinuxInit ( ) ;
    OpenVmTil_Run ( argc, argv ) ;
}

void
OpenVmTil_Run ( int64 argc, char * argv [ ] )
{
    OpenVmTil * ovt ;
    int64 restartCondition = INITIAL_START, restarts = 0, sigSegvs = 0 ;
    while ( 1 )
    {
        if ( _O_ )
        {
            sigSegvs = _O_->SigSegvs ;
            restarts = ++ _O_->Restarts ;
            if ( ovt->Restarts > 20 ) OVT_Exit ( ) ;
        }
        ovt = _O_ = OpenVmTil_New ( _O_, argc, argv ) ;
        OVT_SetRestartCondition ( ovt, restartCondition ) ;
        ovt->SigSegvs = sigSegvs ;
        ovt->Verbosity = 1 ;
        ovt->Restarts = restarts ;
        if ( ovt->Restarts ) OVT_ExceptionState_Print ( ) ;
        if ( ! sigsetjmp ( ovt->JmpBuf0, 0 ) ) // nb. siglongjmp always comes to beginning of the block 
            CSL_Run ( ovt->OVT_CSL, ovt->RestartCondition ) ;
        restartCondition = ovt->RestartCondition ;
        OVT_SetRestartCondition ( ovt, restartCondition ) ;
    }
}

OpenVmTil *
_OpenVmTil_Allocate ( )
{
    OpenVmTil * ovt = ( OpenVmTil* ) Mem_Allocate ( sizeof ( OpenVmTil ), STATIC ) ; // ( mmap_AllocMem ( sizeof ( OpenVmTil ) ) ; //_Mem_Allocate ( 0, sizeof ( OpenVmTil ), 0, ( RETURN_CHUNK_HEADER ) ) ; // don't add this to mem alloc system ; ummap it when done
    ovt->PermanentMemChunkList = _dllist_New ( STATIC ) ;
    ovt->MemorySpaceList = _dllist_New ( STATIC ) ;
    ovt->NBAs = _dllist_New ( STATIC ) ;
    return ovt ;
}

void
_OpenVmTil_Init ( OpenVmTil * ovt, int64 resetHistory )
{
    ovt->MemorySpace0 = MemorySpace_StaticMem_Allocate ( ) ; // nb : memory must be after we set Size values and before lists; list are allocated from memory
    ovt->OpenVmTilSpace = MemorySpace_NBA_OvtNew ( ( byte* ) "OpenVmTilSpace", ovt->OpenVmTilSize, OPENVMTIL ) ;
    ovt->CSLInternalSpace = MemorySpace_NBA_OvtNew ( ( byte* ) "CSLInternalSpace", ovt->CSLSize, T_CSL ) ;
    ovt->InternalObjectSpace = MemorySpace_NBA_OvtNew ( ( byte* ) "InternalObjectSpace", ovt->InternalObjectsSize, INTERNAL_OBJECT_MEM ) ;
    if ( ! ovt->HistorySpace ) History_Init ( ) ;
    ovt->BufferList = _dllist_New ( OPENVMTIL ) ; // put it here to minimize allocating chunks for each node and the list
    ovt->RecycledWordList = _dllist_New ( OPENVMTIL ) ; // put it here to minimize allocating chunks for each node and the list
    ovt->RecycledOptInfoList = _dllist_New ( OPENVMTIL ) ; // put it here to minimize allocating chunks for each node and the list
    MemorySpace_New ( ovt, "DefaultMemorySpace" ) ;
    ovt->VersionString = VERSION ;
    // ? where do we want the init file ?
    if ( _File_Exists ( ( byte* ) "./init.csl" ) )
    {
        ovt->InitString = ( byte* ) "\"./init.csl\" _include" ; // could allow override with a startup parameter
        SetState ( ovt, OVT_IN_USEFUL_DIRECTORY, true ) ;
    }
    else
    {
        ovt->InitString = ( byte* ) "\"/usr/local/lib/csl/init.csl\" _include" ; // could allow override with a startup parameter
        SetState ( ovt, OVT_IN_USEFUL_DIRECTORY, false ) ;
    }
    if ( ovt->Verbosity > 1 )
    {
        Printf ( ( byte* ) "\nRestart : All memory freed, allocated and initialized as at startup. "
            "termios, verbosity and memory category allocation sizes preserved. verbosity = %d.", ovt->Verbosity ) ;
        OpenVmTil_Print_DataSizeofInfo ( 0 ) ;
    }
    ovt->ThrowBuffer = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    _OpenVmTil_ColorsInit ( ovt ) ;
}

OpenVmTil *
OpenVmTil_New ( OpenVmTil * ovt, int64 argc, char * argv [ ] )
{
    int64 restartCondition, startedTimes = 0 ; //, startIncludeTries
    NBA * historySpace = ( ovt ? ovt->HistorySpace : 0 ) ;
    dllist * hsStringList = 0 ;
    dlnode * hsCurrentNode = 0 ;
    ByteArray * hscba = 0 ;
    if ( historySpace )
    {
        hsStringList = ovt->HistorySpace_StringList ;
        hsCurrentNode = hsStringList->l_CurrentNode ;
        hscba = historySpace->ba_CurrentByteArray ;
    }
    if ( ! ovt ) restartCondition = INITIAL_START ;
    else restartCondition = FULL_RESTART ;

    if ( ovt ) startedTimes = ovt->StartedTimes ;
    OpenVmTil_Delete ( ovt ) ;
    _O_ = ovt = _OpenVmTil_Allocate ( ) ;

    OVT_SetRestartCondition ( ovt, restartCondition ) ;
    ovt->Argc = argc ;
    ovt->Argv = argv ;
    ovt->StartedTimes = startedTimes ;

    OVT_GetStartupOptions ( ovt ) ;
    int64 allocSize = 200 * K ;
    ovt->InternalObjectsSize = allocSize ;
    ovt->ObjectsSize = 2 * allocSize ; //1 * M ; 
    ovt->LispSize = allocSize ;
    ovt->LispTempSize = allocSize ;
    ovt->CompilerTempObjectsSize = 2 * allocSize ;
    ovt->BufferSpaceSize = allocSize ; //35 * ( sizeof ( Buffer ) + BUFFER_SIZE ) ;
    ovt->MachineCodeSize = allocSize ;
    ovt->StringSpaceSize = allocSize ;
    ovt->DictionarySize = 1 * M ; //100 * K ;
    ovt->CSLSize = ( 80 * K ) ;
    ovt->OpenVmTilSize = ( 6 * K ) ;
    ovt->DataStackSize = 8 * KB ;
    ovt->TempObjectsSize = 200 * K ; //COMPILER_TEMP_OBJECTS_SIZE ;
    ovt->WordRecylingSize = 1 * K * ( sizeof (Word ) + sizeof (WordData ) ) ; //50 * K ; //COMPILER_TEMP_OBJECTS_SIZE ;
    ovt->SessionObjectsSize = 50 * K ;
    if ( historySpace )
    {
        ovt->HistorySpace = historySpace ;
        ovt->HistorySpace->ba_CurrentByteArray = hscba ;
        if ( hsStringList )
        {
            hsStringList->l_CurrentNode = hsCurrentNode ;
            ovt->HistorySpace_StringList = hsStringList ;
        }
    }
    _OpenVmTil_Init ( ovt, 0 ) ; //exceptionsHandled > 1 ) ; // try to keep history if we can
    Linux_SetupSignals ( &ovt->JmpBuf0, 1 ) ;
    return ovt ;
}

void
Ovt_RunInit ( OpenVmTil * ovt )
{
    ovt->StartedTimes ++ ;
    OVT_SetRestartCondition ( ovt, CSL_RUN_INIT ) ;
}

void
OpenVmTil_Delete ( OpenVmTil * ovt )
{
    if ( ovt )
    {
        FreeChunkList ( ovt->PermanentMemChunkList ) ;
        if ( ovt->Verbosity > 2 ) Printf ( ( byte* ) "\nAll allocated, non-static memory has just been freed.\nRestart : verbosity = %d.", ovt->Verbosity ) ;
    }
    _O_ = 0 ;
}

void
OVT_PrintStartupOptions ( OpenVmTil * ovt )
{
    Printf ( ( byte* ) "\n\nOVT_GetStartupOptions :: ovt->Argv [0] = %s\n\n", ovt->Argv [0] ) ;
    Printf ( ( byte* ) "\n\nOVT_GetStartupOptions :: ovt->Argv [1] = %s\n\n", ovt->Argv [1] ) ;
    Printf ( ( byte* ) "\n\nOVT_GetStartupOptions :: ovt->Argv [2] = %s\n\n", ovt->Argv [2] ) ;
    Printf ( ( byte* ) "\n\nOVT_GetStartupOptions :: ovt->StartupFilename = %s\n\n", ovt->StartupFilename ) ;
    //if ( ovt->Verbosity > 1 ) Pause ( ) ;
}

void
OVT_GetStartupOptions ( OpenVmTil * ovt )
{
    int64 i ;
    byte * arg ;
    for ( i = 0 ; i < ovt->Argc ; i ++ )
    {
        arg = ovt->Argv [ i ] ;
        if ( String_Equal ( "-m", arg ) ) ovt->TotalMemSizeTarget = ( atoi ( ovt->Argv [ ++ i ] ) * MB ) ;
            // -s : a script file with "#! csl -s" -- as first line includes the script file, the #! whole line is treated as a comment
        else if ( String_Equal ( "-f", arg ) || ( String_Equal ( "-s", arg ) ) ) ovt->StartupFilename = ( byte* ) ovt->Argv [ ++ i ] ;
        else if ( String_Equal ( "-e", arg ) ) ovt->StartupString = ( byte* ) ovt->Argv [ ++ i ] ;
    }
}

// only partially working ??

void
OVT_RecycleAllWordsDebugInfo ( )
{
    SetState ( _CSL_, ( RT_DEBUG_ON | GLOBAL_SOURCE_CODE_MODE ), false ) ;
    Tree_Map_Namespaces ( _CSL_->Namespaces->W_List, ( MapSymbolFunction ) CSL_DeleteWordDebugInfo ) ;
    _OVT_MemListFree_WordRecyclingSpace ( ) ;
    OVT_FreeTempMem ( ) ;
    _CSL_->CSL_N_M_Node_WordList = _dllist_New ( T_CSL ) ;
}


