/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    Granny2Reader.cpp

Abstract:

    This module implements the granny2 (*.gr2) frontend interface.  External
	users of the library access gr2 data via the interfaces implemented in
	this module.

--*/

#include "Precomp.h"
#include "Granny2Reader.h"

using namespace granny2;

extern "C" IMAGE_DOS_HEADER __ImageBase;

Granny2Reader::Granny2Reader(
	__in const std::string & Granny2Dll
	)
/*++

Routine Description:

	This routine constructs a new Granny2Reader object and loads the underlying
	granny2.dll library.

Arguments:

	Granny2Dll - Supplies the file name of granny2.dll.

Return Value:

	The newly constructed object.  The routine raises an std::exception should
	GrannyConvertFileToRaw or granny2.dll not be located.

Environment:

	User mode.

--*/
:
#if !GRANNY2_USECONVEXE
  m_Granny2Lib( NULL ),
  m_GrannyConvertFileToRaw( NULL ),
#endif
  m_Granny2Dll( Granny2Dll )
{
	wchar_t ExePath[ MAX_PATH + 1 ];

	//
	// Locate Gr2Conv.exe, which is required to be in the same directory as the
	// main process executable (or DLL).
	//

	if (!GetModuleFileNameW( (HINSTANCE) &__ImageBase, ExePath, MAX_PATH ))
	{
#if GRANNY2_USECONVEXE
		throw std::runtime_error( "Failed to retrieve main EXE path." );
#else
		goto TryDll;
#endif
	}

	if (_wsplitpath_s(
		ExePath,
		NULL,
		0,
		m_Gr2ConvExe,
		RTL_NUMBER_OF( m_Gr2ConvExe ),
		NULL,
		0,
		NULL,
		0))
	{
		m_Gr2ConvExe[ 0 ] = L'\0';

#if GRANNY2_USECONVEXE
		throw std::runtime_error( "Unable to split main EXE path." );
#else
		goto TryDll;
#endif
	}

	if (FAILED( StringCbCatW( m_Gr2ConvExe,  sizeof( m_Gr2ConvExe ), L"\\Gr2Conv.exe" ) ))
	{
		m_Gr2ConvExe[ 0 ] = L'\0';

#if GRANNY2_USECONVEXE
		throw std::runtime_error( "Main EXE path too long." );
#else
		goto TryDll;
#endif
	}

	if (_waccess( m_Gr2ConvExe, 00 ))
	{
		m_Gr2ConvExe[ 0 ] = L'\0';

#if GRANNY2_USECONVEXE
		throw std::runtime_error( "Gr2Conv.exe is missing." );
#else
		goto TryDll;
#endif
	}

#if !GRANNY2_USECONVEXE
TryDll:

	m_Granny2Lib = LoadLibraryA( Granny2Dll.c_str( ) );

	if (m_Granny2Lib == NULL)
		throw std::runtime_error( "Failed to load granny2.dll" );

	try
	{
		m_GrannyConvertFileToRaw = (GrannyConvertFileToRawProc) GetProcAddress(
			m_Granny2Lib,
			"_GrannyConvertFileToRaw@8");

		if (m_GrannyConvertFileToRaw == NULL)
		{
			throw std::runtime_error(
				"Failed to resolve GrannyConvertFileToRaw" );
		}
	}
	catch (...)
	{
		FreeLibrary( m_Granny2Lib );
		throw;
	}
#endif
}

Granny2Reader::~Granny2Reader(
	)
/*++

Routine Description:

	This routine cleans up an already-existing Granny2Reader object.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
#if !GRANNY2_USECONVEXE
	if (m_Granny2Lib != NULL)
	{
		FreeLibrary( m_Granny2Lib );

		m_Granny2Lib = NULL;
	}
#endif
}

granny2::granny2_impl::GR2Info *
Granny2Reader::LoadGranny2File(
	__in const std::string & FileName,
	__in const std::string & TempFileName,
	__in bool PreferOutOfProcess /* = false */
	)
