/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptVM.h

Abstract:

	This module defines the core of the NWScript execution environment, the
	NWScript Virtual Machine.  It provides services for the execution of script
	content and the control of the script environment.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTVM_H
#define _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTVM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "NWScriptStack.h"

class NWScriptReader;
class INWScriptActions;
struct IDebugTextOut;

typedef swutil::SharedPtr< NWScriptReader > NWScriptReaderPtr;

struct _NWACTION_DEFINITION;

typedef const struct _NWACTION_DEFINITION * PCNWACTION_DEFINITION;

typedef ULONG NWSCRIPT_ACTION;

//
// Define the overarching script VM class, which encapsulates the state
// necessary to execute a script.

class NWScriptVM
{

public:

	enum ExecDebugLevel
	{
		//
		// No output is desired.
		//

		EDL_None,

		//
		// Only errors are displayed.  This is the default setting.
		//

		EDL_Errors,

		//
		// Calls are traced.
		//

		EDL_Calls,

		//
		// Detailed debugging information about all instructions executed is
		// displayed.
		//

		EDL_Verbose,

		LastExecDebugLevel
	};

	typedef NWScriptStack::PROGRAM_COUNTER         PROGRAM_COUNTER;
	typedef NWScriptStack::STACK_POINTER           STACK_POINTER;
	typedef NWScriptStack::BASE_STACK_TYPE         BASE_STACK_TYPE;
	typedef NWScriptStack::ENGINE_STRUCTURE_NUMBER ENGINE_STRUCTURE_NUMBER;

	//
	// Define the full state of the script VM, used to save and restore
	// execution (such as for a delayed action).
	//

	struct VMState
	{
		NWScriptStack     Stack;
		NWScriptReaderPtr Script;
		PROGRAM_COUNTER   ProgramCounter;
		NWN::OBJECTID     ObjectSelf;
		NWN::OBJECTID     ObjectInvalid;
		bool              Aborted;

		typedef swutil::SharedPtr< VMState > Ptr;
	};



	//
	// Define a script parameter set passed in to execute a script.  The
	// parameters are dynamically typed and converted on the fly once they are
	// referenced.
	//

	typedef std::vector< std::string > ScriptParamVec;


	//
	// Create a new NWScriptVM instance.  The script VM object can be used to
	// execute any number of scripts (including reentrantly, up to the core
	// recursion limit).  However, the script VM is single threaded.
	//

	NWScriptVM(
		__in INWScriptActions * ActionHandler,
		__in IDebugTextOut * TextOut,
		__in_ecount_opt( ActionCount ) PCNWACTION_DEFINITION ActionDefs = NULL,
		__in NWSCRIPT_ACTION ActionCount = 0
		);

	~NWScriptVM(
		);

	enum ExecuteScriptFlags
	{
		//
		// Ignore stack mismatches at end of script, to support GUI scripts
		// that are called with the wrong number of parameters.  Note that this
		// option can only be used if no return value is used!
		//

		ESF_IGNORE_STACK_MISMATCH     = 0x0000001,

		//
		// Raise an exception on failure as opposed to returning the default
		// return code.  This allows the caller to differentiate between a
		// failure to execute and a successful execution without ambiguity, at
		// the cost of taking an exception on an execution error.
		//

		ESF_RAISE_ON_EXEC_FAILURE     = 0x00000002,

		//
		// Snap all parameter types to their static representation via static
		// analysis instead of permitting dynamic typing.  This is necesary for
		// instances where a save to an INWScriptStack might be needed.
		//

		ESF_STATIC_TYPE_DISCOVERY     = 0x00000004,

		LAST_ESF_FLAG
	};

	//
	// Execute a script.  The script's return value (if any) is returned.
	//

	int
	ExecuteScript(
		__in NWScriptReaderPtr Script,
		__in NWN::OBJECTID ObjectSelf,
		__in NWN::OBJECTID ObjectInvalid,
		__in const ScriptParamVec & Params,
		__in int DefaultReturnCode = 0,
		__in ULONG Flags = 0
		);

	//
	// Execute a saved script situation (that is, a delayed action or other
	// suspended script state).  The script state is consumed by the execution.
	//

	void
	ExecuteScriptSituation(
		__inout VMState & ScriptState
		);

	//
	// Abort the currently running script.
	//

	void
	AbortScript(
		);

	//
	// Check whether the currently executing script has been aborted.  This
	// routine may only be called while a script is executing, from a call
	// stack that originated from the script VM (i.e. the script VM itself or
	// an action service handler).
	//

