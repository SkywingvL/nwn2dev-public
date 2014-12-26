/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptJITLib.h

Abstract:

	This module defines a public wrapper for managing the MSIL JIT library's
	external interface.  It is intended for use by users of the library, and
	not internal users of the library itself.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTJIT_NWSCRIPTJITLIB_H
#define _SOURCE_PROGRAMS_NWNSCRIPTJIT_NWSCRIPTJITLIB_H

#ifdef _MSC_VER
#pragma once
#endif

#include "NWNScriptJIT.h"

//
// Define the overarching wrapper for the script library.  Typically, a user of
// the library (if choosing to use the wrapper) would create a single
// NWScriptJITLib instance for their entire program.
//
// The user bears responsibility for ensuring that any dependent child objects
// created by the NWScriptJITLib are deleted before the NWScriptJITLib is
// permitted to be released.
//

class NWScriptJITLib
{

public:

	//
	// Load the JIT library.  Raises an std::exception on error, such as if the
	// library was incompatible or could not be loaded.
	//

	inline
	NWScriptJITLib(
		__in const std::wstring & JITLibFileName
		)
	: m_JITLib( NULL )
	{
		try
		{
			//
			// Load the library up from the user's supplied path.
			//

			m_JITLib = LoadLibraryW( JITLibFileName.c_str( ) );

			if (m_JITLib == NULL)
			{
				throw std::runtime_error(
					"Failed to load NWScript JIT library." );
			}

			//
			// Now resolve all of the required export symbols.  Each must exist
			// in a compatible library version.
			//

			struct
			{
				void       * * Ptr;
				const char *   Symbol;
			} Symbols[ ] =
			{
				{ (void **) &m_NWScriptGenerateCode            , "NWScriptGenerateCode"             },
				{ (void **) &m_NWScriptDeleteProgram           , "NWScriptDeleteProgram"            },
				{ (void **) &m_NWScriptSaveState               , "NWScriptSaveState"                },
				{ (void **) &m_NWScriptDeleteSavedState        , "NWScriptDeleteSavedState"         },
				{ (void **) &m_NWScriptExecuteScript           , "NWScriptExecuteScript"            },
				{ (void **) &m_NWScriptExecuteScriptSituation  , "NWScriptExecuteScriptSituation"   },
				{ (void **) &m_NWScriptAbortScript             , "NWScriptAbortScript"              },
				{ (void **) &m_NWScriptIsScriptAborted         , "NWScriptIsScriptAborted"          },
				{ (void **) &m_NWScriptCheckVersion            , "NWScriptCheckVersion"             },
				{ (void **) &m_NWScriptGetEngineName           , "NWScriptGetEngineName"            },
				{ (void **) &m_NWScriptDuplicateScriptSituation, "NWScriptDuplicateScriptSituation" },
				{ (void **) &m_NWScriptPushScriptSituation     , "NWScriptPushScriptSituation"      },
				{ (void **) &m_NWScriptPopScriptSituation      , "NWScriptPopScriptSituation"       },
				{ (void **) &m_NWScriptCreateManagedSupport    , "NWScriptCreateManagedSupport"     },
				{ (void **) &m_NWScriptDeleteManagedSupport    , "NWScriptDeleteManagedSupport"     }
			};

			for (size_t i = 0; i < RTL_NUMBER_OF( Symbols ); i += 1)
			{
				*Symbols[ i ].Ptr = GetProcAddress( m_JITLib, Symbols[ i ].Symbol );

				if (*Symbols[ i ].Ptr == NULL)
				{
					std::string ErrorMsg;

					ErrorMsg  = "Failed to resolve symbol '";
					ErrorMsg += Symbols[ i ].Symbol;
					ErrorMsg += "'.";

					throw std::runtime_error( ErrorMsg );
				}
			}

			//
			// Finally, check the version of the library as a basic defense
			// against blatant size mismatches in the C++ interface.
			//
			// N.B.  This is not a foolproof check, only best-effort.
			//

			CheckLibraryVersion( );
		}
		catch (...)
		{
			if (m_JITLib != NULL)
				FreeLibrary( m_JITLib );

			throw;
		}
	}

