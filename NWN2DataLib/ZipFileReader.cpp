/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ZipFileReader.cpp

Abstract:

	This module houses the ZipFileReader object implementation.  The
	ZipFileReader allows resources to be demand-loaded from .zip archives as
	opposed to ERF files.

--*/

#include "Precomp.h"
#include "ZipFileReader.h"
#include "FileWrapper.h"

#define STRICTZIPUNZIP
#include "../zlib/zlib.h"
#define _ZLIB_H // unzip.h mistakenly assumes _ZLIB_H instead of ZLIB_H
#include "../zlib/contrib/minizip/unzip.h"

static void x_fill_fopen_filefunc (zlib_filefunc_def* pzlib_filefunc_def);

template< typename ResRefT >
ZipFileReader< ResRefT >::ZipFileReader(
	__in const std::string & ArchiveName
	)
/*++

Routine Description:

	This routine constructs a new ZipFileReader object and opens the .zip
	archive for reading.

Arguments:

	ArchiveName - Supplies the name of the .zip archive to access.

Return Value:

	The newly constructed object.

Environment:

	User mode.

--*/
: m_Archive( NULL ),
  m_FileOpened( false ),
  m_OpenFileHandle( INVALID_FILE ),
  m_CurrentOffset( 0 ),
  m_FileName( ArchiveName )
{
	//
	// Create directory file entries as necessary.
	//

	m_Archive = OpenArchive( ArchiveName );

	if (m_Archive == NULL)
	{
		try
		{
			std::string ErrorStr;

			ErrorStr  = "Failed to open .zip archive '";
			ErrorStr += ArchiveName;
			ErrorStr += "'.";

			throw std::runtime_error( ErrorStr );
		}
		catch (std::bad_alloc)
		{
			throw std::runtime_error( "Failed to open .zip archive." );
		}
	}

	ScanArchive( m_Archive );
}

template< typename ResRefT >
ZipFileReader< ResRefT >::~ZipFileReader(
	)
