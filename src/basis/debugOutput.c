
#include "../include/csl.h"

void
Debugger_Menu ( Debugger * debugger )
{
    Printf ( ( byte* )
        "\nDebug Menu at : \n%s :\n(m)enu, so(U)rce, dum(p), (e)val, (d)is, dis(a)ccum, dis(A)ccum, (r)egisters, (l)ocals, (v)ariables, (I)nfo, (W)dis, s(h)ow"
        "\n(R)eturnStack, sto(P), (S)tate, (c)ontinue, (s)tep, (o)ver, (i)nto, o(u)t, t(h)ru, s(t)ack, auto(z), (V)erbosity, (q)uit, a(B)ort"
        "\nusi(N)g, s(H)ow DebugWordList, sh(O)w CompilerWordList, Goto(L)ist_Print, T(y)peStackPrint, (w)diss, e(x)it"
        "\n'\\n' - escape, , '\\\' - <esc> - escape, ' ' - <space> - continue", c_gd ( Context_Location ( ) ) ) ;
    SetState ( debugger, DBG_MENU, false ) ;
}

void
_Debugger_Locals_ShowALocal ( Cpu * cpu, Word * localsWord, Word * scWord ) // use a debugger buffer instead ??
{
    Word * word2 = 0 ;
    byte * buffer = Buffer_Data_Cleared ( _CSL_->DebugB ) ; // nvw : new view window
    uint64 * fp = cpu->CPU_FP ; //? ( uint64* ) ((* cpu->CPU_FP)? ( uint64* ) (* cpu->CPU_FP) : (cpu->CPU_FP)) : 0 ;
    //if ( fp > ( uint64* ) 0x7f000000 )
    {
        if ( Compiling ) scWord = _CSL_->SC_Word ;
        int64 localVarFlag = ( localsWord->W_ObjectAttributes & LOCAL_VARIABLE ) ; // nb! not a Boolean with '='
        int64 varOffset = _LocalOrParameterVar_Offset ( localsWord, scWord->W_NumberOfNonRegisterArgs,
            IsFrameNecessary ( scWord->W_NumberOfNonRegisterLocals, scWord->W_NumberOfNonRegisterArgs ) ) ;
        byte * address = ( byte* ) ( uint64 ) ( fp ? fp [ varOffset ] : 0 ) ;

        byte * stringValue = String_CheckForAtAdddress ( address ) ;
        if ( address && ( ! stringValue ) ) word2 = Word_GetFromCodeAddress ( ( byte* ) ( address ) ) ;
        if ( word2 ) sprintf ( ( char* ) buffer, "< %s.%s %s", word2->ContainingNamespace->Name, c_u ( word2->Name ), c_g ( ">" ) ) ;

        if ( localsWord->W_ObjectAttributes & REGISTER_VARIABLE )
        {
            char * registerNames [ 16 ] = { ( char* ) "RAX", ( char* ) "RCX", ( char* ) "RDX", ( char* ) "RBX",
                ( char* ) "RBP", ( char* ) "RSP", ( char* ) "RSI", ( char* ) "RDI", ( char* ) "R8", ( char* ) "R9",
                ( char* ) "R10", ( char* ) "R11", ( char* ) "R12", ( char* ) "R13", ( char* ) "R14", ( char* ) "R15" } ;
            Printf ( ( byte* ) "\n%-018s : index = [%3s:%d   ]  : <register  location> = 0x%016lx : %16s.%-16s : %s",
                "Register Variable", registerNames [ localsWord->RegToUse ], localsWord->RegToUse, cpu->Registers [ localsWord->RegToUse ],
                localsWord->S_ContainingNamespace->Name, c_u ( localsWord->Name ), word2 ? buffer : stringValue ? stringValue : ( byte* ) "" ) ;
        }
        else Printf ( ( byte* ) "\n%-018s : index = [r15%s0x%02x]  : <0x%016lx> = 0x%016lx : %16s.%-16s : %s",
            localVarFlag ? "LocalVariable" : "Parameter Variable", localVarFlag ? "+" : "-", Abs ( varOffset * CELL ),
            fp + varOffset, ( uint64 ) ( fp ? fp [ varOffset ] : 0 ), localsWord->S_ContainingNamespace->Name,
            c_u ( localsWord->Name ), word2 ? buffer : stringValue ? stringValue : ( byte* ) "" ) ;
    }
}

