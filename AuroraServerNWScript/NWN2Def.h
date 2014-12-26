/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    NWN2Defs.h

Abstract:

    This module defines NWN2Server-internal data structures.

--*/

#ifndef _PROGRAMS_AURORASERVERNWSCRIPT_NWN2DEFS_H
#define _PROGRAMS_AURORASERVERNWSCRIPT_NWN2DEFS_H

#ifdef _MSC_VER
#pragma once
#pragma pointers_to_members(full_generality, single_inheritance)
#endif

namespace NWN2Server
{

VOID
__cdecl
FreeNwn2(
	__in PVOID P
	);

class CExoString
{

public:

	inline
	CExoString(
		)
	: m_sString( NULL ),
	  m_nBufferLength( 0 )
	{
	}

	inline
	~CExoString(
		)
	{
		if (m_sString != NULL)
		{
			FreeNwn2( (PVOID) m_sString );
			m_sString = NULL;
		}
	}

	inline
	void
	ReleaseOwnership(
		)
	{
		m_sString       = NULL;
		m_nBufferLength = 0;
	}

	const char    * m_sString;
	ULONG           m_nBufferLength;

};

C_ASSERT( sizeof( CExoString ) == 8 );

typedef int SCRIPT_STATUS;

enum ENGINE_STRUCTURE_TYPE
{
	EST_EFFECT         = 0,
	EST_EVENT          = 1,
	EST_LOCATION       = 2,
	EST_TALENT         = 3,
	EST_ITEMPROPERTY   = 4,


	EST_ENGINE_0       = 0,
	EST_ENGINE_9       = 9,

	EST_LAST
};

class CVirtualMachineCmdImplementer
{

public:

	//
	// Note, the vtable layout for these must match the layout defined by the
	// game!
	//

	virtual void __thiscall dummy_destructor( ) = 0;
	virtual void __thiscall dummy_InitializeCommands( ) = 0;
	virtual SCRIPT_STATUS __thiscall ExecuteCommand( int CommandID, int ArgumentCount ) = 0;
	virtual void __thiscall RunScriptCallback( __in const CExoString & ScriptName ) = 0;
	virtual void __thiscall dummy_ReportError( ) = 0;
	virtual void * __thiscall CreateGameDefinedStructure( ENGINE_STRUCTURE_TYPE EngineStructureType ) = 0;
	virtual void __thiscall DestroyGameDefinedStructure( ENGINE_STRUCTURE_TYPE EngineStructureType, void * EngineStructure ) = 0;
	virtual BOOL __thiscall GetEqualGameDefinedStructure( ENGINE_STRUCTURE_TYPE EngineStructureType, void * EngineStructure1, void * EngineStructure2 ) = 0;
	virtual void * __thiscall CopyGameDefinedStructure( ENGINE_STRUCTURE_TYPE EngineStructureType, void * EngineStructure ) = 0;
	virtual void __thiscall dummy_SaveGameDefinedStructure( ) = 0;
	virtual void __thiscall dummy_LoadGameDefinedStructure( ) = 0;
	virtual void __thiscall dummy_GetGameDefinedStructureName( ) = 0;
	virtual void __thiscall dummy_GetDebuggerLabelName( ) = 0;
	virtual void __thiscall dummy_GetEngineStructureWatchView( ) = 0;
	virtual void __thiscall dummy_GetObjectWatchView( ) = 0;
	virtual void __thiscall dummy_DebugGUIStart( ) = 0;
	virtual void __thiscall dummy_DebugGUIStop( ) = 0;
	virtual void __thiscall dummy_DebugGUISendMessageTo( ) = 0;
	virtual void __thiscall dummy_DebugGUIGetMessageFrom( ) = 0;
	virtual void __thiscall dummy_DebugGUIUpdate( ) = 0;
	virtual void __thiscall dummy_NotifyObjectRunningScript( ) = 0;

	enum { VTABLE_SIZE = 21 };
	enum { VT_RunScriptCallback = 3 };

};

class CNWVirtualMachineCommands : public CVirtualMachineCmdImplementer
{

public:

};

C_ASSERT( sizeof( CVirtualMachineCmdImplementer ) == 4 );

class CRes
{

public:

