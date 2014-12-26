/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	gui_test_timer.nss

Abstract:

	This module houses test code for timer management and related routines.

--*/

void
main(
	float Delay
	)
/*++

Routine Description:

	This script is invoked to test timer management.

Arguments:

	Delay - Supplies the delay to enqueue for a timer.

Return Value:

	None.

Environment:

	GUI script.

--*/
{
	ActionWait( Delay );
	SpeakString( "Spoken before delay elapses." );
	ActionSpeakString( "Spoken after delay elapses." );
	SendMessageToPC( OBJECT_SELF, "Timers set." );
}

