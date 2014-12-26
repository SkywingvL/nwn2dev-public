/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWNScriptJIT.h

Abstract:

	This module defines the externally visible interface of the JIT library.

	N.B.  The external interface is native only and may NOT reference any
	      C++/CLI constructs.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTJIT_NWNSCRIPTJIT_H
#define _SOURCE_PROGRAMS_NWNSCRIPTJIT_NWNSCRIPTJIT_H

#ifdef _MSC_VER
#pragma once
#endif

class NWScriptStack;
class NWScriptReader;
struct INWScriptStack;

//
// Define a handle to a JIT'd NWScript program.  The program entry point may be
// executed using this handle, and the program may be resumed after a saved
// state in conjuction with a resume handle.
//

typedef void * NWSCRIPT_JITPROGRAM;
typedef NWSCRIPT_JITPROGRAM * PNWSCRIPT_JITPROGRAM;

//
// Define a handle to the resume state for a suspended NWScript program.  The
// saved state may be resumed in conjunction with its associated program
// handle.
//

typedef void * NWSCRIPT_JITRESUME;
typedef NWSCRIPT_JITRESUME * PNWSCRIPT_JITRESUME;

//
// Define a handle to the managed support state for managed "scripts".  The
// handle may be used in conjuction with "code generation" for managed
// "scripts".
//

typedef void * NWSCRIPT_JITMANAGEDSUPPORT;
typedef NWSCRIPT_JITMANAGEDSUPPORT * PNWSCRIPT_JITMANAGEDSUPPORT;

#define NWSCRIPTJITAPI __stdcall

//
// N.B.  The NWNScriptJIT logic MUST NOT cause an allocation or deletion to
//       occur with respect to any NWScriptParamVec passed cross-module, or the
//       wrong operator new / operator delete would be invoked, leading to
//       catastrophic memory corruption.
//

typedef std::vector< std::string > NWScriptParamVec;
typedef std::pair< char *, size_t > NeutralString;

typedef ULONG NWSCRIPT_PROGRAM_COUNTER;

struct NWScriptReaderState
{
	const NWScriptReader::SymbolTableRawEntry * SymTab;
	size_t                                      SymTabSize;
	const char                                * ScriptName;
	const unsigned char                       * InstructionStream;
	size_t                                      InstructionStreamSize;
};

struct NWScriptParamString
{
	const char * String;
	size_t       Len;
};

//
// Define extension parameters for NWScriptGenerateCode.
//

typedef enum _NWSCRIPT_CODE_GEN_FLAGS
{
	//
	// Save the output of the code generation to disk.
	//

	NWCGF_SAVE_OUTPUT                 = 0x00000001,

	//
	// Emit code that allows saved states to be serialized to an INWScriptStack
	// with locals, globals, and PROGRAM_COUNTER tracking.  This may increase
	// the size of the emitted script program.
	//

	NWCGF_ENABLE_SAVESTATE_TO_VMSTACK = 0x00000002,

	//
	// Treat the input script instruction stream as though it may have already
	// been patched to nop out the RSADDI at the start of #loader.
	//

	NWCGF_ASSUME_LOADER_PATCHED       = 0x00000004,

	//
	// Enable the use of managed scripts.  The current script may or may not
	// actually be a managed script (auto-detected).
	//

	NWCGF_MANAGED_SCRIPT_SUPPORT      = 0x00000008,

	//
	// Disable script execution guards in emitted script programs (e.g. against
	// unbounded recursion or looping).
	//

	NWCGF_DISABLE_EXECUTION_GUARDS    = 0x00000010,

	//
	// Treat the action service handler list as a Neverwinter Nights-compatible
	// list for purpose of promoting certain service service handlers to
	// built-in intrinsics.
	//

	NWCGF_NWN_COMPATIBLE_ACTIONS      = 0x00000020,

	LAST_NWCGF
} NWSCRIPT_CODE_GEN_FLAGS, * PNWSCRIPT_CODE_GEN_FLAGS;

