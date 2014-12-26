/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	GffFileWriter.h

Abstract:

	This module defines the interface to the Generic File Format (GFF) file
	writer.  GFF files contain extensible structures of many data types,
	particularly those emitted by the area creation toolset.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_GFFFILEWRITER_H
#define _PROGRAMS_NWN2DATALIB_GFFFILEWRITER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "GffFileReader.h"

//
// Define to track structs as they are inserted, versus at write time.
//
// Zero enables on-write tracking, while one eanbles on-insertion tracking.
//
// On-insertion tracking is optimized for repeated writes of the same data
// without rebuilding the GffFileWriter.  Editing that involves deletion of
// structures suffers a significant performance penalty in this mode.
//
// On-write tracking is optimized for editing of data using a new writer object
// that is created each time.  Editing that involves deletion of structures is
// fast in this mode.
//

#define GFFFILEWRITER_PRETRACK_STRUCTS 0

//
// Define the GFF file writer object, used to create new GFF files.
//
// The GFF writer operates in a two-phase manner:
//
// - First, the user prepares the contents of the to-be-written GFF, by using
//   the class APIs.  This may involve duplication from an already-existing GFF
//   reader object.
//
// - Finally, the user commits the GFF to disk (or memory).
//

class GffFileWriter
{

	struct FieldStruct;
	typedef swutil::SharedPtr< FieldStruct > FieldStructPtr;
	typedef std::vector< FieldStructPtr > FieldStructPtrVec;
	typedef std::vector< FieldStruct * > FieldStructPVec;

#if GFFILEWRITER_PRETRACK_STRUCTS
	typedef FieldStructPtrVec FieldStructIdxVec;
#else
	typedef FieldStructPVec FieldStructIdxVec;
#endif

	typedef GffFileReader::GFF_FIELD_TYPE GFF_FIELD_TYPE;
	typedef std::vector< unsigned char > FieldDataVec;

	//
	// Define field flags.
	//

	enum FIELD_FLAGS
	{
		//
		// This field is a complex field and the data must not be stored in
		// the DataOrDataOffset entry.  If this flag is not set, then the field
		// is a simple field that is stored inline in the DataOrDataOffset
		// entry.
		//

		FIELD_FLAG_COMPLEX  = 0x00000001,

		//
		// This field stores data (i.e. it is not a struct/list).
		//

		FIELD_FLAG_HAS_DATA = 0x00000002
	};

	typedef GffFileReader::FIELD_DATA_INDEX FIELD_DATA_INDEX;
	typedef GffFileReader::LABEL_INDEX LABEL_INDEX;
	typedef GffFileReader::GFF_LABEL_ENTRY GFF_LABEL_ENTRY;

	//
	// Define a prepared field entry.
	//

	struct FieldEntry
	{
		inline
		FieldEntry(
			)
		: FieldDataIndex( 0 ),
		  FieldLabelIndex( 0 )
		{
		}

		//
		// Define the field type.
		//

		GFF_FIELD_TYPE    FieldType;

		//
		// Define field flags, drawn from the FIELD_FLAGS enumeration.
		//

		unsigned long     FieldFlags;

		//
		// Define the field label (max 16 characters).
		//

		union
		{
		    char            FieldLabel[ 16 ];
		    GFF_LABEL_ENTRY FieldLabelEntry;
		};

		//
		// Define the label index, which is assigned at write time.
		//

		LABEL_INDEX       FieldLabelIndex;

		//
		// Define the field data.  Regardless of whether this is a complex or
		// simple field, all of the data is held here.
		//
		// N.B.  Only data members are stored here.  Struct and list members
		//       are stored in their respective fields.
		//

		FieldDataVec      FieldData;

		//
		// Define the field data offset, which is assigned at write time.
		//

		FIELD_DATA_INDEX  FieldDataIndex;

		//
		// Define the structure pointer for a child structure.  Only entries of
		// type GFF_STRUCT use this field.
		//

		FieldStructPtr    Struct;

		//
		// Define the structure pointer array for a list structure.  Only
		// entries of type GFF_LIST use this field.
		//

		FieldStructPtrVec List;
	};

	//
	// N.B.  GFF_LABEL_ENTRY must match the size of FieldLabel above.
	//

	C_ASSERT( sizeof( GffFileReader::GFF_LABEL_ENTRY ) == 16 );

	typedef std::vector< FieldEntry > FieldEntryVec;

	typedef GffFileReader::STRUCT_INDEX STRUCT_INDEX;
	typedef GffFileReader::LIST_INDICIES_INDEX LIST_INDICIES_INDEX;
	typedef GffFileReader::LABEL_INDEX LABEL_INDEX;

	//
	// Define a prepared structure entry.
	//

	struct FieldStruct
	{
		inline
		FieldStruct(
			)
		: StructType( 0 ),
		  DataOrDataOffset( 0 ),
		  StructIndex( 0 )
		{
		}

		//
		// Define the type code of the structure.  The type code has a user-
		// defined meaning, except for the root structure, which must have type
		// code 0xFFFFFFFF.
		//

		unsigned long  StructType;

		//
		// Define the data index value.  If the field count is one, then this
		// is the raw FIELD_INDEX of the first field, otherwise it is the
		// FIELD_INDICIES_INDEX (offset) into the field indicies array.  Note
		// that this value is only meaningful during the commit process.
		//

		unsigned long  DataOrDataOffset;

		//
		// Define the struct index, which is only meaningful during the commit
		// process.  This is the index into the struct array on disk.
		//

		STRUCT_INDEX   StructIndex;

		//
		// Define the named fields of the structure.
		//

		FieldEntryVec  StructFields;
	};


public:

	typedef swutil::SharedPtr< GffFileWriter > Ptr;

	//
	// Constructor.  Raises an std::exception on failure.
	//

	GffFileWriter(
		);

	//
	// Destructor.
	//

	~GffFileWriter(
		);

	//
	// Define common GFF-derived file types.
	//

