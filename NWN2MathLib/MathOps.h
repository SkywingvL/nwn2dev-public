/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	MathOps.h

Abstract:

	This module houses the interface to various useful mathematical operators,
	such as used by pathfinding logic.

--*/

#ifndef _PROGRAMS_NWN2MATHLIB_MATHOPS_H
#define _PROGRAMS_NWN2MATHLIB_MATHOPS_H

#ifdef _MSC_VER
#pragma once
#endif

typedef std::vector< NWN::Vector2 > Vector2Vec;
typedef std::vector< NWN::Vector3 > Vector3Vec;

namespace Math
{
	//
	// Define the precision of floating point operations.
	//

	const float Epsilon = 0.00001f;

	//
	// Define the fixed-point precision for the math library.
	//

	const unsigned int FixedPointShift = 1 << 6;

	struct RectFP
	{
		unsigned int left;
		unsigned int top;
		unsigned int right;
		unsigned int bottom;
	};

	struct Vector2FP
	{
		unsigned int x;
		unsigned int y;
	};

	typedef std::vector< Vector2FP > Vector2FPVec;

	inline
	long
	Round(
		__in float F
		)
	{
		return (long) floor( F + 0.5f );
	}

	//
	// Calculate the dot product of two vectors.
	//

	inline
	float
	DotProduct(
		__in const NWN::Vector2 & v1,
		__in const NWN::Vector2 & v2
		)
	{
		return (v1.x * v2.x) + (v1.y * v2.y);
	}

	//
	// Calculate the dot product of two vectors.
	//

	inline
	float
	DotProduct(
		__in const NWN::Vector3 & v1,
		__in const NWN::Vector3 & v2
		)
	{
		return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
	}

	inline
	float
	LengthVector(
		__in const NWN::Vector3 & v
		)
	{
		return sqrtf( DotProduct( v, v ) );
	}

	inline
	float
	Magnitude(
		__in const NWN::Vector3 & v
		)
	{
		return LengthVector( v );
	}

	//
	// Calculate the cross product of two vectors.
	//

	inline
	NWN::Vector3
	CrossProduct(
		__in const NWN::Vector3 & v1,
		__in const NWN::Vector3 & v2
		)
	{
		NWN::Vector3 v3;

		v3.x = v1.y * v2.z - v2.y * v1.z;
		v3.y = v1.z * v2.x - v2.z * v1.x;
		v3.z = v1.x * v2.y - v2.x * v1.y;

		return v3;
	}

	//
	// Add two vectors.
	//

	inline
	NWN::Vector2
	Add(
		__in const NWN::Vector2 & v1,
		__in const NWN::Vector2 & v2
		)
	{
		NWN::Vector2 v3;

		v3.x = v1.x + v2.x;
		v3.y = v1.y + v2.y;

		return v3;
	}

	inline
	NWN::Vector3
	Add(
		__in const NWN::Vector3 & v1,
		__in const NWN::Vector3 & v2
		)
	{
		NWN::Vector3 v3;

		v3.x = v1.x + v2.x;
		v3.y = v1.y + v2.y;
		v3.z = v1.z + v2.z;

		return v3;
	}

	//
	// Subtract two vectors.
	//

	inline
	NWN::Vector2
	Subtract(
		__in const NWN::Vector2 & v1,
		__in const NWN::Vector2 & v2
		)
	{
		NWN::Vector2 v3;

		v3.x = v1.x - v2.x;
		v3.y = v1.y - v2.y;

		return v3;
	}

	inline
	NWN::Vector3
	Subtract(
		__in const NWN::Vector3 & v1,
		__in const NWN::Vector3 & v2
		)
	{
		NWN::Vector3 v3;

		v3.x = v1.x - v2.x;
		v3.y = v1.y - v2.y;
		v3.z = v1.z - v2.z;

		return v3;
	}

	//
	// Multiply a vector by a scalar.
	//

	inline
	NWN::Vector3
	Multiply(
		__in const NWN::Vector3 & v1,
		__in float f
		)
	{
		NWN::Vector3 v0;

		v0.x = v1.x * f;
		v0.y = v1.y * f;
		v0.z = v1.z * f;

		return v0;
	}

	//
	// Calculate the cross product of two vectors.
	//

	inline
	float
	CrossProduct(
		__in const NWN::Vector2 & v1,
		__in const NWN::Vector2 & v2
		)
	{
		return v1.x * v2.y - v2.x * v2.y;
	}

	//
	// Normalize a vector (to unit length).
	//

	inline
	NWN::Vector2
	NormalizeVector(
		__in const NWN::Vector2 & v
		)
	{
		NWN::Vector2 vn;
		float        M;

		M = sqrtf( (v.x * v.x) + (v.y * v.y) );

		if (M == 0.0f)
			return v;

		vn.x = v.x / M;
		vn.y = v.y / M;

		return vn;
	}

	//
	// Normalize a vector (to unit length).
	//

	inline
	NWN::Vector3
	NormalizeVector(
		__in const NWN::Vector3 & v
		)
	{
		NWN::Vector3 vn;
		float        M;

		M = sqrtf( (v.x * v.x) + (v.y * v.y) + (v.z * v.z) );

		if (M <= Epsilon)
		{
			vn.x = 1.0f;
			vn.y = 0.0f;
			vn.z = 0.0f;

			return vn;
		}

		vn.x = v.x / M;
		vn.y = v.y / M;
		vn.z = v.z / M;

		return vn;
	}

	//
	// Convert a Matrix44 to a Matrix33.
	//

	inline
	NWN::Matrix33
	CreateMatrix33FromMatrix44(
		__in const NWN::Matrix44 & M1
		)
	{
		NWN::Matrix33 M0;

		M0._00 = M1._00;
		M0._01 = M1._01;
		M0._02 = M1._02;
		M0._10 = M1._10;
		M0._11 = M1._11;
		M0._12 = M1._12;
		M0._20 = M1._20;
		M0._21 = M1._21;
		M0._22 = M1._22;

		return M0;
	}

	//
	// Convert a Matrix33 to a Matrix44.
	//

	inline
	NWN::Matrix44
	CreateMatrix44FromMatrix33(
		__in const NWN::Matrix33 & M1
		)
	{
		NWN::Matrix44 M0;

		M0._00 = M1._00;
		M0._01 = M1._01;
		M0._02 = M1._02;
		M0._03 = 0.0f;
		M0._10 = M1._10;
		M0._11 = M1._11;
		M0._12 = M1._12;
		M0._13 = 0.0f;
		M0._20 = M1._20;
		M0._21 = M1._21;
		M0._22 = M1._22;
		M0._23 = 0.0f;
		M0._30 = 0.0f;
		M0._31 = 0.0f;
		M0._32 = 0.0f;
		M0._33 = 1.0f;

		return M0;
	}

	//
	// Multiply two 4x4 matricies together.
	//

	inline
	NWN::Matrix44
	Multiply(
		__in const NWN::Matrix44 & M1,
		__in const NWN::Matrix44 & M2
		)
	{
		NWN::Matrix44 M0 =
		{
			( M1._00 * M2._00 ) + ( M1._01 * M2._10 ) + ( M1._02 * M2._20 ) + ( M1._03 * M2._30 ), // _00
			( M1._00 * M2._01 ) + ( M1._01 * M2._11 ) + ( M1._02 * M2._21 ) + ( M1._03 * M2._31 ), // _01
			( M1._00 * M2._02 ) + ( M1._01 * M2._12 ) + ( M1._02 * M2._22 ) + ( M1._03 * M2._32 ), // _02
			( M1._00 * M2._03 ) + ( M1._01 * M2._13 ) + ( M1._02 * M2._23 ) + ( M1._03 * M2._33 ), // _03

			( M1._10 * M2._00 ) + ( M1._11 * M2._10 ) + ( M1._12 * M2._20 ) + ( M1._13 * M2._30 ), // _10
			( M1._10 * M2._01 ) + ( M1._11 * M2._11 ) + ( M1._12 * M2._21 ) + ( M1._13 * M2._31 ), // _11
			( M1._10 * M2._02 ) + ( M1._11 * M2._12 ) + ( M1._12 * M2._22 ) + ( M1._13 * M2._32 ), // _12
			( M1._10 * M2._03 ) + ( M1._11 * M2._13 ) + ( M1._12 * M2._23 ) + ( M1._13 * M2._33 ), // _13

			( M1._20 * M2._00 ) + ( M1._21 * M2._10 ) + ( M1._22 * M2._20 ) + ( M1._23 * M2._30 ), // _20
			( M1._20 * M2._01 ) + ( M1._21 * M2._11 ) + ( M1._22 * M2._21 ) + ( M1._23 * M2._31 ), // _21
			( M1._20 * M2._02 ) + ( M1._21 * M2._12 ) + ( M1._22 * M2._22 ) + ( M1._23 * M2._32 ), // _22
			( M1._20 * M2._03 ) + ( M1._21 * M2._13 ) + ( M1._22 * M2._23 ) + ( M1._23 * M2._33 ), // _23

			( M1._30 * M2._00 ) + ( M1._31 * M2._10 ) + ( M1._32 * M2._20 ) + ( M1._33 * M2._30 ), // _30
			( M1._30 * M2._01 ) + ( M1._31 * M2._11 ) + ( M1._32 * M2._21 ) + ( M1._33 * M2._31 ), // _31
			( M1._30 * M2._02 ) + ( M1._31 * M2._12 ) + ( M1._32 * M2._22 ) + ( M1._33 * M2._32 ), // _32
			( M1._30 * M2._03 ) + ( M1._31 * M2._13 ) + ( M1._32 * M2._23 ) + ( M1._33 * M2._33 )  // _33
		};

		return M0;
	}
	//
	// Multiply two 4x4 matricies together with improved intermediate precision.
	//

