/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ErfFileWriter.h

Abstract:

	This module defines the interface to the Encapsulated Resource File (ERF)
	writer.  ERFs aggregate several independent resource files into a single
	disk file.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_ERFFILEWRITER_H
#define _PROGRAMS_NWN2DATALIB_ERFFILEWRITER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ResourceAccessor.h"
#include "FileWrapper.h"
#include "ErfFileReader.h"

//
// Define the ERF file reader object, used to access ERF files.
//

template< typename ResRefT >
class ErfFileWriter
{

public:

	typedef unsigned long  ResID;
	typedef typename IResourceAccessor< ResRefT >::ResType ResType;

	//
	// Define the type of resref used in the public interface, regardless of
	// the internal on-disk representation.
	//

	typedef NWN::ResRef32 ResRefIf;

	//
	// Constructor.  Raises an std::exception on failure.
	//

	ErfFileWriter(
		);

	//
	// Destructor.
	//

	virtual
	~ErfFileWriter(
		);

	//
	// Define common ERF-derived file types.
	//

	enum
	{
		ERF_FILE_TYPE = ' FRE',
		HAK_FILE_TYPE = ' KAH',
		MOD_FILE_TYPE = ' DOM',
		NWM_FILE_TYPE = ' MWN',

		LAST_FILE_TYPE
	};

	//
	// Define ERF writer flags that control the behavior of the write operation.
	//

	enum
	{
		LAST_ERF_COMMIT_FLAG
	};

	//
	// Commit the contents of the ERF to disk.
	//

	bool
	Commit(
		__in const std::string & FileName,
		__in unsigned long FileType = 0,
		__in unsigned long Flags = 0
		);

	//
	// Commit the contents of the ERF to memory.
	//

	bool
	Commit(
		__out std::vector< unsigned char > & Memory,
		__in unsigned long FileType = 0,
		__in unsigned long Flags = 0
		);

	//
	// Set the default file type (substitued if the override commit file type
	// is zero).
	//

	inline
	void
	SetDefaultFileType(
		__in unsigned long FileType
		)
	{
		m_FileType = FileType;
	}

	inline
	unsigned long
	GetDefaultFileType(
		) const
	{
		return m_FileType;
	}

	//
	// Initialize a writer's contents from a resource accessor.
	//

	void
	InitializeFromResourceAccessor(
		__in IResourceAccessor< ResRefIf > * Accessor,
		__in bool CheckForDuplicates = false
		);

	//
	// Initialize a writer's contents from a resource accessor.  Directory file
	// reader files are not double buffered, but are added as filename
	// references.
	//

	void
	InitializeFromResourceAccessor(
		__in DirectoryFileReader< ResRefIf > * Accessor,
		__in bool CheckForDuplicates = false
		);

	//
	// Remove a pending file from the ERF.  If the file was not already pending
	// then the operation completes successfully.
	//

	bool
	RemoveFile(
		__in const ResRefIf & ResRef,
		__in ResType Type
		);

	//
	// Remove all pending files from the ERF.
	//

	void
	RemoveAllFiles(
		);

	//
	// Add a file to the ERF by filename, which must remain valid until the
	// file is removed or the ErfFileWriter object is deleted.  The caller
	// bears responsibility for ensuring the uniqueness of the file in the ERF.
	//

	void
	AddFile(
		__in const ResRefIf & ResRef,
		__in ResType Type,
		__in const std::string & SourceFileName
		);

	//
	// Add a file to the ERF by shared byte buffer, for which a reference is
	// maintained until the file is removed or the ErfFileWriter object is
	// deleted.  The caller bears responsibility for ensuring the uniqueness of
	// the file in the ERF.
	//

	void
	AddFile(
		__in const ResRefIf & ResRef,
		__in ResType Type,
		__in const swutil::SharedByteVec & Buffer
		);

	//
	// Add a file to the ERF by in-memory buffer, which must remain valid until
	// the file is removed or the ErfFileWriter object is deleted.  The caller
	// bears responsibility for ensuring the uniqueness of the file in the ERF.
	//

	void
	AddFile(
		__in const ResRefIf & ResRef,
		__in ResType Type,
		__in_ecount( FileSize ) const void * FileContents,
		__in size_t FileSize
		);

private:

	//
	// Define the ERF writer context, which supports data append only, to a
	// disk or memory target.
	//

