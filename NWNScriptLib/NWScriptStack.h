/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptStack.h

Abstract:

	This module defines the NWScriptStack object, which represents the
	data stack of a script VM (or a saved script situation).  Management
	routines to allow objects to be placed on (or retrieved from) the script
	stack are provided.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTSTACK_H
#define _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTSTACK_H

#ifdef _MSC_VER
#pragma once
#endif

class EngineStructure;

typedef swutil::SharedPtr< EngineStructure > EngineStructurePtr;



//
// Define the NWScriptStack manipulation interface.  The NWNScriptJIT logic
// CANNOT directly interface with the NWNScriptVM, because the wrong operator
// new / operator delete would be invoked, leading to catastrophic memory
// corruption.
//

struct INWScriptStack
{

public:

	typedef UCHAR ENGINE_STRUCTURE_NUMBER;

	//
	// Define a module-neutral string storage object (pair of buffer, length).
	//
	// Allocations are performed via AllocNeutral / FreeNeutral.
	//

	typedef std::pair< char *, size_t > NeutralString;

	virtual
	void
	StackPushInt(
		__in int Int
		) = 0;

	virtual
	int
	StackPopInt(
		) = 0;


	virtual
	void
	StackPushFloat(
		__in float Float
		) = 0;

	virtual
	float
	StackPopFloat(
		) = 0;


	virtual
	void
	StackPushString(
		__in const char * String
		) = 0;

	virtual
	void
	StackPushString(
		__in const std::string & String
		) = 0;

	virtual
	void
	StackPushStringAsNeutral(
		__in const NeutralString & String
		) = 0;

	virtual
	NeutralString
	StackPopStringAsNeutral(
		) = 0;


	virtual
	void
	StackPushObjectId(
		__in NWN::OBJECTID ObjectId
		) = 0;

	virtual
	NWN::OBJECTID
	StackPopObjectId(
		) = 0;


	virtual
	void
	StackPushVector(
		__in const NWN::Vector3 & Vector
		) = 0;

	virtual
	NWN::Vector3
	StackPopVector(
		) = 0;


	virtual
	void
	StackPushEngineStructure(
		__in EngineStructurePtr Struct
		) = 0;

	virtual
	EngineStructurePtr
	StackPopEngineStructure(
		__in ENGINE_STRUCTURE_NUMBER EngineType
		) = 0;


	typedef enum _BASE_STACK_TYPE
	{
		BST_INT,
		BST_FLOAT,
		BST_OBJECTID,
		BST_STRING,

		BST_ENGINE_0,
		BST_ENGINE_1,
		BST_ENGINE_2,
		BST_ENGINE_3,
		BST_ENGINE_4,
		BST_ENGINE_5,
		BST_ENGINE_6,
		BST_ENGINE_7,
		BST_ENGINE_8,
		BST_ENGINE_9,

		BST_STACK_POINTER,

		BST_INVALID
	} BASE_STACK_TYPE, * PBASE_STACK_TYPE;

	typedef const _BASE_STACK_TYPE * PCBASE_STACK_TYPE;

	//
	// Classify the type of an item on the stack.
	//

	virtual
	BASE_STACK_TYPE
	GetTopOfStackType(
		) const = 0;


};

//
// Define the primary NWScriptStack object.  The stack represents the primary
// data store for the script VM.  Additionally, it provides a parameter passing
// mechanism for action service handler calls.
//

class NWScriptStack : public INWScriptStack
{

public:

	typedef swutil::SharedPtr< NWScriptStack > Ptr;

	//
	// Exceptions may (but are not required to be) derived from these types.
	// If possible the derived types are used for more detailed error
	// information, but memory allocation failures may still be reported as a
	// standard std::bad_alloc (or the like).
	//

