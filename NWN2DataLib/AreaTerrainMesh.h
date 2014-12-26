/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	AreaTerrainMesh.h

Abstract:

	This module defines the AreaTerrainMesh class, which stores mesh mapping
	and texture lookup data related to a terrain patch in an area.

	Terrain patches are divided into a uniform grid in a typical TRX file, but
	this is not strictly required by the implementation contained within.

	Major portions of the TRX on-disk layout courtesy Tero Kivinen
	<kivinen@iki.fi>.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_AREATERRAINMESH_H
#define _PROGRAMS_NWN2DATALIB_AREATERRAINMESH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "DdsImage.h"
#include "MeshLinkage.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif

class MeshManager;

extern const MeshLinkageTraits MLT_AreaTerrainMesh;

//
// Define the AreaTerrainMesh which contains terrain display data (e.g. grass).
//

class AreaTerrainMesh
{

public:

	inline
	AreaTerrainMesh(
		)
	: m_MeshLinkage( &MLT_AreaTerrainMesh )
	{
		Clear( );
	}

	inline
	~AreaTerrainMesh(
		)
	{
	}

#include <pshpack1.h>

	struct TerrainVertex // Vertex
	{
		NWN::Vector3          p;       // Position
		NWN::Vector3          n;       // Normal
		union
		{
			unsigned long     c;       // Color
			struct
			{
				unsigned char b;
				unsigned char g;
				unsigned char r;
				unsigned char a;
			};
		};
		NWN::Vector2          uv;      // XY10
		NWN::Vector2          weights; // XY1
	};

	C_ASSERT( sizeof( TerrainVertex ) == 44 );

	typedef unsigned short TerrainFaceVertexIndex;

	struct TerrainFace // Corner
	{
		unsigned short Vertex[ 3 ]; // Must match TerrainFaceVertexIndex
	};

	C_ASSERT( sizeof( TerrainFace ) == 6 );

	struct TerrainGrassHeader
	{
		NWN::ResRef32 Name;
		NWN::ResRef32 Type;
		unsigned long Blades;
	};

	C_ASSERT( sizeof( TerrainGrassHeader ) == 64 + 4 );

	struct TerrainGrassBlade
	{
		NWN::Vector3 p;    // Position
		NWN::Vector3 n;    // Orientation
		NWN::Vector3 uv_0; // Offset
	};

	C_ASSERT( sizeof( TerrainGrassBlade ) == 3 * (3 * 4 ) );

#include <poppack.h>

	typedef std::vector< TerrainGrassBlade > TerrainGrassBladeVec;

	struct TerrainGrass
	{
		TerrainGrassHeader   Header;
		TerrainGrassBladeVec Blades;
	};

	typedef std::vector< TerrainVertex > TerrainVertexVec;
	typedef std::vector< TerrainFace > TerrainFaceVec;
	typedef std::vector< TerrainGrass > TerrainGrassVec;

	inline
	const TerrainVertexVec &
	GetTerrainVerticies(
		) const
	{
		return m_TerrainVerticies;
	}

	inline
	const TerrainFaceVec &
	GetTerrainFaces(
		) const
	{
		return m_TerrainFaces;
	}

	inline
	const TerrainGrassVec &
	GetTerrainGrass(
		) const
	{
		return m_TerrainGrass;
	}

	inline
	void
	AddTerrainVertex(
		__in const TerrainVertex & Vertex
		)
	{
		m_TerrainVerticies.push_back( Vertex );
	}

	inline
	void
	AddTerrainFace(
		__in const TerrainFace & Face
		)
	{
		m_TerrainFaces.push_back( Face );
	}

	inline
	void
	AddTerrainGrass(
		__in const TerrainGrass & Grass
		)
	{
		m_TerrainGrass.push_back( Grass );
	}

	inline
	void
	Clear(
		)
	{
		m_TerrainVerticies.clear( );
		m_TerrainFaces.clear( );
		m_TerrainGrass.clear( );

		ZeroMemory( &m_Textures[ 0 ], sizeof( m_Textures ) );
		ZeroMemory( &m_TextureColor[ 0 ], sizeof( m_TextureColor ) );

		for (size_t i = 0; i < RTL_NUMBER_OF( m_Images ); i += 1)
			m_Images[ i ].Clear( );
	}

