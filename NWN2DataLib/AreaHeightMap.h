/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	AreaHeightMap.h

Abstract:

	This module defines the AreaHeightMap class, which maintains fast height
	lookup based on the terrain and water mesh at verticies only.

	Typically, most game operations instead use the AreaSurfaceMesh height
	mapping APIs.

	Major portions of the TRX on-disk layout courtesy Tero Kivinen
	<kivinen@iki.fi>.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_AREAHEIGHTMAP_H
#define _PROGRAMS_NWN2DATALIB_AREAHEIGHTMAP_H

#ifdef _MSC_VER
#pragma once
#endif

#include "AreaWaterMesh.h"
#include "AreaTerrainMesh.h"

//
// Define the (sparse) height map for an area.
//

class AreaHeightMap
{

public:

	inline
	AreaHeightMap(
		)
	{
		Clear( );
	}

	typedef std::map< int, float > HeightYMap;

	typedef std::map< int, HeightYMap > HeightMapMap;

	inline
	void
	Clear(
		)
	{
		m_HeightMapMap.clear( );
	}

	//
	// Update the height map for a new terrain mesh.
	//

	inline
	void
	ComputeHeights(
		__in const AreaTerrainMesh & Terrain
		)
	{
		const AreaTerrainMesh::TerrainVertexVec & Verticies = Terrain.GetTerrainVerticies( );

		for (AreaTerrainMesh::TerrainVertexVec::const_iterator it = Verticies.begin( );
		     it != Verticies.end( );
		     ++it)
		{
			int x;
			int y;

			x = (int) (it->p.x * 3 / 5 + 0.5f);
			y = (int) (it->p.y * 3 / 5 + 0.5f);

			m_HeightMapMap[ x ][ y ] = it->p.z;
		}
	}

	//
	// Update the height map for a water mesh.
	//

	inline
	void
	ComputeWaterHeights(
		__in const AreaWaterMesh & Water
		)
	{
		const AreaWaterMesh::WaterVertexVec & Verticies = Water.GetWaterVerticies( );

		for (AreaWaterMesh::WaterVertexVec::const_iterator it = Verticies.begin( );
		     it != Verticies.end( );
		     ++it)
		{
			int x;
			int y;

			x = (int) (it->p.x * 3 / 5 + 0.5f);
			y = (int) (it->p.y * 3 / 5 + 0.5f);

			m_WaterHeightMapMap[ x ][ y ] = it->p.z;
		}
	}

	//
	// Search for a terrain height in the sparse terrain tree.  Any location
	// that did not have terrain data predefined is considered as having a
	// height of zero (ground level).
	//

	inline
	float
	GetHeight(
		__in float x,
		__in float y
		) const
	{
		int                          xx;
		int                          yy;
		HeightMapMap::const_iterator it;
		HeightYMap::const_iterator   iit;

		xx = (int) (x * 3 / 5 + 0.5f);
		yy = (int) (y * 3 / 5 + 0.5f);

		if ((it = m_HeightMapMap.find( xx )) == m_HeightMapMap.end( ))
			return 0.0f;

		if ((iit = it->second.find( yy )) == it->second.end( ))
			return 0.0f;

		return iit->second;
	}

	//
	// Return whether there is water above a given point on the map.
	//

	inline
	bool
	IsPointSubmerged(
		__in float x,
		__in float y,
		__in float z
		) const
	{
		int                          xx;
		int                          yy;
		HeightMapMap::const_iterator it;
		HeightYMap::const_iterator   iit;

		xx = (int) (x * 3 / 5 + 0.5f);
		yy = (int) (y * 3 / 5 + 0.5f);

		//
		// No water here?  Not submerged.
		//

		if ((it = m_WaterHeightMapMap.find( xx )) == m_WaterHeightMapMap.end( ))
			return false;

		//
		// No water here?  Not submerged.
		//

		if ((iit = it->second.find( yy )) == it->second.end( ))
			return false;

		//
		// Above ground level?  Not submerged.
		//

		if (z >= GetHeight( x, y ))
			return false;

		return iit->second >= z;
	}

private:

	HeightMapMap m_HeightMapMap;
	HeightMapMap m_WaterHeightMapMap;

};

#endif
