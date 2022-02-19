
#include "../include/csl.h"

// ?? the logic of exceptions *definitely* could be reworked ??

// this block needs to be rethought but it may be ok for now

void
OVT_CheckThrowState ( int64 signal, int64 restartCondition )
{
    OVT_SetRestartCondition ( _O_, restartCondition ) ;
    if ( _O_->RestartCondition == COMPLETE_INITIAL_START )
    {
        Printf ( "%s", _O_->ExceptionMessage ) ;
        _OVT_SigLongJump ( & _O_->JmpBuf0 ) ;
    }
    if ( GetState ( _O_, OVT_THROW ) ) CSL_FullRestartComplete ( ) ;
    else if ( ( signal & ( SIGSEGV | SIGBUS ) ) ) SetState ( _O_, OVT_THROW, true ) ;
}

// OVT_Throw and related functions should be rethought and/or cleaned up

void
OVT_Throw ( int signal, int64 restartCondition, Boolean pauseFlag )
{
    sigjmp_buf * jb ;
    Word * eword ;
    OVT_CheckThrowState ( signal, restartCondition ) ;
    if ( GetState ( _O_, OVT_FRC ) ) jb = & _O_->JmpBuf0 ;
    else
    {
        _OpenVmTil_ShowExceptionInfo ( ) ;
        if ( signal )
        {
            if ( ( signal == SIGTERM ) || ( signal == SIGKILL ) || ( signal == SIGQUIT ) || ( signal == SIGSTOP ) || ( signal == SIGHUP ) ) OVT_Exit ( ) ;
            else if ( signal & ( SIGSEGV | SIGBUS ) ) _O_->SigSegvs ++ ;
            else if ( signal == SIGBUS )
            {
                jb = & _O_->JmpBuf0 ;
                OVT_SetRestartCondition ( _O_, INITIAL_START ) ;
                _OVT_SimpleFinal_Key_Pause ( _O_ ) ;
                goto jump ;
            }
            if ( ( restartCondition > QUIT ) || ( _O_->Signal == SIGFPE ) )
            {
                if ( _O_->SigSegvs < 2 )
                {
                    jb = & _CSL_->JmpBuf0 ;
                    OpenVmTil_ShowExceptionInfo ( ) ;
                    pauseFlag ++ ;
                    OVT_SetRestartCondition ( _O_, ABORT ) ;
                }
                else OVT_SetRestartCondition ( _O_, INITIAL_START ) ;
                if ( _O_->SigSegvs > 3 ) _OVT_SigLongJump ( & _O_->JmpBuf0 ) ; //OVT_Exit ( ) ;
                else if ( ( _O_->SigSegvs > 1 ) || ( restartCondition == INITIAL_START ) ) jb = & _O_->JmpBuf0 ;
            }
            else jb = & _CSL_->JmpBuf0 ;
        }
        else
        {
            if ( restartCondition >= INITIAL_START ) jb = & _O_->JmpBuf0 ;
            else jb = & _CSL_->JmpBuf0 ;
        }
        //OVT_SetExceptionMessage ( _O_ ) ;
        eword = _Context_->CurrentTokenWord ; //_Context_->CurrentEvalWord ;
        snprintf ( Buffer_Data_Cleared ( _O_->PrintBuffer ), BUFFER_IX_SIZE, "\n%s\n%s %s from %s : the current evalWord is %s.%s -> ...", _O_->ExceptionMessage,
            ( jb == & _CSL_->JmpBuf0 ) ? "reseting csl" : "restarting OpenVmTil",
            ( _O_->Signal == SIGSEGV ) ? ": SIGSEGV" : "", Context_Location ( ),
            ( eword ? ( eword->S_ContainingNamespace ? eword->S_ContainingNamespace->Name : ( byte* ) "" ) : ( byte* ) "" ), ( eword ? eword->Name : ( byte* ) "" ) ) ;
        if ( pauseFlag && ( _O_->SignalExceptionsHandled < 2 ) && ( _O_->SigSegvs < 2 ) ) OVT_Pause ( 0, _O_->SignalExceptionsHandled ) ;
        else if ( _O_->SigSegvs < 3 ) _OVT_SimpleFinal_Key_Pause ( _O_ ) ;
    }
jump:
    _OVT_SigLongJump ( jb ) ;
}

