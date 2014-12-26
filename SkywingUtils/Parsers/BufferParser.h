#ifndef _SOURCE_PROGRAMS_SKYWINGUTILS_PARSERS_BUFFERPARSER_H
#define _SOURCE_PROGRAMS_SKYWINGUTILS_PARSERS_BUFFERPARSER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace swutil
{
	class IBufferParser
	{

	public:

		template< class T >
		inline
		bool
		GetField(
			__out T &Field
			)
		{
			return GetData(
				sizeof( T ),
				&Field
				);
		}

		virtual
		bool
		GetDataPtr(
			__in size_t FieldLength,
			__deref_out_bcount_full_opt( FieldLength ) const void **Field
			) = 0;

		virtual
		bool
		GetData(
			__in size_t FieldLength,
			__out_bcount_full_opt( FieldLength ) void *Field
			) = 0;

		inline
		bool
		GetFieldBit(
			__out bool &FieldBit
			)
		{
			unsigned __int64 Bits;

			if (!GetFieldBits( 1, Bits ))
				return false;

			FieldBit = Bits ? true : false;

			return true;
		}

		inline
		bool
		GetFieldBits(
			__in size_t NumBits,
			__out unsigned char &FieldBits
			)
		{
			unsigned __int64 Bits;

			if (NumBits > 8)
				return false;

			if (!GetFieldBits( NumBits, Bits ))
				return false;

			FieldBits = static_cast< unsigned char >( Bits );

			return true;
		}

		inline
		bool
		GetFieldBits(
			__in size_t NumBits,
			__out unsigned short &FieldBits
			)
		{
			unsigned __int64 Bits;

			if (NumBits > 16)
				return false;

			if (!GetFieldBits( NumBits, Bits ))
				return false;

			FieldBits = static_cast< unsigned short >( Bits );

			return true;
		}

		inline
		bool
		GetFieldBits(
			__in size_t NumBits,
			__out unsigned long &FieldBits
			)
		{
			unsigned __int64 Bits;

			if (NumBits > 32)
				return false;

			if (!GetFieldBits( NumBits, Bits ))
				return false;

			FieldBits = static_cast< unsigned long >( Bits );

			return true;
		}

		virtual
		bool
		GetFieldBits(
			__in size_t NumBits,
			__out unsigned __int64 &FieldBits
			) = 0;

		virtual
		bool
		AtEndOfStream(
			) const = 0;

		//
		// Debugging routine to get the current byte position.
		//

		virtual
		size_t
		GetBytePos(
			) const = 0;

		//
		// Debugging routine to get the base data pointer for the parser.
		//

		virtual
		const unsigned char *
		GetBaseDataPointer(
			) const = 0;

		//
		// Debugging routine to get the current bit position.
		//

		virtual
		size_t
		GetBitPos(
			) const = 0;

		//
		// Debugging routine to get the highest legal bit number in the last
		// byte of the bit stream.
		//

		virtual
		size_t
		GetHighestValidBitPos(
			) const = 0;

		virtual
		inline
		size_t
		GetBytesRemaining(
			) const = 0;

		virtual
		bool
		SkipData(
			__in size_t FieldLength
			) = 0;

		virtual
		bool
		SkipBits(
			__in size_t NumBits
			) = 0;

		virtual
		void
		Reset(
			) = 0;

		virtual
		void
		SetHighestValidBitPos(
			__in size_t HighestValidBitPos
			) = 0;

		virtual
		void
		RebaseBuffer(
			__in const void *Data
			) = 0;

	};

	class BufferParser : public IBufferParser
	{

	public:

		enum BitOrderMode
		{
			BitOrderLowToHigh,
			BitOrderHighToLow
		};

		BufferParser(
			__in_bcount( Length ) const void *Data,
			__in size_t Length,
			__in BitOrderMode BitOrder = BitOrderLowToHigh
			);

		~BufferParser( );

		virtual
		bool
		GetDataPtr(
			__in size_t FieldLength,
			__deref_opt_out const void **Field
			);

		virtual
		bool
		GetData(
			__in size_t FieldLength,
			__out_bcount_full_opt( FieldLength ) void *Field
			);

		inline
		bool
		GetFieldBits(
			__in size_t NumBits,
			__out unsigned char &FieldBits
			)
		{
			unsigned __int64 Bits;

			if (NumBits > 8)
				return false;

			if (!GetFieldBits( NumBits, Bits ))
				return false;

			FieldBits = static_cast< unsigned char >( Bits );

			return true;
		}

		inline
		bool
		GetFieldBits(
			__in size_t NumBits,
			__out unsigned short &FieldBits
			)
		{
			unsigned __int64 Bits;

			if (NumBits > 16)
				return false;

			if (!GetFieldBits( NumBits, Bits ))
				return false;

			FieldBits = static_cast< unsigned short >( Bits );

			return true;
		}

		inline
		bool
		GetFieldBits(
			__in size_t NumBits,
			__out unsigned long &FieldBits
			)
		{
			unsigned __int64 Bits;

			if (NumBits > 32)
				return false;

			if (!GetFieldBits( NumBits, Bits ))
				return false;

			FieldBits = static_cast< unsigned long >( Bits );

			return true;
		}

		virtual
		bool
		GetFieldBits(
			__in size_t NumBits,
			__out unsigned __int64 &FieldBits
			);

		virtual
		inline
		bool
		AtEndOfStream(
			) const
		{
			//
			// If we are in byte mode, just check if we've got any bytes left.
			//

			if (m_BitPos == 8)
				return (m_DataPosRemaining == 0);

			//
			// DataPosRemaining should never be 0 as we have a byte that is
			// still being worked on by definition if we are in bit mode.
			//

			//
			// Otherwise we are in bit mode.  We may have a byte left but have
			// zero valid bits left within it, so check that.
			//

			size_t BitsThisByte  = ((m_DataPosRemaining > 1) ? 8 : m_HighestValidBitPos);
			size_t BitsRemaining = BitsThisByte - ((m_BitPos == 8) ? 0 : m_BitPos);

			return (BitsRemaining == 0);
		}

		virtual
		inline
		size_t
		GetBytePos(
			) const
		{
			return m_DataPos - m_Data;
		}

		virtual
		inline
		const unsigned char *
		GetBaseDataPointer(
			) const
		{
			return m_Data;
		}

		virtual
		inline
		size_t
		GetBitPos(
			) const
		{
			return m_BitPos;
		}

		virtual
		inline
		size_t
		GetHighestValidBitPos(
			) const
		{
			return m_HighestValidBitPos;
		}

		virtual
		inline
		size_t
		GetBytesRemaining(
			) const
		{
			return (m_DataPosRemaining);
		}

		virtual
		bool
		SkipData(
			__in size_t FieldLength
			);

		virtual
		bool
		SkipBits(
			__in size_t NumBits
			);

		virtual
		void
		Reset(
			);

		virtual
		void
		SetHighestValidBitPos(
			__in size_t HighestValidBitPos
			);

		virtual
		void
		RebaseBuffer(
			__in const void *Data
			);

	private:

		//
		// Initial values for parsing reset.
		//

		const unsigned char *m_Data;
		size_t               m_DataLength;

		//
		// Current position and remaining data.
		//

		const unsigned char *m_DataPos;
		size_t               m_DataPosRemaining;
		size_t               m_BitPos;
		size_t               m_HighestValidBitPos;
		BitOrderMode         m_BitOrderMode;

	};

}

#endif


