/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptProgram.h

Abstract:

	This module defines the representation of a JIT'd NWScript program.  The
	NWScriptProgram object is constructed given the NWScriptAnalyzer's IR, and
	encapsulates a series of DynamicMethod objects and supporting state
	describing an MSIL view of the script program's functional nature.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTJIT_NWSCRIPTPROGRAM_H
#define _SOURCE_PROGRAMS_NWNSCRIPTJIT_NWSCRIPTPROGRAM_H

#ifdef _MSC_VER
#pragma once
#endif




class NWScriptAnalyzer;
class INWScriptActions;
struct IDebugTextOut;
class NWScriptStack;

typedef std::vector< std::string > NWScriptParamVec;

namespace NWScript
{

ref class NWScriptSavedState;
ref class NWScriptEngineStructure;
ref class NWScriptCodeGenerator;
ref class NWScriptManagedSupport;



#if 0

//
// Define the typed engine structure wrapper that represents an engine
// structure upon the managed stack.
//

#define DECLARE_ENGINE_STRUCTURE( n )                            \
	public ref class NWScriptEngineStructure##n                  \
	{                                                            \
	                                                             \
	public:                                                      \
	                                                             \
	    inline                                                   \
	    NWScriptEngineStructure##n(                              \
	        __in EngineStructurePtr EngineStructure              \
	        )                                                    \
	    : m_EngineStructure( NULL )                              \
	    {                                                        \
	        m_EngineStructure = new EngineStructurePtr(          \
	            EngineStructure);                                \
	                                                             \
	    }                                                        \
	                                                             \
	    inline                                                   \
	    NWScriptEngineStructure##n(                              \
	        )                                                    \
	    : m_EngineStructure( NULL )                              \
	    {                                                        \
	                                                             \
	    }                                                        \
	                                                             \
	    inline                                                   \
	    ~NWScriptEngineStructure##n(                             \
	        )                                                    \
	    {                                                        \
	        if (m_EngineStructure != NULL)                       \
	            delete m_EngineStructure;                        \
	    }                                                        \
	                                                             \
	    inline                                                   \
	    void                                                     \
	    DeleteEngineStructure(                                   \
	        )                                                    \
	    {                                                        \
	        if (m_EngineStructure != NULL)                       \
	        {                                                    \
	            delete m_EngineStructure;                        \
	                                                             \
	            m_EngineStructure = NULL;                        \
	        }                                                    \
	    }                                                        \
	                                                             \
        typedef System::Reflection::MethodInfo MethodInfo;       \
	                                                             \
	    EngineStructurePtr * m_EngineStructure;                  \
	    static const int     m_sEngineStructureNumber = n;       \
	    static MethodInfo  ^ m_sIntrinsic_VMStackPush;           \
	    static MethodInfo  ^ m_sIntrinsic_VMStackPop;            \
	    static MethodInfo  ^ m_sIntrinsic_Compare;               \
	};                                                       

//
// Define each supported engine structure type (10 maximum).
//

DECLARE_ENGINE_STRUCTURE( 0 )
DECLARE_ENGINE_STRUCTURE( 1 )
DECLARE_ENGINE_STRUCTURE( 2 )
DECLARE_ENGINE_STRUCTURE( 3 )
DECLARE_ENGINE_STRUCTURE( 4 )
DECLARE_ENGINE_STRUCTURE( 5 )
DECLARE_ENGINE_STRUCTURE( 6 )
DECLARE_ENGINE_STRUCTURE( 7 )
DECLARE_ENGINE_STRUCTURE( 8 )
DECLARE_ENGINE_STRUCTURE( 9 )

#undef DECLARE_ENGINE_STRUCTURE
#endif

//
// Define the engine structure wrapper type.
//

public ref class NWScriptEngineStructure : INWScriptEngineStructure
{

public:

	inline
	NWScriptEngineStructure(
		__in EngineStructurePtr && EngineStructure
		)
	: m_EngineStructure( new EngineStructurePtr( EngineStructure ) )
	{
	}

	inline
	~NWScriptEngineStructure(
		)
	{
		this->!NWScriptEngineStructure( );
	}

	inline
	!NWScriptEngineStructure(
		)
	{
		DeleteEngineStructure( );
	}

	inline
	virtual
	void
	DeleteEngineStructure(
		)
	{
		if (m_EngineStructure != NULL)
		{
			delete m_EngineStructure;

			m_EngineStructure = NULL;
		}
	}

	inline
	int
	GetEngineType(
		)
	{
		return (*m_EngineStructure)->GetEngineType( );
	}

	EngineStructurePtr * m_EngineStructure;

};


