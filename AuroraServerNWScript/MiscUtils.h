/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	MiscUtils.h

Abstract:

	This module defines various general utility routines.

--*/

#ifndef _SOURCE_PROGRAMS_AURORASERVERNWSCRIPT_MISCUTILS_H
#define _SOURCE_PROGRAMS_AURORASERVERNWSCRIPT_MISCUTILS_H

#ifdef _MSC_VER
#pragma once
#endif

//
// Return a rough estimate of the amount of available VA space within the
// calling process.  The estimate may change at any time and is not a guarantee
// of an ability to allocate any amount of memory via any allocator.
//

ULONGLONG
GetAvailableVASpace(
	);

#endif