	void                 * Vftable;
	void                 * m_pResource;
	BOOL                   m_bAllocHeaderData;
	BOOL                   m_bAllocTrailingData;
	short                  m_nCRC;
	unsigned short         m_nDemands;
	unsigned short         m_nRequests;
	unsigned long          m_ID[ 4 ]; // RESID
	unsigned long          m_status;
	class CKeyTableEntry * m_pKeyEntry;
	unsigned long          m_nSize;
	unsigned long          m_nRefs;
};

C_ASSERT( sizeof( CRes ) == 0x38 );

class CResNDB : public CRes
{

public:

	unsigned long   m_Data0;
	unsigned long   m_NDBSize;
	void          * m_NDBPtr;

};

C_ASSERT( sizeof( CResNDB ) == 0x44 );
C_ASSERT( offsetof( CResNDB, m_NDBSize ) == 0x3C );
C_ASSERT( offsetof( CResNDB, m_NDBPtr ) == 0x40 );

class CVirtualMachineDebugLoader
{

public:

	void          * Vtable;
	unsigned long   Data0;
	CResNDB       * NDB;
	unsigned long   Data[ 9 ];

	inline CVirtualMachineDebugLoader( )
	{
		memset( this, 0, sizeof( *this ) );
	}

	BOOL DemandDebugInfo( CExoString * FileName );
	BOOL ReleaseDebugInfo( VOID );

	inline void * GetNDBPtr( )
	{
		return NDB->m_NDBPtr;
	}

	inline unsigned long GetNDBSize( )
	{
		return NDB->m_NDBSize;
	}

};

C_ASSERT( sizeof( CVirtualMachineDebugLoader ) == 0x30 );
C_ASSERT( offsetof( CVirtualMachineDebugLoader, NDB ) == 0x8 );

class CVirtualMachine;
class CVirtualMachineCache;

class CVirtualMachineStack
{

public:

	enum STACK_TYPE
	{
		
		ST_INTEGER     = 0x03,
		ST_FLOAT       = 0x04,
		ST_STRING      = 0x05,
		ST_OBJECT      = 0x06,

		ST_ENGINE_0    = 0x10,
		ST_ENGINE_9    = 0x19,

		ST_LAST
	};

	inline
	int
	GetTopOfStackType(
		) const
	{
		if (m_nStackPointer == 0)
			return -1;
		else
			return m_pchStackTypes[ m_nStackPointer - 1 ];
	}

	inline
	bool
	CheckVectorOnTopOfStack(
		) const
	{
		if (m_nStackPointer < 3)
			return false;

		if (m_pchStackTypes[ m_nStackPointer - 1 ] != ST_FLOAT ||
		    m_pchStackTypes[ m_nStackPointer - 2 ] != ST_FLOAT ||
		    m_pchStackTypes[ m_nStackPointer - 3 ] != ST_FLOAT)
		{
			return false;
		}

		return true;
	}

	inline
	int
	GetCurrentSP(
		) const
	{
		return m_nStackPointer;
	}

	inline
	int
	GetCurrentBP(
		) const
	{
		return m_nBasePointer;
	}

	inline
	void
	SetCurrentBP(
		__in int BP
		)
	{
		m_nBasePointer = BP;
	}

	inline
	void
	SaveStack(
		__in INWScriptStack * Stack,
		__in int BPSaveBytes,
		__in int SPSaveBytes,
		__in int SPSaveOffset = 0
		)
	{
		int CellsToCopy;
		int CurBP;
		int CurSP;
		int SrcOffset;

		CurBP = GetCurrentBP( );
		CurSP = GetCurrentSP( );

		//
		// Copy stack cells relative to BP first.
		//

		SrcOffset   = (CurBP - BPSaveBytes);
		CellsToCopy = BPSaveBytes;

		AppendStackContentsToStack(
			Stack,
			SrcOffset,
			CellsToCopy);

		Stack->StackPushInt( CurBP * 4 );

		//
		// Now copy the SP-relative cells.
		//

		SrcOffset   = ((CurSP + SPSaveOffset) - SPSaveBytes);
		CellsToCopy = SPSaveBytes;

		AppendStackContentsToStack(
			Stack,
			SrcOffset,
			CellsToCopy);

		//
		// All done.
		//

		return;
	}