	inline
	NWN::Matrix44
	Multiply_Double(
		__in const NWN::Matrix44 & M1,
		__in const NWN::Matrix44 & M2
		)
	{
		NWN::Matrix44 M0 =
		{
			(float) ( ( (double) M1._00 * (double) M2._00 ) + ( (double) M1._01 * (double) M2._10 ) + ( (double) M1._02 * (double) M2._20 ) + ( (double) M1._03 * (double) M2._30 ) ), // _00
			(float) ( ( (double) M1._00 * (double) M2._01 ) + ( (double) M1._01 * (double) M2._11 ) + ( (double) M1._02 * (double) M2._21 ) + ( (double) M1._03 * (double) M2._31 ) ), // _01
			(float) ( ( (double) M1._00 * (double) M2._02 ) + ( (double) M1._01 * (double) M2._12 ) + ( (double) M1._02 * (double) M2._22 ) + ( (double) M1._03 * (double) M2._32 ) ), // _02
			(float) ( ( (double) M1._00 * (double) M2._03 ) + ( (double) M1._01 * (double) M2._13 ) + ( (double) M1._02 * (double) M2._23 ) + ( (double) M1._03 * (double) M2._33 ) ), // _03

			(float) ( ( (double) M1._10 * (double) M2._00 ) + ( (double) M1._11 * (double) M2._10 ) + ( (double) M1._12 * (double) M2._20 ) + ( (double) M1._13 * (double) M2._30 ) ), // _10
			(float) ( ( (double) M1._10 * (double) M2._01 ) + ( (double) M1._11 * (double) M2._11 ) + ( (double) M1._12 * (double) M2._21 ) + ( (double) M1._13 * (double) M2._31 ) ), // _11
			(float) ( ( (double) M1._10 * (double) M2._02 ) + ( (double) M1._11 * (double) M2._12 ) + ( (double) M1._12 * (double) M2._22 ) + ( (double) M1._13 * (double) M2._32 ) ), // _12
			(float) ( ( (double) M1._10 * (double) M2._03 ) + ( (double) M1._11 * (double) M2._13 ) + ( (double) M1._12 * (double) M2._23 ) + ( (double) M1._13 * (double) M2._33 ) ), // _13

			(float) ( ( (double) M1._20 * (double) M2._00 ) + ( (double) M1._21 * (double) M2._10 ) + ( (double) M1._22 * (double) M2._20 ) + ( (double) M1._23 * (double) M2._30 ) ), // _20
			(float) ( ( (double) M1._20 * (double) M2._01 ) + ( (double) M1._21 * (double) M2._11 ) + ( (double) M1._22 * (double) M2._21 ) + ( (double) M1._23 * (double) M2._31 ) ), // _21
			(float) ( ( (double) M1._20 * (double) M2._02 ) + ( (double) M1._21 * (double) M2._12 ) + ( (double) M1._22 * (double) M2._22 ) + ( (double) M1._23 * (double) M2._32 ) ), // _22
			(float) ( ( (double) M1._20 * (double) M2._03 ) + ( (double) M1._21 * (double) M2._13 ) + ( (double) M1._22 * (double) M2._23 ) + ( (double) M1._23 * (double) M2._33 ) ), // _23

			(float) ( ( (double) M1._30 * (double) M2._00 ) + ( (double) M1._31 * (double) M2._10 ) + ( (double) M1._32 * (double) M2._20 ) + ( (double) M1._33 * (double) M2._30 ) ), // _30
			(float) ( ( (double) M1._30 * (double) M2._01 ) + ( (double) M1._31 * (double) M2._11 ) + ( (double) M1._32 * (double) M2._21 ) + ( (double) M1._33 * (double) M2._31 ) ), // _31
			(float) ( ( (double) M1._30 * (double) M2._02 ) + ( (double) M1._31 * (double) M2._12 ) + ( (double) M1._32 * (double) M2._22 ) + ( (double) M1._33 * (double) M2._32 ) ), // _32
			(float) ( ( (double) M1._30 * (double) M2._03 ) + ( (double) M1._31 * (double) M2._13 ) + ( (double) M1._32 * (double) M2._23 ) + ( (double) M1._33 * (double) M2._33 ) )  // _33
		};

		return M0;
	}
	//
	// Multiply two 4x4 matricies together as a 3x4 with 0, 0, 
	//

	inline
	NWN::Matrix44
	Multiply33_44(
		__in const NWN::Matrix44 & M1,
		__in const NWN::Matrix44 & M2
		)
	{
		NWN::Matrix44 M0 =
		{
			( M1._00 * M2._00 ) + ( M1._01 * M2._10 ) + ( M1._02 * M2._20 ),            // _00
			( M1._00 * M2._01 ) + ( M1._01 * M2._11 ) + ( M1._02 * M2._21 ),            // _01
			( M1._00 * M2._02 ) + ( M1._01 * M2._12 ) + ( M1._02 * M2._22 ),            // _02
			0.0f                                                          ,             // _03

			( M1._10 * M2._00 ) + ( M1._11 * M2._10 ) + ( M1._12 * M2._20 ),            // _10
			( M1._10 * M2._01 ) + ( M1._11 * M2._11 ) + ( M1._12 * M2._21 ),            // _11
			( M1._10 * M2._02 ) + ( M1._11 * M2._12 ) + ( M1._12 * M2._22 ),            // _12
			0.0f                                                          ,             // _13

			( M1._20 * M2._00 ) + ( M1._21 * M2._10 ) + ( M1._22 * M2._20 ),            // _20
			( M1._20 * M2._01 ) + ( M1._21 * M2._11 ) + ( M1._22 * M2._21 ),            // _21
			( M1._20 * M2._02 ) + ( M1._21 * M2._12 ) + ( M1._22 * M2._22 ),            // _22
			0.0f                                                           ,            // _23

			( M1._30 * M2._00 ) + ( M1._31 * M2._10 ) + ( M1._32 * M2._20 ) + M2._30  , // _30
			( M1._30 * M2._01 ) + ( M1._31 * M2._11 ) + ( M1._32 * M2._21 ) + M2._31  , // _31
			( M1._30 * M2._02 ) + ( M1._31 * M2._12 ) + ( M1._32 * M2._22 ) + M2._32  , // _32
			1.0f                                                                        // _33
		};

		return M0;
	}

	//
	// Multiply two 4x4 matricies, but treat them as 3x3.
	//
	// The _3x rows are taken from M3.
	//

	inline
	NWN::Matrix44
	Multiply33_33(
		__in const NWN::Matrix44 & M1,
		__in const NWN::Matrix44 & M2,
		__in const NWN::Matrix44 & M3
		)
	{
		NWN::Matrix44 M0 =
		{
			( M1._00 * M2._00 ) + ( M1._01 * M2._10 ) + ( M1._02 * M2._20 ), // _00
			( M1._00 * M2._01 ) + ( M1._01 * M2._11 ) + ( M1._02 * M2._21 ), // _01
			( M1._00 * M2._02 ) + ( M1._01 * M2._12 ) + ( M1._02 * M2._22 ), // _02
			M3._03                                                         , // _03

			( M1._10 * M2._00 ) + ( M1._11 * M2._10 ) + ( M1._12 * M2._20 ), // _10
			( M1._10 * M2._01 ) + ( M1._11 * M2._11 ) + ( M1._12 * M2._21 ), // _11
			( M1._10 * M2._02 ) + ( M1._11 * M2._12 ) + ( M1._12 * M2._22 ), // _12
			M3._13                                                         , // _13

			( M1._20 * M2._00 ) + ( M1._21 * M2._10 ) + ( M1._22 * M2._20 ), // _20
			( M1._20 * M2._01 ) + ( M1._21 * M2._11 ) + ( M1._22 * M2._21 ), // _21
			( M1._20 * M2._02 ) + ( M1._21 * M2._12 ) + ( M1._22 * M2._22 ), // _22
			M3._23                                                         , // _23

			M3._30                                                         , // _30
			M3._31                                                         , // _31
			M3._32                                                         , // _32
			M3._33                                                           // _33
		};

		return M0;
	}

	//
	// Multiply a 4x4 matrix by a scalar.
	//

	inline
	void
	Multiply(
		__in NWN::Matrix44 & M1,
		__in float F
		)
	{
		M1._00 *= F;
		M1._01 *= F;
		M1._02 *= F;
		M1._03 *= F;

		M1._10 *= F;
		M1._11 *= F;
		M1._12 *= F;
		M1._13 *= F;

		M1._20 *= F;
		M1._21 *= F;
		M1._22 *= F;
		M1._23 *= F;

		M1._30 *= F;
		M1._31 *= F;
		M1._32 *= F;
		M1._33 *= F;
	}
	//
	// Multiply a 4x4 matrix by a scalar.
	//

	inline
	void
	Multiply(
		__in NWN::Matrix44 & M1,
		__in double F
		)
	{
		M1._00 = (float) ((double) M1._00 * F);
		M1._01 = (float) ((double) M1._01 * F);
		M1._02 = (float) ((double) M1._02 * F);
		M1._03 = (float) ((double) M1._03 * F);

		M1._10 = (float) ((double) M1._10 * F);
		M1._11 = (float) ((double) M1._11 * F);
		M1._12 = (float) ((double) M1._12 * F);
		M1._13 = (float) ((double) M1._13 * F);

		M1._20 = (float) ((double) M1._20 * F);
		M1._21 = (float) ((double) M1._21 * F);
		M1._22 = (float) ((double) M1._22 * F);
		M1._23 = (float) ((double) M1._23 * F);

		M1._30 = (float) ((double) M1._30 * F);
		M1._31 = (float) ((double) M1._31 * F);
		M1._32 = (float) ((double) M1._32 * F);
		M1._33 = (float) ((double) M1._33 * F);
	}

	//
	// Add two 4x4 matricies.
	//

