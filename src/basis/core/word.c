#include "../../include/csl.h"

void
Word_Run ( Word * word )
{
    if ( word )
    {
        word->StackPushRegisterCode = 0 ; // nb. used! by the rewriting optInfo
        // keep track in the word itself where the machine code is to go, if this word is compiled or causes compiling code - used for optimization
        if ( ( GetState ( _Compiler_, ( COMPILE_MODE | ASM_MODE ) ) ) ) Word_SetCodingAndSourceCoding ( word, Here ) ; // if we change it later (eg. in lambda calculus) we must change it there because the rest of the compiler depends on this
        _Context_->CurrentlyRunningWord = word ;
        word = _Context_->CurrentlyRunningWord ; // _Context_->CurrentlyRunningWord (= 0) may have been modified by debugger //word->Definition ) ;
        Block_Eval ( word->Definition ) ;
        _Context_->LastRanWord = word ;
        _Context_->CurrentlyRunningWord = 0 ;
    }
}

void
Word_Eval ( Word * word )
{
    if ( word )
    {
        if ( ! sigsetjmp ( _Context_->JmpBuf0, 0 ) ) // siglongjmp from _Debugger_InterpreterLoop
        {
            if ( ! ( GetState ( word, STEPPED ) ) )
            {
                _Context_->CurrentEvalWord = word ;
                if ( IS_MORPHISM_TYPE ( word ) ) CSL_Typecheck ( word ) ;
                DEBUG_SETUP ( word ) ;
                if ( ( word->W_MorphismAttributes & IMMEDIATE ) || ( ! CompileMode ) ) Word_Run ( word ) ;
                else _Word_Compile ( word ) ;
                _DEBUG_SHOW ( word, 0 ) ;
                _Context_->CurrentEvalWord = 0 ;
                _Context_->LastEvalWord = word ;
            }
            SetState ( word, STEPPED, false ) ;
        }
        else Set_DataStackPointers_FromDebuggerDspReg ( ) ;
    }
}

void
_Word_Interpret ( Word * word )
{
    Interpreter_DoWord ( _Interpreter_, word, word->W_RL_Index, word->W_SC_Index ) ;
}

void
_Word_Compile ( Word * word )
{
    Compiler_SCA_Word_SetCodingHere_And_ClearPreviousUse ( word, 0 ) ;
    if ( ! word->Definition ) CSL_SetupRecursiveCall ( ) ;
    else if ( ( GetState ( _CSL_, INLINE_ON ) ) && ( word->W_MorphismAttributes & INLINE ) && ( word->S_CodeSize ) ) _Compile_WordInline ( word ) ;
    else Compile_CallWord_Check_X84_ABI_RSP_ADJUST ( word ) ;
}

Namespace *
_Word_Namespace ( Word * word )
{
    if ( word->W_ObjectAttributes & NAMESPACE ) return ( Namespace * ) word ;
    else return word->ContainingNamespace ;
}

// deep copy from word0 to word

void
_Word_Copy ( Word * word, Word * word0 )
{
    WordData * swdata = word->W_WordData ;
    MemCpy ( word, word0, sizeof ( Word ) + sizeof ( WordData ) ) ;
    word->W_WordData = swdata ; // restore the WordData pointer we overwrote by the above MemCpy
}

Word *
Word_Copy ( Word * word0, uint64 allocType )
{
    Word * word = _Word_Allocate ( allocType ) ;
    _Word_Copy ( word, word0 ) ;
    word->WAllocType = allocType ;
    return word ;
}

void
_Word_Finish ( Word * word )
{
    DObject_Finish ( word ) ;
    _CSL_FinishWordDebugInfo ( word ) ;
    CSL_Finish_WordSourceCode ( _CSL_, word ) ;
    CSL_TypeStackReset ( ) ;
    _CSL_->LastFinished_Word = word ;
    Compiler_Init ( _Compiler_, 0 ) ;
}

