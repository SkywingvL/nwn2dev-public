/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	FileWrapper.h

Abstract:

	This module defines the file wrapper object, which provides various common
	wrapper operations for file I/O.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_FILEWRAPPER_H
#define _PROGRAMS_NWN2DATALIB_FILEWRAPPER_H

#ifdef _MSC_VER
#pragma once
#endif

class FileWrapper
{

public:

	inline
	FileWrapper(
		__in HANDLE File = INVALID_HANDLE_VALUE
		)
		: m_File( File ),
		  m_View( NULL ),
		  m_Offset( 0 ),
		  m_Size( 0 ),
		  m_ExternalView( false )

	{
		if (File != INVALID_HANDLE_VALUE)
			m_Offset = GetFilePointer( );
	}

	inline
	~FileWrapper(
		)
	{
		if ((m_View != NULL) && (!m_ExternalView))
		{
			UnmapViewOfFile( m_View );

			m_View = NULL;
		}
		else if (m_View != NULL)
		{
			m_View = NULL;
		}
	}

	inline
	void
	SetFileHandle(
		__in HANDLE File,
		__in bool AsSection = true
		)
	{
		m_File         = File;
		m_ExternalView = false;

		if (m_View != NULL)
		{
			UnmapViewOfFile( m_View );

			m_View = NULL;
		}

		if ((m_File != INVALID_HANDLE_VALUE) &&
		    (AsSection))
		{
			HANDLE Section;

			Section = CreateFileMapping(
				m_File,
				NULL,
				PAGE_READONLY,
				0,
				0,
				NULL);

			if (Section != NULL)
			{
				unsigned char * View;

				View = (unsigned char *) MapViewOfFile(
					Section,
					FILE_MAP_READ,
					0,
					0,
					0);
				CloseHandle( Section );

				if (View != NULL)
				{
					m_Offset = GetFilePointer( );
					m_Size   = GetFileSize( );
					m_View   = View;
				}
			}
		}
	}

	inline
	void
	SetExternalView(
		__in_bcount( ViewSize ) const unsigned char * View,
		__in ULONGLONG ViewSize
		)
	{
		m_Offset       = 0;
		m_Size         = ViewSize;
		m_View         = (unsigned char *) View; // Still const
		m_ExternalView = true;
	}

	//
	// ReadFile wrapper with descriptive exception raising on failure.
	//

	inline
	void
	ReadFile(
		__out_bcount( Length ) void * Buffer,
		__in size_t Length,
		__in const char * Description
		)
	{
		DWORD Transferred;
		char  ExMsg[ 64 ];

		if (Length == 0)
			return;

		if (m_View != NULL)
		{
			if ((m_Offset + Length < m_Offset) ||
			    (m_Offset + Length > m_Size))
			{
				StringCbPrintfA(
					ExMsg,
					sizeof( ExMsg ),
					"ReadFile( %s ) failed.",
					Description);

				throw std::runtime_error( ExMsg );
			}

			xmemcpy(
				Buffer,
				&m_View[ m_Offset ],
				Length);

			m_Offset += Length;
			return;
		}

		if (::ReadFile(
			m_File,
			Buffer,
			(DWORD) Length,
			&Transferred,
			NULL) && (Transferred == (DWORD) Length))
			return;

		StringCbPrintfA(
			ExMsg,
			sizeof( ExMsg ),
			"ReadFile( %s ) failed.",
			Description);

		throw std::runtime_error( ExMsg );
	}

	//
	// Seek to a particular file offset.
	//

	inline
	void
	SeekOffset(
		__in ULONGLONG Offset,
		__in const char * Description
		)
	{
		LONG  Low;
		LONG  High;
		DWORD NewPtrLow;
		char  ExMsg[ 64 ];

		if (m_View != NULL)
		{
			if (Offset >= m_Size)
			{
				StringCbPrintfA(
					ExMsg,
					sizeof( ExMsg ),
					"SeekOffset( %s ) failed.",
					Description);

				throw std::runtime_error( ExMsg );
			}

			m_Offset = Offset;
			return;
		}

		Low  = (LONG) ((Offset >>  0) & 0xFFFFFFFF);
		High = (LONG) ((Offset >> 32) & 0xFFFFFFFF);

		NewPtrLow = SetFilePointer( m_File, Low, &High, FILE_BEGIN );

		if ((NewPtrLow == INVALID_SET_FILE_POINTER) &&
			(GetLastError( ) != NO_ERROR))
		{
			StringCbPrintfA(
				ExMsg,
				sizeof( ExMsg ),
				"SeekOffset( %s ) failed.",
				Description);

			throw std::runtime_error( ExMsg );
		}
	}

	inline
	ULONGLONG
	GetFileSize(
		) const
	{
		ULONGLONG Size;
		DWORD     SizeHigh;

		if (m_View != NULL)
			return m_Size;

		Size = ::GetFileSize( m_File, &SizeHigh );

		if ((Size == INVALID_FILE_SIZE) && (GetLastError( ) != NO_ERROR))
		{
			throw std::runtime_error( "GetFileSize failed" );
		}

		return Size | ((ULONGLONG) SizeHigh) << 32;
	}

	inline
	ULONGLONG
	GetFilePointer(
		) const
	{
		LARGE_INTEGER Fp;

		if (m_View != NULL)
			return m_Offset;

		Fp.QuadPart = 0;

		if (!SetFilePointerEx( m_File, Fp, &Fp, FILE_CURRENT ))
			throw std::runtime_error( "SetFilePointerEx failed" );

		return Fp.QuadPart;
	}

private:

	DECLSPEC_NORETURN
	inline
	void
	ThrowInPageError(
		)
	{
		throw std::runtime_error( "In-page I/O error accessing file" );
	}

	inline
	void *
	__cdecl
	xmemcpy(
		__out_bcount_full_opt(_Size) void * _Dst,
		__in_bcount_opt(_Size) const void * _Src,
		__in size_t _Size
		)
	{
		__try
		{
			return (memcpy( _Dst, _Src, _Size ));
		}
		__except( (GetExceptionCode( ) == EXCEPTION_IN_PAGE_ERROR ) ?
		          EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
		{
			ThrowInPageError( );
		}
	}

	HANDLE          m_File;
	unsigned char * m_View;
	ULONGLONG       m_Offset;
	ULONGLONG       m_Size;
	bool            m_ExternalView;

};

#endif