	inline
	NWN::Matrix44
	Add(
		__in const NWN::Matrix44 & M1,
		__in const NWN::Matrix44 & M2
		)
	{
		NWN::Matrix44 M0 =
		{
			M1._00 + M2._00, M1._01 + M2._01, M1._02 + M2._02, M1._03 + M2._03, // _00 .. _03
			M1._10 + M2._10, M1._11 + M2._11, M1._12 + M2._12, M1._13 + M2._13, // _10 .. _13
			M1._20 + M2._20, M1._21 + M2._21, M1._22 + M2._22, M1._23 + M2._23, // _20 .. _23
			M1._30 + M2._30, M1._31 + M2._31, M1._32 + M2._32, M1._33 + M2._33  // _30 .. _33
		};

		return M0;
	}

	//
	// Subtract two 4x4 matricies.
	//

	inline
	NWN::Matrix44
	Subtract(
		__in const NWN::Matrix44 & M1,
		__in const NWN::Matrix44 & M2
		)
	{
		NWN::Matrix44 M0 =
		{
			M1._00 - M2._00, M1._01 - M2._01, M1._02 - M2._02, M1._03 - M2._03, // _00 .. _03
			M1._10 - M2._10, M1._11 - M2._11, M1._12 - M2._12, M1._13 - M2._13, // _10 .. _13
			M1._20 - M2._20, M1._21 - M2._21, M1._22 - M2._22, M1._23 - M2._23, // _20 .. _23
			M1._30 - M2._30, M1._31 - M2._31, M1._32 - M2._32, M1._33 - M2._33  // _30 .. _33
		};

		return M0;
	}

	//
	// Multiply a vector by a matrix.
	//

	inline
	NWN::Vector3
	Multiply(
		__in const NWN::Matrix44 & M,
		__in const NWN::Vector3 & V1
		)
	{
		NWN::Vector3 V0;

		V0.x = M._00 * V1.x + M._10 * V1.y + M._20 * V1.z + M._30;
		V0.y = M._01 * V1.x + M._11 * V1.y + M._21 * V1.z + M._31;
		V0.z = M._02 * V1.x + M._12 * V1.y + M._22 * V1.z + M._32;

		return V0;
	}

	//
	// Multiply a vector by a matrix.
	//

	inline
	NWN::Vector3
	MultiplyNormal(
		__in const NWN::Matrix44 & M,
		__in const NWN::Vector3 & V1
		)
	{
		NWN::Vector3 V0;

		V0.x = M._00 * V1.x + M._10 * V1.y + M._20 * V1.z;
		V0.y = M._01 * V1.x + M._11 * V1.y + M._21 * V1.z;
		V0.z = M._02 * V1.x + M._12 * V1.y + M._22 * V1.z;

		return V0;
	}

	//
	// Return the determinant of a 4x4 matrix.
	//

	inline
	float
	Determinant(
		__in const NWN::Matrix44 & M1
		)
	{
		return (
			M1._03 * M1._12 * M1._21 * M1._30 - M1._02 * M1._13 * M1._21 * M1._30 - M1._03 * M1._11 * M1._22 * M1._30 + M1._01 * M1._13 * M1._22 * M1._30 +
			M1._02 * M1._11 * M1._23 * M1._30 - M1._01 * M1._12 * M1._23 * M1._30 - M1._03 * M1._12 * M1._20 * M1._31 + M1._02 * M1._13 * M1._20 * M1._31 +
			M1._03 * M1._10 * M1._22 * M1._31 - M1._00 * M1._13 * M1._22 * M1._31 - M1._02 * M1._10 * M1._23 * M1._31 + M1._00 * M1._12 * M1._23 * M1._31 +
			M1._03 * M1._11 * M1._20 * M1._32 - M1._01 * M1._13 * M1._20 * M1._32 - M1._03 * M1._10 * M1._21 * M1._32 + M1._00 * M1._13 * M1._21 * M1._32 +
			M1._01 * M1._10 * M1._23 * M1._32 - M1._00 * M1._11 * M1._23 * M1._32 - M1._02 * M1._11 * M1._20 * M1._33 + M1._01 * M1._12 * M1._20 * M1._33 +
			M1._02 * M1._10 * M1._21 * M1._33 - M1._00 * M1._12 * M1._21 * M1._33 - M1._01 * M1._10 * M1._22 * M1._33 + M1._00 * M1._11 * M1._22 * M1._33
			);
	}

	//
	// Return the determinant of a 4x4 matrix, using double-precision
	// intermediate calculations.
	//

	inline
	double
	Determinant_Double(
		__in const NWN::Matrix44 & M1
		)
	{
		return (
			(double) M1._03 * (double) M1._12 * (double) M1._21 * (double) M1._30 - (double) M1._02 * (double) M1._13 * (double) M1._21 * (double) M1._30 - (double) M1._03 * (double) M1._11 * (double) M1._22 * (double) M1._30 + (double) M1._01 * (double) M1._13 * (double) M1._22 * (double) M1._30 +
			(double) M1._02 * (double) M1._11 * (double) M1._23 * (double) M1._30 - (double) M1._01 * (double) M1._12 * (double) M1._23 * (double) M1._30 - (double) M1._03 * (double) M1._12 * (double) M1._20 * (double) M1._31 + (double) M1._02 * (double) M1._13 * (double) M1._20 * (double) M1._31 +
			(double) M1._03 * (double) M1._10 * (double) M1._22 * (double) M1._31 - (double) M1._00 * (double) M1._13 * (double) M1._22 * (double) M1._31 - (double) M1._02 * (double) M1._10 * (double) M1._23 * (double) M1._31 + (double) M1._00 * (double) M1._12 * (double) M1._23 * (double) M1._31 +
			(double) M1._03 * (double) M1._11 * (double) M1._20 * (double) M1._32 - (double) M1._01 * (double) M1._13 * (double) M1._20 * (double) M1._32 - (double) M1._03 * (double) M1._10 * (double) M1._21 * (double) M1._32 + (double) M1._00 * (double) M1._13 * (double) M1._21 * (double) M1._32 +
			(double) M1._01 * (double) M1._10 * (double) M1._23 * (double) M1._32 - (double) M1._00 * (double) M1._11 * (double) M1._23 * (double) M1._32 - (double) M1._02 * (double) M1._11 * (double) M1._20 * (double) M1._33 + (double) M1._01 * (double) M1._12 * (double) M1._20 * (double) M1._33 +
			(double) M1._02 * (double) M1._10 * (double) M1._21 * (double) M1._33 - (double) M1._00 * (double) M1._12 * (double) M1._21 * (double) M1._33 - (double) M1._01 * (double) M1._10 * (double) M1._22 * (double) M1._33 + (double) M1._00 * (double) M1._11 * (double) M1._22 * (double) M1._33
			);
	}

	//
	// Return the inverse of a matrix.
	//

	inline
	NWN::Matrix44
	Inverse(
		__in const NWN::Matrix44 & M1
		)
	{
		NWN::Matrix44 M0 =
		{
			M1._12 * M1._23 * M1._31 - M1._13 * M1._22 * M1._31 + M1._13 * M1._21 * M1._32 - M1._11 * M1._23 * M1._32 - M1._12 * M1._21 * M1._33 + M1._11 * M1._22 * M1._33,
			M1._03 * M1._22 * M1._31 - M1._02 * M1._23 * M1._31 - M1._03 * M1._21 * M1._32 + M1._01 * M1._23 * M1._32 + M1._02 * M1._21 * M1._33 - M1._01 * M1._22 * M1._33,
			M1._02 * M1._13 * M1._31 - M1._03 * M1._12 * M1._31 + M1._03 * M1._11 * M1._32 - M1._01 * M1._13 * M1._32 - M1._02 * M1._11 * M1._33 + M1._01 * M1._12 * M1._33,
			M1._03 * M1._12 * M1._21 - M1._02 * M1._13 * M1._21 - M1._03 * M1._11 * M1._22 + M1._01 * M1._13 * M1._22 + M1._02 * M1._11 * M1._23 - M1._01 * M1._12 * M1._23,

			M1._13 * M1._22 * M1._30 - M1._12 * M1._23 * M1._30 - M1._13 * M1._20 * M1._32 + M1._10 * M1._23 * M1._32 + M1._12 * M1._20 * M1._33 - M1._10 * M1._22 * M1._33,
			M1._02 * M1._23 * M1._30 - M1._03 * M1._22 * M1._30 + M1._03 * M1._20 * M1._32 - M1._00 * M1._23 * M1._32 - M1._02 * M1._20 * M1._33 + M1._00 * M1._22 * M1._33,
			M1._03 * M1._12 * M1._30 - M1._02 * M1._13 * M1._30 - M1._03 * M1._10 * M1._32 + M1._00 * M1._13 * M1._32 + M1._02 * M1._10 * M1._33 - M1._00 * M1._12 * M1._33,
			M1._02 * M1._13 * M1._20 - M1._03 * M1._12 * M1._20 + M1._03 * M1._10 * M1._22 - M1._00 * M1._13 * M1._22 - M1._02 * M1._10 * M1._23 + M1._00 * M1._12 * M1._23,

			M1._11 * M1._23 * M1._30 - M1._13 * M1._21 * M1._30 + M1._13 * M1._20 * M1._31 - M1._10 * M1._23 * M1._31 - M1._11 * M1._20 * M1._33 + M1._10 * M1._21 * M1._33,
			M1._03 * M1._21 * M1._30 - M1._01 * M1._23 * M1._30 - M1._03 * M1._20 * M1._31 + M1._00 * M1._23 * M1._31 + M1._01 * M1._20 * M1._33 - M1._00 * M1._21 * M1._33,
			M1._01 * M1._13 * M1._30 - M1._03 * M1._11 * M1._30 + M1._03 * M1._10 * M1._31 - M1._00 * M1._13 * M1._31 - M1._01 * M1._10 * M1._33 + M1._00 * M1._11 * M1._33,
			M1._03 * M1._11 * M1._20 - M1._01 * M1._13 * M1._20 - M1._03 * M1._10 * M1._21 + M1._00 * M1._13 * M1._21 + M1._01 * M1._10 * M1._23 - M1._00 * M1._11 * M1._23,

			M1._12 * M1._21 * M1._30 - M1._11 * M1._22 * M1._30 - M1._12 * M1._20 * M1._31 + M1._10 * M1._22 * M1._31 + M1._11 * M1._20 * M1._32 - M1._10 * M1._21 * M1._32,
			M1._01 * M1._22 * M1._30 - M1._02 * M1._21 * M1._30 + M1._02 * M1._20 * M1._31 - M1._00 * M1._22 * M1._31 - M1._01 * M1._20 * M1._32 + M1._00 * M1._21 * M1._32,
			M1._02 * M1._11 * M1._30 - M1._01 * M1._12 * M1._30 - M1._02 * M1._10 * M1._31 + M1._00 * M1._12 * M1._31 + M1._01 * M1._10 * M1._32 - M1._00 * M1._11 * M1._32,
			M1._01 * M1._12 * M1._20 - M1._02 * M1._11 * M1._20 + M1._02 * M1._10 * M1._21 - M1._00 * M1._12 * M1._21 - M1._01 * M1._10 * M1._22 + M1._00 * M1._11 * M1._22
		};

		Multiply( M0, 1 / Determinant( M1 ) );

		return M0;
	}

