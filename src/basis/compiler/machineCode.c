#include "../../include/csl.h"

// Intel notes - cf. InstructionSet-A-M-253666.pdf - section 2.1; InstructionSet-N-Z-253667.pdf section B.1/B.2 :
// Intel instructions (insn) can only operate on one memory operand per instruction, i.e. one cannot move mem to mem in one insn, you must move mem to reg and reg to mem
// the direction is either mem to reg or reg to mem determined in some instructions by a direction bit in the opCode
// b prefix = binary code
// -----------------------------------------------------------------------
// instuction format ( in bytes )
// prefixes  opcode  modRm     sib     disp   immediate
//  0 - 4    1 - 3   0 - 1    0 - 1    0,1,4    0,1,2,4      -- number of bytes
// optional          ------------optional--------------
//   REX Prefix 
//   0x40 = 0100 + WRSB  W = 64 bit operand flag, R = reg ext. flag bit, S = sib ext, B = r/m reg ext flag bit
// -----------------------------------------------------------------------
//   modRm byte ( bits ) :: mod 0 : no disp ;; mod 1 : 1 byte disp : mod 2 : 4 byte disp ;; mod 3 : reg value :: sections 2.1.3/2.1.5, Table 2-2
//   the mod field is a semantic function on the r/m field determining its meaning either as the reg value itself or the value at the [reg] as an addr + offset
//   Intel InstructionSet-N-Z-253667.pdf section 2.1.5
//    mod     reg     r/m  
//   7 - 6   5 - 3   2 - 0 
//    0-3              4 - b100 => sib, instead of reg ESP   : mod bit determines size of displacement 
// bit fields :
//  mod reg r/m 
//   00 000 000
// *if insn has a mod/rm byte then ::
// #define RM( insnAddr )  (*( (byte*) insnAddr + 1) & 7 )   // binary : 00000111
// #define REG( insnAddr ) (*( (byte*) insnAddr + 1) & 56 )  // binary : 00111000 
// #define MOD( insnAddr ) (*( (byte*) insnAddr + 1) & 192 ) // binary : 11000000 
// -----------------------------------------------------------------------
//  reg/rm codes :
//  EAX 0, ECX 1, EDX 2, EBX 3, ESP 4, EBP 5, ESI 6, EDI 7
// -----------------------------------------------------------------------
//  bit positions encoding :  ...|7|6|5|4|3|2|1|0|  but nb. intel is little endian
// -----------------------------------------------------------------------
//  opCode direction bit 'd' is bit position 1 : 1 => rm/sib to reg ; 0 => reg to rm/sib -- for some instructions
//  sign extend bit 's' is bit position 1 for some instructions
//  operand size bit 'w' is bit position 0 for some instructions
// -----------------------------------------------------------------------
//       sib byte ( bits ) with rm 4 - b100 - ESP
//    scale  index   base
//    7 - 6  5 - 3  2 - 0
//    scale 0 : [index * 1 + base]
//    scale 1 : [index * 2 + base]
//    scale 2 : [index * 4 + base]
//    scale 1 : [index * 4 + base]
// -----------------------------------------------------------------------
// intel syntax : opcode dst, src
// att syntax   : opcode src, dst

// note : x86-32 instruction format : || prefixes : 0-4 bytes | opCode : 1-3 bytes | mod : 0 - 1 byte | sib : 0 - 1 byte | disp : 0-4 bytes | immediate : 0-4 bytes ||
// note : intex syntax  : instruction dst, src - csl uses this order convention
//        att   syntax  : instruction src, dst
// note : rm : reg memory - the register which contains the memory address in mod instructions

// csl uses intel syntax convention

// ----------------------------------
// | intel addressing ideas summary |
// ----------------------------------
// remember : the intel cpus can not reference two memory operands in one instruction so
// the modr/m byte selects with the mod and rm field an operand to use with the reg field value (generally)
// the mod field ( 2 bits ) contols whether the r/m field reg refers to a direct reg or indirect + disp values (disp values are in the displacement field)
// mod 0 is for register indirect -- no displacement the register is interpreted as an address; it refers to a value in a memory address with no disp
// mod 1 is for register indirect -- 8 bit disp the register is interpreted as an address; it refers to a value in a memory address with 8 bit disp
// mod 2 is for register indirect -- 32 bit disp the register is interpreted as an address; it refers to a value in a memory address with 32 bit disp
// mod 3 is for register direct   -- using the direct register value -- not as an address 
// the reg field of the modr/m byte generally refers to to register to use with the mod modified r/m field -- intel can't address two memory fields in any instruction
// --------------------------------------

// controlFlags : bits ::  4      3       2     1      0         
//                       rex    imm    disp   sib    modRm      
//                      rex=16 imm=8, disp=4 sib=2  mod/Rm=1
//                      REX_B  IMM_B  DISP_B SIB_B   MODRM_B

void
_Compile_Write_Instruction_X64 ( Boolean rex, uint8 opCode0, uint8 opCode1, Boolean modRm, int16 controlFlags, Boolean sib, int64 disp, Boolean dispSize, int64 imm, Boolean immSize )
{
    d1 ( byte * here = Here ) ;
    if ( rex ) _Compile_Int8 ( rex ) ;
    if ( opCode0 ) _Compile_Int8 ( ( byte ) opCode0 ) ;
    if ( opCode1 ) _Compile_Int8 ( ( byte ) opCode1 ) ;
    if ( ( controlFlags & MODRM_B ) ) _Compile_Int8 ( modRm ) ;
    if ( sib && ( controlFlags & SIB_B ) ) _Compile_Int8 ( sib ) ;
    if ( disp || ( controlFlags & DISP_B ) ) _Compile_ImmDispData ( disp, dispSize, 0 ) ;
    if ( imm || ( controlFlags & IMM_B ) ) _Compile_ImmDispData ( imm, immSize, ( controlFlags & IMM_B ) ) ;
    if ( _DBI || ( _O_->Dbi > 1 ) )
    {
        d1 ( Debugger_UdisOneInstruction ( _Debugger_, 0, here, ( byte* ) "", ( byte* ) "" ) ; ) ;
        d0 ( _Debugger_Disassemble ( _Debugger_, 0, ( byte* ) here, Here - here, 1 ) ) ;
    }
}

Boolean
RegParameterOrder ( Boolean n )
{
#if 0  // trying to get factorial to use RAX as its reg var
    Compiler * compiler = _Compiler_ ;
    if ( ( ( compiler->NumberOfNonRegisterArgs + compiler->NumberOfNonRegisterLocals ) == 0 ) && ( ( compiler->NumberOfArgs + compiler->NumberOfLocals ) == 1 ) ) //nonRegVars )
    {
        Boolean regOrder [] = { RAX, RSI, RDX, RCX, R8D, R9D, R10D, R11D } ;
        return regOrder [n] ;
    }
    else
    {
        Boolean regOrder [] = { RDI, RSI, RDX, RCX, R8D, R9D, R10D, R11D } ;
        return regOrder [n] ;
    }
#else
    Boolean regOrder [] = { RDI, RSI, RDX, RCX, R8D, R9D, R10D, R11D } ;
    return regOrder [n] ;
#endif    
}

