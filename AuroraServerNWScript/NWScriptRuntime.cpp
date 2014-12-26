/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptRuntime.cpp

Abstract:

	This module houses the NWScriptRuntime object, which bears responsibility
	for executing scripts via the replacement execution environment.

--*/

#include "Precomp.h"
#include "Offsets.h"
#include "NWN2Def.h"
#include "NWScriptBridge.h"
#include "NWScriptRuntime.h"
#include "NWScriptJITPolicy.h"
#include "../NWNScriptLib/NWScriptInternal.h"
#include "MiscUtils.h"

using namespace NWN2Server;

//
// Define to 1 to support NWScriptVM fallback if we fail to JIT a script at
// runtime, such as due to memory pressure.
//

#define NWSCRIPTVM_FALLBACK 1

void
NWScriptRuntime::StoreSavedStateToStack(
	__in NWN2Server::CVirtualMachine * ServerVM
	)
/*++

Routine Description:

	This routine packages the last saved state of the current program into a VM
	stack data set for use as a script situation.

Arguments:

	ServerVM - Supplies the server's current CVirtualMachine instance context.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	ULONG                    ResumeMethodId;
	NWSCRIPT_PROGRAM_COUNTER ResumeMethodPC;
	ULONG                    SaveGlobalCount;
	ULONG                    SaveLocalCount;
	NWN::OBJECTID            ObjectSelf;
	int                      OldSP;
	int                      NewSP;

#if !NWSCRIPTVM_FALLBACK
	if (m_CurrentJITProgram.get( ) == NULL)
		throw std::runtime_error( "No script program is executing." );
#endif

#if NWSCRIPTVM_FALLBACK
	if (m_CurrentJITProgram.get( ) != NULL)
#endif
	{
		//
		// Save the current state.
		//
		
		NWScriptJITLib::SavedState SavedState( m_CurrentJITProgram->CreateSavedState( ) );

		//
		// Now push it on the stack.
		//

		OldSP = ServerVM->GetCurrentSP( );

		m_CurrentJITProgram->PushSavedState(
			&SavedState,
			m_Bridge,
			&ResumeMethodId,
			&ResumeMethodPC,
			&SaveGlobalCount,
			&SaveLocalCount,
			&ObjectSelf);
	}
#if NWSCRIPTVM_FALLBACK
	else
	{
		//
		// Save the current state.
		//

		NWScriptVM::VMState & SavedState( m_VM->GetSavedState( ) );

		//
		// Now push it on the stack.
		//

		OldSP = ServerVM->GetCurrentSP( );

		SavedState.Stack.SaveStack(
			m_Bridge,
			SavedState.Stack.GetCurrentBP( ),
			SavedState.Stack.GetCurrentSP( ) - SavedState.Stack.GetCurrentBP( ));

		ResumeMethodId  = 0;
		ResumeMethodPC  = SavedState.ProgramCounter;
		SaveGlobalCount = (ULONG) SavedState.Stack.GetCurrentBP( ) / SavedState.Stack.GetStackIntegerSize( );
		SaveLocalCount  = (ULONG) (SavedState.Stack.GetCurrentSP( ) - SavedState.Stack.GetCurrentBP( )) / SavedState.Stack.GetStackIntegerSize( );
		ObjectSelf      = SavedState.ObjectSelf;
	}
#endif

	//
	// Save the rest of the intermediate state onto the stack as well; we'll
	// pull it off when the script situation is run.
	//
	// N.B.  The code size does not strictly need to be saved (nor the
	//       SAVED_STATE_ID "header").  These fields are retained for
	//       debugging purposes, i.e. to catch issues where a script
	//       situation is restored using a different script's code.
	//

	m_Bridge->StackPushInt( (int) ResumeMethodId );
	m_Bridge->StackPushInt( (int) ResumeMethodPC );
	m_Bridge->StackPushInt( (int) SaveGlobalCount );
	m_Bridge->StackPushInt( (int) SaveLocalCount );
	m_Bridge->StackPushObjectId( ObjectSelf );
	m_Bridge->StackPushInt( (int) m_CurrentScriptCodeSize );
	m_Bridge->StackPushString( ServerVM->GetScriptName( ) );
	m_Bridge->StackPushInt( (int) SAVED_STATE_ID );

	NewSP = ServerVM->GetCurrentSP( );

	ServerVM->SetScriptSituationState(
		(int) ResumeMethodPC,
		NewSP - OldSP,
		0);
}

void
NWScriptRuntime::ExecuteScriptForServer(
	__in NWSCRIPT_PROGRAM_COUNTER PC,
	__inout_ecount( CodeSize ) unsigned char * InstructionStream,
	__in int CodeSize,
	__in NWN2Server::CVirtualMachine * ServerVM
	)
/*++

Routine Description:

	This routine is called to execute a script on behalf of the server.

Arguments:

	PC - Supplies the initial program counter for the script.  If the request is
	     to execute a script situation, the PC value is non-zero.  Otherwise,
	     the PC points to the beginning of the script (zero).

	InstructionStream - Supplies a pointer to the instruction stream to execute,
	                    representing the entire script.

	CodeSize - Supplies the length, in bytes, of the instruction stream.

	ServerVM - Supplies the active NWN2Server CVirtualMachine context.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	ScriptCacheData                  * ScriptData;
	NWScriptJITLib::Program::Ptr       PrevProgram;
	NWN::ResRef32                      PrevScriptName;
	size_t                             PrevScriptCodeSize;
	bool                               TraceCall;
	ULONG                              Time;

	TraceCall = (m_Bridge->IsDebugLevel( NWScriptVM::EDL_Calls ) );

	//
	// If we were executing a script situation, instantiate a script situation
	// state from the contents that were placed on the VM stack during sript
	// situation restoration in the server.
	//

	if (PC != 0)
	{
		NWSCRIPT_PROGRAM_COUNTER         EffectivePC;
		std::string                      ScriptName;
		ScriptResumeData                 ResumeData;

		RestoreSavedStateFromStack(
			ServerVM,
			InstructionStream,
			CodeSize,
			&ScriptData,
			&ResumeData,
			EffectivePC,
			ScriptName);

		if (EffectivePC != PC)
			throw std::runtime_error( "Resuming script situation at wrong PC." );

		ScriptData->ScriptSituationCount += 1;

		PrevProgram             = m_CurrentJITProgram;
		m_CurrentJITProgram     = ScriptData->JITProgram;
		PrevScriptName          = m_CurrentScriptName;
		m_CurrentScriptName     = ResRef32FromStr( ScriptName );
		PrevScriptCodeSize      = m_CurrentScriptCodeSize;
		m_CurrentScriptCodeSize = CodeSize;
		m_RecursionLevel        = m_RecursionLevel + 1;

		ScriptData->RecursionLevel += 1;

		try
		{
			if (TraceCall)
			{
				m_TextOut->WriteText(
					"NWScriptRuntime::ExecuteScriptForServer: Executing script situation %s.%lu.\n",
					ScriptName.c_str( ),
					EffectivePC);
			}

			Time = ReadPerformanceCounterMilliseconds( );

#if NWSCRIPTVM_FALLBACK
			if (ScriptData->JITProgram.get( ) != NULL)
#endif
			{
				ScriptData->JITProgram->ExecuteScriptSituation(
					ResumeData.ScriptSituationJIT.get( ),
					ServerVM->GetCurrentActionObjectSelf( ) );
			}
#if NWSCRIPTVM_FALLBACK
			else
			{
				//
				// Carefully reset the instruction buffer for the script, but only
				// if we were the first invocation of the script on the call stack.
				// Otherwise we would cache a pointer to a new instruction buffer
				// that will become stale when the recursive instance returns.
				//

				if (ScriptData->RecursionLevel == 1)
				{
					ScriptData->Reader->ResetInstructionBuffer(
						InstructionStream,
						CodeSize);
				}

				ResumeData.ScriptSituation->ObjectSelf = ServerVM->GetCurrentActionObjectSelf( );

				m_VM->ExecuteScriptSituation(
					*ResumeData.ScriptSituation.get( ) );
			}
#endif

			Time = ReadPerformanceCounterMilliseconds( ) - Time;
			ScriptData->Runtime += Time;

			if (TraceCall)
			{
				m_TextOut->WriteText(
					"NWScriptRuntime::ExecuteScriptForServer: Finished executing script situation %s.%lu.\n",
					ScriptName.c_str( ),
					EffectivePC);
			}
		}
		catch (std::exception)
		{
			m_CurrentScriptCodeSize = PrevScriptCodeSize;
			m_CurrentScriptName     = PrevScriptName;
			m_CurrentJITProgram     = PrevProgram;
			m_RecursionLevel        = m_RecursionLevel - 1;

			ScriptData->RecursionLevel -= 1;

			throw;
		}
	}
	else
	{
		NWScriptParamVec Params;
		int              ReturnCode;
		NWN::ResRef32    ScriptName;

		//
		// First, load the script (generating code for it if necessary).
		//

		ScriptName = ResRef32FromStr( ServerVM->GetScriptName( ) );

		if (!LoadScript(
			ScriptName,
			InstructionStream,
			CodeSize,
			&ScriptData))
		{
			m_TextOut->WriteText(
				"NWScriptRuntime::ExecuteScriptForServer: Script \"%s\" is broken, skipping execution.\n",
				ServerVM->GetScriptName( ));

			ServerVM->MarkCleanScriptReturn( );

			return;
		}

		ScriptData->CallCount += 1;

		PrevProgram             = m_CurrentJITProgram;
		m_CurrentJITProgram     = ScriptData->JITProgram;
		PrevScriptName          = m_CurrentScriptName;
		m_CurrentScriptName     = ScriptName;
		PrevScriptCodeSize      = m_CurrentScriptCodeSize;
		m_CurrentScriptCodeSize = CodeSize;
		m_RecursionLevel        = m_RecursionLevel + 1;

		ScriptData->RecursionLevel += 1;

		try
		{
			ConvertScriptParameters( Params, ServerVM );

			if (TraceCall)
			{
				m_TextOut->WriteText(
					"NWScriptRuntime::ExecuteScriptForServer: Executing script %s (%lu arguments).\n",
					ServerVM->GetScriptName( ),
					(unsigned long) Params.size( ));
			}

			Time = ReadPerformanceCounterMilliseconds( );

#if NWSCRIPTVM_FALLBACK
			if (ScriptData->JITProgram.get( ) != NULL)
#endif
			{
				ReturnCode = ScriptData->JITProgram->ExecuteScript(
					m_Bridge,
					ServerVM->GetCurrentActionObjectSelf( ),
					Params,
					0,
					0);
			}
#if NWSCRIPTVM_FALLBACK
			else
			{
				//
				// Carefully reset the instruction buffer for the script, but only
				// if we were the first invocation of the script on the call stack.
				// Otherwise we would cache a pointer to a new instruction buffer
				// that will become stale when the recursive instance returns.
				//

				if (ScriptData->RecursionLevel == 1)
				{
					ScriptData->Reader->ResetInstructionBuffer(
						InstructionStream,
						CodeSize);
				}

				ReturnCode = m_VM->ExecuteScript(
					ScriptData->Reader,
					ServerVM->GetCurrentActionObjectSelf( ),
					NWN::INVALIDOBJID,
					Params,
					0,
					NWScriptVM::ESF_STATIC_TYPE_DISCOVERY);
			}
#endif

			Time = ReadPerformanceCounterMilliseconds( ) - Time;
			ScriptData->Runtime += Time;

			if (TraceCall)
			{
				m_TextOut->WriteText(
					"NWScriptRuntime::ExecuteScriptForServer: Finished executing script %s (%lu arguments).\n",
					ServerVM->GetScriptName( ),
					(unsigned long) Params.size( ));
			}
		}
		catch (std::exception)
		{
			m_CurrentScriptCodeSize = PrevScriptCodeSize;
			m_CurrentScriptName     = PrevScriptName;
			m_CurrentJITProgram     = PrevProgram;
			m_RecursionLevel        = m_RecursionLevel - 1;

			ScriptData->RecursionLevel -= 1;

			//
			// If we ran the script with the VM, we'll only be notified of a
			// problem with the script on the first run.  Track this now.
			//

			if ((ScriptData->FirstRun) && (ScriptData->JITProgram.get( ) == NULL))
				ScriptData->BrokenScript = true;

			throw;
		}

		ServerVM->SetScriptReturnCode( ReturnCode );
	}

	if (ScriptData->FirstRun == true)
		ScriptData->FirstRun = false;

	ServerVM->MarkCleanScriptReturn( );

	m_CurrentScriptCodeSize = PrevScriptCodeSize;
	m_CurrentScriptName     = PrevScriptName;
	m_CurrentJITProgram     = PrevProgram;
	m_RecursionLevel        = m_RecursionLevel - 1;

	ScriptData->RecursionLevel -= 1;

	if (m_RecursionLevel == 0)
		m_TotalScriptRuntime += Time;
}

void
NWScriptRuntime::DumpStatistics(
	)
/*++

Routine Description:

	This routine writes statistics about the runtime to the debug console.

Arguments:

	None.

Return Value:

	None.  Failures are ignored.

Environment:

	User mode.

--*/
{
	try
	{
		ULONG64  ThreadTimeMs;
		FILETIME CreationTime;
		FILETIME ExitTime;
		FILETIME KernelTime;
		FILETIME UserTime;
		ULONG    TotalMemoryCost;

		m_TextOut->WriteText(
			"NWScriptRuntime::DumpStatistics: %lu scripts cached:\n",
			(unsigned long) m_ScriptCache.size( ));

		TotalMemoryCost = 0;

		if (!GetThreadTimes(
			GetCurrentThread( ),
			&CreationTime,
			&ExitTime,
			&KernelTime,
			&UserTime))
		{
			UserTime.dwLowDateTime    = 0;
			UserTime.dwHighDateTime   = 0;
			KernelTime.dwLowDateTime  = 0;
			KernelTime.dwHighDateTime = 0;
		}

		for (ScriptCacheMap::const_iterator it = m_ScriptCache.begin( );
		     it != m_ScriptCache.end( );
		     ++it)
		{
			m_TextOut->WriteText(
				"%s - %s (%lu calls, %lu script situations, %lu bytes VA space usage, %lums runtime).\n",
				StrFromResRef( it->first ).c_str( ),
				it->second.JITProgram.get( ) != NULL ? "(JIT)" : "(VM)",
				(unsigned long) it->second.CallCount,
				(unsigned long) it->second.ScriptSituationCount,
				(unsigned long) it->second.MemoryCost,
				it->second.Runtime);

			TotalMemoryCost += it->second.MemoryCost;
		}

		ThreadTimeMs  = ((ULONG64) UserTime.dwLowDateTime | ((ULONG64) UserTime.dwHighDateTime << 32));
		ThreadTimeMs += ((ULONG64) KernelTime.dwLowDateTime | ((ULONG64) KernelTime.dwHighDateTime << 32));
		ThreadTimeMs /= 10000;

		m_TextOut->WriteText(
			"Total time spent running scripts: %I64lums.\n"
			"Total time spent in thread 0: %I64lums.\n"
			"Scripts consumed %g%% of thread 0 time.\n"
			"Scripts compiled to native code consumed approximately %lu bytes of VA space.\n",
			m_TotalScriptRuntime,
			ThreadTimeMs,
			((double) m_TotalScriptRuntime / (double) ThreadTimeMs) * 100.0,
			TotalMemoryCost);
	}
	catch (std::exception)
	{
	}
}

