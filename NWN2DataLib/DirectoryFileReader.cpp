/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	DirectoryFileReader.cpp

Abstract:

	This module houses the DirectoryFileReader object implementation.  The
	DirectoryFileReader allows resources to be demand-loaded from directories
	as opposed to ERF files.

--*/

#include "Precomp.h"
#include "DirectoryFileReader.h"
#include "FileWrapper.h"

template< typename ResRefT >
DirectoryFileReader< ResRefT >::DirectoryFileReader(
	__in const std::string & DirectoryName
	)
/*++

Routine Description:

	This routine constructs a new DirectoryFileReader object and stores the
	filename of the directory away.

Arguments:

	DirectoryName - Supplies the directory file path.  The path should be a
	                fully-qualified path.

Return Value:

	The newly constructed object.

Environment:

	User mode.

--*/
: m_DirectoryName( DirectoryName )
{
	m_DirectoryName += "//";

	//
	// Create directory file entries as necessary.
	//

	ScanDirectory( m_DirectoryName, 0 );
}

template< typename ResRefT >
DirectoryFileReader< ResRefT >::~DirectoryFileReader(
	)
/*++

Routine Description:

	This routine cleans up an already-existing DirectoryFileReader object.

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
typename DirectoryFileReader< ResRefT >::FileHandle
DirectoryFileReader< ResRefT >::OpenFile(
	__in const ResRefT & FileName,
	__in ResType Type
	)
/*++

Routine Description:

	This routine logically opens a file within the directory.

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
	std::string    FileNameStr( m_DirectoryName );
	const char   * p;
	HANDLE         h;

	p = (const char *)memchr(
		FileName.RefStr,
		'\0',
		sizeof( FileName.RefStr ) );

	if (p == NULL)
		FileNameStr.append( FileName.RefStr, sizeof( FileName.RefStr ) );
	else
		FileNameStr.append( FileName.RefStr, p - FileName.RefStr );

	FileNameStr += ".";
	FileNameStr += ResTypeToExt( Type );

	h = CreateFileA(
		FileNameStr.c_str( ),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (h == INVALID_HANDLE_VALUE)
		return INVALID_FILE;

	return (FileHandle) h;
}

template< typename ResRefT >
typename DirectoryFileReader< ResRefT >::FileHandle
DirectoryFileReader< ResRefT >::OpenFileByIndex(
	__in typename FileId FileIndex
	)
/*++

Routine Description:

	This routine logically opens a file within the directory.

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
	HANDLE h;

	if ((size_t) FileIndex >= m_DirectoryEntries.size( ))
		return INVALID_FILE;

	h = CreateFileA(
		m_DirectoryEntries[ (size_t) FileIndex ].RealFileName.c_str( ),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (h == INVALID_HANDLE_VALUE)
		return INVALID_FILE;

	return (FileHandle) h;
}

template< typename ResRefT >
bool
DirectoryFileReader< ResRefT >::CloseFile(
	__in typename DirectoryFileReader< ResRefT >::FileHandle File
	)
/*++

Routine Description:

	This routine logically closes an encapsulated sub-file within the ERF file.

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

	return CloseHandle( (HANDLE) File ) ? true : false;
}

template< typename ResRefT >
bool
DirectoryFileReader< ResRefT >::ReadEncapsulatedFile(
	__in FileHandle File,
	__in size_t Offset,
	__in size_t BytesToRead,
	__out size_t * BytesRead,
	__out_bcount( BytesToRead ) void * Buffer
	)
/*++

Routine Description:

	This routine logically reads an encapsulated sub-file within the directory.

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
	DWORD Read;

	UNREFERENCED_PARAMETER( Offset );

	if (BytesToRead > ULONG_MAX)
		BytesToRead = ULONG_MAX;

	if (!ReadFile( (HANDLE) File, Buffer, (DWORD) BytesToRead, &Read, NULL ))
		return false;

	*BytesRead = (size_t) Read;

	return true;
}

template< typename ResRefT >
size_t
DirectoryFileReader< ResRefT >::GetEncapsulatedFileSize(
	__in typename DirectoryFileReader< ResRefT >::FileHandle File
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
	FileWrapper Wrap( (HANDLE) File );

	return (size_t) Wrap.GetFileSize( );
}

template< typename ResRefT >
typename DirectoryFileReader< ResRefT >::ResType
DirectoryFileReader< ResRefT >::GetEncapsulatedFileType(
	__in typename DirectoryFileReader< ResRefT >::FileHandle File
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
	File;

	return NWN::ResINVALID; // Not supported.
}

template< typename ResRefT >
bool
DirectoryFileReader< ResRefT >::GetEncapsulatedFileEntry(
	__in typename FileId FileIndex,
	__out typename ResRefT & ResRef,
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
	if ((size_t) FileIndex >= m_DirectoryEntries.size( ))
		return false;

	memcpy(
		&ResRef,
		&m_DirectoryEntries[ (size_t) FileIndex ].Name,
		sizeof( ResRef ) );

	Type = m_DirectoryEntries[ (size_t) FileIndex ].Type;

	return true;
}

template< typename ResRefT >
typename DirectoryFileReader< ResRefT >::FileId
DirectoryFileReader< ResRefT >::GetEncapsulatedFileCount(
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
	return m_DirectoryEntries.size( );
}

template< typename ResRefT >
typename DirectoryFileReader< ResRefT >::AccessorType
DirectoryFileReader< ResRefT >::GetResourceAccessorName(
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

	AccessorName = m_DirectoryName;
	return AccessorTypeDirectory;
}

template< typename ResRefT >
void
DirectoryFileReader< ResRefT >::ScanDirectory(
	__in const std::string & Directory,
	__in size_t RecursionLevel
	)
/*++

Routine Description:

	This routine recursively scans directories for files to add to the
	directory entry list.

Arguments:

	Directory - Supplies the name of the directory to scan.  The name must have
	            a path separator at the end.

	RecursionLevel - Supplies the recursion level.

Return Value:

	The routine returns the count of files present.

Environment:

	User mode.

--*/
{
	WIN32_FIND_DATAA FindData;
	HANDLE           Find;
	std::string      Name;
	ResType          Type;
	char             Ext[ 32 ];
	char             ResName[ MAX_PATH ];
	size_t           Len;

	if (RecursionLevel >= 256)
		return;

	Name = Directory;
	Name += "*.*";

	Find = FindFirstFileA( Name.c_str( ), &FindData );

	if (Find == INVALID_HANDLE_VALUE)
		return;

	try
	{
		do
		{
			if ((!strcmp( FindData.cFileName, "." )) ||
			    (!strcmp( FindData.cFileName, ".." )))
				continue;

			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				Name  = Directory;
				Name += FindData.cFileName;
				Name += "//";

				ScanDirectory( Name, RecursionLevel + 1 );
				continue;
			}

			//
			// Check if this is a known resource type, and add it to our list
			// if it is.
			//

			if (_splitpath_s(
				FindData.cFileName,
				NULL,
				0,
				NULL,
				0,
				ResName,
				sizeof( ResName ),
				Ext,
				sizeof( Ext )) || (Ext[ 0 ] == '\0'))
			{
				continue;
			}

			Type = ExtToResType( Ext + 1 );

			//
			// Ignore unrecognized types.
			//

			if (Type == NWN::ResINVALID)
				continue;

			//
			// Create and append a directory entry for it.
			//

			DirectoryEntry Entry;

			Entry.RealFileName  = Directory;
			Entry.RealFileName += FindData.cFileName;
			Entry.Type          = Type;

			Len = strlen( ResName );

			_strlwr( ResName );

			ZeroMemory( &Entry.Name, sizeof( Entry.Name ) );

			memcpy( &Entry.Name, ResName, min( Len, sizeof( Entry.Name ) ) );

			m_DirectoryEntries.push_back( Entry );

		} while (FindNextFileA( Find, &FindData ) );
	}
	catch (...)
	{
		FindClose( Find );
		throw;
	}

	FindClose( Find );
}

template DirectoryFileReader< NWN::ResRef32 >;
