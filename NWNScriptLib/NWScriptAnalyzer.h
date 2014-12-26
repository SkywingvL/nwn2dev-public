/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptAnalyzer.h

Abstract:

	This module defines the NWScriptAnalyzer object.  The NWScript analyzer
	staticially analyzes a script in order to raise it to a higher level
	intermediate representation (IR).  The higher order IR allows the script
	program to be compiled into a different form (such as native code).

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTANALYZER_H
#define _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTANALYZER_H

#ifdef _MSC_VER
#pragma once
#endif

struct IDebugTextOut;

class NWScriptReader;


//
// Define the built-in action tables for use with the NWScriptAnalyzer.
//
// N.B.  The user may supply their own custom tables if desired.
//

enum { MAX_ACTION_ID_NWN2 = 1058 };
enum { MAX_ACTION_ID_NWN1 = 848 };

extern const NWACTION_DEFINITION NWActions_NWN2[ MAX_ACTION_ID_NWN2 ];
extern const NWACTION_DEFINITION NWActions_NWN1[ MAX_ACTION_ID_NWN1 ];

#include "NWScriptAnalyzerTypes.h"
#include "NWScriptLabel.h"
#include "NWScriptControlFlow.h"
#include "NWScriptVariable.h"
#include "NWScriptInstruction.h"
#include "NWScriptSubroutine.h"

namespace NWNScriptLib
{

//
// Define the NWScriptAnalyzer object, which supports analysis of a single
// script per object lifetime.  The raised IR of the script is stored within
// the NWScriptAnalyzer object.
//

class NWScriptAnalyzer
{

public:

	//
	// Define an exception class to use for errors in script files, 
	// containing the PC and, if applicable, the SP of the error. The SP 
	// can be either positive or negative; a positive value indicates a 
	// parameter, while a negative value indicates a return value.
	//

	class script_error : public std::runtime_error
	{
	public:
		inline
		script_error(
			__in PROGRAM_COUNTER PC,
			__in const char * What
			)
			: runtime_error( What ),
			m_PC( PC ),
			m_StackIndex( invalid_stack_index )
		{
		}

		inline
		script_error(
			__in PROGRAM_COUNTER PC,
			__in int StackIndex,
			__in const char * What
			)
			: runtime_error( What ),
			m_PC( PC ),
			m_StackIndex( StackIndex )
		{
		}

		script_error(
			__in PROGRAM_COUNTER PC,
			__in int StackIndex,
			__in const char * What,
			__in __format_string const char * Fmt,
			...
			)
			: runtime_error( What ),
			m_PC( PC ),
			m_StackIndex( StackIndex )
		{
			char    ErrorMsg[ 1024 ];
			va_list Ap;

			va_start( Ap, Fmt );

			StringCbVPrintfA(
				ErrorMsg,
				sizeof( ErrorMsg ),
				Fmt,
				Ap);

			va_end( Ap );

			m_SpecificInfo = ErrorMsg;
		}

		inline
		PROGRAM_COUNTER
		pc(
			) const
		{
			return m_PC;
		}

		inline
		int
		stack_index(
			) const
		{
			return m_StackIndex;
		}

		inline
		virtual
		const char *
		specific(
			) const
		{
			return m_SpecificInfo.c_str( );
		}

		// Some arbitrary unusual value
		static const int
		invalid_stack_index = (int)0x80000000;

	private:
		PROGRAM_COUNTER m_PC;
		int m_StackIndex;

		//
		// Contains more specific info than what( )
		//

		std::string m_SpecificInfo;
	};

	//
	// Create a script analyzer (with a specific action table array).
	//

	NWScriptAnalyzer(
		__in IDebugTextOut * TextOut,
		__in_ecount( ActionCount ) PCNWACTION_DEFINITION ActionDefs,
		__in NWSCRIPT_ACTION ActionCount
		);

	~NWScriptAnalyzer(
		);

	//
	// Define analysis flags.
	//

	typedef enum _ANALYZE_FLAGS
	{
		//
		// Only the program structure (subroutine list and parameter / return
		// sizes) should be discovered.
		//

		AF_STRUCTURE_ONLY    = 0x00000001,

		//
		// Generate only unoptimized IR (i.e. skip the postprocessing pass).
		//

		AF_NO_OPTIMIZATIONS  = 0x00000002,

		LAST_ANALYZE_FLAG
	} ANALYZE_FLAGS, * PANALYZE_FLAGS;

	typedef const enum _ANALYZE_FLAGS * PCANALYZE_FLAGS;

	//
	// Define the variable value type capable of holding any value that
	// an entry in the NWScript stack can hold. For our purposes, this is 
	// simply used for constants. Automatically frees its string on dstruct.
	//

	typedef struct _VARIABLE_VALUE
	{
		NWACTION_TYPE Type;

		union
		{
			LONG           Int;
			float          Float;
			std::string  * StringPtr;
			STACK_POINTER  StackPtr;
			UINT           Object;
			uintptr_t      RawValue;
		};
	} VARIABLE_VALUE, * PVARIABLE_VALUE;