	//
	// Destruct a NWScriptJITLib instance.
	//

	inline
	~NWScriptJITLib(
		)
	{
		if (m_JITLib != NULL)
			FreeLibrary( m_JITLib );
	}

	//
	// Deallocate a JIT program.  This routine is generally for internal use of
	// derived classes.
	//

	inline
	void
	DeleteProgram(
		__in NWSCRIPT_JITPROGRAM Program
		)
	{
		m_NWScriptDeleteProgram( Program );
	}

	//
	// Deallocate a JIT saved state.  This routine is generally for internal use
	// of derived classes.
	//

	inline
	void
	DeleteSavedState(
		__in NWSCRIPT_JITRESUME ResumeState
		)
	{
		m_NWScriptDeleteSavedState( ResumeState );
	}

	//
	// Deallocate a JIT managed support object.  This routine is generally for
	// internal use of derived classes.
	//

	inline
	void
	DeleteManagedSupport(
		__in NWSCRIPT_JITMANAGEDSUPPORT ManagedSupportHandle
		)
	{
		m_NWScriptDeleteManagedSupport( ManagedSupportHandle );
	}

	//
	// Execute a script.  This routine is generally for internal use of derived
	// classes.  The user should use the API on the Program class.
	//

	inline
	int
	ExecuteScript(
		__in NWSCRIPT_JITPROGRAM Program,
		__in INWScriptStack * Stack,
		__in NWN::OBJECTID ObjectSelf,
		__in const NWScriptParamVec & Params,
		__in int DefaultReturnCode,
		__in ULONG Flags
		)
	{
		std::vector< NWScriptParamString > ConvParams;

		ConvParams.reserve( Params.size( ) );

		for (NWScriptParamVec::const_iterator it = Params.begin( );
		     it != Params.end( );
		     ++it)
		{
			NWScriptParamString Str;

			Str.String = it->c_str( );
			Str.Len    = it->size( );

			ConvParams.push_back( Str );
		}

		return m_NWScriptExecuteScript(
			Program,
			Stack,
			ObjectSelf,
			ConvParams.empty( ) ? NULL : &ConvParams[ 0 ],
			ConvParams.size( ),
			DefaultReturnCode,
			Flags);
	}

	//
	// Execute a script situation.  This routine is generally for internal use
	// of derived classes  The user should use the API on the Program class.
	//

	inline
	void
	ExecuteScriptSituation(
		__in NWSCRIPT_JITRESUME ResumeState,
		__in NWN::OBJECTID ObjectSelf
		)
	{
		m_NWScriptExecuteScriptSituation( ResumeState, ObjectSelf );
	}

	//
	// Abort a script.  This routine is generally for internal use of derived
	// classes.  The user should use the API on the Program class.
	//

	inline
	void
	AbortScript(
		__in NWSCRIPT_JITPROGRAM Program
		)
	{
		m_NWScriptAbortScript( Program );
	}

	//
	// Check whether the currently executing script has been aborted.  This
	// routine may only be called while a script is executing, from a call
	// stack that originated from the script VM (i.e. the script VM itself or
	// an action service handler).
	//
	// This routine is generally for internal use of derived classes.  The user
	// should use the API on the Program class.
	//

	inline
	bool
	IsScriptAborted(
		__in NWSCRIPT_JITPROGRAM Program
		) const
	{
		if (m_NWScriptIsScriptAborted( Program ))
			return true;
		else
			return false;
	}

	//
	// Create a saved state.  This routine is generally intended for use of
	// derived classes.  The user should use the API on the Program class.
	//

	inline
	NWSCRIPT_JITRESUME
	GetSavedState(
		__in NWSCRIPT_JITPROGRAM Program
		)
	{
		NWSCRIPT_JITRESUME ResumeState;

		if (!m_NWScriptSaveState( Program, &ResumeState ))
			throw std::runtime_error( "Failed to save script resume state." );

		return ResumeState;
	}

