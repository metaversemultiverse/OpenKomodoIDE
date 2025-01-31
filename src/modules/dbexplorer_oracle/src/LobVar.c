//-----------------------------------------------------------------------------
// LobVar.c
//   Defines the routines for handling LOB variables.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// LOB type
//-----------------------------------------------------------------------------
typedef struct {
    Variable_HEAD
    OCILobLocator **data;
    udt_Connection *connection;
    int isFile;
} udt_LobVar;

//-----------------------------------------------------------------------------
// Declaration of LOB variable functions.
//-----------------------------------------------------------------------------
static int LobVar_Initialize(udt_LobVar*, udt_Cursor*);
static void LobVar_Finalize(udt_LobVar*);
static PyObject *LobVar_GetValue(udt_LobVar*, unsigned);
static int LobVar_SetValue(udt_LobVar*, unsigned, PyObject*);
static int LobVar_Write(udt_LobVar*, unsigned, PyObject*, ub4, ub4*);

//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
static PyTypeObject g_CLOBVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.CLOB",                   // tp_name
    sizeof(udt_LobVar),                 // tp_basicsize
    0,                                  // tp_itemsize
    0,                                  // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    0,                                  // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0                                   // tp_doc
};


static PyTypeObject g_NCLOBVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.NCLOB",                  // tp_name
    sizeof(udt_LobVar),                 // tp_basicsize
    0,                                  // tp_itemsize
    0,                                  // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    0,                                  // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0                                   // tp_doc
};


static PyTypeObject g_BLOBVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.BLOB",                   // tp_name
    sizeof(udt_LobVar),                 // tp_basicsize
    0,                                  // tp_itemsize
    0,                                  // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    0,                                  // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0                                   // tp_doc
};


static PyTypeObject g_BFILEVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.BFILE",                  // tp_name
    sizeof(udt_LobVar),                 // tp_basicsize
    0,                                  // tp_itemsize
    0,                                  // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    0,                                  // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0                                   // tp_doc
};


//-----------------------------------------------------------------------------
// variable type declarations
//-----------------------------------------------------------------------------
static udt_VariableType vt_CLOB = {
    (InitializeProc) LobVar_Initialize,
    (FinalizeProc) LobVar_Finalize,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) LobVar_SetValue,
    (GetValueProc) LobVar_GetValue,
    (GetBufferSizeProc) NULL,
    &g_CLOBVarType,                     // Python type
    SQLT_CLOB,                          // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    sizeof(OCILobLocator*),             // element length
    1,                                  // is character data
    0,                                  // is variable length
    0,                                  // can be copied
    0                                   // can be in array
};


static udt_VariableType vt_NCLOB = {
    (InitializeProc) LobVar_Initialize,
    (FinalizeProc) LobVar_Finalize,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) LobVar_SetValue,
    (GetValueProc) LobVar_GetValue,
    (GetBufferSizeProc) NULL,
    &g_NCLOBVarType,                    // Python type
    SQLT_CLOB,                          // Oracle type
    SQLCS_NCHAR,                        // charset form
    sizeof(OCILobLocator*),             // element length
    1,                                  // is character data
    0,                                  // is variable length
    0,                                  // can be copied
    0                                   // can be in array
};


static udt_VariableType vt_BLOB = {
    (InitializeProc) LobVar_Initialize,
    (FinalizeProc) LobVar_Finalize,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) LobVar_SetValue,
    (GetValueProc) LobVar_GetValue,
    (GetBufferSizeProc) NULL,
    &g_BLOBVarType,                     // Python type
    SQLT_BLOB,                          // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    sizeof(OCILobLocator*),             // element length
    0,                                  // is character data
    0,                                  // is variable length
    0,                                  // can be copied
    0                                   // can be in array
};


static udt_VariableType vt_BFILE = {
    (InitializeProc) LobVar_Initialize,
    (FinalizeProc) LobVar_Finalize,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) LobVar_SetValue,
    (GetValueProc) LobVar_GetValue,
    (GetBufferSizeProc) NULL,
    &g_BFILEVarType,                    // Python type
    SQLT_BFILE,                         // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    sizeof(OCILobLocator*),             // element length
    0,                                  // is character data
    0,                                  // is variable length
    0,                                  // can be copied
    0                                   // can be in array
};


#include "ExternalLobVar.c"


