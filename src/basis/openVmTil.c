
#include "../include/csl.h"
#define VERSION ((byte*) "0.906.830" ) 

// inspired by :: Logic/Foml (Foundations of Mathematical Logic by Haskell Curry), 
// CT/Oop (Category Theory, Object Oriented Programming, Type Theory), 
// Formal Language Theory (Panini, Chomsky) 
// C/C++/C#, Lisp, RPN/Lag : Reverse Polish Notation, (Left Associative Grammar), 
// State Machines, Push Down Automata (PDA), Turing Machines :: 
// Also Laws of Form, by G.S. Brown, Kurt Goedel and Quantum Field Theory, etc.
// csl : context sensitive language
// (til : a toolkit for implementing languages (maybe even a compiler compiler) based on these ideas),

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
        }
        ovt = _O_ = _OpenVmTil_New ( _O_, argc, argv ) ;
        OVT_SetRestartCondition ( ovt, restartCondition ) ;
        ovt->SigSegvs = sigSegvs ;
        ovt->Verbosity = 1 ;
        ovt->Restarts = restarts ;
        if ( ovt->Restarts ) OVT_ExceptionState_Print ( ) ;
        if ( ! sigsetjmp ( ovt->JmpBuf0, 0 ) ) // nb. siglongjmp always comes to beginning of the block 
            CSL_Run ( ovt->OVT_CSL, ovt->RestartCondition ) ;
        restartCondition = ovt->RestartCondition ;
        OVT_SetRestartCondition ( ovt, restartCondition ) ;
        //sigSegvs = ovt->SigSegvs ;
    }
}

OpenVmTil *
_OpenVmTil_Allocate ( )
{
    OpenVmTil * ovt = ( OpenVmTil* ) mmap_AllocMem ( sizeof ( OpenVmTil ) ) ; //_Mem_Allocate ( 0, sizeof ( OpenVmTil ), 0, ( RETURN_CHUNK_HEADER ) ) ; // don't add this to mem alloc system ; ummap it when done
    dllist_Init ( &ovt->PermanentMemList, &ovt->PML_HeadNode, &ovt->PML_TailNode ) ;
    ovt->OVT_InitialUnAccountedMemory = sizeof ( OpenVmTil ) ; // needed here because 'ovt' was not initialized yet for MemChunk accounting
    return ovt ;
}

// only partially working ??
void
OVT_RecycleAllWordsDebugInfo ( )
{
    SetState ( _CSL_, ( RT_DEBUG_ON | GLOBAL_SOURCE_CODE_MODE ), false ) ;
    OVT_MemListFree_CompilerTempObjects ( ) ;
#if 0 
    CSL_RecycleInit_Compiler_N_M_Node_WordList ( ) ;
#else // partially working ??    
    Tree_Map_Namespaces ( _CSL_->Namespaces->W_List, ( MapSymbolFunction ) CSL_DeleteWordDebugInfo ) ;
#endif    
}