void
_Debugger_Locals_Show_Loop ( Cpu * cpu, Stack * stack, Word * scWord )
{
    int64 n, i ;
    Word * localsWord ;
    if ( stack )
    {
        _Word_ShowSourceCode ( scWord ) ;
        if ( ! Compiling )
        {
            int64 * fp = ( int64* ) cpu->CPU_FP, * dsp = ( int64* ) cpu->CPU_DSP ;
            if ( ( ( uint64 ) fp < 0x7f0000000 ) ) fp = dsp ;
            Printf ( ( byte* ) "\n%s.%s.%s : \nFrame Pointer = R15 = <0x%016lx> = 0x%016lx : Stack Pointer = R14 <0x%016lx> = 0x%016lx",
                scWord->ContainingNamespace ? c_gd ( scWord->ContainingNamespace->Name ) : ( byte* ) "", c_gd ( scWord->Name ), c_gd ( "locals" ),
                ( uint64 ) fp, fp ? *fp : 0, ( uint64 ) dsp, dsp ? *dsp : 0 ) ;
        }
        //Namespace_NamespacesStack_PrintWords ( stack ) ;
        for ( i = 0, n = Stack_Depth ( stack ) ; i < n ; i ++ )
        {
            Namespace * ns = ( Namespace* ) _Stack_N ( stack, i ) ; //Stack_Pop ( stack ) ;
            //_Namespace_PrintWords ( ns ) ;
            dlnode * node, *prevNode ;
            for ( node = dllist_Last ( ns->W_List ) ; node ; node = prevNode )
            {
                prevNode = dlnode_Previous ( node ) ;
                localsWord = ( Word * ) node ;
                _Debugger_Locals_ShowALocal ( cpu, localsWord, scWord ) ;
            }
        }
    }
}

void
Debugger_Locals_Show ( Debugger * debugger )
{
    Word * scWord = Compiling ? _Context_->CurrentWordBeingCompiled : debugger->w_Word ? debugger->w_Word :
        ( debugger->DebugAddress ? Word_UnAlias ( Word_GetFromCodeAddress ( debugger->DebugAddress ) ) : _Context_->CurrentlyRunningWord ) ;
    if ( scWord && ( scWord->W_NumberOfVariables || _Context_->Compiler0->NumberOfVariables ) )
        _Debugger_Locals_Show_Loop ( debugger->cs_Cpu, scWord->NamespaceStack ? scWord->NamespaceStack : _Compiler_->LocalsCompilingNamespacesStack, scWord ) ;
}

void
_Debugger_ShowEffects ( Debugger * debugger, Word * word, Boolean stepFlag, Boolean force, int64 debugLevel )
{
    uint64* dsp = GetState ( debugger, DBG_STEPPING ) ? ( _Dsp_ = debugger->cs_Cpu->R14d ) : _Dsp_ ;
    if ( ! dsp ) CSL_Exception ( STACK_ERROR, 0, QUIT ) ;
    if ( Is_DebugOn && ( _CSL_->DebugLevel >= debugLevel ) && ( force || stepFlag || ( word && ( word != debugger->LastShowEffectsWord ) ) ||
        ( debugger->PreHere && ( Here > debugger->PreHere ) ) ) )
    {
        DebugColors ;
        if ( word && ( word->W_ObjectAttributes & OBJECT_FIELD ) ) Word_PrintOffset ( word, 0, 0 ) ;
        //&& ( ! ( word->W_MorphismAttributes & (DOT|OBJECT_OPERATOR) ) ) ) Word_PrintOffset ( word, 0, 0 ) ;
        _Debugger_DisassembleWrittenCode ( debugger ) ;
        Debugger_ShowChange ( debugger, word, stepFlag, dsp ) ;
        //DebugColors ;
        debugger->LastShowEffectsWord = word ;
        //Set_DataStackPointers_FromDebuggerDspReg ( ) ;
    }
    Debugger_Setup_ResetState ( debugger ) ;
    debugger->ShowLine = 0 ;
}

