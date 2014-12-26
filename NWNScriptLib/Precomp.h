/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	Precomp.h

Abstract:

	This module acts as the precompiled header that pulls in all common systems
	and dependencies of the NWNScriptLib module (internal only).

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTLIB_PRECOMP_H
#define _SOURCE_PROGRAMS_NWNSCRIPTLIB_PRECOMP_H

#ifdef _MSC_VER
#pragma once
#endif

#define STRSAFE_NO_DEPRECATE

#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <math.h>
#include <float.h>
#include <stddef.h>
#include <string>
#include <vector>
#include <map>
#include <hash_map>
#include <hash_set>
#include <set>
#include <list>
#include <stack>
#include <functional>

#include "../SkywingUtils/SkywingUtils.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "../NWN2DataLib/TextOut.h"
#include "../NWN2DataLib/NWScriptReader.h"

#endif

