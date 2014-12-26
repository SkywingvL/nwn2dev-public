/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	AreaSurfaceMesh.cpp

Abstract:

	This module houses the implementations of the AreaSurfaceMesh and
	AreaSurfaceMesh::TileSurfaceMesh classes.  These classes support pathing
	table management and basic pathing primitives (such as straight-path and
	line of sight intersection).

	Major portions of the TRX on-disk layout courtesy Tero Kivinen
	<kivinen@iki.fi>.

--*/

#include "Precomp.h"
#include "TextOut.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#include "SurfaceMeshBase.h"
#include "MeshManager.h"
#include "AreaSurfaceMesh.h"

#define FINDFACE_USE_FIXED_POINT 1

#define STRAIGHT_PATH_DEBUG 0




#if STRAIGHT_PATH_DEBUG

#define StraightPathDebug if (m_TextWriter) m_TextWriter->WriteText
#define StraightPathDebug2 if (SurfaceMesh->m_TextWriter) SurfaceMesh->m_TextWriter->WriteText
#define IntersectSegDebug if (m_TextWriter) m_TextWriter->WriteText

#else

#define StraightPathDebug __noop
#define StraightPathDebug2 __noop
#define IntersectSegDebug __noop

#endif

const float AreaSurfaceMesh::PathingEpsilon = 1.1e-7f;

const MeshLinkageTraits MLT_AreaSurfaceMesh =
{
	/* Type                    = */ MeshLinkageTraits::MT_AreaSurfaceMesh,
	/* MeshLinkageToBodyOffset = */ MESH_LINKAGE_OFFSET( AreaSurfaceMesh ) 
};

const AreaSurfaceMesh::TileSurfaceMesh::SurfaceMeshFace *
AreaSurfaceMesh::TileSurfaceMesh::FindFace(
	__in const NWN::Vector2 & pt,
	__in const PointVec & Points
	) const
/*++

Routine Description:

	This routine locates a face that contains a particular point.

Arguments:

	pt - Supplies the point to search for.

	Points - Supplies the point vector to use.

Return Value:

	Returns a pointer to the given face, else NULL should the face not be
	located.

Environment:

	User mode.

--*/
{
	for (const SurfaceMeshFace * Face = &m_Faces[ 0 ];
	     Face != &m_Faces[ m_NumFaces ];
	     Face += 1)
	{
		if (IsPointInTriangle( Face, pt, Points ))
			return Face;
	}

	return NULL;
}

bool
AreaSurfaceMesh::TileSurfaceMesh::StraightPathExists(
	__in const NWN::Vector2 & Start,
	__in const NWN::Vector2 & End,
	__in const SurfaceMeshFace * Face1,
	__in const SurfaceMeshFace * Face2,
	__in const AreaSurfaceMesh * SurfaceMesh,
	__out_opt const SurfaceMeshFace * * LastFace /* = NULL */
	) const
