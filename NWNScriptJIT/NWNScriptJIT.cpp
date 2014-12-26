/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWNScriptJIT.cpp

Abstract:

	This module houses the externally visible interface of the JIT library.

	N.B.  The external interface is native only and may NOT reference any
	      C++/CLI constructs.

--*/

#include "Precomp.h"
#include "NWNScriptJIT.h"
#include "NWScriptJITLib.h"     // Not used, but just to make sure we compile.
#include "NWScriptProgram.h"
#include "NWScriptSavedState.h"
#include "NWScriptManagedSupport.h"

using System::Runtime::InteropServices::GCHandle;

namespace Internal
{
	bool
	ValidateJITParameters(
		__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams
		)
	/*++

	Routine Description:

		This routine validates the size of the JIT parameters structure.

	Arguments:

		CodeGenParams - Optionally supplies extension code generation parameters.

	Return Value:

		The routine returns a Boolean value indicating TRUE on success, else FALSE
		on failure.

	Environment:

		User mode, C++/CLI.

	--*/

	{
		if (CodeGenParams != NULL)
		{
			switch (CodeGenParams->Size)
			{

			case NWSCRIPT_JIT_PARAMS_SIZE_V0:
			case NWSCRIPT_JIT_PARAMS_SIZE_V1:
			case NWSCRIPT_JIT_PARAMS_SIZE_V2:
				break;

			default:
				return FALSE;

			}
		}

		return TRUE;
	}
}

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
	)
/*++

Routine Description:

	This routine generates native code for a NWScript program, given an
	analyzer that defines the program's function.

	A program handle is returned to the caller, for use with the
	NWScriptExecuteScript API.

	The caller bears responsibility for deleting the generated program via a
	call to the NWScriptDeleteProgram API.

Arguments:

	Script - Supplies a pointer to the script to analyze.

	ActionDefs - Supplies the action table to use when analyzing the script.

	ActionCount - Supplies the count of entries in the action table.

	AnalysisFlags - Supplies flags that control the program analysis.  Legal
	                values are drawn from the ANALYZE_FLAGS enumeration:

	                AF_STRUCTURE_ONLY - Only the program structure is analyzed.

	                AF_NO_OPTIMIZATIONS - Skip the optimization pass.

	TextOut - Optionally supplies an IDebugTextOut interface that receives text
	          debug output from the execution environment.

	DebugLevel - Supplies the debug output level.  Legal values are drawn from
	             the NWScriptVM::ExecDebugLevel family of enumerations.

	ActionHandler - Supplies the engine actions implementation handler.

	ObjectInvalid - Supplies the object id to reference for the 'object
	                invalid' manifest constant.

	CodeGenParams - Optionally supplies extension code generation parameters.

	GeneratedProgram - On success, receives the program native code handle.

Return Value:

	The routine returns a Boolean value indicating TRUE on success, else FALSE
	on failure.

Environment:

	User mode, C++/CLI invoked from external caller.

--*/
{
	if (!Internal::ValidateJITParameters( CodeGenParams ))
		return false;

	try
	{
		NWScriptReader           Reader(
			Script->ScriptName,
			Script->InstructionStream,
			Script->InstructionStreamSize,
			Script->SymTab,
			Script->SymTabSize);
		NWScriptAnalyzer         Analyzer(
			TextOut,
			ActionDefs,
			ActionCount);
		bool                     ManagedScript;
		NWScriptManagedSupport ^ ManagedSupport;
		array< Byte >          ^ ManagedAssembly;

		//
		// If the caller indicates that they have already patched #loader, then
		// set the patch state so that the analyzer knows to accept the patched
		// instruction sequence.
		//

		if ((CodeGenParams != NULL) &&
		    (CodeGenParams->CodeGenFlags & NWCGF_ASSUME_LOADER_PATCHED))
		{
			Reader.SetPatchState( NWScriptReader::NCSPatchState_PatchReturnValue );
		}

		//
		// If managed script support is enabled, check for whether the script
		// is really a managed script.  If so, extract the managed binary and
		// set up for managed execution.
		//

		ManagedScript = false;
		ManagedSupport = nullptr;

		if ((CodeGenParams != NULL)                              &&
		    (CodeGenParams->Size >= NWSCRIPT_JIT_PARAMS_SIZE_V1) &&
		    (CodeGenParams->CodeGenFlags & NWCGF_MANAGED_SCRIPT_SUPPORT))
		{
			GCHandle                      ManagedSupportHandle;
			NWNScriptLib::PROGRAM_COUNTER PlatformBinaryOffset;
			size_t                        PlatformBinarySize;
			
			ManagedSupportHandle = (GCHandle) (IntPtr) CodeGenParams->ManagedSupport;
			ManagedSupport       = (NWScriptManagedSupport ^) ManagedSupportHandle.Target;

			if (Analyzer.IsPlatformNativeScript(
				&Reader,
				NWSCRIPT_MANAGED_SCRIPT_SIGNATURE,
				PlatformBinaryOffset,
				PlatformBinarySize))
			{
				ManagedAssembly = gcnew array< Byte >( (int) PlatformBinarySize );

				pin_ptr< unsigned char > Data = &ManagedAssembly[ 0 ];

				Analyzer.ReadPlatformNativeScript(
					&Reader,
					PlatformBinaryOffset,
					Data,
					PlatformBinarySize);

				ManagedScript = true;
			}
		}

		//
		// Next, generate the IR for the program.
		//
		// N.B.  For a managed script, the IR represents a no-op script.
		//

		Analyzer.Analyze( &Reader, AnalysisFlags );

		//
		// Now translate the IR into MSIL.
		//

		try
		{
			NWScriptProgram ^ Program;
			GCHandle          RetHandle;

			if (ManagedScript)
			{
				Program = gcnew NWScriptProgram(
					&Analyzer,
					TextOut,
					DebugLevel,
					ActionHandler,
					ManagedAssembly,
					ManagedSupport,
					ObjectInvalid,
					CodeGenParams);
			}
			else
			{
				Program = gcnew NWScriptProgram(
					&Analyzer,
					TextOut,
					DebugLevel,
					ActionHandler,
					ObjectInvalid,
					CodeGenParams);
			}

			RetHandle = GCHandle::Alloc( Program );

			*GeneratedProgram = (NWSCRIPT_JITPROGRAM) (IntPtr) RetHandle;

			return TRUE;
		}
		catch (Exception ^)
		{
			return FALSE;
		}
	}
	catch (std::exception &e)
	{
		if ((TextOut != NULL) && (DebugLevel >= NWScriptVM::EDL_Errors))
		{
			TextOut->WriteText(
				"NWScriptGenerateCode: Exception '%s' generating code for script '%s'.\n",
				e.what( ),
				Script->ScriptName);
		}

		return FALSE;
	}
}

