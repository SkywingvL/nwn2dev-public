/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptAnalyzerTypes.h

Abstract:

	This module define base types used by the analyzer system.  These types are
	placed in namespace NWNScriptLib.

	N.B.  This module is used only by the analyzer subsystem.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTANALYZERTYPES_H
#define _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTANALYZERTYPES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace NWNScriptLib
{
	typedef NWScriptStack::STACK_POINTER   STACK_POINTER;
	typedef NWScriptStack::PROGRAM_COUNTER PROGRAM_COUNTER;

	typedef unsigned long                  SCOPE_ID;

	static const PROGRAM_COUNTER INVALID_PC     = (PROGRAM_COUNTER) -1;
	static const STACK_POINTER   INVALID_SP     = (STACK_POINTER) -1;
	static const SCOPE_ID        INVALID_SCOPE  = (SCOPE_ID) -1;
	static const STACK_POINTER   CELL_SIZE      = 4;
	static const STACK_POINTER   CELL_UNALIGNED = (CELL_SIZE - 1);

	typedef std::stack< PROGRAM_COUNTER >        PCStack;
	typedef std::vector< PROGRAM_COUNTER >       PCVec;
	typedef stdext::hash_set< PROGRAM_COUNTER >  PCSet;

	typedef std::pair< PROGRAM_COUNTER, STACK_POINTER > PCSP;
	typedef std::stack< PCSP >                          PCSPStack;
	typedef std::vector< PCSP >                         PCSPVec;

	//
	// Define the representation of a parameter type list.
	//

	typedef std::vector< NWACTION_TYPE > ParameterList;

	//
	// Define the representation of a return type list.
	//

	typedef std::vector< NWACTION_TYPE > ReturnTypeList;


	class NWScriptSubroutine;
	typedef swutil::SharedPtr< NWScriptSubroutine > SubroutinePtr;
	typedef std::vector< SubroutinePtr > SubroutinePtrVec;

	class NWScriptControlFlow;
	typedef swutil::SharedPtr< NWScriptControlFlow > ControlFlowPtr;
}

#endif
