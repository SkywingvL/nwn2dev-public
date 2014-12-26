/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptHost.cpp

Abstract:

	This module houses the NWScriptHost class.  The NWScriptHost object is
	responsible for coordinating actions in the scripting environment,
	including both calls to scripts themselves as well as the implementation of
	engine structure wrappers and engine action handlers.

--*/

#include "Precomp.h"
#include "AppParams.h"
#include "NWScriptHost.h"
#include "../NWN2DataLib/NWScriptReader.h"
#include "../NWNScriptLib/NWScriptAnalyzer.h"
#include "../NWNScriptJIT/NWScriptJITLib.h"


//
// Define to 1 to test script performance.
//

#define SCRIPT_PERF_TEST 0


NWScriptHost::NWScriptHost(
	__in ResourceManager & ResMan,
	__in swutil::TimerManager & TimerManager,
	__in const AppParameters * Params,
	__in IDebugTextOut * TextOut
	)
/*++

Routine Description:

	This routine constructs a new NWScriptHost and initializes it based on the
	parameter set passed in.

Arguments:

	ResMan - Supplies the resource system instance that is used to load script
	         resources from disk.

	TimerManager - Supplies the timer system to register timers with.

	Params - Supplies the application parameter block.

	TextOut - Supplies the text out interface.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
: m_ResourceManager( ResMan ),
  m_TimerManager( TimerManager ),
  m_TextOut( TextOut ),
  m_AppParams( Params ),
  m_VM( NULL ),
  m_JITStack( NULL ),
  m_JITEngine( NULL ),
  m_JITManagedSupport( NULL ),
  m_LastActionFromJIT( false ),
  m_JITScriptAborted( false ),
  m_CurrentScript( NULL ),
  m_CurrentJITProgram( NULL ),
  m_CurrentSelfObjectId( NWN::INVALIDOBJID )
{
	int DebugLevel;

	//
	// Set up the action table and initialize the script VM.
	//

	RegisterActions( );

	m_VM = new NWScriptVM( this, m_TextOut );

	DebugLevel = Params->GetScriptDebug( );

	if ((DebugLevel >= NWScriptVM::EDL_None) &&
	    (DebugLevel <  NWScriptVM::LastExecDebugLevel ))
	{
		m_VM->SetDebugLevel( (NWScriptVM::ExecDebugLevel) DebugLevel );
	}

	m_JITStack = new NWScriptStack( NWN::INVALIDOBJID );

	try
	{
		m_JITEngine = new NWScriptJITLib( L"NWNScriptJIT.dll" );
	}
	catch (std::exception)
	{
		m_JITEngine = NULL;
	}

	//
	// If configured, enable support for managed code scripts.
	//

	try
	{
		if (Params->GetAllowManagedScripts( ))
		{
			NWSCRIPT_JIT_PARAMS CodeGenParams;

			ZeroMemory( &CodeGenParams, sizeof( CodeGenParams ) );

			CodeGenParams.Size             = sizeof( CodeGenParams );
			CodeGenParams.CodeGenFlags     = NWCGF_NWN_COMPATIBLE_ACTIONS;

			m_JITManagedSupport = m_JITEngine->CreateManagedSupportPtr(
				NWActions_NWN2,
				MAX_ACTION_ID_NWN2,
				0,
				m_TextOut,
				DebugLevel,
				this,
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
}

NWScriptHost::~NWScriptHost(
	)
/*++

Routine Description:

	This routine deletes the current NWScriptHost object and its associated
	members.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	//
	// Tear down any lingering script situations, then the script VM itself.
	//

	m_DeferredSituations.clear( );
	m_PendingDeferredSituations.clear( );

	m_VM        = NULL;
	m_JITStack  = NULL;
	m_JITEngine = NULL;
}

int
NWScriptHost::RunScript(
	__in const char * ScriptName,
	__in NWN::OBJECTID ObjectId,
	__in const ScriptParamVec & ScriptParameters,
	__in int DefaultReturnCode, /*= 0 */
	__in ULONG Flags /* = 0 */
	)
