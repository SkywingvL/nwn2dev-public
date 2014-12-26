/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	MathOps.cpp

Abstract:

	This module houses the implementation of various mathematical operators
	useful throughout the client extension (particularly when dealing with
	coordinate systems and pathing).

--*/

#include "Precomp.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "MathOps.h"

//
// Polygon hit test from inpoly.cpp.
//

int                                /*   1=inside, 0=outside                */
inpoly(                            /* is target point inside a 2D polygon? */
const unsigned int *poly,          /*   polygon points, [0]=x, [1]=y       */
int npoints,                       /*   number of points in polygon        */
unsigned int xt,                   /*   x (horizontal) of target point     */
unsigned int yt);                  /*   y (vertical) of target point       */

static
int
IsPointLeftOnLine(
	__in unsigned int lx1,
	__in unsigned int ly1,
	__in unsigned int lx2,
	__in unsigned int ly2,
	__in unsigned int x,
	__in unsigned int y
	)
/*++

Routine Description:

	This routine determines the location disposition of a point with respect to
	an infinite line.

	The point may be considered on the "left", "right", or "on" the infinite
	line.

	This routine is patterned off of logic from [SOFTSUFTER], "Point in Polygon
	Winding Number Inclusion, (C) 2001 softSufter (www.softsufter.com)".

	See http://softsurfer.com/Archive/algorithm_0103/algorithm_0103.htm for
	details.

Arguments:

	lx1 - Supplies the x-axis coordinate of the first line definition point.

	ly1 - Supplies the y-axis coordinate of the first line definition point.

	lx2 - Supplies the x-axis coordinate of the second line definition point.

	ly2 - Supplies the y-axis coordinate of the second line definition point.

	x - Supplies the x-coordinate of the point to hit test.

	y - Supplies the y-coordinate of the point to hit test.

Return Value:

	The routine returns a value indicating the point's disposition to the line,
	as drawn from the following table:

	Return value    Meaning
	------------    -------

	1               Point is to the left of the line.

	0               Point is located on the line.

	-1              Point is to the right of the line.

Environment:

	User mode.

--*/
{
	signed int t1;
	signed int t2;

	t1 = ((int) lx2 - (int) lx1) * ((int) y - (int) ly1);
	t2 = ((int) x - (int) lx1) * ((int) ly2 - (int) ly1);

	if (t1 == t2)
		return 0;
	else if (t1 < t2)
		return -1;
	else
		return 1;
}


static
int
CountWindingNumber2(
	__in const unsigned int * Polygon,
	__in unsigned int Points,
	__in unsigned int x,
	__in unsigned int y
	)
/*++

Routine Description:

	This routine counts the winding number of a polygon to determine whether a
	single point resides entirely within the polygon.

	A point on any edge of the polygon is considered a match.

	This routine is patterned off of logic from [SOFTSUFTER], "Point in Polygon
	Winding Number Inclusion, (C) 2001 softSufter (www.softsufter.com)".

	See http://softsurfer.com/Archive/algorithm_0103/algorithm_0103.htm for
	details.

Arguments:

	Polygon - Supplies the polygon verticies to intersect.  The x-coordinate is
	          expressed, following the y-coordinate.

	Points - Supplies the count, in whole points, of the polygon array.

	x - Supplies the x-coordinate of the point to hit test.

	y - Supplies the y-coordinate of the point to hit test.

Return Value:

	Returns the winding number of the point about the specified polygon.
	Should the winding number be zero, then the point is located entirely
	within the polygon.

Environment:

	User mode.

--*/
{
	int           WindingNumber;
	unsigned int  ni;
	int           Disposition;

	enum { X__ = 0, Y__ = 1 };

	WindingNumber = 0;

	//
	// Iterate over all polygon edges, summing winding numbers.
	//

	for (unsigned int i = 0; i < Points; i += 1)
	{
		ni = (i + 1) == Points ? 0 : (i + 1);

		if (Polygon[ i * 2 + Y__ ] <= y)
		{
			if (Polygon[ ni * 2 + Y__ ] > y)
			{
				//
				// We're in a downward crossing.
				//

				WindingNumber += 1;

				Disposition = IsPointLeftOnLine(
					Polygon[  i * 2 + X__ ],
					Polygon[  i * 2 + Y__ ],
					Polygon[ ni * 2 + X__ ],
					Polygon[ ni * 2 + Y__ ],
					x,
					y);

				if (Disposition > 0)
					WindingNumber += 1;
			}
		}
		else
		{
			if (Polygon[ ni * 2 + Y__ ] <= y)
			{
				//
				// We're in an upward crossing.
				//

				WindingNumber -= 1;

				Disposition = IsPointLeftOnLine(
					Polygon[  i * 2 + X__ ],
					Polygon[  i * 2 + Y__ ],
					Polygon[ ni * 2 + X__ ],
					Polygon[ ni * 2 + Y__ ],
					x,
					y) ;

				if (Disposition < 0)
					WindingNumber -= 1;
			}
		}
	}

	return WindingNumber;
}

#if 0
static
int
CountWindingNumber(
	__in const unsigned int * Polygon,
	__in unsigned int Points,
	__in unsigned int x,
	__in unsigned int y
	)
