/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ModelSkeleton.h

Abstract:

	This module defines the ModelSkeleton class, which represents the skeleton
	for an model, including bones and associated transformations.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_MODELSKELETON_H
#define _PROGRAMS_NWN2DATALIB_MODELSKELETON_H

#ifdef _MSC_VER
#pragma once
#endif

//
// Define the skeleton core.
//

class ModelSkeleton
{

public:

	typedef swutil::SharedPtr< ModelSkeleton > Ptr;

	typedef unsigned short BoneIndex;

	static const BoneIndex BoneIndexInvalid = (BoneIndex) -1;

	enum BoneClass
	{
		BoneClassNone,
		BoneClassFacial,
		BoneClassRender,

		LastBoneClass
	};

	struct BoneTransform
	{
		NWN::Vector3    Origin;
		NWN::Quaternion Rotation;
		NWN::Matrix44   ScaleShear;
	};

	struct Bone
	{
		std::string   Name;
		BoneIndex     ParentIndex;
		BoneTransform Transform;
		NWN::Matrix44 InvWorldTransform; // Scaled to model space
		NWN::Matrix44 WorldTransform; // Unscaled from model space
		BoneClass     Class;
	};

	enum AttachmentPoint
	{
		AttachNone         = 0,
		AttachHandRight    = 1,
		AttachHandLeft     = 2,
		AttachShield       = 3,
		AttachMount        = 4,
		AttachMountLeft    = 5,
		AttachMountRight   = 6,
		AttachFootRight    = 7,
		AttachFootLeft     = 8,
		AttachKneeRight    = 9,
		AttachKneeLeft     = 10,
		AttachPelvis       = 11,
		AttachTorso        = 12,
		AttachBack         = 13,
		AttachMouth        = 14,
		AttachForehead     = 15,
		AttachEyeLeft      = 16,
		AttachEyeRight     = 17,
		AttachEarLeft      = 18,
		AttachEarRight     = 19,
		AttachCollarLeft   = 20,
		AttachCollarRight  = 21,
		AttachArmLeft      = 22,
		AttachArmRight     = 23,
		AttachBracerLeft   = 24,
		AttachBracerRight  = 25,
		AttachCamera       = 26,
		AttachHalo         = 27,

		LastAttach         = 28
	};

	enum SpecialBone
	{
		SpecialBoneHead          = 0,
		SpecialBoneHalo,
		SpecialBoneRealHead,
		SpecialBoneRealCamera,
		SpecialBoneJaw,
		SpecialBoneNeck,
		SpecialBoneSpine,
		SpecialBoneRightArm,
		SpecialBoneEmotion,
		SpecialBoneTalk,
		SpecialBoneRoot,

		LastSpecialBone
	};

	typedef std::vector< Bone > BoneVec;

	//
	// Constructor and destructor.
	//

	ModelSkeleton(
		__in const std::string & SkeletonName
		);

	virtual
	~ModelSkeleton(
		);

	//
	// Global skeleton transform access.
	//

	inline
	const NWN::Matrix44 &
	GetWorldTransform(
		) const
	{
		return m_WorldTrans;
	}

	inline
	NWN::Matrix44 &
	GetWorldTransform(
		)
	{
		return m_WorldTrans;
	}

	//
	// Bone creation.
	//

	void
	AddBone(
		__in const Bone & NewBone
		);

	inline
	BoneIndex
	GetMaxBoneIndex(
		) const
	{
		return (BoneIndex) m_Bones.size( );
	}

	//
	// Bone access.
	//

	inline
	const BoneVec &
	GetBones(
		) const
	{
		return m_Bones;
	}

	inline
	BoneVec &
	GetBones(
		)
	{
		return m_Bones;
	}

	inline
	const Bone *
	GetBone(
		__in BoneIndex Index
		) const
	{
		if (Index >= m_Bones.size( ))
			return NULL;

		return &m_Bones[ Index ];
	}

	inline
	Bone *
	GetBone(
		__in BoneIndex Index
		)
	{
		if (Index >= m_Bones.size( ))
			return NULL;

		return &m_Bones[ Index ];
	}

	inline
	const Bone *
	GetBone(
		__in const char * BoneName
		) const
	{
		for (BoneVec::const_iterator it = m_Bones.begin( );
		     it != m_Bones.end( );
		     ++it)
		{
			if (!_stricmp( BoneName, it->Name.c_str( )))
				return &*it;
		}

		return NULL;
	}