//
// Define the representation of a generated NWScript program, compiled to MSIL.
//

public ref class NWScriptProgram : INWScriptProgram
{

public:

	//
	// Construct a new script program.  An MSIL representation of the script
	// program's functional nature is generated and stored for future use.
	//

	NWScriptProgram(
		__in const NWScriptAnalyzer * Analyzer,
		__in_opt IDebugTextOut * TextOut,
		__in ULONG DebugLevel,
		__in INWScriptActions * ActionHandler,
		__in NWN::OBJECTID ObjectInvalid,
		__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams
		);

	//
	// Construct a script program based off of a pre-compiled managed binary.
	// Type information for the script is retrieved and an initial script
	// object is created.
	//

	NWScriptProgram(
		__in const NWScriptAnalyzer * Analyzer,
		__in_opt IDebugTextOut * TextOut,
		__in ULONG DebugLevel,
		__in INWScriptActions * ActionHandler,
		__in array< Byte > ^ ManagedAssembly,
		__in NWScriptManagedSupport ^ ManagedSupport,
		__in NWN::OBJECTID ObjectInvalid,
		__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams
		);
	
	//
	// Destruct a NWScriptProgram instance.
	//

	inline
	~NWScriptProgram(
		)
	{
		this->!NWScriptProgram( );
	}

	!NWScriptProgram(
		);

	//
	// Execute a script.  The script's return value (if any) is returned.
	//

	int
	ExecuteScript(
		__in INWScriptStack * VMStack,
		__in NWN::OBJECTID ObjectSelf,
		__in_ecount_opt( ParamCount ) const NWScriptParamString * Params,
		__in size_t ParamCount,
		__in int DefaultReturnCode,
		__in ULONG Flags
		);

	//
	// Execute a saved script situation (that is, a delayed action or other
	// suspended script state).  The script state is consumed by the execution.
	//

	void
	ExecuteScriptSituation(
		__inout NWScriptSavedState ^ ScriptState,
		__in NWN::OBJECTID ObjectSelf
		);

	//
	// Abort the currently running script.
	//

	void
	AbortScript(
		);

	//
	// Check whether the currently executing script has been aborted.  This
	// routine may only be called while a script is executing, from a call
	// stack that originated from the script VM (i.e. the script VM itself or
	// an action service handler).
	//

	bool
	IsScriptAborted(
		);

	//
	// Retrieve the current saved state.  This routine may only be called from
	// an action handler that takes an action argument.
	//

	NWScriptSavedState ^
	GetSavedState(
		);

	//
	// Duplicate a saved state.
	//

	NWScriptSavedState ^
	DuplicateSavedState(
		__in NWScriptSavedState ^ SourceState
		);

	//
	// Push a saved state onto a VM stack.
	//

	void
	PushSavedState(
		__in NWScriptSavedState ^ SourceState,
		__in INWScriptStack * Stack,
		__out UInt32 * ResumeMethodId,
		__out NWNScriptLib::PROGRAM_COUNTER * ResumeMethodPC,
		__out UInt32 * SaveGlobalCount,
		__out UInt32 * SaveLocalCount,
		__out NWN::OBJECTID * CurrentActionObjectSelf
		);

	//
	// Pop a saved state off of a VM stack.
	//

	NWScriptSavedState ^
	PopSavedState(
		__in INWScriptStack * Stack,
		__in UInt32 ResumeMethodId,
		__in NWNScriptLib::PROGRAM_COUNTER ResumeMethodPC,
		__in UInt32 SaveGlobalCount,
		__in UInt32 SaveLocalCount,
		__in NWN::OBJECTID CurrentActionObjectSelf
		);

	//
	// Define intrinsic methods invoked by the program class in order to
	// perform a complex operation.
	//
	// N.B.  Although these methods are declared as public (so as to permit
	//       access by the generated methods), they are not designed for use
	//       outside of the generated code itself.
	//

	//
	// Push an integer value onto the VM stack (for an action call).
	//

	virtual
	void
	Intrinsic_VMStackPushInt(
		__in Int32 i
		);

	//
	// Pop an integer value off of the VM stack (for an action call).
	//

	virtual
	Int32
	Intrinsic_VMStackPopInt(
		);

	//
	// Push a float value onto the VM stack (for an action call).
	//

	virtual
	void
	Intrinsic_VMStackPushFloat(
		__in Single i
		);

	//
	// Pop a float value off of the VM stack (for an action call).
	//

	virtual
	Single
	Intrinsic_VMStackPopFloat(
		);

	//
	// Push a string value onto the VM stack (for an action call).
	//

	virtual
	void
	Intrinsic_VMStackPushString(
		__in String ^ s
		);

	//
	// Pop a string value off of the VM stack (for an action call).
	//
	
	virtual
	String ^
	Intrinsic_VMStackPopString(
		);

	//
	// Push an object id value onto the VM stack (for an action call).
	//

	virtual
	void
	Intrinsic_VMStackPushObjectId(
		__in UInt32 o
		);

	//
	// Pop an object id value off of the stack (for an action call).
	//

	virtual
	UInt32
	Intrinsic_VMStackPopObjectId(
		);

	C_ASSERT( sizeof( UInt32 ) == sizeof( NWN::OBJECTID ) );

	//
	// Push an engine structure value onto the VM stack (for an action call),
	// or pop an engine structure value off the VM stack.
	//

#if 0
#define DECLARE_ENGINE_STRUCTURE( n )                               \
	inline                                                          \
	void                                                            \
	Intrinsic_VMStackPushEngineStructure##n(                        \
	    __in NWScriptEngineStructure##n ^ EngineStructure           \
	    )                                                           \
	{                                                               \
		try                                                         \
	    {                                                           \
	        m_Stack->StackPushEngineStructure(                      \
	            *EngineStructure->m_EngineStructure                 \
	            );                                                  \
	    }                                                           \
	    catch (std::exception &e)                                   \
	    {                                                           \
	        throw gcnew Exception( gcnew String( e.what( ) ) );     \
	    }                                                           \
	}                                                               \
	                                                                \
	inline                                                          \
	NWScriptEngineStructure##n ^                                    \
	Intrinsic_VMStackPopEngineStructure##n(                         \
	    )                                                           \
	{                                                               \
		try                                                         \
	    {                                                           \
	        NWScriptEngineStructure##n ^ MSIL_EngStruct =           \
			    gcnew NWScriptEngineStructure##n(                   \
	                m_Stack->StackPopEngineStructure(               \
	                    (INWScriptStack::ENGINE_STRUCTURE_NUMBER) n \
	                    )                                           \
	                );                                              \
	                                                                \
	        return MSIL_EngStruct;                                  \
	    }                                                           \
	    catch (std::exception &e)                                   \
	    {                                                           \
	        throw gcnew Exception( gcnew String( e.what( ) ) );     \
	    }                                                           \
	}                                                               \
	                                                                \
	inline                                                          \
	bool                                                            \
	Intrinsic_CompareEngineStructure##n(                            \
	    __in NWScriptEngineStructure##n ^ EngineStructure1,         \
	    __in NWScriptEngineStructure##n ^ EngineStructure2          \
	    )                                                           \
	{                                                               \
		try                                                         \
	    {                                                           \
	        const EngineStructure * Es1;                            \
			const EngineStructure * Es2;                            \
	                                                                \
	        Es1 = (*EngineStructure1->m_EngineStructure).get( );    \
	        Es2 = (*EngineStructure2->m_EngineStructure).get( );    \
	                                                                \
	        return Es1->CompareEngineStructure( Es2 );              \
	    }                                                           \
	    catch (std::exception &e)                                   \
	    {                                                           \
	        throw gcnew Exception( gcnew String( e.what( ) ) );     \
	    }                                                           \
	}                                                               

	DECLARE_ENGINE_STRUCTURE( 0 )
	DECLARE_ENGINE_STRUCTURE( 1 )
	DECLARE_ENGINE_STRUCTURE( 2 )
	DECLARE_ENGINE_STRUCTURE( 3 )
	DECLARE_ENGINE_STRUCTURE( 4 )
	DECLARE_ENGINE_STRUCTURE( 5 )
	DECLARE_ENGINE_STRUCTURE( 6 )
	DECLARE_ENGINE_STRUCTURE( 7 )
	DECLARE_ENGINE_STRUCTURE( 8 )
	DECLARE_ENGINE_STRUCTURE( 9 )

