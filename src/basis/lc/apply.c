#include "../../include/csl.h"
//===================================================================================================================
//| _LO_Apply 
//===================================================================================================================

#define NEW_LC_COMPILE 0

ListObject *
LO_Apply ( LambdaCalculus * lc, ListObject * l0, ListObject *lfirst, ListObject *lfunction, ListObject *largs, Boolean applyFlag )
{
    SetState ( lc, LC_APPLY, true ) ;
    if ( applyFlag && lfunction && ( ( lfunction->W_MorphismAttributes & ( CPRIMITIVE | CSL_WORD ) ) 
        || ( lfunction->W_LispAttributes & ( T_LISP_COMPILED_WORD ) ) ) )
    {
        if ( GetState ( lc, LC_DEFINE_MODE ) && ( ! CompileMode ) ) return lfirst ;
        else l0 = _LO_Apply ( lfirst, lfunction, largs ) ;
    }
    else if ( lfunction && ( lfunction->W_LispAttributes & T_LAMBDA ) && lfunction->Lo_LambdaFunctionBody )
    {
        // LambdaArgs, the formal args, are not changed by LO_Substitute (locals - lvals are just essentially 'renamed') and thus don't need to be copied
        LO_Substitute ( _LO_First ( ( ListObject * ) lfunction->Lo_LambdaFunctionParameters ), _LO_First ( largs ) ) ;
        lc->CurrentLambdaFunction = lfunction ;
        l0 = ( ListObject * ) lfunction->Lo_LambdaFunctionBody ;
        l0 = LO_EvalList ( lc, l0, largs, applyFlag ) ;
    }
    else
    {
        //these cases seems common sense for what these situations should mean and seem to add something positive to the usual lisp/scheme semantics !?
        if ( ! largs ) l0 = lfunction ;
        else
        {
            LO_AddToHead ( largs, lfunction ) ;
            l0 = largs ;
        }
        SetState ( lc, LC_COMPILE_MODE, false ) ;
    }
    SetState ( lc, LC_APPLY, false ) ;
    return l0 ;
}

ListObject *
_LO_Apply ( ListObject *lfirst, ListObject *lfunction, ListObject *largs )
{
    LambdaCalculus *lc = _O_->OVT_LC ;
    SetState ( lc, LC_APPLY, true ) ;
    ListObject *vReturn ;
    d0 ( if ( Is_DebugModeOn ) LO_Debug_ExtraShow ( 0, 1, 0, ( byte * ) "\n_LO_Apply : \n\tl0 =%s", _LO_PRINT_TO_STRING ( l0 ) ) ) ;
    if ( lfunction->W_LispAttributes & LIST_FUNCTION ) return (( ListFunction ) lfunction->Lo_CSLWord->Definition )( lfirst ) ;
    else if ( lfunction->W_MorphismAttributes & CSL_WORD ) // this case is hypothetical for now
    {
        if ( lfunction->W_LispAttributes & T_LISP_CSL_COMPILED )
        {
            Interpreter_DoWord ( _Context_->Interpreter0, lfunction->Lo_CSLWord, lfunction->W_RL_Index, lfunction->W_SC_Index ) ;
            vReturn = nil ;
        }
        else vReturn = _LO_Do_FunctionBlock ( lfunction, largs ) ;
    }
    else if ( largs ) vReturn = _LO_Do_FunctionBlock ( lfunction, largs ) ;
    else
    {
        lc->ParenLevel -- ;
        if ( CompileMode ) LO_CheckEndBlock ( ) ;
        SetState ( lc, LC_COMPILE_MODE, false ) ;
        vReturn = lfunction ;
    }
    SetState ( lc, LC_APPLY, false ) ;
    //l0->W_Value = (int64) vReturn ; // this seems the right place for vReturn instead of the stack
    return vReturn ;
}

void
_Interpreter_LC_InterpretWord ( Interpreter *interp, ListObject *l0, Boolean functionFlag )
{
    Word * word ;
    {
        word = l0->Lo_CSLWord ;
        if ( ! word ) word = l0 ;
        //if ( kbhit ( ) ) CSL_Quit ( ) ;
        Interpreter_DoWord ( interp, word, l0->W_RL_Index, l0->W_SC_Index ) ;
    }
}

