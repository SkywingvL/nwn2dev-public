/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptSimpleActions.cpp

Abstract:

	This module houses the implementation of trivial action service handlers.

--*/

#include "Precomp.h"
#define NWSCRIPTHOST_INTERNAL
#include "NWScriptHost.h"


SCRIPT_ACTION( PrintString )
/*++

Routine Description:

	This script action displays a string to the server log.

Arguments:

	sString - Supplies the string to display.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	std::string sString = VMStack.StackPopString( );

	m_TextOut->WriteText(
		"PrintString: %s\n",
		sString.c_str( ) );
}

SCRIPT_ACTION( PrintFloat )
/*++

Routine Description:

	This script action displays a string to the server log.

Arguments:

	fFloat - Supplies the float to display.

	nWidth - Supplies the width.

	nDecimals - Supplies the precision.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	char  FmtString[ 64 ];
	float fFloat;
	int   nWidth;
	int   nDecimals;

	fFloat    = VMStack.StackPopFloat( );
	nWidth    = NumArguments >= 2 ? VMStack.StackPopInt( ) : 18;
	nDecimals = NumArguments >= 3 ? VMStack.StackPopInt( ) : 9;

	nWidth    = min( nWidth, 18 );
	nWidth    = max( nWidth, 0 );
	nDecimals = min( nDecimals, 9 );
	nDecimals = max( nDecimals, 0 );

	if (SUCCEEDED( StringCbPrintfA(
		FmtString,
		sizeof( FmtString ),
		"PrintFloat: %%%2d.%02df\n",
		nWidth,
		nDecimals) ))
	{
		m_TextOut->WriteText(
			FmtString,
			fFloat);
	}
}

SCRIPT_ACTION( FloatToString )
/*++

Routine Description:

	This script action converts a floating point value into a string.

Arguments:

	fFloat - Supplies the float to display.

	nWidth - Supplies the width.

	nDecimals - Supplies the precision.

Return Value:

	The routine returns the floating point formatted into a string.

Environment:

	User mode.

--*/
{
	char  FmtString[ 64 ];
	float fFloat;
	int   nWidth;
	int   nDecimals;

	fFloat    = VMStack.StackPopFloat( );
	nWidth    = NumArguments >= 2 ? VMStack.StackPopInt( ) : 18;
	nDecimals = NumArguments >= 3 ? VMStack.StackPopInt( ) : 9;

	nWidth    = min( nWidth, 18 );
	nWidth    = max( nWidth, 0 );
	nDecimals = min( nDecimals, 9 );
	nDecimals = max( nDecimals, 0 );

	if (SUCCEEDED( StringCbPrintfA(
		FmtString,
		sizeof( FmtString ),
		"%%%2d.%02df",
		nWidth,
		nDecimals) ))
	{
		char Formatted[ 64 ];

		StringCbPrintfA(
			Formatted,
			sizeof( Formatted ),
			FmtString,
			fFloat);

		VMStack.StackPushString( Formatted );
	}
	else
	{
		VMStack.StackPushString( "" );
	}
}

SCRIPT_ACTION( PrintInteger )
/*++

Routine Description:

	This script action displays an integer to the server log.

Arguments:

	nInteger - Supplies the integer to display.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	int nInteger = VMStack.StackPopInt( );

	m_TextOut->WriteText(
		"PrintInteger: %d\n",
		nInteger);
}

SCRIPT_ACTION( PrintObject )
/*++

Routine Description:

	This script action displays an object id to the server log.

Arguments:

	oObject - Supplies the object to display.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	NWN::OBJECTID   ObjectId;

	ObjectId = StackPopObjectId( VMStack );

	m_TextOut->WriteText(
		"PrintObject: Object %08X\n",
		ObjectId);
}

SCRIPT_ACTION( GetIsObjectValid )
/*++

Routine Description:

	This script action checks whether an object id reference is valid.

Arguments:

	oObject - Supplies the object id to check validity for.

Return Value:

	The routine returns TRUE if the object id was valid, else FALSE.

Environment:

	User mode.

--*/
{
	NWN::OBJECTID   oObject = StackPopObjectId( VMStack );

	if (oObject == NWN::INVALIDOBJID)
		VMStack.StackPushInt( FALSE );
	else
		VMStack.StackPushInt( TRUE );
}