/*++

Routine Description:

	This routine counts the winding number of a polygon to determine whether a
	single point resides entirely within the polygon.

	A point on a left or bottom edge is considered to be within the polygon,
	while a point on the right or top edge is considered to be not within the
	polygon.

	This routine is patterned off of logic from [SOFTSUFTER], "Point in Polygon
	Winding Number Inclusion, (C) 2001 softSufter (www.softsufter.com)".

	See http://softsurfer.com/Archive/algorithm_0103/algorithm_0103.htm for
	details.

Arguments:

	Polygon - Supplies the polygon verticies to intersect.  The x-coordinate is
	          expressed, following the y-coordinate.

	Points - Supplies the count, in whole points, of the polygon array.

	x - Supplies the x-coordinate of the point to hit test.

	y - Supplies the y-coordinate of the point to hit test.

Return Value:

	Returns the winding number of the point about the specified polygon.
	Should the winding number be zero, then the point is located entirely
	within the polygon.

Environment:

	User mode.

--*/
{
	int           WindingNumber;
	unsigned int  ni;
	int           Disposition;

	enum { X__ = 0, Y__ = 1 };

	WindingNumber = 0;

	//
	// Iterate over all polygon edges, summing winding numbers.
	//

	for (unsigned int i = 0; i < Points; i += 1)
	{
		ni = (i + 1) == Points ? 0 : (i + 1);

		if (Polygon[ i * 2 + Y__ ] <= y)
		{
			if (Polygon[ ni * 2 + Y__ ] > y)
			{
				//
				// We're in a downward crossing.
				//

				Disposition = IsPointLeftOnLine(
					Polygon[  i * 2 + X__ ],
					Polygon[  i * 2 + Y__ ],
					Polygon[ ni * 2 + X__ ],
					Polygon[ ni * 2 + Y__ ],
					x,
					y);

				if (Disposition > 0)
					WindingNumber += 1;
			}
		}
		else
		{
			if (Polygon[ ni * 2 + Y__ ] <= y)
			{
				//
				// We're in an upward crossing.
				//

				Disposition = IsPointLeftOnLine(
					Polygon[  i * 2 + X__ ],
					Polygon[  i * 2 + Y__ ],
					Polygon[ ni * 2 + X__ ],
					Polygon[ ni * 2 + Y__ ],
					x,
					y) ;

				if (Disposition < 0)
					WindingNumber -= 1;
			}
		}
	}

	return WindingNumber;
}
#endif

NWN::Matrix44
AffineInverse(
	__in const NWN::Matrix44 & M1
	)
/*++

Routine Description:

	This routine calculates the inverse of an affine 3D matrix with the last
	row assumed to 0, 0, 0, 1.

	This routine is based on Graphic Gems (II), inverse.c.

Arguments:

	M - Supplies the affine matrix to invert.

Return Value:

	The routine returns the inverted matrix.

Environment:

	User mode.

--*/
{
	float            det_1;
	float            pos;
	float            neg;
	float            temp;
	NWN::Matrix44    M0;

	#define ACCUMULATE    \
	    if (temp >= 0.0)  \
	        pos += temp;  \
	    else              \
	        neg += temp;

	#define PRECISION_LIMIT Math::Epsilon

	/*
	 * Calculate the determinant of submatrix A and determine if the
	 * the matrix is singular as limited by the double precision
	 * floating-point data representation.
	*/
	pos = neg = 0.0;
	temp =  M1._00 * M1._11 * M1._22;
	ACCUMULATE
	temp =  M1._01 * M1._12 * M1._20;
	ACCUMULATE
	temp =  M1._02 * M1._10 * M1._21;
	ACCUMULATE
	temp = -M1._02 * M1._11 * M1._20;
	ACCUMULATE
	temp = -M1._01 * M1._10 * M1._22;
	ACCUMULATE
	temp = -M1._00 * M1._12 * M1._21;
	ACCUMULATE
	det_1 = pos + neg;

	/* Is the submatrix A singular? */
	if ((det_1 == 0.0) || (fabsf(det_1 / (pos - neg)) < PRECISION_LIMIT))
	{
		return M1;
	}
	else
	{
		/* Calculate inverse(A) = adj(A) / det(A) */
		det_1 = 1.0f / det_1;
		M0._00 =   ( M1._11 * M1._22 -
					 M1._12 * M1._21 )
				   * det_1;
		M0._10 = - ( M1._10 * M1._22 -
					 M1._12 * M1._20 )
				   * det_1;
		M0._20 =   ( M1._10 * M1._21 -
					 M1._11 * M1._20 )
				   * det_1;
		M0._01 = - ( M1._01 * M1._22 -
					 M1._02 * M1._21 )
				   * det_1;
		M0._11 =   ( M1._00 * M1._22 -
					 M1._02 * M1._20 )
				   * det_1;
		M0._21 = - ( M1._00 * M1._21 -
					 M1._01 * M1._20 )
				   * det_1;
		M0._02 =   ( M1._01 * M1._12 -
					 M1._02 * M1._11 )
				   * det_1;
		M0._12 = - ( M1._00 * M1._12 -
					 M1._02 * M1._10 )
				   * det_1;
		M0._22 =   ( M1._00 * M1._11 -
					 M1._01 * M1._10 )
				   * det_1;

		/* Calculate -C * inverse(A) */
		M0._30 = - ( M1._30 * M0._00 +
					 M1._31 * M0._10 +
					 M1._32 * M0._20 );
		M0._31 = - ( M1._30 * M0._01 +
					 M1._31 * M0._11 +
					 M1._32 * M0._21 );
		M0._32 = - ( M1._30 * M0._02 +
					 M1._31 * M0._12 +
					 M1._32 * M0._22 );

		/* Fill in last column */
		M0._03 = M0._13 = M0._23 = 0.0;
		M0._33 = 1.0;

		return M0;
	}
}

