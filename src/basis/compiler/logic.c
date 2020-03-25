
#include "../../include/csl.h"

// 'setTtnn' is a notation from the intel manuals

void
_BI_Set_Tttn ( BlockInfo *bi, Boolean setTtn, Boolean setNegFlag, Boolean jccTtt, Boolean jccNegFlag, byte * jccLogicCode )
{
    bi->SetccTtt = setTtn ;
    bi->SetccNegFlag = setNegFlag ;
    bi->JccLogicCode = Here ; // used by combinators
    bi->JccTtt = jccTtt ;
    bi->JccNegFlag = jccNegFlag ;
}

void
BI_Set_Tttn ( BlockInfo *bi, Boolean setTtn, Boolean setNegFlag, Boolean jccTtt, Boolean jccNegFlag )
{
    _BI_Set_Tttn ( bi, setTtn, setNegFlag, jccTtt, jccNegFlag, Here ) ;
}

void
Compiler_Set_BI_Tttn ( Compiler * compiler, Boolean setTtn, Boolean setNegFlag, Boolean jccTtt, Boolean jccNegFlag )
{
    BlockInfo *bi = ( BlockInfo * ) Stack_Top ( compiler->CombinatorBlockInfoStack ) ;
    BI_Set_Tttn ( bi, setTtn, setNegFlag, jccTtt, jccNegFlag ) ;
}

void
_Compile_TestCode ( Boolean reg, Boolean size )
{
    _Compile_TEST_Reg_To_Reg ( reg, reg, size ) ;
}

void
BI_CompileRecord_TestCode_Reg (BlockInfo *bi, Boolean reg, Boolean size)
{
    if ( ( ( ! bi->LogicTestCode ) || ( Here != ( bi->LogicTestCode + 3 ) ) ) ) // prevent two "test reg, reg" in a row
    {
        Compiler_WordStack_SCHCPUSCA ( 0, 1 ) ;
        bi->LogicTestCode = Here ;
        _Compile_TestCode ( reg, size ) ;
    }
}

void
BI_CompileRecord_TestCode_ArgRegNum ( BlockInfo *bi, uint8 argRegNum )
{
    BI_CompileRecord_TestCode_Reg (bi, _COI_GetReg ( _Compiler_->OptInfo, argRegNum ), CELL) ;
}

BlockInfo *
Compiler_BI_CompileRecord_TestCode_Reg ( Compiler * compiler, Boolean reg, Boolean size, Boolean force )
{
    BlockInfo *bi = ( BlockInfo * ) Stack_Top ( compiler->CombinatorBlockInfoStack ) ;
    BI_CompileRecord_TestCode_Reg (bi, reg, size) ;
    return bi ;
}

Boolean
_COI_GetReg ( CompileOptimizeInfo * optInfo, Boolean regNumber )
{
    Boolean reg = ACC, reg1, reg2 ;
    if ( GetState ( _CSL_, OPTIMIZE_ON ) )
    {
        if ( regNumber == 1 )
        {
            if ( ! optInfo->wordArg1 ) reg1 = optInfo->wordArg2 ? optInfo->wordArg2->RegToUse : ACC ;
            else reg1 = optInfo->wordArg1->RegToUse ;
            return reg = reg1 ;
        }
        else if ( regNumber == 2 )
        {
            reg2 = optInfo->wordArg2 ? optInfo->wordArg2->RegToUse : ( optInfo->NumberOfArgs == 2 ) ? OREG : ACC ;
            return reg = reg2 ;
        }
    }
    return reg ;
}

void
Compiler_BI_CompileRecord_TestCode_ArgRegNum ( Compiler * compiler, Boolean argRegNum )
{
    Compiler_BI_CompileRecord_TestCode_Reg ( compiler, _COI_GetReg ( compiler->OptInfo, argRegNum ), CELL, false ) ;
}

