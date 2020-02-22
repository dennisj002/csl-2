
#include "../../include/csl.h"

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

int64
_CSL_ParseQid ( byte * finderToken )
{
    Context * cntx = _Context_ ;
    Lexer * lexer = cntx->Lexer0 ;
    Finder * finder = cntx->Finder0 ;
    int64 nst ;
    Word * word = 0, *ns = 0 ;
    byte * token = finderToken ; // finderToken is also a flag of the caller : (finderToken > 0) ? finder : parser
    while ( 1 )
    {
        if ( ! token )
        {
            token = Lexer_ReadToken ( lexer ) ;
            if ( token && ( token [0] == '.' ) ) token = Lexer_ReadToken ( lexer ) ;
        }
        if ( token && _Lexer_IsTokenForwardDotted ( lexer, 0 ) )
        {
            if ( finderToken ) cntx->Interpreter0->BaseObject = 0 ;
            //word = _Finder_Word_Find ( finder, USING, token ) ; //Finder_Word_FindUsing ( _Finder_, token, 0 ) ;
            if ( ns ) word = _Finder_FindWord_InOneNamespace ( _Finder_, ns, token ) ;
            else word = Finder_Word_FindUsing ( _Finder_, token, 0 ) ; // maybe need to respect a possible qualifying namespace ??
            if ( word && ( nst = word->W_ObjectAttributes & THIS ) ) ns = word, _CSL_SetAsInNamespace ( ns ), Finder_SetQualifyingNamespace ( finder, ns ) ;
            else if ( word && ( nst = word->W_ObjectAttributes & ( finderToken ? ( NAMESPACE_TYPE | THIS ) : ( C_TYPE | C_CLASS | NAMESPACE | THIS ) ) ) )
            {
                ns = word ;
                Finder_SetQualifyingNamespace ( finder, ns ) ;
                _CSL_SetAsInNamespace ( ns ) ;

            }
            else if ( finderToken )
            {
                if ( ! nst ) _CSL_Do_Dot ( cntx, word ) ;
                break ; //return (int64) word ;
            }
            token = 0 ;
        }
        else break ;
    }
#if 0    
    if ( ns )
    {
        //word = _Finder_FindWord_InOneNamespace ( _Finder_, ns, token ) ;
        if ( finderToken ) return ( int64 ) word ;
        else return ( int64 ) token ; //( word ? word->Name : token ) ;
    }
    else
#endif        
    {
        if ( finderToken ) return ( int64 ) word ;
        else return ( int64 ) token ;
    }
}
// ?? seems way to complicated and maybe should be integrated with Lexer_ParseObject

void
_CSL_SingleQuote ( )
{
    Context * cntx = _Context_ ;
    Lexer * lexer = cntx->Lexer0 ;
    ReadLiner * rl = cntx->ReadLiner0 ;
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
        byte * token = ( byte* ) _CSL_ParseQid ( 0 ) ;
        DataStack_Push ( ( int64 ) token ) ;
        if ( ( ! AtCommandLine ( rl ) ) && ( ! GetState ( _CSL_, SOURCE_CODE_STARTED ) ) )
            CSL_InitSourceCode_WithName ( _CSL_, token, 0 ) ;
    }
    _CSL_->SC_QuoteMode = false ;
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


