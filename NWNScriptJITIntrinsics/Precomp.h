/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	Precomp.h

Abstract:

	This module acts as the precompiled header that pulls in all common systems
	and dependencies of the NWNScriptJITIntrinsics module (internal only).

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTJITINTRINSICS_PRECOMP_H
#define _SOURCE_PROGRAMS_NWNSCRIPTJITINTRINSICS_PRECOMP_H

#ifdef _MSC_VER
#pragma once
#endif

#define STRSAFE_NO_DEPRECATE

#if 0

#include <tchar.h>
#include <strsafe.h>
#include <winsock2.h>
#include <windows.h>
#include <math.h>
#include <float.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <hash_map>
#include <hash_set>
#include <functional>
#include <stack>
#include <set>
#include <msclr\marshal_cppstd.h>

#include "../SkywingUtils/SkywingUtils.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "../NWN2DataLib/TextOut.h"
#include "../NWN2DataLib/NWScriptReader.h"
#include "../NWNScriptLib/NWScriptInterfaces.h"
#include "../NWNScriptLib/NWScriptVM.h"
#include "../NWNScriptLib/NWScriptAnalyzer.h"

#endif

#include <specstrings.h>

#include "../ProjectGlobal/ProjGlobalDefs.h"
#include "../ProjectGlobal/VersionConstants.h"

using namespace System;

#endif

