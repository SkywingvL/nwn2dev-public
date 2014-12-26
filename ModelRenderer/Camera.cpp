/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	Camera.cpp

Abstract:

	This module houses the Camera object, which houses a coordinate
	transformation system used to map between Screening and world space.

	Camera logic assembled from various sources.

--*/

#include "Precomp.h"
#include "../NWN2MathLib/NWN2MathLib.h"
#include "Camera.h"
//#include "TextOut.h"

#define CAMERA_ENABLE_MIRRORING 0

Camera::Camera(
	__in float FOV /* = PI / 4 */,
	__in float Near /* = 0.1000000015f */,
	__in float Far /* = 140.0f */,
	__in float Aspect /* = 1.0f */,
	__in float Width /* = 500.0f */,
	__in float Height /* = 500.0f */
	)
/*++

Routine Description:

	This routine constructs a Camera object, initializing the camera parameters
	with supplied values.

Arguments:

	FOV - Supplies the camera field of view (FOV).

	Near - Supplies the near object distance.

	Far - Supplies the far object distance.

	Aspect - Supplies the camera aspect ratio.  The caller guarantees that the
	         aspect ratio matches the initialization width and height.

	Width - Supplies the camera width.

	Height - Supplies the camera height.

Return Value:

	The newly constructed object.

Environment:

	User mode.

--*/
: m_FOV( FOV ),
  m_Near( Near ),
  m_Far( Far ),
  m_AspectRatio( Aspect ),
  m_Width( Width ),
  m_Height( Height ),
  m_HalfWidth( Width / 2.0f ),
  m_HalfHeight( Height / 2.0f ),
  m_MinZ( 0.0f ),
  m_MaxZ( 1.0f )
{
	Math::CreateIdentityMatrix( m_View );
	Math::CreateIdentityMatrix( m_Projection );
	Math::CreateIdentityMatrix( m_ViewProj );
	Math::CreateIdentityMatrix( m_InvView );

	m_Position.x = 0.0f;
	m_Position.y = 0.0f;
	m_Position.z = 0.0f;

	//
	// 0, 0, 1
	// 0, 1, 0
	// 1, 0, 0

	m_Up.x       = 0.0f;
	m_Up.y       = 1.0f;
	m_Up.z       = 0.0f;

	m_LookAt.x   = 0.0f;
	m_LookAt.y   = 0.0f;
	m_LookAt.z   = -1.0f;

	m_Right.x    = 1.0f;
	m_Right.y    = 0.0f;
	m_Right.z    = 0.0f;

	m_Viewport.x = 0.0f;
	m_Viewport.y = 0.0f;

	UpdateTransform( );
}

Camera::~Camera(
	)
/*++

Routine Description:

	This routine cleans up an already-existing Camera object.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
}

NWN::Vector3
Camera::WorldToScreen(
	__in const NWN::Vector3 & World
	) const
/*++

Routine Description:

	This routine maps world-space coordinates to screen-space coordinates.

Arguments:

	World - Supplies the coordinates to map.

Return Value:

	The routine returns the mapped coordinates.

Environment:

	User mode.

--*/
{
	NWN::Vector3 Projected = World;
	NWN::Vector3 Screen;

	//
	// Transform to projected space.
	//

	Projected = Math::Multiply( m_ViewProj, Projected );

	//
	// Convert to screen space.
	//

	Screen.x = -((Projected.x + 1.0f) / 2.0f) * m_Width;
	Screen.y = -((Projected.y + 1.0f) / 2.0f) * m_Height;
	Screen.z = -(Projected.z * (m_MaxZ - m_MinZ) + m_MinZ);

	return Screen;
}

NWN::Vector2
Camera::WorldToScreenViewport(
	__in const NWN::Vector3 & World
	) const
/*++

Routine Description:

	This routine maps world-space coordinates to screen-space coordinates.

	The routine applies perspective based on the viewport parameters.

Arguments:

	World - Supplies the coordinates to map.

Return Value:

	The routine returns the mapped coordinates.

Environment:

	User mode.

--*/
{
	NWN::Vector3 Screen;
	NWN::Vector2 v;
	float        Perspective;

	//
	// Map to screen coordinates.
	//

	Screen = WorldToScreen( World );

	//
	// Apply perspective within the viewport parameters.
	//

	if (fabsf( Screen.z ) < Math::Epsilon)
		Screen.z = 1.0f;

	Perspective = (1 / Screen.z );

	//
	// Test for Screen.z < 0 to disallow mirroring when going under the world.
	//

#if !CAMERA_ENABLE_MIRRORING
	if (Screen.z < 0.0f)
#endif
	{
		Screen.x -= m_Viewport.x;
		Screen.y -= m_Viewport.y;
		Screen.x = ((Screen.x - m_HalfWidth ) * Perspective) + m_HalfWidth;
		Screen.y = ((Screen.y - m_HalfHeight) * Perspective) + m_HalfHeight;
		Screen.x += m_Viewport.x;
		Screen.y += m_Viewport.y;
	}
#if !CAMERA_ENABLE_MIRRORING
	else
	{
		Screen.x = -1.0f;
		Screen.y = -1.0f;
	}
#endif

	v.x = Screen.x;
	v.y = Screen.y;

	return v;
}

