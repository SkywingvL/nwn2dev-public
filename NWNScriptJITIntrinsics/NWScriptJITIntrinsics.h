/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptJITIntrinsics.h

Abstract:

	This module defines the JIT intrinsics that are made available to the
	JIT'd NWScript program.  It is intended for use in conjuction with the
	NWNScriptJIT module.

	The JIT intrinsics defined herein are intended only for consumption by the
	JIT'd code itself.  They are compiled as pure CLR.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTJIT_NWSCRIPTJITINTRINSICS_H
#define _SOURCE_PROGRAMS_NWNSCRIPTJIT_NWSCRIPTJITINTRINSICS_H

namespace NWScript
{

//
// Define the wrapper for an action vector return type.
//

public value struct Vector3
{
	float x;
	float y;
	float z;
};

//
// Define the wrapper for a neutral string, used to declare compatible storage.
//

[System::Runtime::InteropServices::StructLayout( System::Runtime::InteropServices::LayoutKind::Sequential )]
public value struct NeutralStringStorage
{
	IntPtr StrPtr;
	IntPtr Length;
};

//
// Define the engine structure wrapper type.
//

public interface class INWScriptEngineStructure
{

	void
	DeleteEngineStructure(
		);

};

//
// Define the type for an action service handler delegate.  The delegate type
// is only used for native CLR scripts and not scripts written in NWScript.
//

public delegate void ActionDelegate(
	);

//
// Define the interface that a JIT'd program implements.
//

public interface class IGeneratedScriptProgram
{

	//
	// Execute the script and return the entry point return value, if any.
	//

	Int32
	ExecuteScript(
		__in UInt32 ObjectSelf,
		__in array< Object ^ > ^ ScriptParameters,
		__in Int32 DefaultReturnCode
		);

	//
	// Execute a script situation (resume label).
	//

	void
	ExecuteScriptSituation(
		__in UInt32 ScriptSituationId,
		__in array< Object ^ > ^ Locals,
		__in UInt32 ObjectSelf
		);

	//
	// Clone a program instance (i.e. for saved state).
	//

	IGeneratedScriptProgram ^
	CloneScriptProgram(
		);

	//
	// Load saved global variables (i.e. for deserialized saved state restore).
	//

	void
	LoadScriptGlobals(
		__in array< Object ^ > ^ Globals
		);

};

//
// Define the primary NWScriptProgram interface which is used to indirect back
// to native code (but keep the code referenced by the intrinsics as clr:pure)
// and policy check-passing from a peverify perspective.
//

public interface class INWScriptProgram
{

	//
	// Push an integer value onto the VM stack (for an action call).
	//

	void
	Intrinsic_VMStackPushInt(
		__in Int32 i
		);

	//
	// Pop an integer value off of the VM stack (for an action call).
	//

	Int32
	Intrinsic_VMStackPopInt(
		);

	//
	// Push a float value onto the VM stack (for an action call).
	//

	void
	Intrinsic_VMStackPushFloat(
		__in Single i
		);

	//
	// Pop a float value off of the VM stack (for an action call).
	//

	Single
	Intrinsic_VMStackPopFloat(
		);

	//
	// Push a string value onto the VM stack (for an action call).
	//

	void
	Intrinsic_VMStackPushString(
		__in String ^ s
		);

	//
	// Pop a string value off of the VM stack (for an action call).
	//

	String ^
	Intrinsic_VMStackPopString(
		);

	//
	// Push an object id value onto the VM stack (for an action call).
	//

	void
	Intrinsic_VMStackPushObjectId(
		__in UInt32 o
		);

	//
	// Pop an object id value off of the stack (for an action call).
	//

	UInt32
	Intrinsic_VMStackPopObjectId(
		);

	//
	// Push an engine structure onto the stack.
	//

	void
	Intrinsic_VMStackPushEngineStructure(
		__in INWScriptEngineStructure ^ EngineStructure
		);

	//
	// Pop an engine structure from the stack.
	//

	INWScriptEngineStructure ^
	Intrinsic_VMStackPopEngineStructure(
		__in int EngType
		);

	//
	// Compare two engine structures.
	//

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

	void
	Intrinsic_ExecuteActionService(
		__in UInt32 ActionId,
		__in UInt32 NumArguments
		);

	//
	// Store the state of the script program for retrieval later, so that a
	// script situation can be executed.
	//

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

	Object ^
	Intrinsic_ExecuteActionServiceFast(
		__in UInt32 ActionId,
		__in UInt32 NumArguments,
		__in ... array< Object ^ > ^ Arguments
		);

	//
	// Allocate a neutral string given a String object.
	//

	NWScript::NeutralStringStorage
	Intrinsic_AllocateNeutralString(
		__in String ^ Str
		);

	//
	// Release a neutral string (if it was allocated).
	//

	void
	Intrinsic_DeleteNeutralString(
		__in NWScript::NeutralStringStorage % Str
		);

	//
	// Create a String from a neutral string object.
	//

	String ^
	Intrinsic_NeutralStringToString(
		__in NWScript::NeutralStringStorage % Str
		);

	//
	// Raise an abort exception if necessary.
	//

