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
    byte c = 0 ;
    if ( rl->ReadIndex < BUFFER_SIZE ) c = rl->InputLine [ rl->ReadIndex ] ;
    return c ;
}

inline byte
_ReadLine_PeekIndexedChar ( ReadLiner * rl, int64 offset )
{
    byte c = rl->InputLine [ offset ] ;
    return c ;
}

byte
ReadLine_PeekIndexedChar ( ReadLiner * rl, int64 index )
{
    byte c = 0 ;
    if ( index < BUFFER_SIZE ) c = _ReadLine_PeekIndexedChar ( rl, index ) ; //rl->InputLine [ offset ] ;
    return c ;
}

byte
_ReadLine_PeekOffsetChar ( ReadLiner * rl, int64 offset )
{
    byte c = 0 ;
    if ( ( rl->ReadIndex + offset ) < BUFFER_SIZE ) c = _ReadLine_PeekIndexedChar ( rl, rl->ReadIndex + offset ) ;
    return c ;
}

byte
ReadLine_PeekNextChar ( ReadLiner * rl )
{
    byte c = _ReadLine_PeekOffsetChar ( rl, 0 ) ; //_ReadLine_NextChar ( rl );
    return c ;
}

byte
_ReadLine_GetNextChar ( ReadLiner * rl )
{
    byte c = _ReadLine_NextChar ( rl ) ;
    if ( c ) rl->ReadIndex ++ ;
    return c ;
}

void
_ReadLine_EndThisLine ( ReadLiner * rl )
{
    ReadLine_Set_ReadIndex ( rl, - 1 ) ;
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
        atIndex = rl->InputLine [ index ++ ] ;
    }
    while ( atIndex <= ' ' ) ;
    return atIndex ;
}

int64
ReadLine_IsThereNextChar ( ReadLiner * rl )
{
    if ( ! rl->InputLine ) return false ; // in case we are at in a _OpenVmTil_Pause
    char c = ReadLine_PeekNextChar ( rl ) ;
    return c || ( rl->InputFile && ( rl->InputKeyedCharacter != eof ) ) ; // || (c != '\n') ) ;
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
_ReadLine_CharAtACursorPos ( ReadLiner * rl, int64 pos )
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

// char sets would be better here ??

Boolean
ReadLiner_IsTokenForwardDotted ( ReadLiner * rl, int64 index )
{
    int64 i = 0, space = 0 ;
    Boolean escSeqFlag = false, inArray = false ;
    ; //, quoteMode = false ;
    byte * nc = & rl->InputLineString [ index ] ;
    for ( i = 0 ; 1 ; i ++, nc ++ )
    {
        if ( escSeqFlag )
        {
            if ( ( *nc ) != 'm' ) continue ;
            else
            {
                escSeqFlag = false ;
                continue ;
            }
        }
        else
        {
            switch ( *nc )
            {
                case ESC:
                {
                    escSeqFlag = true ;
                    continue ;
                }
                case ']':
                {
                    inArray = false ;
                    continue ;
                }
                case '[': return true ; //{ inArray = true ; continue ; } //continue ;
                case 0: case ',': case ';': case '(': case ')': case '\n': case '\'': return false ;
                case '.':
                {
                    if ( i && ( *( nc + 1 ) != '.' ) )// watch for (double/triple) dot ellipsis
                        return true ;
                    break ;
                }
                case '"':
                {
                    //if ( i > index ) 
                    return false ;
                    //else quoteMode = true ;
                    break ;
                }
                case ' ':
                {
                    space ++ ;
                    break ;
                }
                default:
                {
                    //if ( ( ! GetState ( _Compiler_, ARRAY_MODE ) ) && space && isgraph ( *nc ) ) return false ;
                    //if ( ( ! inArray ) && space && isgraph ( *nc ) ) return false ;
                    if ( space && isgraph ( *nc ) ) return false ;
                    else
                    {
                        space = 0 ;
                        break ;
                    }
                }
            }
        }
    }
    return false ;
}

