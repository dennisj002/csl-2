#include "../../include/csl.h"

// ?? is the frame pointer needed ?? 
// remember LocalsStack is not pushed or popped so ...

/* ------------------------------------------------------
 *     a Locals Stack Frame on the DataStack - referenced by DSP
 * ------------------------------------------------------
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *     higher memory adresses -- nb. reverse from intel push/pop where push moves esp to a lower memory address and pop moves esp to a higher memory address. This seemed more intuitive.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * s <------------------------< new DSP - dsp [ n+1|n+2]   --- used by the new function
 * t           "saved esp"        slot n+1 fp [ n+1]   --- with 'return' function
 * a    "local variable n"        slot n   fp [  n ]
 * c    "local variable 3"        slot 3   fp [  3 ]
 * k    "local variable 2"        slot 2   fp [  2 ]
 *      "local variable 1"        slot 1   fp [  1 ]
 * f  -------------------------
 * r
 * a    saved pre fp = r15 //edi           fp [  0 ]  <--- new fp - FP points here > == r15[0] //edi[0] <pre dsp [ 1] --->
 * m                                                  <--- stack frame size = number of locals + 1 (fp) + 1 (esp)  --->
 * e <-----------------------------------------------
 *      "parameter variable"      slot 1   fp [ -1 ]   --- already on the "locals function" incoming stack     <pre dsp [ 0] == pre esi [0]> 
 *      "parameter variable"      slot 2   fp [ -2 ]   --- already on the "locals function" incoming stack     <pre dsp [-1]>
 *      "parameter variable"      slot x   fp [-etc]   --- already on the "locals function" incoming stack     <pre dsp [-2]>
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *     lower memory addresses  on DataStack - referenced by DSP
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                      notations : fp = FP = Fp   = R15 //32 bit = EDI 
 *                                  sp = DSP = Dsp = R14 //32 bit = ESI
 */

int64
_ParameterVar_Offset ( Word * word, int64 numberOfArgs, Boolean frameFlag )
{
    int64 offset ;
    offset = - ( numberOfArgs - word->Index + ( frameFlag ? 1 : 0 ) ) ; // is this totally correct??
    return offset ;
}

int64
Compiler_ParameterVar_Offset ( Compiler * compiler, Word * word )
{
    int64 offset ;
    offset = _ParameterVar_Offset ( word, compiler->NumberOfArgs, Compiler_IsFrameNecessary ( compiler ) ) ;
    return offset ;
}

inline int64
LocalVar_FpOffset ( Word * word )
{
    return word->Index ;
}

inline int64
LocalVar_Disp ( Word * word )
{
    return ( word->Index * CELL ) ;
}

inline int64
ParameterVar_Disp ( Word * word )
{
    return ( Compiler_ParameterVar_Offset ( _Compiler_, word ) * CELL ) ;
}

inline int64
_LocalOrParameterVar_Offset ( Word * word, int64 numberOfArgs, Boolean frameFlag )
{
    return ( ( word->W_ObjectAttributes & LOCAL_VARIABLE ) ? ( LocalVar_FpOffset ( word ) ) : ( _ParameterVar_Offset ( word, numberOfArgs, frameFlag ) ) ) ;
}

inline int64
LocalOrParameterVar_Offset ( Word * word )
{
    //return ( ( word->CAttribute & LOCAL_VARIABLE ) ? ( LocalVar_FpOffset ( word ) ) : ( ParameterVar_Offset ( word ) ) ) ;
    return _LocalOrParameterVar_Offset ( word, _Compiler_->NumberOfArgs, Compiler_IsFrameNecessary ( _Compiler_ ) ) ;
}

inline int64
LocalOrParameterVar_Disp ( Word * word )
{
    int64 offset = LocalOrParameterVar_Offset ( word ) ;
    return ( offset * CELL_SIZE ) ;
}