	//
	// Return the inverse of a matrix, using double-precision intermediate
	// calculations.
	//

	inline
	NWN::Matrix44
	Inverse_Double(
		__in const NWN::Matrix44 & M1
		)
	{
		NWN::Matrix44 M0 =
		{
			(float) ( (double) M1._12 * (double) M1._23 * (double) M1._31 - (double) M1._13 * (double) M1._22 * (double) M1._31 + (double) M1._13 * (double) M1._21 * (double) M1._32 - (double) M1._11 * (double) M1._23 * (double) M1._32 - (double) M1._12 * (double) M1._21 * (double) M1._33 + (double) M1._11 * (double) M1._22 * (double) M1._33 ),
			(float) ( (double) M1._03 * (double) M1._22 * (double) M1._31 - (double) M1._02 * (double) M1._23 * (double) M1._31 - (double) M1._03 * (double) M1._21 * (double) M1._32 + (double) M1._01 * (double) M1._23 * (double) M1._32 + (double) M1._02 * (double) M1._21 * (double) M1._33 - (double) M1._01 * (double) M1._22 * (double) M1._33 ),
			(float) ( (double) M1._02 * (double) M1._13 * (double) M1._31 - (double) M1._03 * (double) M1._12 * (double) M1._31 + (double) M1._03 * (double) M1._11 * (double) M1._32 - (double) M1._01 * (double) M1._13 * (double) M1._32 - (double) M1._02 * (double) M1._11 * (double) M1._33 + (double) M1._01 * (double) M1._12 * (double) M1._33 ),
			(float) ( (double) M1._03 * (double) M1._12 * (double) M1._21 - (double) M1._02 * (double) M1._13 * (double) M1._21 - (double) M1._03 * (double) M1._11 * (double) M1._22 + (double) M1._01 * (double) M1._13 * (double) M1._22 + (double) M1._02 * (double) M1._11 * (double) M1._23 - (double) M1._01 * (double) M1._12 * (double) M1._23 ),

			(float) ( (double) M1._13 * (double) M1._22 * (double) M1._30 - (double) M1._12 * (double) M1._23 * (double) M1._30 - (double) M1._13 * (double) M1._20 * (double) M1._32 + (double) M1._10 * (double) M1._23 * (double) M1._32 + (double) M1._12 * (double) M1._20 * (double) M1._33 - (double) M1._10 * (double) M1._22 * (double) M1._33 ),
			(float) ( (double) M1._02 * (double) M1._23 * (double) M1._30 - (double) M1._03 * (double) M1._22 * (double) M1._30 + (double) M1._03 * (double) M1._20 * (double) M1._32 - (double) M1._00 * (double) M1._23 * (double) M1._32 - (double) M1._02 * (double) M1._20 * (double) M1._33 + (double) M1._00 * (double) M1._22 * (double) M1._33 ),
			(float) ( (double) M1._03 * (double) M1._12 * (double) M1._30 - (double) M1._02 * (double) M1._13 * (double) M1._30 - (double) M1._03 * (double) M1._10 * (double) M1._32 + (double) M1._00 * (double) M1._13 * (double) M1._32 + (double) M1._02 * (double) M1._10 * (double) M1._33 - (double) M1._00 * (double) M1._12 * (double) M1._33 ),
			(float) ( (double) M1._02 * (double) M1._13 * (double) M1._20 - (double) M1._03 * (double) M1._12 * (double) M1._20 + (double) M1._03 * (double) M1._10 * (double) M1._22 - (double) M1._00 * (double) M1._13 * (double) M1._22 - (double) M1._02 * (double) M1._10 * (double) M1._23 + (double) M1._00 * (double) M1._12 * (double) M1._23 ),

			(float) ( (double) M1._11 * (double) M1._23 * (double) M1._30 - (double) M1._13 * (double) M1._21 * (double) M1._30 + (double) M1._13 * (double) M1._20 * (double) M1._31 - (double) M1._10 * (double) M1._23 * (double) M1._31 - (double) M1._11 * (double) M1._20 * (double) M1._33 + (double) M1._10 * (double) M1._21 * (double) M1._33 ),
			(float) ( (double) M1._03 * (double) M1._21 * (double) M1._30 - (double) M1._01 * (double) M1._23 * (double) M1._30 - (double) M1._03 * (double) M1._20 * (double) M1._31 + (double) M1._00 * (double) M1._23 * (double) M1._31 + (double) M1._01 * (double) M1._20 * (double) M1._33 - (double) M1._00 * (double) M1._21 * (double) M1._33 ),
			(float) ( (double) M1._01 * (double) M1._13 * (double) M1._30 - (double) M1._03 * (double) M1._11 * (double) M1._30 + (double) M1._03 * (double) M1._10 * (double) M1._31 - (double) M1._00 * (double) M1._13 * (double) M1._31 - (double) M1._01 * (double) M1._10 * (double) M1._33 + (double) M1._00 * (double) M1._11 * (double) M1._33 ),
			(float) ( (double) M1._03 * (double) M1._11 * (double) M1._20 - (double) M1._01 * (double) M1._13 * (double) M1._20 - (double) M1._03 * (double) M1._10 * (double) M1._21 + (double) M1._00 * (double) M1._13 * (double) M1._21 + (double) M1._01 * (double) M1._10 * (double) M1._23 - (double) M1._00 * (double) M1._11 * (double) M1._23 ),

			(float) ( (double) M1._12 * (double) M1._21 * (double) M1._30 - (double) M1._11 * (double) M1._22 * (double) M1._30 - (double) M1._12 * (double) M1._20 * (double) M1._31 + (double) M1._10 * (double) M1._22 * (double) M1._31 + (double) M1._11 * (double) M1._20 * (double) M1._32 - (double) M1._10 * (double) M1._21 * (double) M1._32 ),
			(float) ( (double) M1._01 * (double) M1._22 * (double) M1._30 - (double) M1._02 * (double) M1._21 * (double) M1._30 + (double) M1._02 * (double) M1._20 * (double) M1._31 - (double) M1._00 * (double) M1._22 * (double) M1._31 - (double) M1._01 * (double) M1._20 * (double) M1._32 + (double) M1._00 * (double) M1._21 * (double) M1._32 ),
			(float) ( (double) M1._02 * (double) M1._11 * (double) M1._30 - (double) M1._01 * (double) M1._12 * (double) M1._30 - (double) M1._02 * (double) M1._10 * (double) M1._31 + (double) M1._00 * (double) M1._12 * (double) M1._31 + (double) M1._01 * (double) M1._10 * (double) M1._32 - (double) M1._00 * (double) M1._11 * (double) M1._32 ),
			(float) ( (double) M1._01 * (double) M1._12 * (double) M1._20 - (double) M1._02 * (double) M1._11 * (double) M1._20 + (double) M1._02 * (double) M1._10 * (double) M1._21 - (double) M1._00 * (double) M1._12 * (double) M1._21 - (double) M1._01 * (double) M1._10 * (double) M1._22 + (double) M1._00 * (double) M1._11 * (double) M1._22 )
		};

		Multiply( M0, 1 / Determinant_Double( M1 ) );

		return M0;
	}
	//
	// Return the inverse of a 3x3 matrix.
	//

	inline
	NWN::Matrix33
	Inverse(
		__in const NWN::Matrix33 & M1
		)
	{
		return CreateMatrix33FromMatrix44(
			Inverse(
				CreateMatrix44FromMatrix33(
					M1 )
				)
			);
	}


	//
	// Transpose a matrix.
	//

	inline
	NWN::Matrix44
	Transpose(
		__in const NWN::Matrix44 & M1
		)
	{
		NWN::Matrix44 M0 =
		{
			M1._00, M1._10, M1._20, M1._30,
			M1._01, M1._11, M1._21, M1._31,
			M1._02, M1._12, M1._22, M1._32,
			M1._03, M1._13, M1._23, M1._33
		};

		return M0;
	}

	//
	// Return the affine inverse of a matrix.
	//

	NWN::Matrix44
	InverseAffine(
		__in const NWN::Matrix44 & M1
		);

	//
	// Create an identity matrix.
	//

	inline
	void
	CreateIdentityMatrix(
		__out NWN::Matrix44 & M
		)
	{
		memcpy( &M, &NWN::Matrix44::IDENTITY, sizeof( M ) );
	}

	//
	// Set the scale parameters in a matrix.
	//

	inline
	void
	SetScale(
		__inout NWN::Matrix44 & M,
		__in const NWN::Vector3 & Scale
		)
	{
		M._00 = Scale.x;
		M._11 = Scale.y;
		M._22 = Scale.z;
		M._33 = 1.0f;
	}

	//
	// Get the scale parameters in a matrix.
	//

