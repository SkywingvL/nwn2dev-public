/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	GffFileReader.cpp

Abstract:

	This module houses the *.gff file format parser, which is used to read
	Generic File Format (GFF) files.  These files may also have alternative
	extensions such as *.bic.

--*/

#include "Precomp.h"
#include "GffFileReader.h"
#include "ResourceManager.h"
#include "GffInternal.h"

#define SEEK_OFFSET( Offset ) m_FileWrapper.SeekOffset( Offset, #Offset )
#define READ_FILE( P, Length ) m_FileWrapper.ReadFile( P, Length, #P )

GffFileReader::GffFileReader(
	__in const std::string & FileName,
	__in ResourceManager & ResMan
	)
/*++

Routine Description:

	This routine constructs a new GffFileReader object and parses the contents
	of a GFF file by filename.  The file must already exist as it
	immediately deserialized.

Arguments:

	FileName - Supplies the path to the GFF file.

	ResMan - Supplies the resource manager instance that is used to look up
	         STRREFs from talk tables.

Return Value:

	The newly constructed object.

Environment:

	User mode.

--*/
: m_File( INVALID_HANDLE_VALUE ),
  m_FileSize( 0 ),
  m_Language( LangEnglish ),
  m_ResourceManager( ResMan )
{
	HANDLE File;

	File = CreateFileA(
		FileName.c_str( ),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
		NULL);

	if (File == INVALID_HANDLE_VALUE)
	{
		File = CreateFileA(
			FileName.c_str( ),
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
			NULL);

		if (File == INVALID_HANDLE_VALUE)
			throw std::exception( "Failed to open GFF file." );
	}

	m_File = File;

	m_FileWrapper.SetFileHandle( File );

	try
	{
		m_FileSize = GetFileSize( File, NULL );

		if ((m_FileSize == 0xFFFFFFFF) && (GetLastError( ) != NO_ERROR))
			throw std::exception( "Failed to read file size." );

		ParseGffFile( );
	}
	catch (...)
	{
		m_File = INVALID_HANDLE_VALUE;

		CloseHandle( File );

		m_FileWrapper.SetFileHandle( INVALID_HANDLE_VALUE );

		throw;
	}
}

GffFileReader::GffFileReader(
	__in_bcount( DataSize ) const void * GffRawData,
	__in size_t DataSize,
	__in ResourceManager & ResMan
	)
/*++

Routine Description:

	This routine constructs a new GffFileReader object and parses the contents
	of a GFF file by raw in-memory buffer.  The raw memory buffer must remain
	valid for the lifetime of the GffFileReader object.

Arguments:

	GffRawData - Supplies the raw GFF file data to process.  The caller must
	             guarantee that the buffer remains legal throughout the
	             lifetime of the GffFileReader object.

	DataSize - Supplies the length, in bytes, of the raw data buffer.

	ResMan - Supplies the resource manager instance that is used to look up
	         STRREFs from talk tables.

Return Value:

	The newly constructed object.

Environment:

	User mode.

--*/
: m_File( INVALID_HANDLE_VALUE ),
  m_FileSize( (unsigned long) DataSize ),
  m_Language( LangEnglish ),
  m_ResourceManager( ResMan )
{
	m_FileWrapper.SetExternalView(
		(const unsigned char *) GffRawData,
		(ULONGLONG) DataSize);

	ParseGffFile( );
}

GffFileReader::~GffFileReader(
	)
/*++

Routine Description:

	This routine cleans up an already-existing GffFileReader object.

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
GffFileReader::ParseGffFile(
	)
/*++

Routine Description:

	This routine parses the contents of the GFF file, which consists of
	reading the main fixed header block in.

Arguments:

	StringId - Supplies the string ordinal to fetch.

	String - Receives the translated string.

Return Value:

	None.  The routine raises an std::exception on failure, such as a parse
	failure.

Environment:

	User mode.

--*/
{
	GFF_STRUCT_ENTRY RootStructEntry;
	ULONGLONG        FileSize;

	//
	// Pull in the header and make sure that things are sane.
	//

	m_FileWrapper.SeekOffset( 0, "GFFHeader" );
	READ_FILE( &m_Header, sizeof( m_Header ) );

	if (memcmp( &m_Header.Version, GFF_VERSION_CURRENT, 4 ))
		throw std::runtime_error( "Unrecognized GFF version." );

	//
	// Validate header contents.
	//

	FileSize = m_FileWrapper.GetFileSize( );

	if ((ULONGLONG) m_Header.StructCount * sizeof( GFF_STRUCT_ENTRY ) + m_Header.StructOffset > FileSize)
		throw std::runtime_error( "Struct accounting is incorrect." );
	if ((ULONGLONG) m_Header.FieldCount * sizeof( GFF_FIELD_ENTRY ) + m_Header.FieldOffset > FileSize)
		throw std::runtime_error( "Field accounting is incorrect." );
	if ((ULONGLONG) m_Header.LabelCount * sizeof( GFF_LABEL_ENTRY ) + m_Header.LabelOffset > FileSize)
		throw std::runtime_error( "Label accounting is incorrect." );
	if ((ULONGLONG) m_Header.FieldDataCount + m_Header.FieldDataOffset > FileSize)
		throw std::runtime_error( "Field data accounting is incorrect." );
	if ((ULONGLONG) m_Header.FieldIndiciesCount + m_Header.FieldIndiciesOffset > FileSize)
		throw std::runtime_error( "Field indicies accounting is incorrect." );
	if (m_Header.FieldIndiciesCount % sizeof( FIELD_INDEX ) != 0)
		throw std::runtime_error( "Field indicies array is not a multiple of FIELD_INDEX." );
	if ((ULONGLONG) m_Header.ListIndiciesCount + m_Header.ListIndiciesOffset > FileSize)
		throw std::runtime_error( "List indicies accounting is incorrect." );

	//
	// Now pull in the default structure.
	//

	GetStructByIndex( 0, RootStructEntry );

	if (RootStructEntry.Type != 0xFFFFFFFF)
		throw std::runtime_error( "Unexpected root structure type." );

	m_RootStruct.SetReader( this );
	m_RootStruct.SetStructEntry( &RootStructEntry );

	//
	// The remainder of the file is just processed on demand.
	//
}