	enum
	{
		GFF_FILE_TYPE = ' FFG',
		BIC_FILE_TYPE = ' CIB',
		DLG_FILE_TYPE = ' GLD',
		UTC_FILE_TYPE = ' CTU',
		UTI_FILE_TYPE = ' ITU',
		ARE_FILE_TYPE = ' ERA',
		GIT_FILE_TYPE = ' TIG',

		LAST_FILE_TYPE
	};

	//
	// Define GFF writer flags that control the behavior of the write operation.
	//

	enum
	{
		//
		// Arrange the sections of the resultant GFF file in sequential order
		// with respect to the header.  That is, the following ordering is
		// enforced:
		//
		// - Structs, Fields, Labels, Field Data, Field Indicies, List Indicies
		//
		// Some buggy GFF readers, such as the NWN2 Toolset, require this data
		// ordering.  The core NWN/NWN2 game client and server themselves do not.
		//
		// Choosing this option imposes a performance and memory overhead, and is
		// thus not recommended if the GFF is only to be consumed by the game
		// core itself.
		//
	
		GFF_COMMIT_FLAG_SEQUENTIAL = 0x00000001,

		LAST_GFF_COMMIT_FLAG
	};

	//
	// Commit the contents of the GFF to disk.
	//

	bool
	Commit(
		__in const std::string & FileName,
		__in unsigned long FileType = 0,
		__in unsigned long Flags = 0
		);

	//
	// Commit the contents of the GFF to memory.
	//

	bool
	Commit(
		__out std::vector< unsigned char > & Memory,
		__in unsigned long FileType = 0,
		__in unsigned long Flags = 0
		);

	typedef GffFileReader::GFF_LANGUAGE GFF_LANGUAGE;

	//
	// Set the default localization language.
	//

	inline
	void
	SetDefaultLanguage(
		__in GFF_LANGUAGE Language
		)
	{
		m_Language = Language;
	}

	inline
	GFF_LANGUAGE
	GetDefaultLanguage(
		) const
	{
		return m_Language;
	}

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

	friend class GffStruct;

	//
	// Define the GFF structure object, used to access structures in a GFF
	// file.  The GffStruct represents the primary mechanism for navigating the
	// GFF hierarchy.
	//
	// Note that write operations, in general, are assumed to raise an
	// std::exception on failure (such as out of memory).
	//

	class GffStruct
	{

	public:

		inline
		GffStruct(
			)
		: m_Writer( NULL ),
		  m_StructEntry( NULL )
		{
		}

		inline
		GffStruct(
			__in GffFileWriter * Writer,
			__in FieldStructPtr StructEntry
			)
		: m_Writer( Writer ),
		  m_StructEntry( StructEntry )
		{
		}

		inline
		~GffStruct(
			)
		{
		}

		//
		// Return the underlying writer object.
		//

		inline
		GffFileWriter *
		GetWriter(
			)
		{
			return m_Writer;
		}

		//
		// Set the type of this structure.
		//

		inline
		void
		SetType(
			__in unsigned long Type
			)
		{
			m_StructEntry->StructType = Type;
		}

		//
		// Return the count of fields attached to the structure.
		//

		inline
		size_t
		GetFieldCount(
			) const
		{
			return m_StructEntry->StructFields.size( );
		}

		//
		// Initialize the structure (and descendant data) from an input GFF
		// structure.  The structure's current contents are cleared before the
		// data is transferred over.
		//
		// No validation of the contents of data fields transferred is
		// performed (and thus errors in the source GFF may propagate).
		//

		void
		InitializeFromStruct(
			__in const GffFileReader::GffStruct * Struct,
			__in size_t MaxDepth = 32
			);

		//
		// Initialize the structure (and descendant data) from an input GFF
		// structure.  The structure's current contents are cleared before the
		// data is transferred over.
		//
		// No validation of the contents of data fields transferred is
		// performed (and thus errors in the source GFF may propagate).
		//

		void
		InitializeFromStruct(
			__in GffFileWriter::GffStruct & Struct,
			__in size_t MaxDepth = 32
			);

		//
		// Remove a field from the structure.  Note that unlike most other
		// routines, DeleteField does NOT raise an exception if the field did
		// not exist.
		//

		inline
		void
		DeleteField(
			__in const char * Name
			)
		{
			FieldEntryVec::iterator it = GetFieldByNameIt( Name );

			if (it != m_StructEntry->StructFields.end( ))
			{
				//
				// If we were removing a struct or list, then we need to clear
				// out the flat tree representation links for those data nodes.
				//

				if (it->FieldType == GffFileReader::GFF_STRUCT)
					m_Writer->DeleteStruct( it->Struct );
				else if (it->FieldType == GffFileReader::GFF_LIST)
				{
					for (FieldStructPtrVec::iterator lit = it->List.begin( );
					     lit != it->List.end( );
					     ++lit)
					{
						m_Writer->DeleteStruct( *lit );
					}
				}

				m_StructEntry->StructFields.erase( it );
			}
		}

		//
		// Query the type of an already existing field.  Note that unlike most
		// other routines, GetFieldType does NOT raise an exception if the
		// field did not exist.
		//

		inline
		bool
		GetFieldType(
			__in const char * Name,
			__out GffFileReader::GFF_FIELD_TYPE & FieldType
			)
		{
			FieldEntryVec::iterator it = GetFieldByNameIt( Name );

			if (it == m_StructEntry->StructFields.end( ))
				return false;

			FieldType = it->FieldType;

			return true;
		}

		//
		// Data field primitive accessors.  These routines store data into a
		// GFF structure.  The data type is required to exactly match for the
		// routine to succeed.  All data type accessor routines throw an
		// std::exception on failure.
		//

		inline
		void
		SetBYTE(
			__in const char * FieldName,
			__in unsigned __int8 Data
			)
		{
			SetSmallFieldByName( GffFileReader::GFF_BYTE, FieldName, Data );
		}

