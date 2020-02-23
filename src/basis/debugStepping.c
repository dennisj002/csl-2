
#include "../include/csl.h"
#define IS_CALL_INSN(address) ( ( * address == CALLI32 ) || ( ( ( * ( uint16* ) address ) == 0xff49 ) && ( *( address + 2 ) == 0xd1 ) ) )

void
_Debugger_StepOneInstruction ( Debugger * debugger )
{
    debugger->SaveTOS = TOS ;
    debugger->SaveStackDepth = DataStack_Depth ( ) ;
    DefaultColors ; // nb. : so text output will be in default colors
    ( ( VoidFunction ) debugger->StepInstructionBA->BA_Data ) ( ) ;
    DebugColors ;
}

// Debugger_CompileOneInstruction ::
// this function should not affect the C registers at all 
// we save them before we call our stuff and restore them after

byte *
Debugger_CompileOneInstruction ( Debugger * debugger, byte * jcAddress, Boolean showFlag )
{
    ByteArray * svcs = _O_CodeByteArray ;
    _ByteArray_Init ( debugger->StepInstructionBA ) ; // we are only compiling one insn here so clear our BA before each use
    Set_CompilerSpace ( debugger->StepInstructionBA ) ; // now compile to this space
    _Compile_Save_C_CpuState ( _CSL_, showFlag ) ; //&& ( _O_->Verbosity >= 3 ) ) ; // save our c compiler cpu register state
    _Compile_Restore_Debugger_CpuState ( debugger, showFlag ) ; //&& ( _O_->Verbosity >= 3 ) ) ; // restore our runtime state before the current insn
    byte * nextInsn = _Debugger_CompileOneInstruction ( debugger, jcAddress ) ; // the single current stepping insn
    _Compile_Save_Debugger_CpuState ( debugger, showFlag ) ; //showRegsFlag ) ; //&& ( _O_->Verbosity >= 3 ) ) ; // save our runtime state after the instruction : which we will restore before the next insn
    _Compile_Restore_C_CpuState ( _CSL_, showFlag ) ; //&& ( _O_->Verbosity >= 3 ) ) ; // save our c compiler cpu register state
    _Compile_Return ( ) ;
    Set_CompilerSpace ( svcs ) ; // restore compiler space pointer before "do it" in case "do it" calls the compiler
    return nextInsn ;
}

void
_Debugger_CompileAndStepOneInstruction ( Debugger * debugger, byte * jcAddress )
{
    Boolean showExtraFlag = false ;
    byte * svHere = Here ; // save 
    byte * nextInsn = Debugger_CompileOneInstruction ( debugger, jcAddress, showExtraFlag ) ; // compile the insn here
    _Debugger_StepOneInstruction ( debugger ) ;
    if ( showExtraFlag ) Debug_ExtraShow ( Here - svHere, showExtraFlag ) ;
    if ( GetState ( debugger, DBG_AUTO_MODE ) && ( ! GetState ( debugger, DBG_CONTINUE_MODE ) ) ) SetState ( debugger, DBG_SHOW_STACK_CHANGE, false ) ;
    else _Debugger_ShowEffects (debugger, debugger->w_Word, GetState ( debugger, DBG_STEPPING ), showExtraFlag , 0) ;
    if ( GetState ( debugger, ( DBG_AUTO_MODE | DBG_CONTINUE_MODE ) ) ) Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\r", ( byte* ) "" ) ;
    if ( Compiling ) _Debugger_DisassembleWrittenCode ( debugger ) ;
    debugger->DebugAddress = nextInsn ;
}

