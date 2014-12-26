/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ModelSkeleton.cpp

Abstract:

	This module houses the ModelSkeleton class implementation, including bone
	management routines.

--*/

#include "Precomp.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#include "ModelSkeleton.h"

//
// Define to fail bone addition for unrecoginized attachment points.
//

#define FAIL_UNKNOWN_ATTACHMENT_POINTS 0

//
// Define to disallow registering duplicate bones.  Some creature models, such
// as the cat model, require allowing duplicate bones.
//

#define FAIL_DUPLICATE_BONES 0

//
// Define to use a recursive search for bone access, which does not require
// heap allocations but may blow the stack on large numbers of bones.
//

#define RECURSIVE_BONES 0

ModelSkeleton::ModelSkeleton(
	__in const std::string & SkeletonName
	)
/*++

Routine Description:

	This routine constructs a new ModelSkeleton object and initializes it to an
	empty state.

Arguments:

	SkeletonName - Supplies the name of the skeleton, which also servces as the
	               name of the root bone.

Return Value:

	The newly constructed object.

Environment:

	User mode.

--*/
: m_SkeletonName( SkeletonName ),
  m_WorldTrans( NWN::Matrix44::IDENTITY )
{
	for (size_t i = 0; i < LastAttach; i += 1)
		m_AttachBoneIndicies[ i ] = BoneIndexInvalid;
	for (size_t i = 0; i < LastSpecialBone; i += 1)
		m_SpecialBoneIndicies[ i ] = BoneIndexInvalid;
}

ModelSkeleton::~ModelSkeleton(
	)