	inline
	Bone *
	GetBone(
		__in const char * BoneName
		)
	{
		for (BoneVec::iterator it = m_Bones.begin( );
		     it != m_Bones.end( );
		     ++it)
		{
			if (!_stricmp( BoneName, it->Name.c_str( )))
				return &*it;
		}

		return NULL;
	}

	//
	// Bone access by attachment point.
	//

	inline
	const Bone *
	GetAttachmentBone(
		__in AttachmentPoint Attach
		) const
	{
		return GetBone( m_AttachBoneIndicies[ Attach ] );
	}

	inline
	Bone *
	GetAttachmentBone(
		__in AttachmentPoint Attach
		)
	{
		return GetBone( m_AttachBoneIndicies[ Attach ] );
	}

	//
	// Bone access by special bone.
	//

	inline
	const Bone *
	GetSpecialBone(
		__in SpecialBone Special
		) const
	{
		return GetBone( m_SpecialBoneIndicies[ Special ] );
	}

	inline
	Bone *
	GetSpecialBone(
		__in SpecialBone Special
		)
	{
		return GetBone( m_SpecialBoneIndicies[ Special ] );
	}

	//
	// Query the index of a bone by its bone descriptor.
	//

	inline
	BoneIndex
	GetBoneIndex(
		__in const Bone * B
		) const
	{
		return (BoneIndex) (B - &m_Bones[ 0 ]);
	}

	//
	// Skeleton name access.
	//

	inline
	const std::string &
	GetSkeletonName(
		) const
	{
		return m_SkeletonName;
	}

	//
	// Skeleton bone transform access.
	//

	NWN::Matrix44
	GetBoneLocalTransform(
		__in BoneIndex Index
		) const;

	const NWN::Matrix44 &
	GetBoneWorldTransform(
		__in BoneIndex Index
		) const;

	const NWN::Matrix44 &
	GetBoneInvWorldTransform(
		__in BoneIndex Index
		) const;

	//
	// Validation.
	//

	inline
	void
	Validate(
		) const
	{
		for (BoneVec::const_iterator it = m_Bones.begin( );
		     it != m_Bones.end( );
		     ++it)
		{
			if (it->ParentIndex == BoneIndexInvalid)
				continue;

			if (it->ParentIndex >= m_Bones.size( ))
				throw std::runtime_error( "Illegal ParentIndex" );
		}
	}

	//
	// Model naming.
	//

	static
	const char *
	GetAccessoryName(
		__in NWN::NWN2_Accessory Accessory
		);

	static
	NWN::NWN2_BodyPieceSlot
	GetAccessoryBodyPieceSlot(
		__in NWN::NWN2_Accessory Accessory
		);

	static
	const char *
	GetAccessoryAttachBoneName(
		__in NWN::NWN2_Accessory Accessory
		);

	static
	const char *
	GetAnimationStanceName(
		__in NWN::NWN2_AnimationStance Stance
		);

	static
	const char *
	GetArmorPieceSlotName(
		__in NWN::NWN2_ArmorPieceSlot Slot
		);

	static
	NWN::NWN2_BodyPieceSlot
	GetArmorPieceBodyPieceSlot(
		__in NWN::NWN2_ArmorPieceSlot Slot
		);

private:

	typedef std::vector< BoneIndex > BoneIndexVec;

	//
	// These routines register the presence of specially marked bones so that
	// they can be quicly referenced at a later time.
	//

	void
	RegisterAttachmentBone(
		__in const Bone & B,
		__in BoneIndex Index,
		__in AttachmentPoint Attach
		);

	void
	RegisterSpecialBone(
		__in const Bone & B,
		__in BoneIndex Index,
		__in SpecialBone Special
		);

	//
	// This routine calculates the current world transform for a bone, without
	// relying on the cached transform.
	//

	NWN::Matrix44
	CalcBoneWorldTransform(
		__in BoneIndex Index
		) const;

	std::string   m_SkeletonName;
	BoneVec       m_Bones;
	BoneIndex     m_AttachBoneIndicies[ LastAttach ];
	BoneIndex     m_SpecialBoneIndicies[ LastSpecialBone ];
	NWN::Matrix44 m_WorldTrans;

};

#endif