void
Compiler_BI_CompileRecord_TestCode_Set_Tttn ( Compiler * compiler, Boolean reg, Boolean setTtn, Boolean setNegFlag,
    Boolean jccTtt, Boolean jccNegFlag, Boolean force )
{
    BlockInfo *bi = Compiler_BI_CompileRecord_TestCode_Reg ( compiler, reg, CELL, force ) ;
    BI_Set_Tttn ( bi, setTtn, setNegFlag, jccTtt, jccNegFlag ) ; //ZERO_TTT, setNegFlag ) ;
}

// cf. : Compile_BlockInfoTestLogic ( Compiler * compiler, int8 reg, int8 setNegFlag )

void
_Compile_GetTestLogicFromTOS ( BlockInfo *bi )
{
    Compile_Pop_To_Acc ( DSP ) ;
    BI_CompileRecord_TestCode_ArgRegNum ( bi, 1 ) ;
    BI_Set_Tttn ( bi, TTT_ZERO, NEGFLAG_NZ, TTT_ZERO, NEGFLAG_Z ) ;
}

// nb : only blocks with one ret insn can be successfully compiled inline

void
_Compile_LogicResultForStack ( int64 reg, Boolean setTtn, Boolean setNegFlag )
{
    //_Compile_Jcc ( setNegFlag, setTtn, Here + 14, JCC8 ) ; // 8 bit versoin : if eax is zero return not(R8) == 1 else return 0
    _Compile_Jcc ( setNegFlag, setTtn, Here + 11, JCC8 ) ; // if eax is zero return not(R8) == 1 else return 0
    Compile_MoveImm_To_Reg ( reg, 0, CELL_SIZE ) ; // 6 bytes
    //return 0 in reg :
    //_Compile_JumpWithOffset ( 10 ) ; // 8 bit version
    _Compile_JumpWithOffset ( 7 ) ; // 
    //return 1 in reg :
    Compile_MoveImm_To_Reg ( reg, 1, CELL_SIZE ) ;
    //_Set_JccLogicCode ( _Compiler_ ) ;
    //DBI_OFF ;
}

BlockInfo *
Compiler_Set_LogicCode ( Compiler * compiler, Boolean setTtn, Boolean setNegFlag, Boolean jccTtt, Boolean jccNegFlag )
{
    BlockInfo *bi = ( BlockInfo * ) Stack_Top ( compiler->CombinatorBlockInfoStack ) ;
    BI_Set_Tttn ( bi, setTtn, setNegFlag, jccTtt, jccNegFlag ) ; //setTtn, setNegFlag ) ;
    return bi ;
}

void
_Compile_LogicalAnd ( Compiler * compiler )
{
    Compiler_WordStack_SCHCPUSCA ( 0, 1 ) ;
    //DBI_ON ;
    Compiler_BI_CompileRecord_TestCode_Set_Tttn ( compiler, OREG, TTT_ZERO, NEGFLAG_Z, TTT_ZERO, NEGFLAG_NZ, false ) ; // jz
    _Compile_Jcc ( NEGFLAG_Z, TTT_ZERO, Here + 7, JCC8 ) ; // if eax is zero return not(R8) == 1 else return 0
    Compiler_BI_CompileRecord_TestCode_Set_Tttn ( compiler, ACC, TTT_ZERO, NEGFLAG_NZ, TTT_ZERO, NEGFLAG_Z, true ) ;
    _Compile_LogicResultForStack ( ACC, TTT_ZERO, NEGFLAG_NZ ) ; // jnz
    Compiler_Set_LogicCode ( compiler, TTT_ZERO, NEGFLAG_NZ, TTT_ZERO, NEGFLAG_Z ) ;
    CSL_CompileAndRecord_Word0_PushReg ( ACC, true ) ;
    //DBI_OFF ;
}

