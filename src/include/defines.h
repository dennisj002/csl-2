
#define LINUX 1
#define DEBUG 0
#define CLANG 0
#define SIGJMP_BUF 0
#if CLANG
#define NO_GLOBAL_REGISTERS 1 
#else
#define NO_GLOBAL_REGISTERS 0
#define DSP_IS_GLOBAL_REGISTER 1 
#endif
#define EXPERIMENTAL true
#define _DEFINE_DBG 0
#define LC_CONDC 1
#define LC_DEFINE_DBG ( _DEFINE_DBG || Is_DebugOn )
//#define DEFINE_DBG ( _DEFINE_DBG && Is_DebugOn )

#if DEBUG 
#define D( x ) x
#define D1( x ) x
#define D0( x ) 
#define d1( x ) x
#define d0( x ) 
#else 
#define d1( x ) x
#define d0( x ) 
#define D1( x ) if ( Is_DebugOn ) x 
#define D0( x ) 
#define dbi0( x )  
#define dbi1( x ) if ( DBI ) x 
#endif

//#define dO3( x ) x
#define dO3( x ) 
//#define dO31( x ) x
#define dO31( x ) 

// these did work ok
#define SL5 0
#define RETRO 0
#define MARU 0
#define MARU_2_4 0
#define MARU_NILE 0

#if SL5 || MARU || MARU_2_4 || MARU_NILE
#define LISP_IO 1
#else
#define LISP_IO 0
#endif

// these may need some work again to work with the newer IO - LISP_IO
#define JOY 0
#define TINY 0 // femtolisp tiny
#define CHIBI_SCHEME 0
#define PICOLISP 0
#define RACKET 0
#define BOOTSTRAP_SCHEME 0 // needs to add bcsl-gdb to Makefile default target
#define NEWLISP_LIB 0

#define CONSTANT_FOLDING 1
// under construction -- not implemented (yet?)
#define BOOTFORTH 0
#define RETURN_STACK 0 // forth style rstack

#define eof (byte) EOF
//#define ABI 32
#define ABI 64
#define X64 1
#define X86 0
#if 0 //ABI == 64
#define INT_FRMT "%ld"
#define INT_FRMT_02 "%02ld"
#define INT_FRMT_9 "%9ld"
#define INT_FRMT_3 "%3ld"
#define _UINT_FRMT "%lx" 
#define UINT_FRMT_0x "0x%lx"
#define UINT_FRMT_09 "%09lx"
#define UINT_FRMT_0x09 "0x%09lx"
#define UINT_FRMT_0x020 "0x%020lx"
#define UINT_FRMT UINT_FRMT 
#else
#define INT_FRMT "%ld"
#define INT_FRMT_02 "%02ld" 
#define INT_FRMT_9 "%9ld"
#define INT_FRMT_9_LEFT "%-9ld"
#define INT_FRMT_3 "%3ld"
#define INT_FRMT_FOR_HEX "0d%ld"
#define _UINT_FRMT "0x%lx"
#define HEX_INT_FRMT "%lx"
#define HEX_UINT_FRMT "0x%lx"
#define LISP_HEX_FRMT "#x%lx"
#define LISP_DECIMAL_FRMT "#d%ld"
#define UINT_FRMT_0x "0lx%lx"
#define UINT_FRMT_09 "%09lx"
#define UINT_FRMT_08 "%08lx"
#define UINT_FRMT_0x08 "0x%08lx"
#define UINT_FRMT_0x09 "0x%09lx"
#define UINT_FRMT_0x016 "0x%016lx"
#define UINT_FRMT_0x020 "0x%020lx"
#define UINT_FRMT UINT_FRMT_0x016 
#endif
#define BYTE_FRMT_0x "%x"

#define INT_SIZE ( sizeof (int32))

#if X64
#define CELL_SIZE ( sizeof ( int64 ) )
#else
#define CELL_SIZE ( sizeof ( int32 ) )
#endif

#define SLOT_SIZE CELL_SIZE
#define CELL CELL_SIZE

#define true 1
#define false 0
#define on true 
#define off false
#define DONE true
#define NOT_DONE false
#define ESC 27
#define inline
//#define _GCBDW_ 0
#define READLINE 1
#define STACK_CHECK_ERROR 1
#define STACK_EMPTY_POPS_ZERO 1

#define USE_AL_TO_PASS_LOGIC 0 // al x86 register    
#define USE_ZF_TO_PASS_LOGIC 0 // zero flag ; not there yet
#define USE_TOS_TO_PASS_BIGNUM_LOGIC 1 // but is default for interpret mode

#define INIT_FILE_NAME ("./init.f")
#define LABEL_DELIMITER_TOKEN (";")

// use bit fields instead of this ...
#define STACK_ERROR ( (uint64) 1 << 0 )
#define STACK_UNDERFLOW ( (uint64) 1 << 1 )
#define STACK_OVERFLOW ( (uint64) 1 << 2 )
#define MEMORY_ALLOCATION_ERROR ( (uint64) 1 << 3 )
//#define LABEL_NOT_FOUND_ERROR ( (uint64) 1 << 4 )
#define COMPILE_TIME_ONLY ( (uint64) 1 << 5 )
#define LOCALS_UNAVAILABLE ( (uint64) 1 << 7 )
#define NAMESPACE_ERROR ( (uint64) 1 << 8 )
#define NESTED_COMPILE_ERROR ( (uint64) 1 << 9 )
#define SEALED_NAMESPACE_ERROR ( (uint64) 1 << 10 )
#define COLON_QUOTATION_MIX_WARNING ( (uint64) 1 << 11 )
#define MACHINE_CODE_ERROR ( (uint64) 1 << 12 )
#define PARSE_ERROR ( (uint64) 1 << 13 )
#define DEBUG_ERROR ( (uint64) 1 << 14 )
#define OBJECT_SIZE_ERROR ( (uint64) 1 << 15 )
#define VARIABLE_NOT_FOUND_ERROR ( (uint64) 1 << 16 )
#define BUFFER_OVERFLOW ( (uint64) 1 << 17 )
#define INLINE_MULTIPLE_RETURN_ERROR ( (uint64) 1 << 18 )
#define OBJECT_REFERENCE_ERROR ( (uint64) 1 << 19 )
#define CASE_NOT_LITERAL_ERROR ( (uint64) 1 << 20 )
#define LABEL_NOT_FOUND_ERROR ( (uint64) 1 << 21 )
#define ARRAY_DIMENSION_ERROR ( (uint64) 1 << 22 )
#define FIX_ME_ERROR ( (uint64) 1 << 23 )
#define OUT_OF_CODE_MEMORY ( (uint64) 1 << 24 )
#define USEAGE_ERROR ( (uint64) 1 << 25 )
#define SYNTAX_ERROR ( (uint64) 1 << 26 )

#define KB 1024
#define MB ( KB * KB )
#define K KB
#define M MB

#define BUFFER_SIZE (2 * K)
#define BUFFER_IX_SIZE (BUFFER_SIZE - 1)

#define DICTIONARY_SIZE ( (uint64) 1 * MB )
#define OBJECTS_SIZE ( (uint64) 1 * MB )
#define CODE_SIZE ( (uint64) 1 * MB )

#define TEMP_OBJECTS_SIZE ( 100 * K )
#define COMPILER_TEMP_OBJECTS_SIZE ( 100 * K )
#define SESSION_OBJECTS_SIZE ( 100 * K )
#define LISP_TEMP_SIZE ( 100 * K )

#define HISTORY_SIZE ( 10 * K )
#define BUFFER_SPACE_SIZE ( 100 * K )
#define CONTEXT_SIZE ( 5 * K )

#define OPENVMTIL_SIZE ( 2 * KB )
#define STACK_SIZE ( 2 * KB ) // * 4 bytes per slot
#define CSL_SIZE (STACK_SIZE * 4) + (12.5 * K)