/*++

Routine Description:

	This routine cleans up an already-existing ZipFileReader object.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	if (m_Archive != NULL)
	{
		CloseArchive( m_Archive );

		m_Archive = NULL;
	}
}

template< typename ResRefT >
typename ZipFileReader< ResRefT >::FileHandle
ZipFileReader< ResRefT >::OpenFile(
	__in const ResRefT & FileName,
	__in ResType Type
	)
/*++

Routine Description:

	This routine logically opens a file within the archive.

	File handles are simply file identifiers for purposes of accessing the .zip
	archive.

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
	const DirectoryEntry * Entry;

	if ((Entry = LocateFileByName( FileName, Type )) == NULL)
		return INVALID_FILE;

	//
	// Just pass the request on to OpenFileByIndex, so that it may handle the
	// request in a uniform fashion.
	// 

	return OpenFileByIndex( Entry - &m_DirectoryEntries[ 0 ] );
}

template< typename ResRefT >
typename ZipFileReader< ResRefT >::FileHandle
ZipFileReader< ResRefT >::OpenFileByIndex(
	__in typename FileId FileIndex
	)
/*++

Routine Description:

	This routine logically opens a file within the archive.

	File handles are simply file identifiers for purposes of accessing the .zip
	archive.

	Currently, the .zip file reader supports only one concurrent open file.

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
	FileHandle   Handle;
	int          Error;
	unz_file_pos Pos;

	if (m_FileOpened)
		return INVALID_FILE;

	if ((size_t) FileIndex >= m_DirectoryEntries.size( ))
		return INVALID_FILE;

	//
	// Seek to the start of the file and open it.
	//

	Handle = m_DirectoryEntries[ (size_t) FileIndex ].FileHandleToReturn;

	Pos.pos_in_zip_directory = (uLong) (((Handle - 1) >>  0) & 0xFFFFFFFF);
	Pos.num_of_file          = (uLong) (((Handle - 1) >> 32) & 0xFFFFFFFF);

	Error = unzGoToFilePos( (unzFile) m_Archive, &Pos );

	if (Error)
		return INVALID_FILE;

	Error = unzOpenCurrentFile( (unzFile) m_Archive );

	if (Error)
		return INVALID_FILE;

	m_FileOpened     = true;
	m_OpenFileHandle = Handle;
	m_CurrentOffset  = 0;

	return Handle;
}

template< typename ResRefT >
bool
ZipFileReader< ResRefT >::CloseFile(
	__in typename ZipFileReader< ResRefT >::FileHandle File
	)
/*++

Routine Description:

	This routine logically closes an encapsulated sub-file within the .zip
	archive.

	Currently, file handles are implemented as simply file offsets within the
	.zip archive.

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
	//
	// Ensure that the file handle is legal before closing the current file.
	//

	if (File == INVALID_FILE)
		return false;

	if (!m_FileOpened)
		return false;

	if (File != m_OpenFileHandle)
		return false;

	unzCloseCurrentFile( (unzFile) m_Archive );

	m_FileOpened = false;

	return true;
}

template< typename ResRefT >
bool
ZipFileReader< ResRefT >::ReadEncapsulatedFile(
	__in FileHandle File,
	__in size_t Offset,
	__in size_t BytesToRead,
	__out size_t * BytesRead,
	__out_bcount( BytesToRead ) void * Buffer
	)
/*++

Routine Description:

	This routine logically reads an encapsulated sub-file within the .zip file.

	Presently, only sequential scan is supported for .zip files.

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
	int Read;

	if (BytesToRead > ULONG_MAX)
		BytesToRead = ULONG_MAX;

	if (File == INVALID_FILE)
		return false;

	if (!m_FileOpened)
		return false;

	if (File != m_OpenFileHandle)
		return false;

	if (m_CurrentOffset != Offset)
		return false;

	Read = unzReadCurrentFile(
		(unzFile) m_Archive,
		Buffer,
		(unsigned) BytesToRead);

	if (Read <= 0)
		return false;

	*BytesRead = (size_t) Read;

	m_CurrentOffset += *BytesRead;

	return true;
}

template< typename ResRefT >
size_t
ZipFileReader< ResRefT >::GetEncapsulatedFileSize(
	__in typename ZipFileReader< ResRefT >::FileHandle File
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
	unz_file_info FileInfo;
	int           Error;

	if (File == INVALID_FILE)
		return false;

	if (!m_FileOpened)
		return false;

	if (File != m_OpenFileHandle)
		return false;

	Error = unzGetCurrentFileInfo(
		(unzFile) m_Archive,
		&FileInfo,
		NULL,
		0,
		NULL,
		0,
		NULL,
		0);

	if (Error)
		return false;

	return FileInfo.uncompressed_size;
}

template< typename ResRefT >
typename ZipFileReader< ResRefT >::AccessorType
ZipFileReader< ResRefT >::GetResourceAccessorName(
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
	return AccessorTypeZip;
}

template< typename ResRefT >
typename ZipFileReader< ResRefT >::ResType
ZipFileReader< ResRefT >::GetEncapsulatedFileType(
	__in typename ZipFileReader< ResRefT >::FileHandle File
	)
/*++

Routine Description:

	This routine returns the type of an encapsulated file.

Arguments:

	File - Supplies the file handle to query the size of.

Return Value:

	The routine returns the type of the given file.  If a valid file handle is
	supplied, then the routine never fails given a recognized resource type.

	Should an illegal file handle be supplied, the routine returns ResINVALID.
	There is no way to distinguish this condition from legal file handle to a
	file of type ResINVALID.  Only a serious programming error results in a
	caller supplying an illegal file handle.

Environment:

	User mode.

--*/
{
	char   FileName[ 260 ];
	char   Name[ MAX_PATH ];
	char   Ext[ 32 ];
	int    Error;

	if (File == INVALID_FILE)
		return NWN::ResINVALID;

	if (!m_FileOpened)
		return NWN::ResINVALID;

	if (File != m_OpenFileHandle)
		return NWN::ResINVALID;

	strcpy_s( FileName, "Z:\\" ); // Bogus, for splitpath.

	Error = unzGetCurrentFileInfo(
		(unzFile) m_Archive,
		NULL,
		FileName + 3,
		sizeof( FileName ) - 3,
		NULL,
		0,
		NULL,
		0);

	if (Error)
		return NWN::ResINVALID;

	FileName[ sizeof( FileName ) - sizeof( FileName[ 0 ] ) ] = '\0';

	_strlwr( FileName );

	if (_splitpath_s(
		FileName,
		NULL,
		0,
		NULL,
		0,
		Name,
		sizeof( Name ),
		Ext,
		sizeof( Ext )) || (Ext[ 0 ] == '\0'))
	{
		return NWN::ResINVALID;
	}

	return ExtToResType( Ext + 1 );
}

