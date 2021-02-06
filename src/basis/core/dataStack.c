
#include "../../include/csl.h"

// these functions are part of the C vm and can't be compiled
// ! they are should only be called in C functions !
uint64
DataStack_Pop ( )
{
#if 0   
    int64 value = _Dsp_ [ 0 ] ; //
#else
    _DSP_ = _Dsp_ ;
    int64 value = Stack_Pop_WithExceptionFlag ( _DataStack_, 1 ) ;
#endif    
    //CSL_TypeStack_Drop ( ) ;
    _Dsp_ -- ;
    return value ;
}

void
DataStack_Push ( int64 value )
{
    _Dsp_ ++ ;
    _Dsp_ [0] = value ;
}

void
DataStack_Dup ( )
{
    _Dsp_ [ 1 ] = _Dsp_[0] ;
    _Dsp_ ++ ;
    //CSL_TypeStack_Dup ( ) ;
}

void
DataStack_DropN ( int64 n )
{
    _Dsp_ -= n ;
}

void
DataStack_Drop ( )
{
    _Dsp_ -- ;
    CSL_TypeStack_Drop ( ) ;
    //_DSP_ = _Dsp_ ;
}

inline int64
DataStack_Overflow ( )
{
    return ( _Dsp_ > _DataStack_->StackMax ) ;
}

inline int64
DataStack_Underflow ( )
{
    return ( _Dsp_ < _DataStack_->InitialTosPointer ) ;
}

void
DataStack_Check ( )
{
    if ( DataStack_Underflow ( ) || DataStack_Overflow ( ) )
    {
        CSL_Exception ( STACK_OVERFLOW, 0, QUIT ) ;
    }
}

int64
DataStack_Depth ( )
{
    if ( _O_ && _CSL_ && _DataStack_ )
    {
        //_DataStackPointer_ = _Dsp_ ;
        _DSP_ = _Dsp_ ;
        return Stack_Depth ( _DataStack_ ) ;
    }
    return 0 ;
}

// safe form with stack checking

void
_CSL_PrintDataStack ( )
{
    Stack_Print ( _DataStack_, ( byte* ) "DataStack", 0 ) ;
}

void
CSL_PrintDataStack ( )
{
    Set_DataStackPointer_FromDspReg ( ) ;
    _CSL_PrintDataStack ( ) ;
}

void
CSL_PrintStackDepth ( )
{
    Printf ( "\nDataStack : depth = %d", Stack_Depth ( _DataStack_ ) ) ;
}

void
Set_DebuggerDspReg_FromDspReg ( )
{
    _Debugger_->cs_Cpu->R14d = _Dsp_ ;
}

void
Set_DspReg ( uint64 * ptr )
{
    _Dsp_ = ptr ;
}

void
Set_DspReg_FromDebuggerDspReg ( )
{
    Set_DspReg ( _Debugger_->cs_Cpu->R14d ) ;
}

void
_Set_DataStackPointers ( uint64 * ptr )
{
    Set_DspReg ( ptr ) ;
    _CSL_->DataStack->StackPointer = ptr ;
}

void
Set_DataStackPointers_FromDebuggerDspReg ( )
{
    _Set_DataStackPointers ( _Debugger_->cs_Cpu->R14d ) ;
}

void
Set_DataStackPointer_FromDspReg ( )
{
    _CSL_->DataStack->StackPointer = _Dsp_ ;
}

void
Set_DspReg_FromDataStackPointer ( )
{
    _Dsp_ = _CSL_->DataStack->StackPointer ;
}

void
SetDataStackPointer ( uint64 *sp )
{
    _CSL_->DataStack->StackPointer = sp ;
    _Dsp_ = _CSL_->DataStack->StackPointer ;
}

uint64 *
GetDataStackPointer ( )
{
   //_CSL_->DataStack->StackPointer = _Dsp_ ;
    return _Dsp_ ;
}

void
CSL_CheckInitDataStack ( )
{
    if ( Stack_Depth ( _DataStack_ ) < 0 )
    {
        _Stack_PrintHeader ( _DataStack_, ( byte* ) "DataStack" ) ;
        Printf ( c_ad ( "\n\nError : %s : %s : Stack Underflow!" ), _Context_->CurrentlyRunningWord ? _Context_->CurrentlyRunningWord->Name : ( byte * ) "", _Context_Location ( _Context_ ) ) ;
        Printf ( c_gd ( "\nReseting DataStack.\n" ) ) ;
        _Stack_Init ( _CSL_->DataStack, _O_->DataStackSize ) ;
        _CSL_DataStack_Init ( _CSL_ ) ;
        _Stack_PrintHeader ( _DataStack_, ( byte* ) "DataStack" ) ;
    }
}

void
CSL_DataStack_Size ( )
{
    DataStack_Push ( DataStack_Depth ( ) ) ;
}