		inline
		void
		SetBoolAsBYTE(
			__in const char * FieldName,
			__in bool Data
			)
		{
			unsigned __int8 b;

			b = Data ? 1 : 0;

			SetBYTE( FieldName, b );
		}

		inline
		void
		SetCHAR(
			__in const char * FieldName,
			__in signed __int8 Data
			)
		{
			SetSmallFieldByName( GffFileReader::GFF_CHAR, FieldName, Data );
		}

		inline
		void
		SetWORD(
			__in const char * FieldName,
			__in unsigned __int16 Data
			)
		{
			SetSmallFieldByName( GffFileReader::GFF_WORD, FieldName, Data );
		}

		inline
		void
		SetSHORT(
			__in const char * FieldName,
			__in signed __int16 Data
			)
		{
			SetSmallFieldByName( GffFileReader::GFF_SHORT, FieldName, Data );
		}

		inline
		void
		SetDWORD(
			__in const char * FieldName,
			__in unsigned __int32 Data
			)
		{
			SetSmallFieldByName( GffFileReader::GFF_DWORD, FieldName, Data );
		}

		inline
		void
		SetINT(
			__in const char * FieldName,
			__in signed __int32 Data
			)
		{
			SetSmallFieldByName( GffFileReader::GFF_INT, FieldName, Data );
		}

		inline
		void
		SetBoolAsINT(
			__in const char * FieldName,
			__in bool Data
			)
		{
			signed __int32 i;

			i = Data ? 1 : 0;

			SetINT( FieldName, i );
		}

		inline
		void
		SetDWORD64(
			__in const char * FieldName,
			__in unsigned __int64 Data
			)
		{
			SetLargeFieldByName( GffFileReader::GFF_DWORD64, FieldName, Data );
		}

		inline
		void
		SetINT64(
			__in const char * FieldName,
			__in signed __int64 Data
			)
		{
			SetLargeFieldByName( GffFileReader::GFF_INT64, FieldName, Data );
		}

		inline
		void
		SetFLOAT(
			__in const char * FieldName,
			__in float Data
			)
		{
			SetSmallFieldByName( GffFileReader::GFF_FLOAT, FieldName, Data );
		}

		inline
		void
		SetDOUBLE(
			__in const char * FieldName,
			__in double Data
			)
		{
			SetLargeFieldByName( GffFileReader::GFF_DOUBLE, FieldName, Data );
		}

		inline
		void
		SetCExoString(
			__in const char * FieldName,
			__in const std::string & Data
			)
		{
			std::vector< unsigned char > RawData;
			unsigned long                Size;

			Size = (unsigned long) Data.size( );

			if (4 + Size < Size)
				throw std::runtime_error( "Length overflow." );

			RawData.resize( 4 + Size );

			memcpy( &RawData[ 0 ], &Size, 4 );

			if (!Data.empty( ))
				memcpy( &RawData[ 4 ], Data.data( ), Size );

			SetComplexFieldByName( GffFileReader::GFF_CEXOSTRING, FieldName, RawData );
		}

		inline
		void
		SetResRef(
			__in const char * FieldName,
			__in const NWN::ResRef32 & Data
			)
		{
			std::vector< unsigned char > RawData;
			unsigned char                Size;

			for (Size = 0; Size < sizeof( Data.RefStr ); Size += 1)
			{
				if (Data.RefStr[ Size ] == '\0')
					break;
			}

			if (1 + Size < Size)
				throw std::runtime_error( "Length overflow." );

			RawData.resize( 1 + Size );

			memcpy( &RawData[ 0 ], &Size, 1 );

			if (Size != 0)
				memcpy( &RawData[ 1 ], &Data, Size );

			SetComplexFieldByName( GffFileReader::GFF_RESREF, FieldName, RawData );
		}

		inline
		void
		SetResRef(
			__in const char * FieldName,
			__in const std::string & ResRef
			)
		{
			NWN::ResRef32 RealResRef;
			size_t        i;

			ZeroMemory( &RealResRef, sizeof( RealResRef ) );

			i = 0;

			for (std::string::const_iterator it = ResRef.begin( );
			     it != ResRef.end( );
			     ++it)
			{
				RealResRef.RefStr[ i ] = (char) tolower( (int) (unsigned char) *it );

				i += 1;

				if (i == sizeof( RealResRef ))
					break;
			}

			SetResRef( FieldName, RealResRef );
		}


		typedef GffFileReader::GFF_CEXOLOCSTRING_ENTRY GFF_CEXOLOCSTRING_ENTRY;
		typedef GffFileReader::GFF_CEXOLOCSUBSTRING_ENTRY GFF_CEXOLOCSUBSTRING_ENTRY;

		//
		// Note that this SetCExoLocString override creates a single localized
		// string, gender male, with the writer's default language.
		//

		inline
		void
		SetCExoLocString(
			__in const char * FieldName,
			__in const std::string & Data
			)
		{
			std::vector< unsigned char > RawData;
			unsigned long                Size;
			GFF_CEXOLOCSTRING_ENTRY      LocStr;
			GFF_CEXOLOCSUBSTRING_ENTRY   LocSubStr;
			const size_t                 HeaderSize = sizeof( LocStr ) + sizeof( LocSubStr );

			C_ASSERT( HeaderSize == 20 );

			Size = (unsigned long) Data.size( );

			if (HeaderSize + Size < Size)
				throw std::runtime_error( "Length overflow." );

			RawData.resize( HeaderSize + Size );

			LocStr.Length      = (HeaderSize + Size) - 4;
			LocStr.StringRef   = 0xFFFFFFFF;
			LocStr.StringCount = 1;

			LocSubStr.StringID     = ((unsigned long) m_Writer->GetDefaultLanguage( ) << 1 ) | 0x1; // Gender: Male
			LocSubStr.StringLength = Size;

			memcpy( &RawData[ 0 ], &LocStr, sizeof( LocStr ) );
			memcpy( &RawData[ sizeof( LocStr ) ], &LocSubStr, sizeof( LocSubStr ) );

			if (!Data.empty( ))
				memcpy( &RawData[ HeaderSize ], Data.data( ), Size );

			SetComplexFieldByName( GffFileReader::GFF_CEXOLOCSTRING, FieldName, RawData );
		}