	typedef std::map< Variable *, VARIABLE_VALUE > VariableValueMap;

	//
	// Determine whether a script is a platform native script, which
	// encapsulates platform code and not NWScript instructions.
	//

	bool
	IsPlatformNativeScript(
		__in NWScriptReader * Script,
		__in const char * PlatformSignature,
		__out PROGRAM_COUNTER & PlatformBinaryOffset,
		__out size_t & PlatformBinarySize
		);

	//
	// Read the contents of a platform native script.
	//

	void
	ReadPlatformNativeScript(
		__in NWScriptReader * Script,
		__in PROGRAM_COUNTER PlatformBinaryOffset,
		__in_bcount( PlatformBinarySize) void * PlatformBinary,
		__in size_t PlatformBinarySize
		);

	//
	// Analyze a script in order to raise it into the higher level IR.
	//
	// N.B.  It is possible to, by hand, construct a script program that cannot
	//       be raised to the higher level IR.  Such a hand-built program must
	//       be executed by the script VM instead of compiled by the JIT
	//       implementation.
	//

	void
	Analyze(
		__in NWScriptReader * Script,
		__in unsigned long Flags = 0
		);

	//
	// Return the list of subroutines.  The first is the program entry point.
	//
	// N.B.  #loader and #globals are not included.
	//

	inline
	const SubroutinePtrVec &
	GetSubroutines(
		) const
	{
		return m_Subroutines;
	}

	//
	// Return the address of #loader.
	//

	inline
	PROGRAM_COUNTER
	GetLoaderPC(
		) const
	{
		return m_LoaderPC;
	}

	//
	// Return the address of #globals.  The value may be -1 if there is no
	// #globals present.
	//

	inline
	PROGRAM_COUNTER
	GetGlobalsPC(
		) const
	{
		return m_GlobalsPC;
	}

	//
	// Returns the address of the main/StartingConditional
	//

	inline
	PROGRAM_COUNTER
	GetEntryPC(
		) const
	{
		return m_EntryPC;
	}

	//
	// Return the action table parameters that were used to analyze the
	// program.
	//

	inline
	void
	GetActionDefs(
		__out PCNWACTION_DEFINITION & ActionDefs,
		__out NWSCRIPT_ACTION & ActionCount
		) const
	{
		ActionDefs  = m_ActionDefs;
		ActionCount = m_ActionCount;
	}

	//
	// Return the name of the program (gathered from the NWScriptReaderPtr).
	//

	inline
	const std::string &
	GetProgramName(
		) const
	{
		return m_ProgramName;
	}

	//
	// This is a helper function that gets the list of referenced 
	// variables for the current instruction. This must be called 
	// after all the parameters of the instruction have been set up.
	//

	inline
	void
	GetInstructionVariableLists(
		__in const NWScriptInstruction & Instr,
		__out_opt VariableWeakPtrVec * ReadVars,
		__out_opt VariableWeakPtrVec * WriteVars
		) const;

	inline
	VARIABLE_VALUE &
	GetConstantValue(
		__in Variable * Var
		)
	{
		VariableValueMap::iterator ValueIt = m_ConstantValueMap.find( Var );
		assert( ValueIt != m_ConstantValueMap.end( ) );

		return ValueIt->second;
	}

	inline
	const VARIABLE_VALUE &
	GetConstantValue(
		__in Variable * Var
		) const
	{
		VariableValueMap::const_iterator ValueIt = m_ConstantValueMap.find( Var );
		assert( ValueIt != m_ConstantValueMap.end( ) );

		return ValueIt->second;
	}

	//
	// Display the contents of the IR to the debugger console.
	//

	inline
	void
	DisplayIR(
		)
	{
		PrintIR( );
	}


private:

	//
	// Define an analysis queue entry, used to discover the structure
	// of a function.
	//

	struct AnalysisQueueEntry
	{
		//
		// Define the PC value (absolute but within the subroutine) that
		// analysis is to be resumed at.
		//

		PROGRAM_COUNTER   PC;

		//
		// Define the SP value (relative to the base of the subroutine's frame,
		// 0) that analysis is to be resumed at.  This is the SP value before
		// taking into the side effects of the instruction at PC.
		//

		STACK_POINTER     SP;

		//
		// Define the control flow being operated on.
		//

		ControlFlowPtr    Flow;

		//
		// Define the subroutine descriptor of the subroutine that is being
		// analyzed.
		//

		Subroutine      * Function;

		//
		// Define the subroutine descriptor that is being blocked on before the
		// analysis of the current function can complete.
		//

		Subroutine      * BlockedOn;

		//
		// Define label flags for the label that transferred control to the
		// current address, such as Label::SCRIPT_SITUATION.
		//

		unsigned long     LabelFlags;
	};

	typedef std::vector< AnalysisQueueEntry > AnalysisQueueVec;