void
_LO_CompileOrInterpret_One ( ListObject *l0, int64 functionFlag )
{
    // just interpret the non-nil, non-list objects
    // nil means that it doesn't need to be interpreted any more
    if ( l0 && ( ! ( l0->W_LispAttributes & ( LIST | LIST_NODE | T_NIL ) ) ) ) _Interpreter_LC_InterpretWord ( _Interpreter_, l0, functionFlag ) ;
}

void
LO_CompileOrInterpretArgs ( ListObject *largs )
{
    ListObject * arg ;
    for ( arg = _LO_First ( largs ) ; arg ; arg = _LO_Next ( arg ) )
    {
        if ( GetState ( _O_->OVT_LC, LC_INTERP_DONE ) ) return ; // i don't remember why this is here ??
        _LO_CompileOrInterpret_One ( arg, 0 ) ; // research : how does CAttribute get set to T_NIL?
    }
}

void
_LO_CompileOrInterpret ( ListObject *lfunction, ListObject *largs )
{
    ListObject *lfword = lfunction->Lo_CSLWord ;
    //Word_SetTsrliScwi( lfword, lfunction->W_RL_Index, lfunction->W_SC_Index ) ;

    if ( largs && lfword && ( lfword->W_MorphismAttributes & ( CATEGORY_OP_ORDERED | CATEGORY_OP_UNORDERED ) ) ) // ?!!? 2 arg op with multi-args : this is not a precise distinction yet : need more types ?!!?
    {
        Boolean svTcs = GetState ( _CSL_, TYPECHECK_ON ) ; // sometimes ok but for now off here
        SetState ( _CSL_, TYPECHECK_ON, false ) ; // sometimes ok but for now off here
        _LO_CompileOrInterpret_One ( largs, 0 ) ;
        while ( ( largs = _LO_Next ( largs ) ) )
        {
            _LO_CompileOrInterpret_One ( largs, 0 ) ; // two args first then op, then after each arg the operator : nb. assumes word can take unlimited args 2 at a time
            _LO_CompileOrInterpret_One ( lfword, 0 ) ;
        }
        SetState ( _CSL_, TYPECHECK_ON, svTcs ) ;
    }
    else
    {
        LO_CompileOrInterpretArgs ( largs ) ;
        if ( lfword && ( ! ( lfword->W_LispAttributes & T_LISP_csl ) ) ) _LO_CompileOrInterpret_One ( lfword, 1 ) ;
    }
    _O_->OVT_LC->LastInterpretedWord = lfword ;
}

ListObject *
_LO_Do_FunctionBlock ( ListObject *lfunction, ListObject *largs )
{
    LambdaCalculus *lc = _O_->OVT_LC ;
    ListObject *vReturn, *lfargs = _LO_First ( largs ) ;
    _LO_CompileOrInterpret ( lfunction, lfargs ) ;
    lc->ParenLevel -- ;
    // this is necessary in "lisp" mode : eg. if user hits return but needs to be clarified, refactored, maybe renamed, etc.
    if ( ! GetState ( lc, LC_INTERP_DONE ) )
    {
        if ( CompileMode ) LO_CheckEndBlock ( ) ;
        vReturn = LO_PrepareReturnObject ( ) ;
    }
    else vReturn = nil ;
    return vReturn ;
}