template< typename ResRefT >
bool
ZipFileReader< ResRefT >::GetEncapsulatedFileEntry(
	__in typename FileId FileIndex,
	__out typename ResRefT & ResRef,
	__out ResType & Type
	)
/*++

Routine Description:

	This routine reads an encapsulated file directory entry, returning the name
	and type of a particular resource.  The enumeration is stable across calls.

	Currently, no files may be opened at the time of this call as the call
	requires opening a file handle.

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
typename ZipFileReader< ResRefT >::FileId
ZipFileReader< ResRefT >::GetEncapsulatedFileCount(
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
typename ZipFileReader< ResRefT >::ZipArchive
ZipFileReader< ResRefT >::OpenArchive(
	__in const std::string & ArchiveName
	)
/*++

Routine Description:

	This routine opens a .zip archive and returns an archive handle for it.

Arguments:

	ArchiveName - Supplies the name of the .zip to open.

Return Value:

	The routine returns an archive handle on success, else NULL on failure.

Environment:

	User mode.

--*/
{
	zlib_filefunc_def ZIoApis;

	x_fill_fopen_filefunc( &ZIoApis );

	return (ZipArchive) unzOpen2( ArchiveName.c_str( ), &ZIoApis );
}

template< typename ResRefT >
void
ZipFileReader< ResRefT >::CloseArchive(
	__in ZipArchive Archive
	)
