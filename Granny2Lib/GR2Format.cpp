/**********************************************************************
*<
FILE: GR2Format.h

DESCRIPTION:	GR2 File Format

CREATED BY: tazpn (Theo)

HISTORY:

*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/
#include "Precomp.h"
#include "GR2Format.h"
//#include "bspline/BSpline.h"

using namespace std;

namespace granny2 {

IMPLVARTYPE(granny2_impl, granny_transform)
IMPLVARTYPE(granny2_impl, ArtToolInfo)
IMPLVARTYPE(granny2_impl, ExporterInfo)
IMPLVARTYPE(granny2_impl, MIPLevels)
IMPLVARTYPE(granny2_impl, Image)
IMPLVARTYPE(granny2_impl, Layout)
IMPLVARTYPE(granny2_impl, Texture)
IMPLVARTYPE(granny2_impl, granny_Map)
IMPLVARTYPE(granny2_impl, granny_Material)
IMPLVARTYPE(granny2_impl, Bone)
IMPLVARTYPE(granny2_impl, Skeleton)
IMPLVARTYPE(granny2_impl, VertexData)
IMPLVARTYPE(granny2_impl, TriTopology)
IMPLVARTYPE(granny2_impl, Mesh)
IMPLVARTYPE(granny2_impl, Model)
IMPLVARTYPE(granny2_impl, TransformTrack)
IMPLVARTYPE(granny2_impl, PropRef)
IMPLVARTYPE(granny2_impl, TrackGroup)
IMPLVARTYPE(granny2_impl, Animation)
IMPLVARTYPE(granny2_impl, GR2Info)
IMPLVARTYPE(granny2_impl, CurveDataHeader_D3K16uC16u)
IMPLVARTYPE(granny2_impl, CurveDataHeader_D3K8uC8u)
IMPLVARTYPE(granny2_impl, CurveDataHeader_D4nK16uC15u)
IMPLVARTYPE(granny2_impl, CurveDataHeader_DaIdentity)
IMPLVARTYPE(granny2_impl, CurveDataHeader_D4nK8uC7u)
IMPLVARTYPE(granny2_impl, CurveDataHeader_D3Constant32f)
IMPLVARTYPE(granny2_impl, CurveDataHeader_DaK32fC32f)
IMPLVARTYPE(granny2_impl, CurveDataHeader_DaKeyframes32f)
//IMPLVARTYPE(granny2_impl, CurveDataHeader_DaConstant32f)
//IMPLVARTYPE(granny2_impl, CurveDataHeader_D4Constant32f)
//IMPLVARTYPE(granny2_impl, CurveDataHeader_DaK16uC16u)
//IMPLVARTYPE(granny2_impl, CurveDataHeader_DaK8uC8u)


const EnumLookupType PropertyTypes[] =
{
	{granny2_impl::pt_text,              "Text"},
	{granny2_impl::pt_real32,            "granny_real32"},
	{granny2_impl::pt_int32,             "int32"},
	{0,  NULL},
};
#if GRANNY2LIB_ENABLE_DUMP
template<> void granny2::Dump(NWDumpStream& out, LPCSTR name, granny2_impl::PropertyType const & val)
{
	Dump(out, name, EnumToString(val, PropertyTypes));
}
#endif

template<> 
bool granny2::Recall(NWStream& in, ValuePtr<granny2_impl::CurveData>& val)
{
	bool ok = true;
	int off = 0;
	if (ok) ok = Recall(in, off);
	if (ok)
	{
		long cur = in.Tell();
		in.Seek(SEEK_SET, off);

		byte Format = 0;
		byte Degree = 0;
		if (ok) ok = Recall(in, Format);
		if (ok) ok = Recall(in, Degree);
		if (ok)
		{
			// reset back to same position so the curvedata can read it
			in.Seek(SEEK_SET, off);
			switch (Format)
			{
			case 0: //DaKeyframes32f
				if (granny2_impl::CurveDataHeader_DaKeyframes32f* value = new granny2_impl::CurveDataHeader_DaKeyframes32f())
				{
					ok = Recall(in, *value);
					ValuePtr<granny2_impl::CurveData>(value).swap(val);
				}
				break;

			case 1: //DaK32fC32f
				if (granny2_impl::CurveDataHeader_DaK32fC32f* value = new granny2_impl::CurveDataHeader_DaK32fC32f())
				{
					ok = Recall(in, *value);
					ValuePtr<granny2_impl::CurveData>(value).swap(val);
				}
				break;

			case 2: // DaIdentity
				if (granny2_impl::CurveDataHeader_DaIdentity* value = new granny2_impl::CurveDataHeader_DaIdentity())
				{
					ok = Recall(in, *value);
					ValuePtr<granny2_impl::CurveData>(value).swap(val);
				}
				break;
			case 4: // D3Constant32f
				if (granny2_impl::CurveDataHeader_D3Constant32f* value = new granny2_impl::CurveDataHeader_D3Constant32f())
				{
					ok = Recall(in, *value);
					ValuePtr<granny2_impl::CurveData>(value).swap(val);
				}
				break;
			case 8: // D4nK16uC15u
				if (granny2_impl::CurveDataHeader_D4nK16uC15u* value = new granny2_impl::CurveDataHeader_D4nK16uC15u())
				{
					ok = Recall(in, *value);
					ValuePtr<granny2_impl::CurveData>(value).swap(val);
				}
				break;
			case 9: //D4nK8uC7u
				if (granny2_impl::CurveDataHeader_D4nK8uC7u* value = new granny2_impl::CurveDataHeader_D4nK8uC7u())
				{
					ok = Recall(in, *value);
					ValuePtr<granny2_impl::CurveData>(value).swap(val);
				}
				break;

			case 10: // D3K16uC16u
				if (granny2_impl::CurveDataHeader_D3K16uC16u* value = new granny2_impl::CurveDataHeader_D3K16uC16u())
				{
					ok = Recall(in, *value);
					ValuePtr<granny2_impl::CurveData>(value).swap(val);
				}
				break;

			case 11: // D3K8uC8u
				if (granny2_impl::CurveDataHeader_D3K8uC8u* value = new granny2_impl::CurveDataHeader_D3K8uC8u())
				{
					ok = Recall(in, *value);
					ValuePtr<granny2_impl::CurveData>(value).swap(val);
				}
				break;
			default:
				throw std::runtime_error( "Unknown granny2_impl::CurveData Type" );
//				if (IsDebuggerPresent()) {
//					ASSERT(!"Unknown granny2_impl::CurveData Type");
//				}
				break;
			}
		}
		//ok = Recall(in, *val);
		in.Seek(SEEK_SET, cur);
	}
	return ok;
}

#if GRANNY2LIB_ENABLE_STORE
template<> 
bool granny2::Store<granny2_impl::CurveData>(NWStream& out, ValuePtr<granny2_impl::CurveData> const & val)
{
	return false;
}
#endif

#if GRANNY2LIB_ENABLE_DUMP
template<> 
void granny2::Dump<granny2_impl::CurveData>(NWDumpStream& out, LPCSTR name, ValuePtr<granny2_impl::CurveData> const & val)
{
	out.Indent();
	if (val.isNull()) {
		out.PrintF("%s *: <NULL>\n", name);
	} else {
		out.PrintF("%s *:\n", name);
		++out.IndentLevel;

		switch (val->Format)
		{
		case 0: //DaKeyframes32f
			if (granny2_impl::CurveDataHeader_DaKeyframes32f* value = (granny2_impl::CurveDataHeader_DaKeyframes32f*)val.ToPointer())
			{
				Dump(out, name, *value);
			}
			break;

		case 1: //DaK32fC32f
			if (granny2_impl::CurveDataHeader_DaK32fC32f* value = (granny2_impl::CurveDataHeader_DaK32fC32f*)val.ToPointer())
			{
				Dump(out, name, *value);
			}
			break;

		case 2: // DaIdentity
			if (granny2_impl::CurveDataHeader_DaIdentity* value = (granny2_impl::CurveDataHeader_DaIdentity*)val.ToPointer())
			{
				Dump(out, name, *value);
			}
			break;
		case 4: // D3Constant32f
			if (granny2_impl::CurveDataHeader_D3Constant32f* value = (granny2_impl::CurveDataHeader_D3Constant32f*)val.ToPointer())
			{
				Dump(out, name, *value);
			}
			break;
		case 8: // D4nK16uC15u
			if (granny2_impl::CurveDataHeader_D4nK16uC15u* value = (granny2_impl::CurveDataHeader_D4nK16uC15u*)val.ToPointer())
			{
				Dump(out, name, *value);
			}
			break;
		case 9: //D4nK8uC7u
			if (granny2_impl::CurveDataHeader_D4nK8uC7u* value = (granny2_impl::CurveDataHeader_D4nK8uC7u*)val.ToPointer())
			{
				Dump(out, name, *value);
				GR2Array<byte> pts = value->KnotsControls;
				//Dump(out, "Points", pts);
				
				out.Indent();
				out.PrintF("Points:\n", name);
				++out.IndentLevel;
				out.Indent();
				out.PrintF("%-8s %-8s %-8s %-8s", "t", "x", "y", "z");
				out.NewLine();
				for (int i=0,n=pts.size()/4; i<n; ++i)
				{
					out.Indent();
					out.PrintF("%-8u %-8u %-8u %-8u"
						, uint(pts[i]), uint(pts[n + (i*3)+0]), uint(pts[n + (i*3)+1]), uint(pts[n + (i*3)+2])
						);
					out.NewLine();
				}
				--out.IndentLevel;
			}
			break;

		case 10: // D3K16uC16u
			if (granny2_impl::CurveDataHeader_D3K16uC16u* value = (granny2_impl::CurveDataHeader_D3K16uC16u*)val.ToPointer())
			{
				Dump(out, name, *value);
			}
			break;

		case 11: // D3K8uC8u
			if (granny2_impl::CurveDataHeader_D3K8uC8u* value = (granny2_impl::CurveDataHeader_D3K8uC8u*)val.ToPointer())
			{
				Dump(out, name, *value);
			}
			break;
		default:
			//if (IsDebuggerPresent()) {
			//	ASSERT(!"Unknown granny2_impl::CurveData Type");
			//}
			throw std::runtime_error( "Unknown granny2_impl::CurveData Type" );
			break;
		}

		--out.IndentLevel;
	}
}
#endif

template<> bool granny2::Recall(NWStream& in, Text& val)
{
	bool ok = true;
	int offset = 0;
	if (ok) ok = Recall(in, offset);
	if (ok) 
	{
		long cur = in.Tell();
		in.Seek(SEEK_SET, offset);
		// make sure we have at least 5 bytes for this to work
		char buf[8]; memset(buf, 0, sizeof(buf));
		for(;;)
		{
			ok = (1 == in.Read(buf, 4, 1)); // always 4 byte aligned with null terminato
			if (!ok) break;
			val.append(buf);
			if (strlen(buf) < 4)
				break;
		}
		in.Seek(SEEK_SET, cur);
	}
	return ok;
}

#if GRANNY2LIB_ENABLE_STORE
template<> bool granny2::Store(NWStream& out, Text const & val)
{
	bool ok = true;
	long off = AlignWord(out.TellEnd());

	// Strings are stored in 2 parts offset + data.
	if (ok ) ok = Store(out, off);
	if (ok)
	{
		long pos = out.Tell();
		out.Seek(SEEK_END, off);
		ok = ( 1 == out.Write(val.c_str(), AlignWord(val.size() + 1)) );
		out.Seek(SEEK_SET, pos);
	}
	return ok;
}
#endif

#if GRANNY2LIB_ENABLE_DUMP
template<> void granny2::Dump(NWDumpStream& out, LPCSTR name, Text const & val)
{
	out.Indent();
	out.PrintF("%s: %s\n", name, val.c_str());
}
#endif

template<> unsigned int granny2::SizeOf(NWStream& in, Text const & val)
{
	return sizeof(unsigned int);
}

template<> 
bool granny2::Recall<granny2_impl::ExtendedData>(NWStream& in, granny2_impl::ExtendedData& val) 
{
	bool ok = true;
	int cur = in.Tell();

	uint start = 0, dstart = 0;
	if (ok) ok = Recall(in, start);
	if (ok) ok = Recall(in, dstart);
	if (start == 0)
		return true;

	int next = in.Tell();
	if (ok) in.Seek(SEEK_SET, start);

	int sz = granny2::SizeOf(in, granny2_impl::Property());
	for (int i=0; ok; ++i)
	{
		int cpos = in.Tell();
		// Peek ahead and see if next property is valid
		granny2_impl::PropertyType type = granny2_impl::pt_none;
		if (ok) ok = granny2::Recall(in, type);
		if (type == granny2_impl::pt_none) {
			in.Seek(SEEK_CUR, (int)(sz-sizeof(int)));
			break;
		} else {
			in.Seek(SEEK_CUR, (int)(0-sizeof(int)));
		}
		granny2_impl::Property prop;
		if (ok) ok = granny2::Recall(in, prop);
		if (ok)
		{
			if (prop.Type == granny2_impl::pt_none || prop.Name.size() == 0)
				break;
			val.Properties.append(prop);
		}
	}
	//seek to start of data
	if (ok) in.Seek(SEEK_SET, dstart);
	for (int i=0, n=val.Properties.size(); ok && i<n; ++i)
		ok = val.Properties[i].RecallData(in);

	in.Seek(SEEK_SET, next);
	return ok;
}

#if GRANNY2LIB_ENABLE_STORE
template<> 
bool granny2::Store<granny2_impl::ExtendedData>(NWStream& out, granny2_impl::ExtendedData const & val) 
{
	return true;
}
#endif

#if GRANNY2LIB_ENABLE_DUMP
template<> 
void granny2::Dump<granny2_impl::ExtendedData>(NWDumpStream& out,  LPCSTR name, granny2_impl::ExtendedData const & val) 
{
	out.Indent();
	out.PrintF("%s: %s\n", "Properties", name);
	++out.IndentLevel;

	char buffer[100];
	for (int i=0, n=val.Properties.size(); i<n; ++i)
	{
		StringCbPrintfA(buffer, sizeof(buffer), "[%d]", i);
		granny2::Dump(out, buffer, val.Properties[i]);
	}

	--out.IndentLevel;
}
#endif

template<> 
unsigned int granny2::SizeOf<granny2_impl::ExtendedData>(NWStream& in, granny2_impl::ExtendedData const & val) 
{
	return 0;
}
//////////////////////////////////////////////////////////////////////////
granny2_impl::Property::Property()
{
	lpValue = NULL;
	granny2::Init(this->GetInfDefmap(), (LPVOID)this);
}

granny2_impl::Property::~Property()
{
	FreeData();
}

LPVOID granny2_impl::Property::AllocData()
{
	FreeData();
	if (Len == 0)
	{
		switch (Type)
		{
		case granny2_impl::pt_text: lpValue = new Text(); break;
		case granny2_impl::pt_int32: break;
		case granny2_impl::pt_real32: break;

		case granny2_impl::pt_pointer:
			switch (RefInfo->Type)
			{
			case granny2_impl::pt_text:	lpValue = new GR2Array<Text>(); break;
			case granny2_impl::pt_int32:	lpValue = new GR2Array<granny_int32>(); break;
			case granny2_impl::pt_real32:	lpValue = new GR2Array<granny_real32>(); break;
//			default:  if (IsDebuggerPresent()) { ASSERT(FALSE); } break;
			}
			break;

//		default:  if (IsDebuggerPresent()) { ASSERT(FALSE); } break;
		}
	}
	else
	{
		switch (Type)
		{
		case granny2_impl::pt_text:   if ((size_t)Len * sizeof( Text ) < (size_t)Len) throw std::bad_alloc( ); lpValue = new Text[Len]; break;
		case granny2_impl::pt_int32:  if ((size_t)Len * sizeof( granny_int32 ) < (size_t)Len) throw std::bad_alloc( ); lpValue = new granny_int32[Len]; break;
		case granny2_impl::pt_real32: if ((size_t)Len * sizeof( granny_real32 ) < (size_t)Len) throw std::bad_alloc( ); lpValue = new granny_real32[Len]; break;
		
//		default:  if (IsDebuggerPresent()) { ASSERT(FALSE); } break;
		}
	}
	return lpValue;
}

void granny2_impl::Property::FreeData()
{
	if (lpValue != NULL)
	{
		if (Len == 0)
		{
			switch (Type)
			{
			case granny2_impl::pt_text: delete (Text*) lpValue; break;
			case granny2_impl::pt_int32: break;
			case granny2_impl::pt_real32: break;
			case granny2_impl::pt_pointer:
				switch (RefInfo->Type)
				{
				case granny2_impl::pt_text:		delete (GR2Array<Text>*)lpValue; break;
				case granny2_impl::pt_int32:	delete (GR2Array<granny_int32>*)lpValue; break;
				case granny2_impl::pt_real32:	delete (GR2Array<granny_real32>*)lpValue; break;
				}
				break;
			}
		}
		else
		{
			switch (Type)
			{
			case granny2_impl::pt_text:   delete [] (Text*)lpValue; break;
			case granny2_impl::pt_int32:  delete [] (granny_int32*)lpValue; break;
			case granny2_impl::pt_real32: delete [] (granny_real32*)lpValue; break;
			}
		}
		lpValue = NULL;
	}
}

bool granny2_impl::Property::RecallData(NWStream& in)
{
	bool ok = true;
	AllocData();
	if (Len == 0)
	{
		switch (Type)
		{
		case granny2_impl::pt_text: 
			ok = Recall(in, *(Text*)lpValue); 
			break;
		case granny2_impl::pt_int32:  
			ok = Recall(in, (granny_int32&)lpValue); 
			break;
		case granny2_impl::pt_real32:  
			ok = Recall(in, (granny_real32&)lpValue); 
			break;
		case granny2_impl::pt_pointer:
			switch (RefInfo->Type)
			{
			case granny2_impl::pt_text:		
				ok = Recall(in, *(GR2Array<Text>*)lpValue); 
				break;
			case granny2_impl::pt_int32:	
				ok = Recall(in, *(GR2Array<granny_int32>*)lpValue); 
				break;
			case granny2_impl::pt_real32:	
				ok = Recall(in, *(GR2Array<granny_real32>*)lpValue); 
				break;
			}
			break;
		}
	}
	else
	{
		switch (Type)
		{
		case granny2_impl::pt_text:
			if (Text* val = (Text*)lpValue) 
				for (int i=0; ok && i<Len; ++i, ++val)
					ok = Recall(in, *val);
			break;
		case granny2_impl::pt_int32:  
			if (granny_int32* val = (granny_int32*)lpValue) 
				for (int i=0; ok && i<Len; ++i, ++val)
					ok = Recall(in, *val);
			break;
		case granny2_impl::pt_real32:  
			if (granny_real32* val = (granny_real32*)lpValue) 
				for (int i=0; ok && i<Len; ++i, ++val)
					ok = Recall(in, *val);
			break;
		}
	}
	return ok;
}

bool granny2_impl::Property::SaveData(NWStream& out)
{
	bool ok = true;
	if (Len == 0)
	{
		switch (Type)
		{
		case granny2_impl::pt_text: lpValue = new Text(); break;
		}
	}
	else
	{
		///
	}
	return ok;
}

template<> bool granny2::Recall<granny2_impl::Property>(NWStream& in, granny2_impl::Property& val) { 
	return Recall(val.GetInfDefmap(), in, static_cast<LPVOID>(&val));
}

#if GRANNY2LIB_ENABLE_STORE
template<> bool granny2::Store<granny2_impl::Property>(NWStream& out, granny2_impl::Property const & val) {
	return Store(val.GetInfDefmap(), out, static_cast<LPCVOID>(&val));
}
#endif

#if GRANNY2LIB_ENABLE_DUMP
template<> void granny2::Dump<granny2_impl::Property>(NWDumpStream& out,  LPCSTR name, granny2_impl::Property const & val) {
	Dump(val.GetInfDefmap(), val.GetTypeName(), out, name, static_cast<LPCVOID>(&val));
	++out.IndentLevel;
	if (val.Len == 0)
	{
		switch (val.Type)
		{
		case granny2_impl::pt_text:		Dump(out, "Value", *(Text*)val.lpValue);		break;
		case granny2_impl::pt_int32:	Dump(out, "Value", (granny_int32&)val.lpValue);	break;
		case granny2_impl::pt_real32:	Dump(out, "Value", (granny_real32&)val.lpValue);break;
		}
	}
	else
	{
		out.Indent();
		out.Print("Value:");
		switch (val.Type)
		{
		case granny2_impl::pt_text:
			if (Text* v = (Text*)val.lpValue) 
				for (int i=0; i<val.Len; ++i, ++v)
					out.PrintF(" \"%s\"", (*v).c_str());
			break;
		case granny2_impl::pt_int32:  
			if (granny_int32* v = (granny_int32*)val.lpValue) 
				for (int i=0; i<val.Len; ++i, ++v)
					out.PrintF(" %d", v);
			break;
		case granny2_impl::pt_real32:  
			if (granny_real32* v = (granny_real32*)val.lpValue) 
				for (int i=0; i<val.Len; ++i, ++v)
					out.PrintF(" %f", v);
			break;
		}
		out.Print("\n");
	}
	--out.IndentLevel;
}
#endif

template<> unsigned int granny2::SizeOf<granny2_impl::Property>(NWStream& in, granny2_impl::Property const & val) {
	return SizeOf(val.GetInfDefmap(), in, static_cast<LPCVOID>(&val));
}

namespace granny2_impl
{
//////////////////////////////////////////////////////////////////////////

GR2Array<granny_real32> CurveDataHeader_D3K16uC16u::GetKnots() const {
	GR2Array<granny_real32> knots(KnotsControls.size()/4);
	for (int i=0, n=KnotsControls.size()/4; i<n; ++i)
		knots[i] = float(KnotsControls[i])/float(OneOverKnotScaleTrunc);
	return knots;
}

GR2Array<Point3> CurveDataHeader_D3K16uC16u::GetPoint3Data() const {
	GR2Array<Point3> controls(KnotsControls.size()/4);
	for (int i=0, n=KnotsControls.size()/4; i<n; ++i) {
		Point3 &q = controls[i];
		q.x = float(KnotsControls[n+(i*3+0)])*ControlScales.x + ControlOffsets.x;
		q.y = float(KnotsControls[n+(i*3+1)])*ControlScales.y + ControlOffsets.y;
		q.z = float(KnotsControls[n+(i*3+2)])*ControlScales.z + ControlOffsets.z;
	}
	return controls;
}
//////////////////////////////////////////////////////////////////////////
GR2Array<float> CurveDataHeader_D3K8uC8u::GetKnots() const {
	GR2Array<float> knots(KnotsControls.size()/4);
	for (int i=0, n=KnotsControls.size()/4; i<n; ++i)
		knots[i] = float(KnotsControls[i])*65536.0f/256.0f/float(OneOverKnotScaleTrunc);
	return knots;
}

GR2Array<Point3> CurveDataHeader_D3K8uC8u::GetPoint3Data() const {
	GR2Array<Point3> controls(KnotsControls.size()/4);
	for (int i=0, n=KnotsControls.size()/4; i<n; ++i) {
		Point3 &q = controls[i];
		q.x = float(KnotsControls[n+(i*3+0)])*ControlScales.x + ControlOffsets.x;
		q.y = float(KnotsControls[n+(i*3+1)])*ControlScales.y + ControlOffsets.y;
		q.z = float(KnotsControls[n+(i*3+2)])*ControlScales.z + ControlOffsets.z;
	}
	return controls;
}
//////////////////////////////////////////////////////////////////////////

GR2Array<float> CurveDataHeader_D4nK16uC15u::GetKnots() const {
	GR2Array<float> knots(KnotControls.size()/4);
	for (int i=0, n=KnotControls.size()/4; i<n; ++i)
		knots[i] = float(KnotControls[i])/OneOverKnotScale;
	return knots;
}

KeyType CurveDataHeader_D4nK16uC15u::GetKeyType() const {
	return XYZ_ROTATION_KEY;
}

GR2Array<Quat> CurveDataHeader_D4nK16uC15u::GetQuatData() const {

	GR2Array<Quat> controls(KnotControls.size()/4);
	for (int i=0, n=KnotControls.size()/4; i<n; ++i) {
		Point3 q; // convert euler angles to quat?
		q.x = float(KnotControls[n+(i*3+0)])/65535.0f*2.0f*float(M_PI)*2.0f;
		q.y = float(KnotControls[n+(i*3+1)])/65535.0f*2.0f*float(M_PI)*2.0f;
		q.z = float(KnotControls[n+(i*3+2)])/65535.0f*2.0f*float(M_PI)*2.0f;
	}
	return controls;
}

GR2Array<Point3> CurveDataHeader_D4nK16uC15u::GetPoint3Data() const {

	GR2Array<Point3> controls(KnotControls.size()/4);
	for (int i=0, n=KnotControls.size()/4; i<n; ++i) {
		Point3& q = controls[i]; // convert euler angles to quat?
		q.x = (float(KnotControls[n+(i*3+0)])-16379.0f)/65535.0f*float(M_PI)*2.0f;
		q.y = (float(KnotControls[n+(i*3+1)])-32757.0f)/65535.0f*float(M_PI)*2.0f;
		q.z = (float(KnotControls[n+(i*3+2)])-32757.0f)/65535.0f*float(M_PI)*2.0f;
	}
	return controls;
}
//////////////////////////////////////////////////////////////////////////
GR2Array<float> CurveDataHeader_D4nK8uC7u::GetKnots() const {
	GR2Array<float> Knots;
	Knots.resize(KnotsControls.size()/4);
	for (int i=0, n=KnotsControls.size()/4; i<n; ++i) {
		Knots[i] = float(KnotsControls[i])/float(OneOverKnotScale);
	}
	return Knots;
}

KeyType CurveDataHeader_D4nK8uC7u::GetKeyType() const {
	return XYZ_ROTATION_KEY;
}

GR2Array<Quat> CurveDataHeader_D4nK8uC7u::GetQuatData() const {
	GR2Array<Quat> controls;
	return controls;
}

inline granny_real32 toreal(granny_uint16 value) {
	return float(value)/255.0f*float(M_PI)*2.0f;
}

enum ByteState
{
	eq000,
	eq001,
	eq063,
	eq064,
	eq128,
	eq129,
	eq191,
	eq192,
	bt002_062,
	bt065_127,
	bt130_190,
	bt193_255,
};

ByteState GetState(granny_uint8 value)
{
	if (value ==   0) return eq000;
	if (value ==   1) return eq001;
	if (value <=  62) return bt002_062;
	if (value ==  63) return eq063;
	if (value ==  64) return eq064;
	if (value <= 127) return bt065_127;
	if (value == 128) return eq128;
	if (value == 129) return eq129;
	if (value == 191) return eq191;
	if (value == 192) return eq192;
	if (value <= 190) return bt130_190;
	return bt193_255;
}

GR2Array<Point3> CurveDataHeader_D4nK8uC7u::GetPoint3Data() const {
	GR2Array<Point3> controls(KnotsControls.size()/4);
	for (int i=0, n=KnotsControls.size()/4; i<n; ++i) {
		Point3 &q = controls[i];
		granny_uint8 x = KnotsControls[n+(i*3+0)];
		granny_uint8 y = KnotsControls[n+(i*3+1)];
		granny_uint8 z = KnotsControls[n+(i*3+2)];

		ByteState xs = GetState(x);
		ByteState ys = GetState(y);
		ByteState zs = GetState(z);

		// special cases

		//  x-Axis Only Handling
		if      (xs == bt065_127 && ys == eq128     && zs == eq128     ) q.Set(toreal(x-64),  0.0f, 0.0f);
		else if (xs == eq000     && ys == eq000     && zs == bt065_127 ) q.Set(toreal(192-z), 0.0f, 0.0f);
		else if (xs == bt130_190 && ys == eq128     && zs == eq128     ) q.Set(toreal(448-x), 0.0f, 0.0f);

		//  zs-Axis Only Handling
		else if (xs == eq000     && ys == eq128     && zs == bt130_190 ) q.Set(0.0f, 0.0f, toreal(z-192));
		else if (xs == bt065_127 && ys == eq128     && zs == eq000     ) q.Set(0.0f, 0.0f, toreal(192-x));
		else if (xs == eq128     && ys == eq128     && zs == bt130_190 ) q.Set(0.0f, 0.0f, toreal(448-z));

		//  ys-Axis Only Handling
		else if (xs == eq001     && ys == bt130_190 && zs == eq191     ) q.Set(0.0f, toreal(y-192), 0.0f);
		else if (xs == eq063     && ys == bt065_127 && zs == eq129     ) q.Set(0.0f, toreal(192-y), 0.0f);
		else if (xs == eq129     && ys == bt130_190 && zs == eq191     ) q.Set(0.0f, toreal(448-y), 0.0f);

		else if (xs == eq063     && ys == bt130_190 && zs == bt130_190 ) q.Set(0.0f, 0.0f, toreal(z-y));
		else if (xs == bt193_255 && ys == bt065_127 && zs == eq191     ) q.Set(0.0f, 0.0f, toreal(x-y-32));
		else if (xs == bt193_255 && ys == bt002_062 && zs == eq192     ) q.Set(0.0f, 0.0f, toreal(x-y-32));
		else if (xs == bt130_190 && ys == eq191     && zs == bt065_127 ) q.Set(0.0f, 0.0f, toreal(x+z));
		else if (xs == bt065_127 && ys == eq192     && zs == eq001     ) q.Set(0.0f, 0.0f, toreal(x+64));


		else
		{
			q.Set(toreal(x), toreal(y), toreal(z));
//			ASSERT(false);
			throw std::runtime_error( "Unknown D4nK8uC7u byte state" );
		}
	}
	return controls;
}

//////////////////////////////////////////////////////////////////////////

GR2Array<float> CurveDataHeader_D3Constant32f::GetKnots() const { 
	GR2Array<float> a(1); 
	a[0] = 0.0f;
	return a;
}

GR2Array<Point3> CurveDataHeader_D3Constant32f::GetPoint3Data() const { 
	GR2Array<Point3> a(1);
	a[0] = Controls;
	return a;
}

//////////////////////////////////////////////////////////////////////////

KeyType CurveDataHeader_DaK32fC32f::GetKeyType() const {
	return QUADRATIC_KEY;
}

GR2Array<float> CurveDataHeader_DaK32fC32f::GetKnots() const {
	return Knots;
}

GR2Array<Quat> CurveDataHeader_DaK32fC32f::GetQuatData() const {
	GR2Array<Quat> controls(Controls.size() / 4);
	for (int i=0, n=Controls.size()/4; i<n; ++i) {
		Quat& q = controls[i];
		q.x = Controls[(i*4)+0];
		q.y = Controls[(i*4)+1];
		q.z = Controls[(i*4)+2];
		q.w = Controls[(i*4)+3];
	}
	return controls;
}

GR2Array<Point3> CurveDataHeader_DaK32fC32f::GetPoint3Data() const {
	GR2Array<Point3> controls;
	return controls;
}

GR2Array<Matrix3> CurveDataHeader_DaK32fC32f::GetMatrix3Data() const {
	GR2Array<Matrix3> controls;
	if (Degree == 1 && (Controls.size() == (Knots.size() * 9)))
	{
		controls.resize(Controls.size() / 9);
		for (int i=0, n=Controls.size()/9; i<n; ++i) {
			Matrix3& m = controls[i];
			m.SetRow(0, Point3(Controls[(i*9)+0], Controls[(i*9)+1], Controls[(i*9)+2]));
			m.SetRow(1, Point3(Controls[(i*9)+3], Controls[(i*9)+4], Controls[(i*9)+5]));
			m.SetRow(2, Point3(Controls[(i*9)+6], Controls[(i*9)+7], Controls[(i*9)+8]));
		}
	}
	return controls;
}
//////////////////////////////////////////////////////////////////////////
KeyType CurveDataHeader_DaKeyframes32f::GetKeyType() const {
	return QUADRATIC_KEY;
}

GR2Array<float> CurveDataHeader_DaKeyframes32f::GetKnots() const {
	return GR2Array<float>();
}

GR2Array<Quat> CurveDataHeader_DaKeyframes32f::GetQuatData() const {
	GR2Array<Quat> controls(Controls.size() / 4);
	if (Dimension == 4)
	{
		for (int i=0, n=Controls.size()/4; i<n; ++i) {
			Quat& q = controls[i];
			q.x = Controls[(i*4)+0];
			q.y = Controls[(i*4)+1];
			q.z = Controls[(i*4)+2];
			q.w = Controls[(i*4)+3];
		}
	}
	return controls;
}

GR2Array<Point3> CurveDataHeader_DaKeyframes32f::GetPoint3Data() const {
	GR2Array<Point3> controls;
	if (Dimension == 9)
	{
		controls.resize(Controls.size() / 3);
		for (int i=0, n=Controls.size()/3; i<n; ++i) {
			Point3& p = controls[i];
			p.x = Controls[(i*3)+0];
			p.y = Controls[(i*3)+1];
			p.z = Controls[(i*3)+2];
		}
	}
	return controls;
}

GR2Array<Matrix3> CurveDataHeader_DaKeyframes32f::GetMatrix3Data() const {
	GR2Array<Matrix3> controls;
	if (Dimension == 9)
	{
		controls.resize(Controls.size() / 9);
		for (int i=0, n=Controls.size()/9; i<n; ++i) {
			Matrix3& m = controls[i];
			m.SetRow(0, Point3(Controls[(i*9)+0], Controls[(i*9)+1], Controls[(i*9)+2]));
			m.SetRow(1, Point3(Controls[(i*9)+3], Controls[(i*9)+4], Controls[(i*9)+5]));
			m.SetRow(2, Point3(Controls[(i*9)+6], Controls[(i*9)+7], Controls[(i*9)+8]));
		}
	}
	return controls;
}
//////////////////////////////////////////////////////////////////////////
}

}