void
GffFileReader::GetFieldByIndex(
	__in FIELD_INDEX FieldIndex,
	__out GFF_FIELD_ENTRY & FieldEntry
	) const
/*++

Routine Description:

	This routine retrieves the field descriptor for a field by index.

Arguments:

	FieldIndex - Supplies the field index to retrieve.

	FieldEntry - Receives the field descriptor.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	if (FieldIndex >= m_Header.FieldCount)
		throw std::runtime_error( "Illegal field index." );

	SEEK_OFFSET( (ULONGLONG) FieldIndex * sizeof( GFF_FIELD_ENTRY ) + m_Header.FieldOffset );
	READ_FILE( &FieldEntry, sizeof( FieldEntry ) );
}

void
GffFileReader::GetLabelByIndex(
	__in LABEL_INDEX LabelIndex,
	__out std::string & Label
	) const
/*++

Routine Description:

	This routine retrieves the label text of a label by index.

Arguments:

	LabelIndex - Supplies the label index to retrieve.

	Label - Receives the label text.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	GFF_LABEL_ENTRY LabelEntry;

	if (LabelIndex >= m_Header.LabelCount)
		throw std::runtime_error( "Illegal label index." );

	SEEK_OFFSET( (ULONGLONG) LabelIndex * sizeof( GFF_LABEL_ENTRY ) + m_Header.LabelOffset );
	READ_FILE( &LabelEntry, sizeof( LabelEntry ) );

	//
	// Now convert the label to an std::string.
	//

	Label.clear( );
	Label.reserve( sizeof( LabelEntry.Name ) );

	for (size_t i = 0; i < sizeof( LabelEntry.Name ); i += 1)
	{
		if (LabelEntry.Name[ i ] == '\0')
			break;

		Label.push_back( LabelEntry.Name[ i ] );
	}
}

void
GffFileReader::GetStructByIndex(
	__in STRUCT_INDEX StructIndex,
	__out GFF_STRUCT_ENTRY & StructEntry
	) const
/*++

Routine Description:

	This routine retrieves the struct descriptor for a struct by index.

Arguments:

	StructIndex - Supplies the struct index to retrieve.

	StructEntry - Receives the struct descriptor.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	if (StructIndex >= m_Header.StructCount)
		throw std::runtime_error( "Illegal struct index." );

	SEEK_OFFSET( (ULONGLONG) StructIndex * sizeof( GFF_STRUCT_ENTRY ) + m_Header.StructOffset );
	READ_FILE( &StructEntry, sizeof( StructEntry ) );
}

bool
GffFileReader::CompareFieldName(
	__in const GFF_FIELD_ENTRY & FieldEntry,
	__in const char * Name
	) const
/*++

Routine Description:

	This routine determines whether the label of a field matches a given name.

Arguments:

	FieldEntry - Supplies the field descriptor that is being inquired about.

	Name - Supplies the label name string to test.

Return Value:

	The routine returns true if the labels match, else false if they do not.

	On catastrophic failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	size_t          NameLen;
	GFF_LABEL_ENTRY LabelEntry;
	GFF_LABEL_ENTRY CompareEntry;

	if (FieldEntry.LabelIndex >= m_Header.LabelCount)
		throw std::runtime_error( "Illegal label index." );

	SEEK_OFFSET( (ULONGLONG) FieldEntry.LabelIndex * sizeof( GFF_LABEL_ENTRY ) + m_Header.LabelOffset );
	READ_FILE( &LabelEntry, sizeof( LabelEntry ) );

	NameLen = strlen( Name );
	NameLen = min( NameLen, sizeof( LabelEntry.Name ) );

	ZeroMemory( &CompareEntry, sizeof( CompareEntry ) );
	memcpy( CompareEntry.Name, Name, NameLen );

	return !memcmp( CompareEntry.Name, LabelEntry.Name, sizeof( LabelEntry.Name ) );
}

bool
GffFileReader::GetFieldByName(
	__in PCGFF_STRUCT_ENTRY Struct,
	__in const char * FieldName,
	__out GFF_FIELD_ENTRY & FieldEntry
	) const
/*++

Routine Description:

	This routine locates a GFF field that matches a given name and is joined to
	a given struct.  The field descriptor is read if it exists.

Arguments:

	Struct - Supplies the struct entry whose fields are being inspected.

	FieldName - Supplies the name of the field to retrieve.

	FieldEntry - Receives the field descriptor of the matching field, on a
	             successful call.

Return Value:

	The routine returns true on success, else false if there was no such field
	that matched.

Environment:

	User mode.

--*/
{
	try
	{
		if (Struct->FieldCount == 1)
		{
			GetFieldByIndex( Struct->DataOrDataOffset, FieldEntry );

			return (CompareFieldName( FieldEntry, FieldName ) == true);
		}
		else
		{
			for (FIELD_INDICIES_INDEX IndexOffset = 0;
			     IndexOffset < Struct->FieldCount;
			     IndexOffset += 1)
			{
				FIELD_INDEX FieldIndex;

				if ((ULONGLONG) IndexOffset * sizeof( FIELD_INDEX ) + Struct->DataOrDataOffset > m_Header.FieldIndiciesCount)
					throw std::runtime_error( "Illegal field indicies index." );

				SEEK_OFFSET( (ULONGLONG) IndexOffset * sizeof( FIELD_INDEX ) + Struct->DataOrDataOffset + m_Header.FieldIndiciesOffset );
				READ_FILE( &FieldIndex, sizeof( FieldIndex ) );

				GetFieldByIndex( FieldIndex, FieldEntry );

				if (CompareFieldName( FieldEntry, FieldName ))
					return true;
			}

			return false;
		}
	}
	catch (std::exception)
	{
		return false;
	}
}

