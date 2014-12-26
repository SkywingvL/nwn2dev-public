/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	Log.cpp

Abstract:

	This module defines logging support for the NWScript VM plugin.

--*/

#include "Precomp.h"

extern FILE * g_Log;

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
	}

	inline
	~PrintfTextOut(
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
		UNREFERENCED_PARAMETER( Attributes );
		char buf[8193];
		StringCbVPrintfA(buf, sizeof( buf ), fmt, argptr);

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

};

PrintfTextOut   g_PrintfTextOut;
IDebugTextOut * g_TextOut = &g_PrintfTextOut;