typedef const enum _NWSCRIPT_CODE_GEN_FLAGS * PCNWSCRIPT_CODE_GEN_FLAGS;

#define NWSCRIPT_JIT_PARAMS_SIZE_V0 RTL_SIZEOF_THROUGH_FIELD( NWSCRIPT_JIT_PARAMS, CodeGenOutputDir )
#define NWSCRIPT_JIT_PARAMS_SIZE_V1 RTL_SIZEOF_THROUGH_FIELD( NWSCRIPT_JIT_PARAMS, ManagedSupport )
#define NWSCRIPT_JIT_PARAMS_SIZE_V2 RTL_SIZEOF_THROUGH_FIELD( NWSCRIPT_JIT_PARAMS, MaxCallDepth )

//
// Define the signature for a managed script native binary.
//

#define NWSCRIPT_MANAGED_SCRIPT_SIGNATURE "Managed NWScript v1.0"

//
// Define extension JIT code generation parameters.
//

typedef struct _NWSCRIPT_JIT_PARAMS
{
	//
	// Define the size of the structure.
	//

	ULONG                        Size;

	//
	// Define code gen flags to control the JIT process.  Legal values are
	// drawn from the NWSCRIPT_CODE_GEN_FLAGS enumeration.
	//

	ULONG                        CodeGenFlags;

	//
	// Define the directory to save code generation output to.  If not set, the
	// current directory is assumed.  This value is only used if the 
	// NWCGF_SAVE_OUTPUT flag is set.  The name should end in a path separator
	// if the parameter is not NULL.
	//

	const wchar_t              * CodeGenOutputDir;

	//
	// If support for "scripts" authored in native CLR languages is enabled,
	// then a handle to the managed interface library may be supplied here.
	// If this member is avlid, NWCGF_MANAGED_SCRIPT_SUPPORT must be set in the
	// CodeGenFlags.
	//

	NWSCRIPT_JITMANAGEDSUPPORT   ManagedSupport;

	//
	// The maximum number of supported loop iterations permitted in an
	// execution of the script can be set here.  Zero sets the default, which
	// is 100000 iterations (backwards jumps).
	//

	int                          MaxLoopIterations;

	//
	// The maximum call depth of the script can be set here.  Zero sets the
	// defalt, which is 128.
	//

	int                          MaxCallDepth;

} NWSCRIPT_JIT_PARAMS, * PNWSCRIPT_JIT_PARAMS;

typedef const struct _NWSCRIPT_JIT_PARAMS * PCNWSCRIPT_JIT_PARAMS;


//
// Generate a JIT'd native program handle for a script program, given an
// analyzer instance which represents a representation of the program's
// function.
//
// The program may be re-used across multiple executions.  However, the script
// program itself is single threaded and does not support concurrent execution
// across multiple threads.
//
// The routine returns TRUE on success, else FALSE on failure.
//

BOOLEAN
NWSCRIPTJITAPI
NWScriptGenerateCode(
	__in NWScriptReaderState * Script,
	__in_ecount( ActionCount ) PCNWACTION_DEFINITION ActionDefs,
	__in NWSCRIPT_ACTION ActionCount,
	__in ULONG AnalysisFlags,
	__in_opt IDebugTextOut * TextOut,
	__in ULONG DebugLevel,
	__in INWScriptActions * ActionHandler,
	__in NWN::OBJECTID ObjectInvalid,
	__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams,
	__out PNWSCRIPT_JITPROGRAM GeneratedProgram
	);

