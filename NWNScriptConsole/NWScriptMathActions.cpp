/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptSimpleActions.cpp

Abstract:

	This module houses the implementation of math/calculation-related action
	service handlers.

--*/

#include "Precomp.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#define NWSCRIPTHOST_INTERNAL
#include "NWScriptHost.h"

enum { MAX_DICE = 100 };

static
unsigned long
RollDice(
	__in unsigned long NumDice,
	__in unsigned long Sides
	)
/*++

Routine Description:

	This routine rolls dice and returns the reslt.

Arguments:

	NumDice - Supplies the number of dice to roll.

	Sides - Supplies the number of sides of each die to roll.

Return Value:

	The summed roll value is returned.

Environment:

	User mode.

--*/
{
	unsigned long Sum;

	Sum = 0;

	for (unsigned long i = 0; i < NumDice; i += 1)
	{
		Sum += (rand( ) % Sides) + 1;
	}

	return Sum;
}

SCRIPT_ACTION( Random )
/*++

Routine Description:

	This script action generates a random number.

Arguments:

	nMaxInteger - Supplies the ceiling (less one) of the random value that is
	              requested.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	int nMaxInteger = VMStack.StackPopInt( );
	int RandValue;

	if (nMaxInteger == 0)
		RandValue = 0;
	else
		RandValue  = ((rand( ) << 15) | (rand() <<  0)) % nMaxInteger;

	VMStack.StackPushInt( RandValue );
}

SCRIPT_ACTION( VectorNormalize )
/*++

Routine Description:

	This script action normalizes a vector.

Arguments:

	vVector - Supplies the vector to normalize.

Return Value:

	The routine returns the converted vector.

Environment:

	User mode.

--*/
{
	NWN::Vector3 v = VMStack.StackPopVector( );

	VMStack.StackPushVector( Math::NormalizeVector( v ) );
}

SCRIPT_ACTION( AngleToVector )
/*++

Routine Description:

	This script action converts an angle to a vector.

Arguments:

	fAngle - Supplies the angle.

Return Value:

	The routine returns the converted vector.

Environment:

	User mode.

--*/
{
	float        fAngle = VMStack.StackPopFloat( ) * (PI / 180.0f);
	NWN::Vector3 Heading;

	Heading.x = cos( fAngle );
	Heading.y = sin( fAngle );
	Heading.z = 0.0f;

	VMStack.StackPushVector( Heading );
}

SCRIPT_ACTION( VectorToAngle )
/*++

Routine Description:

	This script action converts a vector to an angle.

Arguments:

	vVector - Supplies the vector.

Return Value:

	The routine returns the converted angle.

Environment:

	User mode.

--*/
{
	NWN::Vector3 vVector = VMStack.StackPopVector( );

	VMStack.StackPushFloat( atan2f( vVector.y, vVector.x ) * 180.0f / PI );
}

SCRIPT_ACTION( FeetToMeters )
/*++

Routine Description:

	This script action converts feet to meters.

Arguments:

	fFeet - Supplies the value to convert.

Return Value:

	The routine returns the value converted to meters.

Environment:

	User mode.

--*/
{
	VMStack.StackPushFloat(
		VMStack.StackPopFloat( ) * 0.3048f
		);
}

SCRIPT_ACTION( YardsToMeters )
/*++

Routine Description:

	This script action converts yards to meters.

Arguments:

	fYards - Supplies the value to convert.

Return Value:

	The routine returns the value converted to meters.

Environment:

	User mode.

--*/
{
	VMStack.StackPushFloat(
		VMStack.StackPopFloat( ) * 0.9144f
		);
}

SCRIPT_ACTION( VectorMagnitude )
/*++

Routine Description:

	This script action returns the magnitude of a vector.

Arguments:

	vVector - Supplies the vector whose magnitude is to be retrieved.

Return Value:

	The routine returns the magnitude of the vector.

Environment:

	User mode.

--*/
{
	VMStack.StackPushFloat(
		Math::Magnitude( VMStack.StackPopVector( ) )
		);
}

SCRIPT_ACTION( d2 )
/*++

Routine Description:

	This script action rolls dice.

Arguments:

	nNumDice - Supplies the number of dice to roll.

Return Value:

	The routine returns the roll sum.

Environment:

	User mode.

--*/
{
	int nNumDice = NumArguments >= 1 ? VMStack.StackPopInt( ) : 1;

	if (nNumDice < 1 || nNumDice > MAX_DICE)
		nNumDice = 1;

	VMStack.StackPushInt( RollDice( (unsigned long) nNumDice, 2 ) );
}

SCRIPT_ACTION( d3 )
/*++

Routine Description:

	This script action rolls dice.

Arguments:

	nNumDice - Supplies the number of dice to roll.

Return Value:

	The routine returns the roll sum.

Environment:

	User mode.

--*/
{
	int nNumDice = NumArguments >= 1 ? VMStack.StackPopInt( ) : 1;

	if (nNumDice < 1 || nNumDice > MAX_DICE)
		nNumDice = 1;

	VMStack.StackPushInt( RollDice( (unsigned long) nNumDice, 3 ) );
}

