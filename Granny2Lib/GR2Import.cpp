/**********************************************************************
*<
FILE: ImporterCore.cpp

DESCRIPTION:	Core Import helper routines

CREATED BY: tazpn (Theo)

HISTORY: 

*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/
#include "NWN2Import.h"
#if VERSION_3DSMAX >= ((7000<<16)+(15<<8)+0) // Version 7
#  include <IFrameTagManager.h>
#endif
#include <notetrck.h>
#include "GR2Import.h"
#include "GR2Format.h"
#include "../Common/zutils.h"
#include <stdexcept>
using std::runtime_error;

extern ClassDesc2* GetNWN2ImportDesc();

using namespace NWN2;

typedef DWORD (__stdcall * fnGrannyConvertFileToRaw)(const char* src, const char *dst);
typedef std::map<std::string, INodeTab, ltstr> NameNodeMap;

static const char *GR2Import = "GR2Import";
static const char *MDBImport = "MDBImport";
static const char *MDBCommon = "MDBCommon";

struct GR2ImportImpl
{
   GR2ImportImpl(GR2Importer *owner);
   ~GR2ImportImpl();

   INode *CreateBone(const char* name, Point3 startPos, Point3 endPos, Point3 zAxis);
   INodeTab ImportSkeleton(Skeleton& skel);
   Matrix3 GetLocalTransform(Skeleton& skel, size_t index);
   Matrix3 GetWorldTransform(Skeleton& skel, size_t index);
   void OrderBones(INodeTab& bones);
   void ImportAnimations();
   void ClearAnimation();
   void ClearAnimation(INode* node);
   void ClearAnimation(Control* c);

   void ImportPosition(Control* c, TransformTrack& track, float time, GR2Array<granny_real32>& defaultKeys);
   void ImportRotation(Control* c, TransformTrack& track, float time, GR2Array<granny_real32>& defaultKeys);
   void ImportScale(Control* c, TransformTrack& track, float time, GR2Array<granny_real32>& defaultKeys);

   Control* MakePosition(Control *tmCont, Class_ID clsid);
   Control* MakePositionXYZ(Control *tmCont, Class_ID clsid);
   Control* MakeRotation(Control *tmCont, Class_ID rotClsid, Class_ID rollClsid);
   Control* MakeScale(Control *tmCont, Class_ID clsid);

   BOOL enableFaceMode;
   BOOL clearAnimation;
   stringlist boneMatch;
   stringlist faceBoneMatch;
   Matrix3 wtm;
   HMODULE gr2;
   fnGrannyConvertFileToRaw GrannyConvertFileToRaw;
   GR2Importer *o;
   GR2Info info;
   Matrix3 invTM;
   Quat invQ;
   BOOL enableScale;
   float scaleFactor;
   BOOL enableAnimation;
};

const float FramesPerSecond = 30.0f;
const float FramesIncrement = 1.0f/30.0f;
const int TicksPerFrame = GetTicksPerFrame();

inline TimeValue TimeToFrame(float t) {
	return TimeValue(t * FramesPerSecond * TicksPerFrame);
}
inline float FrameToTime(TimeValue t) {
	return float(t) / (FramesPerSecond * TicksPerFrame);
}


GR2Importer::GR2Importer(const TCHAR *Name,ImpInterface *I,Interface *GI, BOOL SuppressPrompts)
   : BaseImporter()
{
   BaseInit(Name,I,GI,SuppressPrompts);
}

GR2Importer::GR2Importer()
{
}

void GR2Importer::Initialize()
{
}

GR2ImportImpl::GR2ImportImpl(GR2Importer *owner) : o(owner), gr2(NULL), GrannyConvertFileToRaw(NULL), wtm(true)
{
   string granny2 = GetSetting<string>("Granny2Loc");
   gr2 = GetModuleHandle("granny2.dll");
   if (gr2 == NULL)
      gr2 = LoadLibrary(granny2.c_str());
   if (gr2 == NULL)
      gr2 = LoadLibrary("granny2.dll");
   if (gr2 == NULL)
      throw runtime_error(FormatString("Unable to locate file: granny2.dll"));
   GrannyConvertFileToRaw = (fnGrannyConvertFileToRaw)GetProcAddress(gr2, "_GrannyConvertFileToRaw@8");
   if (GrannyConvertFileToRaw == NULL)
      throw runtime_error(FormatString("Unable to locate entry point: GrannyConvertFileToRaw"));

   ParamBlockDesc2 *params = GetNWN2ImportDesc()->GetParamBlockDescByName("parameters");
   boneMatch = TokenizeString(params->GetStr(IMP_PB_BONEMATCH), ";", true);
   faceBoneMatch = TokenizeString(params->GetStr(IMP_PB_FACEBONEMATCH), ";", true);
   clearAnimation = TRUE;

	enableScale = owner->GetIniValue<BOOL>(MDBImport, "EnableScale", 1);
	scaleFactor = owner->GetIniValue<float>(MDBImport, "ScaleFactor", 100.f);
	enableAnimation = owner->GetIniValue<BOOL>(GR2Import, "EnableAnimation", 0);
}

GR2ImportImpl::~GR2ImportImpl()
{
   if (GrannyConvertFileToRaw)
      GrannyConvertFileToRaw = NULL;
   if (gr2) {
      //FreeLibrary(gr2);
      gr2 = NULL;
   }
}

bool GR2Importer::DoImport()
{
   GR2ImportImpl impl(this);

   char tmppath[MAX_PATH];
   char tmpfile[MAX_PATH];
   GetTempPath(MAX_PATH, tmppath);
   GetTempFileName(tmppath, "gr2", 0, tmpfile);
   try
   {
      impl.GrannyConvertFileToRaw(name.c_str(), tmpfile);
      NWStream in(tmpfile, true);
      if (!Recall(in, impl.info))
         throw std::runtime_error(FormatString("Unable to process file: %s", name.c_str()));

	  // Make an inverse transform matrix to reset back to original format
	  //impl.invTM.IdentityMatrix();
	  //if (!impl.info.artInfo.isNull())
	  //{
		 // RHMatrix3 &rhm = impl.info.artInfo->Rotation;
		 // impl.invTM.Set(rhm.m[0], rhm.m[1], rhm.m[2], impl.info.artInfo->Origin);
	  //}
	  //impl.invQ.Set(impl.invTM);
	  //impl.invTM.Invert();

	  impl.invTM = RotateYMatrix(-90);

	  Point3 baseRot(0.0f, 0.0f, 0.0f);

      INodeTab bones;
      impl.OrderBones(bones);

	  impl.ImportAnimations();
   }
   catch (std::exception &e)
   {
      _unlink(tmpfile);
      throw e;
   }
   return true;
}

bool GR2Importer::ImportFromZip(const TCHAR *Zip, INodeTab& bones, bool reorder, bool facemode)
{
   bool ok = false;
   GR2ImportImpl impl(this);
   impl.enableFaceMode = facemode ? TRUE : FALSE ;

   char tmppath[MAX_PATH], gr2file[MAX_PATH], rawfile[MAX_PATH];
   GetTempPath(MAX_PATH, tmppath);
   PathCombine(gr2file, tmppath, name.c_str());
   PathCombine(rawfile, tmppath, name.c_str());
   PathRenameExtension(gr2file, ".gr2");
   PathRenameExtension(rawfile, ".raw");

   char search[MAX_PATH];
   strcpy(search, "*/");
   strcat(search, PathFindFileName(gr2file));
   ok = ExtractFile(Zip, search, gr2file);
   if (!ok) return false;

   try
   {
      impl.GrannyConvertFileToRaw(gr2file, rawfile);

      NWStream in(rawfile, true);
      if (!Recall(in, impl.info))
         throw std::runtime_error(FormatString("Unable to process file: %s", name.c_str()));

      ArtToolInfo& artInfo = *impl.info.artInfo;
      //impl.wtm = artInfo.Rotation;
      impl.wtm.SetTranslate(artInfo.Origin);
      float s = 1.0f / artInfo.UnitsPerMeter;
	  if (impl.enableScale) s *= (impl.scaleFactor / 100.0f);

      impl.wtm *= ScaleMatrix(Point3(s, s, s));
      if (reorder)
      {
         impl.OrderBones(bones);
      }
      else
      {
         bones = impl.ImportSkeleton(*impl.info.Skeletons[0]);
      }
   }
   catch (std::exception &e)
   {
      _unlink(gr2file);
      _unlink(rawfile);
      throw e;
   }
   _unlink(gr2file);
   _unlink(rawfile);
   return ok;
}