#define TEMP_MEM_SIZE (COMPILER_TEMP_OBJECTS_SIZE + SESSION_OBJECTS_SIZE + LISP_TEMP_SIZE + TEMP_OBJECTS_SIZE + HISTORY_SIZE + BUFFER_SPACE_SIZE + CONTEXT_SIZE)
#define VARIABLE_MEM_SIZE TEMP_MEM_SIZE 
#define CORE_MEM_SIZE (DICTIONARY_SIZE + OBJECTS_SIZE + CODE_SIZE) 
#define STATIC_MEM_SIZE (OPENVMTIL_SIZE + CSL_SIZE)
#define PERMANENT_MEM_SIZE (CORE_MEM_SIZE + STATIC_MEM_SIZE)
#define TOTAL_MEM_SIZE (TEMP_MEM_SIZE + CORE_MEM_SIZE + STATIC_MEM_SIZE)
#define MINIMUM_MEM_SIZE (TEMP_MEM_SIZE + STATIC_MEM_SIZE)

#define DICTIONARY_PERCENT (double) (((double) DICTIONARY_SIZE) / ((double) CORE_MEM_SIZE))
#define OBJECTS_PERCENT (double) (((double) OBJECTS_SIZE) / ((double) CORE_MEM_SIZE))
#define CODE_PERCENT (double) (((double) CODE_SIZE) / ((double) CORE_MEM_SIZE))

#define TEMP_OBJECTS_PERCENT (double) (((double) TEMP_OBJECTS_SIZE) / ((double) TOTAL_MEM_SIZE))
#define COMPILER_TEMP_OBJECTS_PERCENT (double) (((double) COMPILER_TEMP_OBJECTS_SIZE) / ((double) TOTAL_MEM_SIZE))
#define SESSION_OBJECTS_PERCENT (double) (((double) SESSION_OBJECTS_SIZE) / ((double) TOTAL_MEM_SIZE))
#define LISP_TEMP_PERCENT (double) (((double) LISP_TEMP_SIZE) / ((double) TOTAL_MEM_SIZE))

#define HISTORY_PERCENT (double) (((double) HISTORY_SIZE) / ((double) TOTAL_MEM_SIZE))
#define BUFFER_SPACE_PERCENT (double) (((double) BUFFER_SPACE_SIZE) / ((double) TOTAL_MEM_SIZE))
#define CONTEXT_PERCENT (double) (((double) CONTEXT_SIZE) / ((double) TOTAL_MEM_SIZE))


// Flags
#define FALSE 0
#define TRUE 1 
#define RVALUE 1
#define LVALUE 2

// PrintStateInfo defines
#define PSI_NEWLINE ( (uint64) 1 << 15 )
#define PSI_PROMPT  ( (uint64) 1 << 16 )

#if 0 // not used
typedef struct
{
    // defines here
    unsigned int CPRIMITIVE : 1 ;
    unsigned int CSL_WORD : 1 ;
    unsigned int CSL_ASM_WORD : 1 ;
    unsigned int T_LISP_SYMBOL : 1 ;
    unsigned int NOT_A_KNOWN_OBJECT : 1 ;
    unsigned int KNOWN_OBJECT : 1 ;
    unsigned int T_HEAD : 1 ;
    unsigned int T_TAIL : 1 ;
    unsigned int RIGHT_BRACKET : 1 ;
    unsigned int LEFT_BRACKET : 1 ;
    unsigned int IMMEDIATE : 1 ;
    unsigned int KEYWORD : 1 ;
    unsigned int BLOCK : 1 ;
    unsigned int INLINE : 1 ;
    unsigned int LEFT_PAREN : 1 ;
    unsigned int RIGHT_PAREN : 1 ;
    unsigned int OP_RAX_PLUS_1ARG : 1 ;
    unsigned int PREFIX : 1 ;
    unsigned int INFIXABLE : 1 ;
    unsigned int OBJECT_OPERATOR : 1 ;
    unsigned int DOT : 1 ;
    unsigned int COMMENT : 1 ;
    unsigned int NO_OP_WORD : 1 ;
    unsigned int SYNTACTIC : 1 ;
    unsigned int CATEGORY_PLUS_PLUS_MINUS_MINUS : 1 ; // =
    unsigned int CATEGORY_PP_MM : 1 ;
    unsigned int CATEGORY_OP_STORE : 1 ;
    unsigned int CATEGORY_DUP : 1 ;
    unsigned int CATEGORY_OP_ORDERED : 1 ;
    unsigned int CATEGORY_OP_LOAD : 1 ;
    unsigned int CATEGORY_OP_EQUAL : 1 ;
    unsigned int CATEGORY_LOGIC : 1 ;
    unsigned int CATEGORY_OP_1_ARG : 1 ;
    unsigned int VOID_RETURN : 1 ;
    unsigned int CATEGORY_OP_UNORDERED : 1 ;
    unsigned int CATEGORY_OP_STACK : 1 ;
    unsigned int CATEGORY_OP_DIVIDE : 1 ;
    unsigned int C_PREFIX : 1 ;
    unsigned int ALIAS : 1 ;
    unsigned int NO_CODING : 1 ;
    unsigned int C_PREFIX_RTL_ARGS : 1 ;
    unsigned int T_TOS : 1 ;
    unsigned int BLOCK_DELIMITER : 1 ;
    unsigned int DEBUG_WORD : 1 ;
    unsigned int BIT_SHIFT : 1 ;
    unsigned int WORD_CREATE : 1 ;
    unsigned int INTERPRET_DBG : 1 ;
    unsigned int INFIX_WORD : 1 ;
    unsigned int DLSYM_WORD : 1 ;
    unsigned int RAX_RETURN : 1 ;
    unsigned int RT_STEPPING_DEBUG : 1 ;
    unsigned int ADDRESS_OF_OP : 1 ;
    unsigned int LOGIC_NEGATE : 1 ;
    unsigned int COMBINATOR : 1 ;
    unsigned int CATEGORY_OP : 1 ;
    unsigned int CATEGORY_OP_OPEQUAL : 1 ;
    unsigned int TEXT_MACRO : 1 ;
    unsigned int STRING_MACRO : 1 ;
    unsigned int T_OBJECT : 1 ;
    unsigned int RECYCLABLE_COPY : 1 ;
    unsigned int RECYCLABLE_LOCAL : 1 ;
    unsigned int T_TYPE_VARIABLE : 1 ;
    unsigned int T_ANY_FIXED_TYPE : 1 ; // =
    unsigned int T_ANY : 1 ;
    unsigned int C_INFIX_OP_EQUAL : 1 ;
    unsigned int STRUCTURE : 1 ; // =
    unsigned int STRUCT : 1 ;
    unsigned int RAW_STRING : 1 ;
    unsigned int T_RAW_STRING : 1 ;
    unsigned int T_INT32 : 1 ;
    unsigned int T_INT16 : 1 ;
    unsigned int T_STRING : 1 ;
    unsigned int T_INT64 : 1 ; // =
    unsigned int T_INT : 1 ;
    unsigned int T_FLOAT : 1 ;
    unsigned int T_BOOLEAN : 1 ;
    unsigned int T_BIG_NUM : 1 ;
    unsigned int T_BYTE : 1 ; // =
    unsigned int T_CHAR : 1 ;
    unsigned int T_VOID : 1 ;
    unsigned int C_TYPE : 1 ;
    unsigned int T_UNDEFINED : 1 ;
    unsigned int OBJECT : 1 ;
    unsigned int OBJECT_FIELD : 1 ;
    unsigned int DYNAMIC_OBJECT : 1 ;
    unsigned int DOBJECT : 1 ;
    unsigned int C_CLASS : 1 ;
    unsigned int C_TYPEDEF : 1 ;
    unsigned int T_SIZE_CHECK : 1 ;
    unsigned int THIS : 1 ;
    unsigned int QUALIFIED_ID : 1 ; // =
    unsigned int QID : 1 ;
    unsigned int CLASS_CLONE : 1 ;
    unsigned int LITERAL : 1 ;
    unsigned int REGISTER_VARIABLE : 1 ;
    unsigned int NAMESPACE_VARIABLE : 1 ;
    unsigned int PARAMETER_VARIABLE : 1 ;
    unsigned int STACK_VARIABLE : 1 ;
    unsigned int LOCAL_VARIABLE : 1 ;
    unsigned int CONSTANT : 1 ;
    unsigned int NAMESPACE : 1 ;
    unsigned int CLASS : 1 ;
    unsigned int LOCAL_OBJECT : 1 ;
    unsigned int CLASS_FIELD : 1 ;
    unsigned int T_POINTER : 1 ;
    unsigned int WT_PREFIX : 1 ;
    unsigned int WT_INFIXABLE : 1 ;
    unsigned int WT_C_PREFIX_RTL_ARGS : 1 ;
    unsigned int WT_POSTFIX : 1 ;
    unsigned int WT_QID : 1 ;
    unsigned int WT_C_SYNTAX : 1 ;
    unsigned int W_COMMENT : 1 ;
    unsigned int W_PREPROCESSOR : 1 ;
    unsigned int T_LAMBDA : 1 ;
    unsigned int T_CONS : 1 ;
    unsigned int LIST : 1 ;
    unsigned int LISP_C_RTL_ARGS : 1 ;
    unsigned int LIST_OBJECT : 1 ;
    unsigned int LIST_QUOTATION : 1 ;
    unsigned int T_LISP_IF : 1 ;
    unsigned int T_LISP_SET : 1 ;
    unsigned int LIST_NODE : 1 ;
    unsigned int T_LISP_SPECIAL : 1 ;
    unsigned int T_LISP_BEGIN : 1 ;
    unsigned int T_LISP_TOKEN : 1 ;
    unsigned int T_LC_LITERAL : 1 ;
    unsigned int T_LISP_MACRO : 1 ;
    unsigned int LISP_VOID_RETURN : 1 ;
    unsigned int T_NIL : 1 ;
    unsigned int LIST_FUNCTION : 1 ; // =
    unsigned int LISP_WORD : 1 ;
    unsigned int T_LISP_COMPILED_WORD : 1 ;
    unsigned int T_LISP_COMPILE : 1 ;
    unsigned int T_LISP_LET : 1 ;
    unsigned int T_LISP_TERMINATING_MACRO : 1 ;
    unsigned int T_LISP_READ_MACRO : 1 ;
    unsigned int T_LISP_LOCALS_ARG : 1 ;
    unsigned int T_LISP_UNQUOTE_SPLICING : 1 ;
    unsigned int T_LC_DEFINE : 1 ;
    unsigned int T_LISP_VALUE : 1 ;
    unsigned int T_LISP_WORD : 1 ;
    unsigned int T_LC_NEW : 1 ;
    unsigned int T_LISP_CSL : 1 ;
    unsigned int T_LISP_COLON : 1 ;
    unsigned int T_LISP_IMMEDIATE : 1 ;
    unsigned int T_LISP_CSL_COMPILED : 1 ;
} AttributeBitField ;
#endif

