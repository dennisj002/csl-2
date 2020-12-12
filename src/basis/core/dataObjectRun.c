
#include "../../include/csl.h"

// rvalue - rhs value - right hand side of '=' - the actual value, used on the right hand side of C statements

// we run all new objects thru here ; good for debugging and understanding 

Boolean
Word_ObjectRun_Finish ( Word * word )
{
    Context * cntx = _Context_ ;
    if ( word->W_ObjectAttributes & ( NAMESPACE | CLASS | CLASS_CLONE ) ) Namespace_Do_Namespace ( word ) ; // namespaces are the only word that needs to run the word set DObject Compile_SetCurrentlyRunningWord_Call_TestRSP created word ??
    Context_ClearState ( cntx ) ;
}

void
Word_ObjectRun ( Word * word )
{
    Context * cntx = _Context_ ;
    cntx->Interpreter0->w_Word = word ;
    Compiler_Word_SCHCPUSCA ( word, 0 ) ;
    if ( word->W_ObjectAttributes & ( LITERAL | CONSTANT ) ) CSL_Do_LiteralWord ( word ) ;
    else
    {
        Context_GetState ( cntx, word ) ;

        if ( word->W_ObjectAttributes & LOCAL_OBJECT ) CSL_Do_LocalObject ( word) ;
        else if ( ( word->W_LispAttributes & T_LISP_SYMBOL ) || ( word->W_ObjectAttributes & T_LISP_SYMBOL ) ) //lambda variables are parsed as CAttribute & T_LISP_SYMBOL
        {
            if ( ! GetState ( cntx, LC_CSL ) ) CSL_Do_LispSymbol ( word ) ;
            else CSL_Do_Variable ( word) ;
        }
        else if ( word->W_ObjectAttributes & DOBJECT ) CSL_Do_DynamicObject ( word, ACC ) ;
        else if ( word->W_ObjectAttributes & OBJECT_FIELD ) CSL_Do_ClassField ( word) ;
        else if ( word->W_ObjectAttributes & ( STRUCTURE ) ) CSL_Do_Object ( word ) ;
        else if ( word->W_ObjectAttributes & ( THIS | OBJECT | NAMESPACE_VARIABLE | LOCAL_VARIABLE | PARAMETER_VARIABLE ) ) CSL_Do_Variable ( word ) ;
        else if ( word->W_ObjectAttributes & ( C_TYPE | C_CLASS ) ) CSL_Do_C_Type ( word ) ;

        Word_ObjectRun_Finish ( word ) ;
    }
}

void
_DataObject_Run ( Word * word0 )
{
    Context * cntx = _Context_ ;
    Word * word = _Context_CurrentWord ( cntx ) ; // seems we don't need to compile definition code for object type words
    Word_ObjectRun ( word ) ;
}

void
DataObject_Run ( )
{
    Word * word = ( Word * ) DataStack_Pop ( ) ; //_Context_->CurrentlyRunningWord ;
    _DataObject_Run ( word ) ;
}

