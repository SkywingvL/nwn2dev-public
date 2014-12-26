/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptSavedState.h

Abstract:

	This module defines the representation of a saved resume state for an MSIL
	compiled NWScript program.  The resume state includes a reference to the
	underlying program text, plus all a copy of all global and local variables
	used by the resume context, plus a subroutine reference to the resume
	point.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTJIT_NWSCRIPTSAVEDSTATE_H
#define _SOURCE_PROGRAMS_NWNSCRIPTJIT_NWSCRIPTSAVEDSTATE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace NWScript
{

ref class NWScriptProgram;

ref class NWScriptSavedState
{

public:

	//
	// Construct a new saved state.
	//

	NWScriptSavedState(
		__in NWScriptProgram ^ Program,
		__in IGeneratedScriptProgram ^ State,
		__in array< Object ^ > ^ Locals,
		__in UInt32 ResumeSubroutineId,
		__in INWScriptStack * Stack,
		__in NWN::OBJECTID CurrentActionObjectSelf,
		__in NWNScriptLib::PROGRAM_COUNTER ResumeSubroutinePC,
		__in_opt array< Object ^ > ^ Globals
		);

	//
	// Destruct a NWScriptSavedState instance.
	//

	inline
	~NWScriptSavedState(
		)
	{
		this->!NWScriptSavedState( );
	}

	!NWScriptSavedState(
		);

	//
	// Return the associated script program instance.
	//

	inline
	NWScriptProgram ^
	GetProgram(
		)
	{
		return m_Program;
	}

	//
	// Return the associated program state object.
	//

	inline
	IGeneratedScriptProgram ^
	GetProgramState(
		)
	{
		return m_ProgramObject;
	}

	//
	// Return the associated resume subroutine identifier, used for the fast
	// resume mechanism.
	//

	inline
	UInt32
	GetResumeMethodId(
		)
	{
		return m_ResumeMethodId;
	}

	//
	// Return the list of locals.
	//

	inline
	array< Object ^ > ^
	GetLocals(
		)
	{
		return m_ResumeLocals;
	}

	//
	// Return the stack to use on action service handler invocations.
	//

	inline
	INWScriptStack *
	GetStack(
		)
	{
		return m_Stack;
	}

	//
	// Return the current self object for OBJECT_SELF references.
	//

	inline
	NWN::OBJECTID
	GetCurrentActionObjectSelf(
		)
	{
		return m_CurrentActionObjectSelf;
	}

	//
	// Return the NWScript resume PC for the resume subroutine.
	//

	inline
	NWNScriptLib::PROGRAM_COUNTER
	GetResumeMethodPC(
		)
	{
		return m_ResumeMethodPC;
	}

	//
	// Return the list of globals.
	//

	inline
	array< Object ^ > ^
	GetGlobals(
		)
	{
		return m_ResumeGlobals;
	}

private:

	typedef System::Reflection::MethodInfo MethodInfo;

	typedef NWNScriptLib::PROGRAM_COUNTER PROGRAM_COUNTER;

	//
	// Define the associated program.
	//

	NWScriptProgram          ^ m_Program;

	//
	// Define the program instance object to restore.
	//

	IGeneratedScriptProgram  ^ m_ProgramObject;

	//
	// Define the method id of the method to invoke on resume.
	//

	UInt32                     m_ResumeMethodId;

	//
	// Define the array of parameters to pass to the resume method (locals).
	//

	array< Object ^ >        ^ m_ResumeLocals;

	//
	// Define the stack to use on action service handler invocation.
	//

	INWScriptStack           * m_Stack;

	//
	// Define the current self object for OBJECT_SELF references.
	//

	NWN::OBJECTID              m_CurrentActionObjectSelf;

	//
	// Define the NWScript program counter of the resume method ID.
	//

	PROGRAM_COUNTER            m_ResumeMethodPC;

	//
	// Define the array of globals tracked for usage by the resume method.
	//
	// N.B.  Currently, all globals are saved.  Tracking is only used to
	//       support saving the saved state to an INWScriptStack, which is an
	//       optional code generation time feature.
	//

	array< Object ^ > ^        m_ResumeGlobals;

};

}

#endif

