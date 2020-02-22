#include "../../include/csl.h"

/*
 * csl namespaces basic understandings :
 * 0. to allow for ordered search thru lists in use ...
 * 1. Namespaces are also linked on the CSL->Namespaces list with status of USING or NOT_USING
 *      and are moved the front or back of that list if status is set to USING with the 'Symbol node'
 *      This list is ordered so we can set an order to our search thru the namespaces in use. As usual
 *      the first word found within the ordered USING list will be used.
 */

void
_Namespace_DoAddSymbol ( Namespace * ns, Symbol * symbol )
{
    dllist_AddNodeToHead ( ns->W_List, ( dlnode* ) symbol ) ;
}

void
Namespace_DoAddSymbol ( Namespace * ns, Symbol * symbol )
{
    if ( ! ns->W_List ) ns->W_List = dllist_New ( ) ;
    _Namespace_DoAddSymbol ( ns, symbol ) ;
    symbol->S_ContainingNamespace = ns ;
}

void
_Namespace_DoAddWord ( Namespace * ns, Word * word, int64 addFlag )
{
    Namespace_DoAddSymbol ( ns, ( Symbol* ) word ) ;
    if ( addFlag ) _CSL_->WordsAdded ++ ;
}

void
Namespace_DoAddWord ( Namespace * ns, Word * word )
{
    if ( ns && word ) _Namespace_DoAddWord ( ns, word, 1 ) ;
}

void
_Namespace_AddToNamespacesHead ( Namespace * ns )
{
    _Namespace_DoAddSymbol ( _CSL_->Namespaces, ns ) ;
}

void
_Namespace_AddToNamespacesTail ( Namespace * ns )
{
    dllist_AddNodeToTail ( _CSL_->Namespaces->W_List, ( dlnode* ) ns ) ;
}

void
Namespace_Do_Namespace ( Namespace * ns, Boolean isForwardDotted )
{
    Context * cntx = _Context_ ;
    if ( ( ! CompileMode ) || GetState ( cntx, C_SYNTAX | LISP_MODE ) ) _Namespace_Do_Namespace ( ns ) ;
    else if ( ( ! isForwardDotted ) && ( ! GetState ( cntx->Compiler0, LC_ARG_PARSING ) ) )
    {
        _Compile_C_Call_1_Arg ( ( byte* ) _Namespace_Do_Namespace, ( int64 ) ns ) ;
    }
}

void
_CSL_SetAsInNamespace ( Namespace * ns )
{
    _CSL_->InNamespace = ns ;
}

void
_Namespace_AddToNamespacesHead_SetAsInNamespace ( Namespace * ns )
{
    _Namespace_AddToNamespacesHead ( ns ) ;
    _CSL_SetAsInNamespace ( ns ) ;
}

Namespace *
_CSL_Namespace_InNamespaceSet ( Namespace * ns )
{
    if ( ns )
    {
        _Namespace_SetState ( ns, USING ) ;
        _Namespace_AddToNamespacesHead_SetAsInNamespace ( ns ) ;
    }
    return ns ;
}

Namespace *
CSL_Namespace_InNamespaceSet ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    _CSL_Namespace_InNamespaceSet ( ns ) ;
    return ns ;
}

Namespace *
_CSL_Namespace_InNamespaceGet ( )
{
    if ( _CSL_->Namespaces && ( ! _CSL_->InNamespace ) )
    {
        _CSL_Namespace_InNamespaceSet ( _Namespace_FirstOnUsingList ( ) ) ; //( Namespace* ) _Tree_Map_FromANode ( ( dlnode* ) CSL->Namespaces, ( cMapFunction_1 ) _Namespace_IsUsing ) ;
    }
    return _CSL_->InNamespace ;
}

Namespace *
CSL_In_Namespace ( )
{
    Namespace * ins ;
    if ( ( ins = Finder_GetQualifyingNamespace ( _Context_->Finder0 ) ) ) return ins ;
    else return _CSL_Namespace_InNamespaceGet ( ) ;
}