void
_Word_DefinitionStore ( Word * word, block code )
{
    _DObject_ValueDefinition_Init ( word, ( int64 ) code, BLOCK, 0, 0 ) ;
}

void
Word_InitFinal ( Word * word, byte * code )
{
    _Word_DefinitionStore ( word, ( block ) code ) ;
    if ( ! word->S_ContainingNamespace ) _Word_Add ( word, 1, 0 ) ; // don't re-add if it is a recursive word cf. CSL_BeginRecursiveWord
    _Word_Finish ( word ) ;
}

void
_Word_Add ( Word * word, int64 addToInNs, Namespace * addToNs )
{
    Namespace * ins = 0, *ns ;
    if ( addToNs ) Namespace_DoAddWord ( addToNs, word ) ;
    else if ( addToInNs )
    {
        if ( ! ( word->W_ObjectAttributes & ( LITERAL ) ) )
        {
            Namespace * ins = _CSL_InNamespace ( ) ; //_CSL_Namespace_InNamespaceGet ( ) ;
            Namespace_DoAddWord ( ins, word ) ;
        }
    }
    if ( _O_->Verbosity > 3 )
    {
        ns = addToNs ? addToNs : ins ;
        if ( ns )
        {
            if ( word->W_MorphismAttributes & BLOCK ) Printf ( ( byte* ) "\nnew Word :: %s.%s", ns->Name, word->Name ) ;
            else Printf ( ( byte* ) "\nnew DObject :: %s.%s", ns->Name, word->Name ) ;
        }
    }
}

Word *
_Word_Allocate ( uint64 allocType )
{
    Word * word = 0 ;
    int64 size = ( sizeof ( Word ) + sizeof ( WordData ) ) ;
    word = ( Word* ) OVT_CheckRecycleableAllocate ( _O_->MemorySpace0->RecycledWordList, size ) ;
    if ( word ) _O_->MemorySpace0->RecycledWordCount ++ ;
    else word = ( Word* ) Mem_Allocate ( size, allocType ) ;
    ( ( DLNode* ) word )->n_Size = size ;
    word->W_WordData = ( WordData * ) ( word + 1 ) ; // nb. "pointer arithmetic"
    return word ;
}

Word *
_Word_Create ( byte * name, uint64 morphismType, uint64 objectType, uint64 lispType, uint64 allocType )
{
    Word * word = _Word_Allocate ( allocType ? allocType : DICTIONARY ) ;
    if ( allocType & ( EXISTING ) ) _Symbol_NameInit ( ( Symbol * ) word, name ) ;
    else _Symbol_Init_AllocName ( ( Symbol* ) word, name, STRING_MEM ) ;
    word->WAllocType = allocType ;
    word->W_MorphismAttributes = morphismType ;
    word->W_ObjectAttributes = objectType ;
    word->W_LispAttributes = lispType ;
    if ( Is_NamespaceType ( word ) ) word->Lo_List = dllist_New ( ) ;
    _Compiler_->Current_Word_Create = word ;
    _CSL_->WordCreateCount ++ ;
    Lexer_Set_ScIndex_RlIndex ( _Lexer_, word, - 1, - 1 ) ; // default values
    Word_SetCodingAndSourceCoding ( word, Here ) ;
    return word ;
}

void
Word_SetLocation ( Word * word )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    if ( rl->InputStringOriginal )
    {
        word->W_WordData->Filename = rl->Filename ;
        word->W_WordData->LineNumber = rl->LineNumber ;
        word->W_CursorPosition = rl->CursorPosition ;
    }
}

Word *
_Word_New ( byte * name, uint64 morphismType, uint64 objectType, uint64 lispType, Boolean addToInNs, Namespace * addToNs, uint64 allocType )
{
    Word * word = _Word_Create ( name, morphismType, objectType, lispType, allocType ) ; // csl_WORD : csl compiled words as opposed to C compiled words
    _Compiler_->Current_Word_New = word ;
    Word_SetLocation ( word ) ;
    _Word_Add ( word, addToInNs, addToNs ) ; // add to the head of the list
    return word ;
}