		inline
		void
		SetVOID(
			__in const char * FieldName,
			__in const std::vector< unsigned char > & Data
			)
		{
			std::vector< unsigned char > RawData;
			unsigned long                Size;

			Size = (unsigned long) Data.size( );

			if (4 + Size < Size)
				throw std::runtime_error( "Length overflow." );

			RawData.resize( 4 + Size );

			memcpy( &RawData[ 0 ], &Size, 4 );

			if (!Data.empty( ))
				memcpy( &RawData[ 4 ], &Data[ 0 ], Size );

			SetComplexFieldByName( GffFileReader::GFF_VOID, FieldName, RawData );
		}

		inline
		GffStruct
		CreateStruct(
			__in_opt const char * FieldName,
			__in unsigned long StructType = 0
			)
		{
			//
			// Supplying a FieldName of NULL allows one to refer to the current
			// structure itself, which is useful for compound accessors.
			//

			if (FieldName == NULL)
				return GffStruct( m_Writer, m_StructEntry );

			bool                    NewField;
			FieldEntryVec::iterator it = CreateField( GffFileReader::GFF_STRUCT, FieldName, NewField );

			try
			{
				if (NewField)
				{
					it->Struct = new FieldStruct;
					it->Struct->StructType = StructType;

					m_Writer->AddStruct( it->Struct );
				}

				//
				// Return a new struct context for the struct field being
				// created.
				//

				return GffStruct( m_Writer, it->Struct );
			}
			catch (...)
			{
				//
				// If we had not finished initializing a new field, then remove
				// it entirely.
				//

				if (NewField)
					m_StructEntry->StructFields.erase( it );

				throw;
			}
		}

		//
		// Return an existing structure.

		//
		// Create a new list.  Note that calling this routine is optional as
		// the list is created on the first reference.  Only if the program
		// that reads the GFF cannot tolerate the lack of the list's existance,
		// even if the list was empty, would this routine be required.
		//

		inline
		void
		CreateList(
			__in const char * FieldName
			)
		{
			bool NewField;

			CreateField( GffFileReader::GFF_LIST, FieldName, NewField );
		}

		//
		// Append an entry to a list.  The list is created if it did not
		// already exist.
		//

		inline
		GffStruct
		AppendListElement(
			__in const char * FieldName,
			__in unsigned long StructType = 0
			)
		{
			bool                    NewField;
			FieldEntryVec::iterator it = CreateField( GffFileReader::GFF_LIST, FieldName, NewField );

			//
			// Append the new struct to the list field, and return a new struct
			// context for the struct being appended.
			//

			try
			{
				FieldStructPtr Struct = new FieldStruct;
				Struct->StructType = StructType;

				it->List.push_back( Struct );
				m_Writer->AddStruct( Struct );

				return GffStruct( m_Writer, Struct );
			}
			catch (...)
			{
				//
				// If we had not finished initializing a new field, then remove
				// it entirely.
				//

				if (NewField)
					m_StructEntry->StructFields.erase( it );

				throw;
			}
		}

		//
		// Add an entry to a list at a given index.  The list is created if it
		// did not already exist.  Any elements after the given position are
		// moved to a higher position number.
		//

		inline
		GffStruct
		AddListElement(
			__in const char * FieldName,
			__in size_t Index,
			__in unsigned long StructType = 0
			)
		{
			bool                    NewField;
			FieldEntryVec::iterator it = CreateField( GffFileReader::GFF_LIST, FieldName, NewField );

			//
			// Append the new struct to the list field, and return a new struct
			// context for the struct being appended.
			//

			try
			{
				FieldStructPtr Struct = new FieldStruct;
				Struct->StructType = StructType;

				if (Index >= it->List.size( ))
					it->List.push_back( Struct );
				else
					it->List.insert( it->List.begin( ) + Index, Struct );

				m_Writer->AddStruct( Struct );

				return GffStruct( m_Writer, Struct );
			}
			catch (...)
			{
				//
				// If we had not finished initializing a new field, then remove
				// it entirely.
				//

				if (NewField)
					m_StructEntry->StructFields.erase( it );

				throw;
			}
		}

		//
		// Return an already-existing list element for editing.
		//
		// N.B.  Unlike most other routines, the API returns false on failure instead
		//       of raising an std::exception.
		//

		inline
		bool
		GetListElement(
			__in const char * FieldName,
			__in size_t Index,
			__out GffStruct & Struct
			)
		{
			FieldEntry * ListField;

			ListField = GetFieldByName( FieldName );

			if (ListField == NULL)
				return false;

			if (Index >= ListField->List.size( ))
				return false;

			try
			{
				Struct = GffStruct( m_Writer, ListField->List[ Index ] );
			}
			catch (std::exception)
			{
				return false;
			}

			return true;
		}

		//
		// Delete a specific list element without removing the rest of the list
		// contents.  Note that unlike most other routine, DeleteListElement does
		// NOT raise an exception if the field did not exist.
		//

		inline
		void
		DeleteListElement(
			__in const char * Name,
			__in size_t Index
			)
		{
			FieldEntryVec::iterator it = GetFieldByNameIt( Name );

			if (it != m_StructEntry->StructFields.end( ))
			{
				if (it->FieldType != GffFileReader::GFF_LIST)
					return;

				if (Index >= it->List.size( ))
					return;

				FieldStructPtrVec::iterator lit = it->List.begin( ) + Index;

				m_Writer->DeleteStruct( *lit );
				it->List.erase( lit );
			}
		}

