/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	MeshManager.cpp

Abstract:

	This module houses the mesh manager, which tracks all mesh objects that are
	loaded as resources.  Services to support low level graphics functionality,
	such as traversing the active mesh list to perform a device dependent
	resource reload are provided.

	The mesh manager itself is not responsible for the actual mesh loading
	operations which are instead driven by the resource manager.

--*/

#include "Precomp.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#include "MeshManager.h"
#include "DdsImage.h"
#include "SurfaceMeshBase.h"
#include "AreaSurfaceMesh.h"
#include "AreaWaterMesh.h"
#include "AreaTerrainMesh.h"
#include "AreaHeightMap.h"
#include "SimpleMesh.h"
#include "MDBFormat.h"
#include "RigidMesh.h"
#include "SkinMesh.h"
#include "CollisionMesh.h"
#include "WalkMesh.h"

#if 0
#define FOR_EACH_MESH_LIST( Ctx, Functor, ListHead, MeshType )                                                        \
	(Functor) ( Ctx, &m_CollisionMeshListHead  , MT_CollisionMesh  , FIELD_OFFSET( CollisionMesh  , &m_MeshLinks ) ); \
	(Functor) ( Ctx, &m_RigidMeshHead          , MT_RigidMesh      , FIELD_OFFSET( RigidMesh      , &m_MeshLinks ) ); \
	(Functor) ( Ctx, &m_SkinMeshListHead       , MT_SkinMesh       , FIELD_OFFSET( SkinMesh       , &m_MeshLinks ) ); \
	(Functor) ( Ctx, &m_WalkMeshListHead       , MT_WalkMesh       , FIELD_OFFSET( WalkMesh       , &m_MeshLinks ) ); \
/*	(Functor) ( Ctx, &m_AreaSurfaceMeshListHead, MT_AreaSurfaceMesh, FIELD_OFFSET( AreaSurfaceMesh, &m_MeshLinks ) ); */ \ 
/*	(Functor) ( Ctx, &m_AreaTerrainMeshListHead, MT_AreaTerrainMesh, FIELD_OFFSET( AreaTerrainMesh, &m_MeshLinks ) ); */ \ 
/*	(Functor) ( Ctx, &m_AreaWaterMeshListHead  , MT_AreaWaterMesh  , FIELD_OFFSET( AreaWaterMesh  , &m_MeshLinks ) ); */
#endif

//
// Execute an enumerator function for each list type.
//
// The enumerator function should be defined as so:
//
// template< class MeshClassType, MeshType TypeCode >
// void
// ForEachMeshListFunctor(
//      __in FunctorContext Ctx,
//      __in PLIST_ENTRY ListHead
//      );
//

#define FOR_EACH_MESH_LIST( Ctx, Functor )                                                                    \
	(Functor< CollisionMesh  , MeshLinkageTraits::MT_CollisionMesh   > )( Ctx, &m_CollisionMeshListHead   );  \
	(Functor< RigidMesh      , MeshLinkageTraits::MT_RigidMesh       > )( Ctx, &m_RigidMeshListHead       );  \
	(Functor< SkinMesh       , MeshLinkageTraits::MT_SkinMesh        > )( Ctx, &m_SkinMeshListHead        );  \
	(Functor< WalkMesh       , MeshLinkageTraits::MT_WalkMesh        > )( Ctx, &m_WalkMeshListHead        );  \
	(Functor< AreaSurfaceMesh, MeshLinkageTraits::MT_AreaSurfaceMesh > )( Ctx, &m_AreaSurfaceMeshListHead );  \
	(Functor< AreaTerrainMesh, MeshLinkageTraits::MT_AreaTerrainMesh > )( Ctx, &m_AreaTerrainMeshListHead );  \
	(Functor< AreaWaterMesh  , MeshLinkageTraits::MT_AreaWaterMesh   > )( Ctx, &m_AreaWaterMeshListHead   );

//
// Invoke the incoming mesh callout for a given type of mesh.
//

#define DELIVER_INCOMING_MESH_NOTIFICATION( Type, ListEntry )                            \
	DeliverIncomingMeshNotification< Type, MeshLinkageTraits::MT_##Type >( ListEntry );

MeshManager::MeshManager(
	)
/*++

Routine Description:

	This routine initializes a MeshManager object.

Arguments:

	None.

Return Value:

	The newly constructed object.

Environment:

	User mode.

--*/
: m_NotificationCallout( NULL )
{
	//
	// Set up the mesh linked lists.
	//

	FOR_EACH_MESH_LIST( 0, InitializeMeshList );
}

MeshManager::~MeshManager(
	)
