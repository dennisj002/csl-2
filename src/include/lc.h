#define LO_Last( lo ) (ListObject*) dllist_Last ( (dllist*) lo->Lo_List )
#define LO_Previous( lo ) ( ListObject* ) dlnode_Previous ( ( dlnode* ) lo )
#define LO_Next( lo ) ( ListObject* ) dlnode_Next ( ( dlnode* ) lo )
#define LO_AddToTail( lo, lo1 ) dllist_AddNodeToTail ( (( ListObject * ) lo)->Lo_List, ( dlnode* ) (lo1) ) 
#define LO_AddToHead( lo, lo1 ) dllist_AddNodeToHead ( (( ListObject * ) lo)->Lo_List, ( dlnode* ) (lo1) ) 
#define LO_New( lType, object ) (ListObject *) DataObject_New (T_LC_NEW, 0, 0, 0, 0, lType, 0, (int64) object, 0, 0, 0 , -1)
#define LambdaArgs( proc ) proc->p[0]
#define LambdaProcedureBody( proc ) proc->p[1]
#define LambdaVals( proc ) proc->p[2]
#define LISP_ALLOC (_LC_ ? (GetState (_LC_, (LC_DEFINE_MODE|LC_READ)) ? LISP : LISP_TEMP) : LISP_TEMP)
#define LO_CopyOne( l0 ) _LO_CopyOne ( l0, LISP_ALLOC )
#define LO_Copy( l0 ) _LO_Copy ( l0, LISP_ALLOC )
#define nil (_LC_ ? _LC_->Nil : 0)
#define LC_SaveStackPointer( lc ) _LC_SaveDsp ( lc )
#define LC_RestoreStackPointer( lc ) _LC_ResetStack ( lc ) 

#define LC_snprintf2( buffer, format, value1, value2 ) snprintf ( ( char* ) buffer, BUFFER_IX_SIZE, ((char*) (format)), value1, value2 ) 
#define LC_snprintf1( buffer, format, value ) snprintf ( ( char* ) buffer, BUFFER_IX_SIZE, ((char*) (format)), value ) 
#define LC_sprintf_String( buffer, format, str ) LC_snprintf1 ( buffer, (format), ((char*) str) ) 
#define LC_sprintString( buffer, str ) LC_sprintf_String (buffer, " %s", (str) ) 
#define LC_sprintAString( buffer, str ) LC_sprintString (buffer, (str) ) 
#define LC_sprintName( buffer, l0 ) LC_sprintString ( buffer, l0->Lo_Name ) 
