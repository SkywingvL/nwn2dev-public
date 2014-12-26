/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	gui_test_intrinsics.nss

Abstract:

	This module houses test code for compiler intrinsics.

--*/

//
// Ensure that (harmless) duplicate definitions of intrinsics are accepted, so
// that they may be used with non-extension code.
//

int __readrelativesp() { return 0; }
int __readbp() { return 0; }
void __writebp(int bp) {}

void
testfunc1(
	)
/*++

Routine Description:

	This routine tests usage of __readsp as variables are declared.

Arguments:

	None.

Return Value:

	None.

Environment:

	GUI script.

--*/
{
	__readsp();
	SendMessageToPC (OBJECT_SELF, "sp = " + IntToString(__readsp()));
	int x;
	SendMessageToPC (OBJECT_SELF, "sp = " + IntToString(__readsp()));
	int y;
	SendMessageToPC (OBJECT_SELF, "sp = " + IntToString(__readsp()));
	int z;
	SendMessageToPC (OBJECT_SELF, "sp = " + IntToString(__readsp()));
	int w;
	SendMessageToPC (OBJECT_SELF, "sp = " + IntToString(__readsp()));
	w = z + y + x;
	x = w;
}

void
testfunc2(
	)
/*++

Routine Description:

	This routine tests usage of __readbp as a standalone function.

Arguments:

	None.

Return Value:

	None.

Environment:

	GUI script.

--*/
{
	__readbp();
}

void
testfunc3(
	)
/*++

Routine Description:

	This routine tests usage of __readbp as a standalone function.

Arguments:

	None.

Return Value:

	None.

Environment:

	GUI script.

--*/
{
	__readsp();
}

int
testfunc4(
	)
/*++

Routine Description:

	This routine tests usage of __readsp as a return value.

Arguments:

	None.

Return Value:

	The ambient SP value within the function main body is returned.

Environment:
	GUI script.

--*/
{
	return __readsp();
}

int g_x;
int g_y;
int g_z;

int g_global;

void
main(
	int i
	)
/*++

Routine Description:

	This script is invoked to test compiler intrinsics.

Arguments:

	i - Supplies an integer to modify stack pointer references by.

Return Value:

	None.

Environment:

	GUI script.

--*/
{
	int val_1;
	int val_1_ptr = __readsp();
	int val_2;
	int val_2_ptr = __readsp();
	int val_3;
	int val_3_ptr = __readsp();
	int oldbp;

	val_1 = val_2 = val_3;
	val_3 = val_1;

	oldbp = __readbp();
	__writebp(val_1_ptr-1);
	PrintInteger(oldbp);
	PrintInteger(val_1_ptr);
	PrintInteger(__readbp());
	g_global = 42;
	__writebp(oldbp);
	PrintInteger(val_1);

	int pc = __readpc();
	SendMessageToPC (OBJECT_SELF, "curPC = " + IntToString(pc));
	testfunc1();
	testfunc3();
	int curSP;

	SendMessageToPC (OBJECT_SELF, "curSP = " + IntToString(curSP = __readsp()));
	if (curSP != __readsp())
		SendMessageToPC (OBJECT_SELF, "readSP mismatch!");
	SendMessageToPC (OBJECT_SELF, "The integer was " + IntToString(i));
	__readrelativesp();
	int tos1 = __readrelativesp();
	int tos2 = __readrelativesp();
	int tos3 = __readrelativesp();
	int tos4 = __readrelativesp();
	__readrelativesp();

	SendMessageToPC (OBJECT_SELF, "topsp = " + IntToString(__readrelativesp()) + ", bp = " + IntToString(__readbp()));

	SendMessageToPC (OBJECT_SELF, "ok!");

	__writebp(__readbp()+4);
	SendMessageToPC (OBJECT_SELF, "newBP = " + IntToString(__readbp()));
	__writebp(__readbp()-4);
	SendMessageToPC (OBJECT_SELF, "oldBP = " + IntToString(__readbp()));

	PrintInteger(i);
	PrintInteger(__readbp() + i);
	PrintInteger(i);
	oldbp = __readbp();
	__writebp(i);
	PrintInteger(__readbp());
	PrintInteger(oldbp);
	__writebp(oldbp);
	PrintInteger(__readbp());
}

