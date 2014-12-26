/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ModelRenderer.cpp

Abstract:

	This module houses the simple model renderer test program.

--*/

#include "Precomp.h"
#include "../NWN2DataLib/TextOut.h"
#include "../NWN2DataLib/ResourceManager.h"
#include "WorldObject.h"
#include "WorldView.h"

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
InitObjects(
	__in WorldView & View
	)
/*++

Routine Description:

	This routine creates objects to display in the world.

Arguments:

	View - Supplies the world view instance.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	std::vector< std::string > MDBResRefs;
	WorldObject::Ptr           WorldObj;
	NWN::Vector3               v;

	MDBResRefs.push_back( "P_HHF_NK_Body01" );
	MDBResRefs.push_back( "P_HHF_Hair01" );
	MDBResRefs.push_back( "P_HHF_Head01" );
	MDBResRefs.push_back( "P_HHF_NK_Gloves01" );
	MDBResRefs.push_back( "P_HHF_CL_Boots01" );

	WorldObj = View.CreateWorldObject( MDBResRefs, "P_HHF_skel" );

	//
	// Place it at ground level, at the center of the "area" (100x100).
	//

	v.x = 50.0f;
	v.y = 50.0f;
	v.z = 0.0f;

	WorldObj->SetPosition( v );

	//
	// Scale it up a bit.
	//

	v.x = 5.0f;
	v.y = 5.0f;
	v.z = 5.0f;

	WorldObj->SetScale( v );
}

int
CALLBACK
WinMain(
	__in HINSTANCE hInstance,
	__in HINSTANCE hPrevInstance,
	__in LPSTR lpCmdLine,
	__in int nShowCmd
	)
/*++

Routine Description:

	This routine is the entry point symbol for the client extension launcher.

Arguments:

	hInstance - Supplies the exe module handle.

	hPrevInstance - Ignored.

	lpCmdLine - Supplies the command line.

	nShowCmd - Supplies the requested show command for the main window.

Return Value:

	The routine returns the process exit code.

Environment:

	User mode.

--*/
{
	std::string InstallDir = "C:\\Program Files (x86)\\Neverwinter Nights 2\\";
	std::string Module     = "Vordan's Hero Creator";
	std::string NWN2Home;

	UNREFERENCED_PARAMETER( nShowCmd );
	UNREFERENCED_PARAMETER( lpCmdLine );
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( hInstance );

	if (getenv( "USERPROFILE" ) == NULL)
		return 0;

	NWN2Home  = getenv( "USERPROFILE" );
	NWN2Home += "\\Documents\\Neverwinter Nights 2\\";

	if (getenv( "NWN2INSTALLDIR" ) != NULL)
		InstallDir = getenv( "NWN2INSTALLDIR" );

	if (getenv( "NWN2MODULE" ) != NULL)
		Module = getenv( "NWN2MODULE" );

	if (!WorldView::RegisterClasses( true ))
		return 0;

	PrintfTextOut   TextOut;
	ResourceManager ResMan( &TextOut );

	try
	{
		WorldView View( ResMan, &TextOut, NULL );
		bool      Quitting;

		ResMan.LoadModuleResources(
			Module,
			"",
			NWN2Home,
			InstallDir,
			std::vector< NWN::ResRef32 >( )
			);

		InitObjects( View );

		//
		// Enter into the standard dispatch loop.
		//

		Quitting = false;

		View.Show( true );

		while (!Quitting)
		{
			unsigned long Timeout;
			MSG           msg;
			DWORD         Status;

			Timeout = INFINITE;

			Status = MsgWaitForMultipleObjects(
				0,
				NULL,
				FALSE,
				Timeout,
				QS_ALLINPUT);

			//
			// Dispatch messages if we woke up with any available.
			//

			if (Status == WAIT_OBJECT_0)
			{
				while (PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ))
				{
					TranslateMessage( &msg );
					DispatchMessage( &msg );

					if (msg.message == WM_QUIT)
					{
						Quitting = true;
						break;
					}
				}
			}
		}
	}
	catch (std::exception &e)
	{
		MessageBoxA(
			NULL,
			e.what( ),
			"Exception!",
			MB_ICONERROR);
	}

	WorldView::RegisterClasses( false );

	return 0;
}
