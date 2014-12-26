/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptRuntime.h

Abstract:

	This module defines the NWScriptRuntime object, which bears responsibility
	for executing scripts via the replacement execution environment.

--*/

#ifndef _SOURCE_PROGRAMS_AURORASERVERNWSCRIPT_NWSCRIPTRUNTIME_H
#define _SOURCE_PROGRAMS_AURORASERVERNWSCRIPT_NWSCRIPTRUNTIME_H

#ifdef _MSC_VER
#pragma once
#endif

namespace NWN2Server
{
	class CVirtualMachine;
	class CVirtualMachineScript;
	class CVirtualMachineFile;
	class CVirtualMachineStack;
	class CVirtualMachineCmdImplementer;
}

class NWScriptBridge;
class INWScriptJITPolicy;

class NWScriptRuntime
{

public:

	inline
	NWScriptRuntime(
		__in IDebugTextOut * TextOut,
		__in NWScriptBridge * Bridge,
		__in const char * DllDir,
		__in INWScriptJITPolicy * JITPolicy
		)
	: m_TextOut( TextOut ),
	  m_Bridge( Bridge ),
	  m_ScriptEngineName( "none" ),
     m_JITEngine( NULL ),
     m_JITManagedSupport( NULL ),
	  m_CurrentJITProgram( NULL ),
	  m_CurrentScriptCodeSize( 0 ),
	  m_FirstScript( true ),
	  m_DllDir( DllDir ),
	  m_VM( NULL ),
	  m_JITPolicy( JITPolicy ),
	  m_RecursionLevel( 0 ),
	  m_TotalScriptRuntime( 0 )
	{
		ZeroMemory( &m_CurrentScriptName, sizeof( m_CurrentScriptName ) );

		//
		// Read the performance counter frequency and map it to a count of
		// performance counter intervals per millisecond.
		//

		if (!QueryPerformanceFrequency( &m_PerfFrequency ))
			m_PerfFrequency.QuadPart = 1;
		else
			m_PerfFrequency.QuadPart /= 1000;

		//
		// Load the JIT system and the VM.
		//

		LoadJITEngine( DllDir );
		CreateVM( );
	}

	virtual
	~NWScriptRuntime(
		)
	{
		m_ScriptCache.clear( );

		if (m_VM != NULL)
		{
			delete m_VM;
			m_VM = NULL;
		}
	}

	//
	// Store the current saved state to the server's VM stack.
	//

	void
	StoreSavedStateToStack(
		__in NWN2Server::CVirtualMachine * ServerVM
		);

	//
	// Execute the server VM's current script.
	//

	void
	ExecuteScriptForServer(
		__in NWSCRIPT_PROGRAM_COUNTER PC,
		__inout_ecount( CodeSize ) unsigned char * InstructionStream,
		__in int CodeSize,
		__in NWN2Server::CVirtualMachine * ServerVM
		);

	//
	// Log statistics to the debug console.
	//

	void
	DumpStatistics(
		);

	//
	// Clear the script cache out, deleting any JIT'd scripts that had been
	// previously converted to native code.
	//

	void
	ClearScriptCache(
		);

	//
	// Called to set the debug tracing level.
	//

	void
	SetDebugLevel(
		__in NWScriptVM::ExecDebugLevel DebugLevel
		);

private:

	enum
	{
		SAVED_STATE_ID = 'NSSJ'
	};

	typedef swutil::SharedPtr< NWScriptReader > NWScriptReaderPtr;
	typedef swutil::SharedPtr< NWScriptVM > NWScriptVMPtr;
	typedef swutil::SharedPtr< NWScriptStack > NWScriptStackPtr;
	typedef swutil::SharedPtr< NWScriptJITLib > NWScriptJITLibPtr;
	typedef swutil::SharedPtr< NWScriptJITManagedSupport > NWScriptJITManagedSupportPtr;

	struct ScriptCacheData
	{
		bool                         BrokenScript;
		bool                         FirstRun;
		NWScriptReaderPtr            Reader;
		NWScriptJITLib::Program::Ptr JITProgram;
		size_t                       CallCount;
		size_t                       ScriptSituationCount;
		size_t                       MemoryCost;
		ULONG                        Runtime;
		size_t                       RecursionLevel;
	};

	struct ScriptResumeData
	{
		NWScriptVM::VMState::Ptr        ScriptSituation;
		NWScriptJITLib::SavedState::Ptr ScriptSituationJIT;
	};

	//
	// Comparison predicate for ResRefModelMap.
	//

	struct ResRefLess : public std::binary_function< const NWN::ResRef32 &, const NWN::ResRef32 &, bool >
	{

		inline
		bool
		operator()(
			__in const NWN::ResRef32 & left,
			__in const NWN::ResRef32 & right
			) const
		{
			return memcmp( &left, &right, sizeof( left ) ) < 0;
		}

	};

	typedef std::map< NWN::ResRef32, ScriptCacheData, ResRefLess > ScriptCacheMap;

	class DllDirectoryHolder
	{

	public:

		inline
		DllDirectoryHolder(
			__in const char * Dir
			)
		{
			SetDllDirectoryA( Dir );
		}

		inline
		~DllDirectoryHolder(
			)
		{
			SetDllDirectoryA( NULL );
		}

	};


	//
	// Load the JIT engine.
	//

	void
	LoadJITEngine(
		__in const char * DllDir
		);

	//
	// Create a saved state from the VM stack.
	//