void
Compile_LogicalAnd ( Compiler * compiler )
{
    int64 optSetupFlag = Compiler_CheckOptimize ( compiler, 0 ) ;
    if ( optSetupFlag & OPTIMIZE_DONE ) return ;
    else if ( optSetupFlag ) _Compile_LogicalAnd ( compiler ) ;
    else
    {
        Word *one = _CSL_WordList ( 1 ) ; // assumes two values ( n m ) on the DSP stack 
        if ( one->StackPushRegisterCode && ( one->RegToUse == ACC ) ) SetHere ( one->StackPushRegisterCode, 1 ) ;
        else _Compile_Stack_PopToReg ( DSP, ACC ) ;
        _Compile_Stack_PopToReg ( DSP, OREG ) ;
        _Compile_LogicalAnd ( compiler ) ;
    }
}

void
_Compile_LogicalNot ( Compiler * compiler )
{
    //DBI_ON ;
    //byte * here = Here ;
    Compiler_WordStack_SCHCPUSCA ( 0, 1 ) ;
    Compiler_BI_CompileRecord_TestCode_Set_Tttn ( compiler, ACC, TTT_ZERO, NEGFLAG_Z, TTT_ZERO, NEGFLAG_NZ, false ) ;
    _Compile_LogicResultForStack ( ACC, TTT_ZERO, NEGFLAG_Z ) ;
    Compiler_Set_LogicCode ( compiler, TTT_ZERO, NEGFLAG_Z, TTT_ZERO, NEGFLAG_NZ ) ;
    CSL_CompileAndRecord_Word0_PushReg ( ACC, true ) ;
    //_Compile_Move_Reg_To_StackN ( DSP, 0, ACC ) ; // we took value from TOS and return 'not' value to TOS
    //_Printf ( (byte*) "\nSize of LogicalNot code = %d bytes\n", Here -here ) ;
    //DBI_OFF ;
}

void
_Compile_SETcc ( Boolean setTtn, Boolean setNegFlag, Boolean reg )
{
    //DBI_ON ;
    uint8 opCode0, opCode1, modRm, rex ;
    rex = Calculate_Rex ( 0, reg, 0, 0 ) ; //REX_B ) ; //( immSize == 8 ) || ( controlFlag & REX_B ) ) ;
    opCode0 = ( byte ) 0x0f ;
    opCode1 = ( ( 0x9 << 4 ) | ( setTtn << 1 ) | setNegFlag ) ;
    modRm = CalculateModRmByte ( REG, reg, 0, 0, 0 ) ;
    //_Compile_Write_Instruction_X64 ( int8 rex, uint8 opCode0, uint8 opCode1, int8 modRm, int16 controlFlags, int8 sib, int64 disp, int8 dispSize, int64 imm, int8 immSize )
    _Compile_Write_Instruction_X64 ( rex, opCode0, opCode1, modRm, MODRM_B, 0, 0, 0, 0, 0 ) ; //controlFlags, sib, disp, dispSize, imm, immSize ) ;
    //DBI_OFF ;
}

// SET : 0x0f 0x9setTtnn mod 000 rm/reg
// ?!? wanna use TEST insn here to eliminate need for _Compile_MOVZX_REG insn ?!? is that possible

void
_Compile_SETcc_Tttn_REG ( Compiler * compiler, Boolean setTtn, Boolean setNegFlag, Boolean jccTtt, Boolean jccNegFlag, Boolean reg, Boolean rm )
{
    Compiler_Set_BI_Tttn ( compiler, setTtn, setNegFlag, jccTtt, jccNegFlag ) ; // 6 : 0f9ec0 : setle al ; 480fb6c0 movzx rax, al :: 7 - 1 for initial 'ret' : i've forgotten *how, exactly* this actually works ??          
    _Compile_SETcc ( setTtn, setNegFlag, reg ) ;
    _Compile_MOVZX_BYTE_REG ( reg, rm ) ;
}

// setTtn n : notation from intel manual 253667 ( N-Z ) - table B-10 : setTtn = condition codes, n is a negation bit
// setTtnn notation is used with the SET and JCC instructions

// note : intex syntax  : instruction dst, src
//        att   syntax  : instruction src, dst
// cmp : compares by subtracting src from dst, dst - src, and setting eflags like a "sub" insn 
// eflags affected : cf of sf zf af pf : Intel Instrucion Set Reference Manual for "cmp"
// ?? can this be done better with test/jcc ??
// want to use 'test eax, 0' as a 0Branch (cf. jonesforth) basis for all block conditionals like if/else, do/while, for ...