/*++

Routine Description:

	This routine executes a server-side script and returns the result of that
	script execution (note that not all scripts return a value).

Arguments:

	ScriptName - Supplies the resource name if the script.

	ObjectId - Optionally supplies the 'self object' for the script execution.

	ScriptParameters - Optionally supplies parameters for the script.  Only
	                   scripts that expect parameters may be called with a set
	                   of script parameters.

	DefaultReturnCode - Supplies the default return code on an error condition,
	                    or if the script did not return a value.

	Flags - Supplies flags that control the execution environment of the
	        script.

Return Value:

	The return value is the script return code.  If the execution failed or the
	script did not return a value, the return value is zero.

Environment:

	User mode.

--*/
{
	NWN::OBJECTID ObjectSelf;

	if (*ScriptName == '\0')
		return DefaultReturnCode;

	if (ObjectId != NWN::INVALIDOBJID)
		ObjectSelf = ObjectId & ~NWN::LISTTYPE_MASK;
	else
		ObjectSelf = NWN::INVALIDOBJID;

	try
	{
		//
		// Save the currently executing script on the stack (because we might
		// be a recursive call), then load the new script and transfer control
		// to the VM.
		//

		NWScriptReaderPtr            PrevScript  = m_CurrentScript;
		NWScriptJITLib::Program::Ptr PrevProgram = m_CurrentJITProgram;
		NWN::OBJECTID                PrevSelf    = m_CurrentSelfObjectId;
		int                          ReturnCode;
		LARGE_INTEGER                PerfFreq;
		LARGE_INTEGER                PerfStart;
		LARGE_INTEGER                PerfEnd;

		QueryPerformanceFrequency( &PerfFreq );

		try
		{
			m_CurrentScript       = LoadScript( ScriptName, m_CurrentJITProgram );
			m_CurrentSelfObjectId = ObjectSelf;

			QueryPerformanceCounter( &PerfStart );

			ReturnCode = 0;
			
#if SCRIPT_PERF_TEST
			for (int i = 0; i < 1000000; i += 1)
#endif
			{
				if (m_CurrentJITProgram.get( ) != NULL)
				{
					ReturnCode = m_CurrentJITProgram->ExecuteScript(
						m_JITStack.get( ),
						ObjectSelf,
						ScriptParameters,
						DefaultReturnCode,
						Flags);
				}
				else
				{
					ReturnCode = m_VM->ExecuteScript(
						m_CurrentScript,
						ObjectSelf,
						NWN::INVALIDOBJID,
						ScriptParameters,
						DefaultReturnCode,
						Flags);
				}

#if SCRIPT_PERF_TEST
				if (PrevScript.get( ) != NULL)
					break;
#endif
			}

			QueryPerformanceCounter( &PerfEnd );
		}
		catch (...)
		{
			m_CurrentSelfObjectId = PrevSelf;
			m_CurrentScript       = PrevScript;
			m_CurrentJITProgram   = PrevProgram;
			PrevSelf              = NULL;
			PrevScript            = NULL;
			throw;
		}

		m_TextOut->WriteText(
			"Execution finished (time = %I64lums).\n",
			(PerfEnd.QuadPart - PerfStart.QuadPart) / (PerfFreq.QuadPart / 1000));

#if 0

		try
		{
			NWScriptAnalyzer Analyzer(
				m_TextOut,
				NWActions_NWN2,
				MAX_ACTION_ID_NWN2);

			Analyzer.Analyze( m_CurrentScript.get( ) );
		}
		catch (std::exception &e)
		{
			m_TextOut->WriteText(
				"WARNING: NWScriptHost::RunScript( %s, %08X ): Exception '%s' analyzing script.\n",
				ScriptName,
				ObjectSelf,
				e.what( ));
		}

		try
		{
			if (m_JITEngine.get( ) == NULL)
				throw std::exception( "JIT not installed." );

			m_TextOut->WriteText( "Generating code...\n" );

			QueryPerformanceCounter( &PerfStart );

			NWScriptJITLib::Program Program(
				m_JITEngine->GenerateCode(
					m_CurrentScript.get( ),
					NWActions_NWN2,
					MAX_ACTION_ID_NWN2,
					NWScriptAnalyzer::AF_NO_OPTIMIZATIONS,
					m_TextOut,
					(ULONG) m_AppParams->GetScriptDebug( ),
					this,
					NWN::INVALIDOBJID
					)
				);

			QueryPerformanceCounter( &PerfEnd );

			m_TextOut->WriteText(
				"Code generation complete (time = %I64lums).\n",
				(PerfEnd.QuadPart - PerfStart.QuadPart) / (PerfFreq.QuadPart / 1000));

			for (size_t i = 0; i < 10; i += 1)
			{
				m_TextOut->WriteText( "Executing code...\n" );

				QueryPerformanceCounter( &PerfStart );

				Program.ExecuteScript(
					m_JITStack.get( ),
					NWN::INVALIDOBJID,
					ScriptParameters,
					DefaultReturnCode,
					Flags);

				QueryPerformanceCounter( &PerfEnd );

				m_TextOut->WriteText(
					"Code execution complete (time = %I64lums).\n",
					(PerfEnd.QuadPart - PerfStart.QuadPart) / (PerfFreq.QuadPart / 1000));
			}
		}
		catch (std::exception &e)
		{
			m_TextOut->WriteText(
				"WARNING: NWScriptHost::RunScript( %s, %08X ): Exception '%s' executing script via JIT.\n",
				ScriptName,
				ObjectSelf,
				e.what( ));
		}
#endif

		m_CurrentSelfObjectId = PrevSelf;
		m_CurrentScript       = PrevScript;
		m_CurrentJITProgram   = PrevProgram;
		PrevSelf              = NULL;
		PrevScript            = NULL;
		PrevProgram           = NULL;
		m_JITScriptAborted    = false;

		return ReturnCode;
	}
	catch (std::exception &e)
	{
		m_TextOut->WriteText(
			"WARNING: NWScriptHost::RunScript( %s, %08X ): Exception '%s' executing script.\n",
			ScriptName,
			ObjectSelf,
			e.what( ));

		return DefaultReturnCode;
	}
}

