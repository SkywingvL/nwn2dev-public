/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    BufferBuilder.cpp

Abstract:

    This module contains the implementation for the BufferBuilder class.

--*/

#include "Precomp.h"
#include "BufferBuilder.h"

#ifdef _MSC_VER

#pragma warning(push)
#pragma warning(disable:4290) // warning C4290: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)

#endif

using swutil::BufferBuilder;

BufferBuilder::BufferBuilder(
	__in size_t SizeHint, /* = 0 */
	__in BitOrderMode BitOrder /* = BitOrderLowToHigh */
	) throw( std::exception )
	: m_BitPos( 8 ),
	  m_BitOrderMode( BitOrder )
{
	if (SizeHint)
		m_Data.reserve( SizeHint );
}

void
BufferBuilder::AddData(
	__in size_t FieldLength,
	__in_bcount( FieldLength ) const void *Field
	) throw( std::exception )
{
	if (m_BitPos != 8)
		throw std::runtime_error( "Incomplete bitfield." );

	size_t Pos = m_Data.size();

	ExpandBuffer( FieldLength );

	memcpy( &m_Data[ Pos ], Field, FieldLength );
}

void
BufferBuilder::AddFieldBits(
	__in size_t NumBits,
	__in unsigned __int64 FieldBits
	) throw( std::exception )
{
	if (NumBits > 64)
		throw std::runtime_error( "Bitfield too large." );

	//
	// In the event that we are adding the first bit, we need to create the
	// bit buffer now.  Normally we defer this so that we can differentiate
	// between the cases of no bit buffer present and bit field items having
	// been added.
	//

	if ((m_Data.empty( )) && (m_BitPos == 8))
	{
		ExpandBuffer( 1 );

		m_BitPos = 0;
	}

	size_t BitsRemaining = 8 - m_BitPos;
	size_t CurrInBit = 0;
	size_t Pos = m_Data.size() - 1;
	bool   PosAdvanced = false;

	if (NumBits > BitsRemaining)
	{
		size_t BitsExtra = NumBits - BitsRemaining;
		size_t BytesRequired = (BitsExtra / 8) + ((BitsExtra % 8) ? 1 : 0);

		ExpandBuffer( BytesRequired );
	}

	while (CurrInBit < NumBits)
	{
		//
		// If we haven't claimed any data for bit writing yet, then we shall
		// need to do so first.  Note that after having done this we need to
		// keep on writing bits until we finish with a byte before the normal
		// byte-oriented write operations will function.
		//
		// N.B. We initially zero any byte that we claim for bit writing so
		// that incomplete bitfields are defined to have zero bits for any
		// uninitialized data when we finalize the buffer.  This also allows us
		// to simply or in the 1-bits and depend on the existing buffer state.
		//

		if (m_BitPos == 8)
		{
			//
			// If we are starting a new call to AddFieldBits then we need to
			// know to skip forward to the next byte here.  Otherwise, we would
			// overwrite the previous byte's contents.  Do not do this if we
			// are still operating in the same call that had already positioned
			// Pos appropriately after a byte switch however.
			//

			if (!PosAdvanced)
				Pos += 1;

			m_BitPos      = 0;

			m_Data[ Pos ] = 0;
		}

		//
		// Grab the current bit.
		//

		unsigned char CurrBit = static_cast< unsigned char >( (FieldBits >> ((NumBits - CurrInBit) - 1)) & 1 );
//		unsigned char CurrBit = static_cast< unsigned char >( (FieldBits >> CurrInBit) & 1 );

		//
		// Or it into the current bitfield byte.
		//

		switch (m_BitOrderMode)
		{

		case BitOrderLowToHigh:
			m_Data[ Pos ] |= CurrBit << (    m_BitPos);
			break;

		case BitOrderHighToLow:
			m_Data[ Pos ] |= CurrBit << (7 - m_BitPos);
			break;

		default:
#ifdef _MSC_VER
			__assume( 0 );
#endif
			break;

		}

		CurrInBit += 1;
		m_BitPos  += 1;

		//
		// If we're at the last bit in this byte, then increment the current
		// write position, but don't claim the first bit just yet.  This
		// allows us to use byte-level addressing until we're called to write a
		// byte quantity once more.
		//

		if (m_BitPos == 8)
		{
			Pos += 1;
			PosAdvanced = true;
		}
	}
}


#ifdef _MSC_VER

#pragma warning(pop)

#endif

