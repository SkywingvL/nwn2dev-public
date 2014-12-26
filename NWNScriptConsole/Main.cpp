/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    Main.cpp

Abstract:

    This module houses the main entry point of the console script host.  Its
	purpose is to initialize the script host (and optionally, the resource
	system), and then execute a script.

--*/

#include "Precomp.h"
#include "AppParams.h"
#include "NWScriptHost.h"
#include "../NWNScriptCompilerLib/Nsc.h"

FILE * g_Log;

//
// Define the debug text output interface, used to write debug or log messages
// to the user.
//

class PrintfTextOut : public IDebugTextOut
{

public:

	inline
	PrintfTextOut(
		)
	{
		DWORD Mode;

//		AllocConsole( );

		if (!GetConsoleMode( GetStdHandle( STD_OUTPUT_HANDLE ), &Mode ))
			m_ConsoleRedirected = true;
		else
			m_ConsoleRedirected = false;
	}

	inline
	~PrintfTextOut(
		)
	{
//		FreeConsole( );
	}

	enum { STD_COLOR = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE };

	inline
	virtual
	void
	WriteText(
		__in __format_string const char* fmt,
		...
		)
	{
		va_list ap;

		va_start( ap, fmt );
		WriteTextV( STD_COLOR, fmt, ap );
		va_end( ap );
	}

	inline
	virtual
	void
	WriteText(
		__in WORD Attributes,
		__in __format_string const char* fmt,
		...
		)
	{
		va_list ap;

		va_start( ap, fmt );
		WriteTextV( Attributes, fmt, ap );
		va_end( ap );

		UNREFERENCED_PARAMETER( Attributes );
	}

	inline
	virtual
	void
	WriteTextV(
		__in __format_string const char* fmt,
		__in va_list ap
		)
	{
		WriteTextV( STD_COLOR, fmt, ap );
	}

	inline
	virtual
	void
	WriteTextV(
		__in WORD Attributes,
		__in const char *fmt,
		__in va_list argptr
		)
	/*++

	Routine Description:

		This routine displays text to the log file and the debug console.

		The console output may have color attributes supplied, as per the standard
		SetConsoleTextAttribute API.

		Additionally, if a log file has been opened, a timestamped log entry is
		written to disk.

	Arguments:

		Attributes - Supplies color attributes for the text as per the standard
					 SetConsoleTextAttribute API (e.g. FOREGROUND_RED).

		fmt - Supplies the printf-style format string to use to display text.

		argptr - Supplies format inserts.

	Return Value:

		None.

	Environment:

		User mode.

	--*/
	{
		HANDLE console = GetStdHandle( STD_OUTPUT_HANDLE );
		char buf[8193];
		StringCbVPrintfA(buf, sizeof( buf ), fmt, argptr);
		DWORD n = (DWORD)strlen(buf);

		if (!m_ConsoleRedirected)
		{
			SetConsoleTextAttribute( console, Attributes );
			WriteConsoleA(console, buf, n, &n, 0);
		}
		else
		{
			puts( buf );
		}

		if (g_Log != NULL)
		{
			time_t      t;
			struct tm * tm;

			time( &t );

			if ((tm = gmtime( &t )) != NULL)
			{
				fprintf(
					g_Log,
					"[%04lu-%02lu-%02lu %02lu:%02lu:%02lu] ",
					tm->tm_year + 1900,
					tm->tm_mon + 1,
					tm->tm_mday,
					tm->tm_hour,
					tm->tm_min,
					tm->tm_sec);
			}

			vfprintf( g_Log, fmt, argptr );
			fflush( g_Log );
		}
	}

private:

	bool m_ConsoleRedirected;

};

//
// No reason these should be globals, except for ease of access to the debugger
// right now.
//

NWScriptHost              * g_ScriptHost;
swutil::TimerManager      * g_TimerManager;
PrintfTextOut               g_TextOut;
ResourceManager           * g_ResMan;

BOOL
WINAPI
AppConsoleCtrlHandler(
	__in DWORD dwCtrlType
	)
/*++

Routine Description:

	This routine handles console control events for the Neverwinter Nights
	script console.  It is used to allow the user to request a shutdown.

Arguments:

	dwCtrlType - Supplies the console control code.

Return Value:

	The routine returns TRUE if it handled the control code.

Environment:

	CSRSS injected console control dispatcher thread.

--*/
{
	switch (dwCtrlType)
	{

	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:

		//
		// Exit the program.
		//

		exit( EXIT_FAILURE );

	default:

		return FALSE;

	}
}

