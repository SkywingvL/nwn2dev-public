/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptUtilities.cpp

Abstract:

	This module houses utility routine for the NWScript code generator and
	NWScript runtime environment.

--*/

#include "Precomp.h"
#include "NWScriptUtilities.h"


std::string
NWScriptUtilities::ConvertString(
	__in String ^ Str
	)
/*++

Routine Description:

	This routine converts a System::String to an std::string.

Arguments:

	Str - Supplies the string to convert.

Return Value:

	The converted std::string is returned.  On failure, a System::Exception, may
	be raised.

Environment:

	User mode, C++/CLI.

--*/
{
	#if 0
	using namespace Runtime::InteropServices;

	std::string  Sstr;
	const char * Chars = (const char *) (Marshal::StringToHGlobalAnsi( Str )).ToPointer( );
   
	try
	{
		Sstr = Chars;
	}
	catch (std::exception)
	{
		Marshal::FreeHGlobal( (IntPtr)( (void *) Chars) );

		throw gcnew Exception( "Failed to convert string." );
	}

	Marshal::FreeHGlobal( (IntPtr)( (void *) Chars) );

	return Sstr;
	#endif

	return msclr::interop::marshal_as< std::string >( Str );
}

namespace NWScript
{

	ref class NWEncoderFallbackBuffer : public EncoderFallbackBuffer
	{

	public:

		//
		// Standard constructor.
		//

		NWEncoderFallbackBuffer(
			)
		{
			Reset( );
		}

		//
		// Get the remaining bytes left to process in the buffer.
		//

		virtual
		property
		int
		Remaining
		{
			int
			get(
				) override
			{
				if (m_FallbackString == nullptr)
					return 0;

				return m_FallbackString->Length - m_Position;
			}
		}

		//
		// Set up for fallback processing given a single character.
		//

		virtual
		bool
		Fallback(
			__in System::Char CharUnknown,
			__in int Index
			) override
		{
			unsigned short Value;
			int            Length;

			UNREFERENCED_PARAMETER( Index );

			Value  = (unsigned short) CharUnknown;
			Length = 1;

			if ((Value & 0xFF00) != 0)
				Length += 1;

			m_FallbackString = gcnew array< Byte >( Length );

			m_FallbackString[ 0 ] = (Value & 0x00FF) >> 0;

			if (Length >= 2)
				m_FallbackString[ 1 ] = (Value & 0xFF00) >> 8;

			m_Position = 0;

			return true;
		}

		//
		// Set up for fallback processing given a surrogate pair.
		//

		virtual
		bool
		Fallback(
			__in System::Char CharUnknownHigh,
			__in System::Char CharUnknownLow,
			__in int Index
			) override
		{
			unsigned short Value;
			int            Length;
			int            i;

			UNREFERENCED_PARAMETER( Index );

			Value  = (unsigned short) CharUnknownHigh;
			Length = 2;

			if ((Value & 0xFF00) != 0)
				Length += 1;

			Value = (unsigned short) CharUnknownLow;

			if ((Value & 0xFF00) != 0)
				Length += 1;

			m_FallbackString = gcnew array< Byte >( Length );
			Value            = (unsigned short) CharUnknownHigh;
			i                = 0;

			m_FallbackString[ i++ ] = (Value & 0x00FF) >> 0;

			if ((Value & 0xFF00) != 0)
				m_FallbackString[ i++ ] = (Value & 0xFF00) >> 8;

			Value = (unsigned short) CharUnknownLow;

			m_FallbackString[ i++ ] = (Value & 0x00FF) >> 0;

			if ((Value & 0xFF00) != 0)
				m_FallbackString[ i++ ] = (Value & 0xFF00) >> 8;

			m_Position = 0;

			return true;
		}

		//
		// Get the next character.
		//

		virtual
		System::Char
		GetNextChar(
			) override
		{
			if (m_Position < Remaining)
				return (System::Char) (unsigned short) m_FallbackString[ m_Position++ ];
			else
				return (System::Char) 0;
		}

		//
		// Back up to the previous character.
		//

		virtual
		bool
		MovePrevious(
			) override
		{
			if (m_Position == 0)
				return false;

			m_Position -= 1;
			return true;
		}

		//
		// Reset the encoder to a clean state.
		//

		virtual
		void
		Reset(
			) override
		{
			m_FallbackString = nullptr;
			m_Position = 0;
		}

		//
		// Convert to a text string.
		//

		virtual
		System::String ^
		ToString(
			) override
		{
			return m_FallbackString->ToString( );
		}

	private:

		//
		// The current string that we are processing.
		//

		array< Byte > ^ m_FallbackString;

		//
		// The index into the string that we're at now.
		//

		int             m_Position;

	};

