#include "../include/csl.h"

void
CSL_Debug_AtAddress ( )
{
    byte * address ;
    address = ( byte* ) DataStack_Pop ( ) ;
    _CSL_Debug_AtAddress ( address ) ;
}

void
_CSL_Debugger_Locals_Show ( )
{
    Debugger_Locals_Show ( _Debugger_ ) ;
    //Pause ( ) ;
}

void
CSL_Debugger_Locals_Show ( )
{
    if ( GetState ( _Debugger_, DBG_AUTO_MODE ) ) _CSL_Debugger_Locals_Show ( ) ;
}

void
_CSL_DebugInfo ( )
{
    _Debugger_->w_Word = _Context_->CurrentlyRunningWord ;
    Debugger_ShowInfo ( _Debugger_, ( byte* ) "\ninfo", 0 ) ;
}

// put this '<dbg>' into csl code for a runtime break into the debugger

void
CSL_DebugInfo ( )
{
    if ( _O_->Verbosity )
    {
        _CSL_DebugInfo ( ) ;
        Debugger_Source ( _Debugger_ ) ;
    }
}

void
CSL_DebugOn ( )
{
    Debugger * debugger = _Debugger_ ;
    if ( ! Is_DebugOn )
    {
        if ( _O_->Verbosity > 1 ) Printf ( ( byte* ) "\nCSL_DebugOn : at %s", Context_Location ( ) ) ;
        debugger->DebugRSP = 0 ;
        Debugger_On ( debugger ) ;
    }
#if 0  // no : because it makes debugger ineffective for words that aren't on the using list
    byte * nextToken = Lexer_Peek_Next_NonDebugTokenWord ( cntx->Lexer0, 0, 0 ) ;
    debugger->EntryWord = Finder_Word_FindUsing (cntx->Interpreter0->Finder0, nextToken, 0) ;
    _Context_->SourceCodeWord = debugger->EntryWord ;
#endif     
}

void
CSL_DebugOff ( )
{
    Debugger_Off ( _Debugger_, 1 ) ;
}

void
CSL_DebugRuntimeBreakpoint ( )
{
    if ( ( ! CompileMode ) ) DebugRuntimeBreakpoint ( ) ;
}

void
CSL_DebugRuntimeBreakpoint_IsDebugShowOn ( )
{
    if ( Is_DebugShowOn ) DebugRuntimeBreakpoint ( ) ;
}

void
CSL_DebugRuntimeBreakpoint_IsDebugOn ( )
{
    if ( Is_DebugOn ) DebugRuntimeBreakpoint ( ) ;
}

