/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	Camera.h

Abstract:

	This module defines the Camera object, which houses a coordinate
	transformation system used to map between viewing and world space.

--*/

#ifndef _PROGRAMS_NWN2REPORTLOG_CAMERA_H
#define _PROGRAMS_NWN2REPORTLOG_CAMERA_H

#ifdef _MSC_VER
#pragma once
#endif

class Camera
{

	enum UpdateTransformReason
	{
		UpdateTrans,
		UpdateTransAndUp,
		UpdateTransAndLookAt,
		UpdateTransAndRight,

		LastUpdateTransReason
	};

public:

	//
	// Constructor.
	//

	Camera(
		__in float FOV = PI / 4,
		__in float Near = 0.1000000015f,
		__in float Far = 140.0f,
		__in float Aspect = 1.0f,
		__in float Width = 500.0f,
		__in float Height = 500.0f
		);

	//
	// Destructor.
	//

	virtual
	~Camera(
		);

	//
	// Configuration parameters.
	//

	inline
	void
	SetCameraSize(
		__in float Width,
		__in float Height
		)
	{
		if (Width < 1e-7)
			Width = 1.0f;
		if (Height < 1e-7)
			Height = 1.0f;

		m_Width       = Width;
		m_Height      = Height;
		m_HalfWidth   = Width / 2.0f;
		m_HalfHeight  = Height / 2.0f;
		m_AspectRatio = (Width / Height);

		UpdateTransform( );
	}

	inline
	void
	SetFOV(
		__in float FOV
		)
	{
		m_FOV = FOV;

		UpdateTransform( );
	}

	inline
	float
	GetFOV(
		) const
	{
		return m_FOV;
	}

	inline
	void
	SetNearFar(
		__in float Near,
		__in float Far
		)
	{
		m_Near = Near;
		m_Far  = Far;

		UpdateTransform( );
	}

	inline
	void
	SetMinMaxZ(
		__in float MinZ,
		__in float MaxZ
		)
	{
		m_MinZ = MinZ;
		m_MaxZ = MaxZ;
	}

	//
	// Orientation parameters.
	//

	inline
	void
	SetCameraPosition(
		__in const NWN::Vector3 & v
		)
	{
		m_Position = v;

		UpdateTransform( );
	}

	inline
	const NWN::Vector3 &
	GetCameraPosition(
		) const
	{
		return m_Position;
	}

	inline
	void
	SetCameraRight(
		__in const NWN::Vector3 & Right
		)
	{
		m_Right = Right;

		UpdateTransform( UpdateTransAndRight );
	}

	inline
	const NWN::Vector3 &
	GetCameraRight(
		) const
	{
		return m_Right;
	}

	inline
	void
	SetCameraUp(
		__in const NWN::Vector3 & Up
		)
	{
		m_Up = Up;

		UpdateTransform( UpdateTransAndUp );
	}

	inline
	const NWN::Vector3 &
	GetCameraUp(
		) const
	{
		return m_Up;
	}

	inline
	void
	SetCameraLookAt(
		__in const NWN::Vector3 & v
		)
	{
		m_LookAt = v;

		UpdateTransform( UpdateTransAndLookAt );
	}

	inline
	const NWN::Vector3 &
	GetCameraLookAt(
		) const
	{
		return m_LookAt;
	}

	inline
	float
	GetNear(
		) const
	{
		return m_Near;
	}

	inline
	float
	GetFar(
		) const
	{
		return m_Far;
	}

	inline
	float
	GetMinZ(
		) const
	{
		return m_MinZ;
	}

	inline
	float
	GetMaxZ(
		) const
	{
		return m_MaxZ;
	}

	//
	// Viewport origin.
	//

	inline
	void
	SetViewport(
		__in const NWN::Vector2 Viewport
		)
	{
		m_Viewport = Viewport;
	}

	inline
	const NWN::Vector2
	GetViewport(
		) const
	{
		return m_Viewport;
	}

	//
	// Simple position and orientation adjustment.
	//

	void
	MoveCameraForward(
		__in float D
		);

	void
	MoveCameraLeft(
		__in float D
		);

	void
	MoveCameraUp(
		__in float D
		);

	void
	RollCamera(
		__in float R
		);

	void
	TurnCameraUp(
		__in float R
		);

	void
	TurnCameraLeft(
		__in float R
		);

	//
	// Coordinate transformation.
	//

	//
	// Transform coordinates to screen space.
	//

	NWN::Vector3
	WorldToScreen(
		__in const NWN::Vector3 & World
		) const;

	//
	// Transform coordinates to screen space, with perspective already applied
	// with respect to the given viewport parameters.
	//

	NWN::Vector2
	WorldToScreenViewport(
		__in const NWN::Vector3 & World
		) const;

	//
	// Unproject screen coordinates back to world coordinates.
	//

	NWN::Vector3
	Unproject(
		__in const NWN::Vector3 & Screen
		) const;

	//
	// Calculate a pick ray for use to map screen coordinates to an object on
	// screen (or a terrain face).
	//

	void
	GetPickRay(
		__in const NWN::Vector3 & ScreenPt,
		__out NWN::Vector3 & Origin,
		__out NWN::Vector3 & NormDir
		);

private:

	//
	// Calculate cached matrix transforms.
	//

	void
	UpdateTransform(
		__in UpdateTransformReason Reason = UpdateTrans
		);

	//
	// Apply view-perspective transformation.
	//

	float
	Transform(
		__in const NWN::Matrix44 & M,
		__inout NWN::Vector3 & Pt
		) const;

	NWN::Matrix44 m_View;
	NWN::Matrix44 m_Projection;

	NWN::Matrix44 m_ViewProj;
	NWN::Matrix44 m_InvViewProj;
	NWN::Matrix44 m_InvView;

	NWN::Vector3  m_Position;
	NWN::Vector3  m_Up;
	NWN::Vector3  m_LookAt;
	NWN::Vector3  m_Right;

	float         m_FOV;
	float         m_Near;
	float         m_Far;
	float         m_AspectRatio;
	float         m_Width;
	float         m_Height;
	float         m_HalfWidth;
	float         m_HalfHeight;
	float         m_MinZ;
	float         m_MaxZ;
	NWN::Vector2  m_Viewport;

};

#endif
