/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	AreaSurfaceMesh.h

Abstract:

	This module defines the AreaSurfaceMesh class, which stores surface mesh
	and pathing table constructs for a walkable area.  Additionally, routines
	to manage pathing table traversal, straight path checking, and other
	pathing related functions.

	This module also defines the AreaSurfaceMesh::TileSurfaceMesh class, which
	is used to divide up pathing table data into a uniform grid.

	Major portions of the TRX on-disk layout courtesy Tero Kivinen
	<kivinen@iki.fi>.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_AREASURFACEMESH_H
#define _PROGRAMS_NWN2DATALIB_AREASURFACEMESH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "SurfaceMeshBase.h"
#include "MeshLinkage.h"

class MeshManager;
struct IDebugTextOut;
typedef std::vector< NWN::Vector2 > Vector2Vec;

extern const MeshLinkageTraits MLT_AreaSurfaceMesh;

//
// Define the AreaSurfaceMesh object which contains walkmesh data.
//

class AreaSurfaceMesh : public SurfaceMeshBase
{

public:

	inline
	AreaSurfaceMesh(
		__in_opt IDebugTextOut * TextWriter = NULL
		)
		: m_TextWriter( TextWriter ),
		  m_MeshLinkage( &MLT_AreaSurfaceMesh )
	{
		Clear( );
	}

	inline
	~AreaSurfaceMesh(
		)
	{
	}

#include <pshpack1.h>
	struct TileSurfaceMeshHeader
	{
		NWN::ResRef32 Name;
		unsigned char OwnsData; // m_bOwnsData
		unsigned long NumVerticies;
		unsigned long NumEdges;
		unsigned long NumFaces; // NumberOfTriangles
		float         XSize;
		float         YSize;
		unsigned long FaceOffset; // Version 0x6C or later
	};

	C_ASSERT( sizeof( TileSurfaceMeshHeader ) == 57 );

	struct PathTableHeader
	{
		unsigned long Flags;
		unsigned long LocalCount; // TileTriangles
		unsigned char NodeCount; // ... WalkableTriangles
		unsigned long RLETableSize;
	};

	C_ASSERT( sizeof( PathTableHeader ) == 13 );

	struct IslandHeader
	{
		unsigned long Index;
		unsigned long Tile;
		NWN::Vector3  Centroid;
		unsigned long FaceCount;
	};

	C_ASSERT( sizeof( IslandHeader ) == 24 );

#include <poppack.h>

	struct IslandPathNode
	{
		unsigned short m_Next;
		unsigned short __padding;
		float          m_Weight;
	};

	C_ASSERT( sizeof( IslandPathNode ) == 8 );

	typedef unsigned char PathNode;

	typedef std::vector< unsigned char > LocalToNodeIndexVec;
	typedef std::vector< unsigned long > NodeToLocalIndexVec;
	typedef std::vector< PathNode > PathNodeVec;

	struct PathTable
	{
		PathTableHeader     m_Header;
		LocalToNodeIndexVec m_LocalToNodeIndex; // Table1
		NodeToLocalIndexVec m_NodeToLocalIndex; // Table2
		PathNodeVec         m_PathNodes; // Table3 Node*Node

		//
		// LocalToNodeIndex : Tables in walkmesh list (0xff skip)
		// NodeToLocalIndex : Table of indicies to Table3
		// PathNodes        : From triangle i -> triangle j at
		//                    [i][j].  0x80 indicates that the
		//                    tile is directly attached.
		//


		enum PathTableFlags
		{
			PATHFLAG_RLE        = 0x01,
			PATHFLAG_ZCOMPRESS  = 0x02,

			PATHFLAG_COMPRESSED = PATHFLAG_RLE | PATHFLAG_ZCOMPRESS,

			LASTPATHFLAG
		};

		//
		// This flag indicates that there is direct line of sight visibility
		// across all points from the previous triangle to the current
		// triangle.
		//
		// If this flag is set, it is not necessary to compute line
		// intersection visibility along this path; all points are directly
		// reachable.
		//
		// If this flag is not set, then there may still be a direct path, but
		// line intersections are required (e.g. a triangle against a corner).
		//

		enum PathNodeFlags
		{
			VISIBLE           = 0x80
		};

		inline
		void
		Clear(
			)
		{
			m_LocalToNodeIndex.clear( );
			m_NodeToLocalIndex.clear( );
			m_PathNodes.clear( );
		}