void
NWScriptRuntime::ClearScriptCache(
	)
/*++

Routine Description:

	This routine removes cached state about scripts, unloading the native code
	for any scripts that were JIT'd.  Note that scripts may still have pending
	script situations outstanding.

Arguments:

	None.

Return Value:

	None.  Failures are ignored.

Environment:

	User mode.

--*/
{
	m_ScriptCache.clear( );
}

void
NWScriptRuntime::SetDebugLevel(
	__in NWScriptVM::ExecDebugLevel DebugLevel
	)
/*++

Routine Description:

	This routine sets the active debug level on the NWScript VM, i.e. if it has
	changed since startup.

Arguments:

	DebugLevel - Supplies the new debug level.

Return Value:

	None.  Failures are ignored.

Environment:

	User mode.

--*/
{
	if (m_VM != NULL)
		m_VM->SetDebugLevel( DebugLevel );
}

void
NWScriptRuntime::LoadJITEngine(
	__in const char * DllDir
	)
/*++

Routine Description:

	This routine loads the just-in-time (JIT) execution engine that will be used
	to service script execution requests.

Arguments:

	DllDir - Supplies the directory to load the JIT engine from.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	UNREFERENCED_PARAMETER( DllDir );

	try
	{
		m_JITEngine = new NWScriptJITLib( L"NWNScriptJIT.dll" );
	}
	catch (std::exception &e)
	{
		m_JITEngine = NULL;

#if NWSCRIPTVM_FALLBACK
		m_TextOut->WriteText(
			"NWScriptRuntime::LoadJITEngine: Exception \"%s\" loading JIT engine, falling back to NWScript VM.\n",
			e.what( ));
#else
		e;
		throw;
#endif
	}

	if (m_JITEngine != NULL)
	{
		try
		{
			swutil::UnicodeToAnsi(
				m_JITEngine->GetEngineName( ), m_ScriptEngineName );
		}
		catch (std::exception)
		{
		}
	}
	else
	{
#if NWSCRIPTVM_FALLBACK
		m_ScriptEngineName = "NWScript VM";
#endif
	}

	m_TextOut->WriteText(
		"Server NWScript engine set to %s.\n", m_ScriptEngineName.c_str( ));

	//
	// If configured, enable support for managed code scripts.
	//

	try
	{
		if (m_JITPolicy->GetAllowManagedScripts( ) && m_JITEngine != NULL)
		{
			NWSCRIPT_JIT_PARAMS CodeGenParams;

			ZeroMemory( &CodeGenParams, sizeof( CodeGenParams ) );

			CodeGenParams.Size             = sizeof( CodeGenParams );
			CodeGenParams.CodeGenFlags     = 0;
			CodeGenParams.CodeGenOutputDir = m_JITPolicy->GetCodeGenOutputDir( );

			if (CodeGenParams.CodeGenOutputDir != NULL)
				CodeGenParams.CodeGenFlags |= NWCGF_SAVE_OUTPUT;

			if (m_JITPolicy->GetOptimizeActionServiceHandlers( ))
				CodeGenParams.CodeGenFlags |= NWCGF_NWN_COMPATIBLE_ACTIONS;

			m_JITManagedSupport = m_JITEngine->CreateManagedSupportPtr(
				NWActions_NWN2,
				MAX_ACTION_ID_NWN2,
				0,
				m_TextOut,
				(ULONG) m_Bridge->GetScriptDebug( ),
				m_Bridge,
				NWN::INVALIDOBJID,
				&CodeGenParams);
		}
	}
	catch (std::exception &e)
	{
		m_TextOut->WriteText(
			"WARNING: Failed to setup managed script support: Exception: '%s'.\n",
			e.what( ) );
	}

	if (m_JITPolicy->GetAllowManagedScripts( ) && m_JITEngine == NULL)
	{
		m_TextOut->WriteText(
			"WARNING: Managed scripts will not run unless the JIT engine is loaded.\n");
	}
}

void
NWScriptRuntime::RestoreSavedStateFromStack(
	__in NWN2Server::CVirtualMachine * ServerVM,
	__in_ecount( CodeSize ) const unsigned char * InstructionStream,
	__in size_t CodeSize,
	__deref_out ScriptCacheData * * ScriptData,
	__out ScriptResumeData * ResumeData,
	__out NWSCRIPT_PROGRAM_COUNTER & PC,
	__out std::string & ScriptName
	)
/*++

Routine Description:

	This routine unpackages a program saved state from the current VM stack, and
	instantiates the saved state into the JIT system.

Arguments:

	ServerVM - Supplies the server's current CVirtualMachine instance context.

	InstructionStream - Supplies the complete script program instruction stream.

	CodeSize - Supplies the length, in bytes, of the instruction stream.

	ScriptData - On success, receives a pointer to the script cache data that
	             tracks the state of the script.

	ResumeData - On success, receives the instantiated script situation state.

	PC - Receives the starting NWScript PC of the saved script situation.

	ScriptName - Receives the name of the script being resumed.

Return Value:

	On success, a pointer to a saved state object is returned.
	On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	ULONG                        ResumeMethodId;
	NWSCRIPT_PROGRAM_COUNTER     ResumeMethodPC;
	ULONG                        SaveGlobalCount;
	ULONG                        SaveLocalCount;
	NWN::OBJECTID                ObjectSelf;
	size_t                       SavedCodeSize;

	UNREFERENCED_PARAMETER( ServerVM );

	//
	// Check that the saved state is valid.
	//

	if (m_Bridge->StackPopInt( ) != (int) SAVED_STATE_ID)
		throw std::runtime_error( "Saved state signature does not match." );

	ScriptName = m_Bridge->StackPopString( );

	if (!LoadScript(
		ResRef32FromStr( ScriptName ),
		InstructionStream,
		CodeSize,
		ScriptData))
	{
		m_TextOut->WriteText(
			"NWScriptRuntime::RestoreSavedStateFromStack: Script \"%s\" is broken, skipping execution.\n",
			ScriptName.c_str( ));

		throw std::runtime_error( "Script previously failed to generate code." );
	}

	SavedCodeSize = (size_t) m_Bridge->StackPopInt( );

	if ((CodeSize != SavedCodeSize) && (SavedCodeSize != 0))
	{
		if (m_Bridge->IsDebugLevel( NWScriptVM::EDL_Errors ))
		{
			m_TextOut->WriteText(
				"NWScriptRuntime::RestoreSavedStateFromStack: Attempting to restore script situation for script '%s' with code size '%lu', but saved code size was '%lu'.\n",
				ScriptName.c_str( ),
				CodeSize,
				SavedCodeSize);
		}

		throw std::runtime_error( "Restoring script situation for wrong script." );
	}

	// 
	// Unpack the state from the stack and instantiate a saved state object.
	//
	// N.B.  It is assumed that the script cache maintains a pointer to the
	//       looked up program object (and subsequently keeps it live) until the
	//       caller has finished.
	//

	ObjectSelf      = m_Bridge->StackPopObjectId( );
	SaveLocalCount  = (ULONG) m_Bridge->StackPopInt( );
	SaveGlobalCount = (ULONG) m_Bridge->StackPopInt( );
	ResumeMethodPC  = (NWSCRIPT_PROGRAM_COUNTER) m_Bridge->StackPopInt( );
	ResumeMethodId  = (ULONG) m_Bridge->StackPopInt( );

	PC = ResumeMethodPC;

#if NWSCRIPTVM_FALLBACK
	if ((*ScriptData)->JITProgram.get( ) != NULL)
#endif
	{
		ResumeData->ScriptSituationJIT = (*ScriptData)->JITProgram->PopSavedStatePtr(
			m_Bridge,
			ResumeMethodId,
			ResumeMethodPC,
			SaveGlobalCount,
			SaveLocalCount,
			ObjectSelf);
	}
#if NWSCRIPTVM_FALLBACK
	else
	{
		NWScriptVM::VMState * State;
		int                   OldBP;

		ResumeData->ScriptSituation = new NWScriptVM::VMState;

		State = ResumeData->ScriptSituation.get( );

		State->Script         = (*ScriptData)->Reader;
		State->ProgramCounter = ResumeMethodPC;
		State->ObjectSelf     = ObjectSelf;
		State->ObjectInvalid  = NWN::INVALIDOBJID;
		State->Aborted        = false;

		OldBP = ServerVM->GetCurrentBP( );
		ServerVM->SetCurrentBP( SaveGlobalCount );

		ServerVM->SaveStack(
			&State->Stack,
			ServerVM->GetCurrentBP( ),
			ServerVM->GetCurrentSP( ) - ServerVM->GetCurrentBP( ));

		ServerVM->SetCurrentBP( OldBP );

		State->Stack.SetCurrentBP(
			(NWScriptStack::STACK_POINTER) SaveGlobalCount * State->Stack.GetStackIntegerSize( )
			);
	}
#endif
}

bool
NWScriptRuntime::LoadScript(
	__in const NWN::ResRef32 & ScriptName,
	__in_ecount( CodeSize ) const unsigned char * InstructionStream,
	__in size_t CodeSize,
	__deref_out ScriptCacheData * * ScriptData
	)
/*++

Routine Description:

	This routine loads a script and returns a compiled script program for the
	script instruction stream.

	If the script already existed in the cache, the cached entry is returned.
	Otherwise, the script is compiled to native code on the fly and returned.

Arguments:

	ScriptName - Supplies the resource name of the script to load.

	InstructionStream - Supplies the complete script program instruction stream.

	CodeSize - Supplies the length, in bytes, of the instruction stream.

	ScriptData - On success, receives a pointer to the script cache data that
	             tracks the state of the script.

Return Value:

	The routine returns true on success, else false if the script is a bad
	script that is previously known to fail.

	Otherwise, errors are reported via raising an std::exception.

Environment:

	User mode.

--*/
{
	NWN::ResRef32            ResRef;
	ScriptCacheMap::iterator it;
	NWSCRIPT_JIT_PARAMS      CodeGenParams;
	ULONG                    StartTick;
	ULONGLONG                StartVASpace;

	//
	// Convert the name to a canonical resref and search for it in our cache.
	//

	for (size_t i = 0; i < sizeof( NWN::ResRef32 ); i += 1)
	{
		ResRef.RefStr[ i ] = (char) tolower(
			(int) (unsigned char) ScriptName.RefStr[ i ] );
	}

	it = m_ScriptCache.find( ResRef );

	if (it != m_ScriptCache.end( ))
	{
		if (it->second.BrokenScript)
			return false;

		*ScriptData = &it->second;
		return true;
	}

	//
	// If we didn't have the script cache, then we shall have to instantiate a
	// new instance.  Do so now.
	//

	ScriptCacheData   Data;
	NWScriptReaderPtr Script;
	std::string       ScriptNameStr( StrFromResRef( ResRef ) );

	Data.BrokenScript         = false;
	Data.FirstRun             = true;
	Data.CallCount            = 0;
	Data.ScriptSituationCount = 0;
	Data.MemoryCost           = 0;
	Data.Runtime              = 0;
	Data.RecursionLevel       = 0;

	//
	// Construct a NWScriptReader for the in-memory instruction stream and hand
	// it off to the JIT engine for code generation.
	//
	// N.B.  Note that the instruction stream is not guaranteed to remain valid
	//       beyond when this routine returns.  If it is used afterwards, such
	//       as by the NWScript VM, the buffer must be rebased to the current
	//       instruction buffer on each execution.
	//

	StartVASpace = GetAvailableVASpace( );
	StartTick    = ReadPerformanceCounterMilliseconds( );

	Script = new NWScriptReader(
		ScriptNameStr.c_str( ),
		InstructionStream,
		CodeSize,
		NULL,
		0);

	//
	// The CVirtualMachine may have already patched #loader for the return
	// value workaround.  Check for this now and inform the VM of it so that it
	// can compensate if we do run the script in the VM and not the JIT engine.
	//

	if (CodeSize >= 2)
	{
		UCHAR Opcode;
		UCHAR TypeOpcode;

		Script->SetInstructionPointer( 0 );
		Script->ReadInstruction( Opcode, TypeOpcode );
		Script->SetInstructionPointer( 0 );

		if (Opcode == OP_NOP)
			Script->SetPatchState( NWScriptReader::NCSPatchState_PatchReturnValue );
	}

	LoadSymbols( *Script, ScriptNameStr );

	ZeroMemory( &CodeGenParams, sizeof( CodeGenParams ) );

	CodeGenParams.Size             = sizeof( CodeGenParams );
	CodeGenParams.CodeGenFlags     = NWCGF_ENABLE_SAVESTATE_TO_VMSTACK |
	                                 NWCGF_ASSUME_LOADER_PATCHED;
	CodeGenParams.CodeGenOutputDir = m_JITPolicy->GetCodeGenOutputDir( );

	if (CodeGenParams.CodeGenOutputDir != NULL)
		CodeGenParams.CodeGenFlags |= NWCGF_SAVE_OUTPUT;

	if (m_JITPolicy->GetOptimizeActionServiceHandlers( ))
		CodeGenParams.CodeGenFlags |= NWCGF_NWN_COMPATIBLE_ACTIONS;

	try
	{
#if NWSCRIPTVM_FALLBACK
		if (ShouldJITScript( CodeSize ))
#endif
		{
			ULONG AnalysisFlags = 0;

			if (!m_JITPolicy->GetEnableIROptimizations( ))
				AnalysisFlags |= NWScriptAnalyzer::AF_NO_OPTIMIZATIONS;

			if (m_JITManagedSupport.get( ) != NULL)
			{
				CodeGenParams.CodeGenFlags   |= NWCGF_MANAGED_SCRIPT_SUPPORT;
				CodeGenParams.ManagedSupport  = m_JITManagedSupport->GetManagedSupport( );
			}

			if (m_JITPolicy->GetDisableExecutionGuards( ))
				CodeGenParams.CodeGenFlags |= NWCGF_DISABLE_EXECUTION_GUARDS;

			CodeGenParams.MaxLoopIterations = m_JITPolicy->GetMaxLoopIterations( );
			CodeGenParams.MaxCallDepth      = m_JITPolicy->GetMaxCallDepth( );

			Data.JITProgram = m_JITEngine->GenerateCodePtr(
				Script.get( ),
				NWActions_NWN2,
				MAX_ACTION_ID_NWN2,
				AnalysisFlags,
				m_TextOut,
				(ULONG) m_Bridge->GetScriptDebug( ),
				m_Bridge,
				NWN::INVALIDOBJID,
				&CodeGenParams);
		}
#if NWSCRIPTVM_FALLBACK
		else
		{
			Data.Reader     = Script;
			Data.JITProgram = NULL;

			it = m_ScriptCache.insert( ScriptCacheMap::value_type( ResRef, Data ) ).first;

			*ScriptData = &it->second;

			m_Bridge->GetTextOut( )->WriteText(
				"Using NWScript VM for script '%s' (%lu bytes compiled script).\n",
				ScriptNameStr.c_str( ),
				(unsigned long) CodeSize);

			return true;
		}
#endif
	}
	catch (std::exception &e)
	{
		m_Bridge->GetTextOut( )->WriteText(
			"NWScriptRuntime::LoadScript: Failed to generate code for script '%s' (%lu bytes compiled script): exception '%s'.\n",
			ScriptNameStr.c_str( ),
			(unsigned long) CodeSize,
			e.what( ));

#if NWSCRIPTVM_FALLBACK
		Data.Reader     = Script;
		Data.JITProgram = NULL;

		it = m_ScriptCache.insert( ScriptCacheMap::value_type( ResRef, Data ) ).first;

		*ScriptData = &it->second;

		return true;
#else
		ScriptData.BrokenScript = true;
		Data.Reader             = NULL;
		Data.JITProgram         = NULL;

		it = m_ScriptCache.insert( ScriptCacheMap::value_type( ResRef, Data ) ).first;

		return false;
#endif
	}

	Data.MemoryCost = (size_t) (StartVASpace - GetAvailableVASpace( ));

	if ((LONG_PTR) Data.MemoryCost < 0)
		Data.MemoryCost = 0;

	m_Bridge->GetTextOut( )->WriteText(
		"NWScriptRuntime::LoadScript: Generated code for script '%s' (%lu bytes compiled script) in %lums, approximately %I64lu bytes additional VA space used.\n",
		ScriptNameStr.c_str( ),
		(unsigned long) CodeSize,
		ReadPerformanceCounterMilliseconds( ) - StartTick,
		(ULONGLONG) Data.MemoryCost);

	//
	// Cache the generated code for future use and return the newly generated
	// script program.
	//

	it = m_ScriptCache.insert( ScriptCacheMap::value_type( ResRef, Data ) ).first;

	*ScriptData = &it->second;

	return true;
}

