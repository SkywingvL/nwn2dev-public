#include "Precomp.h"
#include "Compress.h"

using NWN::Compressor;

bool
Compressor::Uncompress(
	__in_bcount( Length ) const unsigned char * Data,
	__in size_t Length,
	__inout std::vector< unsigned char > & Plain
	)
/*++

Routine Description:

	This routine decompresses a single logical block in stateless mode.

Arguments:

	Data - Supplies the compressed data.

	Length - Supplies the length of the compressed data, in bytes.

	Plain - Receives the uncompressed data.  The buffer must have
	        already been resized to meet the uncompressed size
			requirement.  The caller guarantees that at least one byte
			of uncompressed data will be present.

Return Value:

	A Boolean value indicating true on success, else false on failure.

Environment:

	User mode.

--*/
{
	int   Status;
	uLong UncompressedSize;

	UncompressedSize = (uLong) Plain.size( );

	Status = uncompress(
		&Plain[ 0 ],
		&UncompressedSize,
		Data,
		(uLong) Length);

	if (Status != Z_OK)
		return false;

	Plain.resize( UncompressedSize );

	return true;
}