Boolean
_CSL_IsContainingNamespace ( byte * name, byte * namespaceName )
{
    Word * word = _Finder_Word_Find ( _Finder_, USING, name ) ;
    if ( word && String_Equal ( ( char* ) word->ContainingNamespace->Name, namespaceName ) ) return true ;
    else return false ;
}

void
_Namespace_Do_Namespace ( Namespace * ns )
{
    Context * cntx = _Context_ ;
    if ( ! Lexer_IsTokenForwardDotted ( cntx->Lexer0 ) ) _Namespace_ActivateAsPrimary ( ns ) ; //Namespace_SetState ( ns, USING ) ; //
    else Finder_SetQualifyingNamespace ( cntx->Finder0, ns ) ;
    if ( ! GetState ( cntx->Compiler0, ( LC_ARG_PARSING | ARRAY_MODE ) ) ) cntx->Interpreter0->BaseObject = 0 ;
}

void
Namespace_DoNamespace_Name ( byte * name )
{
    Namespace_Do_Namespace ( Namespace_Find ( name ), 0 ) ;
}

Boolean
_Namespace_IsUsing ( Namespace * ns )
{
    if ( GetState ( ns, USING ) ) return 1 ;
    return 0 ;
}

Boolean
Namespace_IsUsing ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    return _Namespace_IsUsing ( ns ) ;
}

void
_Namespace_SetState ( Namespace * ns, uint64 state )
{
    if ( ns )
    {
        if ( state == USING ) SetState_TrueFalse ( ns, USING, NOT_USING ) ;
        else SetState_TrueFalse ( ns, NOT_USING, USING ) ;
    }
}

Word *
_Namespace_FirstOnUsingList ( )
{
    Word * ns, *nextNs ;
    for ( ns = ( Namespace* ) dllist_First ( ( dllist* ) _CSL_->Namespaces->W_List ) ; ns ; ns = nextNs )
    {
        nextNs = ( Word* ) dlnode_Next ( ( node* ) ns ) ;
        if ( Is_NamespaceType ( ns ) && ( ns->State & USING ) ) return ns ;
    }
    return 0 ;
}

void
_Namespace_ResetFromInNamespace ( Namespace * ns )
{
    if ( ns == _CSL_->InNamespace ) _CSL_SetAsInNamespace ( _Namespace_FirstOnUsingList ( ) ) ; //( Namespace* ) dllist_First ( (dllist*) CSL->Namespaces->W_List ) ;
}

void
_Namespace_AddToNamespacesTail_ResetFromInNamespace ( Namespace * ns )
{
    _Namespace_AddToNamespacesTail ( ns ) ;
    _Namespace_ResetFromInNamespace ( ns ) ;
}

void
Namespaces_PrintList ( Namespace * ns, byte * insert )
{
    //CSL_Namespaces_PrettyPrintTree ( ) ;
    //CSL_Using ( ) ;
    Printf ( ( byte* ) "\n\nNamespace : %s :: %s _Namespace_SetState : \n\t", ns->Name, insert ) ;
    _List_PrintNames ( _CSL_->Namespaces->W_List, 5, 0 ) ;
}

void
Namespace_SetState_AdjustListPosition ( Namespace * ns, uint64 state, Boolean setInNsFlag )
{
    if ( ns )
    {
        d0 ( if ( Is_DebugModeOn ) Namespaces_PrintList ( ns, "Before" ) ) ;
        _Namespace_SetState ( ns, state ) ;
        if ( state == USING )
        {
            //_Namespace_AddToNamespacesHead_SetAsInNamespace ( ns ) ; // make it first on the list
            _Namespace_AddToNamespacesHead ( ns ) ;
            if ( setInNsFlag ) _CSL_SetAsInNamespace ( ns ) ;
        }
        else _Namespace_AddToNamespacesTail_ResetFromInNamespace ( ns ) ;
        d0 ( if ( Is_DebugModeOn )
        {
            Namespaces_PrintList ( ns, "After" ) ; CSL_Using ( ) ; } ) ;
    }
}

