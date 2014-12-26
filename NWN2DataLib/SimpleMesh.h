/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	SimpleMesh.h

Abstract:

	This module defines the base SimpleMesh class, which acts as a base class
	for all MDB-based model meshes.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_SIMPLEMESH_H
#define _PROGRAMS_NWN2DATALIB_SIMPLEMESH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "MeshLinkage.h"

class MeshManager;

//
// Define the mesh type descriptor used to encode additional type data for the
// derived class.
//

struct SimpleMeshTypeDescriptor
{
	const MeshLinkageTraits * LinkageTraits;
};

enum CoordTransMode
{
	CoordTransModeWorld,
	CoordTransModeLocal,
	CoordTransModeLocalWeighted
};

//
// Define the base class for simple model (MDB) meshes.  The simple mesh allows
// verticies to be transformed to match a world transformation matrix.
//
// Vertex types must include a 'Pos' and a 'LocalPos' member for this purpose,
// and Face types must include a 'Normal' member.  Face types must reference
// verticies by index (via a three-element Corners integral array).
//
// Derived classes should include a 'Header' type which is accessed via a
// 'GetHeader' API.  They must also include typedefs for on-disk and in-memory
// vertex and face types (Vertex, VertexFile, Face, FaceFile).
//
// Derived classes should include a 'BaseMesh' type that refers back to the
// base template class with the template parameters used.
//

template<
	typename VertexT,                           // In-memory vertex type
	typename FaceT,                             // In-memory face type
	const SimpleMeshTypeDescriptor * MeshType,  // Derived mesh type descriptor
	CoordTransMode TMode = CoordTransModeLocal, // Coordinates stored in local form?
	size_t   NumWeights  = 1,                   // Number of vertex weights
	typename PointIndexT = unsigned long,       // In-memory vertex index type
	typename FaceIndexT  = unsigned long >      // In-memory face index type
class SimpleMesh
{

public:

	typedef SimpleMesh< VertexT, FaceT, MeshType, TMode, NumWeights, PointIndexT, FaceIndexT > SimpleMeshT;

	typedef std::vector< VertexT > VertexVec;
	typedef std::vector< FaceT > FaceVec;

	typedef PointIndexT PointIndex;
	typedef FaceIndexT FaceIndex;

	static const CoordTransMode CoordTrans       = TMode;
	static const size_t         NumVertexWeights = NumWeights;

	inline
	SimpleMesh(
		)
	: m_MeshLinkage( MeshType->LinkageTraits )
	{
	}

	inline
	~SimpleMesh(
		)
	{
	}

	inline
	void
	Clear(
		)
	{
		m_Faces.clear( );
		m_Points.clear( );
	}

	inline
	const FaceVec &
	GetFaces(
		) const
	{
		return m_Faces;
	}

	inline
	FaceVec &
	GetFaces(
		)
	{
		return m_Faces;
	}

	inline
	const VertexVec &
	GetPoints(
		) const
	{
		return m_Points;
	}

	inline
	VertexVec &
	GetPoints(
		)
	{
		return m_Points;
	}

	inline
	const VertexT &
	GetPoint(
		__in PointIndexT PointId
		) const
	{
		return m_Points[ PointId ];
	}

	//
	// Return a Vector3 in the default coordinate space.  Some meshes, such as
	// collision meshes, always use world space; other meshes typically use
	// local space.
	//

	inline
	NWN::Vector3
	GetPoint3(
		__in PointIndexT PointId
		) const
	{
		const VertexT & Point( GetPoint( PointId ) );
		NWN::Vector3    v;

		v.x = Point.LocalPos.x;
		v.y = Point.LocalPos.y;
		v.z = Point.LocalPos.z;

		return v;
	}

	//
	// Return a Vector3 always in local space, even if the default GetPoint3
	// policy is to use world space.
	//

	inline
	NWN::Vector3
	GetLocalPoint3(
		__in PointIndexT PointId
		) const
	{
		const VertexT & Point( GetPoint( PointId ) );
		NWN::Vector3    v;

		v.x = Point.LocalPos.x;
		v.y = Point.LocalPos.y;
		v.z = Point.LocalPos.z;

		return v;
	}

	//
	// Vertex weighting.
	//

	inline
	float
	GetVertexWeight(
		__in PointIndexT PointId,
		__in unsigned long WeightId
		) const
	{
		UNREFERENCED_PARAMETER( PointId );
		UNREFERENCED_PARAMETER( WeightId );

		return 0.0f;
	}

	inline
	unsigned long
	GetVertexBone(
		__in PointIndexT PointId,
		__in unsigned long WeightId
		) const
	{
		UNREFERENCED_PARAMETER( PointId );
		UNREFERENCED_PARAMETER( WeightId );

		return ULONG_MAX;
	}

	inline
	unsigned long
	GetVertexWeightCount(
		__in PointIndexT PointId
		) const
	{
		UNREFERENCED_PARAMETER( PointId );

		return 1;
	}

	inline
	const FaceT &
	GetFace(
		__in FaceIndexT FaceId
		) const
	{
		return m_Faces[ FaceId ];
	}

	inline
	void
	AddPoint(
		__in const VertexT & Point
		)
	{
		m_Points.push_back( Point );
	}

	inline
	void
	AddFace(
		__in const FaceT & Face
		)
	{
		m_Faces.push_back( Face );
	}

	//
	// Transform the mesh against a 4x4 matrix.
	//

//	void
//	Update(
//		__in const NWN::Matrix44 & M
//		);

	//
	// Validate that the mesh is legal.
	//

	inline
	void
	Validate(
		) const
	{
		for (FaceVec::const_iterator it = m_Faces.begin( );
		     it != m_Faces.end( );
		     ++it)
		{
			for (size_t i = 0; i < 3; i += 1)
			{
				if (it->Corners[ i ] >= m_Points.size( ))
					throw std::runtime_error( "Illegal Face.Corners" );
			}
		}
	}

	//
	// Copy points (without copying the mesh linkage).
	//

	inline
	void
	CopyMeshDataTo(
		__out SimpleMeshT & Mesh
		) const
	{
		Mesh.m_Faces  = m_Faces;
		Mesh.m_Points = m_Points;
	}

	//
	// Precompute data after loading the mesh.
	//

//	void
//	Precalculate(
//		);

private:

	FaceVec     m_Faces;
	VertexVec   m_Points;

public: // Internal use only

	DECLARE_MESH_LINKAGE;

private:

	friend class MeshManager;

};

#endif