	class type_mismatch_exception   : public std::runtime_error { public: type_mismatch_exception( __in const char * s ) : std::runtime_error( s ) {} };
	class stack_underflow_exception : public std::runtime_error { public: stack_underflow_exception( __in const char * s ) : std::runtime_error( s ) {} };
	class stack_overflow_exception  : public std::runtime_error { public: stack_overflow_exception( __in const char * s ) : std::runtime_error( s ) {} };
	class invalid_handle_exception  : public std::runtime_error { public: invalid_handle_exception( __in const char * s ) : std::runtime_error( s ) {} };
	class invalid_stack_exception   : public std::runtime_error { public: invalid_stack_exception( __in const char * s ) : std::runtime_error( s ) {} };
	class guard_zone_exception      : public std::runtime_error { public: guard_zone_exception( __in const char * s ) : std::runtime_error( s ) {} };

	//
	// Define the type of an engine structure type number.
	//
	// Legal values are [0...LAST_ENGINE_STRUCTURE]
	//

	typedef UCHAR ENGINE_STRUCTURE_NUMBER;
	C_ASSERT( sizeof( ENGINE_STRUCTURE_NUMBER ) == 1 );

	//
	// Define the type of a stack pointer.
	//

	typedef LONG STACK_POINTER;
	C_ASSERT( sizeof( STACK_POINTER ) == 4 );

	//
	// Define a pointer to an instruction.
	//

	typedef ULONG PROGRAM_COUNTER;

	//
	// Define a module-neutral string storage object (pair of buffer, length).
	//
	// Allocations are performed via AllocNeutral / FreeNeutral.
	//

	typedef std::pair< char *, size_t > NeutralString;

	//
	// Create a new script stack.
	//

	NWScriptStack(
		__in NWN::OBJECTID InvalidObjId = NWN::INVALIDOBJID
		);

	~NWScriptStack(
		);

	//
	// Stack manipulation; all routines may raise an std::exception on failure.
	//


	void
	StackPushDynamicParameter(
		__in const char * String
		);


	void
	StackPushInt(
		__in int Int
		);

	int
	StackPopInt(
		);


	void
	StackPushFloat(
		__in float Float
		);

	float
	StackPopFloat(
		);


	void
	StackPushString(
		__in const char * String
		);

	void
	StackPushString(
		__in const std::string & String
		);

	void
	StackPushStringAsNeutral(
		__in const NeutralString & String
		);

	std::string
	StackPopString(
		);

	NeutralString
	StackPopStringAsNeutral( // Return module-neutral string
		);


	void
	StackPushObjectId(
		__in NWN::OBJECTID ObjectId
		);

	NWN::OBJECTID
	StackPopObjectId(
		);


	void
	StackPushVector(
		__in const NWN::Vector3 & Vector
		);

	NWN::Vector3
	StackPopVector(
		);


	void
	StackPushEngineStructure(
		__in EngineStructurePtr Struct
		);

	EngineStructurePtr
	StackPopEngineStructure(
		__in ENGINE_STRUCTURE_NUMBER EngineType
		);


	//
	// Stack pointer access.
	//

	//
	// Return the current top of stack (SP).
	//

	STACK_POINTER
	GetCurrentSP(
		) const;

	//
	// Return the current base pointer (BP).
	//

	STACK_POINTER
	GetCurrentBP(
		) const;

	//
	// Get the current return stack depth.
	//

	size_t
	GetReturnStackDepth(
		) const;

	//
	// Return an entry off of the return stack.
	//

	PROGRAM_COUNTER
	GetReturnStackEntry(
		__in size_t Offset
		) const;

	//
	// Adjust SP by a signed displacement (which must be negative).
	//

	void
	AddSP(
		__in STACK_POINTER Displacement
		);

	//
	// Save the current BP and assign the current SP to BP.
	//

	void
	SaveBP(
		);

	//
	// Restore BP to the last saved BP.
	//

	void
	RestoreBP(
		);

	//
	// Set the current BP to a new absolute SP value, which must bei n range.
	//

	void
	SetCurrentBP(
		__in STACK_POINTER AbsoluteBP
		);

	//
	// Save the program counter.
	//

	void
	SaveProgramCounter(
		__in PROGRAM_COUNTER ProgramCounter
		);

