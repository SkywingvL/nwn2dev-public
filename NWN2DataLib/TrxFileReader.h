/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	TrxFileReader.h

Abstract:

	This module defines the interface to the *.trx file reader, which is used
	to retrieve walkmesh data (such as area dimensions).

	The TrxFileReader object defined in this module also supports the reading
	of MDB files, which represent game models.

	Data representation of all MDB and TRX-derived mesh types is made available
	via the TrxFileReader (depending on whether it is operating in TRX or MDB
	modes).

	Major portions of the TRX on-disk layout courtesy Tero Kivinen
	<kivinen@iki.fi>.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_TRXFILEREADER_H
#define _PROGRAMS_NWN2DATALIB_TRXFILEREADER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "FileWrapper.h"

#include "DdsImage.h"
#include "SurfaceMeshBase.h"
#include "AreaSurfaceMesh.h"
#include "AreaWaterMesh.h"
#include "AreaTerrainMesh.h"
#include "AreaHeightMap.h"
#include "SimpleMesh.h"
#include "MDBFormat.h"
#include "RigidMesh.h"
#include "SkinMesh.h"
#include "CollisionMesh.h"
#include "WalkMesh.h"
#include "ModelInstance.h"
#include "ModelCollider.h"

class MeshManager;
struct IDebugTextOut;

//
// 9 inside units per tile
// 40 outside units per tile (subtract two border tiles)
//
// 
//








//
// Define the TrxFileReader object which is responsible for parsing walkmesh
// and area dimension data.
//

class TrxFileReader
{

public:

	typedef enum _MODE
	{
		ModeTRX,
		ModeMDB
	} MODE, * PMODE;

	typedef const _MODE * PCMODE;

	enum { UNITS_PER_INDOOR_TILE = 9 };
	enum { UNITS_PER_OUTDOOR_TILE = 40 };
	enum { INDOOR_TILE_PADDING = 0 };
	enum { OUTDOOR_TILE_PADDING = 2 };

	typedef swutil::SharedPtr< TrxFileReader > Ptr;

	//
	// Load and parse a Trx file, raises an std::exception on failure.
	//

	TrxFileReader(
		__in MeshManager & MeshMgr,
		__in const std::string & FileName,
		__in bool LoadOnlyDimensions,
		__in MODE Mode = ModeTRX,
		__in_opt IDebugTextOut * TextWriter = NULL,
		__in bool RefuseDisplayOnlyModels = false
		);

	~TrxFileReader(
		);

	//
	// Width, in tiles.  (9 units per tile.)
	//

	inline
	ULONG
	GetWidth(
		) const
	{
		return m_Width;
	}

	//
	// Height, in tiles.
	//

	inline
	ULONG
	GetHeight(
		) const
	{
		return m_Height;
	}

	//
	// Surface mesh access.
	//

	inline
	const AreaSurfaceMesh &
	GetSurfaceMesh(
		) const
	{
		return m_Walkmesh;
	}

	//
	// Water mesh access.
	//

	inline
	const AreaWaterMeshVec &
	GetWaterMesh(
		) const
	{
		return m_WaterMesh;
	}

	//
	// Terrain mesh access.
	//

	inline
	const AreaTerrainMeshVec &
	GetTerrainMesh(
		) const
	{
		return m_TerrainMesh;
	}

	//
	// Height map access.
	//

	inline
	const AreaHeightMap &
	GetHeightMap(
		) const
	{
		return m_HeightMap;
	}

	//
	// Model collider access.
	//

	inline
	const ModelCollider &
	GetCollider(
		) const
	{
		return *m_Collider.get( );
	}

	inline
	ModelCollider &
	GetCollider(
		)
	{
		return *m_Collider.get( );
	}

	inline
	ModelColliderPtr &
	GetColliderPtr(
		)
	{
		return m_Collider;
	}

private:

	//
	// Define manifest IDs used for the file header and resource ytpes.
	//

	enum TRX_ID
	{
		TRX_HEADER_ID             = '2NWN',
		TRX_AREA_SURFACE_MESH_ID  = 'MWSA',
		TRX_WIDTH_HEIGHT_ID       = 'HWRT',
		TRX_COMPRESSION_HEADER_ID = 'PMOC',
		TRX_WALKABLE_ID           = 'KLAW',
		TRX_WATER_ID              = 'RTAW',
		TRX_TERRAIN_ID            = 'NRRT',
		TRX_COLLISION2_ID         = '2LOC',
		TRX_COLLISION3_ID         = '3LOC',
		TRX_COLLISIONSPHERES_ID   = 'SLOC',
		TRX_RIGID_ID              = 'DGIR',
		TRX_SKIN_ID               = 'NIKS',
		TRX_HOOK_ID               = 'KOOH',
		TRX_HAIR_ID               = 'RIAH',
		TRX_HELM_ID               = 'MLEH',

