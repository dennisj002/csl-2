#include "../../include/csl.h"

Boolean
LO_IsQuoted ( ListObject *l0 )
{
    return (( l0->State & QUOTED ) || ( ( l0->State & QUASIQUOTED ) && ( ! ( l0->State & ( UNQUOTED | UNQUOTE_SPLICE ) ) ) ) ) ;
}

ListObject *
LC_Eval ( LambdaCalculus * lc, ListObject *l0, ListObject *locals, Boolean applyFlag )
{
    ListObject *l1 = l0 ;
    lc->ApplyFlag = applyFlag ;
    SetState ( lc, LC_EVAL, true ) ;
    lc->L0 = l0 ;
    //LC_Debug (lc, LC_EVAL, 1 ) ;
    if ( kbhit ( ) == ESC ) OpenVmTil_Pause ( ) ;
    if ( l0 && ( ! LO_IsQuoted ( l0 ) ) )
    {
        if ( l0->W_LispAttributes & T_LISP_SYMBOL ) l1 = _LC_EvalSymbol ( lc, l0, locals ) ;
        else if ( l0->W_LispAttributes & ( LIST | LIST_NODE ) ) l1 = LC_EvalList ( lc, l0, locals, applyFlag ) ;
        else if ( GetState ( lc, LC_DEBUG_ON ) ) CSL_Show_SourceCode_TokenLine ( l0, "LC_Debug : ", 0, l0->Name, "" ) ;
    }
    SetState ( lc, LC_EVAL, false ) ;
    lc->L1 = l1 ;
    LC_Debug (lc, LC_EVAL, 0 ) ;
    return l1 ;
}

ListObject *
LC_EvalList ( LambdaCalculus * lc, ListObject *l0, ListObject *locals, Boolean applyFlag )
{
    ListObject *l1, *lfunction, *largs0, *largs, *largs2, *lfirst, *scw ;
    LO_CheckEndBlock ( ) ;
    LO_CheckBeginBlock ( ) ;
    lc->ParenLevel ++ ;
    lfirst = _LO_First ( l0 ) ;
    if ( lc->Lfirst = lfirst )
    {
        if ( lfirst->W_LispAttributes & ( T_LISP_SPECIAL | T_LISP_MACRO ) )
        {
            if ( LO_IsQuoted ( lfirst ) ) return lfirst ;
            l1 = LC_SpecialFunction ( lc, l0, locals ) ;
            lc->ParenLevel -- ;
        }
        else
        {
            //lfunction = _LO_Eval ( lc, LO_CopyOne ( lfirst ), locals, applyFlag ) ;
            lfunction = LC_Eval ( lc, lfirst, locals, applyFlag ) ;
            lc->Lfunction = lfunction ;
            //if ( scw = FindSourceCodeWord ( ( ! ( lfunction->W_LispAttributes & T_LISP_SYMBOL ) ) ? lfunction : lfunction->Lo_CSLWord ) ) lc->Sc_Word = scw ;
            largs0 = _LO_Next ( lfirst ) ;
            lc->Largs = largs0 ;
            //if ( DEFINE_DBG ) _LO_PrintWithValue ( lfunction, "\nLO_EvalList : lfunction = ", "" ),  _LO_PrintWithValue ( largs, "", "" ) ;
            largs = _LC_EvalList ( lc, largs0, locals, applyFlag ) ;
            lc->Largs = largs ;
            l1 = LC_Apply ( lc, lfirst, lfunction, largs, applyFlag ) ;
            //largs2 = _LO_EvalList ( lc, largs1, locals, applyFlag ) ;
            //if ( LC_DEFINE_DBG ) _LO_PrintWithValue ( lfunction, "\nLC_EvalList : lfunction = ", "", 1 ),  _LO_PrintWithValue ( largs0, " : largs0", "", 0 ),  _LO_PrintWithValue ( largs1, " : largs1", "", 0 ) ;
            //if ( DEFINE_DBG ) _LO_PrintWithValue ( lfunction, "\nLO_EvalList : lfunction = ", "" ),  _LO_PrintWithValue ( largs1, "", "" ) ; //,  _LO_PrintWithValue ( largs2, "largs2", "" ) ;
            //if ( DEFINE_DBG ) _LO_PrintWithValue ( l0, " : result l0 = ", "" ) ;
        }
    }
    //lc->ParenLevel -- ;
    lc->L1 = l1 ;
    LC_Debug (lc, LC_EVAL_LIST, 0 ) ;
    return l1 ;
}