// MorphismAttributes, ObjectAttributes & LispAttributes -shared in common 
#define CPRIMITIVE      ( (uint64) 1 << 0 )
#define CSL_WORD     ( (uint64) 1 << 1 )
#define CSL_ASM_WORD  ( (uint64) 1 << 2 ) // machine code words
#define T_LISP_SYMBOL   ( (uint64) 1 << 3 )
#define NOT_A_KNOWN_OBJECT ( (uint64) 1 << 6 )  
#define KNOWN_OBJECT ( (uint64) 1 << 7 )  

#define T_HEAD          ( (uint64) 1 << 8 )
#define T_TAIL          ( (uint64) 1 << 9 )
#define RIGHT_BRACKET       ( (uint64) 1 << 10 ) // ']'
#define LEFT_BRACKET        ( (uint64) 1 << 11 ) // '['

// MorphismAttributes - morphism (non-object/type) defines (generally)
#define IMMEDIATE           ( (uint64) 1 << 12 )
#define KEYWORD             ( (uint64) 1 << 13 )
#define BLOCK               ( (uint64) 1 << 14 )
#define INLINE              ( (uint64) 1 << 15 )

//#define LISP_csl         ( (uint64) 1 << 16 )
#define LEFT_PAREN          ( (uint64) 1 << 17 ) // '('
#define RIGHT_PAREN         ( (uint64) 1 << 18 ) // ')'
#define OP_RAX_PLUS_1ARG    ( (uint64) 1 << 19 )

#define PREFIX              ( (uint64) 1 << 20 )
#define INFIXABLE           ( (uint64) 1 << 21 )
#define OBJECT_OPERATOR     ( (uint64) 1 << 22 ) 
#define DOT                 OBJECT_OPERATOR              

#define PREFIXABLE    ( (uint64) 1 << 23 ) 
#define COMMENT             ( (uint64) 1 << 24 ) 
#define NO_OP_WORD          ( (uint64) 1 << 25 ) 
#define SYNTACTIC           ( (uint64) 1 << 26 ) 
//#define DO_DOES             ( (uint64) 1 << 27 ) 

#define CATEGORY_PLUS_PLUS_MINUS_MINUS ( (uint64) 1 << 28 )
#define CATEGORY_PP_MM CATEGORY_PLUS_PLUS_MINUS_MINUS
#define CATEGORY_OP_STORE ( (uint64) 1 << 29 )
#define CATEGORY_DUP ( (uint64) 1 << 30 )
#define CATEGORY_OP_ORDERED ( (uint64) 1 << 31 )
#define CATEGORY_OP_LOAD ( (uint64) 1 << 32 )
#define CATEGORY_OP_EQUAL ( (uint64) 1 << 33 )
#define CATEGORY_LOGIC ( (uint64) 1 << 34 )
#define CATEGORY_OP_1_ARG ( (uint64) 1 << 35 )
#define VOID_RETURN ( (uint64) 1 << 36 )
#define CATEGORY_OP_UNORDERED ( (uint64) 1 << 37 )
#define CATEGORY_OP_STACK ( (uint64) 1 << 38 )
#define CATEGORY_OP_DIVIDE ( (uint64) 1 << 39 )
#define C_PREFIX ( (uint64) 1 << 40 )
#define ALIAS ( (uint64) 1 << 41 )
#define NO_CODING           ( (uint64) 1 <<  42 ) 
#define C_PREFIX_RTL_ARGS ( (uint64) 1 << 43 )
#define T_TOS ( (uint64) 1 << 44 )

#define BLOCK_DELIMITER     ( (uint64) 1 << 47 )

#define DEBUG_WORD ( (uint64) 1 << 48 )
#define BIT_SHIFT        ( (uint64) 1 << 49 ) 
#define WORD_CREATE ( (uint64) 1 << 50 ) 
#define INTERPRET_DBG ( (uint64) 1 << 51 ) 
#define INFIX_WORD      ( (uint64) 1 << 52 ) 
//#define C_RETURN        ( (uint64) 1 << 53 ) 
#define DLSYM_WORD      ( (uint64) 1 << 54 ) 

#define RAX_RETURN          ( (uint64) 1 << 55 ) 
//#define CATEGORY_SHIFT      ( (uint64) 1 << 56 ) 
#define RT_STEPPING_DEBUG   ( (uint64) 1 << 57 )
#define ADDRESS_OF_OP       ( (uint64) 1 << 58 )

