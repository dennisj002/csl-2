
// Intel notes - cf. InstructionSet-A-M-253666.pdf - section 2.1; InstructionSet-N-Z-253667.pdf section B.1/B.2 :
// b prefix = binary code
// -----------------------------------------------------------------------
// instuction format ( in bytes )
// prefixes  opcode  modRm     sib     disp   immediate
//  0 - 4    1 - 3   0 - 1    0 - 1    0,1,4    0,1,4      -- number of bytes
// optional          ------------optional--------------
// -----------------------------------------------------------------------
//   modRm byte ( bits )  mod 0 : no disp ;; mod 1 : 1 byte disp : mod 2 : 4 byte disp ;; mod 3 : just reg value : sections 2.1.3/2.1.5, Table 2-2
//    mod     reg      rm
//   7 - 6   5 - 3   2 - 0 
//    0-3              4 - b100 => sib, instead of reg ESP   : mod bit determines size of displacement 
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

// note : intex syntax  : instruction dst, src
//        att   syntax  : instruction src, dst
#define x86_emit_byte( x ) _Compile_Int8 ( x )
#define x86_emit_word( x ) _Compile_Int16 ( x )
#define x86_emit_long( x ) _Compile_Int32 ( x )


#define NEW_JMP_CALL 0
#if NEW_JMP_CALL
#define OFFSET_BACKTRACK ( CELL_SIZE + 2 )
#else
#define OFFSET_BACKTRACK CELL_SIZE
#endif

// x86
#define CALLI32 (0xe8 ) // x86 - call opcode
//#define CALL CALLI32
#define _RET (0xc3) // x86 -
#define NOOP ( 0x90 )
#define JMPI32 (0xe9 ) // x86 - jmp opcode
#define JMPI8 (0xeb ) // x86 - jmp opcode
#define JCC8 (0x7)
#define JCC32 (0x0f) //0x0f8x

#define CALL_JMP_MOD_RM 0xff
#define MOD_RM_R8 0x10
#define MOVI 0xc7

#define JEI8 (0x74 ) // x86 - jcc opcode
#define JNEI8 (0x75 ) // x86 - jcc opcode
#define JEI32 (0x84 ) // x86 - jcc opcode : remember 32 bit offset has 0x0f opcode prefix
#define JEI32_W (0x840f ) // x86 - jcc opcode : remember 32 bit offset has 0x0f opcode prefix - little endian
#define JNEI32 (0x85 ) // x86 - jcc opcode : remember 32 bit offset has 0x0f opcode prefix
#define JGEI32 (0x8d ) // x86 - jcc opcode : remember 32 bit offset has 0x0f opcode prefix
#define JLEI32 (0x8e ) // x86 - jcc opcode : remember 32 bit offset has 0x0f opcode prefix
#define JGI32 (0x8f ) // x86 - jcc opcode : remember 32 bit offset has 0x0f opcode prefix
#define JLI32 (0x8c ) // x86 - jcc opcode : remember 32 bit offset has 0x0f opcode prefix

//#define PUSHAD (0x60 )// intel x86 pushad instruction
#define PUSHFD (0x9c )// intel x86 pushfd instruction
#define _POPAD (0x61 )// intel x86 pushad instruction
#define POPFD (0x9d )// intel x86 pushfd instruction
#define PUSHI32 (0x68 )
#define POPToR8 (0x58 )
#define _LAHF (0x9f )
#define _SAHF (0x9e )

#define INT3 (0xcc) // cell_t 3 interrupt code