	ref class NWEncoderFallback : public EncoderFallback
	{

	public:

		//
		// Standard constructor.
		//

		NWEncoderFallback(
			)
		{
		}

		//
		// Create the associated NWEncoderFallbackBuffer object.
		//

		virtual
		EncoderFallbackBuffer ^
		CreateFallbackBuffer(
			) override
		{
			return gcnew NWEncoderFallbackBuffer;
		}

		//
		// We can expand a surrogate pair into four characters that are the
		// ordinal bytes of the surrogate pair.  (Two bytes per character.)
		//

		property int MaxCharCount { virtual int get( ) override { return 4; } }
	};

	ref class NWDecoderFallbackBuffer : public DecoderFallbackBuffer
	{

	public:

		//
		// Standard constructor.
		//

		NWDecoderFallbackBuffer(
			)
		{
			Reset( );
		}

		//
		// Get the remaining bytes left to process in the buffer.
		//

		virtual
		property
		int
		Remaining
		{
			int
			get(
				) override
			{
				if (m_FallbackString == nullptr)
					return 0;

				return (m_EndPosition - m_Position);
			}
		}

		//
		// Set up for fallback processing given a single character.
		//

		virtual
		bool
		Fallback(
			__in array< Byte > ^ BytesUnknown,
			__in int Index
			) override
		{
			int SequenceLength;

			UNREFERENCED_PARAMETER( Index );

			m_Position       = 0;
			m_FallbackString = BytesUnknown;
			SequenceLength   = 0;

			if (BytesUnknown->Length == 0)
				return false;

			//
			// Scan forward for the end position.
			//

			for (m_EndPosition = 0;
			     m_EndPosition < BytesUnknown->Length;
			     m_EndPosition += 1)
			{
				Byte Value = BytesUnknown[ m_EndPosition ];

				//
				// If we've reached a code point without the high bit set then
				// we've reached the start of a new single byte code point
				// sequence (which is, by definition, valid).
				//

				if ((Value & 0x80) == 0)
				{
					SequenceLength = 1;
					break;
				}

				//
				// Otherwise, check for a lead byte of the next multi-byte code
				// point sequence.
				//
				// N.B.  The leader byte values cannot appear within the body
				//       of a sequence, allowing us to safely find the next
				//       code point sequence.
				//
				// N.B.  We don't have to check if the next sequence is, in
				//       itself, invalid.  If it turns out to be so, we will
				//       just 
				//

				else if ((Value & 0xE0) == 0xC0) // Two byte sequence
				{
					SequenceLength = 2;
					break;
				}
				else if ((Value & 0xF0) == 0xE0) // Three byte sequence
				{
					SequenceLength = 3;
					break;
				}
				else if ((Value & 0xF8) == 0xF0) // Four byte sequence
				{
					SequenceLength = 4;
					break;
				}
				else if ((Value & 0xFC) == 0xF8) // Five byte sequence
				{
					SequenceLength = 5;
					break;
				}
				else if ((Value & 0xFE) == 0xFC) // Six byte sequence
				{
					SequenceLength = 6;
					break;
				}
				else // Not a leader byte, keep on scanning forward.
				{
					continue;
				}
			}

			//
			// If we started out at the start of a sequence, then the entire
			// sequence must be invalid (for example, we had an input string
			// that wasn't ever valid UTF-8 to begin with, and not just one
			// that we started in the middle of).
			//
			// In that case, just consume the entire bad sequence.
			//

			if (m_EndPosition == m_Position)
				m_EndPosition = min( SequenceLength, BytesUnknown->Length );

			return true;
		}

		//
		// Get the next character.
		//

		virtual
		System::Char
		GetNextChar(
			) override
		{
			if (m_Position < Remaining)
				return (System::Char) (unsigned short) m_FallbackString[ m_Position++ ];
			else
				return (System::Char) 0;
		}

		//
		// Back up to the previous character.
		//

		virtual
		bool
		MovePrevious(
			) override
		{
			if (m_Position == 0)
				return false;

			m_Position -= 1;
			return true;
		}

		//
		// Reset the encoder to a clean state.
		//

		virtual
		void
		Reset(
			) override
		{
			m_FallbackString = nullptr;
			m_Position = 0;
		}

		//
		// Convert to a text string.
		//

		virtual
		System::String ^
		ToString(
			) override
		{
			return m_FallbackString->ToString( );
		}

	private:

		//
		// The current string that we are processing.
		//

		array< Byte > ^ m_FallbackString;

		//
		// The index into the string that we're at now.
		//

		int             m_Position;

		//
		// The index that is the end of the invalid code point range.
		//

		int             m_EndPosition;

	};

