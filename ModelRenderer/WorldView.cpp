/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	WorldView.cpp

Abstract:

	This module houses the WorldView object, which represents a view of the 3D
	world housing displayed WorldObjects.

--*/

#include "Precomp.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#include "../NWN2DataLib/TextOut.h"
#include "../NWN2DataLib/ResourceManager.h"
#include "../NWN2DataLib/TrxFileReader.h"
#include "../NWN2DataLib/ModelSkeleton.h"
#include "Camera.h"
#include "WorldObject.h"
#include "WorldView.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

#define PATH_DEBUG 0

#if PATH_DEBUG
#define PathDebug m_TextWriter->WriteText
#else
#define PathDebug __noop
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4127) // conditional expression is constant
#endif

LONG
WndprocExceptionFilter(
	__in LPEXCEPTION_POINTERS ExceptionPointers
	);

WorldView::WorldView(
	__in ResourceManager & ResMan,
	__in IDebugTextOut * TextWriter,
	__in_opt HWND Parent /* = NULL */
	)
/*++

Routine Description:

	This routine constructs a new WorldView and associated GUI elements.

Arguments:

	ResMan - Supplies the resource manager instance used to load resources.

	TextWriter - Supplies the debug text output writer interface pointer.

	Parent - Optionally supplies the parent window.

Return Value:

	None.  Raises std::exception on failure.

Environment:

	User mode.

--*/
: m_ResMan( ResMan ),
  m_TextWriter( TextWriter ),
  m_WorldWindow( NULL ),
  m_Camera( NULL ),
  m_AreaWidth( 100.0f ),
  m_AreaHeight( 100.0f ),
  m_OriginX( 0.0f ),
  m_OriginY( 0.0f ),
  m_PaddingX( 0.0f ),
  m_PaddingY( 0.0f ),
  m_CursorX( -1 ),
  m_CursorY( -1 )
{
	POINT pt;

	m_CameraRotateDelta.x = 0;
	m_CameraRotateDelta.y = 0;

	//
	// Create the world view window.
	//

	m_WorldWindow = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"WorldViewClass",
		L"World View",
		WS_OVERLAPPEDWINDOW | WS_SIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		500,
		500,
		Parent,
		NULL,
		(HINSTANCE) &__ImageBase,
		this);

	if (m_WorldWindow == NULL)
		throw std::bad_alloc( );

	if (!GetClientRect( m_WorldWindow, &m_ClientRect ))
		throw std::exception( "Couldn't get client rect for world window!" );

	RecalculateMapRect( &m_ClientRect );

	//
	// Cache the current cursor position if we are within the window, so that
	// we have a reasonable initial state for tooltip purposes.
	//

	if (GetCursorPos( &pt ))
	{
		if (ScreenToClient( m_WorldWindow, &pt ))
		{
			m_CursorX = pt.x;
			m_CursorY = pt.y;
		}
	}

	CreateCamera( );
}

WorldView::WorldObjectPtr
WorldView::CreateWorldObject(
	__in const std::vector< std::string > & MDBResRefs,
	__in const std::string & GR2ResRef
	)
/*++

Routine Description:

	This routine creates a new world object and returns the object pointer to
	the caller so that the object may be oriented as necessary.

	The object is drawn when the world view window is drawn.

	The routine implicitly queues a redraw for the next draw cycle.

Arguments:

	MDBResRefs - Supplies the resource names of the model files to load.

	GR2ResRef - Supplies the resource name of the skeleton file to load.

Return Value:

	The routine returns a pointer to a WorldObject instance on success.

	On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	WorldObjectPtr WorldObj;

	WorldObj = new WorldObject(
		m_ResMan,
		m_TextWriter,
		MDBResRefs,
		GR2ResRef);

	m_WorldObjects.push_back( WorldObj );

	RedrawWorldWindowOnly( false );

	return WorldObj;
}

WorldView::~WorldView(
	)
/*++

Routine Description:

	This routine deletes the current WorldView object and its associated
	GUI elements.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	if (m_WorldWindow != NULL)
		DestroyWindow( m_WorldWindow );
}

bool
WorldView::RegisterClasses(
	__in bool Register
	)
/*++

Routine Description:

	This routine initializes or tears down Win32 window classes that are used
	by the world view.

Arguments:

	Register - Supplies whether to initialize or tear down the window classes.

Return Value:

	True on success, else false on failure.

Environment:

	User mode.

--*/
{
	HINSTANCE hLib;

	hLib = (HINSTANCE) &__ImageBase;

	if (Register)
	{
		WNDCLASSEX wc;

		ZeroMemory( &wc, sizeof( wc ) );

		wc.cbSize        = sizeof( wc );
		wc.style         = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc   = WorldView::WorldViewWindowProc_s;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = hLib;
		wc.hIcon         = LoadIcon( NULL, IDI_APPLICATION );
		wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
		wc.hbrBackground = NULL; // Flicker -- (HBRUSH) GetStockObject( LTGRAY_BRUSH );
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = L"WorldViewClass";

		if (RegisterClassEx( &wc ) == 0)
			return false;
	}
	else
	{
		UnregisterClass( L"ModelRendererWorldView", hLib );
	}

	return true;
}

bool
WorldView::DrawWireframeMesh(
	__in HDC hdc,
	__in COLORREF Color,
	__in const WorldObject * Object
	)
/*++

Routine Description:

	This routine draws a wireframe mesh for an object based on the collision
	mesh for the object.  The C3 mesh is (preferably) rendered, else the C2
	mesh is used if all else fails.

Arguments:

	hdc - Supplies the device context to draw into.

	Color - Supplies the color of pixels to use when drawing the object.

	Object - Supplies the object whose wireframe is to be drawn.

Return Value:

	Returns true if the drawing operation was successful, else false.

Environment:

	User mode.

--*/
{
	const ModelSkeleton * BaseSkeleton;
	const ModelSkeleton * HeadSkeleton;
	const ModelSkeleton * Skeleton;
	const CollisionMesh * Mesh;
	const ModelCollider * Model;

	static const COLORREF ColorCollider = RGB( 128,   0, 192 );

	if (Object == NULL)
		return true;

	Model = Object->GetModel( );

	if (Model == NULL)
		return true;

	BaseSkeleton = Object->GetBodyPieceSkeleton( );
	HeadSkeleton = Object->GetSkeleton( NWN::AS_HEAD );

	for (size_t i = 0; i < NWN::BPS_MAX; i += 1)
	{
		NWN::NWN2_BodyPieceSlot Slot;

		Slot     = (NWN::NWN2_BodyPieceSlot) i;
		Model    = Object->GetModel( Slot );

		if (Model == NULL)
			continue;

		Mesh = &Model->GetC3Mesh( );

		if (Mesh->GetPoints( ).empty( ))
			Mesh = &Model->GetC2Mesh( );

		for (ModelCollider::RigidMeshVec::const_iterator it = Model->GetRigidMeshes( ).begin( );
		     it != Model->GetRigidMeshes( ).end( );
		     ++it)
		{
			if (!DrawSimpleWireframeMesh( hdc, Color, *it, Object, Slot ))
				continue;
		}

		for (ModelCollider::SkinMeshVec::const_iterator it = Model->GetSkinMeshes( ).begin( );
		     it != Model->GetSkinMeshes( ).end( );
			 ++it)
		{
			if (!DrawSimpleWireframeMesh( hdc, Color, *it, Object, Slot ))
				continue;
		}

		if (!Mesh->GetPoints( ).empty( ))
			DrawSimpleWireframeMesh( hdc, ColorCollider, *Mesh, Object );

		Skeleton = Object->GetBodyPieceSkeleton( Slot );

		//
		// The base skeleton is not attached, so do not draw it again as
		// anchored to a bone.
		//

		if ((i != 0) && (Skeleton == BaseSkeleton))
			continue;

		//
		// The head skeleton is just used for facial expressions and is
		// otherwise a partial overlay of the main skeleton, so don't draw it
		// either.
		//

		if (Skeleton == HeadSkeleton)
			continue;

		DrawBones( hdc, RGB( 0xFF, 0x00, 0x00 ), Object, Slot );
	}

	return true;
}

