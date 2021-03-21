
#include "../../include/csl.h"

void
Interpret_DoParenthesizedValue ( )
{
    Compiler * compiler = _Compiler_ ;
    int64 svcm = GetState ( compiler, COMPILE_MODE ) ;
    _Interpret_Until_Including_Token ( _Interpreter_, ( byte * ) ")", 0 ) ;
    SetState ( compiler, COMPILE_MODE, svcm ) ;
}

void
Interpret_C_Block_EndBlock ( byte * tokenToUse, Boolean insertFlag )
{
    if ( tokenToUse ) _CSL_->EndBlockWord->Name = tokenToUse ;
    if ( insertFlag ) SetState ( _Debugger_, DBG_OUTPUT_INSERTION, true ) ;
    if ( tokenToUse ) _Debugger_->SubstitutedWord = _CSL_->EndBlockWord ;
    Interpreter_DoWord ( _Interpreter_, _CSL_->EndBlockWord, - 1, - 1 ) ;
    _CSL_->EndBlockWord->Name = ( byte* ) "}" ;
    _Debugger_->SubstitutedWord = 0 ;
    SetState ( _Debugger_, DBG_OUTPUT_INSERTION, false ) ;
}

void
Interpret_C_Block_BeginBlock ( byte * tokenToUse, Boolean insertFlag )
{
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    // ? source code adjustments ?
    if ( tokenToUse ) _CSL_->BeginBlockWord->Name = tokenToUse ;
    if ( insertFlag ) SetState ( _Debugger_, DBG_OUTPUT_INSERTION, true ) ;
    if ( tokenToUse ) _Debugger_->SubstitutedWord = _CSL_->BeginBlockWord ;
    Interpreter_DoWord ( _Interpreter_, _CSL_->BeginBlockWord, - 1, - 1 ) ;
    _CSL_->BeginBlockWord->Name = ( byte* ) "{" ;
    _Debugger_->SubstitutedWord = 0 ;
    compiler->BeginBlockFlag = false ;
    SetState ( _Debugger_, DBG_OUTPUT_INSERTION, false ) ;
}