/*++

Routine Description:

	This routine calculates whether a straight-edge path exists between two
	points.  Collision detection is performed with the baked walkmesh only
	(which includes static walkmesh blockers such as placeables).  Non-walkmesh
	blockers such as creatures are not considered for purposes of path
	existance.

Arguments:

	Start - Supplies the first point of the line segment.  The line segment
	        must reside entirely within the current tile surface mesh.

	End - Supplies the second point of the line segment.  The line segment must
	      reside entirely within the current tile surface mesh.

	Face1 - Receives the face containing the Start point.  The face must be
	        joined to the current tile surface mesh.

	Face2 - Receives the face containing the End point.  The face must be
	        joined to the current tile surface mesh.

	SurfaceMesh - Supplies a backlink to the parent AreaSurfaceMesh object,
	              which is used to look up edges and verticies.

	LastFace - Optionally receives the face we had most recently successfully
	           traversed on failure.  This parameter is generally only used on
	           failure (it may be NULL if no traversals were actually performed
	           which may happen in both immediate success or immediate failure
	           cases).

Return Value:

	Returns a pointer to the given face, else NULL should the face not be
	located.

Environment:

	User mode.

--*/
{
	unsigned long FaceId1;
	unsigned long FaceId2;
	unsigned long ExcludeEdgeId;

	if (ARGUMENT_PRESENT( LastFace ))
		*LastFace = NULL;

	FaceId1 = SurfaceMesh->GetFaceId( Face1 );
	FaceId2 = SurfaceMesh->GetFaceId( Face2 );

	if (!(Face1->Flags & SurfaceMeshFace::WALKABLE))
		return false;

	if (!(Face2->Flags & SurfaceMeshFace::WALKABLE))
		return false;

	if (FaceId1 == FaceId2)
		return true;

	//
	// First, see if we've precomputed a straight visible path through the two
	// points.  If so, there's no need to go any further as we are guaranteed
	// that a straight line exists.
	//

	if (m_PathTable.GetVisible( FaceId1, FaceId2 ))
		return true;

	ExcludeEdgeId = 0xFFFFFFFF;

	//
	// Attempt to traverse all triangles with edges intersecting our path line,
	// checking for non-walkable triangles.
	//

	StraightPathDebug2(
		"StraightPathExists: Search from (%f, %f) to (%f, %f)\n",
		Face1->Centroid2.x,
		Face1->Centroid2.y,
		Face2->Centroid2.x,
		Face2->Centroid2.y);

	for (unsigned long i = 0; i < 40; i += 1)
	{
		unsigned long EdgeId;
		unsigned long j;

		EdgeId = 0xffffffff; // Compiler satisfaction

		if (!(Face1->Flags & SurfaceMeshFace::WALKABLE))
			return false;

		if (ARGUMENT_PRESENT( LastFace ))
			*LastFace = Face1;

		//
		// Find the next edge to follow.  Because this triangle is intersecting
		// with our path line on at least one side, unless it's a parallel line
		// to one of the edges (which isn't supported), we should have a quick
		// reference to a neighboring triangle that also intersects.
		//
		// We follow the intersecting edge chain until we reach the end of the
		// mesh, or our complexity limit.
		//

		for (j = 0; j < 3; j += 1)
		{
			const SurfaceMeshEdge * Edge;
			NWN::Vector2            v1;
			NWN::Vector2            v2;
			NWN::Vector2            I0;
			bool                    Parallel;

			EdgeId = Face1->Edges[ j ];

			if (EdgeId == ExcludeEdgeId)
				continue;

			Edge = SurfaceMesh->GetEdge( Face1->Edges[ j ] );

			v1.x = SurfaceMesh->GetPoint( Edge->Points1 )->x;
			v1.y = SurfaceMesh->GetPoint( Edge->Points1 )->y;
			v2.x = SurfaceMesh->GetPoint( Edge->Points2 )->x;
			v2.y = SurfaceMesh->GetPoint( Edge->Points2 )->y;

			//
			// Do not attempt to follow child triangles of edges that are not
			// intersecting with our path vector.
			//
			// N.B.  This test is error prone as the pathing solver heavily
			//       tends towards pathing solutions that involve walking to
			//       the midpoint of an edge.  Accumulating precision errors
			//       often conspire to cause actual straight edges to be thrown
			//       out erroneously (i.e. due to line segment intersection
			//       parameters of 1.000005).
			//
			//       Unfortunately, the server pathing engine does not correct
			//       for these errors and thus neither can we.
			//

			if (!Math::IntersectSegments2( Start, End, v1, v2, I0, Parallel ))
			{
				StraightPathDebug2(
					"Discount nonintersecting edge <%08X> %f %f %f %f\n",
					EdgeId,
					v1.x,
					v1.y,
					v2.x,
					v2.y);

				continue;
			}

			StraightPathDebug2(
				"Found an intersecting edge @ %f, %f <%08X> %f %f %f %f\n",
				I0.x,
				I0.y,
				EdgeId,
				v1.x,
				v1.y,
				v2.x,
				v2.y);

			//
			// Walking precisely parallel to a walkmesh edge is not permitted.
			// This restriction exists as the edge relationships only describe
			// triangles adjacent to the current triangle and which share more
			// than a single point with the current triangle.
			//
			// That is, triangles that just share a single vertex and no other
			// points are not accounted to in the neighbor edges list.  This
			// means that if we were forced to travel parallel, we would have
			// no quick way to determine which adjacent triangle we should have
			// picked based on walkmesh traversal.
			//
			// N.B.  It would technically be possible just to draw a ray
			//       extending, but there is also the difficulty of the fact
			//       that we'd be exactly on a walkmesh seam, and the behavior
			//       with respect to which walkmesh triangle is picked on a
			//       seam is not defined deterministically.
			//

			if (Parallel)
				return false;

			if ((Edge->Triangles1 == (unsigned long) -1) ||
			    (Edge->Triangles2 == (unsigned long) -1))
			{
				return false;
			}

			//
			// Check if either of these edge triangles reside within the
			// current tile surface mesh.  Only check triangles within the
			// same tile surface mesh as we know that the line we've been given
			// is only within this tile surface mesh for the segment that we
			// are examining.
			//

			if ((Edge->Triangles1 >= m_FaceOffset)             &&
			    (Edge->Triangles1 < m_FaceOffset + m_NumFaces) &&
			    (Edge->Triangles1 != FaceId1))
			{
				FaceId1 = Edge->Triangles1;
				Face1   = SurfaceMesh->GetFace( FaceId1 );
			}
			else if ((Edge->Triangles2 >= m_FaceOffset)             &&
			         (Edge->Triangles2 < m_FaceOffset + m_NumFaces) &&
			         (Edge->Triangles2 != FaceId1))
			{
				FaceId1 = Edge->Triangles2;
				Face1   = SurfaceMesh->GetFace( FaceId1 );
			}
			else
			{
				StraightPathDebug2(
					"Neither triangles were in this walkmesh\n");

				continue;
			}

			//
			// Don't follow the edge we just traversed, so that we do not get
			// stuck oscillating back and forth.
			//

			ExcludeEdgeId = EdgeId;

			StraightPathDebug2( "Now avoiding edge %08X\n", ExcludeEdgeId );

			//
			// We've got a new face to follow, stop traversing the old face's
			// edges looking for a new way.
			//

			break;
		}

		if (j == 3)
		{
//			WriteText( "!!! No intersections!\n" );
			ExcludeEdgeId = EdgeId;
//			return false;
		}

		//
		// If we reached the destination face, then terminate the search.  We
		// have gotten there.
		//

		if (FaceId1 == FaceId2)
			return true;

		StraightPathDebug2(
			"StraightPathExists: Now search from (%f, %f) to (%f, %f)\n",
			Face1->Centroid2.x,
			Face1->Centroid2.y,
			Face2->Centroid2.x,
			Face2->Centroid2.y);
	}

	StraightPathDebug2( "Walkmesh too complex to determine reachability\n" );

	//
	// Walkmesh too complex, return not reachable.
	//

	return false;
}