template< typename T >
bool
WorldView::DrawSimpleWireframeMesh(
	__inout HDC hdc,
	__in COLORREF Color,
	__in const T & Mesh,
	__in const WorldObject * Object,
	__in NWN::NWN2_BodyPieceSlot Slot /* = NWN::BPS_DEFAULT */
	)
/*++

Routine Description:

	This routine draws a wireframe mesh for an object based on the supplied
	mesh, which may be any mesh compatible with a SimpleMesh-derived mesh.

Arguments:

	hdc - Supplies the device context to draw into.

	Color - Supplies the color of pixels to use when drawing the object.

	Mesh - Supplies the mesh to draw.  The mesh is assumed to have coordinates
	       local to world space.

	Object - Supplies the object whose wireframe is to be drawn.

	Slot - Supplies the body piece slot that the mesh is attached to.

Return Value:

	Returns true if the drawing operation was successful, else false.

Environment:

	User mode.

--*/
{
	HPEN    Pen;
	HBRUSH  Brush;
	bool    Status;

	Pen = CreatePen( PS_SOLID, 1, Color );

	if (Pen == NULL)
		return false;

	Brush = (HBRUSH) GetStockObject( HOLLOW_BRUSH );

	if (Brush == NULL)
	{
		DeleteObject( (HGDIOBJ) Pen );
		return false;
	}

	//
	// Simply draw a polygon with an empty fill and the given outline color.
	//

	Pen   = (HPEN) SelectObject( hdc, (HGDIOBJ) Pen );
	Brush = (HBRUSH) SelectObject( hdc, (HGDIOBJ) Brush );

	Status = DrawSimpleWireframeMesh2( hdc, Mesh, Object, Slot );

	DeleteObject( SelectObject( hdc, (HGDIOBJ) Brush ) );
	DeleteObject( SelectObject( hdc, (HGDIOBJ) Pen ) );

	return Status ? true : false;
}

template< typename T >
bool
WorldView::DrawSimpleWireframeMesh2(
	__inout HDC hdc,
	__in const T & Mesh,
	__in const WorldObject * Object,
	__in NWN::NWN2_BodyPieceSlot Slot
	)
/*++

Routine Description:

	This routine draws a wireframe mesh for an object based on the supplied
	mesh, which may be any mesh compatible with a SimpleMesh-derived mesh.

	The color and fill attributes are assumed to come from the device context.

Arguments:

	hdc - Supplies the device context to draw into.

	Color - Supplies the color of pixels to use when drawing the object.

	Mesh - Supplies the mesh to draw.  The mesh is assumed to have coordinates
	       local to world space.

	Object - Supplies the object whose wireframe is to be drawn.

	Slot - Supplies the body piece slot that the mesh is attached to.

Return Value:

	Returns true if the drawing operation was successful, else false.

Environment:

	User mode.

--*/
{
	std::vector< POINT >  Points;
	std::vector< INT >    PolyCounts;
	int                   Polygons;
	BOOL                  Status;

	//
	// Accumulate all polygons to draw.
	//

	if (!DrawSimpleWireframeMesh3(
		Points,
		PolyCounts,
		Mesh,
		Object,
		Slot,
		true))
	{
		return false;
	}

	Polygons = (int) PolyCounts.size( );

	if (Polygons > 0)
	{
		Status = PolyPolygon(
			hdc,
			&Points[ 0 ],
			&PolyCounts[ 0 ],
			Polygons);
	}
	else
	{
		Status = TRUE;
	}

	return Status ? true : false;
}


template< typename T >
bool
WorldView::DrawSimpleWireframeMesh3(
	__inout std::vector< POINT > & Points,
	__inout std::vector< INT > & PolyCounts,
	__in const T & Mesh,
	__in const WorldObject * Object,
	__in NWN::NWN2_BodyPieceSlot Slot,
	__in bool Reserve
	)
/*++

Routine Description:

	This routine draws a wireframe mesh for an object based on the supplied
	mesh, which may be any mesh compatible with a SimpleMesh-derived mesh.

	The routine accumulates polygons but does not attempt to draw them.

Arguments:

	Points - Supplies the point array to which new points shall be appended to.

	PolyCounts - Supplies the polygon point count array to which new polygon
	             counts shall be appended to.

	Mesh - Supplies the mesh to draw.  The mesh is assumed to have coordinates
	       transformed according to Mesh::CoordTransMode.

	Object - Supplies the object whose wireframe is to be drawn.

	Slot - Supplies the body piece slot that the mesh is attached to.

	Reserve - Supplies a Boolean value indicating true if the routine should
	          precount and reserve storage for individual polygons, else false
	          if the caller has already precounted and prereserved storage for
	          each point and polygon point count entry.

Return Value:

	Returns true if the drawing operation was successful, else false.

Environment:

	User mode.

--*/
{
	const ModelSkeleton  * Skeleton;
	unsigned long          MaxBone;
	int                    Polygons;

	if (Mesh.GetPoints( ).empty( ))
		return false;

	if (Reserve)
	{
		try
		{
			Points.reserve( Points.size( ) + Mesh.GetFaces( ).size( ) * 3 );
			PolyCounts.reserve(
				PolyCounts.size( ) + Mesh.GetFaces( ).size( ) );
		}
		catch (std::exception)
		{
			return false;
		}
	}

	Polygons = 0;

	if (T::CoordTrans == CoordTransModeLocalWeighted)
	{
		Skeleton   = Object->GetBodyPieceSkeleton( Slot );
		MaxBone    = Skeleton->GetMaxBoneIndex( );
	}

	for (T::FaceVec::const_iterator it = Mesh.GetFaces( ).begin( );
	     it != Mesh.GetFaces( ).end( );
	     ++it)
	{
		bool FaceOk;

		FaceOk = true;

		for (size_t i = 0; i < 3; i += 1)
		{
			NWN::Vector3 v3;
			POINT        pt;

			//
			// Convert each coordinate from game-board space into world window
			// client area coordinates.
			//

			if (T::CoordTrans == CoordTransModeLocal)
			{
				v3 = Object->LocalToWorld(
					Mesh.GetPoint3( it->Corners[ i ] ),
					Slot);
			}
			else if (T::CoordTrans == CoordTransModeLocalWeighted)
			{
#define TRANS_SKIN 0
#if TRANS_SKIN
				bool          Trans;
				NWN::Vector3  Vert;
				unsigned long pi;
				unsigned long NumWeights;

				pi   = it->Corners[ i ];
				Vert = Mesh.GetPoint3( pi );
				v3.x = 0.0f;
				v3.y = 0.0f;
				v3.z = 0.0f;

				NumWeights = Mesh.GetVertexWeightCount( pi );

				for (unsigned long wi = 0; wi < NumWeights; wi += 1)
				{
					unsigned long         BoneIndex =
						Mesh.GetVertexBone( pi, wi );
					float                 BoneWeight =
						Mesh.GetVertexWeight( pi, wi );
					NWN::Vector3          v;

					if ((BoneIndex >= MaxBone) || (BoneWeight == 0.0f))
						continue;

					const NWN::Matrix44 & BoneInvWorldTrans =
						Skeleton->GetBoneInvWorldTransform( BoneIndex );
					const NWN::Matrix44 & BoneWorldTrans =
						Skeleton->GetBoneWorldTransform( BoneIndex );

//					v = Math::Multiply( BoneWorldTrans, Vert );
//					v = Math::Subtract( v, Math::GetPosition( BoneWorldTrans ) );
//					v = Math::Subtract( v, Math::GetPosition( BoneInvWorldTrans ) );
//					v = Math::Add( v, Math::GetPosition( BoneWorldTrans ) );
//					v = Math::Add( v, Math::GetPosition( BoneInvWorldTrans ) );
//					v = Math::Multiply( BoneInvWorldTrans, Vert );
//					v = Math::Subtract( v, Math::GetPosition( BoneInvWorldTrans ) );
//					v = Math::Multiply( BoneWorldTrans, v );

					m_TextWriter->WriteText( "V0 %f %f %f\n", Vert.x, Vert.y, Vert.z );
					v = Math::InverseTransform( Vert, BoneInvWorldTrans );
					m_TextWriter->WriteText( "V1 %f %f %f\n", v.x, v.y, v.z );
					v = Math::InverseTransform( v, BoneWorldTrans );
					m_TextWriter->WriteText( "V2 %f %f %f\n", v.x, v.y, v.z );

					v3.x += BoneWeight * v.x;
					v3.y += BoneWeight * v.y;
					v3.z += BoneWeight * v.z;

					Trans = true;
				}

				//
				// If we had no weights, then take the original vertex value
				// as-is, with no manipulation.
				//

				if (!Trans)
					v3 = Object->LocalToWorld( Vert, Slot );
				else
					v3 = Object->LocalToWorld( v3 );
#else
				v3 = Object->LocalToWorld(
					Mesh.GetPoint3( it->Corners[ i ] ),
					Slot);
#endif
			}
			else // CoordTransModeWorld
			{
				v3 = Mesh.GetPoint3( it->Corners[ i ] );
			}

			if (!GameToClientMesh( v3, pt ))
			{
				//
				// Undo any points that we had already pushed onto the array.
				//

				while (i-- > 0)
				{
					Points.pop_back( );
				}

				FaceOk = false;
				break;
			}

			Points.push_back( pt );
		}

		if (!FaceOk)
			continue;

		PolyCounts.push_back( 3 );
		Polygons += 1;
	
	}

	return (Polygons > 0);
}

