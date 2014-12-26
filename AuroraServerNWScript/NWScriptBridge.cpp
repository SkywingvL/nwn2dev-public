/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptBridge.cpp

Abstract:

	This module houses the NWScriptBridge object, which allows the NWNScriptLib
	style script APIs to utilize the NWN2Server VM stack, command implementer,
	and loaded script resource files.

--*/

#include "Precomp.h"
#include "Offsets.h"
#include "NWN2Def.h"
#include "NWScriptBridge.h"

using namespace NWN2Server;

bool
NWScriptBridge::PrepareForRunScript(
	__in NWN2Server::CVirtualMachine * ServerVM
	)
/*++

Routine Description:

	This routine prepares for a RunScript request by setting up the current
	bridge pointers.

Arguments:

	ServerVM - Supplies the active NWN2Server CVirtualMachine context.

Return Value:

	The routine returns true on success, else false on failure.

Environment:

	User mode.

--*/
{
	m_ServerVM             = ServerVM;
	m_ServerVMScript       = &ServerVM->m_pVirtualMachineScript[ ServerVM->m_nRecursionLevel ];
	m_ServerVMFile         = &ServerVM->m_cVMFile;
	m_ServerVMStack        = &ServerVM->m_cRunTimeStack;
	m_ServerCmdImplementer = ServerVM->GetCommandImplementer( );

	return true;
}




void
NWSCRIPTACTAPI
NWScriptBridge::OnExecuteAction(
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

	UNREFERENCED_PARAMETER( VMStack );

	m_LastActionFromJIT = false;

	if (ActionId < MAX_ACTION_ID)
		ActionEntry = &m_ActionHandlerTable[ ActionId ];
	else
		ActionEntry = NULL;

	if (ScriptVM.IsDebugLevel( NWScriptVM::EDL_Calls ))
	{
		m_TextOut->WriteText(
			"NWScriptBridge::OnExecuteAction: Executing action %s (%lu) with %lu arguments.\n",
			ActionEntry != NULL ? ActionEntry->ActionName : "<INVALID>",
			ActionId,
			(unsigned long) NumArguments);
	}

	if (ActionEntry == NULL)
	{
		ScriptVM.AbortScript( );
		return;
	}

	try
	{
		PushParametersToServerVMStack( VMStack, ActionId, NumArguments );
		ExecuteActionServiceHandler( ActionId, NumArguments );
		PopReturnValueFromServerVMStack( VMStack, ActionId, NumArguments );
	}
	catch (std::exception &e)
	{
		if (ScriptVM.IsDebugLevel( NWScriptVM::EDL_Errors ))
		{
			m_TextOut->WriteText(
				"NWScriptBridge::OnExecuteAction: Exception '%s' executing action %s (%lu).\n",
				e.what( ),
				ActionEntry != NULL ? ActionEntry->ActionName : "<INVALID>",
				ActionId);
		}

		ScriptVM.AbortScript( );
	}
}

bool
NWSCRIPTACTAPI
NWScriptBridge::OnExecuteActionFromJIT(
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

	if (IsDebugLevel( NWScriptVM::EDL_Calls ))
	{
		m_TextOut->WriteText(
			"NWScriptBridge::OnExecuteActionFromJIT: Executing action %s (%lu) with %lu arguments.\n",
			ActionEntry != NULL ? ActionEntry->ActionName : "<INVALID>",
			ActionId,
			(unsigned long) NumArguments);
	}

	try
	{
		ExecuteActionServiceHandler( ActionId, NumArguments );
	}
	catch (std::exception &e)
	{
		if (IsDebugLevel( NWScriptVM::EDL_Errors ))
		{
			m_TextOut->WriteText(
				"NWScriptBridge::OnExecuteActionFromJIT: Exception '%s' executing action %s (%lu).\n",
				e.what( ),
				ActionEntry->ActionName,
				ActionId);
		}

		return false;
	}

	return !m_JITScriptAborted;
}

