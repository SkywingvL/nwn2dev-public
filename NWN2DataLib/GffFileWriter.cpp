/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	GffFileReader.cpp

Abstract:

	This module houses the *.gff file format writer, which is used to create
	Generic File Format (GFF) files.  These files may also have alternative
	extensions such as *.bic.

--*/

#include "Precomp.h"
#include "GffFileWriter.h"
#include "GffFileReader.h"
#include "ResourceManager.h"
#include "GffInternal.h"

GffFileWriter::GffFileWriter(
	)
/*++

Routine Description:

	This routine constructs a new GffFileWriter object and initializes the root
	structure.

Arguments:

	None.

Return Value:

	The newly constructed object.

Environment:

	User mode.

--*/
: m_Language( GffFileReader::LangEnglish ),
  m_FileType( GFF_FILE_TYPE )
{
	m_RootStruct = new FieldStruct;

	m_RootStruct->StructType = 0xFFFFFFFF;

	AddStruct( m_RootStruct );
}

GffFileWriter::~GffFileWriter(
	)
/*++

Routine Description:

	This routine cleans up an already-existing GffFileWriter object.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
}

bool
GffFileWriter::Commit(
	__in const std::string & FileName,
	__in unsigned long FileType, /* = 0 */
	__in unsigned long Flags /* = 0 */
	)
/*++

Routine Description:

	This routine writes the staged GFF contents to a disk file.

Arguments:

	FileName - Supplies the name of the file to write to.

	FileType - Supplies the type tag of the file (GFF, BIC, etc.)

	Flags - Supplies flags that control the behavior of the commit operation.
	        Legal values are drawn from the GFF_COMMIT_FLAG_* family of values.

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

		GffWriteContext Context;

		File = CreateFileA(
			FileName.c_str( ),
			GENERIC_WRITE | ((Flags & GFF_COMMIT_FLAG_SEQUENTIAL) ? GENERIC_READ : 0),
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (File == INVALID_HANDLE_VALUE)
			throw std::runtime_error( "Failed to open file." );

		Context.Type = GffWriteContext::ContextTypeFile;
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

bool
GffFileWriter::Commit(
	__out std::vector< unsigned char > & Memory,
	__in unsigned long FileType, /* = 0 */
	__in unsigned long Flags /* = 0 */
	)