void
_OpenVmTil_ShowExceptionInfo ( )
{
    Word * word = _O_->ExceptionWord ;
    Debugger * debugger = _Debugger_ ;
    DebugOn ;
    if ( _Context_->CurrentlyRunningWord ) CSL_Show_SourceCode_TokenLine ( _Context_->CurrentlyRunningWord, "", _O_->RestartCondition, _Context_->CurrentlyRunningWord->Name, "" ) ;
    if ( ! _O_->ExceptionCode & ( STACK_ERROR | STACK_OVERFLOW | STACK_UNDERFLOW ) ) Debugger_Stack ( debugger ) ;
    if ( ! word )
    {
        word = Finder_Word_FindUsing ( _Finder_, _O_->ExceptionToken, 1 ) ;
        if ( ! word )
        {
            if ( _O_->SigAddress ) word = Word_GetFromCodeAddress ( ( byte* ) _O_->SigAddress ) ;
            if ( ( ! debugger->w_Word ) && ( ! debugger->Token ) )
            {
                debugger->w_Word = _Context_->CurrentEvalWord ? _Context_->CurrentEvalWord : _Context_->LastEvalWord ;
            }
        }
    }
    AlertColors ;
    SetState ( debugger, DBG_INFO, true ) ;
    Debugger_Locals_Show ( debugger ) ;

    if ( GetState ( debugger, DBG_STEPPING ) )
    {
        Debugger_Registers ( debugger ) ;
        Debugger_UdisOneInstruction ( debugger, 0, debugger->DebugAddress, ( byte* ) "", ( byte* ) "" ) ;
    }
    Debugger_ShowInfo ( debugger, _O_->ExceptionMessage, _O_->Signal ) ;
    if ( word != _Context_->LastEvalWord ) _CSL_Source ( word, 0 ) ;
    Printf ( "\nOpenVmTil_SignalAction : address = 0x%016lx : %s", _O_->SigAddress, _O_->SigLocation ) ;
}

int64
OpenVmTil_ShowExceptionInfo ( )
{
    if ( _O_->Verbosity )
    {
        if ( _O_->ExceptionMessage )
        {
            Printf ( "\n%s : %s\n",
                _O_->ExceptionMessage, _O_->ExceptionSpecialMessage ? _O_->ExceptionSpecialMessage : Context_Location ( ) ) ;
        }
        if ( ( _O_->SigSegvs < 2 ) && ( _O_->SignalExceptionsHandled ++ < 2 ) && _CSL_ )
        {
            _DisplaySignal ( _O_->Signal ) ;
            _OpenVmTil_ShowExceptionInfo ( ) ;
        }
    }
    int64 rtrn = OVT_Pause ( 0, _O_->SignalExceptionsHandled ) ;
    _O_->Signal = 0 ;
    return rtrn ;
}

void
OVT_PauseInterpret ( Context * cntx, byte key )
{
    ReadLiner * rl = cntx->ReadLiner0 ;
    byte * svPrompt ;
    Boolean svDbgState = GetState ( _CSL_, DEBUG_MODE | _DEBUG_SHOW_ ) ;
    Boolean svcm = GetState ( cntx->Compiler0, ( COMPILE_MODE ) ) ;
    Boolean svath = GetState ( rl, ADD_TO_HISTORY ) ;
    int64 lastKey = 0, loop = 0 ;
    OpenVmTil_AddStringToHistoryOn ( ) ;
    Set_CompileMode ( false ) ;
    DebugOff ;
    ReadLine_Init ( rl, _CSL_Key ) ;
    SetState ( cntx, AT_COMMAND_LINE, true ) ;
    if ( ( key <= ' ' ) || ( key == '\\' ) ) key = 0 ;
    Printf ( "\nPause interpreter : hit <enter> or <esc> to exit : \n" ) ;
    do
    {
        svPrompt = ReadLine_GetPrompt ( rl ) ;
        ReadLine_SetPrompt ( rl, "=> " ) ;
        Context_DoPrompt ( cntx, 0 ) ;
        _ReadLine_GetLine ( rl, key ) ;
        if ( ReadLine_PeekNextChar ( rl ) < ' ' ) break ; // '\n', <esc>, etc.
        Interpret_ToEndOfLine ( cntx->Interpreter0 ) ;
        CSL_NewLine ( ) ;
        if ( key == lastKey ) loop ++ ; // stop looping here, just noticed 0.907.39x??
        else loop = 0 ;
        lastKey = key ;
        key = 0 ;
    }
    while ( loop < 5 ) ;
    ReadLine_SetPrompt ( rl, svPrompt ) ;
    SetState ( cntx, AT_COMMAND_LINE, false ) ;
    ReadLine_SetRawInputFunction ( rl, ReadLine_GetNextCharFromString ) ;
    SetState ( _Context_->ReadLiner0, ADD_TO_HISTORY, svath ) ;
    SetState ( cntx->Compiler0, ( COMPILE_MODE ), svcm ) ;
    SetState ( _CSL_, DEBUG_MODE | _DEBUG_SHOW_, svDbgState ) ;
}

