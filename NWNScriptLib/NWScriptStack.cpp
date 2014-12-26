/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptStack.cpp

Abstract:

	This module houses the NWScriptStack object, which represents the
	data stack of a script VM (or a saved script situation).  Management
	routines to allow objects to be placed on (or retrieved from) the script
	stack are provided.

--*/

#include "Precomp.h"
#include "NWScriptStack.h"

//
// Define the uninitialized fill for not-yet-allocated stack cells.
//

#define UNINITIALIZED_FILL (0xCDCDCDCD)

//
// Define to enable extended stack debugging.  This enforces the following
// constraint checks that are only necessary for internal consistency checks
// (as they represent boundary conditions verified elsewhere):
//
// - All handle references on the stack are valid, and
// - All handle references on the stack are destructed in the correct order,
//   and,
// - All stack pointers are aligned to multiples of a stack cell.
//

#define STACK_DEBUG 1

//
// Define to allow saved BP values to be treated as integers on the stack and
// not their own unique (typesafe) type.
//

#define STACK_SAVEBP_CONVERT_TO_INTEGER 1



NWScriptStack::NWScriptStack(
	__in NWN::OBJECTID InvalidObjId /* = NWN::INVALIDOBJID */
	)
/*++

Routine Description:

	This routine constructs a new NWScriptStack.

Arguments:

	InvalidObjId - Supplies the invalid object id to substitute when an empty
	               dynamic parameter is converted to an object id.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
: m_BP( 0 ),
  m_InvalidObjId( InvalidObjId )
{
}

NWScriptStack::~NWScriptStack(
	)
/*++

Routine Description:

	This routine deletes the current NWScriptStack object and its associated
	members.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
}

void
NWScriptStack::StackPushDynamicParameter(
	__in const char * String
	)
/*++

Routine Description:

	This routine pushes a dynamically-typed parameter onto the stack.  The
	parameter is provided as a string, and is converted on the fly when it is
	referenced.

	String parameters are used as-is, integer and object id parameters are
	converted from base 10 (signed).  Floating point parameters are converted
	from printf format %g.

Arguments:

	String - Supplies the value to push onto the stack.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	STACK_ENTRY Entry;

	if (m_StackStrings.size( ) == ULONG_MAX)
		throw stack_overflow_exception( "out of string stack space" );

	m_StackStrings.push_back( std::string( String ) );

	try
	{
		Entry.String = (STRING_HANDLE) (m_StackStrings.size( ) - 1);

		StackPushRaw( Entry, SET_DYNAMIC );
	}
	catch (...)
	{
		m_StackStrings.pop_back( );
		throw;
	}
}

void
NWScriptStack::StackPushInt(
	__in int Int
	)
/*++

Routine Description:

	This routine pushes an integer onto the stack.

Arguments:

	Int - Supplies the value to push onto the stack.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	STACK_ENTRY Entry;

	Entry.Int = Int;

	StackPushRaw( Entry, SET_INTEGER );
}

int
NWScriptStack::StackPopInt(
	)
/*++

Routine Description:

	This routine returns an integer from the top of the stack.

Arguments:

	None.

Return Value:

	The routine returns the integer at the top of the stack.  An std::exception
	is raised on failure (e.g. type mismatch).

Environment:

	User mode.

--*/
{
	return StackPopRaw( SET_INTEGER ).Int;
}

void
NWScriptStack::StackPushFloat(
	__in float Float
	)
/*++

Routine Description:

	This routine pushes a float onto the stack.

Arguments:

	Float - Supplies the value to push onto the stack.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	STACK_ENTRY Entry;

	Entry.Float = Float;

	StackPushRaw( Entry, SET_FLOAT );
}

float
NWScriptStack::StackPopFloat(
	)
/*++

Routine Description:

	This routine returns a floating-point value from the top of the stack.

Arguments:

	None.

Return Value:

	The routine returns the floating-point value at the top of the stack.  An
	std::exception is raised on failure (e.g. type mismatch).

Environment:

	User mode.

--*/
{
	return StackPopRaw( SET_FLOAT ).Float;
}

void
NWScriptStack::StackPushString(
	__in const char * String
	)
/*++

Routine Description:

	This routine pushes a string onto the stack.

Arguments:

	String - Supplies the value to push onto the stack.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	STACK_ENTRY Entry;

	if (m_StackStrings.size( ) == ULONG_MAX)
		throw stack_overflow_exception( "out of string stack space" );

	m_StackStrings.push_back( std::string( String ) );

	try
	{
		Entry.String = (STRING_HANDLE) (m_StackStrings.size( ) - 1);

		StackPushRaw( Entry, SET_STRING );
	}
	catch (...)
	{
		m_StackStrings.pop_back( );
		throw;
	}
}

void
NWScriptStack::StackPushString(
	__in const std::string & String
	)
/*++

Routine Description:

	This routine pushes a string onto the stack.

Arguments:

	String - Supplies the value to push onto the stack.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	STACK_ENTRY Entry;

	if (m_StackStrings.size( ) == ULONG_MAX)
		throw stack_overflow_exception( "out of string stack space" );

	m_StackStrings.push_back( String );

	try
	{
		Entry.String = (STRING_HANDLE) (m_StackStrings.size( ) - 1);

		StackPushRaw( Entry, SET_STRING );
	}
	catch (...)
	{
		m_StackStrings.pop_back( );
		throw;
	}
}

void
NWScriptStack::StackPushStringAsNeutral(
	__in const NeutralString & String
	)
/*++

Routine Description:

	This routine pushes a string onto the stack.

Arguments:

	String - Supplies the value to push onto the stack.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	STACK_ENTRY Entry;

	if (m_StackStrings.size( ) == ULONG_MAX)
		throw stack_overflow_exception( "out of string stack space" );

	m_StackStrings.push_back( std::string( String.first, String.second ) );

	try
	{
		Entry.String = (STRING_HANDLE) (m_StackStrings.size( ) - 1);

		StackPushRaw( Entry, SET_STRING );
	}
	catch (...)
	{
		m_StackStrings.pop_back( );
		throw;
	}
}

std::string
NWScriptStack::StackPopString(
	)
/*++

Routine Description:

	This routine returns a string value from the top of the stack.

Arguments:

	None.

Return Value:

	The routine returns the string value at the top of the stack.  An
	std::exception is raised on failure (e.g. type mismatch).

Environment:

	User mode.

--*/
{

	STRING_HANDLE StringHandle = StackPopRaw( SET_STRING ).String;

#if STACK_DEBUG
	if ((size_t) StringHandle != m_StackStrings.size( ) - 1)
		throw invalid_handle_exception( "invalid string handle" );
#endif

	std::string   String( m_StackStrings.back( ) );

	m_StackStrings.pop_back( );

	return String;
}

NWScriptStack::NeutralString
NWScriptStack::StackPopStringAsNeutral(
	)
/*++

Routine Description:

	This routine returns a string value from the top of the stack.

Arguments:

	None.

Return Value:

	The routine returns the string value at the top of the stack.  An
	std::exception is raised on failure (e.g. type mismatch).

Environment:

	User mode.

--*/
{

	STRING_HANDLE StringHandle = StackPopRaw( SET_STRING ).String;

#if STACK_DEBUG
	if ((size_t) StringHandle != m_StackStrings.size( ) - 1)
		throw invalid_handle_exception( "invalid string handle" );
#endif

	NeutralString String;

	String.first  = NULL;
	String.second = 0;

	if (!m_StackStrings.back( ).empty( ))
	{
		String.second = m_StackStrings.back( ).size( );
		String.first  = (char *) AllocNeutral( String.second );

		memcpy( String.first, m_StackStrings.back( ).data( ), String.second );
	}

	m_StackStrings.pop_back( );

	return String;
}


void
NWScriptStack::StackPushObjectId(
	__in NWN::OBJECTID ObjectId
	)
/*++

Routine Description:

	This routine pushes an object id onto the stack.

Arguments:

	ObjectId - Supplies the value to push onto the stack.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	STACK_ENTRY Entry;

	Entry.ObjectId = ObjectId;

	StackPushRaw( Entry, SET_OBJECTID );
}

NWN::OBJECTID
NWScriptStack::StackPopObjectId(
	)
/*++

Routine Description:

	This routine returns an object id value from the top of the stack.

Arguments:

	None.

Return Value:

	The routine returns the object id value at the top of the stack.  An
	std::exception is raised on failure (e.g. type mismatch).

Environment:

	User mode.

--*/
{
	return StackPopRaw( SET_OBJECTID ).ObjectId;
}


void
NWScriptStack::StackPushVector(
	__in const NWN::Vector3 & Vector
	)
/*++

Routine Description:

	This routine pushes a vector onto the stack.

Arguments:

	Vector - Supplies the value to push onto the stack.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	STACK_ENTRY Entry;

	//
	// Push the vector onto the stack -- the first entry is tagged as a vector
	// to indicate that vector comparisons are valid.
	//

	Entry.Float = Vector.x;

	StackPushRaw( Entry, SET_VECTOR | SET_FLOAT); 
	StackPushFloat( Vector.y );
	StackPushFloat( Vector.z );
}

NWN::Vector3
NWScriptStack::StackPopVector(
	)
/*++

Routine Description:

	This routine returns a vector value from the top of the stack.

Arguments:

	None.

Return Value:

	The routine returns the vector value at the top of the stack.  An
	std::exception is raised on failure (e.g. type mismatch).

Environment:

	User mode.

--*/
{
	NWN::Vector3 Vector;

	Vector.z = StackPopFloat( );
	Vector.y = StackPopFloat( );
	Vector.x = StackPopRaw( SET_VECTOR | SET_FLOAT ).Float;

	return Vector;
}


void
NWScriptStack::StackPushEngineStructure(
	__in EngineStructurePtr Struct
	)
/*++

Routine Description:

	This routine pushes an engine structure onto the stack.

Arguments:

	Struct - Supplies the value to push onto the stack.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	STACK_ENTRY Entry;

	if (m_StackEngineStructures.size( ) == ULONG_MAX)
		throw stack_overflow_exception( "out of engine structure stack space" );

	m_StackEngineStructures.push_back( Struct );

	try
	{
		Entry.EngineStruct = (ENGINE_HANDLE) (m_StackEngineStructures.size( ) - 1);

		StackPushRaw( Entry, SET_ENGINE_STRUCTURE | Struct->GetEngineType( ) );
	}
	catch (...)
	{
		m_StackEngineStructures.pop_back( );
		throw;
	}
}

EngineStructurePtr
NWScriptStack::StackPopEngineStructure(
	__in ENGINE_STRUCTURE_NUMBER EngineType
	)
/*++

Routine Description:

	This routine returns an engine structure from the top of the stack.

Arguments:

	EngineType - Supplies the type number of the engine structure that is
	             expected to be at the top of the stack.

Return Value:

	The routine returns the engine structure value at the top of the stack.  An
	std::exception is raised on failure (e.g. type mismatch).

Environment:

	User mode.

--*/
{
	ENGINE_HANDLE      EngineStructureHandle = StackPopRaw( SET_ENGINE_STRUCTURE | EngineType ).EngineStruct;

	if ((size_t) EngineStructureHandle != m_StackEngineStructures.size( ) - 1)
		throw invalid_handle_exception( "invalid engine structure handle" );

	EngineStructurePtr EngineStructure( m_StackEngineStructures.back( ) );

	m_StackEngineStructures.pop_back( );

	return EngineStructure;
}