void
_Debugger_ShowInfo ( Debugger * debugger, byte * prompt, int64 signal, int64 force )
{
    if ( force || ( debugger->w_Word != debugger->LastShowInfoWord ) )
    {
        Context * cntx = _Context_ ;
        ReadLiner * rl = cntx->ReadLiner0 ;
        byte *location = ( rl->Filename ) ? rl->Filename : ( byte* ) "<command line>" ;
        byte signalAscii [ 128 ] ;
        int64 iw = false ;
        Word * word = debugger->w_Word ? debugger->w_Word : ((!debugger->Token)? (_Context_->CurrentlyRunningWord ? _Context_->CurrentlyRunningWord : _Context_->CurrentTokenWord) : 0)  ;
        byte wordName [256], aliasName [256], * token0 ;
        if ( word && ( iw = String_Equal ( "init", word->Name ) ) ) { debugger->SubstitutedWord = word ; word = cntx->Interpreter0->w_Word ; } // 'new'
        debugger->w_AliasOf = debugger->SubstitutedWord ? debugger->SubstitutedWord : debugger->w_AliasOf ;
        if ( debugger->w_AliasOf ) //= Word_UnAlias ( word ) )
        {
            if ( iw ) snprintf ( aliasName, 255, "%s.%s", 
                (debugger->w_AliasOf->S_ContainingNamespace ? debugger->w_AliasOf->S_ContainingNamespace->Name : (byte*) ""), debugger->w_AliasOf->Name) ;
            snprintf ( wordName, 255, "%s%s%s%s", word ? word->Name : ( byte* ) "", ( ( char* ) debugger->w_AliasOf ? " -> " : "" ),
                iw ? aliasName : ( debugger->w_AliasOf ? debugger->w_AliasOf->Name : ( byte* ) "" ), debugger->w_AliasOf ? ( byte* ) " " : (byte*)"" ) ;
            debugger->SubstitutedWord = 0 ;
            SetState ( debugger, DBG_OUTPUT_SUBSTITUTION, true ) ;
            token0 = wordName ;
        }
        else token0 = word ? word->Name : debugger->Token ;
        //if ( debugger->w_Word == cntx->LastEvalWord ) word = 0, debugger->w_Word = 0, token0 = cntx->CurrentToken ;

        if ( ! ( cntx && cntx->Lexer0 ) ) Throw ( ( byte* ) "\n_CSL_ShowInfo:", ( byte* ) "\nNo token at _CSL_ShowInfo\n", QUIT ) ;
        if ( ( signal == 11 ) || _O_->SigAddress )
        {
            snprintf ( ( char* ) signalAscii, 127, ( char * ) "Error : signal " INT_FRMT ":: attempting address : \n" UINT_FRMT, signal, ( uint64 ) _O_->SigAddress ) ;
            debugger->DebugAddress = ( byte* ) _O_->SigAddress ;
        }
        else if ( signal ) snprintf ( ( char* ) signalAscii, 127, ( char * ) "Error : signal " INT_FRMT " ", signal ) ;
        else signalAscii[0] = 0 ;

        DebugColors ;

        if ( ( location == debugger->Filename ) && ( GetState ( debugger, DBG_FILENAME_LOCATION_SHOWN ) ) ) location = ( byte * ) "..." ;
        SetState ( debugger, DBG_FILENAME_LOCATION_SHOWN, true ) ;

        if ( token0 ) Debugger_ShowInfo_Token (debugger, word, prompt, signal, token0, signalAscii ) ;
        else
        {
            byte *cc_line = ( char* ) Buffer_Data ( _CSL_->DebugB ) ;
            strcpy ( cc_line, ( char* ) rl->InputLine ) ;
            String_RemoveEndWhitespace ( ( byte* ) cc_line ) ;
            Printf ( ( byte* ) "\n%s %s:: %s : %03d.%03d :> %s", // <:: " INT_FRMT "." INT_FRMT,
                prompt, signal ? signalAscii : ( byte* ) "", location, rl->LineNumber, rl->ReadIndex,
                cc_line ) ;
        }
        DefaultColors ;
        if ( ! String_Equal ( "...", location ) ) debugger->Filename = location ;
        debugger->LastShowInfoWord = word ;
    }
    else SetState_TrueFalse ( _Debugger_, DBG_AUTO_MODE_ONCE, true ) ;
    SetState ( _Debugger_, ( DBG_OUTPUT_SUBSTITUTION ), false ) ;
}

void
Debugger_ShowInfo ( Debugger * debugger, byte * prompt, int64 signal )
{
    Context * cntx = _Context_ ;
    int64 sif = 0 ;
    if ( ( GetState ( debugger, DBG_INFO ) ) || GetState ( debugger, DBG_STEPPING ) )
    {
        _Debugger_ShowInfo ( debugger, prompt, signal, 1 ) ;
        sif = 1 ;
    }
    if ( ! ( cntx && cntx->Lexer0 ) )
    {
        Printf ( ( byte* ) "\nSignal Error : signal = %d\n", signal ) ;
        return ;
    }
    if ( ! GetState ( _Debugger_, DBG_ACTIVE ) )
    {
        debugger->Token = cntx->Lexer0->OriginalToken ;
        Debugger_FindUsing ( debugger ) ;
    }
    else if ( debugger->w_Word ) debugger->Token = debugger->w_Word->Name ;
    if ( ( _O_->SigSegvs < 2 ) && GetState ( debugger, DBG_STEPPING ) )
    {
        Printf ( ( byte* ) "\nDebug Stepping Address : 0x%016lx", ( uint64 ) debugger->DebugAddress ) ;
        Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "", ( byte* ) "" ) ; // the next instruction
    }
    if ( ( ! sif ) && ( ! GetState ( debugger, DBG_STEPPING ) ) && ( GetState ( debugger, DBG_INFO ) ) ) _Debugger_ShowInfo ( debugger, prompt, signal, 1 ) ;
    if ( prompt == _O_->ExceptionMessage ) _O_->ExceptionMessage = 0 ;
}

void
CSL_ShowInfo ( Word * word, byte * prompt, int64 signal )
{
    _Debugger_->w_Word = Word_UnAlias ( word ) ;
    _Debugger_ShowInfo ( _Debugger_, prompt, signal, 1 ) ;
}

