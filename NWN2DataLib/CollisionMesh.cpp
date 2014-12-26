/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	CollisionMesh.cpp

Abstract:

	This module houses the CollisionMesh class implementation, including
	support for transforming the mesh to match a new world transformation.

--*/

#include "Precomp.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#include "CollisionMesh.h"

const MeshLinkageTraits MLT_CollisionMesh =
{
	/* Type                    = */ MeshLinkageTraits::MT_CollisionMesh,
	/* MeshLinkageToBodyOffset = */ MESH_LINKAGE_OFFSET( CollisionMesh ) 
};

const SimpleMeshTypeDescriptor SMTD_CollisionMesh =
{
	/* LinkageTraits = */ &MLT_CollisionMesh
};

void
CollisionMesh::Update(
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
	for (VertexVec::iterator it = GetPoints( ).begin( );
	     it != GetPoints( ).end( );
	     ++it)
	{
		it->Pos = Math::Multiply( M, it->LocalPos );
	}

	//
	// Refresh normals.
	//

	Precalculate( );
}

void
CollisionMesh::Precalculate(
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

	for (FaceVec::iterator it = GetFaces( ).begin( ); it != GetFaces( ).end( ); ++it)
	{
		NWN::Vector3 Tri[ 3 ];

		for (size_t i = 0; i < 3; i += 1)
			Tri[ i ] = GetPoint( it->Corners[ i ] ).Pos;

		it->Normal = Math::ComputeNormalTriangle( Tri );
	}
}

template CollisionMesh::BaseMesh;
