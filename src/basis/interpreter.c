
#include "../include/csl.h"

#if NEW_INTERPRET

void
Interpreter_InitInfixModule ( Interpreter * interp )
{
    //Stack_Init ( interp->InfixOpStack ) ;
    Stack_InitQuick ( interp->InfixOpStack ) ;
    interp->InfixInterpState = IMS_INIT ;
}
#endif

void
_Interpreter_Init ( Interpreter * interp )
{
    _O_->OVT_Interpreter = interp ;
    interp->State = 0 ;
}

void
Interpreter_Init ( Interpreter * interp )
{
    _Interpreter_Init ( interp ) ;
#if NEW_INTERPRET
    Interpreter_InitInfixModule ( interp ) ;
#endif    
}

Interpreter *
Interpreter_New ( uint64 allocType )
{
    Interpreter * interp = ( Interpreter * ) Mem_Allocate ( sizeof (Interpreter ), allocType ) ;

    interp->Lexer0 = Lexer_New ( allocType ) ;
    interp->ReadLiner0 = interp->Lexer0->ReadLiner0 ;
    interp->Finder0 = Finder_New ( allocType ) ;
    interp->Compiler0 = Compiler_New ( allocType ) ;
#if NEW_INTERPRET
    interp->InfixOpStack = Stack_New ( 32, allocType ) ;
#endif    
    _Interpreter_Init ( interp ) ;
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
    _Interpreter_Init ( interp ) ;
    return interp ;
}

int64
Interpreter_IsDone ( Interpreter * interp, uint64 flags )
{
    Interpreter_SetLexState ( interp ) ;
    return GetState ( interp, flags | INTERPRETER_DONE ) ;
}