	//
	// Duplicate a saved state.  An std::exception is raised on failure.
	//

	inline
	NWSCRIPT_JITRESUME
	DuplicateScriptSituation(
		__in NWSCRIPT_JITRESUME SourceState
		)
	{
		NWSCRIPT_JITRESUME ResumeState;

		if (!m_NWScriptDuplicateScriptSituation( SourceState, &ResumeState ))
			throw std::runtime_error( "Failed to duplicate script situation." );

		return ResumeState;
	}

	//
	// Push a script situation onto a VM stack.  An std::exception is raised on
	// failure.
	//

	inline
	void
	PushScriptSituation(
		__in NWSCRIPT_JITRESUME SourceState,
		__in INWScriptStack * VMStack,
		__out PULONG ResumeMethodId,
		__out NWSCRIPT_PROGRAM_COUNTER * ResumeMethodPC,
		__out PULONG SaveGlobalCount,
		__out PULONG SaveLocalCount,
		__out NWN::OBJECTID * ObjectSelf
		)
	{
		if (!m_NWScriptPushScriptSituation(
			SourceState,
			VMStack,
			ResumeMethodId,
			ResumeMethodPC,
			SaveGlobalCount,
			SaveLocalCount,
			ObjectSelf))
			throw std::runtime_error( "Failed to push script situation." );
	}

	//
	// Pop a script situation off of a VM stack, instantiating a resume state.
	// An std::exception is raised on failure.
	//

	inline
	NWSCRIPT_JITRESUME
	PopScriptSituation(
		__in NWSCRIPT_JITPROGRAM Program,
		__in INWScriptStack * VMStack,
		__in ULONG ResumeMethodId,
		__in NWSCRIPT_PROGRAM_COUNTER ResumeMethodPC,
		__in ULONG SaveGlobalCount,
		__in ULONG SaveLocalCount,
		__in NWN::OBJECTID ObjectSelf
		)
	{
		NWSCRIPT_JITRESUME ResumeState;

		if (!m_NWScriptPopScriptSituation(
			Program,
			VMStack,
			ResumeMethodId,
			ResumeMethodPC,
			SaveGlobalCount,
			SaveLocalCount,
			ObjectSelf,
			&ResumeState))
			throw std::runtime_error( "Failed to pop script situation." );

		return ResumeState;
	}

	//
	// Define the representation of a JIT'd managed support object.
	//

	class ManagedSupport
	{

	public:

		typedef swutil::SharedPtr< ManagedSupport > Ptr;

		inline
		ManagedSupport(
			__in NWScriptJITLib * JITLib,
			__in NWSCRIPT_JITMANAGEDSUPPORT ManagedSupportHandle
			)
		: m_JITLib( JITLib ),
		  m_ManagedSupportHandle( ManagedSupportHandle )
		{
		}

		inline
		ManagedSupport(
			)
		: m_JITLib( NULL ),
		  m_ManagedSupportHandle( NULL )
		{
		}

		inline
		~ManagedSupport(
			)
		{
			if (m_JITLib != NULL)
				m_JITLib->DeleteManagedSupport( m_ManagedSupportHandle );
		}

		//
		// Return the underlying managed support state handle.
		//

		inline
		NWSCRIPT_JITMANAGEDSUPPORT
		GetManagedSupport(
			)
		{
			return m_ManagedSupportHandle;
		}

	private:

		//
		// Define the JIT library backlink.
		//

		NWScriptJITLib             * m_JITLib;

		//
		// Define the underlying managed support handle.
		//

		NWSCRIPT_JITMANAGEDSUPPORT   m_ManagedSupportHandle;

	};

	//
	// Define the representation of a JIT'd script saved state.
	//

	class SavedState
	{

	public:

		typedef swutil::SharedPtr< SavedState > Ptr;

