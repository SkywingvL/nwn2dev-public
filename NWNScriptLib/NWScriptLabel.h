/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptLabel.h

Abstract:

	This module defines the NWScriptLabel object, which represents a label in
	an analyzed subroutine.

	N.B.  This module is used only by the analyzer subsystem.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTLABEL_H
#define _SOURCE_PROGRAMS_NWNSCRIPTLIB_NWSCRIPTLABEL_H

#ifdef _MSC_VER
#pragma once
#endif

namespace NWNScriptLib
{

	//
	// Define the representation of a label (a control transfer target).
	//

	class NWScriptLabel
	{

	public:

		inline
		NWScriptLabel(
			__in PROGRAM_COUNTER Address,
			__in STACK_POINTER SP,
			__in ControlFlowPtr Flow,
			__in unsigned long Flags = 0
			)
		: m_Address( Address ),
		  m_SP( SP ),
		  m_Flow( Flow ),
		  m_Flags( Flags )
		{
		}

		//
		// Return the label's address.
		//

		inline
		PROGRAM_COUNTER
		GetAddress(
			) const
		{
			return m_Address;
		}

		//
		// Return the SP value at the start of the label.
		//

		inline
		STACK_POINTER
		GetSP(
			) const
		{
			return m_SP;
		}

		//
		// Return the control flow descriptor whose flow begins at the label
		// address.
		//

		inline
		ControlFlowPtr
		GetControlFlow(
			)
		{
			return m_Flow;
		}

		inline
		const ControlFlowPtr
		GetControlFlow(
			) const
		{
			return m_Flow;
		}

		//
		// Return the label flags
		//

		inline
		unsigned long
		GetFlags(
			) const
		{
			return m_Flags;
		}

	private:

		PROGRAM_COUNTER       m_Address;
		STACK_POINTER         m_SP;
		ControlFlowPtr        m_Flow;
		unsigned long         m_Flags;

	};

	typedef NWScriptLabel Label;

	typedef std::vector< NWScriptLabel > LabelVec;
	typedef LabelVec::size_type LabelId;

	static const LabelId NULL_LABEL = (LabelId) -1;

}

using NWNScriptLib::NWScriptLabel;

#endif