/*++

Routine Description:

	This routine writes the staged GFF contents to an in-memory buffer.

Arguments:

	Memory - Supplies the buffer that receives the GFF contents.  The contents
	         of the buffer are replaced with the GFF contents.

	FileType - Supplies the type tag of the file (GFF, BIC, etc.)

	Flags - Supplies flags that control the behavior of the commit operation.
	        Legal values are drawn from the GFF_COMMIT_FLAG_* family of values.

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

		GffWriteContext Context;

		Context.Type   = GffWriteContext::ContextTypeMemory;
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

void
GffFileWriter::CommitInternal(
	__in GffWriteContext * Context,
	__in unsigned long FileType,
	__in unsigned long Flags
	)
/*++

Routine Description:

	This routine writes the staged GFF contents to a write context, which may
	represent a disk file or an in-memory buffer.

Arguments:

	Context - Supplies the write context that receives the contents of the
	          formatted GFF file.

	FileType - Supplies the type tag of the file (GFF, BIC, etc.)

	Flags - Supplies flags that control the behavior of the commit operation.
	        Legal values are drawn from the GFF_COMMIT_FLAG_* family of values.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	GFF_HEADER        Header;
	FieldStructPtrVec Structs;

	//
	// If the user did not supply an override file type, take the default one.
	//

	if (FileType == 0)
		FileType = m_FileType;

	m_RootStruct->StructType = 0xFFFFFFFF;

	//
	// First, generate and store the header.
	//

	BuildHeader( Header, FileType );

	Context->Write( &Header, sizeof( Header ) );

	//
	// First, we write the labels out.
	//

	Header.LabelOffset = sizeof( Header );

	WriteLabelEntries( Header, Context );

	//
	// Write the field data section out next.
	//

	if (Header.LabelOffset + Header.LabelCount * sizeof( GFF_LABEL_ENTRY ) < Header.LabelOffset)
		throw std::runtime_error( "GFF file is too large." );

	Header.FieldDataOffset = Header.LabelOffset + (Header.LabelCount * sizeof( GFF_LABEL_ENTRY ));

	WriteFieldData( Header, Context );

	//
	// Now write out the field data indicies, since we have assigned these as
	// a part of the field data write process.
	//

	Header.FieldIndiciesOffset = Header.FieldDataOffset + Header.FieldDataCount;

	if (Header.FieldDataOffset + Header.FieldDataCount < Header.FieldDataOffset)
		throw std::runtime_error( "GFF file is too large." );

	WriteFieldIndicies( Header, Context );

	//
	// Write structures.
	//

	if (Header.FieldIndiciesOffset + Header.FieldIndiciesCount < Header.FieldIndiciesOffset)
		throw std::runtime_error( "GFF file is too large." );

	Header.StructOffset = Header.FieldIndiciesOffset + Header.FieldIndiciesCount;

	WriteStructEntries( Header, Context );

	//
	// Write list indicies.
	//

	if (Header.StructOffset + (Header.StructCount * sizeof( GFF_STRUCT_ENTRY )) < Header.StructOffset)
		throw std::runtime_error( "GFF file is too large." );

	Header.ListIndiciesOffset = Header.StructOffset + (Header.StructCount * sizeof( GFF_STRUCT_ENTRY ));

	WriteListIndicies( Header, Context );

	//
	// Write the field descriptors out.
	//

	if (Header.ListIndiciesOffset + Header.ListIndiciesCount < Header.ListIndiciesOffset)
		throw std::runtime_error( "GFF file is too large." );

	Header.FieldOffset = Header.ListIndiciesOffset + Header.ListIndiciesCount;

	WriteFieldEntries( Header, Context );

	//
	// Finally, re-write the updated header.
	//

	Context->SeekOffset( 0, "Write Finalized Header" );
	Context->Write( &Header, sizeof( Header ) );

	if (Flags & GFF_COMMIT_FLAG_SEQUENTIAL)
	{
		GFF_HEADER                   NewHeader;
		std::vector< unsigned char > OrderedContents;

		//
		// If we must arrange the fields in sequential order, i.e. to work around
		// buggy GFF readers, then stitch up the final file here.
		// 

		OrderedContents.resize(
			Header.FieldOffset + (Header.FieldCount * sizeof( GFF_FIELD_ENTRY ) ) );

		//
		// Build the new header up now.
		//

		NewHeader = Header;
		NewHeader.StructOffset        = sizeof( NewHeader );
		NewHeader.FieldOffset         = NewHeader.StructOffset        + (NewHeader.StructCount * sizeof( GFF_STRUCT_ENTRY ));
		NewHeader.LabelOffset         = NewHeader.FieldOffset         + (NewHeader.FieldCount  * sizeof( GFF_FIELD_ENTRY  ));
		NewHeader.FieldDataOffset     = NewHeader.LabelOffset         + (NewHeader.LabelCount  * sizeof( GFF_LABEL_ENTRY  ));
		NewHeader.FieldIndiciesOffset = NewHeader.FieldDataOffset     + (NewHeader.FieldDataCount);
		NewHeader.ListIndiciesOffset  = NewHeader.FieldIndiciesOffset + (NewHeader.FieldIndiciesCount);

		memcpy( &OrderedContents[ 0 ], &NewHeader, sizeof( NewHeader ) );

		//
		// Now copy each section over from the previous write context.
		//

		Context->Read(
			&OrderedContents[ NewHeader.LabelOffset ],
			NewHeader.LabelCount * sizeof( GFF_LABEL_ENTRY ));
		Context->Read(
			&OrderedContents[ NewHeader.FieldDataOffset ],
			NewHeader.FieldDataCount);
		Context->Read(
			&OrderedContents[ NewHeader.FieldIndiciesOffset ],
			NewHeader.FieldIndiciesCount );
		Context->Read(
			&OrderedContents[ NewHeader.StructOffset ],
			NewHeader.StructCount * sizeof( GFF_STRUCT_ENTRY ));
		Context->Read(
			&OrderedContents[ NewHeader.ListIndiciesOffset ],
			NewHeader.ListIndiciesCount);
		Context->Read(
			&OrderedContents[ NewHeader.FieldOffset ],
			NewHeader.FieldCount * sizeof( GFF_FIELD_ENTRY ));

		//
		// Now transfer the sequentially ordered contents back over to the writer
		// context for persistence.
		//

		Context->SeekOffset( 0, "Write Sequentialized File Contents" );
		Context->Write( &OrderedContents[ 0 ], OrderedContents.size( ) );
	}

#if !GFFFILEWRITER_PRETRACK_STRUCTS
	//
	// If we are not pre-tracking structures, clear out the references that we
	// created on the fly for fast structure lookup.
	//

	m_Structs.clear( );
#endif
}

void
GffFileWriter::BuildHeader(
	__out GFF_HEADER & Header,
	__in unsigned long FileType
	)
/*++

Routine Description:

	This routine builds the file header for a GFF commit operation.

Arguments:

	Header - Receives the constructed file header.

	FileType - Supplies the type tag of the file (GFF, BIC, etc.)

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	Header.FileType            = FileType;
	memcpy( &Header.Version, GFF_VERSION_CURRENT, 4 );

	//
	// Now prepare the data section of the header.  The header is updated as we
	// go and then re-written at the end.
	//

	Header.StructOffset        = 0;
	Header.StructCount         = 0;
	Header.FieldOffset         = 0;
	Header.FieldCount          = 0;
	Header.LabelOffset         = 0;
	Header.LabelCount          = 0;
	Header.FieldDataOffset     = 0;
	Header.FieldDataCount      = 0;
	Header.FieldIndiciesOffset = 0;
	Header.FieldIndiciesCount  = 0;
	Header.ListIndiciesOffset  = 0;
	Header.ListIndiciesCount   = 0;
}

void
GffFileWriter::WriteLabelEntries(
	__inout GFF_HEADER & Header,
	__in GffWriteContext * Context
	)
/*++

Routine Description:

	This routine writes the contents of each label out to the writer context.

Arguments:

	Header - Receives the constructed file header.  The header is updated as
	         the write operation progresses.

	Context - Supplies the write context that receives the contents of the
	          formatted GFF file.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	STRUCT_INDEX  StructIndex;
	LabelIndexMap AssignedLabels;

#if !GFFFILEWRITER_PRETRACK_STRUCTS
	//
	// If we are not pre-tracking structures, clear out any lingering state
	// from a previous write attempt and then index each structure in the tree.
	//
	// Note that the index takes weak references as the data tree does not
	// change during the lifetime of the index (m_Structs).
	//

	m_Structs.clear( );

	AddStructRecursive( m_RootStruct.get( ) );
#endif

	//
	// Write the label of each field to disk.  Also, take the opportunity to
	// assign struct indicies now as the struct array is frozen for writing and
	// this is our first pass.
	//

	StructIndex = 0;

	for (FieldStructIdxVec::iterator it = m_Structs.begin( );
	     it != m_Structs.end( );
	     ++it)
	{
		(*it)->StructIndex = StructIndex++;

		for (FieldEntryVec::iterator fit = (*it)->StructFields.begin( );
		     fit != (*it)->StructFields.end( );
		     ++fit)
		{
			LabelIndexMap::iterator lit;

			//
			// If we have not already stored this label, assign a new label
			// index and write it.
			//

			lit = AssignedLabels.find( fit->FieldLabelEntry );

			if (lit == AssignedLabels.end( ))
			{
				Context->Write( fit->FieldLabel, sizeof( fit->FieldLabel ) );

				AssignedLabels.insert(
					LabelIndexMap::value_type(
						fit->FieldLabelEntry,
						Header.LabelCount
						)
					);

				fit->FieldLabelIndex = (LABEL_INDEX) Header.LabelCount;

				Header.LabelCount += 1;
			}
			else
			{
				fit->FieldLabelIndex = lit->second;
			}
		}
	}
}

void
GffFileWriter::WriteFieldData(
	__inout GFF_HEADER & Header,
	__in GffWriteContext * Context
	)
/*++

Routine Description:

	This routine writes the contents of each data field out.

Arguments:

	Header - Receives the constructed file header.  The header is updated as
	         the write operation progresses.

	Context - Supplies the write context that receives the contents of the
	          formatted GFF file.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	//
	// Write each data field's contents out.
	//

	for (FieldStructIdxVec::iterator it = m_Structs.begin( );
	     it != m_Structs.end( );
	     ++it)
	{
		for (FieldEntryVec::iterator fit = (*it)->StructFields.begin( );
		     fit != (*it)->StructFields.end( );
		     ++fit)
		{
			//
			// Skip non-data fields for now (such as structs and lists).  Also,
			// skip fields that are not stored as complex data as that data is
			// not written here.
			//

			if (!(fit->FieldFlags & FIELD_FLAG_HAS_DATA))
				continue;

			if (!(fit->FieldFlags & FIELD_FLAG_COMPLEX))
				continue;

			if (fit->FieldData.empty( ))
				continue;

			//
			// Transfer field contents to the GFF.
			//

			Context->Write( &fit->FieldData[ 0 ], fit->FieldData.size( ) );

			//
			// Now assign the field data index, which is the offset into the
			// field data section.  Also, update accounting in the header for
			// the new data record.
			//

			fit->FieldDataIndex   = Header.FieldDataCount;
			Header.FieldDataCount += (unsigned long) fit->FieldData.size( );
		}
	}
}

void
GffFileWriter::WriteFieldIndicies(
	__inout GFF_HEADER & Header,
	__in GffWriteContext * Context
	)
/*++

Routine Description:

	This routine assigns field indicies for each struct field and write the
	field indicies data out.

Arguments:

	Header - Receives the constructed file header.  The header is updated as
	         the write operation progresses.

	Context - Supplies the write context that receives the contents of the
	          formatted GFF file.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	GffFileReader::FIELD_INDEX FieldIndex;

	//
	// Write the field index of each complex data field out.
	//
	// N.B.  We can compute the field index on the fly as
	//

	FieldIndex = 0;

	for (FieldStructIdxVec::iterator it = m_Structs.begin( );
	     it != m_Structs.end( );
	     ++it)
	{
		//
		// Not all structures need field data indicies assigned.  If we have
		// no fields then there is nothing to write.  If we've got only one
		// field then the field offset for that field is stored inline.
		//

		switch ((*it)->StructFields.size( ))
		{

		case 0: // No data to write.
			continue;

		case 1: // Only one field, we store the index inline within the struct.
			(*it)->DataOrDataOffset = FieldIndex;
			FieldIndex += 1;
			continue;

		default: // Multiple fields, store the offset to the field indicies.
			(*it)->DataOrDataOffset = Header.FieldIndiciesCount;
			break;

		}

		//
		// This structure needs field indicies assigned, write them out now.
		//

		for (FieldEntryVec::iterator fit = (*it)->StructFields.begin( );
		     fit != (*it)->StructFields.end( );
		     ++fit)
		{
			Context->Write( &FieldIndex, sizeof( FieldIndex ) );
			Header.FieldIndiciesCount += sizeof( FieldIndex );
			FieldIndex += 1;
		}
	}
}

void
GffFileWriter::WriteStructEntries(
	__inout GFF_HEADER & Header,
	__in GffWriteContext * Context
	)
/*++

Routine Description:

	This routine writes the contents of each structure entry out to the writer
	context.

	Note that the DataOrDataOffset field of each struct must have been already
	computed by going through the field indicies write process.

Arguments:

	Header - Receives the constructed file header.  The header is updated as
	         the write operation progresses.

	Context - Supplies the write context that receives the contents of the
	          formatted GFF file.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	for (FieldStructIdxVec::iterator it = m_Structs.begin( );
	     it != m_Structs.end( );
	     ++it)
	{
		GFF_STRUCT_ENTRY StructEntry;

		StructEntry.Type             = (*it)->StructType;
		StructEntry.DataOrDataOffset = (*it)->DataOrDataOffset;
		StructEntry.FieldCount       = (unsigned long) (*it)->StructFields.size( );

		Context->Write( &StructEntry, sizeof( StructEntry ) );

		Header.StructCount += 1;
	}
}

void
GffFileWriter::WriteListIndicies(
	__inout GFF_HEADER & Header,
	__in GffWriteContext * Context
	)
/*++

Routine Description:

	This routine write the list indicies array out.  This array refers back to
	the structure array.

Arguments:

	Header - Receives the constructed file header.  The header is updated as
	         the write operation progresses.

	Context - Supplies the write context that receives the contents of the
	          formatted GFF file.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	//
	// Write each data field's contents out.
	//

	for (FieldStructIdxVec::iterator it = m_Structs.begin( );
	     it != m_Structs.end( );
	     ++it)
	{
		for (FieldEntryVec::iterator fit = (*it)->StructFields.begin( );
		     fit != (*it)->StructFields.end( );
		     ++fit)
		{
			LIST_INDICIES_INDEX Count;

			if (fit->FieldType != GffFileReader::GFF_LIST)
				continue;

			Count = (LIST_INDICIES_INDEX) fit->List.size( );

			//
			// Transfer list struct offsets into the GFF.
			//

			Context->Write( &Count, sizeof( Count ) );

			for (FieldStructPtrVec::iterator lit = fit->List.begin( );
			     lit != fit->List.end( );
			     ++lit)
			{
				Context->Write( &(*lit)->StructIndex, sizeof( (*it)->StructIndex ) );
			}

			//
			// Now assign the field data index, which is the offset into the
			// list indicies section for list types.
			//

			fit->FieldDataIndex       = Header.ListIndiciesCount;
			Header.ListIndiciesCount += (unsigned long) fit->List.size( ) * sizeof( STRUCT_INDEX ) + sizeof( Count );
		}
	}
}

void
GffFileWriter::WriteFieldEntries(
	__inout GFF_HEADER & Header,
	__in GffWriteContext * Context
	)
/*++

Routine Description:

	This routine writes the field entry array out.

Arguments:

	Header - Receives the constructed file header.  The header is updated as
	         the write operation progresses.

	Context - Supplies the write context that receives the contents of the
	          formatted GFF file.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	LABEL_INDEX LabelIndex;

	//
	// Write each field entry descriptor out.
	//

	LabelIndex = 0;

	for (FieldStructIdxVec::iterator it = m_Structs.begin( );
	     it != m_Structs.end( );
	     ++it)
	{
		for (FieldEntryVec::iterator fit = (*it)->StructFields.begin( );
		     fit != (*it)->StructFields.end( );
		     ++fit)
		{
			GFF_FIELD_ENTRY FieldEntry;

			//
			// If this is a structure field, the data index actually must point
			// into the struct array.
			//

			if (fit->FieldType == GffFileReader::GFF_STRUCT)
				fit->FieldDataIndex = (FIELD_DATA_INDEX) fit->Struct->StructIndex;

			FieldEntry.Type             = (unsigned long) fit->FieldType;
			FieldEntry.LabelIndex       = (unsigned long) fit->FieldLabelIndex;

			//
			// If this as a complex field, the DataOrDataOffset points into the
			// data section.  Otherwise, the data is stored directly in the
			// DataOrDataOffset field itself.
			//
			// Additionally, structural (non-data) fields already have a
			// special purpose offset assigned in DataOrDataOffset that we need
			// to write out as-is.
			//

			if ((fit->FieldFlags & (FIELD_FLAG_COMPLEX)) ||
			    (!(fit->FieldFlags & FIELD_FLAG_HAS_DATA)))
			{
				FieldEntry.DataOrDataOffset = (unsigned long) fit->FieldDataIndex;
			}
			else
			{
				FieldEntry.DataOrDataOffset = 0;

				if (!fit->FieldData.empty( ))
				{
					memcpy(
						&FieldEntry.DataOrDataOffset,
						&fit->FieldData[ 0 ],
						fit->FieldData.size( ));
				}
			}

			//
			// Transfer the field entry over.
			//

			Context->Write( &FieldEntry, sizeof( FieldEntry ) );

			Header.FieldCount += 1;
		}
	}
}


bool
GffFileWriter::IsComplexType(
	__in GFF_FIELD_TYPE FieldType
	)
/*++

Routine Description:

	This routine determines whether a field type is a complex type or a simple
	type.

	A simple type is stored inline in the DataOrDataOffset field.

Arguments:

	FieldType - Supplies the field type to inquire about.

Return Value:

	The routine returns true if the field type is a complex type.

Environment:

	User mode.

--*/
{
	switch (FieldType)
	{

	case GffFileReader::GFF_BYTE:
	case GffFileReader::GFF_CHAR:
		return false;

	case GffFileReader::GFF_WORD:
	case GffFileReader::GFF_SHORT:
		return false;

	case GffFileReader::GFF_DWORD:
	case GffFileReader::GFF_INT:
		return false;

	case GffFileReader::GFF_DWORD64:
	case GffFileReader::GFF_INT64:
		return true;

	case GffFileReader::GFF_FLOAT:
		return false;

	case GffFileReader::GFF_DOUBLE:
		return true;

	case GffFileReader::GFF_CEXOSTRING:
	case GffFileReader::GFF_RESREF:
	case GffFileReader::GFF_CEXOLOCSTRING:
	case GffFileReader::GFF_VOID:
	case GffFileReader::GFF_VECTOR:
		return true;

	case GffFileReader::GFF_STRUCT:
	case GffFileReader::GFF_LIST:
		return false; // No data attached.

	case GffFileReader::GFF_RESERVED:
	default:
		return false; // Not supported.

	}
}

