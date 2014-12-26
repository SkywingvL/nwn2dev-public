/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWNScriptCompilerDll.h

Abstract:

	This module defines the externally visible interface to the DLL version of
	the NWScript compiler.

	N.B.  The library is assumed to be single threaded.

--*/

#ifndef _PROGRAMS_NWNSCRIPTCOMPILERDLL_NWNSCRIPTCOMPILERDLL_H
#define _PROGRAMS_NWNSCRIPTCOMPILERDLL_NWNSCRIPTCOMPILERDLL_H

#ifdef _MSC_VER
#pragma once
#endif

namespace NscExt
{

//
// Define the IResourceAccessor thunk APIs and types.
//
// These mirror the IResourceAccessor calls exactly.
//

#ifdef NSCEXT_NO_NWN2DATALIB_REFS

typedef ULONG64 FileHandle;
typedef unsigned short ResType;
typedef ULONG64 FileId;

static const FileHandle INVALID_FILE = 0;

typedef enum _NWACTION_TYPE
{
	ACTIONTYPE_VOID,
	ACTIONTYPE_INT,
	ACTIONTYPE_FLOAT,
	ACTIONTYPE_STRING,
	ACTIONTYPE_OBJECT,
	ACTIONTYPE_VECTOR,
	ACTIONTYPE_ACTION,
	ACTIONTYPE_EFFECT,
	ACTIONTYPE_EVENT,
	ACTIONTYPE_LOCATION,
	ACTIONTYPE_TALENT,
	ACTIONTYPE_ITEMPROPERTY,

	ACTIONTYPE_ENGINE_0 = ACTIONTYPE_EFFECT,
	ACTIONTYPE_ENGINE_1,
	ACTIONTYPE_ENGINE_2,
	ACTIONTYPE_ENGINE_3,
	ACTIONTYPE_ENGINE_4,
	ACTIONTYPE_ENGINE_5,
	ACTIONTYPE_ENGINE_6,
	ACTIONTYPE_ENGINE_7,
	ACTIONTYPE_ENGINE_8,
	ACTIONTYPE_ENGINE_9,

	LASTACTIONTYPE
} NWACTION_TYPE, * PNWACTION_TYPE;

typedef const enum _NWACTION_TYPE * PCNWACTION_TYPE;

#else

typedef IResourceAccessor< NWN::ResRef32 >::FileHandle FileHandle;
typedef IResourceAccessor< NWN::ResRef32 >::ResType ResType;
typedef IResourceAccessor< NWN::ResRef32 >::FileId FileId;

static const FileHandle INVALID_FILE = IResourceAccessor< NWN::ResRef32 >::INVALID_FILE;

#endif

typedef
FileHandle
(__stdcall * ResOpenFileProc)(
	__in const NWN::ResRef32 & ResRef,
	__in ResType Type,
	__in void * Context
	);

typedef
FileHandle
(__stdcall * ResOpenFileByIndexProc)(
	__in FileId FileIndex,
	__in void * Context
	);

typedef
bool
(__stdcall * ResCloseFileProc)(
	__in FileHandle File,
	__in void * Context
	);

typedef
bool
(__stdcall * ResReadEncapsulatedFileProc)(
	__in FileHandle File,
	__in size_t Offset,
	__in size_t BytesToRead,
	__out size_t * BytesRead,
	__out_bcount( BytesToRead ) void * Buffer,
	__in void * Context
	);

typedef
size_t
(__stdcall * ResGetEncapsulatedFileSizeProc)(
	__in FileHandle File,
	__in void * Context
	);

typedef
ResType
(__stdcall * ResGetEncapsulatedFileTypeProc)(
	__in FileHandle File,
	__in void * Context
	);

typedef
bool
(__stdcall * ResGetEncapsulatedFileEntryProc)(
	__in FileId FileIndex,
	__out NWN::ResRef32 & ResRef,
	__out ResType & Type,
	__in void * Context
	);

typedef
FileId
(__stdcall * ResGetEncapsulatedFileCountProc)(
	__in void * Context
	);

typedef
void
(__stdcall * NscCompilerDiagnosticOutputProc)(
	__in const char * DiagnosticMessage,
	__in void * Context
	);

typedef
bool
(__stdcall * ResLoadFileProc)(
	__in const NWN::ResRef32 & ResRef,
	__in ResType Type,
	__deref_out_bcount( *FileSize ) void * * FileContents,
	__out size_t * FileSize,
	__in void * Context
	);

typedef
bool
(__stdcall * ResUnloadFileProc)(
	__in void * FileContents,
	__in void * Context
	);

//
// Define the I/O dispatch table for the compiler, when used in external
// resource mode.
//

typedef struct _NSC_COMPILER_DISPATCH_TABLE_V1
{
	ULONG                                Size;
	PVOID                                Context;
	ResOpenFileProc                      ResOpenFile;
	ResOpenFileByIndexProc               ResOpenFileByIndex;
	ResCloseFileProc                     ResCloseFile;
	ResReadEncapsulatedFileProc          ResReadEncapsulatedFile;
	ResGetEncapsulatedFileSizeProc       ResGetEncapsulatedFileSize;
	ResGetEncapsulatedFileTypeProc       ResGetEncapsulatedFileType;
	ResGetEncapsulatedFileEntryProc      ResGetEncapsulatedFileEntry;
	ResGetEncapsulatedFileCountProc      ResGetEncapsulatedFileCount;
	NscCompilerDiagnosticOutputProc      NscCompilerDiagnosticOutput;
} NSC_COMPILER_DISPATCH_TABLE_V1, * PNSC_COMPILER_DISPATCH_TABLE_V1;

typedef const struct _NSC_COMPILER_DISPATCH_TABLE_V1 * PCNSC_COMPILER_DISPATCH_TABLE_V1;

typedef struct _NSC_COMPILER_DISPATCH_TABLE_V2
{
	ULONG                                Size;
	PVOID                                Context;
	ResOpenFileProc                      ResOpenFile;
	ResOpenFileByIndexProc               ResOpenFileByIndex;
	ResCloseFileProc                     ResCloseFile;
	ResReadEncapsulatedFileProc          ResReadEncapsulatedFile;
	ResGetEncapsulatedFileSizeProc       ResGetEncapsulatedFileSize;
	ResGetEncapsulatedFileTypeProc       ResGetEncapsulatedFileType;
	ResGetEncapsulatedFileEntryProc      ResGetEncapsulatedFileEntry;
	ResGetEncapsulatedFileCountProc      ResGetEncapsulatedFileCount;
	NscCompilerDiagnosticOutputProc      NscCompilerDiagnosticOutput;
	ResLoadFileProc                      ResLoadFile;
	ResUnloadFileProc                    ResUnloadFile;
} NSC_COMPILER_DISPATCH_TABLE_V2, * PNSC_COMPILER_DISPATCH_TABLE_V2;

typedef const struct _NSC_COMPILER_DISPATCH_TABLE_V2 * PCNSC_COMPILER_DISPATCH_TABLE_V2;

typedef struct _NSC_COMPILER_DISPATCH_TABLE_V2 _NSC_COMPILER_DISPATCH_TABLE;
typedef struct _NSC_COMPILER_DISPATCH_TABLE_V2 NSC_COMPILER_DISPATCH_TABLE;
typedef struct _NSC_COMPILER_DISPATCH_TABLE_V2 * PNSC_COMPILER_DISPATCH_TABLE;
typedef const struct _NSC_COMPILER_DISPATCH_TABLE_V2 * PCNSC_COMPILER_DISPATCH_TABLE;

struct _NSC_COMPILER_CONTEXT;

typedef struct _NSC_COMPILER_CONTEXT * NSC_COMPILER_HANDLE;

//
// Create a new compiler object; returns NULL on failure.
//

NSC_COMPILER_HANDLE
__stdcall
NscCreateCompiler(
	__in bool EnableExtensions
	);

typedef
NSC_COMPILER_HANDLE
(__stdcall * NscCreateCompilerProc)(
	__in bool EnableExtensions
	);

//
// Delete an existing compiler object.
//

bool
__stdcall
NscDeleteCompiler(
	__in NSC_COMPILER_HANDLE Compiler
	);

typedef
bool
(__stdcall * NscDeleteCompilerProc)(
	__in NSC_COMPILER_HANDLE Compiler
	);

//
// Compile script code.  Returns true of the operation succeeded.  The results
// are stored in the specified directory named based on the input script base
// name (if applicable).
//

bool
__stdcall
NscCompileScriptExternal(
	__in NSC_COMPILER_HANDLE Compiler,
	__in const char * ScriptFileName,
	__in const char * OutputDirectory,
	__in bool FlushResources,
	__in bool GenerateDebugInfo,
	__in bool Optimize,
	__in bool IgnoreIncludes,
	__in int CompilerVersion,
	__in PCNSC_COMPILER_DISPATCH_TABLE DispatchTable
	);

typedef
bool
(__stdcall * NscCompileScriptExternalProc)(
	__in NSC_COMPILER_HANDLE Compiler,
	__in const char * ScriptFileName,
	__in const char * OutputDirectory,
	__in bool FlushResources,
	__in bool GenerateDebugInfo,
	__in bool Optimize,
	__in bool IgnoreIncludes,
	__in int CompilerVersion,
	__in PCNSC_COMPILER_DISPATCH_TABLE DispatchTable
	);

//
// Return the entrypoint symbol of a script.  The compiler must have compiled
// already.  The return value is only valid until the next compile and is NULL
// if the script had no entrypoint.
//

const char *
__stdcall
NscGetEntrypointSymbolName(
	__in NSC_COMPILER_HANDLE Compiler
	);

typedef
const char
(__stdcall * NscGetEntrypointSymbolNameProc)(
	__in NSC_COMPILER_HANDLE Compiler
	);

//
// Return the number of parameters to a script function by name.  -1 is
// returned on failure.
//

int
__stdcall
NscGetFunctionParameterCount(
	__in NSC_COMPILER_HANDLE Compiler,
	__in const char * FunctionName
	);

typedef
int
(__stdcall * NscGetFunctionParameterCountProc)(
	__in NSC_COMPILER_HANDLE Compiler,
	__in const char * FunctionName
	);

//
// Return the build timestamp of the compiler DLL.
//

const char *
__stdcall
NscGetCompilerBuildDate(
	);

typedef
const char *
(__stdcall * NscGetCompilerBuildDateProc)(
	);

//
// Return the type of an indexed parameter to a script function by name.
//

NWACTION_TYPE
__stdcall
NscGetFunctionParameterType(
	__in NSC_COMPILER_HANDLE Compiler,
	__in const char * FunctionName,
	__in int ParameterIndex
	);

typedef
NWACTION_TYPE
(__stdcall * NscGetFunctionParameterTypeProc)(
	__in NSC_COMPILER_HANDLE Compiler,
	__in const char * FunctionName,
	__in int ParameterIndex
	);

//
// Return the return type of a script function by name.
//

NWACTION_TYPE
__stdcall
NscGetFunctionReturnType(
	__in NSC_COMPILER_HANDLE Compiler,
	__in const char * FunctionName
	);

typedef
NWACTION_TYPE
(__stdcall * NscGetFunctionReturnTypeProc)(
	__in NSC_COMPILER_HANDLE Compiler,
	__in const char * FunctionName
	);

} // namespace NscExt

#endif

