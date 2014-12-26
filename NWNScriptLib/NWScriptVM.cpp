/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptVM.cpp

Abstract:

	This module houses the core of the NWScript execution environment, the
	NWScript Virtual Machine.  It provides services for the execution of script
	content and the control of the script environment.

--*/

#include "Precomp.h"
#include "NWScriptVM.h"
#include "NWScriptStack.h"
#include "NWScriptInternal.h"
#include "NWScriptInterfaces.h"
#include "../NWN2MathLib/NWN2MathLib.h"


//
// Define to 1 in order to enable the script VM debugger (verbose tracing of
// each executed instructions).
//

#define VM_DEBUGGER 1

//
// Define to 1 to in order to analyze script structure to determine the correct
// parameter typing for parameterized script invocations.
//

#define ANALYZE_SCRIPT 1

#if ANALYZE_SCRIPT
#include "NWScriptAnalyzer.h"
#endif

//
// STACK_PTR( x ) returns a legal SP reference given a stack offset.  In the
// debugger enabled version however we would like the bogus SP references to
// make it to the NWScriptStack where they can be checked.  In the non-debugger
// version, however, we optimize for performance and just truncate any wrong
// bits if they were set.
//

#if VM_DEBUGGER
#define STACK_PTR( x ) x
#else
#define STACK_PTR( x ) ((x) & ~3) // NOTE: Hardcodes GetStackIntegerSize / STACK_ENTRY_SIZE for performance !
#endif



NWScriptVM::NWScriptVM(
	__in INWScriptActions * ActionHandler,
	__in IDebugTextOut * TextOut,
	__in_ecount_opt( ActionCount ) PCNWACTION_DEFINITION ActionDefs /* = NULL */,
	__in NWSCRIPT_ACTION ActionCount
	)
/*++

Routine Description:

	This routine constructs a new NWScriptVM.

Arguments:

	ActionHandler - Supplies the engine actions implementation handler.

	TextOut - Supplies the text output system for debug prints.

	ActionDefs - Optionally supplies the action table to use when analyzing the
	             script.

	ActionCount - Supplies the count of entries in the action table.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
: m_ActionHandler( ActionHandler ),
  m_TextOut( TextOut ),
  m_DebugLevel( EDL_Errors ),
  m_InstructionsExecuted( 0 ),
  m_RecursionLevel( 0 ),
  m_CurrentActionObjectSelf( NWN::INVALIDOBJID ),
  m_ActionDefs( ActionDefs ),
  m_ActionCount( ActionCount )
{
	m_State.ProgramCounter = 0;
	m_State.ObjectSelf     = NWN::INVALIDOBJID;
	m_State.ObjectInvalid  = NWN::INVALIDOBJID;
	m_State.Aborted        = false;

	m_SavedState = m_State;

	ZeroMemory( (void *) &m_Breakpoints, sizeof( m_Breakpoints ) );
}

NWScriptVM::~NWScriptVM(
	)
/*++

Routine Description:

	This routine deletes the current NWScriptVM object and its associated
	members.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
}

int
NWScriptVM::ExecuteScript(
	__in NWScriptReaderPtr Script,
	__in NWN::OBJECTID ObjectSelf,
	__in NWN::OBJECTID ObjectInvalid,
	__in const ScriptParamVec & Params,
	__in int DefaultReturnCode /* = 0 */,
	__in ULONG Flags /* = 0 */
	)
/*++

Routine Description:

	This routine executes a script main routine.  The main routine is either a
	"void main(void)" or an "int StartingConditional(Params)" routine.

	In the latter case, a return value may be supplied as an integer.
	Otherwise zero is returned.

Arguments:

	Script - Supplies the script byte code to execute.

	ObjectSelf - Supplies the object id to reference for the 'object self'
	             manifest constant.

	ObjectInvalid - Supplies the object id to reference for the 'object
	                invalid' manifest constant.

	Params - Supplies an optional parameter set to pass to the script
	         StartingConditional entry point.

	DefaultReturnCode - Supplies the default return code on an error condition,
	                    or if the script did not return a value.

	Flags - Supplies flags that control the execution environment of the
	        script.

Return Value:

	If the script is a StartingConditional, its return value is returned.
	Otherwise, the default return code is returned.

Environment:

	User mode.

--*/
{
	return ExecuteScriptInternal(
		Script,
		ObjectSelf,
		ObjectInvalid,
		m_State.Stack,
		(PROGRAM_COUNTER) 0,
		&Params,
		DefaultReturnCode,
		Flags);
}

void
NWScriptVM::ExecuteScriptSituation(
	__inout VMState & ScriptState
	)
/*++

Routine Description:

	This routine executes a script situation, which is a saved portion of a
	script that is later run (such as a delayed action).

Arguments:

	ScriptState - Supplies the state of the script to execute.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	(void) ExecuteScriptInternal(
		ScriptState.Script,
		ScriptState.ObjectSelf,
		ScriptState.ObjectInvalid,
		ScriptState.Stack,
		ScriptState.ProgramCounter,
		NULL,
		0,
		0);
}

void
NWScriptVM::AbortScript(
	)
/*++

Routine Description:

	This routine aborts execution of the entire script program, including any
	earlier nested calls.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	m_State.Aborted = true;
}

void
NWScriptVM::SetDebugLevel(
	__in ExecDebugLevel DebugLevel
	)
/*++

Routine Description:

	This routine changes the debug print filter level.  Lower numbered filter
	levels reduce the output sent to the user's IDebugTextOut interface.

Arguments:

	DebugLevel - Supplies the new debug level.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	m_DebugLevel = DebugLevel;
}

int
NWScriptVM::ExecuteScriptInternal(
	__in NWScriptReaderPtr & Script,
	__in NWN::OBJECTID ObjectSelf,
	__in NWN::OBJECTID ObjectInvalid,
	__inout NWScriptStack & VMStack,
	__in PROGRAM_COUNTER ProgramCounter,
	__in_opt const ScriptParamVec * Params,
	__in int DefaultReturnCode,
	__in ULONG Flags
	)
/*++

Routine Description:

	This routine begins execution of a bytecode script.  Both a saved script
	situation and a new script entry point are supported.

Arguments:

	Script - Supplies the script byte code to execute.

	ObjectSelf - Supplies the object id to reference for the 'object self'
	             manifest constant.

	ObjectInvalid - Supplies the object id to reference for the 'object
	                invalid' manifest constant.

	VMStack - Supplies the execution stack for the script.  The stack is
	          consumed during execution.

	ProgramCounter - Supplies the script initial program counter.

	Params - Supplies an optional parameter set to pass to the script
	         StartingConditional entry point.  If this parameter is NULL, then
	         a script situation is assumed and no entry point processing is
	         performed.

	DefaultReturnCode - Supplies the default return code on an error condition,
	                    or if the script did not return a value.

	Flags - Supplies flags that control the execution environment of the
	        script.

Return Value:

	If the script is a StartingConditional, its return value is returned.
	Otherwise, the default return code is returned.

Environment:

	User mode.

--*/
{
	bool           NeedFixup;
	int            ReturnCode;
	size_t         ReturnStackDepth;
#if ANALYZE_SCRIPT
	ScriptParamVec ReplaceParams;
#endif

	//
	// First, perform fixups if we need to.  Note that if we are entering into
	// a saved script situation, we cannot update the fixup state if this is
	// the first time we've seen the script since booting.  This is because we
	// would not know if it took any parameters without being the original
	// calling context.
	//

	if ((Script->GetPatchState( ) == NWScriptReader::NCSPatchState_Unknown) &&
	    (Params != NULL))
	{
#if ANALYZE_SCRIPT
		const NWScriptReader::ScriptAnalyzeState * AnalyzeState;

		//
		// If script analysis is enabled, analyze the script program structure
		// now so that we can authoritatively discover the calling convention
		// of the entry point symbol.
		//
		// N.B.  We must perform the analysis and replace parameters for the
		//       initial call here, or else we might mistakenly set the wrong
		//       match state if the initial call's parameters were incorrect.
		//

		AnalyzeScript( Script, Flags );

		//
		// First, if we have analysis enabled, check if we can adjust the
		// parameter count appropriately instead of relying on the additional
		// parameter hack.
		//

		if ((AnalyzeState = Script->GetAnalyzeState( )) != NULL)
		{
			size_t ExpectParams = (size_t) AnalyzeState->ParameterCells;

			//
			// If the parameter count didn't really match, copy the parameters
			// and adjust them to have the right count.  Any additional
			// parameters have a defaulted value (empty string / 0 / 0.0f).
			//

			if (ExpectParams != Params->size( ))
			{
				ReplaceParams = *Params;
				ReplaceParams.resize( ExpectParams );

				Params = &ReplaceParams;
			}

			//
			// Turn off the 'ignore stack mismatch' bit now that we have
			// ensured that the parameter count really matches.
			//

			Flags &= ~ESF_IGNORE_STACK_MISMATCH;
		}
#endif

		ApplyNCSFixups( Script.get( ), !Params->empty( ) );
	}
#if ANALYZE_SCRIPT
	else if ((Params != NULL) && (Script->GetAnalyzeState( ) == NULL))
	{
		const NWScriptReader::ScriptAnalyzeState * AnalyzeState;

		//
		// This may be a script that the caller preselected the patch state for
		// but we have not yet analyzed (for example, the caller knows that an
		// external caller already patched #loader).  Do the analysis now but
		// do not attempt to select the fixup state for the script.
		//

		AnalyzeScript( Script, Flags );

		if ((AnalyzeState = Script->GetAnalyzeState( )) != NULL)
		{
			size_t ExpectParams = (size_t) AnalyzeState->ParameterCells;

			//
			// If the parameter count didn't really match, copy the parameters
			// and adjust them to have the right count.  Any additional
			// parameters have a defaulted value (empty string / 0 / 0.0f).
			//

			if (ExpectParams != Params->size( ))
			{
				ReplaceParams = *Params;
				ReplaceParams.resize( ExpectParams );

				Params = &ReplaceParams;
			}

			//
			// Turn off the 'ignore stack mismatch' bit now that we have
			// ensured that the parameter count really matches.
			//

			Flags &= ~ESF_IGNORE_STACK_MISMATCH;
		}
	}
#endif

	//
	// Now begin to execute instructions.  First we will need to push the
	// parameters though (and the dummy return value integer if we engaged the
	// early patch for a StartingConditional).  We must only do this if we were
	// starting at the beginning of a script though, which is signified by a
	// non-NULL Params (though the params may be empty).
	//

	if (Params != NULL) 
	{
#if ANALYZE_SCRIPT
		const NWScriptReader::ScriptAnalyzeState * AnalyzeState;

		//
		// First, if we have analysis enabled, check if we can adjust the
		// parameter count appropriately instead of relying on the additional
		// parameter hack.
		//

		if ((AnalyzeState = Script->GetAnalyzeState( )) != NULL)
		{
			size_t ExpectParams = (size_t) AnalyzeState->ParameterCells;

			//
			// If the parameter count didn't really match, copy the parameters
			// and adjust them to have the right count.  Any additional
			// parameters have a defaulted value (empty string / 0 / 0.0f).
			//

			if (ExpectParams != Params->size( ))
			{
				ReplaceParams = *Params;
				ReplaceParams.resize( ExpectParams );

				Params = &ReplaceParams;
			}

			//
			// Turn off the 'ignore stack mismatch' bit now that we have
			// ensured that the parameter count really matches.
			//

			Flags &= ~ESF_IGNORE_STACK_MISMATCH;
		}
#endif

		//
		// Set up the default invalid object id for the VM stack.
		//

		VMStack.SetInvalidObjId( ObjectInvalid );

		//
		// If we nop'd out the return value push in the #loader, then effect
		// the return value store here and now.
		//

		switch (Script->GetPatchState( ))
		{

		case NWScriptReader::NCSPatchState_PatchReturnValue:

			//
			// We will push parameters immediately because we have NOP'd out
			// the RSADDI.
			//

			NeedFixup = false;
			break;

		default:
		case NWScriptReader::NCSPatchState_Normal:

			//
			// No modifications are necessary.
			//

			NeedFixup = false;
			break;

		case NWScriptReader::NCSPatchState_UsesGlobals:
			//
			// We need to defer the rest of the fixup until we execute the
			// first RSADDI after the first SAVEBP.  See ApplyNCSFixups for
			// more details.
			//

			if (!Params->empty( ))
				NeedFixup = true;
			else
				NeedFixup = false;
			break;

		}

		//
		// We are calling the start of a script (entry point), so start at
		// PC = 0.
		//

		Script->SetInstructionPointer( 0 );
	}
	else
	{
		//
		// We are resuming a saved script situation.  Never attempt to apply
		// any sort of fix ups and start out where we left off before for the
		// program counter.
		//

		Script->SetInstructionPointer( ProgramCounter );

		NeedFixup = false;
	}

	//
	// Now check reentrancy and execute the instruction stream.
	//

	if (m_RecursionLevel >= MAX_SCRIPT_RECURSION)
	{
		DebugPrint(
			EDL_Errors,
			"NWScriptVM::ExecuteScriptInternal( %s ): Maximum recursion level reached, aborting.\n",
			Script->GetScriptName( ).c_str( ));

		AbortScript( );

		if (Flags & ESF_RAISE_ON_EXEC_FAILURE)
			throw std::runtime_error( "script VM reached maximum recursion limit" );

		return DefaultReturnCode;
	}

	if (IsScriptAborted( ))
	{
		if (Flags & ESF_RAISE_ON_EXEC_FAILURE)
			throw std::runtime_error( "script is already aborted" );

		return DefaultReturnCode;
	}

	m_RecursionLevel += 1;

	if (IsDebugLevel( EDL_Calls ))
	{
		if (Params == NULL)
		{
			DebugPrint(
				EDL_Calls,
				"NWScriptVM::ExecuteScriptInternal( %s ): Executing script situation (PC = %08X)...\n",
				Script->GetScriptName( ).c_str( ),
				ProgramCounter);
		}
		else
		{
			DebugPrint(
				EDL_Calls,
				"NWScriptVM::ExecuteScriptInternal( %s ): Executing script with %lu parameters (recursion level = %lu)...\n",
				Script->GetScriptName( ).c_str( ),
				(unsigned long) Params->size( ),
				(unsigned long) m_RecursionLevel);
		}
	}

	ReturnStackDepth = VMStack.GetReturnStackDepth( );

	try
	{
		ReturnCode = ExecuteInstructions(
			Script,
			ObjectSelf,
			ObjectInvalid,
			VMStack,
			Params,
			NeedFixup,
			DefaultReturnCode,
			Flags);
	}
	catch (std::exception &e)
	{
		DebugPrint(
			EDL_Errors,
			"NWScriptVM::ExecuteScriptInternal( %s ): Exception '%s' executing script.\n",
			Script->GetScriptName( ).c_str( ),
			e.what( ));

		//
		// Print a stack trace to the user, carefully not going below the initial
		// stack depth (as below that we may have pointers to other scripts).
		//

		if (IsDebugLevel( EDL_Errors ))
		{
			for (size_t CallDepth = VMStack.GetReturnStackDepth( ) - 1;
			     CallDepth != ReturnStackDepth;
			     CallDepth -= 1)
			{
				PROGRAM_COUNTER RetPC = VMStack.GetReturnStackEntry( CallDepth );
				std::string     RetSymbol;

				if (!Script->GetSymbolName( RetPC, RetSymbol, true ))
					RetSymbol.clear( );

				DebugPrint(
					EDL_Errors,
					"NWScriptVM::ExecuteScriptInternal( %s ): ... called from PC=%08X (%s)\n",
					Script->GetScriptName( ).c_str( ),
					RetPC,
					RetSymbol.c_str( ));
			}
		}

		if (Flags & ESF_RAISE_ON_EXEC_FAILURE)
		{
			//
			// If we're to raise an exception on failure, exit the VM immediately
			// and re-raise the underlying exception.
			//

			ExitVM( VMStack );
			throw;
		}

		ReturnCode = 0;
	}

	if (IsDebugLevel( EDL_Calls ))
	{
		DebugPrint(
			EDL_Calls,
			"NWScriptVM::ExecuteScriptInternal( %s ): Script returned %d.\n",
			Script->GetScriptName( ).c_str( ),
			ReturnCode);
	}

	//
	// Exit the script VM, decrementing the recursion level and returning the
	// state to its prior form.
	//

	ExitVM( VMStack );

	return ReturnCode;
}

