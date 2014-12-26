/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptProgram.cpp

Abstract:

	This module houses the representation of a JIT'd NWScript program.  The
	NWScriptProgram object is constructed given the NWScriptAnalyzer's IR, and
	encapsulates a series of DynamicMethod objects and supporting state
	describing an MSIL view of the script program's functional nature.

--*/

#include "Precomp.h"
#include "NWNScriptJIT.h"
#include "NWScriptProgram.h"
#include "NWScriptCodeGenerator.h"
#include "NWScriptSavedState.h"
#include "NWScriptUtilities.h"
#include "NWScriptManagedSupport.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4355) // warning C4355: 'this' : used in base member initializer list
#endif

//
// Define to 1 to save the NWScriptManagedInterface module under its canonical
// assembly name.
//

#define NWSCRIPT_SAVE_NWSCRIPTMANAGEDINTERFACE 0

//
// Define the critical section for instantiating managed scripts, used to
// serialize loading an assembly with the AppDomain-wide assembly resolve event
// hooked.
//

swutil::CriticalSection g_AssemblyResolveEventLock;

NWScriptProgram::NWScriptProgram(
	__in const NWScriptAnalyzer * Analyzer,
	__in_opt IDebugTextOut * TextOut,
	__in ULONG DebugLevel,
	__in INWScriptActions * ActionHandler,
	__in NWN::OBJECTID ObjectInvalid,
	__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams
	)
/*++

Routine Description:

	This routine constructs a new NWScriptProgram.  It bears responsibility for
	translating the NWScriptAnalyzer IR into MSIL.  Generated dynamic methods
	are instantiated for direct invocation by native code.

Arguments:

	Analyzer - Supplies the analysis context that describes the NWScript IR to
	           generate code for.

	TextOut - Optionally supplies an IDebugTextOut interface that receives text
	          debug output from the execution environment.

	DebugLevel - Supplies the debug output level.  Legal values are drawn from
	             the NWScriptVM::ExecDebugLevel family of enumerations.

	ActionHandler - Supplies the engine actions implementation handler.

	ObjectInvalid - Supplies the object id to reference for the 'object
	                invalid' manifest constant.

	CodeGenParams - Optionally supplies extension code generation parameters.

Return Value:

	None.  Raises a System::Exception on failure.

Environment:

	User mode, C++/CLI.

--*/
: m_TextOut( TextOut ),
  m_DebugLevel( DebugLevel ),
  m_ActionHandler( ActionHandler ),
  m_ActionDefs( NULL ),
  m_ActionCount( 0 ),
  m_ProgramObject( nullptr ),
  m_JITIntrinsics( gcnew NWScriptJITIntrinsics( this ) ),
  m_EntryPointReturnsValue( false ),
  m_EntryPointParamTypes( nullptr ),
  m_CurrentActionObjectSelf( NWN::INVALIDOBJID ),
  m_InvalidObjId( ObjectInvalid ),
  m_Stack( NULL ),
  m_Aborted( false ),
  m_NestingLevel( 0 ),
  m_ScriptName( nullptr ),
  m_EngineStructureTypes( nullptr ),
  m_CodeGenFlags( 0 ),
  m_ManagedScript( false ),
  m_ManagedSupport( nullptr ),
  m_StringEncoding( NWScriptUtilities::NW8BitEncoding )
{
	try
	{
		PCNWACTION_DEFINITION   ActionDefs;
		NWSCRIPT_ACTION         ActionCount;
		String                ^ Name;

		if (sizeof( NWScript::NeutralStringStorage ) != sizeof( INWScriptStack::NeutralString ))
			throw gcnew Exception( "Size mismatch between NWScriptStorage::NeutralString and INWScriptStack::NeutralString." );

		Name = gcnew String(
			Analyzer->GetProgramName( ).data( ),
			0,
			(Int32) Analyzer->GetProgramName( ).size( ),
			m_StringEncoding);

		m_ScriptName = Name;

		Analyzer->GetActionDefs( ActionDefs, ActionCount );

		m_ActionDefs  = ActionDefs;
		m_ActionCount = ActionCount;

		if (CodeGenParams == NULL)
			m_CodeGenFlags = 0;
		else
			m_CodeGenFlags = CodeGenParams->CodeGenFlags;

		DiscoverEntryPointParameters( Analyzer );

		//
		// Generate the MSIL program structure.
		//

		GenerateProgramCode(
			Analyzer,
			CodeGenParams,
			ObjectInvalid,
			Name);
	}
	catch (Exception ^ e)
	{
		ErrorException( e );
		throw;
	}
}

NWScriptProgram::NWScriptProgram(
	__in const NWScriptAnalyzer * Analyzer,
	__in_opt IDebugTextOut * TextOut,
	__in ULONG DebugLevel,
	__in INWScriptActions * ActionHandler,
	__in array< Byte > ^ ManagedAssembly,
	__in NWScriptManagedSupport ^ ManagedSupport,
	__in NWN::OBJECTID ObjectInvalid,
	__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams
	)
/*++

Routine Description:

	This routine constructs a new NWScriptProgram based off of an already
	created assembly which links to a managed support object's internal
	interface layer assembly.

	N.B.  The managed support object must have the same object invalid constant
	      as the caller supplies.

Arguments:

	Analyzer - Supplies the analysis context that describes the action service
	           table in use.

	TextOut - Optionally supplies an IDebugTextOut interface that receives text
	          debug output from the execution environment.

	DebugLevel - Supplies the debug output level.  Legal values are drawn from
	             the NWScriptVM::ExecDebugLevel family of enumerations.

	ActionHandler - Supplies the engine actions implementation handler.

	ManagedAssembly - Supplies the contents of the already-generated assembly.

	ManagedSupport - Supplies the support object used for processing managed
	                 code scripts.

	ObjectInvalid - Supplies the object id to reference for the 'object
	                invalid' manifest constant.

	CodeGenParams - Optionally supplies extension code generation parameters.

Return Value:

	None.  Raises a System::Exception on failure.

Environment:

	User mode, C++/CLI.

--*/
: m_TextOut( TextOut ),
  m_DebugLevel( DebugLevel ),
  m_ActionHandler( ActionHandler ),
  m_ActionDefs( NULL ),
  m_ActionCount( 0 ),
  m_ProgramObject( nullptr ),
  m_JITIntrinsics( gcnew NWScriptJITIntrinsics( this ) ),
  m_EntryPointReturnsValue( false ),
  m_EntryPointParamTypes( nullptr ),
  m_CurrentActionObjectSelf( NWN::INVALIDOBJID ),
  m_InvalidObjId( ObjectInvalid ),
  m_Stack( NULL ),
  m_Aborted( false ),
  m_NestingLevel( 0 ),
  m_ScriptName( nullptr ),
  m_EngineStructureTypes( nullptr ),
  m_CodeGenFlags( 0 ),
  m_ManagedScript( true ),
  m_ManagedSupport( nullptr ),
  m_StringEncoding( NWScriptUtilities::NWUTF8Encoding )
{
	try
	{
		PCNWACTION_DEFINITION   ActionDefs;
		NWSCRIPT_ACTION         ActionCount;
		String                ^ Name;

		if (sizeof( NWScript::NeutralStringStorage ) != sizeof( INWScriptStack::NeutralString ))
			throw gcnew Exception( "Size mismatch between NWScriptStorage::NeutralString and INWScriptStack::NeutralString." );

		Name = gcnew String(
			Analyzer->GetProgramName( ).data( ),
			0,
			(Int32) Analyzer->GetProgramName( ).size( ),
			m_StringEncoding);

		m_ScriptName = Name;

		Analyzer->GetActionDefs( ActionDefs, ActionCount );

		m_ActionDefs  = ActionDefs;
		m_ActionCount = ActionCount;

		if (CodeGenParams == NULL)
			m_CodeGenFlags = 0;
		else
			m_CodeGenFlags = CodeGenParams->CodeGenFlags;

		//
		// Load the managed assembly.
		//

		InstantiateManagedScript( ManagedAssembly, ManagedSupport );
	}
	catch (Exception ^ e)
	{
		ErrorException( e );
		throw;
	}
}


#ifdef _MSC_VER
#pragma warning(pop)
#endif

NWScriptProgram::!NWScriptProgram(
	)
/*++

Routine Description:

	This routine deletes the current NWScriptProgram object and its
	associated members.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode, C++/CLI.

--*/
{
}

