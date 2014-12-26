/**********************************************************************
*<
FILE: GR2Format.h

DESCRIPTION:	GR2 File Format

CREATED BY: tazpn (Theo)

HISTORY:

*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/

#ifndef __GR2FORMAT_H__
#define __GR2FORMAT_H__

#include "DataHeader.h"
#include "NWN2Format.h"

// #include <pshpack1.h>

namespace granny2 {

typedef signed char		granny_int8;
typedef unsigned char	granny_uint8;
typedef signed short	granny_int16;
typedef unsigned short	granny_uint16;
typedef signed int		granny_int32;
typedef unsigned int	granny_uint32;
typedef signed __int64	granny_int64;
typedef unsigned __int64 granny_uint64;

typedef float			granny_real32;
typedef double			granny_real64;

typedef Point3			granny_point3;

ROOTVARTYPE(Text)

DEFVARTYPE(granny2_impl, granny_transform)
DEFVARTYPE(granny2_impl, ArtToolInfo)
DEFVARTYPE(granny2_impl, ExporterInfo)
DEFVARTYPE(granny2_impl, MIPLevels)
DEFVARTYPE(granny2_impl, Image)
DEFVARTYPE(granny2_impl, Texture)
DEFVARTYPE(granny2_impl, Layout)
DEFVARTYPE(granny2_impl, granny_Map)
DEFVARTYPE(granny2_impl, granny_Material)
DEFVARTYPE(granny2_impl, Bone)
DEFVARTYPE(granny2_impl, Skeleton)
DEFVARTYPE(granny2_impl, VertexData)
DEFVARTYPE(granny2_impl, TriTopology)
DEFVARTYPE(granny2_impl, Mesh)
DEFVARTYPE(granny2_impl, Model)
DEFVARTYPE(granny2_impl, CurveData)
DEFVARTYPE(granny2_impl, TransformTrack)
DEFVARTYPE(granny2_impl, PropRef)
DEFVARTYPE(granny2_impl, Property)
DEFVARTYPE(granny2_impl, ExtendedData)
DEFVARTYPE(granny2_impl, TrackGroup)
DEFVARTYPE(granny2_impl, Animation)
DEFVARTYPE(granny2_impl, GR2Info)
DEFVARTYPE(granny2_impl, CurveDataHeader_D3K16uC16u)
DEFVARTYPE(granny2_impl, CurveDataHeader_D3K8uC8u)
DEFVARTYPE(granny2_impl, CurveDataHeader_D4nK16uC15u)
DEFVARTYPE(granny2_impl, CurveDataHeader_DaIdentity)
DEFVARTYPE(granny2_impl, CurveDataHeader_D4nK8uC7u)
DEFVARTYPE(granny2_impl, CurveDataHeader_D3Constant32f)
DEFVARTYPE(granny2_impl, CurveDataHeader_DaK32fC32f)
DEFVARTYPE(granny2_impl, CurveDataHeader_DaKeyframes32f)
//DEFVARTYPE(granny2_impl, CurveDataHeader_DaConstant32f)
//DEFVARTYPE(granny2_impl, CurveDataHeader_D4Constant32f)
//DEFVARTYPE(granny2_impl, CurveDataHeader_DaK16uC16u)
//DEFVARTYPE(granny2_impl, CurveDataHeader_DaK8uC8u)

#pragma region GR2Array
/////////////////////////////////////
// GR2Array
template <typename T>
class GR2Array : public NWArray<T>
{
public:
	GR2Array() { }
	GR2Array(int length) : NWArray<T>(length) {}
	GR2Array(int capacity, int length) : NWArray<T>(capacity, length) { }
	GR2Array(const GR2Array& other) : NWArray<T>( other ) {}

	//! Default Destructor
	~GR2Array() {}
};

//////////////////////////////////////////////////////////////////////////
#pragma region GR2Array
template<typename T>
bool Recall(NWStream& in, GR2Array<T>& val) 
{
   bool ok = true;
   int cur = in.Tell();
   int len = 0, off = 0;
   if (ok) ok = Recall(in, len);
   if (ok) ok = Recall(in, off);
   if (ok && off != 0)
   {
//	   ASSERT(len <= 0x65535);
	   val.resize(len);
	   if (len > 0)
	   {
		   long cur = in.Tell();
		   in.Seek(SEEK_SET, off);
		   for (int i=0; i<len && ok; ++i) {
			   ok = Recall(in, val[i]);
		   }
		   in.Seek(SEEK_SET, cur);
	   }
   }
   return ok;
}

#if GRANNY2LIB_ENABLE_STORE
template<typename T>
bool Store(NWStream& out, GR2Array<T> const & val) 
{
	bool ok = true;
	throw std::runtime_error("Not finished");
	for (size_t i=0, n=val.count(); i<n && ok; ++i) {
		ok = Store(out, val[i]);
	}
	return ok;
}
#endif

template<typename T>
bool Recall(NWStream& in, ValuePtr<T>& val) 
{
   bool ok = true;
   int off = 0;
   if (ok) ok = Recall(in, off);
   if (ok && off != 0)
   {
	   long cur = in.Tell();
	   in.Seek(SEEK_SET, off);
	   ok = Recall(in, *val);
	   in.Seek(SEEK_SET, cur);
   }
   return ok;
}

#if GRANNY2LIB_ENABLE_STORE
template<typename T>
bool Store(NWStream& out, ValuePtr<T> const & val) 
{
   bool ok = true;
   throw std::runtime_error("Not finished");
   return ok;
}
#endif

#if GRANNY2LIB_ENABLE_DUMP
template<typename T> 
void Dump(NWDumpStream& out, LPCSTR name, GR2Array<T> const & val) 
{
	if (val.size() == 0)
		out.Indent(), out.PrintF("%s: (size=%d)\n", name, val.size());
	else
		DumpArray(out, name, &val[0], val.count());
}

template<typename T> 
void Dump(NWDumpStream& out, LPCSTR name, ValuePtr<T> const & val) 
{
	out.Indent();
	if (val.isNull()) {
		out.PrintF("%s *: <NULL>\n", name);
	} else {
		out.PrintF("%s *:\n", name);
		++out.IndentLevel;
		Dump(out, name, *val);
		--out.IndentLevel;
	}
}
#endif
#pragma endregion

namespace granny2_impl {

//////////////////////////////////////////////////////////////////////////
struct ExtendedData
{
	GR2Array< Property > Properties;
};

//////////////////////////////////////////////////////////////////

struct granny_transform
{
	DWORD Dimensions; // Flags
	Point3 Origin;
	Quat Rotation;
	RHMatrix3 Scale; // ScaleShear

BEGIN_INI_MAP(granny_transform)
	ADDCLASS(Dimensions)
	ADDCLASS(Origin)
	ADDCLASS(Rotation)
	ADDCLASS(Scale)
END_INI_MAP()
};

//////////////////////////////////////////////////////////////////////////
struct ArtToolInfo
{
   Text FromArtToolName;
   granny_int32 ArtToolMajorRevision;
   granny_int32 ArtToolMinorRevision;
   granny_real32 UnitsPerMeter;
   granny_point3 Origin;
   RHMatrix3 Rotation;

BEGIN_INI_MAP(ArtToolInfo)
	ADDCLASS(FromArtToolName)
	ADDCLASS(ArtToolMajorRevision)
	ADDCLASS(ArtToolMinorRevision)
	ADDCLASS(UnitsPerMeter)
	ADDCLASS(Origin)
	ADDCLASS(Rotation)
END_INI_MAP()
};

struct ExporterInfo
{
   Text ExporterName;
   granny_int32 ExporterMajorRevision;
   granny_int32 ExporterMinorRevision;
   granny_int32 ExporterCustomization;
   granny_int32 ExporterBuildNumber;

BEGIN_INI_MAP(ExporterInfo)
	ADDCLASS(ExporterName)
	ADDCLASS(ExporterMajorRevision)
	ADDCLASS(ExporterMinorRevision)
	ADDCLASS(ExporterCustomization)
	ADDCLASS(ExporterBuildNumber)
END_INI_MAP()
};
//////////////////////////////////////////////////////////////////////////
struct MIPLevels
{
	granny_int32 Stride;
	GR2Array<granny_uint8>	PixelBytes;

BEGIN_INI_MAP(MIPLevels)
	ADDCLASS(Stride)
	ADDCLASS(PixelBytes)
END_INI_MAP()
};
//////////////////////////////////////////////////////////////////////////
struct Image
{
	GR2Array< MIPLevels >	MIPLevels;

BEGIN_INI_MAP(Image)
	//ADDITEM (Unk0,		(granny_int32)0x0100)
	//ADDITEM (Unk1,		(granny_int32)0)
	//ADDITEM (Unk2,		(granny_int32)0)
	ADDCLASS(MIPLevels)
END_INI_MAP()
};
//////////////////////////////////////////////////////////////////////////
struct Layout
{
	granny_int32			BytesPerPixel;
	array<4, granny_int32>	ShiftForComponent;
	array<4, granny_int32>	BitsForComponent;

BEGIN_INI_MAP(Layout)
	ADDCLASS(BytesPerPixel)
	ADDCLASS(ShiftForComponent)
	ADDCLASS(BitsForComponent)
END_INI_MAP()
};
//////////////////////////////////////////////////////////////////////////
struct Texture
{
	Text			FromFileName;
	granny_int32	TextureType;
	granny_int32	Width;
	granny_int32	Height;
	granny_int32	Encoding;
	granny_int32	SubFormat;
	Layout			Layout;
	GR2Array< Image >	Images;
	ExtendedData	ExtendedData;

BEGIN_INI_MAP(Texture)
	ADDCLASS(FromFileName)
	ADDCLASS(TextureType)
	ADDCLASS(Width)
	ADDCLASS(Height)
	ADDCLASS(Encoding)
	ADDCLASS(SubFormat)
	ADDCLASS(Layout)
	ADDCLASS(Images)
	ADDCLASS(ExtendedData)
END_INI_MAP()
};

//////////////////////////////////////////////////////////////////////////
struct granny_Map
{
	Text Usage;
	ValuePtr<granny_Material> Map;

BEGIN_INI_MAP(granny_Map)
	ADDCLASS(Usage)
	ADDCLASS(Map)
END_INI_MAP()
};

//////////////////////////////////////////////////////////////////////////
struct granny_Material
{
	Text Name;
	GR2Array< granny_Map > Maps;
	ValuePtr<Texture> Texture;
	ExtendedData ExtendedData;
BEGIN_INI_MAP(granny_Material)
	ADDCLASS(Name)
	ADDCLASS(Maps)
	ADDCLASS(Texture)
	ADDCLASS(ExtendedData)
END_INI_MAP()
};

//////////////////////////////////////////////////////////////////////////
struct Bone
{
   Text Name;
   granny_int32 ParentIndex;
   granny_transform Transform;
   RHMatrix4 InverseWorldTransform;
   granny_real32 LODError;
   ExtendedData		ExtendedData;
   granny_int32 Unk1;
   //granny_int32 UserDefinedPropertiesDef;
   //granny_int32 UserDefinedPropertiesData;

BEGIN_INI_MAP(Bone)
	ADDCLASS(Name)
	ADDCLASS(ParentIndex)
	ADDCLASS(Transform)
	ADDCLASS(InverseWorldTransform)
	ADDCLASS(LODError)
	ADDCLASS(Unk1)
	ADDCLASS(ExtendedData)
	//ADDCLASS(UserDefinedPropertiesDef)
	//ADDCLASS(UserDefinedPropertiesData)
END_INI_MAP()
};

//////////////////////////////////////////////////////////////////////////
struct Skeleton
{
   Text Name;
   GR2Array<Bone> Bones;

BEGIN_INI_MAP(Skeleton)
	ADDCLASS(Name)
	ADDCLASS(Bones)
END_INI_MAP()
};

//////////////////////////////////////////////////////////////////////////
struct VertexData
{

BEGIN_INI_MAP(VertexData)
END_INI_MAP()
};

//////////////////////////////////////////////////////////////////////////
struct TriTopology
{

BEGIN_INI_MAP(TriTopology)
END_INI_MAP()
};

//////////////////////////////////////////////////////////////////////////

struct Mesh
{

BEGIN_INI_MAP(Mesh)
END_INI_MAP()
};

//////////////////////////////////////////////////////////////////////////

struct Model
{
BEGIN_INI_MAP(Model)
END_INI_MAP()
};

//////////////////////////////////////////////////////////////////////////
enum KeyType
{
	NO_KEY = 0,
	LINEAR_KEY = 1, /*!< Use linear interpolation. */
	QUADRATIC_KEY = 2, /*!< Use quadratic interpolation.  Forward and back tangents will be stored. */
	TBC_KEY = 3, /*!< Use Tension Bias Continuity interpolation.  Tension, bias, and continuity will be stored. */
	XYZ_ROTATION_KEY = 4, /*!< For use only with rotation data.  Separate X, Y, and Z keys will be stored instead of using quaternions. */
};

