
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


int
first_denomination ( int kinds_of_coins)
{
   int rtrn ;
   if ( kinds_of_coins == 1) rtrn = 1 ;
   else if ( kinds_of_coins == 2) rtrn = 5 ;
   else if ( kinds_of_coins == 3) rtrn = 10 ;
   else if ( kinds_of_coins == 4) rtrn = 25 ;
   else if ( kinds_of_coins == 5) rtrn = 50 ;
   return rtrn ;
}

int
cc1 ( int amount, int kinds_of_coins)
{
    int rtrn ;
    //printf ( "\ncc1 : amount = %d : kinds_of_coins = %d", amount, kinds_of_coins ) ;
    if (amount == 0) rtrn = 1  ;
    else if ( ( amount < 0) || ( kinds_of_coins == 0) ) rtrn = 0 ;
    else rtrn = cc1 ( amount, kinds_of_coins - 1) + 
        cc1 (amount - first_denomination (kinds_of_coins), kinds_of_coins) ;
   //printf ( "\ncc1 : rtrn = %d", rtrn ) ;
   return rtrn ;
}

int count_change ( int amount)
{
  //printf ( "\ncount_change = %d", cc ( amount 5) ) ;
  return cc1 ( amount, 5)  ;
}

void
CSL_CountChange ()
{
    TOS = count_change ( TOS ) ;
}