int64
OVT_Pause ( byte * prompt, int64 signalExceptionsHandled )
{
    int64 rtrn = 0 ;
    if ( _CSL_ && _Context_ )
    {
        SetState ( _O_, OVT_PAUSE, true ) ;
        Debugger * debugger = _Debugger_ ;
        if ( _Context_->CurrentlyRunningWord ) _Debugger_->ShowLine = ( byte* ) "" ;
        byte buffer [512], *defaultPrompt =
            ( byte * ) "\n%s\n%s : at %s :: %s'd' (d)ebugger, 't' s(t)ack, c' (c)ontinue, 'q' (q)uit, 'x' e(x)it, 'i' '\\' or <key> (i)interpret, <return> loop%s" ;
        snprintf ( ( char* ) buffer, 512, prompt ? ( char* ) prompt : ( char* ) defaultPrompt, _O_->ExceptionMessage ? ( char* ) _O_->ExceptionMessage : "\r",
            c_gd ( "pause" ), _Context_Location ( _Context_ ), c_gd ( _Debugger_->ShowLine ? _Debugger_->ShowLine : _Context_->ReadLiner0->InputLine ),
            c_gd ( "\n-> " ) ) ;
        DebugColors ;
        int64 tlw = Strlen ( defaultPrompt ) ;
        if ( tlw > _Debugger_->TerminalLineWidth ) _Debugger_->TerminalLineWidth = tlw ;
        if ( signalExceptionsHandled ) Printf ( "\n_OVT_Pause : Signals Handled = %d : signal = %d : restart condition = %d\n",
            signalExceptionsHandled, _O_->Signal,
            _O_->RestartCondition ) ;
        do
        {
            if ( ( ! debugger->w_Word ) && ( ! debugger->Token ) ) debugger->w_Word = Context_CurrentWord ( ) ;
            _Debugger_ShowInfo ( _Debugger_, ( byte* ) "\r", _O_->Signal, 0 ) ;
            Printf ( "%s", buffer ) ;

            int64 key = Key ( ) ;
            _ReadLine_PrintfClearTerminalLine ( ) ;
            Printf ( "\nPause : press <enter> to exit : \n" ) ;
            switch ( key )
            {
                case 'x': case 'X':
                {
                    byte * msg = ( byte * ) "Exit csl from pause?" ;
                    Printf ( "\n%s : 'x' to e(x)it csl : any other <key> to continue%s", msg, c_gd ( "\n-> " ) ) ;
                    key = Key ( ) ;
                    if ( ( key == 'x' ) || ( key == 'X' ) ) OVT_Exit ( ) ;
                    goto done ;
                }
                case 'q':
                {
                    byte * msg = ( byte * ) "Quit to interpreter loop from pause?" ;
                    Printf ( "\n%s : 'q' to (q)uit : any other key to continue%s", msg, c_gd ( "\n-> " ) ) ;
                    key = Key ( ) ;
                    if ( ( key == 'q' ) || ( key == 'Q' ) ) DefaultColors, CSL_Quit ( ) ;
                    goto done ;
                }
                case 'd':
                {
                    if ( ! Is_DebugOn ) CSL_DebugOn ( ) ;
                    SetState ( _Debugger_, DBG_INFO | DBG_MENU | DBG_PROMPT, true ) ;
                    SetState ( debugger, DBG_AUTO_MODE | DBG_AUTO_MODE_ONCE, false ) ; // stop auto mode and get next key command code
                    Debugger_InterpreterLoop ( debugger ) ;
                    //goto done ;
                    goto done ;
                }
                case 'c':
                {
                    // if ( signalsHandled ) ;
                    rtrn = 1 ;
                    goto done ;
                }
                case 't':
                {
                    CSL_PrintDataStack ( ) ;
                    break ;
                }
                case '\n':
                case '\r':
                case ' ':
                {
                    goto done ;
                }
                case 'i':
                {
                    // interpret in context
                    Context * cntx = _Context_ ;
                    OVT_PauseInterpret ( cntx, key ) ;
                    goto done ;
                }
                default:
                {
                    // new context
                    Context * cntx = CSL_Context_PushNew ( _CSL_ ) ;
                    OVT_PauseInterpret ( cntx, key ) ;
                    CSL_Context_PopDelete ( _CSL_ ) ;
                    goto done ;
                }
            }
        }
        while ( 1 ) ;
    }
done:
    DefaultColors ;

    return rtrn ;
}