struct CurveData
{
	granny_uint8 Format;
	granny_uint8 Degree;

	virtual KeyType GetKeyType() const { return NO_KEY; }
};
//DEFVARTYPE(ValuePtr<CurveData>)
}

template<> bool granny2::Recall(NWStream& in, ValuePtr<granny2_impl::CurveData>& val);
#if GRANNY2LIB_ENABLE_STORE
template<> bool granny2::Store(NWStream& out, ValuePtr<granny2_impl::CurveData> const & val);
#endif
#if GRANNY2LIB_ENABLE_DUMP
template<> void granny2::Dump(NWDumpStream& out, LPCSTR name, ValuePtr<granny2_impl::CurveData> const & val);
#endif


namespace granny2_impl {

struct Point3CurveData : public CurveData
{
	virtual KeyType GetKeyType() const { return QUADRATIC_KEY; }
	virtual GR2Array<granny_real32> GetKnots() const { return GR2Array<granny_real32>(); }
	virtual GR2Array<Point3> GetPoint3Data() const { return GR2Array<Point3>(); }
};

struct RotationCurveData : public Point3CurveData
{
	virtual KeyType GetKeyType() const { return QUADRATIC_KEY; }
	virtual GR2Array<granny_real32> GetKnots() const { return GR2Array<granny_real32>(); }
	virtual GR2Array<Quat> GetQuatData() const { return GR2Array<Quat>(); }
	virtual GR2Array<Point3> GetPoint3Data() const { return GR2Array<Point3>(); }
};

struct FloatCurveData : public CurveData
{
	virtual GR2Array<granny_real32> GetKnots() const { return GR2Array<granny_real32>(); }
	virtual GR2Array<granny_real32> GetFloatData() const { return GR2Array<granny_real32>(); }
};

struct Matrix3CurveData : public RotationCurveData
{
	virtual GR2Array<granny_real32> GetKnots() const { return GR2Array<granny_real32>(); }
	virtual KeyType GetKeyType() const { return QUADRATIC_KEY; }
	virtual GR2Array<Quat> GetQuatData() const { return GR2Array<Quat>(); }
	virtual GR2Array<Point3> GetPoint3Data() const { return GR2Array<Point3>(); }
	virtual GR2Array<Matrix3> GetMatrix3Data() const { return GR2Array<Matrix3>(); }
};

//////////////////////////////////////////////////////////////////////////
struct TransformTrack
{
	Text Name;
	//int Flags;
	granny_int32		PosFlags;
	ValuePtr<CurveData> PositionCurve;
	granny_int32		RotFlags;
	ValuePtr<CurveData> OrientationCurve;
	granny_int32		SclFlags;
	ValuePtr<CurveData> ScaleShearCurve;

BEGIN_INI_MAP(TransformTrack)
	ADDCLASS(Name)
	//ADDCLASS(Flags)
	ADDCLASS(PosFlags)
	ADDCLASS(PositionCurve)
	ADDCLASS(RotFlags)
	ADDCLASS(OrientationCurve)
	ADDCLASS(SclFlags)
	ADDCLASS(ScaleShearCurve)
END_INI_MAP()
};
//////////////////////////////////////////////////////////////////////////
enum PropertyType
{
	pt_none   = 0x00,
	pt_pointer= 0x03,
	pt_text	  = 0x08,
	pt_real32 = 0x0A,
	pt_int32  = 0x13,
};
extern const EnumLookupType PropertyTypes[];
}

