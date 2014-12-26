/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	2DAFileReader.cpp

Abstract:

	This module houses the *.2da file format parser, which is used to read
	Two-Dimensional Array files (typically containing game rules and engine
	configuration data).

--*/

#include "Precomp.h"
#include "2DAFileReader.h"

TwoDAFileReader::TwoDAFileReader(
	__in const std::string & FileName
	)
/*++

Routine Description:

	This routine constructs a new TwoDAFileReader object and parses the contents
	of a 2DA file by filename.  The file must already exist as it immediately
	deserialized.

Arguments:

	FileName - Supplies the path to the 2DA file.

Return Value:

	The newly constructed object.

Environment:

	User mode.

--*/
{
	//
	// Load the file up.
	//

	Parse2DAFile( FileName );
}

TwoDAFileReader::~TwoDAFileReader(
	)
/*++

Routine Description:

	This routine cleans up an already-existing TwoDAFileReader object.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
}

bool
TwoDAFileReader::Get2DAString(
	__in const std::string & Column,
	__in size_t Row,
	__out std::string & Value
	) const
/*++

Routine Description:

	This routine fetches the string value of a column at a particular row index
	into the 2DA file.

Arguments:

	Column - Supplies the column name to look up.

	Row - Supplies the row index to pull the column from.

	Value - Receives the string contents of the particular row.

Return Value:

	The routine returns a Boolean value indicating true on success, else false
	on failure (i.e. unknown string).  On catastrophic failure, an
	std::exception is raised.

Environment:

	User mode.

--*/
{
	if (Row >= m_Rows.size( ))
		return false;

	Value = m_Rows[ Row ][ GetColumnIndex( Column ) ];

	if (Value == "****")
		return false;

	return true;
}

void
TwoDAFileReader::Parse2DAFile(
	__in const std::string & FileName
	)
