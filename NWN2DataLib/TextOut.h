/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	TextOut.h

Abstract:

	This module houses the IDebugTextOut interface, which is used by
	the resource manager and datafile accessor library for purposes of writing
	debug log messages.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_TEXTOUT_H
#define _PROGRAMS_NWN2DATALIB_TEXTOUT_H

#ifdef _MSC_VER
#pragma once
#endif

//
// Define the debug text output interface, used to write debug or log messages
// to the user.
//

struct IDebugTextOut
{

	virtual
	void
	WriteText(
		__in __format_string const char* fmt,
		...
		) = 0;

	virtual
	void
	WriteText(
		__in WORD Attributes,
		__in __format_string const char* fmt,
		...
		) = 0;

	virtual
	void
	WriteTextV(
		__in __format_string const char* fmt,
		__in va_list ap
		) = 0;

	virtual
	void
	WriteTextV(
		__in WORD Attributes,
		__in __format_string const char* fmt,
		__in va_list ap
		) = 0;
};

#endif
