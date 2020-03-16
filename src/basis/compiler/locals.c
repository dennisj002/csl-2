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
Compiler_LocalWord_UpdateCompiler ( Compiler * compiler, Word * word, int64 objectType )
{
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
            if ( _CSL_->SC_Word ) _CSL_->SC_Word->W_NumberOfNonRegisterLocals ++ ;
        }
        else
        {
            word->Index = ++ compiler->NumberOfArgs ;
            compiler->NumberOfNonRegisterArgs ++ ;
            if ( _CSL_->SC_Word ) _CSL_->SC_Word->W_NumberOfNonRegisterArgs ++ ;
        }
    }
    word->W_ObjectAttributes |= RECYCLABLE_LOCAL ;
    return word ;
}

Word *
_Compiler_LocalWord ( Compiler * compiler, byte * name, int64 morphismType, int64 objectType, int64 lispType, int64 allocType )
{
    Word *word = _DObject_New ( name, 0, ( morphismType | IMMEDIATE ), objectType, lispType, LOCAL_VARIABLE, ( byte* ) _DataObject_Run, 0, 1, 0, allocType ) ;
    Compiler_LocalWord_UpdateCompiler ( compiler, word, objectType ) ;
    return word ;
}

Namespace *
Compiler_LocalsNamespace_New ( Compiler * compiler )
{
    Namespace * ns = Namespace_FindOrNew_Local ( compiler->LocalsCompilingNamespacesStack, 1 ) ;
    //_CSL_Namespace_InNamespaceSet ( ns ) ; // done by Namespace_FindOrNew_Local
    Finder_SetQualifyingNamespace ( _Finder_, ns ) ;
    return ns ;
}

Word *
Compiler_LocalWord ( Compiler * compiler, byte * name, int64 morphismAttributes, int64 objectAttributes, int64 lispAttributes, int64 allocType )
{
    if ( ( ! GetState ( compiler, DOING_C_TYPE ) && ( ! GetState ( _LC_, LC_BLOCK_COMPILE ) ) ) ) compiler->LocalsNamespace = Compiler_LocalsNamespace_New ( compiler ) ;
    Word * word = _Compiler_LocalWord ( compiler, name, morphismAttributes, objectAttributes, lispAttributes, allocType ) ;
    //if (compiler->LocalsNamespace) Namespace_DoAddWord ( compiler->LocalsNamespace, word ) ;
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
            Compile_GetVarLitObj_RValue_To_Reg ( compiler->ReturnVariableWord, ACC, 0 ) ; // need to copy because ReturnVariableWord may have been used within the word already
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
        //_Compile_LEA ( DSP, FP, 0, - ( parameterVarsSubAmount ? parameterVarsSubAmount + CELL : CELL ) ) ; // restore sp - release locals stack frame
        _Compile_Move_StackN_To_Reg ( FP, DSP, 1 ) ; // restore the saved pre fp - cf AddLocalsFrame
    }
#if 1    
    if ( ( parameterVarsSubAmount > 0 ) && ( ! IsWordRecursive ) ) Compile_SUBI ( REG, DSP, 0, parameterVarsSubAmount, 0 ) ; // remove stack variables
    else
        if ( ! already )
    {
        // add a place on the stack for return value
        if ( parameterVarsSubAmount < 0 ) Compile_ADDI ( REG, DSP, 0, abs ( parameterVarsSubAmount ), 0 ) ;
        else if ( returnValueFlag && ( ! compiler->NumberOfNonRegisterArgs ) && ( parameterVarsSubAmount == 0 ) // && 
            && ( ! compiler->NumberOfArgs ) )
            Compile_ADDI ( REG, DSP, 0, CELL, 0 ) ;
    }
#endif    
    // nb : stack was already adjusted accordingly for this above by reducing the SUBI subAmount or adding if there weren't any parameter variables
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