bool
GffFileReader::GetFieldByIndex(
	__in PCGFF_STRUCT_ENTRY Struct,
	__in FIELD_INDEX FieldIndex,
	__out GFF_FIELD_ENTRY & FieldEntry
	) const
/*++

Routine Description:

	This routine locates a GFF field that matches a given index and is joined to
	a given struct.  The field descriptor is read if it exists.

Arguments:

	Struct - Supplies the struct entry whose fields are being inspected.

	FieldIndex - Supplies the index of the field to query.

	FieldEntry - Receives the field descriptor of the matching field, on a
	             successful call.

Return Value:

	The routine returns true on success, else false if there was no such field
	that matched.

Environment:

	User mode.

--*/
{
	try
	{
		if (FieldIndex >= Struct->FieldCount)
			return false;

		if (Struct->FieldCount == 1)
		{
			//
			// The DataOrDataOffset field is the field index itself.
			//

			GetFieldByIndex( Struct->DataOrDataOffset, FieldEntry );
		}
		else
		{
			FIELD_INDICIES_INDEX IndexOffset;

			//
			// We need to to look in the field indicies table to find the field
			// index.
			//

			IndexOffset = (FIELD_INDICIES_INDEX) FieldIndex;

			if ((ULONGLONG) IndexOffset * sizeof( FIELD_INDEX ) + Struct->DataOrDataOffset > m_Header.FieldIndiciesCount)
				throw std::runtime_error( "Illegal field indicies index." );

			SEEK_OFFSET( (ULONGLONG) IndexOffset * sizeof( FIELD_INDEX ) + Struct->DataOrDataOffset + m_Header.FieldIndiciesOffset );
			READ_FILE( &FieldIndex, sizeof( FieldIndex ) );

			GetFieldByIndex( FieldIndex, FieldEntry );
		}

		return true;
	}
	catch (std::exception)
	{
		return false;
	}
}

bool
GffFileReader::GetFieldIndexByName(
	__in PCGFF_STRUCT_ENTRY Struct,
	__in const char * FieldName,
	__out FIELD_INDEX & FieldIndex
	) const
/*++

Routine Description:

	This routine locates a GFF field that matches a given name and is joined to
	a given struct.  The field index of the field is returned.

Arguments:

	Struct - Supplies the struct entry whose fields are being inspected.

	FieldName - Supplies the name of the field to retrieve.

	FieldIndex - Receives the field index of the matching field, on a
	             successful call.

Return Value:

	The routine returns true on success, else false if there was no such field
	that matched.

Environment:

	User mode.

--*/
{
	try
	{
		GFF_FIELD_ENTRY FieldEntry;

		if (Struct->FieldCount == 1)
		{
			FieldIndex = 0;

			return (CompareFieldName( FieldEntry, FieldName ) == true);
		}
		else
		{
			for (FIELD_INDICIES_INDEX IndexOffset = 0;
			     IndexOffset < Struct->FieldCount;
			     IndexOffset += 1)
			{
				FIELD_INDEX QueryFieldIndex;

				if ((ULONGLONG) IndexOffset * sizeof( FIELD_INDEX ) + Struct->DataOrDataOffset > m_Header.FieldIndiciesCount)
					throw std::runtime_error( "Illegal field indicies index." );

				SEEK_OFFSET( (ULONGLONG) IndexOffset * sizeof( FIELD_INDEX ) + Struct->DataOrDataOffset + m_Header.FieldIndiciesOffset );
				READ_FILE( &QueryFieldIndex, sizeof( QueryFieldIndex ) );

				GetFieldByIndex( QueryFieldIndex, FieldEntry );

				if (CompareFieldName( FieldEntry, FieldName ))
				{
					FieldIndex = QueryFieldIndex;
					return true;
				}
			}

			return false;
		}
	}
	catch (std::exception)
	{
		return false;
	}
}

bool
GffFileReader::GetFieldType(
	__in PCGFF_STRUCT_ENTRY Struct,
	__in FIELD_INDEX FieldIndex,
	__out GFF_FIELD_TYPE & FieldType
	) const
/*++

Routine Description:

	This routine returns the type of a field.

Arguments:

	Struct - Supplies the struct entry whose fields are being inspected.

	FieldIndex - Supplies the index of the field to query.

	FieldType - Receives the type of the matching field, on a successful call.

Return Value:

	The routine returns true on success, else false if there was no such field
	that matched.

Environment:

	User mode.

--*/
{
	GFF_FIELD_ENTRY FieldEntry;

	if (!GetFieldByIndex( Struct, FieldIndex, FieldEntry ))
		return false;

	FieldType = (GFF_FIELD_TYPE) FieldEntry.Type;

	return true;
}

bool
GffFileReader::GetFieldName(
	__in PCGFF_STRUCT_ENTRY Struct,
	__in FIELD_INDEX FieldIndex,
	__out std::string & FieldName
	) const
/*++

Routine Description:

	This routine retrieves the name of a field.

Arguments:

	Struct - Supplies the struct entry whose fields are being inspected.

	FieldIndex - Supplies the index of the field to query.

	FieldName - Receives the name of the field (on success).

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified. 

Environment:

	User mode.

--*/
{
	try
	{
		GFF_FIELD_ENTRY FieldEntry;

		if (!GetFieldByIndex( Struct, FieldIndex, FieldEntry ))
			return false;

		GetLabelByIndex( FieldEntry.LabelIndex, FieldName );

		return true;
	}
	catch (std::exception)
	{
		return false;
	}
}

bool
GffFileReader::GetFieldRawData(
	__in PCGFF_STRUCT_ENTRY Struct,
	__in FIELD_INDEX FieldIndex,
	__out std::vector< unsigned char > & FieldData,
	__out std::string & FieldName,
	__out GFF_FIELD_TYPE & FieldType,
	__out bool & ComplexField
	) const
