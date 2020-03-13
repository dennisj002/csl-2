
#include "../../include/csl.h"


// SEMI :: ';'
// NUMERIC :: DIGIT *
// _ID :: ALPHA_NUMERIC *
// TYPE_NAME :: _ID
// PTR :: '*' _ID | 'struct' _ID '*'
// ID :: _ID | PTR
// ARRAY :: ('[' NUMERIC ] ']')*
// ARRAY_FIELD :: ID ARRAY
// ARRAY_FIELDS :: ARRAY_FIELD (',' ARRAY_FIELD)*
// _ID_OR_ARRAY_FIELD : ID | ARRAY_FIELD
// ID_FIELDS : _ID_OR_ARRAY_FIELD ( ',' _ID_OR_ARRAY_FIELD )*
// TYPE_FIELD :: TYPE_NAME ID_FIELDS
// STRUCTURE :: '{' FIELD* '}'
// _STRUCT_OR_UNION_FIELD :: PTR | _ID STRUCTURE | _ID STRUCTURE ID_FIELDS | STRUCTURE ID_FIELDS 
// STRUCT_OR_UNION_FIELD :: ( 'struct' | 'union' ) _STRUCT_OR_UNION_FIELD
// FIELD :: STRUCT_OR_UNION_FIELD | TYPE_FIELD SEMI
// TYPEDEF :: 'typedef' FIELD
// TYPE :: 'type' FIELD 
// TYPED_FIELD :: TYPEDEF | TYPE

Word *
_CSL_TypedefAlias ( Word * word0, byte * name, Namespace * addToNs )
{
    Word * word = Word_UnAlias ( word0 ), * alias = 0 ;
    if ( word && word->Definition )
    {
        alias = _Word_New ( name, word->W_MorphismAttributes | ALIAS, word->W_ObjectAttributes, word->W_LispAttributes, 0, addToNs, DICTIONARY ) ; // inherit type from original word
        //word = Word_UnAlias ( word0 ) ;
        //Word_InitFinal ( alias, ( byte* ) word->Definition ) ;
        _Word_DefinitionStore ( alias, ( block ) word->Definition ) ;
        //if ( ! word->S_ContainingNamespace ) _Word_Add ( word, 0, addToNs ) ; // don't re-add if it is a recursive word cf. CSL_BeginRecursiveWord
        DObject_Finish ( alias ) ;
        CSL_TypeStackReset ( ) ;
        _CSL_->LastFinished_Word = alias ;
        //_CSL_FinishWordDebugInfo ( word ) ;
        alias->S_CodeSize = word->S_CodeSize ;
        alias->W_AliasOf = word ;
        alias->Size = word->Size ;
        alias->NamespaceStack = word->NamespaceStack ;
        //alias->W_SourceCode = word->W_SourceCode ;
        //Strncpy ( alias->W_TypeSignatureString, word->W_TypeSignatureString, 8 ) ;
        //alias->W_SC_WordList = word->W_SC_WordList ;
        if ( ! word->W_SourceCode )
        {
            CSL_Finish_WordSourceCode ( _CSL_, word ) ;
        }
#if 0       
        else
        {
            //CSL_Finish_WordSourceCode ( _CSL_, alias ) ;
            alias->W_SourceCode = _CSL_GetSourceCode ( ) ;
        }
#endif        
        alias->W_SourceCode = word->W_SourceCode ;
    }
    else Exception ( USEAGE_ERROR, ABORT ) ;
    return alias ;
}

Word *
Parse_Do_IdentifierAlias ( Context * cntx, byte * token )
{
    Compiler * compiler = cntx->Compiler0 ;
    TDSCI * tdsci = ( TDSCI * ) Stack_Top ( compiler->TDSCI_StructUnionStack ) ;
    Namespace * alias = _CSL_TypedefAlias ( tdsci->Tdsci_StructureUnion_Namespace, token, _CSL_Namespace_InNamespaceGet ( ) ) ; //tdsci->Tdsci_InNamespace ) ;
    TypeNamespace_Set ( alias, tdsci->Tdsci_StructureUnion_Namespace ) ;
    return alias ;
}
// we have read the idField : identifier = token

