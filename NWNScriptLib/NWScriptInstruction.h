/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptInstruction.h

Abstract:

	This module defines the NWScriptInstruction object, which represents a high
	level intermediate representation (IR) instruction within an analyzed
	subroutine.

	N.B.  This module is used only by the analyzer subsystem.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTINSTRUCTION_H
#define _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTINSTRUCTION_H

#ifdef _MSC_VER
#pragma once
#endif

namespace NWNScriptLib
{
	class NWScriptSubroutine;

	class EXT_PROGRAM_COUNTER : public std::pair< PROGRAM_COUNTER, unsigned >
	{
	public:
		inline
		EXT_PROGRAM_COUNTER(
			PROGRAM_COUNTER PC,
			unsigned SeqIndex
			)
			: pair( PC, SeqIndex )
		{
		}

		inline
		bool 
		operator < (
			const EXT_PROGRAM_COUNTER & Other
			) const
		{
			if (first < Other.first)
				return true;
			else if (first == Other.first &&
				second < Other.second)
				return true;
			else
				return false;
		}
	};

	//
	// Define the representation of a script instruction.
	//

	class NWScriptInstruction
	{
	private:
		inline
		void
		CheckOperandIndex(
			__in size_t Index
			) const
		{
			if (Index >= sizeof( m_Operands.Values ) / 
				sizeof( m_Operands.Values[0] ))
				throw std::runtime_error( "invalid operand value index" );
		}

	public:

		typedef enum _INSTR
		{
			I_CREATE,       // Create variable with type (no value)
			I_DELETE,       // Delete variable
			I_INITIALIZE,   // Set variable to default value
			I_ASSIGN,       // Copy (assign) variables
			I_JZ,           // Jump if zero
			I_JNZ,          // Jump if not zero
			I_JMP,          // Jump unconditionally
			I_CALL,         // Call subroutine
			I_RETN,         // Return from subroutine
			I_ACTION,       // Call script action
			I_SAVE_STATE,   // Save state for script situation
			I_LOGAND,       // Logical AND (&&)
			I_LOGOR,        // Logical OR (||)
			I_INCOR,        // Bitwise OR (|)
			I_EXCOR,        // Bitwise XOR (^)
			I_BOOLAND,      // Bitwise AND (&)
			I_EQUAL,        // Compare (==)
			//I_EQUALMULTI,   // Compare (==) with multiple variables
			I_NEQUAL,       // Compare (!=)
			//I_NEQUALMULTI,  // Compare (!=) with multiple variables
			I_GEQ,          // Compare (>=)
			I_GT,           // Compare (>)
			I_LT,           // Compare (<)
			I_LEQ,          // Compare (<=)
			I_SHLEFT,       // Shift left (<<)
			I_SHRIGHT,      // Shift signed right (>>, SAR)
			I_USHRIGHT,     // Shift unsigned right (>>)
			I_ADD,          // Add (+), concatenate strings
			I_SUB,          // Subtract (-)
			I_MUL,          // Multiply (*)
			I_DIV,          // Divide (/)
			I_MOD,          // Modulus (%)
			I_NEG,          // Negation (-)
			I_COMP,         // Complement (~)
			I_NOT,          // Logical NOT (!)
			I_INC,          // Increment
			I_DEC,          // Decrement
			I_TEST,			// Set zero/not zero based on variable value

			LASTINSTR
		} INSTR, * PINSTR;

		typedef const enum _INSTR * PCINSTR;

		//
		// N.B. Need parameterless constructor for collections. Should never 
		// actually be used.
		//

		inline
		NWScriptInstruction(
			__in PROGRAM_COUNTER Address = INVALID_PC,
			__in INSTR Type = LASTINSTR
			)
		: m_Address( Address ),
		  m_SeqIndex( 0 ),
		  m_Type( Type )
		{
			m_Operands.Vars[ 0 ] = m_Operands.Vars[ 1 ] = NULL;
			m_ResultVar = NULL;
		}

		inline
		NWScriptInstruction(
			__in PROGRAM_COUNTER Address,
			__in INSTR Type,
			__in_opt Variable * Result,
			__in_opt Variable * First = NULL,
			__in_opt Variable * Second = NULL
			)
		: m_Address( Address ),
		  m_SeqIndex( 0 ),
		  m_Type( Type )
		{
			m_ResultVar = Result;
			m_Operands.Vars[ 0 ] = First;
			m_Operands.Vars[ 1 ] = Second;
		}

		inline
		PROGRAM_COUNTER
		GetAddress(
			) const
		{
			return m_Address;
		}

		inline
		INSTR
		GetType(
			) const
		{
			return m_Type;
		}

		inline
		unsigned
		GetSeqIndex(
			) const
		{
			return m_SeqIndex;
		}

		inline
		void
		SetSeqIndex(
			unsigned SeqIndex
			)
		{
			m_SeqIndex = SeqIndex;
		}

		inline
		EXT_PROGRAM_COUNTER
		GetExtAddress(
			) const
		{
			return EXT_PROGRAM_COUNTER( m_Address, m_SeqIndex );
		}

		//
		// Generic field accessors. These are used for explicitely 
		// accessing different fields by their actual type.
		//

		inline
		Variable *
		GetVar(
			__in size_t Index
			) const
		{
			CheckOperandIndex( Index );
			return m_Operands.Vars[ Index ];
		}

		inline
		Variable *&
		GetVar(
			__in size_t Index
			)
		{
			CheckOperandIndex( Index );
			return m_Operands.Vars[ Index ];
		}

