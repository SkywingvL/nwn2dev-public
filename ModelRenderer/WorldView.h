/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	WorldView.h

Abstract:

	This module defines the simple world viewer class.

--*/

#ifndef _PROGRAMS_MODELRENDERER_WORLDVIEW_H
#define _PROGRAMS_MODELRENDERER_WORLDVIEW_H

#ifdef _MSC_VER
#pragma once
#endif

class WorldObject;
class ResourceManager;
class Camera;
struct IDebugTextOut;

class WorldView
{

public:
	
	typedef swutil::SharedPtr< WorldObject > WorldObjectPtr;

private:

	typedef std::vector< WorldObjectPtr > WorldObjectVec;
	typedef swutil::SharedPtr< Camera > CameraPtr;

public:

	WorldView(
		__in ResourceManager & ResMan,
		__in IDebugTextOut * TextWriter,
		__in_opt HWND Parent = NULL
		);

	~WorldView(
		);

	//
	// Add an object to the world, with the model and skeleton data loaded.
	//
	// The caller may position the object using the returned pointer.
	//

	WorldObjectPtr
	CreateWorldObject(
		__in const std::vector< std::string > & MDBResRefs,
		__in const std::string & GR2ResRef
		);
	//
	// Show or hide the window.
	//

	inline
	void
	Show(
		__in bool Show
		)
	{
		if (Show)
			ShowWindow( m_WorldWindow, SW_SHOW );
		else
			ShowWindow( m_WorldWindow, SW_HIDE );
	}

	//
	// Register or deregister window classes.
	//

	static
	bool
	RegisterClasses(
		__in bool Register
		);

private:

	//
	// Draw a wireframe for an object.
	//

	bool
	DrawWireframeMesh(
		__in HDC hdc,
		__in COLORREF Color,
		__in const WorldObject * Object
		);

	//
	// Draw a SimpleMesh-compatible wireframe mesh.
	//

	template< typename T >
	bool
	DrawSimpleWireframeMesh(
		__in HDC hdc,
		__in COLORREF Color,
		__in const T & Mesh,
		__in const WorldObject * Object,
		__in NWN::NWN2_BodyPieceSlot Slot = NWN::BPS_DEFAULT
		);

	//
	// Draw a SimpleMesh-compatible wireframe mesh.  The color and fill
	// attributes are assumed to come from the device context.
	//

	template< typename T >
	bool
	DrawSimpleWireframeMesh2(
		__in HDC hdc,
		__in const T & Mesh,
		__in const WorldObject * Object,
		__in NWN::NWN2_BodyPieceSlot Slot
		);

	//
	// Draw a SimpleMesh-compatible wireframe mesh.  The routine appends
	// points and polygon definitions into a point and polygon array; no
	// drawing is performed, only the collection of the mesh into a central
	// uniform location.
	//

	template< typename T >
	bool
	DrawSimpleWireframeMesh3(
		__inout std::vector< POINT > & Points,
		__inout std::vector< INT > & PolyCounts,
		__in const T & Mesh,
		__in const WorldObject * Object,
		__in NWN::NWN2_BodyPieceSlot Slot,
		__in bool Reserve
		);

	//
	// Draw the bones for an object.
	//

	bool
	DrawBones(
		__in HDC hdc,
		__in COLORREF Color,
		__in const WorldObject * Object,
		__in NWN::NWN2_BodyPieceSlot Slot = NWN::BPS_DEFAULT
		);

	//
	// Draw the bones for an object.  The color and fill attributes are
	// assumed to come from the device context.
	//

	bool
	DrawBones2(
		__in HDC hdc,
		__in const WorldObject * Object,
		__in NWN::NWN2_BodyPieceSlot Slot
		);

	//
	// Draw the bone for an object.  The routine appends points and polygon
	// definitions into a point and polygon array; no drawing is performed,
	// only the collection of the bones into a central uniform location.
	//

	bool
	DrawBones3(
		__inout std::vector< POINT > & Points,
		__inout std::vector< BYTE > & Types,
		__in const ModelSkeleton * Skeleton,
		__in const WorldObject * Object,
		__in NWN::NWN2_BodyPieceSlot Slot,
		__in bool Reserve
		);

	//
	// Convert game to client coordinates.
	//

	bool
	ClientToGame(
		__in const POINT &pt,
		__out NWN::Vector3 &v,
		__in bool Clip = true
		);

	//
	// Convert client to game coordinates (approximate).
	//

	bool
	GameToClient(
		__in const NWN::Vector3 &v,
		__out POINT &pt,
		__in bool Clip = true
		);

	//
	// Convert game coordinates to client coordinates.
	//

	bool
	GameToClient(
		__inout RECT & rc,
		__in bool Clip = true
		);

	//
	// Convert client to game coordinates (approximate).  This routine is
	// used to specify the correct clipping behavior for mesh drawing.
	//