int64
_OpenVmTil_Pause ( byte * msg )
{
    Printf ( "\n%s", msg ) ;
    return OVT_Pause ( 0, _O_->SignalExceptionsHandled ) ;
}

void
OpenVmTil_Pause ( )
{
    DebugColors ;
    _OpenVmTil_Pause ( Context_Location ( ) ) ;
}

void
OVT_ResetSignals ( int64 signals )
{
    sigset_t signal_set ;
    sigemptyset ( &signal_set ) ;
    sigaddset ( &signal_set, signals ) ;
    sigprocmask ( SIG_UNBLOCK, &signal_set, NULL ) ;
}

void
_OVT_SigLongJump ( sigjmp_buf * jb )
{
    siglongjmp ( *jb, 1 ) ;
}

void
OVT_SigLongJump ( byte * restartMessage, sigjmp_buf * jb )
{
    printf ( "\n%s\n", restartMessage ) ;
    _OVT_SigLongJump ( jb ) ;

}

void
OVT_SetRestartCondition ( OpenVmTil *ovt, int64 restartCondition )
{
    ovt->LastRestartCondition = ovt->RestartCondition ;
    ovt->RestartCondition = restartCondition ;
}

// OVT_Throw needs to be reworked ???

void
OpenVmTil_Throw ( byte * excptMessage, byte * specialMessage, int64 restartCondition, int64 infoFlag )
{
    _O_->ExceptionMessage = excptMessage ;
    _O_->Thrown = restartCondition ;
    _O_->ExceptionSpecialMessage = specialMessage ;

    if ( infoFlag ) OpenVmTil_ShowExceptionInfo ( ) ;
    OVT_Throw ( _O_->Signal, restartCondition, 1 ) ;
}

void
_OpenVmTil_LongJmp_WithMsg ( int64 restartCondition, byte * msg )
{
    OpenVmTil_Throw ( msg, 0, restartCondition, 0 ) ;
}

void
OpenVmTil_SignalAction ( int signal, siginfo_t * si, void * uc ) //nb. void ptr (uc) necessary 
{
    d0 ( Printf ( "\nOpenVmTil_SignalAction :: signal = %d\n", signal ) ) ;
    if ( ( signal == SIGTERM ) || ( signal == SIGKILL ) || ( signal == SIGQUIT ) || ( signal == SIGSTOP ) ) OVT_Exit ( ) ;
    //if ( _O_ )
    {
        _O_->Signal = signal ;
        _O_->SigAddress = si->si_addr ; //( Is_DebugOn && _Debugger_->DebugAddress ) ? _Debugger_->DebugAddress : si->si_addr ;
        _O_->SigLocation = ( ( ! ( signal & ( SIGSEGV | SIGBUS ) ) ) && _Context_ ) ? ( byte* ) c_gd ( Context_Location ( ) ) : ( byte* ) "" ;
        OVT_ResetSignals ( _O_->Signal ) ;
        if ( ( signal >= SIGCHLD ) || ( signal == SIGTRAP ) ) //||( signal == SIGBUS ))
        {
            if ( ( signal != SIGCHLD ) && ( signal != SIGWINCH ) && ( signal != SIGTRAP ) ) OpenVmTil_ShowExceptionInfo ( ) ;
            else
            {
                // ignore this category -- just return
                _O_->SigAddress = 0 ; //|| ( signal == SIGWINCH ) ) _O_->SigAddress = 0 ; // 17 : "CHILD TERMINATED" : ignore; its just back from a shell fork
                _O_->Signal = 0 ;
            }
        }
        else if ( _O_->SigSegvs > 1 )
        {
            OVT_SeriousErrorPause ( ) ;
            _OVT_SigLongJump ( & _O_->JmpBuf0 ) ;
        }
        else OVT_Throw ( _O_->Signal, _O_->RestartCondition, 1 ) ;
    }
    //else  _OVT_SigLongJump ( & _OSMS_->JmpBuf0 ) ; // ?! doesn't work
}