	inline
	NWN::Vector3
	GetScale(
		__in const NWN::Matrix44 & M
		)
	{
		NWN::Vector3 Scale;

		Scale.x = M._00;
		Scale.y = M._11;
		Scale.z = M._22;

		return Scale;
	}

	//
	// Create a scale matrix.
	//

	inline
	void
	CreateScaleMatrix(
		__out NWN::Matrix44 & M,
		__in const NWN::Vector3 & Scale
		)
	{
		CreateIdentityMatrix( M );

		M._00 = Scale.x;
		M._11 = Scale.y;
		M._22 = Scale.z;
		M._33 = 1.0f;
	}

	//
	// Set the translation parameters in a matrix.
	//

	inline
	void
	SetTranslation(
		__inout NWN::Matrix44 & M,
		__in const NWN::Vector3 & Position
		)
	{
		M._03 = Position.x;
		M._13 = Position.y;
		M._23 = Position.z;
		M._33 = 1.0f;
	}

	//
	// Set the position parameters in a matrix.
	//

	inline
	void
	SetPosition(
		__inout NWN::Matrix44 & M,
		__in const NWN::Vector3 & Position
		)
	{
		M._30 = Position.x;
		M._31 = Position.y;
		M._32 = Position.z;
		M._33 = 1.0f;
	}

	//
	// Get the position parameters in a matrix.
	//

	inline
	NWN::Vector3
	GetPosition(
		__in const NWN::Matrix44 & M
		)
	{
		NWN::Vector3 v;

		v.x = M._30;
		v.y = M._31;
		v.z = M._32;

		return v;
	}

	//
	// Perform the inverse rotation function on a Vector3 with a 4x4 matrix.
	//

	inline
	NWN::Vector3
	InverseRotate(
		__in const NWN::Vector3 & v,
		__in const NWN::Matrix44 & M
		)
	{
		NWN::Vector3 vt;

		vt.x = v.x * M._00 + v.y * M._10 + v.z * M._20;
		vt.y = v.x * M._01 + v.y * M._11 + v.z * M._21;
		vt.z = v.x * M._02 + v.y * M._12 + v.z * M._22;

		return vt;
	}

	//
	// Perform the inverse transform function on a Vector3 with a 4x4 matrix.
	//

	inline
	NWN::Vector3
	InverseTransform(
		__in const NWN::Vector3 & v,
		__in const NWN::Matrix44 & M
		)
	{
		NWN::Vector3 vt;

		vt.x = v.x - M._03;
		vt.y = v.y - M._13;
		vt.z = v.z - M._23;

		return Math::InverseRotate( vt, M );
	}

	//
	// Create a translation matrix.
	//

	inline
	void
	CreateTranslationMatrix(
		__out NWN::Matrix44 & M,
		__in const NWN::Vector3 & Position
		)
	{
		CreateIdentityMatrix( M );
		SetTranslation( M, Position );
	}

	//
	// Create a look-at matrix (right-handed).
	//

	inline
	void
	CreateLookAtMatrixRH(
		__out NWN::Matrix44 & M,
		__in const NWN::Vector3 & Eye,
		__in const NWN::Vector3 & At,
		__in const NWN::Vector3 & Up
		)
	{
		NWN::Vector3 xaxis;
		NWN::Vector3 yaxis;
		NWN::Vector3 zaxis;

		zaxis = NormalizeVector( Subtract( Eye, At ) );
		xaxis = NormalizeVector( CrossProduct( Up, zaxis ) );
		yaxis = CrossProduct( zaxis, xaxis );

		M._00 = +xaxis.x;
		M._10 = +xaxis.y;
		M._20 = +xaxis.z;
		M._30 = -DotProduct( xaxis, Eye );

		M._01 = +yaxis.x;
		M._11 = +yaxis.y;
		M._21 = +yaxis.z;
		M._31 = -DotProduct( yaxis, Eye );

		M._02 = +zaxis.x;
		M._12 = +zaxis.y;
		M._22 = +zaxis.z;
		M._32 = -DotProduct( zaxis, Eye );

		M._03 = 0.0f;
		M._13 = 0.0f;
		M._23 = 0.0f;
		M._33 = 1.0f;
	}

	//
	// Create a look-at matrix (left-handed).
	//

	inline
	void
	CreateLookAtMatrixLH(
		__out NWN::Matrix44 & M,
		__in const NWN::Vector3 & Eye,
		__in const NWN::Vector3 & At,
		__in const NWN::Vector3 & Up
		)
	{
		NWN::Vector3 xaxis;
		NWN::Vector3 yaxis;
		NWN::Vector3 zaxis;

		zaxis = NormalizeVector( Subtract( At, Eye ) );
		xaxis = NormalizeVector( CrossProduct( Up, zaxis ) );
		yaxis = CrossProduct( zaxis, xaxis );

		M._00 = +xaxis.x;
		M._10 = +xaxis.y;
		M._20 = +xaxis.z;
		M._30 = -DotProduct( xaxis, Eye );

		M._01 = +yaxis.x;
		M._11 = +yaxis.y;
		M._21 = +yaxis.z;
		M._31 = -DotProduct( yaxis, Eye );

		M._02 = +zaxis.x;
		M._12 = +zaxis.y;
		M._22 = +zaxis.z;
		M._32 = -DotProduct( zaxis, Eye );

		M._03 = 0.0f;
		M._13 = 0.0f;
		M._23 = 0.0f;
		M._33 = 1.0f;
	}

	//
	// Create a rotation matrix (right-handed).
	//

	inline
	NWN::Matrix44
	CreateRotationXMatrixRH(
		__out NWN::Matrix44 & M,
		__in float A
		)
	{
		M._00 = 1.0f;
		M._01 = 0.0f;
		M._02 = 0.0f;
		M._03 = 0.0f;

		M._10 = 0.0f;
		M._11 = +cosf( A );
		M._12 = +sinf( A );
		M._13 = 0.0f;

		M._20 = 0.0f;
		M._21 = -sinf( A );
		M._22 = +cosf( A );
		M._23 = 0.0f;

		M._30 = 0.0f;
		M._31 = 0.0f;
		M._32 = 0.0f;
		M._33 = 1.0f;

		return M;
	}

	inline
	NWN::Matrix44
	CreateRotationYMatrixRH(
		__out NWN::Matrix44 & M,
		__in float A
		)
	{
		M._00 = +cosf( A );
		M._01 = 0.0f;
		M._02 = -sinf( A );
		M._03 = 0.0f;

		M._10 = 0.0f;
		M._11 = 1.0f;
		M._12 = 0.0f;
		M._13 = 0.0f;

		M._20 = +sinf( A );
		M._21 = 0.0f;
		M._22 = +cosf( A );
		M._23 = 0.0f;

		M._30 = 0.0f;
		M._31 = 0.0f;
		M._32 = 0.0f;
		M._33 = 1.0f;

		return M;
	}

	inline
	NWN::Matrix44
	CreateRotationZMatrixRH(
		__out NWN::Matrix44 & M,
		__in float A
		)
	{
		M._00 = +cosf( A );
		M._01 = +sinf( A );
		M._02 = 0.0f;
		M._03 = 0.0f;

		M._10 = -sinf( A );
		M._11 = +cosf( A );
		M._12 = 0.0f;
		M._13 = 0.0f;

		M._20 = 0.0f;
		M._21 = 0.0f;
		M._22 = 1.0f;
		M._23 = 0.0f;

		M._30 = 0.0f;
		M._31 = 0.0f;
		M._32 = 0.0f;
		M._33 = 1.0f;

		return M;
	}

	//
	// Create a rotation matrix about an arbitrary axis.
	//

	inline
	NWN::Matrix44
	CreateRotationAxisMatrix(
		__out NWN::Matrix44 & M,
		__in const NWN::Vector3 & V,
		__in float A
		)
	{
		NWN::Vector3 VN;
		float        A_cos;
		float        A_sin;

		CreateIdentityMatrix( M );

		VN = Math::NormalizeVector( V );

		A_cos = cos( A );
		A_sin = sin( A );

		M._00 = (1.0f - A_cos) * VN.x * VN.x + A_cos;
		M._10 = (1.0f - A_cos) * VN.x * VN.y - A_sin * VN.z;
		M._20 = (1.0f - A_cos) * VN.x * VN.z + A_sin * VN.y;
		M._01 = (1.0f - A_cos) * VN.y * VN.x + A_sin * VN.z;
		M._11 = (1.0f - A_cos) * VN.y * VN.y + A_cos;
		M._21 = (1.0f - A_cos) * VN.y * VN.z - A_sin * VN.x;
		M._02 = (1.0f - A_cos) * VN.z * VN.x - A_sin * VN.y;
		M._12 = (1.0f - A_cos) * VN.z * VN.y + A_sin * VN.x;
		M._22 = (1.0f - A_cos) * VN.z * VN.z + A_cos;

		return M;
	}

	//
	// Create a field of view perspective matrix (right-handed).
	//

	inline
	void
	CreatePerspectiveFovMatrixRH(
		__out NWN::Matrix44 & M,
		__in float fovy,
		__in float Aspect,
		__in float zn,
		__in float zf
		)
	{
		NWN::Vector2 Scale;

		Scale.y = 1.0f / tan( fovy / 2.0f);
		Scale.x = Scale.y / Aspect;

		M._00 = Scale.x;
		M._01 = 0.0f;
		M._02 = 0.0f;
		M._03 = 0.0f;

		M._10 = 0.0f;
		M._11 = Scale.y;
		M._12 = 0.0f;
		M._13 = 0.0f;

		M._20 = 0.0f;
		M._21 = 0.0f;
		M._22 = zf / (zn - zf);
		M._23 = -1.0f;

		M._30 = 0.0f;
		M._31 = 0.0f;
		M._32 = (zn * zf) / (zn - zf);
		M._33 = 0.0f;
	}