BOOLEAN
NWSCRIPTJITAPI
NWScriptDeleteProgram(
	__in NWSCRIPT_JITPROGRAM GeneratedProgram
	)
/*++

Routine Description:

	This routine releases resources allocated by NWScriptGenerateCode.

Arguments:

	GeneratedProgram - Supplies the program to release.

Return Value:

	The routine returns a Boolean value indicating TRUE on success, else FALSE
	on failure.

Environment:

	User mode, C++/CLI invoked from external caller.

--*/
{
	try
	{
		GCHandle Handle = (GCHandle) (IntPtr) GeneratedProgram;

		Handle.Free( );

		return TRUE;
	}
	catch (Exception ^)
	{
		return FALSE;
	}
}

BOOLEAN
NWSCRIPTJITAPI
NWScriptSaveState(
	__in NWSCRIPT_JITPROGRAM GeneratedProgram,
	__out PNWSCRIPT_JITRESUME ResumeState
	)
/*++

Routine Description:

	This routine creates a copy of the most recently saved program state and
	returns it to the caller.  The program state can be used to resume the
	associated program at a SAVE_STATE resume point (via the
	NWScriptExecuteScriptSituation API).

	Note that executing the saved state may be performed only once, but it does
	not delete the saved state.

Arguments:

	GeneratedProgram - Supplies the program to return the last saved state for.

	ResumeState - On success, receives a new resumed state handle.  The state
	              handle may be used only once, after which it must be released
	              via a call to NWScriptDeleteSavedState.

Return Value:

	The routine returns a Boolean value indicating TRUE on success, else FALSE
	on failure.

Environment:

	User mode, C++/CLI invoked from external caller.

--*/
{
	try
	{
		GCHandle             Handle   = (GCHandle) (IntPtr) GeneratedProgram;
		NWScriptProgram    ^ Program  = (NWScriptProgram ^) Handle.Target;
		NWScriptSavedState ^ SavedState;
		GCHandle             RetHandle;

		SavedState = Program->GetSavedState( );
		RetHandle  = GCHandle::Alloc( SavedState );

		*ResumeState = (NWSCRIPT_JITRESUME) (IntPtr) RetHandle;

		return TRUE;
	}
	catch (Exception ^)
	{
		return FALSE;
	}
}