// some checks of the internal consistency of the instruction bits

int64
CalculateSib ( int64 scale, int64 indexReg, int64 baseReg )
{
    //  scale index base bits  : scale 1 = *2, 2 = *4, 3 = *8 ; index and base refer to registers
    //  00    000   000
    //if (( indexReg > 0x7 ) || ( baseReg > 0x7 ) ) Exception ( MACHINE_CODE_ERROR, ABORT ) ;
    Boolean sib = ( scale << 6 ) | ( ( indexReg & 0x7 ) << 3 ) | ( baseReg & 0x7 ) ;
    return sib ;
}

byte
_CalculateRex ( Boolean reg, Boolean rm, Boolean sib, int64 operandSize )
{
    //  0100    WRXB
    Boolean rex = 0 ;
    if ( operandSize > INT32_SIZE )
    {
        //rex += 8 ; // 1 << 3 ;
        byte rex = 0x48 ;
        if ( reg > 0x7 ) rex += 4 ; // (1 << 2) ;
        if ( sib > 0x7 ) rex += 2 ; // 1 << 1 ; // needs to be fixed!! respecting sib first
        if ( rm > 0x7 ) rex += 1 ;
    }
    return rex ;
}
// instruction letter codes : I - immediate data ; 32 : 32 bit , 8 : 8 bit ; EAX, DSP : registers
// we could have a mod of 0 so the modRmImmDispFlag is necessary
// operandSize : specific size of immediate data - BYTE or WORD
// SIB : scale, index, base addressing byte

void
_Compile_ImmDispData ( int64 immDisp, Boolean size, Boolean forceFlag )
{
    // the opcode probably is all that needs to be adjusted for this to not be necessary    
    // to not compile an imm when imm is a parameter, set isize == 0 and imm == 0
    if ( size > 0 )
    {
        if ( size == BYTE )
            _Compile_Int8 ( ( byte ) immDisp ) ;
        else if ( size == 2 )
            _Compile_Int16 ( immDisp ) ;
        else if ( size == 4 )
            _Compile_Int32 ( immDisp ) ;
        else if ( size == CELL )
            _Compile_Cell ( immDisp ) ;
    }
    else // with operandSize == 0 let the compiler use the minimal size ; nb. can't be imm == 0
    {
        if ( abs ( immDisp ) >= 0x7fffffff )
            _Compile_Int64 ( immDisp ) ;
        else if ( abs ( immDisp ) >= 0x7f )
            _Compile_Int32 ( immDisp ) ;
        else if ( immDisp || forceFlag )
            _Compile_Int8 ( ( byte ) immDisp ) ;
    }
}

// Intel - InstructionSet-A-M-253666.pdf - section 2.1 :
//-----------------------------------------------------------------------
// instuction format ( number of bytes )
// prefixes  opcode  modRm   sib       disp    immediate
//  0 - 4    1 - 3   0 - 1  0 - 1    0,1,2,4    0,1,2,4      -- number of bytes
//-----------------------------------------------------------------------
//   modRm byte ( bits )  mod 0 : no disp ; mod 1 : 1 byte disp : mod 2 : 4 byte disp ; mod 3 : just reg value
//    mod     reg      rm
//   7 - 6   5 - 3   2 - 0 
//-----------------------------------------------------------------------
//  reg/rm values :
//  EAX 0, ECX 1, EDX 2, ECX 3, ESP 4, EBP 5, ESI 6, EDI 7
//-----------------------------------------------------------------------
//       sib byte ( bits )
//    scale  index   base
//    7 - 6  5 - 3  2 - 0
//-----------------------------------------------------------------------

uint8
Calculate_Rex ( Boolean reg, Boolean rm, Boolean rex_w_flag, Boolean rex_b_flag )
{
    Boolean rex = ( ( rex_w_flag ? 8 : 0 ) | ( ( reg > 7 ) ? 4 : 0 ) | ( ( rm > 7 ) ? 1 : 0 ) ) ;
    if ( rex || rex_b_flag || rex_w_flag ) rex |= 0x40 ;
    return rex ;
}

Boolean
CalculateModRegardingDisplacement ( Boolean mod, int64 disp )
{
    // bits :
    //  mod reg r/m 
    //   00 000 000
    if ( mod != REG )
    {
        if ( disp == 0 )
            mod = 0 ;
        else if ( disp <= 0x7f ) mod = 1 ; // displacement can be either plus or minus 0x7f ( 127 )
        else mod = 2 ;
    }
    return mod ;
}

//-----------------------------------------------------------------------
//   modRm byte ( bits )  mod 0 : no disp ; mod 1 : 1 byte disp : mod 2 : 4 byte disp ; mod 3 : just reg value
//    mod     reg      rm
//   7 - 6   5 - 3   2 - 0
//-----------------------------------------------------------------------
//  reg/rm values :
//  EAX 0, ECX 1, EDX 2, ECX 3, ESP 4, EBP 5, ESI 6, EDI 7
//-----------------------------------------------------------------------

uint8
CalculateModRmByte ( Boolean mod, Boolean reg, Boolean rm, Boolean sib, int64 disp )
{
    uint8 modRm ;
    mod = CalculateModRegardingDisplacement ( mod, disp ) ;
    if ( ( mod < 3 ) && ( rm == 4 ) ) //|| ( ( rm == 5 ) && ( disp == 0 ) ) ) )
        //if ( ( mod < 3 ) && ( ( ( rm == 4 ) && ( sib == 0 ) ) || ( ( rm == 5 ) && ( disp == 0 ) ) ) )
    {
        // cf. InstructionSet-A-M-253666.pdf Table 2-2
        CSL_Exception ( MACHINE_CODE_ERROR, 0, 1 ) ;
    }
    if ( sib )
    {
        rm = 4 ; // from intel mod tables
        reg = 0 ;
    }
    modRm = ( mod << 6 ) + ( ( reg & 0x7 ) << 3 ) + ( rm & 0x7 ) ; // only use 3 bits of reg/rm
    //modRm = ( mod << 6 ) + ( ( reg ) << 3 ) + ( rm ) ; // only use 3 bits of reg/rm
    return modRm ;
}