void
Debugger_ShowState ( Debugger * debugger, byte * prompt )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    Word * word = debugger->w_Word ;
    int64 cflag = 0 ;
    if ( word && ( word->W_ObjectAttributes & CONSTANT ) ) cflag = 1 ;
    DebugColors ;
    byte * token = debugger->Token ;
    token = String_ConvertToBackSlash ( token ) ;
    if ( word )
    {
        Printf ( ( byte* ) ( cflag ? "\n%s :: %03d.%03d : %s : <constant> : %s%s%s " : word->ContainingNamespace ? "\n%s :: %03d.%03d : %s : <word> : %s%s%s " : "\n%s :: %03d.%03d : %s : <word?> : %s%s%s " ),
            prompt, rl->LineNumber, rl->ReadIndex, Debugger_GetStateString ( debugger ),
            // _O_->CSL->Namespaces doesn't have a ContainingNamespace
            word->ContainingNamespace ? word->ContainingNamespace->Name : ( byte* ) "",
            word->ContainingNamespace ? ( byte* ) "." : ( byte* ) "", // the dot between
            c_gd ( word->Name ) ) ;
    }
    else if ( token )
    {
        Printf ( ( byte* ) ( cflag ? "\n%s :: %03d.%03d : %s : <constant> :> %s " : "\n%s :: %03d.%03d : %s : <literal> :> %s " ),
            prompt, rl->LineNumber, rl->ReadIndex, Debugger_GetStateString ( debugger ), c_gd ( token ) ) ;
    }
    else Printf ( ( byte* ) "\n%s :: %03d.%03d : %s : ", prompt, rl->LineNumber, rl->ReadIndex, Debugger_GetStateString ( debugger ) ) ;
    if ( ! debugger->Key )
    {
        if ( word ) _CSL_Source ( word, 0 ) ;

        if ( GetState ( debugger, DBG_STEPPING ) )
            Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\r", ( byte* ) "" ) ; // current insn
    }
}

void
_Debugger_DoNewlinePrompt ( Debugger * debugger )
{
    Printf ( ( byte* ) "\n%s=> ", GetState ( debugger, DBG_RUNTIME ) ? ( byte* ) "<dbg>" : ( byte* ) "dbg" ) ; //, (char*) ReadLine_GetPrompt ( _Context_->ReadLiner0 ) ) ;
    Debugger_SetNewLine ( debugger, false ) ;
}

void
Debugger_DoState ( Debugger * debugger )
{
    if ( GetState ( debugger, DBG_RETURN ) )
    {
        Printf ( ( byte* ) "\r" ) ;
        SetState ( debugger, DBG_RETURN, false ) ;
    }
    if ( GetState ( debugger, DBG_MENU ) )
    {
        Debugger_Menu ( debugger ) ;
        SetState ( debugger, DBG_FILENAME_LOCATION_SHOWN, false ) ;
    }
    if ( GetState ( debugger, DBG_INFO ) ) Debugger_ShowInfo ( debugger, GetState ( debugger, DBG_RUNTIME ) ? ( byte* ) "<dbg>" : ( byte* ) "dbg", 0 ) ;
    else if ( GetState ( debugger, DBG_PROMPT ) ) Debugger_ShowState ( debugger, GetState ( debugger, DBG_RUNTIME ) ? ( byte* ) "<dbg>" : ( byte* ) "dbg" ) ;
    else if ( GetState ( debugger, DBG_STEPPING | DBG_CONTINUE_MODE ) ) 
    {
        if ( GetState ( debugger, DBG_START_STEPPING ) ) Printf ( ( byte* ) "\n ... Next stepping instruction ..." ) ;
        SetState ( debugger, DBG_START_STEPPING, false ) ;
        debugger->cs_Cpu->Rip = ( uint64 * ) debugger->DebugAddress ;
        Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\r", ( byte* ) "" ) ;
    }
}

int64
Debugger_TerminalLineWidth ( Debugger * debugger )
{
    int64 tw = GetTerminalWidth ( ) ;
    if ( tw > debugger->TerminalLineWidth ) debugger->TerminalLineWidth = tw ;
    return debugger->TerminalLineWidth ;
}

void
Debugger_ShowStackChange ( Debugger * debugger, Word * word, byte * insert, byte * achange, Boolean stepFlag )
{
    int64 sl, i = 0 ;
    char *name, *location, *b, *b2 = ( char* ) Buffer_Data_Cleared ( _CSL_->DebugB2 ) ;
    b = ( char* ) Buffer_Data_Cleared ( _CSL_->DebugB1 ) ;
    if ( stepFlag ) sprintf ( ( char* ) b2, "0x%016lx", ( uint64 ) debugger->DebugAddress ) ;
    location = stepFlag ? ( char* ) c_gd ( b2 ) : ( char* ) Context_Location ( ) ;
    name = word ? ( char* ) c_gd ( String_ConvertToBackSlash ( word->Name ) ) : ( char* ) "" ;
    while ( 1 )
    {
        if ( GetState ( debugger, DBG_STEPPING ) )
            snprintf ( ( char* ) b, BUFFER_IX_SIZE, "\nStack : %s at %s :> %s <: %s", insert, location, ( char* ) c_gd ( name ), ( char* ) c_gd (achange) ) ;
        else snprintf ( ( char* ) b, BUFFER_IX_SIZE, "\nStack : %s at %s :> %s <: %s", insert, ( char* ) location, name, ( char* ) c_gd (achange) ) ;
        if ( ( sl = strlen ( ( char* ) b ) ) > GetTerminalWidth ( ) ) //220 ) //183 ) //GetTerminalWidth ( ) ) //_Debugger_->TerminalLineWidth ) //220 ) 
        {
            location = ( char* ) "..." ;
            if ( ++ i > 1 ) name = ( char* ) "" ;
            if ( i > 2 ) insert = ( byte* ) "" ;
            if ( i > 3 ) achange = ( byte* ) "" ;
            if ( i > 4 ) break ;
        }
        else break ;
    }
    Printf ( ( byte* ) "%s", b ) ;
    if ( _O_->Verbosity > 3 ) _Debugger_PrintDataStack ( 2 ) ;
}