/*++

Routine Description:

	This routine converts a .gr2 file to raw format, and then loads the
	converted temporary file and returns it as a GR2Info pointer.

Arguments:

	FileName - Supplies the name of the *.gr2 file to load.

	TempFileName - Supplies the name of the temp file that represents the raw
	               *.gr2 data form.  The temp file is created by the routine,
	               and it is the responsibility of the caller to delete it.

	PreferOutOfProcess - Supplies a Boolean value that indicates true if the
	                     requestor prefers to make out of process calls to
	                     Granny2.dll to insulate themselves against bugs within
	                     Granny2 core logic.

Return Value:

	The routine returns a pointer to a GR2Info object.  The caller owns the
	object and assumes responsibility for releasing it via operator delete.

Environment:

	User mode.

--*/
{
	granny2::granny2_impl::GR2Info * Gr2Data;
	bool                             FileCreated;
	bool                             Loaded;

	Gr2Data     = NULL;
	FileCreated = false;
	Loaded      = false;

	try
	{
		FileCreated = true;

#if !GRANNY2_USECONVEXE
		//
		// Decompress the file in-process unless we're requested not to.  But
		// always try in-process if the architecture supports it and the out of
		// process helper EXE is missing.
		//

		if ((!PreferOutOfProcess) || (m_Gr2ConvExe[ 0 ] == L'\0'))
		{
			if (!m_GrannyConvertFileToRaw(
				FileName.c_str( ),
				TempFileName.c_str( ) ))
			{
				throw std::runtime_error( "GrannyConvertFileToRaw failed." );
			}

			Loaded = true;
		}
#else
		UNREFERENCED_PARAMETER( PreferOutOfProcess );
#endif

		//
		// Attempt to load out of process if we have not loaded successfully yet.
		//

		if (!Loaded)
		{
			if (!Gr2Conv_GrannyConvertFileToRaw(
				FileName.c_str( ),
				TempFileName.c_str( ) ))
			{
				throw std::runtime_error( "Gr2Conv_GrannyConvertFileToRaw failed." );
			}

			Loaded = true;
		}

		NWStream InStream( TempFileName.c_str( ), true );

		Gr2Data = new granny2::granny2_impl::GR2Info;

		if (!granny2::Recall( InStream, *Gr2Data ))
			throw std::runtime_error( "granny2::Recall failed." );

		return Gr2Data;
	}
	catch (...)
	{
		if (Gr2Data)
			delete Gr2Data;

		//
		// If we've already created the temp (raw) file, then delete it first.
		//

		if (FileCreated)
			DeleteFileA( TempFileName.c_str( ) );

		throw;
	}
}

bool
Granny2Reader::Gr2Conv_GrannyConvertFileToRaw(
	__in const char * FileName,
	__in const char * TempFileName
	)
/*++

Routine Description:

	This routine converts a .gr2 file to raw format by launching the 32-bit
	(x86) converter executable.

	N.B.  This routine is used on non-x86 platforms where the native granny2
	      module cannot be loaded.  An emulation mode executable is launched
	      and acts as a thin wrapper around the Oodle0/Oodle1 decompressor,
	      GrannyConvertFileToRaw.

Arguments:

	FileName - Supplies the name of the *.gr2 file to load.

	TempFileName - Supplies the name of the temp file that represents the raw
	               *.gr2 data form.  The temp file is created by the routine,
				   and it is the responsibility of the caller to delete it.

Return Value:

	The routine returns true on success, else false on failure.

Environment:

	User mode.

--*/
{
	wchar_t             CommandLine[ 3 * MAX_PATH + 128 ];
	PROCESS_INFORMATION ProcessInfo;
	STARTUPINFOW        StartupInfo;
	bool                Succeeded;
	DWORD               ExitCode;

	if (m_Gr2ConvExe[ 0 ] == L'\0')
		return false;

	//
	// Simply launch the x86 converter executable and wait for it to finish.
	//
	// The converter executable takes the following arguments:
	//
	// Gr2Conv.exe <Granny2.dllPath> <InputFile> <OutputFile>
	//

	StringCbPrintfW(
		CommandLine,
		sizeof( CommandLine ),
		L"\"%S\" \"%S\" \"%S\" \"%S\"",
		m_Gr2ConvExe,
		m_Granny2Dll.c_str( ),
		FileName,
		TempFileName);

	ZeroMemory( &StartupInfo, sizeof( StartupInfo ) );

	StartupInfo.cb = sizeof( StartupInfo );

	if (!CreateProcessW(
		m_Gr2ConvExe,
		CommandLine,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&StartupInfo,
		&ProcessInfo))
	{
		return false;
	}

	CloseHandle( ProcessInfo.hThread );
	WaitForSingleObject( ProcessInfo.hProcess, INFINITE );

	//
	// The converter executable returns a return value drawn from the following
	// table:
	//
	// 0 - Success.
	// Other - Failure.
	//

	Succeeded = false;

	if ((GetExitCodeProcess( ProcessInfo.hProcess, &ExitCode)) && (!ExitCode))
		Succeeded = true;

	CloseHandle( ProcessInfo.hProcess );

	return Succeeded;
}
