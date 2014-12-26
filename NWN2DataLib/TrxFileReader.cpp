/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	TrxFileReader.cpp

Abstract:

	This module houses the *.trx file format parser, which is used to read
	walkmesh data (such as area dimensions).

	Major portions of the TRX on-disk layout courtesy Tero Kivinen
	<kivinen@iki.fi>.

--*/

#include "Precomp.h"
#include "TrxFileReader.h"

TrxFileReader::TrxFileReader(
	__in MeshManager & MeshMgr,
	__in const std::string & FileName,
	__in bool LoadOnlyDimensions,
	__in MODE Mode, /* = ModeTRX */
	__in IDebugTextOut * TextWriter, /* = NULL */
	__in bool RefuseDisplayOnlyModels /* = false */
	)
/*++

Routine Description:

	This routine constructs a new TrxFileReader object and parses the contents
	of a TRX file by filename.  The file must already exist as it is
	immediately deserialized.

Arguments:

	MeshMgr - Supplies the mesh manager to which all child meshes are
	          registered to.

	FileName - Supplies the path to the TRX file.

	LoadOnlyDimensions - Supplies a Boolean value indicating if only area size
	                     parameters should be loaded, versus all area mesh data
						 (which is an expensive operation).  This parameter is
						 only effective for ModeTRX.

	Mode - Supplies the parser mode (e.g. TRX vs MDB).

	TextWriter - Optionally supplies the text output implementation that is
	             used to indicate debug log messages upwards.

	RefuseDisplayOnlyModels - Supplies a Boolean value that indicates whether
	                          any model data that is purely display-based is to
	                          not be loaded.

Return Value:

	The newly constructed object.

Environment:

	User mode.

--*/
: m_Width( 0 ),
  m_Height( 0 ),
  m_File( INVALID_HANDLE_VALUE ),
  m_FileSize( 0 ),
  m_LoadOnlyDimensions( LoadOnlyDimensions ),
  m_Walkmesh( TextWriter ),
  m_Mode( Mode ),
  m_TextWriter( TextWriter ),
  m_RefuseDisplayOnlyModels( RefuseDisplayOnlyModels )
{
	HANDLE File;
//	DWORD  Read;
//	DWORD  Pos;

	File = CreateFileA(
		FileName.c_str( ),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);

	if (File == INVALID_HANDLE_VALUE)
		throw std::runtime_error( "Failed to open .trx file." );

	m_File = File;

	m_FileWrapper.SetFileHandle( File );

	try
	{
		m_FileSize = GetFileSize( File, NULL );

		if ((m_FileSize == 0xFFFFFFFF) && (GetLastError( ) != NO_ERROR))
			throw std::runtime_error( "Failed to read file size." );

#if 0
		//
		// TEMP, TEMP:  Old style parsing.
		//

		Pos = SetFilePointer( File, 0x10, NULL, FILE_BEGIN );

		if ((Pos == INVALID_SET_FILE_POINTER) && (GetLastError( ) != NO_ERROR))
			throw std::runtime_error( "Failed to seek to 0x10." );

		if (!::ReadFile( File, &Pos, sizeof( Pos ), &Read, NULL))
			throw std::runtime_error( "Failed to read TRWH header offset." );

		Pos = SetFilePointer( File, Pos + 0x08, NULL, FILE_BEGIN );

		if ((Pos == INVALID_SET_FILE_POINTER) && (GetLastError( ) != NO_ERROR))
			throw std::runtime_error( "Failed to seek to TRWH header." );

		if (!::ReadFile( File, &m_Width, sizeof( m_Width ), &Read, NULL ))
			throw std::runtime_error( "Failed to read width." );

		if (!::ReadFile( File, &m_Height, sizeof( m_Height ), &Read, NULL ))
			throw std::runtime_error( "Failed to read height." );

		//
		// Reset the file pointer so that we can run new-style parsing directly
		// after.
		//

		Pos = SetFilePointer( m_File, 0, NULL, FILE_BEGIN );

		if ((Pos == INVALID_SET_FILE_POINTER) && (GetLastError( ) != NO_ERROR))
			throw std::runtime_error( "Failed to seek to 0x00." );

#endif

		ParseTrxFile( MeshMgr );

		//
		// All done.
		//

		m_File = INVALID_HANDLE_VALUE;

		CloseHandle( File );

		m_FileWrapper.SetFileHandle( INVALID_HANDLE_VALUE );
	}
	catch (...)
	{
		m_File = INVALID_HANDLE_VALUE;

		CloseHandle( File );

		m_FileWrapper.SetFileHandle( INVALID_HANDLE_VALUE );

		throw;
	}
}

TrxFileReader::~TrxFileReader(
	)