//#define PREFIXABLE          ( (uint64) 1 << 59 ) 
#define LOGIC_NEGATE        ( (uint64) 1 << 59 )
#define COMBINATOR          ( (uint64) 1 << 61 )
#define CATEGORY_OP         ( (uint64) 1 << 62 )
#define CATEGORY_OP_OPEQUAL ( (uint64) 1 << 63 )

// ObjectAttributes - object and type defines
//#define ARRAY_TYPE          ( (uint64) 1 <<  8 ) 
//#define VARIABLE            ( (uint64) 1 <<  9 ) 
#define TEXT_MACRO          ( (uint64) 1 << 15 )
#define STRING_MACRO        ( (uint64) 1 << 16 )
#define T_OBJECT            ( (uint64) 1 << 17 )
//#define T_MORPHISM          ( (uint64) 1 << 18 )
#define DOBJECT_FIELD        ( (uint64) 1 << 18 )
#define RECYCLABLE_COPY     ( (uint64) 1 << 21 )
#define RECYCLABLE_LOCAL    ( (uint64) 1 << 22 )
#define T_TYPE_VARIABLE     ( (uint64) 1 << 23 )
#define T_ANY_FIXED_TYPE    ( (uint64) 1 << 28 )
#define T_ANY               T_ANY_FIXED_TYPE    
#define C_INFIX_OP_EQUAL    ( (uint64) 1 << 26 )
#define STRUCTURE           ( (uint64) 1 << 27 )
#define STRUCT              STRUCTURE         
#define RAW_STRING          ( (uint64) 1 << 28 )
#define T_INT32             ( (uint64) 1 << 29 )
#define T_INT16             ( (uint64) 1 << 30 )

#define T_RAW_STRING        ( (uint64) 1 << 31 )
#define T_STRING            ( (uint64) 1 << 32 )
#define T_INT64             ( (uint64) 1 << 33 )
#define T_INT T_INT64
#define T_FLOAT             ( (uint64) 1 << 34 )
#define T_BOOLEAN           ( (uint64) 1 << 35 )
#define T_BIG_NUM           ( (uint64) 1 << 36 )
#define T_BYTE              ( (uint64) 1 << 37 )
#define T_CHAR              T_BYTE
#define T_INT8              T_BYTE
#define T_VOID              ( (uint64) 1 << 38 )

#define C_TYPE              ( (uint64) 1 << 39 ) 
#define T_UNDEFINED         ( (uint64) 1 << 40 )
#define OBJECT              ( (uint64) 1 << 41 )
#define OBJECT_FIELD        ( (uint64) 1 << 42 )
#define DYNAMIC_OBJECT      ( (uint64) 1 << 43 )
#define DOBJECT             DYNAMIC_OBJECT
#define C_CLASS             ( (uint64) 1 << 44 ) 
#define C_TYPEDEF           ( (uint64) 1 << 45 ) 
#define T_SIZE_CHECK        ( (uint64) 1 << 46 ) //??
#define THIS                ( (uint64) 1 << 47 )
#define QUALIFIED_ID        ( (uint64) 1 << 48 ) 
#define QID                 QUALIFIED_ID 
#define CLASS_CLONE         ( (uint64) 1 << 49 ) 

#define LITERAL             ( (uint64) 1 << 50 )
#define REGISTER_VARIABLE   ( (uint64) 1 << 51 )
#define NAMESPACE_VARIABLE  ( (uint64) 1 << 52 )
#define PARAMETER_VARIABLE  ( (uint64) 1 << 53 )
#define STACK_VARIABLE      PARAMETER_VARIABLE 
#define LOCAL_VARIABLE      ( (uint64) 1 << 54 )
#define CONSTANT            ( (uint64) 1 << 55 )
#define NAMESPACE           ( (uint64) 1 << 57 )
#define CLASS               ( (uint64) 1 << 58 )
#define LOCAL_OBJECT        ( (uint64) 1 << 59 )
#define CLASS_FIELD         ( (uint64) 1 << 60 ) 
#define T_POINTER           ( (uint64) 1 << 61 ) 
#define MEMORY_SPACE           ( (uint64) 1 << 62 )

// WAttribute for interpreter word types : added in primitives.c in th LAttribute field
#define WT_PREFIX                 ( (uint64) 1 <<  0 ) 
#define WT_INFIXABLE              ( (uint64) 1 <<  1 ) 
#define WT_C_PREFIX_RTL_ARGS      ( (uint64) 1 <<  2 ) 
#define WT_POSTFIX                ( (uint64) 1 <<  3 ) 
#define WT_QID                    ( (uint64) 1 <<  4 ) 
#define WT_C_SYNTAX               ( (uint64) 1 <<  5 )
#define W_COMMENT                 ( (uint64) 1 <<  6 )
#define W_PREPROCESSOR            ( (uint64) 1 <<  7 )
#define WT_INIT                         ( (uint64) 1 << 8)
#define WT_PREFIXABLE              ( (uint64) 1 <<  9 ) 

// LType - lisp types
#define T_LAMBDA ( (uint64) 1 << 11 )
#define T_CONS ( (uint64) 1 << 12 )
#define LIST ( (uint64) 1 << 13 )
#define LISP_C_RTL_ARGS ( (uint64) 1 << 14 )
#define LIST_OBJECT ( (uint64) 1 << 15 )
#define LIST_QUOTATION ( (uint64) 1 << 16 )
#define T_LISP_IF ( (uint64) 1 << 17 )
#define T_LISP_SET ( (uint64) 1 << 18 )
#define LIST_NODE ( (uint64) 1 << 19 )
#define T_LISP_SPECIAL ( (uint64) 1 << 20 )
#define T_LISP_BEGIN ( (uint64) 1 << 21 )
#define T_LISP_TOKEN ( (uint64) 1 << 22 )
#define T_LC_LITERAL ( (uint64) 1 << 23 )
#define T_LISP_MACRO ( (uint64) 1 << 24 )
#define LISP_VOID_RETURN ( (uint64) 1 << 26 )
#define T_NIL ( (uint64) 1 << 27 )
#define LIST_FUNCTION ( (uint64) 1 << 28 )
#define LISP_WORD LIST_FUNCTION 
#define T_LISP_COMPILED_WORD ( (uint64) 1 << 29 )
#define T_LISP_COMPILE ( (uint64) 1 << 30 )
#define T_LISP_LET ( (uint64) 1 << 31 )
#define T_LISP_TERMINATING_MACRO ( (uint64) 1 << 32 )
#define T_LISP_READ_MACRO ( (uint64) 1 << 33 )
#define T_LISP_LOCALS_ARG ( (uint64) 1 << 35 )
#define T_LISP_UNQUOTE_SPLICING ( (uint64) 1 << 36 )
#define T_LC_DEFINE ( (uint64) 1 << 37 )
#define T_LISP_VALUE ( (uint64) 1 << 38 )
#define T_LISP_WORD ( (uint64) 1 << 39 )
#define T_LC_NEW ( (uint64) 1 << 40 )
#define T_LISP_CSL ( (uint64) 1 << 45 )
#define T_LISP_COLON ( (uint64) 1 << 46 )
#define T_LISP_IMMEDIATE ( (uint64) 1 << 48 )
#define T_LISP_CSL_COMPILED ( (uint64) 1 << 49 )

