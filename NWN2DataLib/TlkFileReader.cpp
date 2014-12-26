/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	TlkFileReader.cpp

Abstract:

	This module houses the *.tlk file format parser, which is used to read
	Talk Table (TLK) files.

--*/

#include "Precomp.h"
#include "TlkFileReader.h"

template< typename ResRefT >
TlkFileReader< ResRefT >::TlkFileReader(
	__in const std::string & FileName
	)
/*++

Routine Description:

	This routine constructs a new TlkFileReader object and parses the contents
	of a TLK file by filename.  The file must already exist as it
	immediately deserialized.

Arguments:

	FileName - Supplies the path to the TLK file.

Return Value:

	The newly constructed object.

Environment:

	User mode.

--*/
: m_File( INVALID_HANDLE_VALUE ),
  m_FileSize( 0 ),
  m_StringsOffset( 0 )
{
	HANDLE File;

	File = CreateFileA(
		FileName.c_str( ),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
		NULL);

	if (File == INVALID_HANDLE_VALUE)
		throw std::exception( "Failed to open TLK file." );

	m_File = File;

	m_FileWrapper.SetFileHandle( File );

	try
	{
		m_FileSize = GetFileSize( File, NULL );

		if ((m_FileSize == 0xFFFFFFFF) && (GetLastError( ) != NO_ERROR))
			throw std::exception( "Failed to read file size." );

		ParseTlkFile( );
	}
	catch (...)
	{
		m_File = INVALID_HANDLE_VALUE;

		CloseHandle( File );

		m_FileWrapper.SetFileHandle( INVALID_HANDLE_VALUE );

		throw;
	}

	C_ASSERT( sizeof( TLK_HEADER ) == 5 * 4 );
	C_ASSERT( sizeof( TLK_STRING ) == 6 * 4 + sizeof( ResRefT ) );
}

template< typename ResRefT >
TlkFileReader< ResRefT >::~TlkFileReader(
	)
/*++

Routine Description:

	This routine cleans up an already-existing TlkFileReader object.

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
bool
TlkFileReader< ResRefT >::GetTalkString(
	__in typename TlkFileReader< ResRefT >::StrRef StringId,
	__out std::string & String
	) const
/*++

Routine Description:

	This routine reads a string from the talk file's string directory.

Arguments:

	StringId - Supplies the string ordinal to fetch.

	String - Receives the translated string.

Return Value:

	The routine returns a Boolean value indicating true on success, else false
	on failure (i.e. unknown string).

Environment:

	User mode.

--*/
{
	PCTLK_STRING StringDesc;

	String.clear( );

	StringDesc = LookupStringDescriptor( StringId );

	if (StringDesc == NULL)
		return false;

	if (!(StringDesc->Flags & TEXT_PRESENT))
		return true;

	String.resize( StringDesc->StringSize );

	if (StringDesc->StringSize == 0)
		return true;

	m_FileWrapper.SeekOffset(
		m_StringsOffset + StringDesc->OffsetToString,
		"Seek to String" );
	m_FileWrapper.ReadFile( &String[ 0 ], String.size( ), "Read String" );

	return true;
}

template< typename ResRefT >
void
TlkFileReader< ResRefT >::ParseTlkFile(
	)
/*++

Routine Description:

	This routine parses the directory structures of an TLK file and generates
	the in-memory string directory.

Arguments:

	None.

Return Value:

	None.  On failure, the routine raises an std::exception.

Environment:

	User mode.

--*/
{
	TLK_HEADER   Header;
	HANDLE       Section;
	void       * View;

	m_FileWrapper.ReadFile( &Header, sizeof( Header ), "Header" );

	if (Header.StringCount * sizeof( TLK_STRING ) < Header.StringCount)
		return;

	if (Header.StringCount * sizeof( TLK_STRING ) + sizeof( TLK_HEADER ) < Header.StringCount * sizeof( TLK_STRING ))
		return;

	if (Header.StringCount < 1024 * 1024)
	{
		m_StringDir.reserve( Header.StringCount );
	}
	else
	{
		m_StringDir.reserve( 1024 * 1024 );
	}

	Section = CreateFileMapping( m_File, NULL, PAGE_READONLY, 0, 0, NULL );

	if (Section == NULL)
		return;

	View = MapViewOfFile( Section, FILE_MAP_READ, 0, 0, 0 );
	CloseHandle( Section );

	if (View == NULL)
		return;

	m_StringsOffset = Header.StringEntriesOffset;

	try
	{
		for (unsigned long i = 0; i < Header.StringCount; i += 1)
		{
			TLK_STRING TlkString;

			memcpy(
				&TlkString,
				((unsigned char *) View) + sizeof( TLK_HEADER ) + i * sizeof( TLK_STRING ),
				sizeof( TlkString ));

//			m_FileWrapper.ReadFile( &TlkString, sizeof( TlkString ), "TlkString" );

			m_StringDir.push_back( TlkString );
		}
	}
	catch (...)
	{
		UnmapViewOfFile( View );
		throw;
	}
	
	UnmapViewOfFile( View );
}

template TlkFileReader< NWN::ResRef16 >;
