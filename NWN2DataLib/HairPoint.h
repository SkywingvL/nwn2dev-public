/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	HairPoint.h

Abstract:

	This module defines the HairPoint class, which represents a HAIR-typed data
	item in an MDB model.  HairPoints are used to specify the hair hiding
	behavior of a model

--*/

#ifndef _PROGRAMS_NWN2DATALIB_HAIRPOINT_H
#define _PROGRAMS_NWN2DATALIB_HAIRPOINT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "MDBFormat.h"

//
// Define the hair point class, which represents that a hair attachment point.
//


//
// Define the hair point core itself.
//

class HairPoint
{

public:

#include <pshpack1.h>

	struct HairHeader
	{
		char           Name[ 32 ];
		unsigned long  HairFlag;
		NWN::Vector3   Position;
		NWN::Matrix33  Orientation;
	};

	C_ASSERT( sizeof( HairHeader ) == 36 + 4 * 3 + 9 * 4 );

#include <poppack.h>

	typedef HairHeader Header;

	inline
	HairPoint(
		)
	{
		ZeroMemory( &m_Header, sizeof( m_Header ) );

		m_Header.HairFlag = NWN::HSB_NORMAL;
	}

	inline
	virtual
	~HairPoint(
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
		if (m_Header.HairFlag >= NWN::LAST_HSB)
			throw std::runtime_error( "Illegal HairFlag" );
	}

private:

	Header       m_Header;

};

#endif