	inline
	bool
	IsScriptAborted(
		) const
	{
		return m_State.Aborted;
	}

	//
	// Change the debug output level.
	//

	void
	SetDebugLevel(
		__in ExecDebugLevel DebugLevel
		);

	//
	// Retrieve the current saved state (which may be NULL).  This routine may
	// only be called from an action handler that takes an action argument.
	//
	// The caller MUST duplicate the VMState object before passing it to a
	// call to ExecuteScriptSituation.
	//

	inline
	VMState &
	GetSavedState(
		)
	{
		return m_SavedState;
	}

	//
	// Check if debug prints are enabled for a level.
	//

	inline
	bool
	IsDebugLevel(
		__in ExecDebugLevel DebugLevel
		) const
	{
		return (m_DebugLevel >= DebugLevel);
	}

	//
	// Return the current self object.  This routine may only be invoked at the
	// start of an action handler, before any reentrant calls to other scripts
	// have been made.
	//

	inline
	NWN::OBJECTID
	GetCurrentActionObject(
		) const
	{
		return m_CurrentActionObjectSelf;
	}

	//
	// Decode an instruction, returning the opcode data and the length.
	//

	inline
	static
	ULONG
	Disassemble(
		__in NWScriptReader * Script,
		__out UCHAR & Opcode,
		__out UCHAR & TypeOpcode,
		__out ULONG & PCOffset
		)
	{
		return DecodeInstruction(
			Script,
			Opcode,
			TypeOpcode,
			PCOffset);
	}

	//
	// Return instruction names for opcode and type opcodes.
	//

	inline
	static
	void
	GetInstructionNames(
		__in UCHAR Opcode,
		__in UCHAR TypeOpcode,
		__deref __out const char * * OpcodeName,
		__deref __out const char * * TypeOpcodeName
		)
	{
		*OpcodeName     = GetInstructionName( Opcode );
		*TypeOpcodeName = GetTypeOpcodeName( TypeOpcode );
	}

	//
	// Define limits on the number of instructions that may be executed within
	// a single execution context, as well as the highest recursion nesting
	// depth permitted.
	//

	enum
	{
		MAX_SCRIPT_INSTRUCTIONS          = 100000,
		MAX_SCRIPT_RECURSION             = 20,
		ANALYSIS_MAX_SCRIPT_INSTRUCTIONS = 10000000
	};

private:

	struct VMBreakpoint
	{
		NWN::ResRef32   ScriptName;
		PROGRAM_COUNTER BreakpointPC;
	};

	//
	// Perform the fundamental script execution operation.
	//

	int
	ExecuteScriptInternal(
		__in NWScriptReaderPtr & Script,
		__in NWN::OBJECTID ObjectSelf,
		__in NWN::OBJECTID ObjectInvalid,
		__inout NWScriptStack & VMStack,
		__in PROGRAM_COUNTER ProgramCounter,
		__in_opt const ScriptParamVec * Params,
		__in int DefaultReturnCode,
		__in ULONG Flags
		);

	//
	// Print debug output to the user.
	//

	void
	DebugPrint(
		__in ExecDebugLevel Level,
		__in __format_string const char * Fmt,
		...
		) const;

	//
	// Check whether an NCS needs fixups and apply them accordingly.
	//

	void
	ApplyNCSFixups(
		__in NWScriptReader * Script,
		__in bool HasParams
		) const;

	//
	// Check if a script uses global variables.
	//

	bool
	ScriptHasGlobals(
		__in NWScriptReader * Script
		) const;

	//
	// Decode an instruction, returning the opcode data and the length.
	//

	static
	ULONG
	DecodeInstruction(
		__in NWScriptReader * Script,
		__out UCHAR & Opcode,
		__out UCHAR & TypeOpcode,
		__out ULONG & PCOffset
		);

	//
	// Execute an instruction stream.
	//

	int
	ExecuteInstructions(
		__in NWScriptReaderPtr & Script,
		__in NWN::OBJECTID ObjectSelf,
		__in NWN::OBJECTID ObjectInvalid,
		__inout NWScriptStack & VMStack,
		__in_opt const ScriptParamVec * Params,
		__in bool NeedFixup,
		__in int DefaultReturnCode,
		__in ULONG Flags
		);

	//
	// Exit the script VM after execution completed.
	//

	void
	ExitVM(
		__inout NWScriptStack & VMStack
		);

	//
	// Analyze a script (and update the parameter information for the
	// script as appropriate).
	//