//#define   ( (uint64) 1 <<  )
#define NEW_RUN_COMPOSITE 0
#define EXIT ( (uint64) 1 << 30 )
#define PAUSE ( (uint64) 1 << 11)
#define COMPLETE_INITIAL_START ( (uint64) 1 << 12)
#define INITIAL_START ( (uint64) 1 << 10)
#define FULL_RESTART ( (uint64) 1 << 9 )
#define RESTART ( (uint64) 1 << 8 )
#define RESET_ALL ( (uint64) 1 << 7 )
#define ABORT ( (uint64) 1 << 6 )
#define QUIT ( (uint64) 1 << 5 )
// don't use 4 because it is also SIGILL ??
#define CSL_RUN_INIT ( (uint64) 1 << 4 )
#define STOP ( (uint64) 1 << 3 )
#define BREAK ( (uint64) 1 << 2 )
#define CONTINUE ( (uint64) 1 << 1 )
#define ON ( (uint64) 1 << 0 )
#define OFF ( 0 )
#define PROMPT "-: "
#define STARTUP_TIME_IT 8 // random number not 0 or 1
// newOptimize
#define OPT_BREAK       2
#define OPT_CONTINUE    1
#define OPT_DONE        0

// GotoInfo Types
#define GI_BREAK ( (uint64) 1 << 0 )
#define GI_RETURN ( (uint64) 1 << 1 )
#define GI_CONTINUE ( (uint64) 1 << 2 )
#define GI_GOTO ( (uint64) 1 << 3 )
#define GI_RECURSE ( (uint64) 1 << 4 )
#define GI_TAIL_CALL ( (uint64) 1 << 5 )
#define GI_LABEL ( (uint64) 1 << 6 )
#define GI_GOTO_LABEL ( (uint64) 1 << 7 )

// MemChunk / Memory Types
#define OPENVMTIL ( (uint64) 1 << 0 )
#define T_CSL ( (uint64) 1 << 1 )
#define PERMANENT ( (uint64) 1 << 2 )
#define TEMPORARY ( (uint64) 1 << 3 )
#define TEMP_OBJECT_MEMORY TEMPORARY
#define DATA_STACK ( (uint64) 1 << 4 )
#define SESSION ( (uint64) 1 << 5 )
#define DICTIONARY ( (uint64) 1 << 6 )
#define EXISTING ( (uint64) 1 << 7 )
// can't use these for chunk types
//#define T_HEAD ( (uint64) 1 << 8 )
//#define T_TAIL ( (uint64) 1 << 9 )
#define CONTEXT ( (uint64) 1 << 10 )
#define BUFFER ( (uint64) 1 << 12 )
#define LISP_TEMP ( (uint64) 1 << 13 )
#define OBJECT_MEM ( (uint64) 1 << 14 )
#define LISP ( (uint64) 1 << 15 )
#define HISTORY ( (uint64) 1 << 16 )
#define CODE ( (uint64) 1 << 17 )
#define COMPILER_TEMP_OBJECT_MEMORY ( (uint64) 1 << 18 )
#define COMPILER_TEMP COMPILER_TEMP_OBJECT_MEMORY 
#define LARGEST_MEMORY_TYPE COMPILER_TEMP_OBJECT_MEMORY
#define OVT_STATIC ( (uint64) 1 << 19 )
#define STRING_MEMORY ( (uint64) 1 << 20 )
#define STRING_MEM STRING_MEMORY
#define SESSION_CODE ( (uint64) 1 << 21 )
#define INTERNAL_OBJECT_MEM ( (uint64) 1 << 22 )
#define WORD_RECYCLING ( (uint64) 1 << 23 )
#define WORD_COPY_MEM ( (uint64) 1 << 24 )
#define _STATIC_ ( (uint64) 1 << 25 )

//#define ALL      ( INTERNAL | EXTERNAL | csl | TEMPORARY | DATA_STACK | SESSION )

#define OPTIMIZE_RM ( (uint64) 1 << 0 )
#define OPTIMIZE_IMM ( (uint64) 1 << 1 )
#define OPTIMIZE_REG_REG ( (uint64) 1 << 2 )
#define OPTIMIZE_REG_MEM ( (uint64) 1 << 3 )
#define OPTIMIZE_MEM_REG ( (uint64) 1 << 4 )
#define OPTIMIZE_R8_ECX ( (uint64) 1 << 5 )
#define OPTIMIZE_ECX_R8 ( (uint64) 1 << 6 )
#define OPTIMIZE_R8_MEM ( (uint64) 1 << 7 )
#define OPTIMIZE_MEM_R8 ( (uint64) 1 << 8 )
#define OPTIMIZE_MEM_ECX ( (uint64) 1 << 9 )
#define OPTIMIZE_ECX_MEM ( (uint64) 1 << 10 )
#define OPTIMIZE_TO_MEM  ( (uint64) 1 << 11 )
#define OPTIMIZE_TO_REG  ( (uint64) 1 << 12 )
#define OPTIMIZE_DONE  ( (uint64) 1 << 13 )
#define OPTIMIZE_REGISTER  ( (uint64) 1 << 14 )
#define OPTIMIZE_DONT_RESET  ( (uint64) 1 << 15 )
#define OPTIMIZE_RESET  ( (uint64) 1 << 16 )
//#define OPTIMIZE_TOS ( (uint64) 1 << 11 )
//#define OPTIMIZE_NOS ( (uint64) 1 << 12 ) // Next On Stack = Dsp [ -1 ]


// Debugger Flags
//#define DBG_DONE ( (uint64) 1 << 0 ) 
#define DBG_MENU ( (uint64) 1 << 1 ) 
#define DBG_STEPPING ( (uint64) 1 << 2 ) 
#define DGB_SAVE_DSP ( (uint64) 1 << 3 ) 
//#define DBG_RESTORE_REGS ( (uint64) 1 << 4 ) 
#define DBG_CAN_STEP ( (uint64) 1 << 5 ) 
#define DBG_NEWLINE ( (uint64) 1 << 6 )
#define DBG_PROMPT ( (uint64) 1 << 7 )
#define DBG_INFO ( (uint64) 1 << 8 )
#define DBG_ACTIVE ( (uint64) 1 << 9 )
#define DBG_AUTO_MODE ( (uint64) 1 << 10 )
#define DBG_INTERNAL ( (uint64) 1 << 11 ) 
#define DBG_STEPPING_THRU_C_IMMEDIATE ( (uint64) 1 << 12 ) 
#define DBG_ESCAPED ( (uint64) 1 << 13 )
#define DBG_COMMAND_LINE ( (uint64) 1 << 14 )
#define DBG_CONTINUE ( (uint64) 1 << 15 )
#define DBG_RETURN ( (uint64) 1 << 16 )
#define DBG_STEPPED ( (uint64) 1 << 17 )
#define DBG_PRE_DONE ( (uint64) 1 << 18 )
#define DBG_SHOW_STACK_CHANGE ( (uint64) 1 << 19 )
#define DBG_BRK_INIT ( (uint64) 1 << 20 )
#define DBG_RUNTIME ( (uint64) 1 << 21 )
#define DBG_COMPILE_MODE ( (uint64) 1 << 22 )
#define DBG_SKIP_INNER_SHOW ( (uint64) 1 << 23 )
#define DBG_debugLevel_SHOW_WRITTEN_CODE ( (uint64) 1 << 24 )
#define DBG_JCC_INSN ( (uint64) 1 << 25 )
#define DBG_INTERPRET_LOOP_DONE ( (uint64) 1 << 26 )
#define DBG_REGS_SAVED ( (uint64) 1 << 27 )
#define DBG_AUTO_MODE_ONCE ( (uint64) 1 << 28 )
#define DBG_FILENAME_LOCATION_SHOWN ( (uint64) 1 << 29 )
#define DBG_STACK_OLD ( (uint64) 1 << 30 )
#define DBG_RUNTIME_BREAKPOINT ( (uint64) 1 << 31 )
#define DBG_EVAL_AUTO_MODE ( (uint64) 1 << 32 )
#define DBG_CONTINUE_MODE ( (uint64) 1 << 32 )
#define DBG_ASM_SHOW_ON ( (uint64) 1 << 33 )
#define DBG_DISASM_ACC ( (uint64) 1 << 34 )
#define DBG_START_STEPPING ( (uint64) 1 << 35 )
#define DBG_OUTPUT_INSERTION ( (uint64) 1 << 36 )
#define DBG_OUTPUT_SUBSTITUTION ( (uint64) 1 << 37 )
#define DBG_SETUP_ADDRESS ( (uint64) 1 << 38 )
#define DBG_INFIX_PREFIX ( (uint64) 1 << 39 )

