/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ServerNWScript.h

Abstract:

	This module defines the externally visible interface for the NWScript VM
	plugin.

--*/

#ifndef _SOURCE_PROGRAMS_AURORASERVERNWSCRIPT_SERVERNWSCRIPT_H
#define _SOURCE_PROGRAMS_AURORASERVERNWSCRIPT_SERVERNWSCRIPT_H

#ifdef _MSC_VER
#pragma once
#endif

#define NWNXAPI __thiscall

namespace NWN2Server
{
	class CExoString;
	class CVirtualMachine;
}

class NWScriptBridge;
class NWScriptRuntime;

class ServerNWScriptPlugin : public NWNX4PluginBase,
                             public INWScriptJITPolicy
{

public:

	inline
	ServerNWScriptPlugin(
		)
	: NWNX4PluginBase(
		"NWSCRIPTVM",
		"MSIL/.NET JIT",
		"1.0.0.0",
		"This plugin accelerates NWScript execution by compiling scripts to MSIL"
		),
	  m_Log( NULL ),
	  m_TextOut( NULL ),
	  m_Enabled( false ),
	  m_Bridge( NULL ),
	  m_Runtime( NULL ),
	  m_PatchedCmdImplementerVtable( NULL ),
	  m_OrigCmdImplementerVtable( NULL ),
	  m_DebugLevel( NWScriptVM::EDL_Errors ),
	  m_UseReferenceVM( false ),
	  m_MinFreeMemoryToJIT( 256 * 1024 * 1024 ),
	  m_MinScriptSizeToJIT( 0 ),
	  m_MaxLoopIterations( 0 ),
	  m_MaxCallDepth( 0 ),
	  m_OptimizeIR( false ),
	  m_LoadDebugSymbols( true ),
	  m_AllowManagedScripts( false ),
	  m_DisableExecutionGuards( false ),
	  m_OptimizeActionServiceHandlers( true )
	{
		m_sPlugin = this;
	}

	virtual
	NWNXAPI
	~ServerNWScriptPlugin(
		)
	{
		m_sPlugin = NULL;

		if (m_Log != NULL)
		{
			//
			// Intentionally don't try and close it, as we will be running at
			// DllMain time and the process is shutting down anyway.
			//

//			fclose( m_Log );
			m_Log = NULL;
		}

		//
		// Intentionally don't try and delete it as we're in process rundown.
		//

		// delete m_Bridge;
		// delete m_Runtime;
	}

	//
	// Initialize the plugin.
	// 

	virtual
	bool
	NWNXAPI
	Init(
		__in const char * NWNXHome
		);

	inline
	virtual
	int
	NWNXAPI
	GetInt(
		__in const char * Function,
		__in const char * Param1,
		__in int Param2
		);

	//
	// Return a pointer to the active plugin instance, if any exists.
	//

	static
	inline
	ServerNWScriptPlugin *
	GetPlugin(
		)
	{
		return m_sPlugin;
	}

	//
	// Called when the server requests to execute a script.  Its purpose is to
	// run the script as appropriate.
	//

	bool
	ExecuteScriptForServer(
		__inout int * PC,
		__inout_ecount( CodeSize ) unsigned char * InstructionStream,
		__in int CodeSize,
		__in NWN2Server::CVirtualMachine * ServerVM
		);

	//
	// Called to save the current saved state to the VM stack.
	//

	unsigned long
	SaveStateToServerVMStack(
		__in NWN2Server::CVirtualMachine * ServerVM
		);

	//
	// Called to remove the saved state placed on the stack by the last call to
	// SaveStateToServerVMStack.
	//

	void
	RemoveSavedStateFromServerVMStack(
		__in NWN2Server::CVirtualMachine * ServerVM,
		__in unsigned long StackElements
		);

	//
	// Choose which engine should be used to run a script (for the first time
	// that the script is run).
	//

	virtual
	NWSCRIPT_ENGINE
	SelectEngineForScript(
		__in size_t ScriptCodeSize
		);

	//
	// Determine whether IR optimizations should be enabled.
	//

	virtual
	bool
	GetEnableIROptimizations(
		);

	//
	// Determine whether debug symbols should be loaded.
	//

	virtual
	bool
	GetLoadDebugSymbols(
		);

	//
	// Return the code generation output directory (for assembly DLLs), else
	// NULL if the code generation output should not be saved.
	//

	virtual
	const wchar_t *
	GetCodeGenOutputDir(
		);

	//
	// Return true if managed scripts are permitted.
	//

	virtual
	bool
	GetAllowManagedScripts(
		);

	//
	// Return the maximum loop iterations to permit for a script.
	//

	virtual
	int
	GetMaxLoopIterations(
		);

	//
	// Return the maximum call stack depth to permit for a script.
	//

	virtual
	int
	GetMaxCallDepth(
		);

	//
	// Return true if execution guards for scripts should be disabled.
	//

	virtual
	bool
	GetDisableExecutionGuards(
		);

	//
	// Return true if action service handlers should be optimized.
	//

	virtual
	bool
	GetOptimizeActionServiceHandlers(
		);

private:

	bool
	ApplyPatches(
		);

	bool
	EstablishBridge(
		);

	bool
	EstablishRuntime(
		__in const char * NWNXHome
		);

	void
	PatchCmdImplementer(
		__in NWN2Server::CVirtualMachine * ServerVM
		);

	static
	void
	__stdcall
	NullRunScriptCallback(
		__in const NWN2Server::CExoString & ScriptName
		);

	void
	LoadSettings(
		__in const char * NWNXHome
		);

	FILE                        * m_Log;
	IDebugTextOut               * m_TextOut;
	bool                          m_Enabled;
	static ServerNWScriptPlugin * m_sPlugin;
	NWScriptBridge              * m_Bridge;
	NWScriptRuntime             * m_Runtime;
	void                      * * m_PatchedCmdImplementerVtable;
	void                        * m_OrigCmdImplementerVtable;
	std::wstring                  m_IniPath;
	std::wstring                  m_CodeGenOutputDirectory;
	NWScriptVM::ExecDebugLevel    m_DebugLevel;
	bool                          m_UseReferenceVM;
	ULONG                         m_MinFreeMemoryToJIT;
	ULONG                         m_MinScriptSizeToJIT;
	int                           m_MaxLoopIterations;
	int                           m_MaxCallDepth;
	bool                          m_OptimizeIR;
	bool                          m_LoadDebugSymbols;
	bool                          m_AllowManagedScripts;
	bool                          m_DisableExecutionGuards;
	bool                          m_OptimizeActionServiceHandlers;

};

#endif

