#include "../../include/csl.h"

#define P_DEBUG 0
#define PP_DEBUG 1


#define PRE_STRUCTURE_NAME 1
#define POST_STRUCTURE_NAME 2
#define TYPE_NAME 4

// _ID :: ALPHA_NUMERIC *
// DIGIT :: NUMERIC *
// ALIAS_ID :: _ID "," ID
// ID :: _ID | ALIAS_ID
// ID_FIELD : ID | ARRAY_FIELD
// ARRAY :: ('[' DIGIT ] ']') | ARRAY*
// ARRAY_FIELD :: ( _ID ARRAY ) | ARRAY_FIELD ',' ARRAY_FIELD
// TYPE_FIELD :: TYPE_NAME '*'? ID_FIELD ';'
// STRUCTURE :: '{' FIELD * '}'
// STRUCT_FIELD :: ('struct' | 'union') ( _ID STRUCTURE ID_FIELD | STRUCTURE ID_FIELD | _ID STRUCTURE ) ';'
// FIELD :: STRUCT_FIELD | TYPE_FIELD
// TYPEDEF :: 'typedef' STRUCT_FIELD
// TYPE :: 'type' TYPE_FIELD

void
TDSCI_DebugPrintWord ( TypeDefStructCompileInfo *tdsci, Word * word )
{
    if ( word ) _Printf ( ( byte* ) "\n%s.%s : field size = %ld : structure size = %ld : total size = %ld : offset == %ld : at %s",
        ( word->TypeNamespace ? word->TypeNamespace->Name : word->S_ContainingNamespace->Name ), word->Name, tdsci->Tdsci_Field_Size, tdsci->Tdsci_StructureUnion_Size,
        tdsci->Tdsci_TotalSize, tdsci->Tdsci_Offset, Context_Location ( ) ) ;
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
TDSCI_ReadToken ( TypeDefStructCompileInfo *tdsci )
{
#if 1   
    //if ( Is_DebugOn )
    {
        do
        {
            tdsci->TdsciToken = Lexer_ReadToken ( _Lexer_ ) ;
            //if ( Is_DebugOn ) _Printf ( ( byte* ) "%s ", tdsci->TdsciToken ) ;
        }
        while ( Parser_Check_Do_Debug_Token ( tdsci->TdsciToken ) ) ;
    }
    //else tdsci->TdsciToken = Lexer_ReadToken ( _Lexer_ ) ;
#endif    
    return tdsci->TdsciToken ;
}

void
TDSCI_Init ( TypeDefStructCompileInfo * tdsci )
{
    if ( tdsci->BackgroundNamespace ) memset ( tdsci, 0, sizeof (TypeDefStructCompileInfo ) ) ;
    tdsci->BackgroundNamespace = _CSL_Namespace_InNamespaceGet ( ) ;
    SetState ( _Compiler_, TDSCI_PARSING, true ) ;
}

TypeDefStructCompileInfo *
TypeDefStructCompileInfo_New ( )
{
    TypeDefStructCompileInfo *tdsci = ( TypeDefStructCompileInfo * ) Mem_Allocate ( sizeof (TypeDefStructCompileInfo ), CONTEXT ) ;
    TDSCI_Init ( tdsci ) ;
    return tdsci ;
}

void
CSL_Parse_Error ( byte * msg, byte * token )
{
    byte * buffer = Buffer_Data_Cleared ( _CSL_->ScratchB1 ) ;
    sprintf ( ( char* ) buffer, "\nCSL_Parse_ClassStructure : %s : \'%s\' at %s", ( char* ) msg, token, Context_Location ( ) ) ;
    _SyntaxError ( ( byte* ) buffer, 1 ) ; // else structure component size error
}

void
Parse_ArrayField ( TypeDefStructCompileInfo * tdsci )
{
    int64 arrayDimensions [ 32 ] ; // 32 : max dimensions for now
    int64 size = tdsci->Tdsci_Field_Size, i, arrayDimensionSize ;
    byte *token = tdsci->TdsciToken ;
    memset ( arrayDimensions, 0, sizeof (arrayDimensions ) ) ;

    for ( i = 0 ; 1 ; i ++ )
    {
        if ( token && String_Equal ( ( char* ) token, "[" ) )
        {
            CSL_InterpretNextToken ( ) ; // next token must be an integer for the array dimension size
            arrayDimensionSize = DataStack_Pop ( ) ;
            size = size * arrayDimensionSize ;
            tdsci->Tdsci_Field_Size = size ;
            token = TDSCI_ReadToken ( tdsci ) ;
            if ( ! String_Equal ( ( char* ) token, "]" ) ) CSL_Exception ( SYNTAX_ERROR, 0, 1 ) ;
            else arrayDimensions [ i ] = arrayDimensionSize ;
            token = TDSCI_ReadToken ( tdsci ) ;
        }
        else
        {
            if ( i )
            {
                tdsci->Tdsci_Field_Object->ArrayDimensions = ( int64 * ) Mem_Allocate ( tdsci->Tdsci_Field_Size, DICTIONARY ) ;
                MemCpy ( tdsci->Tdsci_Field_Object->ArrayDimensions, arrayDimensions, tdsci->Tdsci_Field_Size ) ;
                tdsci->Tdsci_Field_Object->ArrayNumberOfDimensions = i ;
                // print array field
                //if ( dataPtr ) _Printf ( (byte*) "\n\t%s\t%s [ %d ]" = %lx", token0, token1, arrayDimensionSize, (sizeof token0) data [ aoffset ]) ;
            }
            break ;
        }
    }
}

void
TDSCI_Print_Field ( TypeDefStructCompileInfo * tdsci, int64 size )
{
    byte *token = tdsci->TdsciToken, *format ;
    int64 value ;
    switch ( size )
    {
        case 1:
        {
            format = ( byte* ) "\n0x%016lx\t%s\t%s = 0x%02x" ;
            value = * ( ( int8* ) ( &tdsci->DataPtr [ tdsci->Tdsci_Offset ] ) ) ;
            //_Printf ( format, &tdsci->DataPtr [ tdsci->Tdsci_Offset ], tdsci->Tdsci_Field_Type_Namespace->Name, token, *( ( int8* ) ( &tdsci->DataPtr [ tdsci->Tdsci_Offset ] ) ) ) ;
            break ;
        }
        case 2:
        {
            format = ( byte* ) "\n0x%016lx\t%s\t%s = 0x%04x" ;
            value = * ( ( int16* ) ( &tdsci->DataPtr [ tdsci->Tdsci_Offset ] ) ) ;
            //_Printf ( format, &tdsci->DataPtr [ tdsci->Tdsci_Offset ], tdsci->Tdsci_Field_Type_Namespace->Name, token, *( ( int16* ) ( &tdsci->DataPtr [ tdsci->Tdsci_Offset ] ) ) ) ;
            break ;
        }
        case 4:
        {
            format = ( byte* ) "\n0x%016lx\t%s\t%s = 0x%08x" ;
            value = * ( ( int32* ) ( &tdsci->DataPtr [ tdsci->Tdsci_Offset ] ) ) ;
            //_Printf ( format, &tdsci->DataPtr [ tdsci->Tdsci_Offset ], tdsci->Tdsci_Field_Type_Namespace->Name, token, *( ( int32* ) ( &tdsci->DataPtr [ tdsci->Tdsci_Offset ] ) ) ) ;
            break ;
        }
        default:
        case CELL:
        {
            format = ( byte* ) "\n0x%016lx\t%s\t%s = 0x%016lx" ;
            value = * ( ( int64* ) ( &tdsci->DataPtr [ tdsci->Tdsci_Offset ] ) ) ;
            break ;
        }
    }
    //_Printf ( format, &tdsci->DataPtr [ tdsci->Tdsci_Offset ], tdsci->Tdsci_Field_Type_Namespace->Name, token, *( ( int64* ) ( &tdsci->DataPtr [ tdsci->Tdsci_Offset ] ) ) ) ;
    _Printf ( format, &tdsci->DataPtr [ tdsci->Tdsci_Offset ], tdsci->Tdsci_Field_Type_Namespace->Name, token, value ) ;
    tdsci->Tdsci_Field_Size = size ;
}

Word *
Parse_Do_IdentifierAlias ( TypeDefStructCompileInfo * tdsci, byte * token )
{
    Word * id ;
    tdsci->Tdsci_StructureUnion_Namespace = id = _CSL_TypedefAlias ( tdsci->Tdsci_TotalStructureNamespace, token,
        tdsci->BackgroundNamespace ) ;
    Class_Size_Set ( id, tdsci->Tdsci_StructureUnion_Size ) ;
    TypeNamespace_Set ( id, tdsci->Tdsci_TotalStructureNamespace ) ;
    return id ;
}
// we have read the idField name = token

void
Parse_Do_Identifier ( TypeDefStructCompileInfo * tdsci, int64 t_type, int64 size )
{
    byte *token = tdsci->TdsciToken ;
    Word * id, *addToNs ;
    if ( GetState ( tdsci, TDSCI_PRINT ) )
    {
        if ( t_type == TYPE_NAME ) TDSCI_Print_Field ( tdsci, size ) ;
    }
    else if ( ( t_type == POST_STRUCTURE_NAME ) && GetState ( tdsci, TDSCI_STRUCTURE_COMPLETED ) )
    {
        //Finder_SetQualifyingNamespace ( _Finder_, tdsci->Tdsci_TotalStructureNamespace ) ;
        if ( tdsci->Tdsci_TotalStructureNamespace )
        {
            if ( ! tdsci->Tdsci_TotalStructureNamespace->Name )
                tdsci->Tdsci_TotalStructureNamespace->Name = String_New ( token, DICTIONARY ) ;
            Parse_Do_IdentifierAlias ( tdsci, token ) ; //else tdsci->Tdsci_TotalStructureNamespace->Name = String_New ( token, DICTIONARY ) ;
        }
        else
        {
            //_CSL_Namespace_InNamespaceSet ( tdsci->Tdsci_StructureUnion_Namespace ) ;
            tdsci->Tdsci_StructureUnion_Namespace = id = DataObject_New ( CLASS_CLONE, 0, token, 0, 0, 0, 0, 0,
                tdsci->Tdsci_TotalStructureNamespace, 0, - 1, - 1 ) ;
            Class_Size_Set ( id, tdsci->Tdsci_StructureUnion_Size ) ;
            id->W_ObjectAttributes |= ( STRUCTURE ) ; //??
        }
        Class_Size_Set ( tdsci->Tdsci_TotalStructureNamespace, tdsci->Tdsci_TotalSize ) ; //>Tdsci_StructureUnion_Size ) ;
    }
    else
    {
        if ( t_type == PRE_STRUCTURE_NAME )
        {
            tdsci->Tdsci_StructureUnion_Namespace = id = DataObject_New ( CLASS, 0, token, 0, 0, 0, 0, 0, 0, 0, 0, - 1 ) ;
            tdsci->Tdsci_TotalStructureNamespace = id ;
            id->W_ObjectAttributes |= ( STRUCTURE ) ;

        }
        else // if ( t_type == TYPE_NAME )
        {
            addToNs = tdsci->Tdsci_StructureUnion_Namespace->Name ? tdsci->Tdsci_StructureUnion_Namespace : tdsci->Tdsci_TotalStructureNamespace ;
            tdsci->Tdsci_Field_Object = id = DataObject_New ( CLASS_FIELD, 0, token, 0, 0, 0, tdsci->Tdsci_Offset, size, addToNs, 0, 0, 0 ) ;
            TypeNamespace_Set ( id, tdsci->Tdsci_Field_Type_Namespace ) ;
            tdsci->Tdsci_Field_Size = size ;
        }
    }
    if ( _O_->Verbosity > 1 ) TDSCI_DebugPrintWord ( tdsci, id ) ; // print class field
    //if ( dataPtr ) _Printf ( (byte*) "\n\t%s\t%s" = %lx", token0, token1, (sizeof token0) data [ offset ]) ;
}

void
Parse_Identifier_Field ( TypeDefStructCompileInfo * tdsci, int64 t_type, int64 size )
{
    byte *token ;
    token = tdsci->TdsciToken ;
next:
    while ( token && ( token [0] != ';' ) && ( token [0] != '}' ) )
    {
        Parse_Do_Identifier ( tdsci, t_type, size ) ;
        token = TDSCI_ReadToken ( tdsci ) ;
        while ( token && ( token [0] == ',' ) )
        {
            token = TDSCI_ReadToken ( tdsci ) ;
            if ( token && ( token [0] == '*' ) )
            {
                size = CELL ;
                tdsci->Tdsci_Field_Size = size ;
                token = TDSCI_ReadToken ( tdsci ) ;
                Parse_Identifier_Field ( tdsci, TYPE_NAME, size ) ;
                token = tdsci->TdsciToken ;
                goto next ;
            }
            else
            {
                Parse_Do_Identifier ( tdsci, t_type, 0 ) ;
                token = TDSCI_ReadToken ( tdsci ) ;
            }
        }
        if ( token && ( token [0] == '[' ) )
        {
            Parse_ArrayField ( tdsci ) ;
            if ( tdsci->TdsciToken[0] != ';' ) token = TDSCI_ReadToken ( tdsci ) ;
            else token = tdsci->TdsciToken ;
        }
    }
    // print class field
    //if ( dataPtr ) _Printf ( (byte*) "\n\t%s\t%s" = %lx", token0, token1, (sizeof token0) data [ offset ]) ;
    //return rtn ;
}

void
Parse_Structure ( TypeDefStructCompileInfo * tdsci )
{
    Lexer * lexer = _Lexer_ ;
    if ( GetState ( tdsci, TDSCI_CLONE_FLAG ) ) //cloneFlag )
    {
        tdsci->Tdsci_Offset = _Namespace_VariableValueGet ( tdsci->BackgroundNamespace, ( byte* ) "size" ) ; // allows for cloning - prototyping
        tdsci->Tdsci_TotalSize = tdsci->Tdsci_Offset ;
        SetState ( tdsci, TDSCI_CLONE_FLAG, false ) ;
    }
    Lexer_SetTokenDelimiters ( lexer, ( byte* ) " ,\n\r\t", COMPILER_TEMP ) ;
    tdsci->Tdsci_StructureUnion_Size = 0 ;

    TDSCI_ReadToken ( tdsci ) ;

    do
    {
        Parse_A_Typedef_Field ( tdsci ) ; // a structure can other struct/unions as fields
        if ( GetState ( tdsci, TDSCI_UNION ) )
        {
            if ( tdsci->Tdsci_Field_Size > tdsci->Tdsci_StructureUnion_Size ) tdsci->Tdsci_StructureUnion_Size = tdsci->Tdsci_Field_Size ;
        }
        else // not TDSCI_UNION // field may be a struct
        {
            tdsci->Tdsci_StructureUnion_Size += tdsci->Tdsci_Field_Size ;
            tdsci->Tdsci_Offset += tdsci->Tdsci_Field_Size ;
        }
        if ( tdsci->TdsciToken && ( tdsci->TdsciToken [0] == ';' ) ) TDSCI_ReadToken ( tdsci ) ;
    }
    while ( tdsci->TdsciToken && ( tdsci->TdsciToken [0] != '}' ) ) ;
    if ( GetState ( tdsci, TDSCI_UNION ) ) tdsci->Tdsci_Offset += tdsci->Tdsci_StructureUnion_Size ;
    if ( GetState ( tdsci, TDSCI_STRUCT ) ) tdsci->Tdsci_StructureUnion_Size = tdsci->Tdsci_Offset ;
    tdsci->Tdsci_StructureUnion_Namespace->ObjectByteSize = tdsci->Tdsci_StructureUnion_Size ;
    tdsci->Tdsci_StructureUnion_Namespace = tdsci->Tdsci_TotalStructureNamespace ;
    tdsci->Tdsci_Field_Size = 0 ;
    tdsci->Tdsci_TotalSize = tdsci->Tdsci_Offset ;
    if ( tdsci->Tdsci_TotalStructureNamespace ) tdsci->Tdsci_TotalStructureNamespace->ObjectByteSize = tdsci->Tdsci_Offset ;
    SetState_TrueFalse ( tdsci, TDSCI_STRUCTURE_COMPLETED, ( TDSCI_UNION | TDSCI_STRUCT ) ) ;
}

void
Parse_StructOrUnion_Type ( TypeDefStructCompileInfo * tdsci, int64 structOrUnionState )
{
    byte * token ;
    Namespace * type0 ;
    SetState ( tdsci, structOrUnionState, true ) ;
    token = TDSCI_ReadToken ( tdsci ) ;
    if ( token )
    {
        if ( token [0] != '{' )
        {
            byte * token1 = Lexer_Peek_Next_NonDebugTokenWord ( _Lexer_, 0, 0 ) ;
            if ( ( token1 [0] == '*' ) && ( type0 = _Namespace_Find ( token, 0, 0 ) ) )
            {
                Parse_Type_Field ( tdsci, type0 ) ;
                return ;
            }
            else
            {
                // pre-structure identifiers
                Parse_Do_Identifier ( tdsci, PRE_STRUCTURE_NAME, 0 ) ;
                token = TDSCI_ReadToken ( tdsci ) ;
            }
        }
        if ( token [0] == '{' )
        {
            // name will be added 'post structure'
            if ( ! tdsci->Tdsci_StructureUnion_Namespace ) tdsci->Tdsci_StructureUnion_Namespace = DataObject_New ( CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, - 1 ) ;
            if ( ! tdsci->Tdsci_TotalStructureNamespace ) tdsci->Tdsci_TotalStructureNamespace = tdsci->Tdsci_StructureUnion_Namespace ;
            Parse_Structure ( tdsci ) ;
        }
        else CSL_Parse_Error ( "No \'{\' token in struct/union field", token ) ;
        // prost-structure identifiers
        if ( tdsci->TdsciToken && ( tdsci->TdsciToken [0] != ';' ) )
        {
            if ( tdsci->TdsciToken [1] != ';' ) // consider case of "};" token
            {
                TDSCI_ReadToken ( tdsci ) ;
                Parse_Identifier_Field ( tdsci, POST_STRUCTURE_NAME, 0 ) ;
            }
        }

    }
}

void
Parse_Type_Field ( TypeDefStructCompileInfo * tdsci, Namespace * type0 )
{
    byte * token = tdsci->TdsciToken ;
    int64 size = 0 ;
    tdsci->Tdsci_Field_Size = 0 ;
    if ( type0 )
    {
        tdsci->Tdsci_Field_Type_Namespace = type0 ;
        size = CSL_Get_Namespace_SizeVar_Value ( type0 ) ;
        token = TDSCI_ReadToken ( tdsci ) ;
        if ( ( token [0] == '*' ) )
        {
            size = CELL ;
            tdsci->Tdsci_Field_Size = size ;
            token = TDSCI_ReadToken ( tdsci ) ;
        }
        Parse_Identifier_Field ( tdsci, TYPE_NAME, size ) ;
    }
    else CSL_Parse_Error ( "No type in type field", token ) ;
}

void
Parse_A_Typedef_Field ( TypeDefStructCompileInfo * tdsci )
{
    Namespace *type0 ;
    byte * token ;
    if ( ! ( token = tdsci->TdsciToken ) || ( String_Equal ( tdsci->TdsciToken, "typedef" ) ) ) token = TDSCI_ReadToken ( tdsci ) ;
    if ( String_Equal ( ( char* ) token, "struct" ) ) Parse_StructOrUnion_Type ( tdsci, TDSCI_STRUCT ) ;
    else if ( String_Equal ( ( char* ) token, "union" ) ) Parse_StructOrUnion_Type ( tdsci, TDSCI_UNION ) ;
    else if ( type0 = _Namespace_Find ( token, 0, 0 ) ) Parse_Type_Field ( tdsci, type0 ) ;
    else
    {
        token = Lexer_Peek_Next_NonDebugTokenWord ( _Lexer_, 1, 0 ) ;
        if ( token[0] == '{' ) Parse_StructOrUnion_Type ( tdsci, TDSCI_STRUCT ) ;
        else CSL_Parse_Error ( "Can't find type field namespace", token ) ;
    }
    // print class field
    //if ( dataPtr ) _Printf ( (byte*) "\n\t%s\t%s" = %lx", token0, token1, (sizeof token0) data [ offset ]) ;
}

void
Compiler_TypedObjectInit ( Word * word, Namespace * typeNamespace )
{
    int64 size ;
    word->TypeNamespace = typeNamespace ;
    word->W_MorphismAttributes |= typeNamespace->W_MorphismAttributes ;
    if ( typeNamespace->W_ObjectAttributes & CLASS ) word->W_ObjectAttributes |= OBJECT ;
    word->W_ObjectAttributes |= LOCAL_OBJECT ;
    size = _Namespace_VariableValueGet ( word, ( byte* ) "size" ) ;
    word->Size = size ? size : typeNamespace->ObjectByteSize ;
    //_DObject_Init ( Word * word, uint64 value, uint64 ftype, byte * function, int64 arg )
    _DObject_Init ( word, ( int64 ) 0, LOCAL_OBJECT, ( byte* ) _DataObject_Run, 0 ) ;
    _Word_Add ( word, 1, 0 ) ;
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
    if ( lispMode ) args = ( ListObject * ) args->Lo_List->head ;

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
                word = Finder_Word_FindUsing ( finder, token, 1 ) ; // ?? find after Literal - eliminate making strings or numbers words ??
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
                if ( ! localsNs ) localsNs = Namespace_FindOrNew_Local ( nsStack ? nsStack : compiler->LocalsCompilingNamespacesStack, 1 ) ; //! debugFlag ) ;
                if ( svff )
                {
                    objectAttributes |= PARAMETER_VARIABLE ; // aka an arg
                    //if ( lispMode ) objectType |= T_LISP_SYMBOL ;
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
                //if ( String_Equal ( localsNs->Name, "locals_-1") && (String_Equal ( token, "n")||String_Equal ( token, "x")) ) 
                //    _Printf ((byte*)"") ;
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
                typeNamespace = 0 ;
                objectTypeNamespace = 0 ;
                objectAttributes = 0 ;
                if ( String_Equal ( token, "this" ) ) word->W_ObjectAttributes |= THIS ;
            }
        }
        else return 0 ; // Syntax Error or no local or parameter variables
    }
    compiler->State |= getReturn ;

    // we support nested locals and may have locals in other blocks so the indexes are cumulative
    //if ( numberOfRegisterVariables && ( ! debugFlag ) ) Compile_Init_LocalRegisterParamenterVariables ( compiler ) ;
    if ( numberOfRegisterVariables ) Compile_Init_LocalRegisterParamenterVariables ( compiler ) ;
    if ( returnVariable ) compiler->ReturnVariableWord = _Finder_FindWord_InOneNamespace ( _Finder_, localsNs, returnVariable ) ;

    _CSL_->InNamespace = saveInNs ;
    finder->FoundWord = 0 ;
    Lexer_SetTokenDelimiters ( lexer, svDelimiters, COMPILER_TEMP ) ;
    compiler->LocalsNamespace = localsNs ;
    SetState ( compiler, VARIABLE_FRAME, true ) ;
    SetState ( _CSL_, DEBUG_SOURCE_CODE_MODE, scm ) ;
    return localsNs ;
}