/*++

Routine Description:

	This routine retrieves a pointer to the raw data for a field, given its
	index.  The routine is useful for making a copy of a GFF given a source
	GFF.

Arguments:

	Struct - Supplies the struct entry whose fields are being inspected.

	FieldIndex - Supplies the index of the field to query.

	FieldData - Receives the field data (on success).

	FieldName - Receives the name of the field (on success).

	FieldType - Receives the type of the field (on success).

	ComplexField - Receives true if the field is complex (that is, it is stored
	               in the field data section), or false if the field is simple
	               (that is, the DataOrDataIndex field is the data).

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified. 

Environment:

	User mode.

--*/
{
	try
	{
		GFF_FIELD_ENTRY   FieldEntry;
		const void      * FieldDataPtr;
		size_t            FieldDataLength;

		if (!GetFieldByIndex( Struct, FieldIndex, FieldEntry ))
			return false;

		//
		// Determine whether the field was a small field (that pointed directly
		// into the field entry itself), or whether we have to read it from the
		// field data section.
		//
		// If the field is a list or a struct, or is of an unknown type, we'll
		// return false here.
		//

		if (!GetFieldSizeAndData( FieldEntry, &FieldDataPtr, &FieldDataLength ))
			return false;

		FieldData.clear( );

		if (FieldDataLength != 0)
		{
			//
			// This was a small field, the data is inline.  Copy it directly.
			//

			FieldData.resize( FieldDataLength );

			memcpy( &FieldData[ 0 ], FieldDataPtr, FieldDataLength );

			ComplexField = false;
		}
		else
		{
			//
			// This was a large field, so the data comes from the field data
			// section.
			//

			ComplexField = true;

			switch (FieldEntry.Type)
			{

			case GFF_DWORD64:
			case GFF_INT64:
			case GFF_DOUBLE:
				FieldData.resize( 8 );

				if (!ReadFieldData(
					FieldEntry.DataOrDataOffset,
					&FieldData[ 0 ],
					8))
					return false;
				break;

			case GFF_VECTOR:
				FieldData.resize( 12 );

				if (!ReadFieldData(
					FieldEntry.DataOrDataOffset,
					&FieldData[ 0 ],
					12))
					return false;
				break;

			case GFF_CEXOSTRING:
			case GFF_CEXOLOCSTRING:
			case GFF_VOID:
				{
					unsigned __int32 Size;
					FIELD_DATA_INDEX Offset;

					if (!ReadFieldData(
						FieldEntry.DataOrDataOffset,
						&Size,
						sizeof( Size )))
						return false;

					FieldData.resize( 4 );
					memcpy( &FieldData[ 0 ], &Size, 4 );

					Offset = (FIELD_DATA_INDEX) FieldEntry.DataOrDataOffset + 4;

					if (Offset < (FIELD_DATA_INDEX) FieldEntry.DataOrDataOffset)
						return false;

					if (!ValidateFieldDataRange( Offset, Size ))
						return false;

					if (Size != 0)
					{
						if (4 + Size < Size)
							return false;

						FieldData.resize( 4 + Size );

						if (!ReadFieldData(
							Offset,
							&FieldData[ 4 ],
							Size))
							return false;
					}
				}
				break;

			case GFF_RESREF:
				{
					unsigned __int8  Size;
					FIELD_DATA_INDEX Offset;

					if (!ReadFieldData(
						FieldEntry.DataOrDataOffset,
						&Size,
						sizeof( Size )))
						return false;

					FieldData.resize( 1 );
					memcpy( &FieldData[ 0 ], &Size, 1 );

					Offset = (FIELD_DATA_INDEX) FieldEntry.DataOrDataOffset + 1;

					if (Offset < (FIELD_DATA_INDEX) FieldEntry.DataOrDataOffset)
						return false;

					if (!ValidateFieldDataRange( Offset, Size ))
						return false;

					if (Size != 0)
					{
						if (1 + Size < Size)
							return false;

						FieldData.resize( 1 + Size );

						if (!ReadFieldData(
							Offset,
							&FieldData[ 1 ],
							Size))
							return false;
					}
				}
				break;

			default:
				return false;

			}
		}

		//
		// Now return the type and label for the caller.
		//

		FieldType = (GFF_FIELD_TYPE) FieldEntry.Type;

		GetLabelByIndex( FieldEntry.LabelIndex, FieldName );

		return true;
	}
	catch (std::exception)
	{
		return false;
	}
}

bool
GffFileReader::ReadFieldData(
	__in FIELD_DATA_INDEX FieldDataIndex,
	__out_bcount( Length ) void * Buffer,
	__in size_t Length
	) const
/*++

Routine Description:

	This routine copies raw field data from the field data stream.

Arguments:

	FieldDataIndex - Supplies the byte offset into the field data stream to
	                 begin copying from.

	Buffer - Receives the field data copied.

	Length - Supplies the length of the buffer to read.

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified.

Environment:

	User mode.

--*/
{
	try
	{
		ULONGLONG FieldDataOffset;

		if (FieldDataIndex > m_Header.FieldDataCount)
			throw std::runtime_error( "Field data index out of range." );

		FieldDataOffset = (ULONGLONG) FieldDataIndex + m_Header.FieldDataOffset;

		SEEK_OFFSET( FieldDataOffset );
		READ_FILE( Buffer, Length );

		return true;
	}
	catch (std::exception)
	{
		return false;
	}
}

bool
GffFileReader::ReadListIndicies(
	__in LIST_INDICIES_INDEX ListIndiciesIndex,
	__out_bcount( Length ) void * Buffer,
	__in size_t Length
	) const
