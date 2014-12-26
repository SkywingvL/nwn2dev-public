/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	AreaWaterMesh.cpp

Abstract:

	This module houses the implementations of the AreaWaterMesh class.
	These classes encapsulate a WATR mesh from a terrain resource file.

	Major portions of the TRX on-disk layout courtesy Tero Kivinen
	<kivinen@iki.fi>.

--*/

#include "Precomp.h"
#include "TextOut.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#include "MeshManager.h"
#include "AreaWaterMesh.h"

const MeshLinkageTraits MLT_AreaWaterMesh =
{
	/* Type                    = */ MeshLinkageTraits::MT_AreaWaterMesh,
	/* MeshLinkageToBodyOffset = */ MESH_LINKAGE_OFFSET( AreaWaterMesh ) 
};
