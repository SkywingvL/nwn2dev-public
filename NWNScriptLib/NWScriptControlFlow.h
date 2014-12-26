/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptControlFlow.h

Abstract:

	This module defines the NWScriptControlFlow object, which represents a
	section of control flow within a subroutine.

	N.B.  This module is used only by the analyzer subsystem.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTCONTROLFLOW_H
#define _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTCONTROLFLOW_H

#ifdef _MSC_VER
#pragma once
#endif

namespace NWNScriptLib
{

	class NWScriptInstruction;

	//
	// Define the representation of a control flow.
	//

	class NWScriptControlFlow
	{
		typedef swutil::SharedPtr< NWScriptControlFlow > ControlFlowPtr;

	public:

		//
		// This enumeration describes in what way this control flow ends
		//

		typedef enum _TERMINATION_TYPE
		{
			// Not yet discovered
			Unknown = 0,

			// Flow ends with a function return, terminating execution
			Terminate,

			// Flow has no special manner of termination, control continuing 
			// to the next flow. This occurs when the existance of an 
			// intervening branch target label forces a new flow to begin.
			Merge,

			// Flow ends by transferring control to a branch target
			Transfer,

			// Flow ends with execution either branching to a branch target 
			// or continuing to the subsequent flow
			Split,

			NumTerminationTypes
		} TERMINATION_TYPE, * PTERMINATION_TYPE;

		inline
		NWScriptControlFlow(
			__in PROGRAM_COUNTER StartPC,
			__in STACK_POINTER   StartSP
			)
		: m_StartPC( StartPC ),
		  m_StartSP( StartSP ),
		  m_EndPC( INVALID_PC ),
		  m_EndSP( INVALID_SP ),
		  m_TerminationType( Unknown )
		{
			m_Children[ 0 ] = NULL;
			m_Children[ 1 ] = NULL;
		}

		inline
		NWScriptControlFlow(
			)
		: m_StartPC( INVALID_PC ),
		  m_StartSP( INVALID_SP ),
		  m_EndPC( INVALID_PC ),
		  m_EndSP( INVALID_SP ),
		  m_TerminationType( Unknown )
		{
			m_Children[ 0 ] = NULL;
			m_Children[ 1 ] = NULL;
		}

		inline
		PROGRAM_COUNTER
		GetStartPC(
			) const
		{
			return m_StartPC;
		}

		inline
		void
		SetStartPC(
			__in PROGRAM_COUNTER StartPC
			)
		{
			m_StartPC = StartPC;
		}

		inline
		STACK_POINTER
		GetStartSP(
			) const
		{
			return m_StartSP;
		}

		inline
		void
		SetStartSP(
			__in STACK_POINTER StartSP
			)
		{
			m_StartSP = StartSP;
		}

		inline
		PROGRAM_COUNTER
		GetEndPC(
			) const
		{
			return m_EndPC;
		}

		inline
		void
		SetEndPC(
			__in PROGRAM_COUNTER EndPC
			)
		{
			m_EndPC = EndPC;
		}

		inline
		STACK_POINTER
		GetEndSP(
			) const
		{
			return m_EndSP;
		}

		inline
		void
		SetEndSP(
			__in STACK_POINTER EndSP
			)
		{
			m_EndSP = EndSP;
		}

		//
		// Next control flows.  There may be zero, one, or two successor
		// control flows; zero if we have reached the end of the function, one
		// if we are an unconditional flow, else two if we are a conditional
		// (or save state) flow.
		//

		inline
		ControlFlowPtr
		GetChild(
			__in size_t i
			) const
		{
			return m_Children[ i ];
		}

		inline
		void
		SetChild(
			__in size_t i,
			__in ControlFlowPtr Child
			)
		{
			m_Children[ i ] = Child;
		}

		inline
		std::set< NWScriptControlFlow * > &
		GetParents(
			)
		{
			return m_Parents;
		}

		inline
		TERMINATION_TYPE
		GetTerminationType(
			) const
		{
			return m_TerminationType;
		}

		inline
		void
		SetTerminationType(
			__in TERMINATION_TYPE TerminationType
			)
		{
			m_TerminationType = TerminationType;
		}

		inline
		const std::list< NWScriptInstruction > &
		GetIR(
			) const
		{
			return m_Instructions;
		}

		inline
		std::list< NWScriptInstruction > &
		GetIR(
			)
		{
			return m_Instructions;
		}

		inline
		bool
		operator < (
			__in NWScriptControlFlow & Other
			) const
		{
			return ( m_StartPC < Other.m_StartPC );
		}

	private:

		PROGRAM_COUNTER       m_StartPC;
		STACK_POINTER         m_StartSP;
		PROGRAM_COUNTER       m_EndPC;
		STACK_POINTER         m_EndSP;

		TERMINATION_TYPE      m_TerminationType;

		ControlFlowPtr        m_Children[ 2 ];
		std::set< NWScriptControlFlow * >
			                   m_Parents;

		std::list< NWScriptInstruction >
			                   m_Instructions;

	};

	typedef NWScriptControlFlow ControlFlow;

	typedef std::list< NWScriptControlFlow > ControlFlowList;
	typedef std::set< NWScriptControlFlow * > ControlFlowWeakPtrSet;

	typedef swutil::SharedPtr< NWScriptControlFlow > ControlFlowPtr;
	typedef std::map< PROGRAM_COUNTER, ControlFlowPtr > ControlFlowSet;
}

using NWNScriptLib::NWScriptControlFlow;

#endif