/*++

Routine Description:

	This routine copies raw list index data from the list indicies stream.

Arguments:

	ListIndiciesIndex - Supplies the byte offset into the list indicies stream
	                    to begin copying from.

	Buffer - Receives the list index data copied.

	Length - Supplies the length of the buffer to read.

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified.

Environment:

	User mode.

--*/
{
	try
	{
		ULONGLONG ListIndiciesOffset;

		if (ListIndiciesIndex > m_Header.ListIndiciesCount)
			throw std::runtime_error( "List indicies index out of range." );

		ListIndiciesOffset = (ULONGLONG) ListIndiciesIndex + m_Header.ListIndiciesOffset;

		SEEK_OFFSET( ListIndiciesOffset );
		READ_FILE( Buffer, Length );

		return true;
	}
	catch (std::exception)
	{
		return false;
	}
}

bool
GffFileReader::ValidateFieldDataRange(
	__in FIELD_DATA_INDEX FieldDataIndex,
	__in size_t Length
	) const
/*++

Routine Description:

	This routine checks that a read of a field data range is entirely
	contained within the legal field data stream.  It is useful to sanity-check
	sizes before making very large buffer allocations.

Arguments:

	FieldDataIndex - Supplies the offset into the field data stream.

	Length - Supplies the count, in bytes, of data that would be read.

Return Value:

	The routine returns true if the field data range is contained entirely in
	the field data stream, else false if it was invalid.

Environment:

	User mode.

--*/
{
	if ((size_t) ((FIELD_DATA_INDEX) (FieldDataIndex + Length)) < Length)
		return false;

	if ((FIELD_DATA_INDEX) (FieldDataIndex + Length) > m_Header.FieldDataCount)
		return false;

	return true;
}

bool
GffFileReader::GetFieldSizeAndData(
	__in const GFF_FIELD_ENTRY & FieldEntry,
	__deref __out const void * * FieldData,
	__out size_t * FieldDataLength
	) const
/*++

Routine Description:

	This routine returns the size and data pointer of a field.  If the field is
	a small field then the size is returned.  Otherwise if the field is a large
	field then zero is returned for the size.

	If the field type was unrecognized, or the field has no actual data (such
	as a list or a structure), then false is returned.

Arguments:

	FieldEntry - Supplies the field descriptor of the field being read.

	FieldData - Receives a pointer to the field data (on success).

	FieldDataLength - Receives the length of the field data (on success).

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified. 

Environment:

	User mode.

--*/
{
	switch (FieldEntry.Type)
	{

	case GFF_BYTE:
	case GFF_CHAR:
		*FieldData       = &FieldEntry.DataOrDataOffset;
		*FieldDataLength = 1;
		return true;

	case GFF_WORD:
	case GFF_SHORT:
		*FieldData       = &FieldEntry.DataOrDataOffset;
		*FieldDataLength = 2;
		return true;

	case GFF_DWORD:
	case GFF_INT:
		*FieldData       = &FieldEntry.DataOrDataOffset;
		*FieldDataLength = 4;
		return true;

	case GFF_DWORD64:
	case GFF_INT64:
		*FieldData       = NULL;
		*FieldDataLength = 0;
		return true;

	case GFF_FLOAT:
		*FieldData       = &FieldEntry.DataOrDataOffset;
		*FieldDataLength = 4;
		return true;

	case GFF_DOUBLE:
		*FieldData       = NULL;
		*FieldDataLength = 0;
		return true;

	case GFF_CEXOSTRING:
	case GFF_RESREF:
	case GFF_CEXOLOCSTRING:
	case GFF_VOID:
	case GFF_VECTOR:
		*FieldData       = NULL;
		*FieldDataLength = 0;
		return true;

	case GFF_STRUCT:
	case GFF_LIST:
		return false; // No data attached.

	case GFF_RESERVED:
	default:
		return false; // Not supported.

	}
}

bool
GffFileReader::GetTalkString(
	__in unsigned long StrRef,
	__out std::string & Str
	) const
/*++

Routine Description:

	This routine retrieves a talk string from the active resource manager talk
	tables.  It is used to support CExoLocString fields which may include
	STRREFs.

Arguments:

	StrRef - Supplies the STRREF to attempt to look up.

	Str - Receives the localized string if the lookup succeeded.

Return Value:

	The routine returns true if the StrRef matched a talk table entry, else it
	returns false.

Environment:

	User mode.

--*/
{
	return m_ResourceManager.GetTalkString( StrRef, Str );
}

NWN::ResRef32
GffFileReader::ResRef32FromStr(
	__in const std::string & Str
	) const
/*++

Routine Description:

	This routine converts a string to a 32-byte resource reference.

Arguments:

	Str - Supplies the string to convert.

Return Value:

	The routine returns a NWN::ResRef32 derived from the resource string.

Environment:

	User mode.

--*/
{
	return m_ResourceManager.ResRef32FromStr( Str );
}


//
// GffStruct members.
//

bool
GffFileReader::GffStruct::GetFieldName(
	__in FIELD_INDEX FieldIndex,
	__out std::string & FieldName
	) const
/*++

Routine Description:

	This routine retrieves the name of a field.

Arguments:

	FieldIndex - Supplies the index of the field to query.

	FieldName - Receives the name of the field (on success).

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified. 

Environment:

	User mode.

--*/
{
	return m_Reader->GetFieldName(
		&m_StructEntry,
		FieldIndex,
		FieldName);
}

bool
GffFileReader::GffStruct::GetFieldRawData(
	__in FIELD_INDEX FieldIndex,
	__out std::vector< unsigned char > & FieldData,
	__out std::string & FieldName,
	__out GFF_FIELD_TYPE & FieldType,
	__out bool & ComplexField
	) const
/*++

Routine Description:

	This routine retrieves a pointer to the raw data for a field, given its
	index.  The routine is useful for making a copy of a GFF given a source
	GFF.

Arguments:

	FieldIndex - Supplies the index of the field to query.

	FieldData - Receives a pointer to the field data (on success).

	FieldName - Receives the name of the field (on success).

	FieldType - Receives the type of the field (on success).

	ComplexField - Receives true if the field is complex (that is, it is stored
	               in the field data section), or false if the field is simple
	               (that is, the DataOrDataIndex field is the data).

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified. 

Environment:

	User mode.

--*/
{
	return m_Reader->GetFieldRawData(
		&m_StructEntry,
		FieldIndex,
		FieldData,
		FieldName,
		FieldType,
		ComplexField);
}