INode *GR2ImportImpl::CreateBone(const char* name, Point3 startPos, Point3 endPos, Point3 zAxis)
{
   if (FPInterface * fpBones = GetCOREInterface(Interface_ID(0x438aff72, 0xef9675ac)))
   {
      FunctionID createBoneID = fpBones->FindFn(TEXT("createBone"));
      FPValue result;
      FPParams params (3, TYPE_POINT3, &startPos, TYPE_POINT3, &endPos, TYPE_POINT3, &zAxis);     
      FPStatus status = fpBones->Invoke(createBoneID, result, &params);
      if (status == FPS_OK && result.type == TYPE_INODE)
      {
         if (INode *n = result.n)
         {
            n->SetName(const_cast<TCHAR*>(name));
            float len = Length(endPos-startPos);
            float width = max(0.5f, min(3.0f, len * 0.25f));
            if (Object* o = n->GetObjectRef())
            {
               setMAXScriptValue(o->GetReference(0), "width", 0, width);
               setMAXScriptValue(o->GetReference(0), "height", 0, width);
            }
            n->BoneAsLine(1);
            n->ShowBone(2);
            n->IgnoreExtents(TRUE);
         }
         return result.n;
      }
      fpBones->ReleaseInterface();
   }
   return NULL;
}

Matrix3 GR2ImportImpl::GetLocalTransform(Skeleton& skel, size_t index)
{
   Matrix3 m3(true);
   if (index >= 0)
   {
      Bone& bone = skel.Bones[index];
      m3.SetRotate( Inverse(bone.Transform.Rotation) );
      m3.SetTrans(bone.Transform.Origin);
      m3 *= ScaleMatrix(Point3(bone.Transform.Scale.m[0][0], bone.Transform.Scale.m[1][1], bone.Transform.Scale.m[2][2]));

      RHMatrix4& r = bone.InverseWorldTransform;
      Matrix3 tm(
         Point3(r.m[0][0], r.m[0][1], r.m[0][2]),
         Point3(r.m[1][0], r.m[1][1], r.m[1][2]),
         Point3(r.m[2][0], r.m[2][1], r.m[2][2]),
         Point3(r.m[3][0], r.m[3][1], r.m[3][2])
         );
      Matrix3 itm = Inverse(tm);
      //m3 = itm;
   }
   return m3;
}

