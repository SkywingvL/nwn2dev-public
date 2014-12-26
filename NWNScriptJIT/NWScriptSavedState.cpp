/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptSavedState.cpp

Abstract:

	This module houses the representation of a saved resume state for an MSIL
	compiled NWScript program.  The resume state includes a reference to the
	underlying program text, plus all a copy of all global and local variables
	used by the resume context, plus a subroutine reference to the resume
	point.

--*/

#include "Precomp.h"
#include "NWScriptSavedState.h"

NWScriptSavedState::NWScriptSavedState(
	__in NWScriptProgram ^ Program,
	__in IGeneratedScriptProgram ^ State,
	__in array< Object ^ > ^ Locals,
	__in UInt32 ResumeSubroutineId,
	__in INWScriptStack * Stack,
	__in NWN::OBJECTID CurrentActionObjectSelf,
	__in NWNScriptLib::PROGRAM_COUNTER ResumeSubroutinePC,
	__in_opt array< Object ^ > ^ Globals
	)
/*++

Routine Description:

	This routine constructs a new NWScriptSavedState, which contains state
	that is used to 'resume' execution at a continuation method generated to
	support a STORE_STATE operation.

Arguments:

	Program - Supplies the associated script program.

	State - Supplies the program this object.

	Locals - Supplies the list of locals to restore.

	ResumeSubroutineId - Supplies the subroutine id of the subroutine to invoke
	                     to resume the state (for a fast resume).

	Stack - Supplies the VM stack to use for action service handler invocation.

	CurrentActionObjectSelf - Supplies the self object for OBJECT_SELF
	                          references.

	ResumeSubroutinePC - Supplies the NWScript program counter of the
	                     resume subroutine.

	Globals - Optionally supplies the list of globals to track and restore for
	          INWScriptStack serialization.

Return Value:

	None.  Raises a System::Exception on failure.

Environment:

	User mode, C++/CLI.

--*/
: m_Program( Program ),
  m_ProgramObject( State ),
  m_ResumeLocals( Locals ),
  m_ResumeMethodId( ResumeSubroutineId ),
  m_Stack( Stack ),
  m_CurrentActionObjectSelf( CurrentActionObjectSelf ),
  m_ResumeMethodPC( ResumeSubroutinePC ),
  m_ResumeGlobals( Globals )
{
}

NWScriptSavedState::!NWScriptSavedState(
	)
/*++

Routine Description:

	This routine deletes the current NWScriptSavedState object and its
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