bool
GffFileReader::GffStruct::GetLargeFieldData(
	__in const GFF_FIELD_ENTRY & FieldEntry,
	__out_bcount( Size ) void * Data,
	__in size_t Size,
	__in size_t Offset /* = 0 */
	) const
/*++

Routine Description:

	This routine retrieves raw data for a flat, fixed-size field that is
	located in the field data stream.

Arguments:

	FieldEntry - Supplies the field descriptor to read the data for.

	Data - Receives the field data copied.

	Length - Supplies the length of field data element.

	Offset - Optionally supplies an offset into the field data.

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified.

Environment:

	User mode.

--*/
{
	if (FieldEntry.DataOrDataOffset + (FIELD_DATA_INDEX) Offset < FieldEntry.DataOrDataOffset)
		return false;

	return m_Reader->ReadFieldData(
		FieldEntry.DataOrDataOffset + (FIELD_DATA_INDEX) Offset,
		Data,
		Size);
}

bool
GffFileReader::GffStruct::GetListIndiciesData(
	__in const GFF_FIELD_ENTRY & FieldEntry,
	__out_bcount( Size ) void * Data,
	__in size_t Size,
	__in size_t Offset /* = 0 */
	) const
/*++

Routine Description:

	This routine retrieves raw data for a flat, fixed-size field that is
	located in the list indicies data stream.

Arguments:

	FieldEntry - Supplies the field descriptor to read the data for.

	Data - Receives the field data copied.

	Length - Supplies the length of field data element.

	Offset - Optionally supplies an offset into the field data.

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified.

Environment:

	User mode.

--*/
{
	if (FieldEntry.DataOrDataOffset + (LIST_INDICIES_INDEX) Offset < FieldEntry.DataOrDataOffset)
		return false;

	return m_Reader->ReadListIndicies(
		FieldEntry.DataOrDataOffset + (LIST_INDICIES_INDEX) Offset,
		Data,
		Size);
}

bool
GffFileReader::GffStruct::GetCExoString(
	__in const char * FieldName,
	__out std::string & Data
	) const
/*++

Routine Description:

	This routine reads a field of type CExoString.

Arguments:

	FieldName - Supplies the label of the field to read.

	Data - Receives the field data.

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified.

Environment:

	User mode.

--*/
{
	GFF_FIELD_ENTRY  FieldEntry;
	unsigned __int32 Size;

	if (!GetFieldByName( FieldName, FieldEntry ))
		return false;

	if (FieldEntry.Type != GFF_CEXOSTRING)
		return false;

	if (!GetLargeFieldData( FieldEntry, &Size, sizeof( Size ), 0 ))
		return false;

	if (!ValidateFieldDataRange( FieldEntry, 4, Size ))
		return false;

	try
	{
		Data.clear( );
		Data.resize( Size );
	}
	catch (std::exception)
	{
		return false;
	}

	if (Size == 0)
		return true;

	return GetLargeFieldData( FieldEntry, &Data[ 0 ], Size, 4 );
}

bool
GffFileReader::GffStruct::GetCExoStringAsResRef(
	__in const char * FieldName,
	__out NWN::ResRef32 & Data
	) const
/*++

Routine Description:

	This routine reads a field of type CExoString.  The field is converted to a
	NWN::ResRef32 before being returned.  If the string was too long to be a
	legal resref then it is truncated.

Arguments:

	FieldName - Supplies the label of the field to read.

	Data - Receives the field data.

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified.

Environment:

	User mode.

--*/
{
	try
	{
		std::string Str;

		if (!GetCExoString( FieldName, Str ))
			return false;

		Data = m_Reader->ResRef32FromStr( Str );

		return true;
	}
	catch (std::exception)
	{
		return false;
	}
}

bool
GffFileReader::GffStruct::GetResRef(
	__in const char * FieldName,
	__out NWN::ResRef32 & Data
	) const
/*++

Routine Description:

	This routine reads a field of type CResRef.

Arguments:

	FieldName - Supplies the label of the field to read.

	Data - Receives the field data.

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified.

Environment:

	User mode.

--*/
{
	GFF_FIELD_ENTRY  FieldEntry;
	unsigned __int8  Size;

	if (!GetFieldByName( FieldName, FieldEntry ))
		return false;

	if (FieldEntry.Type != GFF_RESREF)
		return false;

	if (!GetLargeFieldData( FieldEntry, &Size, sizeof( Size ), 0 ))
		return false;

	ZeroMemory( &Data, sizeof( Data ) );

	if (Size > sizeof( Data ))
		return false;

	return GetLargeFieldData( FieldEntry, &Data, Size, 1 );
}

bool
GffFileReader::GffStruct::GetCExoLocString(
	__in const char * FieldName,
	__out std::string & Data
	) const