#undef DECLARE_ENGINE_STRUCTURE
#endif

	//
	// Push an engine structure onto the stack.
	//

	virtual
	void
	Intrinsic_VMStackPushEngineStructure(
		__in INWScriptEngineStructure ^ EngineStructure
		);

	//
	// Pop an engine structure from the stack.
	//

	virtual
	INWScriptEngineStructure ^
	Intrinsic_VMStackPopEngineStructure(
		__in int EngType
		);

	//
	// Compare two engine structures.
	//

	virtual
	bool
	Intrinsic_CompareEngineStructure(
		__in INWScriptEngineStructure ^ EngineStructure1,
		__in INWScriptEngineStructure ^ EngineStructure2
		);

	//
	// Create a default value (empty) engine structure of a given type.
	//

	virtual
	INWScriptEngineStructure ^
	Intrinsic_CreateEngineStructure(
		__in int EngType
		);

	//
	// Execute a call to the script host's action service handler.
	//

	virtual
	void
	Intrinsic_ExecuteActionService(
		__in UInt32 ActionId,
		__in UInt32 NumArguments
		);

	//
	// Store the state of the script program for retrieval later, so that a
	// script situation can be executed.
	//

	virtual
	void
	Intrinsic_StoreState(
		__in_opt array< Object ^ > ^ Globals,
		__in array< Object ^ > ^ Locals,
		__in UInt32 ResumeMethodPC,
		__in UInt32 ResumeMethodId,
		__in IGeneratedScriptProgram ^ ProgramObject
		);

	//
	// Execute a fast call to the script host's action service handler.
	//

	virtual
	Object ^
	Intrinsic_ExecuteActionServiceFast(
		__in UInt32 ActionId,
		__in UInt32 NumArguments,
		__in ... array< Object ^ > ^ Arguments
		);

	//
	// Allocate a neutral string given a String object.
	//

	virtual
	NWScript::NeutralStringStorage
	Intrinsic_AllocateNeutralString(
		__in String ^ Str
		);

	//
	// Release a neutral string (if it was allocated).
	//

	virtual
	void
	Intrinsic_DeleteNeutralString(
		__in NWScript::NeutralStringStorage % Str
		);

	//
	// Create a String from a neutral string object.
	//

	virtual
	String ^
	Intrinsic_NeutralStringToString(
		__in NWScript::NeutralStringStorage % Str
		);

	//
	// Raise an abort exception if necessary.
	//

	virtual
	void
	Intrinsic_CheckScriptAbort(
		__in bool ActionSucceeded
		);