//-----------------------------------------------------------------------------
// LobVar_Initialize()
//   Initialize the variable.
//-----------------------------------------------------------------------------
static int LobVar_Initialize(
    udt_LobVar *var,                    // variable to initialize
    udt_Cursor *cursor)                 // cursor created by
{
    sword status;
    ub4 i;

    // initialize members
    Py_INCREF(cursor->connection);
    var->connection = cursor->connection;
    var->isFile = (var->type == &vt_BFILE);

    // initialize the LOB locators
    for (i = 0; i < var->allocatedElements; i++) {
        status = OCIDescriptorAlloc(var->environment->handle,
                (dvoid**) &var->data[i], OCI_DTYPE_LOB, 0, 0);
        if (Environment_CheckForError(var->environment, status,
                "LobVar_Initialize()") < 0)
            return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// LobVar_Finalize()
//   Prepare for variable destruction.
//-----------------------------------------------------------------------------
static void LobVar_Finalize(
    udt_LobVar *var)                    // variable to free
{
    boolean isTemporary;
    ub4 i;

    for (i = 0; i < var->allocatedElements; i++) {
        if (var->data[i]) {
            OCILobIsTemporary(var->environment->handle,
                    var->environment->errorHandle, var->data[i], &isTemporary);
            if (isTemporary) {
                Py_BEGIN_ALLOW_THREADS
                OCILobFreeTemporary(var->connection->handle,
                        var->environment->errorHandle, var->data[i]);
                Py_END_ALLOW_THREADS
            }
            OCIDescriptorFree(var->data[i], OCI_DTYPE_LOB);
        }
    }
    Py_DECREF(var->connection);
}


//-----------------------------------------------------------------------------
// LobVar_Write()
//   Write data to the LOB variable.
//-----------------------------------------------------------------------------
static int LobVar_Write(
    udt_LobVar *var,                    // variable to perform write against
    unsigned position,                  // position to perform write against
    PyObject *dataObj,                  // data object to write into LOB
    ub4 offset,                         // offset into variable
    ub4 *amount)                        // amount to write
{
    ub2 charsetId = CXORA_CHARSETID;
    udt_StringBuffer buffer;
    sword status;

    // verify the data type
    if (var->type == &vt_BFILE) {
        PyErr_SetString(PyExc_TypeError, "BFILEs are read only");
        return -1;
    } else if (var->type == &vt_BLOB) {
        if (!PyBytes_Check(dataObj)) {
            PyErr_SetString(PyExc_TypeError, "BLOBs expect byte data");
            return -1;
        }
        StringBuffer_FromBytes(&buffer, dataObj);
        *amount = buffer.size;
#ifndef WITH_UNICODE
    } else if (var->type == &vt_NCLOB) {
        if (!PyUnicode_Check(dataObj)) {
            PyErr_SetString(PyExc_TypeError, "NCLOBs expect unicode data");
            return -1;
        }
        if (StringBuffer_FromUnicode(&buffer, dataObj) < 0)
            return -1;
        *amount = buffer.size / 2;
        charsetId = OCI_UTF16ID;
#endif
    } else {
        if (!cxString_Check(dataObj)) {
            PyErr_SetString(PyExc_TypeError, "CLOBs expect string data");
            return -1;
        }
        if (StringBuffer_Fill(&buffer, dataObj) < 0)
            return -1;
        if (var->environment->fixedWidth
                && var->environment->maxBytesPerCharacter > 1)
            *amount = buffer.size / var->environment->maxBytesPerCharacter;
        else *amount = buffer.size;
    }

    // nothing to do if no data to write
    if (*amount == 0) {
        StringBuffer_Clear(&buffer);
        return 0;
    }

    Py_BEGIN_ALLOW_THREADS
    status = OCILobWrite(var->connection->handle,
            var->environment->errorHandle, var->data[position], amount, offset,
            (void*) buffer.ptr, buffer.size, OCI_ONE_PIECE, NULL, NULL,
            charsetId, var->type->charsetForm);
    Py_END_ALLOW_THREADS
    StringBuffer_Clear(&buffer);
    if (Environment_CheckForError(var->environment, status,
            "LobVar_Write()") < 0)
        return -1;
    return 0;
}


//-----------------------------------------------------------------------------
// LobVar_GetValue()
//   Returns the value stored at the given array position.
//-----------------------------------------------------------------------------
static PyObject *LobVar_GetValue(
    udt_LobVar *var,                    // variable to determine value for
    unsigned pos)                       // array position
{
    return ExternalLobVar_New(var, pos);
}


//-----------------------------------------------------------------------------
// LobVar_SetValue()
//   Sets the value stored at the given array position.
//-----------------------------------------------------------------------------
static int LobVar_SetValue(
    udt_LobVar *var,                    // variable to determine value for
    unsigned position,                  // array position
    PyObject *value)                    // value to set
{
    boolean isTemporary;
    sword status;
    ub1 lobType;
    ub4 amount;

    // make sure have temporary LOBs set up
    status = OCILobIsTemporary(var->environment->handle,
            var->environment->errorHandle, var->data[position], &isTemporary);
    if (Environment_CheckForError(var->environment, status,
            "LobVar_SetValue(): is temporary?") < 0)
        return -1;
    if (!isTemporary) {
        if (var->type->oracleType == SQLT_BLOB)
            lobType = OCI_TEMP_BLOB;
        else lobType = OCI_TEMP_CLOB;
        Py_BEGIN_ALLOW_THREADS
        status = OCILobCreateTemporary(var->connection->handle,
                var->environment->errorHandle, var->data[position],
                OCI_DEFAULT, var->type->charsetForm, lobType, FALSE,
                OCI_DURATION_SESSION);
        Py_END_ALLOW_THREADS
        if (Environment_CheckForError(var->environment, status,
                "LobVar_SetValue(): create temporary") < 0)
            return -1;
    }

    // trim the current value
    Py_BEGIN_ALLOW_THREADS
    status = OCILobTrim(var->connection->handle,
            var->environment->errorHandle, var->data[position], 0);
    Py_END_ALLOW_THREADS
    if (Environment_CheckForError(var->environment, status,
            "LobVar_SetValue(): trim") < 0)
        return -1;

    // set the current value
    return LobVar_Write(var, position, value, 1, &amount);
}