Word *
Word_New ( byte * name )
{
    Word * word = _Word_New ( name, csl_WORD | WORD_CREATE, 0, 0, 1, 0, DICTIONARY ) ;
    return word ;
}

void
Word_PrintOffset ( Word * word, int64 offset, int64 totalOffset )
{
    Context * cntx = _Context_ ;
    if ( Is_DebugModeOn ) NoticeColors ;
    byte * name = String_ConvertToBackSlash ( word->Name ) ;
    if ( String_Equal ( "]", name ) && cntx->Interpreter0->BaseObject )
    {
        Printf ( ( byte* ) "\n\'%s\' = array end :: base object \'%s\' = 0x%lx : offset = 0x%lx : total offset = 0x%lx => address = 0x%lx",
            name, cntx->Interpreter0->BaseObject->Name, cntx->Interpreter0->BaseObject->W_Value, offset, totalOffset, ( ( byte* ) cntx->Interpreter0->BaseObject->W_PtrToValue ) + totalOffset ) ;
    }
    else
    {
        totalOffset = cntx->Compiler0->AccumulatedOptimizeOffsetPointer ? *cntx->Compiler0->AccumulatedOptimizeOffsetPointer : - 1 ;
        Printf ( ( byte* ) "\n\'%s\' = object field :: type = %s : size (in bytes) = 0x%lx : base object \'%s\' = 0x%lx : offset = 0x%lx : total offset = 0x%lx : address = 0x%lx",
            //name, cntx->Interpreter0->BaseObject ? cntx->Interpreter0->BaseObject->Name : ( byte* ) "",
            name, word->TypeNamespace ? word->TypeNamespace->Name : ( byte* ) "",
            word->ObjectByteSize,
            cntx->Interpreter0->BaseObject ? String_ConvertToBackSlash ( cntx->Interpreter0->BaseObject->Name ) : ( byte* ) "",
            cntx->Interpreter0->BaseObject ? cntx->Interpreter0->BaseObject->W_Value : 0,
            word->Offset, totalOffset, cntx->Interpreter0->BaseObject ? ( ( ( byte* ) cntx->Interpreter0->BaseObject->W_Value ) + totalOffset ) : ( byte* ) - 1 ) ;
    }
    if ( Is_DebugModeOn ) DefaultColors ;
}

byte *
_Word_SourceCodeLocation_pbyte ( Word * word )
{
    byte * b = Buffer_Data ( _CSL_->ScratchB2 ) ;
    if ( word ) sprintf ( ( char* ) b, "%s.%s : %s %ld.%ld", word->ContainingNamespace->Name, word->Name, word->W_WordData->Filename, word->W_WordData->LineNumber, word->W_TokenEnd_ReadLineIndex ) ;
    return String_New ( b, TEMPORARY ) ;
}

void
Word_PrintName ( Word * word )
{
    if ( word ) Printf ( ( byte* ) "%s ", word->Name ) ;
}

byte*
Word_Info ( Word * word )
{
    byte * buffer = 0 ;
    if ( word ) //&& word->ContainingNamespace )
    {
        buffer = Buffer_New_pbyte ( BUFFER_SIZE ) ;
        sprintf ( ( char * ) buffer, "%s.%s", ( char* ) word->ContainingNamespace ? ( char* ) word->ContainingNamespace->Name : "<literal>", ( char * ) word->Name ) ;
    }
    return buffer ;
}

void
Word_Print ( Word * word )
{
    if ( word ) Printf ( ( byte* ) "\n%s", Word_Info ( word ) ) ;
}

void
_Word_Print ( Word * word )
{
    _Context_->WordCount ++ ;
    Printf ( ( byte* ) c_ud ( " %s" ), word->Name ) ;
}