/*++

Routine Description:

	This routine cleans up an already-existing ModelSkeleton object.

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
ModelSkeleton::AddBone(
	__in const Bone & NewBone
	)
/*++

Routine Description:

	This routine creates a new bone on the skeleton.  It initializes the bone
	in the bone array, and links up any special or attachment point indicies.

Arguments:

	NewBone - Supplies the new bone descriptor used as a template to create a
	          new bone.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	if (((BoneIndex) (m_Bones.size( ) + 1) == BoneIndexInvalid) ||
	    ((BoneIndex) (m_Bones.size( ) + 1) < 0))
	{
		throw std::runtime_error( "Too many bones." );
	}

	m_Bones.push_back( NewBone );

	try
	{
		BoneIndex    Index;
		BoneIndex    RibcageBoneIndex;
		Bone       & B = m_Bones.back( );

		Index = (BoneIndex) (&B - &m_Bones[ 0 ] );

		B.Class = BoneClassNone;

		RibcageBoneIndex = 0;

		B.WorldTransform = Math::Multiply_Double(
			Math::Inverse_Double( B.InvWorldTransform ),
			m_WorldTrans );

		for (std::string::iterator it = B.Name.begin( );
		     it != B.Name.end( );
		     ++it)
		{
			*it = (char) tolower( (int) (unsigned char) *it );
		}

		//
		// Check if it's an attachment point bone...
		//

		if (!strncmp( B.Name.c_str( ), "ap_", 3 ))
		{
			bool                Matched;
			static const char * AttachPointNames[ LastAttach + 1 ] =
			{
				"ap_invalid",
				"ap_hand_right",
				"ap_hand_left",
				"ap_shield_left",
				"ap_mount",
				"ap_mount_left",
				"ap_mount_right",
				"ap_foot_right",
				"ap_foot_left",
				"ap_knee_right",
				"ap_knee_left",
				"ap_pelvis",
				"ap_torso",
				"ap_back",
				"ap_mouth",
				"ap_forehead",
				"ap_eye_left",
				"ap_eye_right",
				"ap_ear_left",
				"ap_ear_right",
				"LArm0CollarBone", // ??? will never match
				"RArm0CollarBone", // ??? will never match
				"ap_arm_l",
				"ap_arm_r",
				"ap_bracer_l",
				"ap_bracer_r",
				"ap_camera",
				"ap_halo",
				"ap_max"
			};

			Matched = false;

			for (size_t i = 0; i < LastAttach; i += 1)
			{
				if (B.Name == AttachPointNames[ i ])
				{
					RegisterAttachmentBone( B, Index, (AttachmentPoint) i );

					Matched = true;
					break;
				}
			}

			if (!Matched)
			{
#if FAIL_UNKNOWN_ATTACHMENT_POINTS
				try
				{
					std::string Err;

					Err  = "Unknown attachment point bone '";
					Err += B.Name;
					Err += "'.";

					throw std::runtime_error( Err );
				}
				catch (std::bad_alloc)
				{
					throw std::runtime_error( "Unknown attachment point bone." );
				}
#endif
			}
		}
		else if (!strncmp( B.Name.c_str( ), "f_", 2 ))
		{
			B.Class = BoneClassFacial;
		}
		else if (!strncmp( B.Name.c_str( ), "ribcage", 7 ))
		{
			B.Class = BoneClassRender;
		}
		else
		{
			B.Class = BoneClassRender;
		}

		if (B.Name == "ap_camera")
		{
			RegisterSpecialBone( B, Index, SpecialBoneRealCamera );
			RegisterSpecialBone( B, Index, SpecialBoneHead );
		}
		else if ((m_SpecialBoneIndicies[ SpecialBoneHead ] == BoneIndexInvalid) &&
		         (strstr( B.Name.c_str( ), "head" ) != NULL))
		{
			RegisterSpecialBone( B, Index, SpecialBoneHead );
			RegisterSpecialBone( B, Index, SpecialBoneRealHead );
		}
		else if ((!m_SkeletonName.empty( )) &&
		         (!_stricmp( m_SkeletonName.c_str( ), B.Name.c_str( ) )))
		{
			RegisterSpecialBone( B, Index, SpecialBoneRoot );
		}
		else if (!strncmp( B.Name.c_str( ), "ap_halo", 7 ))
		{
			RegisterSpecialBone( B, Index, SpecialBoneHalo );
		}
		else if (!strncmp( B.Name.c_str( ), "s_jaw", 5 ))
		{
			RegisterSpecialBone( B, Index, SpecialBoneJaw );
		}
		else if (!strncmp( B.Name.c_str( ), "neck", 4 ))
		{
			RegisterSpecialBone( B, Index, SpecialBoneNeck );
		}
		else if (!strncmp( B.Name.c_str( ), "spine", 5 ))
		{
			RegisterSpecialBone( B, Index, SpecialBoneSpine );
		}
		else if (!strncmp( B.Name.c_str( ), "rarm1collarbone", 16 )) // Uhhh??
		{
			RegisterSpecialBone( B, Index, SpecialBoneRightArm );
		}
		else if (!strncmp( B.Name.c_str( ), "f_emotiondummy", 15 )) // Uhhh??
		{
			RegisterSpecialBone( B, Index, SpecialBoneEmotion );
		}
		else if (!strncmp( B.Name.c_str( ), "f_talkdummy", 12 )) // Uhhh??
		{
			RegisterSpecialBone( B, Index, SpecialBoneTalk );
		}
	}
	catch (...)
	{
		//
		// Remove the bone we just added and toss the exception back up.
		//

		m_Bones.pop_back( );
		throw;
	}
}

NWN::Matrix44
ModelSkeleton::GetBoneLocalTransform(
	__in BoneIndex Index
	) const
/*++

Routine Description:

	This routine creates a transformation matrix representing the local
	transform for a bone.

Arguments:

	Index - Supplies the bone index to create the transformation for.

Return Value:

	The routine returns a matrix representing the requested transform.

Environment:

	User mode.

--*/
{
	NWN::Matrix44 M;
	NWN::Matrix44 ScaleM;
	NWN::Vector3  ScaleV;

	Math::CreateIdentityMatrix( M );
	Math::SetRotationMatrix(
		M,
		Math::Inverse( m_Bones[ Index ].Transform.Rotation ) );
	Math::SetPosition( M, m_Bones[ Index ].Transform.Origin );

	ScaleV.x = m_Bones[ Index ].Transform.ScaleShear._00;
	ScaleV.y = m_Bones[ Index ].Transform.ScaleShear._11;
	ScaleV.z = m_Bones[ Index ].Transform.ScaleShear._22;

	Math::CreateIdentityMatrix( ScaleM );
	Math::CreateScaleMatrix( ScaleM, ScaleV );

	return Math::Multiply( M, ScaleM );
}

