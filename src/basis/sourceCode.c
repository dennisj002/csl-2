
#include "../include/csl.h"

void
SC_ShowDbgSourceCodeWord_Or_AtAddress ( Word * scWord0, byte * address )
{
    // ...source code source code TP source code source code ... EOL
    Word * scWord, * word ;
    dllist * list ;
    byte *sourceCode, *token, *buffer ;
    int64 fixed ;
    if ( ! Compiling )
    {
        if ( ! scWord0 ) scWord = Get_SourceCodeWord ( ) ;
        else scWord = scWord0 ;
        if ( scWord )
        {
            list = scWord->W_SC_WordList ? scWord->W_SC_WordList : _CSL_->Compiler_N_M_Node_WordList ; //&& ( scWord->W_SC_MemSpaceRandMarker == _O_->MemorySpace0->TempObjectSpace->InitFreedRandMarker ) ) ? scWord->W_SC_WordList : 0 ; //CSL->CompilerWordList ;
            if ( list )
            {
                sourceCode = scWord->W_SourceCode ; //? scWord->W_SourceCode : String_New ( CSL->SC_Buffer, TEMPORARY ) ;
                if ( ! String_Equal ( sourceCode, "" ) )
                {
                    fixed = 0 ;
                    //if ( Is_DebugOn ) SC_WordList_Show ( list, scWord, 0, 0, 0 ) ;
                    word = DWL_Find ( list, 0, address, 0, 0, 0, 0 ) ;
                    if ( word )
                    {
                        if ( ( scWord->W_TypeAttributes & WT_C_SYNTAX ) && ( String_Equal ( word->Name, "store" ) || String_Equal ( word->Name, "poke" ) ) )
                        {
                            //word->Name = ( byte* ) "=" ;
                            //fixed = 1 ;
                            SetState ( _Debugger_, DBG_OUTPUT_INSERTION | DBG_OUTPUT_SUBSTITUTION, true ) ;
                        }
                        token = GetState ( _Debugger_, DBG_OUTPUT_INSERTION | DBG_OUTPUT_SUBSTITUTION ) ? (byte*) "=" : word->Name ;
                        buffer = DBG_PrepareSourceCodeString ( word, token, sourceCode, 0, 0, 1 ) ;
                        if ( buffer && buffer[0] ) Printf ( ( byte* ) "\n%s", buffer ) ;
                        if ( fixed ) word->Name = ( byte* ) "store" ;
                        if ( _Debugger_ ) _Debugger_->LastSourceCodeWord = word ;
                    }
                }
            }
        }
    }
}

Boolean
SC_ShowSourceCode_In_Word_At_Address ( Word * word, byte * address )
{
    if ( GetState ( _CSL_, GLOBAL_SOURCE_CODE_MODE ) ) //DEBUG_SOURCE_CODE_MODE ) ) // ( _Context_->CurrentlyRunningWord ) && _Context_->CurrentlyRunningWord->W_SC_WordList ) )
    {
        if ( ! word ) word = Get_SourceCodeWord ( ) ;
        SC_ShowDbgSourceCodeWord_Or_AtAddress ( word, address ) ;
        return true ;
    }
    return false ;
}

Boolean
SC_IsWord_BlockOrCombinator ( Word * word )
{
    if ( word && ( ( word->Name[0] == '{' ) || ( word->Name[0] == '}' )
        || ( word->W_MorphismAttributes & ( COMBINATOR | SYNTACTIC ) ) ) ) return true ;
    return false ;
}

Boolean
SC_IsWord_MatchCorrectConsideringBlockOrCombinator ( Word * word )
{
    if ( word )
    {
        if ( SC_IsWord_BlockOrCombinator ( word ) )
        {
            if ( word->SourceCoding [0] == 0xe9 ) return true ; // 0xe9 jmp ins 
            else return false ;
        }
        else return true ;
    }
    return false ;
}

/*
 * 
 * Compiler Word List has nodes (CWLNs) with 2 slots one for the *word and one for a pointer to a Source Code Node (SCN) which has source code index info.
 * CWLN : slot 0 word, slot 1 SCN
 * Source code nodes (SCNs) have three slots for the source code byte index, the code address, and a pointer to the word, they are on the CSL->DebugWordList.
 * SCN : slot 0 : SCN_T_WORD, slot 1 : SCN_T_WORD_SC_INDEX, slot 2 : address is SCN_T_WORD->Coding = word->Coding
 * So, they each have pointers to each other.
 * 
 */
