/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    BufferBuilder.h

Abstract:

    This module contains definitions for the BufferBuilder class.

--*/

#ifndef _SOURCE_PROGRAMS_SKYWINGUTILS_PARSERS_BUFFERBUILDER_H
#define _SOURCE_PROGRAMS_SKYWINGUTILS_PARSERS_BUFFERBUILDER_H

#ifdef _MSC_VER

#pragma once

#pragma warning(push)
#pragma warning(disable:4290) // warning C4290: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)

#endif

namespace swutil
{
	class IBufferBuilder
	{

	public:

		template< class T >
		inline
		void
		AddField(
			__in const T &Field
			) throw( std::exception )
		{
			return AddData(
				sizeof( T ),
				&Field
				);
		}

		virtual
		void
		AddData(
			__in size_t FieldLength,
			__in_bcount( FieldLength ) const void *Field
			) throw( std::exception ) = 0;

		inline
		void
		AddFieldBit(
			__in bool FieldBit
			) throw( std::exception )
		{
			return AddFieldBits(
				1,
				static_cast< unsigned __int64 >( FieldBit ? 1 : 0 )
				);
		}

		inline
		void
		AddFieldBits(
			__in size_t NumBits,
			__in unsigned char FieldBits
			) throw( std::exception )
		{
			if (NumBits > 8)
				throw std::runtime_error( "Bitfield too large" );

			return AddFieldBits(
				NumBits,
				static_cast< unsigned __int64 >( FieldBits )
				);
		}

		inline
		void
		AddFieldBits(
			__in size_t NumBits,
			__in unsigned short FieldBits
			) throw( std::exception )
		{
			if (NumBits > 16)
				throw std::runtime_error( "Bitfield too large." );

			return AddFieldBits(
				NumBits,
				static_cast< unsigned __int64 >( FieldBits )
				);
		}

		inline
		void
		AddFieldBits(
			__in size_t NumBits,
			__in unsigned long FieldBits
			) throw( std::exception )
		{
			if (NumBits > 32)
				throw std::runtime_error( "Bitfield too large." );

			return AddFieldBits(
				NumBits,
				static_cast< unsigned __int64 >( FieldBits )
				);
		}

		virtual
		void
		AddFieldBits(
			__in size_t NumBits,
			__in unsigned __int64 FieldBits
			) throw( std::exception ) = 0;

		//
		// Retrieve a pointed to raw storage memory allocated for the current
		// buffer contents.  Any incomplete bitfields are defined to have zero
		// bits for incomplete bits present.
		//
		// The return data is invalidated if any data is written to the buffer
		// afterwards via one of the Add* routines.
		//

		virtual
		void
		GetBuffer(
			__out unsigned char *&Buffer,
			__out size_t &Length
			) = 0;

		virtual
		size_t
		GetBitPos( ) = 0;
	};

	class BufferBuilder : public IBufferBuilder
	{

	public:

		enum BitOrderMode
		{
			BitOrderLowToHigh,
			BitOrderHighToLow
		};

		explicit BufferBuilder(
			__in size_t SizeHint = 0,
			__in BitOrderMode BitOrder = BitOrderLowToHigh
			) throw( std::exception );

		virtual
		void
		AddData(
			__in size_t FieldLength,
			__in_bcount( FieldLength ) const void *Field
			) throw( std::exception );

		inline
		void
		AddFieldBits(
			__in size_t NumBits,
			__in unsigned char FieldBits
			) throw( std::exception )
		{
			if (NumBits > 8)
				throw std::runtime_error( "Bitfield too large" );

			return AddFieldBits(
				NumBits,
				static_cast< unsigned __int64 >( FieldBits )
				);
		}

		inline
		void
		AddFieldBits(
			__in size_t NumBits,
			__in unsigned short FieldBits
			) throw( std::exception )
		{
			if (NumBits > 16)
				throw std::runtime_error( "Bitfield too large." );

			return AddFieldBits(
				NumBits,
				static_cast< unsigned __int64 >( FieldBits )
				);
		}

		inline
		void
		AddFieldBits(
			__in size_t NumBits,
			__in unsigned long FieldBits
			) throw( std::exception )
		{
			if (NumBits > 32)
				throw std::runtime_error( "Bitfield too large." );

			return AddFieldBits(
				NumBits,
				static_cast< unsigned __int64 >( FieldBits )
				);
		}

		virtual
		void
		AddFieldBits(
			__in size_t NumBits,
			__in unsigned __int64 FieldBits
			) throw( std::exception );

		inline
		virtual
		void
		GetBuffer(
			__out unsigned char *&Buffer,
			__out size_t &Length
			)
		{
			if (m_Data.empty( ))
				Buffer = NULL;
			else
				Buffer = &m_Data[ 0 ];

			Length = m_Data.size();
		}

		inline
		virtual
		size_t
		GetBitPos( )
		{
			return m_BitPos;
		}
	private:

		inline
		void
		ExpandBuffer(
			__in size_t FieldLength
			) throw( std::exception )
		{
			size_t NewLength = m_Data.size() + FieldLength;

			if (NewLength < m_Data.size())
				throw std::runtime_error( "Buffer too large." );

			//
			// Grow twice as much as the new length so as to keep us linear if
			// we keep adding data in small chunks.
			//

			if (m_Data.capacity() < NewLength)
				m_Data.reserve( NewLength * 2 );

			m_Data.resize( NewLength );
		}

		std::vector< unsigned char > m_Data;
		size_t                       m_BitPos;
		BitOrderMode                 m_BitOrderMode;

	};

}

#endif

#ifdef _MSC_VER

#pragma warning(pop)

#endif