void
NWScriptVM::DebugPrint(
	__in ExecDebugLevel Level,
	__in __format_string const char * Fmt,
	...
	) const
/*++

Routine Description:

	This routine prints debug text to the user.

Arguments:

	Level - Supplies the debug log level (for filtering).

	Fmt - Supplies the format string.

	... - Optionally supplies printf-style format inserts.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	va_list ap;

	if (!IsDebugLevel( Level ))
		return;

	va_start( ap, Fmt );
	m_TextOut->WriteTextV( Fmt, ap );
	va_end( ap );
}

void
NWScriptVM::ApplyNCSFixups(
	__in NWScriptReader * Script,
	__in bool HasParams
	) const
/*++

Routine Description:

	This routine performs fixups (if necessary) for a script.  The script will
	be modified if fixups are required.

	Fixups are necessary to support parameterized conditional scripts, which
	have a loader routine that interferes with parameter passing.

	The fixup state is stored in the script's "patch state".

Arguments:

	Script - Supplies the script to inspect (and potentially fixup).

	HasParams - Supplies a Boolean value that indicates if the scirpt takes any
	            external parameters (true) or not (false).

Return Value:

	None.

Environment:

	User mode.

--*/
{
	UCHAR Opcode;
	UCHAR TypeOpcode;
	ULONG PCOffset;
	bool  HasGlobals;

	//
	// No fix ups are required if we don't have parameters for the entry point
	// symbol.
	//

	if (!HasParams)
	{
		Script->SetPatchState( NWScriptReader::NCSPatchState_Normal );
		return;
	}

	Script->SetInstructionPointer( 0 );

	if (Script->ScriptIsEof( ))
	{
		Script->SetPatchState( NWScriptReader::NCSPatchState_Normal );
		return;
	}

	//
	// The fixup we need to do is as follows:
	//
	// Normally, a script with a return value would have the following stack on
	// entry, were it to take two parameters:
	//
	// [sp-04] Parameter 0
	// [sp-08] Parameter 1
	// [sp-0c] Return value (not yet initialized)
	//
	// However, StartingConditional scripts, which return 0, are a special case
	// because they do not have a uniform set of parameters.  Unfortunately
	// the compiler codes in a #loader routine (the first instruction in the
	// file for a simple StartingConditional script) that reserves space for
	// the return value.  This is set in place in the script code stream.
	//
	// The problem arises if we would like to pass parameters to the script,
	// because now we have the return value that the #loader routine would set
	// in place in the way.  The result if we were to call the script directly
	// would be this:
	//
	// [sp-04] Return value << Pushed by #loader
	// [sp-08] Parameter 0  << Pushed by NWScriptVM
	// [sp-0c] Parameter 1  << Pushed by NWScriptVM
	//
	// Clearly, this does not work as the return value and parameters are not
	// right.  This is a result of StartingConditional scripts not really
	// having been designed to take parameters to begin with.
	//
	// As a result we must arrange for the #loader routine's push to be nop'd
	// out (effectively) and we must then push the return value manually, THEN
	// the parameters, onto the stack.
	//
	// This is accomplished by detecting #loader's first instruction, then
	// patching it with a NOP and manually pushing the return value and THEN
	// the parameters.  This is performed only if we have no globals.
	//
	// But there is a further complication!  If the script uses global
	// variables, we cannot do this immediately, because the global variables
	// are actually stored at the stack (and thus pushing the parameters now
	// would cause them to be part of the global variable set).  Thus, if the
	// script uses global variables, we must defer the parameter pushing until
	// #loader and #globals actually run.
	//
	// Even worse, in the case where we have globals, we *still* start out in
	// the #loader (which executes an RSADDI to push the return value onto the
	// stack).
	//
	// Thus in the globals case, we wait for the first RSADDI *AFTER* the first
	// SAVEBP, and only THEN do we we patch things up.  The #globals routine is
	// followed by YET ANOTHER stack push which is the one that we would want
	// to intercept, for a StartingConditional with parameters.
	//

	HasGlobals = ScriptHasGlobals( Script );

	//
	// Check if the first instruction is part of the #loader routine.  If so,
	// then we have a StartingConditional.  We have to fix this up because the
	// calling convention requires the return value to be on the wrong side of
	// the arguments passed in (and the loader creates the return value).
	//
	// If we have globals in use then we need to defer our fixup hackery until
	// after global variables are setup though.
	//

	Script->SetInstructionPointer( 0 );

	DecodeInstruction( Script, Opcode, TypeOpcode, PCOffset );

	DebugPrint(
		EDL_Verbose,
		"NWScriptVM::ApplyNCSFixups: Script Opcode=%02X:%02X (HasGlobals %lu).\n",
		Opcode,
		TypeOpcode,
		HasGlobals ? 1 : 0);

	//
	// The first instruction for a #loader is always an RSADDI.  Check for it.
	//

	if ((Opcode == OP_RSADD)           &&
	    (TypeOpcode == TYPE_UNARY_INT) &&
	    (!HasGlobals))
	{
		//
		// Yes, this is #loader and we have no globals.  Edit it here.
		//

		Script->SetPatchState( NWScriptReader::NCSPatchState_PatchReturnValue );

		Script->PatchBYTE( 0, OP_NOP );
		Script->PatchBYTE( 1, TYPE_UNARY_NONE ); // Really the type opcode.

		DebugPrint(
			EDL_Verbose,
			"NWScriptVM::ApplyNCSFixups: Patching #loader immediately.\n");
	}
	else if (HasGlobals)
	{
		Script->SetPatchState( NWScriptReader::NCSPatchState_UsesGlobals );

		DebugPrint(
			EDL_Verbose,
			"NWScriptVM::ApplyNCSFixups: Deferring parameter push until after #globals.\n");
	}
	else
	{
		//
		// No patching of the script is necessary, instructions should be run
		// as normal.
		//

		Script->SetPatchState( NWScriptReader::NCSPatchState_Normal );
	}
}

bool
NWScriptVM::ScriptHasGlobals(
	__in NWScriptReader * Script
	) const
/*++

Routine Description:

	This routine checks if a script uses global variables.  The check is
	performed by scanning the entire opcode stream for a SAVEBP instruction.

Arguments:

	Script - Supplies the script to inspect.

Return Value:

	The routine returns true if the script uses global variables, else it
	returns false.

Environment:

	User mode.

--*/
{
	//
	// Scan forward for a SAVEBP.  All global variable usage is required to use
	// SAVEBP to set up the global variable base pointer frame.
	//

	Script->SetInstructionPointer( 0 );

	while (!Script->ScriptIsEof( ))
	{
		UCHAR Opcode;
		UCHAR TypeOpcode;
		ULONG Len;
		ULONG Offset;

		Len = DecodeInstruction( Script, Opcode, TypeOpcode, Offset );

		if (Len < Offset)
			break;

		if (Opcode == OP_SAVEBP)
			return true;

		Script->AdvanceInstructionPointer( Len - Offset );
	}

	return false;
}

ULONG
NWScriptVM::DecodeInstruction(
	__in NWScriptReader * Script,
	__out UCHAR & Opcode,
	__out UCHAR & TypeOpcode,
	__out ULONG & PCOffset
	)
