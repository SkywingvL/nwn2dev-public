/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	Gr2FileReader.cpp

Abstract:

	This module houses the *.gr2 file format parser, which is used to read
	granny2 skeleton and animation data.

	The actual granny2 parser core is presently contained in the Granny2Lib
	library; this module simply abstracts that interface and hides Granny2Lib
	from the rest of the resource management system.

--*/

#include "Precomp.h"
#include "../NWNBaseLib/NWNBaseLib.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#include "../Granny2Lib/Granny2Lib.h"
#include "Gr2FileReader.h"
#include "ModelSkeleton.h"

Gr2FileReader::Gr2FileReader(
	__in Gr2Accessor * Accessor,
	__in const std::string & FileName,
	__in bool PreferOutOfProcess
	)
/*++

Routine Description:

	This routine constructs a new Gr2FileReader object and parses the contents
	of a GR2 file by filename.  The file must already exist as it is
	immediately deserialized.

Arguments:

	Accessor - Supplies the granny2 accessor context that is to be used to load
	           the GR2 file.

	FileName - Supplies the path to the GR2 file.

	PreferOutOfProcess - Supplies a Boolean value that indicates true if the
	                     requestor prefers to make out of process calls to
	                     Granny2.dll to insulate themselves against bugs within
	                     Granny2 core logic.

Return Value:

	The newly constructed object.

Environment:

	User mode.

--*/
: m_GR2Info( NULL )
{
	//
	// Simply pass the request to deserialize the GR2 file on to the accessor
	// context, and hold on to the deserialized file contents.
	//

	m_GR2Info = Accessor->LoadGranny2File( FileName, PreferOutOfProcess );
}

Gr2FileReader::~Gr2FileReader(
	)
/*++

Routine Description:

	This routine cleans up an already-existing Gr2FileReader object.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
}

ModelSkeleton *
Gr2FileReader::LoadModelSkeleton(
	)
/*++

Routine Description:

	This routine creates a skeleton object from the bone data stored in the GR2
	file and returns it to the caller.

Arguments:

	None.

Return Value:

	The routine returns a pointer to a new ModelSkeleton object on success.
	The returned object is then owned by the caller, who assumes responsibility
	for releasing the object via operator delete.

	On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	ModelSkeleton * Skeleton;

	Skeleton = NULL;

	try
	{
		NWN::Matrix44                     M;
		NWN::Vector3                      Scale;
		granny2::granny2_impl::Skeleton * GrannySkel;
		float                             s;

		//
		// We expect just one skeleton per file.
		//

		if (m_GR2Info->Skeletons.size( ) < 1)
			throw std::runtime_error( "Wrong skeleton count." );

		GrannySkel = &*m_GR2Info->Skeletons[ 0 ];

		//
		// Create the skeleton context and reserve storage for all bones.
		//

		Skeleton = new ModelSkeleton( GrannySkel->Name.c_str( ) );

		Skeleton->GetBones( ).reserve(
			(size_t) GrannySkel->Bones.size( ) );

		Math::CreateTranslationMatrix(
			Skeleton->GetWorldTransform( ),
			m_GR2Info->artInfo->Origin );

		s = 1.0f / m_GR2Info->artInfo->UnitsPerMeter;

		Scale.x = s;
		Scale.y = s;
		Scale.z = s;

		Math::CreateScaleMatrix( M, Scale );
		Skeleton->GetWorldTransform( ) = Math::Multiply_Double(
			Skeleton->GetWorldTransform( ),
			M );

		//
		// Copy bones from the granny2 data into our in-memory format.
		//

		typedef std::vector< granny2::granny2_impl::Bone > GrannyBoneVec;

		for (GrannyBoneVec::const_iterator it = GrannySkel->Bones.begin( );
		     it != GrannySkel->Bones.end( );
		     ++it)
		{
			NWN::Matrix33       M33;
			NWN::Matrix44       M44;
			ModelSkeleton::Bone Bone;

			Bone.Name                 = it->Name.c_str( );
			Bone.ParentIndex          = (ModelSkeleton::BoneIndex) it->ParentIndex;
			Bone.Transform.Origin     = it->Transform.Origin;
			Bone.Transform.Rotation   = it->Transform.Rotation;
			memcpy( &M33, &it->Transform.Scale, sizeof( M33 ) );
			Bone.Transform.ScaleShear = Math::CreateMatrix44FromMatrix33( M33 );
			memcpy( &M44, &it->InverseWorldTransform, sizeof( M44 ) );
			Bone.InvWorldTransform    = M44;

			Bone.Class                = ModelSkeleton::BoneClassNone;

			Skeleton->AddBone( Bone );

			C_ASSERT( sizeof( M33 ) == sizeof( granny2::RHMatrix3 ) );
			C_ASSERT( sizeof( M44 ) == sizeof( granny2::RHMatrix4 ) );
		}

		//
		// Validate all bones.
		//

		Skeleton->Validate( );
	}
	catch (...)
	{
		if (Skeleton != NULL)
			delete Skeleton;

		throw;
	}

	return Skeleton;
}

Gr2Accessor::Gr2Accessor(
	__in const std::string & Granny2Dll,
	__in const std::string & TempPath
	)
/*++

Routine Description:

	This routine constructs a new Gr2Accessor object, which may be used by any
	number of Gr2Reader objects to parse *.gr2 files.

Arguments:

	Granny2Dll - Supplies the path to granny2.dll.

	TempPath - Supplies the temporary path within which the Gr2Accessor should
	           store any raw temporary files that may be created during *.gr2
	           file loading.  While the Gr2Accessor will always clean up its
	           temporary files, storing them in a known location allows some
	           resiliency against an unclean shutdown when a temporary file
	           still existed.

Return Value:

	None.

Environment:

	User mode.

--*/
: m_Granny2Reader( new granny2::Granny2Reader( Granny2Dll ) ),
  m_TempPath( TempPath ),
  m_Unique( 1 )
{
}

Gr2Accessor::~Gr2Accessor(
	)
/*++

Routine Description:

	This routine cleans up an already-existing Gr2Accessor object.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
}

Gr2Accessor::GR2InfoPtr
Gr2Accessor::LoadGranny2File(
	__in const std::string & FileName,
	__in bool PreferOutOfProcess
	)
/*++

Routine Description:

	This routine loads a .gr2 file into the in-memory format, and returns the
	directly-accessible root GR2Info structure.

Arguments:

	FileName - Supplies the file name of the on-disk *.gr2 file to load.

	PreferOutOfProcess - Supplies a Boolean value that indicates true if the
	                     requestor prefers to make out of process calls to
	                     Granny2.dll to insulate themselves against bugs within
	                     Granny2 core logic.

Return Value:

	The routine returns a GR2InfoPtr on success.  On failure, an std::exception
	is raised.

Environment:

	User mode.

--*/
{
	char                    TempFile[ MAX_PATH + 1 ];
	Gr2Accessor::GR2InfoPtr GR2Info;

	if (!GetTempFileNameA( m_TempPath.c_str( ), "G2R", m_Unique++, TempFile ))
		throw std::runtime_error( "GetTempFileNameA failed." );

	GR2Info = m_Granny2Reader->LoadGranny2File(
		FileName,
		TempFile,
		PreferOutOfProcess );

	DeleteFileA( TempFile );

	return GR2Info;
}