NWScriptStack::STACK_POINTER
NWScriptStack::GetCurrentSP(
	) const
/*++

Routine Description:

	This routine returns the current logical stack pointer.

Arguments:

	None.

Return Value:

	The routine returns the current logical stack pointer.

Environment:

	User mode.

--*/
{
	return (STACK_POINTER) (m_Stack.size( ) * STACK_ENTRY_SIZE);
}

NWScriptStack::STACK_POINTER
NWScriptStack::GetCurrentBP(
	) const
/*++

Routine Description:

	This routine returns the current logical base pointer.

Arguments:

	None.

Return Value:

	The routine returns the current logical base pointer.

Environment:

	User mode.

--*/
{
	return m_BP;
}

size_t
NWScriptStack::GetReturnStackDepth(
	) const
/*++

Routine Description:

	This routine returns the current return stack depth.

Arguments:

	None.

Return Value:

	The routine returns the current return stack depth.

Environment:

	User mode.

--*/
{
	return m_ReturnStack.size( );
}

NWScriptStack::PROGRAM_COUNTER
NWScriptStack::GetReturnStackEntry(
	__in size_t Offset
	) const
/*++

Routine Description:

	This routine returns an entry from the current return stack depth.

Arguments:

	None.

Return Value:

	On success, the routine returns the requested return stack pointer.
	On failure, an exception is raised.

Environment:

	User mode.

--*/
{
#if STACK_DEBUG
	if (Offset >= m_ReturnStack.size( ))
		throw std::invalid_argument( "offset must point into the return stack" );
#endif

	return m_ReturnStack[ Offset ];
}

void
NWScriptStack::AddSP(
	__in STACK_POINTER Displacement
	)
/*++

Routine Description:

	This routine adjusts the pointer by a signed displacement.

Arguments:

	Displacement - Supplies the stack displacement value.

Return Value:

	None.

Environment:

	User mode.

--*/
{
#if STACK_DEBUG
	if (Displacement > 0)
		throw std::invalid_argument( "displacement must be negative" );

	if (Displacement & (STACK_ENTRY_SIZE - 1))
		throw std::invalid_argument( "displacement must be a multiple of STACK_ENTRY_SIZE" );
#endif

	Displacement /= STACK_ENTRY_SIZE;

	while (Displacement)
	{
		STACK_ENTRY     Entry;
		STACK_TYPE_CODE Type;

		StackPopRaw( Entry, Type );

		//
		// If we have an engine structure or string, then we need to remove it
		// from the engine structure or string stack.
		//

		if ((Type & SET_ENGINE_STRUCTURE) && (Type != SET_STACK_POINTER))
		{
#if STACK_DEBUG
			if ((size_t) Entry.EngineStruct != m_StackEngineStructures.size( ) - 1)
				throw invalid_handle_exception( "invalid engine structure handle" );
#endif

			m_StackEngineStructures.pop_back( );
		}
		else if ((!(Type & SET_ENGINE_STRUCTURE)) && (Type & (SET_STRING | SET_DYNAMIC)))
		{
#if STACK_DEBUG
			if ((size_t) Entry.String != m_StackStrings.size( ) - 1)
				throw invalid_handle_exception( "invalid string handle" );
#endif

			m_StackStrings.pop_back( );
		}

		Displacement += 1;
	}
}

void
NWScriptStack::SaveBP(
	)
/*++

Routine Description:

	This routine saves saves the current BP value and assigns BP to the current
	SP value.

Arguments:

	None.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	STACK_ENTRY SavedBP;

	SavedBP.StackPointer = GetCurrentBP( ) / STACK_ENTRY_SIZE;

	StackPushRaw( SavedBP, SET_STACK_POINTER );

	m_BP = GetCurrentSP( ) - STACK_ENTRY_SIZE;
}

void
NWScriptStack::RestoreBP(
	)
/*++

Routine Description:

	This routine returns the BP value to that which was last saved on the BP
	save stack.

Arguments:

	None.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	STACK_ENTRY SavedBP = StackPopRaw( SET_STACK_POINTER );

	SavedBP.StackPointer *= STACK_ENTRY_SIZE;

	if (SavedBP.StackPointer > GetCurrentSP( ))
		throw invalid_stack_exception( "saved BP restored past unwind" );

	m_BP = SavedBP.StackPointer;
}

void
NWScriptStack::SetCurrentBP(
	__in STACK_POINTER AbsoluteBP
	)
/*++

Routine Description:

	This routine resets the current BP value to a specified value.

Arguments:

	None.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	if (AbsoluteBP & (STACK_ENTRY_SIZE - 1 ))
		throw std::invalid_argument( "stack pointer must be a multiple of STACK_ENTRY_SIZE" );

	if (AbsoluteBP > GetCurrentSP( ))
		throw invalid_stack_exception( "illegal stack reference" );

	m_BP = AbsoluteBP;
}

void
NWScriptStack::SaveProgramCounter(
	__in PROGRAM_COUNTER ProgramCounter
	)
/*++

Routine Description:

	This routine saves saves the current PC value on the return stack.

Arguments:

	ProgramCounter - Supplies the program counter value to save.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	m_ReturnStack.push_back( ProgramCounter );
}

NWScriptStack::PROGRAM_COUNTER
NWScriptStack::RestoreProgramCounter(
	)
/*++

Routine Description:

	This routine removes the last PC on the return stack and returns it to the
	caller.

Arguments:

	None.

Return Value:

	The routine returns the most previously saved program counter on the return
	stack.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	PROGRAM_COUNTER ProgramCounter;

	if (m_ReturnStack.empty( ))
		throw invalid_stack_exception( "mismatched SaveProgramCounter/RestoreProgramCounter" );

	ProgramCounter = m_ReturnStack.back( );

	m_ReturnStack.pop_back( );

	return ProgramCounter;
}






void
NWScriptStack::SetStackInt(
	__in STACK_POINTER Displacement,
	__in int Int
	)
/*++

Routine Description:

	This routine assigns a value to an integer relative to the current SP.

Arguments:

	Displacement - Supplies the displacement from the current SP.

	Int - Supplies the value to assign.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	size_t Offset;

	Offset = (GetCurrentSP( ) + Displacement) / STACK_ENTRY_SIZE;

	if (Offset >= m_Stack.size( ))
		throw invalid_stack_exception( "illegal stack reference" );

#if STACK_SAVEBP_CONVERT_TO_INTEGER
	if (m_StackTypes[ Offset ] == SET_STACK_POINTER)
	{
		m_Stack[ Offset ].StackPointer = (STACK_POINTER) Int;
		return;
	}
#endif

	if (m_StackTypes[ Offset ] & SET_ENGINE_STRUCTURE)
		throw type_mismatch_exception( "SetStackInt type mismatch" );
	else if (m_StackTypes[ Offset ] & SET_DYNAMIC)
		SetDynamicStackEntry( Offset, Int );
	else if (m_StackTypes[ Offset ] & SET_INTEGER)
		m_Stack[ Offset ].Int = Int;
	else if (m_StackTypes[ Offset ] == SET_INVALID)
	{
		m_Stack[ Offset ].Int  = Int;
		m_StackTypes[ Offset ] = SET_INTEGER;
	}
	else
		throw type_mismatch_exception( "SetStackInt type mismatch" );
}

int
NWScriptStack::GetStackInt(
	__in STACK_POINTER Displacement
	) const
/*++

Routine Description:

	This routine returns an integer relative to the top of the stack.

Arguments:

	Displacement - Supplies the displacement from the current SP.

Return Value:

	The routine returns the integer at the given stack offset.  An
	std::exception is raised on failure (e.g. type mismatch).

Environment:

	User mode.

--*/
{
	size_t Offset;

	Offset = (GetCurrentSP( ) + Displacement) / STACK_ENTRY_SIZE;

	if (Offset >= m_Stack.size( ))
		throw invalid_stack_exception( "illegal stack reference" );

#if STACK_SAVEBP_CONVERT_TO_INTEGER
	if (m_StackTypes[ Offset ] == SET_STACK_POINTER)
	{
		return (int) m_Stack[ Offset ].StackPointer;
	}
#endif

	if (m_StackTypes[ Offset ] & SET_ENGINE_STRUCTURE)
		throw type_mismatch_exception( "GetStackInt type mismatch" );
	else if (m_StackTypes[ Offset ] & SET_DYNAMIC)
		return GetDynamicStackEntryInteger( Offset );
	else if (m_StackTypes[ Offset ] & SET_INTEGER)
		return m_Stack[ Offset ].Int;
	else
		throw type_mismatch_exception( "GetStackInt type mismatch" );
}


void
NWScriptStack::SetStackFloat(
	__in STACK_POINTER Displacement,
	__in float Float
	)
/*++

Routine Description:

	This routine assigns a value to a float relative to the current SP.

Arguments:

	Displacement - Supplies the displacement from the current SP.

	Float - Supplies the value to assign.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	size_t Offset;

	Offset = (GetCurrentSP( ) + Displacement) / STACK_ENTRY_SIZE;

	if (Offset >= m_Stack.size( ))
		throw invalid_stack_exception( "illegal stack reference" );

	if (m_StackTypes[ Offset ] & SET_ENGINE_STRUCTURE)
		throw type_mismatch_exception( "SetStackFloat type mismatch" );
	else if (m_StackTypes[ Offset ] & SET_DYNAMIC)
		SetDynamicStackEntry( Offset, Float );
	else if (m_StackTypes[ Offset ] & SET_FLOAT)
		m_Stack[ Offset ].Float = Float;
	else if (m_StackTypes[ Offset ] == SET_INVALID)
	{
		m_Stack[ Offset ].Float = Float;
		m_StackTypes[ Offset ]  = SET_FLOAT;
	}
	else
		throw type_mismatch_exception( "SetStackFloat type mismatch" );
}

float
NWScriptStack::GetStackFloat(
	__in STACK_POINTER Displacement
	) const
/*++

Routine Description:

	This routine returns a float relative to the top of the stack.

Arguments:

	Displacement - Supplies the displacement from the current SP.

Return Value:

	The routine returns the float at the given stack offset.  An
	std::exception is raised on failure (e.g. type mismatch).

Environment:

	User mode.

--*/
{
	size_t Offset;

	Offset = (GetCurrentSP( ) + Displacement) / STACK_ENTRY_SIZE;

	if (Offset >= m_Stack.size( ))
		throw invalid_stack_exception( "illegal stack reference" );

	if (m_StackTypes[ Offset ] & SET_ENGINE_STRUCTURE)
		throw type_mismatch_exception( "GetStackFloat type mismatch" );
	else if (m_StackTypes[ Offset ] & SET_DYNAMIC)
		return GetDynamicStackEntryFloat( Offset );
	else if (m_StackTypes[ Offset ] & SET_FLOAT)
		return m_Stack[ Offset ].Float;
	else
		throw type_mismatch_exception( "GetStackFloat type mismatch" );
}