void
Compile_Cmp_Set_Tttn_Logic ( Compiler * compiler, Boolean setTtn, Boolean setNegateFlag, Boolean jccTtt, Boolean jccNegFlag )
{
    Is_DebugOn_DBI ;
    int64 optSetupFlag = Compiler_CheckOptimize ( compiler, 0 ) ;
    if ( optSetupFlag & OPTIMIZE_DONE ) return ;
    else if ( optSetupFlag )
    {
        if ( compiler->OptInfo->OptimizeFlag & OPTIMIZE_IMM )
        {
            if ( ( setTtn == TTT_EQUAL ) && ( compiler->OptInfo->Optimize_Imm == 0 ) ) //Compile_TEST ( compiler->OptInfo->Optimize_Mod, compiler->OptInfo->Optimize_Rm, 0, compiler->OptInfo->Optimize_Disp, compiler->OptInfo->Optimize_Imm, CELL ) ;
            {
                if ( compiler->OptInfo->COIW [2]->StackPushRegisterCode ) SetHere ( compiler->OptInfo->COIW [2]->StackPushRegisterCode, 1 ) ; // leave optInfo_0_two value in ACCUM we don't need to push it
                Compiler_BI_CompileRecord_TestCode_ArgRegNum ( compiler, 1 ) ;
            }
            else
            {
                int64 size ;
                if ( compiler->OptInfo->Optimize_Imm >= 0x100000000 )
                {
                    size = 8 ;
                    setNegateFlag = ! setNegateFlag ;
                }
                else size = 0 ;
                WordList_SetCoding ( 0, Here ) ;
                Compile_CMPI ( compiler->OptInfo->Optimize_Mod, compiler->OptInfo->Optimize_Reg, 
                    compiler->OptInfo->Optimize_Disp, compiler->OptInfo->Optimize_Imm, size ) ;
            }
        }
        else
        {
            // Compile_CMP( toRegOrMem, mod, reg, rm, sib, disp )
            WordList_SetCoding ( 0, Here ) ;
            Compile_CMP ( compiler->OptInfo->Optimize_Dest_RegOrMem, compiler->OptInfo->Optimize_Mod,
                compiler->OptInfo->Optimize_Reg, compiler->OptInfo->Optimize_Rm, 0, compiler->OptInfo->Optimize_Disp, CELL_SIZE ) ;
        }
    }
    else
    {
        _Compile_Move_StackN_To_Reg ( OREG, DSP, 0 ) ;
        _Compile_Move_StackN_To_Reg ( ACC, DSP, - 1 ) ;
        // must do the DropN before the CMP because CMP sets eflags 
        _Compile_Stack_DropN ( DSP, 2 ) ; // before cmp 
        WordList_SetCoding ( 0, Here ) ;
        Compile_CMP ( REG, REG, ACC, OREG, 0, 0, CELL ) ;
    }
    Boolean reg = ACC ; //nb! reg should always be ACC! : immediately after the 'cmp' insn which changes the flags appropriately
    _Compile_SETcc_Tttn_REG ( compiler, setTtn, setNegateFlag, jccTtt, jccNegFlag, reg, reg ) ; //nb! should always be ACC! : immediately after the 'cmp' insn which changes the flags appropriately
    _Word_CompileAndRecord_PushReg ( _CSL_WordList ( 0 ), reg, true ) ; //ACC ) ;
    DBI_OFF ;
}
//  logical equals - "=="

void
Compile_Equals ( Compiler * compiler )
{
    Compile_Cmp_Set_Tttn_Logic ( compiler, TTT_EQUAL, NEGFLAG_OFF, TTT_EQUAL, NEGFLAG_ON ) ; // ??
}

void
Compile_DoesNotEqual ( Compiler * compiler )
{
    Compile_Cmp_Set_Tttn_Logic ( compiler, TTT_EQUAL, NEGFLAG_ON, TTT_EQUAL, NEGFLAG_OFF ) ; // ??
}