const NWN::Matrix44 &
ModelSkeleton::GetBoneWorldTransform(
	__in BoneIndex Index
	) const
/*++

Routine Description:

	This routine creates a transformation matrix representing the world
	transform for a bone.

Arguments:

	Index - Supplies the bone index to create the transformation for.

Return Value:

	The routine returns a matrix representing the requested transform.

Environment:

	User mode.

--*/
{
	return m_Bones[ Index ].WorldTransform;
}

const NWN::Matrix44 &
ModelSkeleton::GetBoneInvWorldTransform(
	__in BoneIndex Index
	) const
/*++

Routine Description:

	This routine creates a transformation matrix representing the
	inverse world transform for a bone.

Arguments:

	Index - Supplies the bone index to create the transformation for.

Return Value:

	The routine returns a matrix representing the requested transform.

Environment:

	User mode.

--*/
{
	return m_Bones[ Index ].InvWorldTransform;
}

void
ModelSkeleton::RegisterAttachmentBone(
	__in const Bone & B,
	__in BoneIndex Index,
	__in AttachmentPoint Attach
	)
/*++

Routine Description:

	This routine registers a bone as an attachment point bone for a particular
	attachment slot.

Arguments:

	B - Supplies the bone to register.

	Index - Supplies the index of the bone to regsiter.

	Attach - Supplies the attachment point to register the bone as.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	//
	// Check for duplicates.
	//

#if FAIL_DUPLICATE_BONES
	if (m_AttachBoneIndicies[ Attach ] != BoneIndexInvalid)
	{
		try
		{
			std::string Err;

			Err  = "Attempted to register duplicate attachment point bone '";
			Err += B.Name;
			Err += "'.";

			throw std::runtime_error( Err );
		}
		catch (std::bad_alloc)
		{
			throw std::runtime_error(
				"Attempted to register duplicate attachment point bone." );
		}
	}
#else
	UNREFERENCED_PARAMETER( B );
#endif

	m_AttachBoneIndicies[ Attach ] = Index;
}

void
ModelSkeleton::RegisterSpecialBone(
	__in const Bone & B,
	__in BoneIndex Index,
	__in SpecialBone Special
	)
/*++

Routine Description:

	This routine registers a bone as a special point bone for a particular
	special bone slot.

Arguments:

	B - Supplies the bone to register.

	Index - Supplies the index of the bone to regsiter.

	Special - Supplies the special bone slot to register the bone as.

Return Value:

	None.  The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	//
	// Check for duplicates.
	//

#if FAIL_DUPLICATE_BONES
	if ((m_SpecialBoneIndicies[ Special ] != BoneIndexInvalid) &&
	   (Special != SpecialBoneHead))
	{
		try
		{
			std::string Err;

			Err  = "Attempted to register duplicate special bone '";
			Err += B.Name;
			Err += "'.";

			throw std::runtime_error( Err );
		}
		catch (std::bad_alloc)
		{
			throw std::runtime_error(
				"Attempted to register duplicate special bone." );
		}
	}
#else
	UNREFERENCED_PARAMETER( B );
#endif

	m_SpecialBoneIndicies[ Special ] = Index;
}


NWN::Matrix44
ModelSkeleton::CalcBoneWorldTransform(
	__in BoneIndex Index
	) const
/*++

Routine Description:

	This routine creates a transformation matrix representing the world
	transform for a bone.  The transformation matrix is calculated given the
	rotation parameters of the bone instead of using the precomputed values.

	N.B.  This routine will typically produce inaccurate results in SSE2 mode
	      due to the cumulative precision loss of numerous matrix multiply
		  operations.

Arguments:

	Index - Supplies the bone index to create the transformation for.

Return Value:

	The routine returns a matrix representing the requested transform.

Environment:

	User mode.

--*/
{
#if RECURSIVE_BONES
	BoneIndex ParentIndex = m_Bones[ Index ].ParentIndex;

	if (ParentIndex != BoneIndexInvalid)
	{
		return Math::Multiply(
			GetBoneLocalTransform( Index ),
			GetBoneWorldTransform( ParentIndex ) );
	}
	else
	{
		return Math::Multiply( GetBoneLocalTransform( Index ), m_WorldTrans );
	}
#else
	NWN::Matrix44 M;
	BoneIndexVec  BoneIndicies;

	M = Math::Multiply( GetBoneLocalTransform( Index ), m_WorldTrans );

	for (; Index != BoneIndexInvalid; Index = m_Bones[ Index ].ParentIndex)
	{
		try
		{
			BoneIndicies.push_back( Index );
		}
		catch (std::exception)
		{
			return NWN::Matrix44::IDENTITY;
		}
	}

	for (BoneIndexVec::reverse_iterator it = BoneIndicies.rbegin( );
	     it != BoneIndicies.rend( );
	     ++it)
	{
		if (it == BoneIndicies.rbegin( ))
			M = Math::Multiply( GetBoneLocalTransform( *it ), m_WorldTrans );
		else
			M = Math::Multiply( GetBoneLocalTransform( *it ), M );
	}

	return M;
#endif
}

