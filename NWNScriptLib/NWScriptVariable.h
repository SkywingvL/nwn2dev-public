/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptVariable.h

Abstract:

	This module defines the NWScriptVariable object, which represents a single
	variable in an analyzed subroutine.  Generally, each stack location is
	represented by a unique variable.

	N.B.  This module is used only by the analyzer subsystem.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTVARIABLE_H
#define _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTVARIABLE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace NWNScriptLib
{
	//
	// Define the representation of a script variable.
	//

	typedef size_t VariableId;
	typedef std::vector< VariableId > VariableIdVec;

	static const VariableId NULL_VARIABLE = (VariableId) -1;

	class NWScriptVariable;

	typedef swutil::SharedPtr< NWScriptVariable > NWScriptVariablePtr;

	class NWScriptVariable
	{

	public:

		typedef swutil::SharedPtr< NWScriptVariable > Ptr;
		typedef swutil::SharedPtr< NWScriptVariable > VariablePtr;

		//
		// Define the class of variable.
		//

		typedef enum _CLASS
		{
			//
			// The variable is a global.
			//

			Global,

			//
			// The variable is a local.
			//

			Local,

			//
			// The variable is a subroutine parameter for a called subroutine.
			//

			CallParameter,

			//
			// The variable is a subroutine return value for a called
			// subroutine.
			//

			CallReturnValue,

			//
			// The variable is a parameter to the current subroutine.
			//

			Parameter,

			//
			// The variable is a return value for the current subroutine.
			//

			ReturnValue,

			//
			// The variable represents a constant in the bytecode,
			// and may be emitted as a constant in the generated code.
			//

			Constant,

			//
			// The usage of the variable hasn't been identified.
			//

			Unknown			
		} CLASS, * PCLASS;

		typedef const enum _CLASS * PCCLASS;

		//
		// Define flags that modify the way a variable behaves. These 
		// flags are generally applied conservatively - parser shortcomings 
		// may fail to detect an opportunity for optimization, but will 
		// never attempt an incorrect optimization that produces faulty 
		// output.
		//

		typedef enum _FLAGS
		{
			//
			// The local variable is localized to a single flow. This can 
			// be used as a hint to the code generator that the variable 
			// can be stored in a short-lived location.
			//

			LocalToFlow = 1,

			//
			// The variable is read-only, and is not changed once assigned.
			// This is another hint to the optimizer/code generator as to 
			// where to allocate space for the variable (if at all), e.g.
			// such a variable could be assigned a register in LLVM.
			//

			SingleAssignment = 2,

			//
			// The variable has been eliminated by the optimizer. The 
			// variable has either been eliminated as unused, or merged 
			// into another variable.
			//

			OptimizerEliminated = 4,

			//
			// The variable is written to but never read from
			//

			WriteOnly = 8,

			//
			// The variable is created in at least two different flows. This 
			// information can be of value to the code generator.
			//

			MultiplyCreated = 16
		} FLAGS, * PFLAGS;

		inline
		NWScriptVariable(
			__in STACK_POINTER SP,
			__in SCOPE_ID Scope,
			__in NWACTION_TYPE Type
			)
		: m_SP( SP ),
		  m_Scope( Scope ),
		  m_Type( Type ),
		  m_Class( Unknown ),
		  m_Flags( 0 ),
		  m_MergedWith( NULL ),
		  m_UserContext( NULL )
		{
		}

		inline
		NWScriptVariable(
			__in STACK_POINTER SP,
			__in CLASS Class,
			__in NWACTION_TYPE Type = ACTIONTYPE_VOID
			)
		: m_SP( SP ),
		  m_Scope( INVALID_SCOPE ),
		  m_Type( Type ),
		  m_Class( Class ),
		  m_Flags( 0 ),
		  m_MergedWith( NULL ),
		  m_UserContext( NULL )
		{
		}

		//
		// SP displacement of the variable within its subroutine frame.  The SP
		// value is a positive offset from the logical base of the frame (0).
		//

		inline
		STACK_POINTER
		GetSP(
			) const
		{
			return m_SP;
		}

		inline
		void
		SetSP(
			__in STACK_POINTER SP
			)
		{
			m_SP = SP;
		}

		//
		// Scope index of the variable within its subroutine.  A scope defines
		// a bounded range of program counter values that defines the lifetime
		// of the variable.  Note that it is possible that the scope may be
		// closed at multiple control flow points (such as two forks of a JZ
		// instruction), although both paths must be logically equally balanced
		// at the joining point.
		//
		// N.B.  A scope index of INVALID_SCOPE indicates that the scope is
		//       still indeterminite, i.e. we have not reached the end of the
		//       scope and don't know the extents of it.
		//

		inline
		SCOPE_ID
		GetScope(
			) const
		{
			return m_Scope;
		}

		inline
		void
		SetScope(
			__in SCOPE_ID Scope
			)
		{
			m_Scope = Scope;
		}

		//
		// Type of the variable.  A variable cannot change type through its
		// lifetime.
		//

		inline
		NWACTION_TYPE
		GetType(
			) const
		{
			return m_Type;
		}

		inline
		void
		SetType(
			__in NWACTION_TYPE Type
			)
		{
			if (Type == ACTIONTYPE_VOID)
				return;
			else if (m_Type != ACTIONTYPE_VOID && Type != m_Type)
				throw std::runtime_error( 
					"variable type mismatch in SetType" );

			m_Type = Type;

			if (!m_EquivalenceClass.get( ))
				return;

			//
			// This variable is part of an untyped equivalence class. 
			// We now know the type, so update all the variables and 
			// dissolve the class.
			//

			// Hold on to a reference to the class
			EquivalenceSetPtr Class = m_EquivalenceClass;

			for (EquivalenceSet::iterator VarIt = Class->begin( );
				VarIt != Class->end( ); VarIt++)
			{
				(*VarIt)->m_Type = Type;
				(*VarIt)->m_EquivalenceClass = NULL;
			}
		}

		//
		// Class (usage) of the variable, such as whether it is a parameter,
		// a return value, a plain local, etc.
		//

		inline
		CLASS
		GetClass(
			) const
		{
			return m_Class;
		}

		inline
		void
		SetClass(
			__in CLASS Class
			)
		{
			m_Class = Class;
		}

		inline
		unsigned
		GetFlags(
			) const
		{
			return m_Flags;
		}

		inline
		void
		SetFlags(
			__in unsigned Flags
			)
		{
			m_Flags = Flags;
		}

		inline
		bool
		IsFlagSet(
			__in FLAGS Flag
			) const
		{
			return (m_Flags & (unsigned)Flag) != 0;
		}

		inline
		void
		SetFlag(
			__in FLAGS Flag,
			__in_opt bool Set = true
			)
		{
			if (Set)
				m_Flags |= (unsigned)Flag;
			else
				m_Flags &= ~(unsigned)Flag;
		}

		//
		// Merged variable access.  A variable may be merged with another
		// variable under two conditions. First, it's possible that a 
		// single variable can be created in two seperate but mutually-
		// exclusive flow paths which ultimately meet; in this case two 
		// separate variables are created, but they are merged when the 
		// flow merge is evaluated (but the two separate creates must be 
		// preserved. Variables may also be optimized away during the 
		// optimization phase; in this case the eliminated variable is 
		// merged into the remaining, and excess instructions are 
		// eliminated (but references to the eliminated variable are left).
		//
		// Regardless of which is the case, GetHeadVariable should always
		// be used to get the actual variable a variable reference refers to.
		//
		// N.B.  Once a variable has been linked, it cannot be deleted without
		//       all variables across all functions being deleted as a unit !
		//

		inline
		NWScriptVariable *
		GetMergedWith(
			) const
		{
			return m_MergedWith;
		}

		inline
		void
		SetMergedWith(
			__in NWScriptVariable * MergedWith
			)
		{
			m_MergedWith = MergedWith;
		}

		//
		// Gets the variable all others in a set have been merged with.
		//

		inline
		NWScriptVariable *
		GetHeadVariable(
			)
		{
			NWScriptVariable *Var = this;
			while (Var->m_MergedWith != NULL)
				Var = Var->m_MergedWith;

			return Var;
		}

		//
		// User context.  The context value is reserved for the logic that
		// consumes the IR (i.e. JIT backend).
		//

		inline
		void *
		GetUserContext(
			)
		{
			return m_UserContext;
		}

		inline
		void
		SetUserContext(
			__in void * UserContext
			)
		{
			m_UserContext = UserContext;
		}

		//
		// Create a type linkage between a variable and this variable, such
		// that both share the same type data.
		//

		inline
		void
		LinkTypes(
			__in NWScriptVariable * Var
			)
		{
			//
			// First, propagate type information (if possible).
			//

			if (m_Type != ACTIONTYPE_VOID)
			{
				Var->SetType( m_Type );
				return;
			}
			else if (Var->GetType( ) != ACTIONTYPE_VOID)
			{
				SetType( Var->GetType( ) );
				return;
			}

			//
			// The variables are both unknown, so we can't resolve the 
			// types now. Link the variables into an equivalence class.
			//

			if (m_EquivalenceClass != NULL)
			{
				if (Var->m_EquivalenceClass != NULL)
				{
					// Merge the other equivalence classes into this one.
					// Make sure we hold on to a reference to the class so 
					// it doesn't disappear during this loop.
					EquivalenceSetPtr Source = Var->m_EquivalenceClass;

					for (EquivalenceSet::iterator VarIt = Source->begin( ); 
						VarIt != Source->end( ); VarIt++)
					{
						m_EquivalenceClass->insert( *VarIt );
						(*VarIt)->m_EquivalenceClass = m_EquivalenceClass;
					}

					// The set itself should be freed automagically when 
					// this scope exits
				}
				else
				{
					m_EquivalenceClass->insert( Var );
					Var->m_EquivalenceClass = m_EquivalenceClass;
				}
			}
			else
			{
				if (Var->m_EquivalenceClass != NULL)
				{
					Var->m_EquivalenceClass->insert( this );
					m_EquivalenceClass = Var->m_EquivalenceClass;
				}
				else
				{
					m_EquivalenceClass = new EquivalenceSet();
					Var->m_EquivalenceClass = m_EquivalenceClass;

					m_EquivalenceClass->insert( this );
					m_EquivalenceClass->insert( Var );
				}
			}
		}

		//
		// Check whether a variable requires explicitly managed storage
		// assignment (i.e. other than just by simple I_CREATE).
		//

		inline
		bool
		GetRequiresExplicitStorage(
			) const
		{
			//
			// Multiply created variables need special allocation handling.
			//

			if (IsFlagSet( MultiplyCreated ))
				return true;

			//
			// So, to, do globals, parameters, and return values.
			//

			switch (GetClass( ))
			{

			case Global:
			case Parameter:
			case ReturnValue:
				return true;

			}

			return false;
		}

	private:

		//
		// When two variables of unknown type are linked, they form an 
		// equivalence set that is resolved when a variable of known type is 
		// added. As an optimization, once the type of a class is known, 
		// further attempts to link additional variables merely set the 
		// types of those variables immediately.
		//

		typedef stdext::hash_set< NWScriptVariable * > EquivalenceSet;
		typedef swutil::SharedPtr< EquivalenceSet > EquivalenceSetPtr;

		//
		// Define the SP value when the variable was created.
		//

		STACK_POINTER       m_SP;

		//
		// Define the scope ID of the variable.
		//

		SCOPE_ID            m_Scope;

		//
		// Define the type of the variable.  All variables are assumed to be of
		// CELL_SIZE size (structures are not raised).  A variable whose type
		// has not yet been identified has type ACTIONTYPE_VOID.
		//

		NWACTION_TYPE       m_Type;

		//
		// Define the class (usage) of the variable, such as whether it is a
		// normal local, a parameter, etc.
		//

		CLASS               m_Class;

		//
		// Define the flags for the variable, e.g. whether it's temporary.
		//

		unsigned            m_Flags;

		//
		// Define the merged with link, which points to a variable that the
		// current variable has been merged into (when flow control chains
		// meet).
		//

		NWScriptVariable  * m_MergedWith;

		//
		// Define the link for the type-equivalence list of
		// this variable.  All variables in the type-equivalence list have a
		// related type (i.e. due to a copy from or copy to operation).  Once
		// the type of any variable in the list is known, the types of all such
		// variables are known.
		//

		EquivalenceSetPtr   m_EquivalenceClass;

		//
		// Define a user context value for use by the consumer of the IR.
		//

		void              *  m_UserContext;
	};

	typedef NWScriptVariable Variable;
	typedef Variable::VariablePtr VariablePtr;

	typedef std::vector< Variable > VariableVec;
	typedef std::vector< VariablePtr > VariablePtrVec;
	typedef std::vector< Variable * > VariableWeakPtrVec;

	typedef stdext::hash_set< VariablePtr > VariableEquivalenceSet;
	typedef swutil::SharedPtr< VariableEquivalenceSet > EquivalenceSetPtr;
	typedef stdext::hash_map< VariablePtr, EquivalenceSetPtr > EquivalenceSetMap;

}

using NWNScriptLib::NWScriptVariable;

#endif
