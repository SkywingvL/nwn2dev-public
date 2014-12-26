/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptBridge.h

Abstract:

	This module defines the NWScriptBridge object, which allows the NWNScriptLib
	style script APIs to utilize the NWN2Server VM stack, command implementer,
	and loaded script resource files.

--*/

#ifndef _SOURCE_PROGRAMS_AURORASERVERNWSCRIPT_NWSCRIPTBRIDGE_H
#define _SOURCE_PROGRAMS_AURORASERVERNWSCRIPT_NWSCRIPTBRIDGE_H

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

class NWScriptBridge : public INWScriptActions,
                       public INWScriptStack
{

public:

	inline
	NWScriptBridge(
		__in IDebugTextOut * TextOut,
		__in NWScriptVM::ExecDebugLevel DebugLevel
		)
	: m_TextOut( TextOut ),
	  m_DebugLevel( DebugLevel ),
	  m_ServerVM( NULL ),
	  m_ServerVMScript( NULL ),
	  m_ServerVMFile( NULL ),
	  m_ServerVMStack( NULL ),
	  m_ServerCmdImplementer( NULL ),
	  m_JITStack( NULL ),
	  m_LastActionFromJIT( false ),
	  m_JITScriptAborted( false ),
	  m_IntegerSPSize( 0 )
	{
		m_JITStack = this;

		RegisterActions( );
	}

	virtual
	~NWScriptBridge(
		)
	{
	}

	bool
	PrepareForRunScript(
		__in NWN2Server::CVirtualMachine * ServerVM
		);

	inline
	NWScriptVM::ExecDebugLevel
	GetScriptDebug(
		) const
	{
		return m_DebugLevel;
	}

	inline
	IDebugTextOut *
	GetTextOut(
		)
	{
		return m_TextOut;
	}

	inline
	bool
	IsDebugLevel(
		__in NWScriptVM::ExecDebugLevel DebugLevel
		)
	{
		return (m_DebugLevel >= DebugLevel);
	}

	inline
	void
	SetDebugLevel(
		__in NWScriptVM::ExecDebugLevel DebugLevel
		)
	{
		m_DebugLevel = DebugLevel;
	}

	//
	// INWScriptActions implementation.
	//

	//
	// This callback routine is invoked when an action is executed.  The
	// implementation is responsible for performing whatever actions are
	// required for the given action call number, which may involve retrieving
	// parameters from the VM stack or pushing a return value on to the VM
	// stack.
	//
	// The action routine is permitted to throw an std::exception on a fatal
	// error condition, which may terminate the entire script chain.  It is
	// also permissible for the action routine to cause a re-entrant call into
	// the script VM (though the script VM may reject the call if the recursion
	// limit has been reached).
	//

	virtual
	void
	NWSCRIPTACTAPI
	OnExecuteAction(
		__in NWScriptVM & ScriptVM,
		__in NWScriptStack & VMStack,
		__in NWSCRIPT_ACTION ActionId,
		__in size_t NumArguments
		);

	//
	// This callback routine creates an empty engine structure of the given
	// engine type number.
	//

	virtual
	EngineStructurePtr
	NWSCRIPTACTAPI
	CreateEngineStructure(
		__in NWScriptStack::ENGINE_STRUCTURE_NUMBER EngineType
		);

	//
	// This callback routine is invoked when an action is executed.  The
	// implementation is responsible for performing whatever actions are
	// required for the given action call number, which may involve retrieving
	// parameters from the VM stack or pushing a return value on to the VM
	// stack.
	//
	// The action routine is permitted to return false on a fatal error
	// error condition, which may terminate the entire script chain.  It is
	// also permissible for the action routine to cause a re-entrant call into
	// the script VM (though the script VM may reject the call if the recursion
	// limit has been reached).
	//
	// N.B.  This routine is invoked by the JIT environment where there is no
	//       VM object in play.
	//

	virtual
	bool
	NWSCRIPTACTAPI
	OnExecuteActionFromJIT(
		__in NWSCRIPT_ACTION ActionId,
		__in size_t NumArguments
		);

	//
	// This callback routine is invoked when an action is executed.  The
	// implementation is responsible for performing whatever actions are
	// required for the given action call number.  Unlike the standard action
	// service handler callback, parameters and return values are not passed on
	// the VM stack.
	//
	// The action routine is permitted to return false on a fatal error
	// error condition, which may terminate the entire script chain.  It is
	// also permissible for the action routine to cause a re-entrant call into
	// the script VM (though the script VM may reject the call if the recursion
	// limit has been reached).
	//
	// N.B.  This routine is invoked by the JIT environment where there is no
	//       VM object in play.
	//
	// N.B.  Presently, engine structures cannot appear in the parameter or
	//       return value list for a fast call.
	//

	virtual
	bool
	NWSCRIPTACTAPI
	OnExecuteActionFromJITFast(
		__in NWSCRIPT_ACTION ActionId,
		__in size_t NumArguments,
		__in_ecount( NumCmds ) PCNWFASTACTION_CMD Cmds,
		__in size_t NumCmds,
		__in uintptr_t * CmdParams
		);



	//
	// INWScriptStack implementation.
	//

	virtual
	void
	StackPushInt(
		__in int Int
		);

	virtual
	int
	StackPopInt(
		);


	virtual
	void
	StackPushFloat(
		__in float Float
		);

	virtual
	float
	StackPopFloat(
		);


	virtual
	void
	StackPushString(
		__in const char * String
		);

	virtual
	void
	StackPushString(
		__in const std::string & String
		);

	virtual
	void
	StackPushStringAsNeutral(
		__in const NeutralString & String
		);

	std::string
	NWScriptBridge::StackPopString( // Note, not interface method
		);

	virtual
	NeutralString
	StackPopStringAsNeutral(
		);


	virtual
	void
	StackPushObjectId(
		__in NWN::OBJECTID ObjectId
		);

	virtual
	NWN::OBJECTID
	StackPopObjectId(
		);


	virtual
	void
	StackPushVector(
		__in const NWN::Vector3 & Vector
		);

	virtual
	NWN::Vector3
	StackPopVector(
		);


	virtual
	void
	StackPushEngineStructure(
		__in EngineStructurePtr Struct
		);

	virtual
	EngineStructurePtr
	StackPopEngineStructure(
		__in ENGINE_STRUCTURE_NUMBER EngineType
		);


	//
	// Classify the type of an item on the stack.
	//

	virtual
	BASE_STACK_TYPE
	GetTopOfStackType(
		) const;




private:


	struct NWScriptActionEntry
	{
		NWSCRIPT_ACTION                ActionId;
		const char                   * ActionName;
		std::vector< unsigned long >   TotalParameterSizes;
	};

	enum { MAX_ACTION_ID = 1058 };


	//
	// Register actions with the script system.
	//

	void
	RegisterActions(
		);

	//
	// Create the NWScript VM, used for interpreter fallback.
	//

	void
	CreateVM(
		);


	inline
	void
	AbortScript(
		)
	{
		throw std::runtime_error( "Script abortively terminated." );
	}

	//
	// Call the server's action service handler for a given action ordinal.
	//

	void
	ExecuteActionServiceHandler(
		__in NWSCRIPT_ACTION ActionId,
		__in size_t NumArguments
		);

	//
	// Transfer action service handler parameters from the NWScriptVM's stack to
	// the server's execution stack, in preparation for an action service
	// handler call.
	//

	void
	PushParametersToServerVMStack(
		__in NWScriptStack & VMStack,
		__in NWSCRIPT_ACTION ActionId,
		__in size_t NumArguments
		);

	//
	// Transfer action service handler return values from the server's execution
	// stack to the NWScriptVM's stack, after an action service handler has run
	// to completion.
	//

	void
	PopReturnValueFromServerVMStack(
		__in NWScriptStack & VMStack,
		__in NWSCRIPT_ACTION ActionId,
		__in size_t NumArguments
		);

	//
	// Return the number of stack bytes consumed by a type.
	//

	unsigned long
	GetTypeSize(
		__in NWACTION_TYPE Type
		);

	IDebugTextOut                             * m_TextOut;
	NWScriptVM::ExecDebugLevel                  m_DebugLevel;
	NWN2Server::CVirtualMachine               * m_ServerVM;
	NWN2Server::CVirtualMachineScript         * m_ServerVMScript;
	NWN2Server::CVirtualMachineFile           * m_ServerVMFile;
	NWN2Server::CVirtualMachineStack          * m_ServerVMStack;
	NWN2Server::CVirtualMachineCmdImplementer * m_ServerCmdImplementer;
	INWScriptStack                            * m_JITStack;
	bool                                        m_LastActionFromJIT;
	bool                                        m_JITScriptAborted;
	NWScriptStack::STACK_POINTER                m_IntegerSPSize;

	//
	// Define the action handler table, which is dispatched by the core
	// OnExecuteAction routine.
	//

	NWScriptActionEntry                         m_ActionHandlerTable[ MAX_ACTION_ID ];

};