void
Do_Variable ( Word * word )
{
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    int64 value ;
    if ( ( ! ( word->W_ObjectAttributes & ( STRUCTURE ) ) ) ) //|| ( GetState ( cntx, IS_FORWARD_DOTTED ) ) || (GetState ( cntx, IS_REVERSE_DOTTED ) ) )
    {
        if ( CompileMode ) //&& ( ! GetState ( compiler, LC_ARG_PARSING )))
        {
            if ( GetState ( cntx, ( C_SYNTAX | INFIX_MODE ) ) && ( ! GetState ( cntx, IS_RVALUE ) )
                && ( ! compiler->LHS_Word ) && ( ! GetState ( cntx, IS_FORWARD_DOTTED ) ) ) compiler->LHS_Word = word ;
            _Do_Compile_Variable ( word ) ;
        }
        else
        {
            if ( word->W_ObjectAttributes & ( OBJECT | THIS ) )
            {
                if ( word->W_ObjectAttributes & THIS )
                {
                    if ( GetState ( cntx, IS_RVALUE ) ) value = ( int64 ) word->W_Value ;
                    value = ( int64 ) word->W_PtrToValue ;
                }
                else value = ( int64 ) word->W_Value ;
            }
            else
            {
                if ( GetState ( cntx, ( C_SYNTAX | INFIX_MODE ) ) ) //| C_INFIX_EQUAL ) ) )
                {
                    if ( GetState ( cntx, IS_RVALUE ) ) value = ( int64 ) * word->W_PtrToValue ;
                    else
                    {
                        if ( ! compiler->LHS_Word ) compiler->LHS_Word = word ;
                        goto done ; // LHS_Word : delayed compile by _CSL_C_Infix_EqualOp
                        //else value = ( int64 ) word->W_PtrToValue ;
                    }
                }
                else if ( GetState ( cntx, IS_RVALUE ) ) value = word->W_Value ;
                else value = ( int64 ) word->W_PtrToValue ;
            }
            if ( GetState ( cntx, IS_REVERSE_DOTTED ) && ( cntx->BaseObject && ( cntx->BaseObject != word ) ) // guessing some logic here ??
                && ( ! GetState ( compiler, C_INFIX_EQUAL ) ) && ( ! ( word->W_ObjectAttributes & ( THIS ) ) ) ) TOS = value ; //?? maybe needs more precise state logic
            else DataStack_Push ( value ) ;
        }
done:
        if ( ( word->W_ObjectAttributes & STRUCT ) || GetState ( cntx, IS_FORWARD_DOTTED ) ) Finder_SetQualifyingNamespace ( cntx->Finder0, word->TypeNamespace ) ;
        CSL_TypeStackPush ( word ) ;
    }
}

void
CSL_Do_Object ( Word * word )
{
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    Interpreter * interp = cntx->Interpreter0 ;
    if ( ( word->W_ObjectAttributes & THIS ) || ( ! ( word->W_ObjectAttributes & ( NAMESPACE_VARIABLE ) ) ) )
    {
        if ( word->W_ObjectAttributes & ( OBJECT | THIS | QID ) || GetState ( word, QID ) ) //|| GetState ( cntx, IS_FORWARD_DOTTED ) ) 
        {
            interp->CurrentObjectNamespace = TypeNamespace_Get ( word ) ;
            Compiler_Init_AccumulatedOffsetPointers ( compiler, word ) ;
            if ( word->W_ObjectAttributes & THIS ) word->S_ContainingNamespace = interp->ThisNamespace ;
        }
    }
    if ( ( ! GetState ( compiler, ARRAY_MODE ) ) && ( ! GetState ( cntx, IS_FORWARD_DOTTED ) ) && ( ! GetState ( cntx, IS_REVERSE_DOTTED ) ) && ( ! word->W_ObjectAttributes & LOCAL_OBJECT ) ) cntx->BaseObject = 0 ;
    Do_Variable ( word ) ;
}

void
_Do_LocalObject_AllocateInit ( Namespace * typeNamespace, byte ** value, int64 size )
{
    Word * word = _CSL_ObjectNew ( size, ( byte* ) "<object>", 0, TEMPORARY ) ;
    _Class_Object_Init ( word, typeNamespace ) ;
    * value = ( byte* ) word->W_Value ;
}

void
CSL_Do_LocalObject ( Word * word)
{
    if ( ( word->W_ObjectAttributes & LOCAL_VARIABLE ) && ( ! GetState ( word, W_INITIALIZED ) ) ) // this is a local variable so it is initialed at creation 
    {
        int64 size = _Namespace_VariableValueGet ( word->TypeNamespace, ( byte* ) "size" ) ;
        Compile_MoveImm_To_Reg ( RDI, ( int64 ) word->TypeNamespace, CELL ) ;
        _Compile_LEA ( RSI, FP, 0, LocalVar_Disp ( word ) ) ;
        //_Compile_Move_Rm_To_Reg ( RSI, RSI, 0 ) ;
        Compile_MoveImm_To_Reg ( RDX, ( int64 ) size, CELL ) ;
        Compile_Call_TestRSP ( ( byte* ) _Do_LocalObject_AllocateInit ) ; // we want to only allocate this object once and only at run time; and not at compile time
        SetState ( word, W_INITIALIZED, true ) ;
    }
    CSL_Do_Object ( word ) ;
}