void
_OpenVmTil_Init ( OpenVmTil * ovt, int64 resetHistory )
{
    MemorySpace_New ( ) ; // nb : memory must be after we set Size values and before lists; list are allocated from memory
    _HistorySpace_New ( ovt, resetHistory ) ;
    //ovt->psi_PrintStateInfo = PrintStateInfo_New ( ) ; // variable init needed by any allocation which call _Printf
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

void
Ovt_RunInit ( OpenVmTil * ovt )
{
    //ovt->SignalExceptionsHandled = 0 ;
    ovt->StartedTimes ++ ;
    OVT_SetRestartCondition ( ovt, CSL_RUN_INIT ) ;
}

void
OpenVmTil_Delete ( OpenVmTil * ovt )
{
    if ( ovt )
    {
        if ( ovt->Verbosity > 2 ) Printf ( ( byte* ) "\nAll allocated memory is being freed.\nRestart : verbosity = %d.", ovt->Verbosity ) ;
        FreeChunkList ( &ovt->PermanentMemList ) ;
        mmap_FreeMem ( ( byte* ) ovt->MemorySpace0, sizeof ( MemorySpace ) ) ;
        mmap_FreeMem ( ( byte* ) ovt, sizeof ( OpenVmTil ) ) ;
    }
    _O_ = 0 ;
}
#define USE_OpenVmTil_CalculateMemSpaceSizes 0
#define _CSL_SIZE (82 * K) // data stack included here
#if USE_OpenVmTil_CalculateMemSpaceSizes
// _OpenVmTil_CalculateMemSpaceSizes is convoluted and needs rework

void
_OpenVmTil_CalculateMemSpaceSizes ( OpenVmTil * ovt, int64 restartCondition, int64 totalMemSizeTarget )
{
    int64 minimalCoreMemorySize, minStaticMemSize, coreMemTargetSize, exceptionsHandled, verbosity, objectsSize, tempObjectsSize,
        sessionObjectsSize, dataStackSize, historySize, lispTempSize, compilerTempObjectsSize, contextSize, bufferSpaceSize, stringSpaceSize,
        openVmTilSize, cslSize, codeSize, dictionarySize ; //, sessionCodeSize ;

    if ( restartCondition < RESTART )
    {
        verbosity = ovt->Verbosity ;
        // preserve values across partial restarts
        sessionObjectsSize = ovt->SessionObjectsSize ;
        dictionarySize = ovt->DictionarySize ;
        lispTempSize = ovt->LispTempSize ;
        codeSize = ovt->MachineCodeSize ;
        objectsSize = ovt->ObjectsSize ;
        tempObjectsSize = ovt->TempObjectsSize ;
        compilerTempObjectsSize = ovt->CompilerTempObjectsSize ;
        dataStackSize = ovt->DataStackSize ;
        historySize = ovt->HistorySize ;
        contextSize = ovt->ContextSize ;
        bufferSpaceSize = ovt->BufferSpaceSize ;
        openVmTilSize = ovt->OpenVmTilSize ;
        cslSize = ovt->CSLSize ;
        stringSpaceSize = ovt->StringSpaceSize ;
        exceptionsHandled = ovt->SignalExceptionsHandled ;
    }
    else if ( totalMemSizeTarget > 0 )
    {
        verbosity = 0 ;

        // volatile mem sizes
        tempObjectsSize = 10 * K ; //TEMP_OBJECTS_SIZE ;
        sessionObjectsSize = 50 * K ; //SESSION_OBJECTS_SIZE ;
        lispTempSize = 10 * K ; //LISP_TEMP_SIZE ;
        compilerTempObjectsSize = 10 * K ; //COMPILER_TEMP_OBJECTS_SIZE ;
        historySize = 1 * K ; //HISTORY_SIZE ;
        contextSize = 5 * FULL_CONTEXT_SIZE ; //CONTEXT_SIZE ;
        bufferSpaceSize = 35 * ( sizeof ( Buffer ) + BUFFER_SIZE ) ; //2153 ; //K ; //BUFFER_SPACE_SIZE ;
        stringSpaceSize = 10 * K ; //BUFFER_SPACE_SIZE ;

        // static mem sizes
        dataStackSize = 2 * K ; // STACK_SIZE
        openVmTilSize = 2 * K ; //OPENVMTIL_SIZE ;
        cslSize = _CSL_SIZE ; //( dataStackSize * 4 ) + ( 12.5 * K ) ; // csl_SIZE
        exceptionsHandled = 0 ;
    }
    else // 0 or -1 get default
    {
        verbosity = 0 ;

        tempObjectsSize = 1 * MB ; //TEMP_OBJECTS_SIZE ;
        sessionObjectsSize = 1 * MB ; // SESSION_OBJECTS_SIZE ;
        lispTempSize = 1 * MB ; // LISP_TEMP_SIZE ;
        compilerTempObjectsSize = 1 * MB ; //COMPILER_TEMP_OBJECTS_SIZE ;
        contextSize = 5 * K ; // CONTEXT_SIZE ;
        bufferSpaceSize = 35 * ( sizeof ( Buffer ) + BUFFER_SIZE ) ;
        stringSpaceSize = 1 * MB ; //BUFFER_SPACE_SIZE ;
        historySize = 1 * MB ; //HISTORY_SIZE ;

        dataStackSize = 8 * KB ; //STACK_SIZE ;
        openVmTilSize = 15 * KB ; //OPENVMTIL_SIZE ;
        cslSize = _CSL_SIZE ; //( dataStackSize * sizeof (int64 ) ) + ( 5 * KB ) ; //csl_SIZE ;

        exceptionsHandled = 0 ;
    }
    minStaticMemSize = tempObjectsSize + sessionObjectsSize + dataStackSize + historySize + lispTempSize + compilerTempObjectsSize +
        contextSize + bufferSpaceSize + openVmTilSize + cslSize, stringSpaceSize ;

    minimalCoreMemorySize = 150 * K, coreMemTargetSize = totalMemSizeTarget - minStaticMemSize ;
    coreMemTargetSize = ( coreMemTargetSize > minimalCoreMemorySize ) ? coreMemTargetSize : minimalCoreMemorySize ;
    // core memory
    objectsSize = 1 * M ; //( int64 ) ( 0.333 * ( ( double ) coreMemTargetSize ) ) ; // we can easily allocate more object and dictionary space but not code space
    dictionarySize = ( int64 ) ( 0.333 * ( ( double ) coreMemTargetSize ) ) ;
    codeSize = ( int64 ) ( 0.333 * ( ( double ) coreMemTargetSize ) ) ;
    codeSize = ( codeSize < ( 500 * K ) ) ? 500 * K : codeSize ;

    ovt->SignalExceptionsHandled = exceptionsHandled ;
    ovt->Verbosity = verbosity ;
    ovt->MachineCodeSize = codeSize ;
    ovt->DictionarySize = dictionarySize ;
    ovt->ObjectsSize = objectsSize ;
    ovt->TempObjectsSize = tempObjectsSize ;
    ovt->SessionObjectsSize = sessionObjectsSize ;
    ovt->DataStackSize = dataStackSize ;
    ovt->HistorySize = historySize ;
    ovt->LispTempSize = lispTempSize ;
    ovt->ContextSize = contextSize ;
    ovt->CompilerTempObjectsSize = compilerTempObjectsSize ;
    ovt->BufferSpaceSize = bufferSpaceSize ;
    ovt->CSLSize = cslSize ;
    ovt->StringSpaceSize = stringSpaceSize ;
    ovt->OpenVmTilSize = openVmTilSize ;
}
#endif

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

OpenVmTil *
_OpenVmTil_New ( OpenVmTil * ovt, int64 argc, char * argv [ ] )
{
    //char errorFilename [256] ;
    int64 restartCondition, exceptionsHandled, startedTimes = 0 ; //, startIncludeTries
    if ( ! ovt ) restartCondition = INITIAL_START ;
    else restartCondition = FULL_RESTART ;
#if 0
    startIncludeTries = ovt ? ovt->StartIncludeTries ++ : 0 ;
    if ( startIncludeTries < 2 )
    {
        if ( ovt && ovt->OVT_Context && ovt->OVT_Context->ReadLiner0 && ovt->OVT_Context->ReadLiner0->Filename )
            strcpy ( errorFilename, ( char* ) ovt->OVT_Context->ReadLiner0->Filename ) ;
        else strcpy ( errorFilename, "Debug Context" ) ;
    }
    else errorFilename [ 0 ] = 0 ;
    //restartCondition = ( ovt && ( restartCondition || ( startIncludeTries < 2 ) ) ) ? ovt->RestartCondition : RESTART ;
    int64 ium = ovt ? ovt->OVT_InitialUnAccountedMemory : 0, ovtv = ovt ? ovt->Verbosity : 0 ;
    if ( ovt && ( restartCondition < INITIAL_START ) && ( ovt->Restarts < 2 ) ) OpenVmTil_Delete ( ovt ) ;
    else if ( ovt )
    {
        printf ( ( byte* ) "\nUnable to reliably delete memory from previous system - rebooting into a new system. 'mem' for more detail on memory.\n" ) ;
        fflush ( stdout ) ;
        if ( ovt->Restarts < 2 ) OpenVmTil_Pause ( ) ; // we may crash here
    }
    d0 ( if ( ovtv > 1 )
    {
        printf ( ( byte* ) "\nTotal Mem Remaining = %9lld : <=: mmap_TotalMemAllocated - mmap_TotalMemFreed - ovt->OVT_InitialUnAccountedMemory", mmap_TotalMemAllocated - mmap_TotalMemFreed - ium ) ;
            fflush ( stdout ) ;
    } )
#else
    if ( ovt ) startedTimes = ovt->StartedTimes, OpenVmTil_Delete ( ovt ) ;
#endif        
    _O_ = ovt = _OpenVmTil_Allocate ( ) ;

    OVT_SetRestartCondition ( ovt, restartCondition ) ;
    ovt->Argc = argc ;
    ovt->Argv = argv ;
    ovt->StartedTimes = startedTimes ;
    //ovt->SavedTerminalAttributes = savedTerminalAttributes ;

    OVT_GetStartupOptions ( ovt ) ;
#if USE_OpenVmTil_CalculateMemSpaceSizes 
    int64 MIN_TotalMemSizeTarget = ( 300 * K ) ;
    if ( ovt->TotalMemSizeTarget < MIN_TotalMemSizeTarget ) ovt->TotalMemSizeTarget = MIN_TotalMemSizeTarget ;
    int64 totalMemSizeTarget = ( ovt->TotalMemSizeTarget < 5 * M ) ? ovt->TotalMemSizeTarget : - 1 ; // 0 or -1 : gets default values     
    _OpenVmTil_CalculateMemSpaceSizes ( ovt, restartCondition, - 1 ) ; //totalMemSizeTarget ) ;
#else    
    ovt->InternalObjectsSize = 100 * K ; //1 * M ; 
    ovt->ObjectsSize = 100 * K ; //1 * M ; 
    ovt->BufferSpaceSize = 100 * K ; //35 * ( sizeof ( Buffer ) + BUFFER_SIZE ) ;
    ovt->StringSpaceSize = 100 * K ;
    ovt->MachineCodeSize = 300 * K ;
    ovt->DictionarySize = 100 * K ;
    ovt->CSLSize = ( 23 * K ) ;
    ovt->OpenVmTilSize = ( 9 * K ) ;
    ovt->DataStackSize = 8 * KB ;
#endif    

    _OpenVmTil_Init ( ovt, exceptionsHandled > 1 ) ; // try to keep history if we can
    Linux_SetupSignals ( &ovt->JmpBuf0, 1 ) ;
    //if ( startIncludeTries ) ovt->ErrorFilename = String_New ( ( byte* ) errorFilename, STRING_MEM ) ;
    return ovt ;
}