	inline
	void
	CreatePerspectiveFovMatrixLH(
		__inout NWN::Matrix44 & M,
		__in float fovy,
		__in float Aspect,
		__in float zn,
		__in float zf
		)
	{
		NWN::Vector2 Scale;

		Scale.y = 1.0f / tan( fovy / 2.0f);
		Scale.x = Scale.y / Aspect;

		M._00 = Scale.x;
		M._01 = 0.0f;
		M._02 = 0.0f;
		M._03 = 0.0f;

		M._10 = 0.0f;
		M._11 = Scale.y;
		M._12 = 0.0f;
		M._13 = 0.0f;

		M._20 = 0.0f;
		M._21 = 0.0f;
		M._22 = zf / (zn - zf);
		M._23 = 1.0f;

		M._30 = 0.0f;
		M._31 = 0.0f;
		M._32 = -zn * M._22;
		M._33 = 0.0f;
	}

	// http://www.j3d.org/matrix_faq/matrfaq_latest.html

	//
	// Retrieve the conjugate of a quaternion.
	//

	inline
	NWN::Quaternion
	Conjugate(
		__in const NWN::Quaternion & Q1
		)
	{
		NWN::Quaternion Q0;

		Q0.w = +Q1.w;
		Q0.x = -Q1.x;
		Q0.y = -Q1.y;
		Q0.z = -Q1.z;

		return Q0;
	}

	//
	// Return the magnitude of a quaternion.
	//

	inline
	float
	Magnitude(
		__in const NWN::Quaternion & Q
		)
	{
		return sqrtf(
			Q.w * Q.w +
			Q.x * Q.x +
			Q.y * Q.y +
			Q.z * Q.z);
	}

	//
	// Normalize a quaternion.
	//

	inline
	NWN::Quaternion
	Normalize(
		__in const NWN::Quaternion & Q1
		)
	{
		NWN::Quaternion Q0;
		float           M;

		Q0 = Q1;
		M  = Magnitude( Q1 );

		if (M <= Math::Epsilon)
			M = 1.0f;

		Q0.x /= M;
		Q0.y /= M;
		Q0.z /= M;
		Q0.w /= M;

		return Q0;
	}

	//
	// Return the inverse of a quaternion.
	//

	inline
	NWN::Quaternion
	Inverse(
		__in const NWN::Quaternion & Q1
		)
	{
		return Math::Normalize( Math::Conjugate( Q1 ) );
	}

	//
	// Multiply a vector by a quaternion.
	//

	inline
	NWN::Vector3
	Multiply(
		__in const NWN::Vector3 & V,
		__in const NWN::Quaternion & Q
		)
	{
		NWN::Vector3 uv;
		NWN::Vector3 uuv;
		NWN::Vector3 qv;

		qv.x = Q.x;
		qv.y = Q.y;
		qv.z = Q.z;

		uv  = Math::CrossProduct( qv, V );
		uuv = Math::CrossProduct( qv, uv );
		uv  = Math::Multiply( uv, 2.0f * Q.w );
		uuv = Math::Multiply( uuv, 2.0f );

		return Math::Add( Math::Add( V, uv ), uuv );
	}

	//
	// Convert a quaternion to a rotational matrix.
	//
	// As per http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
	//

	inline
	void
	SetRotationMatrix(
		__inout NWN::Matrix44 & M,
		__in const NWN::Quaternion & Q
		)
	{
		float sqw = Q.w * Q.w;
		float sqx = Q.x * Q.x;
		float sqy = Q.y * Q.y;
		float sqz = Q.z * Q.z;
		float invs;
		float t1;
		float t2;

		//
		// invs (inverse square lnegth) is only required if quaternion is not
		// already normalied.
		//

		invs = 1 / (sqx + sqy + sqz + sqw);

		M._00 = ( sqx - sqy - sqz + sqw) * invs; // since sqw + sqx + sqy + sqz =1/invs*invs
		M._11 = (-sqx + sqy - sqz + sqw) * invs;
		M._22 = (-sqx - sqy + sqz + sqw) * invs;

		t1 = Q.x * Q.y;
		t2 = Q.z * Q.w;

		M._10 = 2.0f * (t1 + t2) * invs;
		M._01 = 2.0f * (t1 - t2) * invs;

		t1 = Q.x * Q.z;
		t2 = Q.y * Q.w;

		M._20 = 2.0f * (t1 - t2) * invs;
		M._02 = 2.0f * (t1 + t2) * invs;

		t1 = Q.y * Q.z;
		t2 = Q.x * Q.w;

		M._21 = 2.0f * (t1 + t2) * invs;
		M._12 = 2.0f * (t1 - t2) * invs;
	}

	//
	// Convert a rotational matrix to a quaternion.
	//
	// As per http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
	// and: http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/christian.htm
	//

	inline
	NWN::Quaternion
	CreateRotationQuaternion(
		__in const NWN::Matrix44 & M
		)
	{
		NWN::Quaternion Q;

#if 0

		float trace = M._00 + M._11 + M._22;

		if( trace > 0 )
		{
			float s = 0.5f / sqrtf(trace+ 1.0f);
			Q.w = 0.25f / s;
			Q.x = ( M._21 - M._12 ) * s;
			Q.y = ( M._02 - M._20 ) * s;
			Q.z = ( M._10 - M._01 ) * s;
		}
		else
		{
			if ( M._00 > M._11 && M._00 > M._22 )
			{
				float s = 2.0f * sqrtf( 1.0f + M._00 - M._11 - M._22);
				Q.w = (M._21 - M._12 ) / s;
				Q.x = 0.25f * s;
				Q.y = (M._01 + M._10 ) / s;
				Q.z = (M._02 + M._20 ) / s;
			}
			else if (M._11 > M._22)
			{
				float s = 2.0f * sqrtf( 1.0f + M._11 - M._00 - M._22);
				Q.w = (M._02 - M._20 ) / s;
				Q.x = (M._01 + M._10 ) / s;
				Q.y = 0.25f * s;
				Q.z = (M._12 + M._21 ) / s;
			}
			else
			{
				float s = 2.0f * sqrtf( 1.0f + M._22 - M._00 - M._11 );
				Q.w = (M._10 - M._01 ) / s;
				Q.x = (M._02 + M._20 ) / s;
				Q.y = (M._12 + M._21 ) / s;
				Q.z = 0.25f * s;
			}
		}

#else

		Q.w = sqrtf( max( 0, 1 + M._00 + M._11 + M._22 ) ) / 2;
		Q.x = sqrtf( max( 0, 1 + M._00 - M._11 - M._22 ) ) / 2;
		Q.y = sqrtf( max( 0, 1 - M._00 + M._11 - M._22 ) ) / 2;
		Q.z = sqrtf( max( 0, 1 - M._00 - M._11 + M._22 ) ) / 2;
		Q.x = (float) _copysign( Q.x, M._21 - M._12 );
		Q.y = (float) _copysign( Q.y, M._02 - M._20 );
		Q.z = (float) _copysign( Q.z, M._10 - M._01 );

#endif

		return Q;
	}

	//
	// Decompose a scale/rotation/translation matrix into its components.
	// 

	void
	Matrix44SRTDecompose(
		__in const NWN::Matrix44 & WorldTrans,
		__out NWN::Vector3 & Scale,
		__out NWN::Vector3 & Translation,
		__out NWN::Matrix44 & Rotation
		);

	//
	// Decompose a scale/rotation/translation matrix into its components.
	// 

	void
	Matrix44SRTDecompose(
		__in const NWN::Matrix44 & WorldTrans,
		__out NWN::Vector3 & Scale,
		__out NWN::Vector3 & Translation,
		__out NWN::Quaternion & Rotation
		);

	//
	// Return a direction vector from a quaternion.
	//

	inline
	NWN::Vector3
	DirectionVectorFromQuaternion(
		__in const NWN::Quaternion & Q
		)
	{
		NWN::Matrix44 RotationMatrix;
		NWN::Vector3  v;

		SetRotationMatrix( RotationMatrix, Q );

		v.x = 0.0f;
		v.y = 1.0f;
		v.z = 0.0f;

		return Math::Multiply( RotationMatrix, v );
	}

	//
	// Calculate whether a point resides within a bounding box.
	//

	inline
	bool
	PointInRect(
		__in const NWN::Rect & Rect,
		__in const NWN::Vector2 & pt
		)
	{
		if ((pt.x - Rect.left < Epsilon)  ||
		    (pt.x - Rect.right > Epsilon) ||
		    (pt.y - Rect.top < Epsilon)   ||
		    (pt.y - Rect.bottom > Epsilon))
		{
			return false;
		}

		return true;
	}

	//
	// Calculate whether a point resides within a bounding box.
	//

	inline
	bool
	PointInRect(
		__in const RectFP & Rect,
		__in const Vector2FP & pt
		)
	{
		if ((pt.x < Rect.left)    ||
		    (pt.x > Rect.right)   ||
		    (pt.y < Rect.top)     ||
		    (pt.y > Rect.bottom))
		{
			return false;
		}

		return true;
	}

	//
	// Calculate whether a point is on the left side of a line drawn infinitely
	// into the distance of a plane.
	//

	inline
	bool
	PointInLeftHalfspace(
		__in const NWN::Vector2 & Start,
		__in const NWN::Vector2 & End,
		__in const NWN::Vector2 & pt
		)
	{
		NWN::Vector2 Line;
		NWN::Vector2 Line90;
		NWN::Vector2 v;

		Line.x = End.x - Start.x;
		Line.y = End.y - Start.y;
		v.x    = pt.x - Start.x;
		v.y    = pt.y - Start.y;

		//
		// Rotate 90 degrees clockwise.
		//

		Line90.x =  Line.y;
		Line90.y = -Line.x;

		return DotProduct( Line90, v ) <= 0.0f;
	}

	//
	// Calculate whether a triangle is spun clockwise.
	//

