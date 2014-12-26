/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptInterfaces.h

Abstract:

	This module defines interfaces that are used to interact with the core
	script VM.  A user of the script VM implements these interfaces in order to
	provide core functionality in the form of action APIs exposed to script
	code.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTINTERFACES_H
#define _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTINTERFACES_H

#ifdef _MSC_VER
#pragma once
#endif

typedef ULONG NWSCRIPT_ACTION;

class NWScriptVM;
class NWScriptStack;

//
// Define to include names in the NWScript action service table.  This allows
// script backends (such as the JIT backend) to include additional symbolic
// debugging information at the expensive of additional increases to the size
// of the binary.
//

#define NWACTION_DEF_INCLUDE_NAME 1

//
// Define to include prototypes in the action service table descirptors.
//

// #define NWACTION_DEF_INCLUDE_PROTOTYPE 1

//
// Define the base types that can be passed to an action routine.
//

typedef enum _NWACTION_TYPE
{
	ACTIONTYPE_VOID,
	ACTIONTYPE_INT,
	ACTIONTYPE_FLOAT,
	ACTIONTYPE_STRING,
	ACTIONTYPE_OBJECT,
	ACTIONTYPE_VECTOR,
	ACTIONTYPE_ACTION,
	ACTIONTYPE_EFFECT,
	ACTIONTYPE_EVENT,
	ACTIONTYPE_LOCATION,
	ACTIONTYPE_TALENT,
	ACTIONTYPE_ITEMPROPERTY,

	ACTIONTYPE_ENGINE_0 = ACTIONTYPE_EFFECT,
	ACTIONTYPE_ENGINE_1,
	ACTIONTYPE_ENGINE_2,
	ACTIONTYPE_ENGINE_3,
	ACTIONTYPE_ENGINE_4,
	ACTIONTYPE_ENGINE_5,
	ACTIONTYPE_ENGINE_6,
	ACTIONTYPE_ENGINE_7,
	ACTIONTYPE_ENGINE_8,
	ACTIONTYPE_ENGINE_9,

	LASTACTIONTYPE
} NWACTION_TYPE, * PNWACTION_TYPE;

typedef const enum _NWACTION_TYPE * PCNWACTION_TYPE;

//
// Define the action routine table layout.
//

typedef struct _NWACTION_DEFINITION
{
#if NWACTION_DEF_INCLUDE_NAME
	const char          * Name;
#endif
#if NWACTION_DEF_INCLUDE_PROTOTYPE
	const char          * Prototype;
#endif
	NWSCRIPT_ACTION       ActionId;
	unsigned long         MinParameters;
	unsigned long         NumParameters;
	NWACTION_TYPE         ReturnType;
	PCNWACTION_TYPE       ParameterTypes;
//	const unsigned long * TotalParameterSizes;
} NWACTION_DEFINITION, * PNWACTION_DEFINITION;

typedef const struct _NWACTION_DEFINITION * PCNWACTION_DEFINITION;

#ifdef _MSC_VER
#define NWSCRIPTACTAPI __stdcall
#endif



#include "NWScriptStack.h"

//
// Define parameters for a fast script action call (OnExecuteActionFromJITFast)
// that does not use the VM stack.
//

typedef enum _NWFASTACTION_CMD
{
	//
	// Push an int on the VM stack (int).
	//

	NWFASTACTION_PUSHINT,

	//
	// Pop an int from the VM stack (int *).
	//

	NWFASTACTION_POPINT,

	//
	// Push a float on the VM stack (float).
	//

	NWFASTACTION_PUSHFLOAT,

	//
	// Pop a float from the VM stack (float *).
	//

	NWFASTACTION_POPFLOAT,

	//
	// Push an object id on the VM stack (NWN::OBJECTID).
	//

	NWFASTACTION_PUSHOBJECTID,

	//
	// Pop an object id from the VM stack (NWN::OBJECTID *)
	//

	NWFASTACTION_POPOBJECTID,

	//
	// Push a string on the VM stack (INWScriptStack::NeutralString *).
	//

	NWFASTACTION_PUSHSTRING,

	//
	// Pop a string from the VM stack (INWScriptStack::NeutralString *).
	//

	NWFASTACTION_POPSTRING,

	//
	// Call the action service (none).
	//

	NWFASTACTION_CALL,

	LASTNWFACTACTION
} NWFASTACTION_CMD, * PNWFASTACTION_CMD;

