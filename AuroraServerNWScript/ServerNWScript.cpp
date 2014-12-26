/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ServerNWScript.cpp

Abstract:

	This module defines the main entry point for the NWScript VM plugin.

--*/

#include "Precomp.h"
#include "NWNX4Plugin.h"
#include "NWScriptJITPolicy.h"
#include "ServerNWScript.h"
#include "Offsets.h"
#include "NWN2Def.h"
#include "NWScriptBridge.h"
#include "NWScriptRuntime.h"
#include "hdlcommon.h"
#include "MiscUtils.h"

ServerNWScriptPlugin   g_Plugin;
extern IDebugTextOut * g_TextOut;
FILE                 * g_Log;

ServerNWScriptPlugin * ServerNWScriptPlugin::m_sPlugin = NULL;

void                 * NWN2_Heap_Deallocate = NULL;

using namespace NWN2Server;

BOOL
WINAPI
DllMain(
	__in HINSTANCE hinstDLL,
	__in DWORD fdwReason,
	__in LPVOID lpvReserved
	)
/*++

Routine Description:

	This routine acts as the DLL entry point.  It simply turns off TLS callouts
	for process attach and returns TRUE.

Arguments:

	hinstDLL - Supplies the module instance handle.

	fdwReason - Supplies the reason code for the callout.

	lpvReserved - For process attach and detach, non-NULL for static attach,
	              else NULL for dynamic attach.

Return Value:

	The routine always returns TRUE to allow the module to load.

Environment:

	User mode.

--*/
{
	UNREFERENCED_PARAMETER( lpvReserved );

	switch (fdwReason)
	{

	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls( hinstDLL );
		break;

	}

	return TRUE;
}

NWNX4PluginBase *
WINAPI
GetPluginPointerV2(
	)
/*++

Routine Description:

	This routine returns a pointer to the NWNX4 plugin instance that is
	implemented by the module.

Arguments:

	None.

Return Value:

	A pointer the plugin object is returned on success, else NULL is reutrned.

Environment:

	User mode.

--*/
{
	return &g_Plugin;
}

ULONGLONG
GetAvailableVASpace(
	)
/*++

Routine Description:

	This routine estimates the available VA space within the process.  The
	returned counter may change asynchronously and is not a guarantee that the
	heap or any other allocator will actually be able to service a memory
	request.

Arguments:

	None.

Return Value:

	A basic estimate of the remaining free VA space in the process is returned.

Environment:

	User mode.

--*/
{
	MEMORYSTATUSEX MemoryStatus;

	MemoryStatus.dwLength = sizeof( MEMORYSTATUSEX );

	if (!GlobalMemoryStatusEx( &MemoryStatus ))
		return 0xFFFFFFFFFFFFFFFFUI64;

	return MemoryStatus.ullAvailVirtual;
}

bool
NWNXAPI
ServerNWScriptPlugin::Init(
	__in const char * NWNXHome
	)