bool
NWSCRIPTACTAPI
NWScriptBridge::OnExecuteActionFromJITFast(
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

	if (IsDebugLevel( NWScriptVM::EDL_Calls ))
	{
		m_TextOut->WriteText(
			"NWScriptBridge::OnExecuteActionFromJITFast: Executing action %s (%lu) with %lu arguments.\n",
			ActionEntry != NULL ? ActionEntry->ActionName : "<INVALID>",
			ActionId,
			(unsigned long) NumArguments);
	}

	try
	{
		if (!IsDebugLevel( NWScriptVM::EDL_Verbose ))
		{
			for (size_t i = 0; i < NumCmds; i += 1)
			{
				switch (Cmds[ i ])
				{

				case NWFASTACTION_PUSHINT:
					NWScriptBridge::StackPushInt( (int) *CmdParams++ );
					break;

				case NWFASTACTION_POPINT:
					**(int **) CmdParams++ = NWScriptBridge::StackPopInt( );
					break;

				case NWFASTACTION_PUSHFLOAT:
					NWScriptBridge::StackPushFloat( *(float *) &*CmdParams++ );
					break;
				
				case NWFASTACTION_POPFLOAT:
					**(float **) CmdParams++ = NWScriptBridge::StackPopFloat( );
					break;

				case NWFASTACTION_PUSHOBJECTID:
					NWScriptBridge::StackPushObjectId( (NWN::OBJECTID) *CmdParams++ );
					break;

				case NWFASTACTION_POPOBJECTID:
					**(NWN::OBJECTID **) CmdParams++ = NWScriptBridge::StackPopObjectId( );
					break;

				case NWFASTACTION_PUSHSTRING:
					NWScriptBridge::StackPushStringAsNeutral(
						**(const NWScriptStack::NeutralString **) CmdParams++ );
					break;

				case NWFASTACTION_POPSTRING:
					**(NWScriptStack::NeutralString **) CmdParams++ = NWScriptBridge::StackPopStringAsNeutral( );
					break;

				case NWFASTACTION_CALL:
					ExecuteActionServiceHandler( ActionId, NumArguments );
					break;

				default:
					throw std::runtime_error( "Unrecognized fast action command." );

				}
			}
		}
		else
		{
			for (size_t i = 0; i < NumCmds; i += 1)
			{
				switch (Cmds[ i ])
				{

				case NWFASTACTION_PUSHINT:
					m_TextOut->WriteText( "VMPUSH: (int) %d\n", (int) *CmdParams );
					NWScriptBridge::StackPushInt( (int) *CmdParams++ );
					break;

				case NWFASTACTION_POPINT:
					**(int **) CmdParams++ = NWScriptBridge::StackPopInt( );
					m_TextOut->WriteText( "VMPOP: (int) %d\n", **(int **) (CmdParams-1) );
					break;

				case NWFASTACTION_PUSHFLOAT:
					m_TextOut->WriteText( "VMPUSH: (float) %g\n", *(float *) &*CmdParams );
					NWScriptBridge::StackPushFloat( *(float *) &*CmdParams++ );
					break;
				
				case NWFASTACTION_POPFLOAT:
					**(float **) CmdParams++ = NWScriptBridge::StackPopFloat( );
					m_TextOut->WriteText( "VMPOP: (float) %g\n", **(float **) (CmdParams-1) );
					break;

				case NWFASTACTION_PUSHOBJECTID:
					m_TextOut->WriteText( "VMPUSH: (object) %08X\n", (NWN::OBJECTID) *CmdParams );
					NWScriptBridge::StackPushObjectId( (NWN::OBJECTID) *CmdParams++ );
					break;

				case NWFASTACTION_POPOBJECTID:
					**(NWN::OBJECTID **) CmdParams++ = NWScriptBridge::StackPopObjectId( );
					m_TextOut->WriteText( "VMPOP: (object) %08X\n", **(NWN::OBJECTID **) (CmdParams-1) );
					break;

				case NWFASTACTION_PUSHSTRING:
					m_TextOut->WriteText( "VMPUSH: (string) %.*s\n", (unsigned long) (**(const NWScriptStack::NeutralString **) CmdParams).second, (**(const NWScriptStack::NeutralString **) CmdParams).first );
					NWScriptBridge::StackPushStringAsNeutral(
						**(const NWScriptStack::NeutralString **) CmdParams++ );
					break;

				case NWFASTACTION_POPSTRING:
					**(NWScriptStack::NeutralString **) CmdParams++ = NWScriptBridge::StackPopStringAsNeutral( );
					m_TextOut->WriteText( "VMPOP: (string) %.*s\n", (unsigned long) (**(const NWScriptStack::NeutralString **) (CmdParams-1)).second, (**(const NWScriptStack::NeutralString **) (CmdParams-1)).first );
					break;

				case NWFASTACTION_CALL:
					m_TextOut->WriteText( "VMACTION: %lu (%lu arguments)\n", (unsigned long) ActionId, (unsigned long) NumArguments );
					ExecuteActionServiceHandler( ActionId, NumArguments );
					break;

				default:
					throw std::runtime_error( "Unrecognized fast action command." );

				}
			}
		}
	}
	catch (std::exception &e)
	{
		if (IsDebugLevel( NWScriptVM::EDL_Errors ))
		{
			m_TextOut->WriteText(
				"NWScriptBridge::OnExecuteActionFromJITFast: Exception '%s' executing action %s (%lu).\n",
				e.what( ),
				ActionEntry->ActionName,
				ActionId);
		}

		return false;
	}

	return !m_JITScriptAborted;
}