int
NWScriptHost::RunScript(
	__in const NWN::ResRef32 & ScriptName,
	__in NWN::OBJECTID ObjectId,
	__in const ScriptParamVec & ScriptParameters,
	__in int DefaultReturnCode, /* = 0 */
	__in ULONG Flags /* = 0 */
	)
/*++

Routine Description:

	This routine executes a server-side script and returns the result of that
	script execution (note that not all scripts return a value).

Arguments:

	ScriptName - Supplies the resource name if the script.

	ObjectId - Optionally supplies the 'self object' for the script execution.

	ScriptParameters - Optionally supplies parameters for the script.  Only
	                   scripts that expect parameters may be called with a set
	                   of script parameters.

	DefaultReturnCode - Supplies the default return code on an error condition,
	                    or if the script did not return a value.

	Flags - Supplies flags that control the execution environment of the
	        script.

Return Value:

	The return value is the script return code.  If the execution failed or the
	script did not return a value, the return value is the default return code.

Environment:

	User mode.

--*/
{
	char ResourceName[ sizeof( NWN::ResRef32 ) + 1 ];

	memcpy( ResourceName, &ScriptName, sizeof( NWN::ResRef32 ) );
	ResourceName[ sizeof( NWN::ResRef32 ) ] = '\0';

	return RunScript(
		ResourceName,
		ObjectId,
		ScriptParameters,
		DefaultReturnCode,
		Flags);
}

void
NWScriptHost::RunScriptSituation(
	__in NWScriptVM::VMState * ScriptState,
	__in NWScriptJITLib::SavedState::Ptr & ScriptStateJIT,
	__in NWScriptJITLib::Program::Ptr & ProgramJIT
	)
