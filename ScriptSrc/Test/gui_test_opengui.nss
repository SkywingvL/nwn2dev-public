/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	gui_test_opengui.nss

Abstract:

	This module houses test code for opening a GUI panel.

--*/

int global;

int func()
{
	int i = global++;

	return i;
}

int func2()
{
	int i = --global;

	return i;
}

void
main(
	string ScreenName,
	string XmlFileName
	)
/*++

Routine Description:

	This script is invoked to open a client-side GUI screen.

Arguments:

	ScreenName - Supplies the GUI screen to open.

	XmlFileName - Supplies the XML GUI descriptor file that contains the
	              description for the given GUI screen.

Return Value:

	None.

Environment:

	GUI script.

--*/
{
	SendMessageToPC( OBJECT_SELF, "Opening " + ScreenName + " (" + XmlFileName + ")" );
	DisplayGuiScreen( OBJECT_SELF, ScreenName, FALSE, XmlFileName );
	func();
	func2();
}

