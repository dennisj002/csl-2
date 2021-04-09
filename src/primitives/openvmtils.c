
#include "../include/csl.h"

void
OpenVmTil_Verbosity ( )
{
    //if ( Compiling ) Compile_MoveImm_To_Reg ( RAX, ( int64 ) & _O_->Verbosity, CELL ) ;
    //else DataStack_Push ( ( int64 ) & _O_->Verbosity ) ;
    Do_C_Pointer_StackAccess ( (byte* )& _O_->Verbosity );
}

void
OpenVmTil_ShowMachineCodeInstructions ( )
{
    //if ( Compiling ) _Compile_Stack_Push ( DSP, ACC, ( int64 ) & _O_->Dbi ) ; //CSL_CompileAndRecord_Word0_PushReg ( ACC ) ; //_Compile_Stack_Push ( DSP, ( int64 ) & _O_->Verbosity ) ;
    //else DataStack_Push ( ( int64 ) & _O_->Dbi ) ;
    Do_C_Pointer_StackAccess ( (byte* ) & _O_->Dbi );
}

void
Ovt_Optimize ( )
{
    DataStack_Push ( ( int64 ) GetState ( _CSL_, OPTIMIZE_ON ) ? 1 : 0 ) ;
}

void
Ovt_Inlining ( )
{
    DataStack_Push ( ( int64 ) GetState ( _CSL_, INLINE_ON ) ? 1 : 0 ) ;
}

// allows variables to be created on first use without a "var" declaration

void
Ovt_AutoVar ( )
{
    DataStack_Push ( ( int64 ) GetState ( _O_, AUTO_VAR ) ? 1 : 0 ) ;
}

void
Ovt_AutoVarOff ( )
{
    SetState ( _O_, AUTO_VAR, false ) ;
}

// allows variables to be created on first use without a "var" declaration

void
Ovt_AutoVarOn ( )
{
    SetState ( _O_, AUTO_VAR, true ) ;
}

void
OpenVmTil_DataStackSize ( )
{
    DataStack_Push ( ( int64 ) & _O_->DataStackSize ) ;
}

void
OpenVmTil_CodeSize ( )
{
    DataStack_Push ( ( int64 ) & _O_->MachineCodeSize ) ;
}

void
OpenVmTil_SessionObjectsSize ( )
{
    DataStack_Push ( ( int64 ) & _O_->SessionObjectsSize ) ;
}

void
OpenVmTil_CompilerTempObjectsSize ( )
{
    DataStack_Push ( ( int64 ) & _O_->CompilerTempObjectsSize ) ;
}

void
OpenVmTil_ObjectsSize ( )
{
    DataStack_Push ( ( int64 ) & _O_->ObjectSpaceSize ) ;
}

void
OpenVmTil_DictionarySize ( )
{
    DataStack_Push ( ( int64 ) & _O_->DictionarySize ) ;
}

void
OpenVmTil_Print_DataSizeofInfo ( int64 flag )
{
    if ( flag || ( _O_->Verbosity > 1 ) )
    {
        Printf ( "\ndlnode size : %d bytes, ", sizeof (dlnode ) ) ;
        Printf ( "dllist size : %d bytes, ", sizeof (dllist ) ) ;
        Printf ( "dobject size : %d bytes, ", sizeof ( dobject ) ) ;
        Printf ( "DLNode size : %d bytes, ", sizeof ( DLNode ) ) ;
        Printf ( "AttributeInfo size : %d bytes, ", sizeof (AttributeInfo ) ) ;
        //Printf ( "\nObject size : %d bytes, ", sizeof (Object ) ) ;
        Printf ( "Symbol size : %d bytes, ", sizeof (Symbol ) ) ;
        Printf ( "\nWord size : %d bytes, ", sizeof (Word ) ) ;
        Printf ( "ListObject size : %d bytes, ", sizeof ( ListObject ) ) ;
        Printf ( "WordData size : %d bytes, ", sizeof (WordData ) ) ;
        Printf ( "Context size : %d bytes, ", sizeof (Context ) ) ;
        Printf ( "System size : %d bytes, ", sizeof (System ) ) ;
        Printf ( "\nDebugger size : %d bytes, ", sizeof (Debugger ) ) ;
        Printf ( "MemorySpace size : %d bytes, ", sizeof (MemorySpace ) ) ;
        Printf ( "ReadLiner size : %d bytes, ", sizeof (ReadLiner ) ) ;
        Printf ( "Lexer size : %d bytes, ", sizeof (Lexer ) ) ;
        Printf ( "Interpreter size : %d bytes, ", sizeof (Interpreter ) ) ;
        Printf ( "\nFinder size : %d bytes, ", sizeof (Finder ) ) ;
        Printf ( "Compiler size : %d bytes, ", sizeof (Compiler ) ) ;
        Printf ( "ByteArray size : %d bytes, ", sizeof (ByteArray ) ) ;
        Printf ( "NamedByteArray size : %d bytes, ", sizeof (NamedByteArray ) ) ;
        Printf ( "MemChunk size : %d bytes", sizeof ( MemChunk ) ) ;
        Printf ( "\nCSL size : %d bytes, ", sizeof (CSL ) ) ;
        Printf ( "OpenVimTil size : %d bytes, ", sizeof (OpenVmTil ) ) ;
        Printf ( "OVT_Static size : %d bytes, ", sizeof (OVT_Static ) ) ;
        Printf ( "OS_Chunk size : %d bytes, ", sizeof (OS_Chunk ) ) ;
        Printf ( "OS_Node size : %d bytes, ", sizeof (OS_Node ) ) ;
        Printf ( "Stack size : %d bytes", sizeof (Stack ) ) ;
    }
}

void
OVT_Exit ( )
{
    if ( _O_->Verbosity > 0 ) Printf ( "bye\n" ) ;
    exit ( 0 ) ;
}

void
OVT_StartupMessage ( Boolean promptFlag )
{
    if ( _O_->Verbosity > 0 )
    {
        DefaultColors ;
        //if ( CSL->InitSessionCoreTimes > 1 ) CSL_NewLine () ;
        if ( promptFlag && ( _O_->Restarts < 2 ) )
        {
            System_Time ( _CSL_->Context0->System0, 0, ( char* ) "\nSystem Startup", 1 ) ;
            OVT_Time ( ( char* ) "OVT Startup", 1 ) ;

            _CSL_Version ( promptFlag ) ;
        }
        if ( _O_->Verbosity > 1 )
        {
            Printf ( "\nOpenVmTil : csl comes with ABSOLUTELY NO WARRANTY; for details type `license' in the source directory." ) ;
            Printf ( "\nType 'tc' 'demo' for starters" ) ;
            Printf ( "\nType 'bye' to exit" ) ;
        }
    }
    else if ( promptFlag && ( _O_->Restarts < 2 ) ) _O_->Verbosity = 1 ;
}

void
_OVT_Ok ( Boolean control )
{
    if ( _O_->Verbosity > 3 )
    {
        _CSL_SystemState_Print ( 0 ) ;
        Printf ( "\n<Esc> - break, <Ctrl-C> - quit, <Ctrl-D> - restart, \'bye\'/\'exit\' - leave." ) ;
    }
    _Context_Prompt (_O_->OVT_CSL->Context0, control) ;
}

void
OVT_Ok ( )
{
    _OVT_Ok ( 1 ) ;
    //_CSL_Prompt ( _O_->Verbosity && ( ( _O_->RestartCondition < RESET_ALL ) || _O_->StartTimes > 1 ) ) ;
}