void
_Do_Compile_Variable ( Word * word )
{
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    int64 size = CSL_Get_ObjectByteSize ( word ) ;
    if ( GetState ( cntx, C_SYNTAX | INFIX_MODE ) || GetState ( compiler, LC_ARG_PARSING ) )
    {
        if ( GetState ( cntx, IS_RVALUE ) ) Compile_GetVarLitObj_RValue_To_Reg ( word, ACC, size ) ;
        else
        {
            Compiler_Word_SCHCPUSCA ( word, 1 ) ;
            if ( ( word->W_ObjectAttributes & ( OBJECT | THIS | QID ) ) || GetState ( word, QID ) ) _Compile_GetVarLitObj_LValue_To_Reg ( word, ACC, size ) ;
            else // this compilation is delayed to _CSL_C_Infix_Equal/Op
            {
                Word_SetCodingAndSourceCoding ( word, 0 ) ;
                return ;
            }
        }
    }
    else _Compile_GetVarLitObj_LValue_To_Reg ( word, ACC, size ) ;
    _Word_CompileAndRecord_PushReg ( word, ( word->W_ObjectAttributes & REGISTER_VARIABLE ) ? word->RegToUse : ACC, true ) ;
}

void
CSL_Do_Variable ( Word * word )
{
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    Interpreter * interp = cntx->Interpreter0 ;
    if ( ( word->W_ObjectAttributes & THIS ) || ( ! ( word->W_ObjectAttributes & ( NAMESPACE_VARIABLE ) ) ) )
    {
        if ( word->W_ObjectAttributes & ( OBJECT | THIS | QID ) || GetState ( word, QID ) ) //|| GetState ( cntx, IS_FORWARD_DOTTED ) ) 
        {
            word->AccumulatedOffset = 0 ;
            interp->CurrentObjectNamespace = TypeNamespace_Get ( word ) ;
            Compiler_Init_AccumulatedOffsetPointers ( compiler, word ) ;
            word->W_ObjectAttributes |= OBJECT ;
            if ( word->W_ObjectAttributes & THIS ) word->S_ContainingNamespace = _Context_->Interpreter0->ThisNamespace ;
        }
    }
    if ( ( ! GetState ( compiler, ARRAY_MODE ) ) && ( ! GetState ( cntx, IS_FORWARD_DOTTED ) ) && ( ! GetState ( cntx, IS_REVERSE_DOTTED ) ) )
        cntx->BaseObject = 0 ;
    Do_Variable ( word) ;
}

void
_Do_LiteralValue ( int64 value )
{
    if ( CompileMode )
    {
        Compile_MoveImm_To_Reg ( ACC, value, CELL ) ;
        CSL_CompileAndRecord_PushAccum ( ) ; // does word == top of word stack always
    }
    else DataStack_Push ( value ) ;
}

void
CSL_Do_LiteralWord ( Word * word )
{
    _Do_LiteralValue ( word->W_Value ) ;
    CSL_TypeStackPush ( word ) ;
}

void
Compile_C_FunctionDeclaration ( byte * token1 )
{
    byte * token ;
    _Compiler_->C_FunctionBackgroundNamespace = _Compiler_->C_BackgroundNamespace ;
    SetState ( _Compiler_, C_COMBINATOR_PARSING, true ) ;
    CSL_C_Syntax_On ( ) ;
    Word * word = Word_New ( token1 ) ; // "("
    CSL_WordList_PushWord ( word ) ;
    Compiler_Word_SCHCPUSCA ( word, 1 ) ;
    DataStack_Push ( ( int64 ) word ) ;
    CSL_BeginBlock ( ) ; // nb! before CSL_LocalsAndStackVariablesBegin
    CSL_LocalsAndStackVariablesBegin ( ) ;
    Ovt_AutoVarOn ( ) ;
    do // the rare occurence of any tokens between closing locals right paren ')' and beginning block '}'
    {
        if ( token = Lexer_ReadToken ( _Lexer_ ) )
        {
            if ( String_Equal ( token, "s{" ) )
            {
                Interpreter_InterpretAToken ( _Interpreter_, token, - 1, - 1 ) ;
                break ;
            }
            else if ( token [ 0 ] == '{' ) break ; // take nothing else (would be Syntax Error ) -- we have already done CSL_BeginBlock
            else _Lexer_ConsiderDebugAndCommentTokens ( token, 1 ) ;
        }
    }
    while ( token ) ;
    //CSL_Interpret_C_Blocks ( 1, 1, 0 ) ; // ??? seems like this should be used somewhere here 
    Ovt_AutoVarOff ( ) ;
}