Word *
DWL_Find ( dllist * list, Word * iword, byte * address, byte* name, int64 takeFirstFind, byte * newAddress, int64 fromFirstFlag ) // nb fromTop is from the end of the list because it is the top 'push'
{
    byte * naddress ;
    Word *aFoundWord = 0, *foundWord = 0, *maybeFoundWord = 0 ;
    dlnode * anode = 0 ;
    int64 numFound = 0, i, iuFlag ;
    int64 fDiff = 0, minDiffFound = 0, scwi, lastScwi = _Debugger_->LastScwi ? _Debugger_->LastScwi : 0 ;
    if ( list && ( iword || name || address ) )
    {
        for ( i = 0, anode = ( fromFirstFlag ? dllist_First ( list ) : dllist_Last ( list ) ) ; anode ;
            anode = ( fromFirstFlag ? dlnode_Next ( anode ) : dlnode_Previous ( anode ) ), i ++ )
        {
            aFoundWord = ( Word* ) dobject_Get_M_Slot ( ( dobject* ) anode, SCN_T_WORD ) ;
            iuFlag = dobject_Get_M_Slot ( ( dobject* ) anode, SCN_IN_USE_FLAG ) ;
            //if ( ( ! aFoundWord->W_WordData ) || ( ! ( iuFlag & SCN_IN_USE_FOR_SOURCE_CODE ) ) ) continue ;
            if ( ( ! aFoundWord->W_WordData ) || ( ! ( iuFlag & SCN_IN_USE_FOR_SOURCE_CODE ) ) ) continue ;
            scwi = dobject_Get_M_Slot ( ( dobject* ) anode, SCN_SC_WORD_INDEX ) ;
            naddress = aFoundWord->SourceCoding ;
            if ( iword && ( aFoundWord == iword ) ) return aFoundWord ;
            if ( ( _O_->Verbosity > 3 ) ) DWL_ShowWord ( anode, i, 0, ( int64 ) "afound", 0 ) ;
            if ( address && ( address == naddress ) )
            {
                numFound ++ ;
                fDiff = abs ( scwi - lastScwi ) ;
                aFoundWord->W_SC_Index = scwi ; // not sure exactly why this is necessary but it is important for now??
                if ( ( _O_->Verbosity > 2 ) ) DWL_ShowWord ( anode, i, 0, ( int64 ) "FOUND", 0 ) ;
                if ( ( aFoundWord->W_ObjectAttributes & LITERAL ) && ( aFoundWord->Coding[1] == 0xb9 ) )
                {
                    foundWord = aFoundWord ;
                    minDiffFound = fDiff ;
                }
                else if ( ( aFoundWord->W_MorphismAttributes & CATEGORY_PLUS_PLUS_MINUS_MINUS ) && ( aFoundWord->Coding[1] == 0xff ) )
                {
                    foundWord = aFoundWord ;
                    break ;
                }
                else if ( SC_IsWord_MatchCorrectConsideringBlockOrCombinator ( aFoundWord ) )
                {
                    foundWord = aFoundWord ;
                    minDiffFound = fDiff ;
                }
                else if ( ( fDiff < minDiffFound ) && SC_IsWord_MatchCorrectConsideringBlockOrCombinator ( aFoundWord ) )//|| SC_IsWord_MatchCorrectConsideringBlockOrCombinator ( foundWord ) || ( ! SC_IsWord_MatchCorrectConsideringBlockOrCombinator ( aFoundWord ) ) )
                {
                    foundWord = aFoundWord ;
                    minDiffFound = fDiff ;
                }
                else if ( ! foundWord ) maybeFoundWord = aFoundWord ;
                if ( takeFirstFind ) break ;
            }
        }
        if ( ( ! foundWord ) && maybeFoundWord ) foundWord = maybeFoundWord ;
    }
    if ( ( ! newAddress ) && ( numFound ) )
    {
        if ( ( foundWord ) && ( _O_->Verbosity > 2 ) )
        {
            //_Printf ( ( byte* ) "\nNumber Found = %d :: minDiffFound = %d : window = %d : Chosen word = \'%s\' : LastSourceCodeWord = \'%s\'", numFound, minDiffFound, fDiff, foundWord->Name, _Debugger_->LastSourceCodeWord ? _Debugger_->LastSourceCodeWord->Name : ( byte* ) "" ) ;
            _DWL_ShowWord_Print ( foundWord, 0, ( byte* ) "CHOSEN", foundWord->Coding, foundWord->SourceCoding, 0, foundWord->W_SC_Index, - 1 ) ; //_DWL_ShowWord ( foundWord, "CHOSEN", minDiffFound ) ;
        }
        if ( address ) _Debugger_->LastSourceCodeAddress = address ;
        if ( foundWord ) _Debugger_->LastScwi = foundWord->W_SC_Index ;
        return foundWord ;
    }
    return 0 ;
}

