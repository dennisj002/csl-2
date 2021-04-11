
#include "../../include/csl.h"

void
LC_Debug ( LambdaCalculus * lc, byte * lcFuncName, int64 state, Boolean setupFlag )
{
    if ( GetState ( lc, LC_DEBUG_ON ) )
    {
        Debugger * debugger = _Debugger_ ;
        Printf ( "\nLC_Debug :: %s : ", lcFuncName ) ;
        //debugger->Menu = "Debug Menu at : \n%s :\n" ;
        DebugColors ;
        if ( lc->ApplyFlag ) LC_Debug_Output ( lc, state, setupFlag ) ;
        Debugger_InterpreterLoop ( debugger ) ;
        DefaultColors ;
    }
}

void
LC_Debug_Output ( LambdaCalculus * lc, int64 state, Boolean setupFlag )
{
    if ( setupFlag )
    {
        switch ( state )
        {
            case LC_APPLY:
            {
                _LO_PrintWithValue ( lc->Lfunction, "LC_Apply : lfunction = ", "", 1 ), _LO_PrintWithValue ( lc->Largs, " : largs = ", "", 0 ) ;
                break ;
            }
            case LC_EVAL:
            {
                _LO_PrintWithValue ( lc->L0, "\n_LC_Eval : l0 = ", "", 1 ) ;
                break ;
            }
            case LC_SPECIAL_FUNCTION:
            {
                _LO_PrintWithValue ( lc->L0, "\nLC_SpecialFuncion : special function = ", "", 1 ), _LO_PrintWithValue ( lc->Locals, " : locals = ", "", 0 ) ;
                break ;
            }
            default: break ;
        }
    }
    else
    {
        switch ( state )
        {
            case LC_APPLY:
            {
                //_LO_PrintWithValue ( lc->Lfunction, "\nLC_Apply : lfunction = ", "", 1 ), _LO_PrintWithValue ( lc->Largs, " : largs = ", "", 0 ) ;
                //SetState ( debugger, DBG_MENU, true ) ;
                _LO_PrintWithValue ( lc->Lfunction, "LC_Apply : lfunction = ", "", 1 ), _LO_PrintWithValue ( lc->Largs, " : largs = ", "", 0 ), _LO_PrintWithValue ( lc->L1, " : result = ", "", 0 ) ;
                break ;
            }
            case LC_EVAL:
            {
                _LO_PrintWithValue ( lc->L1, "\n_LC_Eval : l1 = ", "", 1 ) ;
                break ;
            }
            case LC_EVAL_LIST:
            {
                //_LO_PrintWithValue ( lc->L1, "\nLC_EvalList : l1 = ", "", 1 ) ;
                _LO_PrintWithValue ( lc->Lfunction, "\nLC_EvalList : lfunction = ", "", 1 ), _LO_PrintWithValue ( lc->Largs, " : largs0 = ", "", 0 ), _LO_PrintWithValue ( lc->Largs1, " : largs1 = ", "", 0 ) ;
                break ;
            }
            case LC_SUBSTITUTE:
            {
                _LO_PrintWithValue ( lc->LambdaParameters, "\nLO_Substitute : lambdaParameters = ", "", 1 ), _LO_PrintWithValue ( lc->FunctionCallValues, " : funcCallValues = ", "", 0 ) ;
                break ;
            }
            default: break ;
        }
    }
}
