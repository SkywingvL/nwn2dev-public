/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    Granny2Reader.h

Abstract:

    This module defines the granny2 (*.gr2) frontend interface.  External users
	of the library access gr2 data via the interfaces defined in this module.

--*/

#ifndef _PROGRAMS_GRANNY2LIB_GRANNY2READER_H
#define _PROGRAMS_GRANNY2LIB_GRANNY2READER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "GR2Format.h"

namespace granny2
{

#ifdef _M_IX86
#define GRANNY2_USECONVEXE 0
#else
#define GRANNY2_USECONVEXE 1
#endif

//
// Define the Granny2Reader object.  Typically, an external user creates a
// single Granny2Reader object that is used to access any number of *.gr2 files
// thereafter.
//

class Granny2Reader
{

public:

	Granny2Reader(
		__in const std::string & Granny2Dll
		);

	virtual
	~Granny2Reader(
		);

	//
	// Load a .gr2 file and return a pointer to the GR2Info instance, which is
	// to be deleted via operator delete.
	//

	granny2::granny2_impl::GR2Info *
	LoadGranny2File(
		__in const std::string & FileName,
		__in const std::string & TempFileName,
		__in bool PreferOutOfProcess = false
		);

private:

	bool
	Gr2Conv_GrannyConvertFileToRaw(
		__in const char * SourceFile,
		__in const char * DestinationFile
		);

	typedef
	bool
	(__stdcall * GrannyConvertFileToRawProc)(
		__in const char * SourceFile,
		__in const char * DestinationFile
		);

	wchar_t                    m_Gr2ConvExe[ MAX_PATH + 1 ];
	std::string                m_Granny2Dll;
#if !GRANNY2_USECONVEXE
	HMODULE                    m_Granny2Lib;
	GrannyConvertFileToRawProc m_GrannyConvertFileToRaw;
#endif

};

}


#endif