		LAST_TRX_ID
	};

#include <pshpack1.h>
	//
	// Define on-disk file structures.
	//

	typedef struct _TRX_HEADER
	{
		unsigned long  TrxHeaderId;
		unsigned short MajorVersion;
		unsigned short MinorVersion;
		unsigned long  ResourceCount;
	} TRX_HEADER, * PTRX_HEADER;

	C_ASSERT( sizeof( TRX_HEADER ) == 0x0C );

	typedef const struct _TRX_HEADER * PCTRX_HEADER;


	typedef struct _RESOURCE_HEADER
	{
		unsigned long ResourceTypeId;
		unsigned long Length;
	} RESOURCE_HEADER, * PRESOURCE_HEADER;

	C_ASSERT( sizeof( RESOURCE_HEADER ) == 0x08 );

	typedef const struct _RESOURCE_HEADER * PCRESOURCE_HEADER;


	typedef struct _RESOURCE_ENTRY
	{
		unsigned long ResourceTypeId;
		unsigned long Offset;
	} RESOURCE_ENTRY, * PRESOURCE_ENTRY;

	C_ASSERT( sizeof( RESOURCE_ENTRY ) == 0x08 );

	typedef const struct _RESOURCE_ENTRY * PCRESOURCE_ENTRY;


	typedef struct _COMPRESSION_HEADER
	{
		unsigned long TypeId;
		unsigned long CompressedSize;
		unsigned long UncompressedSize;
	} COMPRESSION_HEADER, * PCOMPRESSION_HEADER;

	C_ASSERT( sizeof( COMPRESSION_HEADER ) == 0x0C );

	typedef const struct _COMPRESSION_HEADER * PCCOMPRESSION_HEADER;


	typedef struct _TRWH_HEADER
	{
		unsigned long Width;
		unsigned long Height;
		unsigned long IdNumber;
	} TRWH_HEADER, * PTRWH_HEADER;

	C_ASSERT( sizeof( TRWH_HEADER ) == 0x0C );

	typedef const struct _TRWH_HEADER * PCTRWH_HEADER;

	//
	// ASWM on-disk format.
	//

	typedef struct _ASWM_HEADER
	{
		unsigned long Version;
		NWN::ResRef32 Name;
		unsigned char OwnsData;
		unsigned long PointCount;
		unsigned long EdgeCount;
		unsigned long TriangleCount;
		unsigned long FaceOffset;
	} ASWM_HEADER, * PASWM_HEADER;

	C_ASSERT( sizeof( ASWM_HEADER ) == 37 + 16 );

	typedef const struct _ASWM_HEADER * PCASWM_HEADER;

	//
	// MDB mesh packet header.
	//

	typedef struct _MODEL_MESH_HEADER
	{
		NWN::ResRef32    Name;
		MODEL_MATERIAL   Material;
		unsigned long    NumVerts;
		unsigned long    NumFaces;
	} MODEL_MESH_HEADER, * PMODEL_MESH_HEADER;

	C_ASSERT( sizeof( MODEL_MESH_HEADER ) == 40 + sizeof( MODEL_MATERIAL ) );

	typedef const struct _MODEL_MESH_HEADER * PCMODEL_MESH_HEADER;

#include <poppack.h>

	//
	// WALK on-disk format.
	//

	typedef struct _WALK_HEADER
	{
		NWN::ResRef32 Name;
		unsigned long Flags;
		unsigned long VertexCount;
		unsigned long TriangleCount;
	} WALK_HEADER, * PWALK_HEADER;

#include <pshpack1.h>

	//
	// WATR on-disk format.
	//

	typedef struct _WATR_TEXTURE
	{
		NWN::ResRef32 Name;
		NWN::Vector2  Direction;
		float         Rate;
		float         Angle;
	} WATR_TEXTURE, * PWATR_TEXTURE;

	typedef const struct _WATR_TEXTURE * PCWATR_TEXTURE;