	//
	// Restore the previous program counter for a return sequence.
	//

	PROGRAM_COUNTER
	RestoreProgramCounter(
		);

	//
	// Access the value at a displacement relative to the stack (which must be
	// negative).
	//

	void
	SetStackInt(
		__in STACK_POINTER Displacement,
		__in int Int
		);

	int
	GetStackInt(
		__in STACK_POINTER Displacement
		) const;


	void
	SetStackFloat(
		__in STACK_POINTER Displacement,
		__in float Float
		);

	float
	GetStackFloat(
		__in STACK_POINTER Displacement
		) const;


	void
	SetStackString(
		__in STACK_POINTER Displacement,
		__in const char * String
		);

	const std::string &
	GetStackString(
		__in STACK_POINTER Displacement
		) const;


	void
	SetStackObjectId(
		__in STACK_POINTER Displacement,
		__in NWN::OBJECTID ObjectId
		);

	NWN::OBJECTID
	GetStackObjectId(
		__in STACK_POINTER Displacement
		) const;


	void
	SetStackVector(
		__in STACK_POINTER Displacement,
		__in const NWN::Vector3 & Vector
		);

	NWN::Vector3
	GetStackVector(
		__in STACK_POINTER Displacement
		) const;


	void
	SetStackEngineStructure(
		__in STACK_POINTER Displacement,
		__in EngineStructurePtr Struct
		);

	EngineStructurePtr
	GetStackEngineStructure(
		__in STACK_POINTER Displacement,
		__in ENGINE_STRUCTURE_NUMBER EngineType
		) const;


	//
	// Copy stack cells from the top of the stack into an offset relative to
	// the top of the stack (must be negative).  This is often used after a
	// function call to copy the return value of a call into a local variable.
	//
	// The CopyDownSP function implements a general assignment (copy) of one
	// set of stack variables that already exist to another set.
	//
	// The destination space must be reserved or initialized and have the
	// correct type for the bytes being copied.
	//

	void
	CopyDownSP(
		__in STACK_POINTER Destination,
		__in STACK_POINTER BytesToCopy,
		__in bool UseBP = false
		);


	//
	// Duplicate stack cells into the top of the stack (that is, creating new
	// stack cells) relative to a displacement from the original top of the
	// stack.  This is used to allocate a (new) copy of an existing local
	// variable set, such as for a function call parameter.
	//

	void
	CopyTopSP(
		__in STACK_POINTER Source,
		__in STACK_POINTER BytesToCopy,
		__in bool UseBP = false
		);


	//
	// Save a section of the stack away for later restoration.
	//
	// Note that the BP save and return stacks are not saved.  If the caller
	// requires these to be preserved then the entire stack must be copied with
	// operator=.
	//

	NWScriptStack
	SaveStack(
		__in STACK_POINTER BPSaveBytes,
		__in STACK_POINTER SPSaveBytes,
		__in STACK_POINTER SPSaveOffset = 0
		);

	//
	// Save a section of the stack away for later restoration.
	//
	// Note that the BP save and return stacks are not saved.  If the caller
	// requires these to be preserved then the entire stack must be copied with
	// operator=.
	//
	// Note that save to INWScriptStack loses type information for dynamic
	// parameters and for the saved BP value.  It is also less efficient than a
	// save to a conventional NWScriptStack.
	//

	void
	SaveStack(
		__in INWScriptStack * Stack,
		__in STACK_POINTER BPSaveBytes,
		__in STACK_POINTER SPSaveBytes,
		__in STACK_POINTER SPSaveOffset = 0
		);

	//
	// Delete a series of elements on the stack, with a section of the deleted
	// space that is preserved.  This allows a range of local varialbes to be
	// deallocated except for a 'hole' that is kept.
	//

	void
	DestructElements(
		__in STACK_POINTER BytesToRemove,
		__in STACK_POINTER ExcludePointer,
		__in STACK_POINTER BytesToExclude
		);


	//
	// Increment a value at an absolute stack address.
	//