Word *
_Compiler_LocalWord ( Compiler * compiler, byte * name, int64 morphismType, int64 objectType, int64 lispType, int64 allocType )
{
    Word *word = _DObject_New ( name, 0, ( morphismType | IMMEDIATE ), objectType, lispType, LOCAL_VARIABLE, ( byte* ) _DataObject_Run, 0, 1, 0, allocType ) ;
    compiler->NumberOfVariables ++ ;
    if ( objectType & REGISTER_VARIABLE )
    {
        compiler->NumberOfRegisterVariables ++ ;
        if ( objectType & LOCAL_VARIABLE )
        {
            word->Index = ++ compiler->NumberOfRegisterLocals ;
            ++ compiler->NumberOfLocals ;
        }
        else
        {
            word->Index = ++ compiler->NumberOfArgs ;
            ++ compiler->NumberOfRegisterArgs ;
        }
    }
    else
    {
        compiler->NumberOfNonRegisterVariables ++ ;
        if ( objectType & LOCAL_VARIABLE )
        {
            word->Index = ++ compiler->NumberOfNonRegisterLocals ;
            compiler->NumberOfLocals ++ ;
            _CSL_->SC_Word->W_NumberOfNonRegisterLocals ++ ;
        }
        else
        {
            word->Index = ++ compiler->NumberOfArgs ;
            compiler->NumberOfNonRegisterArgs ++ ;
            _CSL_->SC_Word->W_NumberOfNonRegisterArgs ++ ;
        }
    }
    word->W_ObjectAttributes |= RECYCLABLE_LOCAL ;
    //CSL->SC_Word->W_NumberOfNonRegisterArgs += compiler->NumberOfNonRegisterArgs ; // debugOutput.c showLocals need this others ?
    //CSL->SC_Word->W_NumberOfNonRegisterLocals += compiler->NumberOfNonRegisterLocals ;
    return word ;
}

void
Compiler_LocalsNamespace_New ( Compiler * compiler )
{
    Namespace * ns = Namespace_FindOrNew_Local ( compiler->LocalsCompilingNamespacesStack, 1 ) ;
    Finder_SetQualifyingNamespace ( _Finder_, ns ) ;
}

Word *
Compiler_LocalWord ( Compiler * compiler, byte * name, int64 morphismAttributes, int64 objectAttributes, int64 lispAttributes, int64 allocType )
{
    if ( ( ! GetState ( compiler, DOING_C_TYPE ) && ( ! GetState ( _LC_, LC_BLOCK_COMPILE ) ) ) ) Compiler_LocalsNamespace_New ( compiler ) ;
    Word * word = _Compiler_LocalWord ( compiler, name, morphismAttributes, objectAttributes, lispAttributes, allocType ) ;
    return word ;
}

//nb. correct only if Compiling !!

inline Boolean
IsFrameNecessary ( int64 numberOfNonRegisterLocals, int64 numberOfNonRegisterArgs )
{
    return ( ( numberOfNonRegisterLocals || numberOfNonRegisterArgs ) ? true : false ) ;
}

inline Boolean
Compiler_IsFrameNecessary ( Compiler * compiler )
{
    //return ( ( compiler->NumberOfNonRegisterLocals || compiler->NumberOfNonRegisterArgs ) ? true : false ) ;
    return IsFrameNecessary ( compiler->NumberOfNonRegisterLocals, compiler->NumberOfNonRegisterArgs ) ;
}

void
Compile_Init_LocalRegisterParamenterVariables ( Compiler * compiler )
{
    Compiler_WordStack_SCHCPUSCA ( 0, 0 ) ;
    dllist * list = compiler->RegisterParameterList ;
    dlnode * node ;
    Boolean frameFlag = Compiler_IsFrameNecessary ( compiler ) ; // compiler->NumberOfNonRegisterLocals ; 
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = dlnode_Next ( node ) ) //, i -- )
    {
        Word * word = ( Word* ) dobject_Get_M_Slot ( ( dobject* ) node, SCN_T_WORD ) ;
        _Compile_Move_StackN_To_Reg ( word->RegToUse, ( frameFlag ? FP : DSP ), Compiler_ParameterVar_Offset ( compiler, word ) ) ;
    }
}