SCRIPT_ACTION( d4 )
/*++

Routine Description:

	This script action rolls dice.

Arguments:

	nNumDice - Supplies the number of dice to roll.

Return Value:

	The routine returns the roll sum.

Environment:

	User mode.

--*/
{
	int nNumDice = NumArguments >= 1 ? VMStack.StackPopInt( ) : 1;

	if (nNumDice < 1 || nNumDice > MAX_DICE)
		nNumDice = 1;

	VMStack.StackPushInt( RollDice( (unsigned long) nNumDice, 4 ) );
}

SCRIPT_ACTION( d6 )
/*++

Routine Description:

	This script action rolls dice.

Arguments:

	nNumDice - Supplies the number of dice to roll.

Return Value:

	The routine returns the roll sum.

Environment:

	User mode.

--*/
{
	int nNumDice = NumArguments >= 1 ? VMStack.StackPopInt( ) : 1;

	if (nNumDice < 1 || nNumDice > MAX_DICE)
		nNumDice = 1;

	VMStack.StackPushInt( RollDice( (unsigned long) nNumDice, 6 ) );
}

SCRIPT_ACTION( d8 )
/*++

Routine Description:

	This script action rolls dice.

Arguments:

	nNumDice - Supplies the number of dice to roll.

Return Value:

	The routine returns the roll sum.

Environment:

	User mode.

--*/
{
	int nNumDice = NumArguments >= 1 ? VMStack.StackPopInt( ) : 1;

	if (nNumDice < 1 || nNumDice > MAX_DICE)
		nNumDice = 1;

	VMStack.StackPushInt( RollDice( (unsigned long) nNumDice, 8 ) );
}

SCRIPT_ACTION( d10 )
/*++

Routine Description:

	This script action rolls dice.

Arguments:

	nNumDice - Supplies the number of dice to roll.

Return Value:

	The routine returns the roll sum.

Environment:

	User mode.

--*/
{
	int nNumDice = NumArguments >= 1 ? VMStack.StackPopInt( ) : 1;

	if (nNumDice < 1 || nNumDice > MAX_DICE)
		nNumDice = 1;

	VMStack.StackPushInt( RollDice( (unsigned long) nNumDice, 10 ) );
}

SCRIPT_ACTION( d12 )
/*++

Routine Description:

	This script action rolls dice.

Arguments:

	nNumDice - Supplies the number of dice to roll.

Return Value:

	The routine returns the roll sum.

Environment:

	User mode.

--*/
{
	int nNumDice = NumArguments >= 1 ? VMStack.StackPopInt( ) : 1;

	if (nNumDice < 1 || nNumDice > MAX_DICE)
		nNumDice = 1;

	VMStack.StackPushInt( RollDice( (unsigned long) nNumDice, 12 ) );
}

SCRIPT_ACTION( d20 )
/*++

Routine Description:

	This script action rolls dice.

Arguments:

	nNumDice - Supplies the number of dice to roll.

Return Value:

	The routine returns the roll sum.

Environment:

	User mode.

--*/
{
	int nNumDice = NumArguments >= 1 ? VMStack.StackPopInt( ) : 1;

	if (nNumDice < 1 || nNumDice > MAX_DICE)
		nNumDice = 1;

	VMStack.StackPushInt( RollDice( (unsigned long) nNumDice, 20 ) );
}

SCRIPT_ACTION( d100 )
/*++

Routine Description:

	This script action rolls dice.

Arguments:

	nNumDice - Supplies the number of dice to roll.

Return Value:

	The routine returns the roll sum.

Environment:

	User mode.

--*/
{
	int nNumDice = NumArguments >= 1 ? VMStack.StackPopInt( ) : 1;

	if (nNumDice < 1 || nNumDice > MAX_DICE)
		nNumDice = 1;

	VMStack.StackPushInt( RollDice( (unsigned long) nNumDice, 100 ) );
}

SCRIPT_ACTION( RoundsToSeconds )
/*++

Routine Description:

	This script action converts rounds to seconds.

Arguments:

	nRounds - Supplies the value to convert.

Return Value:

	The routine returns the value converted to seconds.

Environment:

	User mode.

--*/
{
	VMStack.StackPushFloat(
		VMStack.StackPopInt( ) * 6.0f
		);
}

SCRIPT_ACTION( HoursToSeconds )
/*++

Routine Description:

	This script action converts hours to seconds.

Arguments:

	nHours - Supplies the value to convert.

Return Value:

	The routine returns the value converted to seconds.

Environment:

	User mode.

--*/
{
	VMStack.StackPushFloat(
		(float) VMStack.StackPopInt( ) * 60 * 60
		);
}

SCRIPT_ACTION( TurnsToSeconds )
/*++

Routine Description:

	This script action converts turns to seconds.

Arguments:

	nTurns - Supplies the value to convert.

Return Value:

	The routine returns the value converted to seconds.

Environment:

	User mode.

--*/
{
	VMStack.StackPushFloat(
		VMStack.StackPopInt( ) * 60.0f
		);
}