	int
	IncrementStackInt(
		__in STACK_POINTER AbsoluteAddress
		);

	//
	// Decrement a value at an absolute stack address.
	//

	int
	DecrementStackInt(
		__in STACK_POINTER AbsoluteAddress
		);


	//
	// Return the size of an integer on the stack.
	//

	STACK_POINTER
	GetStackIntegerSize(
		) const;


	//
	// Determine whether the return stack is empty.
	//

	inline
	bool
	IsReturnStackEmpty(
		) const
	{
		return m_ReturnStack.empty( );
	}


	//
	// Peek at the stack for debugging purposes only.
	//

	bool
	PeekStack(
		__in STACK_POINTER AbsoluteAddress,
		__out ULONG & RawValue,
		__out UCHAR & RawType
		) const;

	//
	// Check if a value is an engine structure (for debugging purposes only).
	//

	bool
	DebugIsEngineStructureType(
		__in UCHAR RawType
		) const;

	//
	// Check if the top of stack is really a saved BP.  This is required for
	// the GUI script wrong parameter number work around.
	//

	bool
	IsParameterUnderrunRestoreBP(
		) const;


	//
	// Classify the type of an item on the stack.
	//

	BASE_STACK_TYPE
	GetStackType(
		__in STACK_POINTER AbsoluteAddress
		) const;

	//
	// Classify the type of the item on the top of the stack.
	//

	virtual
	BASE_STACK_TYPE
	GetTopOfStackType(
		) const;


	//
	// Reset the stack to a clean state.
	//

	void
	ResetStack(
		);


	//
	// Establish a guard zone.
	//

	void
	EstablishGuardZone(
		);

	//
	// De-establish a guard zone previously set up by a call to the
	// EstablishGuardZone routine.
	//

	void
	DeestablishGuardZone(
		);

	//
	// Check an SP-relative access agains the guard zone.
	//

	void
	CheckGuardZone(
		__in STACK_POINTER AbsoluteAddress
		);

	//
	// Assign the default invalid object id.
	//

	void
	SetInvalidObjId(
		__in NWN::OBJECTID InvalidObjId
		);

	//
	// Retrieve the default invalid object id.
	//

	NWN::OBJECTID
	GetInvalidObjId(
		) const;

	//
	// Module-neutral string allocation and free.
	//

	inline
	static
	void *
	AllocNeutral(
		__in size_t s
		)
	{
#ifndef _WIN32
		return new char[ s ];
#else
		void * p;

		if (s == 0)
			return NULL;

		p = (void *) HeapAlloc( GetProcessHeap( ), 0, s );

		if (p == NULL)
			throw std::bad_alloc( );

		return p;
#endif
	}

	inline
	static
	void
	FreeNeutral(
		__in void * p
		)
	{
#ifndef _WIN32
		delete [] (char *) p;
#else
		if (p == NULL)
			return;

		HeapFree( GetProcessHeap( ), 0, p );
#endif
	}

private:

	//
	// Define type codes for the execution stack.  Each stack slot is tagged
	// with the data type that was loaded into it, and attempts to use a wrong
	// data type (except for SET_DYNAMIC) result in an error.
	//

	typedef enum _STACK_ENTRY_TYPE
	{
		SET_INVALID                = 0,      // Illegal to reference directly
		SET_INTEGER                = 1 << 0, // signed integer (32-bit)
		SET_FLOAT                  = 1 << 1, // floating point (32-bit)
		SET_STRING                 = 1 << 2, // string [handle]
		SET_OBJECTID               = 1 << 3, // NWN::OBJECTID
		SET_VECTOR                 = 1 << 4, // First member has SET_VECTOR | SET_FLOAT, subsequent SET_FLOAT
		SET_STRUCTURE              = 1 << 5, // First member has SET_STRUCTURE | SET_xxx, subsequent SET_xxx
		SET_DYNAMIC                = 1 << 6, // String handle, convert on demand
		SET_ENGINE_STRUCTURE       = 1 << 7, // If set, remaining bits are engine struct ordinal [0-126]
		SET_STACK_POINTER          = SET_ENGINE_STRUCTURE | 127, // Stack pointer (i.e. SaveBP).

		LAST_SET_TYPE
	} STACK_ENTRY_TYPE, * PSTACK_ENTRY_TYPE;