#if GRANNY2LIB_ENABLE_DUMP
template<> void granny2::Dump(NWDumpStream& out, LPCSTR name, granny2_impl::PropertyType const & val);
#endif

namespace granny2_impl {

//////////////////////////////////////////////////////////////////////////
struct PropRef
{
	PropertyType			Type;
	Text					Name;
	array<14, granny_int32>	Unk1;

BEGIN_INI_MAP(PropRef)
	ADDITEM (Type,  pt_none)
	ADDCLASS(Name)
	ADDCLASS(Unk1)
END_INI_MAP()
};


//////////////////////////////////////////////////////////////////////////
struct Property
{
	Property();
	~Property();
	LPVOID AllocData();
	void FreeData();
	bool RecallData(NWStream& in);
	bool SaveData(NWStream& out);

	Text			Name;
	PropertyType	Type;
	LPVOID			lpValue;
	ValuePtr<PropRef> RefInfo;
	granny_int32	Len;
	granny_int32	Unk2;
	granny_int32	Unk3;
	granny_int32	Unk4;
	granny_int32	Unk5;

BEGIN_INI_MAP_NOINIT(Property)
	ADDITEM (Type,  pt_none)
	ADDCLASS(Name)
	ADDCLASS(RefInfo)
	ADDITEM (Len,	(granny_int32)0)
	ADDITEM (Unk2,	(granny_int32)0)
	ADDITEM (Unk3,	(granny_int32)0)
	ADDITEM (Unk4,	(granny_int32)0)
	ADDITEM (Unk5,	(granny_int32)0)
END_INI_MAP()
};


//////////////////////////////////////////////////////////////////////////
struct TrackGroup
{
	Text						Name;
	GR2Array<Point3>			VectorTracks;
	GR2Array<TransformTrack>	TransformTracks;
	GR2Array<granny_real32>		TransformLODErrors;
	GR2Array<Text>				TextTracks;
	granny_transform			InitialPlacement;
	granny_int32				AccumulationFlags;
	granny_point3				LoopTranslation;
	VoidPtr						PeriodLoop;
	VoidPtr						RootMotion;
	ExtendedData				ExtendedData;

BEGIN_INI_MAP(TrackGroup)
	ADDCLASS(Name)
	ADDCLASS(VectorTracks)
	ADDCLASS(TransformTracks)
	ADDCLASS(TransformLODErrors)
	ADDCLASS(TextTracks)
	ADDCLASS(InitialPlacement)
	ADDCLASS(AccumulationFlags)
	ADDCLASS(LoopTranslation)
	ADDCLASS(PeriodLoop)
	ADDCLASS(RootMotion)
	ADDCLASS(ExtendedData)
END_INI_MAP()
};

//////////////////////////////////////////////////////////////////////////
struct Animation
{
	Text Name;
	granny_real32 Duration;
	granny_real32 TimeStep;
	granny_real32 Oversampling;
	GR2Array< ValuePtr<TrackGroup> > TrackGroups;
BEGIN_INI_MAP(Animation)
	ADDCLASS(Name)
	ADDCLASS(Duration)
	ADDCLASS(TimeStep)
	ADDCLASS(Oversampling)
	ADDCLASS(TrackGroups)
END_INI_MAP()
};

//////////////////////////////////////////////////////////////////////////
struct GR2Info
{
   ValuePtr<ArtToolInfo>				artInfo;
   ValuePtr<ExporterInfo>				expInfo;
   Text									FromFileName;
   GR2Array< ValuePtr<Texture> >		Textures;
   GR2Array< ValuePtr<granny_Material> > Materials;
   GR2Array< ValuePtr<Skeleton> >		Skeletons;
   GR2Array< ValuePtr<VertexData> >		VertexDatas;
   GR2Array< ValuePtr<TriTopology> >	TriTopologies;
   GR2Array< ValuePtr<Mesh> >			Meshes;
   GR2Array< ValuePtr<Model> >			Models;
   GR2Array< ValuePtr<TrackGroup> >		TrackGroups;
   GR2Array< ValuePtr<Animation> >		Animations;
   ExtendedData							ExtendedData;

BEGIN_INI_MAP(GR2Info)
	ADDCLASS(artInfo)
	ADDCLASS(expInfo)
	ADDCLASS(FromFileName)
	ADDCLASS(Textures)
	ADDCLASS(Materials)
	ADDCLASS(Skeletons)
	ADDCLASS(VertexDatas)
	ADDCLASS(TriTopologies)
	ADDCLASS(Meshes)
	ADDCLASS(Models)
	ADDCLASS(TrackGroups)
	ADDCLASS(Animations)
	ADDCLASS(ExtendedData)
END_INI_MAP()
};

//////////////////////////////////////////////////////////////////


struct CurveDataHeader_D3K16uC16u : public Point3CurveData
{
	granny_uint16 OneOverKnotScaleTrunc;
	granny_point3 ControlScales;
	granny_point3 ControlOffsets;
	GR2Array<granny_uint16> KnotsControls;

