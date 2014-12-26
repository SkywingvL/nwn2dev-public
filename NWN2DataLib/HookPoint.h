/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	HookPoint.h

Abstract:

	This module defines the HookPoint class, which represents a HOOK-typed data
	item in an MDB model.  HookPoints are used to attach rigid models together.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_HOOKPOINT_H
#define _PROGRAMS_NWN2DATALIB_HOOKPOINT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "MDBFormat.h"

//
// Define the hook point class, which represents that a rigid model attachment
// point.
//


//
// Define the hook point core itself.
//

class HookPoint
{

public:

#include <pshpack1.h>

	struct HookHeader
	{
		char           Name[ 32 ];
		unsigned short Type;
		unsigned short Size;
		NWN::Vector3   Position;
		NWN::Matrix33  Orientation;
	};

	C_ASSERT( sizeof( HookHeader ) == 36 + 4 * 3 + 9 * 4 );

#include <poppack.h>

	typedef HookHeader Header;

	inline
	HookPoint(
		)
	{
		ZeroMemory( &m_Header, sizeof( m_Header ) );
	}

	inline
	virtual
	~HookPoint(
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

	//
	// Item attachment point index.
	//

	inline
	unsigned long
	GetItemHookPointIndex(
		) const
	{
		if (memchr( m_Header.Name, '3', sizeof( m_Header.Name ) ))
			return 4;
		else if (memchr( m_Header.Name, '2', sizeof( m_Header.Name ) ))
			return 3;
		else if (memchr( m_Header.Name, '1', sizeof( m_Header.Name ) ))
			return 2;
		else
			return 1;
	}

	inline
	void
	Validate(
		) const
	{
	}

private:

	Header       m_Header;

};

#endif
