/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	SimpleMesh.cpp

Abstract:

	This module houses the SimpleMesh class implementation, which supports
	common functionality used by derived MDB meshes.

--*/

#include "Precomp.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#include "SimpleMesh.h"
#include "MeshManager.h"

#if 0
template<
	typename VertexT,
	typename FaceT,
	const SimpleMeshTypeDescriptor * MeshType,
	CoordTransMode TMode,
	size_t NumWeights,
	typename PointIndexT,
	typename FaceIndexT >
void
SimpleMesh< VertexT, FaceT, TMode, NumWeights, PointIndexT, FaceIndexT >::Update(
	__in const NWN::Matrix44 & M
	)
/*++

Routine Description:

	This routine applies a new world transformation to the collision mesh.

Arguments:

	M - Supplies the transformation to apply.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	for (VertexVec::iterator it = m_Points.begin( );
	     it != m_Points.end( );
	     ++it)
	{
		it->Pos.x = M._00 * it->LocalPos.x + M._10 * it->LocalPos.y + M._20 * it->LocalPos.z + M._30;
		it->Pos.y = M._01 * it->LocalPos.x + M._11 * it->LocalPos.y + M._21 * it->LocalPos.z + M._31;
		it->Pos.z = M._02 * it->LocalPos.x + M._12 * it->LocalPos.y + M._22 * it->LocalPos.z + M._32;
	}

	//
	// Refresh normals.
	//

	Precalculate( );
}

template<
	typename VertexT,
	typename FaceT,
	const SimpleMeshTypeDescriptor * MeshType,
	CoordTransMode TMode,
	size_t NumWeights,
	typename PointIndexT,
	typename FaceIndexT >
void
SimpleMesh< VertexT, FaceT, TMode, NumWeights, PointIndexT, FaceIndexT >::Precalculate(
	)
/*++

Routine Description:

	This routine precomputes useful data about the mesh in advance of any
	queries that might reference it.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	//
	// Calculate the normal of each face.
	//

	for (FaceVec::iterator it = m_Faces.begin( ); it != m_Faces.end( ); ++it)
	{
		NWN::Vector3 Tri[ 3 ];

		for (size_t i = 0; i < 3; i += 1)
			Tri[ i ] = GetPoint( it->Corners[ i ] ).Pos;

		it->Normal = Math::ComputeNormalTriangle( Tri );
	}
}
#endif