void
Compile_LessThan ( Compiler * compiler )
{
    Compile_Cmp_Set_Tttn_Logic ( compiler, TTT_LESS, NEGFLAG_OFF, TTT_LESS, NEGFLAG_ON ) ;
}

void
Compile_GreaterThan ( Compiler * compiler )
{
    Compile_Cmp_Set_Tttn_Logic ( compiler, TTT_LE, NEGFLAG_ON, TTT_LE, NEGFLAG_OFF ) ;
}

void
Compile_LessThanOrEqual ( Compiler * compiler )
{
    Compile_Cmp_Set_Tttn_Logic ( compiler, TTT_LE, NEGFLAG_OFF, TTT_LE, NEGFLAG_ON ) ;
}

void
Compile_GreaterThanOrEqual ( Compiler * compiler )
{
    Compile_Cmp_Set_Tttn_Logic ( compiler, TTT_LESS, NEGFLAG_ON, TTT_LESS, NEGFLAG_OFF ) ;
}

void
Compile_TestLogicAndStackPush (Compiler * compiler, Boolean reg, Boolean setNegFlag, Boolean jccTtt, Boolean jccNegFlag )
{
    Compiler_BI_CompileRecord_TestCode_Set_Tttn ( compiler, reg, TTT_ZERO, setNegFlag, jccTtt, jccNegFlag, false ) ;
    CSL_CompileAndRecord_PushAccum ( ) ;
}

void
Compile_Logical_X ( Compiler * compiler, int64 op, Boolean setTtn, Boolean setNegateFlag, Boolean jccTtt, Boolean jccNegFlag )
{
    int64 optSetupFlag = Compiler_CheckOptimize ( compiler, 0 ) ;
    if ( optSetupFlag == OPTIMIZE_DONE ) return ;
    else if ( optSetupFlag )
    {
        // TODO : this optimization somehow is *very* little used, why is that ?!? 
        // assumes we have unordered operands in eax, ecx
        _Compile_X_Group1 ( op, REG, REG, ACC, OREG, 0, 0, CELL ) ;
    }
    else
    {
        // operands are still on the stack
        _Compile_Move_StackN_To_Reg ( ACC, DSP, 0 ) ;
        //_Compile_Group1 ( int64 code, int64 toRegOrMem, int64 mod, int64 reg, int64 rm, int64 sib, int64 disp, int64 osize )
        _Compile_X_Group1 ( op, REG, MEM, ACC, DSP, 0, CELL, CELL ) ;
        _Compile_Stack_DropN ( DSP, 2 ) ;
    }
    Compile_TestLogicAndStackPush (compiler, ACC, setNegateFlag, jccTtt, jccNegFlag ) ; //NEGFLAG_NZ, TTT_ZERO, NEGFLAG_Z ) ;
}

void
Compile_LogicalNot ( Compiler * compiler )
{
    Word *one = _CSL_WordList ( 1 ) ; // assumes two values ( n m ) on the DSP stack 
    int64 optSetupFlag = Compiler_CheckOptimize ( compiler, 0 ) ; // check especially for cases that optimize literal ops
    CompileOptimizeInfo * optInfo = compiler->OptInfo ;
    if ( optSetupFlag & OPTIMIZE_DONE ) return ;
        // just need to get value to be operated on ( not'ed ) in eax
    else if ( optSetupFlag )
    {
        if ( optInfo->OptimizeFlag & OPTIMIZE_IMM ) Compile_MoveImm_To_Reg ( ACC, optInfo->Optimize_Imm, CELL ) ;
        else if ( optInfo->Optimize_Rm == DSP )
        {
            if ( one->StackPushRegisterCode ) SetHere ( one->StackPushRegisterCode, 1 ) ;
            else _Compile_Move_StackN_To_Reg ( ACC, DSP, 0 ) ;
        }
        else if ( optInfo->Optimize_Rm != ACC ) Compile_GetVarLitObj_RValue_To_Reg (one, ACC , 0) ;
    }
    else
    {
        if ( one->StackPushRegisterCode ) SetHere ( one->StackPushRegisterCode, 1 ) ; // PREFIX_PARSING : nb! could be a prefix function 
        else if ( ! ( one->W_MorphismAttributes & RAX_RETURN ) ) _Compile_Stack_PopToReg ( DSP, ACC ) ;
    }
    _Compile_LogicalNot ( compiler ) ;
}


