
#include "../include/csl.h"

void
CSL_Power_03 ( ) // **
{
    int64 pow = _DspReg_ [ 0 ], base = _DspReg_ [ -1 ], n ;
    for ( n = base ; -- pow ; )
    {
        n *= base ;
    }
    _DspReg_ [ -1 ] = n ;
    DataStack_Drop ( ) ;
}
 
int64
_CFib_O3 ( int64 n )
{
    if ( n < 2 ) return n ;
    else return ( _CFib_O3 ( n - 1 ) + _CFib_O3 ( n - 2 ) ) ; 
}

void
CFib_O3 ( )
{
    TOS = ( _CFib_O3 ( TOS ) ) ;
}

int64
_CFib2_O3 ( int64 n )
{
    int64 fn, fn1, fn2 ;
    for ( fn = 0, fn1 = 0, fn2 = 1 ; n ; n-- ) 
    {   
        fn1 = fn2 ; 
        fn2 = fn ;
        fn = fn1 + fn2 ; 
    }
    return fn ;
}

void
CFib2_O3 ( )
{
    TOS = ( _CFib2_O3 ( TOS ) ) ;
}

void
CFactorial_O3 ( )
{
    int64 n = TOS ;
    if ( n > 1 )
    {
        TOS = TOS - 1 ;
        CFactorial_O3 ( ) ;
        TOS *= n ;
    }
    else TOS = 1 ;
}

int64
_CFactorial_O3 ( int64 n )
{
    if ( n > 1 ) return ( n * _CFactorial_O3 ( n - 1 ) ) ;
    else return 1 ;
}

void
CFactorial2_O3 ( )
{
    TOS = ( _CFactorial_O3 ( TOS ) ) ;
}

void
CFactorial3_O3 ( void )
{
    int64 rec1 = 1, n = TOS ;
    while ( n > 1 ) rec1 *= n -- ;
    TOS = rec1 ;
}

int64
_CFib ( int64 n )
{
    if ( n < 2 ) return n ;

    else return ( _CFib ( n - 1 ) + _CFib ( n - 2 ) ) ;
}

void
CFib ( )
{

    TOS = ( _CFib ( TOS ) ) ;
}

void
CSL_Power ( ) // **
{
    int64 pow = _DspReg_ [ 0 ], base = _DspReg_ [ - 1 ], n ;
    for ( n = base ; -- pow ; )
    {

        n *= base ;
    }
    _DspReg_ [ - 1 ] = n ;
    DataStack_Drop ( ) ;
}

void
CFactorial ( )
{
    int64 n = TOS ;
    if ( n > 1 )
    {
        TOS = TOS - 1 ;
        CFactorial ( ) ;
        TOS *= n ;
    }

    else TOS = 1 ;
}

int64
_CFactorial ( int64 n )
{
    if ( n > 1 ) return ( n * _CFactorial ( n - 1 ) ) ;

    else return 1 ;
}

void
CFactorial2 ( )
{

    TOS = ( _CFactorial ( TOS ) ) ;
}

void
CFactorial3 ( void )
{
    int64 rec1 = 1, n = TOS ;
    while ( n > 1 )
    {

        rec1 *= n -- ;
    }
    TOS = rec1 ;
}


void
ctct ()
{
    struct ct { int64 ar [4][4][4] ; } ;
    struct ct a  ;
    int64 y = DataStack_Pop () ;
    a.ar [0][y + 1][0] = y ;
    a.ar [y ][y + 1][0] = a.ar [0][y + 1][0] ;
    //a.ar [y @ 1 +][y @][y @] dup nl hp a.ar [0][0][0] @ = // if y == 2 this is an array out of bounds reference :: not checked for yet ??
    //a.ar [y @][y @][y @ 1 +] dup nl hp a.ar [0][0][0] @ =
    //TODO ( "array out of bounds checking with variables?!" )
    //a.ar 32 dump
    printf ( "ctct a.ar [y ][y ][0] = %ld", a.ar [y + 1][y + 1][0] ) ;
}

