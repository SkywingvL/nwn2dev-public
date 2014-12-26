/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ListModuleAreas.cpp

Abstract:

	This module houses a sample program that lists information about each area
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
ShowDoorInformation(
	__in const GffFileReader::GffStruct * DoorStruct,
	__in ResourceManager & ResMan,
	__in IDebugTextOut * TextOut
	)
/*++

Routine Description:

	This routine prints information about a door to the text output console.

Arguments:

	DoorStruct - Supplies the GFF instance template struct describing the door
	             object todisplay.

	ResMan - Supplies a reference to the resource manager instance to use in
	         order to load any associated resource data.

	TextOut - Supplies the text output interface.

Return Value:

	None.  An std::exception is raised on failure.

Environment:

	User mode.

--*/
{
	NWN::Vector3 Position;
	std::string  Description;
	std::string  Name;
	std::string  Tag;
	std::string  LinkedTo;

	UNREFERENCED_PARAMETER( ResMan );

	if (!DoorStruct->GetFLOAT( "X", Position.x ))
		throw std::runtime_error( "Failed to read Door.X" );
	if (!DoorStruct->GetFLOAT( "Y", Position.y ))
		throw std::runtime_error( "Failed to read Door.Y" );
	if (!DoorStruct->GetFLOAT( "Z", Position.z ))
		throw std::runtime_error( "Failed to read Door.Z" );
	if (!DoorStruct->GetCExoLocString( "Description", Description ))
		Description.clear( ); // Description is an optional field.
	if (!DoorStruct->GetCExoLocString( "LocName", Name ))
		throw std::runtime_error( "Failed to read Door.LocName" );
	if (!DoorStruct->GetCExoString( "Tag", Tag ))
		throw std::runtime_error( "Failed to read Door.Tag" );
	if (!DoorStruct->GetCExoString( "LinkedTo", LinkedTo ))
		throw std::runtime_error( "Failed to read Door.LinkedTo" );

	TextOut->WriteText(
		"Door %s @ (%g, %g, %g): Tag '%s', LinkedTo '%s', Description: %s\n",
		Name.c_str( ),
		Position.x,
		Position.y,
		Position.z,
		Tag.c_str( ),
		LinkedTo.c_str( ),
		Description.c_str( ));
}

void
ShowAreaInformation(
	__in const NWN::ResRef32 & AreaResRef,
	__in ResourceManager & ResMan,
	__in IDebugTextOut * TextOut
	)
/*++

Routine Description:

	This routine prints information about an area to the text output console.

Arguments:

	AreaResRef - Supplies the resource name of the area to display.

	ResMan - Supplies a reference to the resource manager instance to use in
	         order to load any associated resource data.

	TextOut - Supplies the text output interface.

Return Value:

	None.  An std::exception is raised on failure.

Environment:

	User mode.

--*/
{
	//
	// Areas are comprised of two files, an <area>.are with area parameters,
	// and an <area>.git with the object instance parameters about objects that
	// have been placed in the area via the toolset.
	//

	DemandResource32                 AreFile( ResMan, AreaResRef, NWN::ResARE );
	DemandResource32                 GitFile( ResMan, AreaResRef, NWN::ResGIT );
	GffFileReader                    Are( AreFile, ResMan );
	GffFileReader                    Git( GitFile, ResMan );
	const GffFileReader::GffStruct * RootStruct;
	std::string                      AreaName;
	std::string                      AreaTag;

	RootStruct = Are.GetRootStruct( );

	//
	// Acquire parameters we need from area.git.
	//

	if (!RootStruct->GetCExoLocString( "Name", AreaName ))
		throw std::runtime_error( "Failed to read area Name" );
	if (!RootStruct->GetCExoString( "Tag", AreaTag ))
		throw std::runtime_error( "Failed to read area Tag" );
	
	TextOut->WriteText(
		"Instance information for area %s (tag %s):\n", 
		AreaName.c_str( ),
		AreaTag.c_str( ));

	//
	// Now show instance information about various objects in the area.
	//

	RootStruct = Git.GetRootStruct( );

	for (size_t i = 0; i <= ULONG_MAX; i += 1)
	{
		GffFileReader::GffStruct DoorStruct;

		if (!RootStruct->GetListElement( "Door List", i, DoorStruct ))
			break;

		ShowDoorInformation( &DoorStruct, ResMan, TextOut );	
	}
}

int
__cdecl
main(
	__in int argc,
	__in_ecount( argc ) const char * * argv
	)
/*++

Routine Description:

	This routine is the entry point symbol for the module area lister
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
	const char * ModuleName;
	const char * NWN2Home;
	const char * InstallDir;

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
		// First, load up the module.  We do not load anything in the HAKs right
		// now.  If we wanted to load the HAKs, we would have to first load the
		// module without HAKs, then parse the HAK list to discover what HAKs to
		// load, and then finally re-load the module with the HAK list.  The
		// LoadModuleResourcesLite function speeds this process by avoiding a
		// full load of all install directory based resources if we just wanted
		// to fetch the HAK list out of module.ifo.
		//

		ResMan.LoadModuleResources(
			ModuleName,
			"",
			NWN2Home,
			InstallDir,
			std::vector< NWN::ResRef32 >( )
			);

		//
		// Acquire a file name for module.ifo and load it up using the GFF
		// reader library.
		//

		DemandResourceStr                ModuleIfoFile( ResMan, "module", NWN::ResIFO );
		GffFileReader                    ModuleIfo( ModuleIfoFile, ResMan );
		const GffFileReader::GffStruct * RootStruct = ModuleIfo.GetRootStruct( );
		std::string                      ModName;
		GffFileReader::GffStruct         Struct;

		if (RootStruct->GetCExoLocString( "Mod_Name", ModName ))
			TextOut.WriteText( "The module name is: %s.\n", ModName.c_str( ) );

		//
		// Now look at each area.
		//

		for (size_t i = 0; i <= ULONG_MAX; i += 1)
		{
			NWN::ResRef32 AreaResRef;

			if (!RootStruct->GetListElement( "Mod_Area_list", i, Struct ))
				break;

			if (!Struct.GetResRef( "Area_Name", AreaResRef ))
				throw std::runtime_error( "Mod_Area_list element is missing Area_Name." );

			//
			// Show information about this area.
			//

			ShowAreaInformation( AreaResRef, ResMan, &TextOut );
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