/*++

Routine Description:

	This routine cleans up an already-existing TrxFileReader object.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	if (m_File != INVALID_HANDLE_VALUE)
	{
		CloseHandle( m_File );

		m_File = INVALID_HANDLE_VALUE;
	}
}

void
TrxFileReader::ParseTrxFile(
	__in MeshManager & MeshMgr
	)
/*++

Routine Description:

	This routine parses relevant resources out of a TRX file.  In particular,
	we read walkmesh and area width/height data.

Arguments:

	MeshMgr - Supplies the mesh manager to which all child meshes are
	          registered to.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	bool FoundAswm;
	bool FoundTrwh;

	//
	// Read the file header first.
	//

	ReadFile( &m_FileHeader, sizeof( m_FileHeader ), "FileHeader" );

	if (m_FileHeader.TrxHeaderId != TRX_HEADER_ID)
		throw std::runtime_error( "TRX header ID mismatch (not a .trx file)");

	if (m_FileHeader.ResourceCount < 65536)
		m_ResourceDirectory.reserve( m_FileHeader.ResourceCount );

	//
	// Process each resource directory entry in turn.
	//

	for (unsigned long i = 0;
	     i < m_FileHeader.ResourceCount;
	     ++i)
	{
		RESOURCE_ENTRY ResEntry;

		ReadFile( &ResEntry, sizeof( ResEntry ), "ResourceEntry" );

		m_ResourceDirectory.push_back( ResEntry );
	}

	//
	// Process resource headers we are interested in.
	//

	FoundAswm = false;
	FoundTrwh = false;

	//
	// Create a new model instance if we're loading an MDB.
	//

	if (m_Mode == ModeMDB)
	{
		m_Collider = new ModelCollider( );

		m_Collider->CreateModelInstance( );
	}

	for (ResourceEntryVec::const_iterator it = m_ResourceDirectory.begin( );
	     it != m_ResourceDirectory.end( );
	     ++it)
	{
		RESOURCE_HEADER ResHeader;

		SeekOffset( it->Offset, "Seek to resource header" );

		ReadFile( &ResHeader, sizeof( ResHeader ), "ResourceHeader" );

		if (it->ResourceTypeId != ResHeader.ResourceTypeId)
			throw std::runtime_error( "Resource type id mismatch." );

		if (it->Offset + m_FileSize < it->Offset)
			throw std::runtime_error( "File offset overflow." );

		if (ResHeader.Length > it->Offset + m_FileSize)
			throw std::runtime_error( "Resource extends beyond end of file." );

		//
		// Check if this is a resource that we've got a vested interest in.  If
		// not then just skip it over.  As we have a master directory of
		// resource offsets, we've no need to parse each individual resource
		// type unless we actually need a particular data item.
		//

		switch (it->ResourceTypeId)
		{

		case TRX_AREA_SURFACE_MESH_ID:
			if (m_Mode != ModeTRX)
				continue;

			if (FoundAswm)
				throw std::runtime_error( "Duplicate area surface walkmesh." );

			DecodeAreaSurfaceWalkmesh( &ResHeader, MeshMgr );

			FoundAswm = true;
			break;

		case TRX_WIDTH_HEIGHT_ID:
			if (m_Mode != ModeTRX)
				continue;

			if (FoundTrwh)
				throw std::runtime_error( "Duplicate area width/height data." );

			DecodeAreaWidthHeight( &ResHeader );

			FoundTrwh = true;
			break;

		case TRX_WALKABLE_ID:
			if (m_Mode != ModeMDB)
				continue;

			DecodeWalkable( &ResHeader, MeshMgr );
			break;

		case TRX_WATER_ID:
			if ((m_Mode != ModeTRX) || (m_RefuseDisplayOnlyModels))
				continue;

			DecodeWater( &ResHeader, MeshMgr );
			break;

		case TRX_TERRAIN_ID:
			if ((m_Mode != ModeTRX) || (m_RefuseDisplayOnlyModels))
				continue;

			DecodeTerrain( &ResHeader, MeshMgr );
			break;

		case TRX_COLLISION2_ID:
		case TRX_COLLISION3_ID:
			if (m_Mode != ModeMDB)
				continue;

			DecodeCollisionMesh( &ResHeader, MeshMgr );
			break;

		case TRX_RIGID_ID:
			if ((m_Mode != ModeMDB) || (m_RefuseDisplayOnlyModels))
				continue;

			DecodeRigidMesh( &ResHeader, MeshMgr );
			break;

		case TRX_SKIN_ID:
			if ((m_Mode != ModeMDB) || (m_RefuseDisplayOnlyModels))
				continue;

			DecodeSkinMesh( &ResHeader, MeshMgr );
			break;

		case TRX_COLLISIONSPHERES_ID:
			if (m_Mode != ModeMDB)
				continue;

			DecodeCollisionSpheres( &ResHeader );
			break;

		case TRX_HOOK_ID:
			if (m_Mode != ModeMDB)
				continue;

			DecodeHookPoint( &ResHeader );
			break;

		case TRX_HAIR_ID:
			if (m_Mode != ModeMDB)
				continue;

			DecodeHairPoint( &ResHeader );
			break;

		case TRX_HELM_ID:
			if (m_Mode != ModeMDB)
				continue;

			DecodeHelmPoint( &ResHeader );
			break;

		}
	}

	switch (m_Mode)
	{

	case ModeTRX:

		//
		// Ensure we read walkmesh and TRWH data as both are required.
		//

		if ((!FoundAswm) || (!FoundTrwh))
		{
			throw std::runtime_error(
				"Critical area walkmesh resources missing." );
		}
		break;

	default:
		break;

	}
}

void
TrxFileReader::DecodeAreaSurfaceWalkmesh(
	__in PCRESOURCE_HEADER ResHeader,
	__in MeshManager & MeshMgr
	)
/*++

Routine Description:

	This routine decodes the area surface walkmesh.

Arguments:

	ResHeader - Supplies the current resource object header.
	
	MeshMgr - Supplies the mesh manager to which all child meshes are
	          registered to.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, seeked to the start of the resource in question.

--*/
{
	ASWM_HEADER                    WalkmeshHeader;
	COMPRESSION_HEADER             CompressHeader;
	size_t                         Size;
	std::vector< unsigned char >   Buffer;
	READER_CONTEXT                 BufferReader;
	unsigned long                  u;
	float                          f;
	unsigned long                  IslandCount;
	unsigned long                  FaceOffset;

	if (ResHeader->Length < sizeof( CompressHeader ))
		throw std::exception( "WalkmeshHeader length too small." );

	if (m_LoadOnlyDimensions)
		return;

	ReadFile( &CompressHeader, sizeof( CompressHeader ), "Compress Header" );

	if (CompressHeader.TypeId == TRX_COMPRESSION_HEADER_ID)
	{
		NWN::Compressor              CompressContext;
		std::vector< unsigned char > Compressed;

		//
		// Decompress into a staging buffer.
		//

		Size = CompressHeader.UncompressedSize;

		//
		// Ensure that we can at least fit an ASWM header in there.
		//

		if ((Size < sizeof( ASWM_HEADER ) - 16) || (Size < 1))
			throw std::exception( "Too small compressed walkmesh" );

		//
		// Cap to a reasonable maximum size, just in case.
		//

		if (Size >= 128 * 1024 * 1024)
			throw std::exception( "Too large compressed walkmesh (>128MB)" );

		//
		// Allocate storage for the decompressed block.
		//

		Buffer.resize( Size );

		//
		// Validate lengths and allocate storage for the compressed block.
		//

		if (CompressHeader.CompressedSize < 1)
			throw std::exception( "Too small compressed walkmesh-2" );

		if (CompressHeader.CompressedSize >= 128 * 1024 * 1024)
			throw std::exception( "Too large compressed walkmesh (>128MB)-2" );

		Compressed.resize( CompressHeader.CompressedSize );

		ReadFile(
			&Compressed[ 0 ],
			CompressHeader.CompressedSize,
			"Compressed walkmesh stream");

		//
		// Decompress the compressed stream.
		//

		if (!CompressContext.Uncompress(
			&Compressed[ 0 ],
			Compressed.size( ),
			Buffer))
		{
			throw std::exception( "Walkmesh decompression failed." );
		}

		//
		// Initialize the buffer reader for direct memory reads.
		//

		BufferReader.Buffer = &Buffer[ 0 ];
		BufferReader.Size   = Size;

		//
		// Read the actual ASWM header.
		//

		ReadReaderContext(
			&BufferReader,
			((unsigned char *) (&WalkmeshHeader)),
			sizeof( WalkmeshHeader ),
			"Walkmesh header (remainder)");
	}
	else
	{
		C_ASSERT( sizeof( WalkmeshHeader ) >= sizeof( CompressHeader ) );

		//
		// This is not a compressed ASWM header.  Copy the data we read back
		// into the header.
		//

		memcpy(
			((char *)(&WalkmeshHeader)) + sizeof( CompressHeader ),
			&CompressHeader,
			sizeof( CompressHeader ) );

		//
		// Initialize the buffer reader for file-backed reads.
		//

		Size = ResHeader->Length - (sizeof( CompressHeader ));

		BufferReader.Buffer = NULL;
		BufferReader.Size   = Size;

		//
		// Read the remainder of the ASWM header.
		//

		ReadReaderContext(
			&BufferReader,
			((unsigned char *) (&WalkmeshHeader)) + sizeof( CompressHeader ),
			sizeof( WalkmeshHeader ) - (sizeof( CompressHeader )),
			"Walkmesh header (remainder)");
	}

	if (!WalkmeshHeader.OwnsData)
	{
		throw std::runtime_error(
			"AreaSurfaceWalkmesh should always contain data.");
	}

	if (WalkmeshHeader.Version != 0x6C)
	{
		throw std::runtime_error(
			"Unsupported AreaSurfaceWalkmesh resource version.");
	}

	//
	// Read points.
	//

	for (unsigned long i = 0; i < WalkmeshHeader.PointCount; i += 1)
	{
		NWN::Vector3 v;

		ReadReaderContext(
			&BufferReader,
			&v.x,
			sizeof( v.x ),
			"Point.x");
		ReadReaderContext(
			&BufferReader,
			&v.y,
			sizeof( v.y ),
			"Point.y");
		ReadReaderContext(
			&BufferReader,
			&v.z,
			sizeof( v.z ),
			"Point.z");

		m_Walkmesh.AddPoint( v );
	}

	//
	// Read edges.
	//

	for (unsigned long i = 0; i < WalkmeshHeader.EdgeCount; i += 1)
	{
		AreaSurfaceMesh::SurfaceMeshEdge Edge;

		ReadReaderContext(
			&BufferReader,
			&Edge.Points1,
			sizeof( Edge.Points1 ),
			"Edge.Point1");
		ReadReaderContext(
			&BufferReader,
			&Edge.Points2,
			sizeof( Edge.Points2 ),
			"Edge.Points2");
		ReadReaderContext(
			&BufferReader,
			&Edge.Triangles1,
			sizeof( Edge.Triangles1 ),
			"Edge.Triangles1");
		ReadReaderContext(
			&BufferReader,
			&Edge.Triangles2,
			sizeof( Edge.Triangles2 ),
			"Edge.Triangles2");

		m_Walkmesh.AddEdge( Edge );
	}

	//
	// Read triangles.
	//

	for (unsigned long i = 0; i < WalkmeshHeader.TriangleCount; i += 1)
	{
		AreaSurfaceMesh::SurfaceMeshTriangle Triangle;

		ReadReaderContext(
			&BufferReader,
			&Triangle,
			sizeof( Triangle ),
			"Triangle");

		m_Walkmesh.AddTriangle( Triangle );
	}

	//
	// Tile configuration.
	//

	ReadReaderContext( &BufferReader, &u, sizeof( u ), "Flags" );

	m_Walkmesh.SetFlags( u );

	ReadReaderContext( &BufferReader, &f, sizeof( f ), "TileSize" );

	m_Walkmesh.SetTileSize( f );

	ReadReaderContext( &BufferReader, &u, sizeof( u ), "TileGridHeight" );

	m_Walkmesh.SetTileGridHeight( u );

	ReadReaderContext( &BufferReader, &u, sizeof( u ), "TileGridWidth" );

	m_Walkmesh.SetTileGridWidth( u );

	//
	// Tile data.  Only used to calculate pathing.
	//

	FaceOffset = 0;

	for (unsigned long i = 0; i < m_Walkmesh.GetTileGridHeight( ); i += 1)
	{
		for (unsigned long j = 0; j < m_Walkmesh.GetTileGridWidth( ); j += 1)
		{
			AreaSurfaceMesh::TileSurfaceMesh SurfaceMesh;

			ReadReaderContext(
				&BufferReader,
				&SurfaceMesh.m_Header,
				sizeof( SurfaceMesh.m_Header ),
				"SurfaceMesh.Header");

//			dumphex(
//				(char *)&SurfaceMesh.m_Header,
//				sizeof( SurfaceMesh.m_Header ),
//				33);

			if (SurfaceMesh.m_Header.OwnsData)
			{
				//
				// Read points.
				//

				for (unsigned long i = 0;
				     i < SurfaceMesh.m_Header.NumVerticies;
				     i += 1)
				{
					NWN::Vector3 v;

					ReadReaderContext(
						&BufferReader,
						&v.x,
						sizeof( v.x ),
						"Point.x");
					ReadReaderContext(
						&BufferReader,
						&v.y,
						sizeof( v.y ),
						"Point.y");
					ReadReaderContext(
						&BufferReader,
						&v.z,
						sizeof( v.z ),
						"Point.z");

					SurfaceMesh.AddPoint( v );
				}

				//
				// Read edges.
				//

				for (unsigned long i = 0;
				     i < SurfaceMesh.m_Header.NumEdges;
				     i += 1)
				{
					AreaSurfaceMesh::SurfaceMeshEdge Edge;

					ReadReaderContext(
						&BufferReader,
						&Edge.Points1,
						sizeof( Edge.Points1 ),
						"Edge.Point1");
					ReadReaderContext(
						&BufferReader,
						&Edge.Points2,
						sizeof( Edge.Points2 ),
						"Edge.Points2");
					ReadReaderContext(
						&BufferReader,
						&Edge.Triangles1,
						sizeof( Edge.Triangles1 ),
						"Edge.Triangles1");
					ReadReaderContext(
						&BufferReader,
						&Edge.Triangles2,
						sizeof( Edge.Triangles2 ),
						"Edge.Triangles2");

					SurfaceMesh.AddEdge( Edge );
				}

				//
				// Read triangles.
				//

				for (unsigned long i = 0;
				     i < SurfaceMesh.m_Header.NumFaces;
				     i += 1)
				{
					AreaSurfaceMesh::SurfaceMeshTriangle Triangle;

					ReadReaderContext(
						&BufferReader,
						&Triangle,
						sizeof( Triangle ),
						"Triangle");

					SurfaceMesh.AddTriangle( Triangle );
				}

			}

			//
			// Path table.
			//

			ReadReaderContext(
				&BufferReader,
				&SurfaceMesh.m_PathTable.m_Header,
				sizeof( SurfaceMesh.m_PathTable.m_Header ),
				"SurfaceMesh.m_PathTable.m_Header");

			//
			// Calculate the face offset into the triangles array for this tile
			// of walkmesh.
			//

			SurfaceMesh.m_Faces      = NULL;
			SurfaceMesh.m_NumFaces   = SurfaceMesh.m_Header.NumFaces;
			SurfaceMesh.m_FaceOffset = FaceOffset;

			FaceOffset += SurfaceMesh.m_NumFaces;

			if (SurfaceMesh.m_PathTable.m_Header.Flags & AreaSurfaceMesh::PathTable::PATHFLAG_COMPRESSED)
			{
				throw std::runtime_error(
					"Encountered compressed path table which is unsupported.");
			}

			if (SurfaceMesh.m_PathTable.m_Header.LocalCount != 0)
			{
				if (SurfaceMesh.m_PathTable.m_Header.LocalCount > BufferReader.Size)
					throw std::runtime_error( "Too large walkmesh local node directory." );

				SurfaceMesh.m_PathTable.m_LocalToNodeIndex.resize(
					SurfaceMesh.m_PathTable.m_Header.LocalCount);

				ReadReaderContext(
					&BufferReader,
					&SurfaceMesh.m_PathTable.m_LocalToNodeIndex[ 0 ],
					SurfaceMesh.m_PathTable.m_LocalToNodeIndex.size( ) * 1,
					"SurfaceMesh.m_PathTable.m_LocalToNodeIndex");
			}

			if (SurfaceMesh.m_PathTable.m_Header.NodeCount != 0)
			{
				SurfaceMesh.m_PathTable.m_NodeToLocalIndex.resize(
					SurfaceMesh.m_PathTable.m_Header.NodeCount);

				ReadReaderContext(
					&BufferReader,
					&SurfaceMesh.m_PathTable.m_NodeToLocalIndex[ 0 ],
					SurfaceMesh.m_PathTable.m_NodeToLocalIndex.size( ) * 4,
					"SurfaceMesh.m_PathTable.m_NodeToLocalIndex");

				SurfaceMesh.m_PathTable.m_PathNodes.resize(
					SurfaceMesh.m_PathTable.m_Header.NodeCount *
					SurfaceMesh.m_PathTable.m_Header.NodeCount);

				ReadReaderContext(
					&BufferReader,
					&SurfaceMesh.m_PathTable.m_PathNodes[ 0 ],
					SurfaceMesh.m_PathTable.m_PathNodes.size( ) * 1,
					"SurfaceMesh.m_PathTable.m_PathNodes");
			}

			ReadReaderContext(
				&BufferReader,
				&SurfaceMesh.m_Flags,
				sizeof( SurfaceMesh.m_Flags ),
				"SurfaceMesh.m_Flags");

			m_Walkmesh.AddTileSurfaceMesh( SurfaceMesh );
		}
	}

	if (FaceOffset != m_Walkmesh.GetTriangles( ).size( ))
		throw std::runtime_error( "AreaSurfaceWalkmesh has unclaimed faces" );

	ReadReaderContext( &BufferReader, &u, sizeof( u ), "TileBorderSize" );

	m_Walkmesh.SetTileBorderSize( u );

	ReadReaderContext(
		&BufferReader,
		&IslandCount,
		sizeof( IslandCount ),
		"IslandCount");

	while (IslandCount--)
	{
		AreaSurfaceMesh::Island Island;
		unsigned long           Count;

		ReadReaderContext(
			&BufferReader,
			&Island.m_Header,
			sizeof( Island.m_Header ),
			"Island.m_Header");

		ReadReaderContext(
			&BufferReader,
			&Count,
			sizeof( Count ),
			"NumAdjacent");

		while (Count--)
		{
			unsigned long Adjacent;

			ReadReaderContext(
				&BufferReader,
				&Adjacent,
				sizeof( Adjacent ),
				"Adjacent");

			Island.AddAdjacent( Adjacent );
		}

		ReadReaderContext(
			&BufferReader,
			&Count,
			sizeof( Count ),
			"NumAdjacentDist");

		while (Count--)
		{
			float AdjacentDist;

			ReadReaderContext(
				&BufferReader,
				&AdjacentDist,
				sizeof( AdjacentDist ),
				"AdjacentDist");

			Island.AddAdjacentDist( AdjacentDist );
		}

		ReadReaderContext(
			&BufferReader,
			&Count,
			sizeof( Count ),
			"NumExitFace");

		while (Count--)
		{
			unsigned long ExitFace;

			ReadReaderContext(
				&BufferReader,
				&ExitFace,
				sizeof( ExitFace ),
				"ExitFace");

			Island.AddExitFace( ExitFace );
		}

		m_Walkmesh.AddIsland( Island );
	}

	m_Walkmesh.GetIslandPathTable( ).resize(
		m_Walkmesh.GetIslands( ).size( ) * m_Walkmesh.GetIslands( ).size( ) );

	if (!m_Walkmesh.GetIslandPathTable( ).empty( ))
	{
		ReadReaderContext(
			&BufferReader,
			&m_Walkmesh.GetIslandPathTable( )[ 0 ],
			m_Walkmesh.GetIslandPathTable( ).size( ) *
				sizeof( AreaSurfaceMesh::IslandPathNodeVec::value_type ),
			"IslandPathTable");
	}

	//
	// Validate walkmesh data now that we've read it all in.
	//

	m_Walkmesh.Validate( );

	//
	// Set up the face base pointers for each tile surface mesh.
	//

	for (AreaSurfaceMesh::TileSurfaceMeshVec::iterator it = m_Walkmesh.GetTileSurfaceMeshes( ).begin( );
	     it != m_Walkmesh.GetTileSurfaceMeshes( ).end( );
	     ++it)
	{
		if (it->m_NumFaces != 0)
			it->m_Faces = &m_Walkmesh.GetTriangles( )[ it->m_FaceOffset ];
	}

	//
	// Calculate bounding boxes.
	//

	m_Walkmesh.CalcBoundingBoxes( );

	//
	// Register the mesh with the mesh manager as we have initialized it
	// proper.
	//

	m_Walkmesh.RegisterMesh( MeshMgr );
}


