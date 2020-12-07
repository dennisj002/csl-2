#include "../../include/csl.h"

// we don't want the classes semantics when interpreting

void
CSL_ClassStructureEnd ( void )
{
    Namespace_RemoveFromUsingList_WithCheck ( ( byte* ) "Class" ) ;
}

void
Class_Size_Set ( Namespace * classNs, int64 size )
{
    OVT_Assert ( classNs > 0, (byte * )"Class_Size_Set : No classNs." ) ;
    if ( classNs )
    {
        _Namespace_VariableValueSet ( classNs, ( byte* ) "size", size ) ;
        classNs->ObjectByteSize = size ;
    }
}

void
_ClassTypedef ( Context * cntx, Namespace * ns, Boolean cloneFlag )
{
    //CSL_C_Syntax_On ( ) ;
    TDSCI * tdsci = TDSCI_Start (ns, 0, ( cloneFlag ? TDSCI_CLONE_FLAG : 0 ) ) ;
    Parse_StructUnionDef () ;
    tdsci = TDSCI_Finalize () ;
}

void
CSL_CloneStructureBegin ( void )
{
    _ClassTypedef ( _Context_, 0, 1 ) ;
}

void
CSL_ClassTypedef ( )
{
    _ClassTypedef ( _Context_, 0, 0 ) ;
}
