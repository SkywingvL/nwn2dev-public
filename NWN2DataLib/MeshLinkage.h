/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	MeshLinkage.h

Abstract:

	This module defines the link between a mesh of an arbitrary type and the
	mesh manager, the MeshLinkage class.

	The MeshLinkage class also provides services to allow external attachments
	to be made to a mesh by an arbitrary set of third parties.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_MESHLINKAGE_H
#define _PROGRAMS_NWN2DATALIB_MESHLINKAGE_H

#ifdef _MSC_VER
#pragma once
#endif

class MeshManager;

//
// Define the mesh traits descriptor that describes general characteristics
// about a mesh type.
//

struct MeshLinkageTraits
{
	//
	// Define the types of derived classes supported.
	//

	enum MeshType
	{
		MT_CollisionMesh,
		MT_RigidMesh,
		MT_SkinMesh,
		MT_WalkMesh,
		MT_AreaSurfaceMesh,
		MT_AreaTerrainMesh,
		MT_AreaWaterMesh,

		MT_LastMesh
	};

	//
	// Define the derived type of this mesh.
	//

	MeshType Type;

	//
	// Define the delta from the mesh's MeshLinkage object to the start of the
	// mesh derived class (for instance, RigidMesh or AreaSurfaceMesh).
	//

	LONG     MeshLinkageToBodyOffset;
};

//
// Calculate initializer for MeshLinkageToBodyOffset.  The returned offset is
// the delta for the derived's MeshLinkage object list links to the start of
// the derived's instance.
//

#define MESH_LINKAGE_OFFSET( Derived ) \
	( FIELD_OFFSET( MeshLinkage, m_MeshLinks ) + FIELD_OFFSET( Derived, m_MeshLinkage ) )

//
// Declare inheritance from mesh linkage.  Must be in a public: header.
//

#define DECLARE_MESH_LINKAGE                                    \
	mutable MeshLinkage m_MeshLinkage;                          \
	                                                            \
	inline void RegisterMesh( MeshManager & MeshMgr )           \
	{                                                           \
		m_MeshLinkage.RegisterMesh( MeshMgr );                  \
	}                                                           \
	                                                            \
	inline AssociatedMeshBase * GetAssociatedMesh( ) const      \
	{                                                           \
		return m_MeshLinkage.GetAssociatedMesh( );              \
	}                                                           \
	                                                            \
	inline AssociatedMeshBase::Ptr SetAssociatedMesh(           \
	       __in AssociatedMeshBase::Ptr Mesh                    \
		   ) const                                              \
	{                                                           \
		return m_MeshLinkage.SetAssociatedMesh( Mesh );         \
	}                                                           

//
// Define the associated mesh base, which is used to attach data to a simple
// mesh.
//

class AssociatedMeshBase
{

public:

	typedef swutil::SharedPtr< AssociatedMeshBase > Ptr;

	inline
	virtual
	~AssociatedMeshBase(
		)
	{
	}

private:

};

//
// Define the underlying MeshLinkage object that is attached to all derived
// mesh objects.
//

class MeshLinkage
{

public:

	inline
	MeshLinkage(
		__in const MeshLinkageTraits * Traits
		)
	: m_AttachedMeshManager( NULL ),
	  m_Traits( Traits ),
	  m_AssociatedMesh( NULL )
	{
	}

	inline
	MeshLinkage(
		__in const MeshLinkage & other
		)
	: m_AttachedMeshManager( NULL ),
	  m_Traits( other.m_Traits ),
	  m_AssociatedMesh( other.m_AssociatedMesh )
	{
		//
		// Connect this mesh to the mesh manager if the copied instance was
		// connected.
		//

		if (other.m_AttachedMeshManager != NULL)
			RegisterMesh( *other.m_AttachedMeshManager );
	}

	inline
	~MeshLinkage(
		)
	{
		if (m_AttachedMeshManager != NULL)
		{
			swutil::RemoveEntryList( &m_MeshLinks );
		}
	}

	inline
	MeshLinkage &
	operator=(
		__in const MeshLinkage & other
		)
	{
		m_Traits = other.m_Traits;

		RegisterMesh( *other.m_AttachedMeshManager );

		return *this;
	}

	//
	// Register the mesh with the overarching mesh manager.
	//

	void
	RegisterMesh(
		__in MeshManager & MeshMgr
		);

	//
	// Associated mesh access.  The associated mesh is an arbitrary set of user
	// supplied data that is connected to the raw mesh construct.  Typically it
	// would be graphics mesh objects (D3D, etc).
	//
	// N.B.  Const as the mesh itself does not have data changed, but the user
	//       tagged data may change in the AssociatedMesh.
	//

	inline
	AssociatedMeshBase *
	GetAssociatedMesh(
		) const
	{
		return m_AssociatedMesh.get( );
	}

	inline
	AssociatedMeshBase::Ptr
	SetAssociatedMesh(
		__in AssociatedMeshBase::Ptr AssociatedMesh
		) const
	{
		AssociatedMeshBase::Ptr PrevMesh;

		PrevMesh         = m_AssociatedMesh;
		m_AssociatedMesh = AssociatedMesh;

		return PrevMesh;
	}

private:

	MeshManager                     * m_AttachedMeshManager;
	const MeshLinkageTraits         * m_Traits;
	mutable AssociatedMeshBase::Ptr   m_AssociatedMesh;

public: // N.B.  MeshLinks is for internal use only!

	LIST_ENTRY                m_MeshLinks;

private:

	friend class MeshManager;

};

#endif
