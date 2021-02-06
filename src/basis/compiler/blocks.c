
#include "../../include/csl.h"

BlockInfo *
BI_Block_Copy ( BlockInfo * bi, byte* dstAddress, byte * srcAddress, int64 bsize, Boolean optSetupFlag )
{
    Compiler * compiler = _Compiler_ ;
    if ( ! bi ) bi = ( BlockInfo * ) Stack_Top ( compiler->CombinatorBlockInfoStack ) ;
    byte * saveHere = Here, * saveAddress = srcAddress ;
    ud_t * ud = Debugger_UdisInit ( _Debugger_ ) ;
    int64 isize, left ;
    if ( dstAddress ) SetHere ( dstAddress, 1 ) ;
    bi->CopiedToStart = Here ;
    //CSL_AdjustDbgSourceCode_ScInUseFalse ( srcAddress ) ;
    for ( left = bsize ; ( left > 0 ) ; srcAddress += isize )
    {
        //if ( optSetupFlag ) 
        PeepHole_Optimize ( ) ;
        isize = _Udis_GetInstructionSize ( ud, srcAddress ) ;
        left -= isize ;
        CSL_AdjustDbgSourceCodeAddress ( srcAddress, Here ) ;
        CSL_AdjustLabels ( srcAddress ) ;
        if ( * srcAddress == _RET )
        {
            if ( ( left > 0 ) && ( ( * srcAddress + 1 ) != NOOP ) ) //  noop from our logic overwrite
            {
                // ?? unable at present to compile inline with more than one return in the block
                SetHere ( saveHere, 1 ) ;
                Compile_Call ( saveAddress ) ;
            }
            else break ; // don't include RET
        }
        else if ( * srcAddress == CALLI32 )
        {
            int32 offset = * ( int32* ) ( srcAddress + 1 ) ; // 1 : 1 byte CALLI32 opCode
            if ( ! offset )
            {
                dllist_Map1 ( compiler->GotoList, ( MapFunction1 ) AdjustGotoJmpOffsetPointer, ( int64 ) ( srcAddress + 1 ) ) ;
                CSL_SetupRecursiveCall ( ) ;
                continue ;
            }
            else
            {
                byte * jcAddress = JumpCallInstructionAddress ( srcAddress ) ;
                Word * word = Word_GetFromCodeAddress ( jcAddress ) ;
                if ( word )
                {
                    _Word_Compile ( word ) ;
                    continue ;
                }
            }
        }
        else if ( * srcAddress == JMPI32 )
        {
            int64 offset = * ( int32* ) ( srcAddress + 1 ) ; // 1 : 1 byte JMPI32 opCode - e9
            if ( offset )
            {
                dllist_Map1 ( compiler->GotoList, ( MapFunction1 ) AdjustGotoJmpOffsetPointer, ( int64 ) ( srcAddress + 1 ) ) ;
            }
            else dllist_Map1 ( compiler->GotoList, ( MapFunction1 ) _AdjustGotoInfo, ( int64 ) srcAddress ) ; //, ( int64 ) end ) ;
        }
        if ( * srcAddress )  _CompileN ( srcAddress, isize ) ;
        else break ;
        //if ( _DBI || _O_->Dbi ) Debugger_UdisOneInstruction ( _Debugger_, srcAddress, ( byte* ) "", ( byte* ) "" ) ;
        //if ( _DBI  ) Debugger_UdisOneInstruction ( _Debugger_, srcAddress, ( byte* ) "", ( byte* ) "" ) ;
    }
    bi->CopiedToEnd = Here ;
    bi->CopiedSize = bi->CopiedToEnd - bi->CopiedToStart ;
    d0 ( if ( Is_DebugModeOn ) Debugger_Disassemble ( _Debugger_, bi->CopiedToStart, bi->CopiedSize, 1 ) ) ;
    return bi ;
}