// this is currently kinda rough
// this also interprets the rest of a c function after a type declaration

void
_Compile_C_TypeDeclaration ( )
{
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    byte * token ;
    while ( token = Interpret_C_Until_NotIncluding_Token4 ( cntx->Interpreter0,
        ( byte* ) ",", ( byte* ) ";", ( byte* ) "{", ( GetState ( cntx, C_SYNTAX ) ? ( byte* ) "}" : ( byte* ) "=" ), 0, 0 ) )
    {
        if ( String_Equal ( token, ";" ) )
        {
            Lexer_ReadToken ( _Lexer_ ) ;
            Interpreter_InterpretAToken ( cntx->Interpreter0, token, _Lexer_->TokenStart_ReadLineIndex, _Lexer_->SC_Index ) ;
            break ;
        }
        else if ( String_Equal ( token, "," ) || ( ( ! GetState ( cntx, C_SYNTAX ) ) && String_Equal ( token, "=" ) ) ) Lexer_ReadToken ( _Lexer_ ) ;
        else
        {
            if ( String_Equal ( token, ")" ) && GetState ( compiler, DOING_A_PREFIX_WORD ) ) CSL_PushToken_OnTokenList ( token ) ;
            if ( GetState ( cntx, C_SYNTAX ) ) Compiler_Save_C_BackgroundNamespace ( compiler ) ;

            break ;
        }
        compiler->LHS_Word = 0 ;
    }
}

// nb.Compiling !!

void
Compile_C_TypeDeclaration ( byte * token0 ) //, int64 tsrli, int64 scwi)
{
    // remember : we are not in a C function
    Interpreter * interp = _Interpreter_ ;
    Word * word ;
    byte * token1 ;
    if ( Compiling )
    {
        if ( token0[0] == ')' )
        {
            //  C cast code here ; 
            // nb! : we have no (fully) implemented operations on operand size less than 8 bytes
            token1 = Lexer_ReadToken ( interp->Lexer0 ) ;
            //interp->LastLexedChar = interp->Lexer0->LastLexedChar ;
            if ( token1 )
            {
                Word * word0 = _Interpreter_TokenToWord ( interp, token1, - 1, - 1 ) ;
                word = Compiler_CopyDuplicatesAndPush ( word0, _Lexer_->TokenStart_ReadLineIndex, _Lexer_->SC_Index ) ;
                if ( word )
                {
                    word->ObjectByteSize = CSL_Get_ObjectByteSize ( word ) ;
                    Interpreter_DoWord ( interp, word, - 1, - 1 ) ;
                }
            }
            else SetState ( _Context_->Lexer0, LEXER_END_OF_LINE, true ) ;
        }
        else
        {
            Ovt_AutoVarOn ( ) ;
            Compiler_LocalsNamespace_New ( _Compiler_ ) ;
            word = Lexer_Do_MakeItAutoVar ( _Lexer_, token0, _Lexer_->TokenStart_ReadLineIndex, _Lexer_->SC_Index ) ;
            _Compiler_->LHS_Word = word ;
            Interpreter_DoWord ( interp, word, - 1, - 1 ) ;
            _Compile_C_TypeDeclaration ( ) ;
        }
        Ovt_AutoVarOff ( ) ;
    }
}