void
RunTests(
	__in AppParameters & Params,
	__in ResourceManager & ResMan,
	__in NWScriptHost * ScriptHost
	)
/*++

Routine Description:

	This routine runs a preselected test procedure.

Arguments:

	Params - Supplies the application parameter block.

	ResMan - Supplies the resource manager context.

	ScriptHost - Supplies the script host instance.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	switch (Params.GetTestMode( ))
	{

	case 0:
		break;

	case 1:
		{
			//
			// Run each compiled script.
			//

			for (ResourceManager::FileId Id = ResMan.GetEncapsulatedFileCount( );
				 Id != 0;
				 Id -= 1)
			{
				NWN::ResRef32 ResRef;
				NWN::ResType  ResType;

				if (!ResMan.GetEncapsulatedFileEntry( (Id - 1), ResRef, ResType ))
					continue;

				if (ResType != NWN::ResNCS)
					continue;

				ScriptHost->RunScript( ResRef );
			}
		}
		break;

	case 2:
		{
			//
			// Compile each script.
			//

			std::vector< unsigned char >   Code;
			std::vector< unsigned char >   Symbols;
			NscCompiler                    Compiler( ResMan, true );
			NscResult                      Result;
			int                            CompilerVersion = 169;

			for (ResourceManager::FileId Id = ResMan.GetEncapsulatedFileCount( );
				 Id != 0;
				 Id -= 1)
			{
				NWN::ResRef32 ResRef;
				NWN::ResType  ResType;

				if (!ResMan.GetEncapsulatedFileEntry( (Id - 1), ResRef, ResType ))
					continue;

				if (ResType != NWN::ResNSS)
					continue;

				Result = Compiler.NscCompileScript(
					ResRef,
					CompilerVersion,
					true,
					true,
					Params.GetTextOut( ),
					0,
					Code,
					Symbols);

				if (Result == NscResult_Failure)
				{
					Params.GetTextOut( )->WriteText(
						"ERROR:  Failed to compile script %s.ncs\n",
						ResMan.StrFromResRef( ResRef ).c_str( ));
				}
			}
		}
		break;

	}

}

int
ExecuteScriptHost(
	__in int argc,
	__in_ecount( argc ) wchar_t * * argv
	)
/*++

Routine Description:

	This routine initializes and executes the script host.

Arguments:

	argc - Supplies the count of command line arguments.

	argv - Supplies the command line argument array.

Return Value:

	The script return code is returned.

Environment:

	User mode.

--*/
{
	AppParameters        Params( &g_TextOut, argc, argv );
	int                  ReturnCode;
	bool                 Quiet;
	const char         * Endp;

	Quiet = Params.GetIsNoLogo( );

	if (!Params.GetLogFile( ).empty( ))
		g_Log = fopen( Params.GetLogFile( ).c_str( ), "at" );

	if (!Quiet)
	{
		Params.GetTextOut( )->WriteText(
			"NWNScriptConsole - built %s %s\n"
			NWN2DEV_COPYRIGHT_STR ".\n",
			__DATE__,
			__TIME__);
	}

	if (Params.GetScriptName( ).empty( ))
	{
		Params.GetTextOut( )->WriteText(
			"\n"
			"No script name supplied.  Program usage:\n"
			"\n"
			"  NWNScriptConsole [-module <module>] [-home <homedir>]\n"
			"                   [-installdir <installdir>] [-nologo]\n"
			"                   ScriptName [script arguments]\n"
			"\n"
			"The script name should not contain any extension.  If a module is\n"
			"loaded, then the script will be loaded using standard resource\n"
			"loading semantics; otherwise, it is assumed to be a raw filesystem\n"
			"path (without the .ncs extension).\n"
			"\n");
	
		return 0;
	}

	//
	// Create the resource manager context and load the module, if we are to
	// load one.
	//

	try
	{
		g_ResMan = new ResourceManager( &g_TextOut );
	}
	catch (std::runtime_error &e)
	{
		Params.GetTextOut( )->WriteText(
			"Failed to initialize resource manager: '%s'\n",
			e.what( ) );

		if (g_Log != NULL)
		{
			fclose( g_Log );
			g_Log = NULL;
		}

		return 0;
	}

	try
	{
		g_TimerManager = new swutil::TimerManager( );
	}
	catch (std::exception &e)
	{
		Params.GetTextOut( )->WriteText(
			"Failed to initialize timer manager: '%s'\n",
			e.what( ) );

		delete g_ResMan;
		g_ResMan = NULL;

		if (g_Log != NULL)
		{
			fclose( g_Log );
			g_Log = NULL;
		}

		return 0;
	}

	if (!Params.GetModuleName( ).empty( ))
	{
		//
		// If we've a module name specified, then attempt to load resources. 
		//

		if (!Quiet)
		{
			Params.GetTextOut( )->WriteText(
				"Loading resources for module '%s'...\n",
				Params.GetModuleName( ).c_str( ));
		}

		try
		{
			g_ResMan->LoadModuleResources(
				Params.GetModuleName( ),
				"",
				Params.GetHomeDir( ),
				Params.GetInstallDir( ),
				std::vector< NWN::ResRef32 >( ));
		}
		catch (std::exception &e)
		{
			Params.GetTextOut( )->WriteText(
				"WARNING: Unable to load module resources for module '%s': Exception '%s'.\n",
				Params.GetModuleName( ).c_str( ),
				e.what( ));
		}
	}

	//
	// Now create the script host context.
	//

	try
	{
		g_ScriptHost = new NWScriptHost(
			*g_ResMan,
			*g_TimerManager,
			&Params,
			Params.GetTextOut( ));
	}
	catch (std::exception &e)
	{
		Params.GetTextOut( )->WriteText(
			"Failed to initialize script host: Exception '%s'.\n",
			e.what( ));

		delete g_TimerManager;
		g_TimerManager = NULL;

		delete g_ResMan;
		g_ResMan = NULL;

		if (g_Log != NULL)
		{
			fclose( g_Log );
			g_Log = NULL;
		}

		return 0;
	}

	//
	// Install the ctrl-c handler.
	//

	SetConsoleCtrlHandler( AppConsoleCtrlHandler, TRUE );

	//
	// Execute the script.
	//

	if (!Quiet)
	{
		Params.GetTextOut( )->WriteText(
			"Executing script '%s'.\n",
			Params.GetScriptName( ).c_str( ));
	}

	if ((Endp = strrchr( Params.GetScriptName( ).c_str( ), '.' )) != NULL)
	{
		//
		// If we have a .nss, compile it on the fly.
		//

		if (!_stricmp( Endp, ".nss" ))
		{
			NWN::ResRef32 ResRef( g_ResMan->ResRef32FromStr( Params.GetScriptName( ) ) );
			HANDLE        SrcFile;
			FileWrapper   FileWrap;

			SrcFile = INVALID_HANDLE_VALUE;

			Params.GetTextOut( )->WriteText(
				"Compiling script program...\n");

			//
			// Load the source text and compile the program.
			//

			try
			{
				std::vector< unsigned char >   Code;
				std::vector< unsigned char >   Symbols;
				NscCompiler                    Compiler( *g_ResMan, true );
				NscResult                      Result;
				int                            CompilerVersion = 169;
				std::string                    FileName;
				std::string                    BaseName;
				std::string::size_type         Offs;
				FILE                         * f;

				if (g_ResMan->ResourceExists( ResRef, NWN::ResNSS ))
				{
					Result = Compiler.NscCompileScript(
						ResRef,
						CompilerVersion,
						true,
						true,
						Params.GetTextOut( ),
						0,
						Code,
						Symbols);
				}
				else
				{
					std::string                    DirectScriptStr;
					std::string                    RealScriptName;
					std::vector< unsigned char >   SourceText;

					//
					// For the console script host, allow a script in the
					// working directory to be used directly even if we had no
					// module loaded.  Normally, we would only load scripts
					// via the resource system.
					//

					DirectScriptStr = Params.GetScriptName( );

					if (DirectScriptStr.find( '.' ) == std::string::npos)
					{
						DirectScriptStr += ".";
						DirectScriptStr += g_ResMan->ResTypeToExt( NWN::ResNSS );
					}

					SrcFile = CreateFileA(
						DirectScriptStr.c_str( ),
						GENERIC_READ,
						FILE_SHARE_READ,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);

					if (SrcFile == INVALID_HANDLE_VALUE)
					{
						throw std::runtime_error( 
							"Failed to open script source file" );
					}

					FileWrap.SetFileHandle( SrcFile, true );

					if ((size_t) FileWrap.GetFileSize( ) != 0)
					{
						SourceText.resize( (size_t) FileWrap.GetFileSize( ) );

						FileWrap.ReadFile(
							&SourceText[ 0 ],
							SourceText.size( ),
							"Source file contents");
					}

					Result = Compiler.NscCompileScript(
						ResRef,
						(!SourceText.empty( )) ? &SourceText[ 0 ] : NULL,
						SourceText.size( ),
						CompilerVersion,
						true,
						true,
						Params.GetTextOut( ),
						0,
						Code,
						Symbols);
				}

				switch (Result)
				{

				case NscResult_Success:
					break;

				default:
				case NscResult_Failure:
					Params.GetTextOut( )->WriteText( "Compilation failed.\n" );
					return -1;

				case NscResult_Include:
					Params.GetTextOut( )->WriteText(
						"Script has no entrypoint.\n" );
					return -1;

				}

				BaseName = g_ResMan->StrFromResRef( ResRef );

				Offs = BaseName.rfind( '\\' );

				if (Offs != std::string::npos)
					BaseName = BaseName.substr( Offs + 1 );

				Offs = BaseName.rfind( '/' );

				if (Offs != std::string::npos)
					BaseName = BaseName.substr( Offs + 1 );

				Offs = BaseName.rfind( '.' );

				if (Offs != std::string::npos)
					BaseName.erase( Offs );

				//
				// Now write the compiled NCS/NDB files out so that we can
				// consume them from the script execution environment.
				//

				FileName  = g_ResMan->GetResTempPath( );
				FileName += BaseName;
				FileName += ".";
				FileName += g_ResMan->ResTypeToExt( NWN::ResNCS );

				f = fopen( FileName.c_str( ), "wb" );

				if (f == NULL)
				{
					throw std::runtime_error(
						"Failed to open NCS file for writing." );
				}

				if (!Code.empty( ))
					fwrite( &Code[ 0 ], 1, Code.size( ), f );

				fclose( f );

				Params.SetScriptName( FileName );

				FileName  = g_ResMan->GetResTempPath( );
				FileName += BaseName;
				FileName += ".";
				FileName += g_ResMan->ResTypeToExt( NWN::ResNDB );

				f = fopen( FileName.c_str( ), "wb" );

				if (f == NULL)
				{
					throw std::runtime_error(
						"Failed to open NDB file for writing." );
				}

				if (!Symbols.empty( ))
					fwrite( &Symbols[ 0 ], 1, Symbols.size( ), f );
			}
			catch (std::exception & e)
			{
				if (SrcFile != INVALID_HANDLE_VALUE)
				{
					CloseHandle( SrcFile );
					SrcFile = INVALID_HANDLE_VALUE;
				}

				Params.GetTextOut( )->WriteText(
					"ERROR: Exception '%s' compiling script '%s'.\n",
					e.what( ),
					Params.GetScriptName( ));

				return -1;
			}

			if (SrcFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle( SrcFile );
				SrcFile = INVALID_HANDLE_VALUE;
			}
		}
	}

	ReturnCode = g_ScriptHost->RunScript(
		Params.GetScriptName( ).c_str( ),
		NWN::INVALIDOBJID,
		Params.GetScriptParams( ),
		0);

	switch (Params.GetTestMode( ))
	{

	case 0:
		break;

	default:
		RunTests( Params, *g_ResMan, g_ScriptHost );
		break;

	}

	//
	// Begin the dispatcher execution loop.  If we had a timed script action
	// then we'll wait for it to execute here.
	//

	for (;;)
	{
		ULONG Timeout;

		g_ScriptHost->InitiatePendingDeferredScriptSituations( );

		Timeout = g_TimerManager->RundownTimers( );

		//
		// If we had no more timeouts to run, then terminate.
		//

		if (Timeout == INFINITE)
			break;

		Sleep( Timeout );
	}

	if (!Quiet)
	{
		Params.GetTextOut( )->WriteText(
			"Script program returned %d.\n",
			ReturnCode);
	}

	if (g_Log != NULL)
	{
		fclose( g_Log );
		g_Log = NULL;
	}

	//
	// Remove the ctrl-c handler.
	//

	SetConsoleCtrlHandler( AppConsoleCtrlHandler, FALSE );

	//
	// Now tear down the system.
	//

	g_ScriptHost = NULL;
	delete g_ScriptHost;

	delete g_TimerManager;
	g_TimerManager = NULL;

	delete g_ResMan;
	g_ResMan = NULL;

	return ReturnCode;
}

int
__cdecl
wmain(
	__in int argc,
	__in_ecount( argc ) wchar_t * * argv
	)
/*++

Routine Description:

	This routine is the main program entry point symbol.  It bears
	responsibility for initializing the application.

Arguments:

	argc - Supplies the count of command line arguments.

	argv - Supplies the command line argument array.

Return Value:

	The routine returns the program exit code, which is the return code of the
	script that was executed.

Environment:

	User mode.

--*/
{
	//
	// Launch the script host application.
	//

	return ExecuteScriptHost( argc, argv );
}