		//
		// Transfer a field from a GffFileReader over to the GffFileWriter.  The
		// field should not already exist (the caller must invoke DeleteField if
		// they have not already arranged for this).
		//
		// A new field with the given name is created and then initialized from
		// the source GffFileReader.  If the field was a data field, the raw
		// data is copied over.  If the field was a structural field, then the
		// contents are copied recursively as with InitializeFromStruct.
		//

		void
		CopyField(
			__in const GffFileReader::GffStruct * Struct,
			__in const char * FieldName,
			__in size_t MaxDepth = 32
			);

		void
		CopyField(
			__in const GffFileReader::GffStruct * Struct,
			__in GffFileReader::FIELD_INDEX FieldIndex,
			__in size_t MaxDepth = 32
			);

		//
		// N.B.  Most Vectors are packed as a struct with "x", "y", "z" values.
		//

		inline
		void
		SetVector3_DEPRECATED(
			__in const char * FieldName,
			__in const NWN::Vector3 & v
			)
		{
			SetLargeFieldByName( GffFileReader::GFF_VECTOR, FieldName, v );
		}

		//
		// Simple compound structure accessor helpers.
		//

		inline
		void
		SetVector(
			__in_opt const char * FieldName,
			__in const NWN::Vector3 & v,
			__in unsigned long StructType = 0
			)
		{
			GffStruct s = CreateStruct( FieldName, StructType );

			s.SetFLOAT( "x", v.x );
			s.SetFLOAT( "y", v.y );
			s.SetFLOAT( "z", v.z );
		}

		inline
		void
		SetQuaternion(
			__in_opt const char * FieldName,
			__in const NWN::Quaternion & q,
			__in unsigned long StructType = 0
			)
		{
			GffStruct s = CreateStruct( FieldName, StructType );

			s.SetFLOAT( "x", q.x );
			s.SetFLOAT( "y", q.y );
			s.SetFLOAT( "z", q.z );
			s.SetFLOAT( "w", q.w );
		}

		inline
		void
		SetColor(
			__in_opt const char * FieldName,
			__in const NWN::NWNCOLOR & c,
			__in unsigned long StructType = 0
			)
		{
			GffStruct     s = CreateStruct( FieldName, StructType );
			unsigned char cc;

			cc = (unsigned char) (c.r * 255.0f);

			s.SetBYTE( "r", cc );

			cc = (unsigned char) (c.g * 255.0f);

			s.SetBYTE( "g", cc );

			cc = (unsigned char) (c.b * 255.0f);

			s.SetBYTE( "b", cc );

			cc = (unsigned char) (c.a * 255.0f);

			s.SetBYTE( "a", cc );
		}

		inline
		void
		SetUVScroll(
			__in_opt const char * FieldName,
			__in const NWN::NWN2_UVScrollSet & UVScroll,
			__in unsigned long StructType = 0
			)
		{
			GffStruct s      = CreateStruct( FieldName, StructType );
			GffStruct Scroll = s.CreateStruct( "UVScroll" );

			Scroll.SetBoolAsINT( "Scroll", UVScroll.Scroll );
			Scroll.SetFLOAT( "U", UVScroll.U );
			Scroll.SetFLOAT( "V", UVScroll.V );
		}

		inline
		void
		SetRawTintSet(
			__in_opt const char * FieldName,
			__in const NWN::NWN2_TintSet & TintSet,
			__in unsigned long StructType = 0
			)
		{
			GffStruct   Tint        = CreateStruct( FieldName, StructType );
			static char TintNames[] = "123";

			for (size_t i = 0; i < 3; i += 1)
			{
				char      TintName[ 2 ];

				TintName[ 0 ] = TintNames[ i ];
				TintName[ 1 ] = '\0';

				Tint.SetColor( TintName, TintSet.Colors[ i ] );
			}
		}

		inline
		void
		SetTintSet(
			__in_opt const char * FieldName,
			__in const NWN::NWN2_TintSet & TintSet,
			__in unsigned long StructType = 0
			)
		{
			GffStruct s = CreateStruct( FieldName, StructType );

			s.SetRawTintSet( "Tint", TintSet, StructType );
		}

		inline
		void
		SetTintable(
			__in_opt const char * FieldName,
			__in const NWN::NWN2_TintSet & TintSet,
			__in unsigned long StructType = 0
			)
		{
			GffStruct s = CreateStruct( FieldName, StructType );

			s.SetTintSet( "Tintable", TintSet );
		}

		inline
		void
		SetArmorAccessory(
			__in_opt const char * FieldName,
			__in const NWN::NWN2_ArmorAccessory & Accessory,
			__in unsigned long StructType = 0
			)
		{
			GffStruct s = CreateStruct( FieldName, StructType );

			s.SetBYTE( "Accessory", Accessory.Variation );
			s.SetTintable( NULL, Accessory.Tint );
			s.SetUVScroll( NULL, Accessory.UVScroll );
		}

		inline
		void
		SetArmorPiece(
			__in_opt const char * FieldName,
			__in const NWN::NWN2_ArmorPiece & ArmorPiece,
			__in unsigned long StructType = 0
			)
		{
			GffStruct s = CreateStruct( FieldName, StructType );

			s.SetBYTE( "Variation", ArmorPiece.Variation );
			s.SetBYTE( "ArmorVisualType", ArmorPiece.VisualType );
			s.SetRawTintSet( "ArmorTint", ArmorPiece.Tint );
		}

