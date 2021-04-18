
#include "../../include/csl.h"

void
LC_Debug ( LambdaCalculus * lc, ListObject * lword, int64 state, Boolean setupFlag )
{
    if ( GetState ( lc, LC_DEBUG_ON ) )
    {
        Debugger * debugger = _Debugger_ ;
        //Printf ( "\nLC_Debug :: %s : ", lcFuncName ) ;
        //debugger->Menu = "Debug Menu at : \n%s :\n" ;
        DebugColors ;
        if ( lc->ApplyFlag ) LC_Debug_Output ( lc, state, setupFlag ) ;
        SetState_TrueFalse ( debugger, DBG_NEWLINE, ( DBG_INFO | DBG_PROMPT | DBG_MENU ) ) ;
        //Debugger_Menu ( debugger ) ;
        //if ( lword ) CSL_Show_SourceCode_TokenLine ( lword, "LC_Debug : ", 0, lword->Name, "" ) ;
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
                CSL_Show_SourceCode_TokenLine ( lc->Lfunction, "LC_Debug : ", 0, lc->Lfunction->Name, "" ) ;
                break ;
            }
            case LC_EVAL:
            {
                if ( lc->L0 ) _LO_PrintWithValue ( lc->L0, "\nLC_Eval : l0 = ", "", 1 ) ; //, Printf ( "%s", lc->L0->Name ) ;
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
                if ( lc->Lfunction )
                {
                    _LO_PrintWithValue ( lc->Lfunction, "LC_Apply : lfunction = ", "", 1 ), _LO_PrintWithValue ( lc->Largs, " : largs = ", "", 0 ), Compiling ? Printf ( " : Compiled" ) : _LO_PrintWithValue ( lc->L1, " : result = ", "", 0 ) ;
                    CSL_Show_SourceCode_TokenLine ( lc->Lfunction, "LC_Debug : ", 0, lc->Lfunction->Name, "" ) ;
                }
                break ;
            }
            case LC_EVAL:
            {
                if ( lc->L0 )
                    _LO_PrintWithValue ( lc->L0, "\nLC_Eval : l0 = ", "", 1 ) ;
                else if ( lc->L00 ) _LO_PrintWithValue ( lc->L00, "\nLC_Eval : l00 = ", "", 1 ) ; //Printf ( "\nLC_Eval : l00 = %s", lc->L00->Name ) ;
                if (lc->L1) _LO_PrintWithValue ( lc->L1, " : l1 = ", "", 0 ), Compiling ? Printf ( " : Compiled" ) : 0 ;
                //_LO_PrintWithValue ( lc->L1, "\nLC_Eval : l1 = ", "", 1 ), Compiling ? Printf ( " : Compiled" ) : 0 ;
                break ;
            }
            case LC_EVAL_LIST:
            {
                _LO_PrintWithValue ( lc->Lfunction, "\nLC_EvalList : lfunction = ", "", 1 ), _LO_PrintWithValue ( lc->Largs, " : largs = ", "", 0 ), _LO_PrintWithValue ( lc->Locals, " : locals = ", "", 0 ) ;
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