/*++

Routine Description:

	This routine executes a server-side script situation, which is a
	contiuation of a previous script (generally, a small section of code that 
	is responsible for implementing an "action" object in NWScript).

Arguments:

	ScriptState - Supplies the saved script state to execute.

	ScriptStateJIT - Optionally supplies a JIT'd saved state to execute.

	ProgramJIT - Optionally supplies a JIT'd program to execute the saved state
	             for.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	try
	{
		//
		// Save the currently executing script on the stack (because we might
		// be a recursive call), then transfer control to the VM.
		//

		NWScriptReaderPtr            PrevScript  = m_CurrentScript;
		NWScriptJITLib::Program::Ptr PrevProgram = m_CurrentJITProgram;
		NWN::OBJECTID                PrevSelf    = m_CurrentSelfObjectId;

		try
		{
			m_CurrentScript       = ScriptState->Script;
			m_CurrentJITProgram   = ProgramJIT;
			m_CurrentSelfObjectId = ScriptState->ObjectSelf;

			if (ScriptStateJIT.get( ) != NULL)
			{
				ScriptStateJIT->ExecuteScriptSituation(
					ScriptState->ObjectSelf);
			}
			else
			{
				m_VM->ExecuteScriptSituation( *ScriptState );
			}
		}
		catch (...)
		{
			m_CurrentSelfObjectId = PrevSelf;
			m_CurrentJITProgram   = PrevProgram;
			m_CurrentScript       = PrevScript;
			PrevSelf              = NULL;
			PrevScript            = NULL;
			throw;
		}

		m_CurrentSelfObjectId = PrevSelf;
		m_CurrentJITProgram   = PrevProgram;
		m_CurrentScript       = PrevScript;
		PrevSelf              = NULL;
		PrevScript            = NULL;
	}
	catch (std::exception &e)
	{
		m_TextOut->WriteText(
			"WARNING: NWScriptHost::RunScriptSituation( %s, %08X ): Exception '%s' executing script situation.\n",
			ScriptState->Script->GetScriptName( ).c_str( ),
			ScriptState->ObjectSelf,
			e.what( ));
	}
}

void
NWScriptHost::ClearScriptCache(
	)
/*++

Routine Description:

	This routine clears the script cache of all entries.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	m_ScriptCache.clear( );
}

bool
NWScriptHost::InitiatePendingDeferredScriptSituations(
	)
/*++

Routine Description:

	This routine starts the timers on all pending deferred script situations,
	and transfers them to the main deferred list.

Arguments:

	None.

Return Value:

	The routine returns true if any pending deferred actions were transferred
	over to the deferred list (and thus had their timers started).

Environment:

	User mode.

--*/
{
	bool AnyActions;

	AnyActions = (!m_PendingDeferredSituations.empty( ));

	//
	// Start all of the timers going.
	//

	for (DeferredScriptSitList::iterator it = m_PendingDeferredSituations.begin( );
	     it != m_PendingDeferredSituations.end( );
	     ++it)
	{
		(*it)->Timer->SetPeriod( (*it)->DuePeriod );
	}

	//
	// Now transfer the list entries over.
	//

	m_DeferredSituations.splice(
		m_DeferredSituations.begin( ),
		m_PendingDeferredSituations);

	return AnyActions;
}

void
NWSCRIPTACTAPI
NWScriptHost::OnExecuteAction(
	__in NWScriptVM & ScriptVM,
	__in NWScriptStack & VMStack,
	__in NWSCRIPT_ACTION ActionId,
	__in size_t NumArguments
	)
/*++

Routine Description:

	This routine is invoked by the script instruction stream when an engine
	action invocation is requested.  The routine dispatches to the action
	handler for this action service ordinal in the handler table.

	The routine is responsible for removing all arguments from the script
	stack, and then placing the return value (if any) on the script stack.

Arguments:

	ScriptVM - Supplies the currently executing script VM.

	VMStack - Supplies the currently executing script stack.

	ActionId - Supplies the action service ordinal that was requested.

	NumArguments - Supplies the count of arguments passed to the action ordinal.

Return Value:

	None.

Environment:

	User mode, called from script VM.

--*/
{
	const NWScriptActionEntry * ActionEntry;

	m_LastActionFromJIT = false;

	if (ActionId < MAX_ACTION_ID)
		ActionEntry = &m_ActionHandlerTable[ ActionId ];
	else
		ActionEntry = NULL;

	if (ScriptVM.IsDebugLevel( NWScriptVM::EDL_Calls ))
	{
		m_TextOut->WriteText(
			"NWScriptHost::OnExecuteAction: Executing action %s (%lu) with %lu arguments.\n",
			ActionEntry != NULL ? ActionEntry->ActionName : "<INVALID>",
			ActionId,
			(unsigned long) NumArguments);
	}

	if (ActionEntry == NULL)
	{
		ScriptVM.AbortScript( );
	}
	else
	{
		try
		{
			(this->*ActionEntry->ActionHandler)(
				ScriptVM,
				VMStack,
				ActionId,
				NumArguments);
		}
		catch (std::exception &e)
		{
			if (ScriptVM.IsDebugLevel( NWScriptVM::EDL_Errors ))
			{
				m_TextOut->WriteText(
					"NWScriptHost::OnExecuteAction: Exception '%s' executing action %s (%lu).\n",
					e.what( ),
					ActionEntry->ActionName,
					ActionId);
			}

			ScriptVM.AbortScript( );
		}
	}
}

bool
NWSCRIPTACTAPI
NWScriptHost::OnExecuteActionFromJIT(
	__in NWSCRIPT_ACTION ActionId,
	__in size_t NumArguments
	)