/*++

Routine Description:

	This routine cleans up an already-existing MeshManager object.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
}

void
MeshManager::OnMeshRegister(
	__in PLIST_ENTRY MeshLinks,
	__in const MeshLinkageTraits * Traits
	)
/*++

Routine Description:

	This routine is called when a mesh object is created.  It sets up the 
	tracking links referring to the new mesh object.

	The mesh object removes itself from the list automatically on destruction.

Arguments:

	MeshLinks - Supplies the list links to initialize for the new mesh object.

	Traits - Supplies the mesh traits for the mesh.

Return Value:

	None.

Environment:

	User mode, called from SimpleMesh constructor only !

--*/
{
	switch (Traits->Type)
	{

	case MeshLinkageTraits::MT_CollisionMesh:
		swutil::InsertHeadList( &m_CollisionMeshListHead, MeshLinks );
		DELIVER_INCOMING_MESH_NOTIFICATION( CollisionMesh, MeshLinks );
		break;

	case MeshLinkageTraits::MT_RigidMesh:
		swutil::InsertHeadList( &m_RigidMeshListHead, MeshLinks );
		DELIVER_INCOMING_MESH_NOTIFICATION( RigidMesh, MeshLinks );
		break;

	case MeshLinkageTraits::MT_SkinMesh:
		swutil::InsertHeadList( &m_SkinMeshListHead, MeshLinks );
		DELIVER_INCOMING_MESH_NOTIFICATION( SkinMesh, MeshLinks );
		break;

	case MeshLinkageTraits::MT_WalkMesh:
		swutil::InsertHeadList( &m_WalkMeshListHead, MeshLinks );
		DELIVER_INCOMING_MESH_NOTIFICATION( WalkMesh, MeshLinks );
		break;

	case MeshLinkageTraits::MT_AreaSurfaceMesh:
		swutil::InsertHeadList( &m_AreaSurfaceMeshListHead, MeshLinks );
		DELIVER_INCOMING_MESH_NOTIFICATION( AreaSurfaceMesh, MeshLinks );
		break;

	case MeshLinkageTraits::MT_AreaTerrainMesh:
		swutil::InsertHeadList( &m_AreaTerrainMeshListHead, MeshLinks );
		DELIVER_INCOMING_MESH_NOTIFICATION( AreaTerrainMesh, MeshLinks );
		break;

	case MeshLinkageTraits::MT_AreaWaterMesh:
		swutil::InsertHeadList( &m_AreaWaterMeshListHead, MeshLinks );
		DELIVER_INCOMING_MESH_NOTIFICATION( AreaWaterMesh, MeshLinks );
		break;

	default:
		//
		// A new case statement must be added if a new mesh type is defined.
		//

		C_ASSERT( MeshLinkageTraits::MT_LastMesh == MeshLinkageTraits::MT_AreaWaterMesh + 1 );
		__assume( 0 );

	}
}

void
MeshManager::ForEachMesh(
	__in ForEachMeshProc EnumeratorProc,
	__in void * Context1,
	__in void * Context2
	)
/*++

Routine Description:

	This routine calls an enumerator procedure for each mesh in the system.  A
	pointer to the underlying mesh object and the mesh type, along with context
	parameters, are provided to the enumerator procedure.

	The enumerator procedure must not cause a mesh to be created or deleted.

Arguments:

	EnumeratorProc - Supplies the enumerator procedure to invoke.

	Context1 - Supplies the first opaque context parameter.

	Context2 - Supplies the second opaque context parameter.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	ForEachMeshCtx Ctx;

	Ctx.EnumeratorProc = EnumeratorProc;
	Ctx.Context1       = Context1;
	Ctx.Context2       = Context2;
	Ctx.Stopped        = false;

	FOR_EACH_MESH_LIST( &Ctx, ForEachMesh_EnumerateMeshList );
}

template< class T, MeshManager::MeshType TypeCode >
void
MeshManager::InitializeMeshList(
	__in int Reserved,
	__in PLIST_ENTRY ListHead
	)
/*++

Routine Description:

	This FOR_EACH_MESH_LIST routine initializes the list head of each mesh
	list.

Arguments:

	Reserved - The argument is not used.

	ListHead - Supplies the list head to initialize.

Return Value:

	None.

Environment:

	User mode, invoked by FOR_EACH_MESH_LIST from MeshManager::MeshManager.

--*/
{
	UNREFERENCED_PARAMETER( Reserved );

	swutil::InitializeListHead( ListHead );
}

template< class T, MeshManager::MeshType TypeCode >
void
MeshManager::ForEachMesh_EnumerateMeshList(
	__inout ForEachMeshCtx * Ctx,
	__in PLIST_ENTRY ListHead
	)
/*++

Routine Description:

	This FOR_EACH_MESH_LIST routine invokes a user callback for each entry in
	a mesh list.  The routine supports enumeration of all meshes system-wide.

Arguments:

	Ctx - Supplies the mesh enumerator context.

	ListHead - Supplies the list head to traverse.

Return Value:

	None.

Environment:

	User mode, invoked by FOR_EACH_MESH_LIST from MeshManager::ForEachMesh.

--*/
{
	PLIST_ENTRY ListEntry;

	//
	// If the user has halted enumeration then do not traverse any further.
	//

	if (Ctx->Stopped)
		return;

	for (ListEntry  = ListHead->Flink;
	     ListEntry != ListHead;
	     ListEntry  = ListEntry->Flink)
	{
		T * MeshObject = (T* )(((PCHAR) ListEntry) - MESH_LINKAGE_OFFSET( T ));

		if (!Ctx->EnumeratorProc(
			MeshObject,
			CONTAINING_RECORD( ListEntry, MeshLinkage, m_MeshLinks ),
			TypeCode,
			Ctx->Context1,
			Ctx->Context2))
		{
			Ctx->Stopped = true;
			break;
		}
	}
}

template< class T, MeshManager::MeshType TypeCode >
void
MeshManager::DeliverIncomingMeshNotification(
	__in PLIST_ENTRY ListEntry
	)
/*++

Routine Description:

	This routine invokes the user on mesh arrival callout for a new entry in
	a mesh list.

Arguments:

	ListEntry - Supplies the list links of the new mesh object.

Return Value:

	None.

Environment:

	User mode, invoked by DELIVER_INCOMING_MESH_NOTIFICATION from
	MeshManager::OnRegisterMesh.

--*/
{
	T * MeshObject = (T* )(((PCHAR) ListEntry) - MESH_LINKAGE_OFFSET( T ));

	if (m_NotificationCallout == NULL)
		return;

	m_NotificationCallout->OnMeshManagerRegisterNewMesh(
		MeshObject,
		CONTAINING_RECORD( ListEntry, MeshLinkage, m_MeshLinks ),
		TypeCode);
}