void
Lexer_ParseAsAString ( Lexer * lexer )
{
    if ( lexer->OriginalToken [ 0 ] == '"' )
    {
        lexer->L_ObjectAttributes = ( T_STRING | KNOWN_OBJECT ) ;
        lexer->LiteralString = _String_UnBox ( lexer->OriginalToken ) ;
    }
    else if ( ( lexer->OriginalToken [ 0 ] == ( byte ) '\'' ) && ( strlen ( ( char* ) lexer->OriginalToken ) > 1 ) )
    {
        lexer->L_ObjectAttributes = ( T_CHAR | KNOWN_OBJECT ) ;
        lexer->Literal = ( int64 ) lexer->OriginalToken [ 1 ] ; //buffer  ;
    }
    else
    {
        lexer->L_ObjectAttributes = ( T_RAW_STRING | KNOWN_OBJECT ) ;
        lexer->LiteralString = lexer->OriginalToken ;
    }
    SetState ( lexer, KNOWN_OBJECT, true ) ;
}

void
_Lexer_ParseBinary ( Lexer * lexer, int64 offset )
{
    byte * token = & lexer->OriginalToken [offset] ;
    int64 cc = 0, i, l = Strlen ( ( char* ) token ) ; // 8 bits/byte
    byte current ;
    for ( i = 0 ; i < l ; i ++ )
    {
        current = token [ l - i - 1 ] ; // 1 : remember zero based array indexing
        if ( current == '1' )
            cc += ( 1 << i ) ;
        else if ( current == '0' )
            continue ;
        else if ( current == ' ' )
            continue ;
        else
        {
            SetState ( lexer, KNOWN_OBJECT, false ) ;
            Lexer_Exception ( token, NOT_A_KNOWN_OBJECT, "\n_Lexer_ParseBinary : non binary digits with number base 2" ) ;
        }
    }
    SetState ( lexer, KNOWN_OBJECT, true ) ;
    lexer->Literal = cc ;
}