bool
WorldView::DrawBones(
	__in HDC hdc,
	__in COLORREF Color,
	__in const WorldObject * Object,
	__in NWN::NWN2_BodyPieceSlot Slot /* = NWN::BPS_DEFAULT */
	)
/*++

Routine Description:

	This routine draws each bone in an object.

Arguments:

	hdc - Supplies the device context to draw into.

	Color - Supplies the color of pixels to use when drawing the object.

	Object - Supplies the object whose bones are to be drawn.

	Slot - Supplies the body piece slot that the skeleton is attached to.

Return Value:

	Returns true if the drawing operation was successful, else false.

Environment:

	User mode.

--*/
{
	HPEN                   Pen;
	HBRUSH                 Brush;
	bool                   Status;
	const ModelSkeleton  * Skeleton;

	if ((Skeleton = Object->GetBodyPieceSkeleton( Slot )) == NULL)
		return true;

	if (Skeleton->GetBones( ).empty( ))
		return true;

	Pen = CreatePen( PS_SOLID, 1, Color );

	if (Pen == NULL)
		return false;

	Brush = (HBRUSH) GetStockObject( HOLLOW_BRUSH );

	if (Brush == NULL)
	{
		DeleteObject( (HGDIOBJ) Pen );
		return false;
	}

	//
	// Simply draw a polygon with an empty fill and the given outline color.
	//

	Pen   = (HPEN) SelectObject( hdc, (HGDIOBJ) Pen );
	Brush = (HBRUSH) SelectObject( hdc, (HGDIOBJ) Brush );

	Status = DrawBones2( hdc, Object, Slot );

	DeleteObject( SelectObject( hdc, (HGDIOBJ) Brush ) );
	DeleteObject( SelectObject( hdc, (HGDIOBJ) Pen ) );

	return Status ? true : false;
}

bool
WorldView::DrawBones2(
	__in HDC hdc,
	__in const WorldObject * Object,
	__in NWN::NWN2_BodyPieceSlot Slot
	)
/*++

Routine Description:

	This routine draws each bone in an object.

	The color and fill attributes are assumed to come from the device context.

Arguments:

	hdc - Supplies the device context to draw into.

	Object - Supplies the object whose bones are to be drawn.

	Slot - Supplies the body piece slot that the skeleton is attached to.

Return Value:

	Returns true if the drawing operation was successful, else false.

Environment:

	User mode.

--*/
{
	std::vector< POINT >   Points;
	std::vector< BYTE >    Types;
	int                    Count;
	BOOL                   Status;
	const ModelSkeleton  * Skeleton;

	//
	// Accumulate all polygons to draw.
	//
	
	Skeleton = Object->GetBodyPieceSkeleton( Slot );

	if (Skeleton == NULL)
		return true;

	if (!DrawBones3( Points, Types, Skeleton, Object, Slot, true ))
		return false;

	Count = (int) Types.size( );

	if (Count > 0)
	{
		Status = PolyDraw(
			hdc,
			&Points[ 0 ],
			&Types[ 0 ],
			Count);
	}
	else
	{
		Status = TRUE;
	}

	return Status ? true : false;
}

bool
WorldView::DrawBones3(
	__inout std::vector< POINT > & Points,
	__inout std::vector< BYTE > & Types,
	__in const ModelSkeleton * Skeleton,
	__in const WorldObject * Object,
	__in NWN::NWN2_BodyPieceSlot Slot,
	__in bool Reserve
	)
/*++

Routine Description:

	This routine draws each bone in an object.

	The routine accumulates polygons but does not attempt to draw htem.

Arguments:

	Points - Supplies the point array to which new points shall be appended to.

	Types - Supplies the draw type array to which new point operations are to
	        be appended to.

	Skeleton - Supplies the skeleton to draw.  The mesh is assumed to have
	           coordinates in local space.

	Object - Supplies the object whose bones are to be drawn.

	Slot - Supplies the body piece slot that the skeleton is attached to.

	Reserve - Supplies a Boolean value indicating true if the routine should
	          precount and reserve storage for individual polygons, else false
	          if the caller has already precounted and prereserved storage for
	          each point and polygon point count entry.

Return Value:

	Returns true if the drawing operation was successful, else false.

Environment:

	User mode.

--*/
{
	int Bones;

	if (Skeleton->GetBones( ).empty( ))
		return false;

	if (Reserve)
	{
		try
		{
			Points.reserve(
				Points.size( ) + Skeleton->GetBones( ).size( ) * 2 );
			Types.reserve(
				Types.size( ) + Skeleton->GetBones( ).size( ) * 2 );
		}
		catch (std::exception)
		{
			return false;
		}
	}

	Bones = 0;

	for (ModelSkeleton::BoneIndex i = 0;
	     i < Skeleton->GetMaxBoneIndex( );
	     i += 1)
	{
		const ModelSkeleton::Bone * Bone = Skeleton->GetBone( i );
		NWN::Vector3                Start;
		NWN::Vector3                End;
		POINT                       StartPt;
		POINT                       EndPt;

		//
		// Retrieve coordinates for the bone and its parent.
		//

		Start = Object->LocalToWorld(
			Math::GetPosition(
				Skeleton->GetBoneWorldTransform( i )
				),
			Slot);

		if (Bone->ParentIndex != ModelSkeleton::BoneIndexInvalid)
		{
			End = Object->LocalToWorld(
				Math::GetPosition(
					Skeleton->GetBoneWorldTransform( Bone->ParentIndex )
					),
				Slot);
		}
		else
		{
			End = Start;
		}

#if 0
		Start = Math::Multiply( Object->GetWorldTransform( ), Start );

		if (Bone->ParentIndex != ModelSkeleton::BoneIndexInvalid)
		{
			BoneTrans = Skeleton->GetBoneWorldTransform( Bone->ParentIndex );

			End = Math::GetPosition( BoneTrans );
			End = Math::Multiply( Object->GetWorldTransform( ), End );
		}
		else
		{
			End = Start;
		}
#endif

		//
		// Map to client space and add to the draw list.
		//

		if ((!GameToClientMesh( Start, StartPt )) ||
		    (!GameToClientMesh( End, EndPt )))
		{
			continue;
		}

		Points.push_back( StartPt );
		Types.push_back( PT_MOVETO );
		Points.push_back( EndPt );
		Types.push_back( PT_LINETO );

		Bones += 1;
	}

	return (Bones > 0);
}