void
Debugger_ShowChange ( Debugger * debugger, Word * word, Boolean stepFlag, uint64* dsp )
{
    const char * insert ;
    uint64 change ;
    int64 depthChange ;
    if ( Debugger_IsStepping ( debugger ) )
    {
        change = dsp - debugger->SaveDsp ;
        debugger->SaveDsp = dsp ;
    }
    else
    {
        change = debugger->WordDsp ? ( dsp - debugger->WordDsp ) : 0 ;
        //debugger->WordDsp = dsp ;
    }
    depthChange = DataStack_Depth ( ) - debugger->SaveStackDepth ;
    if ( word && ( debugger->WordDsp && ( GetState ( debugger, DBG_SHOW_STACK_CHANGE ) ) || ( change ) || ( debugger->SaveTOS != TOS ) || ( depthChange ) ) )
    {
        byte * name, pb_change [ 256 ] ;
        char * b = ( char* ) Buffer_Data ( _CSL_->DebugB ), *op ;
        char * c = ( char* ) Buffer_Data ( _CSL_->DebugB2 ) ;
        pb_change [ 0 ] = 0 ;

        if ( GetState ( debugger, DBG_SHOW_STACK_CHANGE ) ) SetState ( debugger, DBG_SHOW_STACK_CHANGE, false ) ;
        if ( depthChange > 0 ) snprintf ( ( char* ) pb_change, 256, "%ld %s%s", depthChange, ( depthChange > 1 ) ? "cells" : "cell", " pushed. " ) ;
        else if ( depthChange ) snprintf ( ( char* ) pb_change, 256, "%ld %s%s", - depthChange, ( depthChange < - 1 ) ? "cells" : "cell", " popped. " ) ;
        if ( dsp && ( debugger->SaveTOS != TOS ) ) op = ( char* ) "changed" ;
        else op = ( char* ) "set" ;
        snprintf ( ( char* ) c, BUFFER_IX_SIZE, ( char* ) "0x%016lx", ( uint64 ) TOS ) ;
        snprintf ( ( char* ) b, BUFFER_IX_SIZE, ( char* ) "TOS %s to %s.", op, c_gd ( c ) ) ;
        strncat ( ( char* ) pb_change, ( char* ) b, 256 ) ; // strcat ( (char*) _change, cc ( ( char* ) c, &_O_->Default ) ) ;
        name = word->Name ;
        if ( name ) name = String_ConvertToBackSlash ( name ) ;
        char * achange = ( char* ) pb_change ;
        if ( stepFlag )
        {
            Word * word = Word_GetFromCodeAddress ( debugger->DebugAddress ) ;
            if ( ( word ) && ( ( byte* ) word->Definition == debugger->DebugAddress ) )
            {
                insert = "function call" ;
                if ( achange [0] ) Debugger_ShowStackChange ( debugger, word, ( byte* ) insert, ( byte* ) achange, stepFlag ) ;
            }
        }
        else
        {
            if ( word ) insert = "word" ;
            else insert = "token" ;
            if ( achange [0] ) Debugger_ShowStackChange ( debugger, word, ( byte* ) insert, ( byte* ) achange, stepFlag ) ;
        }
        if ( GetState ( _Context_->Lexer0, KNOWN_OBJECT ) )
        {
            if ( dsp > debugger->SaveDsp ) Printf ( ( byte* ) "\nLiteral :> 0x%016lx <: was pushed onto the stack ...", TOS ) ;
            else if ( dsp < debugger->SaveDsp ) Printf ( ( byte* ) "\n%s popped %d value off the stack.", insert, ( debugger->SaveDsp - dsp ) ) ;
            DefaultColors ;
        }
        //if ( ( ! ( achange [0] ) ) && ( ( change > 1 ) || ( change < - 1 ) || ( _O_->Verbosity > 1 ) ) ) _Debugger_PrintDataStack ( change + 1 ) ;
        if ( ( ! achange [0] ) && ( change || ( _O_->Verbosity > 1 ) ) ) _Debugger_PrintDataStack ( change + 1 ) ;
    }
}

