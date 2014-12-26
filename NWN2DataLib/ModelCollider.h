/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ModelCollider.h

Abstract:

	This module defines the ModelCollider class, which represents a copyable
	object that encapsulates both shareable (ModelInstance) data about a model,
	and non-shareable (CollisionMesh) data about a model.

	As the name implies, the ModelCollider object also includes all data that
	is necessary to perform collision intersections with its associated meshes.

	For multi-part models, a ModelCollider represents a single part of the
	multi-part model.  A single model part may however have multiple rigid
	meshes associated with it, which is typically the case for a placeable
	object.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_MODELCOLLIDER_H
#define _PROGRAMS_NWN2DATALIB_MODELCOLLIDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ModelInstance.h"
#include "CollisionMesh.h"
#include "CollisionSphere.h"

namespace Math
{
	class QuickRay;
}

//
// Define the model collider, used to contain data for collision hit testing on
// an object.  The model collider also contains a reference to the shared model
// meshes for display purposes.
//

class ModelCollider
{

public:

	inline
	ModelCollider(
		)
	{
	}

	inline
	virtual
	~ModelCollider(
		)
	{
	}

	//
	// Create the attached model instance.
	//

	inline
	void
	CreateModelInstance(
		)
	{
		m_ModelInstance = new ModelInstance( );
	}

	typedef ModelInstance::RigidMeshVec RigidMeshVec;
	typedef ModelInstance::SkinMeshVec SkinMeshVec;
	typedef ModelInstance::HookPointVec HookPointVec;

	//
	// Mesh access.
	//

	inline
	const CollisionMesh &
	GetC2Mesh(
		) const
	{
		return m_C2Mesh;
	}

	inline
	CollisionMesh &
	GetC2Mesh(
		)
	{
		return m_C2Mesh;
	}

	inline
	const CollisionMesh &
	GetC3Mesh(
		) const
	{
		return m_C3Mesh;
	}

	inline
	CollisionMesh &
	GetC3Mesh(
		)
	{
		return m_C3Mesh;
	}

	inline
	const RigidMeshVec &
	GetRigidMeshes(
		) const
	{
		return m_ModelInstance->GetRigidMeshes( );
	}

	inline
	RigidMeshVec &
	GetRigidMeshes(
		)
	{
		return m_ModelInstance->GetRigidMeshes( );
	}

	inline
	const SkinMeshVec &
	GetSkinMeshes(
		) const
	{
		return m_ModelInstance->GetSkinMeshes( );
	}

	inline
	SkinMeshVec &
	GetSkinMeshes(
		)
	{
		return m_ModelInstance->GetSkinMeshes( );
	}

	inline
	const WalkMesh &
	GetWalkMesh(
		) const
	{
		return m_ModelInstance->GetWalkMesh( );
	}

	inline
	WalkMesh &
	GetWalkMesh(
		)
	{
		return m_ModelInstance->GetWalkMesh( );
	}

	//
	// Sphere access.
	//

	inline
	const CollisionSphereList &
	GetCollisionSpheres(
		) const
	{
		return m_Spheres;
	}

	inline
	CollisionSphereList &
	GetCollisionSpheres(
		)
	{
		return m_Spheres;
	}

	//
	// Hook point access.
	//

	inline
	const HookPointVec &
	GetHookPoints(
		) const
	{
		return m_ModelInstance->GetHookPoints( );
	}

	inline
	HookPointVec &
	GetHookPoints(
		)
	{
		return m_ModelInstance->GetHookPoint( );
	}

	//
	// Hair point access.
	//

	inline
	const HairPoint &
	GetHairPoint(
		) const
	{
		return m_ModelInstance->GetHairPoint( );
	}

	inline
	HairPoint &
	GetHairPoint(
		)
	{
		return m_ModelInstance->GetHairPoint( );
	}

	//
	// Helm point access.
	//

