
#include "../include/csl.h"

// ( b top | b < top ) dpans

void
CSL_LessThan ( ) // <
{
    if ( CompileMode ) Compile_LessThan ( _Context_->Compiler0 ) ;
    else
    {
        int64 top, b ;
        top = DataStack_Pop ( ) ;
        b = _DataStack_GetTop ( ) ;
        _DataStack_SetTop ( ( int64 ) ( b < top ) ) ;
    }
}

void
CSL_LessThanOrEqual ( ) // <
{
    if ( CompileMode ) Compile_LessThanOrEqual ( _Context_->Compiler0 ) ;
    else
    {
        int64 top, b ;
        top = DataStack_Pop ( ) ;
        b = _DataStack_GetTop ( ) ;
        _DataStack_SetTop ( ( int64 ) ( b <= top ) ) ;
    }
}

// ( b top | b > top ) dpans

void
CSL_GreaterThan ( ) // >
{
    if ( CompileMode ) Compile_GreaterThan ( _Context_->Compiler0 ) ;
    else
    {
        int64 top, b ;
        top = DataStack_Pop ( ) ;
        b = _DataStack_GetTop ( ) ;
        _DataStack_SetTop ( ( int64 ) ( b > top ) ) ;
    }
}

void
CSL_GreaterThanOrEqual ( ) // >
{
    if ( CompileMode ) Compile_GreaterThanOrEqual ( _Context_->Compiler0 ) ;
    else
    {
        int64 top, b ;
        top = DataStack_Pop ( ) ;
        b = _DataStack_GetTop ( ) ;
        _DataStack_SetTop ( ( int64 ) ( b >= top ) ) ;
    }
}

void
CSL_Logic_Equals ( ) // == 
{
    if ( CompileMode ) Compile_Equals ( _Context_->Compiler0 ) ;
    else
    {
        int64 top, b ;
        top = DataStack_Pop ( ) ;
        b = _DataStack_GetTop ( ) ;
        _DataStack_SetTop ( ( int64 ) ( b == top ) ) ;
    }
}

void
CSL_Logic_DoesNotEqual ( ) // !=
{
    if ( CompileMode ) Compile_DoesNotEqual ( _Context_->Compiler0 ) ;
    else
    {
        int64 top, b ;
        top = DataStack_Pop ( ) ;
        b = _DataStack_GetTop ( ) ;
        _DataStack_SetTop ( ( int64 ) ( b != top ) ) ;
    }
}

void
CSL_LogicalNot ( ) // not
{
    if ( CompileMode ) Compile_LogicalNot ( _Context_->Compiler0 ) ;
    else TOS = ( ! TOS ) ;
}

void
CSL_LogicalAnd ( ) // and
{
    if ( CompileMode ) Compile_LogicalAnd ( _Context_->Compiler0 ) ;
    else
    {
        _Dsp_ [ - 1 ] = _Dsp_ [ - 1 ] && TOS ;
        DataStack_Drop ( ) ;
    }
}

void
CSL_LogicalOr ( ) // or
{
    if ( CompileMode ) Compile_Logical_X ( _Context_->Compiler0, OR, TTT_ZERO, NEGFLAG_ON, TTT_ZERO, NEGFLAG_Z ) ;
    else
    {
        _Dsp_ [ - 1 ] = _Dsp_ [ - 1 ] || TOS ;
        DataStack_Drop ( ) ;
    }
}

void
CSL_LogicalXor ( ) // xor
{
    if ( CompileMode ) Compile_Logical_X ( _Context_->Compiler0, XOR, TTT_ZERO, NEGFLAG_ON, TTT_ZERO, NEGFLAG_Z ) ;
    else
    {
        _Dsp_ [ - 1 ] = _Dsp_ [ - 1 ] ^ TOS ;
        DataStack_Drop ( ) ;
    }
}

