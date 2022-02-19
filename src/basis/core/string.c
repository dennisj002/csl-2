
#include "../../include/csl.h"

Boolean
IsChar_Dot ( byte character )
{
    return character == '.' ;
}

Boolean
IsChar_Whitespace ( byte character )
{
    return character <= ' ' ;
}

Boolean
IsChar_DelimiterOrDot ( byte character )
{
    return _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, character ) ;
}

Boolean
IsChar_ADotAndNotANonDelimiter ( byte character )
{
    return _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, character ) ;
}

// backward parsing

void
Mem_Clear ( byte * buffer, int64 size )
{
    memset ( ( char* ) buffer, 0, size ) ;
}

//|-----------------------------
//| REVERSE PARSING ...
//|-----------------------------

int64
String_IsPreviousCharA_ ( byte * s, int64 pos, byte c )
{
    int64 i ;
    for ( i = pos ; i >= 0 ; i -- )
    {
        if ( s [ i ] == c ) return i ;
        else if ( _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, s [ i ] ) ) continue ;
        else break ;
    }
    return false ;
}

int64
String_IsLastCharADot ( byte * s, int64 pos )
{
    return String_IsPreviousCharA_ ( s, pos, ( byte ) '.' ) ;
}

int64
String_FirstCharOfToken_FromPosOfLastChar ( byte * s, int64 pos )
{
    int64 i ;
    for ( i = pos ; i ; i -- )
    {
        if ( _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, s [i] ) ) break ;
    }
    return _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, s [i] ) ? i + 1 : i ; // nb. we could have 'break' becuase i == 0 - beginning of line
}

int64
String_IsThereADotSeparatorBackFromPosToLastNonDelmiter ( byte * s, int64 pos )
{
    int64 i ;
    for ( i = pos ; i > 0 ; i -- )
    {
        if ( _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, s [i] ) )
        {
            if ( s [i] == '.' )
            {
                if ( s [i - 1] == '.' ) return i - 1 ; // deal with the unique case of the dot, '.', token in the Class namespace 
                else return i ;
            }
        }
        else return 0 ;
    }
    return 0 ;
}
// reverse parsing

byte
String_LastChar ( byte * s )
{
    int64 i ;
    byte rtn ;
    for ( i = 0 ; s[i] ; i ++ ) ;
    rtn = s[( i <= 0 ) ? 0 : ( i - 1 )] ;
    return rtn ;
}

int64
String_LastCharOfLastToken_FromPos ( byte * s, int64 pos )
{
    int64 i, spaces = 0, dotFlag = 0 ;
    for ( i = pos ; i ; i -- )
    {
        if ( ! _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, s[ i ] ) ) break ;
        if ( ( i != _ReadLine_CursorPosition ( _Context_->ReadLiner0 ) ) && ( s [ i ] == ' ' ) ) spaces ++ ;
        if ( s[ i ] == '.' ) dotFlag ++ ;
        // a space with no dot is an end of token
    }
    if ( ( spaces && ( ! dotFlag ) ) || ( dotFlag > 1 ) ) return pos ;
    return i ;
}

int64
String_FirstTokenDelimiter_FromPos ( byte * s, int64 pos )
{
    int64 i, flag = 0 ;
    for ( i = pos ; 1 ; i ++ )
    {
        if ( ! _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, s[ i ] ) ) flag = 1 ;
        if ( flag && _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, s[ i ] ) ) break ;
    }
    return i ;
}

Boolean
String_IsReverseTokenQualifiedID ( byte * s, int64 pos )
{
    //int64 lastChar = ReadLine_LastCharOfLastToken_FromPos ( rl, rl->ReadIndex ) ;
    int64 lastChar = String_LastCharOfLastToken_FromPos ( s, pos ) ;
    //int64 firstChar = ReadLine_FirstCharOfToken_FromLastChar ( rl, lastChar ) ;
    int64 firstChar = String_FirstCharOfToken_FromPosOfLastChar ( s, lastChar ) ;
    //return ReadLine_IsThereADotSeparator ( rl, firstChar - 1 ) ;
    return String_IsThereADotSeparatorBackFromPosToLastNonDelmiter ( s, firstChar ) ;
}

/*
|-----------------------------|
| ... REVERSE PARSING         |
|-----------------------------|
 */

// unbox string 'in place'

byte *
_String_UnBox ( byte * token )
{
    byte * start ;
    if ( token [ 0 ] == '"' )
    {
        char * s = ( char* ) Buffer_Data ( _CSL_->TokenB ) ;
        strcpy ( ( char* ) s, ( char* ) token ) ; // preserve token - this string is used by the Interpreter for SourceCode
        int64 length = Strlen ( ( char* ) s ) ;
        if ( s [ length - 1 ] == '"' )
        {
            s [ length - 1 ] = 0 ;
        }
        s = & s [ 1 ] ;
        start = String_New ( ( byte* ) s, TEMPORARY ) ;
    }
    else start = token ;
    return start ;
}

byte *
_String_InsertColors ( byte * s, Colors * c )
{
    if ( _CSL_ && s )
    {
        Colors * current = _O_->Current ;
        byte * tbuffer = Buffer_Data ( _CSL_->StringInsertB ) ;
        String_ShowColors ( tbuffer, c ) ; // new foreground, new background
        strcat ( ( char* ) tbuffer, ( char* ) s ) ;
        String_ShowColors ( &tbuffer[Strlen ( ( char* ) tbuffer )], current ) ; // old foreground, old background
        tbuffer = String_New ( tbuffer, TEMPORARY ) ;
        return tbuffer ;
    }
    else return ( byte* ) "" ;
}