/*++

Routine Description:

	This routine is invoked by the script instruction stream when an engine
	action invocation is requested.  The routine dispatches to the action
	handler for this action service ordinal in the handler table.

	The routine is responsible for removing all arguments from the script
	stack, and then placing the return value (if any) on the script stack.

Arguments:

	ActionId - Supplies the action service ordinal that was requested.

	NumArguments - Supplies the count of arguments passed to the action ordinal.

Return Value:

	The routine returns true if the action was executed, else false if a fatal
	error occured.

Environment:

	User mode, called from script JIT code.

--*/
{
	const NWScriptActionEntry * ActionEntry;

	m_LastActionFromJIT = true;

	if (ActionId < MAX_ACTION_ID)
		ActionEntry = &m_ActionHandlerTable[ ActionId ];
	else
		ActionEntry = NULL;

	if (m_VM->IsDebugLevel( NWScriptVM::EDL_Calls ))
	{
		m_TextOut->WriteText(
			"NWScriptHost::OnExecuteActionFromJIT: Executing action %s (%lu) with %lu arguments.\n",
			ActionEntry != NULL ? ActionEntry->ActionName : "<INVALID>",
			ActionId,
			(unsigned long) NumArguments);
	}

	if (ActionEntry == NULL)
	{
		return false;
	}
	else
	{
		try
		{
			(this->*ActionEntry->ActionHandler)(
				*m_VM,
				*m_JITStack,
				ActionId,
				NumArguments);
		}
		catch (std::exception &e)
		{
			if (m_VM->IsDebugLevel( NWScriptVM::EDL_Errors ))
			{
				m_TextOut->WriteText(
					"NWScriptHost::OnExecuteActionFromJIT: Exception '%s' executing action %s (%lu).\n",
					e.what( ),
					ActionEntry->ActionName,
					ActionId);
			}

			return false;
		}
	}

	return !m_JITScriptAborted;
}

bool
NWSCRIPTACTAPI
NWScriptHost::OnExecuteActionFromJITFast(
	__in NWSCRIPT_ACTION ActionId,
	__in size_t NumArguments,
	__in_ecount( NumCmds ) PCNWFASTACTION_CMD Cmds,
	__in size_t NumCmds,
	__in uintptr_t * CmdParams
	)
/*++

Routine Description:

	This routine is invoked by the script instruction stream when an engine
	action invocation is requested.  The routine dispatches to the action
	handler for this action service ordinal in the handler table.

	Unlike a standard action service handler call, the arguments are passed via
	a direct array and not via the VM stack.

Arguments:

	ActionId - Supplies the action service ordinal that was requested.

	NumArguments - Supplies the count of arguments passed to the action ordinal.

	Cmds - Supplies the array of fast action commands describing how to pass
	       the parameters to the action handler.

	NumCmds - Supplies the count of fast action commands.

	CmdParams - Supplies the array of fast action command arguments, which are
	            interpreted based on the Cmds array.

Return Value:

	The routine returns true if the action was executed, else false if a fatal
	error occured.

Environment:

	User mode, called from script JIT code.

--*/
{
	const NWScriptActionEntry * ActionEntry;

	m_LastActionFromJIT = true;

	if (ActionId < MAX_ACTION_ID)
		ActionEntry = &m_ActionHandlerTable[ ActionId ];
	else
		ActionEntry = NULL;

	if (m_VM->IsDebugLevel( NWScriptVM::EDL_Calls ))
	{
		m_TextOut->WriteText(
			"NWScriptHost::OnExecuteActionFromJITFast: Executing action %s (%lu) with %lu arguments.\n",
			ActionEntry != NULL ? ActionEntry->ActionName : "<INVALID>",
			ActionId,
			(unsigned long) NumArguments);
	}

	if (ActionEntry == NULL)
	{
		return false;
	}
	else
	{
		try
		{
			for (size_t i = 0; i < NumCmds; i += 1)
			{
				switch (Cmds[ i ])
				{

				case NWFASTACTION_PUSHINT:
					m_JITStack->StackPushInt( (int) *CmdParams++ );
					break;

				case NWFASTACTION_POPINT:
					**(int **) CmdParams++ = m_JITStack->StackPopInt( );
					break;

				case NWFASTACTION_PUSHFLOAT:
					m_JITStack->StackPushFloat( *(float *) &*CmdParams++ );
					break;
					
				case NWFASTACTION_POPFLOAT:
					**(float **) CmdParams++ = m_JITStack->StackPopFloat( );
					break;

				case NWFASTACTION_PUSHOBJECTID:
					m_JITStack->StackPushObjectId( (NWN::OBJECTID) *CmdParams++ );
					break;

				case NWFASTACTION_POPOBJECTID:
					**(NWN::OBJECTID **) CmdParams++ = m_JITStack->StackPopObjectId( );
					break;

				case NWFASTACTION_PUSHSTRING:
					m_JITStack->StackPushStringAsNeutral(
						**(const NWScriptStack::NeutralString **) CmdParams++ );
					break;

				case NWFASTACTION_POPSTRING:
					**(NWScriptStack::NeutralString **) CmdParams++ = m_JITStack->StackPopStringAsNeutral( );
					break;

				case NWFASTACTION_CALL:
					(this->*ActionEntry->ActionHandler)(
						*m_VM,
						*m_JITStack,
						ActionId,
						NumArguments);
					break;

				default:
					throw std::runtime_error( "Unrecognized fast action command." );

				}
			}
		}
		catch (std::exception &e)
		{
			if (m_VM->IsDebugLevel( NWScriptVM::EDL_Errors ))
			{
				m_TextOut->WriteText(
					"NWScriptHost::OnExecuteActionFromJITFast: Exception '%s' executing action %s (%lu).\n",
					e.what( ),
					ActionEntry->ActionName,
					ActionId);
			}

			return false;
		}
	}

	return !m_JITScriptAborted;
}