Matrix3 GR2ImportImpl::GetWorldTransform(Skeleton& skel, size_t index)
{
   Bone& bone = skel.Bones[index];
   if (bone.ParentIndex >= 0)
   {
      return GetLocalTransform(skel, index) * GetWorldTransform(skel, bone.ParentIndex);
   }
   else
   {
      return GetLocalTransform(skel, index) * wtm;
   }
}

INodeTab GR2ImportImpl::ImportSkeleton(Skeleton& skel)
{
   Point3 zAxis(0,0,1);
   INodeTab nodes;
   nodes.SetCount(int(skel.Bones.size()));
   float scale = 1.0f;

   for (size_t i=0, n=skel.Bones.size(); i<n; ++i)
   {
      Bone& bone = skel.Bones[i];

      INode *node = o->gi->GetINodeByName(bone.Name.c_str());
      if (node != NULL)
      {
         nodes[i] = node;
         continue;
      }

      Point3 pp(0.0f,0.0f,0.0f);

      Matrix3 tm = GetWorldTransform(skel, i);
      Point3 p = tm.GetTrans();
      Quat q = tm;

      if (bone.ParentIndex >= 0)
      {
         Matrix3 m3 = GetWorldTransform(skel, bone.ParentIndex);
         pp = m3.GetTrans();
         //pp = skel.Bones[bone.ParentIndex].Transform.Origin;
      }
      node = CreateBone(bone.Name.c_str(), p, pp, zAxis);
      node->SetUserPropInt("GR2BoneIndex", int(i));
      nodes[i] = node;

      //OutputDebugString(FormatText("GR2BoneIndex: %d %s\n", i, bone.Name.c_str()));

      PosRotScaleNode(node, p, q, scale, PosRotScale(prsPos|prsRot));

      if (bone.ParentIndex >= 0)
      {
         if (INode *pn = nodes[bone.ParentIndex])
            pn->AttachChild(node, 1);
      }
   }
   return nodes;
}

