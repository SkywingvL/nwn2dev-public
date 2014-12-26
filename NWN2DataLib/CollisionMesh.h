/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	CollisionMesh.h

Abstract:

	This module defines the CollisionMesh class, which contains a COL2/COL3
	typed mesh that is stored in world coordinate form.  The collision mesh is
	used for ray intersections with a model.

	A model may have a COL2 (C2) and COL3 (C3) mesh, or any combination thereof
	though a single model only supports one of each type of mesh.  Both C2 and
	C3 meshes have identical on-disk and in-memory layouts supported by the
	CollisionMesh class; C2 meshes are a coarse-grained mesh (rejecting
	backfaces) around the model, and C3 meshes are a fine-grained mesh around
	the model (typically with twice or more as many faces as a C2 mesh).

	CollisionMesh objects also support updating a set of bounding coordinates
	that may be used to box the entire (world-transformed) mesh for a further
	improvement in collision performance (optimized for the non-colliding
	case).

--*/

#ifndef _PROGRAMS_NWN2DATALIB_COLLISIONMESH_H
#define _PROGRAMS_NWN2DATALIB_COLLISIONMESH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "SimpleMesh.h"
#include "MDBFormat.h"

//
// Define the collision mesh, used for fine-grained collision testing, e.g. for
// line of sight checks.
//

//
// Define vertex and face types for collision mesh.
//

#include <pshpack1.h>
struct CMVertexFile
{
	NWN::Vector3 Position;
	NWN::Vector3 Normal;
	NWN::Vector3 uvw; // Texture vertex
};

C_ASSERT( sizeof( CMVertexFile ) == 9 * 4 );

struct CMFaceFile
{
	unsigned short Corners[ 3 ];
};

C_ASSERT( sizeof( CMFaceFile ) == 3 * 2 );

#include <poppack.h>

struct CMVertex
{
	NWN::Vector3 LocalPos;
	NWN::Vector3 Normal;
	NWN::Vector3 uvw; // Texture vertex
	NWN::Vector3 Pos; // Current position, calculated (not in file)
};

struct CMFace
{
	unsigned long  Corners[ 3 ];
	NWN::Vector3   Normal;
};

extern const SimpleMeshTypeDescriptor SMTD_CollisionMesh;

//
// Define the collision mesh core itself.
//

class CollisionMesh : public SimpleMesh< CMVertex, CMFace, &SMTD_CollisionMesh, CoordTransModeWorld >
{

public:

	typedef ::CMVertex Vertex;
	typedef ::CMVertexFile VertexFile;
	typedef ::CMFace Face;
	typedef ::CMFaceFile FaceFile;

	typedef SimpleMesh< Vertex, Face, &SMTD_CollisionMesh, CoordTransModeWorld > BaseMesh;

#include <pshpack1.h>

	struct CollisionHeader
	{
		NWN::ResRef32  Name;
		MODEL_MATERIAL Material;
		unsigned long  NumVerts;
		unsigned long  NumFaces;
	};

	C_ASSERT( sizeof( CollisionHeader ) == 32 + sizeof( MODEL_MATERIAL ) + 8 );

#include <poppack.h>

	typedef CollisionHeader Header;

	inline
	CollisionMesh(
		)
	: SimpleMesh< CMVertex, CMFace, &SMTD_CollisionMesh, CoordTransModeWorld >( )
	{
		ZeroMemory( &m_Header, sizeof( m_Header ) );
	}

	inline
	virtual
	~CollisionMesh(
		)
	{
	}

	//
	// Copy constructor and copy assignment.  The CollisionMesh can be freely
	// copied but the associated mesh data is not copied with, as the mesh is
	// considered a new instance (whereas for the other meshes, the instances
	// all have the same underlying data).
	//

	inline
	CollisionMesh(
		__in const CollisionMesh & other
		)
	{
		m_Header = other.m_Header;

		other.CopyMeshDataTo( *this );

		SetAssociatedMesh( NULL );
	}

	inline
	CollisionMesh &
	operator=(
		__in const CollisionMesh & other
		)
	{
		m_Header = other.m_Header;

		other.CopyMeshDataTo( *this );

		SetAssociatedMesh( NULL );

		return *this;
	}

	//
	// Transform the mesh against a 4x4 matrix.
	//

	void
	Update(
		__in const NWN::Matrix44 & M
		);

	//
	// Precompute data after loading the mesh.
	//

	void
	Precalculate(
		);

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

	//
	// Return a Vector3 in the default coordinate space.  Some meshes, such as
	// collision meshes, always use world space; other meshes typically use
	// local space.
	//

	inline
	NWN::Vector3
	GetPoint3(
		__in PointIndex PointId
		) const
	{
		const Vertex  & Point( GetPoint( PointId ) );
		NWN::Vector3    v;

		v.x = Point.Pos.x;
		v.y = Point.Pos.y;
		v.z = Point.Pos.z;

		return v;
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
		Vert.Normal    = FVert.Normal;
		Vert.uvw       = FVert.uvw;
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
	}

	//
	// Update bounding parameters.
	//

	inline
	void
	UpdateBoundingBox(
		__inout NWN::Vector3 & MinBound,
		__inout NWN::Vector3 & MaxBound
		)
	{
		for (FaceVec::const_iterator it = GetFaces( ).begin( );
		     it != GetFaces( ).end( );
		     ++it)
		{
			for (size_t i = 0; i < 3; i += 1)
			{
				NWN::Vector3 Point3 = GetPoint3( it->Corners[ i ] );

				if (Point3.x < MinBound.x)
					MinBound.x = Point3.x;
				if (Point3.y < MinBound.y)
					MinBound.y = Point3.y;
				if (Point3.z < MinBound.z)
					MinBound.z = Point3.z;
				if (Point3.x > MaxBound.x)
					MaxBound.x = Point3.x;
				if (Point3.y > MaxBound.y)
					MaxBound.y = Point3.y;
				if (Point3.z > MaxBound.z)
					MaxBound.z = Point3.z;
			}
		}
	}

private:

	Header    m_Header;

};

#endif