#define SIZEOF_AddressAfterJmpCallStack 16
// TODO : nb. flags need to be edited !!!!! for right category, overlap, use/non-use, etc.
// CSL state flags added to System flags
#define CSL_RUN ( (uint64) 1 << 0 )
#define SOURCE_CODE_STARTED  ( (uint64) 1 << 1 )
#define SOURCE_CODE_ON  ( (uint64) 1 << 2 )
#define DEBUG_MODE ( (uint64) 1 << 7 )
#define DEBUG_SOURCE_CODE_MODE ( (uint64) 1 << 8 )
#define STRING_MACROS_ON ( (uint64) 1 << 9 )
#define GLOBAL_SOURCE_CODE_MODE ( (uint64) 1 << 10 )
#define _DEBUG_SHOW_ ( (uint64) 1 << 11 )
#define DEBUG_SHTL_OFF ( (uint64) 1 << 12 ) // SHTL == _String_HighlightTokenInputLine
#define SC_debugLevel_PUSH ( (uint64) 1 << 13 )
#define DBG_TYPECHECK_ON ( (uint64) 1 << 14 )
//#define TYPECHECK_OFF ( (uint64) 1 << 15 )

// State Flags --
// ? sytem flags ?? -- not all -- todo here
#define ADD_READLINE_TO_HISTORY ( (uint64) 1 << 8 )
//#define SYSTEM_CALLS ( (uint64) 1 << 9 )
#define VERBOSE ( (uint64) 1 << 10 )
#define PREPROCESSOR_DEFINE ( (uint64) 1 << 11 )
#define END_OF_FILE ( (uint64) 1 << 12 )
#define GET_NEWLINE ( (uint64) 1 << 13 )
#define PRINTED ( (uint64) 1 << 14 )
#define JCC8_ON ( (uint64) 1 << 17 ) // Runtime Debug
#define DO_PROMPT ( (uint64) 1 << 18 )
#define AUTO_VAR ( (uint64) 1 << 19 )
#define DO_OK ( (uint64) 1 << 20 )
#define ALWAYS_ABORT_ON_EXCEPION ( (uint64) 1 << 21 )  // for debugging
#define DOT_WORD_COMPLETION ( (uint64) 1 << 22 )
#define DEBUG_ON ( (uint64) 1 << 23 )
#define REGISTERS_MODE_ON ( (uint64) 1 << 24 )
#define OPTIMIZE_ON ( (uint64) 1 << 25 )
#define INLINE_ON ( (uint64) 1 << 26 )
#define READLINE_ECHO_ON ( (uint64) 1 << 27 )
#define IN_OPTIMIZER ( (uint64) 1 << 28 )
#define SCA_ON ( (uint64) 1 << 29 )
#define TYPECHECK_ON ( (uint64) 1 << 30 )
#define RT_DEBUG_ON ( (uint64) 1 << 31 ) // Runtime Debug
// CSL state flags
#define PP_INCLUDE_FILES_ONLY ( (uint64) 1 << 32 )


// interpreter flags
#define INTERPRETER_DONE ( (uint64) 1 << 28 )
#define INTERPRETER_DBG_ACTIVE ( (uint64) 1 << 29 )
#define INTERPRETER_QID ( (uint64) 1 << 30 )
#define PREPROCESSOR_MODE ( (uint64) 1 << 31 )
//#define IS_FORWARD_DOTTED ( (uint64) 1 << 32 )
//#define IS_REVERSE_DOTTED ( (uint64) 1 << 33 )
//#define PREPROCESSOR_IF_TRUE ( (uint64) 1 << 31 )

// lexer flags
#define LEXER_DONE   ( (uint64) 1 << 0)
#define SINGLE_ESCAPE ( (uint64) 1 << 1 )
#define MULTIPLE_ESCAPE ( (uint64) 1 << 2 )
#define ADD_TOKEN_TO_SOURCE ( (uint64) 1 << 3 ) 
//#define NOT_A_KNOWN_OBJECT ( (uint64) 1 << 4 )  
//#define KNOWN_OBJECT ( (uint64) 1 << 5 )  
#define APPEND_NEWLINE ( (uint64) 1 << 6 ) 
#define PARSING_STRING ( (uint64) 1 << 7 ) 
#define LEXER_END_OF_STRING ( (uint64) 1 << 8 )
#define LEXER_LISP_MODE ( (uint64) 1 << 9 )
#define ADD_CHAR_TO_SOURCE ( (uint64) 1 << 10 ) 
#define LEXER_RETURN_NULL_TOKEN ( (uint64) 1 << 11 ) 
#define LEXER_ALLOW_DOT ( (uint64) 1 << 12 ) 
#define LEXER_END_OF_LINE ( (uint64) 1 << 13 ) 
#define LEXER_ESCAPE_SEQUENCE ( (uint64) 1 << 14 ) 
#define LEXER_FORWARD_DOTTED ( (uint64) 1 << 15 ) 

// readline flags and defines
#define PROMPT_LENGTH ( Strlen ( (char*) rl->Prompt ) ) 
#define END_OF_LINE ( (uint64) 1 << 3 )
#define END_OF_INPUT ( (uint64) 1 << 4 )
#define TAB_WORD_COMPLETION ( (uint64) 1 << 5 )
#define CHAR_ECHO ( (uint64) 1 << 6 )
#define ANSI_ESCAPE ( (uint64) 1 << 7 )
#define READLINER_DONE   ( (uint64) 1 << 8)
#define END_OF_STRING ( (uint64) 1 << 9 )
#define STRING_MODE ( (uint64) 1 << 10 )
#define ADD_TO_HISTORY ( (uint64) 1 << 11 )

// compiler flags
#define COMPILE_MODE ( (uint64) 1 << 0 )
#define BLOCK_MODE COMPILE_MODE
#define ADD_FRAME ( (uint64) 1 << 1 )
#define COMPILER_LEFT_BRACKET ( (uint64) 1 << 2 )
#define SAVE_Rsp ( (uint64) 1 << 3 ) 
#define RETURN_NONE ( (uint64) 1 << 5 ) 
#define RETURN_TOS ( (uint64) 1 << 6 )
//#define RETURN_ACCUM ( (uint64) 1 << 7 ) 
#define LISP_COMBINATOR_MODE ( (uint64) 1 << 8 )
#define LISP_MODE ( (uint64) 1 << 9 )
#define INFIX_MODE ( (uint64) 1 << 10 )
#define RETURN_VAR ( (uint64) 1 << 11 )
#define VARIABLE_FRAME ( (uint64) 1 << 12 )
#define DONT_REMOVE_STACK_VARIABLES ( (uint64) 1 << 13 )
#define C_COMBINATOR_LPAREN ( (uint64) 1 << 14 )
#define LC_ARG_PARSING ( (uint64) 1 << 16 )
#define INTERPRET_NBLOCKS ( (uint64) 1 << 17 )
#define PREFIX_ARG_PARSING ( (uint64) 1 << 18 )
#define LC_C_RTL_ARG_PARSING ( (uint64) 1 << 19 )
//#define PREFIX_PARSING ( (uint64) 1 << 20 )
#define DOING_C_TYPE ( (uint64) 1 << 21 )
#define DOING_A_PREFIX_WORD ( (uint64) 1 << 22 )
#define C_INFIX_EQUAL ( (uint64) 1 << 24 )
#define C_COMBINATOR_PARSING ( (uint64) 1 << 25 )
#define INFIX_LIST_INTERPRET ( (uint64) 1 << 26 )
#define COMPILER_WRAP_ON ( (uint64) 1 << 27 )
#define DOING_AN_INFIX_WORD ( (uint64) 1 << 28 )
#define DOING_BEFORE_AN_INFIX_WORD ( (uint64) 1 << 29 )
#define ARRAY_MODE ( (uint64) 1 << 30 )
#define DOING_BEFORE_A_PREFIX_WORD ( (uint64) 1 << 31 )
#define C_CONDITIONAL_IN ( (uint64) 1 << 32 )
#define C_TYPEDEC_WITH_EQUAL ( (uint64) 1 << 33 )
#define ASM_MODE ( (uint64) 1 << 34 )
#define TDSCI_PARSING ( (uint64) 1 << 35 )
#define C_BLOCK_INTERPRETER ( (uint64) 1 << 36 )
#define DOING_C_TYPE_DECLARATION ( (uint64) 1 << 37 )
#define DOING_CASE ( (uint64) 1 << 38 )
#define DOING_RETURN ( (uint64) 1 << 39 )
#define IN_DO_WHILE ( (uint64) 1 << 40 )