void
TrxFileReader::DecodeAreaWidthHeight(
	__in PCRESOURCE_HEADER ResHeader
	)
/*++

Routine Description:

	This routine decodes the area width/height header block and updates the
	reader with the deserialized data.

Arguments:

	ResHeader - Supplies the current resource object header.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, seeked to the start of the resource in question.

--*/
{
	TRWH_HEADER WidthHeight;

	if (ResHeader->Length < sizeof( WidthHeight ))
		throw std::runtime_error( "WidthHeight length too small." );

	ReadFile( &WidthHeight, sizeof( WidthHeight ), "Area Width/Height" );

	m_Width  = WidthHeight.Width;
	m_Height = WidthHeight.Height;
}

void
TrxFileReader::DecodeWalkable(
	__in PCRESOURCE_HEADER ResHeader,
	__in MeshManager & MeshMgr
	)
/*++

Routine Description:

	This routine decodes raw object walk information (non-baked).

Arguments:

	ResHeader - Supplies the current resource object header.
		
	MeshMgr - Supplies the mesh manager to which all child meshes are
	          registered to.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, seeked to the start of the resource in question.

--*/
{
	DecodeSimpleMesh( ResHeader, MeshMgr, GetCollider( ).GetWalkMesh( ) );
}

void
TrxFileReader::DecodeWater(
	__in PCRESOURCE_HEADER ResHeader,
	__in MeshManager & MeshMgr
	)