void
NWScriptRuntime::ConvertScriptParameters(
	__out NWScriptParamVec & Params,
	__in NWN2Server::CVirtualMachine * ServerVM
	)
/*++

Routine Description:

	This routine converts from the server script VM's internal parameter
	representation to the native representation used by the execution
	environment.

Arguments:

	Params - Receives the converted parameter array.

	ServerVM - Supplies the server CVirtualMachine instance which contains the
	           script parameters requested.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	CExoArrayList< CScriptParameterWrapper > * ServerParams;
	char                                       Str[ 256 ];
	CScriptParameterWrapper                  * Param;

	ServerParams = ServerVM->GetScriptParameters( );

	if ((ServerParams == NULL) || (ServerParams->num == 0))
		return;

	Params.reserve( (size_t) ServerParams->num );

	for (int i = 0; i < ServerParams->num; i += 1)
	{
		size_t CpLen;

		Param = &ServerParams->element[ i ];

		switch (Param->m_eType)
		{

		case CScriptParameterWrapper::PT_INT:
			StringCbPrintfA(
				Str,
				sizeof( Str ),
				"%d",
				Param->m_iIntParameter);
			Params.push_back( Str );
			break;

		case CScriptParameterWrapper::PT_FLOAT:
			StringCbPrintfA(
				Str,
				sizeof( Str ),
				"%g",
				Param->m_fFloatParameter);
			Params.push_back( Str );
			break;

		case CScriptParameterWrapper::PT_STRING:
		case CScriptParameterWrapper::PT_OBJECTTAG:
			CpLen = (size_t) Param->m_cStringParameter.m_nBufferLength;

			//
			// Except for the empty string, the buffer length of a CExoString
			// includes the null terminator, which we don't want to include in the
			// body of the internal string.
			//

			while ((CpLen > 0) && (Param->m_cStringParameter.m_sString[ CpLen - 1 ] == '\0'))
				CpLen -= 1;

			Params.push_back(
				std::string(
					Param->m_cStringParameter.m_sString,
					CpLen
					)
				);
			break;

		case CScriptParameterWrapper::PT_OBJECT:
			StringCbPrintfA(
				Str,
				sizeof( Str ),
				"%lu",
				Param->m_oidObjectParameter);
			Params.push_back( Str );
			break;

		default:
			Params.push_back( "" );
			break;

		}
	}
}

void
NWScriptRuntime::CreateVM(
	)
/*++

Routine Description:

	This routine is called to create and initialize the NWScriptVM that acts as
	a fallback if JIT cannot be completed.

Arguments:

	None.

Return Value:

	None.  On failure, an std::exception is raised and m_VM is returned to NULL.

Environment:

	User mode, script runtime initialization time only.

--*/
{
	m_VM = new NWScriptVM(
		m_Bridge,
		m_TextOut,
		NWActions_NWN2,
		MAX_ACTION_ID_NWN2);

	m_VM->SetDebugLevel( m_Bridge->GetScriptDebug( ) );
}