void
Math::Matrix44SRTDecompose(
	__in const NWN::Matrix44 & WorldTrans,
	__out NWN::Vector3 & Scale,
	__out NWN::Vector3 & Translation,
	__out NWN::Matrix44 & Rotation
	)
/*++

Routine Description:

	This routine decomposes a 4x4 Scale/Rotation/Translation matrix into its
	individual components.

	The input matrix must be a legal SRT matrix.

Arguments:

	WorldTrans - Supplies a 4x4 SRT matrix to decompose.

	Scale - Receives the scale component.

	Translation - Receives the translation component.

	Rotation - Receives the rotation component.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	NWN::Vector3  Basis[ 3 ];
	NWN::Matrix44 M;

	Basis[ 0 ] = *(const NWN::Vector3 *) &WorldTrans._00;
	Basis[ 1 ] = *(const NWN::Vector3 *) &WorldTrans._10;
	Basis[ 2 ] = *(const NWN::Vector3 *) &WorldTrans._20;

	//
	// Break out the scale component.
	//

	Scale.x = Math::LengthVector( Basis[ 0 ] );
	Scale.y = Math::LengthVector( Basis[ 1 ] );
	Scale.z = Math::LengthVector( Basis[ 2 ] );

	//
	// Break out the translation component.
	//

	Translation = Math::GetPosition( WorldTrans );

	//
	// Finally, break out the rotation component.
	//

	Math::CreateIdentityMatrix( M );

	*(NWN::Vector3 *) &M._00 = Math::Multiply( Basis[ 0 ], 1.0f / Scale.x );
	*(NWN::Vector3 *) &M._10 = Math::Multiply( Basis[ 1 ], 1.0f / Scale.y );
	*(NWN::Vector3 *) &M._20 = Math::Multiply( Basis[ 2 ], 1.0f / Scale.z );

	Rotation = M;
}

void
Math::Matrix44SRTDecompose(
	__in const NWN::Matrix44 & WorldTrans,
	__out NWN::Vector3 & Scale,
	__out NWN::Vector3 & Translation,
	__out NWN::Quaternion & Rotation
	)
/*++

Routine Description:

	This routine decomposes a 4x4 Scale/Rotation/Translation matrix into its
	individual components.

	The input matrix must be a legal SRT matrix.

Arguments:

	WorldTrans - Supplies a 4x4 SRT matrix to decompose.

	Scale - Receives the scale component.

	Translation - Receives the translation component.

	Rotation - Receives the rotation component.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	NWN::Matrix44 RotationMatrix;

	//
	// First perform the main decomposition step.
	//

	Math::Matrix44SRTDecompose(
		WorldTrans,
		Scale,
		Translation,
		RotationMatrix);

	//
	// Now just map the rotation matrix into a rotation quaternion.
	//

	Rotation = Math::CreateRotationQuaternion( RotationMatrix );
}

NWN::Vector2
Math::PolygonCentroid2(
	__in const Vector2Vec & Polygon
	)
/*++

Routine Description:

	This routine computes the centroid of a 2D polygon.

	The algorithm is based off of Gerard Bashein and Paul R. Detmer's code in
	the article "Centroid of a Polygon", which appeared in "Graphic Gems IV",
	Academic Press, 1994.

Arguments:

	Polygon - Supplies the vertex array defining the bounds of a closed polygon
	          for which the centroid is to be returned.

Return Value:

	Returns a Boolean value indicating true if the given point intersects with
	the polygon in question, or false if no intersection occurred.

Environment:

	User mode.

--*/
{
	NWN::Vector2 Centroid2;
	size_t       i;
	size_t       j;
	size_t       n;
	float        atmp;
	float        xtmp;
	float        ytmp;

	switch (Polygon.size( ))
	{

	case 0:
		throw std::runtime_error( "Attempted to retrieve Centroid2 of empty polygon" );

	case 1:
		return Polygon[ 0 ];

	case 2:
		Centroid2.x = (Polygon[ 1 ].x - Polygon[ 0 ].x) / 2;
		Centroid2.y = (Polygon[ 1 ].y - Polygon[ 0 ].y) / 2;
		return Centroid2;

	}

	//
	// Non-degenerate cases.
	//

	n    = Polygon.size( );
	atmp = 0.0f;
	xtmp = 0.0f;
	ytmp = 0.0f;

	for (i = n - 1, j = 0; j < n; i = j, j++)
	{
		float ai;

		ai = (Polygon[ i ].x * Polygon[ j ].y) -
			(Polygon[ j ].x * Polygon[ i ].y);

		atmp += ai;
		xtmp += (Polygon[ j ].x + Polygon[ i ].x) * ai;
		ytmp += (Polygon[ j ].y + Polygon[ i ].y) * ai;
	}

	//
	// Zero area, just take one of the verticies.
	//

	if ((atmp > -Epsilon) && (atmp < Epsilon))
		return Polygon[ 0 ];

	Centroid2.x = xtmp / (3 * atmp);
	Centroid2.y = ytmp / (3 * atmp);

	return Centroid2;
}