	typedef const enum _STACK_ENTRY_TYPE * PCSTACK_ENTRY_TYPE;

	typedef UCHAR STACK_TYPE_CODE;

	C_ASSERT( sizeof( STACK_TYPE_CODE ) == 1 );

	//
	// Define a stack of type codes, parallel to the main stack.
	//

	typedef std::vector< STACK_TYPE_CODE > VMTypeStack;


	//
	// Define the string handle type, which is an index into the string stack.
	// Instead of pointers to strings, we store string handles, which are then
	// used to retrieve strings from the string stack.
	//

	typedef ULONG STRING_HANDLE;

	C_ASSERT( sizeof( STRING_HANDLE ) == 4 );

	
	//
	// Define a stack of strings, indexed by STRING_HANDLEs.
	//

	typedef std::vector< std::string > StringStack;


	//
	// Define the engine handle type, which is an index into the engine
	// structure stack.  Instead of pointers to engine structures, we store
	// engine handles, which are used to retrieve structures from the stack.
	// 

	typedef ULONG ENGINE_HANDLE;

	C_ASSERT( sizeof( ENGINE_HANDLE ) == 4 );


	//
	// Define a stack of engine structures, indexed by ENGINE_HANDLEs.
	//

	typedef std::vector< EngineStructurePtr > EngineStructStack;


	//
	// Define the underlying stack cell type, which must be 4 bytes wide.  Each
	// entry represents a slot in the virtual machine stack.  A parallel array
	// of STACK_TYPE_CODES represents the type of each stack cell.
	//

	typedef struct _STACK_ENTRY
	{
		union
		{
			int              Int;            // 32-bit signed integer
			float            Float;          // 32-bit float
			NWN::OBJECTID    ObjectId;       // Object reference
			STRING_HANDLE    String;         // String [handle]
			STRING_HANDLE    Dynamic;        // String, convert on demand
			ENGINE_HANDLE    EngineStruct;   // Engine structure [handle]
			STACK_POINTER    StackPointer;   // Saved SP/BP value
			ULONG            Raw;            // Raw value
		};
	} STACK_ENTRY, * PSTACK_ENTRY;

	typedef const struct _STACK_ENTRY * PCSTACK_ENTRY;

	enum { STACK_ENTRY_SIZE = sizeof( STACK_ENTRY ) };
	enum { STACK_MAXIMUM_SIZE = 1 * 1024 * 1024 }; // 1MB max stack

	C_ASSERT( STACK_ENTRY_SIZE == 4 );

	//
	// Define a stack of stack entries (i.e. main stack).
	//

	typedef std::vector< STACK_ENTRY > VMStack;


	//
	// Define a stack of saved stack pointers (i.e. for SaveBP).
	//

	typedef std::vector< STACK_POINTER > StackPtrStack;


	//
	// Define a stack of saved program counter values (i.e. return addresses).
	//

	typedef std::vector< PROGRAM_COUNTER > SavedPCStack;

	//
	// Push an entry onto the stack.
	//

	void
	StackPushRaw(
		__in STACK_ENTRY StackEntry,
		__in STACK_TYPE_CODE StackEntryType
		);

	//
	// Return a raw value from the stack.
	//

	STACK_ENTRY
	StackPopRaw(
		__in STACK_TYPE_CODE StackEntryType
		);

	void
	StackPopRaw(
		__out STACK_ENTRY & Entry,
		__out STACK_TYPE_CODE & Type
		);

	//
	// Change the value of a dynamic stack entry.
	//

	void
	SetDynamicStackEntry(
		__in size_t Offset,
		__in int Int
		);

	void
	SetDynamicStackEntry(
		__in size_t Offset,
		__in float Float
		);