	//
	// Define a code analysis entry, used to analyze a section of program code.
	// code.
	//

	struct CodeAnalysisEntry
	{
		//
		// Define the PC value (absolute but within the subroutine) that
		// analysis is to be resumed at.
		//

		PROGRAM_COUNTER      PC;

		//
		// Define the SP value (relative to the base of the subroutine's frame,
		// 0) that analysis is to be resumed at.  This is the SP value before
		// taking into the side effects of the instruction at PC.
		//

		STACK_POINTER        SP;

		//
		// Define the subroutine descriptor of the subroutine that is being
		// analyzed.
		//

		Subroutine           * Function;

		//
		// Define the variable stack at the point of analysis.  The variable
		// stack represents the virtual stack, and indicies to the virtual
		// stack are indicies into the variable stack.
		//

		VariableWeakPtrVec   VarStack;

		//
		// Define label flags for the label that transferred control to the
		// current address, such as Label::SCRIPT_SITUATION.
		//

		unsigned long        LabelFlags;
	};

	typedef std::vector< CodeAnalysisEntry > CodeAnalysisQueueVec;

	typedef std::set< EXT_PROGRAM_COUNTER > IRPCSet;

	struct VariableData
	{
		Variable * Var;

		InstructionList::iterator CreateAddr;
		InstructionList::iterator DeleteAddr;
		InstructionList::iterator InitAddr;
		InstructionList::iterator AssignAddr;

		bool HaveCreateAddr;
		bool HaveDeleteAddr;
		bool HaveInitAddr;
		bool HaveAssignAddr;

		IRPCSet ReadAddrs;
		IRPCSet WriteAddrs;

		Variable * CopiedFrom;

		inline
		VariableData( )
		{
			Var = CopiedFrom = NULL;

			HaveCreateAddr = HaveDeleteAddr = HaveInitAddr = 
				HaveAssignAddr = false;
			CopiedFrom = NULL;
		}
	};

	typedef stdext::hash_map< Variable *, VariableData > VariableDataMap;
	typedef stdext::hash_multimap< Variable *, Variable * > VariableCopiedToMap;
	typedef std::pair< VariableCopiedToMap::iterator, 
		VariableCopiedToMap::iterator > CopiedToMapEqualRange;
	typedef std::vector< InstructionList::iterator > InstructionItVec;

	struct IRAnalysisData
	{
		VariableDataMap VarDataMap;
		VariableCopiedToMap VarCopiedToMap;

		VariableWeakPtrVec ReadVars;
		VariableWeakPtrVec WriteVars;

		InstructionItVec InstrsToErase;
	};

	//
	// We need to keep a copy of the stack at the end of each control flow
	// in a function so that we can perform merging of stack entries pushed 
	// at different places but represent the same variable.
	//

	typedef stdext::hash_map< PROGRAM_COUNTER, VariableWeakPtrVec > 
		FlowEndStackMap;

	//
	// For simplicity, we precompute the set of parameter and return types 
	// each ACTION call so that we can simply compare stack entries one at 
	// a time with the types of the call.
	//

	typedef std::vector< ReturnTypeList > ReturnValueListVec;
	typedef std::vector< ParameterList > ParameterListVec;

	//
	// For convenience and speed, we create a big hash table of all valid 
	// combinations of opcodes and parameter types. Note that if an entry 
	// for a given opcode has any members, the parameter type will be checked 
	// against the list of allowed types, even if none; e.g.the ACTION
	// opcode, which has a type byte of 0, would have a list containing only
	// 0 for its valid set.
	//

	typedef stdext::hash_set< UCHAR > OpcodeTypeSet;
	typedef stdext::hash_map< UCHAR, OpcodeTypeSet > ValidOpcodeTypeMap;
	//
	// Decode an instruction, returning the opcode data and the length.
	//

	static
	ULONG
	Disassemble(
		__in NWScriptReader * Script,
		__out UCHAR & Opcode,
		__out UCHAR & TypeOpcode,
		__out ULONG & PCOffset
		);

	//
	// Return instruction names for opcode and type opcodes.
	//

	static
	void
	GetInstructionNames(
		__in UCHAR Opcode,
		__in UCHAR TypeOpcode,
		__deref __out const char * * OpcodeName,
		__deref __out const char * * TypeOpcodeName
		);

	//
	// Analyze #loader and #globals.
	//

	PROGRAM_COUNTER
	AnalyzeLoader(
		__in NWScriptReader * Script
		);

	//
	// Discover the basic structure of all subroutines in the script program,
	// including counts of parameters and return values.
	//

	void
	AnalyzeSubroutineStructure(
		__in AnalysisQueueEntry Entry,
		__in NWScriptReader * Script
		);

	//
	// Generate the high level IR (typed variables, instructions, etc).
	//

	void
	AnalyzeSubroutineCode(
		__in NWScriptReader * Script
		);

