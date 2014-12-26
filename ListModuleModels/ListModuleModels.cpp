/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ListModuleModels.cpp

Abstract:

	This module houses a sample program that lists information about each model
	in a module.

--*/

#include "Precomp.h"
#include "../NWN2DataLib/TextOut.h"
#include "../NWN2DataLib/ResourceManager.h"

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
		AllocConsole( );
	}

	inline
	~PrintfTextOut(
		)
	{
		FreeConsole( );
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
		SetConsoleTextAttribute( console, Attributes );
		WriteConsoleA(console, buf, n, &n, 0);
	}

};

void
LoadModule(
	__in ResourceManager & ResMan,
	__in const char * ModuleName,
	__in const char * NWN2Home,
	__in const char * InstallDir
	)
/*++

Routine Description:

	This routine performs a full load of a module, including the TLK file and
	any dependent HAKs.

Arguments:

	ResMan - Supplies the ResourceManager instance that is to load the module.

	ModuleName - Supplies the resource name of the module to load.

	NWN2Home - Supplies the users NWN2 home directory (i.e. NWN2 Documents dir).

	InstallDir - Supplies the game installation directory.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{

	std::vector< NWN::ResRef32 > HAKList;
	std::string                  CustomTlk;

	//
	// Load up the module.  First, we load just the core module resources, then
	// we determine the HAK list and load all of the HAKs up too.
	//

	ResMan.LoadModuleResourcesLite(
		ModuleName,
		NWN2Home,
		InstallDir);

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

	//
	// Now perform a full load with the HAK list and CustomTlk available.
	//
	// N.B.  The DemandResourceStr above must go out of scope before we issue a
	//       new load, as it references a temporary file that will be cleaned up
	//       by the new load request.
	//

	ResMan.LoadModuleResources(
		ModuleName,
		CustomTlk,
		NWN2Home,
		InstallDir,
		HAKList
		);
}

int
__cdecl
main(
	__in int argc,
	__in_ecount( argc ) const char * * argv
	)
/*++

Routine Description:

	This routine is the entry point symbol for the module model lister
	program.

Arguments:

	argc - Supplies the count of command line arguments.

	argv - Supplies the command line argument vector.

Return Value:

	The routine returns the process exit code.

Environment:

	User mode.

--*/
{
	const char               * ModuleName;
	const char               * NWN2Home;
	const char               * InstallDir;
	std::set< std::string >    ModuleModels;

	//
	// First, check that we've got the necessary arguments.
	//

	if (argc < 4)
	{
		wprintf(
			L"Usage: %S <module> <nwn2 home directory> <nwn2 install directory>\n",
			argv[ 0 ] );

		return 0;
	}

	ModuleName = argv[ 1 ];
	NWN2Home   = argv[ 2 ];
	InstallDir = argv[ 3 ];

	//
	// Now spin up a resource manager instance.
	//

	PrintfTextOut   TextOut;
	ResourceManager ResMan( &TextOut );

	try
	{
		//
		// Load the module up all the way, including HAKs.
		//

		LoadModule(
			ResMan,
			ModuleName,
			NWN2Home,
			InstallDir);

		//
		// Now look at each MDB.
		//

		for (ResourceManager::FileId Id = ResMan.GetEncapsulatedFileCount( );
		     Id != 0;
		     Id -= 1)
		{
			NWN::ResRef32 ResRef;
			NWN::ResType  ResType;

			if (!ResMan.GetEncapsulatedFileEntry( (Id - 1), ResRef, ResType ))
				continue;

			//
			// Record this MDB uniquely.  We may have multiple references for the
			// same RESREF if the model is overridden at some level of the
			// resource hierarchy, i.e. if the model was patched in a later game
			// patch zip file.
			//
			// The first reference we see is the most precedent one, but looking
			// up by the ResRef and ResType instead of the FileId will always
			// retrieve the most precedent file.
			//
			// N.B.  The resref we receive is guaranteed to be lowercased if a
			//       well-formed data file set is provided, thus it's not
			//       necessary to separately lowercase it here.
			//

			ModuleModels.insert( ResMan.StrFromResRef( ResRef ) );
		}

		//
		// Now print out a list of all unique model RESREFS.  Each of these listed
		// models has a ResType of NWN::ResMDB.
		//

		for (std::set< std::string >::const_iterator it = ModuleModels.begin( );
		    it != ModuleModels.end( );
		    ++it)
		{
			TextOut.WriteText( "%s\n", it->c_str( ) );
		}
	}
	catch (std::exception &e)
	{
		//
		// Simple print an error message and abort if we went wrong, such as if
		// we couldn't load the module.
		//

		TextOut.WriteText( "ERROR: Exception '%s'.\n", e.what( ) );
	}

	//
	// All done.
	//

	return 0;
}