BOOLEAN
NWSCRIPTJITAPI
NWScriptDeleteSavedState(
	__in NWSCRIPT_JITRESUME ResumeState
	)
/*++

Routine Description:

	This routine releases resources allocated by NWScriptSaveState.

Arguments:

	ResumeState - Supplies the saved state to release.

Return Value:

	The routine returns a Boolean value indicating TRUE on success, else FALSE
	on failure.

Environment:

	User mode, C++/CLI invoked from external caller.

--*/
{
	try
	{
		GCHandle Handle = (GCHandle) (IntPtr) ResumeState;

		Handle.Free( );

		return TRUE;
	}
	catch (Exception ^)
	{
		return FALSE;
	}
}

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
	)
/*++

Routine Description:

	This routine executes a script main routine.  The main routine is either a
	"void main(void)" or an "int StartingConditional(Params)" routine.

	In the latter case, a return value may be supplied as an integer.
	Otherwise zero is returned.

Arguments:

	GeneratedProgram - Supplies the script program handle to execute.

	VMStack - Supplies the stack instance that is used to pass parameters to
	          action service handlers.

	ObjectSelf - Supplies the object id to reference for the 'object self'
	             manifest constant.

	Params - Supplies an optional parameter set to pass to the script
	         StartingConditional entry point.

	ParamCount - Supplies the count of parameters to the entry point.

	DefaultReturnCode - Supplies the default return code on an error condition,
	                    or if the script did not return a value.

	Flags - Supplies flags that control the execution environment of the
	        script.  The flags are the same as those that are accepted by the
	        NWScriptVM::ExecuteScript API.

Return Value:

	If the script is a StartingConditional, its return value is returned.
	Otherwise, the default return code is returned.

Environment:

	User mode, C++/CLI invoked from external caller.

--*/
{
	try
	{
		GCHandle          Handle  = (GCHandle) (IntPtr) GeneratedProgram;
		NWScriptProgram ^ Program = (NWScriptProgram ^) Handle.Target;

		return Program->ExecuteScript(
			VMStack,
			ObjectSelf,
			Params,
			ParamCount,
			DefaultReturnCode,
			Flags);
	}
	catch (Exception ^)
	{
		return DefaultReturnCode;
	}
}

BOOLEAN
NWSCRIPTJITAPI
NWScriptExecuteScriptSituation(
	__in NWSCRIPT_JITRESUME ResumeState,
	__in NWN::OBJECTID ObjectSelf
	)
/*++

Routine Description:

	This routine executes a script situation, which is a saved portion of a
	script that is later run (such as a delayed action).

Arguments:

	ResumeState - Supplies the state of the script to execute.

	ObjectSelf - Supplies the object id to reference for the 'object self'
	             manifest constant.

Return Value:

	The routine returns a Boolean value that indicates TRUE on success, else
	FALSE on failure.

Environment:

	User mode, C++/CLI invoked from external caller.

--*/
{
	try
	{
		GCHandle             Handle  = (GCHandle) (IntPtr) ResumeState;
		NWScriptSavedState ^ State   = (NWScriptSavedState ^) Handle.Target;
		NWScriptProgram    ^ Program = State->GetProgram( );

		Program->ExecuteScriptSituation( State, ObjectSelf );

		return TRUE;
	}
	catch (Exception ^)
	{
		return FALSE;
	}
}

BOOLEAN
NWSCRIPTJITAPI
NWScriptAbortScript(
	__in NWSCRIPT_JITPROGRAM GeneratedProgram
	)
/*++

Routine Description:

	This routine aborts a script prematurely.  Once control returns to the
	script execution environment, the executed program terminates and returns
	to its caller.

	This routine may only be invoked within the context of an action service
	handler.

Arguments:

	GeneratedProgram - Supplies the program to abort.

Return Value:

	The routine returns a Boolean value that indicates TRUE on success, else
	FALSE on failure.

Environment:

	User mode, C++/CLI invoked from external caller.

--*/
{
	try
	{
		GCHandle          Handle  = (GCHandle) (IntPtr) GeneratedProgram;
		NWScriptProgram ^ Program = (NWScriptProgram ^) Handle.Target;

		Program->AbortScript( );

		return TRUE;
	}
	catch (Exception ^)
	{
		return FALSE;
	}
}

