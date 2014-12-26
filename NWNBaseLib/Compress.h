#ifdef _MSC_VER

#pragma once

#pragma warning(push)
#pragma warning(disable:4290) // warning C4290: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)

#endif

#ifndef _NWNCONNLIB_COMPRESS_COMPRESS_H
#define _NWNCONNLIB_COMPRESS_COMPRESS_H

#endif

namespace NWN
{

	//
	// Define the simple zlib compressor wrapper.
	//

	class Compressor
	{

	public:

		inline
		Compressor(
			)
		{
		}

		inline
		virtual
		~Compressor(
			)
		{
		}

		//
		// Perform stateless decompression.
		//

		bool
		Uncompress(
			__in_bcount( Length ) const unsigned char * Data,
			__in size_t Length,
			__inout std::vector< unsigned char > & Plain
			);

	private:

	};
}
