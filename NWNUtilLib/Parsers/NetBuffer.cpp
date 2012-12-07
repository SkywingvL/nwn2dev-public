/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    NetBuffer.cpp

Abstract:

    This module contains the implementation for the ExoParseBuffer and
	ExoBuildBuffer managed classes.

--*/

#include "Precomp.h"
#include "BufferParser.h"
#include "BufferBuilder.h"
#include "..\BaseTypes.h"
#include "NetBuffer.h"

using NWN::Parsers::ExoParseBuffer;
using NWN::Parsers::ExoBuildBuffer;

/*
 * ExoParseBuffer
 */

ExoParseBuffer::ExoParseBuffer(
	__in_bcount( ByteDataLength ) const void *ByteData,
	__in size_t ByteDataLength,
	__in_bcount_opt( BitDataLength ) const void *BitData,
	__in size_t BitDataLength
	)
	: m_ByteStream( NULL ),
	  m_BitStream( NULL )
{
	m_ByteStream = new swutil::BufferParser(
		ByteData,
		ByteDataLength,
		swutil::BufferParser::BitOrderHighToLow );

	try
	{
		m_BitStream = new swutil::BufferParser(
			BitData,
			BitDataLength,
			swutil::BufferParser::BitOrderHighToLow );
	}
	catch (std::exception)
	{
		delete m_ByteStream;
		throw;
	}
}

ExoParseBuffer::~ExoParseBuffer( )
{
	delete m_BitStream;
	delete m_ByteStream;
}

bool
ExoParseBuffer::ReadCExoString(
	__out [Out] String ^% Str,
	__in size_t NumBits /* = 32 */
	)
{
	const void *Data;

	if (NumBits == 32)
	{
		unsigned Length;

		if (!m_ByteStream->GetField( Length ))
			return false;

		if (!m_ByteStream->GetDataPtr( Length, &Data ))
			return false;

		if ((int) Length < 0)
			return false;

		try
		{
			Str = gcnew String(
				reinterpret_cast< const char * >( Data ),
				0,
				(int) Length,
				System::Text::Encoding::UTF8 );
		}
		catch (Exception ^)
		{
			return false;
		}

		return true;
	}

	unsigned __int64 Length;

	if (!ReadSigned( Length, NumBits ))
		return false;

	if (!m_ByteStream->GetDataPtr( static_cast< size_t >( Length ), &Data ))
		return false;

	if ((int) Length < 0)
		return false;

	try
	{
		Str = gcnew String(
			reinterpret_cast< const char * >( Data ),
			0,
			(int) Length,
			System::Text::Encoding::UTF8 );
	}
	catch (Exception ^)
	{
		return false;
	}

	return true;
}

bool
ExoParseBuffer::ReadCExoLocString(
	NWN::ExoLocString ^ LocStr
	)
{
	if (!ReadBOOL( LocStr->IsStrRef ))
		return false;

	if (LocStr->IsStrRef)
	{
		unsigned char Flag;

		if (!ReadBYTE( Flag, 1 ))
			return false;

		LocStr->Flag = Flag ? true : false;

		if (!ReadDWORD( LocStr->StrRef ))
			return false;
	}
	else
	{
		if (!ReadCExoString( LocStr->String ))
			return false;
	}

	return true;
}

bool
ExoParseBuffer::ReadSmallString(
	__out [Out] String ^% Str,
	__in size_t NumBits /* = 8 */
	)
{
	unsigned __int64   Length;
	const void       * Data;

	if (!ReadUnsigned( Length, NumBits ))
		return false;

	if (!m_ByteStream->GetDataPtr( (size_t) Length, &Data ))
		return false;

	if ((int) Length < 0)
		return false;

	try
	{
		Str = gcnew String(
			reinterpret_cast< const char * >( Data ),
			0,
			(int) Length,
			System::Text::Encoding::UTF8 );
	}
	catch (Exception ^)
	{
		return false;
	}

	return true;
}

bool
ExoParseBuffer::ReadSigned(
	__out [Out] unsigned __int64 &FieldBits,
	__in size_t NumBits
	)
{
	bool SignBit;

	if (!m_BitStream->GetFieldBit( SignBit ))
		return false;

	if (!ReadUnsigned( FieldBits, NumBits - 1))
		return false;

	if (!SignBit)
		return true;

	int Low   = -(static_cast< int >( static_cast< unsigned >( (FieldBits >> 0 ) & 0xFFFFFFFF ) )             );
	int High  = -(static_cast< int >( static_cast< unsigned >( (FieldBits >> 32) & 0xFFFFFFFF ) ) + (Low != 0));

	if (!((unsigned)Low | (unsigned)High))
		FieldBits = 0x8000000000000000;
	else
	{
		FieldBits = (static_cast< unsigned         >( Low  ) << 0 ) |
		            (static_cast< unsigned __int64 >( High ) << 32);
	}

	return true;
}

