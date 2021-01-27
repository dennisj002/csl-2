
#include "../../include/csl.h"

#if 0

void
MCP_MoveArrayN_To_Reg ( int64 reg, byte * array, int64 n )
{
    // pop reg, array, n
    // move array to reg
}
#endif

#if 0 // useful but not yet in use

void
Compile_CallCFunction_TestAlignRSP_MoveResultRegRaxToTOS ( byte * cFunction )
{
    Compile_Call_ToAddressThruReg_TestAlignRSP ( cFunction, CALL_THRU_REG ) ;
    _Compile_Stack_PushReg ( DSP, RAX ) ;
}
#endif

void
Compile_WordRun ( )
{
    _Compile_Stack_PopToReg ( DSP, RAX ) ;
    Compile_Move_Rm_To_Reg ( RAX, RAX, 0xa8, 0 ) ; // 0xa8 : Definition offset in Word structure in version 0.902.860
    _Compile_Group5 ( CALL, REG, RAX, 0, 0, 0 ) ;
}

Word *
_CSL_MachineCodePrimitive_NewAdd ( const char * name, uint64 morphismAttributes, int64 objectAttributes, block * callHook, byte * function, int64 functionArg )
{
    Word * word = _DObject_New ( ( byte* ) name, ( uint64 ) function, ( morphismAttributes ), objectAttributes, 0, 0, function, functionArg, 0, 0, DICTIONARY ) ;
    if ( callHook ) *callHook = word->Definition ;
    return word ;
}

void
CSL_MachineCodePrimitive_NewAdd ( const char * name, uint64 morphismAttributes, int64 objectAttributes, 
    block * callHook, byte * function, int64 functionArg, const char *nameSpace, const char * superNamespace )
{
    Word * word = _CSL_MachineCodePrimitive_NewAdd ( name, morphismAttributes, objectAttributes, callHook, function, functionArg ) ;
    _CSL_InitialAddWordToNamespace ( word, ( byte* ) nameSpace, ( byte* ) superNamespace ) ;
}

void
CSL_MachineCodePrimitive_AddWords ( CSL * csl )
{
    Debugger * debugger = _Debugger_ ;
    // this form (below) can and should replace the loop because we need to have variables for some elements
    CSL_MachineCodePrimitive_NewAdd ( "call_ToAddressThruSREG_TestAlignRSP", CSL_WORD | CSL_ASM_WORD, 0, & csl->Call_ToAddressThruSREG_TestAlignRSP, ( byte* ) Compile_Call_ToAddressThruSREG_TestAlignRSP, - 1, "System", "Root" ) ;
    CSL_MachineCodePrimitive_NewAdd ( "restoreCpuState", CSL_WORD | CSL_ASM_WORD, 0, & debugger->RestoreCpuState, ( byte* ) Compile_CpuState_Restore, ( int64 ) debugger->cs_Cpu, "Debug", "Root" ) ;
    CSL_MachineCodePrimitive_NewAdd ( "saveCpuState", CSL_WORD | CSL_ASM_WORD, 0, & debugger->SaveCpuState, ( byte* ) Compile_CpuState_Save, ( int64 ) debugger->cs_Cpu, "Debug", "Root" ) ;
    CSL_MachineCodePrimitive_NewAdd ( "restoreCpuState", CSL_WORD | CSL_ASM_WORD, 0, & csl->RestoreCpuState, ( byte* ) Compile_CpuState_Restore, ( int64 ) csl->cs_Cpu, "System", "Root" ) ;
    CSL_MachineCodePrimitive_NewAdd ( "saveCpuState", CSL_WORD | CSL_ASM_WORD, 0, & csl->SaveCpuState, ( byte* ) Compile_CpuState_Save, ( int64 ) csl->cs_Cpu, "System", "Root" ) ;
    CSL_MachineCodePrimitive_NewAdd ( "restoreCpu2State", CSL_WORD | CSL_ASM_WORD, 0, & csl->RestoreCpu2State, ( byte* ) Compile_CpuState_Restore, ( int64 ) csl->cs_Cpu2, "System", "Root" ) ;
    CSL_MachineCodePrimitive_NewAdd ( "saveCpu2State", CSL_WORD | CSL_ASM_WORD, 0, & csl->SaveCpu2State, ( byte* ) Compile_CpuState_Save, ( int64 ) csl->cs_Cpu2, "System", "Root" ) ;
    CSL_MachineCodePrimitive_NewAdd ( "wrun", CSL_WORD | CSL_ASM_WORD | INLINE, 0, & csl->WordRun, ( byte* ) Compile_WordRun, ( int64 ) - 1, "System", "Root" ) ;
    CSL_MachineCodePrimitive_NewAdd ( "<dbg>", CSL_WORD | CSL_ASM_WORD | RT_STEPPING_DEBUG | DEBUG_WORD, 0, 0, ( byte* ) _CSL_DebugRuntimeBreakpoint, - 1, "Debug", "Root" ) ;
    //{ "<rt-dbg>", CPRIMITIVE|DEBUG_WORD, RT_STEPPING_DEBUG, 0, ( byte* ) _CSL_DebugRuntimeBreakpoint, - 1, "Debug", "Root" },
    //{ "<dso>", CPRIMITIVE|DEBUG_WORD, RT_STEPPING_DEBUG, 0, ( byte* ) _CSL_DebugRuntimeBreakpoint_IsDebugShowOn, - 1, "Debug", "Root" },
    //{ "<d:dbg>", CPRIMITIVE|DEBUG_WORD, RT_STEPPING_DEBUG, 0, ( byte* ) _CSL_DebugRuntimeBreakpoint_IsDebugOn, - 1, "Debug", "Root" },
}