void
GffFileWriter::GffStruct::InitializeFromStruct(
	__in const GffFileReader::GffStruct * Struct,
	__in size_t MaxDepth /* = 32 */
	)
/*++

Routine Description:

	This routine recursively copies the contents of an already-parsed GFF
	structure into the current structure contents.  Any existing contents of
	the current structure are discarded.

Arguments:

	Struct - Supplies the GFF (reader) structure to initialize the contents of
	         the GFF (writer) structure from.

	MaxDepth - Supplies the maximum recursion depth into child structures that
	           is permitted.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	GffFileReader::FIELD_INDEX    FieldCount;
	GffFileReader::GFF_FIELD_TYPE FieldType;
	std::string                   FieldLabel;
	size_t                        NameLen;
	bool                          Complex;

	//
	// Transfer data from each field in the source structure.
	//

	FieldCount = Struct->GetFieldCount( );

	for (GffFileReader::FIELD_INDEX FieldIndex = 0;
	     FieldIndex < FieldCount;
	     FieldIndex += 1)
	{
		FieldEntry * Field;

		Field = NULL;

		//
		// First, determine the field type so that we know whether this is a
		// data field or a structural field.
		//

		if (!Struct->GetFieldType( FieldIndex, FieldType ))
			throw std::runtime_error( "Failed to query field type." );

		switch (FieldType)
		{

		case GffFileReader::GFF_STRUCT:
			{
				GffFileReader::GffStruct FieldStruct;
				std::string              Label;

				//
				// This is a struct field, recursively copy its contents over.
				//

				if (MaxDepth == 0)
					throw std::runtime_error( "Exceeded maximum nested structure depth." );

				if (!Struct->GetStructByIndex( FieldIndex, FieldStruct ))
					throw std::runtime_error( "Failed to retrieve structure by index." );

				if (!Struct->GetFieldName( FieldIndex, Label ))
					throw std::runtime_error( "Failed to retrieve structure label." );

				CreateStruct(
					Label.c_str( ),
					FieldStruct.GetType( )).InitializeFromStruct(
						&FieldStruct,
						MaxDepth - 1);
			}
			break;

		case GffFileReader::GFF_LIST:
			{
				GffFileReader::GffStruct ListStruct;
				std::string              Label;

				//
				// This is a list field, recursively copy each of the struct
				// elements over.
				//

				if (MaxDepth == 0)
					throw std::runtime_error( "Exceeded maximum nested structure depth." );

				if (!Struct->GetFieldName( FieldIndex, Label ))
					throw std::runtime_error( "Failed to retrieve list label." );

				CreateList( Label.c_str( ) );

				for (size_t i = 0; i <= ULONG_MAX; i += 1)
				{
					if (!Struct->GetListElementByIndex(
						FieldIndex,
						i,
						ListStruct))
					{
						break;
					}

					AppendListElement(
						Label.c_str( ),
						ListStruct.GetType( )).InitializeFromStruct(
							&ListStruct,
							MaxDepth - 1);
				}
			}
			break;

		default:
			//
			// This is a data field, just move the raw data over without
			// interpreting the contents (except to the minimum degree that is
			// necessary to determine the length of the field data).
			//

			m_StructEntry->StructFields.push_back( FieldEntry( ) );

			Field             = &m_StructEntry->StructFields.back( );
			Field->FieldFlags = 0;

			if (Struct->GetFieldRawData(
				FieldIndex,
				Field->FieldData,
				FieldLabel,
				Field->FieldType,
				Complex))
			{
				Field->FieldFlags |= FIELD_FLAG_HAS_DATA;

				if (Complex)
					Field->FieldFlags |= FIELD_FLAG_COMPLEX;

				//
				// Now assign the field label.
				//

				NameLen = FieldLabel.size( );
				NameLen = min( NameLen, sizeof( Field->FieldLabel ) );

				ZeroMemory( Field->FieldLabel, sizeof( Field->FieldLabel ) );

				if (NameLen != 0)
					memcpy( Field->FieldLabel, FieldLabel.data( ), NameLen );
			}
			else
			{
				m_StructEntry->StructFields.pop_back( );

				throw std::runtime_error( "Failed to retrieve field raw data." );
			}

			break;
		}

		//
		// On to the next field.
		//
	}

	//
	// All done.
	//
}

void
GffFileWriter::GffStruct::CopyField(
	__in const GffFileReader::GffStruct * Struct,
	__in const char * FieldName,
	__in size_t MaxDepth /* = 32 */
	)
