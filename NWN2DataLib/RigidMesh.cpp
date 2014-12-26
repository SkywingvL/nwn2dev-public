/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	RigidMesh.cpp

Abstract:

	This module houses the RigidMesh class implementation.  Currently, the
	module just instantiates the base SimpleMesh template class.

--*/

#include "Precomp.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#include "RigidMesh.h"

template RigidMesh::BaseMesh;

const MeshLinkageTraits MLT_RigidMesh =
{
	/* Type                    = */ MeshLinkageTraits::MT_RigidMesh,
	/* MeshLinkageToBodyOffset = */ MESH_LINKAGE_OFFSET( RigidMesh ) 
};

const SimpleMeshTypeDescriptor SMTD_RigidMesh =
{
	/* LinkageTraits = */ &MLT_RigidMesh
};