SCRIPT_ACTION( IntToHexString )
/*++

Routine Description:

	This script action converts an integer to a hex string (0x%08x).

Arguments:

	nInteger - Supplies the integer to convert.

Return Value:

	The routine returns the integer formatted into a string.

Environment:

	User mode.

--*/
{
	char Formatted[ 32 ];
	int  nInteger = VMStack.StackPopInt( );

	StringCbPrintfA(
		Formatted,
		sizeof( Formatted ),
		"0x%08x",
		(unsigned long) nInteger);

	VMStack.StackPushString( Formatted );
}

SCRIPT_ACTION( IntToString )
/*++

Routine Description:

	This script action converts an integer to a hex string (%d).

Arguments:

	nInteger - Supplies the integer to convert.

Return Value:

	The routine returns the integer formatted into a string.

Environment:

	User mode.

--*/
{
	char Formatted[ 32 ];
	int  nInteger = VMStack.StackPopInt( );

	StringCbPrintfA(
		Formatted,
		sizeof( Formatted ),
		"%d",
		(unsigned long) nInteger);

	VMStack.StackPushString( Formatted );
}

SCRIPT_ACTION( IntToFloat )
/*++

Routine Description:

	This script action converts an integer to a float.

Arguments:

	nInteger - Supplies the integer to convert.

Return Value:

	The routine returns the converted value.

Environment:

	User mode.

--*/
{
	int nInteger = VMStack.StackPopInt( );

	VMStack.StackPushFloat( (float) nInteger );
}

SCRIPT_ACTION( FloatToInt )
/*++

Routine Description:

	This script action converts a float to an integer.

Arguments:

	fFloat - Supplies the float to convert.

Return Value:

	The routine returns the converted value.

Environment:

	User mode.

--*/
{
	float fFloat = VMStack.StackPopFloat( );

	VMStack.StackPushInt( (int) fFloat );
}

SCRIPT_ACTION( StringToInt )
/*++

Routine Description:

	This script action converts a string to an integer.

Arguments:

	sNumber - Supplies the string to convert.

Return Value:

	The routine returns the converted value.

Environment:

	User mode.

--*/
{
	std::string sNumber = VMStack.StackPopString( );

	VMStack.StackPushInt( (int) _strtoi64( sNumber.c_str( ), NULL, 10 ) );
}

SCRIPT_ACTION( StringToFloat )
/*++

Routine Description:

	This script action converts a string to a float.

Arguments:

	sNumber - Supplies the string to convert.

Return Value:

	The routine returns the converted value.

Environment:

	User mode.

--*/
{
	std::string sNumber = VMStack.StackPopString( );

	VMStack.StackPushFloat( (float) atof( sNumber.c_str( ) ) );
}

SCRIPT_ACTION( IntToObject )
/*++

Routine Description:

	This script action converts an int to an object id.

Arguments:

	nInt - Supplies the int to convert.

Return Value:

	The routine returns the converted value.

Environment:

	User mode.

--*/
{
	StackPushObjectId(
		VMStack,
		(NWN::OBJECTID) VMStack.StackPopInt( ));
}

SCRIPT_ACTION( ObjectToInt )
/*++

Routine Description:

	This script action converts an object id to an int.

Arguments:

	oObj - Supplies the object id to convert.

Return Value:

	The routine returns the converted value.

Environment:

	User mode.

--*/
{
	//
	// N.B.  Intended to use the raw StackPopObjectId so that the list type bit
	//       is -not- set.
	//

	VMStack.StackPushInt(
		(int) VMStack.StackPopObjectId( ) );
}

SCRIPT_ACTION( ObjectToString )
/*++

Routine Description:

	This script action converts an object id to a string (%08x formatted).

Arguments:

	oObj - Supplies the object id to convert.

Return Value:

	The routine returns the converted value.

Environment:

	User mode.

--*/
{
	char Formatted[ 32 ];

	//
	// N.B.  Intended to use the raw StackPopObjectId so that the list type bit
	//       is -not- set.
	//

	StringCbPrintfA(
		Formatted,
		sizeof( Formatted ),
		"%08x",
		(unsigned long) VMStack.StackPopObjectId( ));

	VMStack.StackPushString( Formatted );
}