void
NWScriptStack::SetStackString(
	__in STACK_POINTER Displacement,
	__in const char * String
	)
/*++

Routine Description:

	This routine assigns a value to an integer relative to the current SP.

Arguments:

	Displacement - Supplies the displacement from the current SP.

	String - Supplies the value to assign.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	size_t Offset;

	Offset = (GetCurrentSP( ) + Displacement) / STACK_ENTRY_SIZE;

	if (Offset >= m_Stack.size( ))
		throw invalid_stack_exception( "illegal stack reference" );

	if (m_StackTypes[ Offset ] & SET_ENGINE_STRUCTURE)
		throw type_mismatch_exception( "SetStackString type mismatch" );
	else if (m_StackTypes[ Offset ] & SET_DYNAMIC)
		SetDynamicStackEntry( Offset, String );
	else if (m_StackTypes[ Offset ] & SET_STRING)
	{
#if STACK_DEBUG
		if (m_Stack[ Offset ].String >= m_StackStrings.size( ))
			throw invalid_handle_exception( "invalid string handle" );
#endif

		m_StackStrings[ m_Stack[ Offset ].String ] = String;
	}
	else if (m_StackTypes[ Offset ] == SET_INVALID)
	{
		if (Offset != m_Stack.size( ) - 1)
			throw invalid_stack_exception( "strings may only be stored to uninitialized stack at the top of stack" );

		if (m_StackStrings.size( ) == ULONG_MAX)
			throw stack_overflow_exception( "out of string stack space" );

		m_StackStrings.push_back( String );

		m_Stack[ Offset ].String = (STRING_HANDLE) (m_StackStrings.size( ) - 1);
		m_StackTypes[ Offset ]   = SET_STRING;
	}
	else
		throw type_mismatch_exception( "SetStackString type mismatch" );
}

const std::string &
NWScriptStack::GetStackString(
	__in STACK_POINTER Displacement
	) const
/*++

Routine Description:

	This routine returns a string relative to the top of the stack.

Arguments:

	Displacement - Supplies the displacement from the current SP.

Return Value:

	The routine returns the string at the given stack offset.  An
	std::exception is raised on failure (e.g. type mismatch).

Environment:

	User mode.

--*/
{
	size_t Offset;

	Offset = (GetCurrentSP( ) + Displacement) / STACK_ENTRY_SIZE;

	if (Offset >= m_Stack.size( ))
		throw invalid_stack_exception( "illegal stack reference" );

	if (m_StackTypes[ Offset ] & SET_ENGINE_STRUCTURE)
		throw type_mismatch_exception( "GetStackString type mismatch" );
	else if (m_StackTypes[ Offset ] & SET_DYNAMIC)
		return GetDynamicStackEntryString( Offset );
	else if (m_StackTypes[ Offset ] & SET_STRING)
	{
#if STACK_DEBUG
		if (m_Stack[ Offset ].String >= m_StackStrings.size( ))
			throw invalid_handle_exception( "invalid string handle" );
#endif

		return m_StackStrings[ m_Stack[ Offset ].String ];
	}
	else
		throw type_mismatch_exception( "GetStackString type mismatch" );
}


void
NWScriptStack::SetStackObjectId(
	__in STACK_POINTER Displacement,
	__in NWN::OBJECTID ObjectId
	)
/*++

Routine Description:

	This routine assigns a value to an object id relative to the current SP.

Arguments:

	Displacement - Supplies the displacement from the current SP.

	ObjectId - Supplies the value to assign.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	size_t Offset;

	Offset = (GetCurrentSP( ) + Displacement) / STACK_ENTRY_SIZE;

	if (Offset >= m_Stack.size( ))
		throw invalid_stack_exception( "illegal stack reference" );

	if (m_StackTypes[ Offset ] & SET_ENGINE_STRUCTURE)
		throw type_mismatch_exception( "SetStackObjectId type mismatch" );
	else if (m_StackTypes[ Offset ] & SET_DYNAMIC)
		SetDynamicStackEntry( Offset, ObjectId );
	else if (m_StackTypes[ Offset ] & SET_OBJECTID)
		m_Stack[ Offset ].ObjectId = ObjectId;
	else if (m_StackTypes[ Offset ] == SET_INVALID)
	{
		m_Stack[ Offset ].ObjectId = ObjectId;
		m_StackTypes[ Offset ]     = SET_OBJECTID;
	}
	else
		throw type_mismatch_exception( "SetStackObjectId type mismatch" );
}

NWN::OBJECTID
NWScriptStack::GetStackObjectId(
	__in STACK_POINTER Displacement
	) const
/*++

Routine Description:

	This routine returns an object id relative to the top of the stack.

Arguments:

	Displacement - Supplies the displacement from the current SP.

Return Value:

	The routine returns the object id at the given stack offset.  An
	std::exception is raised on failure (e.g. type mismatch).

Environment:

	User mode.

--*/
{
	size_t Offset;

	Offset = (GetCurrentSP( ) + Displacement) / STACK_ENTRY_SIZE;

	if (Offset >= m_Stack.size( ))
		throw invalid_stack_exception( "illegal stack reference" );

	if (m_StackTypes[ Offset ] & SET_ENGINE_STRUCTURE)
		throw type_mismatch_exception( "GetStackObjectId type mismatch" );
	else if (m_StackTypes[ Offset ] & SET_DYNAMIC)
		return GetDynamicStackEntryObjectId( Offset );
	else if (m_StackTypes[ Offset ] & SET_OBJECTID)
		return m_Stack[ Offset ].ObjectId;
	else
		throw type_mismatch_exception( "GetStackObjectId type mismatch" );
}


void
NWScriptStack::SetStackVector(
	__in STACK_POINTER Displacement,
	__in const NWN::Vector3 & Vector
	)
/*++

Routine Description:

	This routine assigns a value to a vector relative to the current SP.

Arguments:

	Displacement - Supplies the displacement from the current SP.

	Vector - Supplies the value to assign.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	size_t Offset;

	Offset = (GetCurrentSP( ) + Displacement) / STACK_ENTRY_SIZE;

	if (Offset >= m_Stack.size( ))
		throw invalid_stack_exception( "illegal stack reference" );

	if (m_StackTypes[ Offset ] & SET_ENGINE_STRUCTURE)
		throw type_mismatch_exception( "SetStackVector type mismatch" );
	else if (m_StackTypes[ Offset ] & (SET_FLOAT | SET_VECTOR))
		m_Stack[ Offset ].Float = Vector.x;
	else if (m_StackTypes[ Offset ] == SET_INVALID)
	{
		m_Stack[ Offset ].Float = Vector.x;
		m_StackTypes[ Offset ]  = SET_FLOAT | SET_VECTOR;
	}
	else
		throw type_mismatch_exception( "SetStackVector type mismatch" );

	SetStackFloat( Displacement + 1 * STACK_ENTRY_SIZE, Vector.y );
	SetStackFloat( Displacement + 2 * STACK_ENTRY_SIZE, Vector.z );
}

NWN::Vector3
NWScriptStack::GetStackVector(
	__in STACK_POINTER Displacement
	) const
/*++

Routine Description:

	This routine returns a vector relative to the top of the stack.

Arguments:

	Displacement - Supplies the displacement from the current SP.

Return Value:

	The routine returns the vector at the given stack offset.  An
	std::exception is raised on failure (e.g. type mismatch).

Environment:

	User mode.

--*/
{
	size_t Offset;

	Offset = (GetCurrentSP( ) + Displacement) / STACK_ENTRY_SIZE;

	if (Offset >= m_Stack.size( ))
		throw invalid_stack_exception( "illegal stack reference" );

	if (m_StackTypes[ Offset ] & SET_ENGINE_STRUCTURE)
		throw type_mismatch_exception( "GetStackVector type mismatch" );
	else if (m_StackTypes[ Offset ] & (SET_FLOAT | SET_VECTOR))
	{
		NWN::Vector3 Vector;

		Vector.x = m_Stack[ Offset ].Float;
		Vector.y = GetStackFloat( Displacement + 1 * STACK_ENTRY_SIZE );
		Vector.z = GetStackFloat( Displacement + 2 * STACK_ENTRY_SIZE );

		return Vector;
	}
	else
		throw type_mismatch_exception( "GetStackVector type mismatch" );
}


void
NWScriptStack::SetStackEngineStructure(
	__in STACK_POINTER Displacement,
	__in EngineStructurePtr Struct
	)