		inline
		SavedState(
			__in NWScriptJITLib * JITLib,
			__in NWSCRIPT_JITRESUME ResumeState
			)
		: m_JITLib( JITLib ),
		  m_ResumeState( ResumeState )
		{
		}

		inline
		SavedState(
			)
		: m_JITLib( NULL ),
		  m_ResumeState( NULL )
		{
		}

		inline
		~SavedState(
			)
		{
			if (m_JITLib != NULL)
				m_JITLib->DeleteSavedState( m_ResumeState );
		}

		//
		// Execute a saved script situation (that is, a delayed action or other
		// suspended script state).  The script state is consumed by the
		// execution.
		//

		inline
		void
		ExecuteScriptSituation(
			__in NWN::OBJECTID ObjectSelf
			)
		{
			return m_JITLib->ExecuteScriptSituation(
				GetState( ),
				ObjectSelf);
		}

		//
		// Return the underlying resume state handle.  This routine is intended
		// for use only by the Program class.
		//

		inline
		NWSCRIPT_JITRESUME
		GetState(
			)
		{
			return m_ResumeState;
		}

	private:

		//
		// Define the JIT library backlink.
		//

		NWScriptJITLib       * m_JITLib;

		//
		// Define the underlying saved state handle.
		//

		NWSCRIPT_JITRESUME     m_ResumeState;

	};

	//
	// Define the representation of a JIT'd script program.
	//

	class Program
	{

	public:

		typedef swutil::SharedPtr< Program > Ptr;

		inline
		Program(
			__in NWScriptJITLib * JITLib,
			__in NWSCRIPT_JITPROGRAM Program
			)
		: m_JITLib( JITLib ),
		  m_Program( Program )
		{
		}

		inline
		Program(
			)
		: m_JITLib( NULL ),
		  m_Program( NULL )
		{
		}

		inline
		~Program(
			)
		{
			if (m_JITLib != NULL)
				m_JITLib->DeleteProgram( m_Program );
		}

		//
		// Execute the script program.  Any parameters are converted to the
		// appropriate types based on the standard conversion rules of the
		// NWScriptVM.
		//
		// The script's return value (if any) is returned.
		//

		inline
		int
		ExecuteScript(
			__in INWScriptStack * Stack,
			__in NWN::OBJECTID ObjectSelf,
			__in const NWScriptParamVec & Params,
			__in int DefaultReturnCode,
			__in ULONG Flags
			)
		{
			return m_JITLib->ExecuteScript(
				m_Program,
				Stack,
				ObjectSelf,
				Params,
				DefaultReturnCode,
				Flags);
		}

		//
		// Execute a saved script situation (that is, a delayed action or other
		// suspended script state).  The script state is consumed by the
		// execution.
		//

		inline
		void
		ExecuteScriptSituation(
			__in SavedState * State,
			__in NWN::OBJECTID ObjectSelf
			)
		{
			return m_JITLib->ExecuteScriptSituation(
				State->GetState( ),
				ObjectSelf);
		}

		//
		// Abort the currently executing script.
		//

		inline
		void
		AbortScript(
			)
		{
			m_JITLib->AbortScript( m_Program );
		}

		//
		// Check whether the currently executing script has been aborted.  This
		// routine may only be called while a script is executing, from a call
		// stack that originated from the script VM (i.e. the script VM itself
		// or an action service handler).
		//

		inline
		bool
		IsScriptAborted(
			)
		{
			return m_JITLib->IsScriptAborted( m_Program );
		}

		//
		// Create a saved state snapshot from the last saved state checkout.
		//

		inline
		SavedState
		CreateSavedState(
			)
		{
			return SavedState(
				m_JITLib,
				m_JITLib->GetSavedState( m_Program ) );
		}

		inline
		SavedState::Ptr
		CreateSavedStatePtr(
			)
		{
			return new SavedState(
				m_JITLib,
				m_JITLib->GetSavedState( m_Program ) );
		}

		//
		// Store a saved state snapshot onto a stack.
		//

