/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	SkinMesh.cpp

Abstract:

	This module houses the SkinMesh class implementation.  Currently, the
	module just instantiates the base SimpleMesh template class.

--*/

#include "Precomp.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#include "SkinMesh.h"

template SkinMesh::BaseMesh;

const MeshLinkageTraits MLT_SkinMesh =
{
	/* Type                    = */ MeshLinkageTraits::MT_SkinMesh,
	/* MeshLinkageToBodyOffset = */ MESH_LINKAGE_OFFSET( SkinMesh ) 
};

const SimpleMeshTypeDescriptor SMTD_SkinMesh =
{
	/* LinkageTraits = */ &MLT_SkinMesh
};