/*++

Routine Description:

	This routine assigns a value to an engine structure relative to the current
	SP.

Arguments:

	Displacement - Supplies the displacement from the current SP.

	Struct - Supplies the value to assign.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	size_t Offset;

	Offset = (GetCurrentSP( ) + Displacement) / STACK_ENTRY_SIZE;

	if (Offset >= m_Stack.size( ))
		throw invalid_stack_exception( "illegal stack reference" );

	if ((m_StackTypes[ Offset ] & SET_ENGINE_STRUCTURE) && (m_StackTypes[ Offset ] != SET_STACK_POINTER))
	{
		if (m_Stack[ Offset ].String >= m_StackEngineStructures.size( ))
			throw invalid_handle_exception( "invalid engine structure handle" );

		if ((Struct.get( ) != NULL) &&
		    (m_StackEngineStructures[ m_Stack[ Offset ].EngineStruct ].get( ) != NULL))
		{
			if ((m_StackTypes[ Offset ] & ~SET_ENGINE_STRUCTURE) != Struct->GetEngineType( ))
				throw type_mismatch_exception( "SetEngineStructure engine type mismatch" );
		}

		m_StackEngineStructures[ m_Stack[ Offset ].EngineStruct ] = Struct;
	}
	else if (m_StackTypes[ Offset ] == SET_INVALID)
	{
		if (Offset != m_Stack.size( ) - 1)
			throw invalid_stack_exception( "engine structures may only be stored to uninitialized stack at the top of stack" );

		if (m_StackEngineStructures.size( ) == ULONG_MAX)
			throw stack_overflow_exception( "out of engine structure stack space" );

		m_StackEngineStructures.push_back( Struct );

		m_Stack[ Offset ].EngineStruct = (ENGINE_HANDLE) (m_StackEngineStructures.size( ) - 1);
		m_StackTypes[ Offset ]         = SET_ENGINE_STRUCTURE | Struct->GetEngineType( );
	}
	else
		throw type_mismatch_exception( "SetStackEngineStructure type mismatch" );
}

EngineStructurePtr
NWScriptStack::GetStackEngineStructure(
	__in STACK_POINTER Displacement,
	__in ENGINE_STRUCTURE_NUMBER EngineType
	) const
/*++

Routine Description:

	This routine returns an engine structure relative to the top of the stack.

Arguments:

	Displacement - Supplies the displacement from the current SP.

	EngineType - Supplies the type code of the engine structure expected to be
	             at the given stack offset.

Return Value:

	The routine returns the engine structure at the given stack offset.  An
	std::exception is raised on failure (e.g. type mismatch).

Environment:

	User mode.

--*/
{
	size_t Offset;

	Offset = (GetCurrentSP( ) + Displacement) / STACK_ENTRY_SIZE;

	if (Offset >= m_Stack.size( ))
		throw invalid_stack_exception( "illegal stack reference" );

	if ((m_StackTypes[ Offset ] & SET_ENGINE_STRUCTURE) && (m_StackTypes[ Offset ] != SET_STACK_POINTER))
	{
		if ((m_StackTypes[ Offset ] & ~SET_ENGINE_STRUCTURE) != EngineType)
			throw type_mismatch_exception( "GetStackEngineStructure engine type mismatch" );

		if (m_Stack[ Offset ].EngineStruct >= m_StackEngineStructures.size( ))
			throw invalid_handle_exception( "invalid engine structure handle" );

		return m_StackEngineStructures[ m_Stack[ Offset ].EngineStruct ];
	}
	else
		throw type_mismatch_exception( "GetStackEngineStructure type mismatch" );
}


void
NWScriptStack::CopyDownSP(
	__in STACK_POINTER Destination,
	__in STACK_POINTER BytesToCopy,
	__in bool UseBP /* = false */
	)
/*++

Routine Description:

	This routine copies stack cells to a location further downward in the VM
	stack.

Arguments:

	Destination - Supplies the destination of the copy.

	BytesToCopy - Supplies the count of bytes to copy.

	UseBP - Supplies a Boolean value that indicates whether the source of the
	        copy is relative to the current SP (false) or BP (true) value.

Return Value:

	None.  An std::exception is raised on failure.

Environment:

	User mode.

--*/
{
	size_t CellsToCopy;
	size_t SrcOffset;

#if STACK_DEBUG
	//
	// Validate that our size and pointers are legal.
	//

	if ((Destination & (STACK_ENTRY_SIZE - 1)) ||
	    (BytesToCopy & (STACK_ENTRY_SIZE - 1)))
	{
		throw invalid_stack_exception( "misaligned stack reference in CopyDownSP" );
	}
#endif

	if (UseBP)
		Destination += GetCurrentBP( );
	else
	{
		Destination += GetCurrentSP( );

		CheckGuardZone( Destination );
	}

	Destination /= STACK_ENTRY_SIZE;
	CellsToCopy  = BytesToCopy / STACK_ENTRY_SIZE;

	if (((size_t) Destination >= m_Stack.size( ))                    ||
	    ((size_t) Destination + CellsToCopy < (size_t) Destination)  ||
	    ((size_t) (Destination + CellsToCopy) > m_Stack.size( )))
	{
		throw invalid_stack_exception( "destination exceeds stack bounds in CopyDownSP" );
	}

	if (m_Stack.size( ) < CellsToCopy)
		throw invalid_stack_exception( "source exceeds stack bounds in CopyDownSP" );

	SrcOffset = m_Stack.size( ) - CellsToCopy;

	if (SrcOffset == (size_t) Destination)
		return;

	//
	// Now perform the copy, one cell at a time.  The destination cells are
	// required to have the same type as the source cells as this is a bulk
	// assignment operation.
	//

	for (size_t i = 0; i < CellsToCopy; i += 1)
	{
		STACK_TYPE_CODE DestType = m_StackTypes[ Destination + i ];
		STACK_TYPE_CODE SrcType  = m_StackTypes[ SrcOffset + i ];
		bool            StructTypeSame;

		if ((!(SrcType & SET_ENGINE_STRUCTURE)) && (!(DestType & SET_ENGINE_STRUCTURE)) &&
		    ((SrcType & ~(SET_VECTOR | SET_STRUCTURE)) == ((DestType & ~(SET_VECTOR | SET_STRUCTURE)))))
		{
			StructTypeSame = true;
		}
		else
		{
			StructTypeSame = false;
		}

		if ((!(SrcType & SET_ENGINE_STRUCTURE)) && (SrcType & SET_DYNAMIC))
		{
			//
			// We have here a dynamically typed stack entry as the source.
			// Convert it to the appropriate destination type and assign it.
			//

			switch (DestType)
			{

			case SET_INTEGER:
				m_Stack[ Destination + i ].Int = GetDynamicStackEntryInteger( SrcOffset + i );
				break;

			case SET_FLOAT:
				m_Stack[ Destination + i ].Float = GetDynamicStackEntryFloat( SrcOffset + i );
				break;

			case SET_STRING:
			case SET_DYNAMIC:
				if (m_Stack[ SrcOffset + i ].String == m_Stack[ Destination + i ].String)
					break;

#if STACK_DEBUG
				if (m_Stack[ Destination + i ].String >= m_StackStrings.size( ))
					throw invalid_handle_exception( "invalid destination string handle in CopyDownSP" );
#endif

				m_StackStrings[ m_Stack[ Destination + i ].String ] = GetDynamicStackEntryString( SrcOffset + i );
				break;

			case SET_OBJECTID:
				m_Stack[ Destination + i ].ObjectId = GetDynamicStackEntryObjectId( SrcOffset + i );
				break;

#if STACK_SAVEBP_CONVERT_TO_INTEGER
			case SET_STACK_POINTER:
				m_Stack[ Destination + i ].StackPointer = (STACK_POINTER) GetDynamicStackEntryInteger( SrcOffset + i );
				break;
#endif

			default:
				throw type_mismatch_exception( "attempted to copy illegal type from dynamic typed stack entry" );

			}
		}
		else if ((!(DestType & SET_ENGINE_STRUCTURE)) && (DestType & SET_DYNAMIC))
		{
			//
			// We have here a dynamically typed stack entry as the destination.
			// Convert it to the appropriate destination type and assign it.
			//

			switch (SrcType)
			{

			case SET_INTEGER:
				SetDynamicStackEntry( Destination + i, m_Stack[ SrcOffset + i ].Int );
				break;

			case SET_FLOAT:
				SetDynamicStackEntry( Destination + i, m_Stack[ SrcOffset + i ].Float );
				break;

			case SET_STRING:
			case SET_DYNAMIC:
				if (m_Stack[ SrcOffset + i ].String == m_Stack[ Destination + i ].String)
					break;

#if STACK_DEBUG
				if (m_Stack[ SrcOffset + i ].String >= m_StackStrings.size( ))
					throw invalid_handle_exception( "invalid destination string handle in CopyDownSP" );
#endif

				SetDynamicStackEntry( Destination + i, m_StackStrings[ m_Stack[ SrcOffset + i ].String ].c_str( ) );
				break;

			case SET_OBJECTID:
				SetDynamicStackEntry( Destination + i, m_Stack[ SrcOffset + i ].ObjectId );
				break;

#if STACK_SAVEBP_CONVERT_TO_INTEGER
			case SET_STACK_POINTER:
				SetDynamicStackEntry( Destination + i, (int) m_Stack[ SrcOffset + i ].StackPointer );
				break;
#endif

			default:
				throw type_mismatch_exception( "attempted to copy illegal type from dynamic typed stack entry" );

			}
		}
		else if ((SrcType == DestType) || (StructTypeSame))
		{
			if ((!(SrcType & SET_ENGINE_STRUCTURE)) && (SrcType & SET_STRING))
			{
				STRING_HANDLE StringSrc;
				STRING_HANDLE StringDst;

				//
				// We are copying a string, change the data backed by our
				// handles here instead of exchanging the handle values.
				//

				StringSrc = m_Stack[ SrcOffset + i ].String;
				StringDst = m_Stack[ Destination + i ].String;

#if STACK_DEBUG
				if (StringSrc >= m_StackStrings.size( ))
					throw invalid_handle_exception( "invalid source string handle in CopyDownSP" );
				if (StringDst >= m_StackStrings.size( ))
					throw invalid_handle_exception( "invalid destination string handle in CopyDownSP" );
#endif

				if (StringSrc != StringDst)
					m_StackStrings[ StringDst ] = m_StackStrings[ StringSrc ];
			}
			else if ((SrcType & SET_ENGINE_STRUCTURE) &&
			         (SrcType != SET_STACK_POINTER))
			{
				ENGINE_HANDLE EngineSrc;
				ENGINE_HANDLE EngineDst;

				//
				// We are copying an engine structure, change the data backed
				// by our handles here instead of exchanging the handle values.
				//

				EngineSrc = m_Stack[ SrcOffset + i ].EngineStruct;
				EngineDst = m_Stack[ Destination + i ].EngineStruct;

#if STACK_DEBUG
				if (EngineSrc >= m_StackEngineStructures.size( ))
					throw invalid_handle_exception( "invalid source engine handle in CopyDownSP" );
				if (EngineDst >= m_StackEngineStructures.size( ))
					throw invalid_handle_exception( "invalid destination engine handle in CopyDownSP" );
#endif

				if (EngineSrc != EngineDst)
					m_StackEngineStructures[ EngineDst ] = m_StackEngineStructures[ EngineSrc ];
			}
			else
			{
				//
				// The source and destination are of the same type and are not
				// of a handle type.  We can directly copy the values.
				//

				m_Stack[ Destination + i ] = m_Stack[ SrcOffset + i ];
			}
		}
#if STACK_SAVEBP_CONVERT_TO_INTEGER
		else if ((SrcType == SET_STACK_POINTER) && (DestType == SET_INTEGER))
			m_Stack[ Destination + i ] = m_Stack[ SrcOffset + i ];
		else if ((SrcType == SET_INTEGER) && (DestType == SET_STACK_POINTER))
			m_Stack[ Destination + i ] = m_Stack[ SrcOffset + i ];
#endif
		else
		{
			//
			// The types really do not match.  The program is ill-formed as the
			// assignments should always operate on like-typed stack entries.
			//

			throw type_mismatch_exception( "type mismatch in CopyDownSP" );
		}
	}
}

void
NWScriptStack::CopyTopSP(
	__in STACK_POINTER Source,
	__in STACK_POINTER BytesToCopy,
	__in bool UseBP /* = false */
	)