typedef
BOOLEAN
(NWSCRIPTJITAPI *
NWScriptGenerateCodeProc)(
	__in NWScriptReaderState * Script,
	__in_ecount( ActionCount ) PCNWACTION_DEFINITION ActionDefs,
	__in NWSCRIPT_ACTION ActionCount,
	__in ULONG AnalysisFlags,
	__in_opt IDebugTextOut * TextOut,
	__in ULONG DebugLevel,
	__in INWScriptActions * ActionHandler,
	__in NWN::OBJECTID ObjectInvalid,
	__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams,
	__out PNWSCRIPT_JITPROGRAM GeneratedProgram
	);

//
// Delete a generated program.
//

BOOLEAN
NWSCRIPTJITAPI
NWScriptDeleteProgram(
	__in NWSCRIPT_JITPROGRAM GeneratedProgram
	);

typedef
BOOLEAN
(NWSCRIPTJITAPI *
NWScriptDeleteProgramProc)(
	__in NWSCRIPT_JITPROGRAM GeneratedProgram
	);

//
// Generate a resume state from the current state of the JIT program.  This
// routine only returns a usable state should the program have saved state via
// a SAVE_STATE operation (i.e. as an 'action' argument type to a script
// action handler call).
//

BOOLEAN
NWSCRIPTJITAPI
NWScriptSaveState(
	__in NWSCRIPT_JITPROGRAM GeneratedProgram,
	__out PNWSCRIPT_JITRESUME ResumeState
	);

typedef
BOOLEAN
(NWSCRIPTJITAPI *
NWScriptSaveStateProc)(
	__in NWSCRIPT_JITPROGRAM GeneratedProgram,
	__out PNWSCRIPT_JITRESUME ResumeState
	);

//
// Delete a saved state.
//

BOOLEAN
NWSCRIPTJITAPI
NWScriptDeleteSavedState(
	__in NWSCRIPT_JITRESUME ResumeState
	);

typedef
BOOLEAN
(NWSCRIPTJITAPI *
NWScriptDeleteSavedStateProc)(
	__in NWSCRIPT_JITRESUME ResumeState
	);


//
// Execute a script program, returning the results to the caller.
//

int
NWSCRIPTJITAPI
NWScriptExecuteScript(
	__in NWSCRIPT_JITPROGRAM GeneratedProgram,
	__in INWScriptStack * VMStack,
	__in NWN::OBJECTID ObjectSelf,
	__in_ecount_opt( ParamCount ) const NWScriptParamString * Params,
	__in size_t ParamCount,
	__in int DefaultReturnCode,
	__in ULONG Flags
	);

typedef
int
(NWSCRIPTJITAPI *
NWScriptExecuteScriptProc)(
	__in NWSCRIPT_JITPROGRAM GeneratedProgram,
	__in INWScriptStack * VMStack,
	__in NWN::OBJECTID ObjectSelf,
	__in_ecount_opt( ParamCount ) const NWScriptParamString * Params,
	__in size_t ParamCount,
	__in int DefaultReturnCode,
	__in ULONG Flags
	);

//
// Execute a saved script situation (i.e. a save state).
//

BOOLEAN
NWSCRIPTJITAPI
NWScriptExecuteScriptSituation(
	__in NWSCRIPT_JITRESUME ResumeState,
	__in NWN::OBJECTID ObjectSelf
	);

typedef
BOOLEAN
(NWSCRIPTJITAPI *
NWScriptExecuteScriptSituationProc)(
	__in NWSCRIPT_JITRESUME ResumeState,
	__in NWN::OBJECTID ObjectSelf
	);

//
// Abort a script program that is currently executing.  This routine may only
// be invoked by an action service routine that is executing the specified
// program.
//

BOOLEAN
NWSCRIPTJITAPI
NWScriptAbortScript(
	__in NWSCRIPT_JITPROGRAM GeneratedProgram
	);

typedef
BOOLEAN
(NWSCRIPTJITAPI *
NWScriptAbortScriptProc)(
	__in NWSCRIPT_JITPROGRAM GeneratedProgram
	);

//
// Return whether a script program has been aborted early.  This routine may
// only be invoked by an action service routine that is executing the specified
// program.
//

