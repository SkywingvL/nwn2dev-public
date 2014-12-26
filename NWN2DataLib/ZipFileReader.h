/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ZipFileReader.h

Abstract:

	This module defines the interface to the zip file reader, which allows
	resource load requests to be serviced against a .zip archive instead of an
	ERF file.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_ZIPFILEREADER_H
#define _PROGRAMS_NWN2DATALIB_ZIPFILEREADER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ResourceAccessor.h"

//
// Define the directory file reader object, used to access directory files.
//

template< typename ResRefT >
class ZipFileReader : public IResourceAccessor< ResRefT >
{

public:

	//
	// Constructor.  Raises an std::exception on catastrophic failure.
	//

	ZipFileReader(
		__in const std::string & ArchiveName
		);

	//
	// Destructor.
	//

	virtual
	~ZipFileReader(
		);

	//
	// Open an encapsulated file by resref.
	//

	virtual
	FileHandle
	OpenFile(
		__in const ResRefT & ResRef,
		__in ResType Type
		);

	//
	// Open an encapsulated file by file index.
	//

	virtual
	FileHandle
	OpenFileByIndex(
		__in FileId FileIndex
		);

	//
	// Close an encapsulated file.
	//

	virtual
	bool
	CloseFile(
		__in FileHandle File
		);

	//
	// Read an encapsulated file by file handle.  The routine is optimized to
	// operate for sequential file reads.
	//

	virtual
	bool
	ReadEncapsulatedFile(
		__in FileHandle File,
		__in size_t Offset,
		__in size_t BytesToRead,
		__out size_t * BytesRead,
		__out_bcount( BytesToRead ) void * Buffer
		);

	//
	// Return the size of a file.
	//

	virtual
	size_t
	GetEncapsulatedFileSize(
		__in FileHandle File
		);

	//
	// Return the resource type of a file.
	//

	virtual
	ResType
	GetEncapsulatedFileType(
		__in FileHandle File
		);

	//
	// Iterate through resources in this resource accessor.  The routine
	// returns false on failure.
	//

	virtual
	bool
	GetEncapsulatedFileEntry(
		__in FileId FileIndex,
		__out typename ResRefT & ResRef,
		__out ResType & Type
		);

	//
	// Return the count of encapsulated files in this accessor.
	//

	virtual
	FileId
	GetEncapsulatedFileCount(
		);

	//
	// Get the logical name of this accessor.
	//

	virtual
	AccessorType
	GetResourceAccessorName(
		__in FileHandle File,
		__out std::string & AccessorName
		);

private:

	typedef void * ZipArchive;

	struct DirectoryEntry
	{
		FileHandle  FileHandleToReturn;
		ResRefT     Name;
		ResType     Type;
	};

	typedef std::vector< DirectoryEntry > DirectoryEntryVec;

	//
	// Open a new zip archive.
	//

	ZipArchive
	OpenArchive(
		__in const std::string & ArchiveName
		);

	//
	// Close an opened archive.
	//

	void
	CloseArchive(
		__in ZipArchive Archive
		);

	//
	// Scan directories to create directory file entries.
	//

	void
	ScanArchive(
		__in ZipArchive Archive
		);

	//
	// Locate a file in the zip archive.
	//

	const DirectoryEntry *
	LocateFileByName(
		__in const ResRefT & ResRef,
		__in ResType Type
		);

	DirectoryEntryVec m_DirectoryEntries;
	ZipArchive        m_Archive;
	bool              m_FileOpened;     // Is m_OpenFileHandle valid?
	FileHandle        m_OpenFileHandle; // Currently open file handle
	unsigned __int64  m_CurrentOffset;  // Offset into currently opened file
	std::string       m_FileName;

};

typedef ZipFileReader< NWN::ResRef32 > ZipFileReader32;
typedef ZipFileReader< NWN::ResRef16 > ZipFileReader16;


#endif

