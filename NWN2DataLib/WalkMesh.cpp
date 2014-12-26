/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	WalkMesh.cpp

Abstract:

	This module houses the WalkMesh class implementation.  Currently, the
	module just instantiates the base SimpleMesh template class.

--*/

#include "Precomp.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#include "WalkMesh.h"

const float WalkMesh::LinkVertexZ = -1000000.0f;

template WalkMesh::BaseMesh;

const MeshLinkageTraits MLT_WalkMesh =
{
	/* Type                    = */ MeshLinkageTraits::MT_WalkMesh,
	/* MeshLinkageToBodyOffset = */ MESH_LINKAGE_OFFSET( WalkMesh ) 
};

const SimpleMeshTypeDescriptor SMTD_WalkMesh =
{
	/* LinkageTraits = */ &MLT_WalkMesh
};