bool
ExoParseBuffer::ReadUnsigned(
	__out [Out] unsigned __int64 &FieldBits,
	__in size_t NumBits
	)
{
	size_t Offset = 0;

	if (NumBits == 64)
		return m_ByteStream->GetField( FieldBits );

	FieldBits = 0;

	if (NumBits - Offset >= 32)
	{
		unsigned long Bits;

		if (!m_ByteStream->GetField( Bits ))
			return false;

		FieldBits <<= 32;
		FieldBits |= (static_cast< unsigned __int64 >( Bits ));
		Offset    += 32;
	}

	if (NumBits - Offset >= 16)
	{
		unsigned short Bits;

		if (!m_ByteStream->GetField( Bits ))
			return false;

		FieldBits <<= 16;
		FieldBits |= (static_cast< unsigned __int64 >( Bits ));
		Offset    += 16;
	}

	if (NumBits - Offset >= 8)
	{
		unsigned char Bits;

		if (!m_ByteStream->GetField( Bits ))
			return false;

		FieldBits <<= 8;
		FieldBits |= (static_cast< unsigned __int64 >( Bits ));
		Offset    += 8;
	}

	if (NumBits - Offset)
	{
		unsigned char Bits;

		if (!m_BitStream->GetFieldBits( NumBits - Offset, Bits ))
			return false;

		FieldBits <<= NumBits - Offset;
		FieldBits |= (static_cast< unsigned __int64 >( Bits ));
		Offset    += (NumBits - Offset);
	}

	return true;
}

/*
 * ExoBuildBuffer
 */

ExoBuildBuffer::ExoBuildBuffer(
	)
	: m_ByteStream( NULL ),
	  m_BitStream( NULL )
{
	m_ByteStream = new swutil::BufferBuilder(
		32,
		swutil::BufferBuilder::BitOrderHighToLow );

	try
	{
		m_BitStream = new swutil::BufferBuilder(
			1,
			swutil::BufferBuilder::BitOrderHighToLow );
	}
	catch (std::exception)
	{
		delete m_ByteStream;
		throw;
	}
}

ExoBuildBuffer::~ExoBuildBuffer( )
{
	delete m_BitStream;
	delete m_ByteStream;
}

void
ExoBuildBuffer::WriteCExoString(
	__in String ^ Str,
	__in size_t NumBits /* = 32 */
	)
{
	array< Byte > ^ Data = System::Text::Encoding::UTF8->GetBytes( Str );

	if (NumBits == 32)
	{
		m_ByteStream->AddField( static_cast< unsigned long >( Data->Length ) );
	}
	else
	{
		WriteSigned(
			static_cast< unsigned __int64 >( Data->Length ),
			NumBits
			);
	}

	if (Data->Length == 0)
		return;

	pin_ptr< Byte > DataPtr = &Data[ 0 ];

	m_ByteStream->AddData( Data->Length, DataPtr );
}

void
ExoBuildBuffer::WriteCExoLocString(
	__in String ^ Str
	)
{
	//
	// We don't support STRREFs directly, we'll just always send a string.
	//

	WriteBOOL( false );
	WriteCExoString( Str );
}

void
ExoBuildBuffer::WriteCExoLocString(
	__in NWN::ExoLocString ^ Str
	)
{
	WriteBOOL( Str->IsStrRef );

	if (Str->IsStrRef)
	{
		WriteBYTE( Str->Flag, 1 );
		WriteDWORD( Str->StrRef );
	}
	else
	{
		WriteCExoString( Str->String );
	}
}


void
ExoBuildBuffer::WriteSmallString(
	__in String ^ Str,
	__in size_t NumBits /* = 8 */
	)
{
	array< Byte > ^ Data = System::Text::Encoding::UTF8->GetBytes( Str );

	if ((unsigned __int64) Data->Length > (1ui64 << NumBits))
		throw gcnew ApplicationException( "String too long." );

	WriteUnsigned( (unsigned __int64) Data->Length, NumBits );

	if (Data->Length == 0)
		return;

	pin_ptr< Byte > DataPtr = &Data[ 0 ];

	m_ByteStream->AddData( Data->Length, DataPtr );
}

void
ExoBuildBuffer::WriteSigned(
	__in signed __int64 FieldBits,
	__in size_t NumBits
	)
{
	m_BitStream->AddFieldBit( (FieldBits < 0) );
	WriteUnsigned( static_cast< unsigned __int64 >( FieldBits ), NumBits - 1 );
}

void
ExoBuildBuffer::WriteUnsigned(
	__in unsigned __int64 FieldBits,
	__in size_t NumBits
	)
{
	size_t Offset;

	if (NumBits == 64)
		return m_ByteStream->AddField( FieldBits );

	Offset    = NumBits;

	if (Offset >= 32)
	{
		unsigned long Bits = static_cast< unsigned long >( FieldBits >> (Offset - 32) );

		m_ByteStream->AddField( Bits );

		Offset -= 32;
	}

	if (Offset >= 16)
	{
		unsigned short Bits = static_cast< unsigned short >( FieldBits >> (Offset - 16) );

		m_ByteStream->AddField( Bits );

		Offset -= 16;
	}

	if (Offset >= 8)
	{
		unsigned char Bits = static_cast< unsigned char >( FieldBits >> (Offset - 8) );

		m_ByteStream->AddField( Bits );

		Offset -= 8;
	}

	if (Offset)
	{
		unsigned char Bits = static_cast< unsigned char >( FieldBits >> (         0) );

		m_BitStream->AddFieldBits( Offset, Bits );
	}
}
