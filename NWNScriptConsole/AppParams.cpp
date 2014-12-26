/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	AppParams.cpp

Abstract:

	This module houses the AppParams class, which bears responsibility for
	processing the program's command line arguments.

--*/

#include "Precomp.h"
#include "AppParams.h"

void
AppParameters::ParseArguments(
	__in int argc,
	__in_ecount( argc ) const wchar_t * * argv
	)
/*++

Routine Description:

	This routine parses command line arguments for the program parameter block.

Arguments:

	argc - Supplies the count of argument elements.

	argv - Supplies the argument vector.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	bool DebugWait;

	DebugWait = false;

	//
	// Parse the application configuration out of the command line.
	//

	for (int i = 1; i < argc; i += 1)
	{
		if (argv[ i ][ 0 ] == L'@')
		{
			//
			// Pull in additional arguments from the response file.
			//

			if (!m_FileParsed)
			{
				m_FileParsed = true;

				ParseResponseFile( argv[ i ] + 1 );
			}
		}
		else if ((!_wcsicmp( argv[ i ], L"-module" )) && (i < argc - 1))
		{
			std::string Str;

			if (!swutil::UnicodeToAnsi( argv[ i += 1 ], Str ))
				continue;

			SetModuleName( Str.substr( 0, 255 ) );
		}
		else if ((!_wcsicmp( argv[ i ], L"-moduledir" )) && (i < argc - 1))
		{
			std::string Str;

			if (!swutil::UnicodeToAnsi( argv[ i += 1 ], Str ))
				continue;

			SetModuleName( Str.substr( 0, 255 ) );
		}
		else if ((!_wcsicmp( argv[ i ], L"-home" )) && (i < argc - 1))
		{
			std::string Str;

			if (!swutil::UnicodeToAnsi( argv[ i += 1 ], Str ))
				continue;

			Str += "\\";

			SetHomeDir( Str.substr( 0, MAX_PATH ) );
		}
		else if ((!_wcsicmp( argv[ i ], L"-installdir" )) && (i < argc - 1))
		{
			std::string Str;

			if (!swutil::UnicodeToAnsi( argv[ i += 1 ], Str ))
				continue;

			Str += "\\";

			SetInstallDir( Str.substr( 0, MAX_PATH ) );
		}
		else if ((!_wcsicmp( argv[ i ], L"-log" )) && (i < argc - 1))
		{
			std::string Str;

			if (!swutil::UnicodeToAnsi( argv[ i += 1 ], Str ))
				continue;

			SetLogFile( Str );
		}
		else if ((!_wcsicmp( argv[ i ], L"-scriptdebug" )) && (i < argc - 1))
			SetScriptDebug( _wtoi( argv[ i += 1 ] ) );
		else if ((!_wcsicmp( argv[ i ], L"-testmode") ) && (i < argc - 1))
			SetTestMode( _wtoi( argv[ i += 1 ] ) );
		else if ((!_wcsicmp( argv[ i ], L"-nologo" )))
			SetIsNoLogo( true );
		else if ((!_wcsicmp( argv[ i ], L"-allowmanagedscripts" )) && (i < argc - 1))
			SetAllowManagedScripts( _wtoi( argv[ i += 1 ] ) != 0 );
		else if (!_wcsicmp( argv[ i ], L"-debugwait" ))
			DebugWait = true;
		else if (m_ScriptName.empty( ))
		{
			std::string Str;

			if (!swutil::UnicodeToAnsi( argv[ i ], Str ))
				continue;

			SetScriptName( Str );
		}
		else
		{
			std::string Str;

			if (!swutil::UnicodeToAnsi( argv[ i ], Str ))
				continue;

			m_ScriptParams.push_back( Str );
		}
	}

	//
	// If we are to wait for the user to prepare a debugger, block on keyboard
	// input.
	//

	if (DebugWait)
	{
		fprintf(
			stderr,
			"Press <enter> to begin program initialization (process %lu, main thread %lu).\n",
			GetCurrentProcessId( ),
			GetCurrentThreadId( ));

		getc( stdin );

		fprintf(
			stderr,
			"Resuming program initialization...\n");
	}
}

void
AppParameters::ParseResponseFile(
	__in const wchar_t * ResponseFileName
	)
/*++

Routine Description:

	This routine parses command line arguments from a response file.  Each line
	represents an argument.

Arguments:

	ResponseFileName - Supplies the file name of the response file.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	FILE * f;

	f = NULL;

	try
	{
		std::vector< std::wstring >    Args;
		std::vector< const wchar_t * > ArgVector;
		wchar_t                        Line[ 1025 ];

		f = _wfopen( ResponseFileName, L"rt" );

		if (f == NULL)
			throw std::runtime_error( "Failed to open response file." );

		//
		// Tokenize the file into lines and then build a pointer array that is
		// consistent with the standard 'main()' contract.  The first argument
		// is copied from the main argument array, if it exists (i.e. the
		// program name).
		//

		if (m_argc > 0)
			Args.push_back( m_argv[ 0 ] );

		while (fgetws( Line, RTL_NUMBER_OF( Line ) - 1, f ))
		{
			wcstok( Line, L"\r\n" );

			if (!Line[ 0 ])
				continue;

			Args.push_back( Line );
		}

		//
		// N.B.  Beyond this point no modifications may be made to Args as we
		//       are creating pointers into the data storage of each member for
		//       the remainder of the function.
		//

		ArgVector.reserve( Args.size( ) );

		for (std::vector< std::wstring >::const_iterator it = Args.begin( );
		     it != Args.end( );
		     ++it)
		{
			ArgVector.push_back( it->c_str( ) );
		}

		if (!ArgVector.empty( ))
			ParseArguments( (int) ArgVector.size( ), &ArgVector[ 0 ] );
	}
	catch (std::exception &e)
	{
		if (f != NULL)
		{
			fclose( f );
			f = NULL;
		}

		GetTextOut( )->WriteText(
			"WARNING: AppParameters::ParseResponseFile: Exception processing response file '%S': '%s'.\n",
			ResponseFileName,
			e.what( ));
	}
}

void
AppParameters::FindCriticalDirectories(
	)
/*++

Routine Description:

	This routine attempts to automatically detect default directories for the
	installation and home directories that the game will use.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	try
	{
		SetInstallDir( GetNwn2InstallPath( ) );
	}
	catch (std::exception &e)
	{
		GetTextOut( )->WriteText(
			"AppParameters::FindCriticalDirectories: WARNING: Exception '%s' detecting install directory.\nYou may need to manually set the install path with -installdir <path>.\n",
			e.what( ));
	}

	try
	{
		SetHomeDir( GetNwn2HomePath( ) );
	}
	catch (std::exception &e)
	{
		GetTextOut( )->WriteText(
			"AppParameters::FindCriticalDirectories: WARNING: Exception '%s' detecting home directory.\nYou may need to manually set the home path with -home <path>.\n",
			e.what( ));
	}
}

std::string
AppParameters::GetNwn2InstallPath(
	) const
/*++

Routine Description:

	This routine attempts to auto detect the NWN2 installation path from the
	registry.

Arguments:

	None.

Return Value:

	The routine returns the game installation path if successful.  Otherwise,
	an std::exception is raised.

Environment:

	User mode.

--*/
{
	HKEY  Key;
	CHAR  NameBuffer[ MAX_PATH + 1 ];
	DWORD NameBufferSize;
	LONG  Status;

	Status = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Obsidian\\NWN 2\\Neverwinter",
		REG_OPTION_RESERVED,
#ifdef _WIN64
		KEY_QUERY_VALUE | KEY_WOW64_32KEY,
#else
		KEY_QUERY_VALUE,
#endif
		&Key);

	if (Status != NO_ERROR)
	{
		Status = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			L"SOFTWARE\\GOG.com\\GOGNWN2COMPLETE",
			REG_OPTION_RESERVED,
#ifdef _WIN64
			KEY_QUERY_VALUE | KEY_WOW64_32KEY,
#else
			KEY_QUERY_VALUE,
#endif
			&Key);

		if (Status == NO_ERROR)
		{
			NameBufferSize = sizeof( NameBuffer ) - sizeof( NameBuffer[ 0 ] );

			Status = RegQueryValueExA(
				Key,
				"PATH",
				NULL,
				NULL,
				(LPBYTE) NameBuffer,
				&NameBufferSize);

			RegCloseKey( Key );
			Key = NULL;

			if (Status == NO_ERROR)
			{
				//
				// Strip trailing null byte if it exists.
				//

				if ((NameBufferSize > 0) &&
					(NameBuffer[ NameBufferSize - 1 ] == '\0'))
					NameBufferSize -= 1;

				return std::string( NameBuffer, NameBufferSize );
			}
		}

		throw std::exception( "Unable to open NWN2 registry key" );
	}

	try
	{
		bool                FoundIt;
		static const char * ValueNames[ ] =
		{
			"Path",     // Retail NWN2
			"Location", // Steam NWN2
		};

		FoundIt = false;

		for (size_t i = 0; i < _countof( ValueNames ); i += 1)
		{
			NameBufferSize = sizeof( NameBuffer ) - sizeof( NameBuffer[ 0 ] );

			Status = RegQueryValueExA(
				Key,
				ValueNames[ i ],
				NULL,
				NULL,
				(LPBYTE) NameBuffer,
				&NameBufferSize);

			if (Status != NO_ERROR)
				continue;

			//
			// Strip trailing null byte if it exists.
			//

			if ((NameBufferSize > 0) &&
				(NameBuffer[ NameBufferSize - 1 ] == '\0'))
				NameBufferSize -= 1;

			RegCloseKey( Key );
			Key = NULL;

			return std::string( NameBuffer, NameBufferSize );
		}

		throw std::exception( "Unable to read Path from NWN2 registry key" );
	}
	catch (...)
	{
		if (Key != NULL)
			RegCloseKey( Key );
		throw;
	}
}

std::string
AppParameters::GetNwn2HomePath(
	) const
/*++

Routine Description:

	This routine attempts to auto detect the NWN2 home directory path from the
	current user environment.  The home path is where per-user data, such as
	most module data, HAK files, the server vault, etc are stored.

Arguments:

	None.

Return Value:

	The routine returns the game per-user home path if successful.  Otherwise,
	an std::exception is raised.

Environment:

	User mode.

--*/
{
	CHAR        DocumentsPath[ MAX_PATH ];
	std::string HomePath;

	if (!SHGetSpecialFolderPathA( NULL, DocumentsPath, CSIDL_PERSONAL, TRUE ))
		throw std::runtime_error( "Couldn't get user documents path." );

	HomePath  = DocumentsPath;
	HomePath += "\\Neverwinter Nights 2\\";

	return HomePath;
}