bool
WorldView::ClientToGame(
	__in const POINT &pt,
	__out NWN::Vector3 &v,
	__in bool Clip /* = true */
	)
/*++

Routine Description:

	This routine retrieves approximate game coordinates for a game point that
	is nearby the given client window coordinate in the world window.

	There are several coordinate transformations in place:

		- The window client area (0, 0) is relative to the top left, but the
		  game coordinate set (0, 0) is relative to the bottom left.

	    - The map rectangle displayed in the window client area may be offset
		  into the window client area for non-square maps for letterboxing.

	    - The walkable game coordinate rectangle (which is the rectangle that
		  is mapped) may begin at a non-zero origin, such as for an exterior
		  area.

		- Pixel multiplication for object display purposes.

	Additionally, the game supports remapping the view of north, but this is
	only used for visual effects (e.g. the sun's progression) and does not
	influence the coordinate world transformation.  Thus, no transformation
	based on the north mapping is performed.

Arguments:

	pt - Supplies the client coordinate in the world window to convert to game
	     coordinates.

	v - Receives the converted game coordinates.

	Clip - Supplies a Boolean value indicating whether points beyond the edge
	       of the actable game board (i.e. in exterior areas) are rejected
		   (true) or permitted (false).  No bounding is performed in the no
		   clip case.

Return Value:

	Returns true if the conversion was successful, else false if the points
	could not be mapped.

Environment:

	User mode.

--*/
{
	if (!PtInRect( &m_MapRect, pt ))
	{
		if (Clip)
			return false;
	}

	v.x = m_OriginX + (float) (((double)((pt.x - m_MapRect.left) / PIXELS_X_PER_OBJ) /
	               (double)((m_MapRect.right - m_MapRect.left) / PIXELS_X_PER_OBJ)) * (m_AreaWidth - m_OriginX));

	v.y = m_OriginY + (float) (((double)((m_MapRect.bottom - pt.y) / PIXELS_Y_PER_OBJ) /
		           (double)((m_MapRect.bottom - m_MapRect.top) / PIXELS_Y_PER_OBJ)) * (m_AreaHeight - m_OriginY));

	if (m_Camera.get( ) != NULL)
	{
		NWN::Vector3 Origin;
		NWN::Vector3 NormDir;
		float        Distance;
		POINT        RayOrigin;
		POINT        RayTerm;

		//
		// 3D camera modes require ray picking.
		//

		m_Camera->GetPickRay( v, Origin, NormDir );

		m_TextWriter->WriteText( "Pick ray from %f, %f, %f -> %f, %f, %f\n", Origin.x, Origin.y, Origin.z, NormDir.x, NormDir.y, NormDir.z );

		//
		// Check if we viewed any particular point, or if we were straight
		// off into the end of the game board.
		//

		if (!CalcLineOfSightRay(
			Origin,
			NormDir,
			Distance))
		{
//			m_TextWriter->WriteText( "Intersect nothing\n" );
			return false;
		}

		//
		// Convert to a point and return it.
		//

		v = Math::PointFromRayDistance(
			Origin,
			NormDir,
			Distance);

		if ((GameToClient( Origin, RayOrigin )) &&
		    (GameToClient( v, RayTerm )))
		{
			POINT Pts[ 2 ];
			BYTE  Cmds[ 2 ];

			Pts[ 0 ]   = RayOrigin;
			Cmds[ 0 ]  = PT_MOVETO;
			Pts[ 1 ]   = RayTerm;
			Cmds[ 1 ]  = PT_LINETO;

			RedrawWorldWindowOnly( true );
			PolyDraw( GetDC( m_WorldWindow ), Pts, Cmds, 2 );
		}

		m_TextWriter->WriteText( "Hit face @ +%f (-> %f, %f, %f)\n", Distance, v.x, v.y, v.z );

		POINT pt;

		if (GameToClient(v, pt, true))
		{
//			RedrawWorldWindowOnly( true );
			RECT rc = {0, 0, 0, 0 };
			wchar_t DebugInfo[ 256 ];
			HDC hdc;

			StringCbPrintfW(
				DebugInfo,
				sizeof( DebugInfo ),
				L"Hit @ %f, %f, %f\n",
				v.x,
				v.y,
				v.z);

			hdc = GetDC( m_WorldWindow );

			if (hdc)
			{
				DrawText(
					hdc,
					DebugInfo,
					-1,
					&rc,
					DT_NOCLIP | DT_NOPREFIX );

				ReleaseDC( m_WorldWindow, hdc );
			}
		}
		else
		{
			m_TextWriter->WriteText( "No GameToClient for that coord!\n" );
		}

		return true;
	}

#if 0
	//
	// 2D modes assume that the ground was clicked and must determine the z
	// coordinate from ground height.
	//

	Area = GetArea( );

	if (Area != NULL)
	{
		NWN::Vector2 v2;

		v2.x = v.x;
		v2.y = v.y;
		v.z  = Area->GetPointHeight( v2 );
	}
	else
	{
		v.z = 0.0f;
	}

//	m_TextWriter->WriteText( "C2G : %d, %d -> %f, %f, %f\n", pt.x, pt.y, v.x, v.y, v.z );

	return true;
#else
	return false;
#endif
}

bool
WorldView::GameToClient(
	__in const NWN::Vector3 &v,
	__out POINT &pt,
	__in bool Clip /* = true */
	)