byte*
String_New_RemoveColors ( byte * str, uint64 allocType )
{
    //printf ( "%c[%ld;%ldm", ESC, fg, bg )
    byte * buf0 = Buffer_Data_QuickReset ( _CSL_->StringInsertB ), *buf1 ;
    int64 si, bi, j ;
    for ( si = 0, bi = 0 ; str[si] ; bi ++, si ++ )
    {
        if ( str [si] == ESC )
        {
            for ( j = si ; str[j ++] != 'm' ; ) ;
            si = j ;
        }
        else buf0[bi] = str[si] ;
    }
    buf0[bi] = 0 ;
    buf1 = String_New ( buf0, allocType ) ;
    return buf1 ;
}

byte *
_String_Insert_AtIndexWithColors ( byte * token, int64 ndx, Colors * color )
{
    int64 preTokenLen ; // Lexer reads char finds it is delimiter : reading index auto increments index 
    if ( strncmp ( ( char* ) token, ( char* ) &_Context_->ReadLiner0->InputLine [ ndx ], Strlen ( ( char* ) token ) ) )
        return String_RemoveFinalNewline ( String_New ( ( byte* ) _Context_->ReadLiner0->InputLine, TEMPORARY ) ) ;
    byte * buffer = Buffer_Data ( _CSL_->StringInsertB2 ) ;
    byte * tbuffer = Buffer_Data ( _CSL_->StringInsertB3 ) ;

    strncpy ( ( char* ) buffer, ( char* ) _Context_->ReadLiner0->InputLine, BUFFER_IX_SIZE ) ;
    String_RemoveFinalNewline ( ( byte* ) buffer ) ;
    if ( ! _Lexer_IsCharDelimiter ( _Context_->Lexer0, buffer [ ndx ] ) ) ndx ++ ; // Lexer index auto increments index at token end ; dot doesn't incrment index therefore it is a dot at index
    preTokenLen = ndx - Strlen ( ( char* ) token ) ;
    if ( preTokenLen < 0 ) preTokenLen = 0 ;

    Strncpy ( tbuffer, buffer, preTokenLen ) ; // copy upto start of token
    tbuffer [ preTokenLen ] = 0 ; // Strncpy does not necessarily null delimit
    String_ShowColors ( &tbuffer [ Strlen ( ( char* ) tbuffer ) ], color ) ; // new foreground, new background
    strncat ( ( char* ) tbuffer, ( char* ) token, BUFFER_IX_SIZE ) ;
    String_ShowColors ( &tbuffer [ Strlen ( ( char* ) tbuffer ) ], &_O_->Default ) ; // old foreground, old background
    strncat ( ( char* ) tbuffer, ( char* ) &buffer [ ndx ], BUFFER_IX_SIZE ) ; // copy the rest of the buffer after the token : -1 : get the delimiter; 0 based array
    tbuffer = String_New ( tbuffer, TEMPORARY ) ;
    return tbuffer ;
}

byte *
String_ReadLineToken_HighLight ( byte * token )
{
    return _String_Insert_AtIndexWithColors ( token, _Context_->ReadLiner0->ReadIndex - 1, &_O_->User ) ;
}

// ?? use pointers with these string functions ??

void
_String_AppendConvertCharToBackSlash ( byte * dst, byte c, int64 * index, Boolean removeExtraSpacesFlag ) //, int64 quoteMode )
{
    int64 i = index ? ( * index ) : 0 ;
    if ( c < ' ' )
    {
        if ( _CSL_->SC_QuoteMode )
        {
            if ( c == '\n' )
            {
                dst [ i ++ ] = '\\' ;
                dst [ i ++ ] = 'n' ;
            }
            else if ( c == '\r' )
            {
                dst [ i ++ ] = '\\' ;
                dst [ i ++ ] = 'r' ;
            }
            else if ( c == '\t' )
            {
                dst [ i ++ ] = '\\' ;
                dst [ i ++ ] = 't' ;
            }
        }
        else dst [ i ++ ] = ' ' ; // ignore unquoted escape chars
    }
    else
    {
        if ( removeExtraSpacesFlag )
        {
            if ( ! ( ( c == ' ' ) && ( dst [i - 1] == ' ' ) ) ) dst [ i ++ ] = c ; // remove extra spaces
        }
        else dst [ i ++ ] = c ;
    }
    dst [ i ] = 0 ;
    if ( index ) *index = i ;
    //return &dst [ i ] ;
}

byte *
_String_ConvertStringFromBackSlash ( byte * dst, byte * src )
{
    int64 i, j, len = Strlen ( ( char* ) src ), quoteMode = 0 ;
    for ( i = 0, j = 0 ; i < len ; i ++ )
    {
        byte c = src [ i ] ;
        if ( c == '"' )
        {
            if ( quoteMode ) quoteMode = 0 ;
            else quoteMode = 1 ;
        }
        if ( ! quoteMode )
        {
            if ( c == '\\' )
            {
                byte d = src [ ++ i ] ;
                if ( d == 'n' )
                {
                    dst [ j ++ ] = ' ' ;
                    continue ;
                }
                else if ( d == 'r' )
                {
                    dst [ j ++ ] = ' ' ;
                    continue ;
                }
                else if ( c == '\t' )
                {
                    dst [ j ++ ] = '\\' ;
                    dst [ j ++ ] = 't' ;
                }
            }
        }
        dst [ j ++ ] = c ;
    }
    dst [ j ] = 0 ;
    return dst ;
}

byte
String_ConvertEscapeCharToSpace ( byte c )
{
    if ( ( c == '\n' ) || ( c == '\t' ) || ( c == '\r' ) ) c = ' ' ;
    return c ;
}

