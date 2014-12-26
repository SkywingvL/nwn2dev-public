#pragma once
#ifndef _STDINT
#define _STDINT
#ifndef RC_INVOKED
#include <yvals.h>

typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef signed char int_least8_t;
typedef short int_least16_t;
typedef int int_least32_t;

typedef unsigned char uint_least8_t;
typedef unsigned short uint_least16_t;
typedef unsigned int uint_least32_t;

typedef char int_fast8_t;
typedef int int_fast16_t;
typedef int int_fast32_t;

typedef unsigned char uint_fast8_t;
typedef unsigned int uint_fast16_t;
typedef unsigned int uint_fast32_t;

#ifndef _INTPTR_T_DEFINED
 #define _INTPTR_T_DEFINED
 #ifdef _WIN64
typedef __int64 intptr_t;
 #else /* _WIN64 */
typedef _W64 int intptr_t;
 #endif /* _WIN64 */
#endif /* _INTPTR_T_DEFINED */

#ifndef _UINTPTR_T_DEFINED
 #define _UINTPTR_T_DEFINED
 #ifdef _WIN64
typedef unsigned __int64 uintptr_t;
 #else /* _WIN64 */
typedef _W64 unsigned int uintptr_t;
 #endif /* _WIN64 */
#endif /* _UINTPTR_T_DEFINED */

typedef _Longlong int64_t;
typedef _ULonglong uint64_t;

typedef _Longlong int_least64_t;
typedef _ULonglong uint_least64_t;

typedef _Longlong int_fast64_t;
typedef _ULonglong uint_fast64_t;

typedef _Longlong intmax_t;
typedef _ULonglong uintmax_t;

#endif // RC_INVOKED
#endif // _STDINT