	inline
	const HelmPoint &
	GetHelmPoint(
		) const
	{
		return m_ModelInstance->GetHelmPoint( );
	}

	inline
	HelmPoint &
	GetHelmPoint(
		)
	{
		return m_ModelInstance->GetHelmPoint( );
	}

	inline
	NWN::MDB_HAIR_SHORTENING_BEHAVIOR
	GetHairShorteningBehavior(
		) const
	{
		return m_ModelInstance->GetHairShorteningBehavior( );
	}

	inline
	NWN::MDB_HELM_HAIR_HIDING_BEHAVIOR
	GetHelmHairHidingBehavior(
		) const
	{
		return m_ModelInstance->GetHelmHairHidingBehavior( );
	}

	//
	// Intersect a ray with the model and return the intersection normal on a
	// successful intersection.
	//

	bool
	IntersectRay(
		__in const NWN::Vector3 & Origin,
		__in const NWN::Vector3 & NormDir,
		__out NWN::Vector3 & IntersectNormal,
		__out_opt float * IntersectDistance
		) const;

	bool
	IntersectRay(
		__in const Math::QuickRay & Ray,
		__out NWN::Vector3 & IntersectNormal,
		__out_opt float * IntersectDistance
		) const;

	//
	// Transform the collider meshes against a 4x4 matrix.
	//

	void
	Update(
		__in const NWN::Matrix44 & M
		)
	{
		m_C3Mesh.Update( M );
		m_C2Mesh.Update( M );

		//
		// Recalculate the bounding box.
		//

		m_MaxBound.x = -FLT_MAX;
		m_MaxBound.y = -FLT_MAX;
		m_MaxBound.z = -FLT_MAX;
		m_MinBound.x = +FLT_MAX;
		m_MinBound.y = +FLT_MAX;
		m_MinBound.z = +FLT_MAX;

		m_C3Mesh.UpdateBoundingBox( m_MinBound, m_MaxBound );
		m_C2Mesh.UpdateBoundingBox( m_MinBound, m_MaxBound );

//		for (RigidMeshVec::iterator it = m_RigidMeshes.begin( );
//		     it != m_RigidMeshes.end( );
//		     ++it)
//		{
//			it->Update( M );
//		}
//
//		m_SkinMesh.Update( M );
//		m_WalkMesh.Update( M );
	}

	//
	// Return the radius of a 3D sphere that would encapsulate the entire model
	// collision structure, else false if there was no collision data loaded.
	//

	inline
	bool
	CalculateModelSpace(
		__out float & SphereRadius
		) const
	{
		float Rx;
		float Ry;
		float Rz;

		if ((m_C3Mesh.GetFaces( ).empty( )) && (m_C2Mesh.GetFaces( ).empty( )))
			return false;

		Rx = fabs( m_MaxBound.x - m_MinBound.x );
		Ry = fabs( m_MaxBound.y - m_MinBound.y );
		Rz = fabs( m_MaxBound.z - m_MinBound.z );

		SphereRadius = Rx;
		SphereRadius = max( SphereRadius, Ry );
		SphereRadius = max( SphereRadius, Rz );

		SphereRadius /= 2.0f;

		return true;
	}

private:

	typedef swutil::SharedPtr< ModelInstance > ModelInstancePtr;

	//
	// Define the coarse-grained collision mesh.
	//

	CollisionMesh       m_C2Mesh;

	//
	// Define the fine-grained collision mesh.
	//

	CollisionMesh       m_C3Mesh;

	//
	// Define the collision sphere list.
	//

	CollisionSphereList m_Spheres;

	//
	// Shared model instance meshes and other model instance data.
	//

	ModelInstancePtr    m_ModelInstance;

	//
	// Minimum and maximum bounds.
	//

	NWN::Vector3        m_MinBound;
	NWN::Vector3        m_MaxBound;

};

typedef swutil::SharedPtr< ModelCollider > ModelColliderPtr;

#endif