void
_Compiler_AddLocalFrame ( Compiler * compiler )
{
    Compiler_WordStack_SCHCPUSCA ( 0, 0 ) ;
    _Compile_Move_Reg_To_StackN ( DSP, 1, FP ) ; // save pre fp
    _Compile_LEA ( FP, DSP, 0, CELL ) ; // set new fp
    Compile_ADDI ( REG, DSP, 0, 1 * CELL, INT32_SIZE ) ; // 1 : fp - add stack frame -- this value is going to be reset 
    compiler->FrameSizeCellOffset = ( int64* ) ( Here - INT32_SIZE ) ; // in case we have to add to the framesize with nested locals
    d0 ( if ( Is_DebugOn ) Compile_Call_TestRSP ( ( byte* ) _CSL_Debugger_Locals_Show ) ) ;
}

void
Compiler_SetLocalsFrameSize_AtItsCellOffset ( Compiler * compiler )
{
    int64 size = compiler->NumberOfNonRegisterLocals ;
    int64 fsize = compiler->LocalsFrameSize = ( ( ( size <= 0 ? 0 : size ) + 1 ) * CELL ) ; //1 : the frame pointer 
    if ( fsize ) *( ( int32* ) ( compiler->FrameSizeCellOffset ) ) = fsize ; //compiler->LocalsFrameSize ; //+ ( IsSourceCodeOn ? 8 : 0 ) ;
}

// Compiler_RemoveLocalFrame : the logic definitely needs to be simplified???
// Compiler_RemoveLocalFrame has "organically evolved" it need to be logically simplified??

