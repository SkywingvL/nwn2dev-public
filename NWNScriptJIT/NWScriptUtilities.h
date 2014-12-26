/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptUtilities.h

Abstract:

	This module defines utility routine for the NWScript code generator and
	NWScript runtime environment.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTJIT_NWSCRIPTUTILITIES_H
#define _SOURCE_PROGRAMS_NWNSCRIPTJIT_NWSCRIPTUTILITIES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace NWScript
{

public ref class NWScriptUtilities
{

public:

	//
	// Convert a System::String into an std::string.
	//

	static
	std::string
	ConvertString(
		__in String ^ Str
		);

	//
	// Get an encoding object that maps to/from UTF-8 while using QUESTION MARK
	// as the replacement character.
	//

	static
	property
	System::Text::Encoding ^
	NWUTF8Encoding
	{
		System::Text::Encoding ^
		get(
			)
		{
			if (UTF8FallbackEncoding == nullptr)
				SetupEncoding( );

			return UTF8FallbackEncoding;
		}
	}

	//
	// Get an encoding object that truncates to 8-bit ordinal values.  This is
	// designed for use with JIT'd scripts.
	//

	static
	property
	System::Text::Encoding ^
	NW8BitEncoding
	{
		System::Text::Encoding ^
		get
		(
			)
		{
			if (TruncateTo8BitEncoding == nullptr)
				SetupEncoding( );

			return TruncateTo8BitEncoding;
		}
	}

private:

	static
	void
	SetupEncoding(
		);

	static System::Text::Encoding ^ UTF8FallbackEncoding;
	static System::Text::Encoding ^ TruncateTo8BitEncoding;

};

//
// Push an engine structure onto an INWScriptStack.
//

void
VMStackPushEngineStructureInternal(
	__in INWScriptStack * Stack,
	__in EngineStructurePtr * EngStruct
	);


}

#endif
