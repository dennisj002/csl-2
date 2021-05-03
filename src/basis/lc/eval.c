#include "../../include/csl.h"

Boolean
LO_IsQuoted ( ListObject *l0 )
{
    return (( l0->State & QUOTED ) || ( ( l0->State & QUASIQUOTED ) && ( ! ( l0->State & ( UNQUOTED | UNQUOTE_SPLICE ) ) ) ) ) ;
}

ListObject *
LC_Eval ( ListObject *l0, ListObject *locals, Boolean applyFlag )
{
    LambdaCalculus * lc = _LC_ ;
    ListObject *l1 = l0 ;
    lc->ApplyFlag = applyFlag ;
    lc->Locals = locals ;
    SetState ( lc, LC_EVAL, true ) ;
    lc->L0 = l0 ;
    if ( kbhit ( ) == ESC ) OpenVmTil_Pause ( ) ;
    LC_Debug ( lc, LC_EVAL, 1 ) ;
    if ( l0 && ( ! LO_IsQuoted ( l0 ) ) )
    {
        if ( l0->W_LispAttributes & T_LISP_SYMBOL ) l1 = _LC_EvalSymbol () ;
        else if ( l0->W_LispAttributes & ( LIST | LIST_NODE ) ) l1 = LC_EvalList ( l0 ) ;
        else if ( GetState ( lc, LC_DEBUG_ON ) ) CSL_Show_SourceCode_TokenLine ( l0, "LC_Debug : ", 0, l0->Name, "" ) ;
    }
    SetState ( lc, LC_EVAL, false ) ;
    lc->L1 = l1 ;
    LC_Debug ( lc, LC_EVAL, 0 ) ;
    return l1 ;
}

ListObject *
LC_EvalList ( ListObject *l0 )
{
    LambdaCalculus * lc = _LC_ ;
    ListObject *l1, *lfunction, *largs0, *largs, *lfirst ;
    ListObject * locals = lc->Locals ;
    Boolean applyFlag = lc->ApplyFlag ;
    LO_CheckEndBlock ( ) ;
    LO_CheckBeginBlock ( ) ;
    lc->ParenLevel ++ ;
    lfirst = _LO_First ( l0 ) ;
    if ( lfirst )
    {
        if ( lfirst->W_LispAttributes & ( T_LISP_SPECIAL | T_LISP_MACRO ) )
        {
            if ( LO_IsQuoted ( lfirst ) ) return lfirst ;
            l1 = LC_SpecialFunction ( l0, locals ) ;
            lc->ParenLevel -- ;
        }
        else
        {
            lfunction = LC_Eval ( lfirst, locals, applyFlag ) ;
            largs0 = _LO_Next ( lfirst ) ;
            lc->Largs0 = largs0 ;
            largs = _LC_EvalList () ;
            lc->Largs = largs ;
            lc->Lfunction = lfunction ;
            l1 = LC_Apply ( ) ;
        }
    }
    lc->L1 = l1 ;
    LC_Debug ( lc, LC_EVAL_LIST, 0 ) ;
    return l1 ;
}