/*++

Routine Description:

	This routine creates new copies of stack cells located at a displacement
	from the current top of stack.

Arguments:

	Source - Supplies the source of the copy.

	BytesToCopy - Supplies the count of bytes to copy.

	UseBP - Supplies a Boolean value that indicates whether the source of the
	        copy is relative to the current SP (false) or BP (true) value.

Return Value:

	None.  An std::exception is raised on failure.

Environment:

	User mode.

--*/
{
	size_t CellsToCopy;
	size_t SrcOffset;
	size_t DstOffset;

#if STACK_DEBUG
	//
	// Validate that our size and pointers are legal.
	//

	if ((Source & (STACK_ENTRY_SIZE - 1)) ||
	    (BytesToCopy & (STACK_ENTRY_SIZE - 1)))
	{
		throw invalid_stack_exception( "misaligned stack reference in CopyTopSP" );
	}
#endif

	if (UseBP)
		Source += GetCurrentBP( );
	else
	{
		Source += GetCurrentSP( );

		CheckGuardZone( Source );
	}

	Source      /= STACK_ENTRY_SIZE;
	CellsToCopy  = BytesToCopy / STACK_ENTRY_SIZE;

	if (((size_t) Source >= m_Stack.size( ))                    ||
	    ((size_t) Source + CellsToCopy < (size_t) Source)       ||
	    ((size_t) (Source + CellsToCopy) > m_Stack.size( )))
	{
		throw invalid_stack_exception( "source exceeds stack bounds in CopyTopSP" );
	}

	if ((m_Stack.size( ) + CellsToCopy < m_Stack.size( )) ||
	    (m_Stack.size( ) + CellsToCopy >= STACK_MAXIMUM_SIZE))
	{
		throw invalid_stack_exception( "source exceeds stack bounds in CopyTopSP" );
	}

	SrcOffset = (size_t) Source;
	DstOffset = m_Stack.size( );

	//
	// Expand the stack to make room for the copied data.
	//

	GrowStack( CellsToCopy );

	//
	// Now perform the copy, one cell at a time.  The destination cells are
	// pushed onto the stack in turn.
	//

	for (size_t i = 0; i < CellsToCopy; i += 1)
	{
		STACK_TYPE_CODE SrcType = m_StackTypes[ SrcOffset + i ];

		if ((!(SrcType & SET_ENGINE_STRUCTURE)) &&
		    ((SrcType & SET_STRING) || (SrcType & SET_DYNAMIC)))
		{
			STRING_HANDLE StringSrc;

			//
			// We are copying a string, change the data backed by our handles
			// here instead of duplicating handle values.
			//
			// N.B.  Dynamic typed stack entries are implemented as strings and
			//       may be identically handled here.  We must copy the dynamic
			//       type flag here as we don't know what the fixed type is yet
			//       since CopyTopSP is an untyped copy.
			//

			StringSrc = m_Stack[ SrcOffset + i ].String;

#if STACK_DEBUG
			if (StringSrc >= m_StackStrings.size( ))
				throw invalid_handle_exception( "invalid source string handle in CopyTopSP" );
#endif

			if (m_StackStrings.size( ) == ULONG_MAX)
				throw stack_overflow_exception( "out of string stack space" );

			m_StackStrings.push_back( m_StackStrings[ StringSrc ] );

			m_Stack[ DstOffset + i ].String = (STRING_HANDLE) (m_StackStrings.size( ) - 1);
			m_StackTypes[ DstOffset + i ]   = SrcType;
		}
		else if ((SrcType & SET_ENGINE_STRUCTURE) &&
		         (SrcType != SET_STACK_POINTER))
		{
			ENGINE_HANDLE EngineSrc;

			//
			// We are copying an engine structure, change the data backed by
			// our handles here instead of duplicating handle values.
			//

			EngineSrc = m_Stack[ SrcOffset + i ].EngineStruct;

#if STACK_DEBUG
			if (EngineSrc >= m_StackEngineStructures.size( ))
				throw invalid_handle_exception( "invalid source engine handle in CopyTopSP" );
#endif

			if (m_StackEngineStructures.size( ) == ULONG_MAX)
				throw stack_overflow_exception( "out of engine structure stack space" );

			m_StackEngineStructures.push_back( m_StackEngineStructures[ EngineSrc ] );

			m_Stack[ DstOffset + i ].EngineStruct = (ENGINE_HANDLE) (m_StackEngineStructures.size( ) - 1 );
			m_StackTypes[ DstOffset + i ]         = SrcType;
		}
		else
		{
			//
			// This type has no special handling required, just copy it
			// directly.
			//

			m_Stack[ DstOffset + i ].Raw  = m_Stack[ SrcOffset + i ].Raw;
			m_StackTypes[ DstOffset + i ] = SrcType;
		}
	}
}


NWScriptStack
NWScriptStack::SaveStack(
	__in STACK_POINTER BPSaveBytes,
	__in STACK_POINTER SPSaveBytes,
	__in STACK_POINTER SPSaveOffset /* = 0 */
	)
/*++

Routine Description:

	This routine saves a portion of the current stack's contents into a new
	stack object that is returned to the caller.

	Note that the BP restore and program counter restore stacks are not saved.

Arguments:

	BPSaveBytes - Supplies the count of bytes to save relative to the current
	              BP.

	SPSaveBytes - Supplies the count of bytes to save relative to the current
	              SP.

	SPSaveOffset - Supplies the offset relative to the current SP that the SP
	               save bytes should be copied from.

Return Value:

	The routine returns a new stack object that contains only the saved state.
	On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	NWScriptStack NewStack( m_InvalidObjId );
	size_t        CellsToCopy;
	size_t        SrcOffset;
	STACK_POINTER CurSP;
	STACK_POINTER CurBP;

#if STACK_DEBUG
	//
	// Validate stack pointer arguments.
	//

	if ((BPSaveBytes & (STACK_ENTRY_SIZE - 1)) ||
	    (SPSaveBytes & (STACK_ENTRY_SIZE - 1)) ||
	    (SPSaveOffset & (STACK_ENTRY_SIZE - 1)))
	{
		throw invalid_stack_exception( "misaligned stack reference in SaveStack" );
	}
#endif

	if ((BPSaveBytes < 0) || (SPSaveBytes < 0))
		throw invalid_stack_exception( "negative save count in SaveStack" );

	CurSP = GetCurrentSP( );
	CurBP = GetCurrentBP( );

	if (CurSP + SPSaveOffset < 0)
		throw invalid_stack_exception( "stack save offset exceeds stack bounds in SaveStack" );

	if ((BPSaveBytes > CurBP) ||
	    (SPSaveBytes > CurSP + SPSaveOffset))
	{
		throw invalid_stack_exception( "stack save range exceeds stack bounds in SaveStack" );
	}

	//
	// Copy stack cells relative to BP first.
	//

	SrcOffset   = (CurBP - BPSaveBytes) / STACK_ENTRY_SIZE;
	CellsToCopy = BPSaveBytes / STACK_ENTRY_SIZE;

	AppendStackContentsToStack(
		NewStack,
		SrcOffset,
		CellsToCopy);

	NewStack.SaveBP( );

	//
	// Now copy the SP-relative cells.
	//

	SrcOffset   = ((CurSP + SPSaveOffset) - SPSaveBytes) / STACK_ENTRY_SIZE;
	CellsToCopy = SPSaveBytes / STACK_ENTRY_SIZE;

	AppendStackContentsToStack(
		NewStack,
		SrcOffset,
		CellsToCopy);

	//
	// All done.
	//

	return NewStack;
}

void
NWScriptStack::SaveStack(
	__in INWScriptStack * Stack,
	__in STACK_POINTER BPSaveBytes,
	__in STACK_POINTER SPSaveBytes,
	__in STACK_POINTER SPSaveOffset /* = 0 */
	)
