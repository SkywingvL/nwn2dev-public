/**********************************************************************
*<
FILE: DataFormat.h

DESCRIPTION:	Data File Format

CREATED BY: tazpn (Theo)

HISTORY:

*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/

#ifndef __DATAFORMAT_H__
#define __DATAFORMAT_H__

#pragma once

#define DllExport
//#define ASSERT(x) { if (!(x) && IsDebuggerPresent()) __asm { int 3 }; }
#include "point2.h"
#include "point3.h"
#include "point4.h"
#include "color.h"
#include "quat.h"
#include "matrix3.h"
//#include "tab.h"
#include "strclass.h"

#ifndef _countof
#  define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif

#endif