/*++

Routine Description:

	This routine parses the on-disk contents of a 2DA file, building an
	in-memory representation.

	2DA files are tab-delimited, with one file header line, and one column
	header line, followed by a series of line contents.

Arguments:

	FileName - Supplies the name of the .2DA file to open.

Return Value:

	None.  On failure, the routine raises an std::exception.

Environment:

	User mode.

--*/
{
	std::vector< char >   Line;
	FILE                * File;
	enum
	{
		ModeFileHeader,
		ModeFileHeader2,
		ModeColumnHeader,
		ModeContents
	}                     Mode;

	Line.resize( 32768 );

	File = fopen( FileName.c_str( ), "rt" );

	if (File == NULL)
	{
		try
		{
			std::string ErrorStr;

			ErrorStr  = "Failed to open .2DA file '";
			ErrorStr += FileName;
			ErrorStr += "'.";

			throw std::runtime_error( ErrorStr );
		}
		catch (std::bad_alloc)
		{
			throw std::runtime_error( "Failed to open .2DA file." );
		}
	}

	try
	{
		Mode = ModeFileHeader;

		while (fgets( &Line[ 0 ], (int) Line.size( ), File ))
		{
			strtok( &Line[ 0 ], "\r\n" );

			if (!Line[ 0 ])
				break;

			switch (Mode)
			{

			case ModeFileHeader:
				{
					if ((strncmp( &Line[ 0 ], "2DA\tV2.0", 8 )) &&
					    (strncmp( &Line[ 0 ], "2DA V2.0", 8 )))
					{
						try
						{
							std::string ErrorStr;

							ErrorStr  = "Unrecognized file format on .2DA '";
							ErrorStr += FileName;
							ErrorStr += "'.";

							throw std::runtime_error( ErrorStr );
						}
						catch (std::bad_alloc)
						{
							throw std::runtime_error( "Unrecognized file format on .2DA." );
						}
					}

					Mode = ModeFileHeader2;
				}
				break;

			case ModeFileHeader2:
				{
					//
					// TODO: Default value.
					//

					Mode = ModeColumnHeader;

					//
					// Here's a giant hack.  Some 2DAs appear to violate the
					// BioWare spec and actually do not have a second line in
					// the file header, but go right to the column header list.
					//
					// We detect this by looking for a second line that is not
					// entirely composed of whitespace and doesn't contain the
					// default value (which we don't support).  In such a case,
					// we assume the 2DA is damaged, like creaturespeed.2da,
					// and try to work around it.
					//

					if (_strnicmp( &Line[ 0 ], "DEFAULT:", 8 ))
					{
						size_t Len;

						Len = strlen( &Line[ 0 ] );

						if (strspn( &Line[ 0 ], "\t \r\n" ) != Len)
						{
							goto TryColumnHeader;
						}
					}
				}
				break;

			TryColumnHeader:
			case ModeColumnHeader:
				{
					char * State;
					char * p;

					State = NULL;

					for (p = strtok_s( &Line[ 0 ], "\t ", &State );
					     p != NULL;
					     p = strtok_s( NULL, "\t ", &State ))
					{
						m_Columns.push_back( p );
					}

					m_Rows.reserve( 64 );

					Mode = ModeContents;
				}
				break;

			case ModeContents:
				{
					size_t       ColumnIndex;
					char       * p;
					const char * Delim[ 2 ] = { "\t ", "\"" };
					size_t       QuoteMode;
					size_t       Offset;

					m_Rows.push_back( RowValueVec( ) );

					RowValueVec & RowVec = m_Rows.back( );

					RowVec.reserve( m_Columns.size( ) );

					p           = &Line[ 0 ];
					ColumnIndex = 0;
					QuoteMode   = 0;

					//
					// p = "\"string\" string2"
					//

					while (*p != 0)
					{
						while (isspace( (int) (unsigned char) *p ))
							p++;

						if (*p == 0)
							break;

						if (*p == '\"')
						{
							QuoteMode = 1;
							p++;
						}
						else
						{
							QuoteMode = 0;
						}

						Offset = strcspn( p, Delim[ QuoteMode ] );

						//
						// Skip the first column, which should just give us the
						// row index (however it is ignored and may even be out
						// of sync!).
						//

						if (ColumnIndex != 0)
							RowVec.push_back( std::string( p, Offset ) );

						ColumnIndex += 1;

						//
						// Advance to the next delimiter.
						//

						p += Offset;

						if (!*p)
							break;

						//
						// Move beyond the delimiter.
						//

						p += 1;

						if (!*p)
							break;

						//
						// If we were in dquote mode, we need to move one more
						// character beyond as there would be a dquote followed
						// by the next delimiter, and we want to be past the
						// next delimiter.
						//

						if (QuoteMode == 1)
						{
							p += 1;

							if (!*p)
								break;
						}
					}
#if 0

					p = strtok_s( &Line[ 0 ], "\t ", &State );

					if (p == NULL)
						continue;

//					WriteText( "%lu: ", m_Rows.size( ) - 1 );

					for (p = strtok_s( NULL, "\t ", &State );
					     p != NULL;
					     p = strtok_s( NULL, "\t ", &State ))
					{
						RowVec.push_back( p );

//						WriteText( "%s ", p );

						ColumnIndex += 1;
					}

//					WriteText( "\n" );
#endif
					if (ColumnIndex == 0)
					{
						m_Rows.pop_back( );

						continue;
					}

					if (RowVec.size( ) != m_Columns.size( ))
					{
						try
						{
							char ErrorStr[ 256 ];

							StringCbPrintfA(
								ErrorStr,
								sizeof( ErrorStr ),
								"Bad column count on .2DA '%s' / row %lu (line %lu, cols %lu/%lu).",
								FileName.c_str( ),
								(unsigned long) RowVec.size( ),
								(unsigned long) m_Rows.size( ),
								(unsigned long) ColumnIndex,
								(unsigned long) m_Columns.size( ));

							throw std::runtime_error( ErrorStr );
						}
						catch (std::bad_alloc)
						{
							throw std::runtime_error( "Bad column count on .2DA" );
						}
					}
				}
				break;

			default:
				throw std::runtime_error( "Illegal 2DA reader mode." );

			}
		}
	}
	catch (...)
	{
		fclose( File );
		throw;
	}

	fclose( File );
}

