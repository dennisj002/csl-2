#include "../../include/csl.h"

void
ReadLine_Set_ReadIndex ( ReadLiner * rl, int64 pos )
{
    rl->ReadIndex = pos ;
}

byte *
_ReadLine_pb_NextChar ( ReadLiner * rl )
{
    if ( rl->ReadIndex < BUFFER_SIZE ) return &rl->InputLine [ rl->ReadIndex ] ;
    //if ( rl->ReadIndex < BUFFER_SIZE ) return &rl->InputStringCurrent [rl->InputStringIndex] ; 
    else return 0 ;
}

byte
_ReadLine_NextChar ( ReadLiner * rl )
{
    if ( rl->ReadIndex < BUFFER_SIZE ) return rl->InputLine [ rl->ReadIndex ] ;
    else return 0 ;
}

inline byte
_ReadLine_PeekIndexedChar ( ReadLiner * rl, int64 offset )
{
    return rl->InputLine [ offset ] ;
}

byte
ReadLine_PeekIndexedChar ( ReadLiner * rl, int64 index )
{
    if ( index < BUFFER_SIZE ) return _ReadLine_PeekIndexedChar ( rl, index ) ; //rl->InputLine [ offset ] ;
    else return 0 ;
}

byte
_ReadLine_PeekOffsetChar ( ReadLiner * rl, int64 offset )
{
    if ( ( rl->ReadIndex + offset ) < BUFFER_SIZE ) return _ReadLine_PeekIndexedChar ( rl, rl->ReadIndex + offset ) ;
    else return 0 ;
}

byte
ReadLine_PeekNextChar ( ReadLiner * rl )
{
    return _ReadLine_PeekOffsetChar ( rl, 0 ) ; //_ReadLine_NextChar ( rl );
}

byte
_ReadLine_GetNextChar ( ReadLiner * rl )
{
    byte c = _ReadLine_NextChar ( rl ) ;
    if ( c ) rl->ReadIndex++ ;
    return c ;
}

void
_ReadLine_EndThisLine ( ReadLiner * rl )
{
    ReadLine_Set_ReadIndex ( rl, -1 ) ;
}

byte
ReadLine_CurrentReadChar ( ReadLiner * rl )
{
    return rl->InputLine [ rl->ReadIndex ] ;
}

byte *
ReadLine_BytePointerToCurrentReadChar ( ReadLiner * rl )
{
    return &rl->InputLine [ rl->ReadIndex ] ;
}

byte
ReadLine_LastChar ( ReadLiner * rl )
{
    return rl->InputLine [ rl->ReadIndex - 1 ] ;
}

byte
ReadLine_LastReadChar ( ReadLiner * rl )
{
    return rl->InputLine [ rl->ReadIndex - 2 ] ;
}

byte
ReadLine_PeekNextNonWhitespaceChar ( ReadLiner * rl )
{
    int64 index = rl->ReadIndex ;
    byte atIndex = 0 ;
    do 
    {
        if ( index >= BUFFER_SIZE ) break ;
        atIndex = rl->InputLine [ index++ ] ;
    }
    while ( atIndex <= ' ' ) ;
    return atIndex ;
}

int64
ReadLine_IsThereNextChar( ReadLiner * rl ) 
{
    if ( ! rl->InputLine ) return false ;// in case we are at in a _OpenVmTil_Pause
    char c = ReadLine_PeekNextChar ( rl ) ;
    return c || ( rl->InputFile && (rl->InputKeyedCharacter != eof) ) ; // || (c != '\n') ) ;
}

void
ReadLine_UnGetChar ( ReadLiner * rl )
{
    if ( rl->ReadIndex ) rl->ReadIndex -- ;
}

void
ReadLine_PushChar ( ReadLiner * rl, byte c )
{
    rl->InputLine [ rl->ReadIndex ] = c ;
}

void
_ReadLine_ShowCharacter ( ReadLiner * rl, byte chr )
{
    if ( GetState ( rl, CHAR_ECHO ) ) putc ( ( char ) chr, rl->OutputFile ) ;
}

void
ReadLine_ShowCharacter ( ReadLiner * rl )
{
    _ReadLine_ShowCharacter ( rl, rl->InputKeyedCharacter ) ;
}

void
_ReadLine_SetMaxEndPosition ( ReadLiner * rl )
{
    if ( rl->EndPosition > rl->MaxEndPosition ) rl->MaxEndPosition = rl->EndPosition ;
}

void
_ReadLine_SetEndPosition ( ReadLiner * rl )
{
    rl->EndPosition = Strlen ( ( char* ) rl->InputLine ) ;
    _ReadLine_SetOutputLineCharacterNumber ( rl ) ;
    _ReadLine_SetMaxEndPosition ( rl ) ; // especially for the case of show history node
}

byte
_ReadLine_CharAtCursor ( ReadLiner * rl )
{
    return rl->InputLine [ rl->CursorPosition ] ;
}

byte
_ReadLine_CharAtACursorPos ( ReadLiner * rl, int64 pos  )
{
    return rl->InputLine [ pos ] ;
}

void
_ReadLine_CursorToEnd ( ReadLiner * rl )
{
    ReadLine_SetCursorPosition ( rl, rl->EndPosition ) ;
    rl->InputLine [ rl->CursorPosition ] = 0 ;
}

void
_ReadLine_CursorToStart ( ReadLiner * rl )
{
    ReadLine_SetCursorPosition ( rl, 0 ) ;
}

void
_ReadLine_CursorRight ( ReadLiner * rl )
{
    ReadLine_SetCursorPosition ( rl, rl->CursorPosition + 1 ) ;
}

void
_ReadLine_CursorLeft ( ReadLiner * rl )
{
    ReadLine_SetCursorPosition ( rl, rl->CursorPosition - 1 ) ;
}