void
Compile_CalculateWrite_Instruction_X64 ( uint8 opCode0, uint8 opCode1, Boolean mod, uint8 reg, Boolean rm, uint16 controlFlags, Boolean sib, uint64 disp, uint8 dispSize, uint64 imm, uint8 immSize )
{
    Boolean rex = Calculate_Rex ( reg, rm, ( immSize == 8 ) || ( controlFlags & REX_W ), ( controlFlags & REX_B ) ) ;
    //Boolean rex = Calculate_Rex (reg, rm, ( immSize == 8 ), ( controlFlags & REX_B ) ) ;
    uint8 modRm = CalculateModRmByte ( mod, reg, rm, sib, disp ) ;
    _Compile_Write_Instruction_X64 ( rex, opCode0, opCode1, modRm, controlFlags, sib, disp, dispSize, imm, immSize ) ;
}

// intel syntax : opcode dst, src
// mov reg to mem or mem to reg
// note the order of the operands match intel syntax with dst always before src

// the basic move instruction
// mov reg to mem or mem to reg ( but not reg to reg or move immediate )
// note this function uses the bit order of operands in the mod byte : (mod) reg r/m
// not the intel syntax as with _CompileMoveRegToMem _CompileMoveMemToReg
// the 'rmReg' parameter must always refer to a memory location; 'reg' refers to the register, either to or from which we move mem

// controlFlags : bits ::  3       2     1      0         D D - - I D S M
//                       imm    disp   sib    modRm      mod               (mod) only in move insn
//                     imm=8, disp=4 sib=2  mod/Rm=1

// size refers to size of the operand or immediate
// mod :: TO_REG == 3 : TO_MEM == 0,1,2 depending on disp size ; 0 == no disp ; 1 == 8 bit disp ; 2 == 32 bit disp

void
Compile_Move ( uint8 direction, uint8 mod, uint8 reg, uint8 rm, uint8 operandSize, uint8 sib, int64 disp, uint8 dispSize, int64 imm, uint8 immSize )
{
    uint8 opCode0 = 0, opCode, modRm ;
    uint16 controlFlags = ( disp ? DISP_B : 0 ) | ( sib ? SIB_B : 0 ) ;
    if ( ! operandSize ) operandSize = 8 ;
    if ( imm || immSize )
    {
        reg = 0 ; // the rm is the destination and this is move immediate
        controlFlags |= IMM_B ;
        if ( immSize && ( immSize < 8 ) )
        {
            if ( immSize == 1 ) opCode = 0xb0 + rm ;
            else if ( immSize == 2 )
            {
                opCode0 = 0x66 ;
                opCode = 0xb8 | rm ;
                modRm = 0 ;
            }
            else if ( immSize == 4 )
            {
                opCode = 0xb8 | rm ;
                modRm = 0 ;
            }
        }
        else //if ( immSize >= 8 ) 
        {
            //DBI_ON ;
            //if ( abs (imm) <= 2147483647 ) // sign extend 32 bit to 64 bit -> smaller faster insn
            if ( imm <= 2147483647 ) // sign extend 32 bit to 64 bit -> smaller faster insn
            {
                opCode = 0xc7 ;
                reg = 0 ;
                immSize = 4 ;
                controlFlags |= ( MODRM_B ) ;
            }
            else
            {
                opCode = 0xb8 ;
                opCode += ( rm & 7 ) ;
            }
            controlFlags |= ( REX_W ) ;
        }
        //DBI_ON ; 
        // zero out destination
        //Compile_CalculateWrite_Instruction_X64 ( 0, 0xb8 + (rm & 7), mod, reg, rm, controlFlags, sib, disp, dispSize, 0, CELL ) ;
    }
    else // non immediate
    {
        controlFlags |= ( MODRM_B ) ;
        opCode = 0x88 ;
        if ( operandSize > 1 ) opCode += 1 ; //nb. if using rex then we cannot use AH, BH, CH, DH per Intel instruction manual
        if ( direction == TO_REG ) opCode |= 2 ; // 0x8b ; // 0x89 |= 2 ; // d : direction bit = 'bit 1' : 0 == dest is mem ; 1 == dest is reg
        if ( operandSize == 2 ) opCode0 = 0x66 ; // choose 16 bit operand
        if ( operandSize >= 8 ) controlFlags |= ( REX_W ) ;
        if ( ! mod )
        {
            if ( disp == 0 ) mod = 0 ;
            else if ( abs ( disp ) <= 0x7f ) mod = 1 ;
            else if ( abs ( disp ) >= 0x100 ) mod = 2 ;
        }
    }
    Compile_CalculateWrite_Instruction_X64 ( opCode0, opCode, mod, reg, rm, controlFlags, sib, disp, dispSize, imm, immSize ) ;
    //DBI_OFF ;
}

void
Compile_Move_WithSib ( uint8 direction, Boolean mod, Boolean reg, Boolean rm, Boolean scale, Boolean index, Boolean base )
{
    Compile_Move ( direction, mod, reg, rm, 8, CalculateSib ( scale, index, base ), 0, 0, 0, 0 ) ;
}

void
Compile_Move_Reg_To_Rm ( Boolean rm, Boolean reg, int64 disp, byte size )
{
    Compile_Move ( MEM, 0, reg, rm, size, 0, disp, 0, 0, 0 ) ;
}

// intel syntax : opcode dst, src
// mov reg to mem or mem to reg
// note the order of the operands match intel syntax with dst always before src

void
Compile_Move_Rm_To_Reg ( Boolean rm, Boolean reg, int64 disp, byte size )
{
    Compile_Move ( REG, 0, rm, reg, size, 0, disp, 0, 0, 0 ) ;
}
// intel syntax : opcode dst, src
// mov reg to mem or mem to reg
// note the order of the operands match intel syntax with dst always before src

void
Compile_Move_Reg_To_Reg ( Boolean dstReg, int64 srcReg, byte size )
{
    if ( dstReg != srcReg ) Compile_Move ( REG, REG, dstReg, srcReg, size, 0, 0, 0, 0, 0 ) ; //size ) ; // nb! mod == REG in move reg to reg
}

void
Compile_MoveImm ( Boolean mod, Boolean rm, int64 disp, int64 imm, Boolean immSize )
{
    if ( ! immSize ) immSize = 8 ;
    if ( ( mod == MEM ) && ( ( immSize >= 8 ) || ( imm > 0xffffffff ) ) )
    {
        // there is no x64 instruction to move imm64 to mem directly
        uint8 thruReg = THRU_REG ;
        Compile_MoveImm_To_Reg ( thruReg, imm, immSize ) ; // thruReg : SREGD : needs to be a parameter
        Compile_Move_Reg_To_Rm ( rm, thruReg, disp, immSize ) ;
    }
    else Compile_Move ( 0, mod, 0, rm, 0, 0, 0, 0, imm, immSize ) ; // nb. reg == 0 in a move immediate
}

