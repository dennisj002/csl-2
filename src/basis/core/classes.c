#include "../../include/csl.h"

// we don't want the classes semantics when interpreting

void
CSL_ClassStructureEnd ( void )
{
    Namespace_RemoveFromUsingList_WithCheck ( ( byte* ) "Class" ) ;
}

void
CSL_CloneStructureBegin ( void )
{
    TypeDefStructCompileInfo * tdsci = _Compiler_->C_Tdsci = TypeDefStructCompileInfo_New ( ) ;
    SetState ( tdsci, TDSCI_CLONE_FLAG, true ) ;
    Parse_Structure ( tdsci ) ;
    //Parse_A_Typedef_Field ( tdsci ) ; 
}

void
Class_Size_Set ( Namespace * classNs, int64 size )
{
    _Namespace_VariableValueSet ( classNs, ( byte* ) "size", size ) ;
    classNs->ObjectByteSize = size ;
}

void
_ClassTypedef ( Boolean cloneFlag )
{
    Namespace * classNs ;
    TypeDefStructCompileInfo * tdsci = _Compiler_->C_Tdsci = TypeDefStructCompileInfo_New ( ) ;
    tdsci->Tdsci_StructureUnion_Namespace = classNs = tdsci->BackgroundNamespace ;
    Parse_StructOrUnion_Type ( tdsci, TDSCI_STRUCT|(cloneFlag ? TDSCI_CLONE_FLAG : 0) ) ;
    Class_Size_Set ( classNs, tdsci->Tdsci_TotalSize ) ;
    classNs->W_ObjectAttributes |= STRUCTURE ;    
    SetState ( _Compiler_, TDSCI_PARSING, true ) ;
}

void
CSL_ClassTypedef ( )
{
    CSL_PushToken_OnTokenList ( "{" ) ;
    _ClassTypedef ( 0 ) ;
}