		//
		// Returns whether there is a clear line of sight between FromFace and
		// ToFace.
		//

		inline
		bool
		GetVisible(
			__in unsigned long FromFace,
			__in unsigned long ToFace
			) const
		{
			unsigned char NodeIndex1;
			unsigned char NodeIndex2;
			PathNode      Node;

			if (FromFace >= m_LocalToNodeIndex.size( ))
				return false;
			
			NodeIndex1 = m_LocalToNodeIndex[ FromFace ];

			if (NodeIndex1 == 0xFF)
				return false;

			if (ToFace >= m_LocalToNodeIndex.size( ))
				return false;

			NodeIndex2 = m_LocalToNodeIndex[ ToFace ];

			if (NodeIndex2 == 0xFF)
				return false;

			Node = m_PathNodes[ m_Header.NodeCount * NodeIndex1 + NodeIndex2 ];

			if (!(Node & VISIBLE))
				return false;

			if (Node == 0xFF)
				return false;

			return true;
		}

		//
		// Consult the local node pathing table to determine the next face to
		// travel to in order to get from FromFace to ToFace.
		//
		// The returned face will be directly adjacent to FromFace.
		//

		inline
		unsigned long
		GetNext(
			__in unsigned long FromFace,
			__in unsigned long ToFace
			) const
		{
			unsigned char NodeIndex1;
			unsigned char NodeIndex2;
			PathNode      Node;

			if (FromFace >= m_LocalToNodeIndex.size( ))
				return 0xFFFFFFFF;
			
			NodeIndex1 = m_LocalToNodeIndex[ FromFace ];

			if (NodeIndex1 == 0xFF)
				return 0xFFFFFFFF;

			if (ToFace >= m_LocalToNodeIndex.size( ))
				return 0xFFFFFFFF;

			NodeIndex2 = m_LocalToNodeIndex[ ToFace ];

			if (NodeIndex2 == 0xFF)
				return 0xFFFFFFFF;

			Node = m_PathNodes[ m_Header.NodeCount * NodeIndex1 + NodeIndex2 ];

			if (m_NodeToLocalIndex.empty( ))
			{
				if (Node == 0xFF)
					return 0xFFFFFFFF; // BUGBUG: NWN2 returns 0xFF
				else
					return 0xFFFFFFFF; // BUGBUG: NWN2 uses Node & ~VISIBLE
			}

			if (Node == 0xFF)
				return 0xFFFFFFFF;

			return m_NodeToLocalIndex[ Node & ~VISIBLE ];
		}

		//
		// Validate that all table indicies are in range so that they may be
		// used without bounds checking later.
		//

		inline
		void
		Validate(
			__in size_t NumFaces
			) const
		{
			if (m_PathNodes.size( ) != (size_t) (m_Header.NodeCount * m_Header.NodeCount))
				throw std::runtime_error( "Illegal PathNodes size" );

			if (m_NodeToLocalIndex.size( ) != m_Header.NodeCount)
				throw std::runtime_error( "Illegal NodeToLocalIndex size" );

			if (m_LocalToNodeIndex.size( ) != m_Header.LocalCount)
				throw std::runtime_error( "Illegal LocalToNodeIndex size" );

			if (m_NodeToLocalIndex.size( ) < 0x7F)
			{
				for (PathNodeVec::const_iterator it = m_PathNodes.begin( );
				     it != m_PathNodes.end( );
				     ++it)
				{
					if (*it == 0xFF)
						continue;

					if ((size_t) (*it & ~VISIBLE) >= m_NodeToLocalIndex.size( ))
						throw std::runtime_error( "Illegal PathNode" );
				}
			}

			if (m_PathNodes.size( ) < 0xFF)
			{
				for (LocalToNodeIndexVec::const_iterator it = m_LocalToNodeIndex.begin( );
				     it != m_LocalToNodeIndex.end( );
				     ++it)
				{
					if (*it == 0xFF)
						continue;

					if (*it >= m_PathNodes.size( ))
						throw std::runtime_error( "Illegal LocalToNodeIndex" );
				}
			}

			for (NodeToLocalIndexVec::const_iterator it = m_NodeToLocalIndex.begin( );
			     it != m_NodeToLocalIndex.end( );
			     ++it)
			{
				if (*it >= NumFaces)
					throw std::runtime_error( "Illegal NodeToLocalIndex" );
			}
		}
	};