		inline
		void
		SetArmorPieceWithAccessories(
			__in_opt const char * FieldName,
			__in const NWN::NWN2_ArmorPieceWithAccessories & ArmorPiece,
			__in unsigned long StructType = 0
			)
		{
			GffStruct           s = CreateStruct( FieldName, StructType );
			GffStruct           Ac;
			static char * AccessoryNames[ NWN::Num_Accessories ] =
			{
				"ACLtShoulder",
				"ACRtShoulder",
				"ACLtBracer",
				"ACRtBracer",
				"ACLtElbow",
				"ACRtElbow",
				"ACLtArm",
				"ACRtArm",
				"ACLtHip",
				"ACRtHip",
				"ACFtHip",
				"ACBkHip",
				"ACLtLeg",
				"ACRtLeg",
				"ACLtShin",
				"ACRtShin",
				"ACLtKnee",
				"ACRtKnee",
				"ACLtFoot",
				"ACRtFoot",
				"ACLtAnkle",
				"ACRtAnkle"
			};

			for (size_t i = 0; i < NWN::Num_Accessories; i += 1)
			{
				SetArmorAccessory( AccessoryNames[ i ], ArmorPiece.Accessories[ i ], (unsigned long) i );
			}

			SetArmorPiece( FieldName, ArmorPiece, StructType );
		}

		inline
		void
		SetArmorAccessorySet(
			__in_opt const char * FieldName,
			__in const NWN::NWN2_ArmorAccessorySet & ArmorAccessorySet,
			__in unsigned long StructType = 0
			)
		{
			GffStruct s = CreateStruct( FieldName, StructType );

			s.SetArmorPieceWithAccessories( NULL, ArmorAccessorySet.Chest, StructType );

			if (ArmorAccessorySet.HasHelm)
				s.SetArmorPiece( "Helm", ArmorAccessorySet.Helm );
			else
				s.DeleteField( "Helm" );

			if (ArmorAccessorySet.HasGloves)
				s.SetArmorPiece( "Gloves", ArmorAccessorySet.Gloves );
			else
				s.DeleteField( "Gloves" );

			if (ArmorAccessorySet.HasBoots)
				s.SetArmorPiece( "Boots", ArmorAccessorySet.Boots );
			else
				s.DeleteField( "Boots" );

			if (ArmorAccessorySet.HasBelt)
				s.SetArmorPiece( "Belt", ArmorAccessorySet.Belt );
			else
				s.DeleteField( "Belt" );

			if (ArmorAccessorySet.HasCloak)
				s.SetArmorPiece( "Cloak", ArmorAccessorySet.Cloak );
			else
				s.DeleteField( "Cloak" );
		}

		inline
		void
		SetObjectLocation(
			__in_opt const char * FieldName,
			__in const NWN::ObjectLocation & Location,
			__in unsigned long StructType = 0
			)
		{
			GffStruct s = CreateStruct( FieldName, StructType );

			s.SetFLOAT( "XOrientation", Location.Orientation.x );
			s.SetFLOAT( "YOrientation", Location.Orientation.y );

			s.SetFLOAT( "XPosition", Location.Position.x );
			s.SetFLOAT( "YPosition", Location.Position.y );
			s.SetFLOAT( "ZPosition", Location.Position.z );
		}

	private:

		//
		// Locate a field by name.  On failure, the routine returns
		// m_StructEntry->StructFields.end( ).
		//

		inline
		FieldEntryVec::iterator
		GetFieldByNameIt(
			__in const char * Name
			)
		{
			size_t NameLen = strlen( Name );

			//
			// If the name of the label is greater than the maximum possible
			// label length, then there can be no matches, so we bail out here
			// and now.  Otherwise, we would accept any label that is prefixed
			// with the given name.
			//

			if (NameLen > 16)
				return m_StructEntry->StructFields.end( );

			//
			// Otherwise, scan each field for a matching label and return the
			// first (which should be only) match.
			//

			for (FieldEntryVec::iterator it = m_StructEntry->StructFields.begin( );
			     it != m_StructEntry->StructFields.end( );
			     ++it)
			{
				const char * Endp;
				size_t       LabelLen;

				C_ASSERT( sizeof( it->FieldLabel ) == 16 );

				Endp = (const char *) memchr(
					it->FieldLabel,
					'\0',
					sizeof( it->FieldLabel ) );

				if (Endp == NULL)
					LabelLen = sizeof( it->FieldLabel );
				else
					LabelLen = (Endp - it->FieldLabel);

				if (LabelLen != NameLen)
					continue;

				if (memcmp( Name, it->FieldLabel, LabelLen ))
					continue;

				return it;
			}

			//
			// No matches found.
			//

			return m_StructEntry->StructFields.end( );
		}

		//
		// Locate a field by name.  On failure, the routine returns NULL.
		//

		inline
		FieldEntry *
		GetFieldByName(
			__in const char * Name
			)
		{
			FieldEntryVec::iterator it = GetFieldByNameIt( Name );

			if (it == m_StructEntry->StructFields.end( ))
				return NULL;
			else
				return &*it;
		}

		//
		// Locate (or create) a new field with a given type.  On type mismatch
		// or out of memory, an std::exception is raised.
		//

		inline
		FieldEntryVec::iterator
		CreateField(
			__in GFF_FIELD_TYPE FieldType,
			__in const char * FieldName,
			__out bool & NewField
			)
		{
			//
			// First, look for an existing field of that name.
			//

			FieldEntryVec::iterator it = GetFieldByNameIt( FieldName );

			if (it != m_StructEntry->StructFields.end( ))
			{
				//
				// Check that the type is compatible first.
				//

				if (it->FieldType != FieldType)
					throw std::runtime_error( "Type mismatch when updating GFF field." );

				NewField = false;

				return it;
			}

			//
			// No match, we'll have to create it.
			//

			FieldEntry Entry;
			size_t     NameLen;

			NameLen = strlen( FieldName );
			NameLen = min( NameLen, sizeof( it->FieldLabel ) );

			Entry.FieldType  = FieldType;
			Entry.FieldFlags = 0;

//			if (m_Writer->IsComplexType( FieldType ))
//				Entry.FieldFlags |= FIELD_FLAG_COMPLEX;

			ZeroMemory( Entry.FieldLabel, sizeof( Entry.FieldLabel ) );
			memcpy( Entry.FieldLabel, FieldName, NameLen );

			m_StructEntry->StructFields.push_back( Entry );

			NewField = true;

			return m_StructEntry->StructFields.end( ) - 1;
		}