		inline
		void
		PushSavedState(
			__in SavedState * State,
			__in INWScriptStack * VMStack,
			__out PULONG ResumeMethodId,
			__out NWSCRIPT_PROGRAM_COUNTER * ResumeMethodPC,
			__out PULONG SaveGlobalCount,
			__out PULONG SaveLocalCount,
			__out NWN::OBJECTID * ObjectSelf
			)
		{
			m_JITLib->PushScriptSituation(
				State->GetState( ),
				VMStack,
				ResumeMethodId,
				ResumeMethodPC,
				SaveGlobalCount,
				SaveLocalCount,
				ObjectSelf);
		}

		//
		// Restore a saved state snapshot that was saved to a stack, creating a
		// resumable saved state object.
		//

		inline
		SavedState
		PopSavedState(
			__in INWScriptStack * VMStack,
			__in ULONG ResumeMethodId,
			__in NWSCRIPT_PROGRAM_COUNTER ResumeMethodPC,
			__in ULONG SaveGlobalCount,
			__in ULONG SaveLocalCount,
			__in NWN::OBJECTID ObjectSelf
			)
		{
			return SavedState(
				m_JITLib,
				m_JITLib->PopScriptSituation(
					m_Program,
					VMStack,
					ResumeMethodId,
					ResumeMethodPC,
					SaveGlobalCount,
					SaveLocalCount,
					ObjectSelf
					)
				);
		}

		inline
		SavedState::Ptr
		PopSavedStatePtr(
			__in INWScriptStack * VMStack,
			__in ULONG ResumeMethodId,
			__in NWSCRIPT_PROGRAM_COUNTER ResumeMethodPC,
			__in ULONG SaveGlobalCount,
			__in ULONG SaveLocalCount,
			__in NWN::OBJECTID ObjectSelf
			)
		{
			return new SavedState(
				m_JITLib,
				m_JITLib->PopScriptSituation(
					m_Program,
					VMStack,
					ResumeMethodId,
					ResumeMethodPC,
					SaveGlobalCount,
					SaveLocalCount,
					ObjectSelf
					)
				);
		}

	private:

		//
		// Define the JIT library backlink.
		//

		NWScriptJITLib       * m_JITLib;

		//
		// Define the underlying program handle.
		//

		NWSCRIPT_JITPROGRAM    m_Program;

	};

	//
	// Create a JIT representation of a program, given analysis state for it.
	//
	// The operation is performed synchronously and is potentially expensive,
	// so the user may wish to perform the operation in a background worker
	// thread.
	//

	inline
	Program
	GenerateCode(
		__in NWScriptReader * Script,
		__in_ecount( ActionCount ) PCNWACTION_DEFINITION ActionDefs,
		__in NWSCRIPT_ACTION ActionCount,
		__in ULONG AnalysisFlags,
		__in_opt IDebugTextOut * TextOut,
		__in ULONG DebugLevel,
		__in INWScriptActions * ActionHandler,
		__in NWN::OBJECTID ObjectInvalid,
		__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams = NULL
		)
	{
		return Program(
			this,
			InternalGenerateCode(
				Script,
				ActionDefs,
				ActionCount,
				AnalysisFlags,
				TextOut,
				DebugLevel,
				ActionHandler,
				ObjectInvalid,
				CodeGenParams
				)
			);
	}

	inline
	Program::Ptr
	GenerateCodePtr(
		__in NWScriptReader * Script,
		__in_ecount( ActionCount ) PCNWACTION_DEFINITION ActionDefs,
		__in NWSCRIPT_ACTION ActionCount,
		__in ULONG AnalysisFlags,
		__in_opt IDebugTextOut * TextOut,
		__in ULONG DebugLevel,
		__in INWScriptActions * ActionHandler,
		__in NWN::OBJECTID ObjectInvalid,
		__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams = NULL
		)
	{
		return new Program(
			this,
			InternalGenerateCode(
				Script,
				ActionDefs,
				ActionCount,
				AnalysisFlags,
				TextOut,
				DebugLevel,
				ActionHandler,
				ObjectInvalid,
				CodeGenParams
				)
			);
	}