/*++

Routine Description:

	This routine closes an opened .zip archive.

Arguments:

	Archive - Supplies the opened archive handle as returned from OpenArchive.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	if (m_FileOpened)
	{
		unzCloseCurrentFile( (unzFile) Archive );

		m_FileOpened = false;
	}

	unzClose( (unzFile) Archive );
}


template< typename ResRefT >
void
ZipFileReader< ResRefT >::ScanArchive(
	__in ZipArchive Archive
	)
/*++

Routine Description:

	This routine enumerates all files in a .zip archive, and adds them to the
	master directory list.

Arguments:

	Archive - Supplies the .zip archive handle to scaN.

Return Value:

	None.  The routine raises an std::exception on catastrophic failure, such as
	if there was already an open file handle within the archive.

Environment:

	User mode.

--*/
{
	char            FileName[ 260 ];
	int             Error;
	unz_global_info ZInfo;

	if (m_FileOpened)
		throw std::runtime_error( "Cannot scan an archive with a file opened." );

	strcpy_s( FileName, "Z:\\" ); // Bogus, for splitpath.

	Error = unzGoToFirstFile2(
		(unzFile) Archive,
		FileName + 3,
		sizeof( FileName ) - 3 );

	if (Error)
		throw std::runtime_error( "unzGoToFirstFile failed" );

	//
	// Preallocate the directory entry array based on the count of files in
	// this archive.
	//
	// N.B.  The file count also includes directory-only files, but we do not
	//       need account for this as we're just reserving raw storage.
	//

	Error = unzGetGlobalInfo( (unzFile) Archive, &ZInfo );

	if (Error)
		throw std::runtime_error( "unzGetGlobalInfo failed" );

	//
	// Max size that we'll prereserve for.
	//

	if (ZInfo.number_entry > 100 * 1024)
		ZInfo.number_entry = 100 * 1024;

	m_DirectoryEntries.reserve( ZInfo.number_entry );

	//
	// Now iterate through each file, retrieving position and name data so that we
	// may create directory entries as appropriate.
	//

	do
	{
		DirectoryEntry Entry;
		size_t         Len;
		unz_file_pos   FilePos;
		char           Name[ MAX_PATH ];
		char           Ext[ 32 ];

		//
		// Break the name up into its component forms and discern the resource type
		// from the file extension.
		//

		FileName[ sizeof( FileName ) - sizeof( FileName[ 0 ] ) ] = '\0';

		if (!FileName[ 0 ])
			continue;

		_strlwr( FileName );

		if (_splitpath_s(
			FileName,
			NULL,
			0,
			NULL,
			0,
			Name,
			sizeof( Name ),
			Ext,
			sizeof( Ext )) || (Ext[ 0 ] == '\0'))
		{
			//
			// Skip directories.
			//

			if (FileName[ strlen( FileName ) - 1 ] == '/')
				continue;

			try
			{
				std::string ErrorStr;

				ErrorStr  = "_splitpath_s '";
				ErrorStr += FileName;
				ErrorStr += "'. failed";

				throw std::runtime_error( ErrorStr );
			}
			catch (std::bad_alloc)
			{
				throw std::runtime_error( "splitpath_s failed" );
			}
		}

		Len = strlen( Name );

		ZeroMemory( &Entry.Name, sizeof( Entry.Name ) );
		memcpy( &Entry.Name, Name, min( sizeof( Entry.Name ), Len ) );

		Entry.Type = ExtToResType( Ext + 1 );

		//
		// Determine the offset in the zip file to use as a file handle.
		//

		Error = unzGetFilePos( (unzFile) Archive, &FilePos );

		if (Error)
			throw std::runtime_error( "unzGetFilePos failed" );

		Entry.FileHandleToReturn = ((((ULONG64) FilePos.pos_in_zip_directory)) |
		                            (((ULONG64) FilePos.num_of_file) >> 32)) + 1;

		m_DirectoryEntries.push_back( Entry );
	} while ((Error = unzGoToNextFile2(
		(unzFile) Archive,
		FileName + 3,
		sizeof( FileName ) - 3 )) == 0) ;

	if (Error != UNZ_END_OF_LIST_OF_FILE)
		throw std::runtime_error( "unzGoToNextFile failed" );
}

template< typename ResRefT >
const typename ZipFileReader< ResRefT >::DirectoryEntry *
ZipFileReader< ResRefT >::LocateFileByName(
	__in const ResRefT & FileName,
	__in ResType Type
	)
/*++

Routine Description:

	This routine lookups up the directory entry for a file in the directory
	listing of the zip file reader.

Arguments:

	FileName - Supplies the name of the resource file to open.

	Type - Supplies the type of file to open (i.e. ResTRN, ResARE).

Return Value:

	The routine returns a pointer to the directory entry decriptor for the given
	file on success, else NULL on failure.

Environment:

	User mode.

--*/
{
	for (DirectoryEntryVec::const_iterator it = m_DirectoryEntries.begin( );
	     it != m_DirectoryEntries.end( );
	     ++it)
	{
		if (it->Type != Type)
			continue;

		if (memcmp( &FileName, &it->Name, sizeof( FileName ) ))
			continue;

		return &*it;
	}

	return NULL;
}

template ZipFileReader< NWN::ResRef32 >;