/*++

Routine Description:

	This routine retrieves approximate world window client coordinates that
	correspond to a set of game coordinates.

	There are several coordinate transformations in place:

		- The window client area (0, 0) is relative to the top left, but the
		  game coordinate set (0, 0) is relative to the bottom left.

	    - The map rectangle displayed in the window client area may be offset
		  into the window client area for non-square maps for letterboxing.

	    - The walkable game coordinate rectangle (which is the rectangle that
		  is mapped) may begin at a non-zero origin, such as for an exterior
		  area.

		- Pixel multiplication for object display purposes.

	Additionally, the game supports remapping the view of north, but this is
	only used for visual effects (e.g. the sun's progression) and does not
	influence the coordinate world transformation.  Thus, no transformation
	based on the north mapping is performed.

Arguments:

	v - Supplies the game coordinates to convert to world window client
	    coordinates.

	pt - Receives the converted world window client coordinates.

	Clip - Supplies a Boolean value indicating whether points beyond the edge
	       of the actable game board (i.e. in exterior areas) are to be clipped
		   to the nearest edge or whether the operation should simply fail.

Return Value:

	Returns true if the conversion was successful, else false if the points
	could not be mapped.

Environment:

	User mode.

--*/
{
	NWN::Vector2 v2;

	//
	// 3D camera modes.
	//

	if (m_Camera.get( ) != NULL)
		v2 = m_Camera->WorldToScreenViewport( v );
	else
		return false;

	//
	// Clip overhanging coordinate values.  We allow coordinates in the tile
	// border padding region, but we just truncate these to their respective
	// closest map coordinate as we do not visualize the area outside the core
	// moveable portion.
	//

	if (v2.x < m_OriginX)
	{
		if ((m_OriginX - v2.x > m_PaddingX) || (v2.x < 0.0f))
			return false;

		if (!Clip)
			return false;

		v2.x = m_OriginX;
	}

	if (v2.y < m_OriginY)
	{
		if ((m_OriginY - v2.y > m_PaddingY) || (v2.y < 0.0f))
			return false;

		if (!Clip)
			return false;

		v2.y = m_OriginY;
	}

	if (v2.x > m_AreaWidth)
	{
		if (v2.x - m_AreaWidth > m_PaddingX)
			return false;

		if (!Clip)
			return false;

		v2.x = m_AreaWidth;
	}

	if (v2.y > m_AreaHeight)
	{
		if (v2.y - m_AreaHeight > m_PaddingX)
			return false;

		if (!Clip)
			return false;

		v2.y = m_AreaHeight;
	}

	pt.x = m_MapRect.left + (long) (((double)((v2.x - m_OriginX) * PIXELS_X_PER_OBJ) /
		           (double)((m_AreaWidth - m_OriginX) * PIXELS_X_PER_OBJ)) * (m_MapRect.right - m_MapRect.left));
	pt.y = m_MapRect.bottom - (long) (((double)((v2.y - m_OriginX) * PIXELS_Y_PER_OBJ) /
		           (double)((m_AreaHeight - m_OriginY) * PIXELS_Y_PER_OBJ)) * (m_MapRect.bottom - m_MapRect.top));

	if (Clip)
	{
		pt.x = min( pt.x, m_MapRect.right - 1 );
		pt.y = min( pt.y, m_MapRect.bottom - 1 );
		pt.x = max( pt.x, 0 );
		pt.y = max( pt.y, 0 );
	}
	else
	{
		if ((pt.x > m_MapRect.right - 1)  ||
		    (pt.y > m_MapRect.bottom - 1) ||
		    (pt.x < 0)                ||
		    (pt.y < 0))
		{
			return false;
		}
	}

	return true;
}

bool
WorldView::GameToClientMesh(
	__in const NWN::Vector3 &v,
	__out POINT &pt
	)
/*++

Routine Description:

	This routine retrieves approximate world window client coordinates that
	correspond to a set of game coordinates.  See the description of the full
	GameToClient routine for more details.

	This routine is intended for use when converting meshes.  In 3D mode,
	any meshes out of camera are simply not drawn; in 2D mode, they are
	adjusted for a best fit in the viewspace.

Arguments:

	v - Supplies the game coordinates to convert to world window client
	    coordinates.

	pt - Receives the converted world window client coordinates.

Return Value:

	Returns true if the conversion was successful, else false if the points
	could not be mapped.

Environment:

	User mode.

--*/
{
	return GameToClient( v, pt, m_Camera.get( ) != NULL ? false : true );
}

void
WorldView::CreateCamera(
	)
/*++

Routine Description:

	This routine creates the default camera.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	try
	{
		CameraPtr C;

		C = new Camera;

		C->SetCameraSize( m_AreaWidth, m_AreaHeight );

		SetDefaultCameraParameters( C.get( ) );

		m_Camera = C;
	}
	catch (std::exception)
	{
	}
}

void
WorldView::SetDefaultCameraParameters(
	__in Camera * C
	)
/*++

Routine Description:

	This routine adjusts camera parameters to typical defaults, so that the
	entire area is in view.

Arguments:

	C - Supplies the camera to adjust.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	try
	{
		NWN::Vector3 AreaCenter;
		NWN::Vector3 CameraPosition;
		NWN::Vector3 v;
		NWN::Vector2 Midpoint2;
		NWN::Vector2 Viewport;

		Midpoint2.x = (m_AreaWidth  / 2) + m_OriginX;
		Midpoint2.y = (m_AreaHeight / 2) + m_OriginY;

		AreaCenter.x = Midpoint2.x;
		AreaCenter.y = Midpoint2.y;
		AreaCenter.z = 0.0f;

		//
		// Orient the camera so that the enter area is in view.
		//
		// See http://msdn.microsoft.com/en-us/library/bb197900(XNAGameStudio.10).aspx
		//

		CameraPosition   = AreaCenter;
		CameraPosition.z = max( Midpoint2.x, Midpoint2.y ) / sinf( C->GetFOV( ) );

		//
		// Looking down:
		//
		// up      = +0, +1, +0
		// right   = +1, +0, +0
		// look at = +0, +0, -1
		//

		v.x = 0.0;
		v.y = 1.0;
		v.z = 0.0;

		C->SetCameraUp( v );

		v.x = 1.0;
		v.y = 0.0;
		v.z = 0.0;

		C->SetCameraRight( v );

		v.x = 0.0;
		v.y = 0.0;
		v.z = -1.0;

		C->SetCameraLookAt( v );

		C->SetCameraSize( m_AreaWidth, m_AreaHeight );
		C->SetCameraPosition( CameraPosition );

		Viewport.x = m_OriginX;
		Viewport.y = m_OriginY;

		C->SetViewport( Viewport );
	}
	catch (std::exception)
	{
	}
}

bool
WorldView::CalcLineOfSightRay(
	__in const NWN::Vector3 & Origin,
	__in const NWN::Vector3 & NormDir,
	__out float & Distance
	)
/*++

Routine Description:

	This routine calculates whether there exists a clear line of sight from a
	ray to the edge of the map.

Arguments:

	Area - Supplies the area object for line of sight is to be calculated for.

	Origin - Supplies the origin point of the ray.

	NormDir - Supplies the normalized direction of the ray.

	Distance - Receives the distance to the intersection point, should the ray
	           ray intersect with a collider.  The intersection point closest
	           to the origin of the ray is returned.

Return Value:

	Returns a Boolean value indicating true if a clear line of sight existed,
	else false if a clear line of sight did not exist.

Environment:

	User mode.

--*/
{
	bool           Intersected;
	NWN::Vector3   IntersectNormal;
	float          IntersectDistance;

	Intersected = false;

	PathDebug(
		"Check collider LOS from %f, %f, %f\n",
		Origin.x,
		Origin.y,
		Origin.z);

	//
	// Draw a ray starting from the first point, towards the second, and check
	// for any intersections along the way for all objects in the area.
	//

	for (WorldObjectVec::const_iterator it = m_WorldObjects.begin( );
	     it != m_WorldObjects.end( );
	     ++it)
	{
		WorldObject * Object = it->get( );

#if PATH_DEBUG
		PathDebug(
			"Check intersect with object <%p> @ %f, %f, %f\n",
			Object,
			Object->GetPosition( ).x,
			Object->GetPosition( ).y,
			Object->GetPosition( ).z);
#endif

		if (!Object->IntersectRay(
			Origin,
			NormDir,
			IntersectNormal,
			&IntersectDistance))
		{
			continue;
		}

		if (!Intersected)
		{
			Distance    = IntersectDistance;
			Intersected = true;
		}
		else
		{
			if (IntersectDistance < Distance)
				Distance = IntersectDistance;
		}

		PathDebug(
			"Intersection found at distance %f (normal %f, %f, %f)\n",
			IntersectDistance,
			IntersectNormal.x,
			IntersectNormal.y,
			IntersectNormal.z);
	}

	return Intersected;
}

void
WorldView::RecalculateMapRect(
	__in const RECT * ClientRect
	)