BOOLEAN
NWSCRIPTJITAPI
NWScriptIsScriptAborted(
	__in NWSCRIPT_JITPROGRAM GeneratedProgram
	)
/*++

Routine Description:

	This routine returns whether a script program has been flagged for early
	termination.

Arguments:

	GeneratedProgram - Supplies the program to inquire about the abortion
	                   status for.

Return Value:

	The routine returns a Boolean value that indicates TRUE should the script
	program have been flagged for abort, else FALSE if it has not been flagged
	for abort.

	Note that the abort flag is reset to FALSE once the script program has been
	actually aborted.

Environment:

	User mode, C++/CLI invoked from external caller.

--*/
{
	try
	{
		GCHandle          Handle  = (GCHandle) (IntPtr) GeneratedProgram;
		NWScriptProgram ^ Program = (NWScriptProgram ^) Handle.Target;

		if (Program->IsScriptAborted( ))
			return TRUE;
		else
			return FALSE;
	}
	catch (Exception ^)
	{
		return FALSE;
	}
}

BOOLEAN
NWSCRIPTJITAPI
NWScriptCheckVersion(
	__in NWSCRIPT_JIT_VERSION Version,
	__in ULONG VersionValue
	)
/*++

Routine Description:

	This routine compares a version value with that supplied by the user of the
	library.  Its purpose is to provide early detection of some (but not all)
	errors that might be introduced by compilation differences in the exposed
	C+++ interface.

Arguments:

	Version - Supplies the version class to compare.  Legal values are drawn
	          from the NWSCRIPT_JIT_VERSION family of enumerations.

	VersionValue - Supplies the version value to check.

Return Value:

	The routine returns a Boolean value that indicates TRUE should the version
	value be compatible, else FALSE if it is incompatible.

	Should the routine return FALSE, the caller must not make use of the
	library, as it is incompatible with the caller's requirements.

Environment:

	User mode, C++/CLI invoked from external caller.

--*/
{
	try
	{
		switch (Version)
		{

		case NWScriptJITVersion_APIVersion:
			return (VersionValue == NWSCRIPTJITAPI_CURRENT);

		case NWScriptJITVersion_NWScriptReaderState:
			return (VersionValue == sizeof( NWScriptReaderState ));
			
		case NWScriptJITVersion_NWScriptStack:
			return (VersionValue == sizeof( NWScriptStack ));

		case NWScriptJITVersion_NWScriptParamVec:
			return (VersionValue == sizeof( NWScriptParamVec ));

		case NWScriptJITVersion_NWACTION_DEFINITION:
			return (VersionValue == sizeof( NWACTION_DEFINITION ));

		case NWScriptJITVersion_NeutralString:
			return (VersionValue == sizeof( NeutralString ));

		default:
			return FALSE;

		}
	}
	catch (Exception ^)
	{
		return FALSE;
	}
}

const wchar_t *
NWSCRIPTJITAPI
NWScriptGetEngineName(
	void
	)
/*++

Routine Description:

	This routine returns a textural string describing the name of the JIT
	engine.

Arguments:

	None.

Return Value:

	A pointer to a static, null-terminated string describing the JIT engine
	name is returned.

Environment:

	User mode, C++/CLI invoked from external caller.

--*/
{
	return L"MSIL/.NET JIT";
}

BOOLEAN
NWSCRIPTJITAPI
NWScriptDuplicateScriptSituation(
	__in NWSCRIPT_JITRESUME SourceState,
	__out PNWSCRIPT_JITRESUME ResumeState
	)
/*++

Routine Description:

	This routine creates a copy of an existing script program state, which must
	not have already been consumed.  The copy is returned to the caller, and
	can be consumed or deleted as a normal script situation.

	This routine is intended for use when a single script situation may need to
	be executed multiple times.

	Note that executing the saved state may be performed only once, but it does
	not delete the saved state.

Arguments:

	SourceState - Supplies the unconsumed resume state handle to duplicate.

	ResumeState - On success, receives a new resumed state handle.  The state
	              handle may be used only once, after which it must be released
	              via a call to NWScriptDeleteSavedState.

Return Value:

	The routine returns a Boolean value indicating TRUE on success, else FALSE
	on failure.

Environment:

	User mode, C++/CLI invoked from external caller.

--*/
{
	try
	{
		GCHandle             Handle   = (GCHandle) (IntPtr) SourceState;
		NWScriptSavedState ^ Source   = (NWScriptSavedState ^) Handle.Target;
		NWScriptSavedState ^ SavedState;
		GCHandle             RetHandle;

		SavedState = Source->GetProgram( )->DuplicateSavedState( Source );
		RetHandle  = GCHandle::Alloc( SavedState );

		*ResumeState = (NWSCRIPT_JITRESUME) (IntPtr) RetHandle;

		return TRUE;
	}
	catch (Exception ^)
	{
		return FALSE;
	}
}

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
	)
