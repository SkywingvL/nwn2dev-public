/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	gui_test_globals.nss

Abstract:

	This module houses test code for pathological global variable use cases that
	emit unusable code.

--*/

float g_floatglobal = 0.0f;

void
testfunc2(
	)
{
	PrintFloat(g_floatglobal);
}

void
testfunc1(
	float f
	)
{
//	PrintFloat( g_floatglobal );
	PrintFloat( f );
	testfunc2();
}

effect g_effect = EffectDispelMagicAll( 0, testfunc1( g_floatglobal ) );
int g_global = 41;
location g_global2 = (g_global2 == g_global2) ? g_global2 : g_global2;

void
main(
	)
/*++

Routine Description:

	This script is invoked to test global variable cases.

Arguments:

	None.

Return Value:

	None.

Environment:

	GUI script.

--*/
{
	PrintInteger( g_global );
	PrintFloat( g_floatglobal );
	SpeakString( "Done." );
	g_effect = g_effect;
	g_global2 = g_global2;
}

