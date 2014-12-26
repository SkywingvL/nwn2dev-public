/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	GrannyNative.h

Abstract:

	This module houses type definitions for the native file layout of a *.gr2
	file before GrannyConvertToRaw has transformed it.

--*/

#ifndef _PROGRAMS_GRANNY2LIB_GRANNYNATIVE_H
#define _PROGRAMS_GRANNY2LIB_GRANNYNATIVE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace GrannyNative
{
#include <pshpack1.h>

	struct GrnFileBaseHeader
	{
		ULONG Magic[ 4 ];
		ULONG HeaderSize;
		ULONG HeaderFormat;
		ULONG Reserved[ 2  ];
	};

	C_ASSERT( sizeof( GrnFileBaseHeader ) == 0x20 );

	struct GrnReference
	{
		ULONG SectionIndex;
		ULONG Offset;
	};

	C_ASSERT( sizeof( GrnReference ) == 0x8 );

	struct GrnSection
	{
		ULONG Format;
		ULONG DataOffset;
		ULONG DataSize;
		ULONG ExpandedDataSize;
		ULONG First16Bit;
		ULONG First8Bit;
		ULONG PointerFixupArrayOffset;
		ULONG PointerFixupArrayCount;
		ULONG MixedMarshallingFixupArrayOffset;
		ULONG MixedMarshallingFixupArrayCount;
	};

	C_ASSERT( sizeof( GrnSection ) == 0x2C );

	struct GrnPointerFixup
	{
		ULONG        FromOffset;
		GrnReference To;
	};

	C_ASSERT( sizeof( GrnPointerFixup ) == 0xC );

	struct GrnMixedMarshallingFixup
	{
		ULONG        Count;
		ULONG        Offset;
		GrnReference Type[ 2 ];
	};

	C_ASSERT( sizeof( GrnMixedMarshallingFixup ) == 0x10 );

	struct GrnFileHeader
	{
		GrnFileBaseHeader BaseHeader;
		ULONG             Version;
		ULONG             TotalSize;
		ULONG             CRC;
		ULONG             SectionArrayOffset;
		ULONG             SectionArrayCount;
		GrnReference      RootObjectTypeDefinition;
		GrnReference      RootObject;
		ULONG             TypeTag;
		ULONG             ExtraTags;
	};

	C_ASSERT( sizeof( GrnFileHeader ) == 0x38 + sizeof( GrnFileBaseHeader ) );

#include <poppack.h>
}

#endif