// JE, JNE, JZ, JNZ, ... see machineCode.h
byte *
_Compile_Jcc ( int64 setNegFlag, int64 setTtn, byte * jmpToAddr, byte insn )
{
    byte * compiledAtAddress = Here ;
    int32 offset ;
    if ( jmpToAddr || insn == JCC8 )
    {
        int32 offset = jmpToAddr ? _CalculateOffsetForCallOrJump ( Here + 1, jmpToAddr, insn ) : 0 ;
        if ( ( insn != JCC32 ) && ( offset < 127 ) )
        {
            Compile_CalculateWrite_Instruction_X64 ( 0, ( 0x7 << 4 | setTtn << 1 | setNegFlag ), 0, 0, 0, DISP_B, 0, offset, BYTE, 0, 0 ) ;
            return compiledAtAddress ;
        }
    }
    else offset = 0 ; // allow this function to be used to have a delayed compile of the actual address
    Compile_CalculateWrite_Instruction_X64 ( 0x0f, ( 0x8 << 4 | setTtn << 1 | setNegFlag ), 0, 0, 0, DISP_B, 0, offset, INT32_SIZE, 0, 0 ) ;
    return compiledAtAddress ;
}

byte *
_BI_Compile_Jcc ( BlockInfo *bi, byte * jmpToAddress )
{
    if ( bi->CopiedToLogicJccCode ) SetHere ( bi->CopiedToLogicJccCode, 1 ) ;
    else SetHere ( bi->JccLogicCode, 1 ) ;
    bi->ActualCopiedToJccCode = Here ;
    byte insn = GetState ( _CSL_, JCC8_ON ) ? JCC8 : 0 ;
    byte * compiledAtAddress = _Compile_Jcc ( bi->JccNegFlag, bi->JccTtt, jmpToAddress, insn ) ; // we do need to store and get this logic set by various conditions by the compiler : _Compile_SET_Tttn_REG
    return compiledAtAddress ;
}

byte *
BI_Compile_Jcc ( BlockInfo *bi, Boolean setTtn, byte * jmpToAddress ) // , int8 nz
{
    byte * compiledAtAddress ;
    if ( bi->JccLogicCode ) compiledAtAddress = _BI_Compile_Jcc ( bi, jmpToAddress ) ;
    else
    {
        Compile_BlockLogicTest ( bi ) ; // after cmp we test our condition with setcc. if cc is true a 1 will be sign extended in R8 and pushed on the stack 
        // then in the non optimized|inline case we cmp the TOS with 0. If ZERO (zf is 1) we know the test was false (for IF), if N(ot) ZERO we know it was true 
        // (for IF). So, in the non optimized|inline case if ZERO we jmp if N(ot) ZERO we continue. In the optimized|inline case we check result of first cmp; if jcc sees
        // not true (with IF that means jcc N(ot) ZERO) we jmp and if true (with IF that means jcc ZERO) we continue. 
        // nb. without optimize|inline there is another cmp in Compile_GetLogicFromTOS which reverse the polarity of the logic 
        // ?? an open question ?? i assume it works the same in all cases we are using - exceptions ?? 
        // so adjust ...
        compiledAtAddress = _Compile_Jcc ( NEGFLAG_Z, setTtn, 0, 0 ) ;
    }
    return compiledAtAddress ;
}