	typedef std::vector< unsigned long > AdjacentVec;
	typedef std::vector< float > AdjacentDistVec;
	typedef std::vector< unsigned long > FaceIndexVec;

	struct Island
	{
		IslandHeader    m_Header;
		AdjacentVec     m_Adjacent;
		AdjacentDistVec m_AdjacentDist;
		FaceIndexVec    m_ExitFace;

		inline
		void
		Clear(
			)
		{
			m_Adjacent.clear( );
			m_AdjacentDist.clear( );
			m_ExitFace.clear( );
		}

		inline
		const AdjacentVec &
		GetAdjacent(
			) const
		{
			return m_Adjacent;
		}

		inline
		const AdjacentDistVec &
		GetAdjacentDist(
			) const
		{
			return m_AdjacentDist;
		}

		inline
		const FaceIndexVec &
		GetExitFace(
			) const
		{
			return m_ExitFace;
		}

		inline
		void
		AddAdjacent(
			__in unsigned long Adjacent
			)
		{
			m_Adjacent.push_back( Adjacent );
		}

		inline
		void
		AddAdjacentDist(
			__in float AdjacentDist
			)
		{
			m_AdjacentDist.push_back( AdjacentDist );
		}

		inline
		void
		AddExitFace(
			__in unsigned long ExitFace
			)
		{
			m_ExitFace.push_back( ExitFace );
		}

		inline
		void
		Validate(
			__in size_t FaceListSize,
			__in size_t IslandTableSize
			) const
		{
			for (FaceIndexVec::const_iterator it = m_ExitFace.begin( );
			     it != m_ExitFace.end( );
			     ++it)
			{
				if (*it >= FaceListSize)
					throw std::runtime_error( "Illegal Island.ExitFace" );
			}

			if (m_Adjacent.size( ) != m_AdjacentDist.size( ))
				throw std::runtime_error( "Illegal Island.AdjacentDist size" );

			for (AdjacentVec::const_iterator it = m_Adjacent.begin( );
			     it != m_Adjacent.end( );
			     ++it)
			{
				if (*it == 0xFFFF)
					continue;

				if (*it >= IslandTableSize)
					throw std::runtime_error( "Illegal Island.Adjacent" );
			}

			if (m_ExitFace.size( ) != m_Adjacent.size( ))
				throw std::runtime_error( "Illegal Island.ExitFace size" );
		}
	};

	typedef std::vector< Island > IslandVec;

	struct TileSurfaceMesh : public SurfaceMeshBase
	{
		TileSurfaceMeshHeader   m_Header;
		PathTable               m_PathTable;
		const SurfaceMeshFace * m_Faces;
		unsigned long           m_FaceOffset;
		unsigned long           m_NumFaces;
		unsigned long           m_Flags;

		inline
		void
		Clear(
			)
		{
			SurfaceMeshBase::Clear( );
			m_PathTable.Clear( );
		}

		//
		// Validate the walkmesh constructs after loading to ensure that all values
		// are sane.
		//

		inline
		void
		Validate(
			__in size_t IslandTableSize,
			__in size_t FaceTableSize
			) const
		{
			SurfaceMeshBase::Validate( IslandTableSize );

			if (m_FaceOffset + m_NumFaces < m_FaceOffset)
				throw std::runtime_error( "Too many faces(1)." );

			if (m_FaceOffset + m_NumFaces > FaceTableSize)
				throw std::runtime_error( "Too many faces(2)." );

			m_PathTable.Validate( m_NumFaces );
		}

		//
		// Locate which triangle contains a particular point.  Returns NULL on
		// failure.
		//

		const SurfaceMeshFace *
		FindFace(
			__in const NWN::Vector2 & pt,
			__in const PointVec & Points
			) const;

		inline
		const SurfaceMeshFace *
		FindFace(
			__in const NWN::Vector2 & pt,
			__in const AreaSurfaceMesh * SurfaceMesh
			) const
		{
			return FindFace( pt, SurfaceMesh->GetPoints( ) );
		}

		//
		// Return the local face index of a face, which must have been returned
		// from this pathing tile's face list.
		//

		inline
		unsigned long
		GetFaceId(
			__in const SurfaceMeshFace * Face
			) const
		{
			return (unsigned long) (Face - m_Faces);
		}