ListObject *
_LC_EvalSymbol ( LambdaCalculus * lc, ListObject *l0, ListObject *locals )
{
    Word *w ;
    ListObject *l1 = l0 ; // default 
    if ( l1 )
    {
        if ( GetState ( lc, LC_DEBUG_ON ) ) CSL_Show_SourceCode_TokenLine ( l0, "LC_Debug : ", 0, l0->Name, "" ) ;
        w = LC_FindWord ( l1->Name, locals ) ;
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
                //w->State = l1->State ;
            }
            else
            {
                //w->Lo_Value = l1->W_Value ;s
                w->State = l1->State ;
                l1 = w ;
            }
            if ( ( CompileMode ) && LO_CheckBeginBlock ( ) )
                //if ( ( LC_CompileMode ) && LO_CheckBeginBlock ( ) ) 
                _LO_CompileOrInterpret_One ( l1, 0 ) ;
            if ( w->W_MorphismAttributes & COMBINATOR )
                LC_InitForCombinator ( lc ) ;
        }
    }
    // else the node evals to itself
    if ( w && ( ! ( w->W_LispAttributes & T_LAMBDA ) ) )
    {
        if ( w ) w->W_MySourceCodeWord = lc->Sc_Word ;
        if ( l1 ) l1->W_MySourceCodeWord = lc->Sc_Word ;
    }
    l1 = LO_CopyOne ( l1 ) ;
    //if ( DEFINE_DBG ) _LO_PrintWithValue ( l1, "\n_LO_EvalSymbol :", "" ) ;
    return l1 ;
}

ListObject *
_LC_EvalList ( LambdaCalculus * lc, ListObject *lorig, ListObject *locals, Boolean applyFlag )
{
    ListObject *lnew = 0, *lnode, *lnext, *le ;
    if ( lorig )
    {
        lnew = LO_New ( LIST, 0 ) ;
        for ( lnode = lorig ; lnode ; lnode = lnext )
        {
            lnext = _LO_Next ( lnode ) ;
            le = LC_Eval ( lc, lnode, locals, applyFlag ) ;
            //le = _LO_Eval ( lc, LO_CopyOne ( lnode ), locals, applyFlag ) ;
            LO_AddToTail ( lnew, LO_CopyOne ( le ) ) ;
            //if ( DEFINE_DBG )_LO_PrintWithValue ( lnode, "\n_LO_EvalList : lnode : = ", "" ), _LO_PrintWithValue ( le, "\n_LO_EvalList : le : =", " " ),  Printf (" : le->Name = %s", le->Name ), _LO_PrintWithValue ( le1, "\n_LO_EvalList : LO_CopyOne : le1 : ", "" ) ;
        }
    }
    return lnew ;
}

ListObject *
LC_SpecialFunction ( LambdaCalculus * lc, ListObject * l0, ListObject * locals )
{
    ListObject * lfirst, *macro, *l1 = l0 ;
    lc->Locals = locals ;
    LC_Debug (lc, LC_SPECIAL_FUNCTION, 1 ) ;
    if ( lfirst = _LO_First ( l0 ) )
    {
        if ( GetState ( lc, LC_DEBUG_ON ) ) CSL_Show_SourceCode_TokenLine ( lfirst, "LC_Debug : ", 0, lfirst->Name, "" ) ;
        while ( lfirst && ( lfirst->W_LispAttributes & T_LISP_MACRO ) )
        {
            macro = lfirst ;
            macro->W_LispAttributes &= ~ T_LISP_MACRO ; // prevent short recursive loop calling of this function thru LO_Eval below
            if ( GetState ( lc, LC_DEBUG_ON ) ) CSL_Show_SourceCode_TokenLine ( l0, "LC_Debug : ", 0, l0->Name, "" ) ;
            l1 = LC_Eval ( lc, l0, locals, 1 ) ;
            macro->W_LispAttributes |= T_LISP_MACRO ; // restore to its true type
            lfirst = _LO_First ( l0 ) ;
            //macro = 0 ;
        }
        if ( lfirst && lfirst->Lo_CSL_Word && IS_MORPHISM_TYPE ( lfirst->Lo_CSL_Word ) )
        {
            if ( lfirst->W_MorphismAttributes & COMBINATOR ) LC_InitForCombinator ( lc ) ;
            l1 = ( ( LispFunction2 ) ( lfirst->Lo_CSL_Word->Definition ) ) ( lfirst, locals ) ; // ??? : does adding extra parameters to functions not defined with them mess up the the c runtime return stack
        }
        else
        {
            l1 = LC_Eval ( lc, l0, locals, 1 ) ;
        }
    }
    return l1 ;
}