	void
	AnalyzeSubroutineCode(
		__in NWScriptReader * Script,
		__in CodeAnalysisEntry & Entry
		);

	//
	// Perform optimization and other processing on the IR generated by 
	// AnalyzeSubroutineCode
	//

	static
	bool
	IsMCVarInDoubleCreatePair(
		__in VariableData & VarData
		);

	void
	PostProcessIR(
		__in bool Optimize = true
		);

	void
	PostProcessIR(
		__in NWScriptControlFlow & Flow,
		__in IRAnalysisData & Data,
		__in bool Optimize = true
		);

	void
	AppendVarString(
		__in char *String,
		__in size_t & StringLen,
		__in size_t BufferLen,
		__in Variable * Var,
		__in bool HasComma = false
		)
	{
		Variable * HeadVar = Var->GetHeadVariable( );

		const char *Class;
		switch (HeadVar->GetClass( ))
		{
		case Variable::Global:
			Class = "global ";
			break;
		case Variable::Constant:
			Class = "const ";
			break;
		case Variable::Parameter:
			Class = "param ";
			break;
		case Variable::ReturnValue:
			Class = "retval ";
			break;
		default:
			Class = "";
			break;
		}

		StringLen += sprintf_s(String + StringLen, 
			BufferLen - StringLen,
			"%s%p ( %s%s%s%s%s%s%s )",
			HasComma ? ", " : "",
			HeadVar,
			Var->IsFlagSet( Variable::OptimizerEliminated ) ? "merged " : "",
			Class,
			HeadVar->IsFlagSet( Variable::MultiplyCreated ) ? "MC " : "",
			HeadVar->IsFlagSet( Variable::LocalToFlow ) ? "temp " : "",
			HeadVar->IsFlagSet( Variable::WriteOnly ) ? "writeonly " : "",
			HeadVar->IsFlagSet( Variable::SingleAssignment ) ? "SSA " : "",
			GetTypeName( HeadVar->GetType( ) ) );

		if (HeadVar->GetClass( ) == Variable::Constant)
		{
			const VARIABLE_VALUE & Value = GetConstantValue( HeadVar );

			switch (Value.Type)
			{

			case ACTIONTYPE_INT:
				StringLen += sprintf_s(String + StringLen, 
					BufferLen - StringLen,
					" [%d]",
					Value.Int);
				break;

			case ACTIONTYPE_FLOAT:
				StringLen += sprintf_s(String + StringLen,
					BufferLen - StringLen,
					" [%g]",
					Value.Float);
				break;

			case ACTIONTYPE_STRING:
				StringLen += sprintf_s(String + StringLen,
					BufferLen - StringLen,
					" [\"%s\"]",
					Value.StringPtr->c_str( ));
				break;

			case ACTIONTYPE_OBJECT:
				StringLen += sprintf_s(String + StringLen,
					BufferLen - StringLen,
					" [%08X]",
					Value.Object);
				break;

			}
		}
	}

	void
	PrintIR(
		);

	//
	// Analyze a control flow for an occurrance of a particular instruction,
	// returning the first instance (or INVALID_PC if none was found).  The
	// search terminates at an OP_RETN.  An exception is raised on unexpected
	// EOF.
	//

	PROGRAM_COUNTER
	FindInstructionInFlow(
		__in PROGRAM_COUNTER PC,
		__in NWScriptReader * Script,
		__in UCHAR ScanOpcode
		);

	//
	// Prepares for a control flow transition. If the target flow does not
	// already exist, creates a new flow and adds it to the set. If the 
	// target is inside an existing flow, the existing flow is split. Returns
	// true if the target flow has not yet been analyzed. Always returns the
	// corresponding flow.
	//

	bool
	PrepareNewControlFlow(
		__in AnalysisQueueEntry & Entry,
		__in PROGRAM_COUNTER FlowPC,
		__out ControlFlowPtr & TargetFlow,
		__out Label & TargetLabel
		);

	//
	// Looks for a pending analysis branch entry and switches to it, if any.
	// Returns true if one was found (else should move to next function).
	//

	bool
	LoadNextAnalysisLabel(
		__inout AnalysisQueueEntry & Entry,
		__in NWScriptReader * Script
		);

	//
	// Finds the next linear flow in a given function. This is necessary
	// to know because the current analysis must end at this boundary, if
	// not before (some other flow jumps to the boundary).
	//

	ControlFlowPtr
	FindSubseqControlFlow(
		__in AnalysisQueueEntry & Entry
		);

	//
	// Return a subroutine by address.
	//

	inline
	Subroutine *
	GetSubroutine(
		__in PROGRAM_COUNTER SubroutineAddress
		)
	{
		for (SubroutinePtrVec::iterator it = m_Subroutines.begin( );
		     it != m_Subroutines.end( );
		     ++it)
		{
			Subroutine * Sub = it->get( );

			if (Sub->GetAddress( ) != SubroutineAddress)
				continue;

			return Sub;
		}

		return NULL;
	}