void GR2ImportImpl::OrderBones(INodeTab& bones)
{
   if (info.Skeletons.size() == 1)
   {
      NameNodeMap nodes;
      INodeTab rv = ImportSkeleton(*info.Skeletons[0]);
      // Strip out auto-generated bones.  Place in order found in ini file
      for (size_t i = 0, n = rv.Count(); i<n; ++i)
      {
         INode *node = rv[i];

         bool found = false;
         for (stringlist::const_iterator itr=boneMatch.begin(), end=boneMatch.end(); itr != end; ++itr)
         {
            if (wildmatch(*itr, node->GetName())) {
               nodes[*itr].Append(1, &node);
               found = true;
            }
         }
         if (!found)
         {
            node->SetUserPropInt("MDBBoneIndex", int(bones.Count()));
            bones.Append(1, &node);
         }
      }
      for (stringlist::const_iterator itr=boneMatch.begin(), end=boneMatch.end(); itr != end; ++itr)
      {
         INodeTab& map = nodes[*itr];
         for (size_t i = 0, n = map.Count(); i<n; ++i)
         {
            INode *node = map[i];
            node->SetUserPropInt("MDBBoneIndex", int(bones.Count()));
            bones.Append(1, &node);
         }
      }
      // When in face mode, swap the Face Bones for the first N bones in the skeleton.  
      //   Some of the later bones like Head, Neck, Ribcage are still used so we cannot 
      //   discard the whole skeleton.
      if (enableFaceMode)
      {
         size_t curIdx = 0;
         for (size_t i = 0, n = bones.Count(); i<n; ++i)
         {
            INode *node = bones[i];
            for (stringlist::const_iterator itr=faceBoneMatch.begin(), end=faceBoneMatch.end(); itr != end; ++itr)
            {
               if (wildmatch(*itr, node->GetName())) {
                  bones[i] = bones[curIdx];
                  bones[curIdx] = node;
                  node->SetUserPropInt("MDBBoneIndex", int(curIdx));
                  ++curIdx;
                  break;
               }
            }
         }
      }
   }
}

void GR2ImportImpl::ClearAnimation(Control *c)
{
	if (c != NULL)
	{
		if (c->IsColorController())
			return;

		if (IKeyControl *ikeys = GetKeyControlInterface(c)){
			ikeys->SetNumKeys(0);
		}
#if VERSION_3DSMAX > ((5000<<16)+(15<<8)+0) // Version 5
		if (Control *sc = c->GetWController()) { 
			if (sc != c) ClearAnimation(sc); 
		}
#endif
		if (Control *sc = c->GetXController()) { 
			if (sc != c) ClearAnimation(sc); 
		}
		if (Control *sc = c->GetYController()) { 
			if (sc != c) ClearAnimation(sc); 
		}
		if (Control *sc = c->GetZController()) { 
			if (sc != c) ClearAnimation(sc); 
		}
		if (Control *sc = c->GetRotationController()) { 
			if (sc != c) ClearAnimation(sc); 
		}
		if (Control *sc = c->GetPositionController()) { 
			if (sc != c) ClearAnimation(sc); 
		}
		if (Control *sc = c->GetScaleController()) { 
			if (sc != c) ClearAnimation(sc); 
		}
	}
}

void GR2ImportImpl::ClearAnimation(INode *node)
{
	if (node != NULL)
	{
		if (node->HasNoteTracks()){
			for (int i = node->NumNoteTracks()-1; i>=0; --i ){
				if (NoteTrack *nt = node->GetNoteTrack(i))
					node->DeleteNoteTrack(nt, TRUE);
			}
		}
		node->DeleteKeys(TRACK_DOALL);
		ClearAnimation(node->GetTMController());
		for (int i=0, n=node->NumberOfChildren(); i<n; ++i){
			ClearAnimation(node->GetChildNode(i));
		}
	}
}