void
Lexer_ParseBinary ( Lexer * lexer, byte * token, int64 offset )
{
    _Lexer_ParseBinary ( lexer, offset ) ;
    if ( GetState ( lexer, KNOWN_OBJECT ) )
    {
        lexer->L_ObjectAttributes = ( T_INT | KNOWN_OBJECT ) ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    //else Lexer_ParseAsAString ( lexer ) ;
}

void
Lexer_ParseBigNum ( Lexer * lexer, byte * token )
{
    if ( Namespace_IsUsing ( ( byte* ) "BigNum" ) )
    {
        mpfr_t *bfr = ( mpfr_t* ) _BigNum_New ( token ) ;
        lexer->Literal = ( int64 ) bfr ;
        lexer->L_ObjectAttributes = ( T_BIG_NUM | KNOWN_OBJECT ) ;
        lexer->TokenObjectSize = 8 ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
    }
}
// return boolean 0 or 1 if lexer->Literal value is pushed

Boolean
Lexer_ScanForHexInt ( Lexer * lexer, byte * token )
{
    int64 i, sr, scrap, slt = Strlen ( token ) ;
    if ( sr = sscanf ( ( char* ) token, HEX_INT_FRMT, ( uint64* ) & lexer->Literal ) )
    {
        for ( i = 1 ; sr && i < slt ; i ++ )
        {
            sr = sscanf ( ( char* ) &token[i], HEX_INT_FRMT, ( int64* ) & scrap ) ;
        }
        if ( ! sr ) lexer->Literal = 0 ;
    }
    return sr ;
}

void
_Lexer_ParseHex ( Lexer * lexer, byte * token )
{
    // use 0d format for decimal numbers with hex NumberBase state
    if ( sscanf ( ( char* ) token, INT_FRMT_FOR_HEX, ( int64* ) & lexer->Literal ) )
    {
        lexer->L_ObjectAttributes = ( T_INT | KNOWN_OBJECT ) ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else if ( Lexer_ScanForHexInt ( lexer, token ) ) //sscanf ( ( char* ) token, HEX_INT_FRMT, ( uint64* ) & lexer->Literal ) && sscanf ( ( char* ) &token[1], HEX_INT_FRMT, ( int64* ) & scrap ))
    {
        lexer->L_ObjectAttributes = ( T_INT | KNOWN_OBJECT ) ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else if ( sscanf ( ( char* ) token, HEX_UINT_FRMT, ( uint64* ) & lexer->Literal ) )
    {
        lexer->L_ObjectAttributes = ( T_INT | KNOWN_OBJECT ) ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else if ( sscanf ( ( char* ) token, LISP_HEX_FRMT, ( uint64* ) & lexer->Literal ) )
    {
        lexer->L_ObjectAttributes = ( T_INT | KNOWN_OBJECT ) ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else Lexer_ParseAsAString ( lexer ) ;
}

void
_Lexer_ParseDecimal ( Lexer * lexer, byte * token )
{
    float f ;
    // use 0x format for hex numbers with decimal NumberBase state
    //D1 (String_Equal (_ReadLiner_->Filename, "namespaces/test/math.csl") ? CSL_CpuState_Current_Show ( ) : 0 ) ; //Debugger_CSLRegisters (_Debugger_) : 0 ) ;
    if ( sscanf ( ( char* ) token, HEX_UINT_FRMT, ( uint64* ) & lexer->Literal ) ||
        sscanf ( ( char* ) token, INT_FRMT_FOR_HEX, ( uint64* ) & lexer->Literal ) ||
        sscanf ( ( char* ) token, INT_FRMT, ( uint64* ) & lexer->Literal ) ||
        sscanf ( ( char* ) token, LISP_DECIMAL_FRMT, ( uint64* ) & lexer->Literal ) )
    {
        if ( lexer->Literal < 256 )
        {
            lexer->L_ObjectAttributes = ( T_BYTE | KNOWN_OBJECT ) ;
            lexer->TokenObjectSize = 1 ;
        }
        else if ( lexer->Literal <= 65535 )
        {
            lexer->L_MorphismAttributes = ( KNOWN_OBJECT ) ;
            lexer->L_ObjectAttributes |= ( T_INT16 ) ;
            lexer->TokenObjectSize = 2 ;
        }
        else if ( lexer->Literal <= 2147483647 )
        {
            lexer->L_MorphismAttributes = ( KNOWN_OBJECT ) ;
            lexer->L_ObjectAttributes = ( T_INT32 ) ;
            lexer->TokenObjectSize = 4 ;
        }
        else
        {
            lexer->L_ObjectAttributes = ( T_INT | KNOWN_OBJECT ) ;
            lexer->TokenObjectSize = 8 ;
        }
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else if ( sscanf ( ( char* ) token, "%f", &f ) )
    {
        lexer->L_ObjectAttributes = ( T_FLOAT | KNOWN_OBJECT ) ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else Lexer_ParseAsAString ( lexer ) ;
}

void
Lexer_ParseObject ( Lexer * lexer, byte * token )
{
    Context * cntx = _Context_ ;
    int64 offset = 0 ;
    lexer->OriginalToken = token ;
    lexer->Literal = 0 ;
    lexer->L_MorphismAttributes = 0 ;
    lexer->L_ObjectAttributes = 0 ;
    if ( token )
    {
        if ( ( token [0] == '0' ) || ( token [0] == '#' ) ) // following scheme notation
        {
            char c ;
            if ( ( c = tolower ( token [1] ) ) == 'x' )
            {
                token [1] = c ;
                //if ( token [0] == '#' ) token [0] = '0' ; // Scheme format to C format
                //_Lexer_ParseHex ( lexer, token[0] == '#' ? &token[1] : token ) ; // #x
                _Lexer_ParseHex ( lexer, token ) ; // #x
                return ;
            }
            else if ( ( c = tolower ( token [1] ) ) == 'b' )
            {
                if ( token [0] == '#' ) // following scheme notation
                {
                    offset = 2 ;
                    Lexer_ParseBinary ( lexer, token, offset ) ; // #b
                    return ;
                }
            }
            else if ( tolower ( token [1] ) == 'd' )
            {
                _Lexer_ParseDecimal ( lexer, token ) ; // #d
                return ;
            }
            //else if ( tolower ( token [1] ) == 'o' ) goto doOctal ; // #o
        }
        if ( cntx->System0->NumberBase == 10 ) _Lexer_ParseDecimal ( lexer, token ) ;
        else if ( cntx->System0->NumberBase == 2 ) Lexer_ParseBinary ( lexer, token, 0 ) ;
        else if ( cntx->System0->NumberBase == 16 ) _Lexer_ParseHex ( lexer, token ) ;
    }
}

byte *
Parse_Macro ( int64 type )
{
    byte * value ;
    Lexer * lexer = _Context_->Lexer0 ;
    if ( type == STRING_MACRO )
    {
        value = Lexer_ReadToken ( lexer ) ;
        while ( ! String_Equal ( ";", ( char* ) Lexer_ReadToken ( lexer ) ) ) ; // nb. we take only the first string all else ignored
    }
    else if ( type == TEXT_MACRO )
    {
        int64 n = 0 ;
        byte nc, *buffer = Buffer_Data ( _CSL_->ScratchB1 ) ;
        buffer [0] = 0 ;
        do
        {
            nc = _ReadLine_GetNextChar ( _Context_->ReadLiner0 ) ;
            if ( nc == ';' )
            {
                buffer [ n ] = 0 ;
                break ;
            }
            buffer [ n ++ ] = nc ;
        }
        while ( nc ) ;
        value = String_New ( ( byte* ) buffer, TEMPORARY ) ;
        //Buffer_SetAsUnused ( b ) ;
    }
    return value ;
}

byte *
_Lexer_ParseTerminatingMacro ( Lexer * lexer, byte termChar, Boolean includeTermChar )
{
    ReadLiner * rl = _ReadLiner_ ;
    byte * token ;
    if ( ( ! ( GetState ( _Compiler_, ( COMPILE_MODE | ASM_MODE | LC_ARG_PARSING | LC_csl ) ) ) ) && ( ! GetState ( _CSL_, SOURCE_CODE_STARTED ) ) )
        CSL_InitSourceCode_WithCurrentInputChar ( _CSL_, 0 ) ;
    _CSL_->SC_QuoteMode = true ;
    if ( ! includeTermChar ) Lexer_UnAppendCharacterToTokenBuffer ( lexer ) ;
    do
    {
        lexer->TokenInputByte = ReadLine_NextChar ( rl ) ;
        if ( lexer->TokenInputByte == '\\' )
            _BackSlash ( lexer, 1 ) ;
        else Lexer_Append_ConvertedCharacterToTokenBuffer ( lexer ) ;
    }
    while ( lexer->TokenInputByte != termChar ) ;
    if ( ! includeTermChar ) Lexer_UnAppendCharacterToTokenBuffer ( lexer ) ;
    _AppendCharacterToTokenBuffer ( lexer, 0 ) ; // null terminate TokenBuffer
    _CSL_->SC_QuoteMode = false ;
    SetState ( lexer, LEXER_DONE, true ) ;
    token = String_New ( lexer->TokenBuffer, STRING_MEM ) ;
    if ( termChar == '\"' )
    {
        if ( GetState ( _CSL_, STRING_MACROS_ON ) && GetState ( &_CSL_->Sti, STI_INITIALIZED ) ) _CSL_StringMacros_Do ( lexer->TokenBuffer ) ;
        Word * word = Lexer_ParseToken_ToWord ( lexer, token, - 1, - 1 ) ;
        Interpreter_DoWord ( _Interpreter_, word, - 1, - 1 ) ;
    }
    return token ;
}

// ?? seems way to complicated and maybe should be integrated with Lexer_ParseObject

void
_CSL_SingleQuote ( )
{
    ReadLiner * rl = _ReadLiner_ ;
    Lexer * lexer = _Lexer_ ;
    Word *word, * sqWord = _CSL_WordList_TopWord ( ) ; //single quote word
    byte buffer [5] ;
    byte c0, c1, c2 ;
    uint64 charLiteral = 0 ;

    _CSL_->SC_QuoteMode = true ;
    if ( ( ! ( GetState ( _Compiler_, ( COMPILE_MODE | ASM_MODE | LC_ARG_PARSING | LC_csl ) ) ) )
        && ( ! GetState ( _CSL_, SOURCE_CODE_STARTED ) ) ) CSL_InitSourceCode_WithCurrentInputChar ( _CSL_, 0 ) ;
    c0 = _ReadLine_PeekOffsetChar ( rl, 0 ) ; // parse a char type, eg. 'c' 
    c1 = _ReadLine_PeekOffsetChar ( rl, 1 ) ;
    if ( sqWord && sqWord->Name[0] == '\'' && ( c1 == '\'' ) || ( c0 == '\\' ) ) // parse a char type, eg. 'c' 
    {
        // notation :: c0 = original ' ; c1 = next char, etc.
        c0 = _ReadLine_GetNextChar ( rl ) ;
        c1 = _ReadLine_GetNextChar ( rl ) ;
        buffer[0] = '\'' ;
        buffer[1] = c0 ;
        if ( c0 == '\\' )
        {
            c2 = _ReadLine_GetNextChar ( rl ) ; // the closing '\''
            if ( c1 == 't' ) charLiteral = 0x9 ;
            else if ( c1 == 'n' ) charLiteral = 0xa ;
            else if ( c1 == 'r' ) charLiteral = 0xd ;
            else if ( c1 == 'b' ) charLiteral = 0x8 ;
            buffer[2] = c1 ;
            buffer[3] = '\'' ; // c3
            buffer[4] = 0 ;
        }
        else
        {
            charLiteral = c0 ;
            buffer[2] = '\'' ; // c2
            buffer[3] = 0 ;
        }
        CSL_WordLists_PopWord ( ) ; // pop the "'" token
        word = _DObject_New ( buffer, charLiteral, IMMEDIATE, LITERAL | CONSTANT, 0, LITERAL, ( byte* ) _DataObject_Run, 0, 0, 0, DICTIONARY ) ;
        word->ObjectByteSize = 1 ;
        Interpreter_DoWord ( _Interpreter_, word, - 1, - 1 ) ; //_Lexer_->TokenStart_ReadLineIndex ) ;
    }
    else
    {
        if ( ! Compiling ) CSL_InitSourceCode_WithName ( _CSL_, lexer->OriginalToken, 0 ) ;
        byte * token, nchar ;
#if 1     
        while ( 1 )
        {
            int64 i = lexer->TokenEnd_ReadLineIndex ;
            while ( ( i < rl->MaxEndPosition ) && ( rl->InputLine [ i ] == ' ' ) ) i ++ ;
            if ( ( rl->InputLine [ i ] == '.' ) || _Lexer_IsTokenForwardDotted ( _Lexer_, i + 1 ) ) // 1 : pre-adjust for an adjustment in                              _Lexer_IsTokenForwardDotted
            {
                token = Lexer_ReadToken ( lexer ) ;
                word = _Interpreter_TokenToWord ( _Interpreter_, token, - 1, - 1 ) ;
                if ( word && ( word->W_ObjectAttributes & ( C_TYPE | C_CLASS | NAMESPACE ) ) ) Finder_SetQualifyingNamespace ( _Finder_, word ) ;
                else if ( token && ( token[0] = '.' ) ) continue ;
                else
                {
                    DataStack_Push ( ( int64 ) token ) ;
                    goto done ;
                }
            }
            else break ;
        }
        CSL_Token ( ) ;
#else        
        if ( ( nchar = ReadLine_PeekNextChar ( rl ) ) == ' ' )
        {
            while ( nchar = ReadLine_NextChar ( rl ) == ' ' ) ;
            ReadLine_UnGetChar ( rl ) ;
        }
        if ( sqWord->Definition != CSL_SingleQuote )
            token = _Lexer_ParseTerminatingMacro ( lexer, ' ', 0 ) ;
        else token = Lexer_ReadToken ( lexer ) ; // in case of 'quote' instead of '\''
        DataStack_Push ( ( int64 ) token ) ;
#endif        
        if ( ( ! AtCommandLine ( rl ) ) && ( ! GetState ( _CSL_, SOURCE_CODE_STARTED ) ) )
            CSL_InitSourceCode_WithName ( _CSL_, token, 0 ) ;
    }
done:
    _CSL_->SC_QuoteMode = false ;
}

