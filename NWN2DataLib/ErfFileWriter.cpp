/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ErfFileWriter.cpp

Abstract:

	This module houses the *.erf file format writer, which is used to create
	encapsulated resource file format (ERF) files.

--*/

#include "Precomp.h"
#include "ResourceManager.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "ErfFileWriter.h"

template< typename ResRefT >
ErfFileWriter< ResRefT >::ErfFileWriter(
	)
/*++

Routine Description:

	This routine constructs a new ErfFileWriter object.

Arguments:

	None.

Return Value:

	The newly constructed object.

Environment:

	User mode.

--*/
: m_FileType( ERF_FILE_TYPE )
{
}

template< typename ResRefT >
ErfFileWriter< ResRefT >::~ErfFileWriter(
	)
/*++

Routine Description:

	This routine cleans up an already-existing ErfFileWriter object.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
}

template< typename ResRefT >
bool
ErfFileWriter< ResRefT >::Commit(
	__in const std::string & FileName,
	__in unsigned long FileType, /* = 0 */
	__in unsigned long Flags /* = 0 */
	)
/*++

Routine Description:

	This routine writes the staged ERF contents to a disk file.

Arguments:

	FileName - Supplies the name of the file to write to.

	FileType - Supplies the type tag of the file (ERF, MOD, etc.)

	Flags - Supplies flags that control the behavior of the commit operation.
	        Legal values are drawn from the ERF_COMMIT_FLAG_* family of values.

Return Value:

	The routine returns true if the data was committed to disk, else false if
	the commit failed.

Environment:

	User mode.

--*/
{
	HANDLE File;

	File = INVALID_HANDLE_VALUE;

	try
	{
		//
		// Create a write abstraction context for the disk file and perform the
		// commit operation.
		//

		ErfWriteContext Context;

		File = CreateFileA(
			FileName.c_str( ),
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (File == INVALID_HANDLE_VALUE)
			throw std::runtime_error( "Failed to open file." );

		Context.Type = ErfWriteContext::ContextTypeFile;
		Context.File = File;

		CommitInternal( &Context, FileType, Flags );

		CloseHandle( File );
		File = INVALID_HANDLE_VALUE;
	}
	catch (std::exception)
	{
		if (File != INVALID_HANDLE_VALUE)
			CloseHandle( File );

		return false;
	}

	return true;
}

template< typename ResRefT >
bool
ErfFileWriter< ResRefT >::Commit(
	__out std::vector< unsigned char > & Memory,
	__in unsigned long FileType, /* = 0 */
	__in unsigned long Flags /* = 0 */
	)
/*++

Routine Description:

	This routine writes the staged ERF contents to an in-memory buffer.

Arguments:

	Memory - Supplies the buffer that receives the ERF contents.  The contents
	         of the buffer are replaced with the ERF contents.

	FileType - Supplies the type tag of the file (ERF, MOD, etc.)

	Flags - Supplies flags that control the behavior of the commit operation.
	        Legal values are drawn from the ERF_COMMIT_FLAG_* family of values.

Return Value:

	The routine returns true if the data was committed to memory, else false if
	the commit failed.

Environment:

	User mode.

--*/
{

	try
	{
		//
		// Create a write abstraction context for the in-memory buffer and
		// perform the commit operation.
		//

		ErfWriteContext Context;

		Context.Type   = ErfWriteContext::ContextTypeMemory;
		Context.Memory = &Memory;

		Memory.clear( );

		CommitInternal( &Context, FileType, Flags );
	}
	catch (std::exception)
	{
		return false;
	}

	return true;
}

template< typename ResRefT >
void
ErfFileWriter< ResRefT >::InitializeFromResourceAccessor(
	__in IResourceAccessor< ResRefIf > * Accessor,
	__in bool CheckForDuplicates /* = false */
	)
/*++

Routine Description:

	This routine initializes an ERF writer with the contents of an existing
	resource accessor (which may or may not be an ERF reader).  The contents
	are staged for future writing.

	N.B.  It is the responsibility of the caller to ensure that the contents of
	      the resource accessor do not conflict with any existing contents that
	      are staged for writing if CheckForDuplicates is false.

Arguments:

	Accessor - Supplies the resource accessor to pull resources from.

	CheckForDuplicates - Supplies a Boolean value that indicates whether the
	                     existing staged contents are checked for duplicate
	                     entries (which are then deleted) before addition.  The
	                     check for duplicates option is potentially very slow
	                     when working with large resource sets.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	ResRefIf                                         ResRef;
	ResType                                          ResType;
	swutil::SharedByteVec                            Contents;
	typename IResourceAccessor< ResRefIf >::FileId   CurId;
	typename IResourceAccessor< ResRefIf >::FileId   MaxId;

	MaxId = Accessor->GetEncapsulatedFileCount( );

	for (CurId = 0; CurId < MaxId; CurId += 1)
	{
		if (!Accessor->GetEncapsulatedFileEntry( CurId, ResRef, ResType ))
			throw std::runtime_error( "GetEncapsulatedFileEntry failed." );

		//
		// If we are to check for duplicate entries, try and remove any
		// already-existing entry.
		//

		if (CheckForDuplicates)
			RemoveFile( ResRef, ResType );

		//
		// Add the file contents.
		//

		Contents = new std::vector< unsigned char >;

		ResourceManager::LoadEncapsulatedFile( Accessor, CurId, *Contents );

		AddFile( ResRef, ResType, Contents );
	}
}

template< typename ResRefT >
void
ErfFileWriter< ResRefT >::InitializeFromResourceAccessor(
	__in DirectoryFileReader< ResRefIf > * Accessor,
	__in bool CheckForDuplicates /* = false */
	)