byte *
String_HighlightTokenInputLine ( byte * nvw, int64 lef, int64 leftBorder, int64 ts, byte * token, byte * token0, int64 rightBorder, int64 ref )
{
    int64 svState = GetState ( _Debugger_, DEBUG_SHTL_OFF ) ;
    SetState ( _Debugger_, DEBUG_SHTL_OFF, false ) ;
    // |ilw...------ inputLine  -----|lef|--- leftBorder ---|---token---|---  rightBorder  ---|ref|------ inputLine -----...ilw| -- ilw : inputLine window
    // |ilw...------ inputLine  -----|lef|pad?|-------------|tp|---token---|---  rightBorder  ---|ref|------ inputLine -----...ilw| -- ilw : inputLine window
    //_String_HighlightTokenInputLine ( byte * nvw, int8 lef, int64 leftBorder, int64 tokenStart, byte *token, int64 rightBorder, int8 ref )
    //cc_line = _String_HighlightTokenInputLine ( nvw, lef, leftBorder, ts, token, rightBorder, ref ) ; // nts : new token start is a index into b - the nwv buffer
    byte * cc_line = _String_HighlightTokenInputLine ( nvw, lef, leftBorder, ts, token, token0, rightBorder, ref ) ; // nts : new token start is a index into b - the nwv buffer
    SetState ( _Debugger_, DEBUG_SHTL_OFF, svState ) ;
    return cc_line ;
}

byte *
PSCS_Using_WordSC (byte* scs, byte * token, int64 scswci ) // scs : source code string
{
    byte *nvw ;
    // ts : tokenStart ; tp : text point - where we want to start source code text to align with disassembly ; ref : right ellipsis flag
    int64 i, tw, slt, tp, lef, leftBorder, ts, rightBorder, ref, slsc, pad ;
    slsc = strlen ( ( char* ) scs ) ;
    nvw = Buffer_Data_Cleared ( _CSL_->DebugB3 ) ; //Buffer_New_pbyte ( ( slsc > BUFFER_SIZE ) ? slsc : BUFFER_SIZE ) ;
    slt = Strlen ( token ) ;
    tw = Debugger_TerminalLineWidth ( _Debugger_ ) ;
    tp = 42 ; // 42 : aligned with disassembler code
    if ( ( slsc > tp ) && ( scswci > tp ) )
    {
        lef = 4 ;
        leftBorder = ts = tp ;
        rightBorder = tw - ( ts + slt ) ;
        ref = ( slsc - 4 ) > tw ? 4 : 0 ;
        strncpy ( nvw, & scs [scswci - tp], tw - ( lef + ref ) ) ;
    }
    else
    {
        pad = tp - scswci ;
        if ( pad >= 4 ) lef = 4 ;
        else lef = 0 ;
        for ( i = 0 ; i < pad ; i ++ ) strncat ( ( char* ) nvw, " ", BUFFER_IX_SIZE ) ;
        leftBorder = ts = tp ;
        ref = ( slsc - 4 ) > tw ? 4 : 0 ;
        if ( ( ! ref ) && ( tw > slsc - 4 ) ) ref = 4 ;
        rightBorder = tw - ( tp + slt ) - ref ;
        strncat ( nvw, scs, tw - ( lef + pad + ref ) ) ; // must Strncat because we might have done a strcat above based on the 'pad' variable
    }
    return String_HighlightTokenInputLine ( nvw, lef, leftBorder, ts, token, 0, rightBorder, ref ) ;
}