bool
AreaSurfaceMesh::TileSurfaceMesh::CalcContact(
	__in const NWN::Vector3 & Origin,
	__in const NWN::Vector3 & NormDir,
	__in const SurfaceMeshFace * Face1,
	__in const SurfaceMeshFace * Face2,
	__in const AreaSurfaceMesh * SurfaceMesh,
	__out float & IntersectDistance,
	__out_opt const SurfaceMeshFace * * IntersectFace
	) const
/*++

Routine Description:

	This routine calculates the distance that a ray can travel before it comes
	into contact with a face in the surface mesh.

Arguments:

	Origin - Supplies the origin point of the ray.

	NormDir - Supplies the normalized direction of the ray.

	Face1 - Supplies the first face intersecting the ray in this mesh.

	Face2 - Supplies the last face intersecting the ray in this mesh.

	IntersectDistance - Receives the distance from the ray origin for the first
	                    ray intersection, if an intersection existed.

	IntersectFace - Optionally receives the intersecting face pointer, if a
	                intersection existed.

Return Value:

	Returns a Boolean value indicating true if the ray intersected a face
	within the given parameters, else false if no intersection exists.

Environment:

	User mode.

--*/
{
	bool Intersected;

	UNREFERENCED_PARAMETER( Face1 );
	UNREFERENCED_PARAMETER( Face2 );

	Intersected = false;

	//
	// Iterate over all faces, finding the closest intersection (if any).
	//
	// N.B.  We're guaranteed that at least one face intersects this mesh.
	//

	for (const SurfaceMeshFace * Face = &m_Faces[ 0 ];
	     Face != &m_Faces[ m_NumFaces ];
	     Face += 1)
	{
		NWN::Vector3 Tri[ 3 ];
		float        T;

		for (size_t i = 0; i < 3; i += 1)
		{
			Tri[ i ] = *SurfaceMesh->GetPoint( Face->Corners[ i ] );
		}

		//
		// Perform the intersection test.
		//

		if (!Math::IntersectRayTri( Origin, NormDir, Tri, T ))
			continue;

		if (!Intersected)
		{
			Intersected       = true;
			IntersectDistance = T;

			if (ARGUMENT_PRESENT( IntersectFace ))
				*IntersectFace = Face;
		}
		else if (IntersectDistance < T)
		{
			IntersectDistance = T;

			if (ARGUMENT_PRESENT( IntersectFace ))
				*IntersectFace = Face;
		}
	}

	return Intersected;
}