const char *
ModelSkeleton::GetAccessoryName(
	__in NWN::NWN2_Accessory Accessory
	)
/*++

Routine Description:

	This routine determines the model name of an accessory.

Arguments:

	Accessory - Supplies the accessory whose name should be retrieved.

Return Value:

	The routine returns the accessory name, else NULL on an illegal accessory.

Environment:

	User mode.

--*/
{
	static const char * AccessoryNames[ NWN::Num_Accessories ] =
	{
		"LShoulder",
		"RShoulder",
		"LBracer",
		"RBracer",
		"LElbow",
		"RElbow",
		"LUpArm",
		"RUpArm",
		"LHip",
		"RHip",
		"FHip",
		"BHip",
		"LUpLeg",
		"RUpLeg",
		"LLowLeg",
		"RLowLeg",
		"LKnee",
		"RKnee",
		"LFoot",
		"RFoot",
		"LAnkle",
		"RAnkle"
	};

	if ((Accessory < 0) || (Accessory >= NWN::Num_Accessories))
		return NULL;

	return AccessoryNames[ Accessory ];
}

NWN::NWN2_BodyPieceSlot
ModelSkeleton::GetAccessoryBodyPieceSlot(
	__in NWN::NWN2_Accessory Accessory
	)
/*++

Routine Description:

	This routine determines the body placement slot for an accessory.

Arguments:

	Accessory - Supplies the accessory whose slot should be retrieved.

Return Value:

	The routine returns the slot id, else NWN::BPS_MAX on an illegal accessory.

Environment:

	User mode.

--*/
{
	static const NWN::NWN2_BodyPieceSlot AccessorySlotIds[ NWN::Num_Accessories ] =
	{
		NWN::BPS_LEFT_SHOULDER_ACCESSORY,
		NWN::BPS_RIGHT_SHOULDER_ACCESSORY,
		NWN::BPS_LEFT_BRACER_ACCESSORY,
		NWN::BPS_RIGHT_BRACER_ACCESSORY,
		NWN::BPS_LEFT_ELBOW_ACCESSORY,
		NWN::BPS_RIGHT_ELBOW_ACCESSORY,
		NWN::BPS_LEFT_ARM_ACCESSORY,
		NWN::BPS_RIGHT_ARM_ACCESSORY,
		NWN::BPS_LEFT_HIP_ACCESSORY,
		NWN::BPS_RIGHT_HIP_ACCESSORY,
		NWN::BPS_FRONT_HIP_ACCESSORY,
		NWN::BPS_BACK_HIP_ACCESSORY,
		NWN::BPS_LEFT_LEG_ACCESSORY,
		NWN::BPS_RIGHT_LEG_ACCESSORY,
		NWN::BPS_LEFT_SHIN_ACCESSORY,
		NWN::BPS_RIGHT_SHIN_ACCESSORY,
		NWN::BPS_LEFT_KNEE_ACCESSORY,
		NWN::BPS_RIGHT_KNEE_ACCESSORY,
		NWN::BPS_LEFT_FOOT_ACCESSORY,
		NWN::BPS_RIGHT_FOOT_ACCESSORY,
		NWN::BPS_LEFT_ANKLE_ACCESSORY,
		NWN::BPS_RIGHT_ANKLE_ACCESSORY
	};

	if ((Accessory < 0) || (Accessory >= NWN::Num_Accessories))
		return NWN::BPS_MAX;

	return AccessorySlotIds[ Accessory ];
}