		//
		// Assign the field data for a field which fits within the
		// DataOrDataOffset block of a field descriptor.
		//

		template< typename T >
		inline
		void
		SetSmallFieldByName(
			__in GFF_FIELD_TYPE FieldType,
			__in const char * FieldName,
			__in T Data
			)
		{
			//
			// Acquire a reference to the field for creation (or update).
			//

			bool                    NewField;
			FieldEntryVec::iterator it = CreateField( FieldType, FieldName, NewField );

			try
			{
				it->FieldData.clear( );
				it->FieldData.resize( sizeof( T ) );

				//
				// N.B.  Little endian assumed.
				//

				memcpy( &it->FieldData[ 0 ], &Data, sizeof( Data ) );

				it->FieldFlags |= FIELD_FLAG_HAS_DATA;
			}
			catch (...)
			{
				//
				// If we had not finished initializing a new field, then remove
				// it entirely.
				//

				if (NewField)
					m_StructEntry->StructFields.erase( it );

				throw;
			}
		}

		//
		// Assign the field data for a field which is located within the
		// field data stream, but which has a simple (contiguous fixed size)
		// format.
		//

		template< typename T >
		inline
		void
		SetLargeFieldByName(
			__in GFF_FIELD_TYPE FieldType,
			__in const char * FieldName,
			__in T Data
			)
		{
			//
			// Acquire a reference to the field for creation (or update).
			//

			bool                    NewField;
			FieldEntryVec::iterator it = CreateField( FieldType, FieldName, NewField );

			try
			{
				it->FieldData.clear( );
				it->FieldData.resize( sizeof( T ) );

				//
				// N.B.  Little endian assumed.
				//

				memcpy( &it->FieldData[ 0 ], &Data, sizeof( Data ) );

				it->FieldFlags |= FIELD_FLAG_HAS_DATA | FIELD_FLAG_COMPLEX;
			}
			catch (...)
			{
				//
				// If we had not finished initializing a new field, then remove
				// it entirely.
				//

				if (NewField)
					m_StructEntry->StructFields.erase( it );

				throw;
			}
		}

		//
		// Assign the field data for a field which is located within the
		// field data stream, and which has a non-simple format (i.e. non-fixed
		// size not of a base data type).
		//

		inline
		void
		SetComplexFieldByName(
			__in GFF_FIELD_TYPE FieldType,
			__in const char * FieldName,
			__in const std::vector< unsigned char > & Data
			)
		{
			//
			// Acquire a reference to the field for creation (or update).
			//

			bool                    NewField;
			FieldEntryVec::iterator it = CreateField( FieldType, FieldName, NewField );

			try
			{
				it->FieldData = Data;

				it->FieldFlags |= FIELD_FLAG_HAS_DATA | FIELD_FLAG_COMPLEX;
			}
			catch (...)
			{
				//
				// If we had not finished initializing a new field, then remove
				// it entirely.
				//

				if (NewField)
					m_StructEntry->StructFields.erase( it );

				throw;
			}
		}

		//
		// Return the struct entry associated with this struct context.
		//

		inline
		FieldStruct *
		GetStructEntry(
			)
		{
			return m_StructEntry.get( );
		}

		GffFileWriter  * m_Writer;
		FieldStructPtr   m_StructEntry;

		friend class GffFileWriter;
	};

	//
	// Get the root structure for the file.
	//

	inline
	GffStruct
	GetRootStruct(
		)
	{
		return GffStruct( this, m_RootStruct );
	}

	//
	// Initialize a writer's contents from a reader.  This is simply shorthand
	// for performing the work manually via the root structure.
	//
	// N.B.  The caller assumes responsibility for ensuring that the writer's
	//       root structure is empty at the time of this call !
	//

	inline
	void
	InitializeFromReader(
		__in const GffFileReader * Reader
		)
	{
		GetRootStruct( ).InitializeFromStruct( Reader->GetRootStruct( ) );
	}

private:

	//
	// Define the GFF writer context, which supports data append only, to a
	// disk or memory target.
	//

	struct GffWriteContext
	{
		enum ContextType
		{
			ContextTypeFile,
			ContextTypeMemory,

			LastContextType
		};

		inline
		GffWriteContext(
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
						throw std::runtime_error( "GffWriteContext::Write failed to write to file." );
					}

					if ((size_t) Written != Length)
						throw std::runtime_error( "GffWriteContext::Write wrote less than the required count of bytes." );
				}
				break;

			case ContextTypeMemory:
				{
					size_t Tail;

					Tail = Memory->size( );

					if (WritePtr + Length < WritePtr)
						throw std::runtime_error( "GffWriteContext::Write encountered write pointer overflow." );

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
						throw std::runtime_error( "GffWriteContext::Read failed to read from file." );
					}