void
Word_N_M_Node_Print ( dlnode * node )
{
    Word * word = ( Word* ) dobject_Get_M_Slot ( ( dobject* ) node, SCN_T_WORD ) ;
    _Word_Print ( word ) ;
}

void
_Word_ShowSourceCode ( Word * word0 )
{
    Word * word = Word_UnAlias ( word0 ) ;
    if ( word && word->W_WordData ) //&& word->W_SourceCode ) //word->CAttribute & ( CPRIMITIVE | BLOCK ) )
    {
        byte * name, *scd ;
        if ( ( ! ( word->W_MorphismAttributes & CPRIMITIVE ) ) && word->W_SourceCode )
        {
            byte * sc = Buffer_Data_Cleared ( _CSL_->ScratchB1 ) ;
            sc = _String_ConvertStringToBackSlash ( sc, word->W_SourceCode, BUF_IX_SIZE ) ;
            scd = c_gd ( String_FilterMultipleSpaces ( sc, TEMPORARY ) ) ;
        }
        else scd = ( byte* ) "C Primitive" ;
        name = c_gd ( word->Name ) ;
        Printf ( ( byte* ) "\nSourceCode for %s.%s :> \n%s", word->S_ContainingNamespace ? word->S_ContainingNamespace->Name : ( byte* ) "", name, scd ) ;
    }
}

byte *
Word_GetLocalsSourceCodeString ( Word * word, byte * buffer )
{
    byte * start, * sc = word->W_SourceCode ;
    int64 s, e ;
    // find and reconstruct locals source code in a buffer and parse it with the regular locals parse code
    for ( s = 0 ; sc [ s ] && sc [ s ] != '(' ; s ++ ) ;
    if ( sc [ s ] )
    {
        start = & sc [ s + 1 ] ; // right after '(' is how _CSL_Parse_LocalsAndStackVariables is set up
        for ( e = s ; sc [ e ] && sc [ e ] != ')' ; e ++ ) ; // end = & sc [ e ] ;
        if ( sc [ e ] )
        {
            Strncpy ( buffer, start, e - s + 1 ) ;
            buffer [ e - s + 1 ] = 0 ;
        }
    }
    return buffer ;
}

void
Word_ShowSourceCode ( Word * word )
{
    _CSL_Source ( word, 0 ) ;
}

Word *
Word_GetFromCodeAddress ( byte * address )
{
    return Finder_FindWordFromAddress_AnyNamespace ( _Context_->Finder0, address ) ;
}

Word *
Word_GetFromCodeAddress_NoAlias ( byte * address )
{
    return Finder_FindWordFromAddress_AnyNamespace_NoAlias ( _Context_->Finder0, address ) ;
}

void
_CSL_WordName_Run ( byte * name )
{
    Block_Eval ( Finder_Word_FindUsing ( _Context_->Finder0, name, 0 )->Definition ) ;
}

#if 1

Word *
_CSL_TypedefAlias ( Word * word, byte * name, Namespace * addToNs )
{
    Word * alias = 0 ;
    if ( word && word->Definition )
    {
        alias = _Word_New ( name, word->W_MorphismAttributes | ALIAS, word->W_ObjectAttributes, word->W_LispAttributes, 0, addToNs, DICTIONARY ) ; // inherit type from original word
        word = Word_UnAlias ( word ) ;
        //Word_InitFinal ( alias, ( byte* ) word->Definition ) ;
        _Word_DefinitionStore ( alias, ( block ) word->Definition ) ;
        //if ( ! word->S_ContainingNamespace ) _Word_Add ( word, 0, addToNs ) ; // don't re-add if it is a recursive word cf. CSL_BeginRecursiveWord
        DObject_Finish ( alias ) ;
        //CSL_Finish_WordSourceCode ( _CSL_, word ) ;
        CSL_TypeStackReset ( ) ;
        _CSL_->LastFinished_Word = alias ;
        //_CSL_FinishWordDebugInfo ( word ) ;
        alias->S_CodeSize = word->S_CodeSize ;
        alias->W_AliasOf = word ;
        alias->Size = word->Size ;
        //alias->NamespaceStack = word->NamespaceStack ;
        //Strncpy ( alias->W_TypeSignatureString, word->W_TypeSignatureString, 8 ) ;
        if ( ! word->W_SourceCode )
        {
            CSL_Finish_WordSourceCode ( _CSL_, word ) ;
            alias->W_SourceCode = word->W_SourceCode ;
        }
        else CSL_Finish_WordSourceCode ( _CSL_, alias ) ;
        //alias->W_SourceCode = _CSL_GetSourceCode ( ) ;
    }
    else Exception ( USEAGE_ERROR, ABORT ) ;
    return alias ;
}
#endif
// alias : postfix

