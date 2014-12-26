/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	SkinMesh.h

Abstract:

	This module defines the SkinMesh class, which contains a SKIN-typed mesh
	that is stored in local coordinate form and forms the covering skin that is
	laid atop a model which contains a boned skeleton (i.e. a model that
	supports smooth translating animation).

--*/

#ifndef _PROGRAMS_NWN2DATALIB_SKINMESH_H
#define _PROGRAMS_NWN2DATALIB_SKINMESH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "SimpleMesh.h"
#include "MDBFormat.h"

//
// Define the skin mesh, which is used to render skin atop objects with bones, such
// as creatures.
//

//
// Define vertex and face types for skin mesh.
//

#include <pshpack1.h>
struct SMVertexFile
{
	NWN::Vector3  Position;
	NWN::Vector3  Normal;
	float         BoneWeights[ 4 ];
	unsigned char BoneIndicies[ 4 ];
	NWN::Vector3  Tangent;
	NWN::Vector3  Binormal;
	NWN::Vector3  uvw; // Texture vertex
	float         BoneCount;
};

C_ASSERT( sizeof( SMVertexFile ) == 15 * 4 + 4 * 4 + 4 + 4 );

struct SMFaceFile
{
	unsigned short Corners[ 3 ];
};

C_ASSERT( sizeof( SMFaceFile ) == 3 * 2 );

#include <poppack.h>

struct SMVertex
{
	NWN::Vector3  LocalPos;
	NWN::Vector3  Normal;
	float         BoneWeights[ 4 ];
	unsigned long BoneIndicies[ 4 ];
	NWN::Vector3  Tangent;
	NWN::Vector3  Binormal;
	NWN::Vector3  uvw; // Texture vertex
	unsigned long BoneCount;
//	NWN::Vector3  Pos; // Current position, calculated (not in file)
};

struct SMFace
{
	unsigned long  Corners[ 3 ];
//	NWN::Vector3   Normal;
};

extern const SimpleMeshTypeDescriptor SMTD_SkinMesh;

//
// Define the skin mesh core itself.
//

class SkinMesh : public SimpleMesh< SMVertex, SMFace, &SMTD_SkinMesh, CoordTransModeLocalWeighted, 4 >
{

public:

	typedef ::SMVertex Vertex;
	typedef ::SMVertexFile VertexFile;
	typedef ::SMFace Face;
	typedef ::SMFaceFile FaceFile;

	typedef unsigned long FaceVertexIndex; // Must match SMFace::Corners

	typedef SimpleMesh< Vertex, Face, &SMTD_SkinMesh, CoordTransModeLocalWeighted, 4 > BaseMesh;

#include <pshpack1.h>

	struct SkinHeader
	{
		NWN::ResRef32  Name;
		NWN::ResRef32  Skeleton;
		MODEL_MATERIAL Material;
		unsigned long  NumVerts;
		unsigned long  NumFaces;
	};

	C_ASSERT( sizeof( SkinHeader ) == 64 + sizeof( MODEL_MATERIAL ) + 8 );

#include <poppack.h>

	typedef SkinHeader Header;

	inline
	SkinMesh(
		)
	: SimpleMesh< SMVertex, SMFace, &SMTD_SkinMesh, CoordTransModeLocalWeighted, 4 >( )
	{
		ZeroMemory( &m_Header, sizeof( m_Header ) );
	}

	inline
	virtual
	~SkinMesh(
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

	//
	// Vertex weighting.
	//

	inline
	float
	GetVertexWeight(
		__in PointIndex PointId,
		__in unsigned long WeightId
		) const
	{
		return GetPoint( PointId ).BoneWeights[ WeightId ];
	}

	inline
	unsigned long
	GetVertexBone(
		__in PointIndex PointId,
		__in unsigned long WeightId
		) const
	{
		return GetPoint( PointId ).BoneIndicies[ WeightId ];
	}

	inline
	unsigned long
	GetVertexWeightCount(
		__in PointIndex PointId
		) const
	{
		return GetPoint( PointId ).BoneCount;
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

		for (size_t i = 0; i < 4; i += 1)
			Vert.BoneWeights[ i ] = FVert.BoneWeights[ i ];
		for (size_t i = 0; i < 4; i += 1)
		{
			Vert.BoneIndicies[ i ] = FVert.BoneIndicies[ i ];

			if (Vert.BoneIndicies[ i ] == UCHAR_MAX)
				Vert.BoneIndicies[ i ] = ULONG_MAX;
		}

		Vert.Tangent   = FVert.Tangent;
		Vert.Binormal  = FVert.Binormal;
		Vert.uvw       = FVert.uvw;
		Vert.BoneCount = (unsigned long) FVert.BoneCount;
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

	inline
	void
	Validate(
		) const
	{
		BaseMesh::Validate( );

		for (VertexVec::const_iterator it = GetPoints( ).begin( );
		     it != GetPoints( ).end( );
		     ++it)
		{
			if (it->BoneCount > NumVertexWeights)
				throw std::runtime_error( "Illegal BoneCount" );
		}
	}

private:

	Header    m_Header;

};

#endif