/*++

Routine Description:

	This routine initializes an ERF writer with the contents of an existing
	resource accessor (which may or may not be an ERF reader).  The contents
	are staged for future writing.

	N.B.  It is the responsibility of the caller to ensure that the contents of
	      the resource accessor do not conflict with any existing contents that
	      are staged for writing if CheckForDuplicates is false.

Arguments:

	Accessor - Supplies the resource accessor to pull resources from.

	CheckForDuplicates - Supplies a Boolean value that indicates whether the
	                     existing staged contents are checked for duplicate
	                     entries (which are then deleted) before addition.  The
	                     check for duplicates option is potentially very slow
	                     when working with large resource sets.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	ResRefIf                                          ResRef;
	ResType                                           ResType;
	swutil::SharedByteVec                             Contents;
	typename DirectoryFileReader< ResRefIf >::FileId  CurId;
	typename DirectoryFileReader< ResRefIf >::FileId  MaxId;

	MaxId = Accessor->GetEncapsulatedFileCount( );

	for (CurId = 0; CurId < MaxId; CurId += 1)
	{
		if (!Accessor->GetEncapsulatedFileEntry( CurId, ResRef, ResType ))
			throw std::runtime_error( "GetEncapsulatedFileEntry failed." );

		//
		// If we are to check for duplicate entries, try and remove any
		// already-existing entry.
		//

		if (CheckForDuplicates)
			RemoveFile( ResRef, ResType );

		//
		// Add the file contents.  Directory files are not double buffered, but
		// are added as filename references.
		//

		AddFile( ResRef, ResType, Accessor->GetRealFileName( CurId ) );
	}
}

template< typename ResRefT >
bool
ErfFileWriter< ResRefT >::RemoveFile(
	__in const ResRefIf & ResRef,
	__in ResType Type
	)
/*++

Routine Description:

	This routine removes a file from the pending file list of the ERF writer.
	If the file was not already pending, no action is taken.

	N.B.  It is assumed that there is only one match for the given file, i.e.
	      that the ERF being written is well-formed.

Arguments:

	ResRef - Supplies the canonical RESREF of the file to remove.

	Type - Supplies resource type of the file to remove.

Return Value:

	The routine returns a Boolean value indicating true if a file was actually
	removed from the staging list, else false if no action was taken.

Environment:

	User mode.

--*/
{
	for (ErfPendingFileVec::iterator it = m_PendingFiles.begin( );
	     it != m_PendingFiles.end( );
	     ++it)
	{
		if ((*it)->ResType != Type)
			continue;

		if (memcmp( &ResRef, &(*it)->ResRef, sizeof( ResRef ) ))
			continue;

		m_PendingFiles.erase( it );
		return true;
	}

	return false;
}

