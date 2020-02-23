
#include "../include/csl.h"

// we have the address of a jcc insn 
// get the address it jccs to

byte *
JccInstructionAddress_2Byte ( byte * address )
{
    int32 offset = * ( int32* ) ( address + 2 ) ; // 2 : 2 byte opCode
    byte * jcAddress = address + offset + 6 ; // 6 : sizeof 0f jcc insn - 0x0f8x - includes 2 byte opCode
    return jcAddress ;
}

byte *
JccInstructionAddress_1Byte ( byte * address )
{
    int32 offset = ( int32 ) * ( byte* ) ( address + 1 ) ; // 1 : 1 byte opCode
    byte * jcAddress = address + offset + 2 ; // 2 : sizeof 0f jcc insn - 0x7x - includes 1 byte opCode
    return jcAddress ;
}

// we have the address of a jmp/call insn 
// get the address it jmp/calls to

byte *
JumpCallInstructionAddress ( byte * address )
{
    byte *jcAddress ;
    byte insn = * address ;

    if ( ( insn == JMPI32 ) || ( insn == JCC32 ) )
    {
        int32 offset32 = * ( int32* ) ( address + 1 ) ; // 1 : 1 byte opCode
        jcAddress = address + offset32 + 5 ; // 5 : sizeof jmp insn - includes 1 byte opcode
    }
    else
    {
        int8 offset8 = * ( byte* ) ( address + 1 ) ;
        jcAddress = address + offset8 + 2 ;
    }
    return jcAddress ;
}

byte *
JumpCallInstructionAddress_X64ABI ( byte * address )
{
    int64 offset ;
    if ( ( ( * ( address - 20 ) ) == 0x49 ) && ( ( * ( address - 19 ) ) == 0xb8 ) ) offset = 18 ;
    else offset = 8 ; 
    byte * jcAddress = * ( byte** ) ( address - offset ) ; //JumpCallInstructionAddress ( debugger->DebugAddress ) ;
    return jcAddress ;
}

void
_CSL_ACharacterDump ( char aChar )
{
    if ( isprint ( aChar ) ) Printf ( ( byte* ) "%c", aChar ) ;
    else Printf ( ( byte* ) "." ) ;
}

void
CSL_CharacterDump ( byte * address, int64 number )
{
    int64 i ;
    for ( i = 0 ; i < number ; i ++ ) _CSL_ACharacterDump ( address [ i ] ) ;
    Printf ( ( byte* ) " " ) ;
}

void
_CSL_AByteDump ( byte aByte )
{
    Printf ( ( byte* ) "%02x ", aByte ) ;
}

void
CSL_NByteDump ( byte * address, int64 number )
{
    int64 i ;
    for ( i = 0 ; i < number ; i ++ ) _CSL_AByteDump ( address [ i ] ) ;
    Printf ( ( byte* ) " " ) ;
}

byte *
GetPostfix ( byte * address, byte* postfix, byte * buffer )
{
    byte * iaddress = 0, *str ;
    Word * word = 0, *dbgWord = _Debugger_->w_Word ;
    char * prePostfix = ( char* ) "  \t" ;
    if ( iaddress = Calculate_Address_FromOffset_ForCallOrJump ( address ) ) 
    {
        //if ( dbgWord ) //&& ( Is_NamespaceType ( dbgWord ) ) )
        {
            if ( ( word = Word_GetFromCodeAddress_NoAlias ( iaddress ) ) )
            {
                //if ( word = Word_GetFromCodeAddress ( iaddress ) )
                {
                    byte * name = ( byte* ) c_gd ( word->Name ) ;
                    byte *containingNamespace = word->ContainingNamespace ? word->ContainingNamespace->Name : ( byte* ) "" ;
                    if ( ( byte* ) word->CodeStart == iaddress )
                    {
                        snprintf ( ( char* ) buffer, 128, "%s< %s.%s : " UINT_FRMT " >%s", prePostfix, containingNamespace, name, 
                            ( uint64 ) iaddress, postfix ) ;
                    }
                    else
                    {
                        //snprintf ( ( char* ) buffer, 128, "%s< %s.%s+%ld >%s", prePostfix,
                        //    containingNamespace, name, iaddress - ( byte* ) word->CodeStart, postfix ) ;
                        snprintf ( ( char* ) buffer, 128, "%s< %s.%s+%ld", prePostfix,
                            containingNamespace, name, iaddress - ( byte* ) word->CodeStart ) ;//, postfix ) ;
                        strcat ( buffer, c_u ( " >") ) ;
                        //strcat ( buffer, postfix ) ;
                    }
                }
            }
        }
        if ( ! word ) snprintf ( ( char* ) buffer, 128, "%s< %s >", prePostfix, ( char * ) "C compiler code" ) ;
        postfix = buffer ;
    }
    else
    {
        str = String_CheckForAtAdddress ( *( ( byte ** ) ( address + 2 ) ) ) ;
        if ( str )
        {
            snprintf ( ( char* ) buffer, 128, "%s%s", prePostfix, str ) ;
            postfix = buffer ;
        }
    }
    return postfix ;
}

void
Compile_Debug_GetRSP ( ) // where we want the acquired pointer
{
    _Compile_PushReg ( ACC ) ;
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & _Debugger_->DebugRSP, RSP, ACC ) ; // esp 
    _Compile_PopToReg ( ACC ) ;
    _Compile_Stack_PushReg ( DSP, ACC ) ;
}

void
CSL_SetRtDebugOn ( )
{
    SetState ( _CSL_, RT_DEBUG_ON, true ) ;
    SetState ( _Debugger_, (DBG_INTERPRET_LOOP_DONE), true ) ;
}

void
Compile_DebugRuntimeBreakpointFunction ( block function ) // where we want the acquired pointer
{
    Compile_Call_TestRSP ( ( byte* ) CSL_SetRtDebugOn ) ;
    Compile_Call ( ( byte* ) _Debugger_->SaveCpuState ) ;
    Compile_Call_TestRSP ( ( byte* ) function ) ;
}

void
_CSL_DebugRuntimeBreakpoint ( ) // where we want the acquired pointer
{
    Compile_DebugRuntimeBreakpointFunction ( CSL_DebugRuntimeBreakpoint ) ;
}

void
_CSL_DebugRuntimeBreakpoint_IsDebugShowOn ( ) // where we want the acquired pointer
{
    Compile_DebugRuntimeBreakpointFunction ( CSL_DebugRuntimeBreakpoint_IsDebugShowOn ) ;
}

void
_CSL_DebugRuntimeBreakpoint_IsDebugOn ( ) // where we want the acquired pointer
{
    Compile_DebugRuntimeBreakpointFunction ( CSL_DebugRuntimeBreakpoint_IsDebugOn ) ;
}

