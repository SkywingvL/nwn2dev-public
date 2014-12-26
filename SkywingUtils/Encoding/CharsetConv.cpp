#include "Precomp.h"
#include "CharsetConv.h"


//
// Convert 8-bit characters from the given codepage to UTF-8.
// Text and Result MAY be the same buffer.
//
bool
swutil::UTF8Encode(
	__in const std::string & Text,
	__out std::string & Result,
	__in UINT Codepage /* = CP_ACP */
	)
{
	try
	{
		//
		// We need to first convert input to Unicode before we can convert it
		// to UTF-8...
		//

		int    InputChars   = static_cast< int >( Text.size() );
		int    UnicodeChars = MultiByteToWideChar(
			Codepage,
			0,
			Text.data(),
			InputChars,
			0,
			0
			);

		if (!UnicodeChars)
			return false;

		std::vector< wchar_t > UnicodeBuffer( UnicodeChars );

		if (!MultiByteToWideChar(
			Codepage,
			0,
			Text.data(),
			InputChars,
			&UnicodeBuffer[ 0 ],
			UnicodeChars))
			return false;

		//
		// Now that we've got everything translated to Unicode, we can
		// (finally) convert it to UTF-8.
		//

		int UTF8Chars = WideCharToMultiByte(
			CP_UTF8,
			0,
			&UnicodeBuffer[ 0 ],
			UnicodeChars,
			0,
			0,
			0,
			0
			);

		if (!UTF8Chars)
			return false;

		std::vector< char > UTF8Buffer( UTF8Chars );

		if (!WideCharToMultiByte(
			CP_UTF8,
			0,
			&UnicodeBuffer[ 0 ],
			UnicodeChars,
			&UTF8Buffer[ 0 ],
			UTF8Chars,
			0,
			0))
			return false;

		//
		// Store it in the std::string passed in.
		//

		Result.assign( &UTF8Buffer[ 0 ], UTF8Chars );

		return true;
	}
	catch (std::bad_alloc)
	{
		return false;
	}
}

//
// Convert 16-bit characters to UTF-8.
//
bool
swutil::UTF8Encode(
	__in const std::wstring & Text,
	__out std::string & Result
	)
{
	try
	{
		int InputChars = static_cast< int >( Text.size() );
		int UTF8Chars  = WideCharToMultiByte(
			CP_UTF8,
			0,
			Text.data(),
			InputChars,
			0,
			0,
			0,
			0
			);

		if (!UTF8Chars)
			return false;

		std::vector< char > UTF8Buffer( UTF8Chars );

		if (!WideCharToMultiByte(
			CP_UTF8,
			0,
			Text.data(),
			InputChars,
			&UTF8Buffer[ 0 ],
			UTF8Chars,
			0,
			0))
			return false;

		//
		// Store it in the std::string passed in.
		//

		Result.assign( &UTF8Buffer[ 0 ], UTF8Chars );

		return true;
	}
	catch (std::bad_alloc)
	{
		return false;
	}
}

//
// Convert UTF-8 characters to 16-bit characters.
//
bool
swutil::UTF8Decode(
	__in const std::string & UTF8,
	__out std::wstring & Result
	)
{
	try
	{
		//
		// Convert to UTF-8.
		//

		int InputBytes   = static_cast< int >( UTF8.size() );
		int UnicodeChars = MultiByteToWideChar(
			CP_UTF8,
			0,
			UTF8.data(),
			InputBytes,
			0,
			0
			);

		if (!UnicodeChars)
			return false;

		std::vector< wchar_t > UnicodeBuffer( UnicodeChars );

		if (!MultiByteToWideChar(
			CP_UTF8,
			0,
			UTF8.data(),
			InputBytes,
			&UnicodeBuffer[ 0 ],
			UnicodeChars))
			return false;

		//
		// We're done - we wanted 16-bit characters.
		//

		Result.assign( &UnicodeBuffer[ 0 ], UnicodeChars );

		return true;
	}
	catch (std::bad_alloc)
	{
		return false;
	}
}

//
// Convert 8-bit characters to 16-bit characters using the given codepage.
//
bool
swutil::AnsiToUnicode(
	__in const std::string & Ansi,
	__out std::wstring & Result,
	__in UINT Codepage /* = CP_ACP */
	)
{
	try
	{
		int InputCharacters = static_cast< int >( Ansi.size() );
		int UnicodeChars    = MultiByteToWideChar(
			Codepage,
			0,
			Ansi.data(),
			InputCharacters,
			0,
			0
			);

		if (!UnicodeChars)
			return false;

		std::vector< wchar_t > UnicodeBuffer( UnicodeChars );

		if (!MultiByteToWideChar(
			Codepage,
			0,
			Ansi.data(),
			InputCharacters,
			&UnicodeBuffer[ 0 ],
			UnicodeChars))
			return false;

		Result.assign( &UnicodeBuffer[ 0 ], UnicodeChars );

		return true;
	}
	catch (std::bad_alloc)
	{
		return false;
	}
}

//
// Convert 16-bit characters to 8-bit characters using the given codepage.
//
bool
swutil::UnicodeToAnsi(
	__in const std::wstring & Unicode,
	__out std::string & Result,
	__in UINT Codepage /* = CP_ACP */
	)
{
	try
	{
		int InputCharacters = static_cast< int >( Unicode.size() );
		int AnsiChars       = WideCharToMultiByte(
			Codepage,
			0,
			Unicode.data(),
			InputCharacters,
			0,
			0,
			0,
			0
			);

		if (!AnsiChars)
			return false;

		std::vector< char > AnsiBuffer( AnsiChars );

		if (!WideCharToMultiByte(
			Codepage,
			0,
			Unicode.data(),
			InputCharacters,
			&AnsiBuffer[ 0 ],
			AnsiChars,
			0,
			0))
			return false;

		Result.assign( &AnsiBuffer[ 0 ], AnsiChars );

		return true;
	}
	catch (std::bad_alloc)
	{
		return false;
	}
}