// mod reg r/m
// mod
#define MOD_DISP_0 0
#define MOD_DISP_8 1
#define MOD_DISP_32 2
#define MOD_REG 3
#define TO_REG 3
#define TO_MEM MOD_DISP_32
// mod field values
#define REG MOD_REG
#define MEM 0 // 0 - 2 based on displacement, calculated at compile time
#define RM_IS_ADDR MEM // based on displacement, calculated at compile time
#define ADDR MEM // based on displacement, calculated at compile time
#define RM_IS_VALUE REG
#define RM_IS_REG_VALUE REG
#define RM_SIB 0x4
#define VALUE REG
// SIB byte conversions
#define SCALE_1 0
#define SCALE_2 1
#define SCALE_4 2
#define SCALE_8 3
// control flag bits for _Compile_InstructionXxx
#define REX_W       ( 1 << 0 ) 
#define MODRM_B     ( 1 << 1 ) // backwards compatibility
#define SIB_B       ( 1 << 2 ) 
#define DISP_B      ( 1 << 3 ) 
#define IMM_B       ( 1 << 4 ) 
#define REX_B       ( 1 << 5 )
#define BYTE_B      ( 1 << 6 ) // 1 BYTE SIZE 
#define WORD_B      ( 1 << 7 ) // 2 BYTE SIZE 
#define DOUBLE_B    ( 1 << 8 ) // 4 BYTE SIZE 
#define QUAD_B      ( 1 << 9 ) // 8 BYTE SIZE : CELL_SIZE
#define REX 0x40
//#define REX_W 0x8 
#define REX_R 0x4
#define REX_X 0x2
//#define REX_B 0x1 // same symbol with two different contextual meanings but has same internal value
#if X64
#define SCALE_CELL SCALE_8
#else
#define SCALE_CELL SCALE_4
#endif

#define BYTE_SIZE (sizeof (byte))
#define INT32_SIZE (sizeof (int32))
#define INT64_SIZE (sizeof (int64))
// size_w field
#if X64
#define CELL_T INT64_T
#else
#define CELL_T INT32_T
#endif
//#define INT INT32_SIZE
#define DISPLACEMENT_32_SIZE ( sizeof ( int32 ) )
#define DISP_SIZE DISPLACEMENT_32_SIZE


// xx xxx xxx
// intel r/m byte reg field values

#define RAX ( 0x0 )
#define RCX ( 0x1 )
#define RDX ( 0x2 )
#define RBX ( 0x3 )
#define RSP ( 0x4 )
#define RBP ( 0x5 )
#define RSI ( 0x6 )
#define RDI ( 0x7 )
#define R8  ( 0x8 )
#define R9  ( 0x9 )
#define R10 ( 0xa )
#define R11 ( 0xb )
#define R12 ( 0xc )
#define R13 ( 0xd )
#define R14 ( 0xe )
#define R15 ( 0xf )
#if X64
#define R8D R8
#define R9D R9 
#define R10D R10
#define R11D R11
#define R12D R12 // RSP
#define R13D R13 // RBP
#define R14D R14 //Stack Pointer
#define R15D R15 //Frame Pointer
#endif
#define NO_INDEX ( 0x4 ) // for sib byte with no index

//Boolean regOrder [] = { RDI, RSI, RDX, RCX, R8D, R9D, R10D, R11D } ;
// csl uses RAX, RDX, R8D, R9D, R14, R15, RBX
#define ACCUMULATOR_REG  RAX                     // rax
#define ACC                            ACCUMULATOR_REG
#define CPU_ACCUM                RAX
#define ACCUM                        ACCUMULATOR_REG
#define RETURN_REG               ACC
#define RETURN_REG_2           RDX                     // rdx
#define OPERAND_REG            RCX                     // rcx
#define OP_REG                       OPERAND_REG
#define OREG                          OPERAND_REG
#define CPU_OREG                  RCX
#define OPERAND_2_REG        RBX                     // rbx
#define OREG2                        OPERAND_2_REG 
#define CPU_OREG2                RBX
#define THRU_REG                   RAX //R9                      // r9
#define SCRATCH_REG             R11 //RBX //R8 //RDX                      // r8 // eax/edx are both used sometimes by ops ebx/ecx are not ?
#define SREG                            SCRATCH_REG
#define DIV_MUL_REG_2           RDX                     // rdx
#define CALL_THRU_REG          THRU_REG                // R9
#define SCRATCH_REG2           THRU_REG                      // r8 // eax/edx are both used sometimes by ops ebx/ecx are not ?
#define SREG2                          SCRATCH_REG2
#if DSP_IS_GLOBAL_REGISTER 
register uint64 *_Dsp_          asm ( "r14" ) ;
register uint64 *_Fp_           asm ( "r15" ) ;
#endif
#define STACK_POINTER           R14D                    // r14
#define DSP                     STACK_POINTER 
#define CPU_DSP                 R14d
#define FRAME_POINTER           R15D                    // r15
#define CPU_FP                  R15d
#define FP                      FRAME_POINTER

