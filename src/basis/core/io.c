
#include "../../include/csl.h"

int64
GetTerminalWidth ( )
{
#ifdef TIOCGSIZE
    struct ttysize ts ;
    ioctl ( STDIN_FILENO, TIOCGSIZE, &ts ) ;
    //cols = ts.ts_cols ;
    //lines = ts.ts_lines;
    return ts.ts_cols ;
#elif defined(TIOCGWINSZ)
    struct winsize ts ;
    ioctl ( STDIN_FILENO, TIOCGWINSZ, &ts ) ;
    //ioctl ( STDOUT_FILENO, TIOCGWINSZ, &ts ) ;
    //cols = ts.ws_col ;
    //lines = ts.ws_row;
    return ts.ws_col ;
#endif /* TIOCGSIZE */
}

char
kbhit ( void )
{
    int64 oldf ;

    oldf = fcntl ( STDIN_FILENO, F_GETFL, 0 ) ;
    fcntl ( STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK ) ;
    char ch = getchar ( ) ;
    if ( ch < 0 ) ch = 0 ;
    fcntl ( STDIN_FILENO, F_SETFL, oldf ) ;
    return ch ;
}

#if 0

int64
GetC ( )
{
    byte buffer [4] ;
    read ( STDIN_FILENO, buffer, 1 ) ;
    write ( STDOUT_FILENO, buffer, 1 ) ;
    return buffer [0] ;
}

// from : http://stackoverflow.com/questions/16026858/reading-the-device-status-report-ansi-escape-sequence-reply

void
getCursor ( int* x, int* y )
{
    Printf ( ( byte* ) "\033[6n" ) ;
    int out = scanf ( "\033[%d;%dR", x, y ) ;
    //fflush ( stdout ) ; 
    //fflush ( stdin ) ; 
}
#endif

#define KEY() getc ( stdin )

int64
_Key ( FILE * f )
{
    int64 key = getc ( f ) ; // GetC () ;
    return key ;
}

void
Kbhit_Pause ( )
{
    if ( kbhit ( ) == ESC ) OpenVmTil_Pause ( ) ;
}

int64
Key_Kbhit ( FILE * f )
{
    int64 key = _Key ( f ) ;
    if ( ! GetState ( _Debugger_, DBG_STEPPING ) ) Kbhit_Pause ( ) ;
    return key ;
}

int64
Key ( )
{
    return Key_Kbhit ( stdin ) ;
}

byte
_CSL_Key ( ReadLiner * rl )
{
    int key = _Key ( rl->InputFile ) ;
    return ( byte ) key ;
}

void
_CSL_PrintString ( byte * string ) //  '."'
{
    printf ( "%s", string ) ;
    fflush ( stdout ) ;
}

void
_CSL_PrintChar ( byte c ) //  '."'
{
    printf ( "%c", c ) ;
    fflush ( stdout ) ;
}

void
Emit ( byte c )
{
    _CSL_PrintChar ( c ) ;
}

void
Context_DoPrompt ( Context * cntx )
{
    if ( ( ReadLiner_GetLastChar ( ) != '\n' ) || ( ! IS_INCLUDING_FILES ) || ( GetState ( _Debugger_, DBG_ACTIVE ) ) )
    {
        _CSL_PrintChar ( '\n' ) ; //_Printf ( ( byte* ) "\n" ) ;
    }
    Printf ( ( byte* ) "%s", ( char* ) cntx->ReadLiner0->NormalPrompt ) ; // for when including files
}

void
CSL_DoPrompt ( )
{
    Context_DoPrompt ( _Context_ ) ;
}

// all output comes thru here

void
_Printf ( byte *format, ... )
{
    va_list args ;

    va_start ( args, ( char* ) format ) ;
    vprintf ( ( char* ) format, args ) ;
    va_end ( args ) ;
    fflush ( stdout ) ;
}

void
Printf ( byte *format, ... )
{
    if ( kbhit ( ) == ESC ) OpenVmTil_Pause ( ) ;
    if ( _O_->Verbosity ) //GetState ( _ReadLiner_, CHAR_ECHO ) )
    {
        va_list args ;

        va_start ( args, ( char* ) format ) ;
        vprintf ( ( char* ) format, args ) ;
        va_end ( args ) ;
        fflush ( stdout ) ;

        if ( _CSL_ && _CSL_->LogFlag && _CSL_->LogFILE )
        {
            va_start ( args, ( char* ) format ) ;
            vfprintf ( _CSL_->LogFILE, ( char* ) format, args ) ;
            va_end ( args ) ;
            fflush ( _CSL_->LogFILE ) ;
        }
    }
    //ReadLiner_SetLastChar ( 0 ) ; //
}

