/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	CollisionSphere.h

Abstract:

	This module defines the CollisionSphereList class, which represents a
	COLS-typed data stream in an MDB model.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_COLLISIONSPHERE_H
#define _PROGRAMS_NWN2DATALIB_COLLISIONSPHERE_H

#ifdef _MSC_VER
#pragma once
#endif

//
// Define the collision sphere class, which represents a collider sphere that
// is used to perform object-object collision.
//

//
// Define vertex and face types for skin mesh.
//

#include <pshpack1.h>
struct CollisionSphereFile
{
	unsigned long BoneIndex;
	float         Radius;
};

C_ASSERT( sizeof( CollisionSphereFile ) == 8 );
#include <poppack.h>

struct CollisionSphere
{
	//
	// XXX: Transformed...
	//

	unsigned long BoneIndex;
	float         Radius;
};

//
// Define the collision sphere core itself.
//

class CollisionSphereList
{

public:

	typedef ::CollisionSphereFile ItemFile;
	typedef ::CollisionSphere Item;

#include <pshpack1.h>

	struct CollisionSpheresHeader
	{
		unsigned long  NumItems;
	};

	C_ASSERT( sizeof( CollisionSpheresHeader ) == 4 );

#include <poppack.h>

	typedef CollisionSpheresHeader Header;

	typedef std::vector< CollisionSphere > ColSphereVec;

	inline
	CollisionSphereList(
		)
	{
		ZeroMemory( &m_Header, sizeof( m_Header ) );
	}

	inline
	virtual
	~CollisionSphereList(
		)
	{
	}

	inline
	void
	Clear(
		)
	{
		m_Spheres.clear( );
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
	void
	AddSphere(
		__in const CollisionSphere & Sphere
		)
	{
		m_Spheres.push_back( Sphere );
	}

	inline
	const ColSphereVec &
	GetSpheres(
		) const
	{
		return m_Spheres;
	}

	inline
	ColSphereVec &
	GetSpheres(
		)
	{
		return m_Spheres;
	}

	inline
	void
	Validate(
		) const
	{
	}

	static
	inline
	void
	CopyFileItem(
		__in const ItemFile & ItemS,
		__out Item & ItemD
		)
	{
		ItemD.BoneIndex = ItemS.BoneIndex;
		ItemD.Radius    = ItemS.Radius;
	}

private:

	Header       m_Header;
	ColSphereVec m_Spheres;

};

#endif