byte *
_String_ConvertString_EscapeCharToSpace ( byte * dst, byte * src )
{
    int64 i, j, len = Strlen ( ( char* ) src ), quoteMode = 0 ;
    for ( i = 0, j = 0 ; i < len ; i ++ )
    {
        byte c = src [ i ] ;
        if ( c == '"' )
        {
            if ( quoteMode ) quoteMode = 0 ;
            else quoteMode = 1 ;
        }
        else if ( c == ' ' )
        {
            quoteMode = 0 ;
            while ( src [ i + 1 ] == ' ' ) i ++ ;
            dst [ j ++ ] = src [ i ] ;
            continue ;
        }
        else if ( ! quoteMode )
        {
            if ( ( c == '\n' ) || ( c == '\r' ) || ( c == '\t' ) ) c = ' ' ;
            else if ( c == '\\' )
            {
                byte d = src [ ++ i ] ;
                if ( ( d == 'n' ) || ( d == 'r' ) || ( d == 't' ) ) c = ' ' ;
            }
        }
        dst [ j ++ ] = c ;
    }
    dst [ j ] = 0 ;
    return dst ;
}

byte *
String_ConvertString_EscapeCharToSpace ( byte * istring )
{
    byte * nstring = Buffer_Data ( _CSL_->StringInsertB4 ) ;
    _String_ConvertString_EscapeCharToSpace ( nstring, istring ) ;
    nstring = String_New ( ( byte* ) nstring, TEMPORARY ) ;
    return nstring ;
}

byte *
_String_ConvertStringToBackSlash ( byte * dst, byte * src, int64 nchars )
{
    int64 i, j, len, quoted = 1 ; // counting the initial standard quote (raw string?)
    dst[0] = 0 ; //init dst 
    if ( src )
    {
        if ( nchars == - 1 ) len = Strlen ( ( char* ) src ) ;
        else len = nchars ;
    }
    else len = 0 ;
    for ( i = 0, j = 0 ; ( i < len ) ; i ++ )
    {
        byte c = src [ i ] ;

        if ( c == 0 ) break ;
        if ( c == '"' )
        {
            if ( i > 0 )
            {
                if ( ! quoted ) quoted = 1 ;
                else quoted = 0 ;
            }
            dst [ j ++ ] = c ;
        }
        else if ( c == ESC ) dst [ j ++ ] = c ;
        else if ( c < ' ' )
        {
            if ( quoted )
            {
                if ( c == '\n' )
                {
                    dst [ j ++ ] = '\\' ;
                    dst [ j ++ ] = 'n' ;
                }
                else if ( c == '\r' )
                {
                    dst [ j ++ ] = '\\' ;
                    dst [ j ++ ] = 'r' ;
                }
                else if ( c == '\t' )
                {
                    dst [ j ++ ] = '\\' ;
                    dst [ j ++ ] = 't' ;
                }
            }
        }
        else dst [ j ++ ] = c ;
    }
    dst [ j ] = 0 ;

    return dst ;
}

byte *
String_ConvertToBackSlash ( byte * str0 )
{
    if ( str0 )
    {
        byte * buffer = Buffer_Data ( _CSL_->ScratchB2 ) ;
        byte * str1 = _String_ConvertStringToBackSlash ( buffer, str0, - 1 ) ;
        if ( str1 )
        {
            byte * nstr = String_New ( str1, TEMPORARY ) ;
            return nstr ;
        }
    }
    return 0 ;
}

Boolean
Strcmp ( byte * str0, byte * str1 )
{
    int64 i ;
    Boolean result = 0 ;
    if ( str0 == str1 ) return 0 ;
    if ( ( ! str0 ) || ( ! str1 ) ) return 1 ;
    else
    {
        for ( i = 0 ; ( str0 [ i ] || str1 [ i ] ) && ( ! result ) ; i ++ )
        {
            result = str0 [ i ] - str1 [ i ] ;
        }
    }
    return result ;
}

Boolean
Stricmp ( byte * str0, byte * str1 )
{
    int64 i ;
    Boolean result = 0 ;
    for ( i = 0 ; ( str0 [ i ] || str1 [ i ] ) && ( ! result ) ; i ++ )
    {
        result = tolower ( str0 [ i ] ) != tolower ( str1 [ i ] ) ;
    }
    return result ;
}

Boolean
Strncmp ( byte * str0, byte * str1, int64 n )
{
    int64 i ;
    Boolean result = 0 ;
    if ( str0 && str1 )
    {
        for ( i = 0 ; ( str0 [ i ] || str1 [ i ] ) && ( ! result ) && n ; i ++, n -- )
        {
            result = str0 [ i ] != str1 [ i ] ;
        }
    }
    return result ;
}

int64
Strnicmp ( byte * str0, byte * str1, int64 n )
{
    int64 i, result = 0 ;
    for ( i = 0 ; str0 [ i ] && str1 [ i ] && n && ( ! result ) ; i ++, n -- )
    {
        result = tolower ( ( int64 ) str0 [ i ] ) - tolower ( ( int64 ) str1 [ i ] ) ;
    }
    if ( ! n ) return result ;

    else return - 1 ;
}

void
Strcpy ( byte * dst, byte * src )
{
    int64 i ;
    for ( i = 0 ; src [i] ; i ++ ) dst[i] = src[i] ;
}

void
Strncpy ( byte * dst, byte * src, int64 n )
{
    int64 i ;
    for ( i = 0 ; src [i] && ( i < n ) ; i ++ ) dst[i] = src[i] ;
}

Boolean
_C_Syntax_AreWeParsingACFunctionCall ( byte * nc )
{
    while ( *nc ++ != ')' ) ;
    while ( *nc )
    {
        if ( *nc == ';' ) return true ; // we have an rvalue
        else if ( *nc == '{' ) return false ; // we have an rvalue
        nc ++ ;
    }
    return true ;
}

