/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	gui_test_structfwddecl.nss

Abstract:

	This module houses test code for forward declaration references of
	struct types in subroutine prototypes.

--*/

struct TestStruct
testfunc1(
	);

void
testfunc2(
	struct TestStruct P0
	)
/*++

Routine Description:

	This routine tests usage of structures that have not yet been declared.

Arguments:

	P0 - Supplies a dummy argument.

Return Value:

	None.

Environment:

	GUI script.

--*/
{
	struct TestStruct L0;

	L0.i = 0;

	PrintInteger( L0.i );
}

//int Func2;
int Func2()
{
	return 0;
}

//int Struct1;
struct Struct1
{
	int Var;
};

//int OtherStruct;
//int OtherStruct();
//int Func1;
//int Func1();
int Func1;
//int Func1;
/*int Func1()
{
}*/
/*
struct OtherStruct
{
	int x;
};
*/

struct TestStruct
{
	int i;
	float f;
//	struct OtherStruct os;
};

struct OtherStruct
{
	int i;
	float f;
};

void
main(
	)
/*++

Routine Description:

	This script is invoked to test structure forward references.

Arguments:

	None.

Return Value:

	None.

Environment:

	GUI script.

--*/
{
	struct TestStruct T1;
//	int T1;

	testfunc2( T1 );
	PrintString( "Done!" );
}