		//
		// Retrieve a face by local face id.
		//

		inline
		const SurfaceMeshFace *
		GetFace(
			__in unsigned long LocalFaceId
			) const
		{
			return &m_Faces[ LocalFaceId ];
		}

		//
		// Calculate whether a straight line exists between two points, using
		// fine-grained pathing.
		//

		bool
		StraightPathExists(
			__in const NWN::Vector2 & Start,
			__in const NWN::Vector2 & End,
			__in const SurfaceMeshFace * Face1,
			__in const SurfaceMeshFace * Face2,
			__in const AreaSurfaceMesh * SurfaceMesh,
			__out_opt const SurfaceMeshFace * * LastFace = NULL
			) const;


		//
		// Calculate whether a surface mesh contact exists along a ray.
		//

		bool
		CalcContact(
			__in const NWN::Vector3 & Origin,
			__in const NWN::Vector3 & NormDir,
			__in const SurfaceMeshFace * Face1,
			__in const SurfaceMeshFace * Face2,
			__in const AreaSurfaceMesh * SurfaceMesh,
			__out float & IntersectDistance,
			__out_opt const SurfaceMeshFace * * IntersectFace
			) const;

	};

	typedef std::vector< unsigned long > WMTable1Vec;
	typedef std::vector< unsigned long > WMTable2Vec;
	typedef std::vector< TileSurfaceMesh > TileSurfaceMeshVec;
	typedef std::vector< unsigned short> IslandIdVec;
	typedef std::vector< IslandPathNode > IslandPathNodeVec;

	inline
	void
	Clear(
		)
	{
		SurfaceMeshBase::Clear( );
		m_TileSurfaceMeshes.clear( );
		m_Islands.clear( );
		m_IslandPathTable.clear( );

		m_TileGridHeight = 0;
		m_TileGridWidth  = 0;
		m_TileBorderSize = 0;
		m_Flags          = 0;
		m_TileSize       = 1.0f;
	}

	inline
	void
	AddTileSurfaceMesh(
		__in const TileSurfaceMesh & Mesh
		)
	{
		m_TileSurfaceMeshes.push_back( Mesh );
	}

	inline
	void
	AddIsland(
		__in const Island & Isle
		)
	{
		m_Islands.push_back( Isle );
	}

	inline
	unsigned long
	GetTileGridHeight(
		) const
	{
		return m_TileGridHeight;
	}

	inline
	void
	SetTileGridHeight(
		__in unsigned long TileGridHeight
		)
	{
		m_TileGridHeight = TileGridHeight;
	}

	inline
	unsigned long
	GetTileGridWidth(
		) const
	{
		return m_TileGridWidth;
	}

	inline
	void
	SetTileGridWidth(
		__in unsigned long TileGridWidth
		)
	{
		m_TileGridWidth = TileGridWidth;
	}

	inline
	unsigned long
	GetTileBorderSize(
		) const
	{
		return m_TileBorderSize;
	}

	inline
	void
	SetTileBorderSize(
		__in unsigned long TileBorderSize
		)
	{
		m_TileBorderSize = TileBorderSize;
	}

	inline
	unsigned long
	GetFlags(
		) const
	{
		return m_Flags;
	}

	inline
	void
	SetFlags(
		__in unsigned long Flags
		)
	{
		m_Flags = Flags;
	}

	inline
	float
	GetTileSize(
		) const
	{
		return m_TileSize;
	}

	inline
	void
	SetTileSize(
		__in float TileSize
		)
	{
		m_TileSize = TileSize;
	}

	inline
	const IslandPathNodeVec &
	GetIslandPathTable(
		) const
	{
		return m_IslandPathTable;
	}

	inline
	IslandPathNodeVec &
	GetIslandPathTable(
		)
	{
		return m_IslandPathTable;
	}

	inline
	float
	GetTileGridTileSize(
		) const
	{
		return m_TileSize;
	}

	//
	// Validate the walkmesh constructs after loading to ensure that all values
	// are sane.
	//