	C_ASSERT( sizeof( WATR_TEXTURE ) == 48 );

	typedef struct _WATR_HEADER
	{
		NWN::ResRef32 Name;
		unsigned char Data[ 96 ];
		NWN::NWNRGB   WaterColor;
		NWN::Vector2  Ripple;
		float         Smoothness;
		float         RefBias;
		float         RefPower;
		float         SpecularPower;
		float         SpecularCofficient;
		WATR_TEXTURE  Texture[ 3 ];
		NWN::Vector2  UVMapOffset;
		unsigned long VertexCount;
		unsigned long TriangleCount;
	} WATR_HEADER, * PWATR_HEADER;

	typedef const struct _WATR_HEADER * PCWATR_HEADER;

	C_ASSERT( sizeof( WATR_HEADER ) == 128 + 12 + 28 + 48*3 + 16 );

	//
	// TRRN on-disk format.
	//

	typedef struct _TRRN_TEXTURE
	{
		NWN::ResRef32 Name;
	} TRRN_TEXTURE, * PTRRN_TEXTURE;

	typedef const struct _TRRN_TEXTURE * PCTRRN_TEXTURE;

	C_ASSERT( sizeof( TRRN_TEXTURE ) == 32 );
	C_ASSERT( sizeof( TRRN_TEXTURE ) == sizeof( NWN::ResRef32 ) );

	typedef struct _TRRN_HEADER
	{
		char          Name[ 128 ];
		TRRN_TEXTURE  Texture[ 6 ];
		NWN::NWNRGB   TextureColor[ 6 ];
		unsigned long VertexCount;
		unsigned long TriangleCount;
	} TRRN_HEADER, * PTRRN_HEADER;

	C_ASSERT( sizeof( TRRN_HEADER ) == 128 + 6*32 + 6*12 + 2*4 );

	typedef const struct _TRRN_HEADER * PCTRRN_HEADER;

#include <poppack.h>

	typedef std::vector< RESOURCE_ENTRY > ResourceEntryVec;

	//
	// Buffer or file reader context, used to allow a seamless interface for
	// both uncompressed and compressed data.
	//

	typedef struct _READER_CONTEXT
	{
		unsigned char * Buffer;
		size_t          Size;
	} READER_CONTEXT, * PREADER_CONTEXT;

	typedef const struct _READER_CONTEXT * PCREADER_CONTEXT;

	//
	// Define the main parse entrypoint.
	//

	void
	ParseTrxFile(
		__in MeshManager & MeshMgr
		);

	//
	// Define the area surface walkmesh reader.
	//

	void
	DecodeAreaSurfaceWalkmesh(
		__in PCRESOURCE_HEADER ResHeader,
		__in MeshManager & MeshMgr
		);

	//
	// Define the area width/height reader.
	//

	void
	DecodeAreaWidthHeight(
		__in PCRESOURCE_HEADER ResHeader
		);

	//
	// Define the object walkmesh reader.
	//

	void
	DecodeWalkable(
		__in PCRESOURCE_HEADER ResHeader,
		__in MeshManager & MeshMgr
		);

	//
	// Define the water reader.
	//

	void
	DecodeWater(
		__in PCRESOURCE_HEADER ResHeader,
		__in MeshManager & MeshMgr
		);

	//
	// Define the terrain reader.
	//

	void
	DecodeTerrain(
		__in PCRESOURCE_HEADER ResHeader,
		__in MeshManager & MeshMgr
		);

	//
	// Define the collision mesh reader.
	//

	void
	DecodeCollisionMesh(
		__in PCRESOURCE_HEADER ResHeader,
		__in MeshManager & MeshMgr
		);

	//
	// Define the rigid mesh reader.
	//

	void
	DecodeRigidMesh(
		__in PCRESOURCE_HEADER ResHeader,
		__in MeshManager & MeshMgr
		);

	//
	// Define the skin mesh reader.
	//

	void
	DecodeSkinMesh(
		__in PCRESOURCE_HEADER ResHeader,
		__in MeshManager & MeshMgr
		);

	//
	// Define the simple mesh reader.
	//

	template< typename T >
	void
	DecodeSimpleMesh(
		__in PCRESOURCE_HEADER ResHeader,
		__in MeshManager & MeshMgr,
		__inout T & Mesh
		);

	//
	// Define the collision sphere reader.
	//

	void
	DecodeCollisionSpheres(
		__in PCRESOURCE_HEADER ResHeader
		);