bool
AreaSurfaceMesh::GetLineSegmentFacesAndMesh(
	__in const NWN::Vector2 & Start,
	__in const NWN::Vector2 & End,
	__in_opt const TileSurfaceMesh * ExcludeSurfaceMesh,
	__in bool Walkable,
	__out const SurfaceMeshFace * * Face1,
	__out const SurfaceMeshFace * * Face2,
	__out const TileSurfaceMesh * * SurfaceMesh
	) const
/*++

Routine Description:

	Given the start and end bounding points of a line segment, which must
	reside entirely within the same tile surface mesh (although it may reside
	upon a seam), this routine returns the walk mesh faces that contain the
	start and end points, respectively.  Both faces are guaranteed to be joined
	to the same tile surface mesh, which is also returned.

	Only walkable faces are returned.

Arguments:

	Start - Supplies the first point of the line segment.  The line segment
	        must reside entirely within one tile surface mesh.

	End - Supplies the second point of the line segment.  The line segment must
	      reside entirely within one tile surface mesh.

	ExcludeSurfaceMesh - Optionally supplies a surface mesh to exclude if there
	                     should be be a tie between which surface mesh to pick
	                     for a particular triangle.  If the caller is following
	                     a line through several intersecting segments, then
	                     subsequent calls must specify the previous surface
	                     mesh in order to ensure forward progress.

	Walkable - Supplies a Boolean value indicating whether only walkable faces
	           are to be considered.

	Face1 - Receives the face containing the Start point.

	Face2 - Receives the face containing the End point.

	SurfaceMesh - Receives the tile surface mesh that both faces are joined to.

Return Value:

	Returns true two faces joined to the same tile surface mesh and containing
	the Start and End points could be located.  Otherwise, the routine returns
	false.

Environment:

	User mode.

--*/
{
	const SurfaceMeshFace * f1;
	const TileSurfaceMesh * t1;
	const SurfaceMeshFace * f2;
	const TileSurfaceMesh * t2;

	f1 = FindFace( Start, &t1 );

	if (f1 == NULL)
		return false;

	if ((Walkable) && (!(f1->Flags & SurfaceMeshFace::WALKABLE)))
		return false;

	f2 = FindFace( End, &t2 );

	if (f2 == NULL)
		return false;

	if ((Walkable) && (!(f2->Flags & SurfaceMeshFace::WALKABLE)))
		return false;

//	WriteText( "Try (%f, %f) and (%f, %f)\n", f1->Centroid2.x, f1->Centroid2.y, f2->Centroid2.x, f2->Centroid2.y );

	//
	// If the points were not in the same surface mesh then it's possible
	// that we picked the wrong one.  This may happen as it is not very
	// well defined what the behavior should be should we be right on a
	// seam between tile surface walkmesh objects.
	//
	// In the seam case, we iterate through all neighbor edges, looking for
	// the edge that matches the line.  Once we've got the edge, we'll pick
	// the two triangles that are in the same tiele surface walkmesh.
	//

	if ((t1 != t2) || (t1 == ExcludeSurfaceMesh))
	{
		const SurfaceMeshFace * faces[ 4 ];
		const SurfaceMeshEdge * e1;
		const SurfaceMeshEdge * e2;
		NWN::Vector2            v1;
		NWN::Vector2            v2;
		NWN::Vector2            I0;
		unsigned long           TestFaceId;
		bool                    Parallel;

//		WriteText( "We weren't in the same walkmesh!\n" );

		//
		// Build a list of candidate faces.  We start with the two that we've
		// originally found, but these may be on the wrong sides -- that is,
		// they might not be on the correct tile surface walkmesh.  Check the
		// other sides (if applicable); two of the faces will be on opposite
		// walkmesh tiles (i.e. the triangle we just walked through, and the
		// triangle that we'll walk through after we pass through this next
		// walkmesh.  Conversely, two of the faces will be on the correct
		// walkmesh tile -- the next walkmesh tile up.
		//

		faces[ 0 ] = f1;
		faces[ 1 ] = NULL;
		faces[ 2 ] = f2;
		faces[ 3 ] = NULL;

		for (unsigned long i = 0; i < 3; i += 1)
		{
			e1 = &GetEdges( )[ f1->Edges[ i ] ];

			//
			// Check that the edge intersects with our line segment's overlap
			// coordinate set.
			//

			v1.x = GetPoints( )[ e1->Points1 ].x;
			v1.y = GetPoints( )[ e1->Points1 ].y;
			v2.x = GetPoints( )[ e1->Points2 ].x;
			v2.y = GetPoints( )[ e1->Points2 ].y;

			if (!Math::IntersectSegments2( Start, End, v1, v2, I0, Parallel ))
			{
				e1 = NULL;
				continue;
			}

			//
			// We want the triangle on the other edge of this face.  Try it
			// too.
			//

			if (GetFaceId( f1 ) != e1->Triangles1)
				TestFaceId = e1->Triangles1;
			else
				TestFaceId = e1->Triangles2;

			if (TestFaceId == (unsigned long) -1)
				continue;

			faces[ 1 ] = &GetTriangles( )[ TestFaceId ];

			if (!IsPointInTriangle( faces[ 1 ], Start, GetPoints( ) ))
			{
				faces[ 1 ] = NULL;
				continue;
			}

			break;
		}

		for (unsigned long i = 0; i < 3; i += 1)
		{
			e2 = &GetEdges( )[ f2->Edges[ i ] ];

			//
			// Check that the edge intersects with our line segment's overlap
			// coordinate set.
			//

			v1.x = GetPoints( )[ e2->Points1 ].x;
			v1.y = GetPoints( )[ e2->Points1 ].y;
			v2.x = GetPoints( )[ e2->Points2 ].x;
			v2.y = GetPoints( )[ e2->Points2 ].y;

			if (!Math::IntersectSegments2( Start, End, v1, v2, I0, Parallel ))
			{
				e2 = NULL;
				continue;
			}

			//
			// We want the triangle on the other edge of this face.  Try it
			// too.
			//

			if (GetFaceId( f2 ) != e2->Triangles1)
				TestFaceId = e2->Triangles1;
			else
				TestFaceId = e2->Triangles2;

			if (TestFaceId == (unsigned long) -1)
				continue;

			faces[ 3 ] = &GetTriangles( )[ TestFaceId ];

			if (!IsPointInTriangle( faces[ 3 ], End, GetPoints( )))
			{
				faces[ 3 ] = NULL;
				continue;
			}

			break;
		}

//		faces[ 0 ] = e1->Triangles1 == (unsigned long) -1 ? NULL : &GetTriangles( )[ e1->Triangles1 ];
//		faces[ 1 ] = e1->Triangles2 == (unsigned long) -1 ? NULL : &GetTriangles( )[ e1->Triangles2 ];
//		faces[ 2 ] = e2->Triangles1 == (unsigned long) -1 ? NULL : &GetTriangles( )[ e2->Triangles1 ];
//		faces[ 3 ] = e2->Triangles1 == (unsigned long) -1 ? NULL : &GetTriangles( )[ e2->Triangles2 ];

		f1 = NULL;
		f2 = NULL;

		//
		// Check each triangle/edge permutation for the two that have the
		// same tile surface mesh.  These two triangles will be the ones
		// that we're looking for.
		//

		for (unsigned long i = 0; i < 2 && f1 == NULL; i += 1)
		{
			if (faces[ i + 0 ] == NULL)
				continue;

			for (unsigned long j = 0; j < 2; j += 1)
			{
				if (faces[ j + 2 ] == NULL)
					continue;

//				if ((!IsPointInTriangle( faces[ i + 0 ], Start, GetPoints( ) )) ||
//				    (!IsPointInTriangle( faces[ j + 2 ], End, GetPoints( ) )))
//					continue;

				f1 = faces[ i + 0 ];
				f2 = faces[ j + 2 ];
				t1 = &GetTileSurfaceMesh( f1 );
				t2 = &GetTileSurfaceMesh( f2 );

//				WriteText( "Try again: (%f, %f) and (%f, %f)\n", f1->Centroid2.x, f1->Centroid2.y, f2->Centroid2.x, f2->Centroid2.y );

				if ((t1 == t2) &&
				    (t1 != ExcludeSurfaceMesh))
					break;

				f1 = NULL;
				f2 = NULL;
			}
		}

		if (f1 == NULL)
		{
//			WriteText( "No triangles sharing our edge have the same TSM???\n" );
			return false;
		}
	}

//	WriteText( "%f %f %f %f\n", f1->Centroid2.x, f1->Centroid2.y, f2->Centroid2.x, f2->Centroid2.y );

	*Face1 = f1;
	*Face2 = f2;
	*SurfaceMesh = t1;

	return true;
}