#if 0  

void
CSL_MachineCodePrimitive_AddWords ( CSL * csl )
{
    Debugger * debugger = _Debugger_ ;
    //_CSL_MachineCodePrimitive_NewAdd ( "restoreSelectedCpuState", CPRIMITIVE, 0, & csl->RestoreSelectedCpuState, ( byte* ) _Compile_CpuState_RestoreSelected, ( int64 ) csl->cs_Cpu, "System", "Root" ) ;
    //_CSL_MachineCodePrimitive_NewAdd ( "saveSelectedCpuState", CPRIMITIVE, 0, & csl->SaveSelectedCpuState, ( byte* ) _Compile_CpuState_SaveSelected, ( int64 ) csl->cs_Cpu, "System", "Root" ) ;
    //_CSL_MachineCodePrimitive_NewAdd ( "getRsp", CPRIMITIVE, 0, & debugger->getRsp, ( byte* ) Compile_Debug_GetRSP, - 1, "System", "Root" ) ;
    //_CSL_MachineCodePrimitive_NewAdd ( "callCurrentBlock", CPRIMITIVE, 0, & csl->CallCurrentBlock, ( byte* ) Compile_Call_CurrentBlock, - 1, "System", "Root" ) ;
    //_CSL_MachineCodePrimitive_NewAdd ( "set_DataStackPointer_FromDspReg", CPRIMITIVE, 0, & csl->Set_DataStackPointer_FromDspReg, ( byte* ) Compile_Set_DataStackPointer_FromDspReg, - 1, "System", "Root" ) ;
    //_CSL_MachineCodePrimitive_NewAdd ( "set_DspReg_FromDataStackPointer", CPRIMITIVE, 0, & csl->Set_DspReg_FromDataStackPointer, ( byte* ) Compile_Set_DspReg_FromDataStackPointer, - 1, "System", "Root" ) ;
    {
        "rspReg", CPRIMITIVE, 0, 0, ( byte* ) _Compile_RspReg_Get, - 1, "System", "Root"
    },
    {
        "rspReg@", CPRIMITIVE, 0, 0, ( byte* ) _Compile_RspReg_Fetch, - 1, "System", "Root"
    },
    {
        ">rspReg", CPRIMITIVE, 0, 0, ( byte* ) _Compile_RspReg_To, - 1, "System", "Root"
    },
    {
        "rspReg>", CPRIMITIVE, 0, 0, ( byte* ) _Compile_RspReg_From, - 1, "System", "Root"
    },
    {
        "rspRegdrop", CPRIMITIVE, 0, 0, ( byte* ) _Compile_RspReg_Drop, - 1, "Debug", "Root"
    },
    //{ "rspReg!", CPRIMITIVE, 0, 0, ( byte* ) _Compile_RspReg_Store, - 1, "System", "Root" },
    for ( i = 0 ; MachineCodePrimitives [ i ].ccp_Name ; i ++ )
    {
        MachineCodePrimitive p = MachineCodePrimitives [ i ] ;
        functionArg = - 1 ; // this is also flag in _DObject_ValueDefinition_Init
        callHook = 0 ;
        CSL_MachineCodePrimitive_NewAdd ( p.ccp_Name, p.ui64_CAttribute, p.ui64_CAttribute2, callHook, p.Function, functionArg, p.NameSpace, p.SuperNamespace ) ;
    }
}
#endif    