	struct ErfWriteContext
	{
		enum ContextType
		{
			ContextTypeFile,
			ContextTypeMemory,

			LastContextType
		};

		inline
		ErfWriteContext(
			)
		: Type( LastContextType ),
		  WritePtr( 0 )
		{

		}

		ContextType Type;

		union
		{
			HANDLE                         File;
			std::vector< unsigned char > * Memory;
		};

		size_t      WritePtr;

		//
		// Append contents to the write context's target.  The routine raises
		// an std::exception on failure.
		//

		inline
		void
		Write(
			__in_bcount( Length ) const void * Data,
			__in size_t Length
			)
		{
			if (Length == 0)
				return;

			switch (Type)
			{

			case ContextTypeFile:
				{
					DWORD Written;

					if (!WriteFile(
						File,
						Data,
						(DWORD) Length,
						&Written,
						NULL))
					{
						throw std::runtime_error( "ErfWriteContext::Write failed to write to file." );
					}

					if ((size_t) Written != Length)
						throw std::runtime_error( "ErfWriteContext::Write wrote less than the required count of bytes." );
				}
				break;

			case ContextTypeMemory:
				{
					size_t Tail;

					Tail = Memory->size( );

					if (WritePtr + Length < WritePtr)
						throw std::runtime_error( "ErfWriteContext::Write encountered write pointer overflow." );

					if (WritePtr + Length > Tail)
						Memory->resize( WritePtr + Length );

					memcpy( &(*Memory)[ WritePtr ], Data, Length );

					WritePtr += Length;
				}
				break;
			}
		}

		//
		// (Re-)read data from the write context's target.  The data must have
		// been already written with Write.  The routine raises an std::exception
		// on failure.
		//

		inline
		void
		Read(
			__out_bcount( Length ) void * Data,
			__in size_t Length
			)
		{
			if (Length == 0)
				return;

			switch (Type)
			{

			case ContextTypeFile:
				{
					DWORD Read;

					if (!ReadFile(
						File,
						Data,
						(DWORD) Length,
						&Read,
						NULL))
					{
						throw std::runtime_error( "ErfWriteContext::Read failed to read from file." );
					}

					if ((size_t) Read != Length)
						throw std::runtime_error( "ErfWriteContext::Read read less than the required count of bytes." );
				}
				break;

			case ContextTypeMemory:
				{
					size_t Tail;

					Tail = Memory->size( );

					if (WritePtr + Length < WritePtr)
						throw std::runtime_error( "ErfWriteContext::Read encountered read pointer overflow." );

					if (WritePtr + Length > Tail)
						throw std::runtime_error( "ErfWriteContext::Read read past the end of stream." );

					memcpy( Data, &(*Memory)[ WritePtr ], Length );

					WritePtr += Length;
				}
				break;
			}
		}

		//
		// Seek to a particular position.
		//

		inline
		void
		SeekOffset(
			__in ULONGLONG Offset,
			__in const char * Description
			)
		{
			LONG  Low;
			LONG  High;
			DWORD NewPtrLow;
			char  ExMsg[ 64 ];

			switch (Type)
			{

			case ContextTypeMemory:
				WritePtr = (size_t) Offset;
				break;

			case ContextTypeFile:
				Low  = (LONG) ((Offset >>  0) & 0xFFFFFFFF);
				High = (LONG) ((Offset >> 32) & 0xFFFFFFFF);

				NewPtrLow = SetFilePointer( File, Low, &High, FILE_BEGIN );

				if ((NewPtrLow == INVALID_SET_FILE_POINTER) &&
					(GetLastError( ) != NO_ERROR))
				{
					StringCbPrintfA(
						ExMsg,
						sizeof( ExMsg ),
						"SeekOffset( %s ) failed.",
						Description);

					throw std::runtime_error( ExMsg );
				}
				break;
			}
		}
	};

	//
	// Commit the ERF to a write context.
	//

	void
	CommitInternal(
		__in ErfWriteContext * Context,
		__in unsigned long FileType,
		__in unsigned long Flags
		);

	//
	// Define the ERF on-disk file structures.  This data is based on the
	// BioWare Aurora engine documentation.
	//
	// http://nwn.bioware.com/developers/Bioware_Aurora_ERF_Format.pdf
	//

	typedef typename ErfFileReader< ResRefT >::ERF_HEADER ERF_HEADER;
	typedef typename ErfFileReader< ResRefT >::ERF_KEY ERF_KEY;
	typedef typename ErfFileReader< ResRefT >::RESOURCE_LIST_ELEMENT RESOURCE_LIST_ELEMENT;