void
Parse_Identifier ( Context * cntx, int64 t_type )
{
    Compiler * compiler = cntx->Compiler0 ;
    TDSCI * tdsci = ( TDSCI * ) Stack_Top ( compiler->TDSCI_StructUnionStack ) ;
    byte *identifier = tdsci->TdsciToken ;
    Word * id, *addToNs ;

    if ( GetState ( tdsci, TDSCI_PRINT ) )
    {
        if ( t_type == TD_CLASS_FIELD )
        {
            ReadLiner * rl = _ReadLiner_ ;
            if ( ( ! GetState ( tdsci, TDSCI_POINTER ) && ( tdsci->Tdsci_Field_Type_Namespace->W_ObjectAttributes & STRUCTURE_TYPE ) ) )
                Object_PrintStructuredData ( &tdsci->DataPtr [ tdsci->Tdsci_Offset ], &rl->InputLine [ rl->ReadIndex ] ) ;
            else TDSCI_Print_Field ( cntx, t_type, tdsci->Tdsci_Field_Size ) ;
        }
    }
    else if ( t_type == TD_CLASS_FIELD )
    {
        // with a nameless struct or union identifier are added to the background namespace
        addToNs = ( tdsci->Tdsci_StructureUnion_Namespace && tdsci->Tdsci_StructureUnion_Namespace->Name ) ?
            tdsci->Tdsci_StructureUnion_Namespace : tdsci->Tdsci_InNamespace ;
        tdsci->Tdsci_Field_Object = id = DataObject_New ( TD_CLASS_FIELD, 0, identifier, 0, 0, 0, tdsci->Tdsci_Offset, tdsci->Tdsci_Field_Size, addToNs, 0, 0, 0 ) ;
        TypeNamespace_Set ( id, tdsci->Tdsci_Field_Type_Namespace ) ;
    }
    else if ( t_type == POST_STRUCTURE_NAME ) //&& GetState ( tdsci, TDSCI_STRUCTURE_COMPLETED ) )
    {
        if ( tdsci->Tdsci_StructureUnion_Namespace )
        {
            //if ( ( ! tdsci->Tdsci_StructureUnion_Namespace->Name ) || String_Equal ( tdsci->Tdsci_StructureUnion_Namespace->Name, "<unnamed>" ) )
            //    tdsci->Tdsci_StructureUnion_Namespace->Name = String_New ( identifier, DICTIONARY ) ;
            id = Parse_Do_IdentifierAlias ( cntx, identifier ) ; //else tdsci->Tdsci_TotalStructureNamespace->Name = String_New ( token, DICTIONARY ) ;
        }
        else
        {
            tdsci->Tdsci_StructureUnion_Namespace = id = DataObject_New ( CLASS_CLONE, 0, identifier, 0, 0, 0, 0, 0,
                tdsci->Tdsci_InNamespace, 0, - 1, - 1 ) ;
            Class_Size_Set ( id, tdsci->Tdsci_StructureUnion_Size ) ;
        }
        id->W_ObjectAttributes |= ( STRUCT | NAMESPACE ) ; //??
    }
    else if ( t_type == PRE_STRUCTURE_IDENTIFIER )
    {
        //if ( ( ! tdsci->Tdsci_StructureUnion_Namespace->Name ) || String_Equal ( tdsci->Tdsci_StructureUnion_Namespace->Name, "<unnamed>" ) )
        //    tdsci->Tdsci_StructureUnion_Namespace->Name = String_New ( identifier, DICTIONARY ) ;
        tdsci->Tdsci_StructureUnion_Namespace = id = DataObject_New ( CLASS, 0, identifier, 0, 0, 0, 0, 0, 0, 0, 0, - 1 ) ;
        id->W_ObjectAttributes |= ( STRUCT | NAMESPACE ) ; //??
    }
    else CSL_Parse_Error ( "Parse_Identifier : No identifier type given", identifier ) ;

    if ( _O_->Verbosity > 1 ) TDSCI_DebugPrintWord ( cntx, id ) ; // print class field
}

void
Parse_Array ( Context * cntx )
{
    Compiler * compiler = cntx->Compiler0 ;
    TDSCI * tdsci = ( TDSCI * ) Stack_Top ( compiler->TDSCI_StructUnionStack ) ;
    int64 arrayDimensions [ 32 ] ; // 32 : max dimensions for now
    int64 size = tdsci->Tdsci_Field_Size, i, arrayDimensionSize ;
    byte *token = tdsci->TdsciToken ;
    memset ( arrayDimensions, 0, sizeof (arrayDimensions ) ) ;

    for ( i = 0 ; 1 ; i ++ )
    {
        if ( token && ( token[0] == '[' ) )
        {
            CSL_InterpretNextToken ( ) ; // next token must be an integer for the array dimension size
            arrayDimensionSize = DataStack_Pop ( ) ;
            size = size * arrayDimensionSize ;
            tdsci->Tdsci_Field_Size = size ;
            token = TDSCI_ReadToken ( cntx ) ;
            if ( ! String_Equal ( ( char* ) token, "]" ) ) CSL_Exception ( SYNTAX_ERROR, 0, 1 ) ;
            else arrayDimensions [ i ] = arrayDimensionSize ;
            token = TDSCI_ReadToken ( cntx ) ;
        }
        else
        {
            if ( i )
            {

                tdsci->Tdsci_Field_Object->ArrayDimensions = ( int64 * ) Mem_Allocate ( tdsci->Tdsci_Field_Size, DICTIONARY ) ;
                MemCpy ( tdsci->Tdsci_Field_Object->ArrayDimensions, arrayDimensions, tdsci->Tdsci_Field_Size ) ;
                tdsci->Tdsci_Field_Object->ArrayNumberOfDimensions = i ;
            }
            break ;
        }
    }
}