byte *
strToLower ( byte * dest, byte * str )
{
    int64 i ;
    for ( i = 0 ; str [ i ] ; i ++ )
    {
        dest [ i ] = tolower ( str [ i ] ) ;
    }
    dest [i] = 0 ;

    return dest ;
}

byte *
String_RemoveEndWhitespace ( byte * string )
{
    byte * ptr = string + Strlen ( ( char* ) string ) ;
    for ( ; ( ( *ptr ) && ( *ptr <= ' ' ) ) ; ptr -- ) ;
    //*++ ptr = '\n' ;
    *++ ptr = ' ' ;
    *++ ptr = 0 ;
    return string ;
}

byte *
String_FilterMultipleSpaces ( byte * istring, int64 allocType )
{
    int64 i, j ;
    byte * nstring = Buffer_Data ( _CSL_->StringInsertB5 ) ;
    for ( i = 0, j = 0 ; istring [ i ] ; i ++ )
    {
        if ( ( istring [ i ] == ' ' ) && ( istring [ i + 1 ] == ' ' ) ) continue ;
        nstring [ j ++ ] = istring [ i ] ;
    }
    nstring [ j ] = 0 ;
    nstring = String_New ( ( byte* ) nstring, allocType ) ;

    return nstring ;
}

void
String_InsertCharacter ( CString into, int64 position, byte character )
{

    char * b = ( char* ) Buffer_Data ( _CSL_->StringInsertB2 ) ;
    strcpy ( ( char* ) b, into ) ;
    b [ position ] = character ;
    b [ position + 1 ] = 0 ;
    strcat ( ( char* ) b, &into [ position ] ) ;
    strcpy ( into, ( CString ) b ) ;
}

#if 0

CString
String_Wrap ( CString in, CString s, CString pre, CString post )
{
    //if ( Strlen ( s ) + Strlen (pre) + Strlen (post) < BUFFER_SIZE )
    {
        strcpy ( in, pre ) ;
        strcat ( in, s ) ;
        strcat ( in, post ) ;

        return in ;
    }
    //else CSL_Exception ( BUFFER_OVERFLOW, 1 ) ;
}
#endif
// insert data into slot ( startOfSlot, endOfSlot ) in buffer

void
String_InsertDataIntoStringSlot ( byte * str, int64 startOfSlot, int64 endOfSlot, byte * nstr, byte * ostr ) // size in bytes
{
    byte * b = Buffer_DataCleared ( _CSL_->StringInsertB2 ) ;
    int64 slo = ostr ? Strlen ( ostr ) : 0 ; 
    if ( ( Strlen ( ( char* ) str ) + Strlen ( ( char* ) nstr ) ) < BUFFER_IX_SIZE )
    {
        //if ( Strlen ( ( char* ) str ) > ( endOfSlot - startOfSlot ) ) //startOfSlot ) //
        {
            strncpy ( ( char* ) b, ( char* ) str, BUFFER_IX_SIZE ) ;
            strncpy ( ( char* ) & b [ startOfSlot ], ( char* ) nstr, BUFFER_IX_SIZE ) ; // watch for overlapping ??
            if ( str [ startOfSlot + slo ] == ' ' ) strcat ( ( char* ) b, " " ) ;
            strncat ( ( char* ) b, ( char* ) &str [ endOfSlot ], BUFFER_IX_SIZE ) ;
            strncpy ( ( char* ) str, ( char* ) b, BUFFER_IX_SIZE ) ;
        }
        //else strncat ( ( char* ) str, ( char* ) data, BUFFER_IX_SIZE ) ;
    }
    else CSL_Exception ( BUFFER_OVERFLOW, 0, 1 ) ;
}

byte *
String_RemoveFinalNewline ( char * astring )
{
    byte character ;
    int64 index = 1 ;
    if ( astring )
    {
        do
        {
            character = astring [ strlen ( astring ) - ( index ++ ) ] ;
            if ( character == '\n' || character == '\r' || character == eof ) astring [ strlen ( astring ) - 1 ] = 0 ;
            else break ;
        }
        while ( 1 ) ;
    }

    return astring ;
}

// necessary for strings with '"' in them

byte *
String_N_New ( byte * string, int64 n, uint64 allocType )
{
    byte * newString ;
    if ( string )
    {
        newString = Mem_Allocate ( n + 1, ( allocType == TEMPORARY ) ? TEMPORARY : STRING_MEM ) ;
        Strncpy ( newString, string, n ) ;

        return newString ;
    }
    return 0 ;
}

byte *
String_New ( byte * string, uint64 allocType )
{
    byte * newString ;
    int slen ;
    if ( string )
    {
        newString = Mem_Allocate ( slen = Strlen ( ( char* ) string ) + 1, allocType ) ;
        strncpy ( ( char* ) newString, ( char* ) string, slen ) ;
        return newString ;
    }
    return 0 ;
}

byte *
String_New_SourceCode ( byte * string )
{
    return String_New ( string, STRING_MEM ) ;
}

#if 0

byte
_String_NextNonDelimiterChar ( byte * str, byte * cset )
{
    if ( ! str ) return 0 ;
    else
    {
        for ( ; *str ; str ++ )
        {
            if ( ! _CharSet_IsDelimiter ( cset, *str ) ) break ;
        }
    }
    return *str ;
}
#endif

byte
_String_NextPrintChar ( byte * str, byte * cset )
{
    if ( ! str ) return 0 ;
    else
    {
        for ( ; *str ; str ++ )
        {
            if ( CharTable_IsCharType ( *str, CHAR_PRINT ) ) break ;
        }
    }
    return *str ;
}