void
Compiler_RemoveLocalFrame ( Compiler * compiler )
{
    if ( ! GetState ( _Compiler_, LISP_MODE ) ) Compiler_WordStack_SCHCPUSCA ( 0, 0 ) ;
    int64 parameterVarsSubAmount = 0 ;
    Boolean returnValueFlag, already = false ;
    returnValueFlag = ( Context_CurrentWord ( )->W_MorphismAttributes & C_RETURN ) 
        || ( GetState ( compiler, RETURN_TOS | RETURN_ACCUM ) ) || compiler->ReturnVariableWord ;
    if ( compiler->NumberOfArgs ) parameterVarsSubAmount = ( compiler->NumberOfArgs - returnValueFlag ) * CELL ;
    // nb. these variables have no lasting lvalue - they exist on the stack - we can only return their rvalue
    if ( compiler->ReturnVariableWord )
    {
        if ( ! ( compiler->ReturnVariableWord->W_ObjectAttributes & REGISTER_VARIABLE ) ) 
            Compile_GetVarLitObj_RValue_To_Reg (compiler->ReturnVariableWord, ACC , 0) ; // need to copy because ReturnVariableWord may have been used within the word already
    }
    else if ( GetState ( compiler, RETURN_TOS ) || ( compiler->NumberOfNonRegisterArgs && returnValueFlag && ( ! GetState ( compiler, RETURN_ACCUM ) ) ) )
    {
        Word * one = 0 ;
        if ( ! GetState ( compiler, LISP_MODE ) ) one = WordStack ( 1 ) ;
        if ( one && one->StackPushRegisterCode )
        {
            already = true ;
            SetHere ( one->StackPushRegisterCode, 1 ) ;
        }
        else
        {
            byte mov_r14_rax [] = { 0x49, 0x89, 0x06 } ;
            if ( memcmp ( mov_r14_rax, Here - 3, 3 ) ) 
                Compile_Move_TOS_To_ACCUM ( DSP ) ; // save TOS to ACCUM so we can set return it as TOS below
#if 0   // hasn't occurred yet but maybe useful in some cases especially in connection with the below #else block        
            else
            {
                byte add_r14_0x8 [ ] = { 0x49, 0x83, 0xc6, 0x08 } ;
                if ( ( GetState ( compiler, RETURN_ACCUM ) ) && ( ! ( memcmp ( add_r14_0x8, Here - 7, 4 ) ) ) ) // more logic needed here for this to work ??
                {
                    _ByteArray_UnAppendSpace ( _O_CodeByteArray, 7 ) ;
                }
            }
#endif            
        }
    }
    if ( compiler->NumberOfNonRegisterLocals || compiler->NumberOfNonRegisterArgs )
    {
        // remove the incoming parameters -- like in C
        if ( ! GetState ( _Compiler_, LISP_MODE ) ) Compiler_WordStack_SCHCPUSCA ( 0, 0 ) ;
        _Compile_LEA ( DSP, FP, 0, - CELL ) ; // restore sp - release locals stack frame
        _Compile_Move_StackN_To_Reg ( FP, DSP, 1 ) ; // restore the saved pre fp - cf AddLocalsFrame
    }
    if ( ( parameterVarsSubAmount > 0 ) && ( ! IsWordRecursive ) ) Compile_SUBI ( REG, DSP, 0, parameterVarsSubAmount, 0 ) ; // remove stack variables
    else if ( ! already )
    {
        // add a place on the stack for return value
        if ( parameterVarsSubAmount < 0 ) Compile_ADDI ( REG, DSP, 0, abs ( parameterVarsSubAmount ), 0 ) ;
        else if ( returnValueFlag && ( ! compiler->NumberOfNonRegisterArgs ) && ( parameterVarsSubAmount == 0 )  // && 
            && ( ! compiler->NumberOfArgs ) )
            //( GetState ( compiler, RETURN_TOS ) || compiler->ReturnVariableWord ) ) //&& ( ! IsWordRecursive ) ) 
            //( GetState ( compiler, RETURN_TOS ) || compiler->ReturnVariableWord ) ) //&& ( ! IsWordRecursive ) ) 
            //( returnValueFlag ) && ( ! IsWordRecursive ) ) 
            Compile_ADDI ( REG, DSP, 0, CELL, 0 ) ;
    }
    // nb : stack was already adjusted accordingly for this above by reducing the SUBI subAmount or adding if there weren't any parameter variables
#if 1       
    if ( returnValueFlag || ( IsWordRecursive && ( ! GetState ( compiler, RETURN_ACCUM ) ) ) )
    {
        if ( compiler->ReturnVariableWord )
        {
            Compiler_Word_SCHCPUSCA ( compiler->ReturnVariableWord, 0 ) ;
            if ( compiler->ReturnVariableWord->W_ObjectAttributes & REGISTER_VARIABLE )
            {
                _Compile_Move_Reg_To_StackN ( DSP, 0, compiler->ReturnVariableWord->RegToUse ) ;
                return ;
            }
        }
        Compile_Move_ACC_To_TOS ( DSP ) ;
    }
#else // something like this seem more understandable and extendable than what is working now ...
    if ( returnValueFlag ) //|| ( IsWordRecursive && ( ! GetState ( compiler, RETURN_ACCUM ) ) ) )
    {
        if ( compiler->ReturnVariableWord )
        {
            Compiler_Word_SCHCPUSCA ( compiler->ReturnVariableWord, 0 ) ;
            if ( compiler->ReturnVariableWord->W_ObjectAttributes & REGISTER_VARIABLE )
            {
                _Compile_Stack_PushReg ( DSP, compiler->ReturnVariableWord->RegToUse ) ;
                return ;
            }
        }
        _Compile_Stack_PushReg ( DSP, ACC ) ;

    }
#endif    
    d0 ( if ( Is_DebugOn ) Compile_Call_TestRSP ( ( byte* ) _CSL_Debugger_Locals_Show ) ) ;
}

void
CSL_LocalsAndStackVariablesBegin ( )
{
    _CSL_Parse_LocalsAndStackVariables ( 1, 0, 0, 0, 0 ) ;
}

void
CSL_LocalVariablesBegin ( )
{
    _CSL_Parse_LocalsAndStackVariables ( 0, 0, 0, 0, 0 ) ;
}