Word *
_CSL_Alias ( Word * word, byte * name, Namespace * addToNs )
{
    Word * alias = 0 ;
    if ( word && word->Definition )
    {
        alias = _Word_New ( name, word->W_MorphismAttributes | ALIAS, word->W_ObjectAttributes, word->W_LispAttributes, 1, addToNs, DICTIONARY ) ; // inherit type from original word
        word = Word_UnAlias ( word ) ;
        Word_InitFinal ( alias, ( byte* ) word->Definition ) ;
        alias->S_CodeSize = word->S_CodeSize ;
        alias->W_AliasOf = word ;
        alias->Size = word->Size ;
        alias->NamespaceStack = word->NamespaceStack ;
        Strncpy ( alias->W_TypeSignatureString, word->W_TypeSignatureString, 8 ) ;
    }
    else Exception ( USEAGE_ERROR, ABORT ) ;
    return alias ;
}

void
Do_TextMacro ( )
{
    Interpreter * interp = _Context_->Interpreter0 ;
    ReadLiner * rl = _Context_->ReadLiner0 ;
    ReadLiner_InsertTextMacro ( rl, interp->w_Word ) ;
    SetState ( interp, END_OF_LINE | END_OF_FILE | END_OF_STRING | DONE, false ) ; // reset a possible read newline
}

void
Do_StringMacro ( )
{
    Interpreter * interp = _Context_->Interpreter0 ;
    ReadLiner * rl = _Context_->ReadLiner0 ;
    String_InsertDataIntoStringSlot ( rl->InputLine, rl->ReadIndex, rl->ReadIndex, _String_UnBox ( ( byte* ) interp->w_Word->W_Value ) ) ; // size in bytes
    SetState ( interp, END_OF_LINE | END_OF_FILE | END_OF_STRING | DONE, false ) ; // reset a possible read newline
}

void
_CSL_Macro ( int64 mtype, byte * function )
{
    byte * name = _Word_Begin ( ), *macroString ;
    macroString = Parse_Macro ( mtype ) ;
    byte * code = String_New ( macroString, STRING_MEM ) ;
    //_DObject_New ( byte * name, uint64 value, uint64 ctype, uint64 ltype, uint64 ftype, byte * function, int64 arg, int64 addToInNs, Namespace * addToNs, uint64 allocType )
    _DObject_New ( name, ( uint64 ) code, IMMEDIATE, 0, 0, mtype, function, 0, 1, 0, DICTIONARY ) ;
}

Word *
Word_GetOriginalWord ( Word * word )
{
    Word * ow1, *ow0 ;
    for ( ow0 = word, ow1 = ow0->WL_OriginalWord ; ow1 && ( ow1 != ow1->WL_OriginalWord ) ; ow0 = ow1, ow1 = ow0->WL_OriginalWord ) ;
    if ( ! ow0 ) ow0 = word ;
    return ow0 ;
}

Word *
Word_UnAlias ( Word * word )
{
    if ( word ) while ( word->W_AliasOf ) word = word->W_AliasOf ;
    return word ;
}