	inline
	void
	AppendStackContentsToStack(
		__in INWScriptStack * DestStack,
		__in int SrcOffset,
		__in int CellsToCopy
		)
	{
		//
		// Now perform the copy, one cell at a time.  The destination cells are
		// pushed onto the stack in turn.
		//

		for (int i = 0; i < CellsToCopy; i += 1)
		{
			int Type;

			Type = m_pchStackTypes[ SrcOffset + i ];

			if (Type == ST_INTEGER)
			{
				DestStack->StackPushInt( m_pStackNodes[ SrcOffset + i ] );
			}
			else if (Type == ST_FLOAT)
			{
				DestStack->StackPushFloat( *(float *) &m_pStackNodes[ SrcOffset + i ] );
			}
			else if (Type == ST_STRING)
			{
				CExoString * ExoStr = (CExoString *) (ULONG_PTR) (ULONG) m_pStackNodes[ SrcOffset + i ];

				if (ExoStr->m_nBufferLength == 0)
					DestStack->StackPushString( "" );
				else
					DestStack->StackPushString( ExoStr->m_sString );
			}
			else if (Type == ST_OBJECT)
			{
				DestStack->StackPushObjectId( *(NWN::OBJECTID *) &m_pStackNodes[ SrcOffset + i ] );
			}
			else if ((Type >= ST_ENGINE_0) &&
			         (Type <= ST_ENGINE_9))
			{
				void * Representation = (void *) (ULONG_PTR) (ULONG) m_pStackNodes[ SrcOffset + i ];

				WrapAndPushEngineStructure(
					DestStack,
					Representation,
					(ENGINE_STRUCTURE_TYPE) (Type - ST_ENGINE_0));
			}
			else
			{
				throw std::runtime_error( "bad stack type on CVirtualMachineStack" );
			}
		}
	}

	void
	WrapAndPushEngineStructure(
		__in INWScriptStack * Stack,
		__in void * Representation,
		__in ENGINE_STRUCTURE_TYPE EngineType
		);

	int                      m_nStackPointer;
	int                      m_nBasePointer;
	int                      m_nTotalSize;
	char                   * m_pchStackTypes;
	int                    * m_pStackNodes;
	CVirtualMachine        * m_pVMachine;

};

C_ASSERT( sizeof( CVirtualMachineStack ) == 0x18 );

class CVirtualMachineScript
{

public:

	CVirtualMachineStack    * m_pStack;
	int                       m_nStackSize;
	int                       m_nInstructPtr;
	int                       m_nSecondaryInstructPtr;
	char                    * m_pCode;
	int                       m_nCodeSize;
	CExoString                m_sScriptName;
	int                       m_nLoadedFromSave;
	BOOL                      m_bIsCutsceneAction;

};

C_ASSERT( sizeof( CVirtualMachineScript ) == 0x28 );

class CRes;
class CResNCS;

class CVirtualMachineFile
{

public:

	void                    * __VFN__table;
	BOOL                      m_bAutoRequest;
	CResNCS                 * m_pRes;
	NWN::ResRef32             m_cResRef;

};

C_ASSERT( sizeof( CVirtualMachineFile ) == 0x2c );

class CScriptParameterWrapper
{

public:

	enum ParameterType
	{
		PT_INT,
		PT_FLOAT,
		PT_STRING,
		PT_OBJECTTAG,
		PT_OBJECT,
		PT_UNKNOWN
	};

	void                    * __VFN__table;

	union
	{
		int                    m_iIntParameter;
		float                  m_fFloatParameter;
		NWN::OBJECTID          m_oidObjectParameter;
	};

	CExoString                m_cStringParameter;
	ParameterType             m_eType;
};

C_ASSERT( sizeof( CScriptParameterWrapper ) == 0x14 );

template< typename T >
class CExoArrayList
{

public:

	T                       * element;
	int                       num;
	int                       array_size;

};

C_ASSERT( sizeof( CExoArrayList< CScriptParameterWrapper > ) == 0x0c );

class CVirtualMachine
{

public:

	//
	// Called when the server's CVirtualMachine::ExecuteCode is called.
	//

	SCRIPT_STATUS
	__thiscall
	ExecuteCode(
		__inout int * PC,
		__inout_ecount( CodeSize ) unsigned char * InstructionStream,
		__in int CodeSize,
		__in struct CVirtualMachineDebuggingContext * DebugContext
		);

	inline
	static
	void *
	GetAddressOf_ExecuteCode(
		)
	{
		union
		{
			SCRIPT_STATUS
			(__thiscall CVirtualMachine::*ExecuteCode)(
				__inout int * PC,
				__inout_ecount( CodeSize ) unsigned char * InstructionStream,
				__in int CodeSize,
				__in struct CVirtualMachineDebuggingContext * DebugContext
				);
			void * RawPtr;
		} Ptr;
		
		C_ASSERT( sizeof( Ptr ) == sizeof( Ptr.RawPtr ) );

		Ptr.ExecuteCode = &CVirtualMachine::ExecuteCode;
		return Ptr.RawPtr;
	}

	//
	// Stack helpers.
	//

	inline
	bool
	StackPopInteger(
		__out int * Value
		)
	{
		union
		{
			bool
			(__thiscall CVirtualMachine::*StackPopInteger)(
				__out int * Value
				);
			void * RawPtr;
		} Ptr;

		C_ASSERT( sizeof( Ptr ) == sizeof( Ptr.RawPtr ) );

		if (GetTopOfStackType( ) != CVirtualMachineStack::ST_INTEGER)
			return false;

		Ptr.RawPtr = (void *) OFFS_VM_StackPopInteger;

		return (this->*Ptr.StackPopInteger)( Value );
	}

	inline
	bool
	StackPopFloat(
		__out float * Value
		)
	{
		union
		{
			bool
			(__thiscall CVirtualMachine::*StackPopFloat)(
				__out float * Value
				);
			void * RawPtr;
		} Ptr;

		C_ASSERT( sizeof( Ptr ) == sizeof( Ptr.RawPtr ) );

		if (GetTopOfStackType( ) != CVirtualMachineStack::ST_FLOAT)
			return false;

		Ptr.RawPtr = (void *) OFFS_VM_StackPopFloat;

		return (this->*Ptr.StackPopFloat)( Value );
	}

	inline
	bool
	StackPopString(
		__out std::string * Value
		)
	{
		union
		{
			bool
			(__thiscall CVirtualMachine::*StackPopString)(
				__inout CExoString * Value
				);
			void * RawPtr;
		} Ptr;

		C_ASSERT( sizeof( Ptr ) == sizeof( Ptr.RawPtr ) );

		if (GetTopOfStackType( ) != CVirtualMachineStack::ST_STRING)
			return false;

		Ptr.RawPtr = (void *) OFFS_VM_StackPopString;

		CExoString ExoStr;

		if (!(this->*Ptr.StackPopString)( &ExoStr ))
			return false;

		try
		{
			size_t CpLen = (size_t) ExoStr.m_nBufferLength;

			//
			// Except for the empty string, the buffer length of a CExoString
			// includes the null terminator, which we don't want to include in the
			// body of the internal string.
			//

			while ((CpLen > 0) && (ExoStr.m_sString[ CpLen - 1 ] == '\0'))
				CpLen -= 1;

			*Value = std::string( ExoStr.m_sString, CpLen );
		}
		catch (std::exception)
		{
			return false;
		}

		return true;
	}