void
_Namespace_AddToUsingList ( Namespace * ns )
{
    int64 i ;
    Namespace * svNs = ns ;
    Stack * stack = _Compiler_->InternalNamespacesStack ;
    Stack_Init ( stack ) ;
    do
    {
        if ( ns == _CSL_->Namespaces ) break ;
        _Stack_Push ( stack, ( int64 ) ns ) ;
        ns = ns->ContainingNamespace ;
    }
    while ( ns ) ;
    for ( i = Stack_Depth ( stack ) ; i > 0 ; i -- )
    {
        ns = ( Word* ) _Stack_Pop ( stack ) ;
        if ( ns->WL_OriginalWord ) ns = ns->WL_OriginalWord ; //_Namespace_Find ( ns->Name, 0, 0 ) ; // this is needed because of Compiler_PushCheckAndCopyDuplicates
        Namespace_SetState_AdjustListPosition ( ns, USING, 0 ) ;
    }
    if ( ns != svNs )
    {
        //CSL_Using () ;
        Namespace_SetState_AdjustListPosition ( svNs, USING, 1 ) ;
        //CSL_Using () ;
    }
    else _Namespace_SetState ( ns, USING ) ;
}

void
_Namespace_ActivateAsPrimary ( Namespace * ns )
{
    if ( ns )
    {
        ns = Word_UnAlias ( ns ) ;
        Finder_SetQualifyingNamespace ( _Context_->Finder0, ns ) ;
        _Namespace_AddToUsingList ( ns ) ;
        _CSL_SetAsInNamespace ( ns ) ;
        _Context_->Interpreter0->BaseObject = 0 ;
    }
}

void
Namespace_ActivateAsPrimary ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    _Namespace_ActivateAsPrimary ( ns ) ;
}

void
Namespace_MoveToTail ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    _Namespace_AddToNamespacesTail_ResetFromInNamespace ( ns ) ;
}

void
_Namespace_SetAsNotUsing ( Namespace * ns )
{
    if ( ns )
    {
        _Namespace_SetState ( ns, NOT_USING ) ;
        _Namespace_ResetFromInNamespace ( ns ) ;
    }
}

void
_Namespace_SetStateAs_NotUsing ( Namespace * ns )
{
    _Namespace_SetState ( ns, NOT_USING ) ;
}

void
_Namespace_SetState_AsUsing ( Namespace * ns )
{
    _Namespace_SetState ( ns, USING ) ;
}

void
Namespace_SetAsNotUsing ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    _Namespace_SetAsNotUsing ( ns ) ;
}

void
_Namespace_SetAsNotUsing_MoveToTail ( Namespace * ns )
{
    //_Namespace_AddToNamespacesTail_ResetFromInNamespace ( ns ) ;
    _Namespace_SetAsNotUsing ( ns ) ;
    _Namespace_AddToNamespacesTail ( ns ) ;
}

void
Namespace_SetAsNotUsing_MoveToTail ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    _Namespace_SetAsNotUsing_MoveToTail ( ns ) ;
}

void
_Namespace_SetAs_UsingLast ( byte * name )
{
    _Namespace_AddToNamespacesTail_ResetFromInNamespace ( Namespace_Find ( name ) ) ;
}

// this is simple, for more complete use _Namespace_RemoveFromSearchList
// removes all namespaces dependant on 'ns', the whole dependancy tree from the 'ns' root

void
_RemoveSubNamespacesFromUsingList ( Symbol * symbol, Namespace * ns )
{
    Namespace * ns1 = ( Namespace* ) symbol ;
    // if ns contains ns1 => ns1 is dependent on ns ; we are removing ns => we have to remove ns1
    if ( ( ns1->W_ObjectAttributes & NAMESPACE_TYPE ) && ( ns1->ContainingNamespace == ns ) )
    {
        _Namespace_RemoveFromUsingList ( ns1 ) ;
    }
}

void
_Namespace_RemoveFromUsingList ( Namespace * ns )
{
    _Namespace_SetAsNotUsing_MoveToTail ( ns ) ;
    _Namespace_MapUsing_2Args ( ( MapSymbolFunction2 ) _RemoveSubNamespacesFromUsingList, ( int64 ) ns, 0 ) ;
}