/*++

Routine Description:

	This routine recalculates the bounding rectangle within which all map
	drawing operations are performed.  Outside the bounding rectangle, a
	default flood fill is performed to enable a letterboxing effect for areas
	that do not have square dimensions.

Arguments:

	ClientRect - Supplies the current client area rectangle to use for the
	             calculation.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	double       Ratio;
	long         Delta;

	//
	// Update the map rect based on the new client rect first.
	//

	CopyRect( &m_MapRect, ClientRect );

	//
	// Adjust the map rectangle if the game map is not a square, so that we
	// have a letterboxed map.
	//

	if (m_AreaWidth != m_AreaHeight)
	{
		if (m_AreaWidth < m_AreaHeight)
		{
			Ratio = (m_AreaWidth / m_AreaHeight);
			Delta = (long) ((m_MapRect.right - m_MapRect.left) -
				((m_MapRect.right - m_MapRect.left) * Ratio));

			m_MapRect.left  += (Delta / 2);
			m_MapRect.right -= (Delta / 2);
		}
		else
		{
			Ratio = (m_AreaHeight / m_AreaWidth);
			Delta = (long) ((m_MapRect.top - m_MapRect.bottom) -
				((m_MapRect.top - m_MapRect.bottom) * Ratio));

			m_MapRect.bottom += (Delta / 2);
			m_MapRect.top    -= (Delta / 2);
		}
	}

	if (m_Camera.get( ) != NULL)
	{
		m_Camera->SetCameraSize(
			m_AreaWidth,
			m_AreaHeight);
	}
}

void
WorldView::MoveControls(
	)
/*++

Routine Description:

	This routine is called to resize the position of the various control
	windows on the world window after a resize operation.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode, client rect already recalculated.

--*/
{
}

void
WorldView::DrawWorldWindow(
	__in HWND hwnd,
	__in PAINTSTRUCT & ps,
	__in const RECT & UpdateRect
	)
/*++

Routine Description:

	This routine draws the world window into a device context.

Arguments:

	hwnd - Supplies the world window HWND.

	ps - Supplies the paint structure, which must have a legal hdc member.

	UpdateRect - Supplies the region that must be repainted.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	RECT           DrawRect;
	HDC            DrawDC;
	HBITMAP        Bitmap;

	UNREFERENCED_PARAMETER( hwnd );

	DrawDC = CreateCompatibleDC( ps.hdc );

	if (DrawDC == NULL)
		return;

	//
	// Create a double-buffer bitmap so that there is no visible flicker during
	// the rendering step.
	//

	Bitmap = CreateCompatibleBitmap(
		ps.hdc,
		(m_ClientRect.right - m_ClientRect.left) + 0,
		(m_ClientRect.bottom - m_ClientRect.top) + 0);

	if (Bitmap == NULL)
	{
		DeleteDC( DrawDC );
		return;
	}

	Bitmap = (HBITMAP) SelectObject( DrawDC, (HGDIOBJ) Bitmap );

	//
	// Manually draw the background.
	//
	// N.B.  We can safely just draw directly to the paint DC when we are
	//       erasing the background, as changes to the map rect dimensions are
	//       infrequent.
	//

	if ((ps.fErase) && (!EqualRect( &m_MapRect, &m_ClientRect )))
	{
		HBRUSH Brush = (HBRUSH) GetStockObject( LTGRAY_BRUSH );
		RECT   rc;

		//
		// There are three possible drawing scenarios here where the map and
		// client rects may be different.
		//
		// 1)  We're not joined to an area in which case there is nothing to
		//     map.
		//
		//     N.B.  The first case is never true for the standalone, simple
		//     world viewer right now.
		//
		// 2)  The area is wider than it is tall and we have background to draw
		//     above and below the map.
		//
		// 3)  The area is taller than it is wide and we have background to draw
		//     to the left and right of the map.
		//
/*
		if (m_AreaObjectId == NWN::INVALIDOBJID)
		{
			//
			// No map, just fill it and be done with it.
			//

			FillRect( ps.hdc, &m_ClientRect, Brush );
		}
		else */if (m_MapRect.left != m_ClientRect.left)
		{
			//
			// Left and right rects.
			//

			rc.left   = m_ClientRect.left;
			rc.right  = m_MapRect.left;
			rc.top    = m_ClientRect.top;
			rc.bottom = m_ClientRect.bottom;

			FillRect( ps.hdc, &rc, Brush );

			rc.left   = m_MapRect.right;
			rc.right  = m_ClientRect.right;
			rc.top    = m_ClientRect.top;
			rc.bottom = m_ClientRect.bottom;

			FillRect( ps.hdc, &rc, Brush );
		}
		else
		{
			//
			// Top and bottom rects.
			//

			rc.left   = m_ClientRect.left;
			rc.right  = m_ClientRect.right;
			rc.bottom = m_MapRect.top;
			rc.top    = m_ClientRect.top;

			FillRect( ps.hdc, &rc, Brush );

			rc.left   = m_ClientRect.left;
			rc.right  = m_ClientRect.right;
			rc.bottom = m_ClientRect.bottom;
			rc.top    = m_MapRect.bottom;

			FillRect( ps.hdc, &rc, Brush );
		}
	}

	//
	// Draw all of the objects in the scene.
	//

	for (WorldObjectVec::const_iterator it = m_WorldObjects.begin( );
	     it != m_WorldObjects.end( );
	     ++it)
	{
		DrawWireframeMesh(
			DrawDC,
			RGB( 0x00, 0xFF, 0x00 ),
			it->get( ));
	}

	//
	// Flip the draw bitmap to the screen and copy it all over now that we have
	// composited the final image.
	//

	IntersectRect( &DrawRect, &UpdateRect, &m_MapRect );

	(VOID) BitBlt(
		ps.hdc,
		DrawRect.left,
		DrawRect.top,
		(DrawRect.right - DrawRect.left) + 0,
		(DrawRect.bottom - DrawRect.top) + 0,
		DrawDC,
		DrawRect.left,
		DrawRect.top,
		SRCCOPY);

	Bitmap = (HBITMAP) SelectObject( DrawDC, (HGDIOBJ) Bitmap );
	(VOID) DeleteObject( Bitmap );
	DeleteDC( DrawDC );
}

//
// Begin window procedure related processing.
//

LRESULT
CALLBACK
WorldView::WorldViewWindowProc_s(
	__in HWND hwnd,
	__in UINT uMsg,
	__in WPARAM wParam,
	__in LPARAM lParam
	)
/*++

Routine Description:

	This routine thunks window messages destined to the world window to the
	real __thiscall implementation.

Arguments:

	hwnd - Supplies the world window HWND.

	uMsg - Supplies the window message code to process.

	wParam - Supplies a message-specific parameter.

	lParam - Supplies a message-specific parameter.

Return Value:

	Returns a message-specific return value (e.g. 0).

Environment:

	User mode.

--*/
{
	WorldView * This;

	switch (uMsg)
	{

	case WM_NCCREATE:
		{
			LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;

			This = (WorldView *)cs->lpCreateParams;

#pragma warning(push)
#pragma warning(disable:4244) // warning C4244: 'argument' : conversion from 'LONG_PTR' to 'LONG', possible loss of data
			SetWindowLongPtr(
				hwnd,
				GWLP_USERDATA,
				reinterpret_cast< LONG_PTR >( This )
				);
#pragma warning(pop)

			break;

		}

	default:
#pragma warning(push)
#pragma warning(disable:4312) // warning C4312: 'type cast' : conversion from 'LONG' to 'WorldView *' of greater size
			This = (WorldView *)GetWindowLongPtr(
				hwnd,
				GWLP_USERDATA
				);
#pragma warning(pop)

			break;
	}

	//
	// Thunk to the class routine.
	//

	__try
	{
		if (This != NULL)
			return This->WorldViewWindowProc( hwnd, uMsg, wParam, lParam );
	}
	__except (WndprocExceptionFilter( GetExceptionInformation( ) ))
	{
	}

	//
	// Default to DefWindowProc if we've got no object pointer yet.
	//

	return DefWindowProc( hwnd, uMsg, wParam, lParam );
}