int64
_CSL_StrTok ( byte * inBuffer )
{
    StrTokInfo * sti = & _CSL_->Sti ;
    int64 i, start, end ;
    byte * str0 = sti->In = inBuffer, * buffer = sti->Out, *cset = sti->CharSet0, *str1, *str2 ;
    // find start of non-delimiter text
    // str0 is the original string
    for ( i = 0, str1 = str0 ; *str1 ; str1 ++, i ++ )
    {
        if ( ! _CharSet_IsDelimiter ( cset, *str1 ) ) break ;
    }
    start = i ;
    // str1 is the start of our search string - to find the first "delimiter"
    // find first string <== the first delimiter after start of non-delimiter text
    for ( i = 0, str2 = str1 ; *str2 ; str2 ++, i ++ )
    {
        if ( _CharSet_IsDelimiter ( cset, *str2 ) ) break ;
        buffer [ i ] = * str2 ;
    }
    // str2 is either a delimiter or a null, which is also a delimiter. Add 1 for start of a possible next token ...
    if ( *str2 ) // check if it's just a null, the end of string - 0
    {
        buffer [ i ] = 0 ; // remember buffer is sti->Out, the arg to the macro function, the pre-expanded string, if that macro function exists in SMNamespace
        end = start + i ;
        sti->StartIndex = start ;
        sti->EndIndex = end ;
    }
    else end = 0 ;

    return end ;
}

byte *
_StringMacro_Run ( byte * pb_namespaceName, byte * str )
{
    byte *nstr ;
    Word * sword = 0 ;
    Namespace * ns ;
    if ( pb_namespaceName )
    {
        ns = Namespace_Find ( pb_namespaceName ) ;
        if ( ns ) sword = _Finder_FindWord_InOneNamespace ( _Finder_, ns, str ) ;
    }
    else sword = Finder_FindWord_AnyNamespace ( _Finder_, str ) ;
    if ( sword )
    {
        if ( sword->W_ObjectAttributes & TEXT_MACRO ) nstr = sword->TextMacroValue ;
        else //if ( sword->W_ObjectAttributes & STRING_MACRO )
        {
            Word_Morphism_Run ( sword ) ;
            nstr = ( byte* ) DataStack_Pop ( ) ;
        }
        return nstr ;
    }
    return 0 ;
}

void
_CSL_StringMacros_Init ( )
{
    StrTokInfo * sti = & _CSL_->Sti ;
    //byte * pb_nsn = StringMacro_Run ( "Root", "_SMN_" ) ; // _SMN_ StringMacrosNamespace
    byte * pb_nsn = _StringMacro_Run ( 0, ( byte* ) "_SMN_" ) ; // _SMN_ StringMacrosNamespace
    if ( pb_nsn )
    {
        sti->SMNamespace = pb_nsn ;
        byte * delimiters = _StringMacro_Run ( pb_nsn, ( byte* ) "Delimiters" ) ;
        if ( ! delimiters ) delimiters = _Context_->Lexer0->TokenDelimiters ;
        //memset ( sti, 0, sizeof (StrTokInfo ) ) ;
        // sti->In will be set in _CSL_StrTok
        sti->Delimiters = delimiters ;
        sti->CharSet0 = CharSet_New ( delimiters, DICTIONARY ) ; //TEMPORARY ) ;
        CharSet_SetChar ( sti->CharSet0, '"' ) ; // always add a '"' as a delimiter
        sti->Out = Buffer_Data ( _CSL_->StringMacroB ) ;
        SetState ( sti, STI_INITIALIZED, true ) ;
    }
    else SetState ( sti, STI_INITIALIZED, false ) ;
}

byte *
StringMacros_Do ( byte * buffer, byte * namespace, byte * ostr, int64 startIndex, int64 endIndex ) // buffer :: the string to which we apply any set macros also cf. .init.csl beginning for how to initialize 
{
    byte * nstr = _StringMacro_Run ( namespace, ostr ) ;

    if ( nstr )
    {
        //_Lexer_AppendByteToSourceCode ( _Lexer_, ':', 0 ) ;
        String_InsertDataIntoStringSlot ( buffer, startIndex, endIndex, nstr, ostr ) ; // use the original buffer for the total result of the macro
    }
    return nstr ;
}
// _CSL_StringMacros_Do ::
// get first string delimited by the initialized Delimiters variable in 'buffer' variable, find its macro 
// in the initialized Namespace and substitute/insert it into the original string - 'buffer' : (in place - 
// string must have room for expansion) 

void
CSL_StringMacros_Do ( byte * buffer ) // buffer :: the string to which we apply any set macros also cf. .init.csl beginning for how to initialize 
{
    StrTokInfo * sti = & _CSL_->Sti ;
    if ( _CSL_StrTok ( buffer ) ) // ==> sti->Out :: get first string delimited by the initialized Delimiters variable, find its macro and substitute/insert it in the string
    {
        StringMacros_Do ( buffer, sti->SMNamespace, sti->Out, sti->StartIndex, sti->EndIndex ) ; // buffer :: the string to which we apply any set macros also cf. .init.csl beginning for how to initialize 
    }
}

byte *
_String_Get_ReadlineString_ToEndOfLine ( )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    byte * str = String_New ( & rl->InputLine [rl->ReadIndex], TEMPORARY ) ;
    ReadLiner_CommentToEndOfLine ( rl ) ;
    SetState ( _Context_->Lexer0, LEXER_DONE, true ) ;

    return str ;
}

Boolean
IsPunct ( byte b )
{
    if ( ispunct ( b ) && ( b != '_' ) ) return true ;
    else return false ;
}

#if 0
// don't remember why this was needed and it doesn't seem to be needed now
// but i modified it somewhat even though so ... 
// ?? necessary ; works ??

