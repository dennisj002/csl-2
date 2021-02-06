#include "../include/csl.h"

void
_Repl ( block repl )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;

    byte * snp = rl->NormalPrompt, *sap = rl->AltPrompt ;
    SetState ( _LC_, LC_REPL, true ) ;
    rl->NormalPrompt = ( byte* ) "<= " ;
    rl->AltPrompt = ( byte* ) "=> " ;
    //SetState ( _O_->psi_PrintStateInfo, PSI_NEWLINE, true ) ;
    SetState ( _Context_->System0, ADD_READLINE_TO_HISTORY, true ) ;
    start:
    while ( ! setjmp ( _Context_->JmpBuf0 ) )
    {
        while ( 1 )
        {
            uint64 * svDsp = _Dsp_ ;
            Printf ( "<= " ) ;
            //LC_SaveStack ( ) ; // ?!? maybe we should do this stuff differently : literals are pushed on the stack by the interpreter
            ReadLine_GetLine ( rl ) ;
            if ( strstr ( ( char* ) rl->InputLineString, ".." ) || strstr ( ( char* ) rl->InputLineString, "bye" ) || strstr ( ( char* ) rl->InputLineString, "exit" ) ) goto done ;
            repl ( ) ;
            Printf ( "\n" ) ;
            _Dsp_ = svDsp ;
       }
    }
    {
        AlertColors ;
        Printf ( "\n_Repl Error ... continuing" ) ;
        DefaultColors ;
        goto start ;
    }
done:
    rl->NormalPrompt = snp ;
    rl->AltPrompt = sap ;
    SetState ( _LC_, LC_REPL, false ) ;
    //AllowNewlines ;
}

