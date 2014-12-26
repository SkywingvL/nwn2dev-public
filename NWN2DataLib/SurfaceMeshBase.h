/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	SurfaceMeshBase.h

Abstract:

	This module defines the base surface mesh class, which is inherited from by
	AreaSurfaceMesh and TileSurfaceMesh.

	The SurfaceMeshBase class defines the on-disk layout of many of the area
	surface walkmesh core primitives.

	Major portions of the TRX on-disk layout courtesy Tero Kivinen
	<kivinen@iki.fi>.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_SURFACEMESHBASE_H
#define _PROGRAMS_NWN2DATALIB_SURFACEMESHBASE_H

#ifdef _MSC_VER
#pragma once
#endif

//
// Define the base SurfaceMesh container class.
//

class SurfaceMeshBase
{

public:

	//
	// Define the surface mesh edge object, which represents an edge in the
	// walkmesh.
	//

	struct SurfaceMeshEdge
	{
		//
		// Points1, Points2 index into the Points table.
		//
		// Triangles1, Triangles2 index into the Triangles table, where those
		// edges are used.
		//
		// -1 indicates that an index isn't used.
		//

		unsigned long Points1;
		unsigned long Points2;
		unsigned long Triangles1;
		unsigned long Triangles2;
	};

#include <pshpack1.h>
	struct SurfaceMeshTriangle // SurfaceMeshFace
	{
		unsigned long  Corners[ 3 ];
		unsigned long  Edges[ 3 ];
		unsigned long  NeighborTriangles[ 3 ];
		NWN::Vector2   Centroid2;
		NWN::Vector3   Normal;
		float          D; // Dot product at plane
		unsigned short Island;
		unsigned short Flags; // SurfaceType

		enum SurfaceMeshFlags
		{
			WALKABLE  = 0x01,
			CLOCKWISE = 0x04, // Vertexes are wound clockwise and not CCW
			DIRT      = 0x08,
			GRASS     = 0x10,
			STONE     = 0x20,
			WOOD      = 0x40,
			CARPET    = 0x80,
			METAL     = 0x100,
			SWAMP     = 0x200,
			MUD       = 0x400,
			LEAVES    = 0x800,
			WATER     = 0x1000,
			PUDDLES   = 0x2000,

			LAST_FLAG
		};

		//
		// Other observed flags include:
		//
		// - 0x08 (seen outdoors on raised terrain, hills)
		// - 0x20 (seen both inside and outside
		// - 0x40 (seen inside sometimes in hallways)
		// 
	};
#include <poppack.h>

	typedef struct SurfaceMeshTriangle SurfaceMeshFace;
	typedef struct NWN::Vector3 SurfaceMeshPoint;

	C_ASSERT( sizeof( SurfaceMeshTriangle ) == 64 );

	typedef std::vector< NWN::Vector3 > PointVec;
	typedef std::vector< SurfaceMeshEdge > EdgeVec;
	typedef std::vector< SurfaceMeshTriangle > TriangleVec;

	inline
	SurfaceMeshBase(
		)
	{
		Clear( );
	}

	inline
	virtual
	~SurfaceMeshBase(
		)
	{
	}

	inline
	void
	Clear(
		)
	{
		m_Points.clear( );
		m_Edges.clear( );
		m_Triangles.clear( );

		m_MaxBound.x = -FLT_MAX;
		m_MaxBound.y = -FLT_MAX;
		m_MaxBound.z = -FLT_MAX;
		m_MinBound.x = +FLT_MAX;
		m_MinBound.y = +FLT_MAX;
		m_MinBound.z = +FLT_MAX;
	}

	inline
	void
	AddPoint(
		__in const NWN::Vector3 & v
		)
	{
		m_Points.push_back( v );
	}

	inline
	void
	AddEdge(
		__in const SurfaceMeshEdge & Edge
		)
	{
		m_Edges.push_back( Edge );
	}