int64
String_CheckWordSize ( byte * str, int64 wl )
{
    byte * start, *end ;
    int64 i = - 1, length ;
    if ( wl > 1 )
    {
        Boolean lPunctFlag = IsPunct ( str [0] ), rPunctFlag = IsPunct ( str [wl - 1] ) ; // an xPunctFlag means first/last character of word is a punctuation char

        if ( ! lPunctFlag )
        {
            for ( i = - 1 ; abs ( i ) < ( wl + 1 ) ; i -- ) // go to left of str first
            {
                if ( IsPunct ( str[ i ] ) || ( str[ i ] == ' ' ) ) break ;
            }
        }
        start = & str [i + 1] ;
        for ( i = wl ; i < ( wl + 1 ) ; i ++ ) // ... then to the right side of str
        {
            if ( rPunctFlag )
            {
                if ( ( i >= wl ) && ( ! IsPunct ( str[i] ) ) ) break ; // assumes
            }
            else if ( IsPunct ( str[i] ) || ( str[i] == ' ' ) ) break ;
        }
        end = & str [i - 1] ;
        length = end - start + 1 ;
        return length == wl ;
    }
    return true ;
}
#endif

// this code is also used in PrepareSourceCodeString in csl.c 
// it makes or attempts to make sure that that tokenStart (ts) is correct for any string

int64
String_FindStrnCmpIndex ( byte * str, byte* name0, int64 index, int64 wlen, int64 inc )
{
    byte * scspp2, *scspp, *stri ;
    d0 ( scspp = & str [ index ] ) ;
    int64 i, n, slsc = Strlen ( str ) ;
    for ( i = 0, n = inc + 1 ; ( i <= n ) ; i ++ ) // tokens are parsed in different order with parameter and c rtl args, etc. 
    {
        stri = & str [ index + i ] ;
        if ( ( index + i <= slsc ) && ( ! Strncmp ( stri, name0, wlen ) ) )
        {
            //if ( String_CheckWordSize ( scindex, wl0 ) ) 
            {
                index += i ;
                goto done ;
            }
        }
    }
    for ( i = 0, n = inc + 1 ; ( i <= n ) ; i ++ ) // tokens are parsed in different order with parameter and c rtl args, etc. 
    {
        stri = & str [ index - i ] ;
        if ( ( ( index - 1 ) >= 0 ) && ( ! Strncmp ( stri, name0, wlen ) ) )
        {
            //if ( String_CheckWordSize ( scindex, wl0 ) ) 
            {
                index -= i ;
                goto done ;
            }
        }
    }
    //index = index ;
    return - 1 ;
done:
    d0 ( scspp2 = & str [ index ] ) ;
    return index ;
}

// the border (aesthetically) surrounds (equally or sliding) on either side of a token, it's string length, in the tvw - available terminal view window space; 
// token slides in the window which is 2 * border + token length 
// |ilw...------ inputLine  -----|lef|--- leftBorder ---|---token---|---  rightBorder  ---|ref|------ inputLine -----...ilw| -- ilw : inputLine window
// ref : right ellipsis flag
// lef : left ellipsis flag
// dl : diff in length of token and token with highlighting :: dl = slt1 - slt0 :: not currently used

byte *
_String_HighlightTokenInputLine ( byte * nvw, Boolean lef, int64 leftBorder, int64 tokenStart, byte *token, byte * token0, int64 rightBorder, Boolean ref )
{
    int32 slt = Strlen ( token ) ;
    String_RemoveFinalNewline ( nvw ) ;
    if ( ! GetState ( _Debugger_, DEBUG_SHTL_OFF ) )
    {
        byte * b2 = Buffer_Data_Cleared ( _CSL_->DebugB2 ) ;
        byte * b3 = Buffer_Data_Cleared ( _CSL_->ScratchB3 ) ;
        // inputLineW is the inputLine line (window) start that we use here
        // we are building our output in b2
        // our scratch buffer is b3
#if 0    
        if ( GetState ( _Debugger_, ( DBG_OUTPUT_SUBSTITUTION ) ) )
        {
            int64 slt0 = strlen ( token0 ) ;
            Strncpy ( nvw, &il[nws], scswci ) ; //leftBorder + (slt0/2) ) ; //scswci ) ; // tvw ) ; // copy the the new view window to buffer nvw
            Strncat ( nvw, token, slt ) ; // tvw ) ; // copy the the new view window to buffer nvw
            Strncat ( nvw, &il[nws + slt0], tvw ) ; // tvw ) ; // copy the the new view window to buffer nvw
        }
        //else 
#endif    
        if ( leftBorder < 0 ) leftBorder = 0 ; // something more precise here with C syntax is needed !?!?
        if ( lef )
        {
            strncpy ( ( char* ) b3, " .. ", 4 ) ;
            if ( leftBorder > 4 ) strncat ( ( char* ) b3, ( char* ) &nvw [ 4 ], leftBorder - 4 ) ; // 4 : strlen " .. " 
        }
        else strncpy ( ( char* ) b3, ( char* ) nvw, leftBorder ) ;

        strcpy ( ( char* ) b2, ( char* ) cc ( b3, &_O_->Debug ) ) ;
        char * ccToken = ( char* ) cc ( token, &_O_->Notice ) ; //&_O_->Default ) ;
        strcat ( ( char* ) b2, ccToken ) ;

        if ( ref )
        {
            if ( rightBorder > 4 ) strncpy ( ( char* ) b3, ( char* ) &nvw [ tokenStart + slt ], rightBorder - 4 ) ; // 4 : strlen " .. " 
            strcat ( ( char* ) b3, " .. " ) ;
        }
        else strcpy ( ( char* ) b3, ( char* ) &nvw [ tokenStart + slt ] ) ; //, BUFFER_SIZE ) ; // 3 : [0 1 2 3]  0 indexed array
        char * ccR = ( char* ) cc ( b3, &_O_->Debug ) ;
        strcat ( ( char* ) b2, ccR ) ;

        nvw = b2 ;
    }
    SetState ( _Debugger_, DBG_OUTPUT_INSERTION | DBG_OUTPUT_SUBSTITUTION, false ) ;
    return nvw ;
}