/*++

Routine Description:

	This routine decodes an area water mesh.

Arguments:

	ResHeader - Supplies the current resource object header.
		
	MeshMgr - Supplies the mesh manager to which all child meshes are
	          registered to.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, seeked to the start of the resource in question.

--*/
{
	WATR_HEADER                WaterHeader;
	AreaWaterMesh::WaterBitmap Bitmap;
	unsigned long              X;
	unsigned long              Y;

	if (m_LoadOnlyDimensions)
		return;

	//
	// Create a new water mesh object and begin filling it out.
	//

	m_WaterMesh.push_back( AreaWaterMesh( ) );

	AreaWaterMesh & WaterMesh = m_WaterMesh.back( );

	if (ResHeader->Length < sizeof( WaterHeader ))
		throw std::runtime_error( "Water length too small." );

	ReadFile( &WaterHeader, sizeof( WaterHeader ), "Area Water" );

	WaterMesh.SetWaterColor( WaterHeader.WaterColor );

	//
	// Read verticies.
	//

	for (unsigned long i = 0; i < WaterHeader.VertexCount; i += 1)
	{
		AreaWaterMesh::WaterVertex Vertex;

		ReadFile( &Vertex, sizeof( Vertex ), "Water Vertex" );

		WaterMesh.AddWaterVertex( Vertex );
	}

	//
	// Triangles.
	//

	for (unsigned long i = 0; i < WaterHeader.TriangleCount; i += 1)
	{
		AreaWaterMesh::WaterFace Face;

		ReadFile( &Face, sizeof( Face ), "Water Face" );

		WaterMesh.AddWaterFace( Face );
	}

	//
	// Bitmap.
	//

	ReadFile( &Bitmap, sizeof( Bitmap ), "Water Bitmap" );

	WaterMesh.SetWaterBitmap( Bitmap );

	//
	// Image.
	//

	ReadDdsImage( WaterMesh.GetImage( ) );

	ReadFile( &X, sizeof( X ), "Water X" );
	ReadFile( &Y, sizeof( Y ), "Water Y" );

	WaterMesh.SetWaterX( X );
	WaterMesh.SetWaterY( Y );

	//
	// Validate consistency for all indicies.
	//

	WaterMesh.Validate( );

	//
	// Update the height map with new water information.
	//

	m_HeightMap.ComputeWaterHeights( WaterMesh );

	WaterMesh.RegisterMesh( MeshMgr );
}