byte *
_Debugger_CompileOneInstruction ( Debugger * debugger, byte * jcAddress )
{
    byte * newDebugAddress ;
    int64 size ;
    Word * word = 0 ;
    size = Debugger_Udis_GetInstructionSize ( debugger ) ;
    if ( jcAddress ) // jump or call address
    {
        word = Word_UnAlias ( Word_GetFromCodeAddress ( jcAddress ) ) ;
        //if ( _O_->Verbosity > 1 ) 
        if ( IS_CALL_INSN ( debugger->DebugAddress ) ) _Word_ShowSourceCode ( word ) ;
        if ( ( ! word ) || ( ! Debugger_CanWeStep ( debugger, word ) ) )
        {
            Printf ( ( byte* ) "\ncalling thru - a non-native (C) subroutine : %s : .... :> %s ", word ? ( char* ) c_gd ( word->Name ) : "", Context_Location ( ) ) ;
            newDebugAddress = _Debugger_COI_StepThru ( debugger, jcAddress, size ) ;
        }
        else if ( ( debugger->Key == 'I' ) ) // force Into a subroution
        {
            Printf ( ( byte* ) "\nforce calling (I)nto a subroutine : %s : .... :> %s ", word ? ( char* ) c_gd ( word->Name ) : "", Context_Location ( ) ) ;
            newDebugAddress = _Debugger_COI_StepInto ( debugger, word, jcAddress, size ) ;
        }
        else if ( ( debugger->Key == 'h' ) || ( debugger->Key == 'o' ) || ( debugger->Key == 'u' ) )// step 't(h)ru'/(o)ver the native code like a non-native subroutine
        {
            Printf ( ( byte* ) "\ncalling t(h)ru - a subroutine : %s : .... :> %s ", word ? ( char* ) c_gd ( word->Name ) : "", Context_Location ( ) ) ;
            newDebugAddress = _Debugger_COI_StepThru ( debugger, jcAddress, size ) ;
        }
        else newDebugAddress = _Debugger_COI_StepInto ( debugger, word, jcAddress, size ) ;

    }
    else newDebugAddress = _Debugger_COI_Do_Insn_Default ( debugger, size ) ;
    return newDebugAddress ;
}

void
Debugger_CompileAndStepOneInstruction ( Debugger * debugger )
{
    if ( debugger->DebugAddress )
    {
        byte *jcAddress = 0, *adr ;
        if ( * debugger->DebugAddress == _RET )
        {
            Debugger_CASOI_Do_Return_Insn ( debugger ) ;
            Debugger_CASOI_UpdateInfo ( debugger ) ;
        }
        else if ( ( * debugger->DebugAddress == JMPI32 ) || ( * debugger->DebugAddress == CALLI32 ) || ( * debugger->DebugAddress == JMPI8 ) )
        {
            jcAddress = JumpCallInstructionAddress ( debugger->DebugAddress ) ;
            Debugger_CASOI_Do_JmpOrCall_Insn ( debugger, jcAddress ) ;
        }
        else if ( ( ( ( * ( uint16* ) debugger->DebugAddress ) == 0xff49 ) && ( *( debugger->DebugAddress + 2 ) == 0xd1 ) ) )
        {
            jcAddress = JumpCallInstructionAddress_X64ABI ( debugger->DebugAddress ) ;
            Debugger_CASOI_Do_JmpOrCall_Insn ( debugger, jcAddress ) ;
        }
#if 0 // old code ??       
        else if ( ( * debugger->DebugAddress == CALL_JMP_MOD_RM ) && ( _RM ( debugger->DebugAddress ) == 16 ) ) // inc/dec are also opcode == 0xff
        {
            jcAddress = Debugger_CASOI_Do_IncDec_Insn ( debugger, jcAddress ) ;
            _Debugger_CompileAndStepOneInstruction ( debugger, jcAddress ) ;
        }
#endif        
        else if ( adr = debugger->DebugAddress, ( ( * adr == 0x0f ) && ( ( * ( adr + 1 ) >> 4 ) == 0x8 ) ) ||
            ( adr = debugger->DebugAddress + 1, ( * adr == 0x0f ) && ( ( * ( adr + 1 ) >> 4 ) == 0x8 ) ) ) // jcc 
        {
            Debugger_CASOI_Do_Jcc_Insn ( debugger, jcAddress ) ;
            Debugger_CASOI_UpdateInfo ( debugger ) ;
        }
        else if ( ( ( ( byte* ) debugger->DebugAddress )[0] >> 4 ) == 7 )
        {
            Debugger_CASOI_Do_Test_Insn ( debugger, jcAddress ) ;
        }
        else _Debugger_CompileAndStepOneInstruction ( debugger, jcAddress ) ;
    }
}

// simply : copy the current insn to a ByteArray buffer along with
// prefix and postfix instructions that restore and
// save the cpu state; then run that ByteArray code buffer