	inline
	void
	Validate(
		) const
	{
		SurfaceMeshBase::Validate( m_Islands.size( ) );

		if (m_TileGridHeight * m_TileGridWidth != m_TileSurfaceMeshes.size( ))
			throw std::runtime_error( "Incomplete TileSurfaceMesh table" );

		if (m_TileSize <= 0.0f)
			throw std::runtime_error( "Illegal TileSize" );

		//
		// Validate each tile surface mesh in turn.
		//

		for (TileSurfaceMeshVec::const_iterator it = m_TileSurfaceMeshes.begin( );
		     it != m_TileSurfaceMeshes.end( );
		     ++it)
		{
			it->Validate( m_Islands.size( ), GetTriangles( ).size( ) );
		}

		if (m_IslandPathTable.size( ) != m_Islands.size( ) * m_Islands.size( ))
			throw std::runtime_error( "Incomplete island path table" );

		//
		// Ensure that all islands are valid.
		//

		for (IslandVec::const_iterator it = m_Islands.begin( );
		     it != m_Islands.end( );
		     ++it)
		{
			it->Validate( GetTriangles( ).size( ), m_Islands.size( ) );
		}

		//
		// Ensure that island-to-island pathing is legal.
		//

		for (IslandPathNodeVec::const_iterator it = m_IslandPathTable.begin( );
		     it != m_IslandPathTable.end( );
		     ++it)
		{
			if (it->m_Next == 0xFFFF)
				continue;

			if (it->m_Next >= m_Islands.size( ))
				throw std::runtime_error( "Illegal IslandPathTable entry" );
		}
	}

	inline
	const TileSurfaceMeshVec &
	GetTileSurfaceMeshes(
		) const
	{
		return m_TileSurfaceMeshes;
	}

	inline
	TileSurfaceMeshVec &
	GetTileSurfaceMeshes(
		)
	{
		return m_TileSurfaceMeshes;
	}

	inline
	const IslandVec &
	GetIslands(
		) const
	{
		return m_Islands;
	}

	//
	// Return the tile surface mesh for a particular region based on the grid
	// coordinates.
	//

	inline
	const TileSurfaceMesh &
	GetTileSurfaceMesh(
		__in unsigned long x,
		__in unsigned long y
		) const
	{
		size_t Offset;

		if ((x >= m_TileGridWidth) || (y >= m_TileGridHeight))
			throw std::runtime_error( "Illegal Tile X/Y" );

		Offset = y * m_TileGridWidth + x;

		return m_TileSurfaceMeshes[ Offset ];
	}

	//
	// Return the tile surface mesh for a raw offset.
	//

	inline
	const TileSurfaceMesh &
	GetTileSurfaceMesh(
		__in unsigned long Offset
		) const
	{
		if (Offset >= m_TileSurfaceMeshes.size( ))
			throw std::runtime_error( "Illegal TileSurfaceMesh offset" );

		return m_TileSurfaceMeshes[ Offset ];
	}

	//
	// Return the containing tile surface mesh for a particular triangle.
	//

	inline
	const TileSurfaceMesh &
	GetTileSurfaceMesh(
		__in const SurfaceMeshFace * Face
		) const
	{
		unsigned long FaceId;

		FaceId = (unsigned long) (Face - &GetTriangles( )[ 0 ]);

		for (TileSurfaceMeshVec::const_iterator it = m_TileSurfaceMeshes.begin( );
		     it != m_TileSurfaceMeshes.end( );
		     ++it)
		{
			if ((it->m_FaceOffset <= FaceId) &&
			    (it->m_FaceOffset + it->m_NumFaces > FaceId))
			{
				return *it;
			}
		}

		throw std::runtime_error( "Illegal Face for GetTileSurfaceMesh" );
	}

	//
	// Check whether an X/Y coordinate set are within the game board defined by
	// the tile surface mesh checkerboard.
	//

	inline
	bool
	IsPointInTileSurfaceMeshGrid(
		__in const NWN::Vector2 & pt
		) const
	{
		const unsigned int FixedPointShift = 1 << 6;
		unsigned int       x;
		unsigned int       y;
		unsigned int       TileSize;

		if ((pt.x < 0) || (pt.y < 0))
			return false;

		//
		// First, figure out which subdivided tile surface mesh we're in.
		//

		x = (unsigned int) (pt.x * FixedPointShift);
		y = (unsigned int) (pt.y * FixedPointShift);

		TileSize = (unsigned int) (m_TileSize * FixedPointShift);

		x /= TileSize;
		y /= TileSize;

		if ((x >= m_TileGridWidth) || (y >= m_TileGridHeight))
			return false;

		return true;
	}