void
LO_Substitute ( ListObject *lambdaParameters, ListObject * funcCallValues )
{
    while ( lambdaParameters && funcCallValues )
    {
        // ?!? this may not be the right idea but we want it so that we can have transparent lists in the parameters, ie. 
        // no affect with a parenthesized list or just unparaenthesized parameters of the same number
        if ( lambdaParameters->W_LispAttributes & ( LIST | LIST_NODE ) )
        {
            if ( funcCallValues->W_LispAttributes & ( LIST | LIST_NODE ) ) funcCallValues = _LO_First ( funcCallValues ) ;
            //else Error ( "\nLO_Substitute : funcCallValues list structure doesn't match parameter list", QUIT ) ;
            lambdaParameters = _LO_First ( lambdaParameters ) ; // can something like this work
        }
        else if ( funcCallValues->W_LispAttributes & ( LIST | LIST_NODE ) )
        {
            if ( lambdaParameters->W_LispAttributes & ( LIST | LIST_NODE ) ) lambdaParameters = _LO_First ( lambdaParameters ) ; // can something like this work
            //else Error ( "\nLO_Substitute : funcCallValues list structure doesn't match parameter list", QUIT ) ;
            funcCallValues = _LO_First ( funcCallValues ) ;
        }
        // just preserve the name of the arg for the finder
        // so we now have the call values with the parameter names - parameter names are unchanged 
        // so when we eval/print these parameter names they will have the function calling values -- lambda calculus substitution - beta reduction
        funcCallValues->Lo_Name = lambdaParameters->Lo_Name ;
        lambdaParameters = _LO_Next ( lambdaParameters ) ;
        funcCallValues = _LO_Next ( funcCallValues ) ;
    }
}

ListObject *
LO_PrepareReturnObject ( )
{
    uint64 type = 0 ;
    byte * name ;
    if ( ! CompileMode )
    {
        Namespace * ns = CSL_In_Namespace ( ) ;
        name = ns->Name ;
        if ( Namespace_IsUsing ( ( byte* ) "BigNum" ) ) type = T_BIG_NUM ;
        return DataObject_New (T_LC_NEW, 0, 0, 0, LITERAL | type, LITERAL | type, 0, DataStack_Pop ( ), 0, 0, 0, - 1 ) ;
    }
    else return nil ;
}

void
LO_BeginBlock ( )
{
    if ( ! Compiler_BlockLevel ( _Compiler_ ) ) _LC_->SavedCodeSpace = _O_CodeByteArray ;
    CSL_BeginBlock ( ) ;
}

void
LO_EndBlock ( )
{
    Compiler * compiler = _Context_->Compiler0 ;
    if ( _LC_ && _LC_->SavedCodeSpace )
    {
        BlockInfo * bi = ( BlockInfo * ) Stack_Top ( compiler->BlockStack ) ;
        CSL_EndBlock ( ) ;
        if ( ! GetState ( _LC_, LC_COMPILE_MODE ) )
        {
            CSL_BlockRun ( ) ;
        }
        if ( ! Compiler_BlockLevel ( compiler ) ) Set_CompilerSpace ( _LC_->SavedCodeSpace ) ;
        bi->LogicCodeWord = _LC_->LastInterpretedWord ;
    }
}

void
LO_CheckEndBlock ( )
{
    LambdaCalculus * lc = _LC_ ;
    Compiler * compiler = _Context_->Compiler0 ;
    if ( GetState ( compiler, LISP_COMBINATOR_MODE ) )
    {
        int64 cii = Stack_Top ( compiler->CombinatorInfoStack ) ;
        CombinatorInfo ci ; // remember sizeof of CombinatorInfo = 4 bytes
        ci.CI_i32_Info = cii ;
        if ( ( lc->ParenLevel == ci.ParenLevel ) && ( Compiler_BlockLevel ( compiler ) > ci.BlockLevel ) )
        {
            LO_EndBlock ( ) ;
        }
    }
}

int64
_LO_CheckBeginBlock ( )
{
    LambdaCalculus * lc = _LC_ ;
    Compiler * compiler = _Context_->Compiler0 ;
    int64 cii = Stack_Top ( compiler->CombinatorInfoStack ) ;
    CombinatorInfo ci ; // remember sizeof of CombinatorInfo = 4 bytes
    ci.CI_i32_Info = cii ;
    if ( ( GetState ( compiler, LISP_COMBINATOR_MODE ) ) && ( lc->ParenLevel == ci.ParenLevel ) && ( Compiler_BlockLevel ( compiler ) == ci.BlockLevel ) )
    {
        return true ;
    }
    return false ;
}

int32
_LO_CheckBegunBlock ( )
{
    LambdaCalculus * lc = _LC_ ;
    Compiler * compiler = _Context_->Compiler0 ;
    int32 cii = Stack_Top ( compiler->CombinatorInfoStack ) ;
    CombinatorInfo ci ; // remember sizeof of CombinatorInfo = 4 bytes
    ci.CI_i32_Info = cii ;
    if ( ( GetState ( compiler, LISP_COMBINATOR_MODE ) ) && ( lc->ParenLevel == ci.ParenLevel ) && ( Compiler_BlockLevel ( compiler ) > ci.BlockLevel ) )
    {
        return true ;
    }
    return false ;
}

