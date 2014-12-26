/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptManagedSupport.h

Abstract:

	This module defines the representation for supporting data for managed
	language "scripts".

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTJIT_NWSCRIPTMANAGEDSUPPORT_H
#define _SOURCE_PROGRAMS_NWNSCRIPTJIT_NWSCRIPTMANAGEDSUPPORT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace NWScript
{

ref class NWScriptManagedSupport
{

public:

	//
	// Construct a managed support object.
	//

	NWScriptManagedSupport(
		__in const NWScriptAnalyzer * Analyzer,
		__in_opt IDebugTextOut * TextOut,
		__in ULONG DebugLevel,
		__in INWScriptActions * ActionHandler,
		__in NWN::OBJECTID ObjectInvalid,
		__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams
		);

	//
	// Destruct a NWManagedSupport instance.
	//

	inline
	~NWScriptManagedSupport(
		)
	{
		this->!NWScriptManagedSupport( );
	}

	!NWScriptManagedSupport(
		);

	//
	// Return the associated assembly.
	//

	inline
	Assembly ^
	GetAssembly(
		)
	{
		return m_Assembly;
	}

	//
	// Return the engine structure type array.
	//

	inline
	array< Type ^ > ^
	GetEngineStructureTypes(
		)
	{
		return m_EngineStructureTypes;
	}

private:

	//
	// Emit an error diagnostic in response to an exception.
	//

	void
	ErrorException(
		__in Exception ^ Excpt,
		__in_opt IDebugTextOut * TextOut,
		__in ULONG DebugLevel
		);

	//
	// Define the associated generated interface layer assembly.
	//

	Assembly                 ^ m_Assembly;

	//
	// Define the engine structure type array.
	//

	array< Type ^ >          ^ m_EngineStructureTypes;

};

}

#endif