bool
Math::PointInPolygonRegion(
	__in const NWN::Vector2 & v,
	__in const Vector2Vec & Polygon,
	__in const unsigned int FixedPointShift /* = 1 << 6 */
	)
/*++

Routine Description:

	This routine tests to see if a two-dimensional point resides within a
	two-dimensional polygon region described by an array of vertex coordinates.

Arguments:

	v - Supplies the coordinate to test for polygon intersection.

	Polygon - Supplies the vertex array defining the bounds of a closed polygon
	          which is to be examined for an intersection with the given point.

	FixedPointShift - Supplies the left-justified bit shift to use when
	                  converting floating point values to fixed point values
					  for conversion purposes.  The default value allows for
					  6 points of precision past the decimal point which is a
					  reasonable compromise given the maximum coordinate range
					  of a large exterior area.

Return Value:

	Returns a Boolean value indicating true if the given point intersects with
	the polygon in question, or false if no intersection occurred.

Environment:

	User mode.

--*/
{
	std::vector< unsigned int > TestGeometry;

	if ((v.x < 0) ||
		(v.y < 0) ||
		(Polygon.size( ) < 2))
		return false;

	//
	// Convert to fixed point.
	//

	TestGeometry.reserve( 2 * Polygon.size( ) );

	for (Vector2Vec::const_iterator it = Polygon.begin( );
		 it != Polygon.end( );
		 ++it)
	{
		TestGeometry.push_back( (unsigned int) (it->x * FixedPointShift) );
		TestGeometry.push_back( (unsigned int) (it->y * FixedPointShift) );
	}

	//
	// Perform the underlying polygon hit test.
	//
	return (CountWindingNumber2(
		&TestGeometry[ 0 ],
		(int) Polygon.size( ),
		(unsigned int) (v.x * FixedPointShift),
		(unsigned int) (v.y * FixedPointShift))) != 0;
/*
	if (inpoly(
		&TestGeometry[ 0 ],
		(int) Polygon.size( ),
		(unsigned int) (v.x * FixedPointShift),
		(unsigned int) (v.y * FixedPointShift)))
		return true;
	else
		return false;*/
}

bool
Math::PointInPolygonRegion(
	__in const NWN::Vector3 & v,
	__in const Vector3Vec & Polygon,
	__in const unsigned int FixedPointShift /* = 1 << 6 */
	)
/*++

Routine Description:

	This routine tests to see if a two-dimensional point resides within a
	two-dimensional polygon region described by an array of vertex coordinates.

	N.B.  The z coordinate is thrown away and the routine only performs a test
	      against a two-space polygon.

Arguments:

	v - Supplies the coordinate to test for polygon intersection.

	Polygon - Supplies the vertex array defining the bounds of a closed polygon
	          which is to be examined for an intersection with the given point.

	FixedPointShift - Supplies the left-justified bit shift to use when
	                  converting floating point values to fixed point values
					  for conversion purposes.  The default value allows for
					  6 points of precision past the decimal point which is a
					  reasonable compromise given the maximum coordinate range
					  of a large exterior area.

Return Value:

	Returns a Boolean value indicating true if the given point intersects with
	the polygon in question, or false if no intersection occurred.

Environment:

	User mode.

--*/
{
	std::vector< unsigned int > TestGeometry;

	//
	// N.B.  The z coordinate may legally be negative, do not enforce a
	//       positive value domain for it.
	//

	if ((v.x < 0) ||
		(v.y < 0) ||
		(Polygon.size( ) < 2))
		return false;

	//
	// Convert to fixed point.
	//

	TestGeometry.reserve( 2 * Polygon.size( ) );

	for (Vector3Vec::const_iterator it = Polygon.begin( );
		 it != Polygon.end( );
		 ++it)
	{
		TestGeometry.push_back( (unsigned int) (it->x * FixedPointShift) );
		TestGeometry.push_back( (unsigned int) (it->y * FixedPointShift) );
	}

	//
	// Perform the underlying polygon hit test.
	//

	return (CountWindingNumber2(
		&TestGeometry[ 0 ],
		(int) Polygon.size( ),
		(unsigned int) (v.x * FixedPointShift),
		(unsigned int) (v.y * FixedPointShift))) != 0;
/*
	if (inpoly(
		&TestGeometry[ 0 ],
		(int) Polygon.size( ),
		(unsigned int) (v.x * FixedPointShift),
		(unsigned int) (v.y * FixedPointShift)))
		return true;
	else
		return false;*/
}

bool
Math::PointInPolygonRegion(
	__in const Math::Vector2FP & v,
	__in const Math::Vector2FPVec & Polygon
	)
/*++

Routine Description:

	This routine tests to see if a two-dimensional point resides within a
	two-dimensional polygon region described by an array of vertex coordinates.

Arguments:

	v - Supplies the coordinate to test for polygon intersection.

	Polygon - Supplies the vertex array defining the bounds of a closed polygon
	          which is to be examined for an intersection with the given point.

Return Value:

	Returns a Boolean value indicating true if the given point intersects with
	the polygon in question, or false if no intersection occurred.

Environment:

	User mode.

--*/
{
	if (Polygon.size( ) < 2)
		return false;

	//
	// Perform the underlying polygon hit test.
	//
	return (CountWindingNumber2(
		(unsigned int *) &Polygon[ 0 ],
		(int) Polygon.size( ),
		v.x,
		v.y)) != 0;
/*
	if (inpoly(
		(unsigned int *) &Polygon[ 0 ],
		(int) Polygon.size( ),
		v.x,
		v.y))
		return true;
	else
		return false;*/
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4127) // conditional expression is constant
#endif