Boolean
Parse_CheckForPointer ( Context * cntx )
{
    Compiler * compiler = cntx->Compiler0 ;
    TDSCI * tdsci = ( TDSCI * ) Stack_Top ( compiler->TDSCI_StructUnionStack ) ;
    byte *token, *token1, * token0 = tdsci->TdsciToken ; //TDSCI_ReadToken ( cntx ) ;
    if ( ( token0 [0] == '*' ) || ( ( token1 = Lexer_Peek_Next_NonDebugTokenWord ( cntx->Lexer0, 0, 0 ) ), ( token1 [0] == '*' ) ) )
    {
        tdsci->Tdsci_Field_Size = CELL ;
        SetState ( tdsci, TDSCI_POINTER, true ) ;
        token = TDSCI_ReadToken ( cntx ) ; // the '*'
        if ( token0 [0] != '*' ) token = TDSCI_ReadToken ( cntx ) ;

        return true ;
    }
    return false ;
}

void
Parse_Identifier_Or_Array_Field ( Context * cntx, int64 t_type )
{
    Compiler * compiler = cntx->Compiler0 ;
    TDSCI * tdsci = ( TDSCI * ) Stack_Top ( compiler->TDSCI_StructUnionStack ) ;
    byte *token = tdsci->TdsciToken ; //TDSCI_ReadToken ( cntx ) ;
    if ( token && ( token [0] != ';' ) )
    {
        Parse_CheckForPointer ( cntx ) ;
        //int64 itype = ( ( ! GetState ( tdsci, TDSCI_PRINT ) ) && ( GetState ( tdsci, TDSCI_POINTER ) ) ? TYPE_NAME : t_type ) ;
        Parse_Identifier ( cntx, t_type ) ;
        token = TDSCI_ReadToken ( cntx ) ;
        if ( token && ( token [0] == '[' ) )
        {

            Parse_Array ( cntx ) ;
        }
    }
}

void
Parse_Identifier_Fields ( Context * cntx, int64 t_type )
{
    Compiler * compiler = cntx->Compiler0 ;
    TDSCI * tdsci = ( TDSCI * ) Stack_Top ( compiler->TDSCI_StructUnionStack ) ;
    byte * token ;
    while ( 1 )
    {
        Parse_Identifier_Or_Array_Field ( cntx, t_type ) ;
        if ( ( tdsci->TdsciToken ) && ( tdsci->TdsciToken[0] == ',' ) ) token = TDSCI_ReadToken ( cntx ) ;

        else break ;
    }
}

// SEMI :: ';'
// NUMERIC :: DIGIT *
// _ID :: ALPHA_NUMERIC *
// TYPE_NAME :: _ID
// PTR :: '*' _ID | 'struct' _ID '*'
// ID :: _ID | PTR
// ARRAY :: ('[' NUMERIC ] ']')*
// ARRAY_FIELD :: ID ARRAY
// ARRAY_FIELDS :: ARRAY_FIELD (',' ARRAY_FIELD)*
// _ID_OR_ARRAY_FIELD : ID | ARRAY_FIELD
// ID_FIELDS : _ID_OR_ARRAY_FIELD ( ',' _ID_OR_ARRAY_FIELD )*
// TYPE_FIELD :: TYPE_NAME ID_FIELDS
// STRUCTURE :: '{' FIELD* '}'
// _STRUCT_OR_UNION_FIELD :: PTR | _ID STRUCTURE | _ID STRUCTURE ID_FIELDS | STRUCTURE ID_FIELDS 
// STRUCT_OR_UNION_FIELD :: ( 'struct' | 'union' ) _STRUCT_OR_UNION_FIELD
// FIELD :: STRUCT_OR_UNION_FIELD | TYPE_FIELD SEMI
// TYPEDEF :: 'typedef' FIELD
// TYPE :: 'type' FIELD 
// TYPED_FIELD :: TYPEDEF | TYPE

void
Parse_Type_Field ( Context * cntx )
{
    Compiler * compiler = cntx->Compiler0 ;
    TDSCI * tdsci = ( TDSCI * ) Stack_Top ( compiler->TDSCI_StructUnionStack ) ;
    byte * token = tdsci->TdsciToken ;
    Namespace * type0 = _Namespace_Find ( token, 0, 0 ) ; //&& ( token1 [0] == '*' ) )
    if ( type0 )
    {
        tdsci->Tdsci_Field_Type_Namespace = type0 ;
        tdsci->Tdsci_Field_Size = CSL_Get_Namespace_SizeVar_Value ( type0 ) ;
        if ( ! Parse_CheckForPointer ( cntx ) ) token = TDSCI_ReadToken ( cntx ) ;
        //Parse_Identifier ( cntx, TYPE_NAME ) ;
        Parse_Identifier_Fields ( cntx, TD_CLASS_FIELD ) ;
    }

    else CSL_Parse_Error ( "Parse_Type_Field : Can't find field namespace.", token ) ;
}
// '{' token may or may not have already been parsed