BOOLEAN
NWSCRIPTJITAPI
NWScriptIsScriptAborted(
	__in NWSCRIPT_JITPROGRAM GeneratedProgram
	);

typedef
BOOLEAN
(NWSCRIPTJITAPI *
NWScriptIsScriptAbortedProc)(
	__in NWSCRIPT_JITPROGRAM GeneratedProgram
	);

//
// Define API versions of the JIT library.
//

enum
{
	NWSCRIPTJITAPI_0       = 0,
	NWSCRIPTJITAPI_1       = 1, // NWScriptExecuteScriptSituation has OBJECTID
	NWSCRIPTJITAPI_CURRENT = NWSCRIPTJITAPI_1
};

//
// Define version properties that can be compared to ensure compatibility of
// the library with the caller.
//
// N.B.  While these are not foolproof checks, they provide a baseline level of
//       detection for obvious structure size mismatches that would cause
//       difficult-to-debug issues.  Structure layout changes are not so easily
//       detectable, however.
//

typedef enum _NWSCRIPT_JIT_VERSION
{
	//
	// NWSCRIPTJITAPI_CURRENT
	//

	NWScriptJITVersion_APIVersion         = 0,

	//
	// sizeof( NWScriptReaderState )
	//

	NWScriptJITVersion_NWScriptReaderState,

	//
	// sizeof( NWScriptStack )
	//

	NWScriptJITVersion_NWScriptStack,

	//
	// sizeof( NWScriptParamVec )
	//

	NWScriptJITVersion_NWScriptParamVec,

	//
	// sizeof( NWACTION_DEFINITION )
	//

	NWScriptJITVersion_NWACTION_DEFINITION,

	//
	// sizeof( NeutralString )
	//

	NWScriptJITVersion_NeutralString,

	NWScriptJITVersion_Max
} NWSCRIPT_JIT_VERSION, * PNWSCRIPT_JIT_VERSION;

typedef const enum _NWSCRIPT_JIT_VERSION * PCNWSCRIPT_JIT_VERSION;

//
// Verify the compatibility of the library with the caller.
//

BOOLEAN
NWSCRIPTJITAPI
NWScriptCheckVersion(
	__in NWSCRIPT_JIT_VERSION Version,
	__in ULONG VersionValue
	);

typedef
BOOLEAN
(NWSCRIPTJITAPI *
NWScriptCheckVersionProc)(
	__in NWSCRIPT_JIT_VERSION Version,
	__in ULONG VersionValue
	);

//
// Return a textural name of the the JIT engine.
//

const wchar_t *
NWScriptGetEngineName(
	void
	);

typedef
const wchar_t *
(NWSCRIPTJITAPI *
NWScriptGetEngineNameProc)(
	void
	);

//
// Create a duplicate copy of a saved program state.  The saved state can be
// consumed or deleted as a normal saved state.
//

BOOLEAN
NWSCRIPTJITAPI
NWScriptDuplicateScriptSituation(
	__in NWSCRIPT_JITRESUME SourceState,
	__out PNWSCRIPT_JITRESUME ResumeState
	);

typedef
BOOLEAN
(NWSCRIPTJITAPI *
NWScriptDuplicateScriptSituationProc)(
	__in NWSCRIPT_JITRESUME SourceState,
	__out PNWSCRIPT_JITRESUME ResumeState
	);

//
// Push a script situation's locals and globals onto a VM stack, and return the
// internal script situation state for serialization.
//

BOOLEAN
NWSCRIPTJITAPI
NWScriptPushScriptSituation(
	__in NWSCRIPT_JITRESUME SourceState,
	__in INWScriptStack * VMStack,
	__out PULONG ResumeMethodId,
	__out NWSCRIPT_PROGRAM_COUNTER * ResumeMethodPC,
	__out PULONG SaveGlobalCount,
	__out PULONG SaveLocalCount,
	__out NWN::OBJECTID * ObjectSelf
	);

