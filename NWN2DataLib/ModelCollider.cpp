/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ModelCollider.cpp

Abstract:

	This module houses the ModelCollider class implementation.  Routines for
	managing operations such as as collision intersection and world transform
	updating are provided.

--*/

#include "Precomp.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#include "ModelCollider.h"

bool
ModelCollider::IntersectRay(
	__in const NWN::Vector3 & Origin,
	__in const NWN::Vector3 & NormDir,
	__out NWN::Vector3 & IntersectNormal,
	__out_opt float * IntersectDistance
	) const
/*++

Routine Description:

	This routine performs a hit-test between a ray and the transformed
	collision mesh of the object.

Arguments:

	Origin - Supplies the origin of the hit test ray.

	NormDir - Supplies the normalized direction of the hit-test ray.

	IntersectNormal - Receives the normal of the face that the ray intersected,
	                  on successful intersection.

	IntersectDistance - Optionally receives the distance from the ray origin at
	                    which the intersection occurred, on successful.
	                    intersection.

Return Value:

	The routine returns a Boolean value indicating whether an intersection was
	detected or not.

Environment:

	User mode.

--*/
{
	NWN::Vector3 Tri[ 3 ];
	float        T;
	bool         Intersected;

	if ((m_C2Mesh.GetFaces( ).empty( )) &&
	    (m_C3Mesh.GetFaces( ).empty( )))
	{
		return false;
	}
	else
	{
		Math::QuickRay Ray( Origin, NormDir );
		Math::QuickBox Box( m_MinBound, m_MaxBound );

		//
		// First, perform a quick test against the bounding region defined by
		// the world coordinate space extremes of our C2 and C3 meshes.
		//

		if (!Box.IntersectRay( Ray ))
			return false;
	}

	Intersected = false;

	//
	// Check C2 first, examining C3 if we have a C2 hit.
	//

	if (!m_C2Mesh.GetFaces( ).empty( ))
	{
		for (CollisionMesh::FaceVec::const_iterator it = m_C2Mesh.GetFaces( ).begin( );
		     it != m_C2Mesh.GetFaces( ).end( );
		     ++it)
		{
			for (size_t i = 0; i < 3; i += 1)
				Tri[ i ] = m_C2Mesh.GetPoint3( it->Corners[ i ] );

			if (!Math::IntersectRayTriRejectBackface(
				Origin,
				NormDir,
				Tri,
				T))
			{
				continue;
			}

			//
			// If we've no C3 mesh, then this as an intersection.
			//

			if (m_C3Mesh.GetFaces( ).empty( ))
			{
				//
				// Only keep on searching if the caller really requires the closest
				// intersection distance.
				//

				if (!ARGUMENT_PRESENT( IntersectDistance ))
				{
					IntersectNormal = it->Normal;

					return true;
				}

				if (!Intersected)
				{
					Intersected = true;

					*IntersectDistance = T;
					IntersectNormal    = it->Normal;
				}
				else if (*IntersectDistance > T)
				{
					*IntersectDistance = T;
					IntersectNormal    = it->Normal;
				}
			}

			//
			// Check against the C3 mesh.
			//
			// N.B.  The server will continue testing the rest of the C2 mesh
			//       if the C3 mesh search did not turn up a hit.  This is not
			//       necessary as the C3 mesh is a superset of the C2 mesh.
			//

			break;
		}
	}

	//
	// Now check against the fine-grained C3 mesh.  The C3 mesh is the final
	// authority on collisions unless we have only the C2 mesh, in which case
	// the C2 mesh is it.
	//

	for (CollisionMesh::FaceVec::const_iterator it = m_C3Mesh.GetFaces( ).begin( );
	     it != m_C3Mesh.GetFaces( ).end( );
	     ++it)
	{
		for (size_t i = 0; i < 3; i += 1)
			Tri[ i ] = m_C3Mesh.GetPoint3( it->Corners[ i ] );

		if (!Math::IntersectRayTri(
			Origin,
			NormDir,
			Tri,
			T))
		{
			continue;
		}

		//
		// Only keep on searching if the caller really requires the closest
		// intersection distance.
		//

		if (!ARGUMENT_PRESENT( IntersectDistance ))
		{
			IntersectNormal = it->Normal;

			return true;
		}

		if (!Intersected)
		{
			Intersected = true;

			*IntersectDistance = T;
			IntersectNormal    = it->Normal;
		}
		else if (*IntersectDistance < T)
		{
			*IntersectDistance = T;
			IntersectNormal    = it->Normal;
		}
	}

	return Intersected;
}