	inline
	bool
	StackPopString(
		__out INWScriptStack::NeutralString * Value
		)
	{
		union
		{
			bool
			(__thiscall CVirtualMachine::*StackPopString)(
				__inout CExoString * Value
				);
			void * RawPtr;
		} Ptr;

		C_ASSERT( sizeof( Ptr ) == sizeof( Ptr.RawPtr ) );

		if (GetTopOfStackType( ) != CVirtualMachineStack::ST_STRING)
			return false;

		Ptr.RawPtr = (void *) OFFS_VM_StackPopString;

		CExoString ExoStr;

		if (!(this->*Ptr.StackPopString)( &ExoStr ))
			return false;

		try
		{
			Value->second = (size_t) ExoStr.m_nBufferLength;
			Value->first  = NULL;

			//
			// Except for the empty string, the buffer length of a CExoString
			// includes the null terminator, which we don't want to include in the
			// body of the internal string.
			//

			while ((Value->second > 0) && (ExoStr.m_sString[ Value->second - 1 ] == '\0'))
				Value->second -= 1;

			if (Value->second != 0)
			{
				Value->first = (char *) NWScriptStack::AllocNeutral( Value->second );

				memcpy( Value->first, ExoStr.m_sString, Value->second );
			}
			else
			{
				Value->first = NULL;
			}
		}
		catch (std::exception)
		{
			return false;
		}

		return true;
	}

	inline
	bool
	StackPopVector(
		__out NWN::Vector3 * Value
		)
	{
		union
		{
			bool
			(__thiscall CVirtualMachine::*StackPopVector)(
				__out NWN::Vector3 * Value
				);
			void * RawPtr;
		} Ptr;

		C_ASSERT( sizeof( Ptr ) == sizeof( Ptr.RawPtr ) );

		if (!m_cRunTimeStack.CheckVectorOnTopOfStack( ))
			return false;

		Ptr.RawPtr = (void *) OFFS_VM_StackPopVector;

		return (this->*Ptr.StackPopVector)( Value );
	}

	inline
	bool
	StackPopObject(
		__out NWN::OBJECTID * Value
		)
	{
		union
		{
			bool
			(__thiscall CVirtualMachine::*StackPopObject)(
				__out NWN::OBJECTID * Value
				);
			void * RawPtr;
		} Ptr;

		C_ASSERT( sizeof( Ptr ) == sizeof( Ptr.RawPtr ) );

		if (GetTopOfStackType( ) != CVirtualMachineStack::ST_OBJECT)
			return false;

		Ptr.RawPtr = (void *) OFFS_VM_StackPopObject;

		return (this->*Ptr.StackPopObject)( Value );
	}

	inline
	bool
	StackPopEngineStructure(
		__in ENGINE_STRUCTURE_TYPE EType,
		__out void * * Value
		)
	{
		union
		{
			bool
			(__thiscall CVirtualMachine::*StackPopEngineStructure)(
				__in ENGINE_STRUCTURE_TYPE EType,
				__out void * * Value
				);
			void * RawPtr;
		} Ptr;

		C_ASSERT( sizeof( Ptr ) == sizeof( Ptr.RawPtr ) );

		if (GetTopOfStackType( ) != (int) EType + CVirtualMachineStack::ST_ENGINE_0)
			return false;

		Ptr.RawPtr = (void *) OFFS_VM_StackPopEngineStructure;

		return (this->*Ptr.StackPopEngineStructure)( EType, Value );
	}


	inline
	bool
	StackPushInteger(
		__in int Value
		)
	{
		union
		{
			bool
			(__thiscall CVirtualMachine::*StackPushInteger)(
				__in int Value
				);
			void * RawPtr;
		} Ptr;

		C_ASSERT( sizeof( Ptr ) == sizeof( Ptr.RawPtr ) );

		Ptr.RawPtr = (void *) OFFS_VM_StackPushInteger;

		return (this->*Ptr.StackPushInteger)( Value );
	}

	inline
	bool
	StackPushFloat(
		__in float Value
		)
	{
		union
		{
			bool
			(__thiscall CVirtualMachine::*StackPushFloat)(
				__in float Value
				);
			void * RawPtr;
		} Ptr;

		C_ASSERT( sizeof( Ptr ) == sizeof( Ptr.RawPtr ) );

		Ptr.RawPtr = (void *) OFFS_VM_StackPushFloat;

		return (this->*Ptr.StackPushFloat)( Value );
	}