int64
LO_CheckBeginBlock ( )
{
    if ( _LO_CheckBeginBlock ( ) )
    {
        LO_BeginBlock ( ) ;
        return true ;
    }
    return false ;
}

void
Arrays_DoArrayArgs_Lisp ( Word ** pl1, Word * l1, Word * arrayBaseObject, int64 objSize, Boolean saveCompileMode, Boolean *variableFlag )
{
    do
    {
        if ( Do_NextArrayToken ( l1, l1->Name, arrayBaseObject, objSize, saveCompileMode, variableFlag ) ) break ;
    }
    while ( l1 = LO_Next ( l1 ) ) ;
    *pl1 = l1 ;
}

void
_LO_Apply_ArrayArg ( ListObject ** pl1, int64 *i )
{
    _CSL_ArrayBegin ( 1, pl1, i ) ;
}

// compile mode on
void
_LO_Apply_NonMorphismArg ( ListObject ** pl1, int64 *i )
{
    Context * cntx = _Context_ ;
    ListObject *l1 = * pl1 ;
    Word * word = l1 ;
    word = l1->Lo_CSLWord ;
    word = Compiler_CopyDuplicatesAndPush ( word, l1->W_RL_Index, l1->W_SC_Index ) ;
    byte * here = Here ;
    Word_Eval ( word ) ;
    Word *baseObject = _Context_->BaseObject ;
    if ( ( word->Name[0] == '\"' ) || ( ! _Lexer_IsTokenForwardDotted ( cntx->Lexer0, l1->W_RL_Index + Strlen ( word->Name ) - 1 ) ) ) // ( word->Name[0] == '\"' ) : sometimes strings have ".[]" chars within but are still just strings
    {
        if ( word->StackPushRegisterCode ) SetHere ( word->StackPushRegisterCode, 1 ) ;
        else if ( baseObject && baseObject->StackPushRegisterCode ) SetHere ( baseObject->StackPushRegisterCode, 1 ) ;
        Compile_Move_Reg_To_Reg ( RegParameterOrder ( (*i) ++ ), ACC, 0 ) ;
        if ( baseObject ) _Debugger_->PreHere = baseObject->Coding ;
        SetState ( cntx, ADDRESS_OF_MODE, false ) ;
        _Debugger_->PreHere = here ;
    }
}

void
_LO_Apply_Arg ( LambdaCalculus * lc, ListObject ** pl1, int64 * i )
{
    Context * cntx = _Context_ ;
    ListObject *l1 = * pl1, * l2 ;
    Word * word = l1 ;

    Set_CompileMode ( true ) ;
    if ( l1->W_LispAttributes & ( LIST | LIST_NODE ) )
    {
        Set_CompileMode ( false ) ;
        l2 = LO_Eval ( lc, l1 ) ;
        _Debugger_->PreHere = Here ;
        if ( ! l2 || ( l2->W_LispAttributes & T_NIL ) ) Compile_MoveImm_To_Reg ( RegParameterOrder ( (*i) ++ ), DataStack_Pop ( ), CELL_SIZE ) ;
        else Compile_MoveImm_To_Reg ( RegParameterOrder ( (*i) ++ ), ( int64 ) * l2->Lo_PtrToValue, CELL_SIZE ) ;
        _DEBUG_SHOW ( l2, 1, 0 ) ;
    }
    else if ( ( l1->W_ObjectAttributes & NON_MORPHISM_TYPE ) ) _LO_Apply_NonMorphismArg ( pl1, i ) ;
    else if ( ( l1->Name [0] == '.' ) || ( l1->Name [0] == '&' ) )
        Interpreter_DoWord ( cntx->Interpreter0, l1->Lo_CSLWord, l1->W_RL_Index, l1->W_SC_Index ) ;
    else if ( ( l1->Name[0] == '[' ) ) _LO_Apply_ArrayArg ( pl1, i ) ;
    else
    {
        word = Compiler_CopyDuplicatesAndPush ( word, l1->W_RL_Index, l1->W_SC_Index ) ;
        DEBUG_SETUP ( word, 0 ) ;
        _Compile_Move_StackN_To_Reg ( RegParameterOrder ( (*i) ++ ), DSP, 0 ) ;
        _DEBUG_SHOW ( word, 1, 0 ) ;
    }
done:
    _Debugger_->PreHere = Here ;
}
// for calling 'C' functions such as printf or other system functions
// where the arguments are pushed first from the end of the list like 'C' arguments
// this is a little confusing : the args are LO_Read left to Right for C we want them right to left except qid word which remain left to right

