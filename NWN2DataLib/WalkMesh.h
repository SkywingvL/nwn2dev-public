/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	WalkMesh.h

Abstract:

	This module defines the WalkMesh class, which contains a WALK-typed mesh
	that is stored in local coordinate form and forms the basis of the walkable
	region set of a static object that participates in walkmesh cutting, such
	as a placeable.

	Most real-time pathfinding is performed exclusively with the baked
	AreaSurfaceMesh object, which already takes all static WALK-typed objects
	that were present at baking time into account.  WALK meshes are typically
	only used by the content creation tools (i.e. the toolset), and the area
	baking system.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_WALKMESH_H
#define _PROGRAMS_NWN2DATALIB_WALKMESH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "SimpleMesh.h"
#include "MDBFormat.h"

//
// Define the object walk mesh, used to encode walkable/non-walkable regions
// for placeables.  Typically, pathing does not use class WalkMesh directly,
// but instead uses the baked AreaSurfaceMesh.
//

//
// Define vertex and face types for walk mesh.
//

#include <pshpack1.h>
struct WMVertexFile
{
	NWN::Vector3 Position;
};

C_ASSERT( sizeof( WMVertexFile ) == 3 * 4 );

struct WMFaceFile
{
	unsigned short Corners[ 3 ];
	unsigned long  Flags;
};

C_ASSERT( sizeof( WMFaceFile ) == 3 * 2 + 4 );

#include <poppack.h>

struct WMVertex
{
	NWN::Vector3 LocalPos;
//	NWN::Vector3 Pos; // Current position, calculated (not in file)
};

struct WMFace
{
	unsigned long  Corners[ 3 ];
	NWN::Vector3   Normal;
	unsigned long  Flags;

	//
	// See SurfaceMeshBase::SurfaceMeshTriangle for mesh flags.
	//
};

extern const SimpleMeshTypeDescriptor SMTD_WalkMesh;

//
// Define the walk mesh core itself.
//

class WalkMesh : public SimpleMesh< WMVertex, WMFace, &SMTD_WalkMesh >
{

public:

	typedef ::WMVertex Vertex;
	typedef ::WMVertexFile VertexFile;
	typedef ::WMFace Face;
	typedef ::WMFaceFile FaceFile;

	typedef SimpleMesh< Vertex, Face, &SMTD_WalkMesh > BaseMesh;

#include <pshpack1.h>

	struct WalkHeader
	{
		NWN::ResRef32  Name;
		unsigned long  Flags;
		unsigned long  NumVerts;
		unsigned long  NumFaces;
	};

	C_ASSERT( sizeof( WalkHeader ) == 32 + 12 );

#include <poppack.h>

	typedef WalkHeader Header;

	inline
	WalkMesh(
		)
	: SimpleMesh< WMVertex, WMFace, &SMTD_WalkMesh >( )
	{
		ZeroMemory( &m_Header, sizeof( m_Header ) );
	}

	inline
	virtual
	~WalkMesh(
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

	static
	inline
	void
	CopyFileVertex(
		__in const VertexFile & FVert,
		__out Vertex & Vert
		)
	{
		Vert.LocalPos  = FVert.Position;
	}

	static
	inline
	void
	CopyFileFace(
		__in const FaceFile & FFace,
		__out Face & Face
		)
	{
		for (size_t i = 0; i < 3; i += 1)
			Face.Corners[ i ] = FFace.Corners[ i ];

		Face.Flags = FFace.Flags;
	}

	//
	// Magic Z value (possibly used to indicate a linkage point).
	//

	static const float LinkVertexZ;

private:

	Header    m_Header;

};

#endif