void
Parse_StructOrUnion ( Context * cntx ) // 'struct' or 'union'
{
    TDSCI * tdsci = TDSCI_GetTop ( cntx ) ;
    byte * token = tdsci->TdsciToken ;
    if ( ! token ) token = TDSCI_ReadToken ( cntx ) ; // give flexibility ; '{' token may or may not have already been parsed
    if ( String_Equal ( token, "struct" ) || String_Equal ( token, "union" ) ) token = TDSCI_ReadToken ( cntx ) ;
    if ( ( token [0] == '{' ) || ( token[1] == '{' ) || ( token[2] == '{' ) ) TDSCI_ReadToken ( cntx ) ; // consider ":{" and +:{" tokens
    do
    {
        Parse_Field ( cntx ) ;
        Parse_Inter_Struct_FieldAccounting ( cntx ) ;
        tdsci = TDSCI_GetTop ( cntx ) ;
        if ( tdsci->TdsciToken && ( tdsci->TdsciToken [0] == ';' ) ) TDSCI_ReadToken ( cntx ) ;
    }
    while ( tdsci->TdsciToken && ( tdsci->TdsciToken [0] != '}' ) ) ;

    if ( tdsci->TdsciToken[1] != ';' ) token = TDSCI_ReadToken ( cntx ) ;
    Parse_PostStruct_Accounting ( cntx ) ;
}

// SEMI :: ';'
// NUMERIC :: DIGIT *
// _ID :: ALPHA_NUMERIC *
// TYPE_NAME :: _ID
// PTR :: '*' _ID | 'struct' _ID '*'
// ID :: _ID | PTR
// ARRAY :: ('[' NUMERIC ] ']')*
// ARRAY_FIELD :: ID ARRAY
// ARRAY_FIELDS :: ARRAY_FIELD (',' ARRAY_FIELD)*
// _ID_OR_ARRAY_FIELD : ID | ARRAY_FIELD
// ID_FIELDS : _ID_OR_ARRAY_FIELD ( ',' _ID_OR_ARRAY_FIELD )*
// TYPE_FIELD :: TYPE_NAME ID_FIELDS
// STRUCTURE :: '{' FIELD* '}'
// _STRUCT_OR_UNION_FIELD :: PTR | _ID STRUCTURE | _ID STRUCTURE ID_FIELDS | STRUCTURE ID_FIELDS 
// STRUCT_OR_UNION_FIELD :: ( 'struct' | 'union' ) _STRUCT_OR_UNION_FIELD
// FIELD :: STRUCT_OR_UNION_FIELD | TYPE_FIELD SEMI
// TYPEDEF :: 'typedef' FIELD
// TYPE :: 'type' FIELD 
// TYPED_FIELD :: TYPEDEF | TYPE

// 'struct' or 'union' tokens should have already been parsed

void
Parse_StructOrUnion_Field ( Context * cntx, Namespace * ns, int64 state )
{
    TDSCI * tdsci = Parse_PreStruct_Accounting ( cntx, ns, state ) ;
    Namespace * type0 = 0 ;
    byte * token = ( tdsci->TdsciToken ? tdsci->TdsciToken : TDSCI_ReadToken ( cntx ) ) ;
    if ( String_Equal ( token, "struct" ) || String_Equal ( token, "union" ) ) token = TDSCI_ReadToken ( cntx ) ;
    if ( token )
    {
        if ( ( token [0] != '{' ) && ( token[1] != '{' ) )
        {
            if ( type0 && GetState ( tdsci, TDSCI_PRINT ) )
            {
                tdsci->Tdsci_Field_Type_Namespace = type0 ;
                TDSCI_Print_StructNameEtc ( cntx ) ;
            }
            if ( Parse_CheckForPointer ( cntx ) )
                Parse_Identifier_Fields ( cntx, TD_CLASS_FIELD ) ;
            else
            {
                Parse_Identifier ( cntx, PRE_STRUCTURE_IDENTIFIER ) ;
                //else Parse_Type_Field ( cntx ) ; //Parse_Identifier ( cntx, PRE_STRUCTURE_IDENTIFIER ) ;
                token = TDSCI_ReadToken ( cntx ) ;
            }
        }
        if ( ( token [0] == '{' ) || ( token[1] == '{' ) || ( token[2] == '{' ) ) // consider ":{" and +:{" tokens
        {

            Parse_StructOrUnion ( cntx ) ;
        }
        //else if ( token [0] != ';' ) CSL_Parse_Error ( "Parse_StructOrUnion_Field : No \'{\' token in struct/union field", token ) ;
    }
}

