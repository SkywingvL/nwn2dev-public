/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	DdsImage.h

Abstract:

	This module defines the DDS image reader object, which is used to read
	simple DDS images for purposes of terrain mesh access.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_DDSIMAGE_H
#define _PROGRAMS_NWN2DATALIB_DDSIMAGE_H

#ifdef _MSC_VER
#pragma once
#endif


namespace Trx
{
	typedef struct _DDS_PIXELFORMAT {
		unsigned int Size;
		unsigned int Flags;
		unsigned int FourCC;
		unsigned int RGBBitCount;
		unsigned int RBitMask;
		unsigned int GBitMask;
		unsigned int BBitMask;
		unsigned int ABitMask;
	} DDS_PIXELFORMAT;

	C_ASSERT( sizeof( DDS_PIXELFORMAT ) == 32 );

	typedef const struct _DDS_PIXELFORMAT * PCDDS_PIXELFORMAT;

	typedef struct _DDS_HEADER {
		unsigned long Size;
		unsigned long Flags;
		unsigned long Height;
		unsigned long Width;
		unsigned long LinearSize;
		unsigned long Depth;
		unsigned long MipMapCount;
		unsigned long Reserved1[11];
		DDS_PIXELFORMAT ddpf;
		unsigned long Caps;
		unsigned long Caps2;
		unsigned long Caps3;
		unsigned long Caps4;
		unsigned long Reserved2;
	} DDS_HEADER, * PDDS_HEADER;

	C_ASSERT( sizeof( DDS_HEADER ) == 124 );

	typedef const struct _DDS_HEADER * PCDDS_HEADER;

	typedef struct _DDS_FILE
	{
		unsigned long Magic;
		DDS_HEADER    Header;
	} DDS_FILE, * PCDDS_FILE;
}

class DdsImage
{

public:

	inline
	DdsImage(
		)
	{
		Clear( );
	}

	inline
	const Trx::DDS_FILE &
	GetDdsHeader(
		) const
	{
		return m_Header;
	}

	inline
	void
	SetDdsHeader(
		__in const Trx::DDS_FILE & Header
		)
	{
		m_Header = Header;
	}

	inline
	const std::vector< unsigned char > &
	GetImage(
		) const
	{
		return m_Image;
	}

	inline
	void
	SetImage(
		__in const void * ImageData,
		__in size_t Length
		)
	{
		m_Image.resize( Length );

		if (Length != 0)
			memcpy( &m_Image[ 0 ], ImageData, Length );
	}

	inline
	void
	Clear(
		)
	{
		ZeroMemory( &m_Header, sizeof( m_Header ) );

		m_Image.clear( );
	}

	inline
	void
	Validate(
		) const
	{
		size_t ExpectedSize;

		ExpectedSize = m_Header.Header.Width  *
		               m_Header.Header.Height *
		               (m_Header.Header.ddpf.RGBBitCount / 8);

		if (m_Image.size( ) != ExpectedSize)
			throw std::runtime_error( "DDS Image length mismatch." );
	}

	inline
	unsigned long
	GetColorChannel(
		__in int x,
		__in int y,
		__in unsigned long Channel
		) const
	{
		unsigned long * Row;
		unsigned long   Offset;
		unsigned long   PelBytes;
		unsigned long   Mask;
		unsigned long   Color;

		switch (Channel)
		{

		case 0:
			Mask = m_Header.Header.ddpf.RBitMask;
			break;

		case 1:
			Mask = m_Header.Header.ddpf.GBitMask;
			break;

		case 2:
			Mask = m_Header.Header.ddpf.BBitMask;
			break;

		case 3:
			Mask = m_Header.Header.ddpf.ABitMask;
			break;

		default:
			throw std::runtime_error( "Illegal color channel" );

		}

		//
		// Only simple 32-bpp formats are supported.
		//

		if ((m_Header.Header.ddpf.RGBBitCount != 32))
			throw std::runtime_error( "Only 32-bpp images are supported" );

		if ((x < 0) || (y < 0))
			throw std::runtime_error( "Out of range image x/y" );

		if (((unsigned long) x >= m_Header.Header.Height) ||
			((unsigned long) y >= m_Header.Header.Width))
		{
			throw std::runtime_error( "Out of range image x/y (2)" );
		}

		//
		// Fetch the alpha channel value out.
		//

		PelBytes = m_Header.Header.ddpf.RGBBitCount / 8;
		Offset   = y * m_Header.Header.Width * PelBytes;
		Row      = (unsigned long *) (&m_Image[ Offset ]);
		Color    = Row[ x ] & Mask;

		for (unsigned long Shift = 0; Shift < 32; Shift += 1)
		{
			if (Mask & (1 << Shift))
				return Color >> Shift;
		}

		return Color;
	}

private:

	Trx::DDS_FILE                m_Header;
	std::vector< unsigned char > m_Image;
//	std::vector< unsigned char > m_RValues;
//	std::vector< unsigned char > m_GValues;
//	std::vector< unsigned char > m_BValues;
//	std::vector< unsigned char > m_AValues;

};

#endif
