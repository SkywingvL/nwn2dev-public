/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptInternal.h

Abstract:

	This module defines internal constants and types used within the script
	execution environment.  These internal constants are not intended to be
	used by external callers.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTINTERNAL_H
#define _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTINTERNAL_H

#ifdef _MSC_VER
#pragma once
#endif

//
// Define script constant OBJECTIDs only for use with the constant load
// instruction.  These are replaced with the actual values for the current
// script situation at execution time.
//

const NWN::OBJECTID OBJECTID_SELF    = 0;
const NWN::OBJECTID OBJECTID_INVALID = 1;

//
// Define the highest supported engine structure.
//
// We could go up to 126 internally (127 is reserved for return addresses), but
// opcode limitations force us down to 15 (unary types) and then 9 (binary
// types) usable engine structures.
//

enum { LAST_ENGINE_STRUCTURE = 9 };

//
// Define operand types.
//

enum
{
	TYPE_UNARY_FIRST                = 0x00,
	TYPE_UNARY_NONE                 = 0x00,
	TYPE_UNARY_STACKOP              = 0x01,
	TYPE_UNARY_RESERVED2            = 0x02,

	TYPE_UNARY_INT                  = 0x03,
	TYPE_UNARY_FLOAT                = 0x04,
	TYPE_UNARY_STRING               = 0x05,
	TYPE_UNARY_OBJECTID             = 0x06,
	TYPE_UNARY_ENGINE_FIRST         = 0x10,
	TYPE_UNARY_ENGINE_LAST          = 0x19,

	TYPE_BINARY_FIRST               = 0x20,
	TYPE_BINARY_INTINT              = 0x20,
	TYPE_BINARY_FLOATFLOAT          = 0x21,
	TYPE_BINARY_OBJECTIDOBJECTID    = 0x22,
	TYPE_BINARY_STRINGSTRING        = 0x23,
	TYPE_BINARY_STRUCTSTRUCT        = 0x24,
	TYPE_BINARY_INTFLOAT            = 0x25,
	TYPE_BINARY_FLOATINT            = 0x26,
	TYPE_BINARY_ENGINE_FIRST        = 0x30,
	TYPE_BINARY_ENGINE_LAST         = 0x39,
	TYPE_BINARY_VECTORVECTOR        = 0x3A,
	TYPE_BINARY_VECTORFLOAT         = 0x3B,
	TYPE_BINARY_FLOATVECTOR         = 0x3C
};

//
// Define instruction types.
//

enum
{
	OP_CPDOWNSP             = 0x01,
	OP_RSADD                = 0x02,
	OP_CPTOPSP              = 0x03,
	OP_CONST                = 0x04,
	OP_ACTION               = 0x05,
	OP_LOGAND               = 0x06,
	OP_LOGOR                = 0x07,
	OP_INCOR                = 0x08,
	OP_EXCOR                = 0x09,
	OP_BOOLAND              = 0x0A,
	OP_EQUAL                = 0x0B,
	OP_NEQUAL               = 0x0C,
	OP_GEQ                  = 0x0D,
	OP_GT                   = 0x0E,
	OP_LT                   = 0x0F,
	OP_LEQ                  = 0x10,
	OP_SHLEFT               = 0x11,
	OP_SHRIGHT              = 0x12,
	OP_USHRIGHT             = 0x13,
	OP_ADD                  = 0x14,
	OP_SUB                  = 0x15,
	OP_MUL                  = 0x16,
	OP_DIV                  = 0x17,
	OP_MOD                  = 0x18,
	OP_NEG                  = 0x19,
	OP_COMP                 = 0x1A,
	OP_MOVSP                = 0x1B,
	OP_STORE_STATEALL       = 0x1C,
	OP_JMP                  = 0x1D,
	OP_JSR                  = 0x1E,
	OP_JZ                   = 0x1F,
	OP_RETN                 = 0x20,
	OP_DESTRUCT             = 0x21,
	OP_NOT                  = 0x22,
	OP_DECISP               = 0x23,
	OP_INCISP               = 0x24,
	OP_JNZ                  = 0x25,
	OP_CPDOWNBP             = 0x26,
	OP_CPTOPBP              = 0x27,
	OP_DECIBP               = 0x28,
	OP_INCIBP               = 0x29,
	OP_SAVEBP               = 0x2A,
	OP_RESTOREBP            = 0x2B,
	OP_STORE_STATE          = 0x2C,
	OP_NOP                  = 0x2D,
	MAX_NWSCRIPT_OPCODE     = 0x2D,
	OP_T                    = 0x42,

	LAST_OP
};

#endif