EngineStructurePtr
NWSCRIPTACTAPI
NWScriptBridge::CreateEngineStructure(
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
	EngineStructurePtr   Bridge;
	void               * Representation;

	Representation = m_ServerCmdImplementer->CreateGameDefinedStructure(
		(ENGINE_STRUCTURE_TYPE) EngineType
		);

	if (Representation == NULL)
		return NULL;

	//
	// Construct the bridge wrapper and return it.
	//

	try
	{
		Bridge = new EngineStructureBridge(
			EngineType,
			m_ServerCmdImplementer,
			Representation);
	}
	catch (std::exception)
	{
		m_ServerCmdImplementer->DestroyGameDefinedStructure(
			(ENGINE_STRUCTURE_TYPE) EngineType,
			Representation);

		return NULL;
	}

	return Bridge;
}





void
NWScriptBridge::StackPushInt(
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
	if (!m_ServerVM->StackPushInteger( Int ))
		throw std::runtime_error( "StackPushInteger failed" );
}

int
NWScriptBridge::StackPopInt(
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
	int Value;

	if (!m_ServerVM->StackPopInteger( &Value ))
		throw std::runtime_error( "StackPopInteger failed" );

	return Value;
}

void
NWScriptBridge::StackPushFloat(
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
	if (!m_ServerVM->StackPushFloat( Float ))
		throw std::runtime_error( "StackPushFloat failed" );
}

float
NWScriptBridge::StackPopFloat(
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
	float Value;

	if (!m_ServerVM->StackPopFloat( &Value ))
		throw std::runtime_error( "StackPopFloat failed" );

	return Value;
}

void
NWScriptBridge::StackPushString(
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
	if (!m_ServerVM->StackPushString( String ))
		throw std::runtime_error( "StackPushString failed" );
}

void
NWScriptBridge::StackPushString(
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
	if (!m_ServerVM->StackPushString( String ))
		throw std::runtime_error( "StackPushString failed" );
}

void
NWScriptBridge::StackPushStringAsNeutral(
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
	if (!m_ServerVM->StackPushString( std::string( String.first, String.second ) ))
		throw std::runtime_error( "StackPushString failed" );
}

std::string
NWScriptBridge::StackPopString(
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
	std::string Str;

	if (!m_ServerVM->StackPopString( &Str ))
		throw std::runtime_error( "StackPopString failed" );

	return Str;
}

NWScriptBridge::NeutralString
NWScriptBridge::StackPopStringAsNeutral(
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
	NeutralString Str;

	if (!m_ServerVM->StackPopString( &Str ))
		throw std::runtime_error( "StackPopString failed" );

	return Str;
}