	virtual GR2Array<granny_real32> GetKnots() const;
	virtual GR2Array<Point3> GetPoint3Data() const;

BEGIN_INI_MAP(CurveDataHeader_D3K16uC16u)
	ADDCLASS(Format)
	ADDCLASS(Degree)
	ADDCLASS(OneOverKnotScaleTrunc)
	ADDCLASS(ControlScales)
	ADDCLASS(ControlOffsets)
	ADDCLASS(KnotsControls)
END_INI_MAP()
};

struct CurveDataHeader_D3K8uC8u : public Point3CurveData
{
	granny_uint16 OneOverKnotScaleTrunc;
	Point3 ControlScales;
	Point3 ControlOffsets;
	GR2Array<granny_uint8> KnotsControls;

	virtual GR2Array<granny_real32> GetKnots() const;
	virtual GR2Array<Point3> GetPoint3Data() const;

BEGIN_INI_MAP(CurveDataHeader_D3K8uC8u)
	ADDCLASS(Format)
	ADDCLASS(Degree)
	ADDCLASS(OneOverKnotScaleTrunc)
	ADDCLASS(ControlScales)
	ADDCLASS(ControlOffsets)
	ADDCLASS(KnotsControls)
END_INI_MAP()
};

struct CurveDataHeader_D4nK16uC15u : public RotationCurveData
{
	granny_int16 ScaleOffsetTableEntries;
	granny_real32 OneOverKnotScale;
	GR2Array<granny_uint16> KnotControls;