Boolean
SC_List_AdjustAddress ( dlnode * node, byte * address, byte * newAddress )
{
    Word * nword = ( Word* ) dobject_Get_M_Slot ( ( dobject* ) node, SCN_T_WORD ) ;
    if ( nword->W_WordData && ( nword->Coding == address ) )
    {
        Word_SetCoding ( nword, newAddress ) ;
        if ( nword->SourceCoding ) Word_SetSourceCoding ( nword, newAddress ) ;
        dobject_Set_M_Slot ( ( dobject* ) node, SCN_IN_USE_FLAG, SCN_IN_USE_FLAG_ALL ) ; // reset after CSL_AdjustDbgSourceCode_InUseFalse
        return true ;
    }
    return false ;
}

void
CSL_AdjustDbgSourceCodeAddress ( byte * address, byte * newAddress )
{
    dllist * list = _CSL_->Compiler_N_M_Node_WordList ;
    if ( list ) dllist_Map2 ( list, ( MapFunction2 ) SC_List_AdjustAddress, ( int64 ) address, ( int64 ) newAddress ) ;
}

void
CheckRecycleWord ( Node * node )
{
    Word *w = ( Word* ) dobject_Get_M_Slot ( ( dobject* ) node, SCN_T_WORD ) ;
    _CheckRecycleWord ( w ) ;
}

void
DLList_Recycle_WordList ( dllist * list )
{
    dllist_Map ( list, ( MapFunction0 ) CheckRecycleWord ) ;
}

void
DLList_RecycleInit_WordList ( Word * word )
{
    dllist_Map ( word->W_SC_WordList, ( MapFunction0 ) CheckRecycleWord ) ;
    List_Init ( word->W_SC_WordList ) ;
}

void
_CSL_RecycleInit_Compiler_N_M_Node_WordList ( )
{
    if ( ( ! GetState ( _CSL_, ( RT_DEBUG_ON | DEBUG_SOURCE_CODE_MODE | GLOBAL_SOURCE_CODE_MODE ) ) ) )
    {
        DLList_Recycle_WordList ( _CSL_->Compiler_N_M_Node_WordList ) ;
    }
    List_Init ( _CSL_->Compiler_N_M_Node_WordList ) ;
}

void
CSL_WordList_Init ( Word * word )
{
    _CSL_RecycleInit_Compiler_N_M_Node_WordList ( ) ;
    if ( word )
    {
        //if ( ! GetState ( _Compiler_, LISP_MODE ) ) word->W_SC_Index = 0 ; // before pushWord !
        CSL_WordList_PushWord ( word ) ; // for source code
    }
}

inline void
Word_SetSourceCoding ( Word * word, byte * address )
{
    word->SourceCoding = address ;
}

inline void
Word_SetCoding ( Word * word, byte * address )
{
    word->Coding = address ;
}

void
WordList_SetSourceCoding ( int64 index, byte * address )
{
    Word * word = WordStack ( index ) ;
    Word_SetSourceCoding ( word, address ) ;
}

void
Word_SetCodingAndSourceCoding ( Word * word, byte * address )
{
    Word_SetCoding ( word, address ) ;
    Word_SetSourceCoding ( word, address ) ;
}

void
WordList_SetCoding ( int64 index, byte * address )
{
    Word * word = WordStack ( index ) ;
    Word_SetCodingAndSourceCoding ( word, address ) ;
}

void
SC_ListClearAddress ( dlnode * node, byte * address )
{
    Word * nword = ( Word* ) dobject_Get_M_Slot ( ( dobject* ) node, SCN_T_WORD ) ;
    if ( nword->W_WordData && ( nword->SourceCoding == address ) ) //&& ( nword->W_SC_WordIndex != word->W_SC_WordIndex ) )
    {
        Word_SetSourceCoding ( nword, 0 ) ;
        d0 ( if ( Is_DebugModeOn ) Printf ( ( byte* ) "\nnword %s with scwi %d :: cleared for word %s with scwi %d",
            nword->Name, nword->W_SC_Index, nword->Name, nword->W_SC_Index ) ) ;
    }
}