void
NWScriptBridge::StackPushObjectId(
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
	if (!m_ServerVM->StackPushObject( ObjectId ))
		throw std::runtime_error( "StackPushObject failed" );
}

NWN::OBJECTID
NWScriptBridge::StackPopObjectId(
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
	NWN::OBJECTID Value;

	if (!m_ServerVM->StackPopObject( &Value ))
		throw std::runtime_error( "StackPopObject failed" );

	return Value;
}


void
NWScriptBridge::StackPushVector(
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
	if (!m_ServerVM->StackPushVector( &Vector ))
		throw std::runtime_error( "StackPushVector failed" );
}

NWN::Vector3
NWScriptBridge::StackPopVector(
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

	if (!m_ServerVM->StackPopVector( &Vector ))
		throw std::runtime_error( "StackPopVector failed" );

	return Vector;
}


void
NWScriptBridge::StackPushEngineStructure(
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
	EngineStructureBridge * Bridge = (EngineStructureBridge *) Struct.get( );

	if ((Bridge == NULL) || (Bridge->GetRepresentation( ) == NULL))
		throw std::runtime_error( "Attempting to push NULL engine structure" );

	//
	// Push the engine structure's internal representation on to the stack,
	// which creates a copy of it.  We retain our reference to the original.
	//

	if (!m_ServerVM->StackPushEngineStructure(
		(NWN2Server::ENGINE_STRUCTURE_TYPE) Bridge->GetEngineType( ),
		Bridge->GetRepresentation( ) ))
	{
		throw std::runtime_error( "StackPushEngineStructure failed" );
	}
}

EngineStructurePtr
NWScriptBridge::StackPopEngineStructure(
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
	EngineStructureBridge::Ptr   Bridge;
	void                       * Representation;

	if (!m_ServerVM->StackPopEngineStructure(
		(NWN2Server::ENGINE_STRUCTURE_TYPE) EngineType,
		&Representation))
	{
		throw std::runtime_error( "StackPopEngineStructure failed" );
	}

	//
	// Construct the bridge wrapper and return it.
	//

	try
	{
		Bridge = new EngineStructureBridge(
			EngineType,
			m_ServerCmdImplementer,
			Representation);
	}
	catch (std::exception)
	{
		m_ServerCmdImplementer->DestroyGameDefinedStructure(
			(ENGINE_STRUCTURE_TYPE) EngineType,
			Representation);

		throw;
	}

	return Bridge;
}

NWScriptBridge::BASE_STACK_TYPE
NWScriptBridge::GetTopOfStackType(
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
	int Type;

	//
	// Map the server's type encoding to our type encoding.
	//

	Type = m_ServerVM->GetTopOfStackType( );

	switch (Type)
	{

	case -1: // Stack underflow
		throw std::runtime_error( "Stack is empty" );

	case CVirtualMachineStack::ST_INTEGER:
		return BST_INT;

	case CVirtualMachineStack::ST_FLOAT:
		return BST_FLOAT;

	case CVirtualMachineStack::ST_STRING:
		return BST_STRING;

	case CVirtualMachineStack::ST_OBJECT:
		return BST_OBJECTID;

	default:
		//
		// We may have an engine structure type.  Map it as appropriate.
		//

		if ((Type >= CVirtualMachineStack::ST_ENGINE_0) &&
		    (Type <= CVirtualMachineStack::ST_ENGINE_9))
		{
			return (BASE_STACK_TYPE) (BST_ENGINE_0 + (Type - CVirtualMachineStack::ST_ENGINE_0));
		}

		throw std::runtime_error( "Unknown type on stack" );
	}
}

void
NWScriptBridge::RegisterActions(
	)