typedef const enum _NWFASTACTION_CMD * PCNWFASTACTION_CMD;


class INWScriptActions
{

public:

	//
	// This callback routine is invoked when an action is executed.  The
	// implementation is responsible for performing whatever actions are
	// required for the given action call number, which may involve retrieving
	// parameters from the VM stack or pushing a return value on to the VM
	// stack.
	//
	// The action routine is permitted to throw an std::exception on a fatal
	// error condition, which may terminate the entire script chain.  It is
	// also permissible for the action routine to cause a re-entrant call into
	// the script VM (though the script VM may reject the call if the recursion
	// limit has been reached).
	//

	virtual
	void
	NWSCRIPTACTAPI
	OnExecuteAction(
		__in NWScriptVM & ScriptVM,
		__in NWScriptStack & VMStack,
		__in NWSCRIPT_ACTION ActionId,
		__in size_t NumArguments
		) = 0;

	//
	// This callback routine creates an empty engine structure of the given
	// engine type number.
	//

	virtual
	EngineStructurePtr
	NWSCRIPTACTAPI
	CreateEngineStructure(
		__in NWScriptStack::ENGINE_STRUCTURE_NUMBER EngineType
		) = 0;

	//
	// This callback routine is invoked when an action is executed.  The
	// implementation is responsible for performing whatever actions are
	// required for the given action call number, which may involve retrieving
	// parameters from the VM stack or pushing a return value on to the VM
	// stack.
	//
	// The action routine is permitted to return false on a fatal error
	// error condition, which may terminate the entire script chain.  It is
	// also permissible for the action routine to cause a re-entrant call into
	// the script VM (though the script VM may reject the call if the recursion
	// limit has been reached).
	//
	// N.B.  This routine is invoked by the JIT environment where there is no
	//       VM object in play.
	//

	virtual
	bool
	NWSCRIPTACTAPI
	OnExecuteActionFromJIT(
		__in NWSCRIPT_ACTION ActionId,
		__in size_t NumArguments
		) = 0;

	//
	// This callback routine is invoked when an action is executed.  The
	// implementation is responsible for performing whatever actions are
	// required for the given action call number.  Unlike the standard action
	// service handler callback, parameters and return values are not passed on
	// the VM stack.
	//
	// The action routine is permitted to return false on a fatal error
	// error condition, which may terminate the entire script chain.  It is
	// also permissible for the action routine to cause a re-entrant call into
	// the script VM (though the script VM may reject the call if the recursion
	// limit has been reached).
	//
	// N.B.  This routine is invoked by the JIT environment where there is no
	//       VM object in play.
	//
	// N.B.  Presently, engine structures cannot appear in the parameter or
	//       return value list for a fast call.
	//

	virtual
	bool
	NWSCRIPTACTAPI
	OnExecuteActionFromJITFast(
		__in NWSCRIPT_ACTION ActionId,
		__in size_t NumArguments,
		__in_ecount( NumCmds ) PCNWFASTACTION_CMD Cmds,
		__in size_t NumCmds,
		__in uintptr_t * CmdParams
		) = 0;


};

struct INWScriptActions_Vtbl
{
	void * OnExecuteAction;
	void * CreateEngineStructure;
	void * OnExecuteActionFromJIT;
	void * OnExecuteActionFromJITFast;
};

struct INWScriptActions_Raw
{
	INWScriptActions_Vtbl * Vtbl;
};

#endif
