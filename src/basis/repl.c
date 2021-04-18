#include "../include/csl.h"

void
_Repl ( block repl )
{
    Lexer * lexer = _Context_->Lexer0 ;
    ReadLiner * rl = lexer->ReadLiner0 ;

    byte * snp = rl->NormalPrompt, *sap = rl->AltPrompt ;
    SetState ( _LC_, LC_REPL, true ) ;
    if ( _LC_ )
    {
        rl->AltPrompt = ( byte* ) "lc< " ;
        rl->NormalPrompt = ( byte* ) "lc> " ;
    }
    else
    {
        rl->AltPrompt = ( byte* ) "==< " ;
        rl->NormalPrompt = ( byte* ) "==> " ;
    }
    //SetState ( _O_->psi_PrintStateInfo, PSI_NEWLINE, true ) ;
    SetState ( _Context_->System0, ADD_READLINE_TO_HISTORY, true ) ;
    start:
    while ( ! setjmp ( _Context_->JmpBuf0 ) )
    {
        while ( ! GetState ( lexer, END_OF_FILE|END_OF_STRING) )
        {
            uint64 * svDsp = _DspReg_ ;
            //Printf ( "<= " ) ;
            Context_DoPrompt (_Context_, 0) ;
            //LC_SaveStack ( ) ; // ?!? maybe we should do this stuff differently : literals are pushed on the stack by the interpreter
            ReadLine_GetLine ( rl ) ;
            if ( strstr ( ( char* ) rl->InputLineString, ".." ) || strstr ( ( char* ) rl->InputLineString, "bye" ) || strstr ( ( char* ) rl->InputLineString, "exit" )  || strstr ( ( char* ) rl->InputLineString, "x" ) ) 
            {
                Lexer_ReadToken ( _Lexer_ ) ;
                goto done ;
            }
            repl ( ) ;
            //Printf ( "\n" ) ;
            _DspReg_ = svDsp ;
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
    Printf ( "\n_Repl Exiting ... " ) ;
    DefaultColors ;
}