SCRIPT_ACTION( abs )
/*++

Routine Description:

	Provides the absolute value of an integer.

Arguments:

	nInteger - Supplies the integer to make positive.

Return Value:

	nInteger - Absolute value of the integer.

Environment:

	User mode.

--*/
{
	VMStack.StackPushInt( std::abs( VMStack.StackPopInt( ) ) );
}

SCRIPT_ACTION( fabs )
/*++

Routine Description:

	Provides the absolute value of a float.

Arguments:

	nFloat - Supplies the float to make positive.

Return Value:

	nFloat - Absolute value of the float.

Environment:

	User mode.

--*/
{
	VMStack.StackPushFloat( std::abs( VMStack.StackPopFloat( ) ) );
}

SCRIPT_ACTION( cos )
/*++

Routine Description:

	Math operation: cosine of a float.

Arguments:

	nFloat - Float to take cosine of.

Return Value:

	nFloat - Result after cosine operation.

Environment:

	User mode.

--*/
{
	VMStack.StackPushFloat( std::cos( VMStack.StackPopFloat( ) ) );
}

SCRIPT_ACTION( sin )
/*++

Routine Description:

	Math operation: sine of a float.

Arguments:

	nFloat - Float to take a sine of.

Return Value:

	nFloat - Result after a sin operation.

Environment:

	User mode.

--*/
{
	VMStack.StackPushFloat( std::sin( VMStack.StackPopFloat( ) ) );
}

SCRIPT_ACTION( tan )
/*++

Routine Description:

	Math operation: tangent of a float.

Arguments:

	nFloat - Float to take tangent of.

Return Value:

	nFloat - Result after tangent operation.

Environment:

	User mode.

--*/
{
	VMStack.StackPushFloat( std::tan( VMStack.StackPopFloat( ) ) );
}

SCRIPT_ACTION( acos )
/*++

Routine Description:

	Math operation: arc cosine of a float.

Arguments:

	nFloat - Float to take an arc cosine of.

Return Value:

	nFloat - Result after an arc cosine operation.

Environment:

	User mode.

--*/
{
	float nFloat = VMStack.StackPopFloat( );

	if ((nFloat > 1) || (nFloat < -1))
	{
		VMStack.StackPushFloat( 0.0f );
	}
	else
	{
		VMStack.StackPushFloat( std::acos( nFloat ) );
	}
}

SCRIPT_ACTION( asin )
/*++

Routine Description:

	Math operation: arc sine of a float.

Arguments:

	nFloat - Float to take an arc sine of.

Return Value:

	nFloat - Result after an arc sine operation.

Environment:

	User mode.

--*/
{
	float nFloat = VMStack.StackPopFloat( );

	if ((nFloat > 1) || (nFloat < -1))
	{
		VMStack.StackPushFloat( 0.0f );
	}
	else
	{
		VMStack.StackPushFloat( std::asin( nFloat ) );
	}
}

SCRIPT_ACTION( atan )
/*++

Routine Description:

	Math operation: arc tangent of a float.

Arguments:

	nFloat - Float to take an arc tangent of.

Return Value:

	nFloat - Result after an arc tangent operation.

Environment:

	User mode.

--*/
{
	VMStack.StackPushFloat( std::atan( VMStack.StackPopFloat( ) ) );
}

SCRIPT_ACTION( log )
/*++

Routine Description:

	Math operation: natural logarithm of a float.

Arguments:

	nFloat - Float to take a natural logarithm of.

Return Value:

	nFloat - Result after a natural logarithm operation.

Environment:

	User mode.

--*/
{
	float nFloat = VMStack.StackPopFloat();

	if (nFloat <= 0)
	{
		VMStack.StackPushFloat( 0 );
	}
	else
	{
		VMStack.StackPushFloat( std::log( nFloat ) );
	}
}

SCRIPT_ACTION( pow )
/*++

Routine Description:

	Math operation: exponential function of a float.

Arguments:

	fBase - Base number.

	fExponent - Exponent to raise fBase to.

Return Value:

	fRes - Result after a natural logarithm operation.

Environment:

	User mode.

--*/
{
	float fBase = VMStack.StackPopFloat( );
	float fExponent = VMStack.StackPopFloat( );
	
	if ((fBase == 0) || (fExponent < 0))
	{
		VMStack.StackPushFloat( 0.0f );
	}
	else
	{
		VMStack.StackPushFloat( std::pow( fBase, fExponent ) );
	}
}

SCRIPT_ACTION( sqrt )
/*++

Routine Description:

	Math operation: square root of a float.

Arguments:

	fValue - Number to be rooted.

Return Value:

	Square root of fValue.

Environment:

	User mode.

--*/
{
	float fValue = VMStack.StackPopFloat( );
	
	if (fValue < 0)
	{
		VMStack.StackPushFloat( 0.0f );
	}
	else
	{
		VMStack.StackPushFloat( std::sqrt( fValue ) );
	}
}