	typedef std::vector< ERF_KEY > ErfKeyVec;
	typedef std::vector< RESOURCE_LIST_ELEMENT > ErfResVec;

	//
	// Build the ERF header.
	//

	void
	BuildHeader(
		__out ERF_HEADER & Header,
		__in unsigned long FileType,
		__in unsigned long FileVersion
		);

	//
	// Write the resource key list out.
	//

	void
	WriteKeyList(
		__inout ERF_HEADER & Header,
		__in ErfWriteContext * Context
		);

	//
	// Write the resource list out.
	//

	void
	WriteResourceList(
		__inout ERF_HEADER & Header,
		__in ErfWriteContext * Context
		);

	//
	// Write the resource content list out.
	//

	void
	WriteResourceContentList(
		__inout ERF_HEADER & Header,
		__in ErfWriteContext * Context
		);

	//
	// Retrieve the file version that should be defaulted for the new ERF file.
	//

	template< typename T >
	inline
	static
	ULONG
	GetErfFileVersion(
		)
	{
		return 0;
	}

	template< >
	inline
	static
	ULONG
	GetErfFileVersion< NWN::ResRef32 >(
		)
	{
		return '1.1V';
	}

	template< >
	inline
	static
	ULONG
	GetErfFileVersion< NWN::ResRef16 >(
		)
	{
		return '0.1V';
	}

	//
	// Define a pending file that is awaiting addition.
	//

	struct ErfPendingFile
	{
		ResRefIf                        ResRef;
		typename ErfFileWriter::ResType ResType;
		FileWrapper                     Contents;
		HANDLE                          FileHandle;
		swutil::SharedByteVec           Buffer;

		inline
		ErfPendingFile(
			__in const ResRefIf & ResRef,
			__in typename ErfFileWriter::ResType ResType,
			__in const std::string & FileName
			)
		{
			this->ResRef  = ResRef;
			this->ResType = ResType;

			FileHandle = CreateFileA(
				FileName.c_str( ),
				GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_DELETE,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

			if (FileHandle == INVALID_HANDLE_VALUE)
			{
				std::string Msg;

				Msg  = "Failed to open file \"";
				Msg += FileName;
				Msg += "\".";

				throw std::runtime_error( Msg );
			}

#if defined(_WIN64)
			Contents.SetFileHandle( FileHandle, true );
#else
			Contents.SetFileHandle( FileHandle, false );
#endif
		}

		inline
		ErfPendingFile(
			__in const ResRefIf & ResRef,
			__in typename ErfFileWriter::ResType ResType,
			__in_ecount( FileSize ) const void * FileContents,
			__in size_t FileSize
			)
		{
			this->ResRef  = ResRef;
			this->ResType = ResType;

			FileHandle = INVALID_HANDLE_VALUE;

			Contents.SetExternalView(
				(const unsigned char *) FileContents,
				FileSize );
		}

		inline
		ErfPendingFile(
			__in const ResRefIf & ResRef,
			__in typename ErfFileWriter::ResType ResType,
			__in const swutil::SharedByteVec & Buffer
			)
		{
			this->ResRef  = ResRef;
			this->ResType = ResType;

			FileHandle   = INVALID_HANDLE_VALUE;
			this->Buffer = Buffer;

			if (!Buffer->empty( ))
				Contents.SetExternalView( &Buffer->front( ), Buffer->size( ) );
		}

		inline
		~ErfPendingFile(
			)
		{
			if (FileHandle != INVALID_HANDLE_VALUE)
			{
				CloseHandle( FileHandle );
				FileHandle = INVALID_HANDLE_VALUE;
			}
		}
	};

	typedef swutil::SharedPtr< ErfPendingFile > ErfPendingFilePtr;
	typedef std::vector< ErfPendingFilePtr > ErfPendingFileVec;

	//
	// Define the default file type if none is specified for a commit request.
	//

	unsigned long     m_FileType;

	//
	// Define the list of pending files to add to the ERF on the next commit
	// request.
	//

	ErfPendingFileVec m_PendingFiles;

};

typedef ErfFileWriter< NWN::ResRef32 > ErfFileWriter32;
typedef ErfFileWriter< NWN::ResRef16 > ErfFileWriter16;


#endif
