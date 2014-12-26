/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptActionActions.cpp

Abstract:

	This module houses the implementation of action service handlers that deal
	with actions (action queue manipulation, ExecuteScript, DelayCommand, etc).

--*/

#include "Precomp.h"
#define NWSCRIPTHOST_INTERNAL
#include "NWScriptHost.h"


SCRIPT_ACTION( AssignCommand )
/*++

Routine Description:

	This script action queues an immediate script situation to the script
	situation queue.

Arguments:

	oActionSubject - Supplies the object context to run the script situation
	                 under.

	aActionToAssign - Supplies the script situation to assign.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	NWN::OBJECTID   ObjectId;

	ObjectId = StackPopObjectId( VMStack );

	CreateDeferredScriptSituation(
		ScriptVM,
		ObjectId,
		0);
}

SCRIPT_ACTION( DelayCommand )
/*++

Routine Description:

	This script action queues a delayed script situation to the script
	situation queue.  The action context is set to the current object.

Arguments:

	fSeconds - Supplies a delay time expressed in (potentially fractions) of a
	           second.

	aActionToAssign - Supplies the script situation to assign.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	NWN::OBJECTID   ObjectId;
	float           fSeconds;
	ULONG           DueTime;

	fSeconds = VMStack.StackPopFloat( );

	DueTime  = (ULONG) (fSeconds * 1000.0f);
	ObjectId = ScriptVM.GetCurrentActionObject( );

	CreateDeferredScriptSituation(
		ScriptVM,
		ObjectId,
		DueTime);
}

SCRIPT_ACTION( ExecuteScript )
/*++

Routine Description:

	This script action causes a recursive call to a script in the context of
	another object.

Arguments:

	sScript - Supplies the resource name of the script to execute.

	oTarget - Supplies the object id of the target.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	std::string     sScript;
	NWN::OBJECTID   oTarget;

	sScript = VMStack.StackPopString( );
	oTarget = StackPopObjectId( VMStack );

	(void) RunScript( sScript.c_str( ), oTarget );
}