// SEMI :: ';'
// NUMERIC :: DIGIT *
// _ID :: ALPHA_NUMERIC *
// TYPE_NAME :: _ID
// PTR :: '*' _ID | 'struct' _ID '*'
// ID :: _ID | PTR
// ARRAY :: ('[' NUMERIC ] ']')*
// ARRAY_FIELD :: ID ARRAY
// ARRAY_FIELDS :: ARRAY_FIELD (',' ARRAY_FIELD)*
// _ID_OR_ARRAY_FIELD : ID | ARRAY_FIELD
// ID_FIELDS : _ID_OR_ARRAY_FIELD ( ',' _ID_OR_ARRAY_FIELD )*
// TYPE_FIELD :: TYPE_NAME ID_FIELDS
// STRUCTURE :: '{' FIELD* '}'
// _STRUCT_OR_UNION_FIELD :: PTR | _ID STRUCTURE | _ID STRUCTURE ID_FIELDS | STRUCTURE ID_FIELDS 
// STRUCT_OR_UNION_FIELD :: ( 'struct' | 'union' ) _STRUCT_OR_UNION_FIELD
// FIELD :: STRUCT_OR_UNION_FIELD | TYPE_FIELD SEMI
// TYPEDEF :: 'typedef' FIELD
// TYPE :: 'type' FIELD
// TYPED_FIELD :: TYPEDEF | TYPE

void
Parse_Field ( Context * cntx )
{
    TDSCI * tdsci = TDSCI_GetTop ( cntx ) ;
    int64 structOrUnionState = 0 ;
    byte * token = tdsci->TdsciToken ;
    if ( ! token ) token = TDSCI_ReadToken ( cntx ) ;
    if ( token && token[0] != ';' )
    {
        if ( String_Equal ( ( char* ) token, "struct" ) ) structOrUnionState = TDSCI_STRUCT ;
        else if ( String_Equal ( ( char* ) token, "union" ) ) structOrUnionState = TDSCI_UNION ;
        if ( structOrUnionState )
        {
            Parse_StructOrUnion_Field ( cntx, 0, structOrUnionState ) ;
            SetState ( tdsci, TDSCI_UNION | TDSCI_STRUCT, false ) ;
        }
        else
        {

            Parse_Type_Field ( cntx ) ;
            token = TDSCI_ReadToken ( cntx ) ; // ';'
        }
    }
    //if ( ! String_Equal ( tdsci->TdsciToken, "};" ) ) Parse_Identifier_Fields ( cntx, POST_STRUCTURE_NAME ) ; //
}

int64
CSL_Parse_A_Typed_Field ( Context * cntx )
{
    if ( ! cntx ) cntx = _Context_ ;
    Word * word = cntx->CurrentEvalWord ;
    TDSCI * tdsci = TDSCI_Start ( cntx, word, 0, 0 ) ;
    Parse_Field ( cntx ) ;
    tdsci = TDSCI_Finalize ( cntx ) ;

    return tdsci->Tdsci_StructureUnion_Size ;
}

// Struct : in functions here refers to struct or union types

TDSCI *
Parse_PreStruct_Accounting ( Context * cntx, Namespace * ns, int64 state )
{
    TDSCI * ctdsci = 0, *tdsci ;
    if ( ! cntx ) cntx = _Context_ ;
    SetState ( cntx->Compiler0, TDSCI_PARSING, true ) ;
    int64 depth = Stack_Depth ( cntx->Compiler0->TDSCI_StructUnionStack ) ;
    ctdsci = TDSCI_GetTop ( cntx ) ; // current -> previous
    tdsci = TDSCI_Push_New ( cntx ) ; // adding one to the Stack
    if ( ns ) tdsci->Tdsci_StructureUnion_Namespace = ns ;
    if ( ctdsci ) // previous tdsci - Tdsci_PreStructureUnion_Namespace
    {
        tdsci->Tdsci_Offset = ctdsci->Tdsci_Offset ;
        tdsci->TdsciToken = ctdsci->TdsciToken ;
        tdsci->State = ctdsci->State ;
        tdsci->Tdsci_InNamespace = ctdsci->Tdsci_StructureUnion_Namespace ? ctdsci->Tdsci_StructureUnion_Namespace : _CSL_Namespace_InNamespaceGet ( ) ;
        if ( ! tdsci->Tdsci_StructureUnion_Namespace ) tdsci->Tdsci_StructureUnion_Namespace = ctdsci->Tdsci_StructureUnion_Namespace ;
        tdsci->DataPtr = ctdsci->DataPtr ;
    }
    else tdsci->Tdsci_InNamespace = _CSL_Namespace_InNamespaceGet ( ) ;
    //if ( ( ! tdsci->Tdsci_StructureUnion_Namespace ) && ( ! GetState ( tdsci, TDSCI_PRINT ) ) ) tdsci->Tdsci_StructureUnion_Namespace =
    //    DataObject_New ( CLASS, 0, "<unnamed>", 0, 0, 0, 0, 0, tdsci->Tdsci_InNamespace, 0, 0, - 1 ) ;
    tdsci->State |= state ;

    return tdsci ;
}

