
#include "../include/csl.h"

void
Interpreter_Init ( Interpreter * interp )
{
    _O_->OVT_Interpreter = interp ;
    interp->State = 0 ;
}

Interpreter *
Interpreter_New ( uint64 type )
{
    Interpreter * interp = ( Interpreter * ) Mem_Allocate ( sizeof (Interpreter ), type ) ;

    interp->Lexer0 = Lexer_New ( type ) ;
    interp->ReadLiner0 = interp->Lexer0->ReadLiner0 ;
    //interp->Lexer0->OurInterpreter = interp ;
    interp->Finder0 = Finder_New ( type ) ;
    interp->Compiler0 = Compiler_New ( type ) ;
    Interpreter_Init ( interp ) ;
    return interp ;
}

void
_Interpreter_Copy ( Interpreter * interp, Interpreter * interp0 )
{
    MemCpy ( interp, interp0, sizeof (Interpreter ) ) ;
}

Interpreter *
Interpreter_Copy ( Interpreter * interp0, uint64 type )
{
    Interpreter * interp = ( Interpreter * ) Mem_Allocate ( sizeof (Interpreter ), type ) ;
    _Interpreter_Copy ( interp, interp0 ) ;
    Interpreter_Init ( interp ) ;
    return interp ;
}

int64
Interpreter_IsDone ( Interpreter * interp, uint64 flags )
{
    Interpreter_SetLexState ( interp ) ;
    return GetState ( interp, flags | INTERPRETER_DONE ) ;
}