int64
CSL_Interpret_C_Blocks ( int64 blocks, Boolean takesAnElseFlag, Boolean semicolonEndsThisBlock )
{
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    Interpreter * interp = cntx->Interpreter0 ;
    Word * word ;
    byte * token ;
    SetState ( compiler, C_BLOCK_INTERPRETER, true ) ;
    int64 blocksParsed = 0 ;

    while ( blocksParsed < blocks )
    {
        token = Lexer_ReadToken ( cntx->Lexer0 ) ;
        byte chr = token[0] ;
        switch ( chr )
        {
            case '(':
            {
                if ( compiler->TakesLParenAsBlock && ( ! compiler->InLParenBlock ) )
                {
                    // interpret a (possible) 'for' c parenthesis expression
                    compiler->InLParenBlock = true ;
                    Interpret_C_Block_BeginBlock ( ( byte* ) "(", 0 ) ;
                    compiler->TakesLParenAsBlock = false ; // after the first block
                    break ;
                }
                else goto doDefault ;
            }
            case ')':
            {
                if ( compiler->InLParenBlock )
                {
                    List_InterpretLists ( compiler->PostfixLists ) ;
                    compiler->InLParenBlock = false ;
                    compiler->TakesLParenAsBlock = false ;
                    Interpret_C_Block_EndBlock ( ( byte* ) ")", 1 ) ;
                    //CSL_TypeStack_Pop ( ) ; // the logic word
                    //if ( ! _Context_StringEqual_PeekNextToken ( _Context_, ( byte* ) "{", 0 ) )
                    byte *token = Lexer_Peek_Next_NonDebugTokenWord (cntx->Lexer0, 0, 0) ;
                    if ( token && ( ! ( ( String_Equal ( ( char* ) token, ( char* ) "{" ) || ( String_Equal ( ( char* ) token, ( char* ) ";" ) ) ) ) ) )
                    {
                        Interpret_C_Block_BeginBlock ( ( byte* ) "{", 1 ) ;
                        semicolonEndsThisBlock = true ;
                    }
                    blocksParsed ++ ;
                    break ;
                }
                else goto doDefault ;
            }
            case '{':
            {
                Interpret_C_Block_BeginBlock ( 0, 0 ) ;
                semicolonEndsThisBlock = false ;
                break ;
            }
            case '}':
            {
                Interpret_C_Block_EndBlock ( 0, 0 ) ;
                blocksParsed ++ ;
                break ;
            }
                //case ',' :
            case ';':
            {
                List_InterpretLists ( compiler->PostfixLists ) ;
                if ( semicolonEndsThisBlock )
                {
                    Interpret_C_Block_EndBlock ( ( byte* ) ";", 0 ) ;
                    blocksParsed ++ ;
                }
                if ( compiler->InLParenBlock ) Interpret_C_Block_BeginBlock ( ( byte* ) "{", 1 ) ;
                else CSL_C_Semi ( ) ; //? seems like 2 semicolons in a row here ?!
                //else if ( ! semicolonEndsThisBlock ) CSL_C_Semi ( ) ;
                break ;
            }
            case 'r':
            {
                if ( String_Equal ( ( char* ) token, "return" ) )
                {
                    CSL_Return ( ) ;
                    continue ;
                }
                else goto doDefault ;
            }
            case 'e':
            {
                if ( String_Equal ( ( char* ) token, "else" ) )
                {
                    if ( takesAnElseFlag )
                    {
                        takesAnElseFlag = false ;
                        if ( ! _Context_StringEqual_PeekNextToken ( _Context_, ( byte* ) "{", 0 ) )
                        {
                            Interpret_C_Block_BeginBlock ( ( byte* ) "{", 1 ) ;
                            semicolonEndsThisBlock = true ;
                        }
                        continue ;
                    }
                    else _SyntaxError ( ( byte * ) "Syntax Error : incorrect \"else\" (with no \"if\"?) : ", 1 ) ;
                }
                // ... not "else" - drop thru to default:
            }
doDefault:
            default:
            {
                word = _Interpreter_TokenToWord ( interp, token, - 1, - 1 ) ;
                if ( word )
                {
                    Interpreter_DoWord ( interp, word, - 1, - 1 ) ;
                    if ( word->W_MorphismAttributes & COMBINATOR )
                    {
                        if ( semicolonEndsThisBlock )
                        {
                            Interpret_C_Block_EndBlock ( ( byte* ) "}", String_Equal ( token, ";" ) ) ;
                            blocksParsed ++ ;
                        }
                    }
                }
                break ;
            }
        }
        if ( takesAnElseFlag && ( blocksParsed == 2 ) )
        {
            if ( _Context_StringEqual_PeekNextToken ( _Context_, ( byte* ) "else", 1 ) )
            {
                blocks ++ ;
                continue ;
            }
            else break ;
        }
    }
    SetState ( compiler, C_BLOCK_INTERPRETER, false ) ;
    return blocksParsed ;
}

void
CSL_C_LeftParen ( )
{
    Context * cntx = _Context_ ;
    ReadLiner * rl = cntx->ReadLiner0 ;
    if ( ( GetState_TrueFalse ( cntx->Interpreter0, ( PREPROCESSOR_MODE ), ( PREPROCESSOR_DEFINE ) ) ) )
    {
        Interpret_DoParenthesizedValue ( ) ;
    }
    else if ( ReadLine_CheckForLocalVariables ( rl ) )
    {
        CSL_LocalsAndStackVariablesBegin ( ) ;
    }
    else if ( CompileMode && ( ( ! GetState ( cntx->Compiler0, VARIABLE_FRAME ) ) || ( ReadLine_PeekNextNonWhitespaceChar ( rl ) == '|' ) ) )
    {
        CSL_LocalsAndStackVariablesBegin ( ) ;
    }
    else Interpret_DoParenthesizedValue ( ) ;
}

