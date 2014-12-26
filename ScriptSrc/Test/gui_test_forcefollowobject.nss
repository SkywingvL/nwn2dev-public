/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	gui_test_forcefollowobject.nss

Abstract:

	This module houses test code for ActionForceFollowObject and the other
	force move actions.

--*/

void
main(
	object ObjectToFollow,
	float Distance
	)
/*++

Routine Description:

	This script is invoked to test ActionForceFollowObject.  It enqueues the
	force follow object action to the current action object, i.e. the players
	control object.

Arguments:

	ObjectToFollow - Supplies the object that is to be followed.

	Distance - Supplies the minimum distance to attempt to keep between the two
	           objects.

Return Value:

	None.

Environment:

	GUI script.

--*/
{
	ActionForceFollowObject( ObjectToFollow, Distance );
	SendMessageToPC( OBJECT_SELF, "Following " + GetName( ObjectToFollow ) );

	ActionForceMoveToObject( ObjectToFollow, FALSE, Distance, 15.0f );
	ActionRandomWalk( );
}