/*++

Routine Description:

	This routine is called to initialize the action handler table in the
	script bridge.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode, script host initialization time only.

--*/
{
	const NWACTION_DEFINITION * ActionDef;
	unsigned long               ParamsSize;
	NWScriptStack               Stack;

	C_ASSERT( MAX_ACTION_ID == MAX_ACTION_ID_NWN2 );

	m_IntegerSPSize = Stack.GetStackIntegerSize( );

	for (size_t i = 0; i < MAX_ACTION_ID; i += 1)
	{
		ActionDef = &NWActions_NWN2[ i ];

		m_ActionHandlerTable[ i ].ActionId   = (NWSCRIPT_ACTION) i;
		m_ActionHandlerTable[ i ].ActionName = ActionDef->Name;

		//
		// Calculate the total parameter size at each parameter index for the
		// action.  This will be used to thunk action parameters between the
		// NWScriptVM stack and the server's execution stack, for NWScriptVM
		// fallback mode.
		//

		m_ActionHandlerTable[ i ].TotalParameterSizes.reserve(
			ActionDef->NumParameters
			);

		ParamsSize = 0;

		for (unsigned long Param = 0;
		    Param < ActionDef->NumParameters;
		    Param += 1)
		{
			ParamsSize += GetTypeSize( ActionDef->ParameterTypes[ Param ] );
			m_ActionHandlerTable[ i ].TotalParameterSizes.push_back( ParamsSize );
		}
	}
}

void
NWScriptBridge::ExecuteActionServiceHandler(
	__in NWSCRIPT_ACTION ActionId,
	__in size_t NumArguments
	)
/*++

Routine Description:

	This routine is called to invoke an action service handler located in the
	server on behalf of the script execution environment behind the script
	bridge.

Arguments:

	ActionId - Supplies the action service ordinal that was requested.

	NumArguments - Supplies the count of arguments passed to the action ordinal.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	SCRIPT_STATUS Status;

	Status = m_ServerCmdImplementer->ExecuteCommand(
		(int) ActionId,
		(int) NumArguments);

	if (Status < 0)
	{
		if (IsDebugLevel( NWScriptVM::EDL_Errors ))
		{
			m_TextOut->WriteText(
				"NWScriptBridge::ExecuteActionServiceHandler: Error '%d' returned from action service handler %lu (%lu arguments).\n",
				Status,
				(unsigned long) ActionId,
				(unsigned long) NumArguments);
		}

		throw std::runtime_error( "Action service handler invocation failed." );
	}
}

void
NWScriptBridge::PushParametersToServerVMStack(
	__in NWScriptStack & VMStack,
	__in NWSCRIPT_ACTION ActionId,
	__in size_t NumArguments
	)
/*++

Routine Description:

	This routine is invoked when a call is being made from the NWScriptVM to the
	server's action service handler dispatcher.  Its purpose is to move
	arguments from the NWScriptVM stack to the server's execution stack.

Arguments:

	VMStack - Supplies the currently executing script stack.

	ActionId - Supplies the action service ordinal that was requested.

	NumArguments - Supplies the count of arguments passed to the action ordinal.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, called from action service handler dispatching for VM only.

--*/
{
	const NWACTION_DEFINITION    * ActionDef;
	const NWScriptActionEntry    * ActionEntry;
	NWACTION_TYPE                  Type;

	ActionDef   = &NWActions_NWN2[ ActionId ];
	ActionEntry = &m_ActionHandlerTable[ ActionId ];

	if (NumArguments < ActionDef->MinParameters)
		throw std::runtime_error( "Too few arguments for action service handler." );
	else if (NumArguments > ActionDef->NumParameters)
		throw std::runtime_error( "Too many arguments for action service handler." );

	//
	// Carefully copy arguments over to the server execution stack.  Note that
	// we may have defaulted arguments (which must not be copied), and some
	// arguments may take up zero space (actions), and some arguments may take
	// up more than STACK_ENTRY_SIZE (vectors).
	//

	for (unsigned long i = NumArguments; i != 0; i -= 1)
	{
		NWScriptStack::STACK_POINTER SP;

		Type = ActionDef->ParameterTypes[ i - 1 ];

		SP = -(NWScriptStack::STACK_POINTER) ActionEntry->TotalParameterSizes[ i - 1 ];

		switch (Type)
		{

		case ACTIONTYPE_ACTION:
			break;

		case ACTIONTYPE_INT:
			StackPushInt( VMStack.GetStackInt( SP ) );
			break;

		case ACTIONTYPE_FLOAT:
			StackPushFloat( VMStack.GetStackFloat( SP ) );
			break;

		case ACTIONTYPE_STRING:
			StackPushString( VMStack.GetStackString( SP ) );
			break;

		case ACTIONTYPE_OBJECT:
			StackPushObjectId( VMStack.GetStackObjectId( SP ) );
			break;

		case ACTIONTYPE_VECTOR:
			StackPushVector( VMStack.GetStackVector( SP ) );
			break;

		default:
			if ((Type >= ACTIONTYPE_ENGINE_0) &&
				 (Type <= ACTIONTYPE_ENGINE_9))
			{
				StackPushEngineStructure(
					VMStack.GetStackEngineStructure(
						SP,
						(ENGINE_STRUCTURE_NUMBER) (Type - ACTIONTYPE_ENGINE_0)
						)
					);

				break;
			}

			throw std::runtime_error( "Illegal action service handler parameter type" );

		}
	}

	if (NumArguments != 0)
	{
		if (IsDebugLevel( NWScriptVM::EDL_Verbose ))
		{
			m_TextOut->WriteText(
				"NWScriptBridge::PushReturnValuesToServerVMStack: Removed %d bytes of parameters from VM stack for service handler %lu.\n",
				(NWScriptStack::STACK_POINTER) ActionEntry->TotalParameterSizes[ NumArguments - 1 ],
				(unsigned long) ActionId);
		}

		VMStack.AddSP(
			-(NWScriptStack::STACK_POINTER) ActionEntry->TotalParameterSizes[ NumArguments - 1 ]
			);
	}
}