/*++

Routine Description:

	This routine determines the length of the instruction at the current PC,
	including any parameters to the instruction.

Arguments:

	Script - Supplies the script to decode the instruction for.  The current
	         program counter of the script is used.

	Opcode - Receives the instruction opcode.

	TypeOpcode - Receives the instruction type opcode.

	PCOffset - Receives the offset of the new PC relative to the start of the
	           instruction when the routine returns.  Generally, the routine
	           returns 2 bytes into the instruction, but not always.

Return Value:

	The routine returns the length of the instruction (including the opcode
	bytes).  On failure, an std::exception is returned.

Environment:

	User mode.

--*/
{
	PCOffset = 2; // We always read at least the opcode and type opcode.

	Script->ReadInstruction( Opcode, TypeOpcode );

	switch (Opcode)
	{

	case OP_CPDOWNSP:
		return 8;
	case OP_RSADD:
		return 2;
	case OP_CPTOPSP:
		return 8;

	case OP_CONST:
		switch (TypeOpcode)
		{

		case TYPE_UNARY_INT:
		case TYPE_UNARY_FLOAT:
		case TYPE_UNARY_OBJECTID:
			return 6;

		case TYPE_UNARY_STRING:
			PCOffset += 2; // We point after the length field.
			return 4 + Script->ReadINT16( );

		default:
			if ((TypeOpcode >= TYPE_UNARY_ENGINE_FIRST) &&
			    (TypeOpcode <= TYPE_UNARY_ENGINE_LAST))
			{
				return 6;
			}

			throw std::runtime_error( "Illegal OP_CONST." );

		}
		break;

	case OP_ACTION:
		return 5;
	case OP_LOGAND:
		return 2;
	case OP_LOGOR:
		return 2;
	case OP_INCOR:
		return 2;
	case OP_EXCOR:
		return 2;
	case OP_BOOLAND:
		return 2;
	case OP_EQUAL:
		if (TypeOpcode >= TYPE_BINARY_FIRST && TypeOpcode < TYPE_BINARY_STRUCTSTRUCT)
			return 2;
		else if (TypeOpcode == TYPE_BINARY_STRUCTSTRUCT)
			return 4;
		else if (TypeOpcode >= TYPE_BINARY_ENGINE_FIRST && TypeOpcode <= TYPE_BINARY_ENGINE_LAST)
			return 2;
		else
			throw std::runtime_error( "Illegal OP_EQUAL." );
		break;

	case OP_NEQUAL:
		if (TypeOpcode >= TYPE_BINARY_FIRST && TypeOpcode < TYPE_BINARY_STRUCTSTRUCT)
			return 2;
		else if (TypeOpcode == TYPE_BINARY_STRUCTSTRUCT)
			return 4;
		else if (TypeOpcode >= TYPE_BINARY_ENGINE_FIRST && TypeOpcode <= TYPE_BINARY_ENGINE_LAST)
			return 2;
		else
			throw std::runtime_error( "Illegal OP_NEQUAL." );
		break;

	case OP_GEQ:
		return 2;
	case OP_GT:
		return 2;
	case OP_LT:
		return 2;
	case OP_LEQ:
		return 2;
	case OP_SHLEFT:
		return 2;
	case OP_SHRIGHT:
		return 2;
	case OP_USHRIGHT:
		return 2;
	case OP_ADD:
		return 2;
	case OP_SUB:
		return 2;
	case OP_MUL:
		return 2;
	case OP_DIV:
		return 2;
	case OP_MOD:
		return 2;
	case OP_NEG:
		return 2;
	case OP_COMP:
		return 2;
	case OP_MOVSP:
		return 6;
	case OP_STORE_STATEALL:
		return 2;
	case OP_JMP:
		return 6;
	case OP_JSR:
		return 6;
	case OP_JZ:
		return 6;
	case OP_RETN:
		return 2;
	case OP_DESTRUCT:
		return 8;
	case OP_NOT:
		return 2;
	case OP_DECISP:
		return 6;
	case OP_INCISP:
		return 6;
	case OP_JNZ:
		return 6;
	case OP_CPDOWNBP:
		return 8;
	case OP_CPTOPBP:
		return 8;
	case OP_DECIBP:
		return 6;
	case OP_INCIBP:
		return 6;
	case OP_SAVEBP:
		return 2;
	case OP_RESTOREBP:
		return 2;
	case OP_STORE_STATE:
		return 10;
	case OP_NOP:
		return 2;
	case OP_T:
		return 5;
	default:
		throw std::runtime_error( "Unrecognized opcode." );

	}
}

int
NWScriptVM::ExecuteInstructions(
	__in NWScriptReaderPtr & Script,
	__in NWN::OBJECTID ObjectSelf,
	__in NWN::OBJECTID ObjectInvalid,
	__inout NWScriptStack & VMStack,
	__in_opt const ScriptParamVec * Params,
	__in bool NeedFixup,
	__in int DefaultReturnCode,
	__in ULONG Flags
	)
