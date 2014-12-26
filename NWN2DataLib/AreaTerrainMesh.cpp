/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	AreaTerrainMesh.cpp

Abstract:

	This module houses the implementations of the AreaTerrainMesh class.
	These classes encapsulate a TRRN mesh from a terrain resource file.

	Major portions of the TRX on-disk layout courtesy Tero Kivinen
	<kivinen@iki.fi>.

--*/

#include "Precomp.h"
#include "TextOut.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#include "MeshManager.h"
#include "AreaTerrainMesh.h"

const MeshLinkageTraits MLT_AreaTerrainMesh =
{
	/* Type                    = */ MeshLinkageTraits::MT_AreaTerrainMesh,
	/* MeshLinkageToBodyOffset = */ MESH_LINKAGE_OFFSET( AreaTerrainMesh ) 
};