typedef
BOOLEAN
(NWSCRIPTJITAPI *
NWScriptPushScriptSituationProc)(
	__in NWSCRIPT_JITRESUME SourceState,
	__in INWScriptStack * VMStack,
	__out PULONG ResumeMethodId,
	__out NWSCRIPT_PROGRAM_COUNTER * ResumeMethodPC,
	__out PULONG SaveGlobalCount,
	__out PULONG SaveLocalCount,
	__out NWN::OBJECTID * ObjectSelf
	);

//
// Pop a script situation's locals and globals from a VM stack, and create a
// script situation object with the specified resume parameters.
//

BOOLEAN
NWSCRIPTJITAPI
NWScriptPopScriptSituation(
	__in NWSCRIPT_JITPROGRAM GeneratedProgram,
	__in INWScriptStack * VMStack,
	__in ULONG ResumeMethodId,
	__in NWSCRIPT_PROGRAM_COUNTER ResumeMethodPC,
	__in ULONG SaveGlobalCount,
	__in ULONG SaveLocalCount,
	__in NWN::OBJECTID ObjectSelf,
	__out PNWSCRIPT_JITRESUME ResumeState
	);

typedef
BOOLEAN
(NWSCRIPTJITAPI *
NWScriptPopScriptSituationProc)(
	__in NWSCRIPT_JITPROGRAM GeneratedProgram,
	__in INWScriptStack * VMStack,
	__in ULONG ResumeMethodId,
	__in NWSCRIPT_PROGRAM_COUNTER ResumeMethodPC,
	__in ULONG SaveGlobalCount,
	__in ULONG SaveLocalCount,
	__in NWN::OBJECTID ObjectSelf,
	__out PNWSCRIPT_JITRESUME ResumeState
	);



//
// Generate support for scripts authored in native CLR code.  The returned
// managed support handle must be supplied via CodeGenParams to the script
// function.
//
// The managed support object may be re-used across multiple executions.
// However, the managed support object itself is single threaded and does not
// support concurrent execution across multiple threads.
//
// The routine returns TRUE on success, else FALSE on failure.
//

BOOLEAN
NWSCRIPTJITAPI
NWScriptCreateManagedSupport(
	__in_ecount( ActionCount ) PCNWACTION_DEFINITION ActionDefs,
	__in NWSCRIPT_ACTION ActionCount,
	__in ULONG AnalysisFlags,
	__in_opt IDebugTextOut * TextOut,
	__in ULONG DebugLevel,
	__in INWScriptActions * ActionHandler,
	__in NWN::OBJECTID ObjectInvalid,
	__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams,
	__out PNWSCRIPT_JITMANAGEDSUPPORT GeneratedManagedSupport
	);

typedef
BOOLEAN
(NWSCRIPTJITAPI *
NWScriptCreateManagedSupportProc)(
	__in_ecount( ActionCount ) PCNWACTION_DEFINITION ActionDefs,
	__in NWSCRIPT_ACTION ActionCount,
	__in ULONG AnalysisFlags,
	__in_opt IDebugTextOut * TextOut,
	__in ULONG DebugLevel,
	__in INWScriptActions * ActionHandler,
	__in NWN::OBJECTID ObjectInvalid,
	__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams,
	__out PNWSCRIPT_JITMANAGEDSUPPORT GeneratedManagedSupport
	);

//
// Delete a generated managed support object.
//

BOOLEAN
NWSCRIPTJITAPI
NWScriptDeleteManagedSupport(
	__in NWSCRIPT_JITMANAGEDSUPPORT GeneratedManagedSupport
	);

typedef
BOOLEAN
(NWSCRIPTJITAPI *
NWScriptDeleteManagedSupportProc)(
	__in NWSCRIPT_JITMANAGEDSUPPORT GeneratedManagedSupport
	);


#endif

