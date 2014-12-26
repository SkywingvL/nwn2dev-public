/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptJITIntrinsics.cpp

Abstract:

	This module houses the JIT intrinsics that are made available to the
	JIT'd NWScript program.  It is intended for use in conjuction with the
	NWNScriptJIT module.

	The JIT intrinsics defined herein are intended only for consumption by the
	JIT'd code itself.  They are compiled as pure CLR.

--*/

#include "Precomp.h"
#include "NWScriptJITIntrinsics.h"

using namespace NWScript;
using namespace System::Reflection;

NWScriptJITIntrinsics::NWScriptJITIntrinsics(
	__in INWScriptProgram ^ Program
	)
/*++

Routine Description:

	This routine constructs a new NWScriptJITIntrinsics.  It supports
	interop functionality between the JIT'd code and the native code of the
	underlying script host.

Arguments:

	Program - Supplies the overarching INWScriptProgram interface associated
	          with this JIT intrinsics instance.

Return Value:

	None.  Raises a System::Exception on failure.

Environment:

	User mode, C++/CLI.

--*/
: m_Program( Program )
{
}

NWScriptJITIntrinsics::~NWScriptJITIntrinsics(
	)
/*++

Routine Description:

	This routine deletes the current NWScriptJITIntrinsics object and its
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

void
NWScriptJITIntrinsics::Intrinsic_VMStackPushInt(
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
	m_Program->Intrinsic_VMStackPushInt( i );
}

Int32
NWScriptJITIntrinsics::Intrinsic_VMStackPopInt(
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
	return m_Program->Intrinsic_VMStackPopInt( );
}

void
NWScriptJITIntrinsics::Intrinsic_VMStackPushFloat(
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
	m_Program->Intrinsic_VMStackPushFloat( f );
}

Single
NWScriptJITIntrinsics::Intrinsic_VMStackPopFloat(
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
	return m_Program->Intrinsic_VMStackPopFloat( );
}

void
NWScriptJITIntrinsics::Intrinsic_VMStackPushString(
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
	m_Program->Intrinsic_VMStackPushString( s );
}

String ^
NWScriptJITIntrinsics::Intrinsic_VMStackPopString(
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
	return m_Program->Intrinsic_VMStackPopString( );
}

void
NWScriptJITIntrinsics::Intrinsic_VMStackPushObjectId(
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
	m_Program->Intrinsic_VMStackPushObjectId( o );
}

UInt32
NWScriptJITIntrinsics::Intrinsic_VMStackPopObjectId(
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
	return m_Program->Intrinsic_VMStackPopObjectId( );
}

void
NWScriptJITIntrinsics::Intrinsic_ExecuteActionService(
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
	m_Program->Intrinsic_ExecuteActionService( ActionId, NumArguments );
}

void
NWScriptJITIntrinsics::Intrinsic_StoreState(
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
	m_Program->Intrinsic_StoreState(
		Globals,
		Locals,
		ResumeMethodPC,
		ResumeMethodId,
		ProgramObject);
}

Object ^
NWScriptJITIntrinsics::Intrinsic_ExecuteActionServiceFast(
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
	return m_Program->Intrinsic_ExecuteActionServiceFast(
		ActionId,
		NumArguments,
		Arguments);
}

NWScript::NeutralStringStorage
NWScriptJITIntrinsics::Intrinsic_AllocateNeutralString(
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
	return m_Program->Intrinsic_AllocateNeutralString( Str );
}

void
NWScriptJITIntrinsics::Intrinsic_DeleteNeutralString(
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
	m_Program->Intrinsic_DeleteNeutralString( Str );
}

String ^
NWScriptJITIntrinsics::Intrinsic_NeutralStringToString(
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
	return m_Program->Intrinsic_NeutralStringToString( Str );
}

void
NWScriptJITIntrinsics::Intrinsic_CheckScriptAbort(
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
	return m_Program->Intrinsic_CheckScriptAbort( ActionSucceeded );
}