// Context flags
#define C_SYNTAX ( (uint64) 1 << 20 )
#define CONTEXT_PREPROCESSOR_MODE ( (uint64) 1 << 21 )
#define PREFIX_MODE ( (uint64) 1 << 22 )
#define C_LHS ( (uint64) 1 << 23 )
#define C_RHS ( (uint64) 1 << 24 )
#define CONTEXT_PARSING_QID ( (uint64) 1 << 25 ) 
#define CONTEXT_LAST_WORD_IN_QID ( (uint64) 1 << 26 ) 
#define ADDRESS_OF_MODE ( (uint64) 1 << 27 ) 
#define AT_COMMAND_LINE ( (uint64) 1 << 28 ) 
#define LC_INTERPRET     ( (uint64) 1 << 29 ) 
#define LC_CSL ( (uint64) 1 << 30 )
#define IS_FORWARD_DOTTED ( (uint64) 1 << 31 )
#define IS_REVERSE_DOTTED ( (uint64) 1 << 32 )
#define IS_RVALUE ( (uint64) 1 << 33 )
#define ASM_SYNTAX ( (uint64) 1 << 34 )
#define PREPROCESSOR_IF_MODE ( (uint64) 1 << 35 )

#define NON_INLINABLE ( (uint64) 1 << 0 )
#define DONE true

// block flag values
#define debugLevel_COMPILE 1
#define debugLevel_RUN 2

// Word - Namespace State flags
#define TREED ( (uint64) 1 << 2 )
//#define LC_NOT_COMPILED ( (uint64) 1 << 3 )
#define COMPILED_INLINE ( (uint64) 1 << 4 )
#define COMPILED_OPTIMIZED ( (uint64) 1 << 5 )
#define UNQUOTED ( (uint64) 1 << 6 )
#define LISP_EVALLIST_ARG ( (uint64) 1 << 7 )
#define UNQUOTE_SPLICE ( (uint64) 1 << 8 )
#define QUOTED ( (uint64) 1 << 9 )
#define QUASIQUOTED ( (uint64) 1 << 10 )
#define LC_DEFINED ( (uint64) 1 << 11 )
#define SPLICE ( (uint64) 1 << 12 )
#define MORPHISM_WORD ( (uint64) 1 << 13 )
#define NON_MORPHISM_WORD ( (uint64) 1 << 14 )
#define W_INFIX_MODE ( (uint64) 1 << 15 )
#define W_C_SYNTAX ( (uint64) 1 << 16 )
#define STEPPED ( (uint64) 1 << 17 )
#define W_SOURCE_CODE_MODE ( (uint64) 1 << 18 )
#define W_INITIALIZED ( (uint64) 1 << 19 )
#define NOT_USING ( (uint64) 1 << 24 )
#define USING ( (uint64) 1 << 25 )
#define ANY (NOT_USING|USING)
#define DEBUG_INFO_SAVED ( (uint64) 1 << 26 )
#define W_JCC8_ON ( (uint64) 1 << 27 ) // Runtime Debug

// Lisp State flags 
#define LC_PRINT_VALUE          ( (uint64) 1 << 0 )
#define LC_DEFINE_MODE          ( (uint64) 1 << 1 )
//#define LC_CSL_MODE          ( (uint64) 1 << 2 )
#define LC_INTERP_DONE          ( (uint64) 1 << 3 )
#define LC_REPL                 ( (uint64) 1 << 4 )
#define LC_LAMBDA_MODE          ( (uint64) 1 << 5 )
#define LC_COMPILE_MODE         ( (uint64) 1 << 6 )
#define LC_BLOCK_COMPILE        ( (uint64) 1 << 7 )
#define LC_INTERP_MODE          ( (uint64) 1 << 8 )
#define LC_PRINT_ENTERED        ( (uint64) 1 << 9 )
#define LC_READ                 ( (uint64) 1 << 10 )
#define LC_EVAL                 ( (uint64) 1 << 11 )
#define LC_APPLY                ( (uint64) 1 << 12 )
#define LC_PRINT                ( (uint64) 1 << 13 )
#define LC_READ_MACRO_OFF       ( (uint64) 1 << 14 )
#define LC_READ_ONLY            ( (uint64) 1 << 15 )
#define LC_READ_LAMBDA_LOCALS   ( (uint64) 1 << 16 )
#define LC_LAMBDA_COMPILE_MODE  ( (uint64) 1 << 17 )
#define LC_EVAL_APPLY           ( (uint64) 1 << 18 )
#define LC_DONE                 ( (uint64) 1 << 19 )
#define LC_BEGIN_MODE           ( (uint64) 1 << 20 )
//#define COMBINATOR_MODE     ( (uint64) 1 << 21 )

// LambdaBody states
#define USED ( 1<< 0 )
// ListObject states
#define NEW_COPY ( (uint64) 1 << 6 )
#define MULTI_USE ( (uint64) 1 << 7 )

// CharSet types
#define CHAR_DELIMITER  ( (uint64) 1 << 0 )
#define CHAR_BLANK      ( (uint64) 1 << 1 )
#define CHAR_ALPHA      ( (uint64) 1 << 2 )
#define CHAR_DIGIT10 ( (uint64) 1 << 3 )
#define CHAR_DIGIT16 ( (uint64) 1 << 4 )
#define CHAR_LETTER     ( (uint64) 1 << 5 )
#define CHAR_PRINT      ( (uint64) 1 << 6 )

#define _CharSet_IsDelimiter( cset, ch ) cset [ ch ]
#define _CharSet_IsPrint( ch ) CharTable_IsCharType ( ch, CHAR_PRINT )

// OpenVmTil State defines
#define OVT_IN_USEFUL_DIRECTORY     ( (uint64) 1 << 0 )
#define OVT_PAUSE                   ( (uint64) 1 << 1 )
#define OVT_THROW                   ( (uint64) 1 << 2 )
#define OVT_FRC                   ( (uint64) 1 << 3 )

// tree node states
// #define GONE_DEPTH 
#define TREE_VISITED ( (uint64) 1 << 0 )

// optimize types
#define OP 1
#define SUBSTITUTE 2

// Source Code Node (SCN) dobject slot indices
#define SCN_T_WORD  0
//#define SCN_SC_CADDRESS SCN_T_WORD
#define SCN_SC_WORD_INDEX 1
#define SCN_IN_USE_FLAG 2
#define SCN_LAST_SLOT SCN_IN_USE_FLAG
#define SCN_NUMBER_OF_SLOTS (SCN_LAST_SLOT + 1) //1 : remember - 0 indexed array
#define SCN_NOT_IN_USE 0
#define SCN_IN_USE_FOR_OPTIMIZATION 1
#define SCN_IN_USE_FOR_SOURCE_CODE 2 
#define SCN_IN_USE_FLAG_ALL (SCN_IN_USE_FOR_OPTIMIZATION|SCN_IN_USE_FOR_SOURCE_CODE)