void
Debugger_PreStartStepping ( Debugger * debugger )
{
    Word * word = debugger->w_Word ;
    if ( word )
    {
        debugger->WordDsp = _Dsp_ ; // by 'eval' we stop debugger->Stepping and //continue thru this word as if we hadn't stepped
        Debugger_CanWeStep ( debugger, word ) ;
        // we would at least need to save/restore our registers to step thru native c code
        if ( ! GetState ( debugger, DBG_CAN_STEP ) )
        {
            Printf ( ( byte* ) "\nStepping turned off for this word : %s%s%s%s : debugger->DebugAddress = 0x%016lx : (e)valuating",
                c_ud ( word->S_ContainingNamespace ? word->S_ContainingNamespace->Name : ( byte* ) "<literal> " ),
                word->S_ContainingNamespace ? ( byte* ) "." : ( byte* ) "", c_gu ( word->Name ),
                GetState ( debugger, DBG_AUTO_MODE ) ? " : automode turned off" : "",
                debugger->DebugAddress ) ;
            debugger->DebugAddress = 0 ;
            _Debugger_Eval ( debugger, 0 ) ;
            SetState ( _Debugger_, DBG_AUTO_MODE, false ) ;
            return ;
        }
        else
        {
            if ( Word_IsSyntactic ( word ) ) //&& ( ! GetState ( word, STEPPED ) ) )
            {
                uint64 * svDsp = _Dsp_ ;
                Interpreter * interp = _Interpreter_ ;
                interp->w_Word = word ;
                SetState ( _Debugger_, DBG_INFIX_PREFIX, true ) ;
                DebugOff ; // so we don't debug the args
                Interpreter_DoInfixOrPrefixWord ( interp, word ) ; // 
                DebugOn ; // so debug is on for Word_Eval and we can step thru it
                SetState ( _Debugger_, DBG_INFIX_PREFIX, false ) ;
                Dbg_Block_Eval ( word, word->Definition ) ;
                _DEBUG_SHOW ( word, 0, 0 ) ;
                SetState ( word, STEPPED, true ) ;
                SetState ( debugger, ( DBG_STEPPED | DBG_STEPPING ), false ) ; // no longjmp needed at end of Interpreter_Loop
                _Set_DataStackPointers ( svDsp ) ;
                return ; 
            }
            Debugger_SetupStepping ( debugger ) ;
            SetState ( debugger, DBG_NEWLINE | DBG_PROMPT | DBG_INFO | DBG_AUTO_MODE, false ) ;
        }
    }
    else SetState_TrueFalse ( debugger, DBG_NEWLINE, DBG_STEPPING ) ;
    return ;
}

void
Debugger_Step ( Debugger * debugger )
{
    if ( ! GetState ( debugger, DBG_STEPPING ) ) Debugger_PreStartStepping ( debugger ) ;
    else
    {
        Debugger_CompileAndStepOneInstruction ( debugger ) ;
        Debugger_AfterStep ( debugger ) ;
    }
}

void
Debugger_AfterStep ( Debugger * debugger )
{
    debugger->LastRsp = debugger->cs_Cpu->Rsp ;
    if ( ( int64 ) debugger->DebugAddress ) // set by StepOneInstruction
    {
        debugger->SteppedWord = debugger->w_Word ;
        SetState_TrueFalse ( debugger, DBG_STEPPING, ( DBG_INFO | DBG_MENU | DBG_PROMPT ) ) ;
    }
    else SetState_TrueFalse ( debugger, DBG_PRE_DONE | DBG_STEPPED | DBG_NEWLINE | DBG_PROMPT | DBG_INFO, DBG_STEPPING ) ;
}