EngineStructurePtr
NWSCRIPTACTAPI
NWScriptHost::CreateEngineStructure(
	__in NWScriptStack::ENGINE_STRUCTURE_NUMBER EngineType
	)
/*++

Routine Description:

	This routine is invoked by the script instruction stream when an empty
	engine structure of the given type is needed.  The routine is responsible
	for creating the structure and returning it to the script environment.

Arguments:

	EngineType - Supplies the engine structure ordinal, which must match up
	             with the EngType* family of constants.

Return Value:

	The routine returns the engine structure on success, else NULL on failure.

Environment:

	User mode, called from script VM.

--*/
{
	switch (EngineType)
	{

	case EngTypeEffect:
		return new EngEffect( );

	default:
		return EngineStructurePtr( NULL );

	}
}

void
NWScriptHost::RegisterActions(
	)
/*++

Routine Description:

	This routine is called to initialize the action handler table in the
	script host.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode, script host initialization time only.

--*/
{
	//
	// The following macro definition is interpreted once for each action that
	// is defined in the system.  The macro sets up the table registration for
	// that action.
	//

#define DECLARE_NSS_HANDLER( Name, Ordinal )                                        \
	m_ActionHandlerTable[ Ordinal ].ActionHandler = &NWScriptHost::OnAction_##Name; \
    m_ActionHandlerTable[ Ordinal ].ActionId      = Ordinal;                        \
    m_ActionHandlerTable[ Ordinal ].ActionName    = #Name;                  

#include "NWScriptActionDefs.h"
#undef DECLARE_NSS_HANDLER
}

NWScriptHost::NWScriptReaderPtr
NWScriptHost::LoadScript(
	__in const char * ScriptName,
	__out NWScriptJITLib::Program::Ptr & JITProgram
	)
