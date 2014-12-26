/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	WorldObject.cpp

Abstract:

	This module houses the WorldObject object, which represents a 3D object in
	the world.

--*/

#include "Precomp.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#include "../NWN2DataLib/ResourceManager.h"
#include "../NWN2DataLib/TrxFileReader.h"
#include "../NWN2DataLib/ModelSkeleton.h"
#include "WorldObject.h"

WorldObject::WorldObject(
	__in ResourceManager & ResMan,
	__in IDebugTextOut * TextWriter,
	__in const std::vector< std::string > & MDBResRefs,
	__in const std::string & GR2ResRef
	)
/*++

Routine Description:

	This routine constructs a new WorldObject, which represents a object with
	models that is present in model space.

Arguments:

	ResMan - Supplies the resource manager instance to use to load the model
	         data.

	TextWriter - Supplies the debug text output writer.

	MDBResRefs - Supplies the resource names of the model files to load.

	GR2ResRef - Supplies the resource name of the skeleton file to load.

Return Value:

	The newly constructed object.  The routine raises an std::exception on
	failure, such as a failure to load or parse the model in question.

Environment:

	User mode.

--*/
: m_ResMan( ResMan ),
  m_TextWriter( TextWriter ),
  m_Facing( PI / 2) // Straight north
{
	memcpy(
		&m_WorldTrans,
		&NWN::Matrix44::IDENTITY,
		sizeof( m_WorldTrans ) );

	m_Up.x      = 0.0f;
	m_Up.y      = 0.0f;
	m_Up.z      = 1.0f;
	m_Heading.x = 0.0f;
	m_Heading.y = 1.0f;
	m_Heading.z = 0.0f;
	m_Scale.x   = 1.0f;
	m_Scale.y   = 1.0f;
	m_Scale.z   = 1.0f;

	//
	// Now load resources.
	//

	m_Skeleton = LoadSkeleton( m_ResMan.ResRef32FromStr( GR2ResRef ) );

	m_ModelParts.reserve( MDBResRefs.size( ) );

	for (std::vector< std::string >::const_iterator it = MDBResRefs.begin( );
	     it != MDBResRefs.end( );
	     ++it)
	{
		ModelColliderPtr Model = LoadModel( m_ResMan.ResRef32FromStr( *it ) );

		m_ModelParts.push_back( Model );
	}
}

WorldObject::~WorldObject(
	)
/*++

Routine Description:

	This routine cleans up a deleting WorldObject.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
}

bool
WorldObject::IntersectRay(
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
	const ModelCollider * Collider;

	//
	// The first model/collider always holds collision mesh data.
	//

	if ((Collider = GetModel( )) == NULL)
		return false;

	return Collider->IntersectRay(
		Origin,
		NormDir,
		IntersectNormal,
		IntersectDistance );
}

NWN::Vector3
WorldObject::LocalToWorld(
	__in const NWN::Vector3 & pt,
	__in NWN::NWN2_BodyPieceSlot Slot /* = NWN::BPS_DEFAULT */
	) const
/*++

Routine Description:

	This routine performs maps a point from local coordinate space to world
	coordinate space.  It would typically be used to retrieve world space
	coordinates for a local model mesh vertex.

	The routine operates relative to a body piece slot, to which a model is
	typically attached.

Arguments:

	pt - Supplies the point to map between coordinate spaces.

	Slot - Supplies the body piece slot from which the coordinate came.

Return Value:

	The routine returns the mapped point.

Environment:

	User mode.

--*/
{
	//
	// We assume that all model parts are transformed into local space and do
	// not need attached bone transformation for simplicity.
	//

	UNREFERENCED_PARAMETER( Slot );

	return Math::Multiply( m_WorldTrans, pt );
}

void
WorldObject::UpdateWorldTransformForHeading(
	__in const NWN::Vector3 & Heading,
	__in const NWN::Vector3 & Up,
	__in const NWN::Vector3 & Scale
	)
