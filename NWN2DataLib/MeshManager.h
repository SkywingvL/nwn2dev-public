/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	MeshManager.h

Abstract:

	This module defines the interface to the mesh manager, which tracks all
	meshes present in the system.

	The mesh manager tracks meshes independent of their associated objects, and
	is used when operations that must impact all meshes (such as recreation of
	device dependent mesh objects) must be performed irrespective of where the
	meshes themselves are connected to.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_MESHMANAGER_H
#define _PROGRAMS_NWN2DATALIB_MESHMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "MeshLinkage.h"

//
// Define mesh manager callouts.
//

class IMeshManagerNotifications
{

public:

	//
	// OnMeshManagerRegisterNewMesh is invoked when a new mesh is loaded into
	// the system.  Its purpose is to allow renderer-dependent data (such as
	// hardware meshes) to be created corresponding to the new mesh.
	//
	// MeshObject points to the body of a mesh of type Type.
	// Type indicates the type of mesh, drawn from MeshType.
	//

	virtual
	void
	OnMeshManagerRegisterNewMesh(
		__in void * MeshObject,
		__in MeshLinkage * Linkage,
		__in MeshLinkageTraits::MeshType Type
		) = 0;

};

//
// Define the overarching mesh manager itself.
//

class MeshManager
{

public:

	MeshManager(
		);

	~MeshManager(
		);

	//
	// List iterator callback routine.
	//
	// MeshObject points to the body of a mesh of type Type.
	// Type indicates the type of the mesh, drawn from enum MeshType.
	// Context1 supplies an opaque user contextural parameter.
	// Context2 supplies an opaque user contextural parameter.
	//
	// The routine returns true to continue enumeration, else false to stop
	// further enumeration.
	//

	typedef
	bool
	(__stdcall * ForEachMeshProc)(
		__in void * MeshObject,
		__in MeshLinkage * Linkage,
		__in MeshLinkageTraits::MeshType Type,
		__in void * Context1,
		__in void * Context2
		);

	//
	// Called when a mesh is registered with the mesh manager.  A mesh may be
	// registered only once.
	//

	void
	OnMeshRegister(
		__in PLIST_ENTRY MeshLinks,
		__in const MeshLinkageTraits * Traits
		);

	//
	// Enumerate all meshes in the system, calling an enumerator procedure for
	// each mesh with the given mesh's type information.
	//
	// The enumerator procedure must not cause a mesh to be created or deleted.
	//

	void
	ForEachMesh(
		__in ForEachMeshProc EnumeratorProc,
		__in void * Context1,
		__in void * Context2
		);

	//
	// Register a notification callout interface with the mesh manager.  The
	// previous callout interface is returned.
	//

	inline
	IMeshManagerNotifications *
	SetMeshManagerNotifications(
		__in_opt IMeshManagerNotifications * Notifications
		)
	{
		IMeshManagerNotifications * OldNotifications = m_NotificationCallout;

		m_NotificationCallout = Notifications;
		return OldNotifications;
	}

private:

	typedef MeshLinkageTraits::MeshType MeshType;

	struct ForEachMeshCtx
	{
		ForEachMeshProc   EnumeratorProc;
		void            * Context1;
		void            * Context2;
		void            * Context3;
		bool              Stopped;
	};

	//
	// Initializes a mesh list.
	//

	template< class T, MeshType TypeCode >
	void
	InitializeMeshList(
		__in int Reserved,
		__in PLIST_ENTRY ListHead
		);

	//
	// Call an enumerator procedure for each entry in a mesh list.
	//

	template< class T, MeshType TypeCode >
	void
	ForEachMesh_EnumerateMeshList(
		__inout ForEachMeshCtx * Ctx,
		__in PLIST_ENTRY ListHead
		);

	//
	// Call the notification callout for a given incoming mesh.
	//

	template< class T, MeshType TypeCode >
	void
	DeliverIncomingMeshNotification(
		__in PLIST_ENTRY ListLinks
		);

	//
	// Define linked lists for each mesh type that we may need to iterate
	// across.
	//

	LIST_ENTRY                  m_CollisionMeshListHead;
	LIST_ENTRY                  m_RigidMeshListHead;
	LIST_ENTRY                  m_SkinMeshListHead;
	LIST_ENTRY                  m_WalkMeshListHead;
	LIST_ENTRY                  m_AreaSurfaceMeshListHead;
	LIST_ENTRY                  m_AreaTerrainMeshListHead;
	LIST_ENTRY                  m_AreaWaterMeshListHead;

	//
	// Define the notification interface (if registered).
	//

	IMeshManagerNotifications * m_NotificationCallout;

};

#endif