/*++

Routine Description:

	This routine reads a field of type CExoLocString.  The localized string
	that is matched to the default language is returned.

Arguments:

	FieldName - Supplies the label of the field to read.

	Data - Receives the field data.

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified.

Environment:

	User mode.

--*/
{
	GFF_FIELD_ENTRY         FieldEntry;
	GFF_CEXOLOCSTRING_ENTRY LocString;
	size_t                  Offset;
	GFF_LANGUAGE            Language;

	if (!GetFieldByName( FieldName, FieldEntry ))
		return false;

	if (FieldEntry.Type != GFF_CEXOLOCSTRING)
		return false;

	if (!GetLargeFieldData( FieldEntry, &LocString, sizeof( LocString ), 0 ))
		return false;

	if (LocString.Length < sizeof( LocString ) - 4)
		return false;

	//
	// Make two passes to retrieve the string contents.  The first time, we
	// will match a string whose language code matches the default language,
	// and then a STRREF if one existed.
	//
	// The second time around, take the first language string if we had no
	// exact language matches and no STRREF.
	//

	Language = m_Reader->GetDefaultLanguage( );

	for (size_t Pass = 0; Pass < 2; Pass += 1)
	{
		//
		// First, we must search for a string that matches our active language
		// and return it should we find one.
		//

		Offset = sizeof( LocString );

		for (unsigned long i = 0; i < LocString.StringCount; i += 1)
		{
			GFF_CEXOLOCSUBSTRING_ENTRY SubString;

			if (Offset + sizeof( SubString ) > LocString.Length + 4)
				return false;

			if (!GetLargeFieldData(
				FieldEntry,
				&SubString,
				sizeof( SubString ),
				Offset))
				return false;

			Offset += sizeof( SubString );

			if (Offset + SubString.StringLength > LocString.Length + 4)
				return false;

			//
			// Take this string if it matches our expected language.
			//

			if ((Pass == 1) ||
			   ((SubString.StringID >> 1) == (unsigned long) Language))
			{
				if (!ValidateFieldDataRange(
					FieldEntry,
					(FIELD_DATA_INDEX) Offset,
					SubString.StringLength))
				{
					return false;
				}

				try
				{
					Data.clear( );
					Data.resize( SubString.StringLength );
				}
				catch (std::exception)
				{
					return false;
				}

				if (SubString.StringLength == 0)
					return true;

				return GetLargeFieldData(
					FieldEntry,
					&Data[ 0 ],
					SubString.StringLength,
					Offset);
			}

			Offset += SubString.StringLength;
		}

		//
		// If we have a STRREF, try it last.
		//

		if (LocString.StringRef != 0xFFFFFFFF)
		{
			if (m_Reader->GetTalkString( LocString.StringRef, Data ))
				return true;
		}
	}

	//
	// No strings matched our language, abort.
	//

	return false;
}

bool
GffFileReader::GffStruct::GetVOID(
	__in const char * FieldName,
	__out std::vector< unsigned char > & Data
	) const
/*++

Routine Description:

	This routine reads a field of type VOID.

Arguments:

	FieldName - Supplies the label of the field to read.

	Data - Receives the field data.

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified.

Environment:

	User mode.

--*/
{
	GFF_FIELD_ENTRY  FieldEntry;
	unsigned __int32 Size;

	if (!GetFieldByName( FieldName, FieldEntry ))
		return false;

	if (FieldEntry.Type != GFF_VOID)
		return false;

	if (!GetLargeFieldData( FieldEntry, &Size, sizeof( Size ), 0 ))
		return false;

	if (!ValidateFieldDataRange( FieldEntry, sizeof( Size ), Size ))
		return false;

	try
	{
		Data.clear( );
		Data.resize( Size );
	}
	catch (std::exception)
	{
		return false;
	}

	if (Size == 0)
		return true;

	return GetLargeFieldData( FieldEntry, &Data[ 0 ], Size, sizeof( Size ) );
}

bool
GffFileReader::GffStruct::GetStruct(
	__in_opt const char * FieldName,
	__out GffStruct & Struct
	) const
/*++

Routine Description:

	This routine reads a field of type Struct.

Arguments:

	FieldName - Supplies the label of the field to read.

	Data - Receives the field data.

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified.

Environment:

	User mode.

--*/
{
	GFF_FIELD_ENTRY  FieldEntry;
	GFF_STRUCT_ENTRY StructEntry;

	//
	// Allow the empty string to refer to self.  This is useful for allowing
	// the various GetXxx complex type getters to work on lists.
	//

	if (FieldName == NULL)
	{
		Struct = *this;
		return true;
	}

	if (!GetFieldByName( FieldName, FieldEntry ))
		return false;

	if (FieldEntry.Type != GFF_STRUCT)
		return false;

	try
	{
		m_Reader->GetStructByIndex( FieldEntry.DataOrDataOffset, StructEntry );
	}
	catch (std::exception)
	{
		return false;
	}

	Struct.SetReader( const_cast< GffFileReader * >( m_Reader ) );
	Struct.SetStructEntry( &StructEntry );

	return true;
}

bool
GffFileReader::GffStruct::GetStructByIndex(
	__in FIELD_INDEX FieldIndex,
	__out GffStruct & Struct
	) const
/*++

Routine Description:

	This routine reads a field of type Struct.

Arguments:

	FieldIndex - Supplies the index of the field to read.

	Data - Receives the field data.

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified.

Environment:

	User mode.

--*/
{
	GFF_FIELD_ENTRY  FieldEntry;
	GFF_STRUCT_ENTRY StructEntry;

	if (!GetFieldByIndex( FieldIndex, FieldEntry ))
		return false;

	if (FieldEntry.Type != GFF_STRUCT)
		return false;

	try
	{
		m_Reader->GetStructByIndex( FieldEntry.DataOrDataOffset, StructEntry );
	}
	catch (std::exception)
	{
		return false;
	}

	Struct.SetReader( const_cast< GffFileReader * >( m_Reader ) );
	Struct.SetStructEntry( &StructEntry );

	return true;
}

bool
GffFileReader::GffStruct::GetListElement(
	__in const char * FieldName,
	__in size_t Index,
	__out GffStruct & Struct
	) const
/*++

Routine Description:

	This routine reads a field of type Struct out of a List, which is an array
	of Structs attached to another Struct.

Arguments:

	FieldName - Supplies the label of the field to read.

	Index - Supplies the list index of the struct to read.

	Data - Receives the field data.

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified.

Environment:

	User mode.

--*/
{
	GFF_FIELD_ENTRY  FieldEntry;
	GFF_STRUCT_ENTRY StructEntry;
	unsigned __int32 Size;
	STRUCT_INDEX     StructIndex;

	if (!GetFieldByName( FieldName, FieldEntry ))
		return false;

	if (FieldEntry.Type != GFF_LIST)
		return false;

	if (!GetListIndiciesData( FieldEntry, &Size, sizeof( Size ), 0 ))
		return false;

	//
	// Check that we are within range of the list.
	//

	if (Index >= Size)
		return false;

	//
	// Now pull the struct index and the actual struct, and return that to the
	// caller as a new GffStruct object.
	//

	if (!GetListIndiciesData(
		FieldEntry,
		&StructIndex,
		sizeof( StructIndex ),
		Index * sizeof( StructIndex ) + sizeof( Size )))
	{
		return false;
	}

	try
	{
		m_Reader->GetStructByIndex( StructIndex, StructEntry );
	}
	catch (std::exception)
	{
		return false;
	}

	Struct.SetReader( const_cast< GffFileReader * >( m_Reader ) );
	Struct.SetStructEntry( &StructEntry );

	return true;
}