	void
	RestoreSavedStateFromStack(
		__in NWN2Server::CVirtualMachine * ServerVM,
		__in_ecount( CodeSize ) const unsigned char * InstructionStream,
		__in size_t CodeSize,
		__deref_out ScriptCacheData * * ScriptData,
		__out ScriptResumeData * ResumeData,
		__out NWSCRIPT_PROGRAM_COUNTER & PC,
		__out std::string & ScriptName
		);

	//
	// Load a script program.
	//

	bool
	LoadScript(
		__in const NWN::ResRef32 & ScriptName,
		__in_ecount( CodeSize ) const unsigned char * InstructionStream,
		__in size_t CodeSize,
		__deref_out ScriptCacheData * * ScriptData
		);

	//
	// Convert script parameters from the server's internal representation to
	// the native representation used by the execution environment.
	//

	void
	ConvertScriptParameters(
		__out NWScriptParamVec & Params,
		__in NWN2Server::CVirtualMachine * ServerVM
		);

	//
	// Create the NWScript VM, used for interpreter fallback.
	//

	void
	CreateVM(
		);

	//
	// Transfer stack contents from the NWScript bridge into a VM stack.
	//

	void
	PopStackFromBridge(
		__out NWScriptStack & VMStack,
		__in ULONG SaveVarCount
		);

	//
	// Transfer stack contents from a VM stack to the NWScript bridge.
	//

	void
	PushStackToBridge(
		__in NWScriptStack & VMStack,
		__in ULONG SaveVarCount,
		__in NWScriptStack::STACK_POINTER Offset
		);

	//
	// Determine whether a script should run in the JIT engine or in the VM.
	//

	bool
	ShouldJITScript(
		__in size_t ScriptCodeSize
		);

	//
	// Load symbols for a script.
	//

	void
	LoadSymbols(
		__inout NWScriptReader & Reader,
		__in const std::string & ScriptName
		);

	//
	// Sample the performance counter and return a count of millieconds.
	//

	ULONG
	ReadPerformanceCounterMilliseconds(
		) const;

	//
	// Convert a resref into a textural string.
	//

	inline
	std::string
	StrFromResRef(
		__in const NWN::ResRef32 & ResRef
		) const
	{
		std::string   R;
		const char  * p;

		p = (const char *) memchr(
			ResRef.RefStr,
			'\0' ,
			sizeof( ResRef.RefStr ) );

		if (p == NULL)
			R.assign( ResRef.RefStr, sizeof( ResRef.RefStr ) );
		else
			R.assign( ResRef.RefStr, p - ResRef.RefStr );

		for (size_t i = 0; i < R.size( ); i += 1)
		{
			R[ i ] = (char) tolower( (int) (unsigned char) R[ i ] );
		}

		return R;
	}

	//
	// Convert a textural string into a resref.
	//

	inline
	NWN::ResRef32
	ResRef32FromStr(
		__in const std::string & Str
		) const
	{
		NWN::ResRef32 ResRef;
		size_t        l;

		ZeroMemory( &ResRef, sizeof( ResRef ) ); // Satisfy overzealous compiler

		l = Str.length( );
		l = min( l, sizeof( ResRef ) );

		if (l != 0)
		{
			for (size_t i = 0; i < l; i += 1)
				ResRef.RefStr[ i ] = (char) tolower( Str[ i ] );
		}

		if (l < sizeof( ResRef ) )
			memset( &ResRef.RefStr[ l ], 0, sizeof( ResRef ) - l );

		return ResRef;
	}


	//
	// Define the debug output interface.
	// 

	IDebugTextOut                             * m_TextOut;

	//
	// Define the bridge object used to access the server's action service
	// handlers and VM stack.
	//

	NWScriptBridge                            * m_Bridge;

	//
	// Define the name of the script execution engine.
	//

	std::string                                 m_ScriptEngineName;

	//
	// Define the JIT engine instance, used to invoke scripts via JIT.
	//

	NWScriptJITLibPtr                           m_JITEngine;

	//
	// Define the JIT managed support object, used to optionally enable scripts
	// authored in managed code.
	//

	NWScriptJITManagedSupportPtr                m_JITManagedSupport;

	//
	// Define the script cache.  All script entries executed are placed into
	// the cache.
	//

	ScriptCacheMap                              m_ScriptCache;

	//
	// Define the currently executing script JIT program, referenced from action
	// handlers.
	//

	NWScriptJITLib::Program::Ptr                m_CurrentJITProgram;

	//
	// Define the name of the currently executing script.
	//

	NWN::ResRef32                               m_CurrentScriptName;

	//
	// Define the code size of the currently executing script.
	//

	size_t                                      m_CurrentScriptCodeSize;

	//
	// Define whether we have (ever) generated code for a script.
	//

	bool                                        m_FirstScript;

	//
	// Define the location where the execution engine DLLs reside.
	//

	std::string                                 m_DllDir;

	//
	// Define the NWScript VM, used as a fallback if a script cannot be JIT'd to
	// native code.
	//

	NWScriptVM                                * m_VM;

	//
	// Define the JIT policy engine, which selects a preferred execution engine
	// for a script.
	//

	INWScriptJITPolicy                        * m_JITPolicy;

	//
	// Define the current recursion level.
	//

	unsigned long                               m_RecursionLevel;

	//
	// Define total runtime spent in the script VM.
	//

	ULONG64                                     m_TotalScriptRuntime;

	//
	// Define the system performance counter frequency.
	//

	LARGE_INTEGER                               m_PerfFrequency;
};

#endif