/*++

Routine Description:

	This routine copies the contents of a field from a GffFileReader structure
	into the current writer structure.  The field must not already exist in the
	writer (the caller assumes responsibility for ensuring this).

	If the field was a Struct or a List, then a recursive transfer to a new
	child Struct or List field is performed (a-la InitializeFromStruct).

	If the field was a data field, the raw data is copied.

Arguments:

	Struct - Supplies the GFF (reader) structure to initialize the contents of
	         the GFF (writer) structure from.

	FieldName - Supplies the name of the field to copy from the GFF (reader)
	            structure.

	MaxDepth - Supplies the maximum recursion depth into child structures that
	           is permitted.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	GffFileReader::FIELD_INDEX FieldIndex;

	//
	// Map to a field index and perform the rest of the operations on the index
	// itself rather than going to a name search each time.
	//

	if (!Struct->GetFieldIndex( FieldName, FieldIndex ))
		throw std::runtime_error( "Failed to locate source field." );

	return CopyField( Struct, FieldIndex, MaxDepth );
}

void
GffFileWriter::GffStruct::CopyField(
	__in const GffFileReader::GffStruct * Struct,
	__in GffFileReader::FIELD_INDEX FieldIndex,
	__in size_t MaxDepth /* = 32 */
	)
/*++

Routine Description:

	This routine copies the contents of a field from a GffFileReader structure
	into the current writer structure.  The field must not already exist in the
	writer (the caller assumes responsibility for ensuring this).

	If the field was a Struct or a List, then a recursive transfer to a new
	child Struct or List field is performed (a-la InitializeFromStruct).

	If the field was a data field, the raw data is copied.

Arguments:

	Struct - Supplies the GFF (reader) structure to initialize the contents of
	         the GFF (writer) structure from.

	FieldIndex - Supplies the index of the field to copy from the GFF (reader)
	             structure.

	MaxDepth - Supplies the maximum recursion depth into child structures that
	           is permitted.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	GffFileReader::GFF_FIELD_TYPE   FieldType;
	std::string                     FieldLabel;
	size_t                          NameLen;
	bool                            Complex;
	FieldEntry                    * Field;

	Field = NULL;

	//
	// Determine the type of field we're dealing with first.
	//

	if (!Struct->GetFieldType( FieldIndex, FieldType ))
		throw std::runtime_error( "Failed to query field type." );

	//
	// Now perform the copy.  If it's a data field, we just move the raw field
	// data over, otherwise we create a new Struct or List descriptor and move
	// the data over as appropriate.
	//
	// N.B.  This should be the same copy logic as in InitializeFromStruct !
	//

	switch (FieldType)
	{

	case GffFileReader::GFF_STRUCT:
		{
			GffFileReader::GffStruct FieldStruct;
			std::string              Label;

			//
			// This is a struct field, recursively copy its contents over.
			//

			if (MaxDepth == 0)
				throw std::runtime_error( "Exceeded maximum nested structure depth." );

			if (!Struct->GetStructByIndex( FieldIndex, FieldStruct ))
				throw std::runtime_error( "Failed to retrieve structure by index." );

			if (!Struct->GetFieldName( FieldIndex, Label ))
				throw std::runtime_error( "Failed to retrieve structure label." );

			CreateStruct(
				Label.c_str( ),
				FieldStruct.GetType( )).InitializeFromStruct(
					&FieldStruct,
					MaxDepth - 1);
		}
		break;

	case GffFileReader::GFF_LIST:
		{
			GffFileReader::GffStruct ListStruct;
			std::string              Label;

			//
			// This is a list field, recursively copy each of the struct
			// elements over.
			//

			if (MaxDepth == 0)
				throw std::runtime_error( "Exceeded maximum nested structure depth." );

			if (!Struct->GetFieldName( FieldIndex, Label ))
				throw std::runtime_error( "Failed to retrieve list label." );

			CreateList( Label.c_str( ) );

			for (size_t i = 0; i <= ULONG_MAX; i += 1)
			{
				if (!Struct->GetListElementByIndex(
					FieldIndex,
					i,
					ListStruct))
				{
					break;
				}

				AppendListElement(
					Label.c_str( ),
					ListStruct.GetType( )).InitializeFromStruct(
						&ListStruct,
						MaxDepth - 1);
			}
		}
		break;

	default:
		{
			//
			// This is a data field, just move the raw data over without
			// interpreting the contents (except to the minimum degree that is
			// necessary to determine the length of the field data).
			//

			m_StructEntry->StructFields.push_back( FieldEntry( ) );

			Field             = &m_StructEntry->StructFields.back( );
			Field->FieldFlags = 0;

			if (Struct->GetFieldRawData(
				FieldIndex,
				Field->FieldData,
				FieldLabel,
				Field->FieldType,
				Complex))
			{
				Field->FieldFlags |= FIELD_FLAG_HAS_DATA;

				if (Complex)
					Field->FieldFlags |= FIELD_FLAG_COMPLEX;

				//
				// Now assign the field label.
				//

				NameLen = FieldLabel.size( );
				NameLen = min( NameLen, sizeof( Field->FieldLabel ) );

				ZeroMemory( Field->FieldLabel, sizeof( Field->FieldLabel ) );

				if (NameLen != 0)
					memcpy( Field->FieldLabel, FieldLabel.data( ), NameLen );
			}
			else
			{
				m_StructEntry->StructFields.pop_back( );

				throw std::runtime_error( "Failed to retrieve field raw data." );
			}

			break;

		}
		break;

	}

	//
	// All done.
	//
}

void
GffFileWriter::GffStruct::InitializeFromStruct(
	__in GffFileWriter::GffStruct & Struct,
	__in size_t MaxDepth /* = 32 */
	)