void
CSL_Exception ( int64 exceptionCode, byte * message, int64 restartCondition )
{
    byte * b = Buffer_Data ( _CSL_->ScratchB1 ) ;
    AlertColors ;
    _O_->ExceptionCode = exceptionCode ;
    printf ( "\n\nCSL_Exception at %s\n", Context_Location ( ) ) ;
    fflush ( stdout ) ;
    switch ( exceptionCode )
    {
        case CASE_NOT_LITERAL_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Syntax Error : \"case\" only takes a literal/constant as its parameter after the block", 0, restartCondition, 1 ) ;
            break ;
        }
        case DEBUG_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Debug Error : User is not in debug mode", 0, restartCondition, 1 ) ;
            break ;
        }
        case OBJECT_REFERENCE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Object Reference Error", 0, restartCondition, 1 ) ;
            break ;
        }
        case OBJECT_SIZE_ERROR:
        {
            sprintf ( ( char* ) b, "Exception : Warning : Class object size is 0. Did you declare 'size' for %s? ",
                _Context_->CurrentlyRunningWord->ContainingNamespace->Name ) ;
            OpenVmTil_Throw ( b, 0, restartCondition, 1 ) ;
            break ;
        }
        case STACK_OVERFLOW:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Stack Overflow", 0, restartCondition, 1 ) ;
            break ;
        }
        case STACK_UNDERFLOW:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Stack Underflow", 0, restartCondition, 1 ) ;
            break ;
        }
        case STACK_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Stack Error", 0, restartCondition, 1 ) ;
            break ;
        }
        case SEALED_NAMESPACE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : New words can not be added to sealed namespaces", 0, restartCondition, 1 ) ;
            break ;
        }
        case NAMESPACE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Namespace (Not Found?) Error", 0, restartCondition, 1 ) ;
            break ;
        }
        case SYNTAX_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Syntax Error", message, restartCondition, 1 ) ;
            break ;
        }
        case NESTED_COMPILE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Nested Compile Error", 0, restartCondition, 1 ) ;
            break ;
        }
        case COMPILE_TIME_ONLY:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Compile Time Use Only", 0, restartCondition, 1 ) ;
            break ;
        }
        case BUFFER_OVERFLOW:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Buffer Overflow", 0, restartCondition, 1 ) ;
            break ;
        }
        case MEMORY_ALLOCATION_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Memory Allocation Error", 0, restartCondition, 1 ) ;
            break ;
        }
        case LABEL_NOT_FOUND_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Word not found. Misssing namespace qualifier?", 0, QUIT, 1 ) ;
            break ;
        }
        case NOT_A_KNOWN_OBJECT:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Not a known object", message, QUIT, 1 ) ;
            break ;
        }
        case ARRAY_DIMENSION_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Array has no dimensions!?", 0, QUIT, 1 ) ;
            break ;
        }
        case INLINE_MULTIPLE_RETURN_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Multiple return points in a inlined function", 0, restartCondition, 1 ) ;
            break ;
        }
        case MACHINE_CODE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : in machine coding", 0, restartCondition, 1 ) ;
            break ;
        }
        case VARIABLE_NOT_FOUND_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Variable not found error", 0, restartCondition, 1 ) ;
            break ;
        }
        case USEAGE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Useage Error", 0, restartCondition, 1 ) ;
            break ;
        }
        case FIX_ME_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Fix Me", 0, restartCondition, 1 ) ;
            break ;
        }
        case OUT_OF_CODE_MEMORY:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Out of Code Memory : Increase Code Memory Size for Startup!!", 0, INITIAL_START, 1 ) ;
            break ;
        }
        default:
        {
            OpenVmTil_Throw ( message, 0, restartCondition, 1 ) ;
            break ;
        }
    }
    return ;
}

void
CSL_SystemBreak ( )
{
    _OpenVmTil_LongJmp_WithMsg ( BREAK, ( byte* ) "System.interpreterBreak : returning to main interpreter loop." ) ;
}

void
CSL_Quit ( )
{
    _OpenVmTil_LongJmp_WithMsg ( QUIT, ( byte* ) "Quit : Session Memory, temporaries, are reset." ) ;
}

void
CSL_Abort ( )
{
    _OpenVmTil_LongJmp_WithMsg ( ABORT, ( byte* ) "Abort : Session Memory and the DataStack are reset (as in a cold restart)." ) ;
}