	inline
	const NWN::ResRef32 &
	GetTexture(
		__in size_t i
		)
	{
		if (i >= RTL_NUMBER_OF( m_Textures ) )
			throw std::runtime_error( "Invalid terrain texture index." );

		return m_Textures[ i ];
	}

	inline
	void
	SetTextures(
		__in const NWN::ResRef32 * Textures // [ 6 ]
		)
	{
		memcpy( &m_Textures[ 0 ], Textures, sizeof( m_Textures ) );
	}

	inline
	const NWN::NWNRGB &
	GetTextureColor(
		__in size_t i
		) const
	{
		if (i >= RTL_NUMBER_OF( m_TextureColor ) )
			throw std::runtime_error( "Invalid terrain texture index." );

		return m_TextureColor[ i ];
	}

	inline
	void
	SetTextureColor(
		__in const NWN::NWNRGB * TextureColor // [ 6 ]
		)
	{
		memcpy( &m_TextureColor[ 0 ], TextureColor, sizeof( m_TextureColor ) );
	}


	inline
	const DdsImage &
	GetImage(
		__in size_t Index
		) const
	{
		if (Index >= RTL_NUMBER_OF( m_Images ))
			throw std::runtime_error( "Illegal Terrain Image index" );

		return m_Images[ Index ];
	}

	inline
	DdsImage &
	GetImage(
		__in size_t Index
		)
	{
		if (Index >= RTL_NUMBER_OF( m_Images ))
			throw std::runtime_error( "Illegal Terrain Image index" );

		return m_Images[ Index ];
	}

	inline
	unsigned long
	GetColorMask(
		__in int x,
		__in int y,
		__in unsigned long Index
		) const
	{
		if (Index > 7)
			throw std::runtime_error( "Illegal color mask index" );

		if (Index < 4)
			return m_Images[ 0 ].GetColorChannel( x, y, Index );
		else
			return m_Images[ 1 ].GetColorChannel( x, y, Index - 4 );
	}


	//
	// Validate the terrainmesh, ensuring that all indicies are legal.
	//

	inline
	void
	Validate(
		) const
	{
		if ((m_Images[ 0 ].GetDdsHeader( ).Header.Height !=
		     m_Images[ 1 ].GetDdsHeader( ).Header.Height)                 ||
		    (m_Images[ 0 ].GetDdsHeader( ).Header.Width !=
		     m_Images[ 1 ].GetDdsHeader( ).Header.Width)                  ||
		    (m_Images[ 0 ].GetDdsHeader( ).Header.ddpf.RGBBitCount != 32) ||
		    (m_Images[ 1 ].GetDdsHeader( ).Header.ddpf.RGBBitCount != 32))
		{
			throw std::runtime_error( "Illegal terrain alpha channels" );
		}

		for (TerrainFaceVec::const_iterator it = m_TerrainFaces.begin( );
		     it != m_TerrainFaces.end( );
		     ++it)
		{
			for (unsigned long i = 0; i < 3; i += 1)
			{
				if (it->Vertex[ i ] >= m_TerrainVerticies.size( ))
					throw std::runtime_error( "Illegal TerrainFace.Vertex" );
			}
		}

		for (TerrainVertexVec::const_iterator it = m_TerrainVerticies.begin( );
		     it != m_TerrainVerticies.end( );
		     ++it)
		{
			int x;
			int y;

			x = (int) (it->weights.x * 127 + 0.5f);
			y = (int) (it->weights.y * 127 + 0.5f);	

			if ((x < 0) || (y < 0))
				throw std::runtime_error( "Illegal TerrainVertex.weights" );

			if ((unsigned long) x >= m_Images[ 0 ].GetDdsHeader( ).Header.Width)
				throw std::runtime_error( "Illegal TerrainVertex.weights" );

			if ((unsigned long) y >= m_Images[ 0 ].GetDdsHeader( ).Header.Height)
				throw std::runtime_error( "Illegal TerrainVertex.weights" );
		}
	}

private:

	TerrainVertexVec    m_TerrainVerticies;
	TerrainFaceVec      m_TerrainFaces;
	TerrainGrassVec     m_TerrainGrass;
	NWN::ResRef32       m_Textures[ 6 ];
	NWN::NWNRGB         m_TextureColor[ 6 ];
	DdsImage            m_Images[ 2 ];

public: // For internal use only

	DECLARE_MESH_LINKAGE;

private:

	friend class MeshManager;

};

typedef std::vector< AreaTerrainMesh > AreaTerrainMeshVec;

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