void
_CSL_C_Infix_EqualOp ( block op )
{
    Context * cntx = _Context_ ;
    Compiler *compiler = cntx->Compiler0 ;
    Interpreter * interp = cntx->Interpreter0 ;
    Word * wordr, *word0 = CSL_WordList ( 0 ) ;
    Word *lhsWord = compiler->LHS_Word, *word0a, *rword ;
    int64 tsrli = word0 ? word0->W_RL_Index : 0 ;
    int64 svscwi = word0 ? word0->W_SC_Index : 0 ;
    byte * svName ;
    SetState ( compiler, C_INFIX_EQUAL, true ) ;
    _CSL_WordList_PopWords ( 1 ) ; // because we are going to call the opWord in compilable order below 
    word0a = CSL_WordList ( 0 ) ; // rem we just popped a word
    if ( lhsWord )
    {
        if ( ( lhsWord->W_ObjectAttributes & ( OBJECT | OBJECT_FIELD | THIS | QID ) ) || GetState ( lhsWord, QID ) ) wordr = _CSL_->PokeWord ;
        else
        {
            int64 svState = cntx->State ;
            SetState ( cntx, C_SYNTAX | INFIX_MODE, false ) ; // we don't want to just set compiler->LHS_Word
            if ( GetState ( _Context_, ADDRESS_OF_MODE ) ) lhsWord->CompiledDataFieldByteSize = sizeof (byte* ) ;
            Interpreter_DoWord_Default ( interp, lhsWord, lhsWord->W_RL_Index, lhsWord->W_SC_Index ) ;
            SetState ( cntx, C_SYNTAX | INFIX_MODE, svState ) ;
            wordr = _CSL_->StoreWord ;
        }
    }
    else wordr = _CSL_->PokeWord ;
    d0 ( if ( Is_DebugModeOn ) _CSL_SC_WordList_Show ( "\nCSL_C_Infix_EqualOp : before op word", 0, 0 ) ) ;
    if ( op ) Block_Eval ( op ) ;
    else
    {
        rword = wordr ;
        svName = rword->Name ;
        rword->Name = ( byte* ) "=" ;
        if ( ! _String_EqualSingleCharString ( word0->Name, '=' ) )
        {
            SetState ( _Debugger_, DBG_OUTPUT_SUBSTITUTION, true ) ;
            _Debugger_->SubstitutedWord = rword ;
        }
        Interpreter_DoWord_Default ( interp, rword, tsrli, svscwi ) ; // remember _CSL_WordList_PopWords earlier in this function
        SetState ( _Debugger_, DBG_OUTPUT_SUBSTITUTION, false ) ;
        _Debugger_->SubstitutedWord = 0 ;
        rword->Name = svName ;
    }
    if ( GetState ( compiler, C_COMBINATOR_LPAREN ) )
    {
        if ( wordr->StackPushRegisterCode ) SetHere ( wordr->StackPushRegisterCode, 1 ) ; // this is the usual after '=' in non C syntax; assuming optimizeOn
        Compiler_Set_BI_Tttn ( compiler, TTT_ZERO, NEGFLAG_ON, TTT_ZERO, NEGFLAG_Z ) ; // must set logic flag for Compile_ReConfigureLogicInBlock in Block_Compile_WithLogicFlag
    }
    List_InterpretLists ( compiler->PostfixLists ) ;
    compiler->LHS_Word = 0 ;
    if ( ! Compiling ) CSL_InitSourceCode ( _CSL_ ) ;
    SetState ( compiler, C_INFIX_EQUAL, false ) ;

}

void
Do_IncDec_PostFixList ( Word * currentWord, Word * one )
{
    dllist * postfixList = List_New ( CONTEXT ) ;
    List_Push_1Value_NewNode_T_WORD ( postfixList, ( int64 ) currentWord, WORD_RECYCLING ) ; // op 
    List_Push_1Value_NewNode_T_WORD ( postfixList, ( int64 ) one, WORD_RECYCLING ) ; // variable
    List_Push_1Value_NewNode_T_WORD ( _Compiler_->PostfixLists, ( int64 ) postfixList, WORD_RECYCLING ) ;
}