bool
GffFileReader::GffStruct::GetListElementByIndex(
	__in FIELD_INDEX FieldIndex,
	__in size_t Index,
	__out GffStruct & Struct
	) const
/*++

Routine Description:

	This routine reads a field of type Struct out of a List, which is an array
	of Structs attached to another Struct.

Arguments:

	FieldIndex - Supplies the index of the field to read.

	Index - Supplies the list index of the struct to read.

	Data - Receives the field data.

Return Value:

	The routine returns true on success, else false if the read could not be
	entirely satisified.

Environment:

	User mode.

--*/
{
	GFF_FIELD_ENTRY  FieldEntry;
	GFF_STRUCT_ENTRY StructEntry;
	unsigned __int32 Size;
	STRUCT_INDEX     StructIndex;

	if (!GetFieldByIndex( FieldIndex, FieldEntry ))
		return false;

	if (FieldEntry.Type != GFF_LIST)
		return false;

	if (!GetListIndiciesData( FieldEntry, &Size, sizeof( Size ), 0 ))
		return false;

	//
	// Check that we are within range of the list.
	//

	if (Index >= Size)
		return false;

	//
	// Now pull the struct index and the actual struct, and return that to the
	// caller as a new GffStruct object.
	//

	if (!GetListIndiciesData(
		FieldEntry,
		&StructIndex,
		sizeof( StructIndex ),
		Index * sizeof( StructIndex ) + sizeof( Size )))
	{
		return false;
	}

	try
	{
		m_Reader->GetStructByIndex( StructIndex, StructEntry );
	}
	catch (std::exception)
	{
		return false;
	}

	Struct.SetReader( const_cast< GffFileReader * >( m_Reader ) );
	Struct.SetStructEntry( &StructEntry );

	return true;
}



bool
GffFileReader::GffStruct::GetFieldType(
	__in FIELD_INDEX FieldIndex,
	__out GFF_FIELD_TYPE & FieldType
	) const
/*++

Routine Description:

	This routine returns the type of a field.

Arguments:

	FieldIndex - Supplies the index of the field to query.

	FieldType - Receives the type of the matching field, on a successful call.

Return Value:

	The routine returns true on success, else false if there was no such field
	that matched.

Environment:

	User mode.

--*/
{
	return m_Reader->GetFieldType( &m_StructEntry, FieldIndex, FieldType );
}

bool
GffFileReader::GffStruct::GetFieldByName(
	__in const char * FieldName,
	__out GFF_FIELD_ENTRY & FieldEntry
	) const
/*++

Routine Description:

	This routine locates a GFF field that matches a given name and is joined to
	the current struct.  The field descriptor is read if it exists.

Arguments:

	FieldName - Supplies the name of the field to retrieve.

	FieldEntry - Receives the field descriptor of the matching field, on a
	             successful call.

Return Value:

	The routine returns true on success, else false if there was no such field
	that matched.

Environment:

	User mode.

--*/
{
	return m_Reader->GetFieldByName( &m_StructEntry, FieldName, FieldEntry );
}

bool
GffFileReader::GffStruct::GetFieldByIndex(
	__in FIELD_INDEX FieldIndex,
	__out GFF_FIELD_ENTRY & FieldEntry
	) const
/*++

Routine Description:

	This routine retrieves the field descriptor for a field by index (relative
	to the structure).

Arguments:

	FieldIndex - Supplies the field index to retrieve.

	FieldEntry - Receives the field descriptor.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	return m_Reader->GetFieldByIndex( &m_StructEntry, FieldIndex, FieldEntry );
}

bool
GffFileReader::GffStruct::GetFieldIndexByName(
	__in const char * FieldName,
	__out FIELD_INDEX & Index
	) const
/*++

Routine Description:

	This routine locates a GFF field that matches a given name and is joined to
	the current struct.  The field index is returned if it exists.

Arguments:

	FieldName - Supplies the name of the field to retrieve.

	Index - Receives the field index of the matching field, on a successful
	        call.

Return Value:

	The routine returns true on success, else false if there was no such field
	that matched.

Environment:

	User mode.

--*/
{
	return m_Reader->GetFieldIndexByName( &m_StructEntry, FieldName, Index );
}

bool
GffFileReader::GffStruct::ValidateFieldDataRange(
	__in const GFF_FIELD_ENTRY & FieldEntry,
	__in FIELD_DATA_INDEX DataOffset,
	__in size_t Length
	) const
/*++

Routine Description:

	This routine checks that a read of a field data range is entirely
	contained within the legal field data stream.  It is useful to sanity-check
	sizes before making very large buffer allocations.

Arguments:

	FieldEntry - Supplies the field descriptor of the field being read.

	DataOffset - Supplies the offset into the field that the read would begin
	             at.

	Length - Supplies the count, in bytes, of data that would be read.

Return Value:

	The routine returns true if the field data range is contained entirely in
	the field data stream, else false if it was invalid.

Environment:

	User mode.

--*/
{
	FIELD_DATA_INDEX Offset;

	Offset = FieldEntry.DataOrDataOffset + DataOffset;

	if (Offset < FieldEntry.DataOrDataOffset)
		return false;

	return m_Reader->ValidateFieldDataRange( Offset, Length );
}