LRESULT
WorldView::WorldViewWindowProc(
	__in HWND hwnd,
	__in UINT uMsg,
	__in WPARAM wParam,
	__in LPARAM lParam
	)
/*++

Routine Description:

	This routine forms the window procedure for the world window.

Arguments:

	hwnd - Supplies the world window HWND.

	uMsg - Supplies the window message code to process.

	wParam - Supplies a message-specific parameter.

	lParam - Supplies a message-specific parameter.

Return Value:

	Returns a message-specific return value (e.g. 0).

Environment:

	User mode.

--*/
{
	switch (uMsg)
	{

	case WM_ERASEBKGND:
		return 0; // Manual erase.

	case WM_CLOSE:
		return HANDLE_WM_CLOSE( hwnd, wParam, lParam, OnClose );

	case WM_DESTROY:
		return HANDLE_WM_DESTROY( hwnd, wParam, lParam, OnDestroy );

	case WM_GETMINMAXINFO:
		return HANDLE_WM_GETMINMAXINFO( hwnd, wParam, lParam, OnGetMinMaxInfo );

	case WM_PAINT:
		return HANDLE_WM_PAINT( hwnd, wParam, lParam, OnPaint );

	case WM_PRINTCLIENT:
		return OnPrintClient( hwnd, (HDC) wParam, (UINT) lParam ), 0;

	case WM_SIZE:
		return HANDLE_WM_SIZE( hwnd, wParam, lParam, OnSize );

	case WM_MOVE:
		return HANDLE_WM_MOVE( hwnd, wParam, lParam, OnMove );

	case WM_KEYDOWN:
		return HANDLE_WM_KEYDOWN( hwnd, wParam, lParam, OnKey );

	case WM_KEYUP:
		return HANDLE_WM_KEYUP( hwnd, wParam, lParam, OnKey );

	case WM_MOUSEMOVE:
		return HANDLE_WM_MOUSEMOVE( hwnd, wParam, lParam, OnMouseMove );

	case WM_MOUSEWHEEL:
		return HANDLE_WM_MOUSEWHEEL( hwnd, wParam, lParam, OnMouseWheel );

	}

	//
	// Pass the message on to the default handler.
	//

	return DefWindowProc( hwnd, uMsg, wParam, lParam );
}

void
WorldView::OnClose(
	__in HWND hwnd
	)