NWN::Vector3
Camera::Unproject(
	__in const NWN::Vector3 & Screen
	) const
/*++

Routine Description:

	This routine maps screen-space coordinates to world-space coordinates.

	In order to select a mouse position, the caller would typically not use
	this API directly, but rather perform a ray intersection with the aid of
	the GetPickRay API.

Arguments:

	World - Supplies the coordinates to map.

Return Value:

	The routine returns the mapped coordinates.

Environment:

	User mode.

--*/
{
	NWN::Vector3 v;

	v.x = +((2.0f * (Screen.x - m_Viewport.x)) / m_Width - 1.0f);
	v.y = +((2.0f * (Screen.y - m_Viewport.y)) / m_Height - 1.0f);
	v.z = +((Screen.z - m_MinZ) / (m_MaxZ - m_MinZ));

	Transform( m_InvViewProj, v );

	return v;
}

void
Camera::MoveCameraForward(
	__in float D
	)
/*++

Routine Description:

	This routine translates the camera forward or backward.

Arguments:

	D - Supplies the translational delta.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	m_Position = Math::Add( m_Position, Math::Multiply( m_LookAt, D ) );

	UpdateTransform( );
}

void
Camera::MoveCameraLeft(
	__in float D
	)
/*++

Routine Description:

	This routine translates the camera left or right.

Arguments:

	D - Supplies the translational delta.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	m_Position = Math::Subtract( m_Position, Math::Multiply( m_Right, D ) );

	UpdateTransform( );
}

void
Camera::MoveCameraUp(
	__in float D
	)
/*++

Routine Description:

	This routine translates the camera up or down.

Arguments:

	D - Supplies the translational delta.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	m_Position = Math::Add( m_Position, Math::Multiply( m_Up, D ) );

	UpdateTransform( );
}

void
Camera::RollCamera(
	__in float R
	)
/*++

Routine Description:

	This routine rolls the camera.

Arguments:

	R - Supplies the orientation delta (in radians).

Return Value:

	None.

Environment:

	User mode.

--*/
{
	NWN::Matrix44 M;

	Math::CreateRotationAxisMatrix( M, m_LookAt, -R );
	m_Right = Math::MultiplyNormal( M, m_Right );
	m_Up    = Math::MultiplyNormal( M, m_Up );

	UpdateTransform( );
}

void
Camera::TurnCameraUp(
	__in float R
	)
/*++

Routine Description:

	This routine rotates the camera up or down.

Arguments:

	R - Supplies the orientation delta (in radians).

Return Value:

	None.

Environment:

	User mode.

--*/
{
	NWN::Matrix44 M;

	Math::CreateRotationAxisMatrix( M, m_Right, -R );
	m_LookAt = Math::MultiplyNormal( M, m_LookAt );

	UpdateTransform( );
}

void
Camera::TurnCameraLeft(
	__in float R
	)
/*++

Routine Description:

	This routine rotates the camera left or right.

Arguments:

	R - Supplies the orientation delta (in radians).

Return Value:

	None.

Environment:

	User mode.

--*/
{
	NWN::Matrix44 M;

	Math::CreateRotationAxisMatrix( M, m_Up, -R );
	m_LookAt = Math::MultiplyNormal( M, m_LookAt );
	m_Right  = Math::MultiplyNormal( M, m_Right );

	UpdateTransform( );
}

void
Camera::GetPickRay(
	__in const NWN::Vector3 & ScreenPt,
	__out NWN::Vector3 & Origin,
	__out NWN::Vector3 & NormDir
	)
/*++

Routine Description:

	Given a set of screen coordinates, this routine generates a ray that may be
	drawn forward to check for an intersecting object.  Should an object be
	intersected, then the distance along the ray, from the ray origin, may be
	used to calculate the world-space coordinates of a viewed object.

Arguments:

	ScreenPt - Supplies the screen coordinates to map.

	Origin - Receives the origin of the ray to trace.

	NormDir - Receives the normalized direction of the ray to trace.

Return Value:

	None.  This routine does not fail.

Environment:

	User mode.

--*/
{
	NWN::Vector3 v;
	NWN::Vector3 Near;
	NWN::Vector3 Far;

	v.x  = ScreenPt.x;
	v.y  = ScreenPt.y;
	v.z  = m_MinZ;
	Near = Unproject( v );
	v.z  = m_MaxZ;
	Far  = Unproject( v );

//	Near = m_Position;

	NormDir   = Math::NormalizeVector( Math::Subtract( Far, Near ) );
	Origin    = Near;
}