void
Compile_MoveImm_To_Reg ( Boolean rm, int64 imm, Boolean immSize )
{
    Compile_MoveImm ( REG, rm, 0, imm, immSize ) ;
}

void
Compile_MoveImm_To_Mem ( Boolean rm, int64 imm, Boolean immSize )
{
    Compile_MoveImm ( MEM, rm, 0, imm, immSize ) ;
}

void
Compile_MoveMemValue_To_Reg ( Boolean reg, byte * address, Boolean iSize )
{
    Compile_MoveImm_To_Reg ( reg, ( int64 ) address, iSize ) ;
    Compile_Move_Rm_To_Reg ( reg, reg, 0, iSize ) ;
}

void
Compile_MoveMemValue_ToReg_ThruReg ( Boolean reg, byte * address, Boolean iSize, Boolean thruReg )
{
    Compile_MoveImm_To_Reg ( thruReg, ( int64 ) address, iSize ) ;
    Compile_Move_Rm_To_Reg ( reg, thruReg, 0, iSize ) ;
}

void
Compile_MoveReg_ToAddress_ThruReg ( Boolean reg, byte * address, Boolean thruReg )
{
    Compile_MoveImm_To_Reg ( thruReg, ( int64 ) address, CELL_SIZE ) ;
    Compile_Move_Reg_To_Rm ( thruReg, reg, 0, 0 ) ;
}

// set the value at address to reg - value in reg

void
_Compile_SetAtAddress_WithReg ( int64 * address, int64 reg, int64 thruReg )
{
    _Compile_Move_Literal_Immediate_To_Reg ( thruReg, ( int64 ) address, 0 ) ;
    Compile_Move_Reg_To_Rm ( thruReg, reg, 0, 0 ) ;
}

void
_Compile_Move_Literal_Immediate_To_Reg ( int64 reg, int64 value, int size )
{
    Compile_MoveImm_To_Reg ( reg, value, size ) ;
}

// opCode group 1 - 0x80-0x83 : ADD OR ADC SBB AND SUB XOR CMP : but not with immediate data
// s and w bits of the x86 opCode : w seems to refer to word and is still used probably for historic and traditional reasons
// note : the opReg - operand register parameter is always used for the rm field of the resulting machine code
// These are all operating on a memory operand
// for use of immediate data with this group use _Compile_Group1_Immediate

void
_Compile_X_Group1 ( Boolean code, Boolean toRegOrMem, Boolean mod, Boolean reg, Boolean rm, Boolean sib, int64 disp, Boolean operandSize )
{
    int64 opCode = code << 3, controlFlags = DISP_B | MODRM_B ;
    if ( operandSize > BYTE ) opCode |= 1 ;
    if ( toRegOrMem == TO_REG ) opCode |= 2 ;
    // we need to be able to set the size so we can know how big the instruction will be in eg. CompileVariable
    // otherwise it could be optimally deduced but let caller control by keeping operandSize parameter
    // some times we need cell_t where bytes would work
    Compiler_WordStack_SCHCPUSCA ( 0, 0 ) ;
    if ( operandSize < 8 )
    {
        int64 opCode0 ;
        if ( operandSize == 1 ) opCode |= rm ;
        else if ( operandSize == 2 )
        {
            opCode0 = 0x66 ;
            opCode |= rm ;
        }
        else if ( operandSize == 4 )  opCode |= rm ;
        Compile_CalculateWrite_Instruction_X64 ( opCode0, opCode, mod, reg, rm, controlFlags, sib, disp, 0, 0, operandSize ) ;
    }
    else //if ( osize == 8 ) 
    {
        Compile_CalculateWrite_Instruction_X64 ( 0, opCode, mod, reg, rm, controlFlags, sib, disp, 0, 0, operandSize ) ;
    }
}

// opCode group 1 - 0x80-0x83 : ADD OR ADC SBB AND_OPCODE SUB XOR CMP : with immediate data
// this is for immediate operands operating on REG direction
// mod := REG | MEM
// rm is operand register
// ?!? shouldn't we just combine this with _Compile_Group1 (above) ?!?

void
_Compile_X_Group1_Reg_To_Reg ( Boolean code, Boolean dstReg, int64 srcReg )
{
    _Compile_X_Group1 ( code, TO_REG, REG, srcReg, dstReg, 0, 0, CELL ) ;
}

// opCode group 1 - 0x80-0x83 : ADD OR ADC SBB AND_OPCODE SUB XOR CMP
// to reg ( toRm == 0 ) is default
// toRm flag is like a mod field but is coded as part of opCode
// toRm flag = 0 => ( dst is reg, src is rm ) is default - reg  - like mod 3
// toRm flag = 1 => ( dst is rm, src is reg )             [reg] - like mod 0 // check this ??


// X variable op compile for group 1 opCodes : +/-/cmp/and/or/xor - ia32 

// subtract second operand from first and store result in first

void
_Compile_X_Group1_Immediate ( Boolean code, Boolean mod, Boolean rm, int64 disp, uint64 imm, Boolean iSize )
{
    // 0x80 is the base opCode for this group of instructions but 0x80 is an alias for 0x82
    // we always sign extend so opCodes 0x80 and 0x82 are not being used
    // 1000 00sw 
    int64 opCode = 0x80 ;
    if ( ( ( iSize > 4 ) || ( imm >= 0x100000000 ) ) ) //&& disp )
    {
        //_DBI_ON ;
        // x64 has no insn to do a group1_op with imm int64 to mem directly so ...
        // first move to a reg then from that reg group1Op to mem location
        if ( rm != DSP ) //! disp )
        {
            Compile_MoveImm_To_Reg ( THRU_REG, imm, iSize ) ;
            //_Compile_X_Group1_Reg_To_Reg ( code, ACC, OREG ) ;
            _Compile_X_Group1_Reg_To_Reg ( code, rm, THRU_REG ) ;
        }
        else
        {
            Compile_MoveImm_To_Reg ( ACC, imm, iSize ) ;
            _Compile_X_Group1 ( code, REG, MEM, ACC, rm, 0, disp, CELL ) ;
        }
        //DBI_OFF ;
        return ;
    }
    else if ( ( iSize > BYTE ) || ( abs ( ( long ) imm ) >= 0x7f ) ) //( imm >= 0x100 ) )
    {
        opCode |= 1 ;
    }
    else if ( ( iSize <= BYTE ) || ( abs ( ( long ) imm ) < 0x100 ) ) //( imm < 0x100 ) ) 
        opCode |= 3 ;
    // we need to be able to set the size so we can know how big the instruction will be in eg. CompileVariable
    // otherwise it could be optimally deduced but let caller control by keeping operandSize parameter
    // some times we need cell_t where bytes would work
    //_Compile_InstructionX86 ( int8 opCode, int8 mod, int8 reg, int8 rm, int8 controlFlags, int8 sib, int64 disp, int8 dispSize, int64 imm, int8 immSize )
    //DBI_ON ;
    Compile_CalculateWrite_Instruction_X64 ( 0, opCode, mod, code, rm, REX_W | MODRM_B | IMM_B | DISP_B, 0, disp, 0, imm, iSize ) ;
    //DBI_OFF ;
}