/*++

Routine Description:

	This routine saves the information contained within a saved program state
	to a stack instance (which could then be used to transport the saved state
	as necessary).  The saved program state's globals and locals are saved to
	the stack, and resume information is returned to the caller.  This data
	must be provided to a matching call to NWScriptPopScriptSituation in order
	to instantiate a new saved program state instance that can be directly
	executed.

	Note that the saved state can be used even if the underlying generated
	program object is deleted and recreated (or even if the host process is
	restarted), so long as it is guaranteed that the saved state is used with
	a precisely identical script program consisting of the same instruction
	sequences as the original.

	The source state is not consumed by the save operation, and may be used in
	other requests as desired.

Arguments:

	SourceState - Supplies the resume state handle to save to the stack.  The
	              state is not consumed by the operation.

	Stack - Supplies the stack to save the state to.

	ResumeMethodId - Receives the script situation id of the subroutine to
	                 execute on resume.

	ResumeMethodPC - Receives the NWScript program counter of the script
	                 situation to execute on resume.

	SaveGlobalCount - Receives the count of global variables that were placed
	                  on to the stack.

	SaveLocalCount - Receives the count of local variables that were placed on
	                 to the stack.

	CurrentActionObjectSelf - Receives the OBJECT_SELF object identifier for
	                          the saved state.

Return Value:

	The routine returns a Boolean value indicating TRUE on success, else FALSE
	on failure.

Environment:

	User mode, C++/CLI invoked from external caller.

--*/
{
	try
	{
		GCHandle             Handle  = (GCHandle) (IntPtr) SourceState;
		NWScriptSavedState ^ State   = (NWScriptSavedState ^) Handle.Target;
		NWScriptProgram    ^ Program = State->GetProgram( );

		C_ASSERT( sizeof( NWSCRIPT_PROGRAM_COUNTER ) == sizeof( NWNScriptLib::PROGRAM_COUNTER ) );

		Program->PushSavedState(
			State,
			VMStack,
			(UInt32 *) ResumeMethodId,
			ResumeMethodPC,
			(UInt32 *) SaveGlobalCount,
			(UInt32 *) SaveLocalCount,
			ObjectSelf);

		return TRUE;
	}
	catch (Exception ^)
	{
		return FALSE;
	}
}

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
	)
/*++

Routine Description:

	This routine restores the information contained within a saved program
	state from a stack instance, which might be from a source saved program
	state from another process (but is bound to the same compiled script input
	file).

	The saved program state's globals and locals are loaded from the stack, and
	resume information provided by the caller is loaded into a new saved
	program state instance which is then returned to the caller.  The saved
	program state instance that is returned can then be resumed as normal.

	N.B.  The VM stack used to restore the script situation is configured as
	      the stack used to pass arguments to action service handlers when the
	      script situation is resumed.

Arguments:

	GeneratedProgram - Supplies the program to instantiate a resume state for.
	                   The program must have been generated from the same input
	                   compiled script file as the program instance that was
	                   used to create the saved state data.

	Stack - Supplies the stack to restore the state from.  Note that the stack
	        is configured as the active stack for the instance.

	ResumeMethodId - Supplies the script situation id of the subroutine to
	                 execute on resume.

	ResumeMethodPC - Supplies the NWScript program counter of the script
	                 situation to execute on resume.

	SaveGlobalCount - Supplies the count of global variables that were placed
	                  on to the stack.

	SaveLocalCount - Supplies the count of local variables that were placed on
	                 to the stack.

	CurrentActionObjectSelf - Supplies the OBJECT_SELF object identifier for
	                          the saved state.

	ResumeState - On success, receives a new resumed state handle.  The state
	              handle may be used only once, after which it must be released
	              via a call to NWScriptDeleteSavedState.

Return Value:

	The routine returns a Boolean value indicating TRUE on success, else FALSE
	on failure.

Environment:

	User mode, C++/CLI invoked from external caller.

--*/
{
	try
	{
		GCHandle             Handle  = (GCHandle) (IntPtr) GeneratedProgram;
		NWScriptProgram    ^ Program = (NWScriptProgram ^) Handle.Target;
		NWScriptSavedState ^ SavedState;
		GCHandle             RetHandle;

		C_ASSERT( sizeof( NWSCRIPT_PROGRAM_COUNTER ) == sizeof( NWNScriptLib::PROGRAM_COUNTER ) );

		SavedState = Program->PopSavedState(
			VMStack,
			ResumeMethodId,
			ResumeMethodPC,
			SaveGlobalCount,
			SaveLocalCount,
			ObjectSelf);

		RetHandle = GCHandle::Alloc( SavedState );

		*ResumeState = (NWSCRIPT_JITRESUME) (IntPtr) RetHandle;

		return TRUE;
	}
	catch (Exception ^)
	{
		return FALSE;
	}
}

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
	)
