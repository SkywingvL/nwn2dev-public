/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptJITPolicy.h

Abstract:

	This module defines policy controls that are used to specify when the
	NWScript runtime environment will choose to JIT code versus execute code in
	the reference VM.

--*/

#ifndef _SOURCE_PROGRAMS_AURORASERVERNWSCRIPT_NWSCRIPTJITPOLICY_H
#define _SOURCE_PROGRAMS_AURORASERVERNWSCRIPT_NWSCRIPTJITPOLICY_H

#ifdef _MSC_VER
#pragma once
#endif

class INWScriptJITPolicy
{

public:

	enum NWSCRIPT_ENGINE
	{
		NWSCRIPT_ENGINE_VM,
		NWSCRIPT_ENGINE_JIT,

		LAST_NWSCRIPT_ENGINE
	};

	//
	// Choose which engine should be used to run a script (for the first time
	// that the script is run).
	//

	virtual
	NWSCRIPT_ENGINE
	SelectEngineForScript(
		__in size_t ScriptCodeSize
		) = 0;

	//
	// Determine whether IR optimizations should be enabled.
	//

	virtual
	bool
	GetEnableIROptimizations(
		) = 0;

	//
	// Determine whether debug symbols should be loaded.
	//

	virtual
	bool
	GetLoadDebugSymbols(
		) = 0;

	//
	// Return the code generation output directory (for assembly DLLs), else
	// NULL if the code generation output should not be saved.
	//

	virtual
	const wchar_t *
	GetCodeGenOutputDir(
		) = 0;

	//
	// Return true if managed scripts are permitted.
	//

	virtual
	bool
	GetAllowManagedScripts(
		) = 0;

	//
	// Return the maximum loop iterations to permit for a script.
	//

	virtual
	int
	GetMaxLoopIterations(
		) = 0;

	//
	// Return the maximum call stack depth to permit for a script.
	//

	virtual
	int
	GetMaxCallDepth(
		) = 0;

	//
	// Return true if execution guards for scripts should be disabled.
	//

	virtual
	bool
	GetDisableExecutionGuards(
		) = 0;

	//
	// Return true if action service handlers should be optimized.
	//

	virtual
	bool
	GetOptimizeActionServiceHandlers(
		) = 0;

};

#endif