SCRIPT_ACTION( StringToObject )
/*++

Routine Description:

	This script action converts a string to an object id (%08x formatted).

Arguments:

	sString - Supplies the string object id to convert.

Return Value:

	The routine returns the converted value.

Environment:

	User mode.

--*/
{
	NWN::OBJECTID ObjectId;

	ObjectId = (NWN::OBJECTID) strtoul(
		VMStack.StackPopString( ).c_str( ),
		NULL,
		10); // N.B.  Yes, using base 10 in this direction is a bug in NWN2.

	StackPushObjectId( VMStack, ObjectId );
}

SCRIPT_ACTION( PrintVector )
/*++

Routine Description:

	This script action prints a vector to the server log.

Arguments:

	vVector - Supplies the vector to print.

	bPrepend - Supplies TRUE if "PRINTVECTOR:" is to be prepended when printing
	           the log message.

Return Value:

	The routine returns the converted value.

Environment:

	User mode.

--*/
{
	NWN::Vector3 vVector = VMStack.StackPopVector( );
	int          bPrepend = NumArguments >= 2 ? VMStack.StackPopInt( ) : FALSE;

	m_TextOut->WriteText(
		"%s[%g, %g, %g]\n",
		bPrepend ? "PRINTVECTOR: " : "",
		vVector.x,
		vVector.y,
		vVector.z);
}

SCRIPT_ACTION( Vector )
/*++

Routine Description:

	This script action creates a vector.

Arguments:

	x - Supplies the x component.

	y - Supplies the y component.

	z - Supplies the z component.

Return Value:

	The routine returns the converted vector.

Environment:

	User mode.

--*/
{
	NWN::Vector3 v;

	v.x = NumArguments >= 1 ? VMStack.StackPopFloat( ) : 0.0f;
	v.y = NumArguments >= 2 ? VMStack.StackPopFloat( ) : 0.0f;
	v.z = NumArguments >= 3 ? VMStack.StackPopFloat( ) : 0.0f;

	VMStack.StackPushVector( v );
}

SCRIPT_ACTION( GetStringLength )
/*++

Routine Description:

	This script action returns the length of a string.

Arguments:

	sString - Supplies the string to query.

Return Value:

	The routine returns the string length.

Environment:

	User mode.

--*/
{
	std::string sString    = VMStack.StackPopString( );

	VMStack.StackPushInt( (int) sString.size( ) );
}

SCRIPT_ACTION( GetStringUpperCase )
/*++

Routine Description:

	This script action returns the upper-case version of a string.

Arguments:

	sString - Supplies the string to upcase.

Return Value:

	The routine returns the upcased string.

Environment:

	User mode.

--*/
{
	std::string sString    = VMStack.StackPopString( );
	std::string Converted;

	for (std::string::const_iterator it = sString.begin( );
	     it != sString.end( );
	     ++it)
	{
		Converted.push_back( (char) toupper( (int) (unsigned char) *it ) );
	}

	VMStack.StackPushString( Converted );
}

SCRIPT_ACTION( GetStringLowerCase )
/*++

Routine Description:

	This script action returns the lower-case version of a string.

Arguments:

	sString - Supplies the string to downcase.

Return Value:

	The routine returns the downcased string.

Environment:

	User mode.

--*/
{
	std::string sString    = VMStack.StackPopString( );
	std::string Converted;

	for (std::string::const_iterator it = sString.begin( );
	     it != sString.end( );
	     ++it)
	{
		Converted.push_back( (char) tolower( (int) (unsigned char) *it ) );
	}

	VMStack.StackPushString( Converted );
}

SCRIPT_ACTION( GetStringRight )
/*++

Routine Description:

	This script action returns a substring from the right side of a string.

Arguments:

	sString - Supplies the string to query.

	nCount - Supplies the offset from the right side of the string.

Return Value:

	The routine returns the upcased string.

Environment:

	User mode.

--*/
{
	std::string sString    = VMStack.StackPopString( );
	int         nCount     = VMStack.StackPopInt( );
	std::string Substring;

	if (nCount <= 0)
		goto done;

	if ((size_t) nCount > sString.size( ))
		nCount = (int) sString.size( );

	Substring = sString.substr( sString.size( ) - (size_t) nCount );

done:
	VMStack.StackPushString( Substring );
}