/*++

Routine Description:

	This routine generates the managed interface support assembly that is used
	to connect code authored in native CLR languages to the action service
	dispatch mechanism.

	A managed support handle is returned to the caller, for use with the
	NWScriptGenerateCode API.

	The caller bears responsibility for deleting the generated managed support
	object via a call to the NWScriptDeleteManagedSupport API.

Arguments:

	ActionDefs - Supplies the action table to use when analyzing the script.

	ActionCount - Supplies the count of entries in the action table.

	AnalysisFlags - Supplies flags that control the program analysis.  Legal
	                values are drawn from the ANALYZE_FLAGS enumeration:

	                AF_STRUCTURE_ONLY - Only the program structure is analyzed.

	                AF_NO_OPTIMIZATIONS - Skip the optimization pass.

	TextOut - Optionally supplies an IDebugTextOut interface that receives text
	          debug output from the execution environment.

	DebugLevel - Supplies the debug output level.  Legal values are drawn from
	             the NWScriptVM::ExecDebugLevel family of enumerations.

	ActionHandler - Supplies the engine actions implementation handler.

	ObjectInvalid - Supplies the object id to reference for the 'object
	                invalid' manifest constant.

	CodeGenParams - Optionally supplies extension code generation parameters.

	GeneratedManagedSupport - On success, receives the managed support object
	                          handle.

Return Value:

	The routine returns a Boolean value indicating TRUE on success, else FALSE
	on failure.

Environment:

	User mode, C++/CLI invoked from external caller.

--*/
{
	UNREFERENCED_PARAMETER( AnalysisFlags );

	if (!Internal::ValidateJITParameters( CodeGenParams ))
		return false;

	try
	{
		NWScriptAnalyzer Analyzer(
			TextOut,
			ActionDefs,
			ActionCount);

		//
		// Create the managed interface layer assembly and return a handle to
		// the wrapper object.
		//

		try
		{
			NWScriptManagedSupport ^ ManagedSupport;
			GCHandle                 RetHandle;

			ManagedSupport   = gcnew NWScriptManagedSupport(
				&Analyzer,
				TextOut,
				DebugLevel,
				ActionHandler,
				ObjectInvalid,
				CodeGenParams);
			RetHandle = GCHandle::Alloc( ManagedSupport );

			*GeneratedManagedSupport = (NWSCRIPT_JITMANAGEDSUPPORT) (IntPtr) RetHandle;

			return TRUE;
		}
		catch (Exception ^)
		{
			return FALSE;
		}
	}
	catch (std::exception &e)
	{
		if ((TextOut != NULL) && (DebugLevel >= NWScriptVM::EDL_Errors))
		{
			TextOut->WriteText(
				"NWScriptCreateManagedSupport: Exception '%s' generating managed support object.\n",
				e.what( ));
		}

		return FALSE;
	}
}

BOOLEAN
NWSCRIPTJITAPI
NWScriptDeleteManagedSupport(
	__in NWSCRIPT_JITMANAGEDSUPPORT GeneratedManagedSupport
	)
/*++

Routine Description:

	This routine releases resources allocated by NWScriptCreateManagedSupport.

Arguments:

	GeneratedManagedSupport - Supplies the managed support object to release.

Return Value:

	The routine returns a Boolean value indicating TRUE on success, else FALSE
	on failure.

Environment:

	User mode, C++/CLI invoked from external caller.

--*/
{
	try
	{
		GCHandle Handle = (GCHandle) (IntPtr) GeneratedManagedSupport;

		Handle.Free( );

		return TRUE;
	}
	catch (Exception ^)
	{
		return FALSE;
	}
}