/*++

Routine Description:

	This routine is invoked by NWNX when the plugin has been loaded into the
	game server process.  Its purpose is to initialize plugin functionality.

	Once Init returns successfully, the plugin may begin servicing requests.

Arguments:

	NWNXHome - Supplies a pointer to the NWNX4 Home directory path.

Return Value:

	The routine returns true on success, else false on failure.

Environment:

	User mode.

--*/
{
	FILE      * Log;
	char        LogFileName[ MAX_PATH ];
	HINSTANCE   NWN2_MemoryMgr;

	StringCbPrintfA(
		LogFileName,
		sizeof( LogFileName ),
		"%s\\AuroraServerNWScript.log",
		NWNXHome);

	Log = fopen( LogFileName, "at ");

	m_Log     = Log;
	g_Log     = Log;
	m_TextOut = g_TextOut;

	m_TextOut->WriteText(
		"ServerNWScript plugin - built " __DATE__ " " __TIME__ "\n"
		"This release is paired with game build %lu.\n",
		NWN2_BUILD);

	__try
	{
		if (*(PULONG) OFFS_VM_ExecuteCode != CHECK_VM_ExecuteCode)
		{
			m_TextOut->WriteText(
				"Wrong NWN2Server version, (%08X != %08X), not enabling plugin.\n",
				*(PULONG) OFFS_VM_ExecuteCode,
				CHECK_VM_ExecuteCode);
			
			return true;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		m_TextOut->WriteText(
			"Wrong NWN2Server version (exception), not enabling plugin.\n"
			);

		return true;
	}

	m_TextOut->WriteText( "Plugin starting up.\n" );

	if ((NWN2_MemoryMgr = GetModuleHandle( L"NWN2_MemoryMgr_amdxp.dll" )) == NULL)
	{
		m_TextOut->WriteText( "Unable to locate NWN2_MemoryMgr_amdxp.dll.\n" );

		return true;
	}

	LoadSettings( NWNXHome );

	NWN2_Heap_Deallocate = GetProcAddress(
		NWN2_MemoryMgr,
		"?Deallocate@NWN2_Heap@@SAXPAX@Z"
		);

	if (NWN2_Heap_Deallocate == NULL)
	{
		m_TextOut->WriteText( "Unable to locate NWN2_MemoryMgr_amdxp!NWN2_Heap::Deallocate.\n" );

		return true;
	}

	if (!EstablishBridge( ))
	{
		m_TextOut->WriteText( "Failed to create NWScriptBridge.\n" );
	
		return true;
	}

	if (!EstablishRuntime( NWNXHome ))
	{
		m_TextOut->WriteText( "Failed to establish NWScriptRuntime.\n" );

		return true;
	}

	if (!ApplyPatches( ))
	{
		m_TextOut->WriteText( "Failed to apply patches.\n" );

		return true;
	}

	m_Enabled = true;

	return true;
}

int
NWNXAPI
ServerNWScriptPlugin::GetInt(
	__in const char * Function,
	__in const char * Param1,
	__in int Param2
	)
/*++

Routine Description:

	This routine is invoked by NWNX when a GetInt request is made.  Its purpose
	is to interface with the script environment.

Arguments:

	Function - Supplies the plugin-defined function string.

	Param1 - Supplies the plugin-defined first function parameter.

	Param2 - Supplies the plugin-defined second function parameter.

Return Value:

	The routine returns an integer value that is returned from NWNX4GetInt.

Environment:

	User mode, called from NWNX4GetInt.

--*/
{
	if (!strcmp( Function, "LOG SCRIPT STATISTICS" ))
	{
		m_Runtime->DumpStatistics( );
		return 0;
	}
	else if (!strcmp( Function, "SET REFERENCE VM" ))
	{
		m_UseReferenceVM = (Param2 != 0) ? true : false;
		return 0;
	}
	else if (!strcmp( Function, "CLEAR SCRIPT CACHE" ))
	{
		m_Runtime->ClearScriptCache( );
		return 0;
	}
	else if (!strcmp( Function, "GET AVAILABLE VA SPACE" ))
	{
		return (int) GetAvailableVASpace( );
	}
	else if (!strcmp( Function, "SET DEBUG LEVEL" ))
	{
		m_DebugLevel = (NWScriptVM::ExecDebugLevel) Param2;
		m_Runtime->SetDebugLevel( m_DebugLevel );
		m_Bridge->SetDebugLevel( m_DebugLevel );
	}
	else if (!strcmp( Function, "RELOAD CONFIGURATION" ))
	{
		LoadSettings( "" );
	}

	return NWNX4PluginBase::GetInt( Function, Param1, Param2 );
}

bool
ServerNWScriptPlugin::ApplyPatches(
	)
/*++

Routine Description:

	This routine activates the code patches used by the NWScript VM plugin.

Arguments:

	None.

Return Value:

	The routine returns true on success, else false on failure.

Environment:

	User mode.

--*/
{
	int i = 0;

	extern Patch * patches;

	while( patches[ i ].Apply( ) )
		i++;

	return true;
}

bool
ServerNWScriptPlugin::EstablishBridge(
	)
/*++

Routine Description:

	This routine establishes the bridge between NWNScriptLib and NWN2Server's
	CVirtualMachine.

Arguments:

	None.

Return Value:

	The routine returns true on success, else false on failure.

Environment:

	User mode.

--*/
{
	try
	{
		m_Bridge = new NWScriptBridge( m_TextOut, m_DebugLevel );
	}
	catch (std::exception)
	{
		return false;
	}

	return true;
}

bool
ServerNWScriptPlugin::EstablishRuntime(
	__in const char * NWNXHome
	)
/*++

Routine Description:

	This routine establishes the runtime environment for script code.

Arguments:

	NWNXHome - Supplies the NWNX home (installation) directory path.

Return Value:

	The routine returns true on success, else false on failure.

Environment:

	User mode.

--*/
{
	try
	{
		m_Runtime = new NWScriptRuntime( m_TextOut, m_Bridge, NWNXHome, this );
	}
	catch (std::exception &e)
	{
		m_TextOut->WriteText(
			"ServerNWScriptPlugin::EstablishRuntime: ERROR: Exception '%s' establishing script runtime environment.  Check that NWNScriptJIT.dll and NWNScriptJITIntrinsics.dll are placed in the Neverwinter Nights 2 installation directory (next to nwn2server.exe).\n",
			e.what( ));

		return false;
	}

	return true;
}

void
ServerNWScriptPlugin::PatchCmdImplementer(
	__in NWN2Server::CVirtualMachine * ServerVM
	)
/*++

Routine Description:

	This routine patches the virtual function table of the script command
	implementer to redirect the RunScriptCallback to an empty callback.  The
	default callback accumulates scripts that have run since the last AIUpdate,
	for debugging purposes only.  However, since AIUpdate does not occur while
	there are no players logged on, this causes extra CPU and memory usage due
	to the increasingly long ScriptsRun string (until a player logs on).

	By redirecting the RunScriptCallback to an empty callback, the problem is
	avoided.

Arguments:

	ServerVM - Supplies the active NWN2Server CVirtualMachine context.

Return Value:

	None.  Failures are ignored, but m_PatchedCmdImplementerVtable is only
	updated on success.

Environment:

	User mode.

--*/
{
	try
	{
		void * * NewVtable = new void *[ CVirtualMachineCmdImplementer::VTABLE_SIZE ];
		void * * OldVtable = *(void ***) ServerVM->GetCommandImplementer( );

		memcpy(
			NewVtable,
			OldVtable,
			CVirtualMachineCmdImplementer::VTABLE_SIZE * sizeof( void * ));

		NewVtable[ CVirtualMachineCmdImplementer::VT_RunScriptCallback ] =
			&NullRunScriptCallback;

		*(void ***) ServerVM->GetCommandImplementer( ) = NewVtable;

		m_PatchedCmdImplementerVtable = NewVtable;
		m_OrigCmdImplementerVtable    = OldVtable;
	}
	catch (std::exception)
	{
	}
}

void
__stdcall
ServerNWScriptPlugin::NullRunScriptCallback(
	__in const NWN2Server::CExoString & ScriptName
	)
/*++

Routine Description:

	This routine is called when the server executes a script.  Its purpose is
	to replace the default logging RunScriptCallback with one that is empty.

Arguments:

	ScriptName - Supplies the name of the script that is to run.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	UNREFERENCED_PARAMETER( ScriptName );
}

void
ServerNWScriptPlugin::LoadSettings(
	__in const char * NWNXHome
	)
/*++

Routine Description:

	This routine loads configuration settings from the INI file.

Arguments:

	NWNXHome - Supplies a pointer to the NWNX4 Home directory path.

Return Value:

	None.  Failures are ignored, but logged.

Environment:

	User mode.

--*/
{
	try
	{
		wchar_t StrValue[ MAX_PATH + 1 ];
		
		if (m_IniPath.empty( ))
		{
			if (!swutil::AnsiToUnicode( NWNXHome, m_IniPath ))
				throw std::runtime_error( "Character conversion failed." );

			m_IniPath += L"\\AuroraServerNWScript.ini";
		}

		m_DebugLevel = (NWScriptVM::ExecDebugLevel) GetPrivateProfileInt(
			L"Settings",
			L"DebugLevel",
			(INT) m_DebugLevel,
			m_IniPath.c_str( ) );

		m_UseReferenceVM = GetPrivateProfileInt(
			L"Settings",
			L"UseReferenceVM",
			m_UseReferenceVM ? 1 : 0,
			m_IniPath.c_str( ) ) ? 1 : 0;

		m_MinFreeMemoryToJIT = (ULONG) GetPrivateProfileInt(
			L"Settings",
			L"MinFreeMemoryToJIT",
			(INT) m_MinFreeMemoryToJIT,
			m_IniPath.c_str( ) );

		m_MinScriptSizeToJIT = (ULONG) GetPrivateProfileInt(
			L"Settings",
			L"MinScriptSizeToJIT",
			(INT) m_MinScriptSizeToJIT,
			m_IniPath.c_str( ) );

		m_MaxLoopIterations = GetPrivateProfileInt(
			L"Settings",
			L"MaxLoopIterations",
			m_MaxLoopIterations,
			m_IniPath.c_str( ) );

		m_MaxCallDepth = GetPrivateProfileInt(
			L"Settings",
			L"MaxCallDepth",
			m_MaxCallDepth,
			m_IniPath.c_str( ) );

		m_OptimizeIR = GetPrivateProfileInt(
			L"Settings",
			L"OptimizeIR",
			(INT) m_OptimizeIR ? 1 : 0,
			m_IniPath.c_str( ) ) ? true : false;

		m_LoadDebugSymbols = GetPrivateProfileInt(
			L"Settings",
			L"LoadDebugSymbols",
			(INT) m_LoadDebugSymbols ? 1 : 0,
			m_IniPath.c_str( ) ) ? true : false;

		m_AllowManagedScripts = GetPrivateProfileInt(
			L"Settings",
			L"AllowManagedScripts",
			(INT) m_AllowManagedScripts ? 1 : 0,
			m_IniPath.c_str( ) ) ? true : false;

		m_DisableExecutionGuards = GetPrivateProfileInt(
			L"Settings",
			L"DisableExecutionGuards",
			(INT) m_DisableExecutionGuards ? 1 : 0,
			m_IniPath.c_str( ) ) ? true : false;

		m_OptimizeActionServiceHandlers = GetPrivateProfileInt(
			L"Settings",
			L"OptimizeActionServiceHandlers",
			(INT) m_OptimizeActionServiceHandlers ? 1 : 0,
			m_IniPath.c_str( ) ) ? true : false;

		GetPrivateProfileString(
			L"Settings",
			L"CodeGenOutputDirectory",
			L"",
			StrValue,
			MAX_PATH,
			m_IniPath.c_str( ));

		if (StrValue[ 0 ] != L'\0')
		{
			m_CodeGenOutputDirectory = StrValue;

			if (*m_CodeGenOutputDirectory.rbegin( ) != L'\\')
				m_CodeGenOutputDirectory.push_back( L'\\' );
		}

		m_TextOut->WriteText(
			"DebugLevel set to %lu.\n",
			(unsigned long) m_DebugLevel );
		m_TextOut->WriteText(
			"UseReferenceVM set to %lu.\n",
			m_UseReferenceVM ? 1 : 0 );
		m_TextOut->WriteText(
			"MinFreeMemoryToJIT set to %lu.\n",
			(unsigned long) m_MinFreeMemoryToJIT );
		m_TextOut->WriteText(
			"MinScriptSizeToJIT set to %lu.\n",
			(unsigned long) m_MinScriptSizeToJIT );
		m_TextOut->WriteText(
			"MaxLoopIterations set to %d.\n",
			m_MaxLoopIterations );
		m_TextOut->WriteText(
			"MaxCallDepth set to %d.\n",
			m_MaxCallDepth );
		m_TextOut->WriteText(
			"OptimizeIR set to %lu.\n",
			m_OptimizeIR ? 1 : 0 );
		m_TextOut->WriteText(
			"LoadDebugSymbols set to %lu.\n",
			m_LoadDebugSymbols ? 1 : 0 );
		m_TextOut->WriteText(
			"AllowManagedScripts set to %lu.\n",
			m_AllowManagedScripts ? 1 : 0 );
		m_TextOut->WriteText(
			"DisableExecutionGuards set to %lu.\n",
			m_DisableExecutionGuards ? 1 : 0 );
		m_TextOut->WriteText(
			"OptimizeActionServiceHandlers set to %lu.\n",
			m_OptimizeActionServiceHandlers ? 1 : 0 );

		if (m_CodeGenOutputDirectory.empty( ))
		{
			m_TextOut->WriteText(
				"Code generation output will not be saved.\n" );
		}
		else
		{
			m_TextOut->WriteText(
				"CodeGenOutputDirectory set to %S.\n",
				m_CodeGenOutputDirectory.c_str( ) );

			_wmkdir( m_CodeGenOutputDirectory.c_str( ) );
		}

		if (m_Runtime != NULL)
			m_Runtime->SetDebugLevel( m_DebugLevel );
		if (m_Bridge != NULL)
			m_Bridge->SetDebugLevel( m_DebugLevel );
	}
	catch (std::exception &e)
	{
		m_TextOut->WriteText(
			"Failed to read settings: Exception '%s'.\n",
			e.what( ));
	}
}

bool
ServerNWScriptPlugin::ExecuteScriptForServer(
	__inout int * PC,
	__inout_ecount( CodeSize ) unsigned char * InstructionStream,
	__in int CodeSize,
	__in NWN2Server::CVirtualMachine * ServerVM
	)
/*++

Routine Description:

	This routine is called to execute a script on behalf of the server.

Arguments:

	PC - Supplies a pointer to the initial program counter for the script.  On
	     return, the final program counter is returned.

	InstructionStream - Supplies a pointer to the instruction stream to execute,
	                    representing the entire script.

	CodeSize - Supplies the length, in bytes, of the instruction stream.

	ServerVM - Supplies the active NWN2Server CVirtualMachine context.

Return Value:

	The routine returns true on success, else false on failure.

Environment:

	User mode.

--*/
{
	if (m_PatchedCmdImplementerVtable == NULL)
		PatchCmdImplementer( ServerVM );

	//
	// Inform the bridge that we are ready to run a script.
	//

	if (!m_Bridge->PrepareForRunScript( ServerVM ))
		return false;

	//
	// Call the runtime to perform the actual script execution.
	//

	try
	{
		m_Runtime->ExecuteScriptForServer(
			(NWSCRIPT_PROGRAM_COUNTER) *PC,
			InstructionStream,
			CodeSize,
			ServerVM);
	}
	catch (std::exception &e)
	{
		m_TextOut->WriteText(
			"ServerNWScriptPlugin::ExecuteScriptForServer:  ERROR: Failed to execute script '%.32s' at PC %08X (exception: '%s').\n",
			ServerVM->GetScriptResRef( ),
			*PC,
			e.what( ));

		return false;
	}

	return true;
}

//
// Called to save the current saved state to the VM stack.
//

unsigned long
ServerNWScriptPlugin::SaveStateToServerVMStack(
	__in NWN2Server::CVirtualMachine * ServerVM
	)
/*++

Routine Description:

	This routine is invoked to save the current saved state to the server's VM
	stack so that it may be packaged into a saved script situation.

Arguments:

	ServerVM - Supplies a pointer to the server's CVirtualMachine instance.

Return Value:

	The routine returns the number of elements placed on the VM stack.

Environment:

	User mode.

--*/
{
	int  OldSP;
	int  NewSP;
	bool Rollback;

	OldSP = ServerVM->GetCurrentSP( );

	Rollback = false;

	try
	{
		m_Runtime->StoreSavedStateToStack( ServerVM );
	}
	catch (std::exception &e)
	{
		m_TextOut->WriteText(
			"ServerNWScriptPlugin::SaveStateToServerVMStack: ERROR:  NWScriptRuntime::StoreSavedStateToStack failed with exception '%s'.\n",
			e.what( ));

		Rollback = true;
	}

	NewSP = ServerVM->GetCurrentSP( );

	if (Rollback)
	{
		RemoveSavedStateFromServerVMStack( ServerVM, NewSP - OldSP );
		NewSP = OldSP;

		ServerVM->SetScriptSituationState( -1, 0, 0 );
	}

	return (unsigned long) (NewSP - OldSP);
}

void
ServerNWScriptPlugin::RemoveSavedStateFromServerVMStack(
	__in NWN2Server::CVirtualMachine * ServerVM,
	__in unsigned long StackElements
	)
/*++

Routine Description:

	This routine is invoked to remove the current saved state from the server's
	VM stack, so that execution may continue normally after the saved state has
	been captured.

Arguments:

	ServerVM - Supplies a pointer to the server's CVirtualMachine instance.

	StackElements - Supplies the number of elements that need to be removed from
	                the server's VM stack.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	//
	// Cleanly remove elements from the top of the stack until we have undone
	// all of our temporary stack pushes.
	//

	for (unsigned long i = 0; i < StackElements; i += 1)
	{
		int  Type;
		bool Status;

		Type = ServerVM->GetTopOfStackType( );

		switch (Type)
		{

		case -1:  // Nothing on the stack
			{
				m_TextOut->WriteText(
					"ServerNWScriptPlugin::RemoveSavedStateFromServerVMStack: Stack underrun (element %lu of %lu).\n",
					i,
					StackElements);
			}
			return;

		case CVirtualMachineStack::ST_INTEGER:
			{
				int Value;

				Status = ServerVM->StackPopInteger( &Value );
			}
			break;

		case CVirtualMachineStack::ST_FLOAT:
			{
				float Value;

				Status = ServerVM->StackPopFloat( &Value );
			}
			break;

		case CVirtualMachineStack::ST_STRING:
			{
				try
				{
					std::string Value;

					Status = ServerVM->StackPopString( &Value );
				}
				catch (std::exception)
				{
					m_TextOut->WriteText(
						"ServerNWScriptPlugin::RemoveSavedStateFromServerVMStack: Exception removing string.\n");
					return;
				}
			}
			break;

		case CVirtualMachineStack::ST_OBJECT:
			{
				NWN::OBJECTID Value;

				Status = ServerVM->StackPopObject( &Value );
			}
			break;

		default:
			{
				if ((Type >= CVirtualMachineStack::ST_ENGINE_0) &&
				    (Type <= CVirtualMachineStack::ST_ENGINE_9))
				{
					void * Representation;

					Status = ServerVM->StackPopEngineStructure(
						(ENGINE_STRUCTURE_TYPE) (Type - CVirtualMachineStack::ST_ENGINE_0),
						&Representation);

					if (Status)
					{
						ServerVM->GetCommandImplementer( )->DestroyGameDefinedStructure(
							(ENGINE_STRUCTURE_TYPE) (Type - CVirtualMachineStack::ST_ENGINE_0),
							Representation);
					}
				}
				else
				{
					m_TextOut->WriteText(
						"ServerNWScriptPlugin::RemoveSavedStateFromServerVMStack: Unknown type %d on server VM stack.\n",
						Type);
					return;
				}
			}
			break;

		}

		if (!Status)
		{
			m_TextOut->WriteText(
				"ServerNWScriptPlugin::RemoveSavedStateFromServerVMStack: Failed to remove element %lu of %lu (type %d).\n",
				i,
				StackElements,
				Type);
			return;
		}
	}
}

INWScriptJITPolicy::NWSCRIPT_ENGINE
ServerNWScriptPlugin::SelectEngineForScript(
	__in size_t CodeSize
	)
/*++

Routine Description:

	This routine selects whether a script should prefer to be JIT'd versus ran
	in the reference VM.

Arguments:

	CodeSize - Supplies the number of code bytes used for the script, in bytes.

Return Value:

	The routine returns a value from the NWSCRIPT_ENGINE enumeration that
	indicates the preferred execution engine to use for the script.

Environment:

	User mode.

--*/
{
	ULONGLONG AvailableVASpace;

	//
	// If the reference VM was forced, always prefer to use it.
	//

	if (m_UseReferenceVM)
		return INWScriptJITPolicy::NWSCRIPT_ENGINE_VM;

	//
	// If the script was below the minimum size to JIT and we have a minimum
	// size quota enabled, prefer to use the VM.
	//

	if ((m_MinScriptSizeToJIT != 0) && (CodeSize < (size_t) m_MinScriptSizeToJIT))
		return INWScriptJITPolicy::NWSCRIPT_ENGINE_VM;

	//
	// If memory quotas were turned off, always prefer the JIT engine.
	//

	if (m_MinFreeMemoryToJIT == 0)
		return INWScriptJITPolicy::NWSCRIPT_ENGINE_JIT;

	//
	// Otherwise, prefer to use the JIT engine unless we have exceeded the
	// memory quota.
	//

	AvailableVASpace = GetAvailableVASpace( );
	
	if (AvailableVASpace < (ULONGLONG) m_MinFreeMemoryToJIT)
	{
		m_TextOut->WriteText(
			"ServerNWScriptPlugin::SelectEngineForScript: Available VA space (%I64lu) is below limit (%I64lu), not JIT'd script of %I64lu bytes.\n",
			AvailableVASpace,
			(ULONGLONG) m_MinFreeMemoryToJIT,
			(ULONGLONG) CodeSize);

		return INWScriptJITPolicy::NWSCRIPT_ENGINE_VM;
	}

	return INWScriptJITPolicy::NWSCRIPT_ENGINE_JIT;
}

bool
ServerNWScriptPlugin::GetEnableIROptimizations(
	)
/*++

Routine Description:

	This routine determines whether IR optimizations are enabled.

Arguments:

	None.

Return Value:

	The routine returns a Boolean value indicating true if IR optimizations are
	to be enabled.

Environment:

	User mode.

--*/
{
	return m_OptimizeIR;
}

bool
ServerNWScriptPlugin::GetLoadDebugSymbols(
	)
/*++

Routine Description:

	This routine determines whether debug symbols should be loaded.

Arguments:

	None.

Return Value:

	The routine returns a Boolean value indicating true if debug symbols should
	be loaded.

Environment:

	User mode.

--*/
{
	return m_LoadDebugSymbols;
}

const wchar_t *
ServerNWScriptPlugin::GetCodeGenOutputDir(
	)
/*++

Routine Description:

	This routine determines the directory where code generation output debugging
	files should be saved.

Arguments:

	None.

Return Value:

	The routine returns the code generation output directory (with a trailing
	path separator), else NULL if code generation output should not be saved.

Environment:

	User mode.

--*/
{
	if (m_CodeGenOutputDirectory.empty( ))
		return NULL;
	else
		return m_CodeGenOutputDirectory.c_str( );
}

bool
ServerNWScriptPlugin::GetAllowManagedScripts(
	)
/*++

Routine Description:

	This routine determines whether managed scripts are permitted.

Arguments:

	None.

Return Value:

	The routine returns a Boolean value indicating true if managed scripts are
	permitted.

Environment:

	User mode.

--*/
{
	return m_AllowManagedScripts;
}

int
ServerNWScriptPlugin::GetMaxLoopIterations(
	)
/*++

Routine Description:

	This routine determines the maximum loop iteration count for scripts.

Arguments:

	None.

Return Value:

	The routine returns the maximum loop iteration count for scripts.

Environment:

	User mode.

--*/
{
	return m_MaxLoopIterations;
}

int
ServerNWScriptPlugin::GetMaxCallDepth(
	)
/*++

Routine Description:

	This routine determines the maximum call depth for scripts.

Arguments:

	None.

Return Value:

	The routine returns the maximum call depth for scripts.

Environment:

	User mode.

--*/
{
	return m_MaxCallDepth;
}

bool
ServerNWScriptPlugin::GetDisableExecutionGuards(
	)
/*++

Routine Description:

	This routine determines whether script execution guards are disabled.

Arguments:

	None.

Return Value:

	The routine returns a Boolean value indicating true if script execution
	guards are disabled.

Environment:

	User mode.

--*/
{
	return m_DisableExecutionGuards;
}

bool
ServerNWScriptPlugin::GetOptimizeActionServiceHandlers(
	)
/*++

Routine Description:

	This routine determines whether action service handler optimizations are
	enabled.

Arguments:

	None.

Return Value:

	The routine returns a Boolean value indicating true if action service
	handler optimizations should be enabled.

Environment:

	User mode.

--*/
{
	return m_OptimizeActionServiceHandlers;
}