void
Compiler_Word_SetCoding_And_ClearPreviousUseOf_A_SCA ( Word * word, byte * coding, Boolean clearPreviousFlag )
{
    if ( Compiling && word )
    {
        if ( clearPreviousFlag ) dllist_Map1_FromEnd ( _CSL_->Compiler_N_M_Node_WordList, ( MapFunction1 ) SC_ListClearAddress, ( int64 ) coding ) ; //dllist_Map1_FromEnd ( CSL->CompilerWordList, ( MapFunction1 ) SC_ListClearAddress, ( int64 ) Here ) ; //( int64 ) word, ( int64 ) Here ) ;
        Word_SetCodingAndSourceCoding ( word, coding ) ;
    }
}

void
Compiler_SCA_Word_SetCodingHere_And_ClearPreviousUse ( Word * word, Boolean clearPreviousFlag )
{
    Compiler_Word_SetCoding_And_ClearPreviousUseOf_A_SCA ( word, Here, clearPreviousFlag ) ;
}

Word *
_CSL_WordList_TopWord ( )
{
    Word * word = 0 ;
    node * first = _dllist_First ( _CSL_->Compiler_N_M_Node_WordList ) ;
    if ( first ) word = ( Word* ) dobject_Get_M_Slot ( ( dobject* ) first, SCN_T_WORD ) ;
    return word ;
}

#define WL_GET_NODE 1
#define WL_SET_IN_USE_FLAG 2
// zero indexed list notation : ie. the first node is zero - 0

Word *
CSL_WordList_DoOp ( int64 n, int64 op, int64 condition )
{
    dllist * list = _CSL_->Compiler_N_M_Node_WordList ;
    dlnode * node, *nextNode ;
    Word * wordn = 0 ;
    int64 inUseFlag, numDone = 0 ;
    if ( list )
    {
        for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
        {
            nextNode = dlnode_Next ( node ) ;
            dobject * dobj = ( dobject * ) node ;
            inUseFlag = dobject_Get_M_Slot ( dobj, SCN_IN_USE_FLAG ) ;
            if ( ( op == WL_SET_IN_USE_FLAG ) && ( inUseFlag & SCN_IN_USE_FOR_OPTIMIZATION ) )
            {
                dobject_Set_M_Slot ( dobj, SCN_IN_USE_FLAG, condition ) ;
                if ( ( ++ numDone ) >= n ) return 0 ;
            }
                // since we don't remove words and just reset the flags when we 'pop' a word
                // we need to check that the word hasn't already be popped from its SCN_IN_USE_FLAG ...
            else if ( ( op == WL_GET_NODE ) && ( inUseFlag & condition ) )
            {
                // zero indexed list notation : ie. the first node is zero - 0
                if ( ( numDone ++ ) >= n ) return wordn = ( Word* ) dobject_Get_M_Slot ( ( dobject* ) dobj, SCN_T_WORD ) ;

            }
        }
    }
    return 0 ;
}

Word *
_CSL_WordList_PopWords ( int64 n )
{
    Word * rword = CSL_WordList_DoOp ( n, WL_SET_IN_USE_FLAG, SCN_IN_USE_FOR_SOURCE_CODE ) ; // we don't remove just reset the flag
    return rword ;
}

Word *
CSL_WordLists_PopWord ( )
{
    Word * word = _CSL_WordList_PopWords ( 1 ) ;
    return word ;
}

Word *
_CSL_WordList ( int64 n )
{
    //Word * rword = ( Word * ) _dllist_Get_N_InUse_Node_M_Slot ( _CSL_->Compiler_N_M_Node_WordList, n, SCN_T_WORD ) ;
    Word * rword = CSL_WordList_DoOp ( n, WL_GET_NODE, SCN_IN_USE_FOR_OPTIMIZATION ) ;
    return rword ;
}

Word *
CSL_WordList ( int64 n )
{
    return ( Word * ) _CSL_WordList ( n ) ;
}

void
CSL_WordList_Push ( Word * word, Boolean inUseFlag )
{
    _List_PushNew_ForWordList ( _CSL_->Compiler_N_M_Node_WordList, word, inUseFlag ) ;
}