/*++

Routine Description:

	This routine recursively copies the contents of an already-parsed GFF
	structure into the current structure contents.  Any existing contents of
	the current structure are discarded.

Arguments:

	Struct - Supplies the GFF (writer) structure to initialize the contents of
	         the GFF (writer) structure from.

	MaxDepth - Supplies the maximum recursion depth into child structures that
	           is permitted.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	FieldStruct * StructEntry;

	//
	// Transfer data from each field in the source structure.
	//

	StructEntry = Struct.GetStructEntry( );

	for (FieldEntryVec::iterator it = StructEntry->StructFields.begin( );
	     it != StructEntry->StructFields.end( );
	     ++it)
	{
		FieldEntry Entry = *it;

		Entry.Struct = NULL;
		Entry.List.clear( );

		//
		// Most fields can be directly copied, except for structural fields,
		// which need to be deep copied (so that we don't edit the source
		// structure).  Structs deep copied in this fashion must also be
		// registered with the flat list.
		//

		switch (it->FieldType)
		{

		case GffFileReader::GFF_STRUCT:
			{
				//
				// This is a struct field, recursively copy its contents over.
				//

				if (MaxDepth == 0)
					throw std::runtime_error( "Exceeded maximum nested structure depth." );

				Entry.Struct = new FieldStruct;

				Entry.Struct->StructType = it->Struct->StructType;

				GffStruct LocalStruct( m_Writer, Entry.Struct );
				GffStruct RemoteStruct( Struct.m_Writer, it->Struct );
				
				LocalStruct.InitializeFromStruct( RemoteStruct, MaxDepth - 1 );

				m_StructEntry->StructFields.push_back( Entry );
				m_Writer->AddStruct( Entry.Struct );
			}
			break;

		case GffFileReader::GFF_LIST:
			{
				//
				// This is a list field, recursively copy each of the struct
				// elements over.
				//

				if (MaxDepth == 0)
					throw std::runtime_error( "Exceeded maximum nested structure depth." );

				try
				{
					for (FieldStructPtrVec::iterator lit = it->List.begin( );
						 lit != it->List.end( );
						 ++lit)
					{
						FieldStructPtr Element = new FieldStruct;

						Element->StructType = (*lit)->StructType;

						GffStruct LocalStruct( m_Writer, Element );
						GffStruct RemoteStruct( Struct.m_Writer, (*lit) );

						LocalStruct.InitializeFromStruct( RemoteStruct, MaxDepth - 1 );

						Entry.List.push_back( Element );
						m_Writer->AddStruct( Element );
					}

					m_StructEntry->StructFields.push_back( Entry );
				}
				catch (...)
				{
					for (FieldStructPtrVec::iterator lit = Entry.List.begin( );
						 lit != Entry.List.end( );
						 ++lit)
					{
						m_Writer->DeleteStruct( (*lit) );
					}

					throw;
				}
			}
			break;

		default:
			m_StructEntry->StructFields.push_back( Entry );
			break;

		}

		//
		// On to the next field.
		//
	}


	//
	// All done.
	//
}