#if 1 // doesn't work

void
_Compile_XOR_AL_1_Immediate ( )
{
    Compile_CalculateWrite_Instruction_X64 ( 0, 0x34, REG, RAX, RAX, IMM_B, 0, 0, 0, 1, BYTE ) ;
}
#endif
// X variable op compile for group 1 opCodes : +/-/and/or/xor - ia32 

void
_Compile_optInfo_X_Group1 ( Compiler * compiler, int64 op )
{
    CompileOptimizeInfo * optInfo = compiler->OptInfo ;
    Compiler_Word_SCHCPUSCA ( optInfo->opWord, 1 ) ;
    if ( ( optInfo->OptimizeFlag & OPTIMIZE_IMM ) && optInfo->Optimize_Imm )
    {
        int64 imm = optInfo->Optimize_Imm ;
        //Compiler_Word_SCHCPUSCA (optInfo->opWord, 1) ;
        _Compile_X_Group1_Immediate ( op, optInfo->Optimize_Mod,
            optInfo->Optimize_Rm, optInfo->Optimize_Disp,
            optInfo->Optimize_Imm, ( imm >= 0x100000000 ) ? CELL : ( ( imm >= 0x100 ) ? 4 : 1 ) ) ;
    }
    else
    {
        //Compiler_Word_SCHCPUSCA (optInfo->opWord, 0) ;
        _Compile_X_Group1 ( op, optInfo->Optimize_Dest_RegOrMem, optInfo->Optimize_Mod,
            optInfo->Optimize_Reg, optInfo->Optimize_Rm, 0,
            optInfo->Optimize_Disp, CELL_SIZE ) ;
    }
}

void
Compile_X_Group1 ( Compiler * compiler, int64 op, int64 ttt, int64 n )
{
    int64 optSetupFlag = Compiler_CheckOptimize ( compiler, 0 ) ;
    CompileOptimizeInfo * optInfo = compiler->OptInfo ; //Compiler_CheckOptimize may change the optInfo
    if ( optSetupFlag == OPTIMIZE_DONE ) return ;
    else if ( optSetupFlag )
    {
        //DBI ;
        _Compile_optInfo_X_Group1 ( compiler, op ) ;
        Compiler_Set_BI_Tttn ( _Context_->Compiler0, ttt, n, 0, 0 ) ;
        if ( optInfo->Optimize_Rm == DSP ) // if the result is to a reg and not tos
        {
            if ( optInfo->Optimize_Dest_RegOrMem == MEM ) return ; //_Compile_Move_Reg_To_StackN ( DSP, 0, optInfo->Optimize_Reg ) ; //return ;
            else if ( ( optInfo->Optimize_Dest_RegOrMem == REG ) && ( ! optInfo->xBetweenArg1AndArg2 ) ) _Compile_Move_Reg_To_StackN ( DSP, 0, optInfo->Optimize_Reg ) ;
        }
            //else if (( optInfo->wordArg1 ) && ( ! ( optInfo->wordArg1->W_ObjectAttributes & REGISTER_VARIABLE ) ) ) _Word_CompileAndRecord_PushReg ( optInfo->COIW[0], optInfo->Optimize_Reg, true ) ; // 0 : ?!? should be the exact variable 
        else _Word_CompileAndRecord_PushReg ( optInfo->COIW[0], optInfo->Optimize_Reg, true ) ; // 0 : ?!? should be the exact variable 
        //DBI_OFF ;
    }
    else // this works on unoptimized code and should be example for other functions !?
    {
        //DBI_ON ;
        _Compile_Stack_PopToReg ( DSP, RAX ) ;
        _Compile_X_Group1 ( op, MEM, MEM, RAX, DSP, 0, 0, CELL_SIZE ) ; // result is on TOS
        //DBI_OFF ;
    }
}

// Group2 : 0pcodes C0-C3/D0-D3
// ROL RLR RCL RCR SHL SHR SAL SAR
// mod := REG | MEM

void
_Compile_Group2 ( Boolean mod, Boolean regOpCode, Boolean rm, int64 controlFlags, Boolean sib, int64 disp, int64 imm )
{
    //cell opCode = 0xc1 ; // rm32 imm8
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modRmImmDispFlag, sib, disp, imm, immSize )
    Compile_CalculateWrite_Instruction_X64 ( 0, 0xc1, mod, regOpCode, rm, ( controlFlags | REX_W | MODRM_B | ( disp ? DISP_B : 0 ) ), sib, disp, 0, imm, ( imm ? BYTE : 0 ) ) ;
}

void
_Compile_Group2_CL ( Boolean mod, Boolean regOpCode, Boolean rm, Boolean sib, int64 disp )
{
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modRmImmDispFlag, sib, disp, imm, immSize )
    Compile_CalculateWrite_Instruction_X64 ( 0, 0xd3, mod, regOpCode, rm, REX_W | MODRM_B | ( disp ? DISP_B : 0 ), sib, disp, 0, 0, 0 ) ;
}

// some Group 3 code is UNTESTED
// opCodes TEST NOT NEG MUL DIV IMUL IDIV // group 3 - F6-F7
// s and w bits of the x86 opCode : w seems to refer to word and is still used probably for historic reasons
// note : the opReg - operand register parameter is always used for the rm field of the resulting machine code
// operating with either a direct register, or indirect memory operand on a immediate operand
// mod := RegOrMem - direction is REG or MEM
// 'size' is operand size

void
_Compile_Group3 ( Boolean code, Boolean mod, Boolean rm, Boolean controlFlags, Boolean sib, int64 disp, int64 imm, Boolean size )
{
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modRmImmDispFlag, sib, disp, imm, immSize )
    Compile_CalculateWrite_Instruction_X64 ( 0, 0xf7, mod, code, rm, REX_W | MODRM_B | controlFlags, sib, disp, 0, imm, size ) ;
}

// inc/dec only ( not call or push which are also group 5 - cf : sandpile.org )
// inc/dec/push/pop

void
_Compile_Group5 ( Boolean code, Boolean mod, Boolean rm, Boolean sib, int64 disp, Boolean size )
{
    //Compile_CalcWrite_Instruction_X64 (  opCode, mod, code, rm, REX_B | MODRM_B | IMM_B | DISP_B, 0, disp, 0, imm, iSize ) ;
    Compile_CalculateWrite_Instruction_X64 ( 0, 0xff, mod, code, rm, ( REX_W | MODRM_B | DISP_B ), sib, disp, size, 0, 0 ) ;
}

