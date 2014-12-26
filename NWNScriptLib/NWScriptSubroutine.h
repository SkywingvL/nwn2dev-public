/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptSubroutine.h

Abstract:

	This module defines the NWScriptSubroutine object, which represents an
	analyzed subroutine within a script program.

	N.B.  This module is used only by the analyzer subsystem.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTSUBROUTINE_H
#define _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTSUBROUTINE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace NWNScriptLib
{



	//
	// Define the representation of a scope.  A scope represents a bounded
	// region within which a variable is present on the logical stack.  There
	// is exactly one entry point to a scope and one or more exit points to
	// the scope (though all exit points must logically be balanced).
	//

	class Scope
	{

	public:

		//
		// Create a Scope object, which represents a region of instructions
		// that objects <= to a given SP are active within.
		//

		inline
		Scope(
			__in PROGRAM_COUNTER Entry,
			__in STACK_POINTER SP
			)
		: m_ScopeEntry( Entry ),
		  m_SP( SP )
		{
			//
			// Reserve space for two exit points, which is typically sufficient
			// to cover most cases.
			//

			m_ScopeExit.reserve( 2 );
		}

		//
		// Mark a termination point of the scope.
		//

		inline
		void
		CloseScope(
			__in PROGRAM_COUNTER Exit
			)
		{
			m_ScopeExit.push_back( Exit );
		}

		//
		// Return the entry instruction that created the scope.
		//

		inline
		PROGRAM_COUNTER
		GetScopeEntry(
			) const
		{
			return m_ScopeEntry;
		}

		//
		// Return the exit point list of the scope.  A closed scope has at
		// least one (but possibly more) exit points.
		//

		inline
		const PCVec &
		GetScopeExit(
			) const
		{
			return m_ScopeExit;
		}

	private:

		//
		// Define the scope entry offset.
		//

		PROGRAM_COUNTER m_ScopeEntry;

		//
		// Define the SP value that defines the scope.  At all points within
		// the scope, SP is > m_SP.  Once SP falls below m_SP, the scope is
		// closed (by recording an entry in m_ScopeExit).
		//

		STACK_POINTER   m_SP;

		//
		// Define the list of exit instructions that delete the scope's
		// containing variables.
		//

		PCVec           m_ScopeExit;

	};

	typedef std::vector< Scope > ScopeVec;



	//
	// Define the representation of a script subroutine.
	//

	class NWScriptSubroutine
	{

	public:

		//
		// Define subroutine flags.
		//

		enum
		{
			//
			// The function is a script situation (i.e. action) target.  As a
			// result it does not constitute a true call/return pair and thus
			// cannot complete function analysis (SP is allowed to mismatch on
			// return).
			//

			SCRIPT_SITUATION     = 0x00000001,

			//
			// The function contains code to set up a script situation.
			//

			SAVES_STATE          = 0x00000002,

			LAST_SUBROUTINE_FLAG
		};

		//
		// Define sanity check constants.
		//

		enum
		{
			MAX_SUBROUTINE_PARAMETER_SIZE = 4 * 1 * 1024 * 1024,
			MAX_SUBROUTINE_RETURN_SIZE    = 4 * 1 * 1024 * 1024,

			LAST_MAX_SUBROUTINE_SIZE
		};

		inline
		NWScriptSubroutine(
			__in PROGRAM_COUNTER SubroutineAddress,
			__in unsigned long Flags
			)
		: m_Address( SubroutineAddress ),
		  m_Flags( Flags ),
		  m_Analyzed( false ),
		  m_TypeAnalyzed( false ),
		  m_ParamSize( 0 ),
		  m_ReturnSize( 0 )
		{
			m_ReturnTypes.reserve( 3 ); // Reserve space for vector by default
		}

		inline
		NWScriptSubroutine(
			__in PROGRAM_COUNTER SubroutineAddress,
			__in NWACTION_TYPE ReturnType,
			__in const ParameterList & Parameters,
			__in unsigned long Flags
			)
		: m_Address( SubroutineAddress ),
		  m_Parameters( Parameters ),
		  m_Flags( Flags ),
		  m_Analyzed( false ),
		  m_TypeAnalyzed( false ),
		  m_ParamSize( 0 ),
		  m_ReturnSize( 0 )
		{
			m_ReturnTypes.reserve( 3 ); // Reserve space for vector by default

			if (ReturnType != ACTIONTYPE_VOID)
				AddReturnType( ReturnType );
		}

		//
		// Subroutine address management.
		//

		inline
		PROGRAM_COUNTER
		GetAddress(
			) const
		{
			return m_Address;
		}

		inline
		void
		SetAddress(
			__in PROGRAM_COUNTER Address
			)
		{
			m_Address = Address;
		}

		//
		// Subroutine return type management.
		//

		inline
		NWACTION_TYPE
		GetFirstReturnType(
			) const
		{
			if (m_ReturnTypes.empty( ))
				return ACTIONTYPE_VOID;

			return m_ReturnTypes.front( );
		}

		inline
		void
		AddReturnType(
			__in NWACTION_TYPE ReturnType
			)
		{
			//
			// Expand vector into 3 floats.
			//

			if (ReturnType == ACTIONTYPE_VECTOR)
			{
				AddReturnType( ACTIONTYPE_FLOAT );
				AddReturnType( ACTIONTYPE_FLOAT );
				AddReturnType( ACTIONTYPE_FLOAT );
				return;
			}
			else if (ReturnType == ACTIONTYPE_ACTION)
			{
				//
				// Action cannot be returned, only provided as an argument and
				// only to script action handlers.
				//

				throw std::runtime_error( "action cannot be returned" );
			}

			m_ReturnTypes.push_back( ReturnType );
		}

		inline
		size_t
		GetNumReturnTypes(
			) const
		{
			return m_ReturnTypes.size( );
		}

		inline
		const ReturnTypeList &
		GetReturnTypes(
			) const
		{
			return m_ReturnTypes;
		}

		inline
		ReturnTypeList &
		GetReturnTypes(
			)
		{
			return m_ReturnTypes;
		}

		inline
		STACK_POINTER
		GetReturnSize(
			) const
		{
			return m_ReturnSize;
		}

		inline
		void
		SetReturnSize(
			__in STACK_POINTER ReturnSize
			)
		{
			m_ReturnSize = ReturnSize;

			if (m_ReturnSize > MAX_SUBROUTINE_RETURN_SIZE || m_ReturnSize < 0)
				throw std::runtime_error( "subroutine maximum return size exceeded" );
		}

		//
		// Update the return size for a negative stack access.
		//

		inline
		void
		UpdateReturnSize(
			__in STACK_POINTER Offset
			)
		{
			if (Offset > 0)
				return;

			if (GetAddress( ) == 114)
				Offset = Offset;
			if (-Offset > m_ReturnSize)
				m_ReturnSize = -Offset;

			if (m_ReturnSize > MAX_SUBROUTINE_RETURN_SIZE || m_ReturnSize < 0)
				throw std::runtime_error( "subroutine maximum return size exceeded" );
		}

		//
		// Indicates whether the function has a return value.
		//

		inline
		bool
		HasReturnValue(
			) const
		{
			return GetReturnSize( ) != 0;
		}

		//
		// Subroutine parameter management.
		//

		inline
		const ParameterList &
		GetParameters(
			) const
		{
			return m_Parameters;
		}

		inline
		ParameterList &
		GetParameters(
			)
		{
			return m_Parameters;
		}

		inline
		void
		SetParameters(
			__in const ParameterList & Parameters
			)
		{
			m_Parameters = Parameters;
		}

		inline
		STACK_POINTER
		GetParameterSize(
			) const
		{
			return m_ParamSize;
		}

		inline
		void
		SetParameterSize(
			__in STACK_POINTER ParamSize
			)
		{
			m_ParamSize = ParamSize;

			if (m_ParamSize > MAX_SUBROUTINE_PARAMETER_SIZE || m_ParamSize < 0)
				throw std::runtime_error( "subroutine maximum return size exceeded" );
		}

		//
		// Return the subroutine flags (e.g. SCRIPT_SITUATION).
		//

		inline
		unsigned long
		GetFlags(
			) const
		{
			return m_Flags;
		}

		inline
		void
		SetFlags(
			__in unsigned long Flags
			)
		{
			m_Flags = Flags;
		}

		//
		// Label (branch target) and branch analysis list access.
		//

		inline
		const LabelVec &
		GetBranchTargets(
			) const
		{
			return m_BranchTargets;
		}

		inline
		LabelVec &
		GetBranchTargets(
			)
		{
			return m_BranchTargets;
		}

		inline
		LabelVec &
		GetAnalyzeBranches(
			)
		{
			return m_AnalyzeBranches;
		}

		//
		// Function analysis state.  A function is marked as analyzed once the
		// code stream has been traced through to a return at least once (such
		// that the parameters and return value sizes are known).
		//

		inline
		bool
		GetIsAnalyzed(
			) const
		{
			return m_Analyzed;
		}

		inline
		void
		SetIsAnalyzed(
			__in bool Analyzed
			)
		{
			m_Analyzed = Analyzed;
		}

		//
		// Function type analysis state.  A function is marked as type analyzed
		// once all control flows have been traced through to a return at least
		// once (such that all parameter types and return types are known).
		//

		inline
		bool
		GetIsTypeAnalyzed(
			) const
		{
			return m_TypeAnalyzed;
		}

		inline
		void
		SetIsTypeAnalyzed(
			__in bool TypeAnalyzed
			)
		{
			m_TypeAnalyzed = TypeAnalyzed;
		}

		//
		// Variable management.
		//

		inline
		void
		AddLocal(
			__in VariablePtr Var
			)
		{
			m_Locals.push_back( Var );
		}

		inline
		VariablePtrVec &
		GetLocals(
			)
		{
			return m_Locals;
		}

		inline
		const VariablePtrVec &
		GetLocals(
			) const
		{
			return m_Locals;
		}

		//
		// Scope management.
		//

		inline
		void
		AddScope(
			__in Scope & LocalScope
			)
		{
			m_Scopes.push_back( LocalScope );
		}

		inline
		ScopeVec &
		GetScopes(
			)
		{
			return m_Scopes;
		}

		inline
		const ScopeVec &
		GetScopes(
			) const
		{
			return m_Scopes;
		}

		//
		// Control flow management.
		//

		inline
		ControlFlowSet &
		GetControlFlows(
			)
		{
			return m_ControlFlows;
		}

		inline
		const ControlFlowSet &
		GetControlFlows(
			) const
		{
			return m_ControlFlows;
		}

		//
		// Return the control flow for a given address (start PC).
		//

		inline
		ControlFlowPtr
		GetControlFlow(
			__in PROGRAM_COUNTER FlowPC
			) const
		{
			ControlFlowSet::const_iterator it = m_ControlFlows.lower_bound( FlowPC );

			if (it != m_ControlFlows.end() && it->first == FlowPC)
				return it->second;	// This is it
			// Not yet. If it exists, it will be the previous entry in the set.
			else if (it == m_ControlFlows.begin( ))
				return NULL;	// No previous entry

			it--;

			// Is this it?
			if (FlowPC == it->first)
				return it->second;
			else if (it->second->GetEndPC( ) != INVALID_PC && 
				FlowPC >= it->first && 
				FlowPC < it->second->GetEndPC( ))
				return it->second;
			else
				return NULL;
		}

		//
		// Link a variable to a parameter slot.
		//

		inline
		NWScriptVariable &
		GetParameterVariable(
			__in size_t ParamIndex
			)
		{
			if (ParamIndex >= (size_t) m_ParamSize / CELL_SIZE ||
				!m_ParameterVars[ ParamIndex ])
				throw std::runtime_error( "out of range parameter to subroutine" );

			return *m_ParameterVars[ ParamIndex ];
		}

		//
		// Link a variable to a return value slot.
		//

		inline
		NWScriptVariable &
		GetReturnValueVariable(
			__in size_t ReturnIndex
			)
		{
			if (ReturnIndex >= (size_t) m_ReturnSize / CELL_SIZE ||
				!m_ReturnValueVars[ ReturnIndex ])
				throw std::runtime_error( "out of range return value to subroutine" );

			return *m_ReturnValueVars[ ReturnIndex ];
		}

		//
		// Create the Variable instances representing the function 
		// parametes and return values.
		//

		inline
		void
		CreateParameterReturnVariables(
			)
		{
			STACK_POINTER SP = 0;

			m_ParameterVars.reserve( m_ReturnSize / CELL_SIZE );
			m_ReturnValueVars.reserve( m_ParamSize / CELL_SIZE );

			for (STACK_POINTER i = 0; i < m_ReturnSize / CELL_SIZE; i++)
			{
				m_Locals.push_back( 
					new Variable( SP, Variable::ReturnValue ) );
				SP += CELL_SIZE;

				m_ReturnValueVars.push_back( m_Locals.back( ).get( ) );
			}

			for (STACK_POINTER i = 0; i < m_ParamSize / CELL_SIZE; i++)
			{
				m_Locals.push_back(
					new Variable( SP, Variable::Parameter ) );
				SP += CELL_SIZE;

				m_ParameterVars.push_back( m_Locals.back( ).get( ) );
			}
		}

		//
		// Symbol name management.
		//

		inline
		const std::string &
		GetSymbolName(
			) const
		{
			return m_SymbolName;
		}

		inline
		std::string &
		GetSymbolName(
			)
		{
			return m_SymbolName;
		}

		inline
		void
		SetSymbolName(
			__in const std::string & SymbolName
			)
		{
			m_SymbolName = SymbolName;
		}

	private:

		//
		// Define the address of the subroutine.
		//

		PROGRAM_COUNTER      m_Address;

		//
		// Define the return types of the subroutine.  Multiple return types
		// indicate structures.  Note that 'vector' is always expanded to three
		// floats here.
		//

		ReturnTypeList       m_ReturnTypes;

		//
		// Define the arguments (if any) that the subroutine takes.  Note that
		// unused arguments are typed as void and take one stack cell.
		//

		ParameterList        m_Parameters;

		//
		// Define the flags for the subroutine
		//

		unsigned long        m_Flags;

		//
		// Define the list of control flows present in the subroutine.  The
		// addresses are those of branch targets.
		//

		LabelVec             m_BranchTargets;

		//
		// Define the list of branch targets to analyze.
		//

		LabelVec             m_AnalyzeBranches;

		//
		// Define whether the subroutine has been analyzed (i.e. the return
		// value and parameters are accounted for).
		//

		bool                 m_Analyzed;

		//
		// Define whether type data for the subroutine has been analyzed (i.e.
		// the return types and parameter types are accounted for).
		//

		bool                 m_TypeAnalyzed;

		//
		// Define the collective parameter size.
		//

		STACK_POINTER        m_ParamSize;

		//
		// Define the collective return value size.
		//

		STACK_POINTER        m_ReturnSize;

		//
		// Define the local variable list in the function.  This list holds the
		// underlying storage references for the variables.
		//

		VariablePtrVec       m_Locals;

		//
		// Define the control flow list in the function.
		//

		ControlFlowSet       m_ControlFlows;

		//
		// Define the scope list in the function.
		//

		ScopeVec             m_Scopes;

		//
		// Define the lists of Variable entries for the parameters and 
		// return values. Node that these lists do not actually contain the 
		// Variable instances themselves - the variables are contained in 
		// the m_Locals set;
		//

		VariableWeakPtrVec   m_ParameterVars;
		VariableWeakPtrVec   m_ReturnValueVars;

		//
		// Define the name of the subroutine.
		//

		std::string          m_SymbolName;
	};

	typedef NWScriptSubroutine Subroutine;

	typedef std::vector< NWScriptSubroutine > SubroutineVec;

}

using NWNScriptLib::NWScriptSubroutine;

#endif