private:

	typedef array< NWACTION_TYPE > ActionTypeArr;

	typedef NWNScriptLib::PROGRAM_COUNTER PROGRAM_COUNTER;

	//
	// Convert the parameter list for a script entry point invocation into the
	// actual boxed object array for a dynamic invocation.
	//

	array< Object ^ > ^
	ConvertParameterList(
		__in_ecount_opt( ParamCount ) const NWScriptParamString * Params,
		__in size_t ParamCount
		);


	//
	// Create a saved state object for the current state.
	//

	NWScriptSavedState ^
	CreateSavedState(
		__in array< Object ^ > ^ Globals,
		__in array< Object ^ > ^ Locals,
		__in UInt32 ResumeSubroutinePC,
		__in UInt32 ResumeSubroutineId,
		__in IGeneratedScriptProgram ^ ProgramObject
		);


	//
	// Make the transition from a fast action service handler call to the final
	// native code handler.
	//

	void
	FastInvokeActionServiceHandler(
		__in NWSCRIPT_ACTION ActionId,
		__in size_t NumArguments,
		__in_ecount( NumCmds ) PCNWFASTACTION_CMD Cmds,
		__in size_t NumCmds,
		__in uintptr_t * CmdParams
		);


	//
	// Push a variable set to a stack.
	//

	void
	PushVariablesToStack(
		__in INWScriptStack * Stack,
		__in array< Object ^ > ^ Vars
		);

	//
	// Pop a variable set from a stack.
	//

	array< Object ^ > ^
	PopVariablesFromStack(
		__in INWScriptStack * Stack,
		__in UInt32 SaveVarCount
		);


	//
	// Generate code for the script program, given the analyzer IR.
	//

	void
	GenerateProgramCode(
		__in const NWScriptAnalyzer * Analyzer,
		__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams,
		__in NWN::OBJECTID ObjectInvalid,
		__in String ^ Name
		);


	//
	// Discern the NWScript type of an object given its MSIL object instance.
	//

	NWACTION_TYPE
	GetNWScriptType(
		__in Object ^ MSILVariable
		);

	//
	// Discern the NWScript type of an object given its Type handle.
	//

	NWACTION_TYPE
	GetNWScriptType(
		__in Type ^ MSILType
		);





	//
	// Emit an error diagnostic in response to an exception.
	//

	void
	ErrorException(
		__in Exception ^ Excpt
		);

	//
	// Convert a System::String into a INWScriptStack::NeutralString.
	//

	INWScriptStack::NeutralString
	ConvertStringToNeutral(
		__in String ^ Str
		);



	//
	// Instantiate a managed script from a byte array.
	//

	void
	InstantiateManagedScript(
		__in array< Byte > ^ ManagedAssembly,
		__in NWScriptManagedSupport ^ ManagedSupport
		);

	//
	// Handle assembly resolution events for managed script assembly
	// instantiation.
	//

	Assembly ^
	InstantiateManagedScriptResolveAssembly(
		__in Object ^ Sender,
		__in ResolveEventArgs ^ Args
		);

	//
	// Save the canonical NWScriptManagedInterface module to disk.
	//

	void
	SaveManagedInterfaceDll(
		__in const NWScriptAnalyzer * Analyzer,
		__in NWN::OBJECTID ObjectInvalid,
		__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams
		);


	//
	// Discover the parameters to the script entry point (for conversion).
	//

	void
	DiscoverEntryPointParameters(
		__in const NWScriptAnalyzer * Analyzer
		);


	//
	// Determine whether a debug level is enabled.
	//

	inline
	bool
	IsDebugLevel(
		__in ULONG DebugLevel
		)
	{
		return ((m_TextOut != NULL) && (m_DebugLevel >= DebugLevel));
	}


	//
	// Define the count of supported engine structure types.
	//

	static const int NUM_ENGINE_STRUCTURE_TYPES = 10;


	//
	// Define the optional text out interface, for debug prints.
	//

	IDebugTextOut            * m_TextOut;

	//
	// Define the debug level for debug output.  Legal values are drawn from
	// the NWScriptVM::ExecDebugLevel family of enumerations.
	//

	ULONG                      m_DebugLevel;

	//
	// Define the action implementation interface.
	//

	INWScriptActions         * m_ActionHandler;

	//
	// Define the action definition table.
	//

	PCNWACTION_DEFINITION      m_ActionDefs;
	NWSCRIPT_ACTION            m_ActionCount;

	//
	// Define the program type instance used to execute code.
	//

	IGeneratedScriptProgram  ^ m_ProgramObject;

	//
	// Define the JIT intrinsics instance used to provide access to the script
	// host from the JIT'd code.
	//

	NWScriptJITIntrinsics    ^ m_JITIntrinsics;

	//
	// Define whether the entry point returns a value.
	//

	bool                       m_EntryPointReturnsValue;

	//
	// Define the type array for entry point arguments.
	//

	ActionTypeArr            ^ m_EntryPointParamTypes;

	//
	// Define the currently active self object for an action call.  Note that
	// the self object must be captured before a recursive call is made.
	//

	NWN::OBJECTID              m_CurrentActionObjectSelf;

	//
	// Define the invalid object id (for use in parameter conversions).
	//

	NWN::OBJECTID              m_InvalidObjId;

	//
	// Define the execution stack active for the current execution.
	//

	INWScriptStack           * m_Stack;

	//
	// Define the abort flag for the script.
	//

	bool                       m_Aborted;

	//
	// Define the nesting count for script invocations.
	//

	int                        m_NestingLevel;

	//
	// Define the current saved state object.
	//

	NWScriptSavedState       ^ m_SavedState;

	//
	// Define the name of the script.
	//

	String                   ^ m_ScriptName;

	//
	// Define the types for engine structures.
	//

	array< Type ^ >          ^ m_EngineStructureTypes;

	//
	// Define initial code generation flags.
	//

	ULONG                      m_CodeGenFlags;

	//
	// Define whether we are a managed script versus a NWScript script.
	//

	bool                       m_ManagedScript;

	//
	// Define the managed support object currently in use for assembly name
	// resolution.
	//

	NWScriptManagedSupport   ^ m_ManagedSupport;

	//
	// Define the text encoding object used to map character strings.  For a
	// JIT'd NWScript script, this is typically the truncate-to-8-bit encoding
	// object.  For managed scripts, it is the UTF-8 encoding object.
	//

	System::Text::Encoding   ^ m_StringEncoding;

};

//
// Define an auto releasing neutral string.
//

class AutoNeutralString
{

public:

	inline
	AutoNeutralString(
		)
	{
		Str.first  = NULL;
		Str.second = 0;
	}

	inline
	~AutoNeutralString(
		)
	{
		if (Str.first != NULL)
		{
			NWScriptStack::FreeNeutral( Str.first );
			Str.first = NULL;
		}
	}

	NeutralString Str;

};

typedef std::list< AutoNeutralString > NeutralStrList;

}

#endif