void GR2ImportImpl::ClearAnimation()
{
	if (clearAnimation)
	{
#if VERSION_3DSMAX >= ((7000<<16)+(15<<8)+0) // Version 7
		if (IFrameTagManager *tagMgr = (IFrameTagManager*)GetCOREInterface(FRAMETAGMANAGER_INTERFACE)) {

			int n = tagMgr->GetTagCount();
			for (int i=n-1; i>=0; --i){
				tagMgr->DeleteTag( tagMgr->GetTagID(i) );
			}
		}
#endif
		ClearAnimation(o->gi->GetRootNode());
	}
}

//////////////////////////////////////////////////////////////////////////

const Class_ID IPOS_CONTROL_CLASS_ID = Class_ID(0x118f7e02,0xffee238a);
enum {
	IPOS_X_REF	=	0,
	IPOS_Y_REF	=	1,
	IPOS_Z_REF	=	2,
	IPOS_W_REF	=	3,
};

Control* GR2ImportImpl::MakePosition(Control *tmCont, Class_ID clsid)
{
	Interface *ip = o->gi;
	if (Control *c = tmCont->GetPositionController()) {
		if (c->ClassID()!=clsid) {
			if (Control *tmpCtrl = (Control*)ip->CreateInstance(CTRL_POSITION_CLASS_ID, clsid)){
				if (!tmCont->SetPositionController(tmpCtrl))
					tmpCtrl->DeleteThis();
				else
					c = tmpCtrl;
			}
		}
		return c;
	}
	return NULL;
}

Control* GR2ImportImpl::MakePositionXYZ(Control *tmCont, Class_ID clsid)
{
	Interface *ip = o->gi;
	// First make the controller and XYZ Independent position controller, then fix individuals
	if (Control *c = MakePosition(tmCont, IPOS_CONTROL_CLASS_ID)){
		if (Control *x = c->GetXController()){
			if (x->ClassID()!= clsid) {
				if (Control *tmp = (Control*)ip->CreateInstance(CTRL_FLOAT_CLASS_ID, clsid))
					c->SetReference(IPOS_X_REF, tmp);
			}
		}
		if (Control *y = c->GetYController()){
			if (y->ClassID()!= clsid) {
				if (Control *tmp = (Control*)ip->CreateInstance(CTRL_FLOAT_CLASS_ID, clsid))
					c->SetReference(IPOS_Y_REF, tmp);
			}
		}
		if (Control *z = c->GetZController()){
			if (z->ClassID()!= clsid) {
				if (Control *tmp = (Control*)ip->CreateInstance(CTRL_FLOAT_CLASS_ID, clsid))
					c->SetReference(IPOS_Z_REF, tmp);
			}
		}
		return c;
	}
	return NULL;
}

Control* GR2ImportImpl::MakeRotation(Control *tmCont, Class_ID rotClsid, Class_ID rollClsid)
{
	Interface *ip = o->gi;
	if (Control *c = tmCont->GetRotationController()) {
		if (c->ClassID()!=rotClsid) {
			if (Control *tmpCtrl = (Control*)ip->CreateInstance(CTRL_ROTATION_CLASS_ID, rotClsid)) {
				if (!tmCont->SetRotationController(tmpCtrl))
					tmpCtrl->DeleteThis();
				else
					c = tmpCtrl;
			}
		}
		if (Control *r = tmCont->GetRollController()) {
			if (r->ClassID()!=rollClsid) {
				if (Control *r = (Control*)ip->CreateInstance(CTRL_FLOAT_CLASS_ID,rollClsid))
					if (!tmCont->SetRollController(r))
						r->DeleteThis();
			}
		}
		return c;
	}
	return NULL;
}

Control* GR2ImportImpl::MakeScale(Control *tmCont, Class_ID clsid)
{
	Interface *ip = o->gi;
	if (Control *c = tmCont->GetScaleController()) {
		if (c->ClassID()!=clsid) {
			if (Control *tmpCtrl = (Control*)ip->CreateInstance(CTRL_SCALE_CLASS_ID, clsid)){
				if (!tmCont->SetScaleController(tmpCtrl))
					tmpCtrl->DeleteThis();
				else
					c = tmpCtrl;
			}
		}
		return c;
	}
	return NULL;
}