// old docs :
// parse local variable notation to a temporary "_locals_" namespace
// calculate necessary frame size
// the stack frame (Fsp) will be just above TOS -- at higer addresses
// save entry Dsp in a CSL variable (or at Fsp [ 0 ]). Dsp will be reset to just
// above the framestack during duration of the function and at the end of the function
// will be reset to its original value on entry stored in the CSL variable (Fsp [ 0 ])
// so that DataStack pushes and pops during the function will be accessing above the top of the new Fsp
// initialize the words to access a slot in the framesize so that the
// compiler can use the slot number in the function being compiled
// compile a local variable such that when used at runtime it pushes
// the slot address on the DataStack
Namespace *
_CSL_Parse_LocalsAndStackVariables ( int64 svf, int64 lispMode, ListObject * args, Stack * nsStack, Namespace * localsNs ) // stack variables flag
{
    // number of stack variables, number of locals, stack variable flag
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    Lexer * lexer = cntx->Lexer0 ;
    Finder * finder = cntx->Finder0 ;
    int64 scm = IsSourceCodeOn ;
    byte * svDelimiters = lexer->TokenDelimiters ;
    Word * word ;
    int64 objectAttributes = 0, lispAttributes = 0, numberOfRegisterVariables = 0, numberOfVariables = 0 ;
    int64 svff = 0, addWords, getReturn = 0, getReturnFlag = 0, regToUseIndex = 0 ;
    Boolean regFlag = false ;
    byte *token, *returnVariable = 0 ;
    Namespace *typeNamespace = 0, *objectTypeNamespace = 0, *saveInNs = _CSL_->InNamespace ;
    //CSL_DbgSourceCodeOff ( ) ;
    if ( ! CompileMode ) Compiler_Init ( compiler, 0 ) ;

    if ( svf ) svff = 1 ;
    addWords = 1 ;
    if ( lispMode ) args = ( ListObject * ) args->Lo_List->Head ;

    while ( ( lispMode ? ( int64 ) _LO_Next ( args ) : 1 ) )
    {
        if ( lispMode )
        {
            args = _LO_Next ( args ) ;
            if ( args->W_LispAttributes & ( LIST | LIST_NODE ) ) args = _LO_First ( args ) ;
            token = ( byte* ) args->Lo_Name ;
            CSL_AddStringToSourceCode ( _CSL_, token ) ;
        }
        else token = _Lexer_ReadToken ( lexer, ( byte* ) " ,\n\r\t" ) ;
        if ( token )
        {
            if ( String_Equal ( token, "(" ) ) continue ;
            if ( String_Equal ( ( char* ) token, "|" ) )
            {
                svff = 0 ; // set stack variable flag to off -- no more stack variables ; begin local variables
                continue ; // don't add a node to our temporary list for this token
            }
            if ( String_Equal ( ( char* ) token, "-t" ) ) // for setting W_TypeSignatureString
            {
                if ( lispMode )
                {
                    args = _LO_Next ( args ) ;
                    if ( args->W_LispAttributes & ( LIST | LIST_NODE ) ) args = _LO_First ( args ) ;
                    token = ( byte* ) args->Lo_Name ;
                    CSL_AddStringToSourceCode ( _CSL_, token ) ;
                }
                else token = _Lexer_LexNextToken_WithDelimiters ( lexer, 0, 1, 0, 1, LEXER_ALLOW_DOT ) ;
                strncpy ( ( char* ) _Context_->CurrentWordBeingCompiled->W_TypeSignatureString, ( char* ) token, 8 ) ;
                continue ; // don't add a node to our temporary list for this token
            }
            if ( String_Equal ( ( char* ) token, "--" ) ) // || ( String_Equal ( ( char* ) token, "|-" ) == 0 ) || ( String_Equal ( ( char* ) token, "|--" ) == 0 ) )
            {
                if ( ! svf ) break ;
                else
                {
                    addWords = 0 ;
                    getReturnFlag = 1 ;
                    continue ;
                }
            }
            if ( String_Equal ( ( char* ) token, ")" ) ) break ;
            if ( String_Equal ( ( char* ) token, "REG" ) || String_Equal ( ( char* ) token, "REGISTER" ) )
            {
                if ( GetState ( _CSL_, OPTIMIZE_ON ) ) regFlag = true ;
                continue ;
            }
            if ( ( ! GetState ( _Context_, C_SYNTAX ) ) && ( String_Equal ( ( char* ) token, "{" ) ) || ( String_Equal ( ( char* ) token, ";" ) ) )
            {
                //_Printf ( ( byte* ) "\nLocal variables syntax error : no closing parenthesis ')' found" ) ;
                CSL_Exception ( SYNTAX_ERROR, "\nLocal variables syntax error : no closing parenthesis ')' found", 1 ) ;
                break ;
            }
            if ( ! lispMode )
            {
                word = Finder_Word_FindUsing ( finder, token, 1 ) ; // 1: saveQns ?? find after Literal - eliminate making strings or numbers words ??
                if ( word && ( word->W_ObjectAttributes & ( NAMESPACE | CLASS ) ) && ( CharTable_IsCharType ( ReadLine_PeekNextChar ( lexer->ReadLiner0 ), CHAR_ALPHA ) ) )
                {
                    if ( word->W_ObjectAttributes & STRUCTURE ) objectTypeNamespace = word ;
                    else typeNamespace = word ;
                    continue ;
                }
            }
            if ( getReturnFlag )
            {
                addWords = 0 ;
                if ( Stringi_Equal ( token, ( byte* ) "ACC" ) ) getReturn |= RETURN_ACCUM ;
                else if ( Stringi_Equal ( token, ( byte* ) "EAX" ) ) getReturn |= RETURN_ACCUM ;
                else if ( Stringi_Equal ( token, ( byte* ) "RAX" ) ) getReturn |= RETURN_ACCUM ;
                else if ( Stringi_Equal ( token, ( byte* ) "TOS" ) ) getReturn |= RETURN_TOS ;
                else returnVariable = token ; //nb. if there is a return variable it must have already been declared as a parameter of local variable else it is an error
                continue ;
            }
            if ( addWords )
            {
                if ( ! localsNs ) localsNs = Namespace_FindOrNew_Local ( nsStack ? nsStack : compiler->LocalsCompilingNamespacesStack, 1 ) ;
                if ( svff )
                {
                    objectAttributes |= PARAMETER_VARIABLE ; // aka an arg
                    if ( lispMode ) lispAttributes |= T_LISP_SYMBOL ; // no ltype yet for _CSL_LocalWord
                }
                else
                {
                    objectAttributes |= LOCAL_VARIABLE ;
                    if ( lispMode ) lispAttributes |= T_LISP_SYMBOL ; // no ltype yet for _CSL_LocalWord
                }
                if ( regFlag == true )
                {
                    objectAttributes |= REGISTER_VARIABLE ;
                    numberOfRegisterVariables ++ ;
                }
                word = DataObject_New ( objectAttributes, 0, token, 0, objectAttributes, lispAttributes, 0, 0, 0, DICTIONARY, - 1, - 1 ) ;
                if ( _Context_->CurrentWordBeingCompiled ) _Context_->CurrentWordBeingCompiled->W_TypeSignatureString [numberOfVariables ++] = '_' ;
                if ( regFlag == true )
                {
                    word->RegToUse = RegParameterOrder ( regToUseIndex ++ ) ;
                    if ( word->W_ObjectAttributes & PARAMETER_VARIABLE )
                    {
                        if ( ! compiler->RegisterParameterList ) compiler->RegisterParameterList = _dllist_New ( TEMPORARY ) ;
                        _List_PushNew_ForWordList ( compiler->RegisterParameterList, word, 1 ) ;
                    }
                    regFlag = false ;
                }
                if ( objectTypeNamespace )
                {
                    Compiler_TypedObjectInit ( word, objectTypeNamespace ) ;
                    Word_TypeChecking_SetSigInfoForAnObject ( word ) ;
                }
                else if ( typeNamespace ) word->ObjectByteSize = typeNamespace->ObjectByteSize ;
                if ( String_Equal ( token, "this" ) ) word->W_ObjectAttributes |= THIS ;
                typeNamespace = 0 ;
                objectTypeNamespace = 0 ;
                objectAttributes = 0 ;
            }
        }
        else return 0 ; // Syntax Error or no local or parameter variables
    }
    compiler->State |= getReturn ;

    // we support nested locals and may have locals in other blocks so the indices are cumulative
    if ( numberOfRegisterVariables ) Compile_Init_LocalRegisterParamenterVariables ( compiler ) ;
    if ( returnVariable ) compiler->ReturnVariableWord = _Finder_FindWord_InOneNamespace ( cntx->Finder0, localsNs, returnVariable ) ;

    finder->FoundWord = 0 ;
    Lexer_SetTokenDelimiters ( lexer, svDelimiters, COMPILER_TEMP ) ;
    SetState ( compiler, VARIABLE_FRAME, true ) ;
    SetState ( _CSL_, DEBUG_SOURCE_CODE_MODE, scm ) ;
    _CSL_->InNamespace = saveInNs ;
    compiler->LocalsNamespace = localsNs ;
    Qid_Save_Set_InNamespace ( localsNs ) ;
    return localsNs ;
}