void
Compile_X_Group5 ( Compiler * compiler, int64 op )
{
    int64 optSetupFlag = Compiler_CheckOptimize ( compiler, 0 ) ;
    CompileOptimizeInfo * optInfo = compiler->OptInfo ; //Compiler_CheckOptimize may change the optInfo
    Word *one = CSL_WordList ( 1 ) ; // assumes two values ( n m ) on the DSP stack 
    if ( optSetupFlag & OPTIMIZE_DONE ) return ;
    else if ( optSetupFlag )
    {
        if ( optInfo->OptimizeFlag & OPTIMIZE_IMM )
        {
            Compile_MoveImm_To_Reg ( ACC, optInfo->Optimize_Imm, CELL ) ;
            optInfo->Optimize_Mod = REG ;
            optInfo->Optimize_Rm = ACC ;
        }
        //Compiler_Word_SCHCPUSCA ( optInfo->opWord, 0 ) ;
        _Compile_Group5 ( op, optInfo->Optimize_Mod, optInfo->Optimize_Rm, 0, optInfo->Optimize_Disp, 0 ) ;
    }
    else if ( one && one->W_ObjectAttributes & ( PARAMETER_VARIABLE | LOCAL_VARIABLE | NAMESPACE_VARIABLE ) ) // *( ( cell* ) ( TOS ) ) += 1 ;
    {
        SetHere ( one->Coding, 1 ) ;
        if ( one->W_ObjectAttributes & REGISTER_VARIABLE ) _Compile_Group5 ( op, REG, one->RegToUse, 0, 0, 0 ) ;
        else
        {
            //DBI_ON ;
#if 0            
            Compile_GetVarLitObj_RValue_To_Reg ( one, ACC, 0 ) ;
            //_Compile_Group5 ( int8 code, int8 mod, int8 rm, int8 sib, int64 disp, int8 size )
            Compiler_WordStack_SCHCPUSCA ( 0, 0 ) ;
            _Compile_Group5 ( op, REG, ACC, 0, 0, 0 ) ;
            // ++ == += :: -- == -= so :
            _Compile_SetVarLitObj_With_Reg ( one, ACC, OREG ) ;
#else
            //_Compile_Group5 ( Boolean code, Boolean mod, Boolean rm, Boolean sib, int64 disp, Boolean size )
            Compiler_WordStack_SCHCPUSCA ( 0, 0 ) ;
            _Compile_Group5 ( op, MEM, FP, 0, LocalOrParameterVar_Disp ( one ), 0 ) ;
#endif            
            //DBI_OFF ;
            return ;
        }
    }
    else
    {
        // assume rvalue on stack
        _Compile_Group5 ( op, MEM, DSP, 0, 0, 0 ) ;
    }
    Compiler_Set_BI_Tttn ( _Context_->Compiler0, TTT_ZERO, NEGFLAG_NZ, TTT_ZERO, NEGFLAG_Z ) ;
    //if ( ( op != INC ) && ( op != DEC ) ) 
    _Word_CompileAndRecord_PushReg ( CSL_WordList ( 0 ), optInfo->Optimize_Reg, true ) ; // 0 : ?!? should be the exact variable 
}

// load reg with effective address of [ mod rm sib disp ]

void
_Compile_LEA ( Boolean reg, Boolean rm, Boolean sib, int64 disp )
{
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modRmImmDispFlag, sib, disp, imm, immSize )
    Compile_CalculateWrite_Instruction_X64 ( 0, 0x8d, TO_MEM, reg, rm, REX_W | MODRM_B | DISP_B, sib, disp, 0, 0, 0 ) ;
}

void
_Compile_TEST_Reg_To_Reg ( Boolean dstReg, int64 srcReg, Boolean size )
{
    Boolean opCode1 ;
    if ( size == BYTE ) opCode1 = 0x84 ;
    else opCode1 = 0x85 ;
    Compile_CalculateWrite_Instruction_X64 ( 0, opCode1, TO_REG, srcReg, dstReg, REX_W | MODRM_B, 0, 0, 0, 0, 0 ) ;
}

void
Compile_TEST_Reg_To_Reg ( Boolean dstReg, int64 srcReg )
{
    //Compile_CalcWrite_Instruction_X64 ( 0, 0x85, TO_REG, srcReg, dstReg, REX_B | MODRM_B, 0, 0, 0, 0, 0 ) ;
    _Compile_TEST_Reg_To_Reg ( dstReg, srcReg, CELL ) ;
}

// mul reg with imm to rm

void
_Compile_IMUL ( Boolean mod, Boolean reg, Boolean rm, Boolean sib, int64 disp, uint64 imm )
{
    int64 opCode = 0x69 ;
    if ( imm && ( imm < 256 ) )
    {
        opCode |= 2 ;
    }
    //Compile_CalcWrite_Instruction_X64 ( uint8 opCode0, uint8 opCode1, int8 mod, int8 reg, int8 rm, int16 controlFlags, int8 sib, int64 disp, int8 dispSize, uint64 imm, int8 immSize )
    Compile_CalculateWrite_Instruction_X64 ( 0x0f, 0xaf, mod, reg, rm, REX_W | MODRM_B | ( imm ? IMM_B : 0 ), sib, disp, 0, imm, 0 ) ; //size ) ;
}

void
Compile_IMULI ( Boolean mod, Boolean reg, Boolean rm, Boolean sib, int64 disp, uint64 imm )
{
    int64 opCode = 0x69, immSize ;
    if ( imm < 128 )
    {
        opCode |= 2 ;
        immSize = 1 ;
    }
    else immSize = 4 ;
    Compile_CalculateWrite_Instruction_X64 ( 0, opCode, mod, reg, rm, REX_W | MODRM_B | IMM_B, sib, disp, 0, imm, immSize ) ; //size ) ;
}

void
Compile_IMUL ( Boolean mod, Boolean reg, Boolean rm, Boolean controlFlags, Boolean sib, int64 disp )
{
    Boolean rex = Calculate_Rex ( reg, rm, 0, 0 ) ;
    Boolean modRm = CalculateModRmByte ( mod, reg, rm, sib, disp ) ;
    //_Compile_Write_Instruction_X64 ( int8 rex, uint8 opCode0, uint8 opCode1, int8 modRm, int16 controlFlags, int8 sib, int64 disp, int8 dispSize, int64 imm, int8 immSize )
    _Compile_Write_Instruction_X64 ( rex, 0x0f, 0xaf, modRm, controlFlags, sib, disp, 0, 0, 0 ) ;
}

