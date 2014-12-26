/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	Precomp.h

Abstract:

	This module acts as the precompiled header that pulls in all common systems
	and dependencies of the NWNBaseLib module (internal only).

--*/

#ifndef _SOURCE_PROGRAMS_NWNBASELIB_PRECOMP_H
#define _SOURCE_PROGRAMS_NWNBASELIB_PRECOMP_H

#ifdef _MSC_VER
#pragma once
#endif

#define STRSAFE_NO_DEPRECATE

#include <winsock2.h>
#include <windows.h>
#include <string>
#include <vector>

#include "../ProjectGlobal/ProjGlobalDefs.h"
#include "../zlib/zlib.h"

#endif