	ref class NWDecoderFallback : public DecoderFallback
	{

	public:

		//
		// Standard constructor.
		//

		NWDecoderFallback(
			)
		{
		}

		//
		// Create the associated NWDecoderFallbackBuffer object.
		//

		virtual
		DecoderFallbackBuffer ^
		CreateFallbackBuffer(
			) override
		{
			return gcnew NWDecoderFallbackBuffer;
		}

		//
		// We simply map an undecodable byte into a System::Char with the
		// same ordinal value as the byte itself.  This is continued until we
		// reach the end of the current (invalid) UTF-8 sequence, which may be
		// at most six bytes long including the leader byte.
		//
		// Thus, we can map an invalid sequence into at most six System::Char
		// characters.
		//

		property int MaxCharCount { virtual int get( ) override { return 6; } }
	};



	ref class NWTruncateTo8BitEncoderFallbackBuffer : public EncoderFallbackBuffer
	{

	public:

		//
		// Standard constructor.
		//

		NWTruncateTo8BitEncoderFallbackBuffer(
			)
		{
			Reset( );
		}

		//
		// Get the remaining bytes left to process in the buffer.
		//

		virtual
		property
		int
		Remaining
		{
			int
			get(
				) override
			{
				return 1 - m_Position;
			}
		}

		//
		// Set up for fallback processing given a single character.
		//

		virtual
		bool
		Fallback(
			__in System::Char CharUnknown,
			__in int Index
			) override
		{
			UNREFERENCED_PARAMETER( Index );

			if (CharUnknown > 0xFF)
				CharUnknown = '?';

			m_FallbackString = (Byte) CharUnknown;
			m_Position       = 0;

			return true;
		}

		//
		// Set up for fallback processing given a surrogate pair.
		//

		virtual
		bool
		Fallback(
			__in System::Char CharUnknownHigh,
			__in System::Char CharUnknownLow,
			__in int Index
			) override
		{
			UNREFERENCED_PARAMETER( CharUnknownHigh );
			UNREFERENCED_PARAMETER( CharUnknownLow );
			UNREFERENCED_PARAMETER( Index );

			m_FallbackString = (Byte) '?';
			m_Position       = 0;

			return true;
		}

		//
		// Get the next character.
		//

		virtual
		System::Char
		GetNextChar(
			) override
		{
			if (m_Position < Remaining)
			{
				m_Position += 1;
				return (System::Char) m_FallbackString;
			}
			else
			{
				return (System::Char) 0;
			}
		}

		//
		// Back up to the previous character.
		//

		virtual
		bool
		MovePrevious(
			) override
		{
			if (m_Position == 0)
				return false;

			m_Position -= 1;
			return true;
		}

		//
		// Reset the encoder to a clean state.
		//

		virtual
		void
		Reset(
			) override
		{
			m_FallbackString = (Byte) 0;
			m_Position       = 0;
		}

		//
		// Convert to a text string.
		//

		virtual
		System::String ^
		ToString(
			) override
		{
			return m_FallbackString.ToString( );
		}

	private:

		//
		// The current string that we are processing.
		//

		Byte            m_FallbackString;

		//
		// The index into the string that we're at now.
		//

		int             m_Position;

	};

	ref class NWTruncateTo8BitEncoderFallback : public EncoderFallback
	{

	public:

		//
		// Standard constructor.
		//

		NWTruncateTo8BitEncoderFallback(
			)
		{
		}

		//
		// Create the associated NWTruncateTo8BitEncoderFallbackBuffer object.
		//

		virtual
		EncoderFallbackBuffer ^
		CreateFallbackBuffer(
			) override
		{
			return gcnew NWTruncateTo8BitEncoderFallbackBuffer;
		}

		//
		// Each character is truncated to 8 bits.  A surrogate pair is
		// truncated to a QUESTION MARK.
		//

		property int MaxCharCount { virtual int get( ) override { return 1; } }
	};