void
NWScriptBridge::PopReturnValueFromServerVMStack(
	__in NWScriptStack & VMStack,
	__in NWSCRIPT_ACTION ActionId,
	__in size_t NumArguments
	)
/*++

Routine Description:

	This routine is invoked when a call is being made from the NWScriptVM to the
	server's action service handler dispatcher.  Its purpose is to move
	return values from the server's execution stack to the NWScriptVM stack.

Arguments:

	VMStack - Supplies the currently executing script stack.

	ActionId - Supplies the action service ordinal that was requested.

	NumArguments - Supplies the count of arguments passed to the action ordinal.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, called from action service handler dispatching for VM only.

--*/
{
	NWACTION_TYPE Type;

	UNREFERENCED_PARAMETER( NumArguments );

	Type = NWActions_NWN2[ ActionId ].ReturnType;

	switch (Type)
	{

	case ACTIONTYPE_VOID:
		break;

	case ACTIONTYPE_INT:
		VMStack.StackPushInt( StackPopInt( ) );
		break;

	case ACTIONTYPE_FLOAT:
		VMStack.StackPushFloat( StackPopFloat( ) );
		break;

	case ACTIONTYPE_STRING:
		VMStack.StackPushString( StackPopString( ) );
		break;

	case ACTIONTYPE_OBJECT:
		VMStack.StackPushObjectId( StackPopObjectId( ) );
		break;

	case ACTIONTYPE_VECTOR:
		VMStack.StackPushVector( StackPopVector( ) );
		break;

	default:
		if ((Type >= ACTIONTYPE_ENGINE_0) &&
		    (Type <= ACTIONTYPE_ENGINE_9))
		{
			VMStack.StackPushEngineStructure(
				StackPopEngineStructure(
					(ENGINE_STRUCTURE_NUMBER) (Type - ACTIONTYPE_ENGINE_0)
					)
				);

			break;
		}

		throw std::runtime_error( "Illegal action service handler return type" );

	}
}

unsigned long
NWScriptBridge::GetTypeSize(
	__in NWACTION_TYPE Type
	)