void
_Debugger_SetupStepping ( Debugger * debugger, Word * word, byte * address, byte * name )
{
    Printf ( ( byte* ) "\nSetting up stepping : location = %s : debugger->word = \'%s\' : ...", c_gd ( _Context_Location ( _Context_ ) ), word ? word->Name : ( name ? name : ( byte* ) "" ) ) ;
    if ( word )
    {
        _CSL_Source ( debugger->w_Word, 0 ) ;
        if ( ( ! address ) || ( ! GetState ( debugger, ( DBG_BRK_INIT | DBG_SETUP_ADDRESS ) ) ) ) address = ( byte* ) word->Definition ;
    }
    SetState_TrueFalse ( debugger, DBG_STEPPING, DBG_NEWLINE | DBG_PROMPT | DBG_INFO | DBG_MENU ) ;
    debugger->DebugAddress = address ;
    debugger->w_Word = Word_UnAlias ( word ) ;

    if ( ! GetState ( debugger, ( DBG_BRK_INIT | DBG_RUNTIME_BREAKPOINT ) ) ) SetState ( debugger->cs_Cpu, CPU_SAVED, false ) ;
    SetState ( _CSL_->cs_Cpu, CPU_SAVED, false ) ;
    _Debugger_CpuState_CheckSave ( debugger ) ;
    _CSL_CpuState_CheckSave ( ) ;
    debugger->LevelBitNamespaceMap = 0 ;
    SetState ( debugger, DBG_START_STEPPING, true ) ;
    _Debugger_->LastSourceCodeWord = 0 ;
    CSL_NewLine ( ) ;
}

void
Debugger_SetupStepping ( Debugger * debugger )
{
    _Debugger_SetupStepping ( debugger, debugger->w_Word, debugger->DebugAddress, 0 ) ;
}

int64
_Debugger_SetupReturnStackCopy ( Debugger * debugger, int64 size, Boolean showFlag )
{
    if ( _O_->Verbosity > 3 ) _CSL_PrintNReturnStack ( 4, 1 ) ;
    uint64 * rsp = ( uint64* ) debugger->cs_Cpu->Rsp ; //debugger->DebugESP [- 1] ; //debugger->cs_Cpu->Rsp [1] ; //debugger->cs_Cpu->Rsp ;
    if ( rsp )
    {
        uint64 rsc, rsc0 ;
        int64 pushedWindow = 64 ;
        if ( ! debugger->CopyRSP )
        {
            rsc0 = ( uint64 ) Mem_Allocate ( size, COMPILER_TEMP ) ;
            rsc = ( rsc0 + 0xf ) & ( uint64 ) 0xfffffffffffffff0 ; // 16 byte alignment
            debugger->CopyRSP = ( byte* ) rsc + size - pushedWindow ;
            if ( showFlag ) ( _PrintNStackWindow ( ( uint64* ) debugger->CopyRSP, ( byte* ) "ReturnStackCopy", ( byte* ) "RSCP", 4 ) ) ;
        }
        else rsc = ( uint64 ) ( debugger->CopyRSP - size + pushedWindow ) ;
        MemCpy ( ( byte* ) rsc, ( ( byte* ) rsp ) - size + pushedWindow, size ) ; // pushedWindow (32) : account for useful current stack
        if ( showFlag ) ( _PrintNStackWindow ( ( uint64* ) rsp, ( byte* ) "ReturnStack", ( byte* ) "RSP", 4 ) ) ; //pushedWindow ) ) ;
        if ( showFlag ) ( _PrintNStackWindow ( ( uint64* ) debugger->CopyRSP, ( byte* ) "ReturnStackCopy", ( byte* ) "RSCP", 4 ) ) ; //pushedWindow ) ) ;
        debugger->cs_Cpu->Rsp = ( uint64* ) debugger->CopyRSP ;
        SetState ( debugger, DBG_STACK_OLD, false ) ;
        return true ;
    }

    else return false ;
}

void
Debugger_PrintReturnStackWindow ( )
{
    _PrintNStackWindow ( ( uint64* ) _Debugger_->cs_Cpu->Rsp, ( byte* ) "Debugger ReturnStack (RSP)", ( byte* ) "RSP", 4 ) ;
}

// restore the 'internal running csl' cpu state which was saved after the last instruction : debugger->cs_CpuState is the 'internal running csl' cpu state