bool
AreaSurfaceMesh::StraightPathExists(
	__in const NWN::Vector2 & v1,
	__in const NWN::Vector2 & v2,
	__out_opt NWN::Vector2 * LastFaceCentroid /* = NULL */
	) const
/*++

Routine Description:

	This routine calculates whether a straight-edge path exists between two
	points.  Collision detection is performed with the baked walkmesh only
	(which includes static walkmesh blockers such as placeables).  Non-walkmesh
	blockers such as creatures are not considered for purposes of path
	existance.

Arguments:

	v1 - Supplies the proposed starting point of the path.

	v2 - Supplies the proposed ending point of the path.

	LastFaceCentroid - Optionally receives the centroid2 of the last face that
	                   was confirmed to be traversed successfully even if there
	                   was no ultimate straight path (i.e. the last face for
	                   which we still were on a straight walkable path).
	                
	                   N.B.  In some circumstances this may not be the exact
	                         centroid2 (i.e. we went off the grid).
Return Value:

	Returns true if a straight path exists, else false.

Environment:

	User mode.

--*/
{
	Vector2Vec              GridIntersections;
	const TileSurfaceMesh * LastSurfaceMesh;
	NWN::Vector2            v;

	//
	// Break the line up into intersecting grid points that specify which tile
	// surface mesh objects should be searched along the way.  Straight-line
	// pathing requires checking fine-grained pathing, which may only be done
	// within a TileSurfaceMesh (the largest unit for which every point has a
	// reachability matrix to every other point).
	//

	try
	{
		IntersectTileSurfaceMeshGrid( GridIntersections, v1, v2 );

		if (GridIntersections.size( ) < 2)
			return false;
	}
	catch (std::exception)
	{
		return false;
	}

	if (ARGUMENT_PRESENT( LastFaceCentroid ))
		*LastFaceCentroid = v1;

	//
	// Iterate over each grid intersection, calculating whether there
	// exists a direct line from bounding intersection to bounding intersection
	// for each intersected tile surface mesh.
	//

	LastSurfaceMesh = NULL;

	for (Vector2Vec::const_iterator it = GridIntersections.begin( );
	     it != GridIntersections.end( );
	     ++it)
	{
		const SurfaceMeshFace *   Face1;
		const SurfaceMeshFace *   Face2;
		const SurfaceMeshFace * * F;
		const SurfaceMeshFace *   LocalLast;
		const TileSurfaceMesh *   SurfaceMesh;

		if (it == GridIntersections.begin( ))
		{
			v = *it;
			continue;
		}

		//
		// Locate the two faces that contain the grid segment and which are
		// joined to the same tile surface mesh.
		//

		if (!GetLineSegmentFacesAndMesh(
			v,
			*it,
			LastSurfaceMesh,
			true,
			&Face1,
			&Face2,
			&SurfaceMesh))
		{
			StraightPathDebug(
				"Unable to find mesh faces for line segment intersections %f, %f -> %f, %f\n",
				v.x,
				v.y,
				it->x,
				it->y);

			return false;
		}

		StraightPathDebug(
			"Checking straight reachability from %f, %f -> %f, %f\n",
			v.x,
			v.y,
			it->x,
			it->y);

		LocalLast = NULL;

		if (!ARGUMENT_PRESENT( LastFaceCentroid ))
			F = NULL;
		else
			F = &LocalLast;

		//
		// Perform a straight-line test with fine-grained walkmesh information.
		//

		if (!SurfaceMesh->StraightPathExists( v, *it, Face1, Face2, this, F ))
		{
			if ((ARGUMENT_PRESENT( LastFaceCentroid )) && (LocalLast != NULL))
				*LastFaceCentroid = LocalLast->Centroid2;

			return false;
		}

		StraightPathDebug( "Reachable!\n" );

		//
		// Start the next line from our current position and check the next
		// tile surface mesh up along the way in the grid intersection.
		//

		v = *it;

		//
		// Prevent the previous surface mesh from being selected in the event
		// of a tie so that forward progress is ensured.  The first time
		// around, there's typically no tie, or we were on the border of a
		// seam anyway and don't care which mesh we chose to go with if both
		// faces matched it.
		//

		LastSurfaceMesh = SurfaceMesh;

		if (ARGUMENT_PRESENT( LastFaceCentroid ))
			*LastFaceCentroid = v;
	}

	return true;
}

