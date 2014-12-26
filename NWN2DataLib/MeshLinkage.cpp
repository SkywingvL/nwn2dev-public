/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	MeshLinkage.cpp

Abstract:

	This module houses the implementation of the MeshLinkage object which is
	used to connect meshes to the mesh manager.

--*/

#include "Precomp.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "MeshLinkage.h"
#include "MeshManager.h"

void
MeshLinkage::RegisterMesh(
	__in MeshManager & MeshMgr
	)
/*++

Routine Description:

	This routine registers a mesh with the mesh manager.

Arguments:

	MeshMgr - Supplies the mesh manager to register with.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	if (m_AttachedMeshManager != NULL)
		swutil::RemoveEntryList( &m_MeshLinks );

	m_AttachedMeshManager = &MeshMgr;
	MeshMgr.OnMeshRegister( &m_MeshLinks, m_Traits );
}