void
TrxFileReader::DecodeTerrain(
	__in PCRESOURCE_HEADER ResHeader,
	__in MeshManager & MeshMgr
	)
/*++

Routine Description:

	This routine decodes an area terrain mesh.

Arguments:

	ResHeader - Supplies the current resource object header.
		
	MeshMgr - Supplies the mesh manager to which all child meshes are
	          registered to.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, seeked to the start of the resource in question.

--*/
{
	TRRN_HEADER   TerrainHeader;
	unsigned long GrassCount;

	if (m_LoadOnlyDimensions)
		return;

	if (ResHeader->Length < sizeof( TerrainHeader ))
		throw std::runtime_error( "Terrain length too small." );

	ReadFile( &TerrainHeader, sizeof( TerrainHeader ), "Area Terrain" );

	//
	// Create a new terrain mesh object and begin filling it out.
	//

	m_TerrainMesh.push_back( AreaTerrainMesh( ) );

	AreaTerrainMesh & TerrainMesh = m_TerrainMesh.back( );

	TerrainMesh.SetTextures( &TerrainHeader.Texture[ 0 ].Name );
	TerrainMesh.SetTextureColor( &TerrainHeader.TextureColor[ 0 ] );

	//
	// Read verticies.
	//

	for (unsigned long i = 0; i < TerrainHeader.VertexCount; i += 1)
	{
		AreaTerrainMesh::TerrainVertex Vertex;

		ReadFile( &Vertex, sizeof( Vertex ), "Terrain Vertex" );

		TerrainMesh.AddTerrainVertex( Vertex );
	}

	//
	// Read triangles.
	//

	for (unsigned long i = 0; i < TerrainHeader.TriangleCount; i += 1)
	{
		AreaTerrainMesh::TerrainFace Face;

		ReadFile( &Face, sizeof( Face ), "Terrain Face" );

		TerrainMesh.AddTerrainFace( Face );
	}

	//
	// Images.
	//

	ReadDdsImage( TerrainMesh.GetImage( 0 ) );
	ReadDdsImage( TerrainMesh.GetImage( 1 ) );

	ReadFile( &GrassCount, sizeof( GrassCount ), "Terrain Grass Count" );

	for (unsigned long i = 0; i < GrassCount; i += 1)
	{
		AreaTerrainMesh::TerrainGrass Grass;

		ReadFile( &Grass.Header, sizeof( Grass.Header ), "Grass Header" );

		for (unsigned long j = 0; j < Grass.Header.Blades; j += 1)
		{
			AreaTerrainMesh::TerrainGrassBlade Blade;

			ReadFile( &Blade, sizeof( Blade ), "Grass Blade" );

			Grass.Blades.push_back( Blade );
		}

		TerrainMesh.AddTerrainGrass( Grass );
	}

	TerrainMesh.Validate( );

	//
	// Update the height map with new terrain information.
	//

	m_HeightMap.ComputeHeights( TerrainMesh );

	TerrainMesh.RegisterMesh( MeshMgr );
}