void
CSL_IncDec ( int64 op ) // ++/--
{
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    int64 sd = List_Depth ( _CSL_->CSL_N_M_Node_WordList ) ;
    if ( ( sd > 1 ) && ( ! GetState ( cntx, LC_CSL ) ) )
    {
        Word * currentWord = CSL_WordList ( 0 ) ;
        Word *two = 0, *one = ( Word* ) CSL_WordList ( 1 ) ;
        if ( GetState ( _Context_, C_SYNTAX ) && ( one->W_MorphismAttributes & CATEGORY_OP )
            && ( ! ( one->W_MorphismAttributes & CATEGORY_OP_LOAD ) ) ) one = two = CSL_WordList ( 2 ) ;
        byte * nextToken = Lexer_Peek_Next_NonDebugTokenWord (cntx->Lexer0, 1, 0) ;
        Word * nextWord = Finder_Word_FindUsing ( cntx->Interpreter0->Finder0, nextToken, 0 ) ;
        if ( nextWord && IS_MORPHISM_TYPE ( nextWord )
            && ( nextWord->W_MorphismAttributes & ( CATEGORY_OP_ORDERED | CATEGORY_OP_UNORDERED | CATEGORY_OP_DIVIDE | CATEGORY_OP_EQUAL ) ) )
        {
            _CSL_WordList_PopWords ( 1 ) ; // because we are going to call the opWord in compilable order below 
            if ( GetState ( compiler, ( C_INFIX_EQUAL | DOING_AN_INFIX_WORD | DOING_BEFORE_AN_INFIX_WORD ) ) && GetState ( _CSL_, OPTIMIZE_ON ) && CompileMode )
            {
                Do_IncDec_PostFixList ( currentWord, one ) ;
                return ;
            }
            else
            {
                Interpreter_InterpretNextToken ( cntx->Interpreter0 ) ;
                if ( sd > 1 )
                {
                    Interpreter_DoWord ( cntx->Interpreter0, one, - 1, - 1 ) ;
                    Interpreter_DoWord ( cntx->Interpreter0, currentWord, - 1, - 1 ) ;
                    return ;
                }
            }
        }
        if ( one && one->W_ObjectAttributes & ( PARAMETER_VARIABLE | LOCAL_VARIABLE | NAMESPACE_VARIABLE ) )
        {
            if ( GetState ( _Context_, C_SYNTAX ) )
            {
                if ( ! GetState ( compiler, INFIX_LIST_INTERPRET ) ) 
                {
                    _CSL_WordList_PopWords ( 1 ) ; // op
                    if ( GetState ( _CSL_, OPTIMIZE_ON ) && ( ! two ) ) SetHere ( one->Coding, 1 ) ;
                    Do_IncDec_PostFixList ( currentWord, one ) ;
                    return ;
                }
            }
        }
        else if ( nextWord && ( nextWord->W_ObjectAttributes & ( PARAMETER_VARIABLE | LOCAL_VARIABLE | NAMESPACE_VARIABLE ) ) ) // prefix
        {
            _Compile_Group5 ( op, MEM, FP, 0, LocalOrParameterVar_Disp ( nextWord ), 0 ) ;
            return ;
        }
    }
    _CSL_Do_IncDec ( op ) ;
    SetState ( _Debugger_, DEBUG_SHTL_OFF, false ) ;
}

