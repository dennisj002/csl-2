#include "../../include/csl.h"

Boolean
LO_IsQuoted ( ListObject *l0 )
{
    return (( l0->State & QUOTED ) || ( ( l0->State & QUASIQUOTED ) && ( ! ( l0->State & ( UNQUOTED | UNQUOTE_SPLICE ) ) ) ) ) ; //( ! ( l0->State & ( QUOTED | QUASIQUOTED ) )  || (l1->State & UNQUOTED) ) )
}

ListObject *
_LO_Eval ( LambdaCalculus * lc, ListObject *l0, ListObject *locals, Boolean applyFlag )
{
    //d1 ( if ( _Is_DebugOn ) LO_PrintWithValue ( l0 ) ) ;
    SetState ( lc, LC_EVAL, true ) ;
    if ( l0 && ( ! LO_IsQuoted ( l0 ) ) )
    {
        if ( l0->W_LispAttributes & T_LISP_SYMBOL ) l0 = _LO_EvalSymbol ( lc, l0, locals ) ;
        else if ( l0->W_LispAttributes & ( LIST | LIST_NODE ) ) l0 = LO_EvalList ( lc, l0, locals, applyFlag ) ;
    }
    SetState ( lc, LC_EVAL, false ) ;
    //d1 ( if ( _Is_DebugOn ) LO_PrintWithValue ( l0 ) ) ;
    return l0 ;
}

ListObject *
LO_EvalList ( LambdaCalculus * lc, ListObject *l0, ListObject *locals, Boolean applyFlag )
{
    ListObject *lfunction, *largs, *lfirst ;
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
            l0 = LO_SpecialFunction ( lc, l0, locals ) ;
            lc->ParenLevel -- ;
        }
        else
        {
            lfunction = LO_CopyOne ( _LO_Eval ( lc, lfirst, locals, applyFlag ) ) ;
            largs = _LO_EvalList ( lc, _LO_Next ( lfirst ), locals, applyFlag ) ;
            l0 = LO_Apply ( lc, l0, lfirst, lfunction, largs, applyFlag ) ;
        }
    }
    //if ( Is_DebugOn ) Compiler_SC_WordList_Show ( 0, 0, 0 ) ;
    return l0 ;
}

ListObject *
_LO_EvalSymbol ( LambdaCalculus * lc, ListObject *l0, ListObject *locals )
{
    if ( l0 )
    {
        Word *w = LC_FindWord ( l0->Name, locals ) ;
        if ( w )
        {
            Compiler *compiler = _Context_->Compiler0 ;
            if ( w->W_LispAttributes & T_LC_DEFINE ) 
            {
                l0 = ( ListObject * ) w->Lo_Value ;
            }
            else if ( ( w->W_MorphismAttributes & ( CPRIMITIVE | csl_WORD ) )
                || ( w->W_ObjectAttributes & ( LOCAL_VARIABLE | PARAMETER_VARIABLE ) )
                || ( w->W_LispAttributes & ( T_LISP_COMPILED_WORD ) ) )
            {
                l0->Lo_Value = w->W_Value ;
                l0->Lo_CSLWord = w ;
                l0->W_MorphismAttributes |= w->W_MorphismAttributes ;
                l0->W_ObjectAttributes |= w->W_ObjectAttributes ;
                l0->W_LispAttributes |= w->W_LispAttributes ;
            }
            else
            {
                w->W_SC_Index = l0->W_SC_Index ;
                w->W_RL_Index = l0->W_RL_Index ;
                w->State = l0->State ;
                l0 = w ;
            }
            if ( ( CompileMode ) && LO_CheckBeginBlock ( ) ) _LO_CompileOrInterpret_One ( l0, 0 ) ;
            if ( w->W_MorphismAttributes & COMBINATOR )
            {
                SetState ( compiler, LISP_COMBINATOR_MODE, true ) ;
                CombinatorInfo ci ; // remember sizeof of CombinatorInfo = 4 bytes
                ci.BlockLevel = Compiler_BlockLevel ( compiler ) ; //compiler->BlockLevel ;
                ci.ParenLevel = lc->ParenLevel ;
                _Stack_Push ( compiler->CombinatorInfoStack, ( int64 ) ci.CI_i32_Info ) ; // this stack idea works because we can only be in one combinator at a time
            }
        }
    }
    // else the node evals to itself
    return l0 ;
}

ListObject *
_LO_EvalList ( LambdaCalculus * lc, ListObject *lorig, ListObject *locals, Boolean applyFlag )
{
    ListObject *lnew = 0, *lnode, *lnext, *le, *lce ;
    if ( lorig )
    {
        lnew = LO_New ( LIST, 0 ) ;
        for ( lnode = lorig ; lnode ; lnode = lnext ) //_LO_Next ( lnode ) ) // eval each node
        {
            lnext = _LO_Next ( lnode ) ;
            // research : why doesn't this work without copy ? copying here wastes time and memory!!
            //d1 ( if ( _Is_DebugOn ) _LO_PrintWithValue ( lnode, "\n_LO_EvalList : lnode ", "" ) ) ;
            le = _LO_Eval ( lc, lnode, locals, applyFlag ) ;
            lce = LO_CopyOne ( le ) ;
            LO_AddToTail ( lnew, lce ) ;
        }
    }
    return lnew ;
}