TDSCI *
Parse_PostStruct_Accounting ( Context * cntx )
{
    TDSCI * ctdsci = 0, *tdsci ;
    int64 depth = Stack_Depth ( cntx->Compiler0->TDSCI_StructUnionStack ) ;
    if ( depth > 1 ) ctdsci = TDSCI_Pop ( cntx ) ;
    tdsci = TDSCI_GetTop ( cntx ) ;
    if ( ctdsci ) // always should be there but check anyway
    {

        if ( ! tdsci->Tdsci_StructureUnion_Namespace ) tdsci->Tdsci_StructureUnion_Namespace = ctdsci->Tdsci_StructureUnion_Namespace ;
        tdsci->Tdsci_StructureUnion_Size += ctdsci->Tdsci_StructureUnion_Size ;
        tdsci->Tdsci_Offset = tdsci->Tdsci_StructureUnion_Size ;
        tdsci->TdsciToken = ctdsci->TdsciToken ;
        tdsci->Tdsci_Field_Size = 0 ;
        tdsci->State = ctdsci->State ;
    }
    return tdsci ;
}

void
Parse_Inter_Struct_FieldAccounting ( Context * cntx )
{
    TDSCI * tdsci = TDSCI_GetTop ( cntx ) ;
    if ( GetState ( tdsci, TDSCI_UNION ) )
    {
        if ( tdsci->Tdsci_Field_Size > tdsci->Tdsci_StructureUnion_Size ) tdsci->Tdsci_StructureUnion_Size = tdsci->Tdsci_Field_Size ;
    }
    else // not TDSCI_UNION // field may be a struct
    {

        tdsci->Tdsci_StructureUnion_Size += tdsci->Tdsci_Field_Size ;
        tdsci->Tdsci_Offset += tdsci->Tdsci_Field_Size ;
    }
}

void
TDSCI_Init ( TypeDefStructCompileInfo * tdsci )
{
}

TypeDefStructCompileInfo *
TypeDefStructCompileInfo_New ( Context * cntx, uint64 allocType )
{
    if ( ! cntx ) cntx = _Context_ ;
    TypeDefStructCompileInfo * tdsci = ( TypeDefStructCompileInfo * ) Mem_Allocate ( sizeof (TypeDefStructCompileInfo ), allocType ) ;
    TDSCI_Init ( tdsci ) ;

    return tdsci ;
}

TypeDefStructCompileInfo *
TDSCI_Push_New ( Context * cntx )
{
    TypeDefStructCompileInfo *tdsci = TypeDefStructCompileInfo_New ( cntx, CONTEXT ) ;
    Stack_Push ( cntx->Compiler0->TDSCI_StructUnionStack, ( int64 ) tdsci ) ;

    return tdsci ;
}

TDSCI *
TDSCI_Start ( Context * cntx, Word * word, byte* objectBitData, int64 stateFlags )
{
    TypeDefStructCompileInfo * tdsci ;
    TDSCI_STACK_INIT ( cntx ) ;
    tdsci = TDSCI_Push_New ( cntx ) ;
    if ( stateFlags & TDSCI_CLONE_FLAG )
    {
        tdsci->Tdsci_StructureUnion_Namespace = word ;
        tdsci->Tdsci_InNamespace = _CSL_Namespace_InNamespaceGet ( ) ;
        tdsci->Tdsci_Offset = _Namespace_VariableValueGet ( tdsci->Tdsci_InNamespace, ( byte* ) "size" ) ; // allows for cloning - prototyping
        tdsci->Tdsci_StructureUnion_Size = tdsci->Tdsci_Offset ;
    }
    else if ( ! ( stateFlags & TDSCI_PRINT ) ) tdsci->Tdsci_InNamespace = Is_NamespaceType ( word ) ? word : _CSL_Namespace_InNamespaceGet ( ) ;
    tdsci->State |= stateFlags ;
    tdsci->DataPtr = objectBitData ;
    SetState ( _Compiler_, TDSCI_PARSING, true ) ;
    Lexer_SetTokenDelimiters ( cntx->Lexer0, ( byte* ) " ,\n\r\t", CONTEXT ) ;
    if ( ! ( stateFlags & TDSCI_PRINT ) )
    {

        CSL_Lexer_SourceCodeOn ( ) ;
        CSL_InitSourceCode_WithName ( _CSL_, word ? word->Name : 0, 1 ) ;
    }
    return tdsci ;
}

#if 1

void
Parse_Post_Struct_Identifiers ( Context * cntx )
{
    TDSCI * tdsci = TDSCI_GetTop ( cntx ) ;
    byte * token = tdsci->TdsciToken ;
    if ( token && ( token[0] != ';' ) && ( ! String_Equal ( token, "};" ) ) )
    {

        Parse_Identifier_Fields ( cntx, POST_STRUCTURE_NAME ) ;
    }

}
#endif