/*++

Routine Description:

	This routine is invoked to load a script (from cache or disk).  The script
	is fetched and returned to the caller.

Arguments:

	ScriptName - Supplies the name of the script.

	JITProgram - Receives the JIT'd program handle cached for the script, if
	             any existed.

Return Value:

	The routine returns the script contents on success.  On failure, an
	std::exception is raised.

Environment:

	User mode.

--*/
{
	NWN::ResRef32            ResRef;
	ScriptCacheMap::iterator it;
	NWScriptReaderPtr        Script;

	//
	// Convert the name to a canonical resref and search for it in our cache.
	//

	ResRef = m_ResourceManager.ResRef32FromStr( ScriptName );

	it = m_ScriptCache.find( ResRef );

	if (it != m_ScriptCache.end( ))
	{
		JITProgram = it->second.JITProgram;
		return it->second.Reader;
	}

	//
	// If we didn't have the script cached, then we shall have to load it in
	// from disk directly.  Do so here and now.
	//

	try
	{
		DemandResource32  Res( m_ResourceManager, ResRef, NWN::ResNCS );
		ScriptCacheData   Data;

		Script = new NWScriptReader( Res.GetDemandedFileName( ).c_str( ) );

		//
		// Cache the script for future use and return it.
		//

		Script->SetScriptName( ScriptName );

		//
		// Load debug symbols if we have them, but ignore any failures during their
		// processing as debug symbols are optional.
		//

		if (m_ResourceManager.ResourceExists( ResRef, NWN::ResNDB ))
		{
			try
			{
				DemandResource32  NDBRes(
					m_ResourceManager,
					ResRef,
					NWN::ResNDB);

				Script->LoadSymbols( NDBRes.GetDemandedFileName( ).c_str( ) );
			}
			catch (std::exception)
			{
			}
		}

		Data.Reader = Script;

		it = m_ScriptCache.insert( ScriptCacheMap::value_type( ResRef, Data ) ).first;
	}
	catch (std::exception)
	{
		std::string       DirectScriptStr;
		std::string       RealScriptName;
		const char      * p;
		size_t            Offs;
		ScriptCacheData   Data;

		//
		// For the console script host, allow a script in the working directory
		// to be used directly even if we had no module loaded.  Normally, we
		// would only load scripts via the resource system.
		//

		DirectScriptStr  = ScriptName;

		if (DirectScriptStr.find( '.' ) == std::string::npos)
		{
			DirectScriptStr += ".";
			DirectScriptStr += m_ResourceManager.ResTypeToExt( NWN::ResNCS );
		}

		Script = new NWScriptReader( DirectScriptStr.c_str( ) );

		//
		// Cache the script for future use and return it.
		//

		if ((p = strrchr( ScriptName, '\\' )) != NULL)
			RealScriptName = p + 1;
		else
			RealScriptName = ScriptName;

		if ((Offs = RealScriptName.find( '.' )) != std::string::npos)
			RealScriptName = RealScriptName.substr( 0, Offs );

		Script->SetScriptName( RealScriptName.c_str( ) );

		DirectScriptStr  = ScriptName;

		if ((Offs = DirectScriptStr.find( '.' )) == std::string::npos)
			DirectScriptStr += ".";
		else
			DirectScriptStr.erase( Offs + 1 );

		DirectScriptStr += m_ResourceManager.ResTypeToExt( NWN::ResNDB );

		//
		// Load debug symbols if we have them, but ignore any failures during their
		// processing as debug symbols are optional.
		//

		if (!_access( DirectScriptStr.c_str( ), 00 ))
		{
			try
			{
				Script->LoadSymbols( DirectScriptStr.c_str( ) );
			}
			catch (std::exception)
			{
			}
		}

		Data.Reader = Script;

		it = m_ScriptCache.insert( ScriptCacheMap::value_type( ResRef, Data ) ).first;
	}

	if (m_JITEngine.get( ) != NULL)
	{
		try
		{
			NWSCRIPT_JIT_PARAMS CodeGenParams;

			ZeroMemory( &CodeGenParams, sizeof( CodeGenParams ) );

			CodeGenParams.Size             = sizeof( CodeGenParams );
			CodeGenParams.CodeGenFlags     = NWCGF_SAVE_OUTPUT | NWCGF_ENABLE_SAVESTATE_TO_VMSTACK | NWCGF_NWN_COMPATIBLE_ACTIONS;
			CodeGenParams.CodeGenOutputDir = NULL;

			if (m_JITManagedSupport.get( ) != NULL)
			{
				CodeGenParams.CodeGenFlags   |= NWCGF_MANAGED_SCRIPT_SUPPORT;
				CodeGenParams.ManagedSupport  = m_JITManagedSupport->GetManagedSupport( );
			}

			it->second.JITProgram = m_JITEngine->GenerateCodePtr(
				Script.get( ),
				NWActions_NWN2,
				MAX_ACTION_ID_NWN2,
				NWScriptAnalyzer::AF_NO_OPTIMIZATIONS,
				m_TextOut,
				(ULONG) m_AppParams->GetScriptDebug( ),
				this,
				NWN::INVALIDOBJID,
				&CodeGenParams
				);
		}
		catch (std::exception &e)
		{
			if (m_AppParams->GetScriptDebug( ) >= NWScriptVM::EDL_Errors)
			{
				m_TextOut->WriteText(
					"JIT failed for program '%s': Exception '%s'.\n",
					ScriptName,
					e.what( ));
			}
		}
	}

	JITProgram = it->second.JITProgram;

	return Script;
}

NWN::OBJECTID
NWScriptHost::GetCurrentActionObjectId(
	)
/*++

Routine Description:

	This routine returns the current game object (if any) that the action
	handler that is executing should act on.

Arguments:

	None.

Return Value:

	The routine returns the 'self' game object id for the current script, else
	else NWN::INVALIDOBJID if there was no such object.

Environment:

	User mode, called from action handlers only, before recursion.

--*/
{
	return m_CurrentSelfObjectId;
}

