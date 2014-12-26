/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	gui_test_switch_case.nss

Abstract:

	This module houses test code for switch case blocks.

--*/

void
main(
	)
/*++

Routine Description:

	This script is invoked to test switch case code generation.

Arguments:

	None.

Return Value:

	None.

Environment:

	GUI script.

--*/
{
	switch (10)
	{

	case 10: // Uncomment below to test.
//	case 10:
		break;

	case 42:
		break;

	case 7:
		switch (5)
		{
			case 6:
				break;

			case 7:
				break;

/*			case "abcd":
				break;

			case 4.3:
				break;*/

			case 60:

			default:
				break;
		}
		break;

	default: // Uncomment below to test.
//	default:
		break;

case 60:

	}

	return;
}