	bool
	GameToClientMesh(
		__in const NWN::Vector3 &v,
		__out POINT &pt
		);

	//
	// Create (and initialize) the default camera.
	//

	void
	CreateCamera(
		);

	//
	// Set default camera parameters.
	//

	void
	SetDefaultCameraParameters(
		__in Camera * C
		);

	//
	// Calculate whether there exists a clear line of sight from a ray to the
	// edge of the map.
	//

	bool
	CalcLineOfSightRay(
		__in const NWN::Vector3 & Origin,
		__in const NWN::Vector3 & NormDir,
		__out float & Distance
		);

	//
	// Recalculate the map drawing region rectangle.
	//

	void
	RecalculateMapRect(
		__in const RECT * ClientRect
		);

	//
	// Control readjustment after a move.
	//

	void
	MoveControls(
		);

	//
	// Redraw just the world window.
	//

	inline
	void
	RedrawWorldWindowOnly(
		__in bool DrawNow
		)
	{
		UINT Flags;

		Flags = RDW_ERASE | RDW_INVALIDATE;

		if (DrawNow)
			Flags |= RDW_UPDATENOW;

		(VOID) RedrawWindow( m_WorldWindow, NULL, NULL, Flags );
	}

	inline
	bool
	IsCamera3D(
		) const
	{
		return m_Camera.get( ) != NULL;
	}

	//
	// Check if a key is down.
	//

	inline
	bool
	IsKeyDown(
		__in UINT vk
		) const
	{
		return GetKeyState( vk ) & 0x8000 ? true : false;
	}

	//
	// Drawing logic.
	//

	void
	DrawWorldWindow(
		__in HWND hwnd,
		__in PAINTSTRUCT & ps,
		__in const RECT & UpdateRect
		);

	//
	// Window procedure dispatchers.
	//

	static
	LRESULT
	CALLBACK
	WorldViewWindowProc_s(
		__in HWND hwnd,
		__in UINT uMsg,
		__in WPARAM wParam,
		__in LPARAM lParam
		);

	LRESULT
	WorldViewWindowProc(
		__in HWND hwnd,
		__in UINT uMsg,
		__in WPARAM wParam,
		__in LPARAM lParam
		);

	//
	// Window message handlers.
	//

	// WM_CLOSE
	void
	OnClose(
		__in HWND hwnd
		);

	// WM_DESTROY
	void
	OnDestroy(
		__in HWND hwnd
		);

	// WM_GETMINMAXINFO
	void
	OnGetMinMaxInfo(
		__in HWND hwnd,
		__inout LPMINMAXINFO lpMinMaxInfo
		);

	// WM_PAINT
	void
	OnPaint(
		__in HWND hwnd
		);

	// WM_PRINTCLIENT
	void
	OnPrintClient(
		__in HWND hwnd,
		__in HDC hdc,
		__in UINT DrawFlags
		);

	// WM_SIZE
	void
	OnSize(
		__in HWND hwnd,
		__in UINT state,
		__in int cx,
		__in int cy
		);

	// WM_MOVE
	void
	OnMove(
		__in HWND hwnd,
		__in int x,
		__in int y
		);

	// WM_KEYDOWN & WM_KEYUP
	void
	OnKey(
		__in HWND hwnd,
		__in UINT vk,
		__in BOOL fDown,
		__in int cRepeat,
		__in UINT flags
		);

	// WM_MOUSEMOVE
	void
	OnMouseMove(
		__in HWND hwnd,
		__in int x,
		__in int y,
		__in UINT keyFlags
		);

	// WM_MOUSEWHEEL
	void
	OnMouseWheel(
		__in HWND hwnd,
		__in int xPos,
		__in int yPos,
		__in int zDelta,
		__in UINT fwKeys
		);

	// WM_NOTIFY
	LRESULT
	OnNotify(
		__in HWND hwnd,
		__in int idFrom,
		__inout NMHDR * pnmhdr
		);

	enum
	{
		PIXELS_X_PER_OBJ = 1, // Pixel multiplication
		PIXELS_Y_PER_OBJ = 1,

		EXTRA_X_PIXELS   = 4, // Extra pixels around the target.
		EXTRA_Y_PIXELS   = 4,

		HIT_TEST_DISTANCE = 8 // Hit test distance for object clicks.
	};

	//
	// Instance variables.
	//

	ResourceManager & m_ResMan;
	IDebugTextOut   * m_TextWriter;
	RECT              m_MapRect;
	RECT              m_ClientRect;
	HWND              m_WorldWindow;
	CameraPtr         m_Camera;
	float             m_AreaWidth;
	float             m_AreaHeight;
	float             m_OriginX;
	float             m_OriginY;
	float             m_PaddingX;
	float             m_PaddingY;
	int               m_CursorX;
	int               m_CursorY;
	POINT             m_CameraRotateDelta;

	//
	// Active objects being drawn.
	//

	WorldObjectVec    m_WorldObjects;

};

#endif