	void
	AnalyzeScript(
		__in NWScriptReaderPtr & Script,
		__in ULONG Flags
		);

	//
	// Push entrypoint parameters on to the stack.
	//

	void
	PushEntrypointParameters(
		__in const ScriptParamVec * Params,
		__in NWScriptReaderPtr & Script,
		__inout NWScriptStack & VMStack,
		__in ULONG Flags
		);

	//
	// Break into the debugger if a script breakpoint has been reached.
	//

	void
	VMDebuggerCheckForBreakpoint(
		__in const char * ScriptName,
		__in PROGRAM_COUNTER PC,
		__in const NWScriptStack & VMStack,
		__in_opt const char * SymbolName
		);

	//
	// Get the name for an opcode.
	//

	static
	const char *
	GetInstructionName(
		__in UCHAR Opcode
		);

	//
	// Get the name for a type opcode.
	//

	static
	const char *
	GetTypeOpcodeName(
		__in UCHAR TypeOpcode
		);

	//
	// Perform an integer division with safe quotient overflow handling.
	//

	int
	DivideWithExceptionHandler(
		__in int Dividend,
		__in int Divisor,
		__in PROGRAM_COUNTER PC,
		__in const char * ScriptName
		)
	{
		int Quotient;

		if (!DivideWithExceptionHandlerSEH( Dividend, Divisor, &Quotient ))
		{
			DebugPrint(
				EDL_Errors,
				"NWScriptVM::ExecuteInstructions( %s ): @%08X: Quotient overflow in DIVII.\n",
				ScriptName,
				PC);

			throw std::runtime_error( "Quotient overflow in DIVII." );
		}

		return Quotient;
	}

	bool
	DivideWithExceptionHandlerSEH(
		__in int Dividend,
		__in int Divisor,
		__out int * Quotient
		)
	{
		__try
		{
			*Quotient = Dividend / Divisor;
			return true;
		}
		__except (GetExceptionCode( ) == STATUS_INTEGER_OVERFLOW ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			return false;
		}
	}

	int
	ModulusWithExceptionHandler(
		__in int Dividend,
		__in int Divisor,
		__in PROGRAM_COUNTER PC,
		__in const char * ScriptName
		)
	{
		int Remainder;

		if (!ModulusWithExceptionHandlerSEH( Dividend, Divisor, &Remainder ))
		{
			DebugPrint(
				EDL_Errors,
				"NWScriptVM::ExecuteInstructions( %s ): @%08X: Quotient overflow in MODI.\n",
				ScriptName,
				PC);

			throw std::runtime_error( "Quotient overflow in MODI." );
		}

		return Remainder;
	}

	bool
	ModulusWithExceptionHandlerSEH(
		__in int Dividend,
		__in int Divisor,
		__out int * Remainder
		)
	{
		__try
		{
			*Remainder = Dividend % Divisor;
			return true;
		}
		__except (GetExceptionCode( ) == STATUS_INTEGER_OVERFLOW ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			return false;
		}
	}
	//
	// Define the action implementation interface.
	//

	INWScriptActions         * m_ActionHandler;

	//
	// Define the text out handler, used for debug prints.
	//

	mutable IDebugTextOut    * m_TextOut;

	//
	// Define the script debugging level.
	//

	ExecDebugLevel             m_DebugLevel;

	//
	// Define the execution context of the currently running script (if any).
	// Note that this is the base stack of the top level invocation context and
	// not the stack of any nested contexts.
	//

	VMState                    m_State;

	//
	// Define the current count of instructions executed across the current
	// invocation context (including child contexts).
	//

	size_t                     m_InstructionsExecuted;

	//
	// Define the current recursion level within the script VM.
	//

	size_t                     m_RecursionLevel;

	//
	// Define the current saved stack state (generated by a STORE_STATE or
	// similar instruction).
	//

	VMState                    m_SavedState;

	//
	// Define the currently active self object for an action call.  Note that
	// the self object must be captured before a recursive call is made.
	//

	NWN::OBJECTID              m_CurrentActionObjectSelf;

	//
	// Define the active action handler table, that can be used to propagate
	// types from action handlers.
	//

	PCNWACTION_DEFINITION        m_ActionDefs;
	NWSCRIPT_ACTION              m_ActionCount;

	//
	// If debugging the VM, breakpoint state is stored here.  This state is
	// edited by the debugger outside of normal control flow and thus is marked
	// as volatile.
	//

	volatile VMBreakpoint        m_Breakpoints[4];

};

#endif