int64
_IsString ( byte * address, int64 maxLength )
{
    int64 i, count ;
    if ( address < ( byte* ) 0x7f0000000000 ) return false ; // prevent integer 'string's 
    for ( i = 0, count = 0 ; i < maxLength ; i ++ )
    {
        //if ( ! ( isprint ( address [i] ) || iscntrl ( address [i] ) ) ) return false ;
        if ( ! address [i] )
        {
            if ( count ) return true ;
            else return false ;
        }
        if ( ( address [i] >= ' ' && address [i] < 128 ) || ( address [i] == '\n' ) || ( address [i] == '\t' ) ) count ++ ;
        else return false ;
    }
    return true ;
}

byte *
IsString ( byte * address )
{
    if ( _IsString ( address, 64 ) ) return address ;
    else return 0 ;
}

byte *
String_CheckForAtAdddress ( byte * address, Colors * c1, Colors * c2 )
{
    byte *str = 0 ;
    if ( NamedByteArray_CheckAddress ( _O_->MemorySpace0->StringSpace, address )
        || NamedByteArray_CheckAddress ( _O_->MemorySpace0->CompilerTempObjectSpace, address )
        || NamedByteArray_CheckAddress ( _O_->MemorySpace0->SessionObjectsSpace, address )
        || NamedByteArray_CheckAddress ( _O_->MemorySpace0->TempObjectSpace, address )
        || NamedByteArray_CheckAddress ( _O_->MemorySpace0->DictionarySpace, address ) )
    {
        if ( IsString ( address ) )
        {
            str = Buffer_Data_Cleared ( _CSL_->StringInsertB5 ) ;
            byte * bstr = String_ConvertToBackSlash ( address ) ;
            byte * prefix = cc ( "< string : \'", c2 ) ;
            byte * cbstr = cc ( bstr, c1 ) ;
            byte * postfx = cc ( "\' >", c2 ) ;
            snprintf ( ( char* ) str, 128, "%s%s%s", prefix, cbstr, postfx ) ;
            //strcat ( str, cc ( "\' >", c2 ) ) ; 
        }
    }
    return str ;
}

byte *
String_DelimitSourceCodeStartForLispCSL ( char * sc )
{
    byte * start = 0 ;
    while ( *sc )
    {
        switch ( *sc )
        {
            case '(': goto next ;
            case '"':
            {
                sc ++ ;
                while ( *sc != '"' ) sc ++ ;
                break ;
            }
            case ':':
            {
                if ( *( sc + 1 ) == ':' )
                {
                    sc ++ ;
                    break ;
                }
                else
                {
                    //start = sc ;
                    goto next ;
                }
            }
        }
        sc ++ ;
    }
next:
    start = ( byte* ) sc ;
    while ( *sc )
    {
        switch ( *sc )
        {
            case ')': return start ;
            case '"':
            {
                while ( *sc ++ != '"' ) ;
                sc ++ ;
                break ;
            }
            case ';':
            {
                if ( *( sc + 1 ) == ';' )
                {
                    sc += 2 ;
                }
                *( sc + 1 ) = ' ' ;
                *( sc + 2 ) = 0 ;
                return start ;
            }
        }
        sc ++ ;
    }
    return start ;
}

byte *
Buffer_Data_QuickReset ( Buffer * b )
{
    b->B_Data[0] = 0 ;
    return Buffer_Data ( b ) ;
}

byte *
Buffer_Data_Cleared ( Buffer * b )
{
    Mem_Clear ( b->B_Data, b->B_Size ) ;
    return Buffer_Data ( b ) ;
}

void
Buffer_Init ( Buffer * b, int64 flag )
{
    Mem_Clear ( b->B_Data, b->B_Size ) ;
    b->InUseFlag = flag ;
}

void
Buffer_Add ( Buffer * b, int64 flag )
{
    if ( flag & N_PERMANENT ) dllist_AddNodeToTail ( _O_->BufferList, ( dlnode* ) b ) ;
    else dllist_AddNodeToHead ( _O_->BufferList, ( dlnode* ) b ) ;
}

Buffer *
Buffer_Create ( int64 size )
{
    Buffer * b = ( Buffer * ) Mem_Allocate ( sizeof ( Buffer ) + size + 1, BUFFER ) ;
    b->B_CAttribute = BUFFER ;
    b->B_Size = size ;
    b->B_Data = ( byte* ) b + sizeof (Buffer ) ;
    return b ;
}

Buffer *
_Buffer_New ( int64 size, int64 flag )
{
    dlnode * node, * nextNode ;
    Buffer * b = 0 ;
    d0 ( Buffer_PrintBuffers ( ) ) ;
    if ( _O_ && _O_->MemorySpace0 )
    {
        for ( node = dllist_First ( ( dllist* ) _O_->BufferList ) ; node ; node = nextNode )
        {
            nextNode = dlnode_Next ( node ) ;
            b = ( Buffer* ) node ;
            d0 ( if ( b->InUseFlag != N_PERMANENT ) Printf ( "\n_Buffer_New : buffer = 0x%08x : flag = 0x%08x : size = %d : length = %d : data = %s\n", b, b->InUseFlag, b->B_Size, strlen ( b->B_Data ), b->B_Data ) ) ;
            if ( ( b->InUseFlag & ( N_FREE | N_UNLOCKED ) ) && ( b->B_Size >= size ) ) goto init ;
            else continue ; //if ( b->InUseFlag == N_PERMANENT ) continue ; //break ;
            //if ( ( b->InUseFlag != N_PERMANENT ) && ( b->B_Size >= size ) ) goto init ;
        }
    }
    d0 ( Buffer_PrintBuffers ( ) ) ;
    b = Buffer_Create ( size ) ;
init:
    Buffer_Init ( b, flag ) ;
    Buffer_Add ( b, flag ) ;
    return b ;
}
// set all non-permanent buffers as unused - available