void
Namespace_RemoveFromUsingList ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    if ( String_Equal ( ns->Name, "System" ) ) Printf ( ( byte* ) "\n\nSystem namespace being cleared %s", Context_Location ( ) ) ;
    if ( ns ) _Namespace_RemoveFromUsingList ( ns ) ;
}

void
Namespace_MoveToFirstOnUsingList ( byte * name )
{
    _Namespace_AddToUsingList ( Namespace_Find ( name ) ) ; // so it will be first on the list where Find will find it first
}

void
Namespace_RemoveFromUsingList_WithCheck ( byte * name )
{
    if ( ! String_Equal ( "Root", ( char* ) name ) )
    {
        Namespace_RemoveFromUsingList ( name ) ;
    }
    else Throw ( ( byte* ) "Namespace_RemoveFromUsingList_WithCheck", ( byte* ) "Error : can't remove Root namespace", QUIT ) ;
}

void
_Namespace_Clear ( Namespace * ns )
{
    if ( ns )
    {
        DLList_Recycle_NamespaceList ( ns->W_List ) ;
        DLList_RemoveWords ( ns->W_List ) ;
        _dllist_Init ( ns->W_List ) ;
    }
}

void
Namespace_Clear ( byte * name )
{
    _Namespace_Clear ( _Namespace_Find ( name, 0, 0 ) ) ;
}

void
_Namespace_RemoveFromUsingListAndClear ( Namespace * ns )
{
    if ( ns )
    {
        if ( ns == _CSL_->InNamespace ) _CSL_->InNamespace = 0 ; //( Namespace* ) dlnode_Next ( ( dlnode* ) ns ) ; //dllist_First ( (dllist*) _O_->CSL->Namespaces->Lo_List ) ;
        if ( ns == _Context_->Finder0->QualifyingNamespace ) Finder_SetQualifyingNamespace ( _Context_->Finder0, 0 ) ;
        _Namespace_Clear ( ns ) ;
        dlnode_Remove ( ( dlnode* ) ns ) ;
        Word_Recycle ( ns ) ;
    }
}

int64
_Namespace_VariableValueGet ( Namespace * ns, byte * name )
{
    Word * word = _CSL_VariableGet ( ns, name ) ;
    if ( word ) return ( int64 ) word->W_Value ; // value of variable
    else return 0 ;
}

void
_Namespace_VariableValueSet ( Namespace * ns, byte * name, int64 value )
{
    Word * word = _CSL_VariableGet ( ns, name ) ;
    if ( word )
    {
        word->W_Value = value ; // value of variable
        word->W_PtrToValue = & word->W_Value ;
    }
}

Namespace *
Namespace_New ( byte * name, Namespace * containingNs )
{
    Namespace * ns = DataObject_New ( NAMESPACE, 0, name, 0, NAMESPACE, 0, 0, 0, containingNs, 0, 0, - 1 ) ;
    return ns ;
}

// a namespaces internal finder, a wrapper for Symbol_Find - prefer Symbol_Find directly

Namespace *
_Namespace_Find ( byte * name, Namespace * superNamespace, int64 exceptionFlag )
{
    if ( name )
    {
        Word * word = 0 ;
        if ( superNamespace ) word = _Finder_FindWord_InOneNamespace ( _Finder_, superNamespace, name ) ;
        if ( ! word ) word = Finder_FindWord_UsedNamespaces ( _Finder_, name ) ;
        if ( word && Is_NamespaceType ( word ) ) return ( Namespace* ) Word_UnAlias ( word ) ;
        else if ( exceptionFlag )
        {
            Printf ( ( byte* ) "\nUnable to find Namespace : %s\n", name ) ;
            CSL_Exception ( NAMESPACE_ERROR, 0, 1 ) ;
            return 0 ;
        }
    }
    return 0 ;
}

Namespace *
Namespace_Find ( byte * name )
{
    Namespace * ns = _Namespace_Find ( name, 0, 0 ) ;
    return ns ;
}

