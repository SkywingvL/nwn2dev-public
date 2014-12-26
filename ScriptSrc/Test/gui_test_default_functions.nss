/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	gui_default_functions.nss

Abstract:

	This module houses test code for the default functions feature.

--*/

int
testfunc1(
	int a,
	int b,
	int c = 0
	);

struct teststruct1
{
	int a;
	string b;
	talent t;
};

struct teststruct2
{
	float x;
	vector y;
	struct teststruct1 ts;
};

struct teststruct2
testfunc2(
	struct teststruct1 t1,
	struct teststruct2 t2
	);

#pragma default_function(testfunc1)
#pragma default_function(testfunc2)

void
main(
	float f1,
	float f2
	)
/*++

Routine Description:

	This script is invoked to test default functions.

Arguments:

	f1 - Supplies the first parameter.

	f2 - Supplies the second parameter.

Return Value:

	None.

Environment:

	GUI script.

--*/
{
	struct teststruct1 t1;
	struct teststruct2 t2;

	t2.x = f1;
	t2.y.x = f2;

	PrintInteger( testfunc1( d2( ), d2( ) ) );
	PrintInteger( testfunc2( t1, t2 ).ts.a );
}

