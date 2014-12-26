/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    precomp.h

Abstract:

    This module acts as the precompiled header that pulls in all common
	dependencies that typically do not change.

--*/

#ifndef _PROGRAMS_GRANNY2LIB_PRECOMP_H
#define _PROGRAMS_GRANNY2LIB_PRECOMP_H

#ifdef _MSC_VER
#pragma once
#endif

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE_GLOBALS
#define STRSAFE_NO_DEPRECATE

#include <winsock2.h>
#include <windows.h>
#include <math.h>
#include <limits.h>
#include <float.h>

#include <list>
#include <vector>
#include <map>
#include <hash_map>

#ifdef ENCRYPT
#include <protect.h>
#endif

#include <mbctype.h>
#include <io.h>

#include <tchar.h>
#include <strsafe.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <stdexcept>
#include <stdio.h>
#include <stdarg.h>


#include <iostream>
#include <stdexcept>
#include <sstream>
#include <strstream>

#include "../ProjectGlobal/ProjGlobalDefs.h"
#include "../SkywingUtils/SkywingUtils.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "../NWN2MathLib/NWN2MathLib.h"

#endif
