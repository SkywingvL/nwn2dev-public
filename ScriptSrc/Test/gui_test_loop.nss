/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	gui_test_loop.nss

Abstract:

	This module houses test code for loops.

--*/

void
LoopMain(
	int Iterations
	)
/*++

Routine Description:

	This script is invoked to test loop constructs.

Arguments:

	Iterations - Supplies the number of loop iterations to run.

Return Value:

	None.

Environment:

	Continuation script situation from ActionDoCommand.

--*/
{
	string ReturnString;
	location ReturnLocation;
	int PrevIterations = Iterations;

	while (Iterations--)
	{
		string Tag = GetTag( OBJECT_SELF );
		object Area;

		if (Tag == "tag_test")
			SendMessageToPC( OBJECT_SELF, Tag );
		else
			Area = GetAreaFromLocation( GetLocation( OBJECT_SELF ) );

		if (Area == GetArea( OBJECT_SELF ))
			ReturnString = GetName( Area );

		ReturnLocation = GetLocation( OBJECT_SELF );
	}

	SendMessageToPC( OBJECT_SELF, ReturnString );
	SendMessageToPC(
		OBJECT_SELF,
		GetName( GetAreaFromLocation( ReturnLocation ) ) );

	ActionWait( 0.001f );
	ActionDoCommand( LoopMain( PrevIterations ) );
}

void
main(
	int Iterations
	)
/*++

Routine Description:

	This script is invoked to test loop constructs.

Arguments:

	Iterations - Supplies the number of loop iterations to run.

Return Value:

	None.

Environment:

	GUI script.

--*/
{
	AssignCommand( OBJECT_SELF, ActionDoCommand( LoopMain( Iterations ) ) );
}