void
CSL_C_ConditionalExpression ( )
{
    Context * cntx = _Context_ ;
    Interpreter * interp = cntx->Interpreter0 ;
    Compiler * compiler = cntx->Compiler0 ;
    Word * word ;
    if ( ( ! Compiling ) && ( ! GetState ( compiler, C_CONDITIONAL_IN ) ) ) Compiler_Init ( _Compiler_, 0 ) ;
    SetState ( compiler, C_CONDITIONAL_IN, true ) ;
    if ( ! CompileMode ) CSL_If_ConditionalExpression ( ) ;
    else
    {
        word = CSL_WordList ( 1 ) ;
        Compiler_Word_SCHCPUSCA ( word, 0 ) ;
        if ( word && word->StackPushRegisterCode )
        {
            // nb. there is only one block so don't use BlockInfo code ; we may have nested conditionals
            SetHere ( word->StackPushRegisterCode, 1 ) ;
            _Compile_TestCode ( word->RegToUse, CELL ) ;
        }
        else
        {
            Compile_Pop_To_Acc ( DSP ) ;
            _Compile_TestCode ( ACC, CELL ) ;
        }
        byte * compiledAtAddress = Compile_UninitializedJumpEqualZero ( ) ;
        Stack_Push_PointerToJmpOffset ( compiledAtAddress ) ;
        byte * token = Interpret_C_Until_NotIncluding_Token5 (interp, ( byte* ) ":", ( byte* ) ",", ( byte* ) ")", ( byte* ) "}", "#", 0, 0 , 1) ;
        if ( token && _String_EqualSingleCharString ( token, ':' ) )
        {
            Lexer_ReadToken ( _Lexer_ ) ;
            CSL_Else ( ) ;
            Interpret_C_Until_NotIncluding_Token5 (interp, ( byte* ) ";", ( byte* ) ",", ( byte* ) ")", ( byte* ) "}", "]", 0, 0 , 1) ; //( byte* ) "}", ( byte* ) " \n\r\t", 0 ) ;
            CSL_EndIf ( ) ;
        }
        else if ( ! String_Equal ( token, "#" ) ) SyntaxError ( 1 ) ;
    }
    SetState ( compiler, C_CONDITIONAL_IN, false ) ;
}

Boolean
Syntax_AreWeParsingACFunctionCall ( Lexer * lexer )
{
    if ( ! GetState ( _Context_, C_SYNTAX | INFIX_MODE ) ) return false ;
    return _C_Syntax_AreWeParsingACFunctionCall ( & lexer->ReadLiner0->InputLine [ lexer->TokenStart_ReadLineIndex ] ) ;
}

int64
IsLValue_String_CheckForwardToStatementEnd ( byte * nc )
{
    int64 leftBracket = 0 ;
    byte onc1 ;
    while ( *nc )
    {
        switch ( *nc )
        {
            case ';': case ',': case '"': case ')': case '{': case '}': case '\n': return false ;
            case '=':
            {
                if ( * ( nc + 1 ) == '=' ) return false ;
                else
                {
                    onc1 = * ( nc - 1 ) ;
                    if ( ispunct ( onc1 ) )
                    {
                        if ( ( onc1 != '<' ) && ( onc1 != '>' ) && ( onc1 != '!' ) ) return true ; // op equal // ?? >= <=
                        else return false ;
                    }
                    else return true ;
                }
            }
            case '[':
            {
                leftBracket ++ ;
                break ;
            }
            case ']':
            {
                if ( ( -- leftBracket ) && GetState ( _Compiler_, ARRAY_MODE ) ) return false ; // ??
                else break ;
            }
            default: break ;
        }
        nc ++ ;
    }
    return false ;
}

int64
Lexer_CheckForwardToStatementEnd_Is_LValue ( Lexer * lexer, Word * word )
{
    byte * inputPtr ;
    int64 index = ( ( int64 ) word == - 1 ) ? lexer->TokenStart_ReadLineIndex : word->W_RL_Index ;
    inputPtr = & lexer->ReadLiner0->InputLine[index] ;
    return IsLValue_String_CheckForwardToStatementEnd ( inputPtr ) ;

}

// assuming no comments

