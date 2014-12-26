/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	MDBFormat.h

Abstract:

	This module defines the general MDB on-disk layout definitions.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_MDBFORMAT_H
#define _PROGRAMS_NWN2DATALIB_MDBFORMAT_H

#ifdef _MSC_VER
#pragma once
#endif

//
// Common material header for MDB meshes.
//

//
// MDB texture flags.
//

typedef enum _MDB_TEXTURE_FLAGS
{
	// alpha map values from the diffuse map below 50% grey are not drawn
	TEXTURE_ALPHA_TEST           = 0x01,
	// Unused
	TEXTURE_ALPHA_BLEND          = 0x02,
	// Unused
	TEXTURE_ADDITIVE_BLEND       = 0x04,
	// Create a mirroring effect on the object
	TEXTURE_ENVIRONMENT_MAPPING  = 0x08,
	// Likely for highest resolution meshes only used in cutscenes 
	TEXTURE_CUTSCENE_MESH        = 0x10,
	// Enables the illumination map to create a glowing effect 
	TEXTURE_GLOW                 = 0x20,
	// Does not cast shadows
	TEXTURE_NO_CAST_SHADOWS      = 0x40,
	// The projected texture flag means that the model will accept UI
	// projected textures such as the spell targeting cursor
	TEXTURE_PROJECTED_TEXTURES   = 0x80,

	LAST_MDB_TEXTURE_FLAG
} MDB_TEXTURE_FLAGS, * PCMDB_TEXTURE_FLAGS;

//
// MDB material on-disk format.
//

typedef struct _MODEL_MATERIAL
{
	NWN::ResRef32 DiffuseMap;
	NWN::ResRef32 NormalMap;
	NWN::ResRef32 TintMap;
	NWN::ResRef32 GlowMap;
	NWN::Vector3  Kd; // Diffuse color
	NWN::Vector3  Ks; // Specular color
	float         SpecularPower;
	float         SpecularValue;
	unsigned long TextureFlags;
} MODEL_MATERIAL, * PMODEL_MATERIAL;

C_ASSERT( sizeof( MODEL_MATERIAL ) == 128 + 6*4 + 8 + 4 );

typedef const struct _MATERIAL * PCMATERIAL;

#endif