/*++

Routine Description:

	This routine executes an instruction stream in a script.  It forms the main
	worker routine of the virtual machine.

Arguments:

	Script - Supplies the script byte code to execute.  The script's PC must be
	         prepositioned at the corret location.

	ObjectSelf - Supplies the object id to reference for the 'object self'
	             manifest constant.

	ObjectInvalid - Supplies the object id to reference for the 'object
	                invalid' manifest constant.

	VMStack - Supplies the execution stack for the script.  The stack is
	          consumed during execution.

	Params - Supplies an optional parameter set to pass to the script
	         StartingConditional entry point.  If this parameter is NULL, then
	         a script situation is assumed and no entry point processing is
	         performed.

	NeedFixup - Supplies a Boolean value that indicates whether the fixup for
	            StartingConditionals with #globals must be applied (true) or
	            not (false).  Note that even if this parameter is true, we
	            might be executing a function that does not return a value, and
	            we need to handle that case here.

	DefaultReturnCode - Supplies the default return code on an error condition,
	                    or if the script did not return a value.

	Flags - Supplies flags that control the execution environment of the
	        script.

Return Value:

	If the script is a StartingConditional, its return value is returned.
	Otherwise, the default return code is returned.

	Should a catastrophic failure (i.e. out of memory) occur, or should the
	script program be ill-formed, then an std::exception is raised.

Environment:

	User mode.

--*/
{
	enum
	{
		FixupState_WaitingForGlobals,
		FixupState_WaitingForStartingConditional,
		FixupState_GotStartingConditional,
		FixupState_Done
	}               FixupState;
	STACK_POINTER   StartSP;
	STACK_POINTER   EndSP;
	UCHAR           Opcode;
	UCHAR           TypeOpcode;
	ULONG           PCOffset;
	ULONG           InstructionLength;
	ULONG           BPNestingLevel;
	size_t          ReturnStackDepth;
	PROGRAM_COUNTER PC;
	bool            DebugVerbose;
	bool            NoReturnValue;
	bool            ExpectReturnValue;
	std::string     SymbolName;

	//
	// N.B.  NoReturnValue and ExpectReturnValue can only be conclusively
	//       identified if we are in fixup mode.  Otherwise we don't know for
	//       sure until we reach the end and have an extra int on the stack.
	//

	NoReturnValue     = false;
	ExpectReturnValue = false;
	BPNestingLevel    = 0;
	ReturnStackDepth  = VMStack.GetReturnStackDepth( );

	DebugVerbose = IsDebugLevel( EDL_Verbose );

	if (!NeedFixup)
		FixupState = FixupState_Done;
	else
		FixupState = FixupState_WaitingForGlobals;

	//
	// Record our current SP.  We may be unbalanced by one int at the end if we
	// were a StartingConditional, in which case we would need to pull the
	// return value off of the stack.
	//

	StartSP = VMStack.GetCurrentSP( );
	PC      = (PROGRAM_COUNTER) Script->GetInstructionPointer( );

	//
	// If we do not need to defer parameter pushing for the fixup, then do the
	// parameter push now.
	//

	if (Params != NULL)
	{
		switch (Script->GetPatchState( ))
		{

		case NWScriptReader::NCSPatchState_PatchReturnValue:

			//
			// Make room for the return address.
			//

			VMStack.StackPushInt( 0 );

			//
			// Now we can push parameters on.  Because we do not initially know
			// what the parameter types are, we have to push them as
			// dynamically typed parameters (which are really strings that get
			// converted on the fly when referenced via a specific type).
			//
			// If we wanted, we could compile the script now, if we had the
			// source code, such that we could discover the prototype.  It is
			// cleaner to not require source code (and to not require that the
			// compiler be embedded), however, so we use the dynamic typing
			// system instead.
			//

			PushEntrypointParameters( Params, Script, VMStack, Flags );
			break;

		case NWScriptReader::NCSPatchState_Normal:

			//
			// This script does not require any fixups at all.  Immediately push
			// the parameters on.
			//

			PushEntrypointParameters( Params, Script, VMStack, Flags );

			break;
		}
	}

	//
	// Loop executing instructions.
	//

	while (!Script->ScriptIsEof( ))
	{
		if (++m_InstructionsExecuted > MAX_SCRIPT_INSTRUCTIONS)
		{
			DebugPrint(
				EDL_Errors,
				"NWScriptVM::ExecuteInstructions( %s ): Exceeded instruction limit at PC=%08X.\n",
				Script->GetScriptName( ).c_str( ),
				PC);

			throw std::runtime_error( "Too many script instructions." );
		}

		//
		// Decode and dispatch the instruction.
		//

		InstructionLength = DecodeInstruction(
			Script.get( ),
			Opcode,
			TypeOpcode,
			PCOffset);

		if (FixupState == FixupState_WaitingForStartingConditional)
		{
			//
			// If we did not have an RSADD after #globals then this is not a
			// StartingConditional.  This means there should be no return value
			// and we have no further fixup processing to do (but we need to
			// push the parameters finally).
			//

			if (Opcode != OP_RSADD)
			{
				FixupState    = FixupState_Done;
				NoReturnValue = true;

				DebugPrint(
					EDL_Verbose,
					"NWScriptVM::ExecuteInstructions( %s ): RSADD found for fixup, pushing parameters.\n",
					Script->GetScriptName( ).c_str( ));

				PushEntrypointParameters( Params, Script, VMStack, Flags );
			}
			else
			{
				//
				// Otherwise we are ready to perform the last stage of the
				// fixup.  Wait for an RSADDI (which may be this RSADD).
				//

				FixupState = FixupState_WaitingForStartingConditional;

				DebugPrint(
					EDL_Verbose,
					"NWScriptVM::ExecuteInstructions( %s ): Waiting for RSADDI.\n",
					Script->GetScriptName( ).c_str( ));

				FixupState = FixupState_GotStartingConditional;
			}
		}

#if VM_DEBUGGER
		//
		// If we are in verbose mode then show our current state for tracing.
		//

		if (DebugVerbose)
		{
			enum { DBGSTACK = 3 };

			char  TopStack[ 256 ];
			ULONG RawStack[ DBGSTACK ];
			UCHAR RawStackType[ DBGSTACK ];
			ULONG RawStackValid;
			bool  HaveSymbol;

			TopStack[ 0 ] = '\0';

			//
			// Capture the first few values at the top of the stack.
			//

			for (RawStackValid = 0;
			     RawStackValid < DBGSTACK;
			     RawStackValid += 1)
			{
				char                           StackContents[ 64 ];
				NWScriptStack::BASE_STACK_TYPE BaseType;

				if (!VMStack.PeekStack(
					VMStack.GetCurrentSP( ) - VMStack.GetStackIntegerSize( ) * (1 + RawStackValid),
					RawStack[ RawStackValid ],
					RawStackType[ RawStackValid ]))
				{
					break;
				}

				if (VMStack.DebugIsEngineStructureType( RawStackType[ RawStackValid ] ))
				{
					BaseType = NWScriptStack::BST_INVALID;
				}
				else
				{
					BaseType = VMStack.GetStackType(
						VMStack.GetCurrentSP( ) - VMStack.GetStackIntegerSize( ) * (1 + RawStackValid));
				}

				switch (BaseType)
				{

				case NWScriptStack::BST_STRING:
					StringCbPrintfA(
						StackContents,
						sizeof( StackContents ),
						"%08X.%02X<\"%s\"> ",
						RawStack[ RawStackValid ],
						RawStackType[ RawStackValid ],
						VMStack.GetStackString( -VMStack.GetStackIntegerSize( ) * (1 + RawStackValid) ).c_str( ));
					break;

				default:
					StringCbPrintfA(
						StackContents,
						sizeof( StackContents ),
						"%08X.%02X ",
						RawStack[ RawStackValid ],
						RawStackType[ RawStackValid ]);
					break;

				}

				StringCbCatA(
					TopStack,
					sizeof( TopStack ),
					StackContents);
			}

			if (Script->GetSymbolName( PC, SymbolName, true ))
			{
				DebugPrint(
					EDL_Verbose,
					"NWScriptVM::ExecuteInstructions( %s ): PC=%08X(%s): %02X.%02X   %s%s   [SP=%08X BP=%08X]  S=%s\n",
					Script->GetScriptName( ).c_str( ),
					PC,
					SymbolName.c_str( ),
					Opcode,
					TypeOpcode,
					GetInstructionName( Opcode ),
					GetTypeOpcodeName( TypeOpcode ),
					VMStack.GetCurrentSP( ),
					VMStack.GetCurrentBP( ),
					TopStack);

				HaveSymbol = true;
			}
			else
			{
				DebugPrint(
					EDL_Verbose,
					"NWScriptVM::ExecuteInstructions( %s ): PC=%08X: %02X.%02X   %s%s   [SP=%08X BP=%08X]  S=%s\n",
					Script->GetScriptName( ).c_str( ),
					PC,
					Opcode,
					TypeOpcode,
					GetInstructionName( Opcode ),
					GetTypeOpcodeName( TypeOpcode ),
					VMStack.GetCurrentSP( ),
					VMStack.GetCurrentBP( ),
					TopStack);

				HaveSymbol = false;
			}

			VMDebuggerCheckForBreakpoint(
				Script->GetScriptName( ).c_str( ),
				PC,
				VMStack,
				HaveSymbol ? SymbolName.c_str( ) : NULL);
		}
#endif

		switch (Opcode)
		{

		case OP_CPDOWNSP: // Copy down SP (assignment operator)
			{
				STACK_POINTER Offset;
				STACK_POINTER Size;

				Offset = (STACK_POINTER) Script->ReadINT32( );
				Size   = (STACK_POINTER) Script->ReadINT16( );

				Offset = STACK_PTR( Offset );
				Size   = STACK_PTR( Size );

				VMStack.CopyDownSP(
					Offset,
					Size);
			}
			break;

		case OP_RSADD: // Reserve uninitialized space on the stack
			{
				switch (TypeOpcode)
				{

				case TYPE_UNARY_INT:
					VMStack.StackPushInt( 0 );

					//
					// We need to check if this was the first RSADDI after we
					// got into StartingConditional.  If so then we need to
					// push parameters if we've got them.  Note that we only
					// enter this code path if we had parameters in the first
					// place.
					//

					if (FixupState == FixupState_GotStartingConditional)
					{
						DebugPrint(
							EDL_Verbose,
							"NWScriptVM::ExecuteInstructions( %s ): RSADDI found for fixup, pushing parameters.\n",
							Script->GetScriptName( ).c_str( ));

						PushEntrypointParameters(
							Params,
							Script,
							VMStack,
							Flags);

						FixupState        = FixupState_Done;
						ExpectReturnValue = true;
					}
					break;

				case TYPE_UNARY_FLOAT:
					VMStack.StackPushFloat( 0.0f );
					break;

				case TYPE_UNARY_STRING:
					VMStack.StackPushString( "" );
					break;

				case TYPE_UNARY_OBJECTID:
					VMStack.StackPushObjectId( ObjectInvalid );
					break;

				default:
					//
					// This may be a create engine structure request; do that
					// now if it really was.
					//

					if ((TypeOpcode >= TYPE_UNARY_ENGINE_FIRST) &&
					    (TypeOpcode <= TYPE_UNARY_ENGINE_LAST))
					{
						EngineStructurePtr EngineStruct;

						EngineStruct = m_ActionHandler->CreateEngineStructure(
							(NWScriptStack::ENGINE_STRUCTURE_NUMBER) (TypeOpcode - TYPE_UNARY_ENGINE_FIRST));

						if (EngineStruct.get( ) == NULL)
						{
							DebugPrint(
								EDL_Errors,
								"NWScriptVM::ExecuteInstructions( %s ): @%08X: Failed to create engine structure %lu.\n",
								Script->GetScriptName( ).c_str( ),
								PC,
								(NWScriptStack::ENGINE_STRUCTURE_NUMBER) (TypeOpcode - TYPE_UNARY_ENGINE_FIRST));

							throw std::runtime_error( "Failed to create engine structure." );
						}

						VMStack.StackPushEngineStructure( EngineStruct );
						break;
					}

					DebugPrint(
						EDL_Errors,
						"NWScriptVM::ExecuteInstructions( %s ): @%08x: RSADD.%02X not supported.\n",
						Script->GetScriptName( ).c_str( ),
						PC,
						TypeOpcode);

					throw std::runtime_error( "Unimplemented RSADD" );

				}
			}
			break;

		case OP_CPTOPSP: // Read / duplicate local variables
			{
				STACK_POINTER Offset;
				STACK_POINTER Size;

				Offset = (STACK_POINTER) Script->ReadINT32( );
				Size   = (STACK_POINTER) Script->ReadINT16( );

				Offset = STACK_PTR( Offset );
				Size   = STACK_PTR( Size );

				VMStack.CopyTopSP( Offset, Size );
			}
			break;
			
		case OP_CONST: // Push a constant onto the stack
			{
				switch (TypeOpcode)
				{

				case TYPE_UNARY_INT:
					VMStack.StackPushInt( (int) Script->ReadINT32( ) );
					break;

				case TYPE_UNARY_FLOAT:
					VMStack.StackPushFloat( Script->ReadFLOAT( ) );
					break;

				case TYPE_UNARY_STRING:
					VMStack.StackPushString(
						Script->ReadString(
							InstructionLength - 4
							)
						);
					break;

				case TYPE_UNARY_OBJECTID:
					{
						NWN::OBJECTID ObjectId;

						ObjectId = (NWN::OBJECTID) Script->ReadINT32( );

						switch (ObjectId)
						{

						case OBJECTID_SELF:
							VMStack.StackPushObjectId( ObjectSelf );
							break;

						case OBJECTID_INVALID:
							VMStack.StackPushObjectId( ObjectInvalid );
							break;

						default:
							//
							// We really should never see anything other than
							// the two symbolic constants here.  Unfortunately
							// there is a case where we do see the invalid
							// object id (actual value) here, so we must accept
							// that too.  But complain loudly if we see any
							// other value, as it would almost certainly be
							// very wrong (dynamically assigned!).
							//

							if (ObjectId != ObjectInvalid)
							{
								DebugPrint(
									EDL_Errors,
									"NWScriptVM::ExecuteInstructions( %s ): @%08X: Hardcoding dangerous object id %08X in CONSTO.\n",
									Script->GetScriptName( ).c_str( ),
									PC,
									(unsigned long) ObjectId);
							}

							VMStack.StackPushObjectId( ObjectId );
							break;

						}

					}
					break;

				default:
					//
					// This may be a create engine structure request; do that
					// now if it really was.
					//

					if ((TypeOpcode >= TYPE_UNARY_ENGINE_FIRST) &&
					    (TypeOpcode <= TYPE_UNARY_ENGINE_LAST))
					{
						EngineStructurePtr EngineStruct;

						m_CurrentActionObjectSelf = ObjectSelf;

						EngineStruct = m_ActionHandler->CreateEngineStructure(
							(NWScriptStack::ENGINE_STRUCTURE_NUMBER) (TypeOpcode - TYPE_UNARY_ENGINE_FIRST));

						if (EngineStruct.get( ) == NULL)
						{
							DebugPrint(
								EDL_Errors,
								"NWScriptVM::ExecuteInstructions( %s ): @%08X: Failed to create engine structure %lu.\n",
								Script->GetScriptName( ).c_str( ),
								(NWScriptStack::ENGINE_STRUCTURE_NUMBER) (TypeOpcode - TYPE_UNARY_ENGINE_FIRST));

							throw std::runtime_error( "Failed to create engine structure." );
						}

						VMStack.StackPushEngineStructure( EngineStruct );
						break;
					}

					DebugPrint(
						EDL_Errors,
						"NWScriptVM::ExecuteInstructions( %s ): @%08x: CONST.%02X not supported.\n",
						Script->GetScriptName( ).c_str( ),
						PC,
						TypeOpcode);

					throw std::runtime_error( "Unimplemented CONST" );

				}
			}
			break;

		case OP_ACTION: // Call an engine action API
			{
				NWSCRIPT_ACTION ActionId;
				size_t          ArgumentCount;

				ActionId      = (NWSCRIPT_ACTION) Script->ReadINT16( );
				ArgumentCount = (size_t) Script->ReadINT8( );

				m_CurrentActionObjectSelf = ObjectSelf;

				//
				// Dispatch to the action handler for this action.
				//

				m_ActionHandler->OnExecuteAction(
					*this,
					VMStack,
					ActionId,
					ArgumentCount);

				//
				// If the action recursively called this script then the active
				// PC may have been reset.  Ensure that it is correct now.
				//

				Script->SetInstructionPointer( PC + InstructionLength );

				if (IsScriptAborted( ))
					throw std::runtime_error( "Script program execution abortively terminated." );
			}
			break;

		case OP_LOGAND: // Perform logical AND (&&)
			{
				int i1;
				int i2;

				if (TypeOpcode != TYPE_BINARY_INTINT)
				{
					DebugPrint(
						EDL_Errors,
						"NWScriptVM::ExecuteInstructions( %s ): @%08X: LOGAND.%02X not supported.\n",
						Script->GetScriptName( ).c_str( ),
						PC,
						TypeOpcode);

					throw std::runtime_error( "Unsupported LOGAND" );
				}

				i1 = VMStack.StackPopInt( );
				i2 = VMStack.StackPopInt( );

				VMStack.StackPushInt(
					(i2 && i1));
			}
			break;

		case OP_LOGOR: // Perform logical OR (||)
			{
				int i1;
				int i2;

				if (TypeOpcode != TYPE_BINARY_INTINT)
				{
					DebugPrint(
						EDL_Errors,
						"NWScriptVM::ExecuteInstructions( %s ): @%08X: LOGOR.%02X not supported.\n",
						Script->GetScriptName( ).c_str( ),
						PC,
						TypeOpcode);

					throw std::runtime_error( "Unsupported LOGOR" );
				}

				i1 = VMStack.StackPopInt( );
				i2 = VMStack.StackPopInt( );

				VMStack.StackPushInt(
					(i2 || i1 ));
			}
			break;

		case OP_INCOR: // Perform bitwise OR (|)
			if (TypeOpcode != TYPE_BINARY_INTINT)
			{
				DebugPrint(
					EDL_Errors,
					"NWScriptVM::ExecuteInstructions( %s ): @%08X: INCOR.%02X not supported.\n",
					Script->GetScriptName( ).c_str( ),
					PC,
					TypeOpcode);

				throw std::runtime_error( "Unsupported INCOR" );
			}

			VMStack.StackPushInt(
				(VMStack.StackPopInt( ) | VMStack.StackPopInt( )));
			break;

		case OP_EXCOR: // Perform bitwise XOR (^)
			if (TypeOpcode != TYPE_BINARY_INTINT)
			{
				DebugPrint(
					EDL_Errors,
					"NWScriptVM::ExecuteInstructions( %s ): @%08X: EXCOR.%02X not supported.\n",
					Script->GetScriptName( ).c_str( ),
					PC,
					TypeOpcode);

				throw std::runtime_error( "Unsupported EXCOR" );
			}

			VMStack.StackPushInt(
				(VMStack.StackPopInt( ) ^ VMStack.StackPopInt( )));
			break;

		case OP_BOOLAND: // Perform bitwise AND (&)
			if (TypeOpcode != TYPE_BINARY_INTINT)
			{
				DebugPrint(
					EDL_Errors,
					"NWScriptVM::ExecuteInstructions( %s ): @%08X: BOOLAND.%02X not supported.\n",
					Script->GetScriptName( ).c_str( ),
					PC,
					TypeOpcode);

				throw std::runtime_error( "Unsupported BOOLAND" );
			}

			VMStack.StackPushInt(
				(VMStack.StackPopInt( ) & VMStack.StackPopInt( )));
			break;

		case OP_EQUAL: // Compare (==)
		case OP_NEQUAL: // Compare (!=)
			{
				bool IsEqual = false;

				switch (TypeOpcode)
				{

				case TYPE_BINARY_INTINT:
					IsEqual = (VMStack.StackPopInt( ) == VMStack.StackPopInt( ));
					break;

				case TYPE_BINARY_FLOATFLOAT:
					IsEqual = (VMStack.StackPopFloat( ) == VMStack.StackPopFloat( ));
					break;

				case TYPE_BINARY_OBJECTIDOBJECTID:
					IsEqual = NWN::EqualObjectId( VMStack.StackPopObjectId( ), VMStack.StackPopObjectId( ) );
					break;

				case TYPE_BINARY_STRINGSTRING:
					IsEqual = (VMStack.StackPopString( ) == VMStack.StackPopString( ));
					break;

				case TYPE_BINARY_STRUCTSTRUCT:
					{
						USHORT Size;

						Size  = Script->ReadINT16( );

						Size  = STACK_PTR( Size );

						//
						// Check that the extents of the comparison do not trip
						// a guard zone first.
						//

						VMStack.CheckGuardZone( (2 * -((STACK_POINTER) Size)) + VMStack.GetCurrentSP( ) );

						//
						// Compare an arbitrary count of elements on the stack.
						//

						for (USHORT Offset = 0;
						     Offset < Size;
						     Offset += (USHORT) VMStack.GetStackIntegerSize( ))
						{
							STACK_POINTER   Offset1;
							STACK_POINTER   Offset2;
							BASE_STACK_TYPE Type;

							Offset1 = (1 * -((STACK_POINTER) Size)) + Offset;
							Offset2 = (2 * -((STACK_POINTER) Size)) + Offset;

							switch ((Type = VMStack.GetStackType( VMStack.GetCurrentSP( ) + Offset1 )))
							{

							case NWScriptStack::BST_INT:
								IsEqual = (VMStack.GetStackInt( Offset1 ) == VMStack.GetStackInt( Offset2 ) );
								break;

							case NWScriptStack::BST_FLOAT:
								IsEqual = (VMStack.GetStackFloat( Offset1 ) == VMStack.GetStackFloat( Offset2 ) );
								break;

							case NWScriptStack::BST_OBJECTID:
								IsEqual = NWN::EqualObjectId( VMStack.GetStackObjectId( Offset1 ), VMStack.GetStackObjectId( Offset2 ) );
								break;

							case NWScriptStack::BST_STRING:
								IsEqual = (VMStack.GetStackString( Offset1 ) == VMStack.GetStackString( Offset2 ));
								break;

							default:
								{
									//
									// This may be a compare engine structure
									// request; do that now if it really was.
									//

									if ((Type >= NWScriptStack::BST_ENGINE_0) &&
										(Type <= NWScriptStack::BST_ENGINE_9))
									{
										EngineStructurePtr      EngineStruct1;
										EngineStructurePtr      EngineStruct2;
										ENGINE_STRUCTURE_NUMBER EngType;

										EngType = (ENGINE_STRUCTURE_NUMBER) (Type - NWScriptStack::BST_ENGINE_0);

										EngineStruct1 = VMStack.GetStackEngineStructure( Offset1, EngType );
										EngineStruct2 = VMStack.GetStackEngineStructure( Offset2, EngType );

										IsEqual = EngineStruct1->CompareEngineStructure(
											EngineStruct2.get( ) );
										break;
									}
									else
									{
										IsEqual = false;
									}
								}
								break;

							}

							if (!IsEqual)
								break;
						}

						//
						// Now clean the elements from the stack.
						//

						VMStack.AddSP( 2 * -((STACK_POINTER) Size) );
					}
					break;

				default:
					//
					// This may be a compare engine structure request; do that
					// now if it really was.
					//

					if ((TypeOpcode >= TYPE_BINARY_ENGINE_FIRST) &&
					    (TypeOpcode <= TYPE_BINARY_ENGINE_LAST))
					{
						EngineStructurePtr EngineStruct1;
						EngineStructurePtr EngineStruct2;

						EngineStruct1 = VMStack.StackPopEngineStructure(
							(NWScriptStack::ENGINE_STRUCTURE_NUMBER) (TypeOpcode - TYPE_BINARY_ENGINE_FIRST));
						EngineStruct2 = VMStack.StackPopEngineStructure(
							(NWScriptStack::ENGINE_STRUCTURE_NUMBER) (TypeOpcode - TYPE_BINARY_ENGINE_FIRST));

						IsEqual = EngineStruct1->CompareEngineStructure(
							EngineStruct2.get( ) );
						break;
					}

					DebugPrint(
						EDL_Errors,
						"NWScriptVM::ExecuteInstructions( %s ): @%08X: %s.%02X not supported.\n",
						Script->GetScriptName( ).c_str( ),
						PC,
						GetInstructionName( Opcode ),
						TypeOpcode);

					throw std::runtime_error( "Unsupported EQUAL/NEQUAL" );

				}

				if (Opcode == OP_EQUAL)
					VMStack.StackPushInt( IsEqual ? 1 : 0 );
				else
					VMStack.StackPushInt( IsEqual ? 0 : 1 );
			}
			break;

		case OP_GEQ: // Compare (>=)
			{
				bool Result = false;

				switch (TypeOpcode)
				{

				case TYPE_BINARY_INTINT:
					{
						int i1;
						int i2;

						i1 = VMStack.StackPopInt( );
						i2 = VMStack.StackPopInt( );

						Result = (i2 >= i1);
					}
					break;

				case TYPE_BINARY_FLOATFLOAT:
					{
						float f1;
						float f2;

						f1 = VMStack.StackPopFloat( );
						f2 = VMStack.StackPopFloat( );

						Result = (f2 >= f1);
					}
					break;

				default:
					DebugPrint(
						EDL_Errors,
						"NWScriptVM::ExecuteInstructions( %s ): @%08X: GEQ.%02X not supported.\n",
						Script->GetScriptName( ).c_str( ),
						PC,
						TypeOpcode);

					throw std::runtime_error( "Unsupported GEQ" );

				}

				VMStack.StackPushInt( Result ? 1 : 0 );
			}
			break;

		case OP_GT: // Compare (>)
			{
				bool Result = false;

				switch (TypeOpcode)
				{

				case TYPE_BINARY_INTINT:
					{
						int i1;
						int i2;

						i1 = VMStack.StackPopInt( );
						i2 = VMStack.StackPopInt( );

						Result = (i2 > i1);
					}
					break;

				case TYPE_BINARY_FLOATFLOAT:
					{
						float f1;
						float f2;

						f1 = VMStack.StackPopFloat( );
						f2 = VMStack.StackPopFloat( );

						Result = (f2 > f1);
					}
					break;

				default:
					DebugPrint(
						EDL_Errors,
						"NWScriptVM::ExecuteInstructions( %s ): @%08X: GT.%02X not supported.\n",
						Script->GetScriptName( ).c_str( ),
						PC,
						TypeOpcode);

					throw std::runtime_error( "Unsupported GT" );

				}

				VMStack.StackPushInt( Result ? 1 : 0 );
			}
			break;

		case OP_LT: // Compare (<)
			{
				bool Result = false;

				switch (TypeOpcode)
				{

				case TYPE_BINARY_INTINT:
					{
						int i1;
						int i2;

						i1 = VMStack.StackPopInt( );
						i2 = VMStack.StackPopInt( );

						Result = (i2 < i1);
					}
					break;

				case TYPE_BINARY_FLOATFLOAT:
					{
						float f1;
						float f2;

						f1 = VMStack.StackPopFloat( );
						f2 = VMStack.StackPopFloat( );

						Result = (f2 < f1);
					}
					break;

				default:
					DebugPrint(
						EDL_Errors,
						"NWScriptVM::ExecuteInstructions( %s ): @%08X: LT.%02X not supported.\n",
						Script->GetScriptName( ).c_str( ),
						PC,
						TypeOpcode);

					throw std::runtime_error( "Unsupported LT" );

				}

				VMStack.StackPushInt( Result ? 1 : 0 );
			}
			break;

		case OP_LEQ: // Compare (<=)
			{
				bool Result = false;

				switch (TypeOpcode)
				{

				case TYPE_BINARY_INTINT:
					{
						int i1;
						int i2;

						i1 = VMStack.StackPopInt( );
						i2 = VMStack.StackPopInt( );

						Result = (i2 <= i1);
					}
					break;

				case TYPE_BINARY_FLOATFLOAT:
					{
						float f1;
						float f2;

						f1 = VMStack.StackPopFloat( );
						f2 = VMStack.StackPopFloat( );

						Result = (f2 <= f1);
					}
					break;

				default:
					DebugPrint(
						EDL_Errors,
						"NWScriptVM::ExecuteInstructions( %s ): @%08X: LEQ.%02X not supported.\n",
						Script->GetScriptName( ).c_str( ),
						PC,
						TypeOpcode);

					throw std::runtime_error( "Unsupported LEQ" );

				}

				VMStack.StackPushInt( Result ? 1 : 0 );
			}
			break;

		case OP_SHLEFT: // Shift left (<<)
			{
				int Amount;
				int Shift;

				if (TypeOpcode != TYPE_BINARY_INTINT)
				{
					DebugPrint(
						EDL_Errors,
						"NWScriptVM::ExecuteInstructions( %s ): @%08X: SHLEFT.%02X not supported.\n",
						Script->GetScriptName( ).c_str( ),
						PC,
						TypeOpcode);

					throw std::runtime_error( "Unsupported SHLEFT" );
				}

				Shift  = VMStack.StackPopInt( );
				Amount = VMStack.StackPopInt( );

				VMStack.StackPushInt( Amount << Shift );
			}
			break;

		case OP_SHRIGHT: // Shift signed right (>>, SAR)
			{
				int Amount;
				int Shift;

				if (TypeOpcode != TYPE_BINARY_INTINT)
				{
					DebugPrint(
						EDL_Errors,
						"NWScriptVM::ExecuteInstructions( %s ): @%08X: SHRIGHT.%02X not supported.\n",
						Script->GetScriptName( ).c_str( ),
						PC,
						TypeOpcode);

					throw std::runtime_error( "Unsupported SHRIGHT" );
				}

				Shift  = VMStack.StackPopInt( );
				Amount = VMStack.StackPopInt( );

				//
				// N.B.  The operation implemented here is actually a complex
				//       sequence that, if the amount to be shifted is
				//       negative, involves both a front-loaded and end-loaded
				//       negate built on top of a signed shift.
				//

				if (Amount < 0)
				{
					Amount = -Amount;
					VMStack.StackPushInt( -(Amount >> Shift) );
				}
				else
				{
					VMStack.StackPushInt( Amount >> Shift );
				}
			}
			break;

		case OP_USHRIGHT: // Shift unsigned right (>>)
			{
				int Amount;
				int Shift;

				if (TypeOpcode != TYPE_BINARY_INTINT)
				{
					DebugPrint(
						EDL_Errors,
						"NWScriptVM::ExecuteInstructions( %s ): @%08X: USHRIGHT.%02X not supported.\n",
						Script->GetScriptName( ).c_str( ),
						PC,
						TypeOpcode);

					throw std::runtime_error( "Unsupported USHRIGHT" );
				}

				Shift  = VMStack.StackPopInt( );
				Amount = VMStack.StackPopInt( );

				//
				// N.B.  While this operator may have originally been intended
				//       to implement an unsigned shift, it actually performs
				//       an arithmetic (signed) shift.
				//

				VMStack.StackPushInt( Amount >> Shift );
			}
			break;

		case OP_ADD: // Add (+), concatenate strings
			switch (TypeOpcode)
			{

			case TYPE_BINARY_INTINT:
				VMStack.StackPushInt( VMStack.StackPopInt( ) + VMStack.StackPopInt( ) );
				break;

			case TYPE_BINARY_STRINGSTRING:
				VMStack.StackPushString( VMStack.StackPopString( ) + VMStack.StackPopString( ) );
				break;

			case TYPE_BINARY_VECTORVECTOR:
				VMStack.StackPushVector(
					Math::Add(
						VMStack.StackPopVector( ),
						VMStack.StackPopVector( )
						)
					);
				break;

			case TYPE_BINARY_INTFLOAT:
				{
					int   n;
					float f;

					f = VMStack.StackPopFloat( );
					n = VMStack.StackPopInt( );

					VMStack.StackPushFloat( f + (float) n );
				}
				break;

			case TYPE_BINARY_FLOATINT:
				{
					int   n;
					float f;

					n = VMStack.StackPopInt( );
					f = VMStack.StackPopFloat( );

					VMStack.StackPushFloat( f + (float) n );
				}
				break;

			case TYPE_BINARY_FLOATFLOAT:
				VMStack.StackPushFloat( VMStack.StackPopFloat( ) + VMStack.StackPopFloat( ) );
				break;

			default:
				DebugPrint(
					EDL_Errors,
					"NWScriptVM::ExecuteInstructions( %s ): @%08X: ADD.%02X not supported.\n",
					Script->GetScriptName( ).c_str( ),
					PC,
					TypeOpcode);

				throw std::runtime_error( "Unsupported ADD" );
				break;

			}
			break;

		case OP_SUB: // Subtract (-)
			switch (TypeOpcode)
			{

			case TYPE_BINARY_INTINT:
				{
					int i1;
					int i2;

					i1 = VMStack.StackPopInt( );
					i2 = VMStack.StackPopInt( );

					VMStack.StackPushInt( i2 - i1 );
				}
				break;

			case TYPE_BINARY_INTFLOAT:
				{
					int   n;
					float f;

					f = VMStack.StackPopFloat( );
					n = VMStack.StackPopInt( );

					VMStack.StackPushFloat( (float) n - f );
				}
				break;

			case TYPE_BINARY_FLOATINT:
				{
					int   n;
					float f;

					n = VMStack.StackPopInt( );
					f = VMStack.StackPopFloat( );

					VMStack.StackPushFloat( f - (float) n );
				}
				break;

			case TYPE_BINARY_FLOATFLOAT:
				{
					float f1;
					float f2;

					f1 = VMStack.StackPopFloat( );
					f2 = VMStack.StackPopFloat( );

					VMStack.StackPushFloat( f2 - f1 );
				}
				break;

			case TYPE_BINARY_VECTORVECTOR:
				{
					NWN::Vector3 v1;
					NWN::Vector3 v2;

					v1 = VMStack.StackPopVector( );
					v2 = VMStack.StackPopVector( );

					VMStack.StackPushVector( Math::Subtract( v2, v1 ) );
				}
				break;

			default:
				DebugPrint(
					EDL_Errors,
					"NWScriptVM::ExecuteInstructions( %s ): @%08X: SUB.%02X not supported.\n",
					Script->GetScriptName( ).c_str( ),
					PC,
					TypeOpcode);

				throw std::runtime_error( "Unsupported SUB" );
				break;

			}
			break;

		case OP_MUL: // Multiply (*)
			switch (TypeOpcode)
			{

			case TYPE_BINARY_INTINT:
				{
					int i1;
					int i2;

					i1 = VMStack.StackPopInt( );
					i2 = VMStack.StackPopInt( );

					VMStack.StackPushInt( i2 * i1 );
				}
				break;

			case TYPE_BINARY_INTFLOAT:
				{
					int   n;
					float f;

					f = VMStack.StackPopFloat( );
					n = VMStack.StackPopInt( );

					VMStack.StackPushFloat( (float) n * f );
				}
				break;

			case TYPE_BINARY_FLOATINT:
				{
					int   n;
					float f;

					n = VMStack.StackPopInt( );
					f = VMStack.StackPopFloat( );

					VMStack.StackPushFloat( f * (float) n );
				}
				break;

			case TYPE_BINARY_FLOATFLOAT:
				{
					float f1;
					float f2;

					f1 = VMStack.StackPopFloat( );
					f2 = VMStack.StackPopFloat( );

					VMStack.StackPushFloat( f2 * f1 );
				}
				break;

			case TYPE_BINARY_VECTORFLOAT:
				{
					float        f;
					NWN::Vector3 v;

					f = VMStack.StackPopFloat( );
					v = VMStack.StackPopVector( );

					VMStack.StackPushVector( Math::Multiply( v, f ) );
				}
				break;

			case TYPE_BINARY_FLOATVECTOR:
				{
					NWN::Vector3 v;
					float        f;

					v = VMStack.StackPopVector( );
					f = VMStack.StackPopFloat( );

					VMStack.StackPushVector( Math::Multiply( v, f ) );
				}
				break;

			default:
				DebugPrint(
					EDL_Errors,
					"NWScriptVM::ExecuteInstructions( %s ): @%08X: MUL.%02X not supported.\n",
					Script->GetScriptName( ).c_str( ),
					PC,
					TypeOpcode);

				throw std::runtime_error( "Unsupported MUL" );
				break;

			}
			break;

		case OP_DIV: // Divide (/)
			switch (TypeOpcode)
			{

			case TYPE_BINARY_INTINT:
				{
					int i1;
					int i2;

					i1 = VMStack.StackPopInt( );
					i2 = VMStack.StackPopInt( );

					if (i1 == 0)
					{
						DebugPrint(
							EDL_Errors,
							"NWScriptVM::ExecuteInstructions( %s ): @%08X: DIVII by zero.\n",
							Script->GetScriptName( ).c_str( ),
							PC);

						throw std::runtime_error( "Attempted to execute DIVII by zero." );
					}

					VMStack.StackPushInt(
						DivideWithExceptionHandler(
							i2,
							i1,
							PC,
							Script->GetScriptName( ).c_str( ) )
						);
				}
				break;

			case TYPE_BINARY_INTFLOAT:
				{
					int   n;
					float f;

					f = VMStack.StackPopFloat( );
					n = VMStack.StackPopInt( );

					if (f == 0.0f)
					{
						DebugPrint(
							EDL_Errors,
							"NWScriptVM::ExecuteInstructions( %s ): @%08X: DIVIF by zero.\n",
							Script->GetScriptName( ).c_str( ),
							PC);

						throw std::runtime_error( "Attempted to DIVIF by zero." );
					}

					VMStack.StackPushFloat( (float) n / f );
				}
				break;

			case TYPE_BINARY_FLOATINT:
				{
					int   n;
					float f;

					n = VMStack.StackPopInt( );
					f = VMStack.StackPopFloat( );

					if (n == 0)
					{
						DebugPrint(
							EDL_Errors,
							"NWScriptVM::ExecuteInstructions( %s ): @%08X: DIVFI by zero.\n",
							Script->GetScriptName( ).c_str( ),
							PC);

						throw std::runtime_error( "Attempted to DIVFI by zero." );
					}

					VMStack.StackPushFloat( f / (float) n );
				}
				break;

			case TYPE_BINARY_FLOATFLOAT:
				{
					float f1;
					float f2;

					f1 = VMStack.StackPopFloat( );
					f2 = VMStack.StackPopFloat( );

					if (f1 == 0.0f)
					{
						DebugPrint(
							EDL_Errors,
							"NWScriptVM::ExecuteInstructions( %s ): @%08X: DIVFF by zero.\n",
							Script->GetScriptName( ).c_str( ),
							PC);

						throw std::runtime_error( "Attempted to DIVFF by zero." );
					}

					VMStack.StackPushFloat( f2 / f1 );
				}
				break;

			case TYPE_BINARY_VECTORFLOAT:
				{
					float        f;
					NWN::Vector3 v;

					f = VMStack.StackPopFloat( );
					v = VMStack.StackPopVector( );

					if (f == 0.0f)
					{
						DebugPrint(
							EDL_Errors,
							"NWScriptVM::ExecuteInstructions( %s ): @%08X: DIVVF by zero.\n",
							Script->GetScriptName( ).c_str( ),
							PC);

						throw std::runtime_error( "Attempted to DIVVF by zero." );
					}

					VMStack.StackPushVector( Math::Multiply( v, 1.0f / f ) );
				}
				break;

			case TYPE_BINARY_FLOATVECTOR:
				{
					NWN::Vector3 v;
					float        f;

					v = VMStack.StackPopVector( );
					f = VMStack.StackPopFloat( );

					if (f == 0.0f)
					{
						DebugPrint(
							EDL_Errors,
							"NWScriptVM::ExecuteInstructions( %s ): @%08X: DIVFV by zero.\n",
							Script->GetScriptName( ).c_str( ),
							PC);

						throw std::runtime_error( "Attempted to DIVFV by zero." );
					}
					VMStack.StackPushVector( Math::Multiply( v, 1.0f / f ) );
				}
				break;

			default:
				DebugPrint(
					EDL_Errors,
					"NWScriptVM::ExecuteInstructions( %s ): @%08X: DIV.%02X not supported.\n",
					Script->GetScriptName( ).c_str( ),
					PC,
					TypeOpcode);

				throw std::runtime_error( "Unsupported DIV" );
				break;

			}
			break;

		case OP_MOD: // Modulus (%)
			switch (TypeOpcode)
			{

			case TYPE_BINARY_INTINT:
				{
					int n;
					int Divisor;

					Divisor = VMStack.StackPopInt( );
					n       = VMStack.StackPopInt( );

					if (Divisor == 0)
					{
						DebugPrint(
							EDL_Errors,
							"NWScriptVM::ExecuteInstructions( %s ): @%08X: MODI by zero.\n",
							Script->GetScriptName( ).c_str( ),
							PC);

						throw std::runtime_error( "Attempted to execute MODI by zero." );
					}

					VMStack.StackPushInt(
						ModulusWithExceptionHandler(
							n,
							Divisor,
							PC,
							Script->GetScriptName( ).c_str( ) )
						);
				}
				break;

			default:
				DebugPrint(
					EDL_Errors,
					"NWScriptVM::ExecuteInstructions( %s ): @%08X: MOD.%02X not supported.\n",
					Script->GetScriptName( ).c_str( ),
					PC,
					TypeOpcode);

				throw std::runtime_error( "Unsupported MOD" );
				break;

			}
			break;

		case OP_NEG: // Negation (-)
			switch (TypeOpcode)
			{

			case TYPE_UNARY_INT:
				VMStack.StackPushInt(
					-VMStack.StackPopInt( ));
				break;

			case TYPE_UNARY_FLOAT:
				VMStack.StackPushFloat(
					-VMStack.StackPopFloat( ));
				break;

			default:
				DebugPrint(
					EDL_Errors,
					"NWScriptVM::ExecuteInstructions( %s ): @%08X: NEG.%02X not supported.\n",
					Script->GetScriptName( ).c_str( ),
					PC,
					TypeOpcode);

				throw std::runtime_error( "Unsupported NEG" );
				break;

			}
			break;

		case OP_COMP: // Complement (~)
			switch (TypeOpcode)
			{

			case TYPE_UNARY_INT:
				VMStack.StackPushInt(
					~VMStack.StackPopInt( ));
				break;

			default:
				DebugPrint(
					EDL_Errors,
					"NWScriptVM::ExecuteInstructions( %s ): @%08X: COMP.%02X not supported.\n",
					Script->GetScriptName( ).c_str( ),
					PC,
					TypeOpcode);

				throw std::runtime_error( "Unsupported COMP" );
				break;

			}
			break;

		case OP_MOVSP: // add sp, <n> (always deallocates stack, negative <n>)
			{
				ULONG Displacement;

				Displacement = Script->ReadINT32( );

				Displacement = STACK_PTR( Displacement );

				VMStack.AddSP( Displacement );
			}
			break;

		case OP_STORE_STATEALL: // Save a script situation state
			m_SavedState.Stack = VMStack.SaveStack(
				VMStack.GetCurrentBP( ),
				VMStack.GetCurrentSP( ) - VMStack.GetCurrentBP( ));

			m_SavedState.Script         = Script;
			m_SavedState.ProgramCounter = PC + (PROGRAM_COUNTER) TypeOpcode;
			m_SavedState.ObjectSelf     = ObjectSelf;
			m_SavedState.ObjectInvalid  = ObjectInvalid;
			m_SavedState.Aborted        = false;
			break;

		case OP_JMP: // Unconditional jump
			{
				PROGRAM_COUNTER RelPC;

				RelPC = (PROGRAM_COUNTER) Script->ReadINT32( );

				if (RelPC == 0)
					throw std::runtime_error( "Trivial infinite loop (JMP) detected." );

				PC += RelPC;
				Script->SetInstructionPointer( PC );
				continue; // Skip normal PC adjustment for this instruction.
			}
			break;

		case OP_JSR: // Jump to subroutine (call)
			{
				PROGRAM_COUNTER RelPC;

				RelPC = (PROGRAM_COUNTER) Script->ReadINT32( );

				if (RelPC == 0)
					throw std::runtime_error( "Trivial infinite loop (JSR) detected." );

				VMStack.SaveProgramCounter( PC + InstructionLength );

				PC += RelPC;
				Script->SetInstructionPointer( PC );
				continue; // Skip normal PC adjustment for this instruction.
			}
			break;

		case OP_JZ: // Jump if zero
			{
				PROGRAM_COUNTER RelPC;
				int             i;

				RelPC = (PROGRAM_COUNTER) Script->ReadINT32( );
				i     = VMStack.StackPopInt( );

				//
				// If we did not meet the condition, don't execute the jump.
				//

				if (i)
					break;

				if (RelPC == 0)
					throw std::runtime_error( "Trivial infinite loop (JZ) detected." );

				PC += RelPC;
				Script->SetInstructionPointer( PC );
				continue; // Skip normal PC adjustment for this instruction.
			}
			break;

		case OP_RETN: // Return from call
			{
				//
				// If we have returned out of main or StartingConditional, then
				// we are ready to exit the script VM entirely.
				//

				if (VMStack.GetReturnStackDepth( ) == ReturnStackDepth)
					goto main_returned;

				//
				// Otherwise this is a standard procedural return within the
				// script VM; restore a value from the PC return stack.
				//

				PC = VMStack.RestoreProgramCounter( );
				Script->SetInstructionPointer( PC );
				continue; // Skip normal PC adjustment for this instruction.
			}
			break;

		case OP_DESTRUCT: // Deallocate stack space except for a 'hole'
			{
				STACK_POINTER Size;
				STACK_POINTER ExcludeOffset;
				STACK_POINTER ExcludeSize;
				STACK_POINTER CurSP;
				
				CurSP         = VMStack.GetCurrentSP( );
				Size          = (STACK_POINTER) Script->ReadINT16( );
				ExcludeOffset = (STACK_POINTER) Script->ReadINT16( );
				ExcludeSize   = (STACK_POINTER) Script->ReadINT16( );

				Size          = STACK_PTR( Size );
				ExcludeOffset = STACK_PTR( ExcludeOffset );
				ExcludeSize   = STACK_PTR( ExcludeSize );

				VMStack.CheckGuardZone( CurSP - Size );

				VMStack.DestructElements(
					(STACK_POINTER) Size,
					(STACK_POINTER) ExcludeOffset,
					(STACK_POINTER) ExcludeSize);
			}
			break;

		case OP_NOT: // Logical NOT (!)
			switch (TypeOpcode)
			{

			case TYPE_UNARY_INT:
				VMStack.StackPushInt(
					!VMStack.StackPopInt( ));
				break;

			default:
				DebugPrint(
					EDL_Errors,
					"NWScriptVM::ExecuteInstructions( %s ): @%08X: NOT.%02X not supported.\n",
					Script->GetScriptName( ).c_str( ),
					PC,
					TypeOpcode);

				throw std::runtime_error( "Unsupported NOT" );
				break;

			}
			break;

		case OP_DECISP: // Decrement local variable (sp-relative)
			{
				STACK_POINTER Offset;
				STACK_POINTER CurSP;

				Offset = (STACK_POINTER) Script->ReadINT32( );
				Offset = STACK_PTR( Offset );
				CurSP  = VMStack.GetCurrentSP( );

				if (TypeOpcode == TYPE_UNARY_INT)
				{
					VMStack.CheckGuardZone( Offset + CurSP );
					VMStack.DecrementStackInt( Offset + CurSP );
				}
			}
			break;

		case OP_INCISP: // Increment local variable (sp-relative)
			{
				STACK_POINTER Offset;
				STACK_POINTER CurSP;

				Offset = (STACK_POINTER) Script->ReadINT32( );
				Offset = STACK_PTR( Offset );
				CurSP  = VMStack.GetCurrentSP( );

				if (TypeOpcode == TYPE_UNARY_INT)
				{
					VMStack.CheckGuardZone( Offset + CurSP );
					VMStack.IncrementStackInt( Offset + CurSP );
				}
			}
			break;

		case OP_JNZ: // Jump if not zero
			{
				PROGRAM_COUNTER RelPC;
				int             i;

				RelPC = (PROGRAM_COUNTER) Script->ReadINT32( );
				i     = VMStack.StackPopInt( );

				//
				// If we did not meet the condition, don't execute the jump.
				//

				if (!i)
					break;

				if (RelPC == 0)
					throw std::runtime_error( "Trivial infinite loop (JNZ) detected." );

				PC += RelPC;
				Script->SetInstructionPointer( PC );
				continue; // Skip normal PC adjustment for this instruction.
			}

		case OP_CPDOWNBP: // Assign to global variables
			{
				STACK_POINTER Offset;
				STACK_POINTER Size;

				Offset = (STACK_POINTER) Script->ReadINT32( );
				Size   = (STACK_POINTER) Script->ReadINT16( );

				Offset = STACK_PTR( Offset );
				Size   = STACK_PTR( Size );

				VMStack.CopyDownSP(
					Offset,
					Size,
					true);
			}
			break;

		case OP_CPTOPBP: // Read / duplicate global variables
			{
				STACK_POINTER Offset;
				STACK_POINTER Size;

				Offset = (STACK_POINTER) Script->ReadINT32( );
				Size   = (STACK_POINTER) Script->ReadINT16( );

				Offset = STACK_PTR( Offset );
				Size   = STACK_PTR( Size );

				VMStack.CopyTopSP( Offset, Size, true );
			}
			break;

		case OP_DECIBP: // Decrement global variable (bp-relative)
			{
				STACK_POINTER Offset;
				STACK_POINTER CurBP;

				Offset = (STACK_POINTER) Script->ReadINT32( );
				Offset = STACK_PTR( Offset );
				CurBP  = VMStack.GetCurrentBP( );

				if (TypeOpcode == TYPE_UNARY_INT)
					VMStack.DecrementStackInt( Offset + CurBP );
			}
			break;

		case OP_INCIBP: // Increment global variable (bp-relative)
			{
				STACK_POINTER Offset;
				STACK_POINTER CurBP;

				Offset = (STACK_POINTER) Script->ReadINT32( );
				Offset = STACK_PTR( Offset );
				CurBP  = VMStack.GetCurrentBP( );

				if (TypeOpcode == TYPE_UNARY_INT)
					VMStack.IncrementStackInt( Offset + CurBP );
			}
			break;

		case OP_SAVEBP: // Set global variables pointer
			if (FixupState == FixupState_WaitingForGlobals)
			{
				//
				// If we were waiting for #globals to run, mark it as so.
				//

				FixupState = FixupState_WaitingForStartingConditional;

				DebugPrint(
					EDL_Verbose,
					"NWScriptVM::ExecuteInstructions( %s ): Transitioning to FixupState_WaitingForStartingConditional.\n",
					Script->GetScriptName( ).c_str( ));
			}

			VMStack.SaveBP( );

			BPNestingLevel++;
			break;

		case OP_RESTOREBP: // Restore global variables pointer

			if ((NeedFixup)                     &&
			    (!Params->empty( ))             &&
			    (FixupState == FixupState_Done) &&
			    (BPNestingLevel == 1)           &&
			    (Flags & ESF_IGNORE_STACK_MISMATCH))
			{
				//
				// This is the second half of the workaround for GUI scripts
				// with wrong parameter counts.  If we push too many parameters
				// and the GUI script had globals, we'll still have some
				// dynamic parameters on the stack before the actual saved BP.
				//
				// In this case, only if the caller has engaged the hack-o-rama
				// should we just blindly remove these extra stack entries.
				//

				if ((IsDebugLevel( EDL_Verbose)) &&
				    (VMStack.IsParameterUnderrunRestoreBP( )))
				{
					DebugPrint(
						EDL_Verbose,
						"NWScriptVM::ExecuteInstructions( %s ): Removing extra parameters for ESF_IGNORE_STACK_MISMATCH parameter underrun.\n",
						Script->GetScriptName( ).c_str( ));
				}

				while (VMStack.IsParameterUnderrunRestoreBP( ))
					VMStack.AddSP( -VMStack.GetStackIntegerSize( ) );
			}

			VMStack.RestoreBP( );

			BPNestingLevel--;
			break;

		case OP_STORE_STATE: // Save a script situation state
			{
				ULONG SaveBP;
				ULONG SaveSP;

				SaveBP = Script->ReadINT32( );
				SaveSP = Script->ReadINT32( );

				SaveBP = STACK_PTR( SaveBP );
				SaveSP = STACK_PTR( SaveSP );

				m_SavedState.Stack = VMStack.SaveStack(
					SaveBP,
					SaveSP);

				m_SavedState.Script         = Script;
				m_SavedState.ProgramCounter = PC + (PROGRAM_COUNTER) TypeOpcode;
				m_SavedState.ObjectSelf     = ObjectSelf;
				m_SavedState.ObjectInvalid  = ObjectInvalid;
				m_SavedState.Aborted        = false;
			}
			break;

		case OP_NOP: // No operation (ignored).
			break;

		default:
			DebugPrint(
				EDL_Errors,
				"NWScriptVM::ExecuteInstructions( %s ): @%08X: %02X.%02X not supported.\n",
				Script->GetScriptName( ).c_str( ),
				PC,
				Opcode,
				TypeOpcode);

			throw std::runtime_error( "Unimplemented instruction" );

		
		}

		//
		// If we fell through, then this was not a control transfer (jump), and
		// so the PC incremented linearly.  Account for this here.
		//

		PC += InstructionLength;
	}

main_returned:

	//
	// We're done executing.  Now we just need to check whether a return value
	// was left for us on the stack.  If so, then we need to pull it out, else
	// we're set.
	//
	// N.B.  If there was any more than a four byte displacement then the
	//       script was ill-formed and the program must abort.  This is because
	//       we might have been in a nested call on this stack, and in that
	//       case we have no way to recover the earlier call frames.
	//

	EndSP = VMStack.GetCurrentSP( );

	if (StartSP == EndSP)
	{
		if ((m_RecursionLevel == 1) &&
		    (Flags & ESF_IGNORE_STACK_MISMATCH))
		{
			//
			// The script has offset the stack, but we are the top level call
			// and the user has instructed us to ignore it for compatibility
			// with buggy scripts that the server had let through before.  We
			// do not raise an abort here as a result of this mismatch.
			//
			// Note that the stack is trashed at this point so we can only do
			// this if we really were the top level invocation.
			//

			return DefaultReturnCode;
		}

		if (ExpectReturnValue)
		{
			DebugPrint(
				EDL_Errors,
				"NWScriptVM::ExecuteInstructions( %s ): WARNING: StartingConditional appears to have not returned a value.\n",
				Script->GetScriptName( ).c_str( ));
		}
		else if (Script->GetPatchState( ) == NWScriptReader::NCSPatchState_PatchReturnValue)
		{
			//
			// Note that if we patched the #loader routine directly, we will
			// appear to have the same SP here (as the StartSP we have is after
			// we pushed the parameters and return value).  Thus we will need
			// to fish things out in that case.
			//

			try
			{
				int ReturnCode = VMStack.StackPopInt( );

				return ReturnCode;
			}
			catch (std::exception &e)
			{
				DebugPrint(
					EDL_Errors,
					"NWScriptVM::ExecuteInstructions( %s ): Failed to retrieve return value (patched): Exception '%s'.\n",
					Script->GetScriptName( ).c_str( ),
					e.what( ));

				AbortScript( );

				if (Flags & ESF_RAISE_ON_EXEC_FAILURE)
					throw;

				return DefaultReturnCode;
			}
		}

		return DefaultReturnCode;
	}

	if (EndSP != StartSP + VMStack.GetStackIntegerSize( ))
	{
		if ((!ExpectReturnValue)    &&
		    (m_RecursionLevel == 1) &&
		    (Flags & ESF_IGNORE_STACK_MISMATCH))
		{
			//
			// The script has offset the stack, but we are the top level call
			// and the user has instructed us to ignore it for compatibility
			// with buggy scripts that the server had let through before.  We
			// do not raise an abort here as a result of this mismatch.
			//
			// Note that the stack is trashed at this point so we can only do
			// this if we really were the top level invocation.
			//

			return DefaultReturnCode;
		}

		DebugPrint(
			EDL_Verbose,
			"NWScriptVM::ExecuteInstructions( %s ): Script StartSP (%d) / EndSP (%d) mismatch.\n",
			Script->GetScriptName( ).c_str( ),
			StartSP,
			EndSP);

		AbortScript( );

		if (Flags & ESF_RAISE_ON_EXEC_FAILURE)
			throw std::runtime_error( "Script StartSP / EndSP mismatch." );

		return DefaultReturnCode;
	}

	//
	// If we were not expecting a return value, but we got one anyways, log
	// it as we've likely got a bug.
	//

	if (NoReturnValue)
	{
		DebugPrint(
			EDL_Errors,
			"NWScriptVM::ExecuteInstructions( %s ): WARNING: Non-StartingConditional appears to be returning a value.\n",
			Script->GetScriptName( ).c_str( ));
	}

	//
	// Pull the return value off and return it.
	//

	try
	{
		int ReturnCode = VMStack.StackPopInt( );

		return ReturnCode;
	}
	catch (std::exception &e)
	{
		DebugPrint(
			EDL_Errors,
			"NWScriptVM::ExecuteInstructions( %s ): Failed to retrieve return value: Exception '%s'.\n",
			Script->GetScriptName( ).c_str( ),
			e.what( ));

		AbortScript( );

		if (Flags & ESF_RAISE_ON_EXEC_FAILURE)
			throw;

		return DefaultReturnCode;
	}
}

