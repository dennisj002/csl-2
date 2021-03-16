#include "../../include/csl.h"

Boolean
LO_IsQuoted ( ListObject *l0 )
{
    return (( l0->State & QUOTED ) || ( ( l0->State & QUASIQUOTED ) && ( ! ( l0->State & ( UNQUOTED | UNQUOTE_SPLICE ) ) ) ) ) ;
}

ListObject *
_LO_Eval ( LambdaCalculus * lc, ListObject *l0, ListObject *locals, Boolean applyFlag )
{
    if ( LC_DEFINE_DBG ) _LO_PrintWithValue (l0, "\n_LO_Eval : l0 = ", "" , 1) ;
    ListObject *l1 = l0 ;
    SetState ( lc, LC_EVAL, true ) ;
    if ( kbhit ( ) == ESC ) OpenVmTil_Pause ( ) ;
    if ( l0 && ( ! LO_IsQuoted ( l0 ) ) )
    {
        if ( l0->W_LispAttributes & T_LISP_SYMBOL ) l1 = _LO_EvalSymbol ( lc, l0, locals ) ;
        else if ( l0->W_LispAttributes & ( LIST | LIST_NODE ) ) l1 = LO_EvalList ( lc, l0, locals, applyFlag ) ;
    }
    SetState ( lc, LC_EVAL, false ) ;
    if ( LC_DEFINE_DBG ) _LO_PrintWithValue (l1, "\n_LO_Eval : l1 = ", "" , 1) ;
    return l1 ;
}

ListObject *
LO_EvalList ( LambdaCalculus * lc, ListObject *l0, ListObject *locals, Boolean applyFlag )
{
    ListObject *lfunction, *largs, *largs1, *largs2, *lfirst, *scw ;
    if ( CompileMode )
    {
        LO_CheckEndBlock ( ) ;
        LO_CheckBeginBlock ( ) ;
    }
    lc->ParenLevel ++ ;
    lfirst = _LO_First ( l0 ) ;
    if ( lfirst )
    {
        if ( lfirst->W_LispAttributes & ( T_LISP_SPECIAL | T_LISP_MACRO ) )
        {
            if ( LO_IsQuoted ( lfirst ) ) return lfirst ;
            //if ( DEFINE_DBG ) _LO_PrintWithValue ( l0, "\nLO_EvalList : special function = ", "" ) ;
            //if ( DEFINE_DBG ) _LO_PrintWithValue ( locals, "\nLO_EvalList : locals = ", "" ) ;
            l0 = LO_SpecialFunction ( lc, l0, locals ) ;
            lc->ParenLevel -- ;
        }
        else
        {
            //lfunction = _LO_Eval ( lc, LO_CopyOne ( lfirst ), locals, applyFlag ) ;
            lfunction = _LO_Eval ( lc, lfirst, locals, applyFlag ) ;
            if ( scw = FindSourceCodeWord ( lfunction ) ) lc->Sc_Word = scw ;
            largs = _LO_Next ( lfirst ) ;
            //if ( DEFINE_DBG ) _LO_PrintWithValue ( lfunction, "\nLO_EvalList : lfunction = ", "" ),  _LO_PrintWithValue ( largs, "", "" ) ;
            largs1 = _LO_EvalList ( lc, largs, locals, applyFlag ) ;
            l0 = LO_Apply ( lc, l0, lfirst, lfunction, largs1, applyFlag ) ;
            //largs2 = _LO_EvalList ( lc, largs1, locals, applyFlag ) ;
            if ( LC_DEFINE_DBG ) _LO_PrintWithValue ( lfunction, "\nLO_EvalList : lfunction = ", "", 1 ),  _LO_PrintWithValue ( largs, " : largs", "", 0 ),  _LO_PrintWithValue ( largs1, " : args1", "", 0 ) ;
            //if ( DEFINE_DBG ) _LO_PrintWithValue ( lfunction, "\nLO_EvalList : lfunction = ", "" ),  _LO_PrintWithValue ( largs1, "", "" ) ; //,  _LO_PrintWithValue ( largs2, "largs2", "" ) ;
            //if ( DEFINE_DBG ) _LO_PrintWithValue ( l0, " : result l0 = ", "" ) ;
        }
    }
    //if ( DEFINE_DBG ) _LO_PrintWithValue ( l0, "\n_LO_EvalList : l0 = ", "" ) ;
    return l0 ;
}

ListObject *
_LO_EvalSymbol ( LambdaCalculus * lc, ListObject *l0, ListObject *locals )
{
    Word *w ;
    ListObject *l1 = l0 ;
    if ( l1 )
    {
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
                || ( w->W_ObjectAttributes & ( LOCAL_VARIABLE | PARAMETER_VARIABLE ) )
                || ( w->W_LispAttributes & ( T_LISP_COMPILED_WORD ) ) )
            {
                l1->Lo_Value = w->W_Value ;
                l1->Lo_CSLWord = w ;
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
_LO_EvalList ( LambdaCalculus * lc, ListObject *lorig, ListObject *locals, Boolean applyFlag )
{
    ListObject *lnew = 0, *lnode, *lnext, *le ;
    if ( lorig )
    {
        lnew = LO_New ( LIST, 0 ) ;
        for ( lnode = lorig ; lnode ; lnode = lnext )
        {
            lnext = _LO_Next ( lnode ) ;
            le = _LO_Eval ( lc, lnode, locals, applyFlag ) ;
            //le = _LO_Eval ( lc, LO_CopyOne ( lnode ), locals, applyFlag ) ;
            LO_AddToTail ( lnew, LO_CopyOne ( le ) ) ;
            //if ( DEFINE_DBG )_LO_PrintWithValue ( lnode, "\n_LO_EvalList : lnode : = ", "" ), _LO_PrintWithValue ( le, "\n_LO_EvalList : le : =", " " ),  Printf (" : le->Name = %s", le->Name ), _LO_PrintWithValue ( le1, "\n_LO_EvalList : LO_CopyOne : le1 : ", "" ) ;
        }
    }
    return lnew ;
}