void
_Namespace_Do_C_Type ( Namespace * ns )
{
    Lexer * lexer = _Lexer_ ;
    byte * token1, *token2 ;
    if ( ! Compiling ) _Namespace_ActivateAsPrimary ( ns ) ;
    SetState ( _Compiler_, DOING_C_TYPE_DECLARATION, true ) ;
    token1 = _Lexer_Next_NonDebugOrCommentTokenWord ( lexer, 0, 1, 0 ) ;
    token2 = Lexer_Peek_Next_NonDebugTokenWord ( lexer, 1, 0 ) ;
    if ( token2 && ( token2 [0] == '(' ) ) Compile_C_FunctionDeclaration ( token1 ) ;
    else
    {
        if ( Compiling )
        {
            _Compiler_->AutoVarTypeNamespace = ns ;
            Compile_C_TypeDeclaration ( token1 ) ;
            _Compiler_->AutoVarTypeNamespace = 0 ;
        }
        else Interpreter_InterpretAToken ( _Interpreter_, token1, - 1, - 1 ) ; //_Lexer_->TokenStart_ReadLineIndex, _Lexer_->SC_Index ) ;
    }
    SetState ( _Compiler_, DOING_C_TYPE_DECLARATION, false ) ;
}

void
CSL_Do_C_Type ( Namespace * ns )
{
    Context * cntx = _Context_ ;
    if ( ! GetState ( cntx, IS_FORWARD_DOTTED ) )
    {
        Compiler * compiler = cntx->Compiler0 ;
        if ( GetState ( cntx, C_SYNTAX ) ) Compiler_Save_C_BackgroundNamespace ( compiler ) ;
        if ( ( ! Compiling ) || ( ! GetState ( _CSL_, SOURCE_CODE_STARTED ) ) ) CSL_InitSourceCode_WithCurrentInputChar ( _CSL_, 0 ) ; // must be here for wdiss and add addToHistory
        if ( ! GetState ( compiler, DOING_C_TYPE ) )
        {
            SetState ( compiler, DOING_C_TYPE, true ) ;
            if ( ! GetState ( compiler, LC_ARG_PARSING ) )
            {
                LC_Delete ( _LC_ ) ;
                if ( ! Compiling )
                {
                    Compiler_Init ( compiler, 0 ) ;
                    CSL_SourceCode_Init ( ) ;
                    CSL_WordList_Init ( 0 ) ;
                }
                _Namespace_Do_C_Type ( ns ) ;
            }
            SetState ( compiler, DOING_C_TYPE, false ) ;
        }
        if ( GetState ( cntx, C_SYNTAX ) ) Compiler_SetAs_InNamespace_C_BackgroundNamespace ( compiler ) ;
    }
    else Namespace_Do_Namespace ( ns ) ;
}

void
CSL_Do_ClassField ( Word * word )
{
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    byte * offsetPtr = 0 ;
    cntx->Interpreter0->CurrentObjectNamespace = word ; // update this namespace 
    compiler->ArrayEnds = 0 ;

    if ( GetState ( cntx, ( C_SYNTAX | INFIX_MODE ) ) && ( ! compiler->LHS_Word ) && ( ! GetState ( cntx, IS_FORWARD_DOTTED ) ) && ( ! GetState ( cntx, IS_RVALUE ) ) ) compiler->LHS_Word = word ;
    if ( word->Offset ) offsetPtr = Compiler_IncrementCurrentAccumulatedOffset ( compiler, word->Offset ) ;
    if ( ! ( ( CompileMode ) || GetState ( compiler, LC_ARG_PARSING ) ) ) CSL_Do_AccumulatedAddress ( word, ( byte* ) TOS, word->Offset ) ;
    if ( GetState ( cntx, IS_FORWARD_DOTTED ) ) Finder_SetQualifyingNamespace ( cntx->Finder0, word->TypeNamespace ) ;
    word->BaseObject = cntx->BaseObject ;
    CSL_TypeStack_SetTop ( word ) ;
    Word_SetSourceCoding ( word, offsetPtr - 3 ) ; // 3 : sizeof add immediate insn with rex

}

// a constant is, of course, a literal

void
CSL_Do_LispSymbol ( Word * word )
{
    // rvalue - rhs for stack var
    if ( Compiling )
    {
        _Compile_GetVarLitObj_RValue_To_Reg ( word, ACC, 0 ) ;
        _Word_CompileAndRecord_PushReg ( word, ACC, true ) ;
    }
    CSL_TypeStackPush ( word ) ;
}

void
Array_Do_AccumulatedAddress ( int64 totalOffset )
{
    byte * address = ( ( byte* ) _Context_->BaseObject->W_Value ) + totalOffset ;
    TOS = ( uint64 ) address ; // C lvalue
}