byte *
PSCS_Using_ReadlinerInputString ( byte* il, byte * token1, byte* token0, int64 scswci, int64 tvw ) // scs : source code string ; tvw : text view sliding window 
{
    byte *nvw ;
    // ts : tokenStart ; tp : text point - where we want to start source code text to align with disassembly ; ref : right ellipsis flag
    int64 slt0, slt1, lef, leftBorder, ts, rightBorder, ref, slil ;
    int64 totalBorder, idealBorder, nws, nts, slNvw, lbm ;
    slil = strlen ( ( char* ) il ) ;
    nvw = Buffer_New_pbyte ( ( slil > BUFFER_SIZE ) ? slil : BUFFER_SIZE ) ;
    slt0 = strlen ( token0 ) ;
    slt1 = Strlen ( token1 ) ;
    totalBorder = ( tvw - slt1 ) ; // the borders allow us to slide token within the window of tvw
    idealBorder = ( totalBorder / 2 ) ;
    leftBorder = idealBorder ; // tentatively set leftBorder/rightBorder as ideally equal
    rightBorder = idealBorder ; // tentatively set leftBorder/rightBorder as ideally equal
    nws = scswci - idealBorder ;
    nts = idealBorder ;
    if ( nws < 0 )
    {
        nws = 0 ;
        nts = leftBorder = scswci ;
        rightBorder = totalBorder - leftBorder ;
    }
    else if ( (lbm = slil - ( scswci + slt1 + idealBorder )) > 0 )
    {
        nws = slil - tvw ;
        rightBorder = slil - ( scswci + slt1 ) ; // nb! : try to keep all on right beyond token - the cutoff should be on the left side
        if ( nws < 0 )
        {
            nws = 0 ;
            rightBorder += ( tvw - slil ) ;
        }
        leftBorder = totalBorder - rightBorder ;
        nts = leftBorder ; //+ (ins ? (slt/2 + 1) : 0 ) ;
    }
    //else { use the defaults above }
#if 1   
    if ( GetState ( _Debugger_, ( DBG_OUTPUT_SUBSTITUTION ) ) )
    {
        Strncpy ( nvw, &il[nws], leftBorder ) ; //scswci ) ; // tvw ) ; // copy the the new view window to buffer nvw
        Strncat ( nvw, token1, slt1 ) ; // tvw ) ; // copy the the new view window to buffer nvw
        //Strncat ( nvw, &il[nws + slt0], tvw ) ; // tvw ) ; // copy the the new view window to buffer nvw
        Strncat ( nvw, &il[nws + leftBorder + slt0], rightBorder ) ; // - slt1 ) ; // tvw ) ; // copy the the new view window to buffer nvw
    }
    else
#endif    
        Strncpy ( nvw, &il[nws], tvw ) ; // copy the the new view window to buffer nvw
    slNvw = Strlen ( nvw ) ;
    if ( slNvw > ( tvw + 8 ) ) // is there a need for ellipsis
    {
        if ( ( scswci - leftBorder ) < 4 ) lef = 0, ref = 1 ;
        else lef = ref = 1 ;
    }
    else if ( slNvw > ( tvw + 4 ) ) // is there a need for one ellipsis
    {
        if ( ( scswci - leftBorder ) < 4 ) lef = 0, ref = 1 ;
        else lef = 1, ref = 0 ; // choose lef as preferable
    }
    else lef = ref = 0 ;
    ts = nts ;
    return String_HighlightTokenInputLine ( nvw, lef, leftBorder, ts, token1, token0, rightBorder, ref ) ;
}
// ...source code source code TP source code source code ... EOL

byte *
DBG_PrepareSourceCodeString ( Word * word, byte* token0, byte* il, int tvw, int rlIndex, Boolean useScFlag ) // otoken : original token; il : instruction line ; tvw text view window
{
    // usingSC == 1 denotes il string is from word->W_SourceCode else il is copied from rl->InputLineString
    Debugger * debugger = _Debugger_ ;
    byte * cc_line = ( byte* ) "" ;
    if ( ( word || token0 ) && il )
    {
        byte *token = ( word ? word->Name : token0 ), * token1, *token2 ;
        int64 scswi0, slt, slsc, scswci ;
        if ( GetState ( debugger, DBG_OUTPUT_INSERTION | DBG_OUTPUT_SUBSTITUTION ) )
        {
            token1 = debugger->SubstitutedWord ? debugger->SubstitutedWord->Name : token0 ? token0 : word->Name ;
            //subsToken = debugger->SubstitutedWord->Name ;
            //if ( String_Equal ( token0, token ) ) sub_ins = 0 ;
        }
        else token1 = token0 ;
        token2 = String_ConvertToBackSlash ( token1 ) ;
        slt = Strlen ( token2 ) ;
        slsc = strlen ( ( char* ) il ) ;
        scswi0 = useScFlag ? word->W_SC_Index : rlIndex ? rlIndex : 0 ; //word->W_SC_Index ;
        scswci = String_FindStrnCmpIndex ( il, token2, scswi0, slt, slt ) ;
        if ( scswci != - 1 ) // did we find token in scs
        {
            // these two functions can probably be integrated
            if ( useScFlag ) cc_line = PSCS_Using_WordSC (il, token2, scswci ) ;
            else cc_line = PSCS_Using_ReadlinerInputString ( il, token2, token, scswci, tvw ) ; // scs : source code string
        }
    }
    return cc_line ;
}


// NB!! : remember the highlighting formatting characters don't add any additional *visible length* to the output line
// ots : original token start (index into the source code), nws : new window start ; tvw: targetViewWidth ; nts : new token start
// lef : left ellipsis flag, ref : right ellipsis flag