void
_CSL_WordList_PushWord ( Word * word, Boolean inUseFlag )
{
    CSL_WordList_Push ( word, inUseFlag ? SCN_IN_USE_FLAG_ALL : 0 ) ;
}

void
CSL_WordList_PushWord ( Word * word )
{
    int64 inUseFlag = 0 ;
    if ( ( word->W_MorphismAttributes & ( OBJECT_OPERATOR ) ) ) inUseFlag = 0 ;
    else
    {
        // not the clearest logic ??
        if ( ( word->W_ObjectAttributes & ( DOBJECT | NAMESPACE_VARIABLE ) ) || ( ! ( ( word->W_MorphismAttributes & ( OBJECT_OPERATOR ) )
            || ( word->W_ObjectAttributes & ( NAMESPACE | OBJECT_FIELD ) ) ) ) ) inUseFlag |= SCN_IN_USE_FLAG_ALL ;
        if ( ( word->W_ObjectAttributes & ( DOBJECT | NAMESPACE_VARIABLE | NAMESPACE | OBJECT_FIELD | LOCAL_OBJECT ) ) ) inUseFlag |= SCN_IN_USE_FOR_SOURCE_CODE ;
    }
    CSL_WordList_Push ( word, inUseFlag ) ;
}

// too many showWord functions ??

void
_DWL_ShowWord_Print ( Word * word, int64 index, byte * prefix, byte * coding, byte * sourceCoding, byte * newSourceCoding,
    int64 scwi, Boolean iuFlag )
{
    if ( word )
    {
        //int64 lastScwi = _Debugger_->LastSourceCodeWord ? _Debugger_->LastSourceCodeWord->W_SC_Index : 0 ;
        byte * name = String_ConvertToBackSlash ( word->Name ), biuFlag [32] ;
        biuFlag[0] = 0 ;
        if ( iuFlag == - 1 ) strncat ( biuFlag, ( byte* ) "", 31 ) ;
        else
        {
            if ( iuFlag & SCN_IN_USE_FOR_SOURCE_CODE ) strncat ( biuFlag, ( byte* ) "sc", 31 ) ;
            if ( iuFlag & SCN_IN_USE_FOR_OPTIMIZATION )
            {
                if ( biuFlag [0] ) strncat ( biuFlag, ( byte* ) "|", 31 ) ;
                strncat ( biuFlag, ( byte* ) "opt", 31 ) ;
            }
            if ( iuFlag == SCN_IN_USE_FLAG_ALL ) strncpy ( biuFlag, ( byte* ) "all", 31 ) ;
            if ( ! iuFlag ) strncat ( biuFlag, ( byte* ) "false", 31 ) ;
        }
        if ( newSourceCoding )
        {
            Printf ( ( byte* ) "\n %s :: word = 0x%08x : \'%-12s\' : coding  = 0x%08x : oldCoding  = 0x%08x : newCoding = 0x%08x : scwi = %03d, inUse = %s",
                prefix, word, name, coding, sourceCoding, newSourceCoding, scwi, biuFlag ) ;
        }
        else if ( index )
        {
            Printf ( ( byte* ) "\n WordList : index %3d : word = 0x%08x : \'%-12s\' : sourceCoding = 0x%08x : scwi = %03d : inUse = %s",
                index, word, name, sourceCoding, scwi, biuFlag ) ;
        }
        else //if ( scwiDiff )
        {
            Printf ( ( byte* ) "\n %s :: \'%-12s\' : sourceCoding  = 0x%08x : scwi = %03d : inUse = %s",
                prefix, name, sourceCoding, scwi, biuFlag ) ;
        }
    }
}

void
DWL_ShowWord ( dlnode * anode, int64 index, int64 inUseOnlyFlag, int64 prefix, int64 four )
{
    if ( anode )
    {
        Word * word = ( Word* ) dobject_Get_M_Slot ( ( dobject * ) anode, SCN_T_WORD ) ;
        int64 scwi = dobject_Get_M_Slot ( ( dobject* ) anode, SCN_SC_WORD_INDEX ) ;
        int64 iuFlag = dobject_Get_M_Slot ( ( dobject * ) anode, SCN_IN_USE_FLAG ) ;
        if ( word && ( ( ! inUseOnlyFlag ) || ( inUseOnlyFlag && iuFlag ) ) )
        {
            _DWL_ShowWord_Print ( word, index, ( byte* ) prefix, word->Coding, word->SourceCoding, 0, scwi, iuFlag ) ;
        }
    }
}