/*++

Routine Description:

	This routine returns the logical size of a type on the NWScriptVM stack.

Arguments:

	Type - Supplies the type to inquire about.

Return Valuei

	The type size is returned.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	unsigned long IntegerSize;

	switch (Type)
	{

	case ACTIONTYPE_VOID:
		IntegerSize = 0;
		break;

	case ACTIONTYPE_INT:
		IntegerSize = 1;
		break;

	case ACTIONTYPE_FLOAT:
		IntegerSize = 1;
		break;

	case ACTIONTYPE_STRING:
		IntegerSize = 1;
		break;

	case ACTIONTYPE_OBJECT:
		IntegerSize = 1;
		break;

	case ACTIONTYPE_VECTOR:
		IntegerSize = 3;
		break;

	case ACTIONTYPE_ACTION:
		IntegerSize = 0;
		break;

	default:
		if ((Type >= ACTIONTYPE_ENGINE_0) &&
		    (Type <= ACTIONTYPE_ENGINE_9))
		{
			IntegerSize = 1;
			break;
		}

		throw std::runtime_error( "Unable to query stack size for illegal type." );

	}

	return (unsigned long) m_IntegerSPSize * IntegerSize;
}




bool
EngineStructureBridge::CompareEngineStructure(
	__in const EngineStructure * Other
	) const
/*++

Routine Description:

	This routine compares whether two engine structures of the same type have
	identical contents.

Arguments:

	Other - Supplies the other engine structure to compare agianst the current
	        engine structure.

Return Value:

	The routine returns a Boolean value indicating true if the structures were
	identical, else false if the structures were different.

Environment:

	User mode.

--*/
{
	void * LhsRep;
	void * RhsRep;

	LhsRep = GetRepresentation( );
	RhsRep = ((EngineStructureBridge *) Other)->GetRepresentation( );

	if (LhsRep == RhsRep)
		return true;

	if (LhsRep == NULL || RhsRep == NULL)
		return false;

	if (m_CmdImplementer->GetEqualGameDefinedStructure(
		(ENGINE_STRUCTURE_TYPE) GetEngineType( ),
		LhsRep,
		RhsRep))
	{
		return true;
	}

	return false;
}

void
EngineStructureBridge::DeleteRepresentation(
	)
/*++

Routine Description:

	This routine destroys the internal representation of the engine structure.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	if (m_Representation == NULL)
		return;

	m_CmdImplementer->DestroyGameDefinedStructure(
		(ENGINE_STRUCTURE_TYPE) GetEngineType( ),
		m_Representation);

	m_Representation = NULL;
}





void
NWN2Server::CVirtualMachineStack::WrapAndPushEngineStructure(
	__in INWScriptStack * Stack,
	__in void * Representation,
	__in ENGINE_STRUCTURE_TYPE EngineType
	)
/*++

Routine Description:

	This routine packages an engine structure into the bridge wrapper and pushes
	it onto a conventional INWScriptStack.

Arguments:

	Stack - Supplies the stack to push the engine structure onto.

	Representation - Supplies the internal representation of the engine
	                 structure to push.

	EngineType - Supplies the type number of the engine structure that is to be
	             pushed.

Return Value:

	The routine returns the engine structure value at the top of the stack.  An
	std::exception is raised on failure (e.g. type mismatch).

Environment:

	User mode.

--*/
{
	EngineStructureBridge::Ptr      Bridge;
	void                          * NewRepresentation;
	CVirtualMachineCmdImplementer * CmdImplementer;

	//
	// Construct the bridge wrapper and place it on the stack.
	//

	CmdImplementer = m_pVMachine->GetCommandImplementer( );

	NewRepresentation = CmdImplementer->CopyGameDefinedStructure(
		EngineType,
		Representation);

	if (NewRepresentation == NULL)
		throw std::runtime_error( "failed to copy engine structure" );

	try
	{
		Bridge = new EngineStructureBridge(
			(INWScriptStack::ENGINE_STRUCTURE_NUMBER) EngineType,
			CmdImplementer,
			NewRepresentation);
	}
	catch (std::exception)
	{
		CmdImplementer->DestroyGameDefinedStructure(
			EngineType,
			NewRepresentation);

		throw;
	}

	Stack->StackPushEngineStructure( Bridge );
}