#ifdef USE_CRTIO_HACK
#define _fread_nolock fread
#define _fwrite_nolock fwrite
#define _ftell_nolock ftell
#define _fseek_nolock fseek

#if 0
size_t __cdecl crtfread_nolock(
    void *buffer,
    size_t elementSize,
    size_t count,
    FILE *stream);

size_t __cdecl crtfwrite_nolock (
        const void *buffer,
        size_t size,
        size_t num,
        FILE *stream);

long __cdecl crtftell_nolock (


        FILE *str
        );

int __cdecl crtfseek_nolock (


        FILE *str,
        long offset,
        int whence
        );
#endif

#endif

//
// Define file I/O wrappers for the unzip library.
//

static voidpf ZCALLBACK x_fopen_file_func OF((
   voidpf opaque,
   const char* filename,
   int mode));

static uLong ZCALLBACK x_fread_file_func OF((
   voidpf opaque,
   voidpf stream,
   void* buf,
   uLong size));

static uLong ZCALLBACK x_fwrite_file_func OF((
   voidpf opaque,
   voidpf stream,
   const void* buf,
   uLong size));

static long ZCALLBACK x_ftell_file_func OF((
   voidpf opaque,
   voidpf stream));

static long ZCALLBACK x_fseek_file_func OF((
   voidpf opaque,
   voidpf stream,
   uLong offset,
   int origin));

static int ZCALLBACK x_fclose_file_func OF((
   voidpf opaque,
   voidpf stream));

static int ZCALLBACK x_ferror_file_func OF((
   voidpf opaque,
   voidpf stream));

#define BUFSIZE 1024

//
// On 64-bit builds, use mapped file reader for the major performance gains
// from avoiding all of the tiny read calls.  On 32-bit, we're stuck with the
// CRT-like I/O calls due to address space pressure.
//

#ifdef _WIN64
#define ZLIBIO_USE_MAPPED_FILE_READER 1
#else
#define ZLIBIO_USE_MAPPED_FILE_READER 0
#endif

struct FILE_CTX
{
    FILE          * stream;
    size_t          offset;
#if ZLIBIO_USE_MAPPED_FILE_READER
	FileWrapper   * MappedFileReader;
	int             MFRError;
	bool            MFRAtEof;
	uLong           MFRFileSize;
#endif
};

#define _XSTRM(o) ((FILE_CTX*)o)->stream
#define _XOFFS(o) ((FILE_CTX*)o)->offset