#define PP_SKIP 0
#define PP_INTERPRET 1
#define PP_INTERP PP_INTERPRET
#define PP_ELIF 2
#define PP_ELSE 1
#define PP_IFDEF 3

#define CPU_SAVED    ( (uint64) 1 << 0 )
#define CPU_SELECTED_SAVED ( (uint64) 1 << 1 )
#define FULL_CONTEXT_SIZE ( 10 * K ) //( sizeof (Context) + sizeof (System) + sizeof (Interpreter) + sizeof ( Lexer ) + sizeof (Compiler) + sizeof ( Finder ) + sizeof (ReadLiner))

#define optInfo_0_zero  optInfo->COIW[0]
#define optInfo_0_one   optInfo->COIW[1]
#define optInfo_0_two   optInfo->COIW[2]
#define optInfo_0_three optInfo->COIW[3]    
#define optInfo_0_four  optInfo->COIW[4]
#define optInfo_0_five  optInfo->COIW[5]
#define optInfo_0_six   optInfo->COIW[6]

// CpuPipelineState
#define ONE_REG_ARG             ( (uint8) 1 << 0 )
#define TWO_REG_ARGS            ( (uint8) 1 << 1 )
#define ONE_STACK_ARG           ( (uint8) 1 << 2 )
#define TWO_STACK_ARGS          ( (uint8) 1 << 3 )
#define ONE_REG_ONE_STACK_ARG   ( (uint8) 1 << 4 )
#define ONE_STACK_ONE_REG_ARG   ( (uint8) 1 << 5 )

#define T_WORD                  ( (uint8) 1 << 0 )
#define T_PREPROCESSOR          ( (uint8) 1 << 1 )   

// Interpreter Infix Module State or input type
#define IMS_INIT            ( (uint8) 1 << 0 )
#define IMS_LHS            ( (uint8) 1 << 1 )
#define IMS_EQUAL       ( (uint8) 1 << 2 )
#define IMS_OPERAND   ( (uint8) 1 << 3 )
#define IMS_OP              ( (uint8) 1 << 4 )
#define IMS_INC_DEC     ( (uint8) 1 << 5 )
#define IMS_VARIABLE   ( (uint8) 1 << 6 )
#define IMS_FUNC          ( (uint8) 1 << 7 )
#define IMS_LPAREN        ( (uint8) 1 << 8 )
#define IMS_RPAREN        ( (uint8) 1 << 9 )
#define IMS_LITERAL      ( (uint8) 1 << 10 )
#define IMS_OP_EQUAL    ( (uint8) 1 << 11 )
#define IMS_OBJECT    ( (uint8) 1 << 12 )

#if 0
// abstract types - 't'
// combinators
#define ARROW               ( (uint64) 1 << 0 ) // MORPHISM : t '->' t 
#define CATESIAN_PRODUCT    ( (uint64) 1 << 1 ) // t 'x' t
#define CARTESIAN_PRODUCT             CATESIAN_PRODUCT // t 'x' t
// base types
#define INT64             ( (uint64) 1 << 2 ) 
#define INT               INT64 
#define BOOLEAN           ( (uint64) 1 << 3 ) 
#define STRING            ( (uint64) 1 << 4 ) 
#define BIGNUM            ( (uint64) 1 << 5 ) 
#define FLOAT             ( (uint64) 1 << 6 ) 

// insn op codes for CATEGORY_OP words
#define OP_PLUS_PLUS                ( (uint64) 1 << 0 )
#define OP_MINUS_MINUS              ( (uint64) 1 << 0 )
#define OP_LOGICAL_NOT              ( (uint64) 1 << 1 )
#define OP_EQUAL                    ( (uint64) 1 << 2 ) 
#define OP_LOGICAL_OR               ( (uint64) 1 << 3 ) 
#define OP_LOGICAL_XOR              ( (uint64) 1 << 4 ) 
#define OP_LOGICAL_AND              ( (uint64) 1 << 5 ) 
#define OP_LESS_THAN                ( (uint64) 1 << 6 ) 
#define OP_GREATER_THAN             ( (uint64) 1 << 7 ) 
#define OP_EQUALS                   ( (uint64) 1 << 8 ) 
#define OP_DOES_NOT_EQUAL           ( (uint64) 1 << 9 ) 
#define OP_LESS_THAN_OR_EQUAL       ( (uint64) 1 << 10 ) 
#define OP_GREATER_THAN_OR_EQUAL    ( (uint64) 1 << 11 ) 
#define OP_PLUS                     ( (uint64) 1 << 12 ) 
#define OP_MINUS                    ( (uint64) 1 << 13 ) 
#define OP_PLUS_EQUAL               ( (uint64) 1 << 14 ) 
#define OP_MINUS_EQUAL              ( (uint64) 1 << 15 ) 
#define OP_MULTIPLY                 ( (uint64) 1 << 16 ) 
#define OP_DIVIDE                   ( (uint64) 1 << 17 ) 
#define OP_MOD                      ( (uint64) 1 << 18 ) 
#define OP_MULTIPLY_EQUAL           ( (uint64) 1 << 19 ) 
#define OP_DIVIDE_EQUAL             ( (uint64) 1 << 20 ) 
#define OP_SHIFT_LEFT               ( (uint64) 1 << 21 ) 
#define OP_SHIFT_RIGHT              ( (uint64) 1 << 22 ) 
#define OP_BITWISE_OR               ( (uint64) 1 << 23 ) 
#define OP_BITWISE_AND              ( (uint64) 1 << 24 ) 
#define OP_BITWISE_XOR              ( (uint64) 1 << 25 ) 
#define OP_BITWISE_NOT              ( (uint64) 1 << 26 ) 
#define OP_BITWISE_NEG              ( (uint64) 1 << 27 ) 
#define OP_SHIFT_LEFT_EQUAL         ( (uint64) 1 << 28 ) 
#define OP_SHIFT_RIGHT_EQUAL        ( (uint64) 1 << 29 ) 
#define OP_BITWISE_AND_EQUAL        ( (uint64) 1 << 30 ) 
#define OP_BITWISE_OR_EQUAL         ( (uint64) 1 << 31 ) 
#define OP_PEEK                     ( (uint64) 1 << 32 ) 
#define OP_STORE                    ( (uint64) 1 << 33 ) 
#define OP_POKE                     ( (uint64) 1 << 34 ) 

#define OP_LC 1 // literal/constant
#define OP_VAR 2 
#define OP_FETCH 3 
#define OP_STORE 4 
//#define OP_CPRIMITIVE 4  
#define OP_ORDERED 5 
#define OP_UNORDERED 6 
#define OP_LOGIC 7 
#define OP_1_ARG 8 
//#define OP_RECURSE 9 
#define OP_EQUAL 9 
#define OP_DUP 10  // stack or local var
#define OP_DIVIDE 11
#define OP_OBJECT 12  
#define OP_STACK 13  
#define OP_C_RETURN 14
//#define OP_STACKING 17
#define OP_OPEQUAL 15 
// #define O_BITS 4 // bits needed to encode category codes
#define O_BITS 4 // experiment with 64 bit encoding 
#define DONT_PUSH_R8 (( byte* ) 1 )
#define PUSH_R8 (( byte* ) 2 )
#endif
#define STACK_PUSH_REGISTER_CODE_SIZE 7 // bytes
#define STORE 17 // basically randomly chosen
#define POKE 18
#define EQUAL POKE