ListObject *
_LO_Apply_C_LtoR_ArgList ( LambdaCalculus * lc, ListObject * l0, Word * word )
{
    Context * cntx = _Context_ ;
    ListObject *l1 ;
    ByteArray * scs = _O_CodeByteArray ;
    Compiler * compiler = cntx->Compiler0 ;
    int64 i, svcm = CompileMode ;

    d0 ( if ( Is_DebugModeOn ) LO_Debug_ExtraShow ( 0, 2, 0, ( byte* ) "\nEntering _LO_Apply_ArgList..." ) ) ;
    if ( l0 )
    {
        Set_CompileMode ( true ) ;
        if ( ! svcm ) CSL_BeginBlock ( ) ;
        if ( word->W_MorphismAttributes & ( DLSYM_WORD | C_PREFIX ) ) Set_CompileMode ( true ) ;
        _Debugger_->PreHere = Here ;
        for ( i = 0, l1 = _LO_First ( l0 ) ; l1 ; l1 = LO_Next ( l1 ) ) _LO_Apply_Arg ( lc, &l1, &i ) ;
        Set_CompileMode ( true ) ;
        _Debugger_->PreHere = Here ;
        Word_SetCodingAndSourceCoding ( word, Here ) ;
        word = Compiler_CopyDuplicatesAndPush ( word, word->W_RL_Index, word->W_SC_Index ) ;
        // for printf ?? others 
        //System V ABI : "%rax is used to indicate the number of vector arguments passed to a function requiring a variable number of arguments"
        if ( ( String_Equal ( word->Name, "printf" ) || ( String_Equal ( word->Name, "sprintf" ) ) ) ) Compile_MoveImm_To_Reg ( RAX, i, CELL ) ; 
        //if ( ( String_Equal ( word->Name, "printf" ) || ( String_Equal ( word->Name, "sprintf" ) ) ) ) Compile_MoveImm_To_Reg ( RAX, 0, CELL ) ; 
        //Compiler_SCA_Word_SetCodingHere_And_ClearPreviousUse ( word, 1 ) ;
        Word_Eval ( word ) ;
        if ( word->W_MorphismAttributes & RAX_RETURN ) _Word_CompileAndRecord_PushReg ( word, ACC, true ) ;
        if ( ! svcm )
        {
            CSL_EndBlock ( ) ;
            block b = ( block ) DataStack_Pop ( ) ;
            Set_CompileMode ( svcm ) ;
            Set_CompilerSpace ( scs ) ;
            _Debugger_->PreHere = Here ; // prevent debugger from showing the 1 byte ret for 'word'
            _DEBUG_SETUP (word, 0, (byte*) 0, 1 , 0) ;
            _DEBUG_SHOW ( word, 1, 0 ) ;
            Dbg_Block_Eval (word, b) ;
        }
        d0 ( if ( Is_DebugModeOn ) LO_Debug_ExtraShow ( 0, 2, 0, ( byte* ) "\nLeaving _LO_Apply_ArgList..." ) ) ;
        SetState ( compiler, LC_ARG_PARSING, false ) ;
    }
    return nil ;
}

