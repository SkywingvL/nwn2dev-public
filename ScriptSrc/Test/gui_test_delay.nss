/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	gui_test_delay.nss

Abstract:

	This module houses test code for DelayCommand continuations.

--*/

int g_global = 41;
location g_global2;

const float CONTINUATION_DELAY = 10.0f;

void
DelayContinuationProc(
	int local,
	int iteration
	)
/*++

Routine Description:

	This routine represents the DelayCommand test continuation context.  Its
	purpose is to ensure that delay state is saved and restored correctly.

Arguments:

	local - Supplies a persisted local variable value.

	iteration - Supplies the current iteration number.

Return Value:

	None.

Environment:

	DelayCommand continuation.

--*/
{
	if (g_global != 42)
		SendMessageToPC( GetFirstPC( ), "g_global not saved properly!" );

	if (local != 4242)
		SendMessageToPC( GetFirstPC( ), "local not saved properly!" );

	SendMessageToPC(
		GetFirstPC( ),
		"Finished continuation " + IntToString( iteration ) );

	g_global2 = g_global2;

	ActionJumpToLocation( g_global2 );

	DelayCommand(
		CONTINUATION_DELAY,
		DelayContinuationProc( local, ++iteration ) );
}

void
main(
	)
/*++

Routine Description:

	This script is invoked to test DelayCommand continuation.

Arguments:

	None.

Return Value:

	None.

Environment:

	GUI script.

--*/
{
	object Self;

	ActionWait( CONTINUATION_DELAY );
	SpeakString( "Spoken before delay elapses." );
	ActionSpeakString( "Spoken after delay elapses." );
	g_global = 42;
	DelayCommand(
		CONTINUATION_DELAY,
		DelayContinuationProc( 4242, 0 ) );
	SendMessageToPC( OBJECT_SELF, "Timers set." );

	Self = OBJECT_SELF;

	AssignCommand(
		GetModule( ),
		SendMessageToPC( Self, ObjectToString( OBJECT_SELF ) + " " + ObjectToString( Self ) )
		);
}

