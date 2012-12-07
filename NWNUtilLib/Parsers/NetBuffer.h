/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    NetBuffer.h

Abstract:

    This module contains definitions for the ExoParseBuffer and ExoBuildBuffer
	managed classes.

--*/

#ifndef _NWNUTILLIB_PARSERS_NETBUFFER_H
#define _NWNUTILLIB_PARSERS_NETBUFFER_H

#pragma once

namespace NWN
{
	namespace Parsers
	{
		public ref class ExoParseBuffer
		{

		public:

			ExoParseBuffer(
				__in_bcount( ByteDataLength ) const void *ByteData,
				__in size_t ByteDataLength,
				__in_bcount_opt( BitDataLength ) const void *BitData,
				__in size_t BitDataLength
				);

			~ExoParseBuffer( );

			//
			// Reads a counted string from the wire.  If the length is 32 bits,
			// then it is treated as an unsigned value as far as where the length
			// bits come from.  Otherwise it is treated as a signed value, though
			// we return a blank string as per the NWN implementation if we get a
			// negative value.
			//

			inline
			bool
			ReadCExoString(
				__out [Out] String ^% Str
				)
			{
				return ReadCExoString( Str, 32 );
			}

			bool
			ReadCExoString(
				__out [Out] String ^% Str,
				__in size_t NumBits
				);

			bool
			ReadCExoLocString(
				__out [Out] ExoLocString ^ LocStr
				);

			//
			// Read a string with an unsigned 8-bit length prefix.
			//

			inline
			bool
			ReadSmallString(
				__out [Out] String ^%String
				)
			{
				return ReadSmallString( String, 8 );
			}

			bool
			ReadSmallString(
				__out [Out] String ^%String,
				__in size_t NumBits
				);

			//
			// Read a NWN1-style 16-byte CResRef.
			//

			bool
			ReadCResRef16(
				__out [Out] ResRef16 ^Ref
				)
			{
				return ReadCResRef16( Ref, 16 );
			}

			bool
			ReadCResRef16(
				__out [Out] ResRef16 ^Ref,
				__in size_t NumBytes
				)
			{
				NativeResRef16 NativeRef;
				size_t i;

				if (NumBytes > sizeof( NativeRef ))
					NumBytes = sizeof( NativeRef );

				if (!m_ByteStream->GetData( NumBytes, &NativeRef ))
					return false;

				for (i = 0; i < sizeof( NativeRef ); i += 1)
				{
					if (NativeRef.RefStr[ i ] == 0)
						break;
				}

				try
				{
					Ref->RefStr = gcnew String(
						&NativeRef.RefStr[ 0 ],
						0,
						(int) i,
						System::Text::Encoding::UTF8 );
				}
				catch (Exception ^)
				{
					return false;
				}

				return true;
			}

			//
			// Read a NWN2-style 32-byte CResRef.
			//

			bool
			ReadCResRef32(
				__out [Out] ResRef32 ^ Ref
				)
			{
				return ReadCResRef32( Ref, 32 );
			}

			bool
			ReadCResRef32(
				__out [Out] ResRef32 ^ Ref,
				__in size_t NumBytes
				)
			{
				NativeResRef32 NativeRef;
				size_t i;

				if (NumBytes > sizeof( NativeRef ))
					NumBytes = sizeof( NativeRef );

				if (!m_ByteStream->GetData( NumBytes, &NativeRef ))
					return false;

				for (i = 0; i < sizeof( NativeRef ); i += 1)
				{
					if (NativeRef.RefStr[ i ] == 0)
						break;
				}

				try
				{
					Ref->RefStr = gcnew String(
						&NativeRef.RefStr[ 0 ],
						0,
						(int) i,
						System::Text::Encoding::UTF8 );
				}
				catch (Exception ^)
				{
					return false;
				}

				return true;
			}

			inline
			bool
			ReadBOOL(
				__out [Out] bool %Value
				)
			{
				unsigned __int64 Bits;

				if (!ReadUnsigned( Bits, 1 ))
					return false;

				Value = Bits ? true : false;

				return true;
			}

			inline
			bool
			ReadCHAR(
				__out [Out] signed char % Value
				)
			{
				return ReadCHAR( Value, 8 );
			}

			inline
			bool
			ReadCHAR(
				__out [Out] signed char %Value,
				__in size_t NumBits
				)
			{
				if (NumBits == 8)
				{
					signed char ValueN;

					if (!m_ByteStream->GetField( ValueN ))
						return false;

					Value = ValueN;
					return true;
				}

				unsigned __int64 Bits;

				if (!ReadSigned( Bits, NumBits ))
					return false;

				if ((Bits >> 32) == 0x80000000)
					Value = SCHAR_MIN;
				else
					Value = static_cast< signed char >( Bits & 0xFF );

				return true;
			}

			inline
			bool
			ReadSHORT(
				__out [Out] signed short %Value
				)
			{
				return ReadSHORT( Value, 16 );
			}

			inline
			bool
			ReadSHORT(
				__out [Out] signed short %Value,
				__in size_t NumBits
				)
			{
				if (NumBits == 16)
				{
					signed short ValueN;

					if (!m_ByteStream->GetField( ValueN ))
						return false;

					Value = ValueN;
					return true;
				}

				unsigned __int64 Bits;

				if (!ReadSigned( Bits, NumBits ))
					return false;

				if ((Bits >> 32) == 0x80000000)
					Value = SHRT_MIN;
				else
					Value = static_cast< signed short >( Bits & 0xFFFF );

				return true;
			}

			inline
			bool
			ReadINT(
				__out [Out] signed int %Value
				)
			{
				return ReadINT( Value, 32 );
			}

			inline
			bool
			ReadINT(
				__out [Out] signed int %Value,
				__in size_t NumBits
				)
			{
				if (NumBits == 32)
				{
					signed int ValueN;

					if (!m_ByteStream->GetField( ValueN ))
						return false;

					Value = ValueN;
					return true;
				}

				unsigned __int64 Bits;

				if (!ReadSigned( Bits, NumBits ))
					return false;

				if ((Bits >> 32) == 0x80000000)
					Value = INT_MIN;
				else
					Value = static_cast< signed int >( Bits & 0xFFFFFFFF );

				return true;
			}

			inline
			bool
			ReadINT64(
				__out [Out] signed __int64 %Value
				)
			{
				return ReadINT64( Value, 64 );
			}

			inline
			bool
			ReadINT64(
				__out [Out] signed __int64 %Value,
				__in size_t NumBits
				)
			{
				if (NumBits == 64)
				{
					signed __int64 ValueN;

					if (!m_ByteStream->GetField( ValueN ))
						return false;

					Value = ValueN;
					return true;
				}

				unsigned __int64 Bits;

				if (!ReadSigned( Bits, NumBits ))
					return false;

				Value = static_cast< signed __int64 >( Bits );

				return true;
			}

			inline
			bool
			ReadBYTE(
				__out [Out] unsigned char %Value
				)
			{
				return ReadBYTE( Value, 8 );
			}

			inline
			bool
			ReadBYTE(
				__out [Out] unsigned char %Value,
				__in size_t NumBits
				)
			{
				if (NumBits == 8)
				{
					unsigned char ValueN;

					if (!m_ByteStream->GetField( ValueN ))
						return false;

					Value = ValueN;
					return true;
				}

				unsigned __int64 Bits;

				if (!ReadUnsigned( Bits, NumBits ))
					return false;

				Value = static_cast< unsigned char >( Bits & 0xFF );

				return true;
			}

			inline
			bool
			ReadWORD(
				__out [Out] unsigned short %Value
				)
			{
				return ReadWORD( Value, 16 );
			}

			inline
			bool
			ReadWORD(
				__out [Out] unsigned short %Value,
				__in size_t NumBits
				)
			{
				if (NumBits == 16)
				{
					unsigned short ValueN;

					if (!m_ByteStream->GetField( ValueN ))
						return false;

					Value = ValueN;
					return true;
				}

				unsigned __int64 Bits;

				if (!ReadUnsigned( Bits, NumBits ))
					return false;

				Value = static_cast< unsigned short >( Bits & 0xFFFF );

				return true;
			}

			inline
			bool
			ReadDWORD(
				__out [Out] unsigned long %Value
				)
			{
				return ReadDWORD( Value, 32 );
			}

			inline
			bool
			ReadDWORD(
				__out [Out] unsigned long %Value,
				__in size_t NumBits
				)
			{
				if (NumBits == 32)
				{
					unsigned long ValueN;

					if (!m_ByteStream->GetField( ValueN ))
						return false;

					Value = ValueN;
					return true;
				}

				unsigned __int64 Bits;

				if (!ReadUnsigned( Bits, NumBits ))
					return false;

				Value = static_cast< unsigned long >( Bits & 0xFFFFFFFF );

				return true;
			}

			inline
			bool
			ReadDWORD(
				__out [Out] uint32_t %Value
				)
			{
				return ReadDWORD( Value, 32 );
			}

			inline
			bool
			ReadDWORD(
				__out [Out] uint32_t %Value,
				__in size_t NumBits
				)
			{
				if (NumBits == 32)
				{
					uint32_t ValueN;

					if (!m_ByteStream->GetField( ValueN ))
						return false;

					Value = ValueN;
					return true;
				}

				unsigned __int64 Bits;

				if (!ReadUnsigned( Bits, NumBits ))
					return false;

				Value = static_cast< uint32_t >( Bits & 0xFFFFFFFF );

				return true;
			}

			inline
			bool
			ReadDWORD64(
				__out [Out] unsigned __int64 %Value
				)
			{
				return ReadDWORD64( Value, 64 );
			}

			inline
			bool
			ReadDWORD64(
				__out [Out] unsigned __int64 %Value,
				__in size_t NumBits
				)
			{
				if (NumBits == 64)
				{
					unsigned __int64 ValueN;

					if (!m_ByteStream->GetField( ValueN ))
						return false;

					Value = ValueN;
					return true;
				}
				else
				{
					unsigned __int64 ValueN;

					if (!ReadUnsigned( ValueN, NumBits ))
						return false;

					Value = ValueN;
					return true;
				}
			}

			inline
			bool
			ReadFLOAT(
				__out [Out] float %Value
				)
			{
				return ReadFLOAT( Value, sizeof( float ) * 8, 1.0 );
			}

			inline
			bool
			ReadFLOAT(
				__out [Out] float %Value,
				__in size_t NumBits,
				__in float Scale
				)
			{
				if ((NumBits == sizeof( float ) * 8) && (Scale == 1.0))
				{
					float ValueN;

					if (!m_ByteStream->GetField( ValueN ))
						return false;

					Value = ValueN;
					return true;
				}

				unsigned __int64 Bits;

				if (!ReadUnsigned( Bits, NumBits ))
					return false;

				Value = static_cast< float >(
					static_cast< unsigned long >( Bits & 0xFFFFFFFF )
					);

				Value /= Scale;

				return true;
			}

			//
			// Read a floating point value clamped to a range of [Scale1, Scale2]
			// and packed into an arbitrary number of bits less than 64.
			//

			inline
			bool
			ReadFLOAT(
				__out [Out] float %Value,
				__in float Scale1,
				__in float Scale2
				)
			{
				return ReadFLOAT( Value, Scale1, Scale2, sizeof( float ) * 8 );
			}

			inline
			bool
			ReadFLOAT(
				__out [Out] float %Value,
				__in float Scale1,
				__in float Scale2,
				__in size_t NumBits
				)
			{
				if (NumBits == sizeof( float ) * 8)
				{
					float ValueN;

					if (!m_ByteStream->GetField( ValueN ))
						return false;

					Value = ValueN;
					return true;
				}

				unsigned __int64 Bits;

				if (!ReadUnsigned( Bits, NumBits ))
					return false;

				float Factor  = (Scale2 - Scale1) / static_cast< float >( (1 << NumBits) - 1 );

				Value         = static_cast< float >( static_cast< double >( Bits ) * Factor );
				Value        += Scale1;

				return true;
			}

			inline
			bool
			ReadDOUBLE(
				__out [Out] double %Value
				)
			{
				return ReadDOUBLE( Value, sizeof( double ) * 8, 1.0 );
			}
		 
			inline
			bool
			ReadDOUBLE(
				__out [Out] double %Value,
				__in size_t NumBits,
				__in double Scale
				)
			{
				if ((NumBits == sizeof( double ) * 8) && (Scale == 1.0))
				{
					double ValueN;

					if (!m_ByteStream->GetField( ValueN ))
						return false;

					Value = ValueN;
					return true;
				}

				unsigned __int64 Bits;

				if (!ReadUnsigned( Bits, NumBits ))
					return false;

				Value = static_cast< double >( Bits );

				Value /= Scale;

				return true;
			}

			inline
			bool
			ReadDOUBLE(
				__out [Out] double %Value,
				__in double Scale1,
				__in double Scale2
				)
			{
				return ReadDOUBLE( Value, Scale1, Scale2, sizeof( float ) * 8 );
			}

			inline
			bool
			ReadDOUBLE(
				__out [Out] double %Value,
				__in double Scale1,
				__in double Scale2,
				__in size_t NumBits
				)
			{
				if (NumBits == sizeof( float ) * 8)
				{
					double ValueN;

					if (!m_ByteStream->GetField( ValueN ))
						return false;

					Value = ValueN;
					return true;
				}

				unsigned __int64 Bits;

				if (!ReadUnsigned( Bits, NumBits ))
					return false;

				Value  = (Scale2 - Scale1) / static_cast< double >( (1 << NumBits) - 1);
				Value  = static_cast< double >( static_cast< double >( Bits ) * Value );
				Value += Scale1;

				return true;
			}

			inline
			bool
			ReadOBJECTID(
				__out [Out] OBJECTID &ObjectId
				)
			{
				return m_ByteStream->GetField( ObjectId );
			}

			inline
			const void *
			ReadVOIDPtr(
				__in size_t FieldLength
				)
			{
				const void *DataPtr;

				if (!m_ByteStream->GetDataPtr( FieldLength, &DataPtr ))
					return NULL;

				return DataPtr;
			}

			inline
			bool
			ReadVector3(
				__out [Out] Vector3 %Value
				)
			{
				return ReadVector3( Value, sizeof( float ) * 8, 1.0 );
			}

			inline
			bool
			ReadVector3(
				__out [Out] Vector3 %Value,
				__in size_t NumBits,
				__in float Scale
				)
			{
				if (!ReadFLOAT( Value.x, NumBits, Scale ))
					return false;

				if (!ReadFLOAT( Value.y, NumBits, Scale ))
					return false;

				if (!ReadFLOAT( Value.z, NumBits, Scale ))
					return false;

				return true;
			}

			inline
			bool
			ReadVector3(
				__out [Out] Vector3 %Value,
				__in float Scale1,
				__in float Scale2
				)
			{
				return ReadVector3( Value, Scale1, Scale2, sizeof( float ) * 8 );
			}

			inline
			bool
			ReadVector3(
				__out [Out] Vector3 %Value,
				__in float Scale1,
				__in float Scale2,
				__in size_t NumBits
				)
			{
				if (!ReadFLOAT( Value.x, Scale1, Scale2, NumBits ))
					return false;

				if (!ReadFLOAT( Value.y, Scale1, Scale2, NumBits ))
					return false;

				if (!ReadFLOAT( Value.z, Scale1, Scale2, NumBits ))
					return false;

				return true;
			}

			inline
			bool
			ReadVector2(
				__out [Out] Vector2 %Value
				)
			{
				return ReadVector2( Value, sizeof( float ) * 8, 1.0 );
			}

			inline
			bool
			ReadVector2(
				__out [Out] Vector2 %Value,
				__in size_t NumBits,
				__in float Scale
				)
			{
				if (!ReadFLOAT( Value.x, NumBits, Scale ))
					return false;

				if (!ReadFLOAT( Value.y, NumBits, Scale ))
					return false;

				return true;
			}

			inline
			bool
			ReadVector2(
				__out [Out] Vector2 %Value,
				__in float Scale1,
				__in float Scale2
				)
			{
				return ReadVector2( Value, Scale1, Scale2, sizeof( float ) * 8 );
			}

			inline
			bool
			ReadVector2(
				__out [Out] Vector2 %Value,
				__in float Scale1,
				__in float Scale2,
				__in size_t NumBits
				)
			{
				if (!ReadFLOAT( Value.x, Scale1, Scale2, NumBits ))
					return false;

				if (!ReadFLOAT( Value.y, Scale1, Scale2, NumBits ))
					return false;

				return true;
			}

			inline
			bool
			ReadColor(
				__out [Out] NWNCOLOR %Value
				)
			{
				unsigned char v;

				if (!ReadBYTE( v ))
					return false;

				Value.r = (float) (((float) v) * (1.0/255.0));

				if (!ReadBYTE( v ))
					return false;

				Value.g = (float) (((float) v) * (1.0/255.0));

				if (!ReadBYTE( v ))
					return false;

				Value.b = (float) (((float) v) * (1.0/255.0));

				if (!ReadBYTE( v ))
					return false;

				Value.a = (float) (((float) v) * (1.0/255.0));

				return true;
			}

			inline
			bool
			ReadNWN2_DataElement(
				__out [Out] NWN2_DataElement ^ Element,
				__in bool Server
				)
			{
				int Count;

				Element->Bools.Clear();
				Element->Ints.Clear();
				Element->Floats.Clear();
				Element->StrRefs.Clear();
				Element->Strings.Clear();
				Element->LocStrings.Clear();
				Element->ObjectIds.Clear();

				if (!ReadINT( Count ))
					return false;

				while (Count--)
				{
					bool v;

					if (!ReadBOOL( v ))
						return false;

					Element->Bools.Add( v );
				}

				if (!ReadINT( Count ))
					return false;

				while (Count--)
				{
					int v;

					if (!ReadINT( v ))
						return false;

					Element->Ints.Add( v );
				}

				if (!ReadINT( Count ))
					return false;

				while (Count--)
				{
					float v;

					if (!ReadFLOAT( v ))
						return false;

					Element->Floats.Add( v );
				}

				if (!ReadINT( Count ))
					return false;

				while (Count--)
				{
					int v;

					if (!ReadINT( v ))
						return false;

					Element->StrRefs.Add( (unsigned long) v );
				}

				if (!ReadINT( Count ))
					return false;

				while (Count--)
				{
					String ^ v;

					if (!ReadCExoString( v ))
						return false;

					Element->Strings.Add( v );
				}

				if (Server)
				{
					if (!ReadINT( Count ))
						return false;

					while (Count--)
					{
						NWN::ExoLocString ^ v = gcnew NWN::ExoLocString;

						if (!ReadCExoLocString( v ))
							return false;

						Element->LocStrings.Add( v );
					}
				}

				if (!ReadINT( Count ))
					return false;

				while (Count--)
				{
					OBJECTID v;

					if (!ReadOBJECTID( v ))
						return false;

					Element->ObjectIds.Add( v );
				}

				return true;
			}

			inline
			bool
			ReadBits(
				__out [Out] unsigned __int64 %Value,
				__in size_t Bits
				)
			{
				unsigned __int64 ValueN;

				if (!m_BitStream->GetFieldBits( Bits, ValueN ))
					return false;

				Value = ValueN;
				return true;
			}

			inline
			bool
			AtEndOfStream(
				)
			{
				return ((m_ByteStream->AtEndOfStream( )) &&
					(m_BitStream->AtEndOfStream( )));
			}

			inline
			size_t
			GetBytesRemaining(
				)
			{
				return (m_ByteStream->GetBytesRemaining( ));
			}

			//
			// Debugging use only routine to get the current byte remaining count
			// for the bit stream.  The current byte being parsed is included in
			// the count should there be any legal bits left within it.
			//

			inline
			size_t
			GetBitStreamBytesRemaining(
				)
			{
				return (m_BitStream->GetBytesRemaining( ));
			}

			//
			// Debugging use only routine to get the current extraction position in
			// the byte stream.
			//

			inline
			size_t
			GetBytePos(
				)
			{
				return m_ByteStream->GetBytePos( );
			}

			//
			// Debugging use only routine to get a pointer into the raw byte data
			// stream.
			//

			inline
			const unsigned char *
			GetByteStreamBaseDataPointer(
				)
			{
				return m_ByteStream->GetBaseDataPointer( );
			}

			//
			// Debugging use only routine to get a pointer into the raw bit data
			// stream.
			//

			inline
			const unsigned char *
			GetBitStreamBaseDataPointer(
				)
			{
				return m_BitStream->GetBaseDataPointer( );
			}

			//
			// Debugging use only routine to get the current bit position in the
			// bit stream.
			//

			inline
			size_t
			GetBitPos(
				)
			{
				return m_BitStream->GetBitPos( );
			}

			//
			// Debugging use only routine to get the highest valid bit number in
			// the last byte of the bit stream.
			//

			inline
			size_t
			GetHighestValidBitPos(
				)
			{
				return m_BitStream->GetHighestValidBitPos( );
			}

			inline
			void
			SetHighestValidBitPos(
				__in size_t HighestValidBitPos
				)
			{
				return m_BitStream->SetHighestValidBitPos( HighestValidBitPos );
			}

		private:

			bool
			ReadSigned(
				__out [Out] unsigned __int64 &FieldBits,
				__in size_t NumBits
				);

			bool
			ReadUnsigned(
				__out [Out] unsigned __int64 &FieldBits,
				__in size_t NumBits
				);

			swutil::BufferParser * m_ByteStream;
			swutil::BufferParser * m_BitStream;

		};