byte *
Debugger_PrepareDbgSourceCodeString ( Debugger * debugger, Word * word, byte* token, int64 twAlreayUsed )
{
    byte * cc_line = ( byte* ) "" ;
    if ( word || token )
    {
        ReadLiner * rl = _Context_->ReadLiner0 ;
        byte * il = Buffer_Data_Cleared ( _CSL_->StringInsertB4 ) ; //nb! dont tamper with the input line. eg. removing its newline will affect other code which depends on newline
        strncpy ( il, rl->InputLineString, BUFFER_IX_SIZE ) ;
        String_RemoveEndWhitespace ( ( byte* ) il ) ;
        int64 fel, tw, tvw ;
        fel = 32 - 1 ; //fe : formatingEstimate length : 2 formats with 8/12 chars on each sude - 32/48 :: 1 : a litte leave way
        tw = Debugger_TerminalLineWidth ( debugger ) ; // 139 ; //139 : nice width :: Debugger_TerminalLineWidth ( debugger ) ; 
        tvw = tw - ( ( twAlreayUsed > fel ) ? ( twAlreayUsed - fel ) : fel ) ; //subtract the formatting chars which don't add to visible length
        cc_line = DBG_PrepareSourceCodeString ( word, token, il, tvw, word ? word->W_RL_Index : _Lexer_->TokenStart_ReadLineIndex, 0 ) ; //tvw, tvw/2 ) ;// sc : source code ; scwi : source code word index
    }
    return cc_line ;
}

void
Debugger_ShowInfo_Token (Debugger * debugger, Word * word, byte * prompt, int64 signal, byte * token0, byte * signalAscii )
{
    ReadLiner * rl = _ReadLiner_ ;
    char * compileOrInterpret = ( char* ) ( ( signal || ( int64 ) signalAscii[0] ) ?
        ( CompileMode ? "\n[c] " : "\n[i] " ) : ( CompileMode ? "[c] " : "[i] " ) ) ;
    byte * buffer = Buffer_Data_Cleared ( _CSL_->ScratchB2 ) ;
    byte * obuffer = Buffer_Data_Cleared ( _CSL_->DebugB1 ) ;
    byte * token1 = String_ConvertToBackSlash ( token0 ) ;
    char * cc_Token = ( char* ) cc ( token1, &_O_->Notice ) ;
    char * cc_location = ( char* ) cc ( Context_Location (), &_O_->Debug ) ;

    prompt = prompt ? prompt : ( byte* ) "" ;
    strncpy ( buffer, prompt, BUFFER_IX_SIZE ) ;
    strncat ( buffer, compileOrInterpret, 32 ) ;
    prompt = ( byte* ) buffer ;
    if ( word )
    {
        if ( word->W_MorphismAttributes & CPRIMITIVE )
        {
            snprintf ( ( char* ) obuffer, BUFFER_IX_SIZE, "\n%s%s:: %s : %03ld.%03ld : %s :> %s <: cprimitive :> ", // <:: " INT_FRMT "." INT_FRMT " ",
                prompt, signal ? ( char* ) signalAscii : " ", cc_location, rl->LineNumber, rl->ReadIndex,
                word->ContainingNamespace ? ( char* ) word->ContainingNamespace->Name : "<literal>",
                cc_Token ) ;
        }
        else
        {
            snprintf ( ( char* ) obuffer, BUFFER_IX_SIZE, "\n%s%s:: %s : %03ld.%03ld : %s :> %s <: 0x%016lx :> ", // <:: " INT_FRMT "." INT_FRMT " ",
                prompt, signal ? ( char* ) signalAscii : " ", cc_location, rl->LineNumber, rl->ReadIndex,
                word->ContainingNamespace ? ( char* ) word->ContainingNamespace->Name : ( char* ) "<literal>",
                ( char* ) cc_Token, ( uint64 ) word ) ;
        }
        //fflush ( stdout ) ;
    }
    else
    {
        snprintf ( ( char* ) obuffer, BUFFER_IX_SIZE, "\n%s%s:: %s : %03ld.%03ld : %s :> %s <::> ", // <:: " INT_FRMT "." INT_FRMT " ",
            prompt, signal ? signalAscii : ( byte* ) " ", cc_location, rl->LineNumber, rl->ReadIndex,
            "<literal>", cc_Token ) ; //, _O_->StartedTimes, _O_->SignalExceptionsHandled ) ;
    }
    byte *cc_line = ( char* ) Debugger_PrepareDbgSourceCodeString ( debugger, word, token1, ( int64 ) Strlen ( obuffer ) ) ;
    if ( cc_line ) strncat ( obuffer, cc_line, BUFFER_IX_SIZE ) ;
    _Printf ( ( byte* ) "%s", obuffer ) ;
}

void
LO_Debug_ExtraShow ( int64 showStackFlag, int64 verbosity, int64 wordList, byte *format, ... )
{
    if ( GetState ( _CSL_, DEBUG_MODE ) )
    {
        if ( _O_->Verbosity > verbosity )
        {
            va_list args ;
            va_start ( args, ( char* ) format ) ;
            char * out = ( char* ) Buffer_Data ( _CSL_->DebugB ) ;
            vsprintf ( ( char* ) out, ( char* ) format, args ) ;
            va_end ( args ) ;
            DebugColors ;
            if ( wordList ) _CSL_SC_WordList_Show ( ( byte* ) out, 0, 0 ) ;
            else
            {
                printf ( "%s", out ) ;
                fflush ( stdout ) ;
            }
            if ( showStackFlag && _O_->Verbosity > verbosity ) Stack ( ) ;
            DefaultColors ;
        }
    }
}