template< typename ResRefT >
void
ErfFileWriter< ResRefT >::RemoveAllFiles(
	)
/*++

Routine Description:

	This routine removes all staged files that are in the list of pending files
	in the ERF writer.

Arguments:

	None.

Return Value:

	None.  This routine does not fail.

Environment:

	User mode.

--*/
{
	m_PendingFiles.clear( );
}

template< typename ResRefT >
void
ErfFileWriter< ResRefT >::AddFile(
	__in const ResRefIf & ResRef,
	__in ResType Type,
	__in const std::string & SourceFileName
	)
/*++

Routine Description:

	This routine stages a disk file for future commit to an ERF.

	N.B.  The caller bears responsibility for ensuring that duplicate files are
	      not added.

Arguments:

	ResRef - Supplies the canonical RESREF of the file to add.

	Type - Supplies resource type of the file to add.

	SourceFileName - Supplies the disk file name of the file to add.  The file
	                 name reference must remain valid through any commit calls.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	ErfPendingFilePtr File;

	File = new ErfPendingFile( ResRef, Type, SourceFileName );

	m_PendingFiles.push_back( File );
}

template< typename ResRefT >
void
ErfFileWriter< ResRefT >::AddFile(
	__in const ResRefIf & ResRef,
	__in ResType Type,
	__in const swutil::SharedByteVec & Buffer
	)
/*++

Routine Description:

	This routine stages a shared byte buffer for future commit to an ERF.

	N.B.  The caller bears responsibility for ensuring that duplicate files are
	      not added.

Arguments:

	ResRef - Supplies the canonical RESREF of the file to add.

	Type - Supplies resource type of the file to add.

	Buffer - Supplies the file contents to add.  A reference on the buffer is
	         taken for the duration of the file's presence in the staging list.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	ErfPendingFilePtr File;

	File = new ErfPendingFile( ResRef, Type, Buffer );

	m_PendingFiles.push_back( File );
}

template< typename ResRefT >
void
ErfFileWriter< ResRefT >::AddFile(
	__in const ResRefIf & ResRef,
	__in ResType Type,
	__in_ecount( FileSize ) const void * FileContents,
	__in size_t FileSize
	)
/*++

Routine Description:

	This routine stages a memory buffer for future commit to an ERF.

	N.B.  The caller bears responsibility for ensuring that duplicate files are
	      not added.

Arguments:

	ResRef - Supplies the canonical RESREF of the file to add.

	Type - Supplies resource type of the file to add.

	FileContents - Supplies the contents of the file.  The file contents must
	               remain valid through any commit calls.

	FileSize - Supplies the length, in bytes, of the file contents to add.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	ErfPendingFilePtr File;

	File = new ErfPendingFile( ResRef, Type, FileContents, FileSize );

	m_PendingFiles.push_back( File );
}

template< typename ResRefT >
void
ErfFileWriter< ResRefT >::CommitInternal(
	__in ErfWriteContext * Context,
	__in unsigned long FileType,
	__in unsigned long Flags
	)
/*++

Routine Description:

	This routine writes the staged ERF contents to a write context, which may
	represent a disk file or an in-memory buffer.

Arguments:

	Context - Supplies the write context that receives the contents of the
	          formatted ERF file.

	FileType - Supplies the type tag of the file (ERF, MOD, etc.)

	Flags - Supplies flags that control the behavior of the commit operation.
	        Legal values are drawn from the ERF_COMMIT_FLAG_* family of values.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	ERF_HEADER        Header;

	UNREFERENCED_PARAMETER( Flags );

	//
	// If the user did not supply an override file type, take the default one.
	//

	if (FileType == 0)
		FileType = m_FileType;

	//
	// First, generate and store the header.
	//

	BuildHeader( Header, FileType, GetErfFileVersion< ResRefT >( ) );

	Context->Write( &Header, sizeof( Header ) );

	//
	// If talk strings were supported, we would write them out now.  However,
	// they are generally an unused ERF features and, as such, are not
	// implemented in this context.
	//

	Header.OffsetToLocalizedString = sizeof( Header );

	//
	// Write the key list out.
	//

	Header.OffsetToKeyList = sizeof( Header );

	WriteKeyList( Header, Context );

	//
	// Write the resource list out.
	//

	if (Header.OffsetToKeyList + Header.EntryCount * sizeof( ERF_KEY ) < Header.OffsetToKeyList)
		throw std::runtime_error( "ERF file is too large." );

	Header.OffsetToResourceList = Header.OffsetToKeyList + (Header.EntryCount * sizeof( ERF_KEY ) );

	WriteResourceList( Header, Context );

	//
	// Write the resource content list out.
	//

	WriteResourceContentList( Header, Context );

	//
	// Finally, re-write the updated header.
	//

	Context->SeekOffset( 0, "Write Finalized Header" );
	Context->Write( &Header, sizeof( Header ) );
}

template< typename ResRefT >
void
ErfFileWriter< ResRefT >::BuildHeader(
	__out ERF_HEADER & Header,
	__in unsigned long FileType,
	__in unsigned long FileVersion
	)
/*++

Routine Description:

	This routine builds the file header for an ERF commit operation.

Arguments:

	Header - Receives the constructed file header.

	FileType - Supplies the type tag of the file (ERF, MOD, etc.)

	FileVersion - Supplies the file version (V1.0, V1.1, etc).

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	time_t      t;
	struct tm * gmt;

	t = time( NULL );
	gmt = gmtime( &t );

	ZeroMemory( &Header, sizeof( Header ) );

	if (m_PendingFiles.size( ) > ULONG_MAX)
		throw std::runtime_error( "Maximum ERF resource count exceeded." );

	Header.FileType            = FileType;
	Header.Version             = FileVersion;

	//
	// Now prepare the data section of the header.  The header is updated as we
	// go and then re-written at the end.
	//

	Header.LanguageCount           = 0;
	Header.LocalizedStringSize     = 0;
	Header.EntryCount              = (unsigned long) m_PendingFiles.size( );
	Header.OffsetToLocalizedString = 0;
	Header.OffsetToKeyList         = 0;
	Header.OffsetToResourceList    = 0;
	Header.BuildYear               = (gmt != NULL) ? (unsigned long) gmt->tm_year : 0;
	Header.BuildDay                = (gmt != NULL) ? (unsigned long) gmt->tm_yday : 0;
	Header.DescriptionStrRef       = 0;

	ZeroMemory( Header.Reserved, sizeof( Header.Reserved ) );
}

template< typename ResRefT >
void
ErfFileWriter< ResRefT >::WriteKeyList(
	__inout ERF_HEADER & Header,
	__in ErfWriteContext * Context
	)
/*++

Routine Description:

	This routine writes the contents of each resource key out to the writer
	context.

Arguments:

	Header - Receives the constructed file header.  The header is updated as
	         the write operation progresses.

	Context - Supplies the write context that receives the contents of the
	          formatted ERF file.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	ResID ResourceId;

	UNREFERENCED_PARAMETER( Header );

	ResourceId = 0;

	for (ErfPendingFileVec::const_iterator it = m_PendingFiles.begin( );
	     it != m_PendingFiles.end( );
	     ++it)
	{
		ERF_KEY Key;

		C_ASSERT( sizeof( Key.FileName ) <= sizeof( (*it)->ResRef ) );

		memcpy( &Key.FileName, &(*it)->ResRef, sizeof( Key.FileName ) );

		Key.ResourceID = ResourceId;
		Key.Type       = (*it)->ResType;
		Key.Reserved   = 0;

		//
		// Transfer the key to the ERF.
		//

		Context->Write( &Key, sizeof( Key ) );

		ResourceId += 1;
	}
}

template< typename ResRefT >
void
ErfFileWriter< ResRefT >::WriteResourceList(
	__inout ERF_HEADER & Header,
	__in ErfWriteContext * Context
	)
/*++

Routine Description:

	This routine writes the contents of each resource list element out to the
	writer context.

Arguments:

	Header - Receives the constructed file header.  The header is updated as
	         the write operation progresses.

	Context - Supplies the write context that receives the contents of the
	          formatted ERF file.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	unsigned long OffsetToResource;

	UNREFERENCED_PARAMETER( Header );

	//
	// Calculate the end of the resource list.
	//

	if (Header.OffsetToResourceList + Header.EntryCount * sizeof( RESOURCE_LIST_ELEMENT ) < Header.OffsetToResourceList)
		throw std::runtime_error( "ERF resource list is too large." );

	OffsetToResource = Header.OffsetToResourceList + Header.EntryCount * sizeof( RESOURCE_LIST_ELEMENT );

	for (ErfPendingFileVec::const_iterator it = m_PendingFiles.begin( );
	     it != m_PendingFiles.end( );
	     ++it)
	{
		RESOURCE_LIST_ELEMENT ListElement;

		if ((*it)->Contents.GetFileSize( ) > ULONG_MAX)
			throw std::runtime_error( "Resource size exceeds maximum ERF resource size limit." );

		ListElement.OffsetToResource = OffsetToResource;
		ListElement.ResourceSize     = (ULONG) (*it)->Contents.GetFileSize( );

		//
		// Transfer the resource list element to the ERF.
		//

		Context->Write( &ListElement, sizeof( ListElement ) );

		if (ListElement.OffsetToResource + ListElement.ResourceSize < ListElement.OffsetToResource)
			throw std::runtime_error( "ERF file contents exceed maximum ERF file size limit." );

		OffsetToResource = ListElement.OffsetToResource + ListElement.ResourceSize;
	}
}

template< typename ResRefT >
void
ErfFileWriter< ResRefT >::WriteResourceContentList(
	__inout ERF_HEADER & Header,
	__in ErfWriteContext * Context
	)
/*++

Routine Description:

	This routine writes the contents of each resource out to the writer
	context.

Arguments:

	Header - Receives the constructed file header.  The header is updated as
	         the write operation progresses.

	Context - Supplies the write context that receives the contents of the
	          formatted ERF file.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	UNREFERENCED_PARAMETER( Header );

	enum { CHUNK_SIZE = 4096 };

	for (ErfPendingFileVec::const_iterator it = m_PendingFiles.begin( );
	     it != m_PendingFiles.end( );
	     ++it)
	{
		unsigned char Buffer[ CHUNK_SIZE ];
		unsigned long FileSize;
		unsigned long BytesLeft;
		unsigned long Offset;
		unsigned long Read;

		//
		// Transfer the resource file contents to the ERF.  It has already been
		// verified that the resource will fit in the ERF, and has a size that
		// fits within ULONG_MAX.
		//

		FileSize = (unsigned long) (*it)->Contents.GetFileSize( );

		if (FileSize == 0)
			continue;

		BytesLeft = FileSize;
		Offset    = 0;

		while (BytesLeft != 0)
		{
			Read = min( BytesLeft, CHUNK_SIZE );

			(*it)->Contents.ReadFile(
				Buffer,
				Read,
				"Read Pending File Contents");

			Offset    += Read;
			BytesLeft -= Read;

			Context->Write( Buffer, Read );
		}
	}
}

template ErfFileWriter< NWN::ResRef32 >;
template ErfFileWriter< NWN::ResRef16 >;