	inline
	void
	AddTriangle(
		__in const SurfaceMeshTriangle & Triangle
		)
	{
		m_Triangles.push_back( Triangle );
	}


	//
	// Validate the walkmesh constructs after loading to ensure that all values
	// are sane.
	//

	inline
	void
	Validate(
		__in size_t IslandTableSize
		) const
	{
		for (EdgeVec::const_iterator it = m_Edges.begin( );
		     it != m_Edges.end( );
		     ++it)
		{
#if 0
			if ((it->Points1 != (unsigned long) -1) &&
				(it->Points1 >= m_Points.size( )))
			{
				throw std::runtime_error( "Illegal Points1" );
			}

			if ((it->Points2 != (unsigned long) -1) &&
				(it->Points2 >= m_Points.size( )))
			{
				throw std::runtime_error( "Illegal Points2" );
			}
#endif

			if ((it->Triangles1 != (unsigned long) -1) &&
				(it->Triangles1 >= m_Triangles.size( )))
			{
				throw std::runtime_error( "Illegal Triangles1" );
			}

			if ((it->Triangles2 != (unsigned long) -1) &&
				(it->Triangles2 >= m_Triangles.size( )))
			{
				throw std::runtime_error( "Illegal Triangle2" );
			}
		}

		for (TriangleVec::const_iterator it = m_Triangles.begin( );
		     it != m_Triangles.end( );
		     ++it)
		{
			for (unsigned long i = 0; i < 3; i += 1)
			{
				if (it->Corners[ i ] >= m_Points.size( ))
					throw std::runtime_error( "Illegal Triangle.Corners" );
			}

			for (unsigned long i = 0; i < 3; i += 1)
			{
				if (it->Edges[ i ] >= m_Edges.size( ))
					throw std::runtime_error( "Illegal Triangle.Edges" );
			}

			for (unsigned long i = 0; i < 3; i += 1)
			{
				if (it->NeighborTriangles[ i ] == (unsigned long) -1 )
					continue;

				if (it->NeighborTriangles[ i ] >= m_Triangles.size( ))
					throw std::runtime_error( "Illegal Triangle.NeighborTriangles" );
			}

			if (it->Island != 0xFFFF && it->Island >= IslandTableSize)
				throw std::runtime_error( "Illegal Triangle.Island" );
		}
	}


	inline
	const PointVec &
	GetPoints(
		) const
	{
		return m_Points;
	}

	inline
	const EdgeVec &
	GetEdges(
		) const
	{
		return m_Edges;
	}

	inline
	const TriangleVec &
	GetTriangles(
		) const
	{
		return m_Triangles;
	}

	//
	// Triangle search.
	//

	static
	bool
	IsPointInTriangle(
		__in const SurfaceMeshFace * Face,
		__in const NWN::Vector2 & pt,
		__in const PointVec & Points
		);

	//
	// Update bounding parameters.
	//

	inline
	void
	UpdateBoundingBox(
		__in const SurfaceMeshPoint * Pt
		)
	{
		if (Pt->x < m_MinBound.x)
			m_MinBound.x = Pt->x;
		if (Pt->y < m_MinBound.y)
			m_MinBound.y = Pt->y;
		if (Pt->z < m_MinBound.z)
			m_MinBound.z = Pt->z;
		if (Pt->x > m_MaxBound.x)
			m_MaxBound.x = Pt->x;
		if (Pt->y > m_MaxBound.y)
			m_MaxBound.y = Pt->y;
		if (Pt->z > m_MaxBound.z)
			m_MaxBound.z = Pt->z;
	}

	inline
	const NWN::Vector3 &
	GetMinBound(
		) const
	{
		return m_MinBound;
	}

	inline
	const NWN::Vector3 &
	GetMaxBound(
		) const
	{
		return m_MaxBound;
	}

private:

	PointVec           m_Points;
	EdgeVec            m_Edges;
	TriangleVec        m_Triangles;
	NWN::Vector3       m_MinBound;
	NWN::Vector3       m_MaxBound;

};

#endif