void
NWScriptHost::CreateDeferredScriptSituation(
	__in NWScriptVM & ScriptVM,
	__in NWN::OBJECTID ObjectId,
	__in ULONG DuePeriod
	)
/*++

Routine Description:

	This routine creates a deferred script situation, which is a timer-based
	deferral of script execution continuation.  (The timer may be set to expire
	immediately.)

	The deferred script situation only runs once control returns to the main
	loop and thus timer completion has been enabled.  Thus, deferred script
	situations provide a useful means to break out large sections of work along
	several execution steps.

	N.B.  While deferred script situations cannot be directly aborted, they are
	      never executed should an object be deleted.  Thus, no resources may
	      be held by a deferred script situation unless those resources are
	      automatically cleaned up on object deletion.

Arguments:

	ScriptVM - Supplies the active script VM context.  The VM must have a saved
	           script situation state ready for use.

	ObjectId - Supplies the self object for which the script situation is
	           associated with.

	DuePeriod - Supplies a count, in milliseconds, of the time from the current
	            time when the deferred script situation should run.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	DeferredScriptSituation::Ptr Situation = new DeferredScriptSituation;

	if (m_LastActionFromJIT)
	{
		Situation->ProgramJIT              = m_CurrentJITProgram;
		Situation->ScriptSituationJIT      = m_CurrentJITProgram->CreateSavedStatePtr( );
		Situation->ScriptSituation.Script  = m_CurrentScript;
	}
	else
	{
		Situation->ScriptSituation = ScriptVM.GetSavedState( );
	}

	Situation->ScriptSituation.ObjectSelf = ObjectId;

	Situation->Timer = m_TimerManager.CreateTimer(
		&OnDeferredScriptSituationTimerThunk,
		this,
		Situation.get( ));

	Situation->DuePeriod = max( DuePeriod, 1 );

	//
	// Push the situation onto the pending list and do not start the timer now.
	//
	// Instead, we must wait until the main loop clears.  Otherwise, we could
	// get into a state where a script situation queues an immediate expiration
	// timer continually, which would prevent us from ever exiting the timer
	// dispatcher.
	//

	m_PendingDeferredSituations.push_back( Situation );
}

bool
NWScriptHost::OnDeferredScriptSituationTimer(
	__in DeferredScriptSituation * Situation
	)
/*++

Routine Description:

	This routine is called when a deferred script situation timer elapses.  The
	routine calls in to the script VM to execute the situation, then deletes
	the saved situation.

Arguments:

	Situation - Supplies the deferred script situation to execute.

Return Value:

	The routine returns false to indicate that the timer has been deleted.

Environment:

	User mode, called from timer dispatcher.

--*/
{
	RunScriptSituation(
		&Situation->ScriptSituation,
		Situation->ScriptSituationJIT,
		Situation->ProgramJIT);

	Situation->Timer->Deactivate( );

	for (DeferredScriptSitList::iterator it = m_DeferredSituations.begin( );
	     it != m_DeferredSituations.end( );
	     ++it)
	{
		if (it->get( ) == Situation)
		{
			m_DeferredSituations.erase( it );
			return false;
		}
	}

	//
	// We should never have a script situation timer fire while the list is out
	// of sync.
	//

	NWN_ASSERT( 0 );

	return false;
}

bool
__stdcall
NWScriptHost::OnDeferredScriptSituationTimerThunk(
	__in void * Context1,
	__in void * Context2,
	__in swutil::TimerRegistration * Timer
	)
/*++

Routine Description:

	This routine is called when a deferred script situation timer elapses.  The
	routine calls in to the script VM to execute the situation, then deletes
	the saved situation.

	The routine thunks to the class function.

Arguments:

	Context1 - Supplies the NWScriptHost this pointer.

	Context2 - Supplies the DeferredScriptSituation instance pointer.

	Timer - Supplies the timer object that fired.

Return Value:

	The routine returns false to indicate that the timer has been deleted.

Environment:

	User mode, called from timer dispatcher.

--*/
{
	NWScriptHost            * This;
	DeferredScriptSituation * Situation;

	UNREFERENCED_PARAMETER( Timer );

	This      = reinterpret_cast< NWScriptHost * >( Context1 );
	Situation = reinterpret_cast< DeferredScriptSituation * >( Context2 );

	return This->OnDeferredScriptSituationTimer( Situation );
}