void
_Compile_Test ( Boolean mod, Boolean reg, Boolean rm, Boolean controlFlags, int64 disp, int64 imm )
{
    Compile_CalculateWrite_Instruction_X64 ( 0, 0xf7, mod, reg, rm, REX_W | MODRM_B | controlFlags, 0, disp, 0, imm, 0 ) ; //??
}

byte *
Calculate_Address_FromOffset_ForCallOrJump ( byte * address )
{
    byte * iaddress = 0 ;
    int64 offset ;
    if ( ( * address == JMPI8 ) || ( * address == JCC8 ) )
    {
        offset = * ( ( int8 * ) ( address + 1 ) ) ;
        iaddress = address + offset + 1 + BYTE_SIZE ;
    }
    else if ( ( * address == JMPI32 ) || ( * address == CALLI32 ) )
    {
        offset = * ( ( int32 * ) ( address + 1 ) ) ;
        iaddress = address + offset + 1 + INT32_SIZE ;
    }
    else if ( ( ( * address == 0x0f ) && ( ( * ( address + 1 ) >> 4 ) == 0x8 ) ) )
    {
        offset = * ( ( int32 * ) ( address + 2 ) ) ;
        iaddress = address + offset + 2 + INT32_SIZE ;
    }
    else if ( ( ( * ( uint16* ) address ) == 0xff49 ) && ( ( *( address + 2 ) == 0xd2 ) || ( *( address + 2 ) == 0xd3 ) ) ) // call r10/r11
    {
        if ( ( ( * ( uint16* ) ( address - 20 ) ) == 0xbb49 ) ) iaddress = ( byte* ) ( * ( ( uint64* ) ( address - 18 ) ) ) ; //mov r11, 0xxx in Compile_Call_TestRSP  
        else iaddress = ( byte* ) ( * ( ( uint64* ) ( address - CELL ) ) ) ;
    }
    else if ( ( ( * ( uint16* ) address ) == 0xff48 ) && ( *( address + 2 ) == 0xd3 ) ) // call rax
    {
        iaddress = ( byte* ) ( * ( ( uint64* ) ( address - CELL ) ) ) ;
    }
    return iaddress ;
}


// compileAtAddress is the address of the offset to be compiled
// for compileAtAddress of the disp : where the space has *already* been allocated
// call 32BitOffset ; <- intel call instruction format
// endOfCallingInstructionAddress + disp = jmpToAddr
// endOfCallingInstructionAddress = compileAtAddress + 4 ; for ! 32 bit disp only !
// 		-> disp = jmpToAddr - compileAtAddress - 4

int32
_CalculateOffsetForCallOrJump ( byte * offsetAddress, byte * jmpToAddr, byte insn )
{
    int32 offset = jmpToAddr - ( offsetAddress + 1 ) ;
    byte * insnAddr = offsetAddress - 1, offsetSize ;
    insn = insn ? insn : * insnAddr ; // when compiling a jmp/jcc insn *insnAddr == 0 ; nb! : insn is necessary in this case
    offsetSize = ( ( insn == JMPI32 ) || ( insn == JCC32 ) || ( insn == CALLI32 ) || ( abs ( offset ) > 127 ) ) ? 4 : 1 ;

    //if ( ( offsetSize == 4 ) || ( abs ( jmpToAddr - ( offsetAddress + offsetSize ) ) > 255 ) ) offset = ( jmpToAddr - ( offsetAddress + 4 ) ) ; // operandSize sizeof offset //call/jmp insn x64/x86 mode //sizeof (cell) ) ; // we have to go back the instruction size to get to the start of the insn 
    if ( offsetSize == 4 )
    {
        if ( insn == JCC32 ) offset = ( jmpToAddr - ( offsetAddress + 1 + 4 ) ) ; // 1 : JCC32 is a 2 byte insn code
        else offset = ( jmpToAddr - ( offsetAddress + 4 ) ) ;
    }
    //else offset = ( jmpToAddr - ( offsetAddress + 1 ) ) ; //BYTE ) ) 
    return offset ;
}

byte *
CalculateAddressFromOffset ( byte * compileAtAddress, int32 offset )
{
    byte * address = compileAtAddress - 2 + offset ;
    return address ;
}

void
_SetOffsetForCallOrJump ( byte * offsetAddress, byte * jmpToAddr, byte insn )
{
    if ( offsetAddress )
    {
        byte * insnAddr = offsetAddress - 1 ;
        insn = insn ? insn : * insnAddr ;
        int32 offset = _CalculateOffsetForCallOrJump ( offsetAddress, jmpToAddr, insn ) ;
        if ( ( insn != JMPI32 ) && ( insn != JCC32 ) && ( abs ( offset ) < 127 ) ) * ( ( int8* ) offsetAddress ) = ( byte ) offset ;
        else if ( insn == JMPI32 ) * ( ( int32* ) offsetAddress ) = offset ; //offset ;
        else if ( insn == JCC32 ) * ( ( int32* ) ( offsetAddress + 1 ) ) = offset ; //offset ;
    }
}

void
_Compile_JumpToDisp ( int64 disp, byte insn )
{
    if ( ( insn == JMPI8 ) || ( abs ( disp ) < 128 ) ) // with jmp instruction : disp is compiled as an immediate offset
        Compile_CalculateWrite_Instruction_X64 ( 0, JMPI8, 0, 0, 0, DISP_B, 0, disp, BYTE, 0, 0 ) ;
    else Compile_CalculateWrite_Instruction_X64 ( 0, JMPI32, 0, 0, 0, DISP_B, 0, disp, INT32_SIZE, 0, 0 ) ;
}

void
_Compile_JumpToAddress ( byte * jmpToAddr, byte insn ) // runtime
{
    byte * offsetAddress = Here + 1 ;
    //if ( (!insn) || (( ( insn == 0x0f ) || ( insn == JMPI32 ) ) && abs ( (int64) (jmpToAddr - ( Here + 6 + ( insn == 0x0f ) ? 1 : 0 ) ) ) ) ) // optimization : don't need to jump to the next instruction
    {
        int64 disp = _CalculateOffsetForCallOrJump ( offsetAddress, jmpToAddr, insn ) ;
        _Compile_JumpToDisp ( disp, insn ) ;
    }
}

void
_Compile_JumpToRegAddress ( Boolean reg )
{
    _Compile_Group5 ( JMP, 0, reg, 0, 0, 0 ) ;
}

byte *
Compile_UninitializedJumpEqualZero ( )
{
    byte * compiledAtAddress = Here ;
    _Compile_Jcc ( NEGFLAG_Z, TTT_ZERO, 0, 0 ) ;
    return compiledAtAddress ;
}