bool
AreaSurfaceMesh::CalcContact(
	__in const NWN::Vector3 & Origin,
	__in const NWN::Vector3 & NormDir,
	__out float & IntersectDistance,
	__out_opt const SurfaceMeshFace * * IntersectFace
	) const
/*++

Routine Description:

	This routine calculates the distance that a ray can travel before it comes
	into contact with a face in the surface mesh.

Arguments:

	Origin - Supplies the origin point of the ray.

	NormDir - Supplies the normalized direction of the ray.

	IntersectDistance - Receives the distance from the ray origin for the first
	                    ray intersection, if an intersection existed.

	IntersectFace - Optionally receives the intersecting face pointer, if a
	                intersection existed.

Return Value:

	Returns a Boolean value indicating true if the ray intersected a face
	within the given parameters, else false if no intersection exists.

Environment:

	User mode.

--*/
{
	Vector2Vec              GridIntersections;
	bool                    Intersected;
	Math::QuickRay          Ray( Origin, NormDir );
	Math::QuickBox          Box( GetMinBound( ), GetMaxBound( ) );
	const SurfaceMeshFace * Face;
#if STRAIGHT_PATH_DEBUG
	const SurfaceMeshFace * TempFace;

	if (!ARGUMENT_PRESENT( IntersectFace ))
		IntersectFace = &TempFace;
#endif

	//
	// If we're outside the entire walkmesh region, then don't bother.
	//

	if (!Box.IntersectRay( Ray ))
		return false;

	//
	// Iterate over all tile surface mesh objects, checking whether our ray
	// intersects with the overall bounding region of the mesh.  If not, then
	// we won't bother to hit test against it.
	//

	Intersected = false;

	for (TileSurfaceMeshVec::const_iterator it = m_TileSurfaceMeshes.begin( );
	     it != m_TileSurfaceMeshes.end( );
	     ++it)
	{
		float T;

		if (it->m_NumFaces == 0)
			continue;

		Box.SetBounds( it->GetMinBound( ), it->GetMaxBound( ) );

		if (!Box.IntersectRay( Ray ))
			continue;

		if (it->CalcContact(
			Origin,
			NormDir,
			NULL,
			NULL,
			this,
			T,
			&Face ))
		{
			StraightPathDebug(
				"Intersect face (%f, %f) (dist %f)\n",
				Face->Centroid2.x,
				Face->Centroid2.y,
				IntersectDistance);

			if (!Intersected)
			{
				Intersected       = true;
				IntersectDistance = T;

				if (IntersectFace)
					*IntersectFace = Face;
			}
			else
			{
				if (T < IntersectDistance)
					IntersectDistance = T;

				if (IntersectFace)
					*IntersectFace = Face;
			}
		}
	}

	return Intersected;
}