Namespace *
Namespace_FindOrNew_SetUsing ( byte * name, Namespace * containingNs, int64 setUsingFlag )
{
    if ( ! containingNs ) containingNs = _CSL_->Namespaces ;
    Namespace * ns = _Finder_FindWord_InOneNamespace ( _Finder_, containingNs, name ) ; //_Namespace_Find ( name, containingNs, 0 ) ;
    if ( ! ns ) ns = Namespace_New ( name, containingNs ) ;
    if ( setUsingFlag ) Namespace_SetState_AdjustListPosition ( ns, USING, 1 ) ;
    return ns ;
}

Namespace *
_Namespace_FindOrNew_Local ( Stack * nsStack )
{
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    Namespace * ns ;
    int64 d = Stack_Depth ( compiler->BlockStack ) ;
    byte bufferData [ 32 ], *name = ( byte* ) bufferData ;
    snprintf ( ( char* ) name, 32, "locals_%ld", d - 1 ) ; // 1 : BlockStack starts at 1 
    ns = _Namespace_Find ( name, _CSL_->Namespaces, 0 ) ;
    if ( ! ns )
    {
        ns = Namespace_New ( name, _CSL_->Namespaces ) ;
        if ( CompileMode ) Stack_Push ( nsStack, ( int64 ) ns ) ; // nb. this is where the the depth increase
        else compiler->NonCompilingNs = ns ;
    }
    Namespace_SetState_AdjustListPosition ( ns, USING, 1 ) ;
    _Namespace_ActivateAsPrimary ( ns ) ;
    return ns ;
}

Namespace *
Namespace_FindOrNew_Local ( Stack * nsStack, Boolean setBlockFlag )
{
    Namespace * ns = _Namespace_FindOrNew_Local ( nsStack ) ;
    if ( setBlockFlag )
    {
        BlockInfo * bi = ( BlockInfo * ) Stack_Top ( _Context_->Compiler0->BlockStack ) ;
        if ( bi ) bi->BI_LocalsNamespace = ns ;
    }
    return ns ;
}

void
Symbol_NamespacePrettyPrint ( Symbol * symbol, int64 indentFlag, int64 indentLevel )
{
    Namespace * ns = ( Namespace* ) symbol ;
    Namespace_PrettyPrint ( ns, indentFlag, indentLevel ) ;
}

void
_Namespace_PrintWords ( Namespace * ns )
{
    dllist_Map1 ( ns->Lo_List, ( MapFunction1 ) _Word_Print, 0 ) ;
}

void
Namespace_PrintWords ( byte * name )
{
    Namespace * ns ;
    if ( ns = Namespace_Find ( name ) ) dllist_Map1 ( ns->Lo_List, ( MapFunction1 ) _Word_Print, 0 ) ;
}

void
_Namespace_MapAny_2Args ( MapSymbolFunction2 msf2, int64 one, int64 two )
{
    Tree_Map_Namespaces_State_2Args ( _CSL_->Namespaces->W_List, ANY, msf2, one, two ) ;
}

void
_Namespace_MapUsing_2Args ( MapSymbolFunction2 msf2, int64 one, int64 two )
{
    Tree_Map_Namespaces_State_2Args ( _CSL_->Namespaces->W_List, USING, msf2, one, two ) ;
}

void
CSL_SetInNamespaceFromBackground ( )
{
    Context * cntx = _Context_ ;
    if ( cntx->Compiler0->C_FunctionBackgroundNamespace ) _CSL_Namespace_InNamespaceSet ( cntx->Compiler0->C_FunctionBackgroundNamespace ) ;
    else Compiler_SetAs_InNamespace_C_BackgroundNamespace ( cntx->Compiler0 ) ;
}

#if 0

void
Namespace_AddToNamespaces_SetUsing ( Namespace * ns, Boolean addToHeadFlag, Boolean usingFlag )
{
    if ( ns )
    {
        if ( addToHeadFlag ) _Namespace_AddToNamespacesHead ( ns ) ;
        else _Namespace_AddToNamespacesTail ( ns ) ;
        //if ( usingFlag ) SetState_TrueFalse ( ns, USING, NOT_USING ) ;
        //else SetState_TrueFalse ( ns, NOT_USING, USING ) ;
        _Namespace_SetState ( ns, usingFlag ) ;
    }
}
#endif