ListObject *
_LC_EvalSymbol ()
{
    LambdaCalculus * lc = _LC_ ;
    Word *w ;
    ListObject *l1 = lc->L0 ; // default 
    if ( l1 )
    {
        if ( GetState ( lc, LC_DEBUG_ON ) ) CSL_Show_SourceCode_TokenLine ( l1, "LC_Debug : ", 0, l1->Name, "" ) ;
        w = LC_FindWord ( l1->Name, lc->Locals ) ;
        if ( w )
        {
            w->W_SC_Index = l1->W_SC_Index ;
            w->W_RL_Index = l1->W_RL_Index ;
            if ( w->W_LispAttributes & T_LAMBDA )
            {
                lc->Sc_Word = l1 = w ;
                w->State = l1->State ;
            }
            else if ( w->W_LispAttributes & T_LC_DEFINE )
            {
                l1 = ( ListObject * ) w->Lo_Value ;
            }
            else if ( ( w->W_MorphismAttributes & ( CPRIMITIVE | CSL_WORD ) )
                || ( w->W_ObjectAttributes & ( LOCAL_VARIABLE | PARAMETER_VARIABLE | NAMESPACE_VARIABLE ) )
                || ( w->W_LispAttributes & ( T_LISP_COMPILED_WORD ) ) )
            {
                l1->Lo_Value = w->W_Value ;
                l1->Lo_CSL_Word = w ;
                l1->W_MorphismAttributes |= w->W_MorphismAttributes ;
                l1->W_ObjectAttributes |= w->W_ObjectAttributes ;
                l1->W_LispAttributes |= w->W_LispAttributes ;
            }
            else
            {
                w->State = l1->State ;
                l1 = w ;
            }
            if ( ( CompileMode ) && LO_CheckBeginBlock ( ) ) _LO_CompileOrInterpret_One ( l1, 0 ) ;
            if ( w->W_MorphismAttributes & COMBINATOR ) LC_InitForCombinator ( lc ) ;
        }
    }
    // else the node evals to itself
    if ( w && ( ! ( w->W_LispAttributes & T_LAMBDA ) ) )
    {
        if ( w ) w->W_MySourceCodeWord = lc->Sc_Word ;
        if ( l1 ) l1->W_MySourceCodeWord = lc->Sc_Word ;
    }
    l1 = LO_CopyOne ( l1 ) ;
    return l1 ;
}

ListObject *
_LC_EvalList () 
{
    LambdaCalculus * lc = _LC_ ;
    ListObject *l1 = 0, *lnode, *lnext, *le, *locals = lc->Locals ;
    if ( lc->Largs0 )
    {
        l1 = LO_New ( LIST, 0 ) ;
        for ( lnode = lc->Largs0 ; lnode ; lnode = lnext )
        {
            lnext = _LO_Next ( lnode ) ;
            le = LC_Eval ( lnode, locals, lc->ApplyFlag ) ; // lc->Locals could be changed by eval
            LO_AddToTail ( l1, LO_CopyOne ( le ) ) ;
        }
    }
    return l1 ;
}

ListObject *
LC_SpecialFunction ( ListObject * l0, ListObject * locals )
{
    LambdaCalculus * lc = _LC_ ;
    ListObject * lfirst, *macro, *l1 = l0 ;
    lc->Locals = locals ;
    LC_Debug ( lc, LC_SPECIAL_FUNCTION, 1 ) ;
    if ( lfirst = _LO_First ( l0 ) )
    {
        if ( GetState ( lc, LC_DEBUG_ON ) ) CSL_Show_SourceCode_TokenLine ( lfirst, "LC_Debug : ", 0, lfirst->Name, "" ) ;
        while ( lfirst && ( lfirst->W_LispAttributes & T_LISP_MACRO ) )
        {
            macro = lfirst ;
            macro->W_LispAttributes &= ~ T_LISP_MACRO ; // prevent short recursive loop calling of this function thru LO_Eval below
            if ( GetState ( lc, LC_DEBUG_ON ) ) CSL_Show_SourceCode_TokenLine ( l0, "LC_Debug : ", 0, l0->Name, "" ) ;
            l1 = LC_Eval ( l0, locals, 1 ) ;
            macro->W_LispAttributes |= T_LISP_MACRO ; // restore to its true type
            lfirst = _LO_First ( l0 ) ;
        }
        if ( lfirst && lfirst->Lo_CSL_Word && IS_MORPHISM_TYPE ( lfirst->Lo_CSL_Word ) )
        {
            if ( lfirst->W_MorphismAttributes & COMBINATOR ) LC_InitForCombinator ( lc ) ;
            l1 = ( ( ListFunction2 ) ( lfirst->Lo_CSL_Word->Definition ) ) ( lfirst, locals ) ; // ??? : does adding extra parameters to functions not defined with them mess up the the c runtime return stack
        }
        else l1 = LC_Eval ( l0, locals, 1 ) ;
    }
    return l1 ;
}