TDSCI *
TDSCI_Finalize ( Context * cntx )
{
    //if ( ! String_Equal ( tdsci->TdsciToken, "};" ) ) Parse_Identifier_Fields ( cntx, POST_STRUCTURE_NAME ) ; //
    Parse_Post_Struct_Identifiers ( cntx ) ;
    TDSCI * tdsci = TDSCI_GetTop ( cntx ) ;
    //TDSCI * tdsci = Parse_PostStruct_Accounting ( cntx ) ;
    CSL_Finish_WordSourceCode ( _CSL_, tdsci->Tdsci_StructureUnion_Namespace ) ;
    if ( ! GetState ( tdsci, TDSCI_PRINT ) ) Class_Size_Set ( tdsci->Tdsci_StructureUnion_Namespace ? tdsci->Tdsci_StructureUnion_Namespace : tdsci->Tdsci_InNamespace, tdsci->Tdsci_StructureUnion_Size ) ;
    SetState ( _Compiler_, TDSCI_PARSING, false ) ;

    return tdsci ; // should return the initial tdsci from TDSCI_Start with the final fields filled in
}

TDSCI *
TDSCI_GetTop ( Context * cntx )
{
    TDSCI * tdsci ;

    return tdsci = ( TDSCI * ) Stack_Top ( CONTEXT_TDSCI_STACK ( cntx ) ) ;
}

TDSCI *
TDSCI_Pop ( Context * cntx )
{
    TDSCI * tdsci ;

    return tdsci = ( TDSCI* ) Stack_Pop ( CONTEXT_TDSCI_STACK ( cntx ) ) ;
}

void
TDSCI_STACK_INIT ( Context * cntx )
{

    Stack_Init ( CONTEXT_TDSCI_STACK ( cntx ) ) ;
}

void
TDSCI_Print_StructNameEtc ( Context * cntx )
{

    TDSCI * tdsci = TDSCI_GetTop ( cntx ) ;
    Printf ( "\n\t%16s : %s : size = %d : at %016lx",
        tdsci->Tdsci_Field_Type_Namespace->Name, tdsci->TdsciToken, CSL_Get_ObjectByteSize ( tdsci->Tdsci_Field_Type_Namespace ),
        &tdsci->DataPtr [ tdsci->Tdsci_Offset ] ) ;
}

void
TDSCI_Print_StructField ( Context * cntx )
{

    TDSCI * tdsci = TDSCI_GetTop ( cntx ) ;
    //CSL_Get_ObjectByteSize ( tdsci->Tdsci_Field_Type_Namespace ) ;
    //Printf ( "\n0x%016lx\t%16s : size = %d : at %016lx", &tdsci->DataPtr [ tdsci->Tdsci_Offset ],
    TDSCI_Print_StructNameEtc ( cntx ) ;
    //Object_PrintStructuredData ( ( byte* ) & tdsci->DataPtr [ tdsci->Tdsci_Offset ], tdsci->Tdsci_Field_Type_Namespace->W_SourceCode ) ;
    byte * token = TDSCI_ReadToken ( cntx ) ;
    _Object_Continue_PrintStructuredData ( cntx, &tdsci->DataPtr [ tdsci->Tdsci_Offset ] ) ; //byte * objectBits ) ;
    CSL_NewLine ( ) ;
}

void
TDSCI_Print_Field ( Context * cntx, int64 t_type, int64 size )
{
#define FRMT "\n0x%016lx  %-16s%-3s  %-24s"    
#define FRMT_CHAR2 "0x%02x"    
    TDSCI * tdsci = TDSCI_GetTop ( cntx ) ;
    byte *token = tdsci->TdsciToken, *format ; //, *format1 = "\n0x%016lx\t%12s%s\t%16s" ;
    int64 value ;
    if ( tdsci->Tdsci_Field_Type_Namespace )
    {
#if 0
        if ( ( tdsci->Tdsci_Field_Type_Namespace->W_ObjectAttributes && STRUCTURE )
            && ( ! GetState ( tdsci, TDSCI_POINTER ) ) && ( tdsci->Tdsci_Field_Type_Namespace->W_SourceCode ) )
        {
            TDSCI_Print_StructField ( cntx ) ;
            //token = TDSCI_ReadToken ( cntx ) ;
            //_Object_Continue_PrintStructuredData ( cntx, &tdsci->DataPtr [ tdsci->Tdsci_Offset ] ) ; //byte * objectBits ) ;
            //ReadLiner * rl = _ReadLiner_ ;
            //Object_PrintStructuredData ( &tdsci->DataPtr [ tdsci->Tdsci_Offset ], &(rl->InputLine [ rl->ReadIndex ]) ) ; //&_ReadLiner_->InputStringCurrent[_ReadLiner_->InputStringIndex] ) ;

            //CSL_Parse_A_Typed_Field ( cntx, 0, tdsci->DataPtr ) ;
            //Parse_Structure ( cntx ) ;
            //token = TDSCI_ReadToken ( cntx ) ;
            //Parse_Field ( cntx ) ;
        }
        else
#endif            
        {
            switch ( size )
            {
                case 1:
                {
                    format = ( byte* ) FRMT " = \'%c\' : " FRMT_CHAR2 ;
                    value = * ( ( int8* ) ( &tdsci->DataPtr [ tdsci->Tdsci_Offset ] ) ) ;
                    break ;
                }
                case 2:
                {
                    format = ( byte* ) FRMT " = 0x%04x" ;
                    value = * ( ( int16* ) ( &tdsci->DataPtr [ tdsci->Tdsci_Offset ] ) ) ;
                    break ;
                }
                case 4:
                {
                    format = ( byte* ) FRMT " = 0x%08lx" ;
                    value = * ( ( int32* ) ( &tdsci->DataPtr [ tdsci->Tdsci_Offset ] ) ) ;
                    break ;
                }
                default:
                case CELL:
                {
                    format = ( byte* ) FRMT " = 0x%016lx" ;
                    value = * ( ( int64* ) ( &tdsci->DataPtr [ tdsci->Tdsci_Offset ] ) ) ;
                    break ;
                }
            }
            if ( size == 1 )
            {
                Printf ( format, &tdsci->DataPtr [ tdsci->Tdsci_Offset ], tdsci->Tdsci_Field_Type_Namespace->Name,
                    ( GetState ( tdsci, TDSCI_POINTER ) ? " * " : "" ), token, value, value ) ;
            }
            else Printf ( format, &tdsci->DataPtr [ tdsci->Tdsci_Offset ], tdsci->Tdsci_Field_Type_Namespace->Name,
                ( GetState ( tdsci, TDSCI_POINTER ) ? " * " : "" ), token, value ) ;

            if ( ! ( t_type & ( POST_STRUCTURE_NAME | PRE_STRUCTURE_IDENTIFIER ) ) ) tdsci->Tdsci_Field_Size = size ;
            SetState ( tdsci, TDSCI_POINTER, false ) ;
        }
    }
}