	virtual KeyType GetKeyType() const;
	virtual GR2Array<granny_real32> GetKnots() const;
	virtual GR2Array<Quat> GetQuatData() const;
	virtual GR2Array<Point3> GetPoint3Data() const;

BEGIN_INI_MAP(CurveDataHeader_D4nK16uC15u)
	ADDCLASS(Format)
	ADDCLASS(Degree)
	ADDCLASS(ScaleOffsetTableEntries)
	ADDCLASS(OneOverKnotScale)
	ADDCLASS(KnotControls)
END_INI_MAP()
};

struct CurveDataHeader_DaIdentity : public Matrix3CurveData
{
	granny_int16 Dimension;

BEGIN_INI_MAP(CurveDataHeader_DaIdentity)
	ADDCLASS(Format)
	ADDCLASS(Degree)
	ADDCLASS(Dimension)
END_INI_MAP()
};

struct CurveDataHeader_D4nK8uC7u : public RotationCurveData
{
	granny_int16 ScaleOffsetTableEntries;
	granny_real32 OneOverKnotScale;
	GR2Array<byte> KnotsControls;

	virtual KeyType GetKeyType() const;
	virtual GR2Array<granny_real32> GetKnots() const;
	virtual GR2Array<Quat> GetQuatData() const;
	virtual GR2Array<Point3> GetPoint3Data() const;

BEGIN_INI_MAP(CurveDataHeader_D4nK8uC7u)
	ADDCLASS(Format)
	ADDCLASS(Degree)
	ADDCLASS(ScaleOffsetTableEntries)
	ADDCLASS(OneOverKnotScale)
	ADDCLASS(KnotsControls)
END_INI_MAP()
};
//////////////////////////////////////////////////////////////////////////
struct CurveDataHeader_D3Constant32f : public Point3CurveData
{
	granny_int16 Padding;
	Point3 Controls;