void
TrxFileReader::DecodeCollisionMesh(
	__in PCRESOURCE_HEADER ResHeader,
	__in MeshManager & MeshMgr
	)
/*++

Routine Description:

	This routine decodes a collision mesh.

Arguments:

	ResHeader - Supplies the current resource object header.
		
	MeshMgr - Supplies the mesh manager to which all child meshes are
	          registered to.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, seeked to the start of the resource in question.

--*/
{
	CollisionMesh * Mesh;

	//
	// Determine where to store the resultant mesh.
	//

	switch (ResHeader->ResourceTypeId)
	{

	case TRX_COLLISION2_ID:
		Mesh = &GetCollider( ).GetC2Mesh( );
		break;

	case TRX_COLLISION3_ID:
		Mesh = &GetCollider( ).GetC3Mesh( );
		break;

	default:
		throw std::runtime_error( "Unrecognized collision mesh type." );

	}

	//
	// Read the mesh.
	//

	DecodeSimpleMesh( ResHeader, MeshMgr, *Mesh );

	//
	// Precalculate normals.
	//

	Mesh->Precalculate( );
}

void
TrxFileReader::DecodeRigidMesh(
	__in PCRESOURCE_HEADER ResHeader,
	__in MeshManager & MeshMgr
	)
/*++

Routine Description:

	This routine decodes a rigid mesh.

	The same model may have multiple rigid meshes if meshes require different
	textures.  Each mesh is already pretransformed into a unified local
	coordinate space; that is, there is no per-rigid-mesh local coordinate
	space.

Arguments:

	ResHeader - Supplies the current resource object header.

	MeshMgr - Supplies the mesh manager to which all child meshes are
	          registered to.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, seeked to the start of the resource in question.

--*/
{
	GetCollider( ).GetRigidMeshes( ).push_back( RigidMesh( ) );

	DecodeSimpleMesh( ResHeader, MeshMgr, GetCollider( ).GetRigidMeshes( ).back( ) );
}