	inline
	bool
	TriangleSpunClockwise(
		__in_ecount(3) const NWN::Vector2 * Tri
		)
	{
		return PointInLeftHalfspace( Tri[ 1 ], Tri[ 0 ], Tri[ 2 ] );
	}


	//
	// Calculate whether a point resides within a triangle.
	//

	inline
	bool
	PointInTriangle(
		__in_ecount(3) const NWN::Vector2 * Tri,
		__in const NWN::Vector2 & pt,
		__in bool Clockwise
		)
	{
		if (Clockwise)
		{
			return ((PointInLeftHalfspace( Tri[ 1 ], Tri[ 0 ], pt )) &&
			        (PointInLeftHalfspace( Tri[ 2 ], Tri[ 1 ], pt )) &&
			        (PointInLeftHalfspace( Tri[ 0 ], Tri[ 2 ], pt )));
		}
		else
		{
			return ((PointInLeftHalfspace( Tri[ 0 ], Tri[ 1 ], pt )) &&
			        (PointInLeftHalfspace( Tri[ 1 ], Tri[ 2 ], pt )) &&
			        (PointInLeftHalfspace( Tri[ 2 ], Tri[ 0 ], pt )));
		}
	}

	//
	// Return the normal of a triangle.
	//

	inline
	NWN::Vector3
	ComputeNormalTriangle(
		__in_ecount(3) const NWN::Vector3 * Tri
		)
	{
		NWN::Vector3 v1;
		NWN::Vector3 v2;

		v1.x = Tri[ 1 ].x - Tri[ 0 ].x;
		v1.y = Tri[ 1 ].y - Tri[ 0 ].y;
		v1.z = Tri[ 1 ].z - Tri[ 0 ].z;

		v2.x = Tri[ 2 ].x - Tri[ 0 ].x;
		v2.y = Tri[ 2 ].y - Tri[ 0 ].y;
		v2.z = Tri[ 2 ].z - Tri[ 0 ].z;

		return Math::NormalizeVector( Math::CrossProduct( v1, v2 ) );
	}

	//
	// Return the coordinates of a point given its distance along the path of
	// a ray.
	//

	inline
	NWN::Vector3
	PointFromRayDistance(
		__in const NWN::Vector3 & Origin,
		__in const NWN::Vector3 & NormDir,
		__in float Distance
		)
	{
		NWN::Vector3 v;

		v.x = Origin.x + (NormDir.x * Distance);
		v.y = Origin.y + (NormDir.y * Distance);
		v.z = Origin.z + (NormDir.z * Distance);

		return v;
	}

	//
	// Return the coordinates of a point given its distance along the path of
	// a ray.
	//

	inline
	NWN::Vector2
	PointFromRayDistance(
		__in const NWN::Vector2 & Origin,
		__in const NWN::Vector2 & NormDir,
		__in float Distance
		)
	{
		NWN::Vector2 v;

		v.x = Origin.x + (NormDir.x * Distance);
		v.y = Origin.y + (NormDir.y * Distance);

		return v;
	}

	inline
	double
	PlaneHeightAtPoint(
		__in const NWN::Vector3 & Normal,
		__in float D,
		__in const NWN::Vector2 & Pt
		)
	{
		NWN::Vector2 Normal2;

		if (Normal.z == 0.0f)
			return 0.0f;

		Normal2.x = Normal.x;
		Normal2.y = Normal.y;

		return -((DotProduct( Normal2, Pt ) + D) / Normal.z);
	}

	//
	// Calculate the distance from a ray origin towards a plane, should the ray
	// intersect the plane.  Should the ray not intersect the plane, a negative
	// distance is returned.
	//

	inline
	float
	RayPlaneDistance(
		__in const NWN::Vector3 & Origin,
		__in const NWN::Vector3 & NormDir,
		__in const NWN::Vector3 & PlaneNormal,
		__in float PlaneD
		)
	{
		float cA;
		float dD;

		cA = DotProduct( NormDir, PlaneNormal );

		//
		// If we're parallel, there's no intersection.
		//

		if (fabsf( cA ) <= Math::Epsilon)
			return -1.0f;

		dD = PlaneD - DotProduct( Origin, PlaneNormal );

		return dD / cA;
	}

	template< class T >
	inline
	T
	GCD(
		__in T a,
		__in T b
		)
	{
		T t;

		while (b != 0)
		{
			t = b;
			b = a % b;
			a = t;
		}

		return a;
	}

	//
	// Distance calculation helper.
	//

	inline
	float
	Distance(
		__in const NWN::Vector3 & v1,
		__in const NWN::Vector3 & v2
		)
	{
		double d;
		
		d = (v2.x - v1.x) * (v2.x - v1.x) +
			(v2.y - v1.y) * (v2.y - v1.y) +
			(v2.z - v1.z) * (v2.z - v1.z);

		return (float) sqrt( d );
	}

	inline
	float
	Distance(
		__in const NWN::Vector2 & v1,
		__in const NWN::Vector2 & v2
		)
	{
		double d;
		
		d = (v2.x - v1.x) * (v2.x - v1.x) +
			(v2.y - v1.y) * (v2.y - v1.y);

		return (float) sqrt( d );
	}


	inline
	float
	DistanceSq(
		__in const NWN::Vector3 & v1,
		__in const NWN::Vector3 & v2
		)
	{
		double d;
		
		d = (v2.x - v1.x) * (v2.x - v1.x) +
			(v2.y - v1.y) * (v2.y - v1.y) +
			(v2.z - v1.z) * (v2.z - v1.z);

		return (float) d;
	}

	inline
	float
	DistanceSq(
		__in const NWN::Vector2 & v1,
		__in const NWN::Vector2 & v2
		)
	{
		double d;
		
		d = (v2.x - v1.x) * (v2.x - v1.x) +
			(v2.y - v1.y) * (v2.y - v1.y);

		return (float) d;
	}

	//
	// Return the 2D centroid of a polygon.
	//

	NWN::Vector2
	PolygonCentroid2(
		__in const Vector2Vec & Polygon
		);

	//
	// Test if a point resides within a 2D polygon.
	//

	bool
	PointInPolygonRegion(
		__in const NWN::Vector2 & v,
		__in const Vector2Vec & Polygon,
		__in const unsigned int FixedPointShift = Math::FixedPointShift
		);

	//
	// Test if a point resides within a 2D polygon (Z coordinate ignored).
	//

	bool
	PointInPolygonRegion(
		__in const NWN::Vector3 & v,
		__in const Vector3Vec & Polygon,
		__in const unsigned int FixedPointShift = Math::FixedPointShift
		);

	//
	// Test if a point resides within a 2D polygon.
	//

	bool
	PointInPolygonRegion(
		__in const Vector2FP & v,
		__in const Vector2FPVec & Polygon
		);

	//
	// Intersect a ray with a triangle and return the distance to the
	// intersection point on successful intersection.
	//

	bool
	IntersectRayTri(
		__in const NWN::Vector3 & Origin,
		__in const NWN::Vector3 & NormDir,
		__in_ecount(3) const NWN::Vector3 * Tri,
		__out float & T
		);

	//
	// Intersect a ray with a triangle and return the distance to the
	// intersection point on successful intersection.
	//
	// Backfaces are not considered intersecting.
	//

	bool
	IntersectRayTriRejectBackface(
		__in const NWN::Vector3 & Origin,
		__in const NWN::Vector3 & NormDir,
		__in_ecount(3) const NWN::Vector3 * Tri,
		__out float & T
		);

	//
	// Intersect a ray with a sphere.
	//

	bool
	IntersectRaySphere(
		__in const NWN::Vector3 & RayOrigin,
		__in const NWN::Vector3 & RayNormDir,
		__in const NWN::Vector3 & SphereOrigin,
		__in const float SphereRadiusSq,
		__out float & T
		);

	//
	// Intersect two line segments and return whether they actually crossed or
	// not.  The routine also calculates whether the line segments are parallel
	// and returns the parallel status via the Parallel argument.
	//

	bool
	IntersectSegments2(
		__in const NWN::Vector2 & s1_p1,
		__in const NWN::Vector2 & s1_p2,
		__in const NWN::Vector2 & s2_p1,
		__in const NWN::Vector2 & s2_p2,
		__out NWN::Vector2 & IntersectionPoint,
		__out bool & Parallel
		);

	//
	// Intersect a line segment and a polygon, and return the intersection
	// segment (if any).
	//

	bool
	IntersectSegmentPolygon(
		__in const NWN::Vector2 & s_p1,
		__in const NWN::Vector2 & s_p2,
		__in_ecount( NumPoints ) const NWN::Vector2 * PolygonPoints,
		__in size_t NumPoints,
		__out NWN::Vector2 & I_p1,
		__out NWN::Vector2 & I_p2
		);

	//
	// Determine whether a line segment bounded by two points intersects with
	// a single point.

	bool
	PointInSegment(
		__in const NWN::Vector2 & s_p1,
		__in const NWN::Vector2 & s_p2,
		__in const NWN::Vector2 & pt
		);
	
	//
	// Line projection.
	//

	inline
	NWN::Vector3
	LineProject3(
		__in const NWN::Vector3 & P0,
		__in const NWN::Vector3 & P1,
		__in const NWN::Vector3 & A
		)
	{
		NWN::Vector3 W;
		float        T;
		float        D;

		W = Math::Subtract( P1, P0 );
		D = (sqrtf( W.x ) + sqrtf( W.y ) + sqrtf( W.z ));
		T = Math::DotProduct( W, Math::Subtract( A, P0 ) );

		if (fabsf( D ) > Math::Epsilon)
			T /= D;

		return Math::Add( P0, Math::Multiply( W, T ) );
	}

	//
	// Fast ray / box intersection.
	//
	// Courtesy http://people.csail.mit.edu/amy/papers/box-jgt.pdf
	//

	class QuickRay
	{

	public:

