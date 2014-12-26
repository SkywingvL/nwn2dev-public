/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ErfFileReader.cpp

Abstract:

	This module houses the *.erf file format parser, which is used to read
	encapsulated resource file format (ERF) files.

--*/

#include "Precomp.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "ErfFileReader.h"

template< typename ResRefT >
ErfFileReader< ResRefT >::ErfFileReader(
	__in const std::string & FileName
	)
/*++

Routine Description:

	This routine constructs a new ErfFileReader object and parses the contents
	of an ERF file by filename.  The file must already exist as it is
	immediately deserialized.

Arguments:

	FileName - Supplies the path to the ERF file.

Return Value:

	The newly constructed object.

Environment:

	User mode.

--*/
: m_File( INVALID_HANDLE_VALUE ),
  m_FileSize( 0 ),
  m_NextOffset( 0 ),
  m_FileName( FileName )
{
	HANDLE File;

	File = CreateFileA(
		FileName.c_str( ),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (File == INVALID_HANDLE_VALUE)
	{
		File = CreateFileA(
				FileName.c_str( ),
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

		if (File == INVALID_HANDLE_VALUE)
			throw std::exception( "Failed to open ERF file." );
	}

	m_File = File;

	//
	// N.B.  We don't use memory mapped I/O for .HAKs due to address space 
	//       pressure on 32-bit builds (if we do, we tend to run the client
	//       out of address space in client extension mode).  On 64-bit builds,
	//       the available address space is so much larger than the sum total
	//       of content loaded that it's better to just use a mapped view.
	//

#if !defined(_WIN64)
	m_FileWrapper.SetFileHandle( File, false );
#else
	m_FileWrapper.SetFileHandle( File, true );
#endif

	try
	{
		m_FileSize = GetFileSize( File, NULL );

		if ((m_FileSize == 0xFFFFFFFF) && (GetLastError( ) != NO_ERROR))
			throw std::exception( "Failed to read file size." );

		ParseErfFile( );
	}
	catch (...)
	{
		m_File = INVALID_HANDLE_VALUE;

		CloseHandle( File );

		throw;
	}

	C_ASSERT( sizeof( ERF_HEADER ) == 160 );
	C_ASSERT( sizeof( ERF_KEY ) == 8 + sizeof( ResRefT ) );
	C_ASSERT( sizeof( RESOURCE_LIST_ELEMENT ) == 8 );
}

template< typename ResRefT >
ErfFileReader< ResRefT >::~ErfFileReader(
	)
/*++

Routine Description:

	This routine cleans up an already-existing ErfFileReader object.

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

template< typename ResRefT >
typename ErfFileReader< ResRefT >::FileHandle
ErfFileReader< ResRefT >::OpenFile(
	__in const ResRefIf & FileName,
	__in ResType Type
	)
/*++

Routine Description:

	This routine logically opens an encapsulated sub-file within the ERF file.

	Currently, file handles are implemented as simply ResID indicies.
	Thus, "opening" a file simply involves looking up its ResID.

Arguments:

	FileName - Supplies the name of the resource file to open.

	Type - Supplies the type of file to open (i.e. ResTRN, ResARE).

Return Value:

	The routine returns a new file handle on success.  The file handle must be
	closed by a call to CloseFile on successful return.

	On failure, the routine returns the manifest constant INVALID_FILE, which
	should not be closed.

Environment:

	User mode.

--*/
{
	PCERF_KEY Key;

//	WriteText( "Checking for key %.32s/%lu -- %lu keys\n", FileName.RefStr, Type, m_KeyDir.size( ) );

	Key = LookupResourceKey( FileName, Type );

	if (Key == NULL)
		return INVALID_FILE;

	return (FileHandle) (Key->ResourceID + 1);
}

template< typename ResRefT >
typename ErfFileReader< ResRefT >::FileHandle
ErfFileReader< ResRefT >::OpenFileByIndex(
	__in typename FileId FileIndex
	)
/*++

Routine Description:

	This routine logically opens an encapsulated sub-file within the ERF file.

	Currently, file handles are implemented as simply ResID indicies.
	Thus, "opening" a file simply involves looking up its ResID.

Arguments:

	FileIndex - Supplies the directory index of the file to open.

Return Value:

	The routine returns a new file handle on success.  The file handle must be
	closed by a call to CloseFile on successful return.

	On failure, the routine returns the manifest constant INVALID_FILE, which
	should not be closed.

Environment:

	User mode.

--*/
{
	PCERF_KEY Key;

	Key = LookupResourceKey( (ResID) FileIndex );

	if (Key == NULL)
		return INVALID_FILE;

	return (FileHandle) (Key->ResourceID + 1);
}

template< typename ResRefT >
bool
ErfFileReader< ResRefT >::CloseFile(
	__in typename ErfFileReader< ResRefT >::FileHandle File
	)
/*++

Routine Description:

	This routine logically closes an encapsulated sub-file within the ERF file.

	Currently, file handles are implemented as simply ResID indicies.
	Thus, "closing" a file involves no operation.

Arguments:

	File - Supplies the file handle to close.

Return Value:

	The routine returns a Boolean value indicating true on success, else false
	on failure.  A return value of false typically indicates a serious
	programming error upon the caller (i.e. reuse of a closed file handle).

Environment:

	User mode.

--*/
{
	if (File == INVALID_FILE)
		return false;

	return true;
}

template< typename ResRefT >
bool
ErfFileReader< ResRefT >::ReadEncapsulatedFile(
	__in FileHandle File,
	__in size_t Offset,
	__in size_t BytesToRead,
	__out size_t * BytesRead,
	__out_bcount( BytesToRead ) void * Buffer
	)
/*++

Routine Description:

	This routine logically reads an encapsulated sub-file within the ERF file.

	File reading is optimized for sequential scan.

Arguments:

	File - Supplies a file handle to the desired sub-file to read.

	Offset - Supplies the offset into the desired sub-file to read from.

	BytesToRead - Supplies the requested count of bytes to read.

	BytesRead - Receives the count of bytes transferred.

	Buffer - Supplies the address of a buffer to transfer raw encapsulated file
	         contents to.

Return Value:

	The routine returns a Boolean value indicating true on success, else false
	on failure.  An attempt to read from an invalid file handle, or an attempt
	to read beyond the end of file would be examples of failure conditions.

Environment:

	User mode.

--*/
{
	PCRESOURCE_LIST_ELEMENT ResElem;
	ULONGLONG               NextOffset;

	ResElem = LookupResourceDirectory( ((ResID) File) - 1 );

	*BytesRead = 0;

	if (ResElem == NULL)
		return false;

	if (Offset >= ResElem->ResourceSize)
		return false;

	BytesToRead = min( BytesToRead, ResElem->ResourceSize - Offset);

	try
	{
		NextOffset = (ULONGLONG) ResElem->OffsetToResource + Offset;

		if (NextOffset != m_NextOffset)
		{
			m_FileWrapper.SeekOffset(
				NextOffset,
				"OffsetToResource + Offset");

			m_NextOffset = NextOffset;
		}

		m_FileWrapper.ReadFile( Buffer, BytesToRead, "File Contents" );

		m_NextOffset += BytesToRead;

		*BytesRead = BytesToRead;

		return true;
	}
	catch (std::exception)
	{
		return false;
	}
}

template< typename ResRefT >
size_t
ErfFileReader< ResRefT >::GetEncapsulatedFileSize(
	__in typename ErfFileReader< ResRefT >::FileHandle File
	)
/*++

Routine Description:

	This routine returns the size, in bytes, of an encapsulated file.

Arguments:

	File - Supplies the file handle to query the size of.

Return Value:

	The routine returns the size of the given file.  If a valid file handle is
	supplied, then the routine never fails.

	Should an illegal file handle be supplied, the routine returns zero.  There
	is no way to distinguish this condition from legal file handle to a file
	with zero length.  Only a serious programming error results in a caller
	supplying an illegal file handle.

Environment:

	User mode.

--*/
{
	PCRESOURCE_LIST_ELEMENT ResElem;

	ResElem = LookupResourceDirectory( ((ResID) File) - 1 );

	if (ResElem == NULL)
		return false;

	return ResElem->ResourceSize;
}

template< typename ResRefT >
typename ErfFileReader< ResRefT >::AccessorType
ErfFileReader< ResRefT >::GetResourceAccessorName(
	__in FileHandle File,
	__out std::string & AccessorName
	)
/*++

Routine Description:

	This routine returns the logical name of the resource accessor.

Arguments:

	File - Supplies the file handle to inquire about.

	AccessorName - Receives the logical name of the resource accessor.

Return Value:

	The routine returns the accessor type.  An std::exception is raised on
	failure.

Environment:

	User mode.

--*/
{
	UNREFERENCED_PARAMETER( File );

	AccessorName = m_FileName;
	return AccessorTypeErf;
}

template< typename ResRefT >
typename ErfFileReader< ResRefT >::ResType
ErfFileReader< ResRefT >::GetEncapsulatedFileType(
	__in typename ErfFileReader< ResRefT >::FileHandle File
	)
/*++

Routine Description:

	This routine returns the type of an encapsulated file.

Arguments:

	File - Supplies the file handle to query the size of.

Return Value:

	The routine returns the type of the given file.  If a valid file handle is
	supplied, then the routine never fails.

	Should an illegal file handle be supplied, the routine returns ResINVALID.
	There is no way to distinguish this condition from legal file handle to a
	file of type ResINVALID.  Only a serious programming error results in a
	caller supplying an illegal file handle.

Environment:

	User mode.

--*/
{
	PCERF_KEY Key;

	Key = LookupResourceKey( ((ResID) File) - 1 );

	if (Key == NULL)
		return NWN::ResINVALID;

	return Key->Type;
}

template< typename ResRefT >
bool
ErfFileReader< ResRefT >::GetEncapsulatedFileEntry(
	__in typename FileId FileIndex,
	__out typename ResRefIf & ResRef,
	__out ResType & Type
	)
/*++

Routine Description:

	This routine reads an encapsulated file directory entry, returning the name
	and type of a particular resource.  The enumeration is stable across calls.

Arguments:

	FileIndex - Supplies the index into the logical directory entry to reutrn.

	ResRef - Receives the resource name.

	Type - Receives the resource type.

Return Value:

	The routine returns a Boolean value indicating success or failure.  The
	routine always succeeds as long as the caller provides a legal file index.

Environment:

	User mode.

--*/
{
	PCERF_KEY ResKey;

	C_ASSERT( sizeof( ResRefT ) <= sizeof( ResRefIf ) );

	ResKey = LookupResourceKey( (ResID) FileIndex );

	if (ResKey == NULL)
		return false;

	ZeroMemory( &ResRef, sizeof( ResRef ) );
	memcpy( &ResRef, &ResKey->FileName, sizeof( ResKey->FileName ) );
	Type = ResKey->Type;

	return true;
}

template< typename ResRefT >
typename ErfFileReader< ResRefT >::FileId
ErfFileReader< ResRefT >::GetEncapsulatedFileCount(
	)
/*++

Routine Description:

	This routine returns the count of files in this resource accessor.  The
	highest valid file index is the returned count minus one, unless there are
	zero files, in which case no file index values are legal.

Arguments:

	None.

Return Value:

	The routine returns the count of files present.

Environment:

	User mode.

--*/
{
	return m_KeyDir.size( );
}

template< typename ResRefT >
void
ErfFileReader< ResRefT >::ParseErfFile(
	)
/*++

Routine Description:

	This routine parses the directory structures of an ERF file and generates
	the in-memory key and resource list entry directories.

Arguments:

	None.

Return Value:

	None.  On failure, the routine raises an std::exception.

Environment:

	User mode.

--*/
{
	ERF_HEADER Header;

	m_FileWrapper.ReadFile( &Header, sizeof( Header ), "Header" );

	if (Header.EntryCount < 1024 * 1024)
	{
		m_KeyDir.reserve( Header.EntryCount );
		m_ResDir.reserve( Header.EntryCount );
	}
	else
	{
		m_KeyDir.reserve( 1024 * 1024 );
		m_ResDir.reserve( 1024 * 1024 );
	}

	m_FileWrapper.SeekOffset( Header.OffsetToKeyList, "OffsetToKeyList" );

	for (unsigned long i = 0; i < Header.EntryCount; i += 1)
	{
		ERF_KEY Key;

		m_FileWrapper.ReadFile( &Key, sizeof( Key ), "Key" );

		//
		// Saved game module.ifo files are written with some entries that have
		// invalid ResourceIDs that don't match their actual indicies.  Fix
		// them as we have no other choice.
		//

		if ((ResID) i != Key.ResourceID)
		{
			if (Key.ResourceID == 0)
				Key.ResourceID = (ResID) i;
			else
				throw std::runtime_error( "Key.ResourceID mismatch" );
		}

		m_KeyDir.push_back( Key );
	}

	m_FileWrapper.SeekOffset(
		Header.OffsetToResourceList,
		"OffsetToResourceList" );

	for (unsigned long i = 0; i < Header.EntryCount; i += 1)
	{
		RESOURCE_LIST_ELEMENT Entry;

		m_FileWrapper.ReadFile( &Entry, sizeof( Entry ), "Entry" );

		if ((size_t) Entry.OffsetToResource + Entry.ResourceSize < Entry.OffsetToResource)
			throw std::runtime_error( "Entry.OffsetToResource overflow" );

		if ((ULONGLONG) Entry.OffsetToResource + Entry.ResourceSize > (ULONGLONG) m_FileSize)
			throw std::runtime_error( "ERF entry exceeds file size" );

		m_ResDir.push_back( Entry );
	}
}

template ErfFileReader< NWN::ResRef32 >;
template ErfFileReader< NWN::ResRef16 >;