	inline
	bool
	StackPushString(
		__in const std::string & Value
		)
	{
		union
		{
			bool
			(__thiscall CVirtualMachine::*StackPushString)(
				__inout CExoString * Value
				);
			void * RawPtr;
		} Ptr;

		C_ASSERT( sizeof( Ptr ) == sizeof( Ptr.RawPtr ) );

		Ptr.RawPtr = (void *) OFFS_VM_StackPushString;

		CExoString ExoStr;

		if (Value.empty( ))
		{
			ExoStr.m_sString       = NULL;
			ExoStr.m_nBufferLength = 0;
		}
		else
		{
			ExoStr.m_sString = Value.c_str( );
			ExoStr.m_nBufferLength = (int) Value.size( ) + 1;
		}

		if (!(this->*Ptr.StackPushString)( &ExoStr ))
		{
			ExoStr.ReleaseOwnership( );
			return false;
		}

		ExoStr.ReleaseOwnership( );

		return true;
	}

	inline
	bool
	StackPushVector(
		__in const NWN::Vector3 * Value
		)
	{
		union
		{
			bool
			(__thiscall CVirtualMachine::*StackPushVector)(
				__in const NWN::Vector3 * Value
				);
			void * RawPtr;
		} Ptr;

		C_ASSERT( sizeof( Ptr ) == sizeof( Ptr.RawPtr ) );

		Ptr.RawPtr = (void *) OFFS_VM_StackPushVector;

		return (this->*Ptr.StackPushVector)( Value );
	}

	inline
	bool
	StackPushObject(
		__in NWN::OBJECTID Value
		)
	{
		union
		{
			bool
			(__thiscall CVirtualMachine::*StackPushObject)(
				__in NWN::OBJECTID Value
				);
			void * RawPtr;
		} Ptr;

		C_ASSERT( sizeof( Ptr ) == sizeof( Ptr.RawPtr ) );

		Ptr.RawPtr = (void *) OFFS_VM_StackPushObject;

		return (this->*Ptr.StackPushObject)( Value );
	}

	inline
	bool
	StackPushEngineStructure(
		__in ENGINE_STRUCTURE_TYPE EType,
		__in const void * Value
		)
	{
		union
		{
			bool
			(__thiscall CVirtualMachine::*StackPushEngineStructure)(
				__in ENGINE_STRUCTURE_TYPE EType,
				__in const void * Value
				);
			void * RawPtr;
		} Ptr;

		C_ASSERT( sizeof( Ptr ) == sizeof( Ptr.RawPtr ) );

		Ptr.RawPtr = (void *) OFFS_VM_StackPushEngineStructure;

		return (this->*Ptr.StackPushEngineStructure)( EType, Value );
	}

	inline
	int
	GetTopOfStackType(
		) const
	{
		return m_cRunTimeStack.GetTopOfStackType( );
	}

	inline
	int
	GetCurrentSP(
		) const
	{
		return m_cRunTimeStack.GetCurrentSP( );
	}

	inline
	int
	GetCurrentBP(
		) const
	{
		return m_cRunTimeStack.GetCurrentBP( );
	}

	inline
	void
	SetCurrentBP(
		__in int BP
		)
	{
		m_cRunTimeStack.SetCurrentBP( BP );
	}

	inline
	void
	SaveStack(
		__in INWScriptStack * Stack,
		__in int BPSaveBytes,
		__in int SPSaveBytes,
		__in int SPSaveOffset = 0
		)
	{
		return m_cRunTimeStack.SaveStack(
			Stack,
			BPSaveBytes,
			SPSaveBytes,
			SPSaveOffset);
	}

	inline
	const NWN::ResRef32 *
	GetScriptResRef(
		) const
	{
		return &m_cVMFile.m_cResRef;
	}

	inline
	const char *
	GetScriptName(
		) const
	{
		return m_pVirtualMachineScript[ m_nRecursionLevel ].m_sScriptName.m_sString;
	}