	//
	// Return whether there are any analysis queue entries created for a
	// given subroutine.
	//

	inline
	AnalysisQueueEntry *
	GetSubroutineQueueEntry(
		__in PROGRAM_COUNTER SubroutineAddress
		)
	{
		for (AnalysisQueueVec::iterator it = m_AnalysisQueue.begin( );
		     it != m_AnalysisQueue.end( );
		     ++it)
		{
			if (it->Function->GetAddress( ) != SubroutineAddress)
				continue;

			return &*it;
		}

		return NULL;
	}

	//
	// Raise an std::runtime_error given a formatted string.
	//

	inline
	void
	ThrowError(
		__in __format_string const char * Fmt,
		...
		)
	{
		char    ErrorMsg[ 256 ];
		va_list Ap;

		va_start( Ap, Fmt );

		StringCbVPrintfA(
			ErrorMsg,
			sizeof( ErrorMsg ),
			Fmt,
			Ap);

		va_end( Ap );

		throw std::runtime_error( ErrorMsg );
	}

	//
	// Decodes the variable type given the instruction operand type. Returns 
	// the first type, and sets the SecondType parameter to the second type
	// if instruction is binary and SecondType is not NULL. Does not support
	// TYPE_BINARY_STRUCTSTRUCT and will throw.
	//

	static
	NWACTION_TYPE
	GetOperandType(
		__in UCHAR OperandType,
		__out_opt NWACTION_TYPE * SecondType = NULL
		);

	//
	// Return the size of a variable type (in terms of stack usage).
	//

	inline
	static
	STACK_POINTER
	GetTypeSize(
		__in NWACTION_TYPE Type
		)
	{
		switch (Type)
		{

		case ACTIONTYPE_VOID:
		case ACTIONTYPE_ACTION:
			return 0 * CELL_SIZE;

		case ACTIONTYPE_VECTOR:
			return 3 * CELL_SIZE;

		default:
			return 1 * CELL_SIZE;

		}
	}

	//
	// Gets the human-friendly name of native action types
	//

	inline
	static
	const char *
	GetTypeName(
		__in NWACTION_TYPE Type
		)
	{
		static const char *TypeNames[] =
		{
			"void",
			"int",
			"float",
			"string",
			"object",
			"vector",
			"action",
			"effect",
			"event",
			"location",
			"talent",
			"itemproperty"
		};

		if (Type < sizeof( TypeNames ) / sizeof( TypeNames[ 0 ] ))
			return TypeNames[ Type ];
		else
			throw std::runtime_error( "invalid type ID" );
	}

	void
	CreateValidOpcodeTypeMap(
		);

	inline
	OpcodeTypeSet *
	_AddOpcodeTypesToMap(
		__in UCHAR Opcode,
		__in const UCHAR Types[],
		__in size_t NumTypes
		)
	{
		OpcodeTypeSet &Set = m_ValidOpcodeTypeMap[ Opcode ];

		for (size_t i = 0; i < NumTypes; i++)
			Set.insert( Types[ i ] );

		return &Set;
	}

	//
	// The purpose of this little spawn of evil is simply to allow us to 
	// supply a raw list of values, rather than having to specify the 
	// number of values as well (less convenient).
	//

#define AddOpcodeTypesToMap( SET_PTR, OPCODE, ... ) \
	{ \
		const UCHAR temp[] = __VA_ARGS__; \
		SET_PTR = _AddOpcodeTypesToMap( (NWACTION_TYPE) OPCODE, \
			temp, sizeof( temp ) / sizeof( temp[0] ) ); \
	}

	inline
	void
	CopyOpcodeTypes(
		__in UCHAR DestOpcode,
		__in OpcodeTypeSet *TypeSet
		)
	{
		m_ValidOpcodeTypeMap[ DestOpcode ] = *TypeSet;
	}

	inline
	void
	CheckOpcodeType(
		__in PROGRAM_COUNTER PC,
		__in UCHAR Opcode,
		__in UCHAR Type
		) const
	{
		ValidOpcodeTypeMap::const_iterator It = 
			m_ValidOpcodeTypeMap.find( Opcode );

		// If it doesn't have an entry, don't check it
		if (It == m_ValidOpcodeTypeMap.end( ) || It->second.size( ) == 0)
			return;

		// Is the combination valid?
		if (It->second.count( Type ) == 0)
		{
			throw script_error( PC, INVALID_SP, 
				"invalid opcode type value",
				"type %02X is not valid for opcode %02X",
				Type, Opcode );
		}
	}

	//
	// Maps a NWScript opcode to the core IR opcode. In instructions where 
	// temporary variables are created or deleted, the return opcode 
	// corresponds to the actual op, not the creation or deletion. This is 
	// mainly used in the analyzer where multiple NWScript opcodes result 
	// in the same IR behavior pattern with only the core opcode different.
	//
	// N.B. Some complex NWScript opcodes do not have corresponding IR 
	// opcodes. In this case, MapIROpcode will throw an error.
	//
	