	//
	// Locate which triangle contains a particular point.  Returns NULL on
	// failure.
	//

	inline
	const SurfaceMeshFace *
	FindFace(
		__in const NWN::Vector2 & pt,
		__out_opt const TileSurfaceMesh * * SurfaceMesh = NULL
		) const
	{
		unsigned long      x;
		unsigned long      y;
		unsigned long      TileSize;
		const unsigned int FixedPointShift = 1 << 6;

		//
		// First, figure out which subdivided tile surface mesh we're in.
		//

		x = (unsigned int) (pt.x * FixedPointShift);
		y = (unsigned int) (pt.y * FixedPointShift);

		TileSize = (unsigned int) (m_TileSize * FixedPointShift);

		x /= TileSize;
		y /= TileSize;

//		WriteText( "%f.%f - search at tile[%lu][%lu]\n",
//			pt.x,
//			pt.y,
//			y,
//			x);

		//
		// Search through that tile surface mesh's faces for a match.
		//

		try
		{
			const SurfaceMeshFace * Face;
			unsigned long           xend;
			unsigned long           yend;
			const TileSurfaceMesh & Mesh = GetTileSurfaceMesh( x, y );

			//
			// First, try the exact tile match for this coordinate.  This will
			// in almost all cases (unless we're on a seam) be the correct tile
			// surface mesh.
			//

			Face = Mesh.FindFace( pt, GetPoints( ) );

			if (Face != NULL)
			{
				if (SurfaceMesh != NULL)
					*SurfaceMesh = &Mesh;

				return Face;
			}

			//
			// This mesh wasn't the right one.  Try searching all adjacent mesh
			// objects as we shouldn't be off by more than one mesh.
			//

			xend = min( x + 1, m_TileGridWidth );
			yend = min( y + 1, m_TileGridHeight );

			for (unsigned long ix = x > 0 ? x - 1 : x; ix < xend; ix += 1)
			{
				for (unsigned long iy = y > 0 ? y - 1 : y; iy < yend; iy += 1)
				{
					//
					// Already tried [y][x], so we can skip it.
					//

					if (ix == x && iy == y)
						continue;

					const TileSurfaceMesh & Mesh = GetTileSurfaceMesh( ix, iy );

					Face = Mesh.FindFace(
						pt,
						GetPoints( ));

					if (Face != NULL)
					{
						if (SurfaceMesh != NULL)
							*SurfaceMesh = &Mesh;

						return Face;
					}
				}
			}

			return NULL;

#if 0
			for (TileSurfaceMeshVec::const_iterator it = GetTileSurfaceMeshes( ).begin( );
				it != GetTileSurfaceMeshes( ).end( );
				++it)
			{
				const SurfaceMeshFace * Face;

				Face = it->FindFace( pt, GetPoints( ) );

				if (Face != NULL)
				{
					if (SurfaceMesh != NULL)
						*SurfaceMesh = &*it;

					return Face;
				}
			}

			WriteText( "No containing face for %f, %f\n", pt.x, pt.y );

			return NULL;
#endif
		}
		catch (std::runtime_error)
		{
			return NULL;
		}
	}

	//
	// Returns the pair of faces residing in the same tile surface mesh which
	// contain the bounding points of a line segment.  The line segment must
	// reside entirely within the same tile surface mesh, although it may
	// reside on a seam bordering other tile surface meshes.
	//

	bool
	GetLineSegmentFacesAndMesh(
		__in const NWN::Vector2 & Start,
		__in const NWN::Vector2 & End,
		__in_opt const TileSurfaceMesh * ExcludeSurfaceMesh,
		__in bool Walkable,
		__out const SurfaceMeshFace * * Face1,
		__out const SurfaceMeshFace * * Face2,
		__out const TileSurfaceMesh * * SurfaceMesh
		) const;

	//
	// Return the global face index of a face, which must have been returned
	// from the AreaSurfaceMesh's face list.
	//

	inline
	unsigned long
	GetFaceId(
		__in const SurfaceMeshFace * Face
		) const
	{
		return (unsigned long) (Face - &GetTriangles( )[ 0 ]);
	}

	//
	// Retrieve a face by global face id.
	//

	inline
	const SurfaceMeshFace *
	GetFace(
		__in unsigned long FaceId
		) const
	{
		return &GetTriangles( )[ FaceId ];
	}