//register uint64 *_RspReg_ asm ( "r10" ) ;
#define CSL_RETURN_STACK_POINTER         RBX         // rbx
#define CSL_RSP                             CSL_RETURN_STACK_POINTER
#define CPU_CSL_RSP                         RBX

// EFLAGS
#define CARRY_FLAG ( 1 << 0 )
#define PARITY_FLAG ( 1 << 2 )
#define AUX_FLAG ( 1 << 4 )
#define ZERO_FLAG ( 1 << 6 )
#define SIGN_FLAG ( 1 << 7 )
#define OVERFLOW_FLAG ( 1 << 11 )


#define BYTE ((byte) 1)
#define LABEL ((byte) 2)
#define INT32 sizeof (int32) 
//#define XSIZE ((byte) 8)

// sign extend field
//#define SIGN_EXT 1
//#define NO_SIGN_EXT 0
// group 1 code field
#define ADD 0
#define OR 1
#define ADC 2
#define SBB 3
#define AND 4
//#define AND_OPCODE 4
#define SUB 5
#define XOR 6
#define CMP 7
#define MUL_OP 8  // internal code - not an intel code
#define DIV_OP 9  // internal code - not an intel code

// group 5 codes : inc/dec call/jmp push
// 0xfe-0xff
#define INC 0
#define DEC 1
#define CALL 2
#define CALLF 3
#define JMP 4
#define JMPF 5
#define PUSH 6
#define POP 7

// group 2
// shift instructions immediate count reg or mem
// 0xc0-0xc1 0xd0-0xd4
// reg field of modRm byte : ( mod reg rm )
#define ROL 0
#define ROR 1
#define RCL 2
#define RCR 3
#define SHL 4
#define SHR 5
#define SAL 6
#define SAR 7

// group 3
// arithmetic instructions
// 0x84-0x85 0xf6-0xf7
#define TEST 0
#define TEST1 1
#define NOT 2
#define NEG 3
#define MUL 4
#define IMUL 5
#define DIV 6
#define IDIV 7
#define IMUL_f ( 0x8 | IMUL ) // distinguish from other overloaded group codes
#define IDIV_f ( 0x8 | IDIV )

// below are arbitrarily chosen
#define MODULO 9
#define MOV 10
#define DIVIDE 1
#define MOD 2

// conditional jump
// jcc32 0x0f ( 0x8 << 8 | ttt << 1 | n ) : 2 bytes : little endian -> { 0x8 << 8 | ttt << 1 | n, 0x0f }
// setcc32 0x0f ( 0x9 << 8 | ttt << 1 | n ) : 2 bytes : little endian -> { 0x9 << 8 | ttt << 1 | n, 0x0f }
// jcc8 ( 0x7 << 4 | ttt << 1 | n ) : 1 byte
// n : on/off bit
#define ZERO 0
#define NEGFLAG_Z ZERO 
#define NOT_ZERO 1
#define NEGFLAG_NZ NOT_ZERO
#define NEGFLAG_TRUE NOT_ZERO
#define NEGFLAG_FALSE ZERO
#define NEGFLAG_ON NEGFLAG_TRUE //NEGFLAG_NZ
#define NEGFLAG_OFF NEGFLAG_FALSE //NEGFLAG_Z
// ttt : condition codes
// remember these are left shifted one
#define TTT_OVERFLOW 0
#define TTT_BELOW 1
#define TTT_EQUAL 2    
#define TTT_ZERO 2
#define TTT_BE 3   // BE below or equal    
#define TTT_A 3 // A above
#define TTT_SIGN 4
#define TTT_PARITY 5
#define TTT_LESS 6
#define TTT_LE 7   

#define _RM( insnAddr )  (*( (byte*) insnAddr + 1) & 7 )   // binary : 00000111
#define _REG( insnAddr ) (*( (byte*) insnAddr + 1) & 56 )  // binary : 00111000 
#define _MOD( insnAddr ) (*( (byte*) insnAddr + 1) & 192 ) // binary : 11000000 

#define WORD 2
#define DOUBLE 4
#define QUAD 8
#define OCTAL 8