void
_Compile_UninitializedJmpOrCall ( Boolean jmpOrCall )
{
    //if ( ((* (Here - 5 )) != JMPI32 ) && (* (int32*) (Here - 4 )) != 0 ) 
    Compile_CalculateWrite_Instruction_X64 ( 0, jmpOrCall, 0, 0, 0, DISP_B, 0, 0, INT32_SIZE, 0, 0 ) ;
    //else _Printf ( (byte*) "") ;
}

void
_Compile_JumpWithOffset ( int64 disp )
{
    //Compile_CalculateWrite_Instruction_X64 ( 0, JMPI32, 0, 0, 0, DISP_B, 0, disp, INT32_SIZE, 0, 0 ) ;
    _Compile_JumpToDisp ( disp, 0 ) ;
}

byte *
_Compile_UninitializedCall ( )
{
    byte * compiledAtAddress = Here ;
    _Compile_UninitializedJmpOrCall ( CALLI32 ) ;
    return compiledAtAddress ;
}

byte *
Compile_UninitializedJump ( )
{
    byte * compiledAtAddress = Here ;
    _Compile_UninitializedJmpOrCall ( JMPI32 ) ;
    return compiledAtAddress ;
}

void
_Compile_CallReg ( Boolean reg, Boolean regOrMem )
{
    //DBI_ON ;
    _Compile_Group5 ( CALL, regOrMem, reg, 0, 0, 0 ) ;
    //DBI_OFF ;
}

void
Compile_CallThru_AdjustRSP ( Boolean reg, Boolean regOrMem )
{
    Compile_SUBI ( REG, RSP, 0, 8, 0 ) ;
    _Compile_CallReg ( reg, regOrMem ) ;
    Compile_ADDI ( REG, RSP, 0, 8, 0 ) ;
}

void
Compile_Call ( byte * address )
{
    Compile_MoveImm_To_Reg ( CALL_THRU_REG, ( int64 ) address, CELL ) ;
    _Compile_CallReg ( CALL_THRU_REG, REG ) ;
}

void
_Compile_Call_ThruReg_TestAlignRSP ( Boolean thruReg )
{
    //DBI_ON ;
    Compile_Move_Reg_To_Reg ( RAX, RSP, 0 ) ;
    Compile_TEST_AL_ImmByte ( 0x8 ) ;
    _Compile_Jcc ( NEGFLAG_Z, TTT_ZERO, Here + 15, JCC8 ) ;
    Compile_CallThru_AdjustRSP ( thruReg, REG ) ;
    _Compile_JumpToDisp ( 3, JMPI8 ) ; // runtime
    _Compile_CallReg ( thruReg, REG ) ;
    //DBI_OFF ;
}

void
Compile_Call_ToAddressThruReg_TestAlignRSP ( byte * address, Boolean thruReg )
{
    Compile_MoveImm_To_Reg ( thruReg, ( int64 ) address, CELL ) ;
    _Compile_Call_ThruReg_TestAlignRSP ( thruReg ) ;
    //Compile_Call ( (byte*) _CSL_->Call_ToAddressThruSREG_TestAlignRSP ) ;
}

void
Compile_Call_ToAddressThruSREG_TestAlignRSP ( )
{
    _Compile_Call_ThruReg_TestAlignRSP ( SREG ) ;
}

void
Compile_Call_TestRSP ( byte * address )
{
    Compile_MoveImm_To_Reg ( SREG, ( int64 ) address, CELL ) ;
    Compile_Call ( ( byte* ) _CSL_->Call_ToAddressThruSREG_TestAlignRSP ) ;
}

void
Compile_Call_X84_ABI_RSP_ADJUST ( byte * address )
{
    Compile_Call_TestRSP ( address ) ;
}

void
Compile_CallWord_Check_X84_ABI_RSP_ADJUST ( Word * word )
{
    if ( word->W_MorphismAttributes & CPRIMITIVE )
    {
        d0 ( Printf ( "\n_Word_Compile : %s : name = %s", Context_Location ( ), word->Name ) ) ;
        // there is an slight overhead for CPRIMITIVE functions to align RSP for ABI-X64
        Compile_Call_TestRSP ( ( byte* ) word->Definition ) ;
    }
    else Compile_Call ( ( byte* ) word->Definition ) ;
}

void
_Compile_PushReg ( Boolean reg )
{
    // only EAX ECX EDX EBX : 0 - 4
    Compile_CalculateWrite_Instruction_X64 ( 0, 0x50 + reg, REG, 0, 0, REX_W, 0, 0, 0, 0, 0 ) ;
}

// pop from the C esp based stack with the 'pop' instruction

void
_Compile_PopToReg ( Boolean reg )
{
    // only EAX ECX EDX EBX : 0 - 4
    Compile_CalculateWrite_Instruction_X64 ( 0, 0x58 + reg, REG, 0, 0, REX_W, 0, 0, 0, 0, 0 ) ;
}

void
_Compile_PopFD ( )
{
    Compile_CalculateWrite_Instruction_X64 ( 0, 0x9d, 0, 0, 0, REX_W, 0, 0, 0, 0, 0 ) ;
}

void
_Compile_PushFD ( )
{
    Compile_CalculateWrite_Instruction_X64 ( 0, 0x9c, 0, 0, 0, REX_W, 0, 0, 0, 0, 0 ) ;
}

void
Compile_TEST_AL_ImmByte ( byte imm )
{
    Compile_CalculateWrite_Instruction_X64 ( 0, 0xa8, 0, 0, 0, IMM_B, 0, 0, 0, imm, BYTE ) ;
}

void
_Compile_INT80 ( )
{
    Compile_CalculateWrite_Instruction_X64 ( 0xcd, 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0 ) ;
}

void
_Compile_Noop ( )
{
    Compile_CalculateWrite_Instruction_X64 ( 0, 0x90, 0, 0, 0, 0, 0, 0, 0, 0, 0 ) ;
}

// Zero eXtend from byte to cell

void
_Compile_MOVZX_BYTE_REG ( Boolean reg, Boolean rm )
{
    //Compile_CalculateWrite_Instruction_X64 ( 0x0f, 0xb6, REG, reg, rm, ( REX_W | MODRM_B ), 0, 0, 0, 0, 0 ) ;
    Compile_CalculateWrite_Instruction_X64 ( 0x0f, 0xb6, REG, reg, rm, ( REX_W | MODRM_B ), 0, 0, 0, 0, 0 ) ;
}

void
_Compile_Return ( )
{
    //Compiler_WordStack_SCHCPUSCA( 0, 0 )  ;
    Compile_CalculateWrite_Instruction_X64 ( 0, 0xc3, 0, 0, 0, 0, 0, 0, 0, 0, 0 ) ;
}