	//
	// Retrieve a vertex point by vertex point id.
	//

	inline
	const SurfaceMeshPoint *
	GetPoint(
		__in unsigned long PointId
		) const
	{
		return &GetPoints( )[ PointId ];
	}

	//
	// Retrieve an edge by edge id.
	//

	inline
	const SurfaceMeshEdge *
	GetEdge(
		__in unsigned long EdgeId
		) const
	{
		return &GetEdges( )[ EdgeId ];
	}

	//
	// Given the exit face for an island and the next desired island, find the
	// next enterance face for the next island.
	//

	inline
	const SurfaceMeshFace *
	GetNextEnteranceFace(
		__in const SurfaceMeshFace * ExitFace,
		__in unsigned short NextIsland
		) const
	{
		for (size_t i = 0; i < 3; i += 1)
		{
			unsigned long TriangleId;

			TriangleId = ExitFace->NeighborTriangles[ i ];

			if (TriangleId == (unsigned long) -1)
				continue;

			//
			// If this neighboring triangle has the desired island id, then use
			// it as our preferred enterance.
			//

			if (GetTriangles( )[ TriangleId ].Island == NextIsland)
				return &GetTriangles( )[ TriangleId ];
		}

		return NULL;
	}

	//
	// Check whether a point is walkable.
	//

	inline
	bool
	PositionWalkable(
		__in const NWN::Vector2 & v
		) const
	{
		const SurfaceMeshFace * Face = FindFace( v );

		if (Face == NULL)
			return false;

		return (Face->Flags & SurfaceMeshTriangle::WALKABLE);
	}

	//
	// Return the next island along the way between Island1 and Island2.
	//

	inline
	unsigned short
	GetNextIsland(
		__in unsigned short Island1,
		__in unsigned short Island2
		) const
	{
//		if ((Island1 >= m_Islands.size( )) ||
//		    (Island2 >= m_Islands.size( )))
//			return 0xFFFF;

		return m_IslandPathTable[ m_Islands.size( ) * Island1 + Island2 ].m_Next;
	}

	//
	// Return the exit face id to travel from Island1 to Island2.
	//

	inline
	unsigned long
	GetNextIslandExit(
		__in unsigned short Island1,
		__in unsigned short Island2
		) const
	{
		unsigned short NextIsland;
		size_t         i;

		NextIsland = GetNextIsland( Island1, Island2 );

		if (NextIsland == 0xFFFF)
			return 0xFFFFFFFF;

		const Island & Isle = m_Islands[ Island1 ];

		for (i = 0; i < Isle.m_Adjacent.size( ); i += 1)
		{
			if (Isle.m_Adjacent[ i ] != NextIsland)
				continue;

			return Isle.m_ExitFace[ i ];
		}

		return 0xFFFFFFFF;
	}

	//
	// Check whether any path exists between two points.
	//

	inline
	bool
	PathExists(
		__in const NWN::Vector2 & v1,
		__in const NWN::Vector2 & v2
		) const
	{
		const SurfaceMeshFace * face1;
		const SurfaceMeshFace * face2;

		//
		// Look up the corresponding faces.
		//

		face1 = FindFace( v1 );

		if (face1 == NULL)
			return false;

		face2 = FindFace( v2 );

		if (face2 == NULL)
			return false;

		if ((face1->Island == 0xFFFF) || (face2->Island == 0xFFFF))
			return false;

		//
		// On the same island?  Pathable.
		//

		if (face1->Island == face2->Island)
			return true;

		//
		// Perform a single path step from one island to the next.  If there is
		// a path at all then there should be a reachable route.
		//

		return GetNextIsland( face1->Island, face2->Island ) != 0xFFFF;
	}

	//
	// Check whether a straight-edge path exists between two points.
	//

	bool
	StraightPathExists(
		__in const NWN::Vector2 & v1,
		__in const NWN::Vector2 & v2,
		__out_opt NWN::Vector2 * LastFaceCentroid = NULL
		) const;

	//
	// Determine the distance a ray can travel before contacting a face in the
	// area surface mesh.
	//
	// The routine returns false if there were no contacts.
	//

	bool
	CalcContact(
		__in const NWN::Vector3 & Origin,
		__in const NWN::Vector3 & NormDir,
		__out float & IntersectDistance,
		__out_opt const SurfaceMeshFace * * IntersectFace
		) const;