		public ref class ExoBuildBuffer
		{

		public:

			explicit ExoBuildBuffer( );

			~ExoBuildBuffer( );

			void
			WriteCExoString(
				__in String ^ Str
				)
			{
				WriteCExoString( Str, 32 );
			}

			void
			WriteCExoString(
				__in String ^ Str,
				__in size_t NumBits
				);

			void
			WriteCExoLocString(
				__in String ^ Str
				);

			void
			WriteCExoLocString(
				__in NWN::ExoLocString ^ Str
				);

			void
			WriteSmallString(
				__in String ^ Str
				)
			{
				WriteSmallString( Str, 8 );
			}

			void
			WriteSmallString(
				__in String ^ Str,
				__in size_t NumBits
				);

			inline
			void
			WriteResRef16(
				__in const ResRef16 ^ Ref
				)
			{
				return WriteResRef16( Ref, 16 );
			}

			inline
			void
			WriteResRef16(
				__in const ResRef16 ^ Ref,
				__in size_t NumBytes
				)
			{
				array< Byte > ^ Data = System::Text::Encoding::UTF8->GetBytes( Ref->RefStr );

				if ((size_t) Data->Length < NumBytes)
				{
					array< Byte > ^ RawData = gcnew array< Byte >( NumBytes );

					for (int i = 0; i < Data->Length; i += 1)
						RawData[ i ] = Data[ i ];

					Data = RawData;
				}

				pin_ptr< Byte > DataPtr = &Data[ 0 ];

				m_ByteStream->AddData( NumBytes, DataPtr );
			}

			inline
			void
			WriteResRef32(
				__in const ResRef32 ^ Ref
				)
			{
				return WriteResRef32( Ref, 32 );
			}

			inline
			void
			WriteResRef32(
				__in const ResRef32 ^ Ref,
				__in size_t NumBytes
				)
			{
				array< Byte > ^ Data = System::Text::Encoding::UTF8->GetBytes( Ref->RefStr );

				if ((size_t) Data->Length < NumBytes)
				{
					array< Byte > ^ RawData = gcnew array< Byte >( NumBytes );

					for (int i = 0; i < Data->Length; i += 1)
						RawData[ i ] = Data[ i ];

					Data = RawData;
				}

				pin_ptr< Byte > DataPtr = &Data[ 0 ];

				m_ByteStream->AddData( NumBytes, DataPtr );
			}

			inline
			void
			WriteBOOL(
				__in bool Value
				)
			{
				m_BitStream->AddFieldBit( Value );
			}

			inline
			void
			WriteCHAR(
				__in signed char Value
				)
			{
				return WriteCHAR( Value, 8 );
			}

			inline
			void
			WriteCHAR(
				__in signed char Value,
				__in size_t NumBits
				)
			{
				if (NumBits == 8)
					return m_ByteStream->AddField( Value );

				return WriteSigned(
					static_cast< signed __int64 >( Value ),
					NumBits
					);
			}

			inline
			void
			WriteSHORT(
				__in signed short Value
				)
			{
				return WriteSHORT( Value, 16 );
			}

			inline
			void
			WriteSHORT(
				__in signed short Value,
				__in size_t NumBits
				)
			{
				if (NumBits == 16)
					return m_ByteStream->AddField( Value );

				return WriteSigned(
					static_cast< signed __int64 >( Value ),
					NumBits
					);
			}

			inline
			void
			WriteINT(
				__in signed int Value
				)
			{
				return WriteINT( Value, 32 );
			}

			inline
			void
			WriteINT(
				__in signed int Value,
				__in size_t NumBits
				)
			{
				if (NumBits == 32)
					return m_ByteStream->AddField( Value );

				return WriteSigned(
					static_cast< signed __int64 >( Value ),
					NumBits
					);
			}

			inline
			void
			WriteINT64(
				__in signed __int64 Value
				)
			{
				return WriteINT64( Value, 64 );
			}

			inline
			void
			WriteINT64(
				__in signed __int64 Value,
				__in size_t NumBits
				)
			{
				if (NumBits == 64)
					return m_ByteStream->AddField( Value );

				return WriteSigned(
					static_cast< signed __int64 >( Value ),
					NumBits
					);
			}

			inline
			void
			WriteBYTE(
				__in unsigned char Value
				)
			{
				return WriteBYTE( Value, 8 );
			}

			inline
			void
			WriteBYTE(
				__in unsigned char Value,
				__in size_t NumBits
				)
			{
				if (NumBits == 8)
					return m_ByteStream->AddField( Value );

				return WriteUnsigned(
					static_cast< unsigned __int64 >( Value ),
					NumBits
					);
			}

			inline
			void
			WriteWORD(
				__in unsigned short Value
				)
			{
				return WriteWORD( Value, 16 );
			}

			inline
			void
			WriteWORD(
				__in unsigned short Value,
				__in size_t NumBits
				)
			{
				if (NumBits == 16)
					return m_ByteStream->AddField( Value );

				return WriteUnsigned(
					static_cast< unsigned __int64 >( Value ),
					NumBits
					);
			}

			inline
			void
			WriteDWORD(
				__in unsigned long Value
				)
			{
				return WriteDWORD( Value, 32 );
			}

			inline
			void
			WriteDWORD(
				__in unsigned long Value,
				__in size_t NumBits
				)
			{
				if (NumBits == 32)
					return m_ByteStream->AddField( Value );

				return WriteUnsigned(
					static_cast< unsigned __int64 >( Value ),
					NumBits
					);
			}

			inline
			void
			WriteDWORD64(
				__in unsigned __int64 Value
				)
			{
				return WriteDWORD64( Value, 64 );
			}

			inline
			void
			WriteDWORD64(
				__in unsigned __int64 Value,
				__in size_t NumBits
				)
			{
				if (NumBits == 64)
					return m_ByteStream->AddField( Value );

				return WriteUnsigned(
					static_cast< unsigned __int64 >( Value ),
					NumBits
					);
			}

			inline
			void
			WriteFLOAT(
				__in float Value
				)
			{
				return WriteFLOAT( Value, sizeof( float ) * 8, 1.0 );
			}

			inline
			void
			WriteFLOAT(
				__in float Value,
				__in size_t NumBits,
				__in float Scale
				)
			{
				if ((NumBits == sizeof( float ) * 8) && (Scale == 1.0))
					return m_ByteStream->AddField( Value );

				return WriteUnsigned(
					static_cast< unsigned __int64 >( Value * Scale ),
					NumBits
					);
			}

			//
			// Write a floating point value camped to a range of [Scale1, Scale2]
			// and packed into an arbitrary number of bits less than 64.
			//

			inline
			void
			WriteFLOAT(
				__in float Value,
				__in float Scale1,
				__in float Scale2
				)
			{
				return WriteFLOAT( Value, Scale1, Scale2, sizeof( float ) * 8 );
			}

			inline
			void
			WriteFLOAT(
				__in float Value,
				__in float Scale1,
				__in float Scale2,
				__in size_t NumBits
				)
			{
				if (NumBits == sizeof( float ) * 8)
					return m_ByteStream->AddField( Value );

				float Factor             = (Scale2 - Scale1) / static_cast< float >( (1 << NumBits) - 1 );
				float Scaled             = (Value - Scale1) / Factor;
				signed int    ScaledBits = static_cast< int >( Scaled );
				signed int   OScaledBits = ScaledBits;
				float         ScaledF    = static_cast< float >( ScaledBits );

				if (ScaledBits < 0)
					ScaledF += 4.294967296e9;

				ScaledF    *= Factor;
				ScaledBits += 1;

				ScaledF    += Scale1;
				ScaledF     = Value - ScaledF;

				float ScaledF2 = static_cast< float >( ScaledBits );

				if (ScaledBits < 0)
					ScaledF2 += 0.05f;

				Factor *= ScaledF2;
				Scale1 += Factor;
				Scale1 -= Value;

				if (ScaledF > Scale1)
				{
					return WriteUnsigned(
						static_cast< unsigned __int64 >( ScaledBits ),
						NumBits);
				}
				else
				{
					return WriteUnsigned(
						static_cast< unsigned __int64 >( OScaledBits ),
						NumBits);
				}
			}

			inline
			void
			WriteOBJECTID(
				__in OBJECTID ObjectId
				)
			{
				m_ByteStream->AddField( ObjectId );
			}

			inline
			void
			WriteVOIDPtr(
				__in_bcount( Length ) const void *Data,
				__in size_t Length
				)
			{
				m_ByteStream->AddData( Length, Data );
			}

			inline
			void
			WriteVector3(
				__in const Vector3 %Value
				)
			{
				return WriteVector3( Value, sizeof( float ) * 8, 1.0 );
			}

			inline
			void
			WriteVector3(
				__in const Vector3 %Value,
				__in size_t NumBits,
				__in float Scale
				)
			{
				WriteFLOAT( Value.x, NumBits, Scale );
				WriteFLOAT( Value.y, NumBits, Scale );
				WriteFLOAT( Value.z, NumBits, Scale );
			}

			inline
			void
			WriteVector3(
				__in const Vector3 %Value,
				__in size_t NumBits,
				__in float Scale1,
				__in float Scale2
				)
			{
				WriteFLOAT( Value.x, Scale1, Scale2, NumBits );
				WriteFLOAT( Value.y, Scale1, Scale2, NumBits );
				WriteFLOAT( Value.z, Scale1, Scale2, NumBits );
			}

			inline
			void
			WriteVector2(
				__in const Vector2 %Value
				)
			{
				return WriteVector2( Value, sizeof( float ) * 8, 1.0 );
			}

			inline
			void
			WriteVector2(
				__in const Vector2 %Value,
				__in size_t NumBits,
				__in float Scale
				)
			{
				WriteFLOAT( Value.x, NumBits, Scale );
				WriteFLOAT( Value.y, NumBits, Scale );
			}

			inline
			void
			WriteColor(
				__in const NWNCOLOR %Value
				)
			{
				WriteBYTE( (unsigned char) (Value.r * 255.0) );
				WriteBYTE( (unsigned char) (Value.g * 255.0) );
				WriteBYTE( (unsigned char) (Value.b * 255.0) );
				WriteBYTE( (unsigned char) (Value.a * 255.0) );
			}

			inline
			void
			WriteNWN2_DataElement(
				__in NWN2_DataElement ^ Element,
				__in bool Server
				)
			{
				WriteINT( (int) Element->Bools.Count );

				for each (bool Value in Element->Bools)
				{
					WriteBOOL( Value );
				}

				WriteINT( (int) Element->Ints.Count );

				for each (int Value in Element->Ints)
				{
					WriteINT( Value );
				}

				WriteINT( (int) Element->Floats.Count );

				for each (float Value in Element->Floats)
				{
					WriteFLOAT( Value );
				}

				WriteINT( (int) Element->StrRefs.Count );

				for each (unsigned long Value in Element->StrRefs)
				{
					WriteINT( (int) Value );
				}

				WriteINT( (int) Element->Strings.Count );

				for each (String ^ Str in Element->Strings)
				{
					WriteCExoString( Str );
				}

				if (Server)
				{
					WriteINT( (int) Element->LocStrings.Count );

					for each (ExoLocString ^ LocStr in Element->LocStrings)
					{
						WriteCExoLocString( LocStr );
					}
				}

				WriteINT( (int) Element->ObjectIds.Count );

				for each (OBJECTID ObjectId in Element->ObjectIds)
				{
					WriteOBJECTID( ObjectId );
				}
			}

			inline
			void
			WriteBits(
				__in unsigned __int64 Value,
				__in size_t Bits
				)
			{
				m_BitStream->AddFieldBits( Bits, Value );
			}

			inline
			void
			GetBuffer(
				__out [Out] unsigned char *% ByteBuffer,
				__out [Out] size_t % ByteBufferLength,
				__out [Out] unsigned char *% BitBuffer,
				__out [Out] size_t % BitBufferLength
				)
			{
				unsigned char * BufferN;
				size_t SizeN;

				m_ByteStream->GetBuffer( BufferN, SizeN );
				ByteBuffer = BufferN;
				ByteBufferLength = SizeN;

				m_BitStream->GetBuffer( BufferN, SizeN );
				BitBuffer = BufferN;
				BitBufferLength = SizeN;

				//
				// If we have a bit stream, then let's write the highest valid bit
				// position into the buffer.  There are guaranteed to always be at
				// least three bits in the bit stream, which delimit the maximum
				// bit offset in the last byte of the bit stream.
				//

				if (BitBufferLength > 0)
				{
					size_t HighestValidBitPos = m_BitStream->GetBitPos( );

					BitBuffer[ 0 ] = (BitBuffer[ 0 ] & 0x1F) |
						static_cast< unsigned char >( (HighestValidBitPos << 5) & 0xFF );
				}
			}

			//
			// Perform a fast check as to whether any update data was written for
			// this buffer or not.
			//

			inline
			bool
			GetIsDataWritten(
				)
			{
				unsigned char * Buf;
				size_t          Length;

				m_ByteStream->GetBuffer( Buf, Length );

				if (Length != 0)
					return true;

				//
				// Now check if the bit stream is empty.  Discount the first three
				// bits we reserved space for (for the highest valid bit position)
				// as they are not really user data.
				//

				m_BitStream->GetBuffer( Buf, Length );

				if (Length > 1)
					return true;

				if (m_BitStream->GetBitPos( ) > 3)
					return true;

				return false;
			}

			//
			// Return the total size of the message payload being constructed, so
			// far.
			//
			
			inline
			size_t
			GetMessagePayloadSize(
				)
			{
				unsigned char * Buf;
				size_t          Length;
				size_t          TotalLength;

				m_ByteStream->GetBuffer( Buf, Length );

				TotalLength = Length;

				m_BitStream->GetBuffer( Buf, Length );

				TotalLength += Length;

				return TotalLength;
			}

		private:

			void
			WriteSigned(
				__in signed __int64 FieldBits,
				__in size_t NumBits
				);

			void
			WriteUnsigned(
				__in unsigned __int64 FieldBits,
				__in size_t NumBits
				);

			swutil::BufferBuilder * m_ByteStream;
			swutil::BufferBuilder * m_BitStream;

		};

	}
}

#endif