void
TrxFileReader::DecodeSkinMesh(
	__in PCRESOURCE_HEADER ResHeader,
	__in MeshManager & MeshMgr
	)
/*++

Routine Description:

	This routine decodes a skin mesh.

Arguments:

	ResHeader - Supplies the current resource object header.

	MeshMgr - Supplies the mesh manager to which all child meshes are
	          registered to.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, seeked to the start of the resource in question.

--*/
{
	GetCollider( ).GetSkinMeshes( ).push_back( SkinMesh( ) );

	DecodeSimpleMesh( ResHeader, MeshMgr, GetCollider( ).GetSkinMeshes( ).back( ) );
}

template< typename T >
void
TrxFileReader::DecodeSimpleMesh(
	__in PCRESOURCE_HEADER ResHeader,
	__in MeshManager & MeshMgr,
	__inout T & Mesh
	)
/*++

Routine Description:

	This template routine decodes a simple mesh that conforms to the
	requirements for a SimpleMesh-derived template class.

Arguments:

	ResHeader - Supplies the current resource object header.

	Mesh - Supplies the mesh to update (or replace) with the file data.

	MeshMgr - Supplies the mesh manager to which all child meshes are
	          registered to.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, seeked to the start of the resource in question.

--*/
{
	T::Header & MeshHeader = Mesh.GetHeader( );

	//
	// Pull the header in and determine where to store the resultant mesh.
	//

	if (ResHeader->Length < sizeof( MeshHeader ))
		throw std::runtime_error( "Simple mesh length too small." );

	ReadFile( &MeshHeader, sizeof( MeshHeader ), "Simple Mesh Header" );

	//
	// Multiple of the same mesh resource aren't supported, so ensure that the
	// mesh we are looking at is really empty before we begin.
	//

	Mesh.Clear( );

	//
	// Pull in the verticies and triangles.
	//

	if (MeshHeader.NumVerts <= 0xFFFF)
		Mesh.GetPoints( ).reserve( MeshHeader.NumVerts );
	else
		Mesh.GetPoints( ).reserve( 0xFFFF );

	if (MeshHeader.NumFaces <= 0xFFFF)
		Mesh.GetFaces( ).reserve( MeshHeader.NumFaces );
	else
		Mesh.GetFaces( ).reserve( 0xFFFF );

	for (unsigned long i = 0; i < MeshHeader.NumVerts; i += 1)
	{
		T::VertexFile VertFile;
		T::Vertex     Vert;

		ReadFile( &VertFile, sizeof( VertFile ), "Simple Mesh Vertex" );
		T::CopyFileVertex( VertFile, Vert );
		Mesh.AddPoint( Vert );
	}

	for (unsigned long i = 0; i < MeshHeader.NumFaces; i += 1)
	{
		T::FaceFile FaceFile;
		T::Face     Face;

		ReadFile( &FaceFile, sizeof( FaceFile ), "Simple Mesh Face" );
		T::CopyFileFace( FaceFile, Face );
		Mesh.AddFace( Face );
	}

	//
	// Validate the mesh contents.
	//

	Mesh.Validate( );

	Mesh.RegisterMesh( MeshMgr );
}

