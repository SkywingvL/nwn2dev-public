/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	Main.cpp

Abstract:

	This module houses the Granny2 converter program.  Its purpose is to provide
	an emulation mode (x86) shell around Granny2!GrannyConvertToRaw, for use by
	non-x86 clients and servers.

--*/

#include <windows.h>
#include <stdio.h>

typedef
bool
(__stdcall * GrannyConvertFileToRawProc)(
	__in const char * SourceFile,
	__in const char * DestinationFile
	);

int
__cdecl
main(
	__in int argc,
	__in_ecount( argc ) char * * argv
	)
/*++

Routine Description:

	This routine is the main program entry point symbol.  It bears
	responsibility for initializing the application.

Arguments:

	argc - Supplies the count of command line arguments.

	argv - Supplies the command line argument array.

Return Value:

	The routine returns the program exit code, which is zero on success
	and non-zero on failure.

Environment:

	User mode.

--*/
{
	const char                 * Granny2DllPath;
	const char                 * InputFile;
	const char                 * OutputFile;
	GrannyConvertFileToRawProc   GrannyConvertFileToRaw;
	HMODULE                      Granny2Dll;
	bool                         Status;

	if (argc != 4)
		return -1;

	Granny2DllPath = argv[ 1 ];
	InputFile      = argv[ 2 ];
	OutputFile     = argv[ 3 ];

	if ((Granny2Dll = LoadLibraryA( Granny2DllPath )) == NULL)
		return -2;	

	GrannyConvertFileToRaw = (GrannyConvertFileToRawProc) GetProcAddress(
		Granny2Dll,
		"_GrannyConvertFileToRaw@8");

	if (GrannyConvertFileToRaw == NULL)
	{
		FreeLibrary( Granny2Dll );
		return -3;
	}

	Status = GrannyConvertFileToRaw( InputFile, OutputFile );
	FreeLibrary( Granny2Dll );

	return Status ? 0 : -4;
}