	//
	// Create a JIT managed support object.
	//
	// The operation is performed synchronously and is potentially expensive,
	// so the user may wish to perform the operation in a background worker
	// thread.
	//

	inline
	ManagedSupport
	CreateManagedSupport(
		__in_ecount( ActionCount ) PCNWACTION_DEFINITION ActionDefs,
		__in NWSCRIPT_ACTION ActionCount,
		__in ULONG AnalysisFlags,
		__in_opt IDebugTextOut * TextOut,
		__in ULONG DebugLevel,
		__in INWScriptActions * ActionHandler,
		__in NWN::OBJECTID ObjectInvalid,
		__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams = NULL
		)
	{
		return ManagedSupport(
			this,
			InternalCreateManagedSupport(
				ActionDefs,
				ActionCount,
				AnalysisFlags,
				TextOut,
				DebugLevel,
				ActionHandler,
				ObjectInvalid,
				CodeGenParams
				)
			);
	}

	inline
	ManagedSupport::Ptr
	CreateManagedSupportPtr(
		__in_ecount( ActionCount ) PCNWACTION_DEFINITION ActionDefs,
		__in NWSCRIPT_ACTION ActionCount,
		__in ULONG AnalysisFlags,
		__in_opt IDebugTextOut * TextOut,
		__in ULONG DebugLevel,
		__in INWScriptActions * ActionHandler,
		__in NWN::OBJECTID ObjectInvalid,
		__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams = NULL
		)
	{
		return new ManagedSupport(
			this,
			InternalCreateManagedSupport(
				ActionDefs,
				ActionCount,
				AnalysisFlags,
				TextOut,
				DebugLevel,
				ActionHandler,
				ObjectInvalid,
				CodeGenParams
				)
			);
	}

	//
	// Return the name of the JIT engine.
	//

	inline
	const wchar_t *
	GetEngineName(
		)
	{
		return m_NWScriptGetEngineName( );
	}

private:

	//
	// Check that the version of the JIT library is compatible with the caller.
	//
	// N.B.  The check is best effort only and is not a guarantee.
	//

	inline
	void
	CheckLibraryVersion(
		)
	{
		struct
		{
			NWSCRIPT_JIT_VERSION VersionClass;
			ULONG                VersionValue;
		} VersionChecks[ ] =
		{
			{ NWScriptJITVersion_APIVersion          , NWSCRIPTJITAPI_CURRENT        },
			{ NWScriptJITVersion_NWScriptReaderState , sizeof( NWScriptReaderState ) },
			{ NWScriptJITVersion_NWScriptStack       , sizeof( NWScriptStack )       },
			{ NWScriptJITVersion_NWScriptParamVec    , sizeof( NWScriptParamVec )    },
			{ NWScriptJITVersion_NWACTION_DEFINITION , sizeof( NWACTION_DEFINITION ) },
			{ NWScriptJITVersion_NeutralString       , sizeof( NeutralString )       }
		};

		//
		// Validate that all of the significant version fields are compatible.
		//

		for (size_t i = 0; i < RTL_NUMBER_OF( VersionChecks ); i += 1)
		{
			if (!m_NWScriptCheckVersion(
				VersionChecks[ i ].VersionClass,
				VersionChecks[ i ].VersionValue))
			{
				throw std::runtime_error( "Incompatible library version." );
			}
		}
	}

	//
	// Generate code for a script program.
	//