	//
	// Define the hook point reader.
	//

	void
	DecodeHookPoint(
		__in PCRESOURCE_HEADER ResHeader
		);

	//
	// Define the hair point reader.
	//

	void
	DecodeHairPoint(
		__in PCRESOURCE_HEADER ResHeader
		);

	//
	// Define the helm point reader.
	//

	void
	DecodeHelmPoint(
		__in PCRESOURCE_HEADER ResHeader
		);

	//
	// ReadFile wrapper with descriptive exception raising on failure.
	//

	inline
	void
	ReadFile(
		__out_bcount( Length ) void * Buffer,
		__in size_t Length,
		__in const char * Description
		)
	{
		return m_FileWrapper.ReadFile( Buffer, Length, Description );
	}

	//
	// ReadFile wrapper for DDS images.
	//

	inline
	void
	ReadDdsImage(
		__out DdsImage & Image
		)
	{
		Trx::DDS_FILE                Header;
		unsigned long                Length;
		std::vector< unsigned char > Data;

		ReadFile( &Length, sizeof( Length ), "DDS Image Length" );

		if (Length < sizeof( Header ))
			throw std::exception( "DDS Image Length too short" );

		ReadFile( &Header, sizeof( Header ), "DDS Header" );

		Image.SetDdsHeader( Header );

		if (Length - sizeof( Header ) != 0)
		{
			unsigned long ImgLen;

			ImgLen = Length - sizeof( Header );

			if (ImgLen > 64 * 1024 * 1024)
				throw std::exception( "DDS Image too long" );

			Data.resize( ImgLen );
			ReadFile( &Data[ 0 ], ImgLen, "DDS Image Data" );

			Image.SetImage( &Data[ 0 ], Data.size( ) );
		}

		Image.Validate( );
	}

	//
	// Seek to a particular file offset.
	//

	inline
	void
	SeekOffset(
		__in ULONGLONG Offset,
		__in const char * Description
		)
	{
		return m_FileWrapper.SeekOffset( Offset, Description );
	}

	//
	// Wrapper to read from a reader context.
	//

	inline
	void
	ReadReaderContext(
		__inout PREADER_CONTEXT ReaderContext,
		__out void * Buffer,
		__in size_t Length,
		__in const char * Description
		)
	{
		char ExMsg[ 64 ];

		//
		// Check and update length.
		//

		if (ReaderContext->Size < Length)
		{
			StringCbPrintfA(
				ExMsg,
				sizeof( ExMsg ),
				"Reader overrun - %s",
				Description);

			throw std::exception( ExMsg );
		}

		ReaderContext->Size -= Length;

		//
		// Read from the disk file if we've no uncompressed buffer, else read
		// from the buffer directly.
		//

		if (ReaderContext->Buffer == NULL)
			ReadFile( Buffer, Length, Description );
		else
		{
			memcpy( Buffer, ReaderContext->Buffer, Length );

			ReaderContext->Buffer += Length;
		}
	}

	//
	// Define area width/height parameters.
	//

	ULONG                     m_Width;
	ULONG                     m_Height;

	//
	// Define file book-keeping data.
	//

	HANDLE                    m_File;
	ULONG                     m_FileSize;
	FileWrapper               m_FileWrapper;

	//
	// Record whether we were only to load dimension data and not other area
	// parameters.
	//

	bool                      m_LoadOnlyDimensions;

	//
	// Define the raw file header.
	//

	TRX_HEADER                m_FileHeader;

	//
	// Define the resource directory.
	//

	ResourceEntryVec          m_ResourceDirectory;

	//
	// Define the walkmesh data structure.
	//

	AreaSurfaceMesh           m_Walkmesh;

	//
	// Define the watermesh data structure.
	//

	AreaWaterMeshVec          m_WaterMesh;

	//
	// Define the terrainmesh data structure.
	//

	AreaTerrainMeshVec        m_TerrainMesh;

	//
	// Define the heightmap data structure.
	//

	AreaHeightMap             m_HeightMap;

	//
	// Define the parse mode.
	//

	MODE                      m_Mode;

	IDebugTextOut           * m_TextWriter;

	//
	// Define the model collider data.
	//

	ModelColliderPtr          m_Collider;

	//
	// Define whether display only models should not be loaded.  This is used
	// to reduce memory consumption on the server.
	//

	bool                      m_RefuseDisplayOnlyModels;

};

#endif