void
CSL_DebugStop ( )
{
    _OpenVmTil_LongJmp_WithMsg ( STOP, ( byte* ) "Stop : Debug Stop. " ) ;
}

void
CSL_ResetAll ( )
{
    _OpenVmTil_LongJmp_WithMsg ( RESET_ALL, ( byte* ) "ResetAll. " ) ;
}

void
CSL_Restart ( )
{
    _OpenVmTil_LongJmp_WithMsg ( RESTART, ( byte* ) "Restart. " ) ;
}

void
CSL_WarmInit ( )
{
    _CSL_Init_SessionCore ( _CSL_, 1, 1 ) ;
}

// cold init

void
CSL_RestartInit ( )
{
    _OpenVmTil_LongJmp_WithMsg ( RESET_ALL, ( byte* ) "Restart Init... " ) ;
}

void
CSL_FullRestart ( )
{
    _O_->Signal = 0 ;
    SetState ( _O_, OVT_THROW, false ) ;
    _OpenVmTil_LongJmp_WithMsg ( INITIAL_START, ( byte* ) "Full Initial Re-Start : ..." ) ;
}

void
CSL_FullRestartComplete ( )
{
    _O_->Signal = 0 ;
    _O_->State = OVT_FRC ;
    Printf ( "\nHistory will be deleted with this FullRestartComplete... \n" ) ;
    //fflush ( stdout ) ;
    _OpenVmTil_LongJmp_WithMsg ( COMPLETE_INITIAL_START, ( byte* ) "Complete Initial Re-Start : ...\n" ) ;
}

void
_Error ( byte * msg, uint64 state )
{
    AlertColors ;
    if ( ( state ) <= PAUSE )
    {
        CSL_NewLine ( ) ;
        CSL_Location ( ) ;
        Printf ( msg ) ;
        CSL_Show_SourceCode_TokenLine ( _Context_->CurrentlyRunningWord, "", _O_->RestartCondition, _Context_->CurrentlyRunningWord->Name, "" ) ;
        Pause ( ) ;
        DebugColors ;
    }
    else
    {
        if ( ( state ) > QUIT )
        {
            Throw ( msg, "", state ) ;
        }
    }
}

void
OVT_ExceptionState_Print ( )
{
    Printf ( "\nSignalExceptionsHandled = %d ; SigSegvs = %d ; Restarts = %d\nStartedTimes = %d ; RestartCondition = %s ; LastRestartCondtion = %s\n",
        _O_->SignalExceptionsHandled, _O_->SigSegvs, _O_->Restarts, _O_->StartedTimes, Convert_RestartCondtion ( _O_->LastRestartCondition ),
        Convert_RestartCondtion ( _O_->RestartCondition ) ) ;
}

byte
_OVT_SimpleFinal_Key_Pause ( OpenVmTil * ovt )
{
    //OVT_CheckThrowState ( ) ;
    byte * msg = Buffer_Data ( ovt->PrintBuffer ) ;
    byte key, * instr = ".: (p)ause, e(x)it, <key> restart" ;
    if ( ovt->SigSegvs < 3 ) printf ( "%s\n%s : at %s : (SIGSEGVs == %ld)", msg, instr, ( ( ovt->SigSegvs ++ < 2 ) ? Context_Location ( ) : ( byte* ) "" ), ovt->SigSegvs ), fflush ( stdout ) ;
    key = Key ( ) ;
    if ( key == 'p' )
    {
        Pause ( ) ;
        return false ;
    }
    else if ( key == 'x' ) OVT_Exit ( ) ;
    return key ;
}

void
OVT_SeriousErrorPause ( )
{
    _OVT_SimpleFinal_Key_Pause ( _O_ ) ;
    fflush ( stdout ) ;
    Key ( ) ;
}

void
OVT_Assert ( Boolean testBoolean, byte * msg )
{
    d1
        (
    if ( ! ( testBoolean ) )
    {
        Printf ( "\nAssert failed : %s : at %s", msg ? msg : ( byte* ) "", _Context_Location ( _Context_ ) ) ;
            _Throw ( QUIT ) ;
    }
    ) ;
}
#if 0

void
OVT_SetExceptionMessage ( OpenVmTil * ovt )
{
    if ( ovt->RestartCondition == INITIAL_START ) ovt->ExceptionMessage = ( byte* ) "Full Initial Re-Start : ..." ;
    else if ( ovt->RestartCondition == ABORT ) ovt->ExceptionMessage = ( byte* ) "Aborting : ..." ;
}
#endif