void GR2ImportImpl::ImportAnimations()
{
	if (info.Animations.size() == 0 || !enableAnimation)
		return;

	ClearAnimation();

	//for (int anim=0, nanim=info.Animations.size(); anim<nanim; ++anim)
	//{
	//	Animation& anim = (*info.Animations[anim]);
	//}
	Interval range; range.SetInstant(0);

	float time = FrameToTime(0);
	for (int ianim=0, nanim=info.Animations.size(); ianim<nanim; ++ianim)
	{
		Animation& anim = (*info.Animations[ianim]);
		TimeValue animEnd = TimeToFrame(time + anim.Duration);
		if (animEnd > range.End())
			range.SetEnd(animEnd);
		// Build Default Time
		int nkeys = anim.Duration / anim.TimeStep;
		GR2Array<granny_real32> defaultKeys(nkeys);
		granny_real32 curtime = 0.0f;
		for (int ikeys=0; ikeys<nkeys; ++ikeys, curtime += anim.TimeStep)
			defaultKeys[ikeys] = curtime;

		for (int grp=0, ngrp=anim.TrackGroups.size(); grp<ngrp; ++grp)
		{
			TrackGroup& group = (*anim.TrackGroups[grp]);
			if (INode *root = o->gi->GetINodeByName(group.Name))
			{
				Point3 s( group.InitialPlacement.Scale.m[0][0]
						, group.InitialPlacement.Scale.m[1][1]
						, group.InitialPlacement.Scale.m[2][2] );
				for (int itrack=0, ntrack=group.TransformTracks.size(); itrack<ntrack; ++itrack)
				{
					TransformTrack& track = group.TransformTracks[itrack];
					if (INode *node = o->gi->GetINodeByName(track.Name))
					{
						if (Control *c = node->GetTMController())
						{
							DWORD flags=INHERIT_ALL;
							c->SetInheritanceFlags(INHERIT_ALL,FALSE);

							ImportPosition(c, track, time, defaultKeys);
							ImportRotation(c, track, time, defaultKeys);
							ImportScale(c, track, time, defaultKeys);
						}
					}
				}
				Matrix3 rot(true); group.InitialPlacement.Rotation.MakeMatrix(rot);
				Matrix3 m = TransMatrix(group.InitialPlacement.Origin) * Inverse(rot) * ScaleMatrix( s );
				PosRotScaleNode(root, m);
				// TODO: Move to initial transform
			}
		}
	}
	o->gi->SetAnimRange(range);
}

void GR2ImportImpl::ImportPosition(Control* c, TransformTrack& track, float time, GR2Array<granny_real32>& defaultKeys)
{
	// Better use linear for now
	if (Control *subCtrl = MakePositionXYZ(c, Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0))) 
	{
		IKeyControl *xkeys = GetKeyControlInterface(subCtrl->GetXController());
		IKeyControl *ykeys = GetKeyControlInterface(subCtrl->GetYController());
		IKeyControl *zkeys = GetKeyControlInterface(subCtrl->GetZController());
		if (xkeys && ykeys && zkeys) 
		{
			if (Point3CurveData* posData = dynamic_cast<Point3CurveData*>(track.PositionCurve.ToPointer()))
			{
				NWArray<float> times = posData->GetKnots();
				NWArray<Point3> points = posData->GetPoint3Data();
				if (times.size() == 0 && points.size() != 0)
					times = defaultKeys;

				xkeys->SetNumKeys(times.size());
				ykeys->SetNumKeys(times.size());
				zkeys->SetNumKeys(times.size());
				for (int i=0; i<times.size(); ++i)
				{
					IBezFloatKey rKey;
					memset(&rKey, 0, sizeof(rKey));
					rKey.time = TimeToFrame(times[i] + time);
					SetInTanType(rKey.flags, BEZKEY_SLOW);
					SetOutTanType(rKey.flags, BEZKEY_SLOW);
					rKey.val = points[i].x;
					xkeys->SetKey(i, &rKey);
					rKey.val = points[i].y;
					ykeys->SetKey(i, &rKey);
					rKey.val = points[i].z;
					zkeys->SetKey(i, &rKey);
				}
				xkeys->SortKeys();
				ykeys->SortKeys();
				zkeys->SortKeys();
			}
		}
	}
}