void
NWScriptRuntime::PushStackToBridge(
	__in NWScriptStack & VMStack,
	__in ULONG SaveVarCount,
	__in NWScriptStack::STACK_POINTER Offset
	)
/*++

Routine Description:

	This routine pushes a portion of a NWScriptStack onto the server's execution
	stack.

Arguments:

	VMStack - Supplies the VM stack to push the contents of.

	SaveVarCount - Supplies the count of variables to push on to the stack.

	Offset - Supplies the offset from the top of the stack to begin copying
	         variables from.  The value must be less than or equal to zero.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	UNREFERENCED_PARAMETER( VMStack );
	UNREFERENCED_PARAMETER( SaveVarCount );
	UNREFERENCED_PARAMETER( Offset );

	throw std::runtime_error( "Not implemented." );
}

void
NWScriptRuntime::PopStackFromBridge(
	__in NWScriptStack & VMStack,
	__in ULONG SaveVarCount
	)
/*++

Routine Description:

	This routine pops data from the server's execution stack, and copies it to a
	NWScriptStack.

Arguments:

	VMStack - Supplies the VM stack that receives the copied data.

	SaveVarCount - Supplies the count of variables to push on to the stack.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	UNREFERENCED_PARAMETER( VMStack );
	UNREFERENCED_PARAMETER( SaveVarCount );

	throw std::runtime_error( "Not implemented." );
}

bool
NWScriptRuntime::ShouldJITScript(
	__in size_t CodeSize
	)
/*++

Routine Description:

	This routine selects whether a script should prefer to be JIT'd versus ran
	in the reference VM.

Arguments:

	CodeSize - Supplies the number of code bytes used for the script, in bytes.

Return Value:

	The routine returns a Boolean value indicating true if the script should be
	run with the JIT engine, else false if the script should be run with the
	reference VM.

Environment:

	User mode.

--*/
{
#if NWSCRIPTVM_FALLBACK
	if (m_JITEngine == NULL)
		return false;
#endif

	switch (m_JITPolicy->SelectEngineForScript( CodeSize ))
	{

	case INWScriptJITPolicy::NWSCRIPT_ENGINE_VM:
		return false;

	case INWScriptJITPolicy::NWSCRIPT_ENGINE_JIT:
		return true;

	default:
		return true;

	}
}