SCRIPT_ACTION( GetStringLeft )
/*++

Routine Description:

	This script action returns a substring from the left side of a string.

Arguments:

	sString - Supplies the string to query.

	nCount - Supplies the count of characters from the left side of the string.

Return Value:

	The routine returns the upcased string.

Environment:

	User mode.

--*/
{
	std::string sString    = VMStack.StackPopString( );
	int         nCount     = VMStack.StackPopInt( );
	std::string Substring;

	if (nCount <= 0)
		goto done;

	if ((size_t) nCount > sString.size( ))
		nCount = (int) sString.size( );

	Substring = sString.substr( 0, (size_t) nCount );

done:
	VMStack.StackPushString( Substring );
}

SCRIPT_ACTION( InsertString )
/*++

Routine Description:

	This script action inserts a string into another string.

Arguments:

	sDestination - Supplies the string to insert into.

	sString - Supplies the string to insert.

	nPosition - Supplies the inserting offset.

Return Value:

	The routine returns the upcased string.

Environment:

	User mode.

--*/
{
	std::string sDestination = VMStack.StackPopString( );
	std::string sString      = VMStack.StackPopString( );
	int         nPosition    = VMStack.StackPopInt( );

	if (nPosition < 0)
		nPosition = 0;
	else if ((size_t) nPosition > sDestination.size( ))
		nPosition = (int) sDestination.size( );

	if (nPosition == (int) sDestination.size( ))
		sDestination += sString;
	else
		sDestination.insert( (size_t) nPosition, sString );

	VMStack.StackPushString( sDestination );
}

SCRIPT_ACTION( GetSubString )
/*++

Routine Description:

	This script action returns a substring of a string.

Arguments:

	sString - Supplies the string to query.

	nStart - Supplies the starting offset from the left side of the string.

	nCount - Supplies the count of characters to return.

Return Value:

	The routine returns the upcased string.

Environment:

	User mode.

--*/
{
	std::string sString    = VMStack.StackPopString( );
	int         nStart     = VMStack.StackPopInt( );
	int         nCount     = VMStack.StackPopInt( );
	std::string Substring;

	if ((nStart < 0) || ((size_t) nStart > sString.size( )))
		goto done;

	//
	// We must permit script programs to specify bogus character count values
	// because the stock server was never strict about checking these.  As a
	// result we are forced to support negative count values being taken to
	// mean the remainder of the substring (xp_craft depends on this bug).
	//

	if (nCount < 0)
		nCount = (int) sString.size( ) - nStart;

	if ((nCount <= 0) || (nStart + nCount < nStart))
		goto done;

	if ((size_t) (nStart + nCount) > sString.size( ))
		goto done;

	Substring = sString.substr( (size_t) nStart, (size_t) nCount );

done:
	VMStack.StackPushString( Substring );
}

SCRIPT_ACTION( FindSubString )
/*++

Routine Description:

	This script action performs a substring search in a string.

Arguments:

	sString - Supplies the string to search in.

	sSubString - Supplies the string to search for.

	nStart - Supplies the starting offset for the search.

Return Value:

	The routine returns the substring index, else -1 if there was no substring.

Environment:

	User mode.

--*/
{
	std::string sString    = VMStack.StackPopString( );
	std::string sSubString = VMStack.StackPopString( );
	int         nStart     = NumArguments >= 3 ? VMStack.StackPopInt( ) : 0;
	size_t      Offset;
	int         FoundOffset;

	Offset      = (size_t) nStart;
	FoundOffset = -1;

	if (Offset >= sString.size( ))
		goto done;

	Offset = sString.find( sSubString, Offset );

	if (Offset != std::string::npos)
		FoundOffset = (int) Offset;

done:
	VMStack.StackPushInt( FoundOffset );
}