void
Compile_BlockLogicTest (BlockInfo * bi)
{
    int64 diff ;
    if ( bi )
    {
        if ( ( bi->JccLogicCode || ( bi->LogicCodeWord && bi->LogicCodeWord->StackPushRegisterCode ) ) )
        {
            if ( ! bi->JccLogicCode ) bi->JccLogicCode = bi->LogicCodeWord->StackPushRegisterCode ;
            diff = bi->JccLogicCode - bi->bp_First ; // find its diff position in original block
            bi->CopiedToLogicJccCode = bi->CopiedToStart + diff ; // use diff in copied block
            if ( bi->LogicCodeWord && ( bi->LogicCodeWord->W_OpInsnCode != CMP ) ) // cmp sets flags no need for test code //CAttribute & CATEGORY_LOGIC ) ) )
            {
                SetHere ( bi->CopiedToLogicJccCode, 1 ) ;
                Compiler_SCA_Word_SetCodingHere_And_ClearPreviousUse ( bi->LogicCodeWord, 0 ) ;
                BI_CompileRecord_TestCode_Reg ( bi, bi->LogicCodeWord->RegToUse, CELL ) ;
                bi->CopiedToLogicJccCode = Here ;
                //if ( logicFlag == 2 ) BI_Set_Tttn ( bi, TTT_ZERO, NEGFLAG_OFF, TTT_ZERO, NEGFLAG_OFF ) ;
                //else 
                BI_Set_Tttn ( bi, TTT_ZERO, NEGFLAG_ON, TTT_ZERO, NEGFLAG_OFF ) ;
            }
            else if ( bi->LogicCodeWord && ( bi->LogicCodeWord->W_MorphismAttributes & CATEGORY_OP_1_ARG ) && ( bi->LogicCodeWord->W_MorphismAttributes & LOGIC_NEGATE ) )
            {
                SetHere ( bi->LogicCodeWord->Coding, 1 ) ;
                Compiler_SCA_Word_SetCodingHere_And_ClearPreviousUse ( bi->LogicCodeWord, 0 ) ;
                BI_CompileRecord_TestCode_Reg ( bi, bi->LogicCodeWord->RegToUse, CELL ) ;
                bi->CopiedToLogicJccCode = Here ;
                //if ( logicFlag == 2 ) BI_Set_Tttn ( bi, TTT_ZERO, NEGFLAG_OFF, TTT_ZERO, NEGFLAG_OFF ) ;
                //else 
                BI_Set_Tttn ( bi, TTT_ZERO, NEGFLAG_ON, TTT_ZERO, NEGFLAG_ON ) ;
            }
        }
        else
        {
            _Compile_GetTestLogicFromTOS ( bi ) ;
            bi->CopiedToLogicJccCode = Here ;
        }
    }
}

byte *
Block_CopyCompile ( byte * srcAddress, int64 bindex, Boolean jccFlag, byte* jmpToAddr )
{
    byte * compiledAtAddress = 0 ;
    Compiler * compiler = _Context_->Compiler0 ;
    BlockInfo *bi = ( BlockInfo * ) _Stack_Pick ( compiler->CombinatorBlockInfoStack, bindex ) ;
    BI_Block_Copy ( bi, Here, srcAddress, bi->bp_Last - bi->bp_First, 1 ) ; //nb!! 0 : turns off peephole optimization ; peephole optimization will be done in CSL_EndCombinator
    if ( jccFlag )
    {
        Boolean logicFlag = (jccFlag == 2) ;
        Compile_BlockLogicTest (bi) ;
        compiledAtAddress = _BI_Compile_Jcc ( bi, jmpToAddr, logicFlag ) ;
        Stack_Push_PointerToJmpOffset ( compiledAtAddress ) ;
        bi->CopiedToEnd = Here ;
        bi->CopiedSize = bi->CopiedToEnd - bi->CopiedToStart ;
        //d1 ( if ( Is_DebugModeOn ) Debugger_Disassemble ( _Debugger_, ( byte* ) bi->CopiedToStart, bi->CopiedSize, 1 ) ) ;
    }
    return compiledAtAddress ;
}

