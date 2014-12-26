/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    Precomp.h

Abstract:

    This module acts as the precompiled header that pulls in all common system,
	SkywingUtils, and NWNConnLib definitions that are used by other modules.

--*/

#ifndef _PROGRAMS_NWNSCRIPTCONSOLE_PRECOMP_H
#define _PROGRAMS_NWNSCRIPTCONSOLE_PRECOMP_H

#ifdef _MSC_VER
#pragma once
#endif

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE_GLOBALS
#define _STRSAFE_NO_DEPRECATE

#include <winsock2.h>
#include <windows.h>
#include <windowsx.h>
#undef GetFirstChild
#include <shlobj.h>
#include <shellapi.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include <functional>
#include <queue>
#include <tchar.h>
#include <strsafe.h>
#include <hash_map>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <assert.h>
#include <cmath>
#include <hash_set>
#include <stack>

#include "../ProjectGlobal/ProjGlobalDefs.h"
#include "../ProjectGlobal/VersionConstants.h"
#include "../SkywingUtils/SkywingUtils.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#include "../NWN2DataLib/NWN2DataLib.h"

#endif