/*++

Routine Description:

	This routine is called when the user closes the world window.

Arguments:

	hwnd - Supplies the world window HWND.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	//
	// Hide the window; the user can re-show it later.
	//

	ShowWindow( hwnd, SW_HIDE );

	PostQuitMessage( 0 );
}

void
WorldView::OnDestroy(
	__in HWND hwnd
	)
/*++

Routine Description:

	This routine is called when the world window is destroyed.

Arguments:

	hwnd - Supplies the world window HWND.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	UNREFERENCED_PARAMETER( hwnd );

	m_WorldWindow = NULL;
}

void
WorldView::OnGetMinMaxInfo(
	__in HWND hwnd,
	__inout LPMINMAXINFO lpMinMaxInfo
	)
/*++

Routine Description:

	This routine is called when the system desires to query the min/max size
	parameters for the world window.

Arguments:

	hwnd - Supplies the world window HWND.

	lpMinMaxInfo - Supplies the default min/max parameters, and receives any
	               override parameters specified.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	UNREFERENCED_PARAMETER( hwnd );
	UNREFERENCED_PARAMETER( lpMinMaxInfo );

	//
	// The defaults are fine.
	//
}

void
WorldView::OnPaint(
	__in HWND hwnd
	)
/*++

Routine Description:

	This routine prints the area map to the client area of the world window.

Arguments:

	hwnd - Supplies the world window HWND.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	RECT        UpdateRect;
	PAINTSTRUCT ps;

	if (!GetUpdateRect( hwnd, &UpdateRect, TRUE ))
		return;

	if (BeginPaint( hwnd, &ps ) == NULL)
		return;

	DrawWorldWindow( hwnd, ps, UpdateRect );

	EndPaint( hwnd, &ps );
}

void
WorldView::OnPrintClient(
	__in HWND hwnd,
	__in HDC hdc,
	__in UINT DrawFlags
	)
/*++

Routine Description:

	This routine prints the area map to a specified device context.

Arguments:

	hwnd - Supplies the world window HWND.

	hdc - Supplies the device context to draw into.

	DrawFlags - Supplies the print draw flags.  Legal constants are drawn from
	            the PRF_* family of constants.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	PAINTSTRUCT ps;

	if (!(DrawFlags & PRF_CLIENT))
		return;

	ps.hdc    = hdc;
	ps.fErase = FALSE;

	CopyRect( &ps.rcPaint, &m_ClientRect );
	DrawWorldWindow( hwnd, ps, ps.rcPaint );
}

void
WorldView::OnSize(
	__in HWND hwnd,
	__in UINT state,
	__in int cx,
	__in int cy
	)
/*++

Routine Description:

	This routine handles changes to the world window size on-screen.  It bears
	responsibility for recalculating the map dimensions and initiating a
	redraw as needed.

Arguments:

	hwnd - Supplies the world window HWND.

	state - Supplies the type of sizing requested.

	cx - Supplies the new client area width.

	cy - Supplies the new client area height.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	UNREFERENCED_PARAMETER( state );
	UNREFERENCED_PARAMETER( cx );
	UNREFERENCED_PARAMETER( cy );

	//
	// Cache the new client region size.
	//

	if (!GetClientRect( hwnd, &m_ClientRect ))
		return;

	//
	// Update control positions.
	//

	MoveControls( );

	//
	// Update the map rectangle appropriately.
	//

	RecalculateMapRect( &m_ClientRect );
}

void
WorldView::OnMove(
	__in HWND hwnd,
	__in int x,
	__in int y
	)
/*++

Routine Description:

	This routine handles changes to the world window position on-screen.  It
	bears responsibility for storing window position parameters in the
	persisted configuration database.

Arguments:

	hwnd - Supplies the world window HWND.

	x - Supplies the new window x-coordinate.

	y - Supplies the new window y-coordinate.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	UNREFERENCED_PARAMETER( hwnd );
	UNREFERENCED_PARAMETER( x );
	UNREFERENCED_PARAMETER( y );
}

void
WorldView::OnKey(
	__in HWND hwnd,
	__in UINT vk,
	__in BOOL fDown,
	__in int cRepeat,
	__in UINT flags
	)
/*++

Routine Description:

	This routine handles handles keyboard input that is sent to the world window.

	It allows for camera control.

Arguments:

	hwnd - Supplies the world window HWND.

	vk - Supplies the virtual key code that was pressed (or released).

	fDown - Supplies a Boolean value indicating TRUE if the key was pressed, or
	        FALSE if the key was released.

	cRepeat - Supplies the repeat count for the keystroke.

	flags - Supplies the key state flags; see the WM_KEYDOWN documentation for
	        more details.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	switch (vk)
	{

	case 'Z':
		{
			//
			// Reset 3D camera to default positions (toggling between top-down
			// and follow views).
			//

			if (!fDown)
				break;

			if (m_Camera.get( ) != NULL)
			{
//				const NWN::Vector3 & LookAt = m_Camera->GetCameraLookAt( );

				SetDefaultCameraParameters( m_Camera.get( ) );

				m_TextWriter->WriteText( "Set camera to default parameters.\n" );

				//
				// Redraw so that the camera changes take effect.
				//

				RedrawWorldWindowOnly( false );
			}
		}
		break;

#if 1
		//
		// TEMP, TEMP.  Camera manipulation.
		//

	case 'H':
		if (m_Camera.get( ))
			m_Camera->TurnCameraLeft( PI / 32 );

		//
		// Request a redraw as the background image has changed.
		//

		RedrawWorldWindowOnly( false );
		break;

	case 'K':
		if (m_Camera.get( ))
			m_Camera->TurnCameraLeft( -PI / 32 );

		//
		// Request a redraw as the background image has changed.
		//

		RedrawWorldWindowOnly( false );
		break;

	case 'U':
		if (m_Camera.get( ))
			m_Camera->TurnCameraUp( PI / 32 );

		//
		// Request a redraw as the background image has changed.
		//

		RedrawWorldWindowOnly( false );
		break;

	case 'J':
		if (m_Camera.get( ))
			m_Camera->TurnCameraUp( -PI / 32 );

		//
		// Request a redraw as the background image has changed.
		//

		RedrawWorldWindowOnly( false );
		break;

	case 'E':
		if (m_Camera.get( ))
			m_Camera->MoveCameraLeft( 1 );

		//
		// Request a redraw as the background image has changed.
		//

		RedrawWorldWindowOnly( false );
		break;

	case 'R':
		if (m_Camera.get( ))
			m_Camera->MoveCameraLeft( -1 );

		//
		// Request a redraw as the background image has changed.
		//

		RedrawWorldWindowOnly( false );
		break;

	case 'F':
		if (m_Camera.get( ))
			m_Camera->MoveCameraForward( 1 );

		//
		// Request a redraw as the background image has changed.
		//

		RedrawWorldWindowOnly( false );
		break;

	case 'V':
		if (m_Camera.get( ))
			m_Camera->MoveCameraForward( -1 );

		//
		// Request a redraw as the background image has changed.
		//

		RedrawWorldWindowOnly( false );
		break;

	case 'N':
		if (m_Camera.get( ))
			m_Camera->MoveCameraUp( 1 );

		//
		// Request a redraw as the background image has changed.
		//

		RedrawWorldWindowOnly( false );
		break;

	case 'M':
		if (m_Camera.get( ))
			m_Camera->MoveCameraUp( -1 );

		//
		// Request a redraw as the background image has changed.
		//

		RedrawWorldWindowOnly( false );
		break;

#endif

	}

	UNREFERENCED_PARAMETER( hwnd );
	UNREFERENCED_PARAMETER( fDown );
	UNREFERENCED_PARAMETER( cRepeat );
	UNREFERENCED_PARAMETER( flags );
}

void
WorldView::OnMouseMove(
	__in HWND hwnd,
	__in int x,
	__in int y,
	__in UINT keyFlags
	)
/*++

Routine Description:

	This routine handles mouse move indications from the window manager.  Its
	purpose is to adjust the tooltip based on the object under the cursor, and
	to redraw the drag selection rectangle if the window is in drag mode.

Arguments:

	hwnd - Supplies the world window HWND.

	x - Supplies the client-area x coordinate of the mouse cursor.

	y - Supplies the client-area y coordinate of the mouse cursor.

	keyFlags - Supplies a bitmask of flags (MK_*) indicating which mouse
	           buttons were pressed at the time when the message was sent.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	//
	// First, handle camera rotation in 3D mode.
	//

	if ((keyFlags & MK_RBUTTON) && (IsCamera3D( )))
	{
		POINT pt;
		int   dx;
		int   dy;

		dx = (x - m_CursorX);
		dy = (y - m_CursorY);

		m_CameraRotateDelta.x += dx;
		m_CameraRotateDelta.y += dy;

		if (abs( m_CameraRotateDelta.x ) > 1)
		{
			m_Camera->TurnCameraLeft( (float) -m_CameraRotateDelta.x / 800.0f );

			m_CameraRotateDelta.x = 0;
		}

		if (abs( m_CameraRotateDelta.y ) > 1)
		{
			m_Camera->TurnCameraUp( (float) -m_CameraRotateDelta.y / 800.0f );

			m_CameraRotateDelta.y = 0;
		}

		pt.x = m_CursorX;
		pt.y = m_CursorY;

		if (ClientToScreen( m_WorldWindow, &pt ))
			SetCursorPos( pt.x, pt.y );

		//
		// Redraw the background now.
		//

		RedrawWorldWindowOnly( true );

		return;
	}

	m_CameraRotateDelta.x = 0;
	m_CameraRotateDelta.y = 0;

	//
	// Update the cursor position.
	//

	m_CursorX = x;
	m_CursorY = y;

	UNREFERENCED_PARAMETER( hwnd );
	UNREFERENCED_PARAMETER( keyFlags );

	//
	// Test ray picking.
	//

	if (IsKeyDown( VK_CONTROL ) && (IsCamera3D( )))
	{
		POINT        MousePt;
		NWN::Vector3 v;

		MousePt.x = x;
		MousePt.y = y;

		(void) ClientToGame( MousePt, v );
	}
}

void
WorldView::OnMouseWheel(
	__in HWND hwnd,
	__in int xPos,
	__in int yPos,
	__in int zDelta,
	__in UINT fwKeys
	)
/*++

Routine Description:

	This routine handles mouse wheel indications from the window manager.  Its
	purpose is to adjust the camera forward or backward along its look-at axis.

Arguments:

	hwnd - Supplies the world window HWND.

	xPos - Supplies the client-area x coordinate of the mouse cursor.

	yPos - Supplies the client-area y coordinate of the mouse cursor.

	fwKeys - Supplies a bitmask of flags (MK_*) indicating which mouse buttons
	         were pressed at the time when the message was sent.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	UNREFERENCED_PARAMETER( hwnd );
	UNREFERENCED_PARAMETER( xPos );
	UNREFERENCED_PARAMETER( yPos );

	if (IsCamera3D( ))
	{
		if (!(fwKeys & MK_RBUTTON))
		{
			//
			// Adjust the camera in or out.
			//

			m_Camera->MoveCameraForward( ((float)(zDelta) / WHEEL_DELTA) * 6.0f );
		}
		else
		{
			//
			// Roll the camera.
			//

			m_Camera->RollCamera( ((float)(zDelta) / WHEEL_DELTA) / 10.0f );
		}

		//
		// Redraw the background now if we were in 3D mode.
		//

		RedrawWorldWindowOnly( false );
	}
}



LONG
WndprocExceptionFilter(
	__in LPEXCEPTION_POINTERS ExceptionPointers
	)
/*++

Routine Description:

	This routine acts as the unhandled exception filter for window procedures.

	It passes the exception directly to the system unhandled exception filter,
	should the exception not be a guard page violation.

	Some system versions incorrectly swallow exceptions at the win32k/user32
	interface, and this exception filter prevents this from occuring.

Arguments:

	ExceptionPointers - Supplies a pointer to the exception descriptor.

Return Value:

	The routine returns EXCEPTION_CONTINUE_SEARCH to continue a normal search,
	else it returns UnhandledExceptionFilter's continue status.

Environment:

	User mode, arbitrary context.

--*/
{
	//
	// Never touch guard page exceptions.
	//

	if (ExceptionPointers->ExceptionRecord->ExceptionCode == EXCEPTION_GUARD_PAGE)
		return EXCEPTION_CONTINUE_SEARCH;

	return UnhandledExceptionFilter( ExceptionPointers );
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