void
LC_CompileRun_C_ArgList ( Word * word ) // C protocol - x64 : left to right arguments put into registers 
{
    Namespace * backgroundNamespace = _CSL_Namespace_InNamespaceGet ( ) ;
    LambdaCalculus * lc = LC_Init_Runtime ( ) ;
    Context * cntx = _Context_ ;
    Lexer * lexer = cntx->Lexer0 ;
    Compiler * compiler = cntx->Compiler0 ;
    byte *svDelimiters = lexer->TokenDelimiters ;
    ListObject * l0 ;

    byte * token = _Lexer_ReadToken ( lexer, ( byte* ) " ,;\n\r\t" ) ;
    if ( word->W_MorphismAttributes & ( C_PREFIX | C_PREFIX_RTL_ARGS ) )
    {
        if ( ( ! token ) || strcmp ( "(", ( char* ) token ) ) Error ( "Syntax error : C RTL Args : no '('", ABORT ) ; // should be '('
    }
    lc->ParenLevel = 1 ;
    if ( word->W_MorphismAttributes & ( C_PREFIX | C_PREFIX_RTL_ARGS ) )
    {
        if ( ! Compiling ) Compiler_Init (compiler, 0) ;
        SetState ( compiler, LC_ARG_PARSING, true ) ;
        int64 svcm = CompileMode ;
        Set_CompileMode ( false ) ; // we must have the arguments pushed and not compiled for _LO_Apply_C_Rtl_ArgList which will compile them for a C_Rtl function
        int64 svDs = GetState ( _CSL_, _DEBUG_SHOW_ ) ;
        DebugShow_Off ;
        l0 = _LO_Read ( lc ) ;
        SetState ( _CSL_, _DEBUG_SHOW_, svDs ) ;
        Set_CompileMode ( svcm ) ; // we must have the arguments pushed and not compiled for _LO_Apply_C_Rtl_ArgList which will compile them for a C_Rtl function
        _LO_Apply_C_LtoR_ArgList ( lc, l0, word ) ;
        LC_LispNamespacesOff ( ) ;
        SetState ( compiler, LC_ARG_PARSING | LC_C_RTL_ARG_PARSING, false ) ;
    }
    _CSL_Namespace_InNamespaceSet ( backgroundNamespace ) ;
    Lexer_SetTokenDelimiters ( lexer, svDelimiters, COMPILER_TEMP ) ;
    LC_RestoreStackPointer ( lc ) ;
}

// assumes list contains only one application 

block
CompileLispBlock ( ListObject *args, ListObject * body )
{
    LambdaCalculus * lc = _LC_ ;
    block code ;
    byte * here = Here ;
    Word * word = _Context_->CurrentWordBeingCompiled ;
    LO_BeginBlock ( ) ; // must have a block before local variables if there are register variables because _CSL_Parse_LocalsAndStackVariables will compile something
    SetState ( lc, ( LC_COMPILE_MODE | LC_BLOCK_COMPILE ), true ) ; // before _CSL_Parse_LocalsAndStackVariables
    Namespace * locals = _CSL_Parse_LocalsAndStackVariables (1, 1, args, 0, 0 ) ; //(GetState ( _LC_, LC_BLOCK_COMPILE|LC_BEGIN_MODE ))) ;// ;false ) ;
    word->W_MorphismAttributes = BLOCK ;
    word->W_LispAttributes |= T_LISP_COMPILED_WORD ;
    _LO_Eval ( lc, body, locals, 1 ) ;
    if ( GetState ( lc, LC_COMPILE_MODE ) )
    {
        LO_EndBlock ( ) ;
        code = ( block ) DataStack_Pop ( ) ;
        //SetState ( lc, ( LC_COMPILE_MODE | LC_BLOCK_COMPILE ), false ) ;
    }
    else // nb. LISP_COMPILE_MODE : this state can change with some functions that can't be compiled yet
    {
        SetHere ( here, 1 ) ; //recover the unused code space
        code = 0 ;
        word->W_LispAttributes &= ~ T_LISP_COMPILED_WORD ;
        if ( _O_->Verbosity > 1 )
        {
            AlertColors ;
            Printf ( ( byte* ) "\nLisp can not compile this word yet : %s : -- interpreting ...\n ", _Word_SourceCodeLocation_pbyte ( word ) ) ;
            DefaultColors ;
        }
    }
    _Word_DefinitionStore ( word, ( block ) code ) ; // not _Word_InitFinal because this is already CSL->CurrentWordCompiling with W_SourceCode, etc.
    SetState ( lc, ( LC_BLOCK_COMPILE ), false ) ; // necessary !!
    return code ;
}