SCRIPT_ACTION( StringCompare )
/*++

Routine Description:

	This script action performs a C-style strcmp/_stricmp.

Arguments:

	sString1 - Supplies the first string to compare.

	sString2 - Supplies the second string to compare.

	nCaseInsensitive - Supplies TRUE if the comparision is case insensitive.

Return Value:

	If the strings are equal, the return value is zero.
	If the first string is greater, the return value is positive.
	If the second string is greater, the return value is negative.

Environment:

	User mode.

--*/
{
	std::string sString1         = VMStack.StackPopString( );
	std::string sString2         = VMStack.StackPopString( );
	int         nCaseInsensitive = NumArguments >= 3 ? VMStack.StackPopInt( ) : FALSE;
	int         Result;

	if (nCaseInsensitive)
		Result = strcmp( sString1.c_str( ), sString2.c_str( ) );
	else
		Result = _stricmp( sString1.c_str( ), sString2.c_str( ) );

	VMStack.StackPushInt( Result );
}

SCRIPT_ACTION( CharToASCII )
/*++

Routine Description:

	This script action returns the numeric value of the first character in a
	string.

Arguments:

	sString - Supplies the string to query.

Return Value:

	The numeric value of the first character is returned (or zero if there is
	no first character).

Environment:

	User mode.

--*/
{
	std::string sString1         = VMStack.StackPopString( );

	VMStack.StackPushInt( (sString1.empty( ) ? '\0' : sString1[ 0 ]) );
}

SCRIPT_ACTION( SpawnScriptDebugger )
/*++

Routine Description:

	This script action launches the script debugger (if it was inactive).

	Note that as the script debugger is not implemented, no action is taken.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
}

SCRIPT_ACTION( GetIsSinglePlayer )
/*++

Routine Description:

	This script action checks whether the server is a single player server.

Arguments:

	None.

Return Value:

	The routine returns TRUE if the server is single player only.

Environment:

	User mode.

--*/
{
	VMStack.StackPushInt( FALSE );
}

SCRIPT_ACTION( DoSinglePlayerAutoSave )
/*++

Routine Description:

	This script action saves the autosave in single player mode if autosave was
	enabled.

	This routine is not implemented and does nothing.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
}

SCRIPT_ACTION( GetStringByStrRef )
/*++

Routine Description:

	This script action looks up a StrRef and returns its associated string for
	the server's language.

Arguments:

	nStrRef - Supplies the string to look up.

	nGender - Supplies the gender of the string.

Return Value:

	The routine returns the string for the StrRef in question in the server's
	language.

Environment:

	User mode.

--*/
{
	unsigned long      StrRef      = (unsigned long) VMStack.StackPopInt( );
	int                nGender     = NumArguments >= 2 ? VMStack.StackPopInt( ) : 1;
	std::string        Str;

	m_ResourceManager.GetTalkString( StrRef, Str );

	VMStack.StackPushString( Str );

	nGender;
}

SCRIPT_ACTION( GetGameDifficulty )
/*++

Routine Description:

	This script action returns the difficulty level of the game.

	Only difficult mode is supported presently.

Arguments:

	None.

Return Value:

	The difficulty level (GAME_DIFFICULTY_*) is returned.

Environment:

	User mode.

--*/
{
	VMStack.StackPushInt( 0 );
}

SCRIPT_ACTION( Get2DAString )
/*++

Routine Description:

	This script action looks up a 2DA string.

Arguments:

	s2DA - Supplies the 2DA name.

	sColumn - Supplies the column name.

	nRow - Supplies the row number.

Return Value:

	The routine returns the corresponding 2DA value.

Environment:

	User mode.

--*/
{
	std::string s2DA    = VMStack.StackPopString( );
	std::string sColumn = VMStack.StackPopString( );
	int         nRow    = VMStack.StackPopInt( );
	std::string Value;

	m_ResourceManager.Get2DAString( s2DA, sColumn, (size_t) nRow, Value );

	VMStack.StackPushString( Value );
}

SCRIPT_ACTION( GetNum2DARows )
/*++

Routine Description:

	This script action returns the count of rows in a 2DA.

Arguments:

	s2DAName - Supplies the 2DA name.

Return Value:

	The routine returns the count of rows in a 2DA.

Environment:

	User mode.

--*/
{
	std::string s2DAName = VMStack.StackPopString( );

	VMStack.StackPushInt( (int) m_ResourceManager.Get2DARowCount( s2DAName ) );
}