static voidpf ZCALLBACK x_fopen_file_func(
   voidpf opaque,
   const char* filename,
   int mode)
{
    FILE* file = NULL;
    FILE_CTX * ctx = NULL;
    const char* mode_fopen = NULL;
    UNREFERENCED_PARAMETER(opaque);
    if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rbSN";
    else
    if (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+bSN";
    else
    if (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wbSN";

#if ZLIBIO_USE_MAPPED_FILE_READER
	if ((mode_fopen != NULL) && (!(mode & ZLIB_FILEFUNC_MODE_CREATE)))
	{
		HANDLE hf;

		hf = CreateFile(
			filename,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hf != INVALID_HANDLE_VALUE)
		{
			ctx = (FILE_CTX *) malloc( sizeof( FILE_CTX ) );

			try
			{
				ctx->MappedFileReader = new FileWrapper( INVALID_HANDLE_VALUE );
				ctx->MappedFileReader->SetFileHandle( hf, true );
				ctx->stream = (FILE *) hf;
				ctx->offset = 0;
				ctx->MFRError = 0;
				ctx->MFRAtEof = false;
				ctx->MFRFileSize = (uLong) ctx->MappedFileReader->GetFileSize( );
				return ctx;
			}
			catch (std::exception)
			{
				free( ctx );
				ctx = NULL;
				CloseHandle( hf );
			}
		}
	}
#endif

    if ((filename!=NULL) && (mode_fopen != NULL))
        file = fopen(filename, mode_fopen);

    if (file != NULL)
        setvbuf(file, NULL, _IOFBF, BUFSIZE);
	else
		return NULL;

    ctx = (FILE_CTX *)malloc(sizeof(FILE_CTX));
    if (ctx == NULL)
	{
        fclose(file);
        return NULL;
	}
    ctx->stream = file;
    ctx->offset = 0;
#if ZLIBIO_USE_MAPPED_FILE_READER
	ctx->MappedFileReader = NULL;
#endif
    return ctx;
}


static uLong ZCALLBACK x_fread_file_func(
   voidpf opaque,
   voidpf stream,
   void* buf,
   uLong size)
{
    uLong ret;
	FILE_CTX *ctx;
    UNREFERENCED_PARAMETER(opaque);
	ctx = (FILE_CTX *)stream;
#if ZLIBIO_USE_MAPPED_FILE_READER
	if (ctx->MappedFileReader != NULL)
	{
		if (ctx->MFRAtEof)
			return 0;

		try
		{
			ctx->MappedFileReader->ReadFile(
				buf,
				size,
				"zlib minizip file read" );
		}
		catch (std::exception)
		{
			((FILE_CTX *)stream)->MFRError = 1;
			return 0;
		}
		return (uLong) size;
	}
#endif

    ret = (uLong)_fread_nolock(buf, 1, (size_t)size, _XSTRM(stream));
    _XOFFS(stream) += ret;
    return ret;
}


static uLong ZCALLBACK x_fwrite_file_func(
   voidpf opaque,
   voidpf stream,
   const void* buf,
   uLong size)
{
    uLong ret;
    UNREFERENCED_PARAMETER(opaque);
#if ZLIBIO_USE_MAPPED_FILE_READER
	if (((FILE_CTX *)stream)->MappedFileReader != NULL)
		return 0;
#endif
    ret = (uLong)_fwrite_nolock(buf, 1, (size_t)size, _XSTRM(stream));
    _XOFFS(stream) += ret;
    return ret;
}

inline static long ZCALLBACK x_ftell_file_func(
   voidpf opaque,
   voidpf stream)
{
    long ret;
	FILE_CTX *ctx;
    UNREFERENCED_PARAMETER(opaque);
	ctx = (FILE_CTX *)stream;
#if ZLIBIO_USE_MAPPED_FILE_READER
	if (ctx->MappedFileReader != NULL)
	{
		if (ctx->MFRAtEof)
			return (long)ctx->MFRFileSize;
		else
			return (long)ctx->MappedFileReader->GetFilePointer();
	}
#endif
//  ret = _ftell_nolock(_XSTRM(stream));
    ret = (long)_XOFFS(stream);
    return ret;
}

static long ZCALLBACK x_fseek_file_func(
   voidpf opaque,
   voidpf stream,
   uLong offset,
   int origin)
{
    long curpos;
    uLong delta;
    int tryread;
    int fseek_origin=0;
    long ret;
    UNREFERENCED_PARAMETER(opaque);
#if ZLIBIO_USE_MAPPED_FILE_READER
	if (((FILE_CTX*)stream)->MappedFileReader != NULL)
	{
		FILE_CTX *ctx = (FILE_CTX*)stream;
		FileWrapper *wrapper = ctx->MappedFileReader;
		long newoffs;

		try
		{
			switch (origin)
			{
			case ZLIB_FILEFUNC_SEEK_CUR:
				newoffs = x_ftell_file_func(opaque, stream) + (long)offset;
				break;
			case ZLIB_FILEFUNC_SEEK_END:
				newoffs = (long)ctx->MFRFileSize + (long)offset;
				break;
			case ZLIB_FILEFUNC_SEEK_SET:
				newoffs = (long)offset;
				break;
			default:
				return -1;
			}

			if (newoffs == (long)ctx->MFRFileSize)
			{
				ctx->MFRAtEof = true;
			}
			else
			{
				ctx->MFRAtEof = false;
				wrapper->SeekOffset((ULONGLONG)newoffs, "zlib minizip file seek");
			}

			return 0;
		}
		catch (std::exception)
		{
			((FILE_CTX *)stream)->MFRError = 1;
			return -1;
		}
	}
#endif
    switch (origin)
    {
    case ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        break;
    case ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        break;
    case ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        break;
    default: return -1;
    }
    ret = 0;
    tryread = 0;
    delta = 0; // Compiler satisfaction
    if ((fseek_origin == SEEK_CUR))
    {
        if (offset == 0) return 0;
        if (offset < BUFSIZE)
        {
            curpos = (long)_XOFFS(stream);
            delta = (uLong) offset;
            tryread = 1;
        }
    }
    else if ((fseek_origin == SEEK_SET))
    {
        curpos = (long)_XOFFS(stream);
        if (offset == (uLong) curpos) return 0;
        if (offset > (uLong) curpos)
        {
            delta = offset - (uLong) curpos;
            if (delta <= BUFSIZE)
                tryread = 1;
//			else
//                WriteText("Delta too large...%d\n", delta);
        }// else WriteText("Seek back %d [%lu->%lu]\n", offset - curpos, curpos, offset);
    }
    if (tryread)
    {
        static char dummy[BUFSIZE];
        size_t d;
//        WriteText("Quick seek %lu (%lu)\n", offset, (size_t)delta);
        d = _fread_nolock(dummy, 1, (size_t)delta, _XSTRM(stream));
        _XOFFS(stream) += d;
        if (d == delta) return 0;
  //      WriteText("Quick seek failed\n", offset, fseek_origin);
    }
//	WriteText("Slow seek %d %d\n", offset, fseek_origin);
    _fseek_nolock(_XSTRM(stream), offset, fseek_origin);
    _XOFFS(stream) = _ftell_nolock(_XSTRM(stream));
    return ret;
}

static int ZCALLBACK x_fclose_file_func(
   voidpf opaque,
   voidpf stream)
{
    int ret;
	FILE_CTX *ctx;
    UNREFERENCED_PARAMETER(opaque);
	ctx = (FILE_CTX*)stream;
	
#if ZLIBIO_USE_MAPPED_FILE_READER
	if (ctx->MappedFileReader == NULL)
	{
#endif
		ret = fclose(_XSTRM(stream));
#if ZLIBIO_USE_MAPPED_FILE_READER
	}
	else
	{
		delete ctx->MappedFileReader;
		CloseHandle( (HANDLE) ctx->stream );
		ret = 0;
	}
#endif

    free((FILE_CTX*)stream);
    return ret;
}

static int ZCALLBACK x_ferror_file_func(
   voidpf opaque,
   voidpf stream)
{
    int ret;
	FILE_CTX *ctx;
    UNREFERENCED_PARAMETER(opaque);
	ctx = (FILE_CTX*)stream;

#if ZLIBIO_USE_MAPPED_FILE_READER
	if (ctx->MappedFileReader == NULL)
#endif
		ret = ferror(_XSTRM(stream));
#if ZLIBIO_USE_MAPPED_FILE_READER
	else
		ret = ctx->MFRError;
#endif

    return ret;
}

static void x_fill_fopen_filefunc(
  zlib_filefunc_def* pzlib_filefunc_def)
{
    pzlib_filefunc_def->zopen_file = x_fopen_file_func;
    pzlib_filefunc_def->zread_file = x_fread_file_func;
    pzlib_filefunc_def->zwrite_file = x_fwrite_file_func;
    pzlib_filefunc_def->ztell_file = x_ftell_file_func;
    pzlib_filefunc_def->zseek_file = x_fseek_file_func;
    pzlib_filefunc_def->zclose_file = x_fclose_file_func;
    pzlib_filefunc_def->zerror_file = x_ferror_file_func;
    pzlib_filefunc_def->opaque = NULL;
}
