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
    ListObject *l1, *lfunction, *lfirst, *largs, *lnode, *lnext, *le ;
eval_start:
    lc->ApplyFlag = applyFlag ; //= ! GetState (lc, LC_DEFINE_MODE);
    lc->Locals = locals ;
    SetState ( lc, LC_EVAL, true ) ;
    lc->L0 = lc->L1 = l1 = l0 ; // default
    if ( kbhit ( ) == ESC ) OpenVmTil_Pause ( ) ;
    LC_Debug ( lc, LC_EVAL, 1 ) ;
    if ( l0 && ( ! LO_IsQuoted ( l0 ) ) )
    {
        if ( l0->W_LispAttributes & T_LISP_SYMBOL )
            //ListObject * _LC_EvalSymbol ( )
        {
            Word *w ;
            ListObject *l1 = lc->L0 ; // default 
            if ( l1 )
            {
                LC_Debug ( lc, LC_EVAL_SYMBOL, 1 ) ;
                w = LC_FindWord ( l1->Name ) ;
                if ( w )
                {
                    w->W_SC_Index = l1->W_SC_Index ;
                    w->W_RL_Index = l1->W_RL_Index ;
                    if ( w->W_LispAttributes & T_LAMBDA )
                    {
                        lc->Sc_Word = l1 = w ;
                        w->State = l1->State ;
                    }
                    else if ( w->W_LispAttributes & T_LC_DEFINE ) l1 = ( ListObject * ) w->Lo_Value ;
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
                    if ( ( ! ( w->W_LispAttributes & T_LAMBDA ) ) )
                    {
                        w->W_MySourceCodeWord = lc->Sc_Word ;
                        l1->W_MySourceCodeWord = lc->Sc_Word ;
                    }
                }
            }
            // else the node evals to itself
            //lc->L1 = LO_CopyOne ( l1 ) ;
            lc->L1 = l1 ;
            LC_Debug ( lc, LC_EVAL_SYMBOL, 0 ) ;
            return lc->L1 ;
        }

        else if ( l0->W_LispAttributes & ( LIST | LIST_NODE ) )
            //ListObject * LC_EvalList ( )
        {
            LO_CheckEndBlock ( ) ;
            LO_CheckBeginBlock ( ) ;
            lc->ParenLevel ++ ;
            lfirst = _LO_First ( lc->L0 ) ;
            LC_Debug ( lc, LC_EVAL_LIST, 1 ) ;
            lc->Lfirst = lfirst ;
            if ( lfirst )
            {
                if ( lfirst->W_LispAttributes & ( T_LISP_SPECIAL | T_LISP_MACRO ) )
                {
                    if ( LO_IsQuoted ( lfirst ) ) return lfirst ;
                    //ListObject *LC_SpecialFunction ( )
                    ListObject *macro, *lnext, *l1 = lc->L0 ;
                    LC_Debug ( lc, LC_SPECIAL_FUNCTION, 1 ) ;
                    if ( lfirst )
                    {
                        while ( lfirst && ( lfirst->W_LispAttributes & T_LISP_MACRO ) )
                        {
                            lnext = _LO_Next ( lfirst ) ;
                            macro = lfirst ;
                            macro->W_LispAttributes &= ~ T_LISP_MACRO ; // prevent short recursive loop calling of this function thru LO_Eval below
                            l1 = LC_Eval ( macro, locals, applyFlag ) ; //1 ) ;
                            macro->W_LispAttributes |= T_LISP_MACRO ; // restore to its true type
                            lfirst = lnext ;
                        }
                        if ( lfirst && lfirst->Lo_CSL_Word && IS_MORPHISM_TYPE ( lfirst->Lo_CSL_Word ) )
                        {
                            if ( lfirst->W_MorphismAttributes & COMBINATOR ) LC_InitForCombinator ( lc ) ;
                            lc->Lfirst = lfirst ;
                            l1 = ( ( ListFunction0 ) ( lfirst->Lo_CSL_Word->Definition ) ) ( ) ; // ??? : does adding extra parameters to functions not defined with them mess up the the c runtime return stack
                        }
#if 1
                        else l1 = LC_Eval ( lc->L0, locals, applyFlag ) ; //1 ) ;
#else                        
                        else
                        {
                            l0 = lc->L0 ;
                            goto eval_start ;
                        }
#endif                        
                    }
                    LC_Debug ( lc, LC_SPECIAL_FUNCTION, 0 ) ;
                    lc->L1 = l1 ;
                    lc->ParenLevel -- ;
                }
                else
                {
                    lfunction = LC_Eval ( lfirst, locals, applyFlag ) ;
                    lc->Largs0 = _LO_Next ( lfirst ) ;
                    //ListObject * _LC_EvalList ( )
                    //ListObject *l1 = lc->Largs0, *locals = lc->Locals ;
                    l1 = lc->Largs0 ; //, *locals = lc->Locals ;
                    if ( lc->Largs0 )
                    {
                        l1 = LO_New ( LIST, 0 ) ;
                        for ( lnode = lc->Largs0 ; lnode ; lnode = lnext )
                        {
                            lnext = _LO_Next ( lnode ) ;
                            le = LC_Eval ( lnode, locals, applyFlag ) ; // lc->Locals could be changed by eval
                            LO_AddToTail ( l1, LO_CopyOne ( le ) ) ;
                            //LO_AddToTail ( l1, le ) ;
                        }
                    }
                    largs = lc->Largs = l1 ;
                    //ListObject *LC_Apply ( )
                    SetState ( lc, LC_APPLY, true ) ;
                    LC_Debug ( lc, LC_APPLY, 1 ) ;
                    if ( applyFlag && lfunction && ( ( lfunction->W_MorphismAttributes & ( CPRIMITIVE | CSL_WORD ) ) ||
                        ( lfunction->W_LispAttributes & ( T_LISP_COMPILED_WORD | T_LC_IMMEDIATE ) ) ) )
                    {
                        //ListObject * _LO_Apply ( )
                        SetState ( lc, LC_APPLY, true ) ;
                        //if ( ( ! largs ) && ( ( lfunction->W_MorphismAttributes & ( CPRIMITIVE | CSL_WORD ) ) || ( lfunction->W_LispAttributes & ( T_LC_IMMEDIATE ) )
                        if ( ( ! largs ) && ( lfunction->W_MorphismAttributes & ( CSL_WORD ) ) || ( lfunction->W_LispAttributes & ( T_LC_IMMEDIATE ) ) // allows for lisp.csl macros !? but better logic is probably available
                            || ( lfunction->W_LispAttributes & T_LISP_CSL_COMPILED ) )
                        {
                            Interpreter_DoWord ( _Context_->Interpreter0, lfunction->Lo_CSL_Word, lfunction->W_RL_Index, lfunction->W_SC_Index ) ;
                            l1 = nil ;
                        }
                        else if ( largs ) l1 = _LO_Do_FunctionBlock ( lfunction, largs ) ;
                        else
                        {
                            lc->ParenLevel -- ;
                            LO_CheckEndBlock ( ) ;
                            SetState ( lc, LC_COMPILE_MODE, false ) ;
                            l1 = lfunction ;
                        }
                        SetState ( lc, LC_APPLY, false ) ;
                        return lc->L1 = l1 ; // certainly we are done here but why can't we just fall thru to the end??
                    }
                    else if ( lfunction && ( lfunction->W_LispAttributes & T_LAMBDA ) && lfunction->Lo_LambdaBody )
                    {
                        // LambdaArgs, the formal args, are not changed by LO_Substitute (locals - lvals are just essentially 'renamed') and thus don't need to be copied
                        lc->FunctionParameters = lfunction->Lo_LambdaParameters, lc->FunctionArgs = largs ;
                        LC_Substitute ( ) ;
                        lc->L0 = lfunction->Lo_LambdaBody ;
#if 1                        
                        l1 = LC_Eval ( lfunction->Lo_LambdaBody, largs, applyFlag ) ;
#else                        
                        l0 = lfunction->Lo_LambdaBody ;
                        locals = largs ;
                        goto eval_start ;
#endif                        
                    }
                    else
                    {
                        //these cases seems common sense for what these situations should mean and seem to add something positive to the usual lisp/scheme semantics !?
                        if ( ! largs ) l1 = lfunction ;
                        else
                        {
                            LO_AddToHead ( largs, lfunction ) ;
                            l1 = largs ;
                        }
                        if ( ! ( lfunction->W_MorphismAttributes & COMBINATOR ) )
                        {
                            //if ( GetState ( lc, LC_COMPILE_MODE ) )
                            {
                                SetState ( lc, LC_COMPILE_MODE, false ) ;
                                if ( _O_->Verbosity > 1 )
                                {
                                    _LO_PrintWithValue ( lc->Lread, "\nLC_Apply : lc->Lread = ", "", 0 ) ;
                                    CSL_Show_SourceCode_TokenLine ( lfunction, "LC_Debug : ", 0, lfunction->Name, "" ) ;
                                    Printf ( "\nCan't compile this define because \'%s\' is not a function/combinator. Function variables are not yet implemented?!", lfunction->Name ) ;
                                    Printf ( "\nHowever, it should run interpreted." ) ;
                                }
                            }
                        }
                    }
                    lc->L1 = l1 ;
                    SetState ( lc, LC_APPLY, false ) ;
                    LC_Debug ( lc, LC_APPLY, 0 ) ;
                    //OVT_MemList_FreeNBAMemory ( ( byte* ) "LispCopySpace", 2 * M, 1 ) ;
                }
            }
            LC_Debug ( lc, LC_EVAL_LIST, 0 ) ;
        }
        else if ( GetState ( lc, LC_DEBUG_ON ) ) CSL_Show_SourceCode_TokenLine ( l0, "LC_Debug : ", 0, l0->Name, "" ) ;
    }
    SetState ( lc, LC_EVAL, false ) ;
    LC_Debug ( lc, LC_EVAL, 0 ) ;
    return lc->L1 ;
}