	inline
	NWSCRIPT_JITPROGRAM
	InternalGenerateCode(
		__in NWScriptReader * Script,
		__in_ecount( ActionCount ) PCNWACTION_DEFINITION ActionDefs,
		__in NWSCRIPT_ACTION ActionCount,
		__in ULONG AnalysisFlags,
		__in_opt IDebugTextOut * TextOut,
		__in ULONG DebugLevel,
		__in INWScriptActions * ActionHandler,
		__in NWN::OBJECTID ObjectInvalid,
		__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams
		)
	{
		NWSCRIPT_JITPROGRAM                    GeneratedProgram;
		NWScriptReaderState                    ReaderState;
		NWScriptReader::SymbolTableRawEntryVec SymTab;

		//
		// Package the NWScriptReader into its internal state so that it can be
		// safely transported cross-module.
		//

		Script->StoreInternalState(
			ReaderState.InstructionStream,
			ReaderState.InstructionStreamSize,
			SymTab);

		ReaderState.ScriptName = Script->GetScriptName( ).c_str( );
		ReaderState.SymTab      = (SymTab.empty( ) ? NULL : &SymTab[ 0 ]);
		ReaderState.SymTabSize  = SymTab.size( );

		//
		// Now generate the code for the script.
		//

		if (!m_NWScriptGenerateCode(
			&ReaderState,
			ActionDefs,
			ActionCount,
			AnalysisFlags,
			TextOut,
			DebugLevel,
			ActionHandler,
			ObjectInvalid,
			CodeGenParams,
			&GeneratedProgram))
			throw std::runtime_error( "Failed to generate code for script." );

		return GeneratedProgram;
	}

	//
	// Create a managed support object.
	//

	inline
	NWSCRIPT_JITMANAGEDSUPPORT
	InternalCreateManagedSupport(
		__in_ecount( ActionCount ) PCNWACTION_DEFINITION ActionDefs,
		__in NWSCRIPT_ACTION ActionCount,
		__in ULONG AnalysisFlags,
		__in_opt IDebugTextOut * TextOut,
		__in ULONG DebugLevel,
		__in INWScriptActions * ActionHandler,
		__in NWN::OBJECTID ObjectInvalid,
		__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams
		)
	{
		NWSCRIPT_JITMANAGEDSUPPORT             GeneratedManagedSupport;

		//
		// Now generate the code for the managed support interface.
		//

		if (!m_NWScriptCreateManagedSupport(
			ActionDefs,
			ActionCount,
			AnalysisFlags,
			TextOut,
			DebugLevel,
			ActionHandler,
			ObjectInvalid,
			CodeGenParams,
			&GeneratedManagedSupport))
			throw std::runtime_error( "Failed to generate code for managed support." );

		return GeneratedManagedSupport;
	}

	//
	// Define the module handle of the JIT library itself.
	//

	HMODULE                            m_JITLib;

	//
	// Define the library export symbols that have been resolved.
	//

	NWScriptGenerateCodeProc             m_NWScriptGenerateCode;
	NWScriptDeleteProgramProc            m_NWScriptDeleteProgram;
	NWScriptSaveStateProc                m_NWScriptSaveState;
	NWScriptDeleteSavedStateProc         m_NWScriptDeleteSavedState;
	NWScriptExecuteScriptProc            m_NWScriptExecuteScript;
	NWScriptExecuteScriptSituationProc   m_NWScriptExecuteScriptSituation;
	NWScriptAbortScriptProc              m_NWScriptAbortScript;
	NWScriptIsScriptAbortedProc          m_NWScriptIsScriptAborted;
	NWScriptCheckVersionProc             m_NWScriptCheckVersion;
	NWScriptGetEngineNameProc            m_NWScriptGetEngineName;
	NWScriptDuplicateScriptSituationProc m_NWScriptDuplicateScriptSituation;
	NWScriptPushScriptSituationProc      m_NWScriptPushScriptSituation;
	NWScriptPopScriptSituationProc       m_NWScriptPopScriptSituation;
	NWScriptCreateManagedSupportProc     m_NWScriptCreateManagedSupport;
	NWScriptDeleteManagedSupportProc     m_NWScriptDeleteManagedSupport;

};

typedef NWScriptJITLib::Program NWScriptJITProgram;
typedef NWScriptJITLib::SavedState NWScriptJITSavedState;
typedef NWScriptJITLib::ManagedSupport NWScriptJITManagedSupport;

#endif