SCRIPT_ACTION( GetNum2DAColumns )
/*++

Routine Description:

	This script action returns the count of columns in a 2DA.

Arguments:

	s2DAName - Supplies the 2DA name.

Return Value:

	The routine returns the count of columns in a 2DA.

Environment:

	User mode.

--*/
{
	std::string s2DAName = VMStack.StackPopString( );

	VMStack.StackPushInt( (int) m_ResourceManager.Get2DAColumnCount( s2DAName ) );
}

SCRIPT_ACTION( Clear2DACache )
/*++

Routine Description:

	This script action causes cached 2DAs to be reloaded.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	m_ResourceManager.Clear2DACache( );
}

SCRIPT_ACTION( NWNXGetInt )
/*++

Routine Description:

	This script action calls into NWNX.

Arguments:

	sPlugin - Supplies the plugin name.

	sFunction - Supplies the function name.

	sParam1 - Supplies the first function parameter.

	nParam2 - Supplies the second function parameter.

Return Value:

	The routine returns an integer.

Environment:

	User mode.

--*/
{
	std::string sPlugin    = VMStack.StackPopString( );
	std::string sFunction  = VMStack.StackPopString( );
	std::string sParam1    = VMStack.StackPopString( );
	int         nParam2    = VMStack.StackPopInt( );
	int         Value;

	Value = 0;

	VMStack.StackPushInt( Value );

	sPlugin;
	sFunction;
	sParam1;
	nParam2;
}

SCRIPT_ACTION( NWNXGetFloat )
/*++

Routine Description:

	This script action calls into NWNX.

Arguments:

	sPlugin - Supplies the plugin name.

	sFunction - Supplies the function name.

	sParam1 - Supplies the first function parameter.

	nParam2 - Supplies the second function parameter.

Return Value:

	The routine returns a float.

Environment:

	User mode.

--*/
{
	std::string sPlugin    = VMStack.StackPopString( );
	std::string sFunction  = VMStack.StackPopString( );
	std::string sParam1    = VMStack.StackPopString( );
	int         nParam2    = VMStack.StackPopInt( );
	float       Value;

	Value = 0.0f;

	VMStack.StackPushFloat( Value );

	sPlugin;
	sFunction;
	sParam1;
	nParam2;
}

SCRIPT_ACTION( NWNXGetString )
/*++

Routine Description:

	This script action calls into NWNX.

Arguments:

	sPlugin - Supplies the plugin name.

	sFunction - Supplies the function name.

	sParam1 - Supplies the first function parameter.

	nParam2 - Supplies the second function parameter.

Return Value:

	The routine returns a string.

Environment:

	User mode.

--*/
{
	std::string   sPlugin    = VMStack.StackPopString( );
	std::string   sFunction  = VMStack.StackPopString( );
	std::string   sParam1    = VMStack.StackPopString( );
	int           nParam2    = VMStack.StackPopInt( );
	std::string   Value;

	Value = "";

	VMStack.StackPushString( Value );

	sPlugin;
	sFunction;
	sParam1;
	nParam2;
}

SCRIPT_ACTION( NWNXSetInt )
/*++

Routine Description:

	This script action calls into NWNX.

Arguments:

	sPlugin - Supplies the plugin name.

	sFunction - Supplies the function name.

	sParam1 - Supplies the first function parameter.

	nParam2 - Supplies the second function parameter.

	nValue - Supplies the value to set

Return Value:

	None.

Environment:

	User mode.

--*/
{
	std::string sPlugin    = VMStack.StackPopString( );
	std::string sFunction  = VMStack.StackPopString( );
	std::string sParam1    = VMStack.StackPopString( );
	int         nParam2    = VMStack.StackPopInt( );
	int         nValue     = VMStack.StackPopInt( );

	sPlugin;
	sFunction;
	sParam1;
	nParam2;
	nValue;
}

