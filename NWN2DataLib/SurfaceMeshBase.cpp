/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	SurfaceMeshBase.cpp

Abstract:

	This module houses the SurfaceMeshBase class implementation, which supports
	the core surface mesh functionality used by surface mapping.

	Major portions of the TRX on-disk layout courtesy Tero Kivinen
	<kivinen@iki.fi>.

--*/

#include "Precomp.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#include "SurfaceMeshBase.h"

bool
SurfaceMeshBase::IsPointInTriangle(
	__in const SurfaceMeshFace * Face,
	__in const NWN::Vector2 & pt,
	__in const PointVec & Points
	)
/*++

Routine Description:

	This routine tests whether a particular point resides with a triangle.

Arguments:

	Face - Supplies the triangle to perform the hit test for.

	pt - Supplies the point to search for.  The point should reside within
	     positive coordinate space.

	Points - Supplies the point vector to use.

Return Value:

	Returns true should the triangle contain the given point, else false.

Environment:

	User mode.

--*/
{
	NWN::Vector2 Tri[ 3 ];
	bool         Clockwise;

	for (size_t i = 0; i < 3; i += 1)
	{
		Tri[ i ].x = Points[ Face->Corners[ i ] ].x;
		Tri[ i ].y = Points[ Face->Corners[ i ] ].y;
	}

	Clockwise = (Face->Flags & SurfaceMeshFace::CLOCKWISE) != 0;

	return Math::PointInTriangle( Tri, pt, Clockwise );
}