// http://people.csail.mit.edu/amy//papers/box-jgt.pdf

//
// "Fast, Minimum Storage Ray/Triangle Intersection"
// http://www.cs.virginia.edu/~gfx/Courses/2003/ImageSynthesis/papers/Acceleration/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
//

#define EPSILON 1.1e-7f
#define CROSS(dest,v1,v2) \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define SUB(dest,v1,v2) \
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2]; 

template< bool TEST_CULL >
static
int
intersect_triangle(const float orig[3], const float dir[3],
                   const float vert0[3], const float vert1[3], const float vert2[3],
                   float *t, float *u, float *v)
{
   float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
   float det,inv_det;

   /* find vectors for two edges sharing vert0 */
   SUB(edge1, vert1, vert0);
   SUB(edge2, vert2, vert0);

   /* begin calculating determinant - also used to calculate U parameter */
   CROSS(pvec, dir, edge2);

   /* if determinant is near zero, ray lies in plane of triangle */
   det = DOT(edge1, pvec);

   if (TEST_CULL)
   {
   if (det < EPSILON)
      return 0;

   /* calculate distance from vert0 to ray origin */
   SUB(tvec, orig, vert0);

   /* calculate U parameter and test bounds */
   *u = DOT(tvec, pvec);
   if (*u < 0.0 || *u > det)
      return 0;

   /* prepare to test V parameter */
   CROSS(qvec, tvec, edge1);

    /* calculate V parameter and test bounds */
   *v = DOT(dir, qvec);
   if (*v < 0.0 || *u + *v > det)
      return 0;

   /* calculate t, scale parameters, ray intersects triangle */
   *t = DOT(edge2, qvec);
   inv_det = 1.0f / det;
   *t *= inv_det;
   *u *= inv_det;
   *v *= inv_det;
   } else {                    /* the non-culling branch */
//   WriteText( "Det: %f\n", det );
   if (det > -EPSILON && det < EPSILON)
     return 0;
   inv_det = 1.0f / det;

   /* calculate distance from vert0 to ray origin */
   SUB(tvec, orig, vert0);

   /* calculate U parameter and test bounds */
   *u = DOT(tvec, pvec) * inv_det;
//   WriteText( "U: %f\n", *u );
   if (*u < 0.0 || *u > 1.0)
     return 0;

   /* prepare to test V parameter */
   CROSS(qvec, tvec, edge1);

   /* calculate V parameter and test bounds */
   *v = DOT(dir, qvec) * inv_det;
//   WriteText( "V: %f\n", *v );
   if (*v < 0.0 || *u + *v > 1.0)
     return 0;

   /* calculate t, ray intersects triangle */
   *t = DOT(edge2, qvec) * inv_det;
   }

   if (*t < 0.0)
      return 0;
   else
      return 1;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#undef EPSILON
#undef CROSS
#undef SUB
#undef DOT

bool
Math::IntersectRayTri(
	__in const NWN::Vector3 & Origin,
	__in const NWN::Vector3 & NormDir,
	__in_ecount(3) const NWN::Vector3 * Tri,
	__out float & T
	)
/*++

Routine Description:

	This routine performs an intersection test between a ray defined by an
	origin and a normalized direction, and a triangle defined by three
	verticies.

Arguments:

	Origin - Supplies the origin point of the ray.

	NormDir - Supplies the normalized direction of the ray.

	Tri - Supplies the triangle verticies defining the triangle to intersect.

	T - Receives the distance from the origin to the intersection point, should
	    the routine return true.

Return Value:

	Returns a Boolean value indicating true if the given ray intersects with
	the triangle in question, or false if no intersection occurred.

Environment:

	User mode.

--*/
{
	float U;
	float V;

	if (intersect_triangle< false >(
		(const float *) &Origin,
		(const float *) &NormDir,
		(const float *) &Tri[ 0 ],
		(const float *) &Tri[ 1 ],
		(const float *) &Tri[ 2 ],
		&T,
		&U,
		&V))
	{
		return true;
	}

	return false;
}

bool
Math::IntersectRayTriRejectBackface(
	__in const NWN::Vector3 & Origin,
	__in const NWN::Vector3 & NormDir,
	__in_ecount(3) const NWN::Vector3 * Tri,
	__out float & T
	)
/*++

Routine Description:

	This routine performs an intersection test between a ray defined by an
	origin and a normalized direction, and a triangle defined by three
	verticies.

	Backfaces are not considered as intersecting.

Arguments:

	Origin - Supplies the origin point of the ray.

	NormDir - Supplies the normalized direction of the ray.

	Tri - Supplies the triangle verticies defining the triangle to intersect.

	T - Receives the distance from the origin to the intersection point, should
	    the routine return true.

Return Value:

	Returns a Boolean value indicating true if the given ray intersects with
	the triangle in question, or false if no intersection occurred.

Environment:

	User mode.

--*/
{
	float U;
	float V;

	if (intersect_triangle< true >(
		(const float *) &Origin,
		(const float *) &NormDir,
		(const float *) &Tri[ 0 ],
		(const float *) &Tri[ 1 ],
		(const float *) &Tri[ 2 ],
		&T,
		&U,
		&V))
	{
		return true;
	}

	return false;
}


bool
Math::IntersectRaySphere(
		__in const NWN::Vector3 & RayOrigin,
		__in const NWN::Vector3 & RayNormDir,
		__in const NWN::Vector3 & SphereOrigin,
		__in const float SphereRadiusSq,
		__out float & T
	)
/*++

Routine Description:

	This routine performs an intersection test between a ray defined by an
	origin and a normalized direction, and a sphere defined by an origin and a
	radius (squared).

Arguments:

	RayOrigin - Supplies the origin point of the ray.

	RayNormDir - Supplies the normalized direction of the ray.

	SphereOrigin - Supplies the origin point of the sphere.

	SphereRadiusSq - Supplies the (squared) radius of the sphere.

	T - Receives the distance from the origin to the intersection point, should
	    the routine return true.

Return Value:

	Returns a Boolean value indicating true if the given ray intersects with
	the sphere in question, or false if no intersection occurred.

Environment:

	User mode.

--*/
{
	NWN::Vector3 dst;
	float        B;
	float        C;
	float        D;

	dst.x = RayOrigin.x - SphereOrigin.x;
	dst.y = RayOrigin.y - SphereOrigin.y;
	dst.z = RayOrigin.z - SphereOrigin.z;

	B = Math::DotProduct( dst, RayNormDir );
	C = Math::DotProduct( dst, dst ) - SphereRadiusSq;
	D = (B * B) - C;

	if (D > 0)
	{
		T = -B - sqrtf( D );
		return true;
	}
	else
	{
		return false;
	}
}

//
// Wrappers for intersect2D_Segments.
//

struct Vector
{
	float x;
	float y;

	inline Vector( )
	: x( 0.0f ),
	  y( 0.0f )
	{
	}

	inline Vector( __in const NWN::Vector2 & v )
	: x( v.x ),
	  y( v.y )
	{
	}

	inline Vector operator+( Vector w) const        // vector add
	{
		Vector v;

		v.x = x + w.x;
		v.y = y + w.y;

		return v;
	}

	inline Vector operator-( Vector w) const        // vector subtract
	{
		Vector v;

		v.x = x - w.x;
		v.y = y - w.y;

		return v;
	}

	inline bool operator!=( Vector w) const
	{
		return (x != w.x) && (y != w.y);
	}

	inline bool operator==( Vector w) const
	{
		return (x == w.x) && (y == w.y);
	}

};

inline Vector operator*( Vector v, float d ) // Scalar multiplication
{
	v.x *= d;
	v.y *= d;

	return v;
}


inline Vector operator*( float d, Vector v ) // Scalar multiplication
{
	v.x *= d;
	v.y *= d;

	return v;
}

typedef Vector Point;

struct Segment
{
	Point P0;
	Point P1;
};

static
int
inSegment(
	__in const Point & P,
	__in const Segment & S
	);

static
int
intersect2D_Segments(
	__in const Segment & S1,
	__in const Segment & S2,
	__out bool* Parallel,
	__out Point* I0,
	__out Point* I1
	);

static
int
intersect2D_SegPoly(
	__in const Segment & S,
	__in_ecount( n ) const NWN::Vector2 * PolygonPoints,
	__in size_t n,
	__out Segment * IS
	);

bool
Math::IntersectSegments2(
	__in const NWN::Vector2 & s1_p1,
	__in const NWN::Vector2 & s1_p2,
	__in const NWN::Vector2 & s2_p1,
	__in const NWN::Vector2 & s2_p2,
	__out NWN::Vector2 & IntersectionPoint,
	__out bool & Parallel
	)
/*++

Routine Description:

	This routine calculates whether two line segments (s1 and s2) intersect.
	Should the line segments intersect, the routine computes the intersection
	point (first intersection point).  The routine also computes whether the
	two line segments were parallel.

Arguments:

	s1_p1 - Supplies the first line segment's first point.

	s1_p2 - Supplies the first line segment's second point.

	s2_p1 - Supplies the second line segment's first point.

	s2_p2 - Supplies the second line segment's second point.

	IntersectionPoint - Receives the first intersection point (I0) if the two
	                    line segments do intersect.

	Parallel - Receives a Boolean value indicating whether the two line
	           segments were parallel or not, should the segments intersect.

Return Value:

	Returns a Boolean value indicating true if the two line segments intersect,
	or false if the two line segments did not intersect.

Environment:

	User mode.

--*/
{
	Segment S1;
	Segment S2;
	Point   I0;
	Point   I1;
	bool    Parallel_;

	S1.P0.x = s1_p1.x;
	S1.P0.y = s1_p1.y;
	S1.P1.x = s1_p2.x;
	S1.P1.y = s1_p2.y;

	S2.P0.x = s2_p1.x;
	S2.P0.y = s2_p1.y;
	S2.P1.x = s2_p2.x;
	S2.P1.y = s2_p2.y;

	if (!intersect2D_Segments( S1, S2, &Parallel_, &I0, &I1 ))
		return false;

	Parallel = Parallel_;

	IntersectionPoint.x = I0.x;
	IntersectionPoint.y = I0.y;

	return true;
}

bool
Math::IntersectSegmentPolygon(
	__in const NWN::Vector2 & s_p1,
	__in const NWN::Vector2 & s_p2,
	__in_ecount( NumPoints ) const NWN::Vector2 * PolygonPoints,
	__in size_t NumPoints,
	__out NWN::Vector2 & I_p1,
	__out NWN::Vector2 & I_p2
	)
/*++

Routine Description:

	This routine calculates whether a line segment and a 2D polygon intersect.
	Should the line segment and polygon intersect, the routine computes the
	intersection line segment (start of intersection and end of intersection).

Arguments:

	s_p1 - Supplies the test line segment's first point.

	s_p2 - Supplies the test line segment's second point.

	PolygonPoints - Supplies the polygon vertex array.

	NumPoints - Supplies the count of verticies in the polygon vertex array.

	I_p1 - On success, receives the point of first intersection.

	I_p2 - On success, receives the point of last intersection.

Return Value:

	Returns a Boolean value indicating true if the line segment and polygon
	intersected, or false if they did not intersect.

Environment:

	User mode.

--*/
{
	Segment S;
	Segment IS;

	S.P0.x = s_p1.x;
	S.P0.y = s_p1.y;
	S.P1.x = s_p2.x;
	S.P1.y = s_p2.y;

	if (!intersect2D_SegPoly( S, PolygonPoints, NumPoints, &IS ))
		return false;

	I_p1.x = IS.P0.x;
	I_p1.y = IS.P0.y;
	I_p2.x = IS.P1.x;
	I_p2.y = IS.P1.y;

	return true;
}

bool
Math::PointInSegment(
	__in const NWN::Vector2 & s_p1,
	__in const NWN::Vector2 & s_p2,
	__in const NWN::Vector2 & pt
	)
/*++

Routine Description:

	This routine determines whether a line segment (as bounded by two points
	inclusive) intersects with a single point.

Arguments:

	s1_p1 - Supplies the line segment's first point.

	s1_p2 - Supplies the line segment's second point.

	pt - Supplies the point to test for line segment intersection.

Return Value:

	Returns a Boolean value indicating true if the point intersects the line
	segment, else false if there was no intersection.

Environment:

	User mode.

--*/
{
	Segment S1;
	Segment S2;
	Point   I0;
	Point   I1;
	bool    Parallel;

	//
	// Just pass the request to intersect2D_Segments, which handles the
	// degenerate case.
	//
	// Most callers hide this logic path behind a quick rect bounding box test.
	//

	S1.P0.x = s_p1.x;
	S1.P0.y = s_p1.y;
	S1.P1.x = s_p2.x;
	S1.P1.y = s_p2.y;
	S2.P0.x = pt.x;
	S2.P0.y = pt.y;
	S2.P1.x = pt.x;
	S2.P1.y = pt.y;

	return intersect2D_Segments( S1, S2, &Parallel, &I0, &I1 ) != 0;
}

//
// The following copyright notice applies to intersect2D_Segments,
// inSegment, and intersect2D_SegPoly only.
// 

// Copyright 2001, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.


// Assume that classes are already given for the objects:
//    Point and Vector with
//        coordinates {float x, y, z;}
//        operators for:
//            == to test equality
//            != to test inequality
//            Point  = Point ± Vector
//            Vector = Point - Point
//            Vector = Scalar * Vector    (scalar product)
//            Vector = Vector * Vector    (3D cross product)
//    Line and Ray and Segment with defining points {Point P0, P1;}
//        (a Line is infinite, Rays and Segments start at P0)
//        (a Ray extends beyond P1, but a Segment ends at P1)
//    Plane with a point and a normal {Point V0; Vector n;}
//===================================================================


// inSegment(): determine if a point is inside a segment
//    Input:  a point P, and a collinear segment S
//    Return: 1 = P is inside S
//            0 = P is not inside S
static
int
inSegment(
	__in const Point & P,
	__in const Segment & S
	)
{
    if (S.P0.x != S.P1.x) {    // S is not vertical
        if (S.P0.x <= P.x && P.x <= S.P1.x)
            return 1;
        if (S.P0.x >= P.x && P.x >= S.P1.x)
            return 1;
    }
    else {    // S is vertical, so test y coordinate
        if (S.P0.y <= P.y && P.y <= S.P1.y)
            return 1;
        if (S.P0.y >= P.y && P.y >= S.P1.y)
            return 1;
    }
    return 0;
}



#define SMALL_NUM  Math::Epsilon // anything that avoids division overflow
// dot product (3D) which allows vector operations in arguments
#define dot(u,v)   ((u).x * (v).x + (u).y * (v).y/* + (u).z * (v).z*/)
#define perp(u,v)  ((u).x * (v).y - (u).y * (v).x)  // perp product (2D)

inline
bool
FloatLt(
	__in float f1,
	__in float f2
	)
{
	return fabsf( f1 - f2 ) < Math::Epsilon;
}

inline
bool
FloatGt(
	__in float f1,
	__in float f2
	)
{
	return fabsf( f1 - f2 ) > Math::Epsilon;
}
 

// intersect2D_2Segments(): the intersection of 2 finite 2D segments
//    Input:  two finite segments S1 and S2
//    Output: *I0 = intersect point (when it exists)
//            *I1 = endpoint of intersect segment [I0,I1] (when it exists)
//    Return: 0=disjoint (no intersect)
//            1=intersect in unique point I0
//            2=overlap in segment from I0 to I1
static
int
intersect2D_Segments(
	__in const Segment & S1,
	__in const Segment & S2,
	__out bool* Parallel,
	__out Point* I0,
	__out Point* I1
	)
{
	Vector     u = S1.P1 - S1.P0;
	Vector     v = S2.P1 - S2.P0;
	Vector     w = S1.P0 - S2.P0;
    float      D = perp(u,v);

    // test if they are parallel (includes either being a point)
    if (fabs(D) < SMALL_NUM) {          // S1 and S2 are parallel
        *Parallel = true;
        if (perp(u,w) != 0 || perp(v,w) != 0) {
            return 0;                   // they are NOT collinear
        }
        // they are collinear or degenerate
        // check if they are degenerate points
        float du = dot(u,u);
        float dv = dot(v,v);
        if (du==0 && dv==0) {           // both segments are points
            if (S1.P0 != S2.P0)         // they are distinct points
                return 0;
            *I0 = S1.P0;                // they are the same point
            return 1;
        }
        if (du==0) {                    // S1 is a single point
            if (inSegment(S1.P0, S2) == 0)  // but is not in S2
                return 0;
            *I0 = S1.P0;
            return 1;
        }
        if (dv==0) {                    // S2 a single point
            if (inSegment(S2.P0, S1) == 0)  // but is not in S1
                return 0;
            *I0 = S2.P0;
            return 1;
        }
        // they are collinear segments - get overlap (or not)
        float t0, t1;                   // endpoints of S1 in eqn for S2
        Vector w2 = S1.P1 - S2.P0;
        if (v.x != 0) {
                t0 = w.x / v.x;
                t1 = w2.x / v.x;
        }
        else {
                t0 = w.y / v.y;
                t1 = w2.y / v.y;
        }
        if (t0 > t1) {                  // must have t0 smaller than t1
                float t=t0; t0=t1; t1=t;    // swap if not
        }
        if (t0 > 1 || t1 < 0) {
            return 0;     // NO overlap
        }
        t0 = t0<0? 0 : t0;              // clip to min 0
        t1 = t1>1? 1 : t1;              // clip to max 1
        if (t0 == t1) {                 // intersect is a point
            *I0 = S2.P0 + t0 * v;
            return 1;
        }

        // they overlap in a valid subsegment
        *I0 = S2.P0 + t0 * v;
        *I1 = S2.P0 + t1 * v;
        return 2;
    }

	*Parallel = false;

    // the segments are skew and may intersect in a point
    // get the intersect parameter for S1
    float     sI = perp(v,w) / D;
//	WriteText( "sI: %f\n", sI );
    if (sI < 0 || sI > 1)               // no intersect with S1
        return 0;

    // get the intersect parameter for S2
    float     tI = perp(u,w) / D;
//	WriteText( "tI: %f\n", tI );
    if (tI < 0 || tI > 1)               // no intersect with S2
        return 0;

    *I0 = S1.P0 + sI * u;               // compute S1 intersect point
    return 1;
}

// intersect2D_SegPoly():
//    Input:  S = 2D segment to intersect with the convex polygon
//            n = number of 2D points in the polygon
//            V[] = array of n+1 vertex points with V[n]=V[0]
//      Note: The polygon MUST be convex and
//                have vertices oriented counterclockwise (ccw).
//            This code does not check for and verify these conditions.
//    Output: *IS = the intersection segment (when it exists)
//    Return: FALSE = no intersection
//            TRUE  = a valid intersection segment exists
int
intersect2D_SegPoly(
	__in const Segment & S,
	__in_ecount( n ) const NWN::Vector2 * PolygonPoints,
	__in size_t n,
	__out Segment* IS
	)
{
#if 0
    if (S.P0 == S.P1) {        // the segment S is a single point
        // test for inclusion of S.P0 in the polygon
        *IS = S;               // same point if inside polygon
        return cn_PnPoly( S.P0, V, n );  // March 2001 Algorithm
    }
#endif

    float  tE = 0;             // the maximum entering segment parameter
    float  tL = 1;             // the minimum leaving segment parameter
    float  t, N, D;            // intersect parameter t = N / D
    Vector dS = S.P1 - S.P0;   // the segment direction vector
    Vector e;                  // edge vector
    // Vector ne;              // edge outward normal (not explicit in code)

    for (size_t i=0; i < n; i++)  // process polygon edge V[i]V[i+1]
    {
        Vector v_next = PolygonPoints[i+1];
		  Vector v_this = PolygonPoints[i];
	
        e = v_next - v_this;
        N = perp(e, S.P0-v_this);// = -dot(ne, S.P0-V[i])
        D = -perp(e, dS);      // = dot(ne, dS)
        if (fabs(D) < SMALL_NUM) { // S is nearly parallel to this edge
            if (N < 0)             // P0 is outside this edge, so
                return FALSE;      // S is outside the polygon
            else                   // S cannot cross this edge, so
                continue;          // ignore this edge
        }

        t = N / D;
        if (D < 0) {           // segment S is entering across this edge
            if (t > tE) {      // new max tE
                tE = t;
                if (tE > tL)   // S enters after leaving polygon
                    return FALSE;
            }
        }
        else {                 // segment S is leaving across this edge
            if (t < tL) {      // new min tL
                tL = t;
                if (tL < tE)   // S leaves before entering polygon
                    return FALSE;
            }
        }
    }

    // tE <= tL implies that there is a valid intersection subsegment
    IS->P0 = S.P0 + tE * dS;   // = P(tE) = point where S enters polygon
    IS->P1 = S.P0 + tL * dS;   // = P(tL) = point where S leaves polygon
    return TRUE;
}




//===================================================================