/*++

Routine Description:

	This routine saves a portion of the current stack's contents into a new
	stack object that is provided as an INWScriptStack interface by the
	caller.

	Note that the BP restore and program counter restore stacks are not saved.

	Note that type information for saved BP values and dynamic parameters is
	not saved when saving to an INWScriptStack (versus a NWScriptStack).

Arguments:

	Stack - Supplies the stack object that receives the copied stack contents.

	BPSaveBytes - Supplies the count of bytes to save relative to the current
	              BP.

	SPSaveBytes - Supplies the count of bytes to save relative to the current
	              SP.

	SPSaveOffset - Supplies the offset relative to the current SP that the SP
	               save bytes should be copied from.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	size_t        CellsToCopy;
	size_t        SrcOffset;
	STACK_POINTER CurSP;
	STACK_POINTER CurBP;

#if STACK_DEBUG
	//
	// Validate stack pointer arguments.
	//

	if ((BPSaveBytes & (STACK_ENTRY_SIZE - 1)) ||
	    (SPSaveBytes & (STACK_ENTRY_SIZE - 1)) ||
	    (SPSaveOffset & (STACK_ENTRY_SIZE - 1)))
	{
		throw invalid_stack_exception( "misaligned stack reference in SaveStack" );
	}
#endif

	if ((BPSaveBytes < 0) || (SPSaveBytes < 0))
		throw invalid_stack_exception( "negative save count in SaveStack" );

	CurSP = GetCurrentSP( );
	CurBP = GetCurrentBP( );

	if (CurSP + SPSaveOffset < 0)
		throw invalid_stack_exception( "stack save offset exceeds stack bounds in SaveStack" );

	if ((BPSaveBytes > CurBP) ||
	    (SPSaveBytes > CurSP + SPSaveOffset))
	{
		throw invalid_stack_exception( "stack save range exceeds stack bounds in SaveStack" );
	}

	//
	// Copy stack cells relative to BP first.
	//

	SrcOffset   = (CurBP - BPSaveBytes) / STACK_ENTRY_SIZE;
	CellsToCopy = BPSaveBytes / STACK_ENTRY_SIZE;

	AppendStackContentsToStack(
		Stack,
		SrcOffset,
		CellsToCopy);

	Stack->StackPushInt( (int) CurBP );

	//
	// Now copy the SP-relative cells.
	//

	SrcOffset   = ((CurSP + SPSaveOffset) - SPSaveBytes) / STACK_ENTRY_SIZE;
	CellsToCopy = SPSaveBytes / STACK_ENTRY_SIZE;

	AppendStackContentsToStack(
		Stack,
		SrcOffset,
		CellsToCopy);

	//
	// All done.
	//

	return;
}


void
NWScriptStack::DestructElements(
	__in STACK_POINTER BytesToRemove,
	__in STACK_POINTER ExcludePointer,
	__in STACK_POINTER BytesToExclude
	)
/*++

Routine Description:

	This routine deallocates a section of the stack, though potentially saving
	a 'hole' of the stack without logically deleting it.  Generally, this
	operation is used to retrieve a single member variable out of a structure
	that has been pushed on the stack (e.g. as a return value).

Arguments:

	BytesToRemove - Supplies the count of bytes to deallocate.  The region to
	                delete is the current SP less BytesToRemove through the
	                current SP.

	ExcludePointer - Supplies a pointer to the region to exclude.

	BytesToExclude - Supplies the count of bytes starting from the exclude

Return Value:

	The routine returns a new stack object that contains only the saved state.
	On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	size_t        SrcOffset;
	size_t        CellsToCopy;

	//
	// If we have no exclude region then just delete the stack region and
	// return.
	//

	if (BytesToExclude == 0)
	{
		AddSP( -BytesToRemove );
		return;
	}

	//
	// We have a portion to save.  Implement the operation as two parts:
	//
	// 1)  Copy the section to save to a temporary stack, and
	// 2)  Invoke AddSP to deallocate the entire deallocation region, and
	// 3)  Restore the temporary stack back to the current stack.
	//

#if STACK_DEBUG
	if ((BytesToRemove & (STACK_ENTRY_SIZE - 1))  ||
	    (ExcludePointer & (STACK_ENTRY_SIZE - 1)) ||
	    (BytesToExclude & (STACK_ENTRY_SIZE - 1)))
	{
		throw invalid_stack_exception( "misaligned stack reference in DestructElements" );
	}
#endif

	//
	// First save the exclude region to the temporary stack.  We use a
	// temporary stack so as to simplify the handling of handle references in
	// the saved region.  (The handle logic assumes that handle references are
	// always deleted in inverse creation order.)
	//

	NWScriptStack SaveStack( m_InvalidObjId );

	SrcOffset   = ((GetCurrentSP( ) - BytesToRemove) + ExcludePointer) / STACK_ENTRY_SIZE;
	CellsToCopy = BytesToExclude / STACK_ENTRY_SIZE;

	AppendStackContentsToStack( SaveStack, SrcOffset, CellsToCopy );

	//
	// Now deallocate the region that we are dumping.
	//

	AddSP( -BytesToRemove );

	//
	// Finally, append the temporary stack contents back to ourselves.
	//

	SaveStack.AppendStackContentsToStack(
		*this,
		0,
		SaveStack.m_Stack.size( ) );
}

int
NWScriptStack::IncrementStackInt(
	__in STACK_POINTER AbsoluteAddress
	)
/*++

Routine Description:

	This routine increments an integer given an absolute stack address (that is
	an address relative to the bottom of the stack).

Arguments:

	AbsoluteAddress - Supplies the stack address of the integer to adjust.

Return Value:

	The routine returns the adjusted value.  Raises an std::exception on
	failure.

Environment:

	User mode.

--*/
{
	size_t Offset;

	Offset = AbsoluteAddress / STACK_ENTRY_SIZE;

	if (Offset >= m_Stack.size( ))
		throw invalid_stack_exception( "illegal stack reference" );

	if (m_StackTypes[ Offset ] == SET_DYNAMIC)
	{
		int Value = GetDynamicStackEntryInteger( Offset ) + 1;

		SetDynamicStackEntry( Offset, Value );

		return Value;
	}
#if STACK_SAVEBP_CONVERT_TO_INTEGER
	else if (m_StackTypes[ Offset ] == SET_STACK_POINTER)
		return (int) (m_Stack[ Offset ].StackPointer += 1);
#endif
	else if ((m_StackTypes[ Offset ] & SET_INTEGER) &&
	         (!(m_StackTypes[ Offset ] & SET_ENGINE_STRUCTURE)))
		return m_Stack[ Offset ].Int += 1;
	else
		throw type_mismatch_exception( "IncrementStackInt type mismatch" );
}

int
NWScriptStack::DecrementStackInt(
	__in STACK_POINTER AbsoluteAddress
	)
/*++

Routine Description:

	This routine decrements an integer given an absolute stack address (that is
	an address relative to the bottom of the stack).

Arguments:

	AbsoluteAddress - Supplies the stack address of the integer to adjust.

Return Value:

	The routine returns the adjusted value.  Raises an std::exception on
	failure.

Environment:

	User mode.

--*/
{
	size_t Offset;

	Offset = AbsoluteAddress / STACK_ENTRY_SIZE;

	if (Offset >= m_Stack.size( ))
		throw invalid_stack_exception( "illegal stack reference" );

	if (m_StackTypes[ Offset ] == SET_DYNAMIC)
	{
		int Value = GetDynamicStackEntryInteger( Offset ) - 1;

		SetDynamicStackEntry( Offset, Value );

		return Value;
	}
#if STACK_SAVEBP_CONVERT_TO_INTEGER
	else if (m_StackTypes[ Offset ] == SET_STACK_POINTER)
		return (int) (m_Stack[ Offset ].StackPointer -= 1);
#endif
	else if ((m_StackTypes[ Offset ] & SET_INTEGER) &&
	         (!(m_StackTypes[ Offset ] & SET_ENGINE_STRUCTURE)))
		return m_Stack[ Offset ].Int -= 1;
	else
		throw type_mismatch_exception( "DecrementStackInt type mismatch" );
}

NWScriptStack::STACK_POINTER
NWScriptStack::GetStackIntegerSize(
	) const
/*++

Routine Description:

	This routine returns the size of an integer as pushed onto the stack.

Arguments:

	None.

Return Value:

	The routine returns the count of stack bytes that an integer would consume
	on the VM stack.

Environment:

	User mode.

--*/
{
	return STACK_ENTRY_SIZE;
}

bool
NWScriptStack::PeekStack(
	__in STACK_POINTER AbsoluteAddress,
	__out ULONG & RawStack,
	__out UCHAR & RawType
	) const
/*++

Routine Description:

	This debugging use only routine inspects the contents of the current VM
	stack.  It reads a single stack cell and returns it to the caller.

Arguments:

	AbsoluteAddress - Supplies the absolute stack offset to read.

	RawStack - Receives the raw stack contents (which may be a handle).

	RawType - Receives the raw stack type.

Return Value:

	The routine returns true if the read was completed, else false if it could
	not be.

Environment:

	User mode, invoked from the script VM debugger only.

--*/
{
	size_t Offset;

	if ((AbsoluteAddress & STACK_ENTRY_SIZE - 1))
		return false;

	Offset = AbsoluteAddress / STACK_ENTRY_SIZE;

	if (Offset >= m_Stack.size( ))
		return false;

	RawStack = m_Stack[ Offset ].Raw;
	RawType  = (UCHAR) m_StackTypes[ Offset ];

	return true;
}

bool
NWScriptStack::DebugIsEngineStructureType(
	__in UCHAR RawType
	) const
/*++

Routine Description:

	This debugging use only routine returns whether a type indicates an engine
	or other internal structure (or saved stack pointer) or not.

	These types are not legal to pass to GetStackType, so the debugger must be
	able to avoid doing so.

Arguments:

	RawType - Supplies the raw stack type.

Return Value:

	The routine returns true if the read was completed, else false if it could
	not be.

Environment:

	User mode, invoked from the script VM debugger only.

--*/
{
	//
	// N.B.  Saved stack pointers are returned here as engine structures too!
	//

	if (RawType & SET_ENGINE_STRUCTURE)
		return true;
	else
		return false;
}

bool
NWScriptStack::IsParameterUnderrunRestoreBP(
	) const
/*++

Routine Description:

	This routine supports the work around for GUI scripts that have global
	variables and get called with an incorrect argument count.  It checks
	whether the restored BP pointer is legal.

Arguments:

	None.

Return Value:

	The routine returns true if the current stack entry is not a legal saved
	BP value.  The caller would only call this routine during RESTOREBP to see
	if the we passed too many parameters to a script as a part of the work
	around.

Environment:

	User mode, invoked from RESTOREBP handling only.

--*/
{
	UCHAR Type;

	if (m_Stack.empty( ))
		return false;

	Type = m_StackTypes[ m_Stack.size( ) - 1 ];

	if (Type == SET_STACK_POINTER)
		return false;
	else
		return true;
}

NWScriptStack::BASE_STACK_TYPE
NWScriptStack::GetStackType(
	__in STACK_POINTER AbsoluteAddress
	) const
/*++

Routine Description:

	This routine determines the type of an entry at the given stack address.

	Only basic types are supported, and the stack is not modified.

Arguments:

	AbsoluteAddress - Supplies the absolute stack offset to read.

Return Value:

	The routine returns the basic type classification of the stack entry in
	question.  On failure, an std::exception is raised.

Environment:

	User mode, invoked from the script VM debugger only unless querying the top
	of the stack.

--*/
{
	size_t Offset;

	Offset = AbsoluteAddress / STACK_ENTRY_SIZE;

	if (Offset >= m_Stack.size( ))
		throw invalid_stack_exception( "illegal stack reference" );

	if ((m_StackTypes[ Offset ] & SET_ENGINE_STRUCTURE) == 0)
	{
		if (m_StackTypes[ Offset ] & SET_INTEGER)
			return BST_INT;
		else if (m_StackTypes[ Offset ] & SET_FLOAT)
			return BST_FLOAT;
		else if (m_StackTypes[ Offset ] & SET_OBJECTID)
			return BST_OBJECTID;
		else if (m_StackTypes[ Offset ] & (SET_STRING | SET_DYNAMIC))
			return BST_STRING;
		else
			throw type_mismatch_exception( "illegal base type on stack" );
	}
	else if ((m_StackTypes[ Offset ] != SET_STACK_POINTER) && (m_StackTypes[ Offset ] & SET_ENGINE_STRUCTURE))
		return (BASE_STACK_TYPE) ((unsigned long) BST_ENGINE_0 + (m_StackTypes[ Offset ] & (~SET_ENGINE_STRUCTURE)));
	else if (m_StackTypes[ Offset ] == SET_STACK_POINTER)
	{
#if STACK_SAVEBP_CONVERT_TO_INTEGER
		return BST_INT;
#else
		return BST_STACK_POINTER;
#endif
	}
	else
		throw type_mismatch_exception( "illegal base type on stack" );
}

NWScriptStack::BASE_STACK_TYPE
NWScriptStack::GetTopOfStackType(
	) const
