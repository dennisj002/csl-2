#include "../include/csl.h"

void
CSL_Peek ( ) // @
{
    if ( CompileMode ) Compile_Peek ( _Context_->Compiler0, DSP ) ;
    else TOS = * ( int64* ) TOS ;
    //cntx->BaseObject = 0 ;
}

// reg -- puts reg value TOS

// peekReg ( reg -- regValue ) //reg : reg number :: returns reg value on top of stack

void
CSL_PeekReg ( ) // @
{
    //DBI_ON ;
    ByteArray * svcs = _O_CodeByteArray, *ba = _CSL_->PeekPokeByteArray ;
    _ByteArray_DataClear ( ba ) ;
    Set_CompilerSpace ( ba ) ; // now compile to this space
    Compile_Move_Reg_To_Rm ( DSP, TOS & 0xf, 0, 0 ) ;
    _Compile_Return ( ) ;
    Set_CompilerSpace ( svcs ) ; // now compile to this space
    ( ( VoidFunction ) ba->BA_Data ) ( ) ;
    //DBI_OFF ;
}

// pokeRegWithValue ( lvalue reg -- puts value of reg at lvalue address )

void
CSL_PokeRegWithValue ( ) // @
{
    //DBI_ON ;
    uint64 reg = DataStack_Pop ( ) ;
    uint64 value = DataStack_Pop ( ) ;
    ByteArray * svcs = _O_CodeByteArray, *ba = _CSL_->PeekPokeByteArray ;
    _ByteArray_DataClear ( ba ) ;
    Set_CompilerSpace ( ba ) ; // now compile to this space
    Compile_MoveImm ( REG, reg, 0, ( uint64 ) value, CELL ) ;
    _Compile_Return ( ) ;
    Set_CompilerSpace ( svcs ) ; // now compile to this space
    ( ( VoidFunction ) ba->BA_Data ) ( ) ;
    //DBI_OFF ;
}

// pokeRegAtAddress ( address, reg -- )

void
CSL_PokeRegAtAddress ( ) // @
{
    uint64 reg = DataStack_Pop ( ) ;
    uint64 address = DataStack_Pop ( ) ;
    ByteArray * svcs = _O_CodeByteArray, *ba = _CSL_->PeekPokeByteArray ;
    _ByteArray_Init ( ba ) ;
    Set_CompilerSpace ( ba ) ; // now compile to this space
    _Compile_PushReg ( ACC ) ;
    Compile_MoveImm ( REG, ACC, 0, ( uint64 ) address, CELL ) ;
    Compile_Move_Reg_To_Rm ( ACC, reg, 0, 0 ) ;
    _Compile_PopToReg ( ACC ) ;
    _Compile_Return ( ) ;
    Set_CompilerSpace ( svcs ) ; // reset compiler space pointer
    ( ( VoidFunction ) ba->BA_Data ) ( ) ;
}

int
Word_LvalueObjectByteSize ( Word * lvalueWord, Word * rValueWord ) // = 
{
    int size = ( ( rValueWord->CompiledDataFieldByteSize == 8 ) || ( lvalueWord->W_ObjectAttributes & ( BIGNUM | REGISTER_VARIABLE ) ) || ( Namespace_IsUsing ( ( byte* ) "BigNum" ) ) ) ? 
        ( sizeof (int64 ) ) :
        lvalueWord->CompiledDataFieldByteSize ? lvalueWord->CompiledDataFieldByteSize : ( sizeof (int64 ) ) ;
    return size ;
}

void
Do_MoveErrorReport ( int64 value, byte * one, byte * two )
{
    byte *buffer = Buffer_Data_QuickReset ( _CSL_->ScratchB3 ) ; 
    snprintf ( buffer, 127, "\n_CSL_Move : Type Error : value == %ld : is greater than %s - max value for sizeof %s", value, one, two ) ;
    Error ( buffer, QUIT ) ;
}

void
_CSL_Move ( int64 * address, int64 value, int64 lvalueSize )
{
    switch ( lvalueSize )
    {
        case 1:
        {
            if ( value > 255 ) Do_MoveErrorReport ( value, "255", "(byte)" ) ;
            else * ( byte* ) address = ( byte ) value ;
            break ;
        }
        case 2:
        {
            if ( value > 65535 ) Do_MoveErrorReport ( value, "65535", "(int16)" ) ;
            else * ( int16* ) address = ( int16 ) value ;
            break ;
        }
        case 4:
        {
            if ( value > 2147483647 ) Do_MoveErrorReport ( value, "2147483647", "(int32)" ) ;
            else * ( int32* ) address = ( int32 ) value ;
            break ;
        }
        case 8:
        default:
        {
            * ( int64 * ) address = ( int64 ) value ;
            break ;
        }
    }
}
// ( addr n -- ) // (*addr) = n

void
CSL_Poke ( ) // = 
{
    Word * lvalueWord = TWS ( - 1 ), *rvalueWord = TWS ( 0 ) ;
    int64 lvalueSize = Word_LvalueObjectByteSize ( lvalueWord, rvalueWord ) ;
    //if ( ! _TypeMismatch_CheckError_Print ( lvalueWord, rvalueWord, 1 ) )
    {
        if ( CompileMode ) Compile_Poke ( _Context_->Compiler0, lvalueSize ) ;
        else
        {
            _CSL_Move ( ( int64 * ) NOS, TOS, lvalueSize ) ;
            _Dsp_ -= 2 ;
        }
    }
}

void
CSL_AtEqual ( ) // !
{
    Word * lvalueWord = TWS ( 0 ), *rvalueWord = TWS ( - 1 ) ;
    //Word * lvalueWord = TWS ( - 1 ), *rvalueWord = TWS ( 0 ) ;
    int64 lvalueSize = Word_LvalueObjectByteSize ( lvalueWord, rvalueWord ) ;
    //if ( ! _TypeMismatch_CheckError_Print ( lvalueWord, rvalueWord, 1 ) )
    {
        if ( CompileMode ) Compile_AtEqual ( DSP ) ;
        else
        {
            _CSL_Move ( ( int64 * ) NOS, * ( int64* ) TOS, lvalueSize ) ;
            _Dsp_ -= 2 ;
        }
    }
}

// ( n addr -- ) // (*addr) = n

void
CSL_Store ( ) // !
{
    Word * lvalueWord = TWS ( 0 ), *rvalueWord = TWS ( - 1 ) ;
    int64 lvalueSize = Word_LvalueObjectByteSize ( lvalueWord, rvalueWord ) ;
    _TypeMismatch_CheckError_Print ( lvalueWord, rvalueWord, 1 ) ;
    {
        if ( CompileMode ) Compile_Store ( _Context_->Compiler0, Word_LvalueObjectByteSize ( lvalueWord, rvalueWord ) ) ;
        else
        {
            _CSL_Move ( ( int64 * ) TOS, NOS, lvalueSize ) ;
            _Dsp_ -= 2 ;
        }
    }
}


