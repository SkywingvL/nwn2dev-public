/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ModelInstance.h

Abstract:

	This module defines the ModelInstance class, which represents a unique
	instance of all data that may be shared about a game model, including mesh
	and skeleton data.

	All data represented by the ModelInstance object is required to be stored
	in local coordinate form; thus, collision meshes are not included.

	Typically, a ModelInstance object is thus referred to by a ModelCollider
	object, which contains a reference to a shared ModelInstance and a private
	copy of world-transformed collision meshes.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_MODELINSTANCE_H
#define _PROGRAMS_NWN2DATALIB_MODELINSTANCE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "RigidMesh.h"
#include "SkinMesh.h"
#include "WalkMesh.h"
#include "HookPoint.h"
#include "HairPoint.h"
#include "HelmPoint.h"

//
// Define shared model mesh that is kept for display purposes.
//

class ModelInstance
{

public:

	inline
	ModelInstance(
		)
	{
	}

	inline
	virtual
	~ModelInstance(
		)
	{
	}

	typedef std::vector< RigidMesh > RigidMeshVec;
	typedef std::vector< SkinMesh > SkinMeshVec;
	typedef std::vector< HookPoint > HookPointVec;

	//
	// Mesh access.
	//

	inline
	const RigidMeshVec &
	GetRigidMeshes(
		) const
	{
		return m_RigidMeshes;
	}

	inline
	RigidMeshVec &
	GetRigidMeshes(
		)
	{
		return m_RigidMeshes;
	}

	inline
	const SkinMeshVec &
	GetSkinMeshes(
		) const
	{
		return m_SkinMeshes;
	}

	inline
	SkinMeshVec &
	GetSkinMeshes(
		)
	{
		return m_SkinMeshes;
	}

	inline
	const WalkMesh &
	GetWalkMesh(
		) const
	{
		return m_WalkMesh;
	}

	inline
	WalkMesh &
	GetWalkMesh(
		)
	{
		return m_WalkMesh;
	}

	//
	// Hook point access.
	//

	inline
	const HookPointVec &
	GetHookPoints(
		) const
	{
		return m_HookPoints;
	}

	inline
	HookPointVec &
	GetHookPoint(
		)
	{
		return m_HookPoints;
	}

	//
	// Hair point access.
	//

	inline
	const HairPoint &
	GetHairPoint(
		) const
	{
		return m_HairPoint;
	}

	inline
	HairPoint &
	GetHairPoint(
		)
	{
		return m_HairPoint;
	}

	//
	// Helm point access.
	//

	inline
	const HelmPoint &
	GetHelmPoint(
		) const
	{
		return m_HelmPoint;
	}

	inline
	HelmPoint &
	GetHelmPoint(
		)
	{
		return m_HelmPoint;
	}

	inline
	NWN::MDB_HAIR_SHORTENING_BEHAVIOR
	GetHairShorteningBehavior(
		) const
	{
		return (NWN::MDB_HAIR_SHORTENING_BEHAVIOR) (GetHairPoint( ).GetHeader( ).HairFlag);
	}

	inline
	NWN::MDB_HELM_HAIR_HIDING_BEHAVIOR
	GetHelmHairHidingBehavior(
		) const
	{
		return (NWN::MDB_HELM_HAIR_HIDING_BEHAVIOR) (GetHelmPoint( ).GetHeader( ).HelmFlag);
	}

private:

	//
	// Rigid body mesh for immobile objects.
	//

	RigidMeshVec     m_RigidMeshes;

	//
	// Skinnable mesh layered atop bones for mobile objects.
	//

	SkinMeshVec      m_SkinMeshes;

	//
	// Object walkmesh.
	//
	// N.B.  Pathing typically uses the ASWM instead, which includes baked
	//       pathing details for all static objects with walkmeshes.
	//

	WalkMesh         m_WalkMesh;

	//
	// Hook points.
	//

	HookPointVec     m_HookPoints;

	//
	// Hair point.
	//

	HairPoint        m_HairPoint;

	//
	// Helm point.
	//

	HelmPoint        m_HelmPoint;

};

#endif
