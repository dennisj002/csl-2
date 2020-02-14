
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
    //if ( kbhit ( ) == ESC ) OpenVmTil_Pause ( ) ;
    //if ( _O_->Verbosity ) //GetState ( _ReadLiner_, CHAR_ECHO ) )
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

#if 0
// try not to (don't) print extra newlines
// this is called on exceptions so alot of checking 

void
Printf ( byte *format, ... )
{
    if ( kbhit ( ) == ESC ) OpenVmTil_Pause ( ) ; //CSL_Quit ( ) ;
    if ( _O_ && _CSL_ && _O_->Verbosity )
    {
        va_list args ;
        va_start ( args, ( char* ) format ) ;
        char * out = ( char* ) Buffer_Data ( _CSL_->PrintfB ) ;
        vsprintf ( ( char* ) out, ( char* ) format, args ) ;
        va_end ( args ) ;
        int64 len = Strlen ( ( char* ) out ) ;
        byte final = out [ len - 1 ] ;
        if ( _O_->psi_PrintStateInfo )
        {
            if ( out [0] == '\n' )
            {
                if ( ( _O_->psi_PrintStateInfo->OutputLineCharacterNumber < 2 ) && ( GetState ( _O_->psi_PrintStateInfo, PSI_NEWLINE ) ) ) out = & out [1] ;
                else if ( _O_->psi_PrintStateInfo && GetState ( _O_->psi_PrintStateInfo, PSI_PROMPT ) )
                {
                    out [0] = '\r' ;
                    SetState ( _O_->psi_PrintStateInfo, PSI_PROMPT, false ) ;
                }
            }
        }
        printf ( "%s", out ) ;
        if ( _CSL_ && _CSL_->LogFlag ) fprintf ( _CSL_->LogFILE, "%s", out ) ;
        if ( _O_->psi_PrintStateInfo )
        {
            if ( ( final == '\n' ) || ( final == '\r' ) )
            {
                _O_->psi_PrintStateInfo->OutputLineCharacterNumber = 0 ;
                ConserveNewlines ;
            }
            else
            {
                _O_->psi_PrintStateInfo->OutputLineCharacterNumber += len ;
                AllowNewlines ;
            }
        }
        fflush ( stdout ) ;
    }
}

PrintStateInfo *
PrintStateInfo_New ( )
{
    PrintStateInfo * psi = ( PrintStateInfo * ) Mem_Allocate ( sizeof ( PrintStateInfo ), OPENVMTIL ) ;
    //PrintStateInfo * psi = ( PrintStateInfo * ) MemList_AllocateChunk ( &_MemList_, sizeof ( PrintStateInfo ), OPENVMTIL ) ; ;
    SetState ( psi, PSI_PROMPT, false ) ;
    SetState ( psi, PSI_NEWLINE, true ) ;
    return psi ;
}
#endif

#if LISP_IO

byte *
_vprintf ( FILE * f, char *format, ... )
{
    va_list args ;
    va_start ( args, ( char* ) format ) ;
    __Printf ( ( byte* ) format, args ) ;
}

uint64
Getc ( FILE * f )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    if ( f != stdin ) return fgetc ( f ) ;
    if ( Maru_RawReadFlag ) return ReadLine_Get_Key ( rl ) ;
    else return ( int64 ) ReadLine_NextChar ( rl ) ;
}

uint64
Getwc ( FILE * f )
{
    return btowc ( Getc ( f ) ) ;
}

void
UnGetc ( int64 c, FILE * f )
{
    if ( f == stdin )
        ReadLine_UnGetChar ( _Context_->ReadLiner0 ) ;
    else ungetc ( c, f ) ;
}

void
UnGetwc ( int64 c, FILE * f )
{
    return UnGetc ( wctob ( c ), f ) ;
}
#endif

#if 0

void
__CSL_Emit ( byte c )
{
    if ( ( c == '\n' ) || ( c == '\r' ) )
    {
        if ( _Context_->ReadLiner0->OutputLineCharacterNumber == 0 ) return ;
        else
        {
            //if ( ! overWrite ) 
            c = '\n' ; // don't overwrite the existing line
            _Context_->ReadLiner0->OutputLineCharacterNumber = 0 ;
        }
    }
    else _Context_->ReadLiner0->OutputLineCharacterNumber ++ ;
    if ( _O_->Verbosity ) putc ( c, _Context_->ReadLiner0->OutputFile ) ;
}

void
_CSL_EmitString ( byte * string )
{
#if 1
    int64 i ;
    //if ( _Context->ReadLiner0->Flags & CHAR_ECHO )
    {
        for ( i = 0 ; string [ i ] ; i ++ )
        {
            if ( kbhit ( ) == ESC ) CSL_Quit ( ) ;
            __CSL_Emit ( string [ i ] ) ;
        }
    }
#else
    if ( kbhit ( ) == ESC ) CSL_Quit ( ) ;
    puts ( ( char* ) string ) ;
#endif
}
#endif

