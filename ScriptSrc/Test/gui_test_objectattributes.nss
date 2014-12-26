/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	gui_test_objectattributes.nss

Abstract:

	This module houses test code for the NWNX4 ObjectAttributes API.

--*/

#include "nwnx_objectattributes_include"

void
main(
	string Command,
	string Parameter
	)
/*++

Routine Description:

	This script is invoked to test NWNX4 Object Attributes manipulation.

Arguments:

	Command - Supplies the command to run.

	Parameter - Supplies a parameter for the command.

Return Value:

	None.

Environment:

	GUI script.

--*/
{
	SpeakString( "Executing command " + Command + ", Parameter " + Parameter );

	if (Command == "SetHeadVariation")
	{
		SpeakString( "Setting head variation..." );
		XPObjectAttributesSetHeadVariation(
			OBJECT_SELF,
			StringToInt( Parameter ));
	}
	else if (Command == "SetHairVariation")
	{
		SpeakString( "Setting hair variation..." );
		XPObjectAttributesSetHairVariation(
			OBJECT_SELF,
			StringToInt( Parameter ));
	}
	else if (Command == "SetWingVariation")
	{
		SpeakString( "Setting wing variation..." );
		XPObjectAttributesSetWingVariation(
			OBJECT_SELF,
			StringToInt( Parameter ));
	}
	else if (Command == "SetTailVariation")
	{
		SpeakString( "Setting tail variation..." );
		XPObjectAttributesSetTailVariation(
			OBJECT_SELF,
			StringToInt( Parameter ));
	}
	else if (Command == "SetFacialHairVariation")
	{
		SpeakString( "Setting facial hair variation..." );
		XPObjectAttributesSetFacialHairVariation(
			OBJECT_SELF,
			StringToInt( Parameter ));
	}
	else if (Command == "SetBodyTint")
	{
		float Color;

		Color = StringToFloat( Parameter );

		SpeakString( "Setting body tint..." );
		XPObjectAttributesSetBodyTint(
			OBJECT_SELF,
			CreateXPObjectAttributes_TintSet(
				CreateXPObjectAttributes_Color( Color, Color, Color, 1.0f ),
				CreateXPObjectAttributes_Color( Color, Color, Color, 1.0f ),
				CreateXPObjectAttributes_Color( Color, Color, Color, 1.0f )
				)
			);
	}
	else if (Command == "SetHairTint")
	{
		float Color;

		Color = StringToFloat( Parameter );

		SpeakString( "Setting hair tint..." );
		XPObjectAttributesSetHairTint(
			OBJECT_SELF,
			CreateXPObjectAttributes_TintSet(
				CreateXPObjectAttributes_Color( Color, Color, Color, 1.0f ),
				CreateXPObjectAttributes_Color( Color, Color, Color, 1.0f ),
				CreateXPObjectAttributes_Color( Color, Color, Color, 1.0f )
				)
			);
	}
	else if (Command == "SetHeadTint")
	{
		float Color;

		Color = StringToFloat( Parameter );

		SpeakString( "Setting head tint..." );
		XPObjectAttributesSetHeadTint(
			OBJECT_SELF,
			CreateXPObjectAttributes_TintSet(
				CreateXPObjectAttributes_Color( Color, Color, Color, 1.0f ),
				CreateXPObjectAttributes_Color( Color, Color, Color, 1.0f ),
				CreateXPObjectAttributes_Color( Color, Color, Color, 1.0f )
				)
			);
	}
	else if (Command == "SetRace")
	{
		int Race;

		Race = StringToInt( Parameter );
		SpeakString( "Setting race..." );
		XPObjectAttributesSetRace(
			OBJECT_SELF,
			Race);
	}
	else if (Command == "SetSubRace")
	{
		int Race;

		Race = StringToInt( Parameter );
		SpeakString( "Setting subrace..." );
		XPObjectAttributesSetSubRace(
			OBJECT_SELF,
			Race);
	}
	else
	{
		SpeakString( "Invalid command." );
	}
}

