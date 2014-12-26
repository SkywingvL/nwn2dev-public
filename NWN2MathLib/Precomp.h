/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    precomp.h

Abstract:

    This module acts as the precompiled header that pulls in all common
	dependencies that typically do not change.

--*/

#ifndef _PROGRAMS_NWN2MATHLIB_PRECOMP_H
#define _PROGRAMS_NWN2MATHLIB_PRECOMP_H

#ifdef _MSC_VER
#pragma once
#endif

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE_GLOBALS
#define STRSAFE_NO_DEPRECATE

#include <math.h>
#include <limits.h>
#include <float.h>

#include <list>
#include <vector>

#ifdef ENCRYPT
#include <protect.h>
#endif

#include "../SkywingUtils/SkywingUtils.h"
#include "../NWNBaseLib/NWNBaseLib.h"

#define ARGUMENT_PRESENT( x )  ( (x) )

#define PI                 3.14159265f
#define DEGREES_TO_RADIANS 0.0174532925f

#endif