void
Debugger_SetupReturnStackCopy ( Debugger * debugger, int64 showFlag ) // restore the running csl cpu state
{
    // restore the 'internal running csl' cpu state which was saved after the last instruction : debugger->cs_CpuState is the 'internal running csl' cpu state
    // we don't have to worry so much about the compiler 'spoiling' our insn with restore 
    int64 stackSetupFlag = 0 ;
    if ( ( ! debugger->CopyRSP ) || GetState ( debugger, DBG_STACK_OLD ) )
        stackSetupFlag = _Debugger_SetupReturnStackCopy ( debugger, 8 * K, showFlag ) ;
    if ( showFlag ) Compile_Call_TestRSP ( ( byte* ) _Debugger_CpuState_Show ) ; // also dis insn
}

void
_Compile_Restore_Debugger_CpuState ( Debugger * debugger, int64 showFlag ) // restore the running csl cpu state
{
    // restore the 'internal running csl' cpu state which was saved after the last instruction : debugger->cs_CpuState is the 'internal running csl' cpu state
    // we don't have to worry so much about the compiler 'spoiling' our insn with restore 
    Debugger_SetupReturnStackCopy ( debugger, showFlag ) ; // restore the running csl cpu state
    _Compile_CpuState_Restore ( debugger->cs_Cpu, 1 ) ;
    if ( showFlag ) Compile_Call_TestRSP ( ( byte* ) _Debugger_CpuState_Show ) ; // also dis insn
}

void
_Compile_Restore_C_CpuState ( CSL * csl, int64 showFlag )
{
    _Compile_CpuState_Restore ( csl->cs_Cpu, 1 ) ;
    if ( showFlag ) Compile_Call_TestRSP ( ( byte* ) CSL_CpuState_Show ) ;
}

// restore the 'internal running csl' cpu state which was saved after the last instruction : debugger->cs_CpuState is the 'internal running csl' cpu state

void
_Compile_Save_C_CpuState ( CSL * csl, int64 showFlag )
{
    Compile_CpuState_Save ( csl->cs_Cpu ) ;
    if ( showFlag ) Compile_Call_TestRSP ( ( byte* ) _CSL_CpuState_CheckSave ) ;
}

void
_Compile_Save_Debugger_CpuState ( Debugger * debugger, int64 showFlag )
{
    Compile_CpuState_Save ( debugger->cs_Cpu ) ;
    if ( showFlag ) Compile_Call_TestRSP ( ( byte* ) CSL_Debugger_UdisOneInsn ) ;
    if ( ( _O_->Verbosity > 3 ) && ( debugger->cs_Cpu->Rsp != debugger->LastRsp ) ) Debugger_PrintReturnStackWindow ( ) ;
    if ( showFlag ) Compile_Call_TestRSP ( ( byte* ) CSL_Debugger_CheckSaveCpuStateShow ) ;
}

void
Debugger_Stepping_Off ( Debugger * debugger )
{
    if ( Debugger_IsStepping ( debugger ) )
    {
        Debugger_SetStepping ( debugger, false ) ;
        debugger->DebugAddress = 0 ;
    }
}

Word *
Debugger_GetWordFromAddress ( Debugger * debugger )
{
    Word * word = 0 ;
    if ( debugger->DebugAddress ) word = Word_GetFromCodeAddress ( debugger->DebugAddress ) ;
    if ( ( ! word ) && debugger->Token ) word = _Finder_Word_Find ( _Finder_, USING, debugger->Token ) ; //Finder_FindWord_UsedNamespaces ( _Finder_, debugger->Token ) ;
    if ( word ) debugger->w_Word = word = Word_UnAlias ( word ) ;
    return word ;
}

