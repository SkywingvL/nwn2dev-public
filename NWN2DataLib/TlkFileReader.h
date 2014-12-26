/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	TlkFileReader.h

Abstract:

	This module defines the interface to the Talk Table (TLK) file reader.  TLK
	files are used to localize string resources in the game.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_TLKFILEREADER_H
#define _PROGRAMS_NWN2DATALIB_TLKFILEREADER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "FileWrapper.h"

//
// Define the TLK file reader object, used to access TLK files.
//

template< typename ResRefT >
class TlkFileReader
{

public:

	typedef unsigned long  StrRef;

	//
	// Constructor.  Raises an std::exception on parse failure.
	//

	TlkFileReader(
		__in const std::string & FileName
		);

	//
	// Destructor.
	//

	virtual
	~TlkFileReader(
		);

	//
	// Look up a string based on STRREF.  Returns false on failure, i.e. if the
	// string could not be found.
	//

	bool
	GetTalkString(
		__in StrRef StringId,
		__out std::string & String
		) const;

	//
	// Define the TLK on-disk file structures.  This data is based on the
	// BioWare Aurora engine documentation.
	//
	// http://nwn.bioware.com/developers/Bioware_Aurora_TalkTable_Format.pdf
	//

#include <pshpack1.h>

	typedef enum _TLK_LANGUAGE
	{
		LangEnglish            = 0,
		LangFrench             = 1,
		LangGerman             = 2,
		LangItalian            = 3,
		LangSpanish            = 4,
		LangPolish             = 5,
		LangKorean             = 128,
		LangChineseTraditional = 129,
		LangChineseSimplified  = 130,
		LangJapanese           = 131,
		LangLastTlkLanguage
	} TLK_LANGUAGE, * PTLK_LANGUAGE;

private:

	//
	// Parse the on-disk format and read the base directory data in.
	//

	void
	ParseTlkFile(
		);

	enum
	{
		TEXT_PRESENT      = 0x01,
		SND_PRESENT       = 0x02,
		SNDLENGTH_PRESENT = 0x04,

		LAST_FLAG
	};

	typedef struct _TLK_HEADER
	{
		unsigned long FileType;                // "TLK "
		unsigned long Version;                 // "V3.0"
		unsigned long LanguageId;              // 0=ENU, 1=FR, etc
		unsigned long StringCount;             // # of strings in file
		unsigned long StringEntriesOffset;     // from beginning of file
	} TLK_HEADER, * PTLK_HEADER;

	typedef const struct _TLK_HEADER * PCTLK_HEADER;

	typedef struct _TLK_STRING
	{
		unsigned long  Flags;
		ResRefT        SoundResRef;
		unsigned long  VolumeVariance;
		unsigned long  PitchVariance;
		unsigned long  OffsetToString;
		unsigned long  StringSize;
		float          SoundLength;
	} TLK_STRING, * PTLK_STRING;

	typedef const struct _TLK_STRING * PCTLK_STRING;

#include <poppack.h>

	typedef std::vector< TLK_STRING > TlkStringVec;

	//
	// Locate a string descriptor by its reference id.
	//

	inline
	PCTLK_STRING
	LookupStringDescriptor(
		__in StrRef ResourceId
		) const
	{
		if (ResourceId >= m_StringDir.size( ))
			return NULL;

		return &m_StringDir[ ResourceId ];
	}

	//
	// Define file book-keeping data.
	//

	HANDLE                m_File;
	unsigned long         m_FileSize;
	mutable FileWrapper   m_FileWrapper;
	ULONGLONG             m_StringsOffset;

	//
	// Resource list data.
	//

	TlkStringVec          m_StringDir;

};

typedef TlkFileReader< NWN::ResRef32 > TlkFileReader32;
typedef TlkFileReader< NWN::ResRef16 > TlkFileReader16;

//
// N.B.  NWN2 uses 16-char resrefs for its tlk files, unlike most other data.
//


#endif