void GR2ImportImpl::ImportRotation(Control* c, TransformTrack& track, float time, GR2Array<granny_real32>& defaultKeys)
{
	if (RotationCurveData* rotData = dynamic_cast<RotationCurveData*>(track.OrientationCurve.ToPointer()))
	{
		if (rotData->GetKeyType() == QUADRATIC_KEY)
		{
			if (Control *subCtrl = MakeRotation(c, Class_ID(HYBRIDINTERP_ROTATION_CLASS_ID,0), Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0))) 
			{
				IKeyControl *keys = GetKeyControlInterface(subCtrl);
				if (keys) 
				{
					NWArray<float> times = rotData->GetKnots();
					NWArray<Quat> points = rotData->GetQuatData();
					if (times.size() == 0 && points.size() != 0)
						times = defaultKeys;

					keys->SetNumKeys(times.size());
					for (int i=0; i<times.size(); ++i)
					{
						Quat q = points[i];
						q.w = -q.w;
						IBezQuatKey rKey;
						memset(&rKey, 0, sizeof(rKey));
						rKey.time = TimeToFrame(times[i] + time);
						SetInTanType(rKey.flags, BEZKEY_SLOW);
						SetOutTanType(rKey.flags, BEZKEY_SLOW);
						rKey.val = q;
						keys->SetKey(i, &rKey);
					}
					keys->SortKeys();
				}
			}
		}
		else if (rotData->GetKeyType() == XYZ_ROTATION_KEY)
		{
			if (Control *subCtrl = MakeRotation(c, Class_ID(EULER_CONTROL_CLASS_ID,0), Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0)))
			{
				IKeyControl *xkeys = GetKeyControlInterface(subCtrl->GetXController());
				IKeyControl *ykeys = GetKeyControlInterface(subCtrl->GetYController());
				IKeyControl *zkeys = GetKeyControlInterface(subCtrl->GetZController());
				if (xkeys && ykeys && zkeys) 
				{
					NWArray<float> times = rotData->GetKnots();
					NWArray<Point3> points = rotData->GetPoint3Data();
					if (times.size() == 0 && points.size() != 0)
						times = defaultKeys;
					
					xkeys->SetNumKeys(times.size());
					ykeys->SetNumKeys(times.size());
					zkeys->SetNumKeys(times.size());
					for (int i=0; i<times.size(); ++i)
					{
						Point3 &p = points[i];
						//Quat q; Point3 p;
						//EulerToQuat(points[i], q);
						//q = invQ * q;
						//QuatToEuler(q, p);

						IBezFloatKey rKey;
						memset(&rKey, 0, sizeof(rKey));
						rKey.time = TimeToFrame(times[i] + time);
						SetInTanType(rKey.flags, BEZKEY_SLOW);
						SetOutTanType(rKey.flags, BEZKEY_SLOW);
						rKey.val = p.x;
						xkeys->SetKey(i, &rKey);
						rKey.val = p.y;
						ykeys->SetKey(i, &rKey);
						rKey.val = p.z;
						zkeys->SetKey(i, &rKey);
					}
					xkeys->SortKeys();
					ykeys->SortKeys();
					zkeys->SortKeys();
				}
			}
		}
	}
}

void GR2ImportImpl::ImportScale(Control* c, TransformTrack& track, float time, GR2Array<granny_real32>& defaultKeys)
{
	if (Control *subCtrl = MakeScale(c, Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0))) 
	{
		if (IKeyControl *keys = GetKeyControlInterface(subCtrl)) 
		{
			if (Matrix3CurveData* posData = dynamic_cast<Matrix3CurveData*>(track.ScaleShearCurve.ToPointer()))
			{
				NWArray<float> times = posData->GetKnots();
				NWArray<Matrix3> values = posData->GetMatrix3Data();
				if (times.size() == 0 && values.size() != 0)
					times = defaultKeys;

				keys->SetNumKeys(times.size());
				for (int i=0; i<times.size(); ++i)
				{
					Matrix3 &m = values[i];
					IBezScaleKey rKey;
					memset(&rKey, 0, sizeof(rKey));
					rKey.time = TimeToFrame(times[i] + time);
					SetInTanType(rKey.flags, BEZKEY_SLOW);
					SetOutTanType(rKey.flags, BEZKEY_SLOW);
					rKey.val = Point3(m.GetRow(0)[0], m.GetRow(1)[1], m.GetRow(2)[2] );
					keys->SetKey(i, &rKey);
				}
				keys->SortKeys();
			}
		}
	}
}