	static
	NWScriptInstruction::INSTR
	MapIROpcode(
		__in unsigned NWScriptOpcode
		);

	//
	// Gets the human-friendly name of an IR opcode
	//

	inline
	static
	const char *
	GetIROpcodeName(
		__in NWScriptInstruction::INSTR Opcode
		)
	{
		static const char * OpcodeNames[] =
		{
			"CREATE", 
			"DELETE", 
			"INITIALIZE",
			"ASSIGN", 
			"JZ", 
			"JNZ",
			"JMP",
			"CALL", 
			"RETN", 
			"ACTION", 
			"SAVE_STATE", 
			"LOGAND", 
			"LOGOR",
			"INCOR",
			"EXCOR",
			"BOOLAND",
			"EQUAL",
			//"EQUALMULTI", 
			"NEQUAL", 
			//"NEQUALMULTI",
			"GEQ",
			"GT", 
			"LT", 
			"LEQ",
			"SHLEFT", 
			"SHRIGHT",
			"USHRIGHT", 
			"ADD",
			"SUB",
			"MUL",
			"DIV",
			"MOD",
			"NEG",
			"COMP", 
			"NOT",
			"INC",
			"DEC",
			"TEST"
		};

		if (Opcode < sizeof( OpcodeNames ) / sizeof( OpcodeNames[ 0 ] ))
			return OpcodeNames[Opcode];
		else
			throw std::runtime_error( "invalid opcode" );
	}

	//
	// Check a global variable access.
	//

	void
	CheckGlobalAccess(
		__in CodeAnalysisEntry & Entry,
		__in STACK_POINTER Offset,
		__in STACK_POINTER Size = CELL_SIZE
		) const
	{
		if ((Offset & CELL_UNALIGNED) ||
		    (Size & CELL_UNALIGNED))
			throw script_error( Entry.PC, "unaligned global access" );
		else if (Offset + Size > 0)
			throw script_error( Entry.PC, "positive global access" );
		else if (-Offset > 
			(STACK_POINTER) m_GlobalVariables.size( ) * CELL_SIZE)
			throw script_error( Entry.PC, 0, 
				"global access violation",
				"global offset of %X, total global size %X",
				Offset, m_GlobalVariables.size( ) * CELL_SIZE );
	}

	//
	// Access a global variable by SP offset within #globals.
	//
	// N.B. SP will always be negative, given relative to BP (the end of 
	// our global variable list)
	//

	inline
	Variable &
	GetGlobalVariable(
		__in STACK_POINTER SP
		)
	{
		STACK_POINTER Idx = (SP / CELL_SIZE);

		if (Idx >= 0 || size_t( -Idx ) > m_GlobalVariables.size( ))
			throw std::runtime_error( "illegal global variable SP reference" );

		return *m_GlobalVariables[ m_GlobalVariables.size( ) - size_t( -Idx ) ];
	}

	//
	// Check a local SP access.
	//

	static
	void
	CheckStackAccess(
		__in CodeAnalysisEntry & Entry,
		__in STACK_POINTER MinSP,
		__in STACK_POINTER Offset,
		__in STACK_POINTER Size
		)
	{
		if ((Offset & CELL_UNALIGNED) ||
		    (Size & CELL_UNALIGNED))
			throw script_error( Entry.PC, "unaligned stack access" );
		else if (Offset + Size > 0)
			throw script_error( Entry.PC, "positive stack access" );
		else if (Offset + Entry.SP < MinSP)
			throw script_error( Entry.PC, 
				Entry.SP / CELL_SIZE + 1, 
				"stack access violation",
				"stack offset of %X, effective stack size %X",
				Offset, Entry.SP - MinSP );
	}

	inline
	static
	void
	CheckStackAccess(
		__in CodeAnalysisEntry & Entry,
		__in STACK_POINTER MinSP,
		__in STACK_POINTER Size = CELL_SIZE
		)
	{
		CheckStackAccess( Entry, MinSP, -Size, Size );
	}

	//
	// Access a local variable.
	//

	inline
	Variable *
	GetLocalVariable(
		__in CodeAnalysisEntry & Entry,
		__in STACK_POINTER SP
		)
	{
		size_t Idx = (SP / CELL_SIZE);

		if (Idx >= Entry.VarStack.size( ))
			throw std::runtime_error( "illegal local variable SP reference" );

		return Entry.VarStack[ Idx ];
	}

	//
	// Create a local variable.
	//

	inline
	Variable *
	CreateLocal(
		__in CodeAnalysisEntry & Entry,
		__in NWScriptVariable::CLASS Class,
		__in_opt NWACTION_TYPE Type = ACTIONTYPE_VOID
		)
	{
		VariablePtr Var = new Variable( Entry.SP, Class, Type );

		Entry.Function->AddLocal( Var );
		Entry.VarStack.push_back( Var.get( ) );
		Entry.SP += CELL_SIZE;

		return Var.get( );
	}

