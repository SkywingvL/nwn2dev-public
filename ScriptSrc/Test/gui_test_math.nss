/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	gui_test_math.nss

Abstract:

	This module houses test code for math operations.

--*/

void
main(
	float f1,
	float f2
	)
/*++

Routine Description:

	This script is invoked to test math operations.

Arguments:

	f1 - Supplies the first parameter.

	f2 - Supplies the second parameter.

Return Value:

	None.

Environment:

	GUI script.

--*/
{
	int i = 350;
	float f = 350.0f;
	PrintFloat( (i * 10) / 100.0f );
	PrintFloat( 100.0f / (i * 10) );
	PrintInteger( (i * 10) / 100 );
	PrintFloat( 100.0f * i );
	PrintFloat( i * 100.0f );
	PrintFloat( 100.0f + i );
	PrintFloat( i + 100.0f );
	PrintFloat( 100.0f - i );
	PrintFloat( i - 100.0f );
}