byte *
Debugger_DoJcc ( Debugger * debugger, int64 numOfBytes )
{
    byte * jcAddress = ( numOfBytes == 2 ) ? JccInstructionAddress_2Byte ( debugger->DebugAddress ) : JccInstructionAddress_1Byte ( debugger->DebugAddress ) ;
    int64 tttn, ttt, n ;
    tttn = ( numOfBytes == 2 ) ? ( debugger->DebugAddress[1] & 0xf ) : ( debugger->DebugAddress[0] & 0xf ) ;
    ttt = ( tttn & 0xe ) >> 1 ;
    n = tttn & 1 ;

    // cf. Intel Software Developers Manual v1. (253665), Appendix B
    // ?? nb. some of this may not be (thoroughly) tested as of 11-14-2011 -- but no known problems after months of usual testing ??
    // setting jcAddress to 0 means we don't branch and just continue with the next instruction
    if ( ttt == TTT_BELOW ) // ttt 001
    {
        if ( ( n == 0 ) && ! ( ( uint64 ) debugger->cs_Cpu->RFlags & CARRY_FLAG ) ) jcAddress = 0 ;
        else if ( ( n == 1 ) && ( ( uint64 ) debugger->cs_Cpu->RFlags & CARRY_FLAG ) ) jcAddress = 0 ;
    }
    else if ( ttt == TTT_ZERO ) // ttt 010
    {
        if ( ( n == 0 ) && ! ( ( uint64 ) debugger->cs_Cpu->RFlags & ZERO_FLAG ) ) jcAddress = 0 ;
        else if ( ( n == 1 ) && ( ( uint64 ) debugger->cs_Cpu->RFlags & ZERO_FLAG ) ) jcAddress = 0 ;
    }
    else if ( ttt == TTT_BE ) // ttt 011 :  below or equal
        // the below code needs to be rewritten :: re. '|' and '^' :: TODO ??
    {
        if ( ( n == 0 ) && ! ( ( ( uint64 ) debugger->cs_Cpu->RFlags & CARRY_FLAG ) | ( ( uint64 ) debugger->cs_Cpu->RFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( ( uint64 ) debugger->cs_Cpu->RFlags & CARRY_FLAG ) | ( ( uint64 ) debugger->cs_Cpu->RFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == TTT_LESS ) // ttt 110
    {
        if ( ( n == 0 ) && ! ( ( ( uint64 ) debugger->cs_Cpu->RFlags & SIGN_FLAG ) ^ ( ( uint64 ) debugger->cs_Cpu->RFlags & OVERFLOW_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( ( uint64 ) debugger->cs_Cpu->RFlags & SIGN_FLAG )
            ^ ( ( uint64 ) debugger->cs_Cpu->RFlags & OVERFLOW_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == TTT_LE ) // ttt 111
    {
        if ( ( n == 0 ) &&
            ! ( ( ( ( uint64 ) debugger->cs_Cpu->RFlags & SIGN_FLAG )
            ^ ( ( uint64 ) debugger->cs_Cpu->RFlags & OVERFLOW_FLAG ) ) | ( ( uint64 ) debugger->cs_Cpu->RFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        if ( ( n == 1 ) &&
            ( ( ( ( uint64 ) debugger->cs_Cpu->RFlags & SIGN_FLAG )
            ^ ( ( uint64 ) debugger->cs_Cpu->RFlags & OVERFLOW_FLAG ) ) | ( ( uint64 ) debugger->cs_Cpu->RFlags & ZERO_FLAG ) ) )
        {

            jcAddress = 0 ;
        }
    }
    return jcAddress ;
}

int64
Debugger_CanWeStep ( Debugger * debugger, Word * word )
{
    int64 result = true ;
    //if ( ( ! debugger->DebugAddress ) || ( GetState ( debugger, DBG_SETUP_ADDRESS ) ) )
    if ( ! word ) result = false ;
    else if ( word->W_MorphismAttributes & ( CSL_WORD | csl_ASM_WORD ) ) result = true ;
    else if ( ! word->CodeStart ) result = false ;
    else if ( word->W_MorphismAttributes & ( CPRIMITIVE | DLSYM_WORD | C_PREFIX_RTL_ARGS ) ) result = false ;
    else if ( ! NamedByteArray_CheckAddress ( _O_CodeSpace, word->CodeStart ) ) result = false ;
    SetState ( debugger, DBG_CAN_STEP, result ) ;
    return result ;
}

void
Debug_ExtraShow ( int64 size, Boolean force )
{
    Debugger * debugger = _Debugger_ ;
    if ( force || ( _O_->Verbosity > 3 ) )
    {
        if ( force || ( _O_->Verbosity > 4 ) )
        {
            Printf ( ( byte* ) "\n\ndebugger->SaveCpuState" ) ;
            _Debugger_Disassemble ( debugger, ( byte* ) debugger->SaveCpuState, 1000, 1 ) ; //137, 1 ) ;
            Printf ( ( byte* ) "\n\ndebugger->RestoreCpuState" ) ;
            _Debugger_Disassemble ( debugger, ( byte* ) debugger->RestoreCpuState, 1000, 2 ) ; //100, 0 ) ;
        }
        Printf ( ( byte* ) "\n\ndebugger->StepInstructionBA->BA_Data" ) ;
        _Debugger_Disassemble ( debugger, ( byte* ) debugger->StepInstructionBA->BA_Data, size, 0 ) ;
    }
}

#if 0

byte *
Debugger_COI_CompileCallOut ( Debugger * debugger, byte * jcAddress, int64 size )
{
    Compile_Call_X84_ABI_RSP_ADJUST ( jcAddress ) ; // this will call jcAddress subroutine and return to our code to be compiled next
    // so that newDebugAddress, below, will be our next stepping insn
    byte * newDebugAddress = debugger->DebugAddress + size ;
    return newDebugAddress ;
}
#endif
// do instruction default

// COI : _Debugger_CompileOneInstruction

byte *
_Debugger_COI_Do_Insn_Default ( Debugger * debugger, int64 size )
{
    byte * newDebugAddress = debugger->DebugAddress + size ;
    if ( ! GetState ( debugger, DBG_JCC_INSN ) )
    {
        ByteArray_AppendCopy ( debugger->StepInstructionBA, size, debugger->DebugAddress ) ;
    }
    return newDebugAddress ;
}

byte *
_Debugger_COI_StepInto ( Debugger * debugger, Word * word, byte * jcAddress, int64 size )
{
    byte * newDebugAddress ;
    while ( word->W_MorphismAttributes & ( ALIAS ) ) word = word->W_AliasOf ;
    if ( ( * debugger->DebugAddress == CALLI32 ) || ( ( ( * ( uint16* ) debugger->DebugAddress ) == 0xff49 ) && ( *( debugger->DebugAddress + 2 ) == 0xd1 ) ) )
    {
        Printf ( ( byte* ) "\nstepping into a csl compiled function : %s : .... :> %s ", word ? ( char* ) c_gd ( word->Name ) : "", Context_Location ( ) ) ;
        _Stack_Push ( debugger->ReturnStack, ( int64 ) ( debugger->DebugAddress + size ) ) ; // the return address
        // push the return address this time around; next time code at newDebugAddress will be processed
        // when ret is the insn Debugger_StepOneInstruction will handle it 
    }
    // for either jmp/call/jcc ...
    return newDebugAddress = jcAddress ;
}

byte *
_Debugger_COI_StepThru ( Debugger * debugger, byte * jcAddress, int64 size )
{
    byte * newDebugAddress ;
    Compile_Call_X84_ABI_RSP_ADJUST ( jcAddress ) ; // this will call jcAddress subroutine and return to our code to be compiled next
    // so that newDebugAddress, below, will be our next stepping insn
    newDebugAddress = debugger->DebugAddress + size ;
    return newDebugAddress ;
}

// CASOI : Debugger_CompileAndStepOneInstruction 

void
Debugger_CASOI_Do_Return_Insn ( Debugger * debugger )
{
    if ( Stack_Depth ( debugger->ReturnStack ) )
    {
        debugger->DebugAddress = ( byte* ) Stack_Pop ( debugger->ReturnStack ) ;
        //if ( _O_->Verbosity > 1 ) CSL_PrintReturnStack ( ) ;
        Debugger_GetWordFromAddress ( debugger ) ;
    }
    else
    {
        //Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\r", ( byte* ) "" ) ;
        if ( GetState ( debugger, ( DBG_AUTO_MODE | DBG_CONTINUE_MODE ) ) ) 
            Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\r", ( byte* ) "" ) ;
        SetState ( debugger, DBG_STACK_OLD, true ) ;
        debugger->CopyRSP = 0 ;
        if ( GetState ( debugger, DBG_BRK_INIT ) ) SetState_TrueFalse ( debugger, DBG_INTERPRET_LOOP_DONE | DBG_STEPPED, DBG_ACTIVE | DBG_BRK_INIT | DBG_STEPPING ) ;
        else SetState_TrueFalse ( debugger, DBG_INTERPRET_LOOP_DONE | DBG_STEPPED, DBG_ACTIVE | DBG_STEPPING ) ;
        if ( debugger->w_Word ) SetState ( debugger->w_Word, STEPPED, true ) ;
        debugger->DebugAddress = 0 ;
        SetState ( debugger->cs_Cpu, CPU_SAVED, false ) ;
        Set_DataStackPointers_FromDebuggerDspReg ( ) ;
    }
}

Boolean
_Debugger_CASOI_Do_JmpOrCall_Insn ( Debugger * debugger, byte * jcAddress )
{
    //jcAddress = JumpCallInstructionAddress ( debugger->DebugAddress ) ;
    Word * word = Word_UnAlias ( Word_GetFromCodeAddress ( jcAddress ) ) ;
    debugger->w_Word = word ;
    if ( word && ( word->W_MorphismAttributes & ( DEBUG_WORD | RT_STEPPING_DEBUG ) ) )
    {
        SetState ( debugger, DBG_AUTO_MODE, false ) ;
        // we are already stepping here and now, so skip
        Printf ( ( byte* ) "\nskipping over a rt breakpoint debug word : %s : at 0x%-8x", ( char* ) c_gd ( word->Name ), debugger->DebugAddress ) ;
        //Pause () ;
        debugger->DebugAddress += 3 ; // 3 : sizeof call reg insn
        //goto end ; // skip it
        return false ;
    }
    return true ;
}

void
Debugger_CASOI_Do_JmpOrCall_Insn ( Debugger * debugger, byte * jcAddress )
{
    if ( ! _Debugger_CASOI_Do_JmpOrCall_Insn ( debugger, jcAddress ) )
    {
        Debugger_CASOI_UpdateInfo ( debugger ) ;
    }
    else _Debugger_CompileAndStepOneInstruction ( debugger, jcAddress ) ;
}

void
Debugger_CASOI_Do_Jcc_Insn ( Debugger * debugger, byte * jcAddress )
{
    SetState ( debugger, DBG_JCC_INSN, true ) ;
    jcAddress = Debugger_DoJcc ( debugger, 2 ) ;
    _Debugger_CompileAndStepOneInstruction ( debugger, jcAddress ) ;
    SetState ( debugger, DBG_JCC_INSN, false ) ;
}

// ... not quite sure about this one ... ??

byte *
Debugger_CASOI_Do_IncDec_Insn ( Debugger * debugger, byte * jcAddress )
{
    //-----------------------------------------------------------------------
    //   modRm byte ( bits )  mod 0 : no disp ; mod 1 : 1 byte disp : mod 2 : 4 byte disp ; mod 3 : just reg value
    //    mod     reg      rm
    //   7 - 6   5 - 3   2 - 0
    //-----------------------------------------------------------------------
    byte * address = debugger->DebugAddress ;
    byte modRm = * ( byte* ) ( address + 1 ) ; // 1 : 1 byte opCode
    if ( modRm & 32 ) SyntaxError ( 1 ) ; // we only currently compile call reg code 2/3, << 3 ; not jmp; jmp reg code == 4/5 : reg code 100/101 ; inc/dec 0/1 : << 3
    int64 mod = modRm & 192 ; // 192 : CALL_JMP_MOD_RM : RM not inc/dec
    if ( mod == 192 ) jcAddress = ( byte* ) _Debugger_->cs_Cpu->Rax ;
    // else it could be inc/dec
    return jcAddress ;
}

void
Debugger_CASOI_Do_Test_Insn ( Debugger * debugger, byte * jcAddress )
{
    SetState ( debugger, DBG_JCC_INSN, true ) ;
    jcAddress = Debugger_DoJcc ( debugger, 1 ) ;
    _Debugger_CompileAndStepOneInstruction ( debugger, jcAddress ) ;
    SetState ( debugger, DBG_JCC_INSN, false ) ;
    Debugger_CASOI_UpdateInfo ( debugger ) ;
}

void
Debugger_CASOI_UpdateInfo ( Debugger * debugger )
{
    if ( debugger->DebugAddress )
    {
        // keep eip - instruction pointer - up to date ..
        debugger->cs_Cpu->Rip = ( uint64 * ) debugger->DebugAddress ;
        _Dsp_ = debugger->cs_Cpu->R14d ;
    }
}