Boolean
Lexer_IsLValue_CheckBackToLastSemiForParenOrBracket ( Lexer * lexer, Word * word )
{
    ReadLiner * rl = lexer->ReadLiner0 ;
    byte * nc ;
    if ( rl->InputStringOriginal ) nc = & rl->InputStringOriginal [lexer->TokenStart_FileIndex] ;
    else nc = & rl->InputLineString [rl->CursorPosition] ;
    while ( 1 )
    {
        switch ( *nc )
        {
            case ';': case ',': case '}': case '{': case '&': return true ;
            case '=': case '\n': return false ;
            case ')': case '(': return Lexer_CheckForwardToStatementEnd_Is_LValue ( lexer, word ) ;
            default: break ;
        }
        nc -- ;
    }
    return true ;
}

// assuming no comments

Boolean
Lexer_IsLValue_CheckForwardToNextSemiForArrayVariable ( Lexer * lexer, Word * word )
{
    if ( ( word->W_ObjectAttributes & ( OBJECT | THIS | QID ) ) || GetState ( word, QID ) )
    {
        ReadLiner * rl = lexer->ReadLiner0 ;
        byte * nc = & rl->InputStringOriginal [lexer->TokenStart_FileIndex] ;
        Boolean space = false, inArray = false ;
        byte onc1 ;
        while ( 1 )
        {
            switch ( *nc )
            {
                case ';': case ',': case '}': case '{': case '.': case ')': case ' ': case 0: case (byte ) ( uint8 ) EOF: return true ;
                case '=':
                {
                    if ( * ( nc + 1 ) == '=' ) return false ;
                    else
                    {
                        onc1 = * ( nc - 1 ) ;
                        if ( ispunct ( onc1 ) )
                        {
                            if ( ( onc1 != '<' ) && ( onc1 != '>' ) ) return true ; // op equal // ?? >= <=
                            else return false ;
                        }
                        else return true ;
                    }
                }
                case '[': inArray = true, space = false ;
                case ']': inArray = false, space = false ;
                default: break ;
            }
            if ( ( ! inArray ) && space && isalnum ( *nc ) ) return false ; // false means word is to be an rvalue
            if ( inArray && isalpha ( *nc ) ) // true means we have an array varible with this object
                return true ;
            nc ++ ;
        }
        return true ;
    }
    else return false ;
}

Boolean
Is_LValue ( Context * cntx, Word * word )
{
    Boolean isLValue = true ; // in postfix generally 
    Compiler * compiler = cntx->Compiler0 ;
    if ( GetState ( cntx, ADDRESS_OF_MODE ) ) isLValue = true ;
    else if ( GetState ( cntx, C_SYNTAX | INFIX_MODE ) ) // lvalue is a C syntax concept
    {
        if ( GetState ( compiler, ARRAY_MODE ) ) isLValue = Lexer_IsLValue_CheckForwardToNextSemiForArrayVariable ( cntx->Lexer0, word ) ;
        else isLValue = Lexer_CheckForwardToStatementEnd_Is_LValue ( cntx->Lexer0, word ) ;
    }
    return isLValue ;
}

// char sets would be better here ??

Boolean
Lexer_IsTokenReverseDotted ( Lexer * lexer )
{
    ReadLiner * rl = lexer->ReadLiner0 ;
    int64 space = 0, i, graph = 0, start = lexer->TokenStart_ReadLineIndex - 1 ;
    byte * nc = & rl->InputLineString [ start ] ;
    i = start ; //??
    while ( 1 )
    {
        switch ( *nc )
        {
            case ']': case '[': return false ;
            case '\n': case ',': case ';': case '(': case ')': case '\'': return false ;
            case '.':
            {
                if ( *( nc + 1 ) != '.' ) // watch for (double/triple) dot ellipsis
                    return true ;
                break ;
            }
            case '"':
            {
                graph ++ ;
                if ( i < start ) return false ; //??
                break ;
            }
            case ' ':
            {
                if ( graph ) space ++ ;
                break ;
            }
            case 0: return false ;
            default:
            {
                graph ++ ;
                if ( ( ! GetState ( _Compiler_, ARRAY_MODE ) ) && space && isgraph ( *nc ) ) return false ;
                else
                {
                    space = 0 ;
                    break ;
                }
            }
        }
        i -- ;
        nc -- ;
    }
    return false ;
}

