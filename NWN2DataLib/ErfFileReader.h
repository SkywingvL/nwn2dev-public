/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ErfFileReader.h

Abstract:

	This module defines the interface to the Encapsulated Resource File (ERF)
	reader.  ERFs aggregate several independent resource files into a single
	disk file.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_ERFFILEREADER_H
#define _PROGRAMS_NWN2DATALIB_ERFFILEREADER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ResourceAccessor.h"
#include "FileWrapper.h"

template< typename ResRefT >
class ErfFileWriter;

//
// Define the ERF file reader object, used to access ERF files.
//

template< typename ResRefT >
class ErfFileReader : public IResourceAccessor< NWN::ResRef32 >
{

public:

	typedef unsigned long  ResID;

	//
	// Define the type of resref used in the public interface, regardless of
	// the internal on-disk representation.
	//

	typedef NWN::ResRef32 ResRefIf;

	//
	// Constructor.  Raises an std::exception on parse failure.
	//

	ErfFileReader(
		__in const std::string & FileName
		);

	//
	// Destructor.
	//

	virtual
	~ErfFileReader(
		);

	//
	// Open an encapsulated file by resref.
	//

	virtual
	FileHandle
	OpenFile(
		__in const ResRefIf & ResRef,
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
		__out typename ResRefIf & ResRef,
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

	//
	// Parse the on-disk format and read the base directory data in.
	//

	void
	ParseErfFile(
		);

	//
	// Define the ERF on-disk file structures.  This data is based on the
	// BioWare Aurora engine documentation.
	//
	// http://nwn.bioware.com/developers/Bioware_Aurora_ERF_Format.pdf
	//

#include <pshpack1.h>

	typedef struct _ERF_HEADER
	{
		unsigned long FileType;                // "ERF ", "MOD ", etc
		unsigned long Version;                 // "V1.0"
		unsigned long LanguageCount;           // # of strings in string table
		unsigned long LocalizedStringSize;     // # of bytes in string table
		unsigned long EntryCount;              // # of files in ERF
		unsigned long OffsetToLocalizedString; // from beginning of file
		unsigned long OffsetToKeyList;         // from beginning of file
		unsigned long OffsetToResourceList;    // from beginning of file
		unsigned long BuildYear;               // Since 1900
		unsigned long BuildDay;                // Since January 1
		unsigned long DescriptionStrRef;       // Strref for file description
		unsigned char Reserved[ 116 ];         // Reserved for future use [MBZ]
	} ERF_HEADER, * PERF_HEADER;

	typedef const struct _ERF_HEADER * PCERF_HEADER;

	typedef struct _ERF_KEY
	{
		ResRefT        FileName;
		ResID          ResourceID;
		ResType        Type;
		unsigned short Reserved;
	} ERF_KEY, * PERF_KEY;

	typedef const struct _ERF_KEY * PCERF_KEY;

	typedef struct _RESOURCE_LIST_ELEMENT
	{
		unsigned long OffsetToResource;
		unsigned long ResourceSize;
	} RESOURCE_LIST_ELEMENT, * PRESOURCE_LIST_ELEMENT;

	typedef const struct _RESOURCE_LIST_ELEMENT * PCRESOURCE_LIST_ELEMENT;

#include <poppack.h>

	typedef std::vector< ERF_KEY > ErfKeyVec;
	typedef std::vector< RESOURCE_LIST_ELEMENT > ErfResVec;

	//
	// Define helper routines for looking up resource data.
	//

	//
	// Locate a resource by its resref name.
	//

	inline
	PCERF_KEY
	LookupResourceKey(
		__in const ResRefIf & Name,
		__in ResType Type
		) const
	{
		C_ASSERT( sizeof( ResRefT ) <= sizeof( ResRefIf ) );

		for (ErfKeyVec::const_iterator it = m_KeyDir.begin( );
		     it != m_KeyDir.end( );
		     ++it)
		{
			if (it->Type != Type)
				continue;

			if (!memcmp( &Name, &it->FileName, sizeof( ResRefT ) ))
				return &*it;
		}

		return NULL;
	}

	//
	// Locate a resource by its resource id.
	//

	inline
	PCERF_KEY
	LookupResourceKey(
		__in ResID ResourceId
		) const
	{
		if (ResourceId >= m_ResDir.size( ))
			return NULL;

		return &m_KeyDir[ ResourceId ];
	}

	//
	// Locate a resource directory entry by resource id.
	//

	inline
	PCRESOURCE_LIST_ELEMENT
	LookupResourceDirectory(
		__in ResID ResourceId
		) const
	{
		if (ResourceId >= m_ResDir.size( ))
			return NULL;

		return &m_ResDir[ ResourceId ];
	}

	//
	// Define file book-keeping data.
	//

	HANDLE             m_File;
	unsigned long      m_FileSize;
	FileWrapper        m_FileWrapper;
	ULONGLONG          m_NextOffset;
	std::string        m_FileName;

	//
	// Resource list data.
	//

	ErfKeyVec          m_KeyDir;
	ErfResVec          m_ResDir;

	friend class ErfFileWriter< ResRefT >;

};

typedef ErfFileReader< NWN::ResRef32 > ErfFileReader32;
typedef ErfFileReader< NWN::ResRef16 > ErfFileReader16;


#endif