void
SC_WordList_Show ( dllist * list, Word * scWord, Boolean fromFirstFlag, Boolean inUseOnlyFlag, byte * listName )
{
    if ( scWord ) Printf ( ( byte* ) "\n%s WordList : for word \'%s\' :", listName, scWord->Name ) ;
    if ( list ) dllist_Map4_FromFirstFlag_Indexed ( list, fromFirstFlag, DWL_ShowWord, inUseOnlyFlag, ( int64 ) "", 0 ) ;
}

// too much/many shows ?? combine some

void
CSL_WordList_Show ( Word * word, byte * prefix, Boolean inUseOnlyFlag, Boolean showInDebugColors )
{
    //dllist * list = Compiling ? CSL->Compiler_N_M_Node_WordList : ( word && word->W_SC_WordList ) ? word->W_SC_WordList : CSL->Compiler_N_M_Node_WordList ;
    dllist * list = ( word && word->W_SC_WordList ) ? word->W_SC_WordList : _CSL_->Compiler_N_M_Node_WordList ;
    byte *buffer = Buffer_Data ( _CSL_->ScratchB1 ) ;
    buffer[0] = 0 ;
    if ( list )
    {
        if ( Is_DebugModeOn || showInDebugColors ) NoticeColors ;
        if ( word )
        {
            sprintf ( ( char* ) buffer, "%sWord = %s = %lx :: list = %s %lx : %s", prefix ? prefix : ( byte* ) "", ( char* ) word->Name, ( int64 ) word,
                ( list == _CSL_->Compiler_N_M_Node_WordList ) ? "CSL WordList" : "source code word list = ", ( int64 ) list, inUseOnlyFlag ? "in use only" : "all" ) ;
        }
        SC_WordList_Show ( list, word, 0, inUseOnlyFlag, buffer ) ;
        if ( Is_DebugModeOn || showInDebugColors ) DefaultColors ;
    }
}

void
_CSL_SC_WordList_Show ( byte * prefix, Boolean inUseOnlyFlag, Boolean showInDebugColors )
{
    Word * scWord = Get_SourceCodeWord ( ) ;
    CSL_WordList_Show ( scWord, prefix, inUseOnlyFlag, showInDebugColors ) ;
}

void
CSL_SC_WordList_Show ( )
{
    _CSL_SC_WordList_Show ( 0, 0, 0 ) ;
}

void
CSL_DbgSourceCodeOff ( )
{
    SetState ( _CSL_, GLOBAL_SOURCE_CODE_MODE, false ) ;
}

void
CSL_DbgSourceCodeOn ( )
{
    SetState ( _CSL_, GLOBAL_SOURCE_CODE_MODE, true ) ;
}

void
CSL_DbgSourceCodeOn_Global ( )
{
    SetState ( _CSL_, ( DEBUG_SOURCE_CODE_MODE | GLOBAL_SOURCE_CODE_MODE ), true ) ;
}

void
CSL_DbgSourceCodeOff_Global ( )
{
    SetState ( _CSL_, ( DEBUG_SOURCE_CODE_MODE | GLOBAL_SOURCE_CODE_MODE ), false ) ;
}

// debug source code functions (above)
//=================================================
// text source code functions (below)

void
_CSL_AddStringToSourceCode ( CSL * csl, byte * str )
{
    if ( str )
    {
        strcat ( ( char* ) csl->SC_Buffer, ( char* ) str ) ;
        strcat ( ( CString ) csl->SC_Buffer, ( CString ) " " ) ;
    }
}

void
CSL_AddStringToSourceCode ( CSL * csl, byte * str )
{
    _CSL_AddStringToSourceCode ( csl, str ) ;
    csl->SC_Index += ( Strlen ( ( char* ) str ) + 1 ) ; // 1 : add " " (above)
}

void
_CSL_SC_ScratchPadIndex_Init ( CSL * csl )
{
    csl->SC_Index = Strlen ( ( char* ) _CSL_->SC_Buffer ) ;
}

// don't be confused by SourceCode_Init is different from InitSourceCode

void
_CSL_SourceCode_Init ( CSL * csl )
{
    if ( GetState ( _CSL_, SOURCE_CODE_ON ) )
    {
        csl->SC_Buffer [ 0 ] = 0 ;
        csl->SC_Index = 0 ;
        SetState ( csl, SOURCE_CODE_STARTED, false ) ;
    }
}