	//
	// Return the count of path islands required to traverse between two
	// coordinate points.
	//

	inline
	unsigned long
	PathIslandLength(
		__in const NWN::Vector2 & v1,
		__in const NWN::Vector2 & v2
		) const
	{
		const SurfaceMeshFace * face1;
		const SurfaceMeshFace * face2;
		unsigned long           IslandCount;
		unsigned short          Island1;
		unsigned short          Island2;
		IslandIdVec             Visited;

		//
		// Look up the corresponding faces.
		//

		face1 = FindFace( v1 );

		if (face1 == NULL)
			return 0xFFFFFFFF;

		face2 = FindFace( v2 );

		if (face2 == NULL)
			return 0xFFFFFFFF;

		if (face2->Island == 0xFFFF)
			return 0xFFFFFFFF;

		//
		// Iterate through each intermediary pathing island until we reach the
		// end destination.
		//

		IslandCount = 0;

		for (Island1 = face1->Island, Island2 = face2->Island;
		     Island1 != Island2 && Island1 != 0xFFFF;
		     Island1 = GetNextIsland( Island1, Island2 ))
		{
			IslandCount += 1;

			if (std::find(
				Visited.begin( ),
				Visited.end( ),
				Island1 ) != Visited.end( ))
			{
				return 0xFFFFFFFF;
			}

			try
			{
				Visited.push_back( Island1 );
			}
			catch (std::exception)
			{
				return 0xFFFFFFFF;
			}
		}

		if (Island1 == 0xFFFF)
			return 0xFFFFFFFF;

		return IslandCount;
	}

	//
	// Calculate all surface mesh bounds, both for the overall bounding box and
	// those of each individual mesh.
	//

	inline
	void
	CalcBoundingBoxes(
		)
	{
		for (TileSurfaceMeshVec::iterator it = m_TileSurfaceMeshes.begin( );
		     it != m_TileSurfaceMeshes.end( );
		     ++it)
		{
			for (unsigned long FaceId = it->m_FaceOffset;
			     FaceId < it->m_FaceOffset + it->m_NumFaces;
			     FaceId += 1)
			{
				const SurfaceMeshFace * Face = GetFace( FaceId );

				for (unsigned long i = 0; i < 3; i += 1)
				{
					const SurfaceMeshPoint * Point;
					
					Point = GetPoint( Face->Corners[ i ] );

					it->UpdateBoundingBox( Point );
					UpdateBoundingBox( Point );
				}
			}
		}
	}

	//
	// Look up point height for a containing walkmesh triangle for a given
	// point.
	//

	float
	GetPointHeight(
		__in const NWN::Vector2 & pt
		) const;

	//
	// Convenience wrapper for a Vector3 that only has two components; looks
	// up the point height for a containing walkmesh triangle for a given
	// point.
	//

	inline
	float
	GetPointHeight(
		__in const NWN::Vector3 & pt3
		) const
	{
		NWN::Vector2 pt;

		pt.x = pt3.x;
		pt.y = pt3.y;

		return GetPointHeight( pt );
	}

	//
	// Calculate all intersection points from a line between Start and End
	// through the tile surface mesh grid.  Within the grid, fine-grained
	// pathing may be performed.
	//
	// Raises an std::exception on failure (e.g. out of memory).
	//

	void
	IntersectTileSurfaceMeshGrid(
		__out Vector2Vec & IntersectionPoints,
		__in const NWN::Vector2 & Start,
		__in const NWN::Vector2 & End
		) const;

private:

	//
	// Snap a point to the preceeding grid boundary.
	//

	inline
	float
	TileGridFloor(
		__in float f
		) const
	{
		return (f * m_TileSize) / m_TileSize;
	}

	static const float        PathingEpsilon;

	IDebugTextOut           * m_TextWriter;

	TileSurfaceMeshVec        m_TileSurfaceMeshes;
	IslandVec                 m_Islands;
	IslandPathNodeVec         m_IslandPathTable;

	unsigned long             m_TileGridHeight; // TileRows
	unsigned long             m_TileGridWidth;  // TileColumns
	unsigned long             m_TileBorderSize;
	unsigned long             m_Flags;
	float                     m_TileSize;

public: // For internal use only

	DECLARE_MESH_LINKAGE;

private:

	friend class MeshManager;

};

#endif