		inline
		QuickRay(
			__in const NWN::Vector3 & o,
			__in const NWN::Vector3 & d
			)
			: m_origin( o ),
			  m_direction( d )
		{
			m_origin          = o;
			m_direction       = d;
			m_inv_direction.x = 1.0f / d.x;
			m_inv_direction.y = 1.0f / d.y;
			m_inv_direction.z = 1.0f / d.z;
			m_sign[ 0 ]       = (m_inv_direction.x < 0.0f);
			m_sign[ 1 ]       = (m_inv_direction.y < 0.0f);
			m_sign[ 2 ]       = (m_inv_direction.z < 0.0f);
		}

		NWN::Vector3 m_origin;
		NWN::Vector3 m_direction;
		NWN::Vector3 m_inv_direction;
		int          m_sign[ 3 ];

	};

	class QuickBox
	{

	public:

		inline
		QuickBox(
			__in const NWN::Vector3 & min,
			__in const NWN::Vector3 & max
			)
		{
			m_bounds[ 0 ] = min;
			m_bounds[ 1 ] = max;
		}

		inline
		bool
		IntersectRay(
			__in const QuickRay & r,
			__in float t0 = 0.0f,
			__in float t1 = FLT_MAX
			) const
		{
			float tmin;
			float tmax;
			float tymin;
			float tymax;
			float tzmin;
			float tzmax;

			tmin  = (m_bounds[ r.m_sign[ 0 ] ].x - r.m_origin.x) * r.m_inv_direction.x;
			tmax  = (m_bounds[ 1-r.m_sign[ 0 ] ].x - r.m_origin.x) * r.m_inv_direction.x;
			tymin = (m_bounds[ r.m_sign[ 1 ] ].y - r.m_origin.y) * r.m_inv_direction.y;
			tymax = (m_bounds[ 1-r.m_sign[ 1 ] ].y - r.m_origin.y) * r.m_inv_direction.y;

			if ( (tmin > tymax) || (tymin > tmax) )
				return false;

			if (tymin > tmin)
				tmin = tymin;
			if (tymax < tmax)
				tmax = tymax;

			tzmin = (m_bounds[ r.m_sign[ 2 ] ].z - r.m_origin.z) * r.m_inv_direction.z;
			tzmax = (m_bounds[ 1-r.m_sign[ 2 ] ].z - r.m_origin.z) * r.m_inv_direction.z;

			if ( (tmin > tzmax) || (tzmin > tmax) )
				return false;
			if (tzmin > tmin)
				tmin = tzmin;
			if (tzmax < tmax)
				tmax = tzmax;

			return ( (tmin < t1) && (tmax > t0) );
		}

		inline
		void
		SetBounds(
			__in const NWN::Vector3 & MinBound,
			__in const NWN::Vector3 & MaxBound
			)
		{
			m_bounds[ 0 ] = MinBound;
			m_bounds[ 1 ] = MaxBound;
		}

		inline
		bool
		IntersectPoint(
			__in const NWN::Vector3 & Point
			) const
		{
			return Point.x >= m_bounds[ 0 ].x &&
			       Point.x <= m_bounds[ 1 ].x &&
			       Point.y >= m_bounds[ 0 ].y &&
			       Point.y <= m_bounds[ 1 ].y &&
			       Point.z >= m_bounds[ 0 ].z &&
			       Point.z <= m_bounds[ 1 ].z;
		}

	private:

		NWN::Vector3 m_bounds[ 2 ];

	};

	//
	// Define a simple 2D bounding box.
	//

	class BoundBox2
	{

	public:

		inline
		BoundBox2(
			__in const NWN::Vector2 & BoundMin,
			__in const NWN::Vector2 & BoundMax
			)
		: m_MinBound( BoundMin ),
		  m_MaxBound( BoundMax )
		{
		}

		inline
		~BoundBox2(
			)
		{
		}

		//
		// Check if a point resides within the bounded region.
		//

		inline
		bool
		PointIntersect(
			__in const NWN::Vector2 & v
			) const
		{
			if ((v.x >= m_MinBound.x) &&
			    (v.x <= m_MaxBound.x) &&
			    (v.y >= m_MinBound.y) &&
			    (v.y <= m_MaxBound.y))
			{
				return true;
			}

			return false;
		}

		inline
		void
		SetBounds(
			__in const NWN::Vector2 & MinBound,
			__in const NWN::Vector2 & MaxBound
			)
		{
			m_MinBound = MinBound;
			m_MaxBound = MaxBound;
		}

	private:

		NWN::Vector2 m_MinBound;
		NWN::Vector2 m_MaxBound;

	};

	//
	// Optimized 3D sphere collision testing object.
	//

	class QuickSphere
	{

	public:

		QuickSphere(
			__in const NWN::Vector3 & Origin,
			__in float Radius
			)
		: m_Origin( Origin ),
		  m_RadiusSq( Radius * Radius )
		{
		}

		inline
		bool
		IntersectPoint(
			__in const NWN::Vector3 & Point
			) const
		{
			return (Math::DistanceSq( m_Origin, Point ) <= m_RadiusSq);
		}

	private:

		NWN::Vector3 m_Origin;
		float        m_RadiusSq;

	};

	//
	// Optimized 3D cylinder collision testing object (allows for arbitrary
	// orientations).
	//
	// Courtesy Greg James - gjames@nvidia.com
	//

	class QuickCylinder
	{

	public:

		QuickCylinder(
			__in const NWN::Vector3 & Origin,
			__in const NWN::Vector3 & Terminator,
			__in float Length,
			__in float Radius
			)
		: m_Origin( Origin ),
		  m_OriginToTerm( Math::Subtract( Terminator, Origin ) ),
		  m_LengthSq( Length * Length ),
		  m_RadiusSq( Radius * Radius )
		{
		}

		//
		// Determine whether a point lies within the cylinder.  If the point is
		// outside the cylinder, -1.0f is returned, else the distance squared
		// from the cylinder center is returned.
		//

		inline
		float
		IntersectPoint(
			__in const NWN::Vector3 & Point
			) const
		{
			NWN::Vector3 pd;
			float        dot;
			float        dsq;

			//
			// Dot the m_OriginToTerm and pd vectors to see if the point lies
			// behind the cylinder cap at m_Origin.
			//

			pd  = Math::Subtract( Point, m_Origin );
			dot = Math::DotProduct( m_OriginToTerm, pd );

			//
			// If dot is less than zero, then the point is behind the m_Origin
			// cap.
			// If dot is greater than the cylinder axis line segment length,
			// then the point is outside the other end cap at m_Terminator.
			//

			if ((dot < 0.0f) || (dot > m_LengthSq))
				return -1.0f;

			//
			// The point resides within the parallel caps, so find the distance
			// squared from point to line, using the fact that sin^2 + cos^2
			// = 1.
			//
			// dot = cos * |d||pd|, and cross*cross = sin^2 | |d|^2 * |pd|^2
			//
			// (* -> multiply for scalars and dotproduct for vectors)
			//
			// In short, where dist is the point distance to cylinder axis:
			//
			// dist = sin( pd to d ) * |pd|
			// distsq = dsq = (1 - cos^2( pd to d)) * |pd|^2
			// dsq = pd * pd - dot * dot / m_LengthSq
			//

			dsq = (Math::DotProduct( pd, pd )) - dot * dot / m_LengthSq;

			if (dsq > m_RadiusSq)
				return -1.0f;
			else
				return dsq;
		}

	private:

		NWN::Vector3  m_Origin;
		NWN::Vector3  m_OriginToTerm;
		float         m_LengthSq;
		float         m_RadiusSq;

	};

	//
	// Optimized cone collision testing object (allows for arbitrary
	// orientations).
	//

	class QuickCone
	{

	public:

		QuickCone(
			__in const NWN::Vector3 & Origin,
			__in const NWN::Vector3 & Terminator,
			__in float Length,
			__in float Radius
			)
		: m_Origin( Origin ),
		  m_OriginToTerm( Math::Subtract( Terminator, Origin ) ),
		  m_LengthSq( Length * Length ),
		  m_RadiusSq( Radius * Radius )
		{
		}

		//
		// Determine whether a point lies within the cylinder.  If the point is
		// outside the cylinder, -1.0f is returned, else the distance squared
		// from the cylinder center is returned.
		//

		inline
		float
		IntersectPoint(
			__in const NWN::Vector3 & Point
			) const
		{
			NWN::Vector3 pd;
			float        dot;
			float        dsq;
			float        T;

			//
			// Dot the m_OriginToTerm and pd vectors to see if the point lies
			// behind the cylinder cap at m_Origin.
			//

			pd  = Math::Subtract( Point, m_Origin );
			dot = Math::DotProduct( m_OriginToTerm, pd );

			//
			// If dot is less than zero, then the point is behind the m_Origin
			// cap.
			// If dot is greater than the cylinder axis line segment length,
			// then the point is outside the other end cap at m_Terminator.
			//

			if ((dot < 0.0f) || (dot > m_LengthSq))
				return -1.0f;

			T = dot / m_LengthSq;

			//
			// The point resides within the parallel caps, so find the distance
			// squared from point to line, using the fact that sin^2 + cos^2
			// = 1.
			//
			// dot = cos * |d||pd|, and cross*cross = sin^2 | |d|^2 * |pd|^2
			//
			// (* -> multiply for scalars and dotproduct for vectors)
			//
			// In short, where dist is the point distance to cylinder axis:
			//
			// dist = sin( pd to d ) * |pd|
			// distsq = dsq = (1 - cos^2( pd to d)) * |pd|^2
			// dsq = pd * pd - dot * dot / m_LengthSq
			//

			dsq = (Math::DotProduct( pd, pd )) - dot * dot / m_LengthSq;

			if (dsq > m_RadiusSq * (T * T))
				return -1.0f;
			else
				return dsq;
		}

	private:

		NWN::Vector3  m_Origin;
		NWN::Vector3  m_OriginToTerm;
		float         m_LengthSq;
		float         m_RadiusSq;

	};

}

#endif