void
NWScriptRuntime::LoadSymbols(
	__inout NWScriptReader & Reader,
	__in const std::string & ScriptName
	)
/*++

Routine Description:

	This routine opportunistically attempts to load debug symbols for a script.

	Debug symbols are optional, but improve the quality of stack traces when
	errors occur.

Arguments:

	Reader - Supplies the reader context that will receive symbols data.

	ScriptName - Supplies the name of the script.

Return Value:

	None.  Failures are ignored.

Environment:

	User mode.

--*/
{
	//
	// If symbol loading is disabled, don't try and load anything.
	//

	if (!m_JITPolicy->GetLoadDebugSymbols( ))
		return;

	CVirtualMachineDebugLoader   DebugLoader;
	CExoString                   ScriptNameS;
	FILE                       * f;
	char                         TempPath[ MAX_PATH + 1 ];
	char                         FileName[ MAX_PATH + 1 ];

	ScriptNameS.m_sString       = ScriptName.c_str( );
	ScriptNameS.m_nBufferLength = ScriptName.size( ) + 1;

	if (!DebugLoader.DemandDebugInfo( &ScriptNameS ))
	{
		ScriptNameS.ReleaseOwnership( );
		return;
	}

	ScriptNameS.ReleaseOwnership( );

	if ((GetTempPathA( MAX_PATH, TempPath )) &&
	    (GetTempFileNameA( TempPath, "NDB", 0, FileName )))
	{
#if _NTDDI_VERSION >= NTDDI_VISTA
		if (!fopen_s( &f, FileName, "wb" ))
#else
		if ((f = fopen( FileName, "wb" )) != NULL)
#endif
		{
			fwrite( DebugLoader.GetNDBPtr( ), DebugLoader.GetNDBSize( ), 1, f );
			fclose( f );

			if (Reader.LoadSymbols( FileName ))
			{
				m_Bridge->GetTextOut( )->WriteText(
					"NWScriptRuntime::LoadSymbols: Loaded debug symbols for script '%s'.\n",
					ScriptName.c_str( ));
			}
		}

		_unlink( FileName );
	}

	DebugLoader.ReleaseDebugInfo( );
}

ULONG
NWScriptRuntime::ReadPerformanceCounterMilliseconds(
	) const
/*++

Routine Description:

	This routine returns an approximate count of milliseconds from the
	performance counter.

Arguments:

	None.

Return Value:

	An approximate count of milliseconds in performance counter intervals is
	returned.

Environment:

	User mode.

--*/
{
	LARGE_INTEGER PerfCounter;

	if (!QueryPerformanceCounter( &PerfCounter ))
		return 0;

	return (ULONG) (PerfCounter.QuadPart / m_PerfFrequency.QuadPart);
}