/*++

Routine Description:

	This routine determines the type of the entry residing at the top of the
	stack.

	Only basic types are supported, and the stack is not modified.

Arguments:

	None.

Return Value:

	The routine returns the basic type classification of the stack entry in
	question.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	return GetStackType( GetCurrentSP( ) - STACK_ENTRY_SIZE );
}

void
NWScriptStack::ResetStack(
	)
/*++

Routine Description:

	This routine resets the stack to a clean state.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	m_ReturnStack.clear( );
	m_Stack.clear( );
	m_StackTypes.clear( );
	m_StackStrings.clear( );
	m_StackEngineStructures.clear( );
	m_GuardZoneStack.clear( );

	m_BP = 0;
}


void
NWScriptStack::EstablishGuardZone(
	)
/*++

Routine Description:

	This routine establishes a "guard zone" on the stack.  No references that
	are relative to SP are allowed to pass ahead of the guard zone.

Arguments:

	None.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	m_GuardZoneStack.push_back( GetCurrentSP( ) );
}

void
NWScriptStack::DeestablishGuardZone(
	)
/*++

Routine Description:

	This routine removes the most recently created guard zone from the stack.

Arguments:

	None.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	if (m_GuardZoneStack.empty( ))
		throw stack_underflow_exception( "cannot remove non-existant guard zone" );

	m_GuardZoneStack.pop_back( );
}

void
NWScriptStack::CheckGuardZone(
	__in STACK_POINTER AbsoluteAddress
	)
/*++

Routine Description:

	This routine is called to check an absolute address against the currently
	active guard zone (if any).  The caller may only invoke this routine for an
	operand formed relative to SP (not BP).

	If the stack address has passed ahead of the guard zone, then a guard zone
	violation exception is raised.

Arguments:

	AbsoluteAddress - Supplies the absolute address into the stack to verify
	                  against the currently active guard zone (if any).

Return Value:

	None.  The routine raises a guard_zone_exception should the guard zone be
	violated.

Environment:

	User mode.

--*/
{
	if (m_GuardZoneStack.empty( ))
		return;

	if (m_GuardZoneStack.back( ) >= AbsoluteAddress)
		throw guard_zone_exception( "illegal stack reference beyond guard zone" );
}

void
NWScriptStack::SetInvalidObjId(
	__in NWN::OBJECTID InvalidObjId
	)
/*++

Routine Description:

	This routine sets the default invalid object id that is used when a
	dynamic parameter conversion to an object type fails.

Arguments:

	InvalidObjId - Supplies the invalid object id value.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	m_InvalidObjId = InvalidObjId;
}

NWN::OBJECTID
NWScriptStack::GetInvalidObjId(
	) const
/*++

Routine Description:

	This routine returns the default invalid object id that is used when a
	dynamic parameter conversion to an object type fails.

Arguments:

	None.

Return Value:

	The routine returns the default invalid object id value.

Environment:

	User mode.

--*/
{
	return m_InvalidObjId;
}


void
NWScriptStack::StackPushRaw(
	__in STACK_ENTRY StackEntry,
	__in STACK_TYPE_CODE StackEntryType
	)
/*++

Routine Description:

	This routine pushes an entry onto the stack.

Arguments:

	StackEntry - Supplies the raw stack entry.

	StackEntryType - Supplies the stack entry type.

Return Value:

	None.  An std::exception is raised on failure.

Environment:

	User mode.

--*/
{
	if (m_Stack.size( ) > STACK_MAXIMUM_SIZE)
		throw stack_overflow_exception( "maximum stack size exceeded" );

	m_Stack.push_back( StackEntry );

	try
	{
		m_StackTypes.push_back( StackEntryType );
	}
	catch (...)
	{
		m_Stack.pop_back( );
		throw;
	}
}

NWScriptStack::STACK_ENTRY
NWScriptStack::StackPopRaw(
	__in STACK_TYPE_CODE StackEntryType
	)
/*++

Routine Description:

	This routine returns the entry at the top of the stack.  Note that if a
	handle was referenced, the handle's associated stack is unmodified.

Arguments:

	StackEntryType - Supplies the stack entry type that is expected.

Return Value:

	None.  An std::exception is raised on failure.

Environment:

	User mode.

--*/
{
	STACK_ENTRY Entry;
	UCHAR       Type;

	if (m_Stack.empty( ))
		throw stack_underflow_exception( "attempted to pop entry from empty stack" );

	CheckGuardZone( GetCurrentSP( ) - STACK_ENTRY_SIZE );

	Type = m_StackTypes.back( );

#if STACK_SAVEBP_CONVERT_TO_INTEGER
	//
	// If we are to allow conversion of stack pointers to integers, then force
	// a type match in such a case.
	//

	if (Type != StackEntryType)
	{
		if (StackEntryType == SET_STACK_POINTER)
		{
			if ((Type & SET_INTEGER | SET_DYNAMIC) &&
				(!(Type & SET_ENGINE_STRUCTURE)))
			{
				//
				// We are requesting a stack pointer but have a dynamic or
				// integer type on the stack.  Change the request to be for an
				// integer.
				//

				StackEntryType = SET_INTEGER;
			}
		}
		else if ((StackEntryType & (SET_INTEGER | SET_DYNAMIC)) &&
		         (!(StackEntryType & SET_ENGINE_STRUCTURE)))
		{
			if (Type == SET_STACK_POINTER)
			{
				//
				// We are requesting an integer or dynamic type but have a
				// stack pointer on the stack.  Change the stack to be an
				// integer.
				//

				Type = SET_INTEGER;
			}
		}
	}
#endif

	//
	// Now compare the member type.  (We allow a vector to be treated as a
	// float, and a structure member as its base type, for instance.)
	//

	if ((Type & ~(SET_VECTOR | SET_STRUCTURE)) != (StackEntryType & ~(SET_VECTOR | SET_STRUCTURE)))
	{
		//
		// If we were dynamic typed, then handle this here.
		//

		if ((!(Type & SET_ENGINE_STRUCTURE)) && (!(StackEntryType & SET_ENGINE_STRUCTURE)))
		{
			if (Type & SET_DYNAMIC)
			{
				if (StackEntryType & SET_INTEGER)
					Entry.Int = GetDynamicStackEntryInteger( m_Stack.size( ) - 1 );
				else if (StackEntryType & SET_FLOAT)
					Entry.Float = GetDynamicStackEntryFloat( m_Stack.size( ) - 1 );
				else if (StackEntryType & SET_STRING)
					Entry.String = m_Stack.back( ).Dynamic;
				else if (StackEntryType & SET_OBJECTID)
					Entry.ObjectId = GetDynamicStackEntryObjectId( m_Stack.size( ) - 1 );
				else
					throw type_mismatch_exception( "attempted to pop entry of wrong type from stack" );

				//
				// We need to clean the next entry off of the string stack if
				// we were removing a dynamic parameter, -unless- the caller
				// was requesting a string.  If the caller wanted a string,
				// then they'll be the ones to remove the next string stack
				// entry.
				//

				if (!(StackEntryType & SET_STRING))
				{
#if STACK_DEBUG
					if ((size_t) m_Stack.back( ).Dynamic != m_StackStrings.size( ) - 1)
						throw invalid_handle_exception( "invalid string handle" );
#endif

					m_StackStrings.pop_back( );
				}

				m_Stack.pop_back( );
				m_StackTypes.pop_back( );

				return Entry;
			}
		}

		throw type_mismatch_exception( "attempted to pop entry of wrong type from stack" );
	}

	Entry = m_Stack.back( );

	m_Stack.pop_back( );
	m_StackTypes.pop_back( );

	return Entry;
}

void
NWScriptStack::StackPopRaw(
	__out STACK_ENTRY & StackEntry,
	__out STACK_TYPE_CODE & StackEntryType
	)
/*++

Routine Description:

	This routine returns the entry at the top of the stack.  Note that if a
	handle was referenced, the handle's associated stack is unmodified.

	The caller is responsible for performing any type checking desired.

Arguments:

	StackEntry - Receives the raw stack entry.

	StackEntryType - Receives the stack entry type.

Return Value:

	None.  An std::exception is raised on failure.

Environment:

	User mode.

--*/
{
	if (m_Stack.empty( ))
		throw stack_underflow_exception( "attempted to pop entry from empty stack" );

	CheckGuardZone( GetCurrentSP( ) - STACK_ENTRY_SIZE );

	StackEntry     = m_Stack.back( );
	StackEntryType = m_StackTypes.back( );

	m_Stack.pop_back( );
	m_StackTypes.pop_back( );
}

void
NWScriptStack::SetDynamicStackEntry(
	__in size_t Offset,
	__in int Int
	)
/*++

Routine Description:

	This routine assigns a value to a dynamic-typed stack entry.

Arguments:

	Offset - Supplies the offset into the stack (count of slots).

	Int - Supplies the value to assigned.

Return Value:

	None.  An std::exception is raised on failure.

Environment:

	User mode.

--*/
{
	STRING_HANDLE String;
	char          Str[ 32 ];

	String = m_Stack[ Offset ].String;

	if (String >= m_StackStrings.size( ))
		throw invalid_handle_exception( "illegal dynamic string stack handle" );

	StringCbPrintfA(
		Str,
		sizeof( Str ),
		"%d",
		Int);

	m_StackStrings[ String ] = Str;
}

void
NWScriptStack::SetDynamicStackEntry(
	__in size_t Offset,
	__in float Float
	)
/*++

Routine Description:

	This routine assigns a value to a dynamic-typed stack entry.

Arguments:

	Offset - Supplies the offset into the stack (count of slots).

	Float - Supplies the value to assigned.

Return Value:

	None.  An std::exception is raised on failure.

Environment:

	User mode.

--*/
{
	STRING_HANDLE String;
	char          Str[ 32 ];

	String = m_Stack[ Offset ].String;

	if (String >= m_StackStrings.size( ))
		throw invalid_handle_exception( "illegal dynamic string stack handle" );

	StringCbPrintfA(
		Str,
		sizeof( Str ),
		"%g",
		Float);

	m_StackStrings[ String ] = Str;
}

void
NWScriptStack::SetDynamicStackEntry(
	__in size_t Offset,
	__in const char * String
	)
/*++

Routine Description:

	This routine assigns a value to a dynamic-typed stack entry.

Arguments:

	Offset - Supplies the offset into the stack (count of slots).

	String - Supplies the value to assigned.

Return Value:

	None.  An std::exception is raised on failure.

Environment:

	User mode.

--*/
{
	STRING_HANDLE StringH;

	StringH = m_Stack[ Offset ].String;

	if (StringH >= m_StackStrings.size( ))
		throw invalid_handle_exception( "illegal dynamic string stack handle" );

	m_StackStrings[ StringH ] = String;
}

void
NWScriptStack::SetDynamicStackEntry(
	__in size_t Offset,
	__in NWN::OBJECTID ObjectId
	)
/*++

Routine Description:

	This routine assigns a value to a dynamic-typed stack entry.

Arguments:

	Offset - Supplies the offset into the stack (count of slots).

	ObjectId - Supplies the value to assigned.

Return Value:

	None.  An std::exception is raised on failure.

Environment:

	User mode.

--*/
{
	STRING_HANDLE String;
	char          Str[ 32 ];

	String = m_Stack[ Offset ].String;

	if (String >= m_StackStrings.size( ))
		throw invalid_handle_exception( "illegal dynamic string stack handle" );

	StringCbPrintfA(
		Str,
		sizeof( Str ),
		"%d",
		(int) ObjectId);

	m_StackStrings[ String ] = Str;
}