void
CSL_SourceCode_InitStart ( CSL * csl )
{
    if ( GetState ( _CSL_, SOURCE_CODE_ON ) )
    {
        _CSL_SourceCode_Init ( csl ) ;
        csl->SCI.SciFileIndexScStart = _ReadLiner_->FileCharacterNumber ;
        SetState ( csl, SOURCE_CODE_STARTED, true ) ;
    }
}

void
_CSL_InitSourceCode ( CSL * csl )
{
    if ( GetState ( _CSL_, SOURCE_CODE_ON ) )
    {
        CSL_SourceCode_InitStart ( csl ) ;
        Lexer_SourceCodeOn ( _Context_->Lexer0 ) ;
    }
}

void
CSL_InitSourceCode ( CSL * csl )
{
    if ( GetState ( _CSL_, SOURCE_CODE_ON ) )
    {
        _CSL_InitSourceCode ( csl ) ;
        _CSL_SC_ScratchPadIndex_Init ( csl ) ;
    }
}

void
CSL_InitSourceCode_WithName ( CSL * csl, byte * name, Boolean force )
{
    if ( force || ( GetState ( _CSL_, SOURCE_CODE_ON ) && ( ( ! Compiling ) || ( ! ( GetState ( csl, SOURCE_CODE_STARTED ) ) ) ) ) )
    {
        _CSL_InitSourceCode ( csl ) ;
        _CSL_AddStringToSourceCode ( csl, name ) ;
        _CSL_SC_ScratchPadIndex_Init ( csl ) ;
    }
}

void
CSL_InitSourceCode_WithCurrentInputChar ( CSL * csl, Boolean force )
{
    if ( force || ( GetState ( _CSL_, SOURCE_CODE_ON ) && ( ( ! Compiling ) || ( ! ( GetState ( csl, SOURCE_CODE_STARTED ) ) ) ) ) )
    {
        Lexer * lexer = _Context_->Lexer0 ;
        _CSL_InitSourceCode ( csl ) ;
        _Lexer_AppendCharToSourceCode ( lexer, lexer->TokenInputByte, 0 ) ;
    }
}

void
CSL_SourceCode_Init ( )
{
    Word * word = _Interpreter_->w_Word ;
    CSL_InitSourceCode_WithName ( _CSL_, word ? word->Name : 0, 1 ) ;
}

void
CSL_Lexer_SourceCodeOn ( )
{
    Lexer_SourceCodeOn ( _Context_->Lexer0 ) ;
}

byte *
_CSL_GetSourceCode ( )
{
    byte * sc = String_New_SourceCode ( _CSL_->SC_Buffer ) ;
    return sc ;
}

void
_CSL_SetSourceCodeWord ( Word * word )
{
    _CSL_->SCI.SciWord = word ; //SC_Word = word ;
}

void
CSL_SetSourceCodeWord ( )
{
    Word * word = ( Word* ) DataStack_Pop ( ) ;
    _CSL_SetSourceCodeWord ( word ) ;
}

void
CSL_Finish_WordSourceCode ( CSL * csl, Word * word )
{
    if ( word )
    {
        if ( ! word->W_SourceCode ) word->W_SourceCode = _CSL_GetSourceCode ( ) ;
        Lexer_SourceCodeOff ( _Lexer_ ) ;
        word->SC_FileIndex_Start = csl->SCI.SciFileIndexScStart ;
        word->SC_FileIndex_End = csl->SCI.SciFileIndexScEnd ;
        _CSL_SourceCode_Init ( csl ) ;
    }
}

void
SCN_Set_NotInUse ( dlnode * node )
{
    dobject_Set_M_Slot ( ( dobject* ) node, SCN_IN_USE_FLAG, 0 ) ;
}

void
SCN_Set_NotInUseForOptimization ( dlnode * node )
{
    dobject_Set_M_Slot ( ( dobject* ) node, SCN_IN_USE_FLAG, SCN_IN_USE_FOR_SOURCE_CODE ) ;
}
// the logic needs to be reworked with recycling in these functions

void
_CSL_UnAppendFromSourceCode_NChars ( CSL * csl, int64 nchars )
{
    int64 plen = Strlen ( ( CString ) csl->SC_Buffer ) ;
    if ( plen >= nchars ) csl->SC_Buffer [ Strlen ( ( CString ) csl->SC_Buffer ) - nchars ] = 0 ;
    _CSL_SC_ScratchPadIndex_Init ( csl ) ;
}