class EngineStructureBridge : public EngineStructure
{

public:

	inline
	EngineStructureBridge(
		__in ENGINE_STRUCTURE_NUMBER EngineType,
		__in NWN2Server::CVirtualMachineCmdImplementer * CmdImplementer,
		__in void * Representation
		)
	: EngineStructure( EngineType ),
	  m_CmdImplementer( CmdImplementer ),
	  m_Representation( Representation )
	{
	}

	inline
	virtual
	~EngineStructureBridge(
		)
	{
		DeleteRepresentation( );
	}

	//
	// Compare to structures of identical types.  The routine returns ture if
	// the structures are logically identical, else false is returned.
	//

	virtual
	bool
	CompareEngineStructure(
		__in const EngineStructure * Other
		) const;

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
	
	//
	// Return the opaque NWN2Server representation of the engine structure that
	// is encapsulated within the EngineStructureBridge.
	//

	inline
	void *
	GetRepresentation(
		) const
	{
		return m_Representation;
	}

	//
	// Release ownership of the representation.
	//

	inline
	void
	ReleaseOwnership(
		)
	{
		m_Representation = NULL;
	}

private:

	void
	DeleteRepresentation(
		);

	NWN2Server::CVirtualMachineCmdImplementer * m_CmdImplementer;
	void                                      * m_Representation;

};

#endif