		inline
		Label *
		GetLabel(
			__in size_t Index
			) const
		{
			CheckOperandIndex( Index );
			return m_Operands.Labels[ Index ];
		}

		inline
		Label *&
		GetLabel(
			__in size_t Index
			)
		{
			CheckOperandIndex( Index );
			return m_Operands.Labels[ Index ];
		}

		inline
		NWScriptSubroutine *
		GetSub(
			__in size_t Index
			) const
		{
			CheckOperandIndex( Index );
			return m_Operands.Subs[ Index ];
		}

		inline
		NWScriptSubroutine *&
		GetSub(
			__in size_t Index
			)
		{
			CheckOperandIndex( Index );
			return m_Operands.Subs[ Index ];
		}

		inline
		uintptr_t
		GetValue(
			__in size_t Index
			) const
		{
			CheckOperandIndex( Index );
			return m_Operands.Values[ Index ];
		}

		inline
		uintptr_t &
		GetValue(
			__in size_t Index
			)
		{
			CheckOperandIndex( Index );
			return m_Operands.Values[ Index];
		}

		inline
		Variable *
		GetResultVar(
			) const
		{
			return m_ResultVar;
		}

		inline
		void
		SetResultVar(
			__in Variable * Var
			)
		{
			m_ResultVar = Var;
		}

		inline
		const VariableWeakPtrVec *
		GetParamVarList(
			) const
		{
			return m_ParamVarList;
		}

		inline
		VariableWeakPtrVec *
		GetParamVarList(
			)
		{
			if (!m_ParamVarList)
				m_ParamVarList = new VariableWeakPtrVec();

			return m_ParamVarList;
		}

		//
		// Special-purpose field accessors. These are used for specific 
		// instructions that make irregular use of the fields.
		//

		inline
		Label *
		GetJumpTarget(
			) const
		{
			return m_Operands.Labels[ 0 ];
		}

		inline
		void
		SetJumpTarget(
			__in Label * Target
			)
		{
			m_Operands.Labels[ 0 ] = Target;
		}

		inline
		NWScriptVariable *
		GetConditionVariable(
			) const
		{
			return m_Operands.Vars[ 0 ];
		}

		inline
		void
		SetConditionVariable(
			__in NWScriptVariable * Var
			)
		{
			m_Operands.Vars[ 0 ] = Var;
		}

		inline
		NWScriptSubroutine *
		GetSubroutine(
			) const
		{
			return m_Operands.Subs[ 0 ];
		}

		inline
		void
		SetSubroutine(
			__in NWScriptSubroutine * Sub
			)
		{
			m_Operands.Subs[ 0 ] = Sub;
		}

		/*inline
		uintptr_t
		GetNumReturnValues(
			) const
		{
			return m_Operands.Values[ 1 ];
		}

		inline
		void
		SetNumReturnValues(
			__in uintptr_t NumValues
			)
		{
			m_Operands.Values[ 1 ] = NumValues;
		}*/

		inline
		uintptr_t
		GetActionIndex(
			) const
		{
			return m_Operands.Values[ 0 ];
		}

		inline
		void
		SetActionIndex(
			__in uintptr_t Index
			)
		{
			m_Operands.Values[ 0 ] = Index;
		}

		inline
		uintptr_t
		GetActionParameterCount(
			) const
		{
			return m_Operands.Values[ 1 ];
		}

		inline
		void
		SetActionParameterCount(
			__in uintptr_t Count
			)
		{
			m_Operands.Values[ 1 ] = Count;
		}

		//
		// In SAVE_STATE, both saved globals and locals are stored in 
		// m_ParamVarList. The globals come first, of this number. The 
		// locals come second, with number size( ) - GetStateNumGlobals( ).
		//

		inline
		uintptr_t
		GetStateNumGlobals(
			) const
		{
			return m_Operands.Values[ 1 ];
		}

		inline
		void
		SetStateNumGlobals(
			__in uintptr_t Num
			)
		{
			m_Operands.Values[ 1 ] = Num;
		}

	private:

		//
		// Define the address of the instruction.
		//

		PROGRAM_COUNTER m_Address;

		//
		// Define the sequence index of the instruction. This is used to 
		// distinguish multiple instructions that all have the same PC, 
		// which occurs when a single bytecode instruction is decomposed 
		// into multiple IR instructions.
		//

		unsigned        m_SeqIndex;

		//
		// Define the type of instruction.
		//

		INSTR           m_Type;

		//
		// Define the instruction operands.
		//

		union
		{
			Variable *	 Vars[2];
			Label *		 Labels[2];
			NWScriptSubroutine * Subs[2];
			uintptr_t	 Values[2];
		} m_Operands;

		//
		// Define the ternary operand (generally the result of a computation)
		// and friends. Most instructions only use m_ResultVar plus 
		// m_Operands, but CALLs, ACTIONs, and SAVE_STATES require additional
		// storage, so a vector is used to hold parameters and return values.
		// Note that in this case both return values and parameters are 
		// stored in m_ParamVarList, with return values first.
		// MULVF/MULFV/DIVVF and EQUALTT/NEQUALTT also will not fit into a 
		// three variable slots, but it's assumed these will be deconstructed
		// into separate IR instructions (~3 instructions each).
		//

		union
		{
			Variable *				m_ResultVar;
			VariableWeakPtrVec *	m_ParamVarList;
		};
	};

	typedef NWScriptInstruction Instruction;
	typedef std::list< Instruction > InstructionList;
};

using NWNScriptLib::NWScriptInstruction;

#endif