void
Do_AccumulatedAddress ( byte * accumulatedOffsetPointer, int64 offset, byte size )
{
    Context * cntx = _Context_ ;
    accumulatedOffsetPointer += offset ;
    if ( GetState ( cntx, IS_RVALUE ) )
    {
        if ( size == 1 ) TOS = ( int64 ) ( ( * ( byte* ) accumulatedOffsetPointer ) ) ; // C rvalue
        else if ( size == 2 ) TOS = ( int64 ) ( ( * ( int16* ) accumulatedOffsetPointer ) ) ; // C rvalue
        else if ( size == 4 ) TOS = ( int64 ) ( ( * ( int32* ) accumulatedOffsetPointer ) ) ; // C rvalue
        else TOS = ( * ( int64* ) accumulatedOffsetPointer ) ; // default and 8 bytes - cell
    }
    else TOS = ( uint64 ) accumulatedOffsetPointer ; // C lvalue
}

void
CSL_Do_AccumulatedAddress ( Word * word, byte * accumulatedAddress, int64 offset )
{
    Context * cntx = _Context_ ;
    Namespace * ns ;
    byte size = ( ( ns = TypeNamespace_Get ( word ) ) ? ( int64 ) _Namespace_VariableValueGet ( ns, ( byte* ) "size" ) : CELL ) ;
    Do_AccumulatedAddress ( accumulatedAddress, offset, size ) ;
}

// nb. 'word' is the previous word to the '.' (dot) cf. CSL_Dot so it can be recompiled, a little different maybe, as an object

void
_CSL_Do_Dot ( Context * cntx, Word * word ) // .
{
    if ( word && ( ! cntx->BaseObject ) )
    {
        if ( word->W_ObjectAttributes & NAMESPACE_TYPE ) Finder_SetQualifyingNamespace ( cntx->Finder0, word ) ;
        else cntx->BaseObject = word ;
    }
}

void
CSL_Dot ( ) // .
{
    Context * cntx = _Context_ ;
    SetState ( cntx, CONTEXT_PARSING_QID, true ) ;
    Word * word = Compiler_PreviousNonDebugWord ( 0 ) ; // 0 : rem: we just popped the WordStack above
    _CSL_Do_Dot ( cntx, word ) ;
}

void
_Word_CompileAndRecord_PushReg ( Word * word, int64 reg, Boolean recordFlag )
{
    if ( word )
    {
        if ( recordFlag ) word->StackPushRegisterCode = Here ; // nb. used! by the rewriting optInfo
        if ( word->W_ObjectAttributes & REGISTER_VARIABLE ) _Compile_Stack_PushReg ( DSP, word->RegToUse ) ;
        else
        {

            _Compile_Stack_PushReg ( DSP, reg ) ;
            word->RegToUse = reg ;
        }
    }
}

void
Compiler_Save_C_BackgroundNamespace ( Compiler * compiler )
{
    compiler->C_BackgroundNamespace = _Namespace_FirstOnUsingList ( ) ; //nb! must be before CSL_LocalsAndStackVariablesBegin else CSL_End_C_Block will 
}

void
Compiler_SetAs_InNamespace_C_BackgroundNamespace ( Compiler * compiler )
{
    if ( compiler->C_BackgroundNamespace ) _CSL_Namespace_InNamespaceSet ( compiler->C_BackgroundNamespace ) ;
}

void
Compiler_Clear_Qid_BackgroundNamespace ( Compiler * compiler )
{
    compiler->Qid_BackgroundNamespace = 0 ;
}

void
Compiler_Save_Qid_BackgroundNamespace ( Compiler * compiler )
{
    //if ( ! compiler->Qid_BackgroundNamespace ) 
    compiler->Qid_BackgroundNamespace = _CSL_Namespace_InNamespaceGet ( ) ; // _Namespace_FirstOnUsingList ( ) ; 
}

void
Compiler_SetAs_InNamespace_Qid_BackgroundNamespace ( Compiler * compiler )
{
    if ( compiler->Qid_BackgroundNamespace ) _CSL_Namespace_InNamespaceSet ( compiler->Qid_BackgroundNamespace ) ;
}