	inline
	Variable *
	CreateLocal(
		__in CodeAnalysisEntry & Entry,
		__in InstructionList * IR,
		__in_opt InstructionList::iterator It,
		__in_opt NWACTION_TYPE Type = ACTIONTYPE_VOID,
		__in_opt NWScriptVariable::CLASS Class = NWScriptVariable::Local
		)
	{
		Variable * Var = CreateLocal( Entry, Class, Type );

		if (IR)
		{
			IR->insert( It, Instruction( 
				Entry.PC, Instruction::I_CREATE, NULL, Var ) );
		}

		return Var;
	}

	inline
	Variable *
	CreateLocal(
		__in CodeAnalysisEntry & Entry,
		__in_opt NWACTION_TYPE Type = ACTIONTYPE_VOID,
		__in_opt NWScriptVariable::CLASS Class = NWScriptVariable::Local
		)
	{
		return CreateLocal( Entry, Class, Type );
	}

	inline
	Variable *
	CreateLocal(
		__in CodeAnalysisEntry & Entry,
		__in InstructionList * IR,
		__in_opt NWACTION_TYPE Type = ACTIONTYPE_VOID,
		__in_opt NWScriptVariable::CLASS Class = NWScriptVariable::Local
		)
	{
		return CreateLocal( Entry, IR, IR->end( ), Type, Class );
	}

	inline
	Variable *
	DeleteTopLocal(
		__in CodeAnalysisEntry & Entry,
		__in_opt InstructionList * IR
		)
	{
		Variable * Var;

		if (Entry.VarStack.empty( ))
			throw std::runtime_error( "DeleteTopLocal: removing from empty VarStack" );
		
		Var = Entry.VarStack.back( );
		Entry.VarStack.pop_back( );
		Entry.SP -= CELL_SIZE;

		if (IR)
		{
			IR->push_back( Instruction( Entry.PC, Instruction::I_DELETE ) );
			IR->back( ).GetVar( 0 ) = Var;
		}

		return Var;
	}

	inline
	void
	DeleteTopLocals(
		__in CodeAnalysisEntry & Entry,
		__in STACK_POINTER TopSize,
		__in_opt InstructionList * IR
		)
	{
		for (STACK_POINTER i = 0; i < TopSize; i += CELL_SIZE)
			DeleteTopLocal( Entry, IR );
	}

	//
	// Define the debug text output interface.
	//

	mutable IDebugTextOut      * m_TextOut;

	//
	// Define the active action handler table, that can be used to propagate
	// types from action handlers.
	//

	PCNWACTION_DEFINITION        m_ActionDefs;
	NWSCRIPT_ACTION              m_ActionCount;

	//
	// Define the list of valid opcode + type combinations in programs
	//

	ValidOpcodeTypeMap           m_ValidOpcodeTypeMap;

	//
	// Define the name of the analyzed program.
	//

	std::string                  m_ProgramName;

	ParameterListVec             m_ActionParameters;

	//
	// Define the addresses of the #loader and #globals pseudo-routines (if
	// any such routines exist).  The program counter value of -1 is used to
	// indicate a not present routine.
	//

	PROGRAM_COUNTER              m_LoaderPC;   // #loader
	PROGRAM_COUNTER              m_GlobalsPC;  // #globals [optional]
	PROGRAM_COUNTER              m_EntryPC;    // Entry point routine

	//
	// Define the return value of the entry point routine.
	//

	NWACTION_TYPE                m_EntryReturnType;

	//
	// Define the discovered subroutine list.
	//

	SubroutinePtrVec             m_Subroutines;

	//
	// Define the global variable list.
	//
	// N.B. This list points to variables that are actual locals of #globals
	//

	VariableWeakPtrVec           m_GlobalVariables;

	//
	// Define the values of the constant variables
	//

	VariableValueMap             m_ConstantValueMap;

	//
	// Define the queue of functions to analyze (for program structure).
	//