int64
Buffer_SetAsFree ( Buffer * b, int64 force )
{
    if ( b->InUseFlag & ( force ? ( N_IN_USE | N_LOCKED | N_UNLOCKED ) : ( N_UNLOCKED ) ) )
    {
        _Buffer_SetAsFree ( b ) ; // must check ; others may be permanent or locked ( true + 1, true + 2) .
        return true ;
    }
    return false ;
}

void
Buffers_SetAsUnused ( int64 force )
{
    dlnode * node, * nextNode ;
    Buffer * b ;
    int64 total = 0, setFree = 0 ;
    if ( _O_ && _O_->MemorySpace0 )
    {
        for ( node = dllist_First ( ( dllist* ) _O_->BufferList ) ; node ; node = nextNode )
        {
            nextNode = dlnode_Next ( node ) ;
            b = ( Buffer* ) node ;
            if ( Buffer_SetAsFree ( b, force ) ) setFree ++ ;
            total ++ ;
        }
    }
    d0 ( if ( setFree > 2 ) Printf ( "\nBuffers_SetAsUnused : total = %d : freed = %d", total, setFree ) ) ;
}

void
Buffer_PrintBuffers ( )
{
    dlnode * node, * nextNode ;
    Buffer * b ;
    int64 total = 0, free = 0, locked = 0, unlocked = 0, permanent = 0 ;
    if ( _O_ && _O_->MemorySpace0 )
    {
        for ( node = dllist_First ( ( dllist* ) _O_->BufferList ) ; node ; node = nextNode )
        {
            b = ( Buffer* ) node ;
            //Printf ( "\nBuffer_PrintBuffers : buffer = 0x%08x : nextNode = 0x%08x : flag = 0x%08x : size = %d : length = %d : data = %s\n", b, dlnode_Next ( node ), b->InUseFlag, b->B_Size, strlen ( b->B_Data ), b->B_Data ) ;
            nextNode = dlnode_Next ( node ) ;
            if ( b->InUseFlag & N_FREE ) free ++ ;
            else if ( b->InUseFlag & N_UNLOCKED ) unlocked ++ ;
            else if ( b->InUseFlag & N_LOCKED ) locked ++ ;
            else if ( b->InUseFlag & N_PERMANENT ) permanent ++ ;
            total ++ ;
        }
    }
    if ( _O_->Verbosity > 1 ) Printf ( "\nBuffer_PrintBuffers : total = %d : free = %d : unlocked = %d : locked = %d : permanent = %d", total, free, unlocked, locked, permanent ) ;
}

Buffer *
Buffer_New ( int64 size )
{
    return _Buffer_New ( size, N_UNLOCKED ) ;
}

Buffer *
Buffer_NewLocked ( int64 size )
{
    return _Buffer_New ( size, N_LOCKED ) ;
}

Buffer *
_Buffer_NewPermanent ( int64 size )
{
    return _Buffer_New ( size, N_PERMANENT ) ;
}

byte *
_Buffer_New_pbyte ( int64 size, int64 flag )
{
    Buffer *b = _Buffer_New ( size, flag ) ;
    return Buffer_Data ( b ) ;
}

byte *
Buffer_New_pbyte ( int64 size )
{
    //Buffer *b = Buffer_NewLocked ( size ) ;
    Buffer *b = _Buffer_New ( size, N_LOCKED ) ;
    return Buffer_Data ( b ) ;
}

void
_MemCpy ( byte *dst, byte *src, int64 size )
{
    int64 i ;
    for ( i = 0 ; i < size ; i ++ ) dst [i] = src[i] ;
}


#if 0 // some future possibly useful string functions
// returns end : an offset from 0 from which a strtok for a possible next token can be undertaken
// token found is in caller's buffer arg

int64
_StrTok ( byte * str0, byte * buffer, byte * cset )
{
    int64 i, start, end ;
    byte *str1, *str2 ;
    // find start of non-delimiter text
    // str0 is the original string
    for ( i = 0, str1 = str0 ; *str1 ; str1 ++, i ++ )
    {
        if ( ! _CharSet_IsDelimiter ( cset, *str1 ) ) break ;
    }
    start = i ;
    // str1 is the start of our search string - to find the first "delimiter"
    // find first string <== the first delimiter after start of non-delimiter text
    for ( i = 0, str2 = str1 ; *str2 ; str2 ++, i ++ )
    {
        if ( _CharSet_IsDelimiter ( cset, *str2 ) ) break ;
        buffer [ i ] = * str2 ;
    }
    buffer [ i ] = 0 ;
    // str2 is either a delimiter or a null, which is also a delimiter. Add 1 for start of a possible next token ...
    if ( *str2 ) // check if it's just a null, the end of string - 0
    {
        // remember buffer is sti->Out, the arg to the macro function, the pre-expanded string, if that macro function exists in SMNamespace
        end = start + i ; // end is an offset from 0 from which a strtok for a possible next token can be undertaken
    }
    else end = 0 ;

    return end ;
}

byte *
String_GetDelimitedString ( byte * str0, byte delimiter )
{
    int64 i ;
    byte * str = String_New ( str0, TEMPORARY ) ;
    for ( i = 0 ; str [i] ; i ++ )
    {
        if ( str [i] == delimiter )
        {
            str [i] = 0 ;

            return str ;
        }
    }
}

int64
_String_CountTabs ( byte * start, byte * end )
{
    int64 n ;
    for ( n = 0 ; start != end ; start ++ )
    {

        if ( *start == '\t' ) n ++ ;
    }
    return n ;
}
#endif