float
AreaSurfaceMesh::GetPointHeight(
	__in const NWN::Vector2 & pt
	) const
/*++

Routine Description:

	This routine computes the height at a particular point.

Arguments:

	pt - Supplies the point to compute the height for.

Return Value:

	Returns the height, or 0.0f if the coordinates were invalid.

Environment:

	User mode.

--*/
{
	const SurfaceMeshFace * Face;

	Face = FindFace( pt );

	if (Face == NULL)
		return 0.0f;

	return (float) Math::PlaneHeightAtPoint( Face->Normal, Face->D, pt );
}

void
AreaSurfaceMesh::IntersectTileSurfaceMeshGrid(
	__out Vector2Vec & IntersectionPoints,
	__in const NWN::Vector2 & Start,
	__in const NWN::Vector2 & End
	) const
/*++

Routine Description:

	This routine computes all intersection points along the tile surface mesh
	grid for a line drawn between a starting point and an ending point.

	The routine solves y = mx+b and plots intersection coordinates along the
	x-axis, and then again along the y-axis in tile-sized increments.  Then,
	the results are spliced together in order of the movement vector.

	The first point in the returned point array is the starting point, and the
	last point is the ending point.  Intervening points are intersection
	coordinate values.

Arguments:

	IntersectionPoints - Receives the series of intersection points in
	                     traversal order.

	Start - Supplies the starting point of the line.

	End - Supplies the ending point of the line.

Return Value:

	None.  Raises an std::exception on catastrophic failure.

Environment:

	User mode.

--*/
{
	float        Scale;
	float        m;
	float        b;
	float        dx;
	float        dy;
	float        dxA;
	float        dyA;
	int          TilesX;
	int          TilesY;
	int          TileX;
	int          TileY;
	int          sx;
	int          sy;
	bool         Vertical;
	bool         Horizontal;
	Vector2Vec   InterceptsX;
	Vector2Vec   InterceptsY;
	NWN::Vector2 v;

	Scale = 1.0f / m_TileSize;

	dx = (End.x - Start.x);
	dy = (End.y - Start.y);

	dxA = fabs( dx );
	dyA = fabs( dy );

	//
	// Solve y = mx + b for m, and calculate the count of tiles traversed in
	// both x-axis and y-axis directions.
	//

	if (dxA < PathingEpsilon)
	{
		TileX    = 0; // Compiler satisfaction
		TilesX   = 0;
		m        = 1.0;
		Vertical = true;

		IntersectSegDebug( "Line dx is vertical\n" );
	}
	else
	{
		m        = (dy / dx);
		TileX    = (int) (Scale * Start.x);
		TilesX   = abs( (int) (Scale * End.x) - (int) (Scale * Start.x) );
		Vertical = false;

		IntersectSegDebug( "Line dx is normal\n" );
	}

	if (dyA < PathingEpsilon)
	{
		TileY      = 0; // Compiler satisfaction
		TilesY     = 0;
		Horizontal = true;

		IntersectSegDebug( "Line dy is horizontal\n" );
	}
	else
	{
		TileY      = (int) (Scale * Start.y);
		TilesY     = abs( (int) (Scale * End.y) - (int) (Scale * Start.y) );
		Horizontal = false;

		IntersectSegDebug( "Line dy is normal\n" );
	}

	//
	// Calculate the sign of the x-axis and y-axis traversal, and solve
	// y = mx + b for b.
	//

	sx = (End.x > Start.x) ? 1 : -1;
	sy = (End.y > Start.y) ? 1 : -1;

	b = Start.y - m * Start.x;

	InterceptsX.reserve( TilesX );
	InterceptsY.reserve( TilesY );
	IntersectionPoints.reserve( TilesX + TilesY + 2 );

	IntersectionPoints.push_back( Start );

	if (!Vertical)
	{
		for (int i = 0; i <= TilesX; i += 1)
		{
			v.x = ((i + 0) * sx + TileX) * m_TileSize;
			v.y = m * v.x + b;

			if (sx > 0)
			{
				if (v.x <= Start.x)
					continue;
			}
			else if (sx < 0)
			{
				if (v.x >= Start.x)
					continue;
			}

			if (Horizontal)
			{
				if (sx > 0)
				{
					if (v.x > End.x)
						continue;
				}
				else if (sx < 0)
				{
					if (v.x < End.x)
						continue;
				}

				v.y = Start.y;
				InterceptsY.push_back( v );
				continue;
			}

			if (sy > 0)
			{
				if (v.y >= End.y)
					continue;
			}
			else if (sx < 0)
			{
				if (v.y <= End.y)
					continue;
			}

			InterceptsX.push_back( v );
		}
	}

	if (!Horizontal)
	{
		for (int i = 0; i <= TilesY; i += 1)
		{
			v.y = ((i + 0) * sy + TileY) * m_TileSize;

			if (sy > 0)
			{
				if (v.y <= Start.y)
					continue;
			}
			else if (sy < 0)
			{
				if (v.y >= Start.y)
					continue;
			}

			if (Vertical)
			{
				if (sy > 0)
				{
					if (v.y > End.y)
						continue;
				}
				else if (sy < 0)
				{
					if (v.y < End.y)
						continue;
				}

				v.x = Start.x;
				InterceptsY.push_back( v );
				continue;
			}

			v.x = (v.y - b) / m;

			if (sx > 0)
			{
				if (v.x >= End.x)
					continue;
			}
			else if (sx < 0)
			{
				if (v.x <= End.x)
					continue;
			}

			InterceptsY.push_back( v );
		}
	}

	//
	// Now splice them together in sorted order.
	//

	Vector2Vec::iterator xit;
	Vector2Vec::iterator yit;

	for (xit = InterceptsX.begin( ), yit = InterceptsY.begin( );
	     xit != InterceptsX.end( ) || yit != InterceptsY.end( );
	     )
	{
		if ((xit != InterceptsX.end( )) &&
		    (yit != InterceptsY.end( )))
		{
			if (sx < 0)
			{
				if (xit->x > yit->x)
					IntersectionPoints.push_back( *xit++ );
				else
					IntersectionPoints.push_back( *yit++ );
			}
			else
			{
				if (xit->x < yit->x)
					IntersectionPoints.push_back( *xit++ );
				else
					IntersectionPoints.push_back( *yit++ );
			}
		}
		else
		{
			if (xit != InterceptsX.end( ))
				IntersectionPoints.push_back( *xit++ );
			else
				IntersectionPoints.push_back( *yit++ );
		}
	}

	IntersectionPoints.push_back( End );

#if STRAIGHT_PATH_DEBUG
	IntersectSegDebug(
		"** Intersection segments for (%g, %g) -> (%g, %g):\n",
		Start.x,
		Start.y,
		End.x,
		End.y);

	for (Vector2Vec::const_iterator it = IntersectionPoints.begin( );
	     it != IntersectionPoints.end( );
	     ++it)
	{
		IntersectSegDebug( "> %g, %g\n", it->x, it->y );
	}
#endif
}