void
NWScriptVM::ExitVM(
	__inout NWScriptStack & VMStack
	)
/*++

Routine Description:

	This routine performs cleanup work to return the script VM to its prior state
	after a script returns from instruction execution (whether successful or
	not).

	If the exit request represents the outermost call to the script VM, then the
	abort flag is reset.

Arguments:

	VMStack - Supplies the active VM stack.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	m_RecursionLevel -= 1;

	//
	// Clear the abort flag and saved state script reference if we are done.
	//

	if (m_RecursionLevel == 0)
	{
		m_State.Aborted = false;

		m_SavedState.Script = NULL;
		m_SavedState.Stack.ResetStack( );

		VMStack.ResetStack( );

		m_InstructionsExecuted = 0;
	}
}

#if ANALYZE_SCRIPT
void
NWScriptVM::AnalyzeScript(
	__in NWScriptReaderPtr & Script,
	__in ULONG Flags
	)
/*++

Routine Description:

	This routine analyzes a script's structure and determines the extent of the
	parameters passed to the entry point symbol.

Arguments:

	Script - Supplies the script to analyze.

	Flags - Supplies flags that control the execution environment of the
	        script.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	//
	// If we have no registered action list, then don't bother to analyze the
	// script.
	//

	if (m_ActionCount == 0)
		return;

	try
	{
		//
		// Create a script analyzer context and analyze the program structure,
		// which gets us the stack usage data for the entry point symbol.
		//

		NWScriptAnalyzer Analyzer( m_TextOut, m_ActionDefs, m_ActionCount );
		unsigned long    AnalyzerFlags;

		AnalyzerFlags = 0;

		if ((Flags & ESF_STATIC_TYPE_DISCOVERY) == 0)
			AnalyzerFlags |= NWScriptAnalyzer::AF_STRUCTURE_ONLY;

		Analyzer.Analyze( Script.get( ), AnalyzerFlags );

		//
		// If we had no entry point symbol, then there's nothing to set.
		//

		if (Analyzer.GetSubroutines( ).empty( ))
			return;

		const NWScriptSubroutine * Entrypoint;
		
		Entrypoint = Analyzer.GetSubroutines( ).front( ).get( );

		//
		// Now stash the usage information away so that we can refer to it in
		// the future without re-analyzing the script program.
		//

		NWScriptReader::ScriptAnalyzeState AnalyzeState;

		ZeroMemory( &AnalyzeState, sizeof( AnalyzeState ) );

		AnalyzeState.ReturnCells    = (unsigned long) Entrypoint->GetReturnSize( )    / NWNScriptLib::CELL_SIZE;
		AnalyzeState.ParameterCells = (unsigned long) Entrypoint->GetParameterSize( ) / NWNScriptLib::CELL_SIZE;
		AnalyzeState.ArgumentTypes  = NULL;

		//
		// If we are to perform full static type discover, identify the types
		// of the entrypoint symbol arguments now.
		//

		if ((Flags & ESF_STATIC_TYPE_DISCOVERY) &&
		    (AnalyzeState.ParameterCells != 0)  &&
		    (Analyzer.GetEntryPC( ) != NWNScriptLib::INVALID_PC))
		{
			AnalyzeState.ArgumentTypes = new unsigned long[ AnalyzeState.ParameterCells ];

			for (unsigned long i = 0; i < AnalyzeState.ParameterCells; i += 1)
				AnalyzeState.ArgumentTypes[ i ] = (unsigned long) Entrypoint->GetParameters( )[ i ];
		}

		Script->SetAnalyzeState( &AnalyzeState );

		DebugPrint(
			EDL_Verbose,
			"NWScriptVM::AnalyzeScript( %s ): Entry point symbol at PC=%08X has ReturnCells=%lu, ParameterCells=%lu.\n",
			Script->GetScriptName( ).c_str( ),
			Entrypoint->GetAddress( ),
			AnalyzeState.ReturnCells,
			AnalyzeState.ParameterCells);
	}
	catch (std::exception &e)
	{
		DebugPrint(
			EDL_Errors,
			"NWScriptVM::AnalyzeScript( %s ): Exception analyzing script: '%s'.\n",
			Script->GetScriptName( ).c_str( ),
			e.what( ));
	}
}
#endif

void
NWScriptVM::PushEntrypointParameters(
	__in const ScriptParamVec * Params,
	__in NWScriptReaderPtr & Script,
	__inout NWScriptStack & VMStack,
	__in ULONG Flags
	)
/*++

Routine Description:

	This routine pushes parameters to an entry point symbol onto the VM stack.

Arguments:

	Params - Supplies the parameters to push.

	Script - Supplies the script that is to be run.

	VMStack - Supplies the execution stack for the script.

	Flags - Supplies flags that control the execution environment of the
	        script.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	if ((Flags & ESF_STATIC_TYPE_DISCOVERY) == 0)
	{
		//
		// The types for the arguments will be dynamically discovered at
		// runtime.  Push the arguments as dynamic parameters.
		//

		for (ScriptParamVec::const_reverse_iterator it = Params->rbegin( );
		     it != Params->rend( );
		     ++it)
		{
			VMStack.StackPushDynamicParameter( it->c_str( ) );
		}
	}
	else
	{
		const NWScriptReader::ScriptAnalyzeState * AnalyzeState;
		unsigned long                              ParamIdx;

		//
		// The types for the arguments have been statically discovered through
		// analyzing the script instruction stream.  Push the arguments as
		// fully statically typed parameters.
		//

		if ((AnalyzeState = Script->GetAnalyzeState( )) == NULL)
			throw std::runtime_error( "script analysis did not succeed" );
		else if ((size_t) AnalyzeState->ParameterCells != Params->size( ))
			throw std::runtime_error( "wrong number of script arguments" );
		else if ((AnalyzeState->ArgumentTypes == NULL) && (!Params->empty( )))
			throw std::runtime_error( "script was not analyzed with type discovery" );

		ParamIdx = AnalyzeState->ParameterCells;

		for (ScriptParamVec::const_reverse_iterator it = Params->rbegin( );
		     it != Params->rend( );
		     ++it)
		{
			switch (AnalyzeState->ArgumentTypes[ ParamIdx - 1 ])
			{

			case ACTIONTYPE_INT:
			case ACTIONTYPE_VOID: // Unused parameters default to integers
				VMStack.StackPushInt( atoi( it->c_str( ) ) );
				break;

			case ACTIONTYPE_FLOAT:
				VMStack.StackPushFloat( (float) atof( it->c_str( ) ) );
				break;

			case ACTIONTYPE_STRING:
				VMStack.StackPushString( it->c_str( ) );
				break;

			case ACTIONTYPE_OBJECT:
				{
					char          * Endp;
					NWN::OBJECTID   ObjectId;

					ObjectId = (NWN::OBJECTID) _strtoui64(
						it->c_str( ),
						&Endp,
						10);

					//
					// If the conversion failed, return the invalid object id.
					//

					if (*Endp)
						ObjectId = VMStack.GetInvalidObjId( );

					VMStack.StackPushObjectId( ObjectId );
				}
				break;

			default:
				throw std::runtime_error( "illegal script entrypoint argument type" );

			}

			ParamIdx -= 1;
		}
	}
}

__declspec(noinline)
void
NWScriptVM::VMDebuggerCheckForBreakpoint(
	__in const char * ScriptName,
	__in PROGRAM_COUNTER PC,
	__in const NWScriptStack & VMStack,
	__in_opt const char * SymbolName
	)
/*++

Routine Description:

	This routine checks for a debugger breakpoint in the breakpoint table.  If
	reached, the contents of the stack are displayed and a native breakpoint is
	executed to break into the native debugger.

Arguments:

	ScriptName - Supplies the name of the script.

	PC - Supplies the current PC value.

	VMStack - Supplies the current VM stack.

	SymbolName - Optionally supplies the name of the current symbol.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	char                  Formatted[ 512 ];
	const NWN::ResRef32 * BreakResRef;
	NWN::ResRef32         ResRef;
	size_t                Len;
	STACK_POINTER         SP;

	ZeroMemory( &ResRef, sizeof( ResRef ) );
	Len = strlen( ScriptName );
	memcpy( &ResRef, ScriptName, min( Len, sizeof( ResRef ) ) );

	for (size_t i = 0; i < RTL_NUMBER_OF( m_Breakpoints ); i += 1)
	{
		if (m_Breakpoints[ i ].BreakpointPC != PC)
			continue;

		BreakResRef = (const NWN::ResRef32 *) &m_Breakpoints[ i ].ScriptName;

		if (memcmp( BreakResRef, &ResRef, sizeof( ResRef ) ))
			continue;

		try
		{
			StringCbPrintfA(
				Formatted,
				sizeof( Formatted ),
				"**** Debugger breakpoint reached at %s:%08X (%s)\n",
				ScriptName,
				(unsigned long) PC,
				SymbolName != NULL ? SymbolName : "<no symbols>");
			OutputDebugStringA( Formatted );
			StringCbPrintfA(
				Formatted,
				sizeof( Formatted ),
				"     PC=%08X SP=%08X BP=%08X\n",
				(unsigned long) PC,
				(unsigned long) VMStack.GetCurrentSP( ),
				(unsigned long) VMStack.GetCurrentBP( ));
			OutputDebugStringA( Formatted );
			OutputDebugStringA( "**** Stack dump: \n" );
			OutputDebugStringA( "SPOffset Value   .Ty< String Data >\n" );
			OutputDebugStringA( "===================================\n" );

			for (SP = VMStack.GetCurrentSP( );
			     SP != 0;
			     SP -= VMStack.GetStackIntegerSize( ))
			{
				ULONG                          RawStack;
				UCHAR                          RawStackType;
				NWScriptStack::BASE_STACK_TYPE BaseType;

				if (!VMStack.PeekStack(
					VMStack.GetCurrentSP( ) - (SP - VMStack.GetStackIntegerSize( )),
					RawStack,
					RawStackType ))
				{
					break;
				}

				if (VMStack.DebugIsEngineStructureType( RawStackType ))
				{
					BaseType = NWScriptStack::BST_INVALID;
				}
				else
				{
					BaseType = VMStack.GetStackType(
						VMStack.GetCurrentSP( ) - (SP - VMStack.GetStackIntegerSize( )) );
				}

				switch (BaseType)
				{

				case NWScriptStack::BST_STRING:
					StringCbPrintfA(
						Formatted,
						sizeof( Formatted ),
						"%08X %08X.%02X<\"%s\">\n",
						(unsigned long) -(SP - VMStack.GetStackIntegerSize( )),
						RawStack,
						RawStackType,
						VMStack.GetStackString( -(SP - VMStack.GetStackIntegerSize( )) ).c_str( ));
					break;

				default:
					StringCbPrintfA(
						Formatted,
						sizeof( Formatted ),
						"%08X %08X.%02X\n",
						(unsigned long) -(SP - VMStack.GetStackIntegerSize( )),
						RawStack,
						RawStackType);
					break;

				}

				OutputDebugStringA( Formatted );
			}
		}
		catch (std::exception)
		{
		}

		__debugbreak( );
	}
}

const char *
NWScriptVM::GetInstructionName(
	__in UCHAR Opcode
	)
/*++

Routine Description:

	This routine returns the mnemonic for an opcode.

Arguments:

	Opcode - Supplies the opcode to inspect.

Return Value:

	The routine returns the nmenomic of the opcode.

Environment:

	User mode.

--*/
{
	static const char * Opcode_Names[ MAX_NWSCRIPT_OPCODE + 1 ] =
	{
		"???",
		"CPDOWNSP",
		"RSADD",
		"CPTOPSP",
		"CONST",
		"ACTION",
		"LOGAND",
		"LOGOR",
		"INCOR",
		"EXCOR",
		"BOOLAND",
		"EQUAL",
		"NEQUAL",
		"GEQ",
		"GT",
		"LT",
		"LEQ",
		"SHLEFT",
		"SHRIGHT",
		"USHRIGHT",
		"ADD",
		"SUB",
		"MUL",
		"DIV",
		"MOD",
		"NEG",
		"COMP",
		"MOVSP",
		"STORE_STATEALL",
		"JMP",
		"JSR",
		"JZ",
		"RETN",
		"DESTRUCT",
		"NOT",
		"DECISP",
		"INCISP",
		"JNZ",
		"CPDOWNBP",
		"CPTOPBP",
		"DECIBP",
		"INCIBP",
		"SAVEBP",
		"RESTOREBP",
		"STORE_STATE",
		"NOP"
	};

	if (Opcode < MAX_NWSCRIPT_OPCODE + 1)
		return Opcode_Names[ Opcode ];
	else
		return "???";
}