int
NWScriptProgram::ExecuteScript(
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

	ParamCount - Supplies the count of parameters to the entry point

	DefaultReturnCode - Supplies the default return code on an error condition,
	                    or if the script did not return a value.

	Flags - Supplies flags that control the execution environment of the
	        script.  The flags are the same as those that are accepted by the
	        NWScriptVM::ExecuteScript API.

Return Value:

	If the script is a StartingConditional, its return value is returned.
	Otherwise, the default return code is returned.

Environment:

	User mode, C++/CLI.

--*/
{
	NWN::OBJECTID             CurrentActionObjectSelf;
	INWScriptStack          * Stack;
	IGeneratedScriptProgram ^ ProgramObject;
	IGeneratedScriptProgram ^ UseProgramObject;
	bool                      SetNestingLevel;

	//
	// Presently, none of the flags bits are meaningful to the script JIT
	// engine itself as they all deal with recovery from ill-formed scripts, or
	// passing incorrect argument counts to scripts.  Neither of these
	// situations can happen if we have gotten to the point of successfully
	// emitting the MSIL code for the script IR.
	//

	UNREFERENCED_PARAMETER( Flags );

	//
	// First, save the current OBJECT_SELF on the stack in case we are a
	// recursive call.
	//

	CurrentActionObjectSelf = m_CurrentActionObjectSelf;
	Stack                   = m_Stack;
	ProgramObject           = m_ProgramObject;

	SetNestingLevel = false;

	try
	{
		int      ReturnCode;
//		Object ^ ReturnValue;

		//
		// If we're a nested invocation, we need to create a new instance to
		// allow for it to have separate global variables.
		//

		if (m_NestingLevel > 0)
		{
			UseProgramObject = m_ProgramObject->CloneScriptProgram( );
		}
		else
		{
			UseProgramObject = ProgramObject;
		}

		m_NestingLevel  += 1;
		SetNestingLevel  = true;

		m_CurrentActionObjectSelf = ObjectSelf;
		m_Stack                   = VMStack;
		m_ProgramObject           = UseProgramObject;

//		m_FldCurrentActionObjectSelf->SetValue(
//			m_ProgramObject,
//			m_CurrentActionObjectSelf);

		//
		// Now execute the actual entry point.
		//

		if (IsDebugLevel( NWScriptVM::EDL_Calls ))
		{
			m_TextOut->WriteText(
				"NWScriptProgram::ExecuteScript: Running script %s (nesting level %d).\n",
				NWScriptUtilities::ConvertString( m_ScriptName ).c_str( ),
				m_NestingLevel);
		}

//		ReturnValue = m_NWScriptEntryPoint->Invoke(
//			m_ProgramObject,
//			BindingFlags::Default,
//			nullptr,
//			ConvertParameterList( Params, ParamCount ),
//			nullptr);

		ReturnCode = UseProgramObject->ExecuteScript(
			m_CurrentActionObjectSelf,
			ConvertParameterList( Params, ParamCount ),
			DefaultReturnCode);

		//
		// If we returned a value, convert it.
		//

//		if (m_EntryPointReturnsValue)
//			ReturnCode = (int) ReturnValue;
//		else
//			ReturnCode = DefaultReturnCode;

		//
		// Restore the per-invocation members back to their saved states in
		// case we were a recursive call unwinding.
		//

		m_CurrentActionObjectSelf = CurrentActionObjectSelf;
		m_Stack                   = Stack;
		m_ProgramObject           = ProgramObject;

		m_NestingLevel -= 1;

		if (m_NestingLevel == 0)
			m_Aborted = false;

		return ReturnCode;
	}
	catch (Exception ^ e)
	{
		ErrorException( e );

		m_CurrentActionObjectSelf = CurrentActionObjectSelf;
		m_Stack                   = Stack;
		m_ProgramObject           = ProgramObject;

		if (SetNestingLevel)
			m_NestingLevel -= 1;

		if (m_NestingLevel == 0)
			m_Aborted = false;

		return DefaultReturnCode;
	}
}

void
NWScriptProgram::ExecuteScriptSituation(
	__inout NWScriptSavedState ^ ScriptState,
	__in NWN::OBJECTID ObjectSelf
	)
/*++

Routine Description:

	This routine executes a script situation, which is a saved portion of a
	script that is later run (such as a delayed action).

Arguments:

	ScriptState - Supplies the state of the script to execute.

	ObjectSelf - Supplies the object id to reference for the 'object self'
	             manifest constant.

Return Value:

	None.

Environment:

	User mode, C++/CLI.

--*/
{
	NWN::OBJECTID             CurrentActionObjectSelf;
	INWScriptStack          * Stack;
	IGeneratedScriptProgram ^ ProgramObject;
	bool                      SetNestingLevel;

	//
	// First, save the current OBJECT_SELF on the stack in case we are a
	// recursive call.
	//

	CurrentActionObjectSelf = m_CurrentActionObjectSelf;
	Stack                   = m_Stack;
	ProgramObject           = m_ProgramObject;

	SetNestingLevel = false;

	try
	{
		UInt32 ResumeMethodId;

		m_NestingLevel  += 1;
		SetNestingLevel  = true;

		m_CurrentActionObjectSelf = ScriptState->GetCurrentActionObjectSelf( );
		m_Stack                   = ScriptState->GetStack( );
		m_ProgramObject           = ScriptState->GetProgramState( );

		ResumeMethodId = ScriptState->GetResumeMethodId( );

		if (IsDebugLevel( NWScriptVM::EDL_Calls ))
		{
			m_TextOut->WriteText(
				"NWScriptProgram::ExecuteScript: Running situation %lu for script %s (nesting level %d).\n",
				ResumeMethodId,
				NWScriptUtilities::ConvertString( m_ScriptName ).c_str( ),
				m_NestingLevel);
		}

		//
		// Now execute the actual entry point.
		//

//		ResumeMethod->Invoke(
//			m_ProgramObject,
//			BindingFlags::Default,
//			nullptr,
//			ScriptState->GetLocals( ),
//			nullptr);

		m_ProgramObject->ExecuteScriptSituation(
			ResumeMethodId,
			ScriptState->GetLocals( ),
			ObjectSelf );

		//
		// Restore the per-invocation members back to their saved states in
		// case we were a recursive call unwinding.
		//

		m_CurrentActionObjectSelf = CurrentActionObjectSelf;
		m_Stack                   = Stack;
		m_ProgramObject           = ProgramObject;

		m_NestingLevel -= 1;

		if (m_NestingLevel == 0)
			m_Aborted = false;
	}
	catch (Exception ^ e)
	{
		ErrorException( e );

		m_CurrentActionObjectSelf = CurrentActionObjectSelf;
		m_Stack                   = Stack;
		m_ProgramObject           = ProgramObject;

		if (SetNestingLevel)
			m_NestingLevel -= 1;

		if (m_NestingLevel == 0)
			m_Aborted = false;
	}
}

void
NWScriptProgram::AbortScript(
	)
/*++

Routine Description:

	This routine aborts execution of the entire script program.  Unlike the
	script VM, nested calls are not aborted.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode, C++/CLI.

--*/
{
	m_Aborted = true;
}

bool
NWScriptProgram::IsScriptAborted(
	)
/*++

Routine Description:

	This routine returns whether the script program has been flagged for
	abortive termination (but has not yet exited).

Arguments:

	None.

Return Value:

	The routine returns a Boolean value that indicates whether an abort has
	been requested (true) or not (false).

Environment:

	User mode, C++/CLI.

--*/
{
	return m_Aborted;
}

NWScriptSavedState ^
NWScriptProgram::GetSavedState(
	)
/*++

Routine Description:

	This routine returns a handle to the most recently created saved state
	snapshot.  Note that the object is not copied, so multiple instances cannot
	be created from the same saved state attempt.

Arguments:

	None.

Return Value:

	The routine returns a NWScriptSavedState handle on success.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	try
	{
		NWScriptSavedState ^ SavedState;

		if (m_SavedState == nullptr)
			throw gcnew Exception( "No saved state is ready." );

		SavedState   = m_SavedState;
		m_SavedState = nullptr;

		return SavedState;
	}
	catch (Exception ^ e)
	{
		ErrorException( e );
		throw;
	}
}

NWScriptSavedState ^
NWScriptProgram::DuplicateSavedState(
	__in NWScriptSavedState ^ SourceState
	)
/*++

Routine Description:

	This routine creates a copy of an existing saved state.

Arguments:

	SourceState - Supplies the source saved state to duplicate.

	ResumeSubroutineId - Supplies the script situation id of the subroutine to
	                     execute on resume.

	ProgramObject - Supplies a clone of the program object, including the
	                current global variables state.

Return Value:

	The routine returns a new NWScriptSavedState handle on success.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	try
	{
		//
		// Construct the underlying saved state object, and return it out to
		// the caller.
		//

		return gcnew NWScriptSavedState(
			this,
			SourceState->GetProgramState( )->CloneScriptProgram( ),
			gcnew array< Object ^ >{ SourceState->GetLocals( ) },
			SourceState->GetResumeMethodId( ),
			SourceState->GetStack( ),
			SourceState->GetCurrentActionObjectSelf( ),
			SourceState->GetResumeMethodPC( ),
			gcnew array< Object ^ >{ SourceState->GetGlobals( ) });
	}
	catch (Exception ^ e)
	{
		ErrorException( e );
		throw;
	}
}

void
NWScriptProgram::PushSavedState(
	__in NWScriptSavedState ^ SourceState,
	__in INWScriptStack * Stack,
	__out UInt32 * ResumeMethodId,
	__out PROGRAM_COUNTER * ResumeMethodPC,
	__out UInt32 * SaveGlobalCount,
	__out UInt32 * SaveLocalCount,
	__out NWN::OBJECTID * CurrentActionObjectSelf
	)
/*++

Routine Description:

	This routine creates a copy of an existing saved state by pushing it onto
	a VM stack (perhaps for serialization purposes).

Arguments:

	SourceState - Supplies the source saved state to save to the stack.  The
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

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	try
	{
		array< Object ^ > ^ Vars;

		if ((m_CodeGenFlags & NWCGF_ENABLE_SAVESTATE_TO_VMSTACK) == 0)
			throw gcnew Exception( "Script program not generated with save state to stack enabled." );

		*ResumeMethodId          = SourceState->GetResumeMethodId( );
		*ResumeMethodPC          = SourceState->GetResumeMethodPC( );
		*CurrentActionObjectSelf = SourceState->GetCurrentActionObjectSelf( );

		Vars = SourceState->GetGlobals( );

		if (Vars == nullptr)
			*SaveGlobalCount = 0;
		else
			*SaveGlobalCount = (UInt32) Vars->Length;

		if (Vars != nullptr)
			PushVariablesToStack( Stack, Vars );

		//
		// Push a dummy saved BP on to the stack so that the saved state image
		// is compatible with the VM.
		//

		PushVariablesToStack( Stack, gcnew array< Object ^ >{ gcnew Int32( 0 ) } );

		Vars = SourceState->GetLocals( );

		if (Vars == nullptr)
			*SaveLocalCount = 0;
		else
			*SaveLocalCount = (UInt32) Vars->Length;

		if (Vars != nullptr)
			PushVariablesToStack( Stack, Vars );
	}
	catch (Exception ^ e)
	{
		ErrorException( e );
		throw;
	}
}

NWScriptSavedState ^
NWScriptProgram::PopSavedState(
	__in INWScriptStack * Stack,
	__in UInt32 ResumeMethodId,
	__in PROGRAM_COUNTER ResumeMethodPC,
	__in UInt32 SaveGlobalCount,
	__in UInt32 SaveLocalCount,
	__in NWN::OBJECTID CurrentActionObjectSelf
	)
/*++

Routine Description:

	This routine instantiates a saved state from variables stored on a VM
	stack.

	N.B.  Attempting to restore a stack with an incorrect number of values on
	      the stack will result in a restore-time exception.  Attempting to
	      restore a stack with incorrect types on the stack may result in a
	      script abort at runtime (but will not compromise the integrity of
	      the host process, though the behavior of the script is
	      unpredictable in such a case).

Arguments:

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

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	try
	{
		IGeneratedScriptProgram ^ Program;
		array< Object ^ >       ^ Locals;
		array< Object ^ >       ^ SavedBP;
		array< Object ^ >       ^ Globals;

		if ((m_CodeGenFlags & NWCGF_ENABLE_SAVESTATE_TO_VMSTACK) == 0)
			throw gcnew Exception( "Script program not generated with save state to stack enabled." );

		Program = m_ProgramObject->CloneScriptProgram( );
		Locals  = PopVariablesFromStack( Stack, SaveLocalCount );
		SavedBP = PopVariablesFromStack( Stack, 1 );
		Globals = PopVariablesFromStack( Stack, SaveGlobalCount );

		//
		// Update globals with the values loaded from the stack.
		//

		if (Globals != nullptr)
			Program->LoadScriptGlobals( Globals );

		//
		// Construct the underlying saved state object, and return it out to
		// the caller.
		//

		return gcnew NWScriptSavedState(
			this,
			Program,
			Locals,
			ResumeMethodId,
			Stack,
			CurrentActionObjectSelf,
			ResumeMethodPC,
			Globals);
	}
	catch (Exception ^ e)
	{
		ErrorException( e );
		throw;
	}
}


void
NWScriptProgram::Intrinsic_VMStackPushInt(
	__in Int32 i
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to push a value onto
	the VM stack for an action service handler call.

Arguments:

	i - Supplies the value to push onto the VM stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
	try
	{
		if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
			m_TextOut->WriteText( "VMPUSH: (int) %d\n", i );

		m_Stack->StackPushInt( i );
	}
	catch (...)
	{
		throw gcnew Exception( "StackPushInt failed." );
	}
}

Int32
NWScriptProgram::Intrinsic_VMStackPopInt(
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to pop a value off
	of the VM stack for an action service handler call.

Arguments:

	None.

Return Value:

	An integer value.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
	try
	{
		Int32 i;

		i =  m_Stack->StackPopInt( );

		if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
			m_TextOut->WriteText( "VMPOP: (int) %d\n", i );

		return i;
	}
	catch (...)
	{
		throw gcnew Exception( "StackPopInt failed." );
	}
}

void
NWScriptProgram::Intrinsic_VMStackPushFloat(
	__in Single f
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to push a value onto
	the VM stack for an action service handler call.

Arguments:

	f - Supplies the value to push onto the VM stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
	try
	{
		if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
			m_TextOut->WriteText( "VMPUSH: (float) %g\n", (float) f );

		m_Stack->StackPushFloat( f );
	}
	catch (...)
	{
		throw gcnew Exception( "StackPushFloat failed." );
	}
}

Single
NWScriptProgram::Intrinsic_VMStackPopFloat(
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to pop a value off
	of the VM stack for an action service handler call.

Arguments:

	None.

Return Value:

	A floating point value.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
	try
	{
		Single f;
		
		f = m_Stack->StackPopFloat( );

		if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
			m_TextOut->WriteText( "VMPOP: (float) %g\n",(float)  f );

		return f;
	}
	catch (...)
	{
		throw gcnew Exception( "StackPopFloat failed." );
	}
}

void
NWScriptProgram::Intrinsic_VMStackPushString(
	__in String ^ s
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to push a value onto
	the VM stack for an action service handler call.

Arguments:

	s - Supplies the value to push onto the VM stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
	try
	{
		if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
			m_TextOut->WriteText( "VMPUSH: (string) %s\n", NWScriptUtilities::ConvertString( s ).c_str( ) );

		INWScriptStack::NeutralString NeutralStr;

		NeutralStr = ConvertStringToNeutral( s );

		try
		{
			m_Stack->StackPushStringAsNeutral( NeutralStr );
		}
		catch (...)
		{
			NWScriptStack::FreeNeutral( NeutralStr.first );
			throw;
		}

		NWScriptStack::FreeNeutral( NeutralStr.first );
	}
	catch (...)
	{
		throw gcnew Exception( "StackPushString failed." );
	}
}

String ^
NWScriptProgram::Intrinsic_VMStackPopString(
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to pop a value off
	of the VM stack for an action service handler call.

Arguments:

	None.

Return Value:

	A string value.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
	try
	{
		INWScriptStack::NeutralString Str(
			m_Stack->StackPopStringAsNeutral( ) );

		try
		{
			String ^ MSIL_Str;

			if (Str.second != 0)
			{
				MSIL_Str = gcnew String(
						Str.first,
						0,
						(int) Str.second,
						m_StringEncoding);
			}
			else
			{
				MSIL_Str = "";
			}

			NWScriptStack::FreeNeutral( Str.first );

			Str.first = NULL;

			if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
				m_TextOut->WriteText( "VMPOP: (string) %s\n", NWScriptUtilities::ConvertString( MSIL_Str ).c_str( ) );

			return MSIL_Str;
		}
		catch (Exception ^)
		{
			NWScriptStack::FreeNeutral( Str.first );
			throw;
		}
	}
	catch (...)
	{
		throw gcnew Exception( "StackPopString failed." );
	}
}

void
NWScriptProgram::Intrinsic_VMStackPushObjectId(
	__in UInt32 o
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to push a value onto
	the VM stack for an action service handler call.

Arguments:

	o - Supplies the value to push onto the VM stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
	try
	{
		if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
			m_TextOut->WriteText( "VMPUSH: (object) %08X\n", o );

		m_Stack->StackPushObjectId( o );
	}
	catch (...)
	{
		throw gcnew Exception( "StackPushObjectId failed." );
	}
}

UInt32
NWScriptProgram::Intrinsic_VMStackPopObjectId(
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to pop a value off
	of the VM stack for an action service handler call.

Arguments:

	None.

Return Value:

	An object id value.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
	try
	{
		UInt32 o;

		o = m_Stack->StackPopObjectId( );

		if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
			m_TextOut->WriteText( "VMPOP: (object) %08X\n", o );

		return o;
	}
	catch (...)
	{
		throw gcnew Exception( "StackPopObjectId failed." );
	}
}

void
NWScriptProgram::Intrinsic_VMStackPushEngineStructure(
	__in INWScriptEngineStructure ^ EngineStructure
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to push a value onto
	the VM stack for an action service handler call.

Arguments:

	EngineStructure - Supplies the value to push onto the VM stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
	try
	{
		NWScriptEngineStructure ^ EngStruct;

		if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
			m_TextOut->WriteText( "VMPUSH: (EngineStructure)\n" );

		EngStruct = (NWScriptEngineStructure ^) EngineStructure;

		VMStackPushEngineStructureInternal(
			m_Stack,
			EngStruct->m_EngineStructure );
	}
	catch (...)
	{
		throw gcnew Exception( "StackPushEngineStructure failed." );
	}
}

INWScriptEngineStructure ^
NWScriptProgram::Intrinsic_VMStackPopEngineStructure(
	__in int EngType
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to pop a value off
	of the VM stack for an action service handler call.

Arguments:

	EngType - Supplies the type of the engine structure.

Return Value:

	An engine structure handle.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
	try
	{
		NWScriptEngineStructure ^ EngStruct;

		EngStruct = gcnew NWScriptEngineStructure(
			m_Stack->StackPopEngineStructure(
				(INWScriptStack::ENGINE_STRUCTURE_NUMBER) EngType
				)
			);

		if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
			m_TextOut->WriteText( "VMPOP: (EngineStructure_%d)\n", EngType );

		return EngStruct;
	}
	catch (...)
	{
		throw gcnew Exception( "StackPopEngineStructure failed." );
	}
}

bool
NWScriptProgram::Intrinsic_CompareEngineStructure(
	__in INWScriptEngineStructure ^ EngineStructure1,
	__in INWScriptEngineStructure ^ EngineStructure2
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to compare two
	different engine structures for logical equality.

Arguments:

	EngineStructure1 - Supplies the first engine structure.

	EngineStructure2 - Supplies the second engine structure.

Return Value:

	The routine returns true if the engine structures are logically equal, else
	false if they are logically not equal.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
	NWScriptEngineStructure ^ EngStruct1;
	NWScriptEngineStructure ^ EngStruct2;
	const EngineStructure   * RawEs1;
	const EngineStructure   * RawEs2;

	EngStruct1 = (NWScriptEngineStructure ^) EngineStructure1;
	EngStruct2 = (NWScriptEngineStructure ^) EngineStructure2;

	RawEs1 = (*EngStruct1->m_EngineStructure).get( );
	RawEs2 = (*EngStruct2->m_EngineStructure).get( );

	if ((RawEs1 == NULL) || (RawEs2 == NULL))
	{
		if (RawEs1 == RawEs2)
			return true;
		else
			return false;
	}

	if (RawEs1->GetEngineType( ) != RawEs2->GetEngineType( ))
		throw gcnew Exception( "Comparing engine structures of incompatible types." );

	return RawEs1->CompareEngineStructure( RawEs2 );
}

INWScriptEngineStructure ^
NWScriptProgram::Intrinsic_CreateEngineStructure(
	__in int EngType
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to create a default
	value (empty) engine structure of a given type.

Arguments:

	EngType - Supplies the type of the engine structure.

Return Value:

	An engine structure handle.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
	try
	{
		NWScriptEngineStructure ^ EngStruct;

		EngStruct = gcnew NWScriptEngineStructure(
			m_ActionHandler->CreateEngineStructure(
				(INWScriptStack::ENGINE_STRUCTURE_NUMBER) EngType
				)
			);

		return EngStruct;
	}
	catch (...)
	{
		throw gcnew Exception( "CreateEngineStructure failed." );
	}
}

void
NWScriptProgram::Intrinsic_ExecuteActionService(
	__in UInt32 ActionId,
	__in UInt32 NumArguments
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to transfer control
	a script host's action service handler.

	Action service handler parameters and return values are passed on the VM
	stack object supplied by the user.  The VM stack is assumed to have been
	already prepared by the time this routine is invoked.

Arguments:

	ActionId - Supplies the action service ordinal that was requested.

	NumArguments - Supplies the count of arguments passed to the action ordinal.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
	try
	{
		if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
			m_TextOut->WriteText( "VMACTION: %lu (%lu arguments)\n", (unsigned long) ActionId, (unsigned long) NumArguments );

		if (!m_ActionHandler->OnExecuteActionFromJIT(
			(NWSCRIPT_ACTION) ActionId,
			(size_t) NumArguments))
		{
			String ^ Str = "Action service ";
			
			Str += ActionId;
			Str += " invocation failed.";

			throw gcnew Exception( Str );
		}

		if (IsScriptAborted( ))
			throw gcnew Exception( "Script aborted." );
	}
	catch (...)
	{
		throw gcnew Exception( "ExecuteActionService failed." );
	}
}

void
NWScriptProgram::Intrinsic_StoreState(
	__in_opt array< Object ^ > ^ Globals,
	__in array< Object ^ > ^ Locals,
	__in UInt32 ResumeMethodPC,
	__in UInt32 ResumeMethodId,
	__in IGeneratedScriptProgram ^ ProgramObject
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to save the state of
	the current subroutine for resume at a resume subroutine.

	All global variables, plus the specified locals are stored.  The locals are
	turned into parameters for a call to the resume method.

Arguments:

	Globals - Optionally supplies the global variables to save.

	Locals - Supplies the local variables to save.

	ResumeMethodPC - Supplies the NWScript program counter corresponding to the
	                 resume method.

	ResumeMethodId - Supplies the script situation identifier of the resume
	                 method, for a fast resume.

	ProgramObject - Supplies a clone of the program object, including the
	                current global variables state.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
	//
	// Update the current saved state snapshot.
	//

	m_SavedState = CreateSavedState(
		Globals,
		Locals,
		ResumeMethodPC,
		ResumeMethodId,
		ProgramObject);

#if NWSCRIPT_TEST_SAVE_VMSTACK
	if (m_CodeGenFlags & NWCGF_ENABLE_SAVESTATE_TO_VMSTACK)
	{
		UInt32          ResumeMethodId;
		PROGRAM_COUNTER ResumeMethodPC;
		UInt32          SaveGlobalCount;
		UInt32          SaveLocalCount;
		NWN::OBJECTID   CurrentActionObjectSelf;

		PushSavedState(
			m_SavedState,
			m_Stack,
			&ResumeMethodId,
			&ResumeMethodPC,
			&SaveGlobalCount,
			&SaveLocalCount,
			&CurrentActionObjectSelf);

		m_SavedState = PopSavedState(
			m_Stack,
			ResumeMethodId,
			ResumeMethodPC,
			SaveGlobalCount,
			SaveLocalCount,
			CurrentActionObjectSelf);
	}
#endif

}

Object ^
NWScriptProgram::Intrinsic_ExecuteActionServiceFast(
	__in UInt32 ActionId,
	__in UInt32 NumArguments,
	__in ... array< Object ^ > ^ Arguments
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to transfer control
	a script host's action service handler.  The fast action service intrinsic
	combines the operations of pushing parameters on the stack, calling the
	action service handler, and popping return values off of the stack.

	Action service handler parameters and return values are passed on the VM
	stack object supplied by the user.  The VM stack is assumed to have been
	already prepared by the time this routine is invoked.

Arguments:

	ActionId - Supplies the action service ordinal that was requested.

	NumArguments - Supplies the count of arguments passed to the action ordinal.

	Arguments - Supplies the actual argument array to pass.

Return Value:

	The return value, if any, is returned.  If the return type of the action
	service handler was an ACTIONTYPE_VECTOR, a NWScript::Vector3 is returned.
	
	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
	PCNWACTION_DEFINITION          CalledAction;
	PNWFASTACTION_CMD              Cmds;
	uintptr_t                    * CmdParams;
	size_t                         NumCmds;
	size_t                         NumReturnTypes;
	size_t                         NumParams;
	Int32                        ^ RetInt;
	Single                       ^ RetFloat;
	UInt32                       ^ RetObjectId;
	String                       ^ RetString;
	NWScript::Vector3            ^ RetVector;
	size_t                         ParamIndex;
	Int32                          i;
	NeutralStrList                 ParamStrings;

	//
	// Retrieve the action definition from the data table, allocate storage for
	// the command and command parameter arrays, and determine the number of
	// return types.
	//
	// The command and command parameter arrays will be filled in based on the
	// action definition.
	//

#if NWSCRIPT_DEBUG_ACTION_CALLS
	if (ActionId >= m_ActionCount)
		throw gcnew Exception( "Attempt to invoke out of range action service handler." );
#endif

	CalledAction = &m_ActionDefs[ ActionId ];

	switch (CalledAction->ReturnType)
	{

	case ACTIONTYPE_VECTOR:
		NumReturnTypes = 3;
		RetVector      = gcnew NWScript::Vector3;
		break;

	case ACTIONTYPE_VOID:
		NumReturnTypes = 0;
		break;

	case ACTIONTYPE_INT:
		NumReturnTypes = 1;
		RetInt         = gcnew Int32;
		break;

	case ACTIONTYPE_FLOAT:
		NumReturnTypes = 1;
		RetFloat       = gcnew Single;
		break;

	case ACTIONTYPE_OBJECT:
		NumReturnTypes = 1;
		RetObjectId    = gcnew UInt32;
		break;

	case ACTIONTYPE_STRING:
		NumReturnTypes = 1;
		RetString      = nullptr;
		break;

	default:
		throw gcnew Exception( "Unsupported return type for fast action service call." );

	}

#if NWSCRIPT_DEBUG_ACTION_CALLS
	if (Arguments->Length + 1 < Arguments->Length)
		throw gcnew Exception( "Too many arguments." );
	else if (Arguments->Length + 1 + NumReturnTypes < Arguments->Length)
		throw gcnew Exception( "Too many arguments." );
#endif

	NumCmds = (size_t) (Arguments->Length + 1 + NumReturnTypes);
	Cmds    = (PNWFASTACTION_CMD) _alloca( sizeof( *Cmds ) * (NumCmds) );

	if (Arguments->Length != 0)
		CmdParams = (uintptr_t *) _alloca( sizeof( *CmdParams ) * (NumCmds - 1) );
	else
		CmdParams = NULL;

	NumParams = (size_t) Arguments->Length;

	//
	// Now prepare the command and command parameter lists.  First, create push
	// orders for arguments.
	//
	// N.B.  The argument order has already been swapped in the argument array
	//       for ease of matching with the action descriptor.
	//

	ParamIndex = 0;
	i          = (Int32) NumParams;

	for (Int32 n = 0; n < (Int32) NumParams; n += 1)
	{
#if NWSCRIPT_DEBUG_ACTION_CALLS
		if (ParamIndex >= CalledAction->NumParameters)
			throw gcnew Exception( "Invoked action with too many parameters." );
#endif

		switch (CalledAction->ParameterTypes[ ParamIndex ])
		{

		case ACTIONTYPE_ACTION:
			ParamIndex += 1;
			n -= 1;

			continue;

		case ACTIONTYPE_VECTOR:
#if NWSCRIPT_DEBUG_ACTION_CALLS
			if (i - 3 < 0)
				throw gcnew Exception( "Passed partial vector to action service handler." );
#endif

			for (Int32 ii = 0; ii < 3; ii += 1)
			{
				i -= 1;

				Cmds[ i ]                  = NWFASTACTION_PUSHFLOAT;
				*(float *) &CmdParams[ i ] = (float) (Single) Arguments[ n + ii ];
			}

			n          += 2; // +1 after loop
			ParamIndex += 1;
			break;

		case ACTIONTYPE_INT:
			i -= 1;

			Cmds[ i ]      = NWFASTACTION_PUSHINT;
			CmdParams[ i ] = (uintptr_t) (Int32) Arguments[ n ];
			ParamIndex += 1;
			break;

		case ACTIONTYPE_FLOAT:
			{
				i -= 1;

				Cmds[ i ]                  = NWFASTACTION_PUSHFLOAT;
				*(float *) &CmdParams[ i ] = (float) (Single) Arguments[ n ];
				ParamIndex += 1;
			}
			break;

		case ACTIONTYPE_OBJECT:
			i -= 1;

			Cmds[ i ]      = NWFASTACTION_PUSHOBJECTID;
			CmdParams[ i ] = (uintptr_t) (UInt32) Arguments[ n ];
			ParamIndex += 1;
			break;

		case ACTIONTYPE_STRING:
			{
#if NWNSCRIPTJIT_CPP0X
				ParamStrings.emplace_back( AutoNeutralString( ) );
#else
				ParamStrings.push_back( AutoNeutralString( ) );
#endif

				INWScriptStack::NeutralString & NeutralStr = ParamStrings.back( ).Str;

				NeutralStr = ConvertStringToNeutral( (String ^) Arguments[ n ] );

				i -= 1;

				Cmds[ i ]      = NWFASTACTION_PUSHSTRING;
				CmdParams[ i ] = (uintptr_t) &NeutralStr;

				ParamIndex += 1;
			}
			break;

#if NWSCRIPT_DEBUG_ACTION_CALLS
		default:
			throw gcnew Exception( "Illegal parameter type for fast action service." );
#endif

		}
	}

	Cmds[ NumParams ] = NWFASTACTION_CALL;

	switch (CalledAction->ReturnType)
	{

	case ACTIONTYPE_VECTOR:
		{
			pin_ptr< NWScript::Vector3 > PinRetVector = &*RetVector;

			Cmds[ NumParams + 1 ]     = NWFASTACTION_POPFLOAT;
			CmdParams[ NumParams + 0 ] = (uintptr_t) (float *) &PinRetVector->z;
			Cmds[ NumParams + 2 ]     = NWFASTACTION_POPFLOAT;
			CmdParams[ NumParams + 1 ] = (uintptr_t) (float *) &PinRetVector->y;
			Cmds[ NumParams + 3 ]     = NWFASTACTION_POPFLOAT;
			CmdParams[ NumParams + 2 ] = (uintptr_t) (float *) &PinRetVector->x;

			FastInvokeActionServiceHandler(
				ActionId,
				NumArguments,
				Cmds,
				NumCmds,
				CmdParams);

			return RetVector;
		}
		break;

	case ACTIONTYPE_VOID:
		{
			FastInvokeActionServiceHandler(
				ActionId,
				NumArguments,
				Cmds,
				NumCmds,
				CmdParams);

			return nullptr;
		}
		break;

	case ACTIONTYPE_INT:
		{
			pin_ptr< Int32 > PinRetInt = &*RetInt;

			Cmds[ NumParams + 1 ]      = NWFASTACTION_POPINT;
			CmdParams[ NumParams + 0 ] = (uintptr_t) (int *) &PinRetInt;

			FastInvokeActionServiceHandler(
				ActionId,
				NumArguments,
				Cmds,
				NumCmds,
				CmdParams);

			return RetInt;
		}
		break;

	case ACTIONTYPE_FLOAT:
		{
			pin_ptr< Single > PinRetFloat = &*RetFloat;

			Cmds[ NumParams + 1 ]      = NWFASTACTION_POPFLOAT;
			CmdParams[ NumParams + 0 ] = (uintptr_t) (float *) &PinRetFloat;

			FastInvokeActionServiceHandler(
				ActionId,
				NumArguments,
				Cmds,
				NumCmds,
				CmdParams);

			return RetFloat;
		}
		break;

	case ACTIONTYPE_OBJECT:
		{
			pin_ptr< UInt32 > PinRetObjectId = &*RetObjectId;

			Cmds[ NumParams + 1 ]      = NWFASTACTION_POPOBJECTID;
			CmdParams[ NumParams + 0 ] = (uintptr_t) (NWN::OBJECTID *) &PinRetObjectId;

			FastInvokeActionServiceHandler(
				ActionId,
				NumArguments,
				Cmds,
				NumCmds,
				CmdParams);

			return RetObjectId;
		}

	case ACTIONTYPE_STRING:
		{
			AutoNeutralString RetStr;

			Cmds[ NumParams + 1 ]      = NWFASTACTION_POPSTRING;
			CmdParams[ NumParams + 0 ] = (uintptr_t) &RetStr.Str;

			FastInvokeActionServiceHandler(
				ActionId,
				NumArguments,
				Cmds,
				NumCmds,
				CmdParams);

			if (RetStr.Str.second != 0)
			{
				RetString = gcnew String(
					RetStr.Str.first,
					0,
					(Int32) RetStr.Str.second,
					m_StringEncoding);
			}
			else
			{
				RetString = "";
			}

			return RetString;
		}
		break;

	default:
		throw gcnew Exception( "Illegal return type for fast action service." );

	}
}

NWScript::NeutralStringStorage
NWScriptProgram::Intrinsic_AllocateNeutralString(
	__in String ^ Str
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to convert a String
	into a NeutralString, for a call to an action service.

Arguments:

	Str - Supplies the string to convert.

Return Value:

	The allocated string is returned.  On failure, a System::Exception is
	raised.

	N.B.  The caller must manually clean up the NeutralStringStorage object !!
          There is no automatic cleanup finalizer, cleanup must be performed
	      in conjuction with an exception handler.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
#if NWSCRIPT_DIRECT_FAST_ACTION_CALLS
	NWScript::NeutralStringStorage NStr;
	INWScriptStack::NeutralString  NStrNative;

	NStrNative = ConvertStringToNeutral( Str );

	NStr.StrPtr = (IntPtr) (void *) NStrNative.first;
	NStr.Length = (IntPtr) (void *) NStrNative.second;

	return NStr;
#else
	Str;

	throw gcnew Exception( "Intrinsic_AllocateNeutralString is not supported if direct fast action calls are not enabled." );
#endif
}

void
NWScriptProgram::Intrinsic_DeleteNeutralString(
	__in NWScript::NeutralStringStorage % Str
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to release the
	native resources associated with a NeutralString object.

Arguments:

	Str - Supplies the string to release.  The caller bears responsibility for
	      not deleting a string twice.

Return Value:

	None.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
#if NWSCRIPT_DIRECT_FAST_ACTION_CALLS
	void * Ptr;

	Ptr = (void *) Str.StrPtr;

	if (Ptr == NULL)
		return;

	Str.StrPtr = IntPtr::Zero;

	NWScriptStack::FreeNeutral( Ptr );
#else
	Str;

	throw gcnew Exception( "Intrinsic_DeleteNeutralString is not supported if direct fast action calls are not enabled." );
#endif
}

String ^
NWScriptProgram::Intrinsic_NeutralStringToString(
	__in NWScript::NeutralStringStorage % Str
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to convert a
	NeutralString object to a CLR String.

Arguments:

	Str - Supplies the string to convert to a CLR string.

Return Value:

	The corresponding CLR string for the given neutral string is returned.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
#if NWSCRIPT_DIRECT_FAST_ACTION_CALLS
	if (Str.Length == IntPtr::Zero)
	{
		return "";
	}
	else
	{
		return gcnew String(
			(char *) (void *) Str.StrPtr,
			0,
			(Int32) Str.Length,
			m_StringEncoding);
	}
#else
	Str;

	throw gcnew Exception( "Intrinsic_NeutralStringToString is not supported if direct fast action calls are not enabled." );
#endif
}

void
NWScriptProgram::Intrinsic_CheckScriptAbort(
	__in bool ActionSucceeded
	)
/*++

Routine Description:

	This routine is invoked on behalf of NWScript in order to raise a program
	abort exception (if necessary).

Arguments:

	ActionSucceeded - Supplies a Boolean value indicating whether the previous
	                  action service call succeeded.  If ActionSucceeded is
	                  false, or the internal abort flag on the program object
	                  has been set, then an abort exception is raised.

Return Value:

	None.  The routine raises a System::Exception if an abort condition was
	detected.

Environment:

	User mode, C++/CLI, invoked as intrinsic from emitted code.

--*/
{
	if ((!ActionSucceeded) || (IsScriptAborted( )))
		throw gcnew Exception( "Action service handler invocation failed." );
}



array< Object ^ > ^
NWScriptProgram::ConvertParameterList(
	__in_ecount_opt( ParamCount ) const NWScriptParamString * Params,
	__in size_t ParamCount
	)
/*++

Routine Description:

	This routine converts parameters to a script to their native types.

Arguments:

	Params - Supplies the parameters to convert.

	ParamCount - Supplies the count of parameters.

Return Value:

	A handle to an array describing parameter instances for the given set of
	parameter values is returned.  If there were no parameters to actually pass
	to the entry point, nullptr is returned.

Environment:

	User mode, C++/CLI.

--*/
{
	if (m_EntryPointParamTypes == nullptr)
		return nullptr;

	array< Object ^ > ^ NativeParams = gcnew array< Object ^ >(
		m_EntryPointParamTypes->Length);

	//
	// Now map the untyped parameters into typed values based on the expected
	// types of each one in the script program.
	//

	for (int i = 0; i < m_EntryPointParamTypes->Length; i += 1)
	{
		const char * ParamValue;
		size_t       ParamValueLen;

		if (i < (int) ParamCount)
		{
			ParamValue    = Params[ i ].String;
			ParamValueLen = Params[ i ].Len;
		}
		else
		{
			ParamValue    = "";
			ParamValueLen = 0;
		}

		switch (m_EntryPointParamTypes[ i ])
		{

		case ACTIONTYPE_INT:
		case ACTIONTYPE_VOID: // Unused parameters default to integers
			NativeParams[ i ] = gcnew Int32( atoi( ParamValue ) );
			break;

		case ACTIONTYPE_FLOAT:
			NativeParams[ i ] = gcnew Single( (float) atof( ParamValue ) );
			break;

		case ACTIONTYPE_STRING:
			NativeParams[ i ] = gcnew String(
				ParamValue,
				0,
				(int) ParamValueLen,
				m_StringEncoding);
			break;

		case ACTIONTYPE_OBJECT:
			{
				char          * Endp;
				NWN::OBJECTID   ObjectId;

				ObjectId = (NWN::OBJECTID) _strtoui64(
					ParamValue,
					&Endp,
					10);

				//
				// If the conversion failed, return the invalid object id.
				//

				if (*Endp)
					ObjectId = m_InvalidObjId;

				NativeParams[ i ] = gcnew UInt32( ObjectId );
			}
			break;

		default:
			{
				String ^ Str = "Illegal entry point parameter type ";
				int      Pt;

				Pt  = (int) m_EntryPointParamTypes[ i ];
				Str += Pt;
				Str += " (#";
				Str += i;
				Str += ").";

				throw gcnew Exception( Str );
			}
			break;

		}
	}

	return NativeParams;
}


NWScriptSavedState ^
NWScriptProgram::CreateSavedState(
	__in_opt array< Object ^ > ^ Globals,
	__in array< Object ^ > ^ Locals,
	__in UInt32 ResumeSubroutinePC,
	__in UInt32 ResumeSubroutineId,
	__in IGeneratedScriptProgram ^ ProgramObject
	)
/*++

Routine Description:

	This routine saves a copy of the state necessary to support the most recent
	SAVE_STATE operation.

Arguments:

	Globals - Optionally supplies the global variables to save.

	Locals - Optionally supplies the list of locals to save.

	ResumeMethodPC - Supplies the NWScript program counter corresponding to the
	                 resume method.

	ResumeSubroutineId - Supplies the script situation id of the subroutine to
	                     execute on resume.

	ProgramObject - Supplies a clone of the program object, including the
	                current global variables state.

Return Value:

	The routine returns a new NWScriptSavedState handle on success.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	try
	{
		//
		// Construct the underlying saved state object, and return it out to
		// the caller.
		//

		return gcnew NWScriptSavedState(
			this,
			ProgramObject,
			Locals,
			ResumeSubroutineId,
			m_Stack,
			m_CurrentActionObjectSelf,
			ResumeSubroutinePC,
			Globals);
	}
	catch (Exception ^ e)
	{
		ErrorException( e );
		throw;
	}
}


void
NWScriptProgram::FastInvokeActionServiceHandler(
	__in NWSCRIPT_ACTION ActionId,
	__in size_t NumArguments,
	__in_ecount( NumCmds ) PCNWFASTACTION_CMD Cmds,
	__in size_t NumCmds,
	__in uintptr_t * CmdParams
	)
/*++

Routine Description:

	This routine wraps the transition to native code when performing a fast
	action service handler call.

Arguments:

	ActionId - Supplies the action service ordinal that was requested.

	NumArguments - Supplies the count of arguments passed to the action ordinal.

	Cmds - Supplies the array of fast action commands describing how to pass
	       the parameters to the action handler.

	NumCmds - Supplies the count of fast action commands.

	CmdParams - Supplies the array of fast action command arguments, which are
	            interpreted based on the Cmds array.


Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	try
	{
		if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
			m_TextOut->WriteText( "VMACTIONFAST: %lu (%lu arguments)\n", ActionId, NumArguments );

		if (!m_ActionHandler->OnExecuteActionFromJITFast(
			(NWSCRIPT_ACTION) ActionId,
			(size_t) NumArguments,
			Cmds,
			NumCmds,
			CmdParams))
		{
			String ^ Str = "Action service ";
			
			Str += ActionId;
			Str += " invocation failed.";

			throw gcnew Exception( Str );
		}

		if (IsScriptAborted( ))
			throw gcnew Exception( "Script aborted." );
	}
	catch (...)
	{
		throw gcnew Exception( "ExecuteActionServiceFast failed." );
	}
}


void
NWScriptProgram::PushVariablesToStack(
	__in INWScriptStack * Stack,
	__in array< Object ^ > ^ Vars
	)
/*++

Routine Description:

	This routine places a variable set onto a VM stack.

Arguments:

	Stack - Supplies the stack to store the variables to.

	Vars - Supplies the variable set to place on the stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	if (Vars == nullptr)
		return;

	try
	{
		for (Int32 n = Vars->Length; n != 0; n -= 1)
		{
			NWACTION_TYPE VarType;

			VarType = GetNWScriptType( Vars[ n - 1 ] );

			switch (VarType)
			{

			case ACTIONTYPE_INT:
				{
					Int32 i = (Int32) Vars[ n - 1 ];

					if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
						m_TextOut->WriteText( "VMPUSH: (int) %d\n", i );

					Stack->StackPushInt( i );
				}
				break;

			case ACTIONTYPE_FLOAT:
				{
					Single f = (Single) Vars[ n - 1 ];

					if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
						m_TextOut->WriteText( "VMPUSH: (float) %g\n", (float) f );

					Stack->StackPushFloat( f );
				}
				break;

			case ACTIONTYPE_STRING:
				{
					String ^ s = (String ^) Vars[ n - 1 ];

					if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
						m_TextOut->WriteText( "VMPUSH: (string) %s\n", NWScriptUtilities::ConvertString( s ).c_str( ) );

					INWScriptStack::NeutralString NeutralStr;

					NeutralStr = ConvertStringToNeutral( s );

					try
					{
						Stack->StackPushStringAsNeutral( NeutralStr );
					}
					catch (...)
					{
						NWScriptStack::FreeNeutral( NeutralStr.first );
						throw;
					}

					NWScriptStack::FreeNeutral( NeutralStr.first );
				}
				break;

			case ACTIONTYPE_OBJECT:
				{
					UInt32 o = (UInt32) Vars[ n - 1 ];

					if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
						m_TextOut->WriteText( "VMPUSH: (object) %08X\n", o );

					Stack->StackPushObjectId( o );
				}
				break;

#define DECL_ENGINE_STRUCTURE_CASE(EType)                                                   \
			case ACTIONTYPE_ENGINE_##EType:                                                 \
				{                                                                           \
					NWScriptEngineStructure##EType ^ EngStruct;                             \
					NWScriptEngineStructure        ^ EngStructI;                            \
					                                                                        \
					EngStruct  = (NWScriptEngineStructure##EType ^) Vars[ n - 1 ];          \
					EngStructI = (NWScriptEngineStructure ^) EngStruct->m_EngineStructure;  \
					                                                                        \
					Stack->StackPushEngineStructure( *EngStructI->m_EngineStructure );      \
				}                                                                           \
				break;                                                                      

			DECL_ENGINE_STRUCTURE_CASE(0);
			DECL_ENGINE_STRUCTURE_CASE(1);
			DECL_ENGINE_STRUCTURE_CASE(2);
			DECL_ENGINE_STRUCTURE_CASE(3);
			DECL_ENGINE_STRUCTURE_CASE(4);
			DECL_ENGINE_STRUCTURE_CASE(5);
			DECL_ENGINE_STRUCTURE_CASE(6);
			DECL_ENGINE_STRUCTURE_CASE(7);
			DECL_ENGINE_STRUCTURE_CASE(8);
			DECL_ENGINE_STRUCTURE_CASE(9);

#undef DECL_ENGINE_STRUCTURE_CASE

			default:
				throw gcnew Exception( "Attempted to save variable of unknown type." );

			}
		}
	}
	catch (...)
	{
		throw gcnew Exception( "StackPush failed." );
	}
}

array< Object ^ > ^
NWScriptProgram::PopVariablesFromStack(
	__in INWScriptStack * Stack,
	__in UInt32 SaveVarCount
	)
/*++

Routine Description:

	This routine restores a variable set from a VM stack.

Arguments:

	Stack - Supplies the stack to store the variables to.

	SaveVarCount - Supplies the count of variables to remove from the stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	array< Object ^ > ^ Vars = gcnew array< Object ^ >( (Int32) SaveVarCount );
	Int32               VarIdx;

	VarIdx = 0;

	try
	{
		for (VarIdx = 0; VarIdx < (Int32) SaveVarCount; VarIdx += 1)
		{
			INWScriptStack::BASE_STACK_TYPE VarType;

			VarType = Stack->GetTopOfStackType( );

			switch (VarType)
			{

			case INWScriptStack::BST_INT:
				{
					Int32 i = Stack->StackPopInt( );

					if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
						m_TextOut->WriteText( "VMPOP: (int) %d\n", i );

					Vars[ VarIdx ] = i;
				}
				break;

			case INWScriptStack::BST_FLOAT:
				{
					Single f = Stack->StackPopFloat( );

					if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
						m_TextOut->WriteText( "VMPOP: (float) %g\n", (float) f );

					Vars[ VarIdx ] = f;
				}
				break;

			case INWScriptStack::BST_STRING:
				{
					INWScriptStack::NeutralString Str(
						Stack->StackPopStringAsNeutral( ) );

					try
					{
						String ^ MSIL_Str;

						if (Str.second != 0)
						{
							MSIL_Str = gcnew String(
								Str.first,
								0,
								(int) Str.second,
								m_StringEncoding);
						}
						else
						{
							MSIL_Str = "";
						}

						NWScriptStack::FreeNeutral( Str.first );

						Str.first = NULL;

						if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
							m_TextOut->WriteText( "VMPOP: (string) %s\n", NWScriptUtilities::ConvertString( MSIL_Str ).c_str( ) );

						Vars[ VarIdx ] = MSIL_Str;
					}
					catch (Exception ^)
					{
						NWScriptStack::FreeNeutral( Str.first );
						throw;
					}
				}
				break;

			case INWScriptStack::BST_OBJECTID:
				{
					UInt32 o = Stack->StackPopObjectId( );

					if ((m_DebugLevel >= NWScriptVM::EDL_Verbose) && (m_TextOut != NULL))
						m_TextOut->WriteText( "VMPOP: (object) %08X\n", o );

					Vars[ VarIdx ] = o;
				}
				break;

#define DECL_ENGINE_STRUCTURE_CASE(EType)                                                   \
			case INWScriptStack::BST_ENGINE_##EType:                                        \
				{                                                                           \
					NWScriptEngineStructure##EType ^ EngStruct;                             \
					NWScriptEngineStructure        ^ EngStructI;                            \
					                                                                        \
					EngStructI = gcnew NWScriptEngineStructure(                             \
						Stack->StackPopEngineStructure(                                     \
							(INWScriptStack::ENGINE_STRUCTURE_NUMBER) (EType)               \
							)                                                               \
						);                                                                  \
					EngStruct = gcnew NWScriptEngineStructure##EType( EngStructI );         \
					                                                                        \
					Vars[ VarIdx ] = EngStruct;                                             \
				}                                                                           \
				break;                                                                      

			DECL_ENGINE_STRUCTURE_CASE(0);
			DECL_ENGINE_STRUCTURE_CASE(1);
			DECL_ENGINE_STRUCTURE_CASE(2);
			DECL_ENGINE_STRUCTURE_CASE(3);
			DECL_ENGINE_STRUCTURE_CASE(4);
			DECL_ENGINE_STRUCTURE_CASE(5);
			DECL_ENGINE_STRUCTURE_CASE(6);
			DECL_ENGINE_STRUCTURE_CASE(7);
			DECL_ENGINE_STRUCTURE_CASE(8);
			DECL_ENGINE_STRUCTURE_CASE(9);

#undef DECL_ENGINE_STRUCTURE_CASE

			default:
				throw gcnew Exception( "Attempted to restore variable of unknown type." );

			}
		}
	}
	catch (...)
	{
		throw gcnew Exception( "StackPop failed." );
	}

	return Vars;
}


void
NWScriptProgram::GenerateProgramCode(
	__in const NWScriptAnalyzer * Analyzer,
	__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams,
	__in NWN::OBJECTID ObjectInvalid,
	__in String ^ Name
	)
/*++

Routine Description:

	This routine constructs an MSIL representation of the functional nature of
	a NWScript program.  The representation is saved for invocation by native
	code (through the NWScriptProgram::ExecuteScript C++/CLI adapter).

Arguments:

	Analyzer - Supplies the analysis context that describes the NWScript IR to
	           generate code for.

	CodeGenParams - Optionally supplies extension code generation parameters.

	ObjectInvalid - Supplies the object id to reference for the 'object
	                invalid' manifest constant.

	Name - Supplies the name prefix to apply to the type within which the
	       script program MSIL representation is placed.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	NWScriptCodeGenerator              ^ Generator;
	NWScriptCodeGenerator::ProgramInfo   Program;

	//
	// Generate code for the program assembly.
	//

	Generator = gcnew NWScriptCodeGenerator(
		m_TextOut,
		m_DebugLevel,
		INWScriptProgram::typeid,
		IGeneratedScriptProgram::typeid);

	Generator->GenerateProgramCode(
		Analyzer,
		m_ActionHandler,
		ObjectInvalid,
		CodeGenParams,
		Name,
		m_StringEncoding,
		Program);

	delete Generator;

#if NWSCRIPT_SAVE_NWSCRIPTMANAGEDINTERFACE
	SaveManagedInterfaceDll( Analyzer, ObjectInvalid, CodeGenParams );
#endif

	m_EngineStructureTypes = Program.EngineStructureTypes;

	//
	// Now instantiate a copy of the compiled script program type.
	//

	m_ProgramObject = (IGeneratedScriptProgram ^) Program.Assembly->CreateInstance(
		Program.Type->FullName,
		false,
		BindingFlags::CreateInstance,
		nullptr,
		gcnew array< Object ^ >{ m_JITIntrinsics, this },
		nullptr,
		nullptr);
}


NWACTION_TYPE
NWScriptProgram::GetNWScriptType(
	__in Object ^ MSILVariable
	)
/*++

Routine Description:

	This routine returns the NWScript type code for an MSIL variable by its
	Object handle.

Arguments:

	MSILVariable - Supplies the MSIL object handle to return the NWScript type
	               code for.

Return Value:

	The routine returns the NWScript type code (drawn from the NWACTION_TYPE
	family of enumerations) for the given MSIL type.  If there is no mapping to
	NWScript types, i.e. the type is not a valid NWScript type, then a
	System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked at program runtime.

--*/
{
	Type ^ MSILType;

	MSILType = MSILVariable->GetType( );

	return GetNWScriptType( MSILType );
}

NWACTION_TYPE
NWScriptProgram::GetNWScriptType(
	__in Type ^ MSILType
	)
/*++

Routine Description:

	This routine returns the NWScript type code for an MSIL variable by its
	Type handle.

Arguments:

	MSILType - Supplies the MSIL type handle to return the NWScript type code
	           for.

Return Value:

	The routine returns the NWScript type code (drawn from the NWACTION_TYPE
	family of enumerations) for the given MSIL type.  If there is no mapping to
	NWScript types, i.e. the type is not a valid NWScript type, then a
	System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked at program construction time ONLY.

--*/
{
	if (MSILType == Int32::typeid)
		return ACTIONTYPE_INT;
	else if (MSILType == Single::typeid)
		return ACTIONTYPE_FLOAT;
	else if (MSILType == String::typeid)
		return ACTIONTYPE_STRING;
	else if (MSILType == UInt32::typeid)
		return ACTIONTYPE_OBJECT;
	else
	{
		for (int i = 0; i < NUM_ENGINE_STRUCTURE_TYPES; i += 1)
		{
			if (MSILType == m_EngineStructureTypes[ i ])
				return (NWACTION_TYPE) (ACTIONTYPE_ENGINE_0 + i);
		}

		throw gcnew ArgumentException(
			"Invalid type argument to GetNWScriptType." );
	}
}




void
NWScriptProgram::ErrorException(
	__in Exception ^ Excpt
	)
/*++

Routine Description:

	This routine issues an error diagnostic for an error-level exception.

Arguments:

	Excpt - Supplies the exception to issue the diagnostic for.

Return Value:

	None.  This routine does not fail.

Environment:

	User mode, C++/CLI.

--*/
{
	if (m_TextOut == NULL)
		return;

	if (m_DebugLevel < NWScriptVM::EDL_Errors)
		return;

	//
	// Issue the diagnostic warning to the user.
	//

	try
	{
		StringBuilder ^ Diagnostic = gcnew StringBuilder( "NWScriptProgram: Exception: '" );
		Exception     ^ InnerException;

		Diagnostic->Append( Excpt->Message );
		Diagnostic->Append( "', stack: " );
		Diagnostic->Append( Excpt->StackTrace );

		if ((InnerException = Excpt->InnerException) != nullptr)
		{
			Diagnostic->Append( ", InnerException: '" );
			Diagnostic->Append( InnerException->Message );
			Diagnostic->Append( "', stack: " );
			Diagnostic->Append( InnerException->StackTrace );
		}

		Diagnostic->Append( "." );

		std::string ConvStr = NWScriptUtilities::ConvertString( Diagnostic->ToString( ) );

		m_TextOut->WriteText(
			"%s\n",
			ConvStr.c_str( ));
	}
	catch (Exception ^)
	{
		//
		// Silently consume any exceptions without allowing them to propagate
		// upwards, so that the original failure is not obscured.
		//
	}
}


INWScriptStack::NeutralString
NWScriptProgram::ConvertStringToNeutral(
	__in String ^ Str
	)
/*++

Routine Description:

	This routine converts a System::String to a INWScriptStack::NeutralString.

Arguments:

	Str - Supplies the string to convert.

Return Value:

	The converted std::string is returned.  On failure, a System::Exception, may
	be raised.

Environment:

	User mode, C++/CLI.

--*/
{
	INWScriptStack::NeutralString NeutralStr;

	NeutralStr.first  = NULL;
	NeutralStr.second = 0;

	if (Str->Length == 0)
		return NeutralStr;

	//
	// If we're a JIT'd script, don't map as UTF-8.  Instead, just truncate the
	// bits down to 8 bit (which matches the behavior of the truncate to 8-bit
	// encoder we use on the input side).
	//
	// Only managed scripts see characters mapped to their 'true' Unicode
	// values rather than being tunneled as a raw byte stream.  This allows for
	// compatibility with (broken) scripts that pull apart strings in the
	// middle of a UTF-8 code point and expect to be able to re-assemble them,
	// while allowing managed scripts to treat characters natively.
	//

	if (m_ManagedScript == false)
	{
		int                         Size       = Str->Length;
		cli::pin_ptr<const wchar_t> Pinned_ptr = PtrToStringChars( Str );

		try
		{
			NeutralStr.first = (char *) NWScriptStack::AllocNeutral( (size_t) Size + 1 );
		}
		catch (std::exception)
		{
			throw gcnew Exception( "Out of memory." );
		}

		NeutralStr.second = (size_t) Size;

		for (int i = 0; i < Size; i += 1)
		{
			NeutralStr.first[ i ] = (char) Pinned_ptr[ i ];
		}

		NeutralStr.first[ Size ] = '\0';

		return NeutralStr;
	}

	try
	{
		size_t                      Written;
		size_t                      Size       = 0;
		cli::pin_ptr<const wchar_t> Pinned_ptr = PtrToStringChars( Str );
		
		Size = ::WideCharToMultiByte(
			CP_UTF8,
			0,
			Pinned_ptr,
			Str->Length,
			NULL,
			0,
			NULL,
			NULL);

		if (Size == 0)
			throw gcnew System::ArgumentException( "ConvertStringToNeutral: WideCharToMultiByte (1) failed." );

		NeutralStr.second = Size;

		Size += 1; // Null terminator

		try
		{
			NeutralStr.first = (char *) NWScriptStack::AllocNeutral( Size );
		}
		catch (std::exception)
		{
			throw gcnew Exception( "Out of memory." );
		}

		Written = ::WideCharToMultiByte(
			CP_UTF8,
			0,
			Pinned_ptr,
			Str->Length,
			NeutralStr.first,
			static_cast< int >( Size ),
			NULL,
			NULL);

		if ((Written >= Size) || (Written == 0))
			throw gcnew System::ArgumentException( "ConvertStringToNeutral: WideCharToMultiByte (2) failed." );
	}
	catch (Exception ^)
	{
		if (NeutralStr.first != NULL)
			NWScriptStack::FreeNeutral( NeutralStr.first );

		throw;
	}

	return NeutralStr;
}

void
NWScriptProgram::InstantiateManagedScript(
	__in array< Byte > ^ ManagedAssembly,
	__in NWScriptManagedSupport ^ ManagedSupport
	)
/*++

Routine Description:

	This routine initializes an assembly for a managed script given its
	pre-generated constituent PE image as a byte array.

Arguments:

	ManagedAssembly - Supplies a byte array describing the managed script
	                  image to instantiate.

	ManagedSupport - Supplies the managed support object containing the
	                 interface layer assembly to resolve to the managed script
	                 assembly.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	AppDomain       ^ CurrentDomain;
	Assembly        ^ ScriptAssembly;
	Exception       ^ Ex;
	Type            ^ ScriptType;
	FieldInfo       ^ ParametersField;
	array< Type ^ > ^ ParameterTypes;

	CurrentDomain = AppDomain::CurrentDomain;

	m_ManagedSupport = ManagedSupport;

	//
	// With the assembly resolve event lock held, temporarily hook the resolve
	// assembly handler and resolve the missing managed interface assembly
	// against the managed support object.
	//

	{
		swutil::ScopedLock AssemblyResolveLock( g_AssemblyResolveEventLock );

		CurrentDomain->AssemblyResolve += gcnew ResolveEventHandler(
			this,
			&NWScriptProgram::InstantiateManagedScriptResolveAssembly);

		try
		{
			ScriptAssembly = CurrentDomain->Load( ManagedAssembly );

			//
			// Now locate the script type by looking for the first type that
			// derives from the IScriptProgram interface.
			//

			for each (Type ^ T in ScriptAssembly->GetTypes( ))
			{
				if (!T->IsVisible)
					continue;
				if (T->GetInterface( "IGeneratedScriptProgram" ) == nullptr)
					continue;

				ScriptType = T;
				break;
			}

			if (ScriptType == nullptr)
				throw gcnew ApplicationException( "Module does not implement IGeneratedScriptProgram" );

			//
			// Instantiate a copy of the script program object.
			//

			m_EngineStructureTypes = ManagedSupport->GetEngineStructureTypes( );

			m_ProgramObject = (IGeneratedScriptProgram ^) ScriptAssembly->CreateInstance(
				ScriptType->FullName,
				false,
				BindingFlags::CreateInstance,
				nullptr,
				gcnew array< Object ^ >{ m_JITIntrinsics, this },
				nullptr,
				nullptr);
		}
		catch (Exception ^ e)
		{
			Ex = e;
		}

		CurrentDomain->AssemblyResolve -= gcnew ResolveEventHandler(
			this,
			&NWScriptProgram::InstantiateManagedScriptResolveAssembly);

		m_ManagedSupport = nullptr;

		if (Ex != nullptr)
			throw Ex;
	}

	//
	// If the script program type defines parameter types for type checking on
	// the entry point symbol, cache the required types.
	//

	ParametersField = ScriptType->GetField( "ScriptParameterTypes" );
	
	if (ParametersField != nullptr)
	{
		ParameterTypes = (array< Type ^ > ^) ParametersField->GetValue(
			m_ProgramObject);

		m_EntryPointParamTypes = gcnew ActionTypeArr( ParameterTypes->Length );

		for (int i = 0; i < ParameterTypes->Length; i += 1)
		{
			m_EntryPointParamTypes[ i ] = GetNWScriptType(
				ParameterTypes[ i ]);
		}
	}
}

void
NWScriptProgram::SaveManagedInterfaceDll(
	__in const NWScriptAnalyzer * Analyzer,
	__in NWN::OBJECTID ObjectInvalid,
	__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams
	)
/*++

Routine Description:

	This routine saves the MSIL representation of a canonically-named managed
	interface DLL (to re-generate the NWScript SDK).

Arguments:

	Analyzer - Supplies the analysis context that describes the NWScript IR to
	           generate code for.

	ObjectInvalid - Supplies the object id to reference for the 'object
	                invalid' manifest constant.

	CodeGenParams - Optionally supplies extension code generation parameters.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	NWScriptCodeGenerator              ^ Generator;
	NWScriptCodeGenerator::ProgramInfo   Program;
	static bool                          GeneratedIt = false;

	if (!ARGUMENT_PRESENT( CodeGenParams ))
		return;

	if (!(CodeGenParams->CodeGenFlags & NWCGF_SAVE_OUTPUT))
		return;

	if (GeneratedIt)
		return;

	Generator = gcnew NWScriptCodeGenerator(
		m_TextOut,
		m_DebugLevel,
		INWScriptProgram::typeid,
		IGeneratedScriptProgram::typeid);

	Generator->GenerateInterfaceLayerCode(
		Analyzer,
		m_ActionHandler,
		ObjectInvalid,
		CodeGenParams,
		"NWScriptManagedInterface",
		NWScriptUtilities::NWUTF8Encoding,
		Program);

	GeneratedIt = true;
}

Assembly ^
NWScriptProgram::InstantiateManagedScriptResolveAssembly(
	__in Object ^ Sender,
	__in ResolveEventArgs ^ Args
	)
/*++

Routine Description:

	This routine is invoked in the context of InstantiateManagedScript when
	assembly name resolution fails to resolve the "NWScriptManagedInterface"
	virtual assembly.  Its purpose is to link the requestor to the canonical
	managed interface assembly bound to the current INWScriptActions object.

Arguments:

	Sender - Supplies the source object.

	Args - Supplies the event arguments.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	UNREFERENCED_PARAMETER( Sender );

	//
	// If this is not a request for the managed interface virtual assembly,
	// pass on the request.
	//

	if (Args->Name != "NWScriptManagedInterface, Version=0.0.0.0, Culture=neutral, PublicKeyToken=null")
		return nullptr;

	//
	// Otherwise, link the caller to the canonical interface assembly for the
	// INWScriptActions context.
	//

	return m_ManagedSupport->GetAssembly( );
}

void
NWScriptProgram::DiscoverEntryPointParameters(
	__in const NWScriptAnalyzer * Analyzer
	)
/*++

Routine Description:

	This routine records the types of each parameter to the entry point symbol.

	Its purpose is to gather the data necessary to perform on-the-fly
	conversion of parameterized script arguments.

Arguments:

	Analyzer - Supplies the analysis context that describes the NWScript IR to
	           generate code for.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	NWScriptSubroutine * IREntry;

	IREntry = Analyzer->GetSubroutines( ).front( ).get( );

	if (IREntry->GetParameters( ).empty( ))
	{
		m_EntryPointParamTypes = nullptr;
		return;
	}

	m_EntryPointParamTypes = gcnew ActionTypeArr(
		(int) IREntry->GetParameters( ).size( ) );

	for (int i = 0; i < (int) IREntry->GetParameters( ).size( ); i += 1)
		m_EntryPointParamTypes[ i ] = IREntry->GetParameterVariable( i ).GetType( );
}