	void
	Intrinsic_CheckScriptAbort(
		__in bool ActionSucceeded
		);

};


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
	        __in INWScriptEngineStructure ^ EngineStructure      \
	        )                                                    \
	    : m_EngineStructure( EngineStructure )                   \
	    {                                                        \
	                                                             \
	    }                                                        \
	                                                             \
	    inline                                                   \
	    NWScriptEngineStructure##n(                              \
	        )                                                    \
	    : m_EngineStructure( nullptr )                           \
	    {                                                        \
	                                                             \
	    }                                                        \
	                                                             \
	    inline                                                   \
	    ~NWScriptEngineStructure##n(                             \
	        )                                                    \
	    {                                                        \
	        this->!NWScriptEngineStructure##n( );                \
	    }                                                        \
	                                                             \
	    inline                                                   \
	    !NWScriptEngineStructure##n(                             \
	        )                                                    \
	    {                                                        \
	        DeleteEngineStructure( );                            \
	    }                                                        \
	                                                             \
	    inline                                                   \
	    void                                                     \
	    DeleteEngineStructure(                                   \
	        )                                                    \
	    {                                                        \
	        if (m_EngineStructure != nullptr)                    \
	        {                                                    \
	            m_EngineStructure->DeleteEngineStructure( );     \
	                                                             \
	            m_EngineStructure = nullptr;                     \
	        }                                                    \
	    }                                                        \
	                                                             \
        INWScriptEngineStructure ^ m_EngineStructure;            \
	                                                             \
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

//
// Define the JIT intrinsics object, which is called back to by the emitted IL.
//

public ref class NWScriptJITIntrinsics
{

public:

	//
	// Construct a new NWScriptJITIntrinsics object, which is associated with a
	// single NWScriptProgram instance.
	//

	NWScriptJITIntrinsics(
		__in INWScriptProgram ^ Program
		);

	//
	// Destruct a NWScriptJITIntrinsics instance.
	//

	~NWScriptJITIntrinsics(
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

	void
	Intrinsic_VMStackPushInt(
		__in Int32 i
		);

	//
	// Pop an integer value off of the VM stack (for an action call).
	//

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

	String ^
	Intrinsic_VMStackPopString(
		);

	//
	// Push an object id value onto the VM stack (for an action call).
	//

	void
	Intrinsic_VMStackPushObjectId(
		__in UInt32 o
		);

	//
	// Pop an object id value off of the stack (for an action call).
	//

	UInt32
	Intrinsic_VMStackPopObjectId(
		);

	//
	// Push an engine structure value onto the VM stack (for an action call),
	// or pop an engine structure value off the VM stack.
	//

#define DECLARE_ENGINE_STRUCTURE( n )                               \
	inline                                                          \
	void                                                            \
	Intrinsic_VMStackPushEngineStructure##n(                        \
	    __in NWScriptEngineStructure##n ^ EngineStructure           \
	    )                                                           \
	{                                                               \
	   m_Program->Intrinsic_VMStackPushEngineStructure(             \
	       EngineStructure->m_EngineStructure                       \
	       );                                                       \
	}                                                               \
	                                                                \
	inline                                                          \
	NWScriptEngineStructure##n ^                                    \
	Intrinsic_VMStackPopEngineStructure##n(                         \
	    )                                                           \
	{                                                               \
	   NWScriptEngineStructure##n ^ EngStruct;                      \
	                                                                \
	   EngStruct = gcnew NWScriptEngineStructure##n(                \
	       m_Program->Intrinsic_VMStackPopEngineStructure( n )      \
	       );                                                       \
	                                                                \
	   return EngStruct;                                            \
	}                                                               \
	                                                                \
	inline                                                          \
	bool                                                            \
	Intrinsic_CompareEngineStructure##n(                            \
	    __in NWScriptEngineStructure##n ^ EngineStructure1,         \
	    __in NWScriptEngineStructure##n ^ EngineStructure2          \
	    )                                                           \
	{                                                               \
		return m_Program->Intrinsic_CompareEngineStructure(         \
		    EngineStructure1->m_EngineStructure,                    \
	        EngineStructure2->m_EngineStructure                     \
	        );                                                      \
	}                                                               \
	                                                                \
	inline                                                          \
	NWScriptEngineStructure##n ^                                    \
	Intrinsic_CreateEngineStructure##n(                             \
	    )                                                           \
	{                                                               \
	   NWScriptEngineStructure##n ^ EngStruct;                      \
	                                                                \
	   EngStruct = gcnew NWScriptEngineStructure##n(                \
	       m_Program->Intrinsic_CreateEngineStructure( n )          \
	       );                                                       \
	                                                                \
	   return EngStruct;                                            \
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

	//
	// Execute a call to the script host's action service handler.
	//

	void
	Intrinsic_ExecuteActionService(
		__in UInt32 ActionId,
		__in UInt32 NumArguments
		);

	//
	// Store the state of the script program for retrieval later, so that a
	// script situation can be executed.
	//

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

	Object ^
	Intrinsic_ExecuteActionServiceFast(
		__in UInt32 ActionId,
		__in UInt32 NumArguments,
		__in ... array< Object ^ > ^ Arguments
		);

	//
	// Allocate a neutral string given a String object.
	//

	NWScript::NeutralStringStorage
	Intrinsic_AllocateNeutralString(
		__in String ^ Str
		);

	//
	// Release a neutral string (if it was allocated).
	//

	void
	Intrinsic_DeleteNeutralString(
		__in NWScript::NeutralStringStorage % Str
		);

	//
	// Create a String from a neutral string object.
	//

	String ^
	Intrinsic_NeutralStringToString(
		__in NWScript::NeutralStringStorage % Str
		);

	//
	// Raise an abort exception if necessary.
	//

	void
	Intrinsic_CheckScriptAbort(
		__in bool ActionSucceeded
		);

private:

	//
	// Define the associated program interface.
	//

	INWScriptProgram      ^ m_Program;

};

}

#endif