	void
	SetDynamicStackEntry(
		__in size_t Offset,
		__in const char * String
		);

	void
	SetDynamicStackEntry(
		__in size_t Offset,
		__in NWN::OBJECTID ObjectId
		);

	//
	// Return the value of a dynamic stack entry.
	//

	int
	GetDynamicStackEntryInteger(
		__in size_t Offset
		) const;

	float
	GetDynamicStackEntryFloat(
		__in size_t Offset
		) const;

	const std::string &
	GetDynamicStackEntryString(
		__in size_t Offset
		) const;

	NWN::OBJECTID
	GetDynamicStackEntryObjectId(
		__in size_t StackOffset
		) const;



	//
	// Expand the stack to support a given number of additional slots.
	//

	void
	GrowStack(
		__in size_t NumSlots
		);

	//
	// Append a section of the stack into a new stack.
	//

	void
	AppendStackContentsToStack(
		__in NWScriptStack & DestStack,
		__in size_t SrcOffset,
		__in size_t CellsToCopy
		);

	//
	// Append a section of the stack into a new stack.
	//

	void
	AppendStackContentsToStack(
		__in INWScriptStack * DestStack,
		__in size_t SrcOffset,
		__in size_t CellsToCopy
		);





	//
	// Now define execution variables.
	//

	//
	// Define the stack of saved return PC values.
	//

	SavedPCStack      m_ReturnStack;

	//
	// Define the main exection stack itself.
	//

	VMStack           m_Stack;

	//
	// Define the type stack.  The type stack is parallel to m_Stack, except
	// that it is indexed in +1 increments (vs +4 increments).
	//

	VMTypeStack       m_StackTypes;

	//
	// Define the string stack, which is referenced by STRING_HANDLEs.
	//

	StringStack       m_StackStrings;

	//
	// Define the engine structure stack, which is referenced by ENGINE_HANDLE
	// entries.
	//

	EngineStructStack m_StackEngineStructures;

	//
	// Define the current base pointer value.
	//

	STACK_POINTER     m_BP;

	//
	// Define the guard zone stack.  If we have a guard zone defined at the top
	// of the guard zone stack, no SP-relative references may pass below it
	// without raising a guard zone violation exception.
	//

	StackPtrStack     m_GuardZoneStack;

	//
	// Define the default invalid object id.
	//

	NWN::OBJECTID     m_InvalidObjId;

};

//
// Define the base engine structure class, from which all implementation
// defined structures that may be pushed onto the VM stack must be derived.
//

class EngineStructure
{

public:

	typedef swutil::SharedPtr< EngineStructure >  Ptr;
	typedef NWScriptStack::ENGINE_STRUCTURE_NUMBER ENGINE_STRUCTURE_NUMBER;

	inline
	EngineStructure(
		__in ENGINE_STRUCTURE_NUMBER EngineType
		)
	: m_EngineType( EngineType )
	{
	}

	inline
	virtual
	~EngineStructure(
		)
	{
	}

	//
	// Return the type code of the structure.
	//

	inline
	ENGINE_STRUCTURE_NUMBER
	GetEngineType(
		) const
	{
		return m_EngineType;
	}

	//
	// Compare to structures of identical types.  The routine returns ture if
	// the structures are logically identical, else false is returned.
	//

	virtual
	bool
	CompareEngineStructure(
		__in const EngineStructure * Other
		) const = 0;

	//
	// Because EngineStructure objects may be allocated by the main script host
	// module, but deleted by the NWScript JIT engine, it is necessary to
	// enable all EngineStructure-derived objects for cross-module allocation
	// and deletion.
	//
	// The contract is that the main script host module always performs the
	// underlying allocations, but the NWScript JIT engine (which is permitted
	// to reside in a separate DLL as an optional component) may perform frees
	// in some circumstances.
	//

	DECLARE_SWUTIL_CROSS_MODULE_NEW( );

private:

	//
	// Define the engine structure type ordinal.
	//

	ENGINE_STRUCTURE_NUMBER m_EngineType;

};


#endif