byte *
Compiler_Compile_Jcc ( Compiler * compiler, int64 bindex, Boolean setTtn ) // , int8 nz
{
    BlockInfo *bi = ( BlockInfo * ) _Stack_Pick ( compiler->CombinatorBlockInfoStack, bindex ) ; // -1 : remember - stack is zero based ; stack[0] is top
    byte * compiledAtAddress = BI_Compile_Jcc ( bi, setTtn, 0 ) ;
    return compiledAtAddress ;
}
// non-combinator 'if'

void
CSL_If_ConditionalExpression ( )
{
    if ( CompileMode )
    {
        byte * compiledAtAddress = Compiler_Compile_Jcc ( _Compiler_, 0, TTT_ZERO ) ;
        // N, ZERO : use inline|optimize logic which needs to get flags immediately from a 'cmp', jmp if the zero flag is not set
        // for non-inline|optimize ( reverse polarity : cf. _Compile_Jcc comment ) : jmp if cc is not true; cc is set by setcc after 
        // the cmp, or is a value on the stack. 
        // We cmp that value with zero and jmp if this second cmp sets the flag to zero else do the immediately following block code
        // ?? an explanation of the relation of the setcc terms with the flags is not clear to me yet (20110801) from the intel literature ?? 
        // but by trial and error this works; the logic i use is given in _Compile_Jcc.
        // ?? if there are problems check this area ?? cf. http://webster.cs.ucr.edu/AoA/Windows/HTML/IntegerArithmetic.html
        Stack_Push_PointerToJmpOffset ( compiledAtAddress ) ;
    }
    else
    {
        if ( String_IsPreviousCharA_ ( _Context_->ReadLiner0->InputLine, _Context_->Lexer0->TokenStart_ReadLineIndex - 1, '}' ) )
            CSL_If2Combinator ( ) ;
        else if ( String_IsPreviousCharA_ ( _Context_->ReadLiner0->InputLine, _Context_->Lexer0->TokenStart_ReadLineIndex - 1, '#' ) )
            CSL_If_ConditionalInterpret ( ) ;
        else if ( GetState ( _Context_, C_SYNTAX | PREFIX_MODE | INFIX_MODE ) ) CSL_If_PrefixCombinators ( ) ;
        else
        {
            Interpreter * interp = _Context_->Interpreter0 ;
            if ( DataStack_Pop ( ) )
            {
                byte * token ;
                // interpret until ":", "else" or "endif"
                token = Interpret_C_Until_Token4 ( interp, ( byte* ) "else", ( byte* ) "endif", ( byte* ) ":", 0, ( byte* ) " ", 1 ) ;
                if ( ( token == 0 ) || ( String_Equal ( token, "endif" ) ) ) return ;
                Parse_SkipUntil_EitherToken_OrNewline ( ( byte* ) "endif", 0 ) ;
            }
            else
            {
                // skip until ":" or "else"
                Parse_SkipUntil_EitherToken_OrNewline ( ( byte* ) ":", ( byte* ) "else" ) ;
                Interpret_C_Until_Token4 ( interp, ( byte* ) ";", ( byte* ) ",", ( byte* ) "endif", ( byte* ) "}", ( byte* ) " ", 1 ) ;
            }
        }
    }
}

// same as CSL_JMP

void
CSL_Else ( )
{
    if ( CompileMode )
    {
        byte * compiledAtAddress = Compile_UninitializedJump ( ) ; // at the end of the 'if block' we need to jmp over the 'else block'
        CSL_CalculateAndSetPreviousJmpOffset_ToHere ( ) ;
        Stack_Push_PointerToJmpOffset ( compiledAtAddress ) ;
    }
    else
    {
        if ( String_IsPreviousCharA_ ( _Context_->ReadLiner0->InputLine, _Context_->Lexer0->TokenStart_ReadLineIndex - 1, '#' ) ) CSL_Else_ConditionalInterpret ( ) ;
        else
        {
            Interpret_Until_Token ( _Context_->Interpreter0, ( byte* ) "endif", 0 ) ;
        }
    }
}

void
CSL_EndIf ( )
{
    if ( CompileMode )
    {
        CSL_CalculateAndSetPreviousJmpOffset_ToHere ( ) ;
    }
    //else { ; //nop  }
}



