/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    Main.cpp

Abstract:

    This module houses the main entry point of the compiler driver.  The
    compiler driver provides a user interface to compile scripts under user
    control.

--*/

#include "Precomp.h"
#include "../NWN2DataLib/TextOut.h"
#include "../NWN2DataLib/ResourceManager.h"
#include "../NWN2DataLib/GffFileWriter.h"
#include "../NWN2DataLib/NWScriptReader.h"
#include "../NWNScriptCompilerLib/Nsc.h"

typedef std::vector< std::wstring > WStringVec;
typedef std::vector< const wchar_t * > WStringArgVec;

typedef enum _NSCD_FLAGS
{
	//
	// Stop processing files on the first error.
	//

	NscDFlag_StopOnError             = 0x00000001,

	NscDFlag_LastFlag
} NSCD_FLAGS, * PNSCD_FLAGS;

typedef const enum _NSCD_FLAGS * PCNSCD_FLAGS;

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

class WriteFileTextOut : public IDebugTextOut
{

public:

	inline
	WriteFileTextOut(
		__in FILE * OutFile
		)
	: m_OutFile( OutFile )
	{
	}

	inline
	~WriteFileTextOut(
		)
	{
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

		This routine displays text to the output file associated with the output
		object.

		The console output may have color attributes supplied, as per the standard
		SetConsoleTextAttribute API.

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
		char buf[8193];
		StringCbVPrintfA(buf, sizeof( buf ), fmt, argptr);

		fputs( buf, m_OutFile );

		UNREFERENCED_PARAMETER( Attributes );
	}

private:

	FILE * m_OutFile;

};

//
// No reason these should be globals, except for ease of access to the debugger
// right now.
//

PrintfTextOut               g_TextOut;
ResourceManager           * g_ResMan;

NWACTION_TYPE
ConvertNscType(
	__in NscType Type
	);

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

std::string
GetNwn2InstallPath(
	)
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
GetNwn1InstallPath(
	)