					if ((size_t) Read != Length)
						throw std::runtime_error( "GffWriteContext::Read read less than the required count of bytes." );
				}
				break;

			case ContextTypeMemory:
				{
					size_t Tail;

					Tail = Memory->size( );

					if (WritePtr + Length < WritePtr)
						throw std::runtime_error( "GffWriteContext::Read encountered read pointer overflow." );

					if (WritePtr + Length > Tail)
						throw std::runtime_error( "GffWriteContext::Read read past the end of stream." );

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
	// Commit the GFF to a write context.
	//

	void
	CommitInternal(
		__in GffWriteContext * Context,
		__in unsigned long FileType,
		__in unsigned long Flags
		);

	typedef GffFileReader::GFF_HEADER GFF_HEADER;
	typedef GffFileReader::GFF_STRUCT_ENTRY GFF_STRUCT_ENTRY;
	typedef GffFileReader::GFF_FIELD_ENTRY GFF_FIELD_ENTRY;
	typedef GffFileReader::GFF_LIST_ENTRY GFF_LIST_ENTRY;

	//
	// Build the GFF header.
	//

	void
	BuildHeader(
		__out GFF_HEADER & Header,
		__in unsigned long FileType
		);

	//
	// Write the label entries out.
	//

	void
	WriteLabelEntries(
		__inout GFF_HEADER & Header,
		__in GffWriteContext * Context
		);

	//
	// Write the field data out.
	//

	void
	WriteFieldData(
		__inout GFF_HEADER & Header,
		__in GffWriteContext * Context
		);

	//
	// Write the field indicies out.
	//

	void
	WriteFieldIndicies(
		__inout GFF_HEADER & Header,
		__in GffWriteContext * Context
		);

	//
	// Write the structure entries out.
	//

	void
	WriteStructEntries(
		__inout GFF_HEADER & Header,
		__in GffWriteContext * Context
		);

	//
	// Write the list indicies out.
	//

	void
	WriteListIndicies(
		__inout GFF_HEADER & Header,
		__in GffWriteContext * Context
		);

	//
	// Write the field entries out.
	//

	void
	WriteFieldEntries(
		__inout GFF_HEADER & Header,
		__in GffWriteContext * Context
		);

	//
	// Determine whether a field type is a complex type or a simple type.
	//
	// A simple type is stored inline in the DataOrDataOffset field.
	//

	static
	bool
	IsComplexType(
		__in GFF_FIELD_TYPE FieldType
		);

	//
	// Add a structure to the tracking list.
	//

	inline
	void
	AddStruct(
		__in FieldStructPtr Struct
		)
	{
#if GFFFILEWRITER_PRETRACK_STRUCTS
		m_Structs.push_back( Struct );
#else
		UNREFERENCED_PARAMETER( Struct );
#endif
	}

	//
	// Delete a structure from the tracking list.
	//
	// Note that deletion is potentially expensive if we are removing a large
	// section of the tree.  The editor is not optimized for large deletions as
	// they are not common operations.  Furthermore, a sequential scan through
	// the flattened list is required for each struct that is being deleted.
	//

	inline
	void
	DeleteStruct(
		__in FieldStructPtr Struct
		)
	{
#if GFFFILEWRITER_PRETRACK_STRUCTS
		//
		// Recursively unlink all children of this tree section, then delete
		// the struct itself once we are (finally) done.
		//

		for (FieldEntryVec::iterator it = Struct->StructFields.begin( );
		     it != Struct->StructFields.end( );
		     ++it)
		{
			if (it->FieldType == GffFileReader::GFF_STRUCT)
				DeleteStruct( it->Struct );
			else if (it->FieldType == GffFileReader::GFF_LIST)
			{
				for (FieldStructPtrVec::iterator lit = it->List.begin( );
				     lit != it->List.end( );
				     ++lit)
				{
					DeleteStruct( *lit );
				}
			}
		}

		//
		// Finally, search the flat list for a matching entry for this struct
		// and delete it.
		//

		FieldStructIdxVec::iterator rit;

		for (rit = m_Structs.begin( );
		     rit != m_Structs.end( );
		     ++rit)
		{
			if (rit->get( ) == Struct.get( ))
				break;
		}

		if (rit != m_Structs.end( ))
			m_Structs.erase( rit );
		else
		{
			//
			// We should never be out of sync!
			//

			NWN_ASSERT( rit != m_Structs.end( ) );
		}
#else
		UNREFERENCED_PARAMETER( Struct );
#endif
	}

#if !GFFFILEWRITER_PRETRACK_STRUCTS
	inline
	void
	AddStructRecursive(
		__in FieldStruct * Struct
		)
	{
		//
		// Recursively link all children of this tree section after linking the
		// struct itself.
		//

		m_Structs.push_back( Struct );

		for (FieldEntryVec::iterator it = Struct->StructFields.begin( );
		     it != Struct->StructFields.end( );
		     ++it)
		{
			if (it->FieldType == GffFileReader::GFF_STRUCT)
				AddStructRecursive( it->Struct.get( ) );
			else if (it->FieldType == GffFileReader::GFF_LIST)
			{
				for (FieldStructPtrVec::iterator lit = it->List.begin( );
				     lit != it->List.end( );
				     ++lit)
				{
					AddStructRecursive( (*lit).get( ) );
				}
			}
		}
	}
#endif


	typedef GffFileReader::GFF_LANGUAGE GFF_LANGUAGE;

	//
	// Comparison predicate for LabelIndexMap.
	//

	struct LabelLess : public std::binary_function< const GFF_LABEL_ENTRY &, const GFF_LABEL_ENTRY &, bool >
	{

		inline
		bool
		operator()(
			__in const GFF_LABEL_ENTRY & left,
			__in const GFF_LABEL_ENTRY & right
			) const
		{
			return memcmp( &left, &right, sizeof( left ) ) < 0;
		}

	};

	typedef std::map< GFF_LABEL_ENTRY, LABEL_INDEX, LabelLess > LabelIndexMap;



	//
	// Define the default language for localized strings.
	//

	GFF_LANGUAGE      m_Language;

	//
	// Define the default file type if none is specified for a commit request.
	//

	unsigned long     m_FileType;

	//
	// Define the root of the GFF data tree.  Each structure that is present in
	// the final file is present in the tree.
	//

	FieldStructPtr    m_RootStruct;

	//
	// Define the flattened list of all structures within the data tree.  For
	// performance reasons, we store links to the data tree in both tree form
	// (via m_RootStruct) and flattened form (m_Structs).
	//
	// In pre-track mode (GFFFILEWRITER_PRETRACK_STRUCTS):
	//
	// Data insertion and editing operations primarily use the tree form of the
	// data tree, whereas disk writing primarily uses the flattened form.  By
	// amortizing the cost of building a flattened representation across each
	// insertion, we avoid unpleasantries such as deep recursion or multiple
	// expensive rescans of the entire data tree during write commits.
	//
	// In on-write track mode (!GFFFILEWRITER_PRETRACK_STRUCTS):
	//
	// Disk writing recursively builds the flattened index on each commit.
	//

	FieldStructIdxVec m_Structs;

};

#endif