const char *
NWScriptVM::GetTypeOpcodeName(
	__in UCHAR TypeOpcode
	)
/*++

Routine Description:

	This routine returns the mnemonic for a type opcode.

Arguments:

	TypeOpcode - Supplies the type opcode to inspect.

Return Value:

	The routine returns the nmenomic of the type opcode.

Environment:

	User mode.

--*/
{
	switch (TypeOpcode)
	{

	case TYPE_UNARY_NONE:
	case TYPE_UNARY_STACKOP:
	case TYPE_UNARY_RESERVED2:
		return "";

	case TYPE_UNARY_INT:
		return "I";
	case TYPE_UNARY_FLOAT:
		return "F";
	case TYPE_UNARY_STRING:
		return "S";
	case TYPE_UNARY_OBJECTID:
		return "O";

	case TYPE_BINARY_INTINT:
		return "II";
	case TYPE_BINARY_FLOATFLOAT:
		return "FF";
	case TYPE_BINARY_OBJECTIDOBJECTID:
		return "OO";
	case TYPE_BINARY_STRINGSTRING:
		return "SS";
	case TYPE_BINARY_STRUCTSTRUCT:
		return "TT";
	case TYPE_BINARY_INTFLOAT:
		return "IF";
	case TYPE_BINARY_FLOATINT:
		return "FI";
	case TYPE_BINARY_VECTORVECTOR:
		return "VV";
	case TYPE_BINARY_VECTORFLOAT:
		return "VF";
	case TYPE_BINARY_FLOATVECTOR:
		return "FV";

	default:
		if ((TypeOpcode >= TYPE_UNARY_ENGINE_FIRST) &&
		    (TypeOpcode <= TYPE_UNARY_ENGINE_LAST))
		{
			return "E";
		}
		else if ((TypeOpcode >= TYPE_BINARY_ENGINE_FIRST) &&
		         (TypeOpcode <= TYPE_BINARY_ENGINE_LAST))
		{
			return "EE";
		}

		return "??";

	}
}