void
TrxFileReader::DecodeCollisionSpheres(
	__in PCRESOURCE_HEADER ResHeader
	)
/*++

Routine Description:

	This routine decodes a list of collision spheres.

Arguments:

	ResHeader - Supplies the current resource object header.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, seeked to the start of the resource in question.

--*/
{
	CollisionSphereList         & Spheres    = GetCollider( ).GetCollisionSpheres( );
	CollisionSphereList::Header & ListHeader = Spheres.GetHeader( );

	//
	// Pull the header in read the item contents.
	//

	if (ResHeader->Length < sizeof( ListHeader ))
		throw std::runtime_error( "Collision sphere list length too small." );

	ReadFile( &ListHeader, sizeof( ListHeader ), "Collision Spheres Header" );

	//
	// Ensure we don't already have a collision sphere list, as there is only
	// one authoritative list per MDB.
	//

	Spheres.Clear( );

	//
	// Pull in the spheres.
	//

	if (ListHeader.NumItems <= 0xFFFF)
		Spheres.GetSpheres( ).reserve( ListHeader.NumItems );
	else
		Spheres.GetSpheres( ).reserve( 0xFFFF );

	for (unsigned long i = 0; i < ListHeader.NumItems; i += 1)
	{
		CollisionSphereList::ItemFile ItemF;
		CollisionSphereList::Item     ItemM;

		ReadFile( &ItemF, sizeof( ItemF ), "Collision Sphere Item" );
		CollisionSphereList::CopyFileItem( ItemF, ItemM );
		Spheres.AddSphere( ItemM );
	}

	//
	// Validate the sphere contents.
	//

	Spheres.Validate( );
}

void
TrxFileReader::DecodeHookPoint(
	__in PCRESOURCE_HEADER ResHeader
	)
/*++

Routine Description:

	This routine decodes a hook point.

Arguments:

	ResHeader - Supplies the current resource object header.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, seeked to the start of the resource in question.

--*/
{
	HookPoint Hook;

	//
	// Pull the header in read the item contents.
	//

	if (ResHeader->Length < sizeof( HookPoint::Header ))
		throw std::runtime_error( "Hook point is too small." );

	ReadFile( &Hook.GetHeader( ), sizeof( HookPoint::Header ), "Hook Header" );

	//
	// Validate the hook point.
	//

	Hook.Validate( );

	//
	// Add it to the hook point list.
	//

	GetCollider( ).GetHookPoints( ).push_back( Hook );
}

void
TrxFileReader::DecodeHairPoint(
	__in PCRESOURCE_HEADER ResHeader
	)
/*++

Routine Description:

	This routine decodes a hair point.

Arguments:

	ResHeader - Supplies the current resource object header.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, seeked to the start of the resource in question.

--*/
{
	HairPoint & Hair = GetCollider( ).GetHairPoint( );

	//
	// Pull the header in read the item contents.
	//

	if (ResHeader->Length < sizeof( HairPoint::Header ))
		throw std::runtime_error( "Hair point is too small." );

	ReadFile( &Hair.GetHeader( ), sizeof( HairPoint::Header ), "Hair Header" );

	//
	// Validate the hair point.
	//

	Hair.Validate( );
}

void
TrxFileReader::DecodeHelmPoint(
	__in PCRESOURCE_HEADER ResHeader
	)
/*++

Routine Description:

	This routine decodes a helm point.

Arguments:

	ResHeader - Supplies the current resource object header.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, seeked to the start of the resource in question.

--*/
{
	HelmPoint & Helm = GetCollider( ).GetHelmPoint( );

	//
	// Pull the header in read the item contents.
	//

	if (ResHeader->Length < sizeof( HelmPoint::Header ))
		throw std::runtime_error( "Helm point is too small." );

	ReadFile( &Helm.GetHeader( ), sizeof( HelmPoint::Header ), "Helm Header" );

	//
	// Validate the helm point.
	//

	Helm.Validate( );
}
