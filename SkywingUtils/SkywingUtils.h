#ifndef _SOURCE_PROGRAMS_SKYWINGUTILS_SKYWINGUTILS_H
#define _SOURCE_PROGRAMS_SKYWINGUTILS_SKYWINGUTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Precomp.h"

#define BEGIN_NS_SWUTIL() namespace swutil {
#define END_NS_SWUTIL() }

#include "Ref/Ref.h"
#include "Synchronization/ListAPI.h"
#include "Parsers/BufferParser.h"
#include "Encoding/Encoding.h"
#include "Timer/TimerManager.h"

#undef BEGIN_NS_SWUTIL
#undef END_NS_SWUTIL

#endif