SCRIPT_ACTION( NWNXSetFloat )
/*++

Routine Description:

	This script action calls into NWNX.

Arguments:

	sPlugin - Supplies the plugin name.

	sFunction - Supplies the function name.

	sParam1 - Supplies the first function parameter.

	nParam2 - Supplies the second function parameter.

	fValue - Supplies the value to set.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	std::string sPlugin    = VMStack.StackPopString( );
	std::string sFunction  = VMStack.StackPopString( );
	std::string sParam1    = VMStack.StackPopString( );
	int         nParam2    = VMStack.StackPopInt( );
	float       fValue     = VMStack.StackPopFloat( );

	sPlugin;
	sFunction;
	sParam1;
	nParam2;
	fValue;
}

SCRIPT_ACTION( NWNXSetString )
/*++

Routine Description:

	This script action calls into NWNX.

Arguments:

	sPlugin - Supplies the plugin name.

	sFunction - Supplies the function name.

	sParam1 - Supplies the first function parameter.

	nParam2 - Supplies the second function parameter.

	sValue - Supplies the value to set.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	std::string   sPlugin    = VMStack.StackPopString( );
	std::string   sFunction  = VMStack.StackPopString( );
	std::string   sParam1    = VMStack.StackPopString( );
	int           nParam2    = VMStack.StackPopInt( );
	std::string   sValue     = VMStack.StackPopString( );

	sPlugin;
	sFunction;
	sParam1;
	nParam2;
	sValue;
}

SCRIPT_ACTION( WriteTimestampedLogEntry )
/*++

Routine Description:

	This script action displays a string to the server log.

Arguments:

	sLogEntry - Supplies the string to display.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	std::string sLogEntry = VMStack.StackPopString( );

	m_TextOut->WriteText(
		"[Script]: %s\n",
		sLogEntry.c_str( ) );
}

SCRIPT_ACTION( GetTalkTableLanguage )
/*++

Routine Description:

	This script action returns the language of the server-side talk table.

Arguments:

	None.

Return Value:

	The server-side talk table language is returned.

Environment:

	User mode.

--*/
{
	VMStack.StackPushInt( GffFileReader::LangEnglish );
}

SCRIPT_ACTION( EndGame )
/*++

Routine Description:

	This script action shuts down the server.

Arguments:

	sEndMovie - Supplies the ending movie to play.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	std::string sEndMovie = VMStack.StackPopString( );

	m_TextOut->WriteText(
		"Script requests server shutdown via EndGame (%s).\n",
		sEndMovie.c_str( ));

	sEndMovie; // Not implemented
}

SCRIPT_ACTION( PackCampaignDatabase )
/*++

Routine Description:

	This script packs a campaign database.  It is a no-op.

Arguments:

	sCampaignName - Supplies the name of the campaign to compact.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	std::string sCampaignName = VMStack.StackPopString( );

	sCampaignName; // Not used.
}

SCRIPT_ACTION( GetOnePartyMode )
/*++

Routine Description:

	This script action returns the one party only flag of the server.

Arguments:

	None.

Return Value:

	The routine returns TRUE if the server is one one party only mode.

Environment:

	User mode.

--*/
{
	VMStack.StackPushInt( FALSE );
}

SCRIPT_ACTION( GetPartyName )
/*++

Routine Description:

	This script action returns the SoZ party name.

Arguments:

	None.

Return Value:

	The party name string is returned.

Environment:

	User mode.

--*/
{
	VMStack.StackPushString( "default-party-name" );
}

SCRIPT_ACTION( GetPartyMotto )
/*++

Routine Description:

	This script action returns the SoZ party motto.

Arguments:

	None.

Return Value:

	The party motto string is returned.

Environment:

	User mode.

--*/
{
	VMStack.StackPushString( "default-party-motto" );
}

SCRIPT_ACTION( GetSoundFileDuration )
/*++

Routine Description:

	This script action returns the duration of a sound file, as expressed in a
	count of milliseconds.

Arguments:

	sSoundFile - Supplies the sound file to query.

Return Value:

	The sound file duration is returned.

Environment:

	User mode.

--*/
{
	std::string sSoundFile = VMStack.StackPopString( );

	VMStack.StackPushInt( 0 );
}

SCRIPT_ACTION( GetModuleName )
/*++

Routine Description:

	This script action returns the module name.

Arguments:

	None.

Return Value:

	The module name is returned.

Environment:

	User mode.

--*/
{
	VMStack.StackPushString( "modulename" );
}