/*++

Routine Description:

	This routine updates the world transformation remap the object's rotation
	for a new heading.

Arguments:

	Heading - Supplies the new object heading.

	Up - Supplies the world up vector.

	Scale - Supplies the object scale vector.

Return Value:

	None.

Environment:

	User mode, heading already updated and world transformation not yet
	updated.

--*/
{
	NWN::Vector3  Cross;
	NWN::Vector3  NHeading;
	NWN::Matrix44 M1;
//	NWN::Matrix44 M2;
//	NWN::Vector3  V1;
//	NWN::Vector3  V2;

	Cross    = Math::CrossProduct( Heading, Up );
	NHeading = Math::CrossProduct( Up, Cross );

	Math::CreateScaleMatrix( M1, Scale );

	m_WorldTrans._00 = Cross.x;
	m_WorldTrans._01 = Cross.y;
	m_WorldTrans._02 = Cross.z;
	m_WorldTrans._10 = NHeading.x;
	m_WorldTrans._11 = NHeading.y;
	m_WorldTrans._12 = NHeading.z;
	m_WorldTrans._20 = Up.x;
	m_WorldTrans._21 = Up.y;
	m_WorldTrans._22 = Up.z;

	m_WorldTrans = Math::Multiply33_33( M1, m_WorldTrans, m_WorldTrans );

	OnUpdateWorldTransform( );

	/*
	m_WorldTrans._00 = Cross.x;
	m_WorldTrans._01 = Cross.y;
	m_WorldTrans._02 = Cross.z;
	m_WorldTrans._10 = NHeading.x;
	m_WorldTrans._11 = NHeading.y;
	m_WorldTrans._12 = NHeading.z;
	m_WorldTrans._20 = m_Up.x;
	m_WorldTrans._21 = m_Up.y;
	m_WorldTrans._22 = m_Up.z;

	memcpy( &M1, m_WorldTrans, sizeof( M1 ) );

	M2._11           = m_Scale.y;
	M2._22           = m_Scale.z;
	V1.x             = M1._20 * 0;
	M2._00           = m_Scale.x;
	V2.y             = M1._10 * 0;
	V2.x             = M1._21 * 0;
	m_WorldTrans._00 = (M1._00 * M2._00) + V1.x + V2.y;
	V2.z             = M1._11 * 0;
	V1.y             = M1._12 * 0;
	V1.z             = M1._22 * 0;
	m_WorldTrans._02 = (M1._02 * M2._00) + V1.z + V1.y;
	m_WorldTrans._01 = (M1._01 * M2._00) + V2.x + V2.z;
	m_WorldTrans._10 = (M1._10 * M2._11) + (M1._00 * 0) + V1.x;
	V1.x             = (M1._01 * 0);
	m_WorldTrans._11 = (M1._11 * M2._11) + (M1._01 * 0) + V2.x;
	m_WorldTrans._12 = (M1._12 * M2._11) + (M1._02 * 0) + V1.z;
	m_WorldTrans._20 = (M1._20 * M2._22) + (M1._00 * 0) + V2.y;
	m_WorldTrans._21 = (M1._21 * M2._22) + V1.x + V2.z;
	m_WorldTrans._22 = (M1._22 * M1._01) + (M1._02 * 0) + V1.y;

	*/
}

void
WorldObject::UpdateWorldTransformForRotation(
	__in const NWN::Quaternion & Rotation
	)
/*++

Routine Description:

	This routine updates the world transformation remap the object's rotation
	for a new rotational quaternion.

Arguments:

	Rotation - Supplies the new rotational parameters.

Return Value:

	None.

Environment:

	User mode, world transformation not yet updated.

--*/
{
	Math::SetRotationMatrix( m_WorldTrans, Rotation );

	OnUpdateWorldTransform( );
}

void
WorldObject::UpdateWorldTransformForScale(
	__in const NWN::Vector3 & Scale,
	__in const NWN::Vector3 & PrevScale
	)
/*++

Routine Description:

	This routine updates the world transformation remap the object's scale for
	for a new scale factor.

Arguments:

	Scale - Supplies the new object scale.

	PrevScale - Supplies the previous object scale.

Return Value:

	None.

Environment:

	User mode, scale not yet updated and world transformation not yet
	updated.

--*/
{
	NWN::Matrix44 M1;
	NWN::Vector3  RelScale;

	RelScale.x = Scale.x / PrevScale.x;
	RelScale.y = Scale.y / PrevScale.y;
	RelScale.z = Scale.z / PrevScale.z;

	Math::CreateScaleMatrix( M1, RelScale );
	m_WorldTrans = Math::Multiply33_44( M1, m_WorldTrans );

	OnUpdateWorldTransform( );
}

void
WorldObject::OnUpdateWorldTransform(
	)
/*++

Routine Description:

	This routine is invoked when the world transformation for the object is
	altered.  It fires any events depending on the world transformation state,
	such as collider updating.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode, transformation matrix already updated.

--*/
{
}

WorldObject::ModelColliderPtr
WorldObject::LoadModel(
	__in const NWN::ResRef32 & ResRef
	)
/*++

Routine Description:

	This routine loads an MDB file from disk and parses the contents out.  The
	MDB file is returned in the form of a ModelCollider object.

Arguments:

	ResRef - Supplies the resource reference identifying the name of the
	         resource to load.

Return Value:

	The routine returns a ModelCollider instance pointer (owned by the caller)
	on success.  On failure, an std::exception is raised.

Environment:

	User mode, game state lock held.

--*/
{
	TrxFileReader::Ptr MdbObject;
	ModelColliderPtr   Model;
	DemandResource32   Res( m_ResMan, ResRef, NWN::ResMDB );

	MdbObject = new TrxFileReader(
		m_ResMan.GetMeshManager( ),
		Res,
		false,
		TrxFileReader::ModeMDB,
		m_TextWriter);

	Model = new ModelCollider( MdbObject->GetCollider( ) );

	return Model;
}

WorldObject::ModelSkeletonPtr
WorldObject::LoadSkeleton(
	__in const NWN::ResRef32 & ResRef
	)
/*++

Routine Description:

	This routine loads a GR2 file from disk and parses the contents out.  The
	first skeleton in the file is then loaded and returned to the caller.

Arguments:

	ResRef - Supplies the resource reference identifying the name of the
	         resource to load.

Return Value:

	The routine returns a ModelSkeleton instance pointer (owned by the caller)
	on success.  On failure, an std::exception is raised.

Environment:

	User mode, game state lock held.

--*/
{
	Gr2FileReader::Ptr Gr2Object;
	DemandResource32   Res( m_ResMan, ResRef, NWN::ResGR2 );

	Gr2Object = m_ResMan.OpenGr2File( Res );

	return Gr2Object->LoadModelSkeleton( );
}