/*++

Routine Description:

	This routine attempts to auto detect the NWN1 installation path from the
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
	HKEY Key;
	LONG Status;

	Status = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\BioWare\\NWN\\Neverwinter",
		REG_OPTION_RESERVED,
#ifdef _WIN64
		KEY_QUERY_VALUE | KEY_WOW64_32KEY,
#else
		KEY_QUERY_VALUE,
#endif
		&Key);

	if (Status != NO_ERROR)
		throw std::runtime_error( "Unable to open NWN1 registry key" );

	try
	{
			CHAR                NameBuffer[ MAX_PATH + 1 ];
			DWORD               NameBufferSize;
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

				return std::string( NameBuffer, NameBufferSize );
			}

			throw std::exception( "Unable to read Path from NWN1 registry key" );
	}
	catch (...)
	{
		RegCloseKey( Key );
		throw;
	}
}
std::string
GetNwn2HomePath(
	)
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

void
LoadModule(
	__inout ResourceManager & ResMan,
	__in const std::string & ModuleName,
	__in const std::string & NWN2Home,
	__in const std::string & InstallDir,
	__in bool Erf16,
	__in const std::string & CustomModPath
	)
/*++

Routine Description:

	This routine loads a module into the resource system.

Arguments:

	ResMan - Supplies the ResourceManager instance that is to load the module.

	ModuleName - Supplies the resource name of the module to load.  If an
	             empty string is supplied, only base game resources are loaded.

	NWN2Home - Supplies the users NWN2 home directory (i.e. NWN2 Documents dir).

	InstallDir - Supplies the game installation directory.

	Erf16 - Supplies a Boolean value indicating true if 16-byte ERFs are to be
	        used (i.e. for NWN1-style modules), else false if 32-byte ERFs are
	        to be used (i.e. for NWN2-style modules).

	CustomModPath - Optionally supplies an override path to search for a module
	                file within, bypassing the standard module load heuristics.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	std::vector< NWN::ResRef32 >        HAKList;
	std::string                         CustomTlk;
	ResourceManager::ModuleLoadParams   LoadParams;
	ResourceManager::StringVec          KeyFiles;

	ZeroMemory( &LoadParams, sizeof( LoadParams ) );

	if (!ModuleName.empty( ) || !CustomModPath.empty( ))
	{
		//
		// Load up the module.  First, we load just the core module resources,
		// then we determine the HAK list and load all of the HAKs up too.
		//
		// Turn off granny2 loading as it's unnecessary for this program, and
		// prefer to load directory modules (as changes to ERF modules aren't
		// saved).
		//

		LoadParams.SearchOrder = ResourceManager::ModSearch_PrefDirectory;
		LoadParams.ResManFlags = ResourceManager::ResManFlagNoGranny2          |
		                         ResourceManager::ResManFlagLoadCoreModuleOnly |
		                         ResourceManager::ResManFlagRequireModuleIfo;

		if (Erf16)
			LoadParams.ResManFlags |= ResourceManager::ResManFlagErf16;

		if (!CustomModPath.empty( ))
			LoadParams.CustomModuleSourcePath = CustomModPath.c_str( );

		ResMan.LoadModuleResources(
			ModuleName,
			"",
			NWN2Home,
			InstallDir,
			HAKList,
			&LoadParams);

		{
			DemandResourceStr                ModuleIfoFile( ResMan, "module", NWN::ResIFO );
			GffFileReader                    ModuleIfo( ModuleIfoFile, ResMan );
			const GffFileReader::GffStruct * RootStruct = ModuleIfo.GetRootStruct( );
			GffFileReader::GffStruct         Struct;
			size_t                           Offset;

			RootStruct->GetCExoString( "Mod_CustomTlk", CustomTlk );

			//
			// Chop off the .tlk extension in the CustomTlk field if we had one.
			//

			if ((Offset = CustomTlk.rfind( '.' )) != std::string::npos)
				CustomTlk.erase( Offset );

			for (size_t i = 0; i <= UCHAR_MAX; i += 1)
			{
				GffFileReader::GffStruct Hak;
				NWN::ResRef32            HakRef;

				if (!RootStruct->GetListElement( "Mod_HakList", i, Hak ))
					break;

				if (!Hak.GetCExoStringAsResRef( "Mod_Hak", HakRef ))
					throw std::runtime_error( "Failed to read Mod_HakList.Mod_Hak" );

				HAKList.push_back( HakRef );
			}

			//
			// If there were no haks, then try the legacy field.
			//

			if (HAKList.empty( ))
			{
				NWN::ResRef32 HakRef;

				if ((RootStruct->GetCExoStringAsResRef( "Mod_Hak", HakRef )) &&
					 (HakRef.RefStr[ 0 ] != '\0'))
				{
					HAKList.push_back( HakRef );
				}
			}
		}
	}

	//
	// Now perform a full load with the HAK list and CustomTlk available.
	//
	// N.B.  The DemandResourceStr above must go out of scope before we issue a
	//       new load, as it references a temporary file that will be cleaned up
	//       by the new load request.
	//

	ZeroMemory( &LoadParams, sizeof( LoadParams ) );

	//
	// Load up the module.  First, we load just the core module resources, then
	// we determine the HAK list and load all of the HAKs up too.
	//
	// Turn off granny2 loading as it's unnecessary for this program, and prefer
	// to load directory modules (as changes to ERF modules aren't saved).
	//

	LoadParams.SearchOrder = ResourceManager::ModSearch_PrefDirectory;
	LoadParams.ResManFlags = ResourceManager::ResManFlagNoGranny2         |
	                         ResourceManager::ResManFlagRequireModuleIfo;

	if (Erf16)
	{
		LoadParams.ResManFlags |= ResourceManager::ResManFlagErf16;

		KeyFiles.push_back( "xp3" );
		KeyFiles.push_back( "xp2patch" );
		KeyFiles.push_back( "xp2" );
		KeyFiles.push_back( "xp1patch" );
		KeyFiles.push_back( "xp1" );
		KeyFiles.push_back( "chitin" );

		LoadParams.KeyFiles = &KeyFiles;
	}

	if (ModuleName.empty( ) && CustomModPath.empty( ))
		LoadParams.ResManFlags |= ResourceManager::ResManFlagBaseResourcesOnly;

	if (!CustomModPath.empty( ))
		LoadParams.CustomModuleSourcePath = CustomModPath.c_str( );

	ResMan.LoadModuleResources(
		ModuleName,
		CustomTlk,
		NWN2Home,
		InstallDir,
		HAKList,
		&LoadParams
		);
}

bool
LoadFileFromDisk(
	__in const std::string & FileName,
	__out std::vector< unsigned char > & FileContents
	)
/*++

Routine Description:

	This routine loads a file from a raw disk
	This routine canonicalizes an input file name to its resource name and
	resource type, and then loads the entire file contents into memory.

	The input file may be a short filename or a filename with a path.  It may be
	backed by the raw filesystem or by the resource system (in that order of
	precedence).

Arguments:

	ResMan - Supplies the resource manager to use to service file load requests.

	InFile - Supplies the filename of the file to load.

	FileResRef - Receives the canonical RESREF name of the input file.

	FileResType - Receives the canonical ResType (extension) of the input file.

	FileContents - Receives the contents of the input file.

Return Value:

	The routine returns a Boolean value indicating true on success, else false
	on failure.

Environment:

	User mode.

--*/
{
	FileWrapper FileWrap;
	HANDLE      SrcFile;

	FileContents.clear( );

	SrcFile = CreateFileA(
		FileName.c_str( ),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (SrcFile == INVALID_HANDLE_VALUE)
		return false;

	try
	{
		FileWrap.SetFileHandle( SrcFile, true );

		if ((size_t) FileWrap.GetFileSize( ) != 0)
		{
			FileContents.resize( (size_t) FileWrap.GetFileSize( ) );

			FileWrap.ReadFile(
				&FileContents[ 0 ],
				FileContents.size( ),
				"LoadFileFromDisk File Contents");
		}
	}
	catch (std::exception)
	{
		CloseHandle( SrcFile );
		SrcFile = INVALID_HANDLE_VALUE;
		return false;
	}

	CloseHandle( SrcFile );
	SrcFile = INVALID_HANDLE_VALUE;
	
	return true;
}

bool
LoadInputFile(
	__in ResourceManager & ResMan,
	__in IDebugTextOut * TextOut,
	__in const std::string & InFile,
	__out NWN::ResRef32 & FileResRef,
	__out NWN::ResType & FileResType,
	__out std::vector< unsigned char > & FileContents
	)
/*++

Routine Description:

	This routine canonicalizes an input file name to its resource name and
	resource type, and then loads the entire file contents into memory.

	The input file may be a short filename or a filename with a path.  It may be
	backed by the raw filesystem or by the resource system (in that order of
	precedence).

Arguments:

	ResMan - Supplies the resource manager to use to service file load requests.

	TextOut - Supplies the text out interface used to receive any diagnostics
	          issued.

	InFile - Supplies the filename of the file to load.

	FileResRef - Receives the canonical RESREF name of the input file.

	FileResType - Receives the canonical ResType (extension) of the input file.

	FileContents - Receives the contents of the input file.

Return Value:

	The routine returns a Boolean value indicating true on success, else false
	on failure.

	On catastrophic failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	char                   Drive[ _MAX_DRIVE ];
	char                   Dir[ _MAX_DIR ];
	char                   FileName[ _MAX_FNAME ];
	char                   Extension[ _MAX_EXT ];

	//
	// First, canonicalize the filename.
	//

	if (_splitpath_s(
		InFile.c_str( ),
		Drive,
		Dir,
		FileName,
		Extension))
	{
		TextOut->WriteText(
			"Error: Malformed file pathname \"%s\".\n", InFile.c_str( ));

		return false;
	}

	if (Extension[ 0 ] != '.')
		FileResType = NWN::ResINVALID;
	else
		FileResType = ResMan.ExtToResType( Extension + 1 );

	FileResRef = ResMan.ResRef32FromStr( FileName );

	//
	// Load the file directly if we can, otherwise attempt it via the resource
	// system.
	//

	if (!_access( InFile.c_str( ), 00 ))
	{
		return LoadFileFromDisk( InFile, FileContents );
	}
	else
	{
		DemandResource32 DemandRes( ResMan, FileResRef, FileResType );

		return LoadFileFromDisk( DemandRes, FileContents );
	}
}

bool
CompileSourceFile(
	__in NscCompiler & Compiler,
	__in int CompilerVersion,
	__in bool Optimize,
	__in bool IgnoreIncludes,
	__in bool SuppressDebugSymbols,
	__in bool Quiet,
	__in bool VerifyCode,
	__in IDebugTextOut * TextOut,
	__in UINT32 CompilerFlags,
	__in const NWN::ResRef32 & InFile,
	__in const std::vector< unsigned char > & InFileContents,
	__in const std::string & OutBaseFile
	)
/*++

Routine Description:

	This routine compiles a single source file according to the specified set of
	compilation options.

Arguments:

	NscCompiler - Supplies the compiler context that will be used to process the
	              request.

	CompilerVersion - Supplies the BioWare-compatible compiler version number.

	Optimize - Supplies a Boolean value indicating true if the script should be
	           optimized.

	IgnoreIncludes - Supplies a Boolean value indicating true if include-only
	                 source files should be ignored.

	SuppressDebugSymbols - Supplies a Boolean value indicating true if debug
	                       symbol generation should be suppressed.

	Quiet - Supplies a Boolean value that indicates true if non-critical
	        messages should be silenced.

	VerifyCode - Supplies a Boolean value that indicates true if generated code
	             is to be verified with the analyzer/verifier if compilation was
	             successful.

	TextOut - Supplies the text out interface used to receive any diagnostics
	          issued.

	CompilerFlags - Supplies compiler control flags.  Legal values are drawn
	                from the NscCompilerFlags enumeration.

	InFile - Supplies the RESREF corresponding to the input file name.

	InFileContents - Supplies the contents of the input file.

	OutBaseFile - Supplies the base name (potentially including path) of the
	              output file.  No extension is present.

Return Value:

	The routine returns a Boolean value indicating true on success, else false
	on failure.

	On catastrophic failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	std::vector< unsigned char >   Code;
	std::vector< unsigned char >   Symbols;
	NscResult                      Result;
	std::string                    FileName;
	FILE                         * f;

	if (!Quiet)
	{
		TextOut->WriteText(
			"Compiling: %.32s.NSS\n",
			InFile.RefStr);
	}

	//
	// Execute the main compilation pass.
	//

	Result = Compiler.NscCompileScript(
		InFile,
		(!InFileContents.empty( )) ? &InFileContents[ 0 ] : NULL,
		InFileContents.size( ),
		CompilerVersion,
		Optimize,
		IgnoreIncludes,
		TextOut,
		CompilerFlags,
		Code,
		Symbols);

	switch (Result)
	{

	case NscResult_Failure:
		TextOut->WriteText(
			"Compilation aborted with errors.\n");

		return false;

	case NscResult_Include:
		if (!Quiet)
		{
			TextOut->WriteText(
				"%.32s.nss is an include file, ignored.\n",
				InFile.RefStr);
		}

		return true;

	case NscResult_Success:
		break;

	default:
		TextOut->WriteText(
			"Unknown compiler status code.\n");

		return false;

	}

	//
	// If we compiled successfully, write the results to disk.
	//

	FileName  = OutBaseFile;
	FileName += ".ncs";

	f = fopen( FileName.c_str( ), "wb" );

	if (f == NULL)
	{
		TextOut->WriteText(
			"Error: Unable to open output file \"%s\".\n",
			FileName.c_str( ));

		return false;
	}

	if (!Code.empty( ))
	{
		if (fwrite( &Code[ 0 ], Code.size( ), 1, f ) != 1)
		{
			fclose( f );

			TextOut->WriteText(
				"Error: Failed to write to output file \"%s\".\n",
				FileName.c_str( ));

			return false;
		}
	}

	fclose( f );

	if (!SuppressDebugSymbols)
	{
		FileName  = OutBaseFile;
		FileName += ".ndb";

		f = fopen( FileName.c_str( ), "wb" );

		if (f == NULL)
		{
			TextOut->WriteText(
				"Error: Failed to open debug symbols file \"%s\".\n",
				FileName.c_str( ));

			return false;
		}

		if (!Symbols.empty( ))
		{
			if (fwrite( &Symbols[ 0 ], Symbols.size( ), 1, f ) != 1)
			{
				fclose( f );

				TextOut->WriteText(
					"Error: Failed to write to debug symbols file \"%s\".\n",
					FileName.c_str( ));

				return false;
			}
		}

		fclose( f );
	}

	if (VerifyCode)
	{
		try
		{
			std::vector< NWACTION_DEFINITION >          ActionDefs;
			std::list< NscPrototypeDefinition >         ActionPrototypes;
			std::list< std::vector< NWACTION_TYPE > >   ActionTypes;
			NWSCRIPT_ACTION                             ActionId;

			//
			// Build the action definition table for the static analysis phase.
			// The table is generated dynamically based on the compiled
			// nwscript.nss
			//

			for (ActionId = 0;
				  ;
				  ActionId += 1)
			{
				NWACTION_DEFINITION          ActionDef;
				NscPrototypeDefinition       ActionPrototype;

				if (!Compiler.NscGetActionPrototype( (int) ActionId, ActionPrototype ))
					break;

				ActionPrototypes.push_back( ActionPrototype );

				ZeroMemory( &ActionDef, sizeof( ActionDef ) );

				ActionDef.Name            = ActionPrototypes.back( ).Name.c_str( );
				ActionDef.ActionId        = ActionId;
				ActionDef.MinParameters   = ActionPrototype.MinParameters;
				ActionDef.NumParameters   = ActionPrototype.NumParameters;
				ActionDef.ReturnType      = ConvertNscType( ActionPrototype.ReturnType );

				//
				// Convert parameter types over.
				//

				ActionTypes.push_back( std::vector< NWACTION_TYPE >( ) );
				std::vector< NWACTION_TYPE > & ReturnTypes = ActionTypes.back( );

				ReturnTypes.resize( ActionPrototype.ParameterTypes.size( ) );

				for (size_t i = 0; i < ActionPrototype.ParameterTypes.size( ); i += 1)
				{
					ReturnTypes[ i ] = ConvertNscType(
						ActionPrototype.ParameterTypes[ i ] );
				}

				if (!ReturnTypes.empty( ))
					ActionDef.ParameterTypes = &ReturnTypes[ 0 ];
				else
					ActionDef.ParameterTypes = NULL;
			
				ActionDefs.push_back( ActionDef );
			}

			FileName  = OutBaseFile;
			FileName += ".ncs";

			//
			// Create a script reader over the compiled script, and hand it off to
			// an analyzer instance.
			//

			NWScriptReader ScriptReader( FileName.c_str( ) );

			if (!SuppressDebugSymbols)
			{
				FileName  = OutBaseFile;
				FileName += ".ndb";

				ScriptReader.LoadSymbols( FileName.c_str( ) );
			}

			//
			// Perform the analysis and generate the IR.
			//

			NWScriptAnalyzer ScriptAnalyzer(
				TextOut,
				(!ActionDefs.empty( )) ? &ActionDefs[ 0 ] : NULL,
				(NWSCRIPT_ACTION) ActionDefs.size( ));

			ScriptAnalyzer.Analyze(
				&ScriptReader,
				0 );
		}
		catch (NWScriptAnalyzer::script_error &e)
		{
			TextOut->WriteText(
				"Error: (Verifier error): Analyzer exception '%s' ('%s') at PC=%08X, SP=%08X analyzing script \"%.32s.ncs\".\n",
				e.what( ),
				e.specific( ),
				(unsigned long) e.pc( ),
				(unsigned long) e.stack_index( ),
				InFile.RefStr);

			return false;
		}
		catch (std::exception &e)
		{
			TextOut->WriteText(
				"Error: (Verifier error): Exception '%s' analyzing script \"%.32s.ncs\".\n",
				e.what( ),
				OutBaseFile.c_str( ));

			return false;
		}
	}

	return true;
}

NWACTION_TYPE
ConvertNscType(
	__in NscType Type
	)
/*++

Routine Description:

	This routine converts a compiler NscType to an analyzer NWACTION_TYPE.

Arguments:

	Type - Supplies the NscType-encoding type to convert.

Return Value:

	The routine returns the corresponding NWACTION_TYPE for the given NscType.
	If there was no matching conversion (i.e. a user defined type was in use),
	then an std::exception is raised.

Environment:

	User mode.

--*/
{
	switch (Type)
	{

	case NscType_Void:
		return ACTIONTYPE_VOID;
	case NscType_Integer:
		return ACTIONTYPE_INT;
	case NscType_Float:
		return ACTIONTYPE_FLOAT;
	case NscType_String:
		return ACTIONTYPE_STRING;
	case NscType_Object:
		return ACTIONTYPE_OBJECT;
	case NscType_Vector:
		return ACTIONTYPE_VECTOR;
	case NscType_Action:
		return ACTIONTYPE_ACTION;
	default:
		if ((Type >= NscType_Engine_0) && (Type < NscType_Engine_0 + 10))
			return (NWACTION_TYPE) (ACTIONTYPE_ENGINE_0 + (Type - NscType_Engine_0));
		else
			throw std::runtime_error( "Illegal NscType for action service handler." );
	}
}

bool
DisassembleScriptFile(
	__in ResourceManager & ResMan,
	__in NscCompiler & Compiler,
	__in bool Quiet,
	__in IDebugTextOut * TextOut,
	__in const NWN::ResRef32 & InFile,
	__in const std::vector< unsigned char > & InFileContents,
	__in const std::vector< unsigned char > & DbgFileContents,
	__in const std::string & OutBaseFile
	)
/*++

Routine Description:

	This routine processes a single input file according to the desired compile
	or diassemble options.

Arguments:

	ResMan - Supplies the resource manager to use to service file load requests.

	NscCompiler - Supplies the compiler context that will be used to process the
	              request.

	Quiet - Supplies a Boolean value that indicates true if non-critical
	        messages should be silenced.

	TextOut - Supplies the text out interface used to receive any diagnostics
	          issued.

	InFile - Supplies the RESREF corresponding to the input file name.

	InFileContents - Supplies the contents of the input file.

	DbgFileContents - Supplies the contents of the associated debug symbols, if
	                  any could be located.  This may be empty.

	OutBaseFile - Supplies the base name (potentially including path) of the
	              output file.  No extension is present.

Return Value:

	The routine returns a Boolean value indicating true on success, else false
	on failure.

	On catastrophic failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	std::string                                 Disassembly;
	std::string                                 FileName;
	std::string                                 ScriptTempFile;
	std::string                                 SymbolsTempFile;
	FILE                                      * f;
	std::vector< NWACTION_DEFINITION >          ActionDefs;
	std::list< NscPrototypeDefinition >         ActionPrototypes;
	std::list< std::vector< NWACTION_TYPE > >   ActionTypes;
	NWSCRIPT_ACTION                             ActionId;

	if (!Quiet)
	{
		TextOut->WriteText(
			"Diassembling: %.32s.NCS\n",
			InFile.RefStr);
	}

	//
	// Disassemble the script to raw assembly.
	//

	Compiler.NscDisassembleScript(
		(!InFileContents.empty( )) ? &InFileContents[ 0 ] : NULL,
		InFileContents.size( ),
		Disassembly);

	FileName  = OutBaseFile;
	FileName += ".pcode";

	f = fopen( FileName.c_str( ), "wt" );

	if (f == NULL)
	{
		TextOut->WriteText(
			"Error: Unable to open disassembly file \"%s\".\n",
			FileName.c_str( ));

		return false;
	}

	if (!Disassembly.empty( ))
	{
		if (fwrite( &Disassembly[ 0 ], Disassembly.size( ), 1, f ) != 1)
		{
			fclose( f );

			TextOut->WriteText(
				"Error: Failed to write to disassembly file \"%s\".\n",
				FileName.c_str( ));

			return false;
		}
	}

	fclose( f );

	//
	// Build the action definition table for the static analysis phase.  The
	// table is generated dynamically based on the compiled nwscript.nss.
	//

	for (ActionId = 0;
	     ;
	     ActionId += 1)
	{
		NWACTION_DEFINITION          ActionDef;
		NscPrototypeDefinition       ActionPrototype;

		if (!Compiler.NscGetActionPrototype( (int) ActionId, ActionPrototype ))
			break;

		ActionPrototypes.push_back( ActionPrototype );

		ZeroMemory( &ActionDef, sizeof( ActionDef ) );

		ActionDef.Name            = ActionPrototypes.back( ).Name.c_str( );
		ActionDef.ActionId        = ActionId;
		ActionDef.MinParameters   = ActionPrototype.MinParameters;
		ActionDef.NumParameters   = ActionPrototype.NumParameters;
		ActionDef.ReturnType      = ConvertNscType( ActionPrototype.ReturnType );

		//
		// Convert parameter types over.
		//

		ActionTypes.push_back( std::vector< NWACTION_TYPE >( ) );
		std::vector< NWACTION_TYPE > & ReturnTypes = ActionTypes.back( );

		ReturnTypes.resize( ActionPrototype.ParameterTypes.size( ) );

		for (size_t i = 0; i < ActionPrototype.ParameterTypes.size( ); i += 1)
		{
			ReturnTypes[ i ] = ConvertNscType(
				ActionPrototype.ParameterTypes[ i ] );
		}

		if (!ReturnTypes.empty( ))
			ActionDef.ParameterTypes = &ReturnTypes[ 0 ];
		else
			ActionDef.ParameterTypes = NULL;
	
		ActionDefs.push_back( ActionDef );
	}

	//
	// Now attempt to raise the script to the high level IR and print the IR out
	// as well.
	//
	// The script analyzer only operates on disk files, and the input file may
	// have come from the resource system, so we'll need to write it back out to
	// a temporary location first.
	//

	FileName  = ResMan.GetResTempPath( );
	FileName += "NWNScriptCompilerTempScript.ncs";

	f = fopen( FileName.c_str( ), "wb" );

	if (f == NULL)
	{
		TextOut->WriteText(
			"Error: Unable to open script temporary file \"%s\".\n",
			FileName.c_str( ));

		return false;
	}

	if (!InFileContents.empty( ))
	{
		if (fwrite( &InFileContents[ 0 ], InFileContents.size( ), 1, f ) != 1)
		{
			fclose( f );

			TextOut->WriteText(
				"Error: Failed to write to script temporary file \"%s\".\n",
				FileName.c_str( ));

			return false;
		}
	}

	ScriptTempFile = FileName;

	fclose( f );

	f = NULL;

	if (!DbgFileContents.empty( ))
	{
		FileName  = ResMan.GetResTempPath( );
		FileName += "NWNScriptCompilerTempScript.ndb";

		f = fopen( FileName.c_str( ), "wb" );

		if (f == NULL)
		{
			TextOut->WriteText(
				"Error: Unable to open symbols temporary file \"%s\".\n",
				FileName.c_str( ));

			return false;
		}

		if (!DbgFileContents.empty( ))
		{
			if (fwrite( &DbgFileContents[ 0 ], DbgFileContents.size( ), 1, f ) != 1)
			{
				fclose( f );

				TextOut->WriteText(
					"Error: Failed to write to symbols temporary file \"%s\".\n",
					FileName.c_str( ));

				return false;
			}
		}

		fclose( f );

		SymbolsTempFile = FileName;
	}

	f = NULL;

	//
	// Generate unoptimized IR.
	//

	try
	{
		//
		// Create a script reader over the compiled script, and hand it off to an
		// analyzer instance with the analyzer debug output rerouted to the .ir
		// file.
		//

		NWScriptReader ScriptReader( ScriptTempFile.c_str( ) );

		if (!SymbolsTempFile.empty( ))
			ScriptReader.LoadSymbols( SymbolsTempFile.c_str( ) );

		FileName  = OutBaseFile;
		FileName += ".ir";

		f = fopen( FileName.c_str( ), "wt" );

		if (f == NULL)
		{
			TextOut->WriteText(
				"Error: Unable to open IR file \"%s\".\n",
				FileName.c_str( ));

			return false;
		}

		//
		// Perform the analysis and generate the IR.
		//

		WriteFileTextOut CaptureOut( f );
		NWScriptAnalyzer ScriptAnalyzer(
			&CaptureOut,
			(!ActionDefs.empty( )) ? &ActionDefs[ 0 ] : NULL,
			(NWSCRIPT_ACTION) ActionDefs.size( ));

		ScriptAnalyzer.Analyze(
			&ScriptReader,
			NWScriptAnalyzer::AF_NO_OPTIMIZATIONS );

		ScriptAnalyzer.DisplayIR( );

		fclose( f );
		f = NULL;

	}
	catch (NWScriptAnalyzer::script_error &e)
	{
		if (f != NULL)
		{
			fclose( f );

			f = NULL;
		}

		TextOut->WriteText(
			"Error: Analyzer exception '%s' ('%s') at PC=%08X, SP=%08X analyzing script \"%.32s.ncs\".\n",
			e.what( ),
			e.specific( ),
			(unsigned long) e.pc( ),
			(unsigned long) e.stack_index( ),
			InFile.RefStr);

		return false;
	}
	catch (std::exception &e)
	{
		if (f != NULL)
		{
			fclose( f );

			f = NULL;
		}

		TextOut->WriteText(
			"Error: Exception '%s' analyzing script \"%.32s.ncs\".\n",
			e.what( ),
			InFile.RefStr);

		return false;
	}

	try
	{
		//
		// Create a script reader over the compiled script, and hand it off to an
		// analyzer instance with the analyzer debug output rerouted to the .ir
		// file.
		//

		NWScriptReader ScriptReader( ScriptTempFile.c_str( ) );

		if (!SymbolsTempFile.empty( ))
			ScriptReader.LoadSymbols( SymbolsTempFile.c_str( ) );

		FileName  = OutBaseFile;
		FileName += ".ir-opt";

		f = fopen( FileName.c_str( ), "wt" );

		if (f == NULL)
		{
			TextOut->WriteText(
				"Error: Unable to open IR file \"%s\".\n",
				FileName.c_str( ));

			return false;
		}

		//
		// Perform the analysis and generate the IR.
		//

		WriteFileTextOut CaptureOut( f );
		NWScriptAnalyzer ScriptAnalyzer(
			&CaptureOut,
			(!ActionDefs.empty( )) ? &ActionDefs[ 0 ] : NULL,
			(NWSCRIPT_ACTION) ActionDefs.size( ));

		ScriptAnalyzer.Analyze(
			&ScriptReader,
			0 );

		ScriptAnalyzer.DisplayIR( );

		fclose( f );
		f = NULL;
	}
	catch (NWScriptAnalyzer::script_error &e)
	{
		if (f != NULL)
		{
			fclose( f );

			f = NULL;
		}

		TextOut->WriteText(
			"Error: Analyzer exception '%s' ('%s') at PC=%08X, SP=%08X analyzing script \"%.32s.ncs\".\n",
			e.what( ),
			e.specific( ),
			(unsigned long) e.pc( ),
			(unsigned long) e.stack_index( ),
			InFile.RefStr);

		return false;
	}
	catch (std::exception &e)
	{
		if (f != NULL)
		{
			fclose( f );

			f = NULL;
		}

		TextOut->WriteText(
			"Error: Exception '%s' analyzing script \"%.32s.ncs\".\n",
			e.what( ),
			InFile.RefStr);

		return false;
	}

	return true;
}

bool
ProcessInputFile(
	__in ResourceManager & ResMan,
	__in NscCompiler & Compiler,
	__in bool Compile,
	__in int CompilerVersion,
	__in bool Optimize,
	__in bool IgnoreIncludes,
	__in bool SuppressDebugSymbols,
	__in bool Quiet,
	__in bool VerifyCode,
	__in IDebugTextOut * TextOut,
	__in UINT32 CompilerFlags,
	__in const std::string & InFile,
	__in const std::string & OutBaseFile
	)
/*++

Routine Description:

	This routine processes a single input file according to the desired compile
	or diassemble options.

Arguments:

	ResMan - Supplies the resource manager to use to service file load requests.

	NscCompiler - Supplies the compiler context that will be used to process the
	              request.

	Compile - Supplies a Boolean value indicating true if the input file is to
	          be compiled, else false if it is to be disassembled.

	CompilerVersion - Supplies the BioWare-compatible compiler version number.

	Optimize - Supplies a Boolean value indicating true if the script should be
	           optimized.

	IgnoreIncludes - Supplies a Boolean value indicating true if include-only
	                 source files should be ignored.

	SuppressDebugSymbols - Supplies a Boolean value indicating true if debug
	                       symbol generation should be suppressed.

	Quiet - Supplies a Boolean value that indicates true if non-critical
	        messages should be silenced.

	VerifyCode - Supplies a Boolean value that indicates true if generated code
	             is to be verified with the analyzer/verifier if compilation was
	             successful.

	TextOut - Supplies the text out interface used to receive any diagnostics
	          issued.

	CompilerFlags - Supplies compiler control flags.  Legal values are drawn
	                from the NscCompilerFlags enumeration.

	InFile - Supplies the path to the input file.

	OutBaseFile - Supplies the base name (potentially including path) of the
	              output file.  No extension is present.

Return Value:

	The routine returns a Boolean value indicating true on success, else false
	on failure.

	On catastrophic failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	NWN::ResRef32                FileResRef;
	NWN::ResType                 FileResType;
	std::vector< unsigned char > InFileContents;

	//
	// Pull in the input file first.
	//

	if (!LoadInputFile(
		ResMan,
		TextOut,
		InFile,
		FileResRef,
		FileResType,
		InFileContents))
	{
		TextOut->WriteText(
			"Error: Unable to read input file '%s'.\n", InFile.c_str( ) );

		return false;
	}

	//
	// Now execute the main operation.
	//

	if (Compile)
	{
		return CompileSourceFile(
			Compiler,
			CompilerVersion,
			Optimize,
			IgnoreIncludes,
			SuppressDebugSymbols,
			Quiet,
			VerifyCode,
			TextOut,
			CompilerFlags,
			FileResRef,
			InFileContents,
			OutBaseFile);
			
	}
	else
	{
		std::vector< unsigned char > DbgFileContents;
		std::string                  DbgFileName;
		std::string::size_type       Offs;

		DbgFileName = InFile;

		Offs = DbgFileName.find_last_of( '.' );

		if (Offs != std::string::npos)
		{
			NWN::ResRef32 DbgFileResRef;
			NWN::ResType  DbgFileResType;

			DbgFileName.erase( Offs );
			DbgFileName += ".ndb";

			try
			{
				LoadInputFile(
					ResMan,
					TextOut,
					DbgFileName,
					DbgFileResRef,
					DbgFileResType,
					DbgFileContents);
			}
			catch (std::exception)
			{
			}
		}

		return DisassembleScriptFile(
			ResMan,
			Compiler,
			Quiet,
			TextOut,
			FileResRef,
			InFileContents,
			DbgFileContents,
			OutBaseFile);
	}
}

bool
ProcessWildcardInputFile(
	__in ResourceManager & ResMan,
	__in NscCompiler & Compiler,
	__in bool Compile,
	__in int CompilerVersion,
	__in bool Optimize,
	__in bool IgnoreIncludes,
	__in bool SuppressDebugSymbols,
	__in bool Quiet,
	__in bool VerifyCode,
	__in unsigned long Flags,
	__in IDebugTextOut * TextOut,
	__in UINT32 CompilerFlags,
	__in const std::string & InFile,
	__in const std::string & BatchOutDir
	)
/*++

Routine Description:

	This routine processes a wildcard input file according to the desired
	compile or diassemble options.

Arguments:

	ResMan - Supplies the resource manager to use to service file load requests.

	NscCompiler - Supplies the compiler context that will be used to process the
	              request.

	Compile - Supplies a Boolean value indicating true if the input file is to
	          be compiled, else false if it is to be disassembled.

	CompilerVersion - Supplies the BioWare-compatible compiler version number.

	Optimize - Supplies a Boolean value indicating true if the script should be
	           optimized.

	IgnoreIncludes - Supplies a Boolean value indicating true if include-only
	                 source files should be ignored.

	SuppressDebugSymbols - Supplies a Boolean value indicating true if debug
	                       symbol generation should be suppressed.

	Quiet - Supplies a Boolean value that indicates true if non-critical
	        messages should be silenced.

	VerifyCode - Supplies a Boolean value that indicates true if generated code
	             is to be verified with the analyzer/verifier if compilation was
	             successful.

	Flags - Supplies control flags that alter the behavior of the operation.
	        Legal values are drawn from the NSCD_FLAGS enumeration.

	        NscDFlag_StopOnError - Halt processing on first error.

	TextOut - Supplies the text out interface used to receive any diagnostics
	          issued.

	CompilerFlags - Supplies compiler control flags.  Legal values are drawn 
	                from the NscCompilerFlags enumeration.

	InFile - Supplies the path to the input file.  This may end in a wildcard.

	BatchOutDir - Supplies the batch compilation mode output directory.  This
	              may be empty (or else it must end in a path separator).

Return Value:

	The routine returns a Boolean value indicating true on success, else false
	on failure.

	On catastrophic failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	struct _finddata_t     FindData;
	intptr_t               FindHandle;
	char                   Drive[ _MAX_DRIVE ];
	char                   Dir[ _MAX_DIR ];
	char                   FileName[ _MAX_FNAME ];
	char                   Extension[ _MAX_EXT ];
	std::string            WildcardRoot;
	std::string            MatchedFile;
	std::string            OutFile;
	std::string::size_type Offs; 
	bool                   Status;
	bool                   ThisStatus;
	unsigned long          Errors;

	Errors = 0;

	if (_splitpath_s(
		InFile.c_str( ),
		Drive,
		Dir,
		FileName,
		Extension))
	{
		TextOut->WriteText(
			"Error: Malformed input wildcard path \"%s\".\n", InFile.c_str( ));

		return false;
	}

	WildcardRoot  = Drive;
	WildcardRoot += Dir;

	FindHandle = _findfirst( InFile.c_str( ), &FindData );

	if (FindHandle == -1)
	{
		TextOut->WriteText(
			"Error: No matching files for input wildcard path \"%s\".\n",
			InFile.c_str( ));

		return false;
	}

	Status = true;

	//
	// Operate over all files matching the wildcard, performing the requested
	// compile or disassemble operation.
	//

	do
	{
		if (FindData.attrib & _A_SUBDIR)
			continue;

		MatchedFile  = WildcardRoot;
		MatchedFile += FindData.name;

		if (BatchOutDir.empty( ))
		{
			OutFile = MatchedFile;
		}
		else
		{
			OutFile  = BatchOutDir;
			OutFile += FindData.name;
		}

		Offs = OutFile.find_last_of( '.' );

		if (Offs != std::string::npos)
			OutFile.erase( Offs );

		ThisStatus = ProcessInputFile(
			ResMan,
			Compiler,
			Compile,
			CompilerVersion,
			Optimize,
			IgnoreIncludes,
			SuppressDebugSymbols,
			Quiet,
			VerifyCode,
			&g_TextOut,
			CompilerFlags,
			MatchedFile,
			OutFile);

		if (!ThisStatus)
		{
			TextOut->WriteText(
				"Error: Failed to process file \"%s\".\n",
				MatchedFile.c_str( ));

			Status = false;

			Errors += 1;

			if (Flags & NscDFlag_StopOnError)
			{
				TextOut->WriteText("Stopping processing on first error.\n" );
				break;
			}
		}
	} while (!_findnext( FindHandle, &FindData )) ;

	_findclose( FindHandle );

	if (Errors)
		TextOut->WriteText( "%lu error(s); see above for context.\n", Errors );

	return Status;
}

bool
LoadResponseFile(
	__in int argc,
	__in_ecount( argc ) wchar_t * * argv,
	__in const wchar_t * ResponseFileName,
	__out WStringVec & Args,
	__out WStringArgVec & ArgVector
	)
/*++

Routine Description:

	This routine loads command line arguments from a response file.  Each line
	represents an argument.  The contents are read into a vector for later
	processing.

Arguments:

	argc - Supplies the original command line argument count.

	argv - Supplies the original command line argument vector.

	ResponseFileName - Supplies the file name of the response file.

	Args - Received the lines in the response file.

	ArgVector - Receives an array of pointers to the each line in Args.

Return Value:

	The routine returns a Boolean value indicating true if the response file was
	loaded, else false if an error occurred.

Environment:

	User mode.

--*/
{
	FILE * f;

	f = NULL;

	try
	{
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

		if (argc > 0)
			Args.push_back( argv[ 0 ] );

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

		return true;
	}
	catch (std::exception &e)
	{
		if (f != NULL)
		{
			fclose( f );
			f = NULL;
		}

		wprintf(
			L"Error: Exception parsing response file '%s': '%S'.\n",
			ResponseFileName,
			e.what( ));

		return false;
	}
}

int
ExecuteScriptCompilerInternal(
	__in int argc,
	__in_ecount( argc ) wchar_t * * argv
	)
/*++

Routine Description:

	This routine initializes and executes the script compiler.

Arguments:

	argc - Supplies the count of command line arguments.

	argv - Supplies the command line argument array.

Return Value:

	On success, zero is returned; otherwise, a non-zero value is returned.
	On catastrophic failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	std::vector< std::string > SearchPaths;
	std::vector< std::string > InFiles;
	std::string                OutFile;
	std::string                ModuleName;
	std::string                InstallDir;
	std::string                HomeDir;
	std::string                ErrorPrefix;
	std::string                BatchOutDir;
	std::string                CustomModPath;
	WStringVec                 ResponseFileText;
	WStringArgVec              ResponseFileArgs;
	bool                       Compile            = true;
	bool                       Optimize           = false;
	bool                       EnableExtensions   = false;
	bool                       NoDebug            = false;
	bool                       Quiet              = false;
	int                        CompilerVersion    = 999999;
	bool                       Error              = false;
	bool                       LoadResources      = false;
	bool                       Erf16              = false;
	bool                       ResponseFile       = false;
	int                        ReturnCode         = 0;
	bool                       VerifyCode         = false;
	unsigned long              Errors             = 0;
	unsigned long              Flags              = NscDFlag_StopOnError;
	UINT32                     CompilerFlags      = 0;
	ULONG                      StartTime;

	StartTime = GetTickCount( );

	SearchPaths.push_back( "." );

	do
	{
		//
		// Parse arguments out.
		//

		for (int i = 1; i < argc && !Error; i += 1)
		{
			//
			// If it's a switch, consume it.  Otherwise it is an ipnut file.
			//

			if (argv[ i ][ 0 ] == L'-')
			{
				const wchar_t * Switches;
				wchar_t         Switch;

				Switches = &argv[ i ][ 1 ];

				while ((*Switches != L'\0') && (!Error))
				{
					Switch = *Switches++;

					switch (towlower( (wint_t) (unsigned) Switch ))
					{

					case L'1':
						Erf16 = true;
						break;

					case L'a':
						VerifyCode = true;
						break;

					case L'b':
						{
							if (i + 1 >= argc)
							{
								wprintf( L"Error: Malformed arguments.\n" );
								Error = true;
								break;
							}

							if (!swutil::UnicodeToAnsi( argv[ i + 1 ], BatchOutDir ))
							{
								wprintf(
									L"Error: Failed to convert batch output directory '%s' from wchar_t to char.\n",
									argv[ i + 1]);
								Error = true;
								break;
							}

							if (BatchOutDir.empty( ))
								BatchOutDir = ".";

							BatchOutDir.push_back( '/' );

							i += 1;
						}
						break;

					case L'c':
						Compile = true;
						break;

					case L'd':
						Compile = false;
						break;

					case L'e':
						EnableExtensions = true;
						break;

					case L'g':
						NoDebug = true;
						break;

					case L'h':
						{
							if (i + 1 >= argc)
							{
								wprintf( L"Error: Malformed arguments.\n" );
								Error = true;
								break;
							}

							if (!swutil::UnicodeToAnsi( argv[ i + 1 ], HomeDir ))
							{
								wprintf(
									L"Error: Failed to convert home directory '%s' from wchar_t to char.\n",
									argv[ i + 1]);
								Error = true;
								break;
							}

							i += 1;
						}
						break;

					case L'i':
						{
							wchar_t     * Token     = NULL;
							wchar_t     * NextToken = NULL;
							std::string   Ansi;

							if (i + 1 >= argc)
							{
								wprintf( L"Error: Malformed arguments.\n" );
								Error = true;
								break;
							}

							for (Token = wcstok_s( argv[ i + 1 ], L";", &NextToken );
								  Token != NULL;
								  Token = wcstok_s( NULL, L";", &NextToken ))
							{
								if (!swutil::UnicodeToAnsi( Token, Ansi ))
								{
									wprintf(
										L"Error: Failed to convert include path '%s' from wchar_t to char.\n",
										Token);
									Error = true;
									break;
								}

								SearchPaths.push_back( Ansi );
							}

							i += 1;
						}
						break;

					case L'j':
						CompilerFlags |= NscCompilerFlag_ShowIncludes;
						break;

					case L'k':
						CompilerFlags |= NscCompilerFlag_ShowPreprocessed;
						break;

					case L'l':
						LoadResources = true;
						break;

					case L'm':
						{
							LoadResources = true;

							if (i + 1 >= argc)
							{
								wprintf( L"Error: Malformed arguments.\n" );
								Error = true;
								break;
							}

							if (!swutil::UnicodeToAnsi( argv[ i + 1 ], ModuleName ))
							{
								wprintf(
									L"Error: Failed to convert module name '%s' from wchar_t to char.\n",
									argv[ i + 1 ]);
								Error = true;
								break;
							}

							if (ModuleName.empty( ))
							{
								wprintf(
									L"Error: Module resource name must not be empty.\n");
								Error = true;
								break;
							}

							i += 1;
						};
						break;

					case L'n':
						{
							if (i + 1 >= argc)
							{
								wprintf( L"Error: Malformed arguments.\n" );
								Error = true;
								break;
							}

							if (!swutil::UnicodeToAnsi( argv[ i + 1 ], InstallDir ))
							{
								wprintf(
									L"Error: Failed to convert install directory '%s' from wchar_t to char.\n",
									argv[ i + 1]);
								Error = true;
								break;
							}

							if ((!InstallDir.empty( ))          &&
							    (*InstallDir.rbegin( ) != '\\') &&
							    (*InstallDir.rbegin( ) != '/'))
							{
								InstallDir.push_back( '/' );
							}

							i += 1;
						}
						break;

					case L'o':
						Optimize = true;
						break;

					case L'p':
						CompilerFlags |= NscCompilerFlag_DumpPCode;
						break;

					case L'q':
						Quiet = true;
						break;

					case L'r':
						{
							if (i + 1 >= argc)
							{
								wprintf( L"Error: Malformed arguments.\n" );
								Error = true;
								break;
							}

							if (!swutil::UnicodeToAnsi( argv[ i + 1 ], CustomModPath ))
							{
								wprintf(
									L"Failed to convert custom module path '%s' from wchar_t to char.\n",
									argv[ i + 1 ]);
								Error = true;
								break;
							}

							i += 1;
						}
						break;

					case L'v':
						{
							CompilerVersion = 0;

							while (*Switches != L'\0')
							{
								wchar_t Digit = *Switches++;

								if (iswdigit( (wint_t) (unsigned) Digit ))
								{
									CompilerVersion = CompilerVersion * 10 + (Digit - L'0');
								}
								else if (Digit == L'.')
								{
									//
									// Permitted, but ignored.
									//
								}
								else
								{
									wprintf(
										L"Error: Invalid digit in version number.\n" );
									Error = true;
									break;
								}
							}
						}
						break;

					case L'x':
						{
							if (i + 1 >= argc)
							{
								wprintf( L"Error: Malformed arguments.\n" );
								Error = true;
								break;
							}

							if (!swutil::UnicodeToAnsi( argv[ i + 1 ], ErrorPrefix ))
							{
								wprintf(
									L"Failed to convert error prefix '%s' from wchar_t to char.\n",
									argv[ i + 1 ]);
								Error = true;
								break;
							}

							i += 1;
						}
						break;

					case 'y':
						Flags &= ~(NscDFlag_StopOnError);
						break;

					default:
						{
							wprintf( L"Error: Unrecognized option \"%c\".\n", Switch );
							Error = true;
						}
						break;

					}
				}
			}
			else if (argv[ i ][ 0 ] == L'@')
			{
				if (ResponseFile)
				{
					wprintf( L"Error: Nested response files are unsupported.\n" );
					Error = true;
					break;
				}

				if (!LoadResponseFile(
					argc,
					argv,
					&argv[ i ][ 1 ],
					ResponseFileText,
					ResponseFileArgs))
				{
					Error = true;
					break;
				}

				ResponseFile = true;
			}
			else
			{
				std::string Ansi;

				if (!swutil::UnicodeToAnsi( argv[ i ] , Ansi ))
				{
					wprintf(
						L"Error: Failed to convert filename '%s' from wchar_t to char.\n",
						argv[ i ]);
					Error = true;
					break;
				}

				//
				// If we're running in batch mode, all filenames just go onto the
				// input file list.
				//

				if (!BatchOutDir.empty( ))
				{
					InFiles.push_back( Ansi );
					continue;
				}

				if (InFiles.empty( ))
				{
					InFiles.push_back( Ansi );
				}
				else if (OutFile.empty( ))
				{
					OutFile = Ansi;
				}
				else
				{
					wprintf( L"Error: Too many file arguments.\n" );
					Error = true;
					break;
				}
			}
		}

		if (ResponseFile)
		{
			//
			// If we have no response file data, then stop parsing.  The first
			// element is a duplicate of argv[ 0 ].
			//

			if (ResponseFileArgs.size( ) < 2)
				break;

			//
			// If we just finished parsing the response file arguments, then we
			// are done.
			// 

			if (argv[ 0 ] == ResponseFileArgs[ 0 ])
				break;

			argc = (int) ResponseFileArgs.size( );

			if (argc < 1)
				break;

			argv = (wchar_t * *) &ResponseFileArgs[ 0 ];
		}
		else
		{
			break;
		}
	} while (!Error) ;


	if (!Quiet)
	{
		wprintf(
			L"NWNScriptCompiler - built %S %S\n"
			NWN2DEV_COPYRIGHT_STR_L L".\n"
			L"Portions copyright (C) 2002-2003, Edward T. Smith.\n"
			L"Portions copyright (C) 2003, The Open Knights Consortium.\n",
			__DATE__,
			__TIME__);
	}

	if ((Error) || (InFiles.empty( )))
	{
		wprintf(
			L"Usage:\n"
			L"NWNScriptCompiler [-1acdegjkloq] [-b batchoutdir] [-h homedir]\n"
			L"                  [[-i pathspec] ...] [-m resref] [-n installdir]\n"
			L"                  [-r modpath] [-v#] [-x errprefix] [-y]\n"
			L"                  infile [outfile|infiles]\n"
			L"  batchoutdir - Supplies the location at which batch mode places\n"
			L"                output files and enables multiple input filenames.\n"
			L"  homedir - Per-user NWN2 home directory (i.e. Documents\\NWN2).\n"
			L"  pathspec - Semicolon separated list of directories to search for\n"
			L"             additional includes.\n"
			L"  resref - Resource name of module to load (without extension).\n"
			L"           Note that loading a module is potentially slow.\n"
			L"  installdir - Per-machine NWN2 install directory.\n"
			L"  modpath - Supplies the full path to the .mod (or directory) that\n"
			L"            contains the module.ifo for the module to load.  This\n"
			L"            option overrides the [-r resref] option.\n"
			L"  errprefix - Prefix string to prepend to compiler errors (replacing\n"
			L"              the default of \"Error\").\n"
			L"  -1 - Assume NWN1-style module and KEY/BIF resources instead of\n"
			L"       NWN2-style module and ZIP resources.\n"
			L"  -a - Analyze generated code and verify that it is consistent\n"
			L"       (increases compilation time).\n"
			L"  -c - Compile the script (default, overrides -d).\n"
			L"  -d - Disassemble the script (overrides -c).\n"
			L"  -e - Enable non-BioWare extensions.\n"
			L"  -g - Suppress generation of .ndb debug symbols file.\n"
			L"  -j - Show where include file are being sourced from.\n"
			L"  -k - Show preprocessed source text to console output.\n"
			L"  -l - Load base game resources even if -m isn't supplied (slow),\n"
			L"       so that \"in-box\" standard includes can be resolved.\n"
			L"  -o - Optimize the compiled script.\n"
			L"  -p - Dump internal PCode for compiled script contributions.\n"
			L"  -q - Silence most messages.\n"
			L"  -vx.xx - Set the version of the compiler.\n"
			L"  -y - Continue processing input files even on error.\n"
			);

		return -1;
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
		g_TextOut.WriteText(
			"Failed to initialize resource manager: '%s'\n",
			e.what( ) );

		if (g_Log != NULL)
		{
			fclose( g_Log );
			g_Log = NULL;
		}

		return 0;
	}

	if (LoadResources)
	{
		//
		// If we're to load module resources, then do so now.
		//

		if (!Quiet)
		{
			if (ModuleName.empty( ))
			{
				g_TextOut.WriteText(
					"Loading base game resources...\n");
			}
			else
			{
				g_TextOut.WriteText(
					"Loading resources for module '%s'...\n",
					ModuleName.c_str( ));
			}
		}

		if (InstallDir.empty( ))
		{
			if (!Erf16)
				InstallDir = GetNwn2InstallPath( );
			else
				InstallDir = GetNwn1InstallPath( );
		}

		if (HomeDir.empty( ))
			HomeDir = GetNwn2HomePath( );

		LoadModule(
			*g_ResMan,
			ModuleName,
			HomeDir,
			InstallDir,
			Erf16,
			CustomModPath);
	}

	//
	// Now create the script compiler context.
	//

	NscCompiler Compiler( *g_ResMan, EnableExtensions );

	if (!SearchPaths.empty( ))
		Compiler.NscSetIncludePaths( SearchPaths );

	if (!ErrorPrefix.empty( ))
		Compiler.NscSetCompilerErrorPrefix( ErrorPrefix.c_str( ) );

	Compiler.NscSetResourceCacheEnabled( true );

	//
	// Install the ctrl-c handler.
	//

	SetConsoleCtrlHandler( AppConsoleCtrlHandler, TRUE );

	//
	// Process each of the input files in turn.
	//

	for (std::vector< std::string >::const_iterator it = InFiles.begin( );
	    it != InFiles.end( );
	    ++it)
	{
		std::string            ThisOutFile;
		std::string::size_type Offs;
		bool                   Status;

		//
		// Load the source text and compile the program.
		//

		if (it->find_first_of( "*?" ) != std::string::npos)
		{
			//
			// We've a wildcard, process it appropriately.
			//

			Status = ProcessWildcardInputFile(
				*g_ResMan,
				Compiler,
				Compile,
				CompilerVersion,
				Optimize,
				true,
				NoDebug,
				Quiet,
				VerifyCode,
				Flags,
				&g_TextOut,
				CompilerFlags,
				*it,
				BatchOutDir);
		}
		else
		{
			if (BatchOutDir.empty( ))
			{
				ThisOutFile = OutFile;

				if (ThisOutFile.empty( ))
					ThisOutFile = *it;

				Offs = ThisOutFile.find_last_of( '.' );

				if (Offs != std::string::npos)
					ThisOutFile.erase( Offs );
			}
			else
			{
				char FileName[ _MAX_FNAME ];

				if (_splitpath_s(
					it->c_str( ),
					NULL,
					0,
					NULL,
					0,
					FileName,
					_MAX_FNAME,
					NULL,
					0))
				{
					g_TextOut.WriteText(
						"Error: Invalid path: \"%s\".\n",
						it->c_str( ));

					ReturnCode = -1;
					continue;
				}
					
				ThisOutFile  = BatchOutDir;
				ThisOutFile += FileName;
			}

			//
			// We've a regular (single) file name, process it.
			//

			Status = ProcessInputFile(
				*g_ResMan,
				Compiler,
				Compile,
				CompilerVersion,
				Optimize,
				true,
				NoDebug,
				Quiet,
				VerifyCode,
				&g_TextOut,
				CompilerFlags,
				*it,
				ThisOutFile);
		}

		if (!Status)
		{
			ReturnCode = -1;

			Errors += 1;

			if (Flags & NscDFlag_StopOnError)
			{
				g_TextOut.WriteText( "Processing aborted.\n" );
				break;
			}
		}
	}

	if (!Quiet)
	{
		g_TextOut.WriteText(
			"Total Execution time = %lums\n",
			GetTickCount( ) - StartTime);
	}

	if (Errors > 1)
		g_TextOut.WriteText( "%lu error(s) processing input files.\n", Errors );

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

	delete g_ResMan;
	g_ResMan = NULL;

	return ReturnCode;
}

int
ExecuteScriptCompiler(
	__in int argc,
	__in_ecount( argc ) wchar_t * * argv
	)
/*++

Routine Description:

	This routine initializes and executes the script compiler.

Arguments:

	argc - Supplies the count of command line arguments.

	argv - Supplies the command line argument array.

Return Value:

	On success, zero is returned; otherwise, a non-zero value is returned.

Environment:

	User mode.

--*/
{
	try
	{
		return ExecuteScriptCompilerInternal( argc, argv );
	}
	catch (std::exception &e)
	{
		if (g_ResMan != NULL)
		{
			delete g_ResMan;
			g_ResMan = NULL;
		}

		wprintf(
			L"Error: Exception '%S' executing compiler (fatal).\n",
			e.what( ) );

		return -1;
	}
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

	The routine returns the program exit code, which is zero on success, and
	non-zero on failure.

Environment:

	User mode.

--*/
{
	//
	// Launch the script compiler application.
	//

	return ExecuteScriptCompiler( argc, argv );
}