	ref class NWTruncateTo8BitDecoderFallbackBuffer : public DecoderFallbackBuffer
	{

	public:

		//
		// Standard constructor.
		//

		NWTruncateTo8BitDecoderFallbackBuffer(
			)
		{
			Reset( );
		}

		//
		// Get the remaining bytes left to process in the buffer.
		//

		virtual
		property
		int
		Remaining
		{
			int
			get(
				) override
			{
				if (m_FallbackString == nullptr)
					return 0;

				return (m_FallbackString->Length - m_Position);
			}
		}

		//
		// Set up for fallback processing given a single character.
		//

		virtual
		bool
		Fallback(
			__in array< Byte > ^ BytesUnknown,
			__in int Index
			) override
		{
			UNREFERENCED_PARAMETER( Index );

			m_Position       = 0;
			m_FallbackString = BytesUnknown;

			if (BytesUnknown->Length == 0)
				return false;

			return true;
		}

		//
		// Get the next character.
		//

		virtual
		System::Char
		GetNextChar(
			) override
		{
			if (m_Position < Remaining)
				return (System::Char) (Byte) m_FallbackString[ m_Position++ ];
			else
				return (System::Char) 0;
		}

		//
		// Back up to the previous character.
		//

		virtual
		bool
		MovePrevious(
			) override
		{
			if (m_Position == 0)
				return false;

			m_Position -= 1;
			return true;
		}

		//
		// Reset the encoder to a clean state.
		//

		virtual
		void
		Reset(
			) override
		{
			m_FallbackString = nullptr;
			m_Position = 0;
		}

		//
		// Convert to a text string.
		//

		virtual
		System::String ^
		ToString(
			) override
		{
			return m_FallbackString->ToString( );
		}

	private:

		//
		// The current string that we are processing.
		//

		array< Byte > ^ m_FallbackString;

		//
		// The index into the string that we're at now.
		//

		int             m_Position;

	};

	ref class NWTruncateTo8BitDecoderFallback : public DecoderFallback
	{

	public:

		//
		// Standard constructor.
		//

		NWTruncateTo8BitDecoderFallback(
			)
		{
		}

		//
		// Create the associated NWTruncateTo8BitDecoderFallbackBuffer object.
		//

		virtual
		DecoderFallbackBuffer ^
		CreateFallbackBuffer(
			) override
		{
			return gcnew NWTruncateTo8BitDecoderFallbackBuffer;
		}

		//
		// We simply each byte into a System::Char with the same ordinal value.
		//

		property int MaxCharCount { virtual int get( ) override { return 1; } }
	};


}

void
NWScriptUtilities::SetupEncoding(
	)
/*++

Routine Description:

	This routine sets up the encoders for use with the NWScript JIT system.

	This is necessary to support (broken) scripts that perform an operation
	like this, which treats a UTF-8 sequence as a series of independent bytes
	that can be broken apart individually:

	while (GetStringLength(s) > 0)
	{
	    s = GetStringRight(s, GetStringLength(s)-1);
	}

	Without a truncating fallback, when such a loop encounters an invalid UTF-8
	code point, GetStringRight would fail to actually decrease the length of
	the string, as the Unicode replacement character U+FFFD would be inserted 
	whenever an invalid code point byte was encountered.  This would add
	another byte to the string and cause the loop to enter into a runaway cycle
	of allocating longer and longer strings.

	                                       

	N.B.  This represents a difference in behavior between the JIT engine and
	      the standard VM when invalid UTF-8 code points are processed.  The
	      VM will not transform invalid code points into U+003F's, but will
	      leave them as-is.  We cannot do this, as invalid code points, when
	      upcasted to their ordinal values, would re-encode into more than one
	      byte.

	N.B.  No synchronization is provided, but GC makes this not an issue as we
	      would just double-create a UTF8Encoding object that would then get
	      cleaned up by GC later on anyway.

Arguments:

	None.

Return Value:

	None.  On failure, a System::Exception, may be raised.

Environment:

	User mode, C++/CLI.

--*/
{
	System::Text::Encoding ^ Encoder = (System::Text::Encoding ^) System::Text::Encoding::ASCII->Clone( );

	Encoder->DecoderFallback = gcnew NWTruncateTo8BitDecoderFallback;
	Encoder->EncoderFallback = gcnew NWTruncateTo8BitEncoderFallback;

	UTF8FallbackEncoding = Encoding::UTF8;
	TruncateTo8BitEncoding = Encoder;

	return;
}

#pragma managed(push, off)
void
NWScript::VMStackPushEngineStructureInternal(
	__in INWScriptStack * Stack,
	__in EngineStructurePtr * EngStruct
	)
/*++

Routine Description:

	This unmanaged routine consolidates the various unmanaged routine calls
	that are necessary to push an engine structure onto the VM stack.  It is
	implemented here, in unmanaged code, to avoid the expensive multiple
	reverse P/Invokes associated with the EngineStructurePtr copy constructor.

Arguments:

	Stack - Supplies the VM stack to push the value onto.

	EngStruct - Supplies the value to push onto the VM stack.

Return Value:

	None.  On failure, a std::exception is raised.

Environment:

	User mode, unmanaged code.

--*/
{
	Stack->StackPushEngineStructure( *EngStruct );
}

#pragma managed(pop)