void
Camera::UpdateTransform(
	__in UpdateTransformReason Reason /* = UpdateTrans */
	)
/*++

Routine Description:

	This routine is invoked when the projection or view matrix changes.  It is
	responsible for updating the mapping matrix in either direction to account
	for the change.

Arguments:

	Reason - Supplies why the transformation needed to be updated.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	//
	// Perform any preliminary coordinate adjustment tasks.
	//

	switch (Reason)
	{

	case UpdateTransAndUp:
		m_Up     = Math::NormalizeVector( m_Up );
		m_LookAt = Math::CrossProduct( m_Up, m_Right );
		break;

	case UpdateTransAndLookAt:
		m_LookAt = Math::NormalizeVector( m_LookAt );
		m_Right  = Math::CrossProduct( m_Up, m_LookAt );
		break;

	case UpdateTransAndRight:
		m_Right  = Math::NormalizeVector( m_Right );
		m_LookAt = Math::CrossProduct( m_Right, m_Up );
		break;

	}

	//
	// Build the view and projection matricies.
	//

	Math::CreateLookAtMatrixRH(
		m_View,
		m_Position,
		Math::Add( m_Position, m_LookAt ),
		m_Up);
	Math::CreatePerspectiveFovMatrixRH(
		m_Projection,
		m_FOV,
		m_AspectRatio,
		m_Near,
		m_Far);

	//
	// Cache the world to screen and inverse view matricies as well.
	//

	m_ViewProj    = Math::Multiply( m_View, m_Projection );
	m_InvView     = Math::Inverse( m_View );
	m_InvViewProj = Math::Inverse( m_ViewProj );

//	WriteText(
//		"World to screen:\n"
//		"[%03.3f %03.3f %03.3f %03.3f]\n"
//		"[%03.3f %03.3f %03.3f %03.3f]\n"
//		"[%03.3f %03.3f %03.3f %03.3f]\n"
//		"[%03.3f %03.3f %03.3f %03.3f]\n",
//		m_ViewProj._00, m_ViewProj._01, m_ViewProj._02, m_ViewProj._03,
//		m_ViewProj._10, m_ViewProj._11, m_ViewProj._12, m_ViewProj._13,
//		m_ViewProj._20, m_ViewProj._21, m_ViewProj._22, m_ViewProj._23,
//		m_ViewProj._30, m_ViewProj._31, m_ViewProj._32, m_ViewProj._33
//		);
}

float
Camera::Transform(
	__in const NWN::Matrix44 & M,
	__inout NWN::Vector3 & Pt
	) const
/*++

Routine Description:

	This routine applies a camera perspective transformation for a point,
	mapping from world to screen space.

Arguments:

	M - Supplies the view-projection matrix.

	Pt - Supplies the world coordinates to transform, and receives the mapped
	     screen coordinates on return.  'w' coordinates are assumed to be one
         for purposes of multiplying the vector with the Matrix44.

Return Value:

	The routine returns the clip-space w factor.

Environment:

	User mode.

--*/
{
	NWN::Vector3 TPt;
	float        w;

	TPt.x = Pt.x * M._00 + Pt.y * M._10 + Pt.z * M._20 + M._30;
	TPt.y = Pt.x * M._01 + Pt.y * M._11 + Pt.z * M._21 + M._31;
	TPt.z = Pt.x * M._02 + Pt.y * M._12 + Pt.z * M._22 + M._32;

	w     = Pt.x * M._03 + Pt.y * M._13 + Pt.z * M._23 + M._33;

	if (fabsf( w ) > Math::Epsilon)
	{
		TPt.x /= w;
		TPt.y /= w;
		TPt.z /= w;
	}

	Pt = TPt;

	return w;
}


/*
void GetPickRay(
	__in int x,
	__in int y,
	__in int w,
	__in int h
	)
{
	NWN::Matrix44 M;
	NWN::Vector3  Origin;
	NWN::Vector3  Dir;
	NWN::Vector3  v;
	NWN::Vector3  v2;


	M = Math::Inverse( m_ViewProj );

	v2.y = ((-2.0f * (h - y)) / h) - 1.0f;
	v.y  = v2.y * M._11;
	v.z  = v2.y * M._10;

	v2.x = ((2.0f * x) / (w)) - 1.0f; // xmm0

	Origin.x = (((1.0f) / ((M._13 * (v2.y) + (M._03 * (v2.x)) + (M._23 * 0.0f) + M._33)))) * (0.0f * M._20 + M._00 * v2.x + v.z + M._30);
	Origin.y = v.x;


}
*/