	AnalysisQueueVec             m_AnalysisQueue;

};

//
// N.B.  This routine is (hackishly) placed in the header so that the JIT
//       backend emits a local representation of the routine.  This ensures
//       that the correct allocator is used for the vectors.
//

inline
void
NWScriptAnalyzer::GetInstructionVariableLists(
	__in const NWScriptInstruction & Instr,
	__out_opt NWNScriptLib::VariableWeakPtrVec * ReadVars,
	__out_opt NWNScriptLib::VariableWeakPtrVec * WriteVars
	) const
/*++

Routine Description:

	This routine retrieves the input- and output- variable lists that associate
	an IR instruction with its parameter and return value operands.

	N.B.  It is permissible for a variable to appear in both read and write
	      lists.

	Note that non-data instructions (such as I_CREATE) do not report read or
	write variable lists via this API.

Arguments:

	Instr - Supplies the instruction to query.

	ReadVars - Optionally receives the list of variables read by the IR
	           instruction.

	WriteVars - Optionally receives the list of variables written to by the IR
	            instruction.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, IR generation completed.

--*/
{
	//
	// Deal with simple instruction formats first
	//

	if (ReadVars)
	{
		switch (Instr.GetType( ))
		{
		// x = y op z instructions
		case NWScriptInstruction::I_LOGAND:
		case NWScriptInstruction::I_LOGOR:
		case NWScriptInstruction::I_INCOR:
		case NWScriptInstruction::I_EXCOR:
		case NWScriptInstruction::I_BOOLAND:
		case NWScriptInstruction::I_EQUAL:
		case NWScriptInstruction::I_NEQUAL:
		case NWScriptInstruction::I_GEQ:
		case NWScriptInstruction::I_GT:
		case NWScriptInstruction::I_LT:
		case NWScriptInstruction::I_LEQ:
		case NWScriptInstruction::I_SHLEFT:
		case NWScriptInstruction::I_SHRIGHT:
		case NWScriptInstruction::I_USHRIGHT:
		case NWScriptInstruction::I_ADD:
		case NWScriptInstruction::I_SUB:
		case NWScriptInstruction::I_MUL:
		case NWScriptInstruction::I_DIV:
		case NWScriptInstruction::I_MOD:
			ReadVars->push_back( Instr.GetVar( 1 ) );

			// Fallthrough

		// x = y instructions
		case NWScriptInstruction::I_ASSIGN:
		case NWScriptInstruction::I_NEG:
		case NWScriptInstruction::I_COMP:
		case NWScriptInstruction::I_NOT:
		case NWScriptInstruction::I_INC:
		case NWScriptInstruction::I_DEC:
		case NWScriptInstruction::I_TEST:
			ReadVars->push_back( Instr.GetVar( 0 ) );

			break;
		}
	}

	if (WriteVars)
	{
		switch (Instr.GetType( ))
		{
		// x = ... instructions
		case NWScriptInstruction::I_ASSIGN:
		case NWScriptInstruction::I_LOGAND:
		case NWScriptInstruction::I_LOGOR:
		case NWScriptInstruction::I_INCOR:
		case NWScriptInstruction::I_EXCOR:
		case NWScriptInstruction::I_BOOLAND:
		case NWScriptInstruction::I_EQUAL:
		case NWScriptInstruction::I_NEQUAL:
		case NWScriptInstruction::I_GEQ:
		case NWScriptInstruction::I_GT:
		case NWScriptInstruction::I_LT:
		case NWScriptInstruction::I_LEQ:
		case NWScriptInstruction::I_SHLEFT:
		case NWScriptInstruction::I_SHRIGHT:
		case NWScriptInstruction::I_USHRIGHT:
		case NWScriptInstruction::I_ADD:
		case NWScriptInstruction::I_SUB:
		case NWScriptInstruction::I_MUL:
		case NWScriptInstruction::I_DIV:
		case NWScriptInstruction::I_MOD:
		case NWScriptInstruction::I_NEG:
		case NWScriptInstruction::I_COMP:
		case NWScriptInstruction::I_NOT:
		case NWScriptInstruction::I_INC:
		case NWScriptInstruction::I_DEC:
		case NWScriptInstruction::I_INITIALIZE:
			WriteVars->push_back( Instr.GetResultVar( ) );

			break;
		}
	}

	// Now the complicated instructions
	switch (Instr.GetType( ))
	{
	case NWScriptInstruction::I_CALL:
	case NWScriptInstruction::I_ACTION:
		if (Instr.GetParamVarList( ))
		{
			size_t NumReturnValues;

			if (Instr.GetType( ) == NWScriptInstruction::I_CALL)
			{
				Subroutine * Sub = Instr.GetSubroutine( );

				NumReturnValues = Sub->GetReturnTypes( ).size( );
			}
			else
			{
				const NWACTION_DEFINITION & ActionDef = 
					m_ActionDefs[ Instr.GetActionIndex( ) ];

				NumReturnValues = size_t( 
					GetTypeSize( ActionDef.ReturnType ) / CELL_SIZE );
			}

			if (WriteVars)
			{
				for (size_t i = 0; i < NumReturnValues; i++)
					WriteVars->push_back( Instr.GetParamVarList( )->at( i ) );
			}
			if (ReadVars)
			{
				for (size_t i = NumReturnValues; 
					i < Instr.GetParamVarList( )->size( ); i++)
					ReadVars->push_back( Instr.GetParamVarList( )->at( i ) );
			}
		}

		break;

	case NWScriptInstruction::I_SAVE_STATE:
		// Uses the parameter list as input parameters
		if (ReadVars && Instr.GetParamVarList( ))
		{
			for (size_t i = 0; i < Instr.GetParamVarList( )->size( ); i++)
				ReadVars->push_back( Instr.GetParamVarList( )->at( i ) );
		}

		break;
	}
}

} // namespace NWNScriptLib

using NWNScriptLib::NWScriptAnalyzer;

#endif