	inline
	NWN::OBJECTID
	GetCurrentActionObjectSelf(
		) const
	{
		if (!m_bValidObjectRunScript[ m_nRecursionLevel ])
			return NWN::INVALIDOBJID;
		else
			return m_oidObjectRunScript[ m_nRecursionLevel ];
	}

	inline
	void
	SetScriptSituationState(
		__in int ScriptSituationPC,
		__in int ScriptSituationSPSave,
		__in int ScriptSituationBPSave
		)
	{
		m_nSecondaryInstructionPointer = ScriptSituationPC;
		m_nStackSizeToSave             = ScriptSituationSPSave * 4;
		m_nBaseStackSizeToSave         = ScriptSituationBPSave * 4;
	}

	inline
	void
	SetScriptReturnCode(
		__in int ReturnCode
		)
	{
		m_nReturnValueParameterType = CVirtualMachineStack::ST_INTEGER;
		m_pReturnValue              = (void *) ReturnCode;

		StackPushInteger( ReturnCode );
	}

	inline
	void
	MarkCleanScriptReturn(
		)
	{
		m_nInstructPtrLevel -= 1;
	}

	CExoArrayList< CScriptParameterWrapper > *
	GetScriptParameters(
		)
	{
		return m_pExternalParameters;
	}

	inline
	CVirtualMachineCmdImplementer *
	GetCommandImplementer(
		)
	{
		return m_pCmdImplementer;
	}

	enum { NUM_NESTED_SCRIPTS = 8 };
	enum { MAX_CALL_DEPTH = 128 };

	int                                            m_nReturnValueParameterType;
	void                                         * m_pReturnValue;
	int                                            m_nInstructionsExecuted;
	int                                            m_nRecursionLevel;
	CVirtualMachineScript                          m_pVirtualMachineScript[ NUM_NESTED_SCRIPTS ]; // Note, not actually a pointer despite the name
	bool                                           m_bValidObjectRunScript[ NUM_NESTED_SCRIPTS ];
	NWN::OBJECTID                                  m_oidObjectRunScript[ NUM_NESTED_SCRIPTS ];
	CVirtualMachineStack                           m_cRunTimeStack;
	int                                            m_nInstructPtrLevel; // Return stack pointer in active script
	int                                            m_pnRunTimeInstructPtr[ MAX_CALL_DEPTH ]; // Return stack (note, not actually a pointer despite the name)
	int                                            m_nSecondaryInstructionPointer;  // OP_SAVE_STATE/OP_SAVE_STATEALL resume PC
	int                                            m_nStackSizeToSave; // OP_SAVE_STATE/OP_SAVE_STATEALL locals to save
	int                                            m_nBaseStackSizeToSave; // OP_SAVE_STATE/OP_SAVE_STATEALL globals to save
	CVirtualMachineCmdImplementer                * m_pCmdImplementer;
	BOOL                                           m_bDebugGUIRequired;
	BOOL                                           m_bDebuggerSpawned;
	CVirtualMachineFile                            m_cVMFile;
	CVirtualMachineCache                         * m_pCache;
	CExoArrayList< CScriptParameterWrapper >     * m_pExternalParameters;
	BOOL                                           m_bInCutsceneAction;
	CExoString                                     m_sSpeakerTag;
	CExoString                                     m_sLastScriptRun;
};

C_ASSERT( sizeof( CVirtualMachine ) == 0x3f4 );
C_ASSERT( FIELD_OFFSET( CVirtualMachine, m_oidObjectRunScript ) == 0x158 );
C_ASSERT( FIELD_OFFSET( CVirtualMachine, m_nInstructPtrLevel  ) == 0x190 );
C_ASSERT( FIELD_OFFSET( CVirtualMachine, m_pCmdImplementer    ) == 0x3a0 );

//
// Pick a relatively unlikely error to use if something goes wrong.
//

const int NWSCRIPT_ERR_FAKE_SHORTCUT_LOGICAL_OPERATION   = -643;
const int NWSCRIPT_ERR_FAKE_ABORT_SCRIPT                 = -645;
const int NWSCRIPT_ERR_IP_OUT_OF_CODE_SEGMENT            = -646;

}

#endif