void
TDSCI_DebugPrintWord ( Context * cntx, Word * word )
{
    TDSCI * tdsci = TDSCI_GetTop ( cntx ) ;

    if ( word ) Printf ( ( byte* ) "\n%s.%s : field size = %ld : structure size = %ld : total size = %ld : offset == %ld : at %s",
        ( word->TypeNamespace ? word->TypeNamespace->Name : word->S_ContainingNamespace->Name ), word->Name, tdsci->Tdsci_Field_Size, tdsci->Tdsci_StructureUnion_Size,
        tdsci->Tdsci_StructureUnion_Size, tdsci->Tdsci_Offset, Context_Location ( ) ) ;
}

Boolean
Parser_Check_Do_CommentWord ( Word * word )
{
    if ( word && ( word->W_MorphismAttributes & ( COMMENT | DEBUG_WORD ) ) )
    {
        Interpreter_DoWord ( _Interpreter_, word, _Lexer_->TokenStart_ReadLineIndex, _Lexer_->SC_Index ) ;
        return true ;
    }

    else return false ;
}

Boolean
Parser_Check_Do_Debug_Token ( byte * token )
{
    Word * word = Finder_Word_FindUsing ( _Finder_, token, 0 ) ;

    return Parser_Check_Do_CommentWord ( word ) ;
}

byte *
TDSCI_ReadToken ( Context * cntx )
{
    TDSCI * tdsci = TDSCI_GetTop ( cntx ) ;
#if 1   
    //if ( Is_DebugOn )
    {
        do
        {
            tdsci->TdsciToken = Lexer_ReadToken ( cntx->Lexer0 ) ;
            if ( ! ( GetState ( tdsci, TDSCI_PRINT ) ) ) DEBUG_SETUP_TOKEN ( tdsci->TdsciToken, 0 ) ;
            //if ( Is_DebugOn ) _Printf ( ( byte* ) "%s ", tdsci->TdsciToken ) ;
        }

        while ( Parser_Check_Do_Debug_Token ( tdsci->TdsciToken ) ) ;
        //tdsci = TDSCI_GetTop ( cntx ) ;
        tdsci->LineNumber = cntx->Lexer0->LineNumber ;
        tdsci->Token_StartIndex = cntx->Lexer0->TokenStart_FileIndex ;
        tdsci->Token_EndIndex = cntx->Lexer0->TokenEnd_FileIndex ;
    }
    //else tdsci->TdsciToken = Lexer_ReadToken ( cntx->Lexer0 ) ;
#endif    
    return tdsci->TdsciToken ;
}

void
CSL_Parse_Error ( byte * msg, byte * token )
{
    byte * buffer = Buffer_Data_Cleared ( _CSL_->ScratchB1 ) ;
    snprintf ( ( char* ) buffer, BUFFER_IX_SIZE, "\nCSL_Parse_Error : %s : \'%s\' at %s", ( char* ) msg, token, Context_Location ( ) ) ;
    //TDSCI * tdsci = TDSCI_GetTop ( _Context_ ) ;
    _Debugger_->Token = token ;
    _SyntaxError ( ( byte* ) buffer, 1 ) ; // else structure component size error
}

