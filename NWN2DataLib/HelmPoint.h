/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	HelmPoint.h

Abstract:

	This module defines the HelmPoint class, which represents a HELM-typed data
	item in an MDB model.  HelmPoints are used to specify the helm hair hiding
	behavior of a model

--*/

#ifndef _PROGRAMS_NWN2DATALIB_HELMPOINT_H
#define _PROGRAMS_NWN2DATALIB_HELMPOINT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "MDBFormat.h"

//
// Define the helm point class, which represents that a helm attachment point.
//


//
// Define the helm point core itself.
//

class HelmPoint
{

public:

#include <pshpack1.h>

	struct HelmHeader
	{
		char           Name[ 32 ];
		unsigned long  HelmFlag;
		NWN::Vector3   Position;
		NWN::Matrix33  Orientation;
	};

	C_ASSERT( sizeof( HelmHeader ) == 36 + 4 * 3 + 9 * 4 );

#include <poppack.h>

	typedef HelmHeader Header;

	inline
	HelmPoint(
		)
	{
		ZeroMemory( &m_Header, sizeof( m_Header ) );
	}

	inline
	virtual
	~HelmPoint(
		)
	{
	}

	inline
	Header &
	GetHeader(
		)
	{
		return m_Header;
	}

	inline
	const Header &
	GetHeader(
		) const
	{
		return m_Header;
	}

	inline
	const char *
	GetName(
		) const
	{
		return m_Header.Name;
	}

	inline
	const NWN::Vector3 &
	GetPoint(
		) const
	{
		return m_Header.Position;
	}

	inline
	const NWN::Matrix33 &
	GetTransformMatrix(
		) const
	{
		return m_Header.Orientation;
	}

	inline
	void
	Validate(
		) const
	{
		if (m_Header.HelmFlag >= NWN::LAST_HHHB)
			throw std::runtime_error( "Illegal HelmFlag" );
	}

private:

	Header       m_Header;

};

#endif