int
NWScriptStack::GetDynamicStackEntryInteger(
	__in size_t Offset
	) const
/*++

Routine Description:

	This routine performs a type conversion of a dynamic-typed stack entry.

Arguments:

	Offset - Supplies the offset into the stack (count of slots).

Return Value:

	The routine returns the converted value.  An std::exception is raised on
	failure.

Environment:

	User mode.

--*/
{
	STRING_HANDLE String;

	String = m_Stack[ Offset ].String;

	if (String >= m_StackStrings.size( ))
		throw invalid_handle_exception( "illegal dynamic string stack handle" );

	return atoi( m_StackStrings[ String ].c_str( ) );
}

float
NWScriptStack::GetDynamicStackEntryFloat(
	__in size_t Offset
	) const
/*++

Routine Description:

	This routine performs a type conversion of a dynamic-typed stack entry.

Arguments:

	Offset - Supplies the offset into the stack (count of slots).

Return Value:

	The routine returns the converted value.  An std::exception is raised on
	failure.

Environment:

	User mode.

--*/
{
	STRING_HANDLE String;

	String = m_Stack[ Offset ].String;

	if (String >= m_StackStrings.size( ))
		throw invalid_handle_exception( "illegal dynamic string stack handle" );

	return (float) atof( m_StackStrings[ String ].c_str( ) );
}

const std::string &
NWScriptStack::GetDynamicStackEntryString(
	__in size_t Offset
	) const
/*++

Routine Description:

	This routine performs a type conversion of a dynamic-typed stack entry.

Arguments:

	Offset - Supplies the offset into the stack (count of slots).

Return Value:

	The routine returns the converted value.  An std::exception is raised on
	failure.

Environment:

	User mode.

--*/
{
	STRING_HANDLE String;

	String = m_Stack[ Offset ].String;

	if (String >= m_StackStrings.size( ))
		throw invalid_handle_exception( "illegal dynamic string stack handle" );

	return m_StackStrings[ String ];
}

NWN::OBJECTID
NWScriptStack::GetDynamicStackEntryObjectId(
	__in size_t Offset
	) const
/*++

Routine Description:

	This routine performs a type conversion of a dynamic-typed stack entry.

Arguments:

	Offset - Supplies the offset into the stack (count of slots).

Return Value:

	The routine returns the converted value.  An std::exception is raised on
	failure.

Environment:

	User mode.

--*/
{
	STRING_HANDLE    String;
	char           * Endp;
	NWN::OBJECTID    ObjectId;

	String = m_Stack[ Offset ].String;

	if (String >= m_StackStrings.size( ))
		throw invalid_handle_exception( "illegal dynamic string stack handle" );

	if (m_StackStrings[ String ].empty( ))
		return m_InvalidObjId;

	ObjectId = (NWN::OBJECTID) _strtoui64(
		m_StackStrings[ String ].c_str( ),
		&Endp,
		10);

	//
	// If the conversion failed, return the invalid object id.
	//

	if (*Endp)
		ObjectId = m_InvalidObjId;

	return ObjectId;
}


void
NWScriptStack::GrowStack(
	__in size_t NumSlots
	)
/*++

Routine Description:

	This routine expands the stack to allocate storage for a given number of
	slots.

Arguments:

	NumSlots - Supplies the count of slots to expand the stack by.  The slots
	           are marked as untyped and uninitialized.

Return Value:

	None.  An std::exception is raised on failure.

Environment:

	User mode.

--*/
{
	STACK_ENTRY DefStackEntry;

	DefStackEntry.Raw = UNINITIALIZED_FILL;

	m_Stack.resize( NumSlots + m_Stack.size( ), DefStackEntry );
	m_StackTypes.resize( NumSlots + m_StackTypes.size( ), SET_INVALID );
}

void
NWScriptStack::AppendStackContentsToStack(
	__in NWScriptStack & DestStack,
	__in size_t SrcOffset,
	__in size_t CellsToCopy
	)
/*++

Routine Description:

	This routine copies a portion of the current stack over to a different
	stack, including any handle references.  Stack cells are appended to the
	destination stack's current logical SP.

	Note that the caller guarantees that the offset and pointers are valid.

Arguments:

	DestStack - Supplies the stack to copy into.

	SrcOffset - Supplies the offset into the local stack to copy from.

	CellsToCopy - Supplies the count of stack cells to copy to the destination
	              stack.

Return Value:

	None.  An std::exception is raised on failure.

Environment:

	User mode.

--*/
{
	size_t DstOffset;

	DstOffset = DestStack.m_Stack.size( );

	//
	// Expand the stack to make room for the copied data.
	//

	DestStack.GrowStack( CellsToCopy );

	//
	// Now perform the copy, one cell at a time.  The destination cells are
	// pushed onto the stack in turn.
	//

	for (size_t i = 0; i < CellsToCopy; i += 1)
	{
		STACK_TYPE_CODE SrcType = m_StackTypes[ SrcOffset + i ];

		if ((!(SrcType & SET_ENGINE_STRUCTURE)) &&
		    ((SrcType & SET_STRING) || (SrcType & SET_DYNAMIC)))
		{
			STRING_HANDLE StringSrc;

			//
			// We are copying a string, change the data backed by our handles
			// here instead of duplicating handle values.
			//
			// N.B.  Dynamic typed stack entries are implemented as strings and
			//       may be identically handled here.  We must copy the dynamic
			//       type flag here as we don't know what the fixed type is yet
			//       since CopyTopSP is an untyped copy.
			//

			StringSrc = m_Stack[ SrcOffset + i ].String;

#if STACK_DEBUG
			if (StringSrc >= m_StackStrings.size( ))
				throw invalid_handle_exception( "invalid source string handle in AppendStackContentsToStack" );
#endif

			if (DestStack.m_StackStrings.size( ) == ULONG_MAX)
				throw stack_overflow_exception( "out of string stack space" );

			DestStack.m_StackStrings.push_back( m_StackStrings[ StringSrc ] );

			DestStack.m_Stack[ DstOffset + i ].String = (STRING_HANDLE) (DestStack.m_StackStrings.size( ) - 1);
			DestStack.m_StackTypes[ DstOffset + i ]   = SrcType;
		}
		else if ((SrcType & SET_ENGINE_STRUCTURE) &&
		         (SrcType != SET_STACK_POINTER))
		{
			ENGINE_HANDLE EngineSrc;

			//
			// We are copying an engine structure, change the data backed by
			// our handles here instead of duplicating handle values.
			//

			EngineSrc = m_Stack[ SrcOffset + i ].EngineStruct;

#if STACK_DEBUG
			if (EngineSrc >= m_StackEngineStructures.size( ))
				throw invalid_handle_exception( "invalid source engine handle in AppendStackContentsToStack" );
#endif

			if (DestStack.m_StackEngineStructures.size( ) == ULONG_MAX)
				throw stack_overflow_exception( "out of engine structure stack space" );

			DestStack.m_StackEngineStructures.push_back( m_StackEngineStructures[ EngineSrc ] );

			DestStack.m_Stack[ DstOffset + i ].EngineStruct = (ENGINE_HANDLE) (DestStack.m_StackEngineStructures.size( ) - 1 );
			DestStack.m_StackTypes[ DstOffset + i ]         = SrcType;
		}
		else
		{
			//
			// This type has no special handling required, just copy it
			// directly.
			//

			DestStack.m_Stack[ DstOffset + i ].Raw  = m_Stack[ SrcOffset + i ].Raw;
			DestStack.m_StackTypes[ DstOffset + i ] = SrcType;
		}
	}
}

void
NWScriptStack::AppendStackContentsToStack(
	__in INWScriptStack * DestStack,
	__in size_t SrcOffset,
	__in size_t CellsToCopy
	)
/*++

Routine Description:

	This routine copies a portion of the current stack over to a different
	stack, including any handle references.  Stack cells are appended to the
	destination stack's current logical SP.

	Note that the caller guarantees that the offset and pointers are valid.

Arguments:

	DestStack - Supplies the stack to copy into.

	SrcOffset - Supplies the offset into the local stack to copy from.

	CellsToCopy - Supplies the count of stack cells to copy to the destination
	              stack.

Return Value:

	None.  An std::exception is raised on failure.

Environment:

	User mode.

--*/
{
	//
	// Now perform the copy, one cell at a time.  The destination cells are
	// pushed onto the stack in turn.
	//

	for (size_t i = 0; i < CellsToCopy; i += 1)
	{
		STACK_TYPE_CODE SrcType = m_StackTypes[ SrcOffset + i ];

		if ((!(SrcType & SET_ENGINE_STRUCTURE)) &&
		    ((SrcType & SET_STRING) || (SrcType & SET_DYNAMIC)))
		{
			STRING_HANDLE StringSrc;

			//
			// We are copying a string, change the data backed by our handles
			// here instead of duplicating handle values.
			//
			// N.B.  Dynamic typed stack entries are implemented as strings and
			//       may be identically handled here.  We must copy the dynamic
			//       type flag here as we don't know what the fixed type is yet
			//       since CopyTopSP is an untyped copy.
			//

			StringSrc = m_Stack[ SrcOffset + i ].String;

#if STACK_DEBUG
			if (StringSrc >= m_StackStrings.size( ))
				throw invalid_handle_exception( "invalid source string handle in AppendStackContentsToStack" );
#endif
			DestStack->StackPushString( m_StackStrings[ StringSrc ] );
		}
		else if ((SrcType & SET_ENGINE_STRUCTURE) &&
		         (SrcType != SET_STACK_POINTER))
		{
			ENGINE_HANDLE EngineSrc;

			//
			// We are copying an engine structure, change the data backed by
			// our handles here instead of duplicating handle values.
			//

			EngineSrc = m_Stack[ SrcOffset + i ].EngineStruct;

#if STACK_DEBUG
			if (EngineSrc >= m_StackEngineStructures.size( ))
				throw invalid_handle_exception( "invalid source engine handle in AppendStackContentsToStack" );
#endif

			DestStack->StackPushEngineStructure( m_StackEngineStructures[ EngineSrc ] );
		}
		else if (SrcType == SET_STACK_POINTER)
		{
			DestStack->StackPushInt( (int) m_Stack[ SrcOffset + i ].StackPointer );
		}
		else if (SrcType & SET_INTEGER)
		{
			DestStack->StackPushInt( m_Stack[ SrcOffset + i ].Int );
		}
		else if (SrcType & SET_FLOAT)
		{
			DestStack->StackPushFloat( m_Stack[ SrcOffset + i ].Float );
		}
		else if (SrcType & SET_OBJECTID)
		{
			DestStack->StackPushObjectId( m_Stack[ SrcOffset + i ].ObjectId );
		}
		else
		{
			throw invalid_stack_exception( "invalid type on stack for save to INWScriptStack" );
		}
	}
}