void
_CSL_UnAppendTokenFromSourceCode ( CSL * csl, byte * tkn )
{
    if ( GetState ( _Lexer_, ( ADD_TOKEN_TO_SOURCE | ADD_CHAR_TO_SOURCE ) ) ) _CSL_UnAppendFromSourceCode_NChars ( csl, Strlen ( ( CString ) tkn ) + 1 ) ;
}

void
_CSL_AppendCharToSourceCode ( CSL * csl, byte c )
{
    csl->SC_Buffer [ csl->SC_Index ++ ] = c ;
    csl->SC_Buffer [ csl->SC_Index ] = 0 ;
}

void
CSL_AppendCharToSourceCode ( CSL * csl, byte c )
{
    if ( csl->SC_Index < ( BUF_IX_SIZE ) )
    {
        if ( c == '"' )
        {
            if ( csl->SC_QuoteMode ) csl->SC_QuoteMode = 0 ;
            else csl->SC_QuoteMode = 1 ;
            _CSL_AppendCharToSourceCode ( csl, c ) ;
        }
        else _String_AppendConvertCharToBackSlash ( csl->SC_Buffer, c, &csl->SC_Index, true ) ;
    }
}

Word *
Get_SourceCodeWord ( )
{
    Debugger * debugger = _Debugger_ ;
    Word * scWord ;
    if ( ! ( scWord = _Context_->CurrentDisassemblyWord ) )
    {
        if ( GetState ( debugger, DBG_STEPPING ) ) scWord = debugger->w_Word ;
        if ( ( ! scWord ) && ( ! ( scWord = GetState ( _Compiler_, ( COMPILE_MODE | ASM_MODE ) ) ?
            _Context_->CurrentWordBeingCompiled : _CSL_->LastFinished_Word ?
            _CSL_->LastFinished_Word : _CSL_->SC_Word ? _CSL_->SC_Word : _Compiler_->Current_Word_Create ) ) )
        {
            if ( debugger && Is_DebugOn )
            {
                if ( debugger->LastShowEffectsWord ) scWord = debugger->LastShowEffectsWord ;
                else scWord = Word_GetFromCodeAddress ( debugger->DebugAddress ) ;
            }
        }
    }
    return (scWord && scWord->W_WordData ) ? scWord : 0 ;
}

#if 0

void
CSL_SourceCodeCompileOn_Colon ( )
{
    CSL_DbgSourceCodeOn ( ) ;
    CSL_SourceCode_Init ( ) ;
    //CSL_WordList_RecycleInit ( CSL ) 
    if ( ! GetState ( _Context_, C_SYNTAX ) ) _CSL_Colon ( 0 ) ;
}

void
CSL_SourceCodeCompileOff_SemiColon ( )
{
    CSL_DbgSourceCodeOff ( ) ;
    if ( ! GetState ( _Context_, C_SYNTAX ) ) CSL_SemiColon ( ) ;
}

void
CSL_DbgSourceCodeBeginBlock ( )
{
    SetState ( _CSL_, DEBUG_SOURCE_CODE_MODE, true ) ;
    _SC_Global_On ;
    if ( ! GetState ( _Context_, C_SYNTAX ) ) CSL_BeginBlock ( ) ;
}

void
CSL_DbgSourceCodeEndBlock ( )
{
    if ( ! GetState ( _Context_, C_SYNTAX ) ) CSL_EndBlock ( ) ;
    CSL_DbgSourceCodeOff ( ) ;
}

#endif

#if 0

void
SC_List_Set_NotInUseForSC ( dlnode * node, byte * address )
{
    Word * word = ( Word* ) dobject_Get_M_Slot ( ( dobject* ) node, SCN_T_WORD ) ;
    if ( word->SourceCoding >= address ) dobject_Set_M_Slot ( ( dobject* ) node, SCN_IN_USE_FLAG, SCN_IN_USE_FOR_OPTIMIZATION ) ;
}

void
CSL_AdjustDbgSourceCode_ScInUseFalse ( byte * address )
{
    dllist * list = _CSL_->Compiler_N_M_Node_WordList ;
    if ( list ) dllist_Map1 ( list, ( MapFunction1 ) SC_List_Set_NotInUseForSC, ( int64 ) address ) ;
}
#endif