bool
ModelCollider::IntersectRay(
	__in const Math::QuickRay & Ray,
	__out NWN::Vector3 & IntersectNormal,
	__out_opt float * IntersectDistance
	) const
/*++

Routine Description:

	This routine performs a hit-test between a ray and the transformed
	collision mesh of the object.

Arguments:

	Ray - Supplies the hit test ray.

	IntersectNormal - Receives the normal of the face that the ray intersected,
	                  on successful intersection.

	IntersectDistance - Optionally receives the distance from the ray origin at
	                    which the intersection occurred, on successful.
	                    intersection.

Return Value:

	The routine returns a Boolean value indicating whether an intersection was
	detected or not.

Environment:

	User mode.

--*/
{
	NWN::Vector3 Tri[ 3 ];
	float        T;
	bool         Intersected;

	if ((m_C2Mesh.GetFaces( ).empty( )) &&
	    (m_C3Mesh.GetFaces( ).empty( )))
	{
		return false;
	}
	else
	{
		Math::QuickBox Box( m_MinBound, m_MaxBound );

		//
		// First, perform a quick test against the bounding region defined by
		// the world coordinate space extremes of our C2 and C3 meshes.
		//

		if (!Box.IntersectRay( Ray ))
			return false;
	}

	Intersected = false;

	//
	// Check C2 first, examining C3 if we have a C2 hit.
	//

	if (!m_C2Mesh.GetFaces( ).empty( ))
	{
		for (CollisionMesh::FaceVec::const_iterator it = m_C2Mesh.GetFaces( ).begin( );
		     it != m_C2Mesh.GetFaces( ).end( );
		     ++it)
		{
			for (size_t i = 0; i < 3; i += 1)
				Tri[ i ] = m_C2Mesh.GetPoint3( it->Corners[ i ] );

			if (!Math::IntersectRayTriRejectBackface(
				Ray.m_origin,
				Ray.m_direction,
				Tri,
				T))
			{
				continue;
			}

			//
			// If we've no C3 mesh, then this as an intersection.
			//

			if (m_C3Mesh.GetFaces( ).empty( ))
			{
				//
				// Only keep on searching if the caller really requires the closest
				// intersection distance.
				//

				if (!ARGUMENT_PRESENT( IntersectDistance ))
				{
					IntersectNormal = it->Normal;

					return true;
				}

				if (!Intersected)
				{
					Intersected = true;

					*IntersectDistance = T;
					IntersectNormal    = it->Normal;
				}
				else if (*IntersectDistance > T)
				{
					*IntersectDistance = T;
					IntersectNormal    = it->Normal;
				}
			}

			//
			// Check against the C3 mesh.
			//
			// N.B.  The server will continue testing the rest of the C2 mesh
			//       if the C3 mesh search did not turn up a hit.  This is not
			//       necessary as the C3 mesh is a superset of the C2 mesh.
			//

			break;
		}
	}

	//
	// Now check against the fine-grained C3 mesh.  The C3 mesh is the final
	// authority on collisions unless we have only the C2 mesh, in which case
	// the C2 mesh is it.
	//

	for (CollisionMesh::FaceVec::const_iterator it = m_C3Mesh.GetFaces( ).begin( );
	     it != m_C3Mesh.GetFaces( ).end( );
	     ++it)
	{
		for (size_t i = 0; i < 3; i += 1)
			Tri[ i ] = m_C3Mesh.GetPoint3( it->Corners[ i ] );

		if (!Math::IntersectRayTri(
			Ray.m_origin,
			Ray.m_direction,
			Tri,
			T))
		{
			continue;
		}

		//
		// Only keep on searching if the caller really requires the closest
		// intersection distance.
		//

		if (!ARGUMENT_PRESENT( IntersectDistance ))
		{
			IntersectNormal = it->Normal;

			return true;
		}

		if (!Intersected)
		{
			Intersected = true;

			*IntersectDistance = T;
			IntersectNormal    = it->Normal;
		}
		else if (*IntersectDistance > T)
		{
			*IntersectDistance = T;
			IntersectNormal    = it->Normal;
		}
	}

	return Intersected;
}