const char *
ModelSkeleton::GetAccessoryAttachBoneName(
	__in NWN::NWN2_Accessory Accessory
	)
/*++

Routine Description:

	This routine determines the attachmment bone name of an accessory.

Arguments:

	Accessory - Supplies the accessory whose bone name should be retrieved.

Return Value:

	The routine returns the bone name, else NULL on an illegal accessory.

Environment:

	User mode.

--*/
{
	static const char * AccessoryBoneNames[ NWN::Num_Accessories ] =
	{
		"LArm010",
		"RArm110",
		"LArm02",
		"RArm12",
		"LArm02",
		"RArm12",
		"LArm010",
		"RArm110",
		"LHip1",
		"RHip1",
		"FHip1",
		"BHip1",
		"LLeg1",
		"RLeg1",
		"LLeg2",
		"RLeg2",
		"LLeg2",
		"RLeg2",
		"LLegAnkleDigit011",
		"RLegAnkleDigit011",
		"LLegAnkle",
		"RLegAnkle"
	};

	if ((Accessory < 0) || (Accessory >= NWN::Num_Accessories))
		return NULL;

	return AccessoryBoneNames[ Accessory ];
}

const char *
ModelSkeleton::GetAnimationStanceName(
	__in NWN::NWN2_AnimationStance Stance
	)
/*++

Routine Description:

	This routine determines the name of an animation stance.

Arguments:

	Stance - Supplies the stance whose name should be returned.

Return Value:

	The routine returns the stance name, else NULL on an illegal stance.

Environment:

	User mode.

--*/
{
	static const char * AnimationStanceNames[ NWN::ANIMSTANCE_END ] =
	{
		"Una",
		"1hss",
		"1hs",
		"d2h",
		"bow",
		"c2h",
		"o2ht",
		"o2hs",
		"cbow",
		"thrw",
		"m1hss",
		"m1hls",
		"mbow",
		"muna"
	};

	if ((Stance < 0) || (Stance >= NWN::ANIMSTANCE_END))
		return NULL;

	return AnimationStanceNames[ Stance ];
}

const char *
ModelSkeleton::GetArmorPieceSlotName(
	__in NWN::NWN2_ArmorPieceSlot Slot
	)
/*++

Routine Description:

	This routine determines the name of an armor piece slot.

Arguments:

	Slot - Supplies the armor piece slot whose name should be returned.

Return Value:

	The routine returns the slot name, else NULL on an illegal slot.

Environment:

	User mode.

--*/
{
	static const char * ArmorPieceSlotNames[ NWN::Num_ArmorPieceSlots ] =
	{
		"_Body",
		"_Helm",
		"_Gloves",
		"_Boots",
		"_Belt",
		"_ExtraA",
		"_ExtraB",
		"_ExtraC",
		"_Cloak"
	};

	if ((Slot < 0) || (Slot >= NWN::Num_ArmorPieceSlots))
		return NULL;

	return ArmorPieceSlotNames[ Slot ];
}

NWN::NWN2_BodyPieceSlot
ModelSkeleton::GetArmorPieceBodyPieceSlot(
	__in NWN::NWN2_ArmorPieceSlot Slot
	)
/*++

Routine Description:

	This routine determines the body placement slot for an armor piece.

Arguments:

	Slot - Supplies the armor piece slot whose BPS should be retrieved.

Return Value:

	The routine returns the slot id, else NWN::BPS_MAX on an illegal accessory.

Environment:

	User mode.

--*/
{
	static const NWN::NWN2_BodyPieceSlot ArmorPieceSlotIds[ NWN::Num_ArmorPieceSlots ] =
	{
		NWN::BPS_BODY,
		NWN::BPS_HELM,
		NWN::BPS_GLOVES,
		NWN::BPS_FEET,
		NWN::BPS_BELT,
		NWN::BPS_EXTRA_A,
		NWN::BPS_EXTRA_B,
		NWN::BPS_EXTRA_C,
		NWN::BPS_CLOAK
	};

	if ((Slot < 0) || (Slot >= NWN::Num_ArmorPieceSlots))
		return NWN::BPS_MAX;

	return ArmorPieceSlotIds[ Slot ];
}