	virtual GR2Array<granny_real32> GetKnots() const;
	virtual GR2Array<Point3> GetPoint3Data() const;

BEGIN_INI_MAP(CurveDataHeader_D3Constant32f)
	ADDCLASS(Format)
	ADDCLASS(Degree)
	ADDCLASS(Padding)
	ADDCLASS(Controls)
END_INI_MAP()
};

//////////////////////////////////////////////////////////////////////////
struct CurveDataHeader_DaK32fC32f : public Matrix3CurveData
{
	granny_int16 Padding;
	GR2Array<granny_real32> Knots;
	GR2Array<granny_real32> Controls;

	virtual KeyType GetKeyType() const;
	virtual GR2Array<granny_real32> GetKnots() const;
	virtual GR2Array<Quat> GetQuatData() const;
	virtual GR2Array<Point3> GetPoint3Data() const;
	virtual GR2Array<Matrix3> GetMatrix3Data() const;


BEGIN_INI_MAP(CurveDataHeader_DaK32fC32f)
	ADDCLASS(Format)
	ADDCLASS(Degree)
	ADDCLASS(Padding)
	ADDCLASS(Knots)
	ADDCLASS(Controls)
END_INI_MAP()
};
//////////////////////////////////////////////////////////////////////////
struct CurveDataHeader_DaKeyframes32f : public Matrix3CurveData
{
	granny_int16 Dimension;
	GR2Array<granny_real32> Controls;

	virtual KeyType GetKeyType() const;
	virtual GR2Array<granny_real32> GetKnots() const;
	virtual GR2Array<Quat> GetQuatData() const;
	virtual GR2Array<Point3> GetPoint3Data() const;
	virtual GR2Array<Matrix3> GetMatrix3Data() const;


	BEGIN_INI_MAP(CurveDataHeader_DaKeyframes32f)
		ADDCLASS(Format)
		ADDCLASS(Degree)
		ADDCLASS(Dimension)
		ADDCLASS(Controls)
	END_INI_MAP()
};

}
}
// #include <poppack.h>

#endif
