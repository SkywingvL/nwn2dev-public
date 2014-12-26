/**********************************************************************
*<
FILE: GR2Import.h

DESCRIPTION:	GR2 Importer 

CREATED BY: tazpn (Theo)

HISTORY:

*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/

#ifndef __GR2IMPORT_H__
#define __GR2IMPORT_H__

#include "BaseImporter.h"

// GR2 Importer
class GR2Importer : public BaseImporter//, public IniFileSection
{
public:
   GR2Importer(const TCHAR *Name,ImpInterface *I,Interface *GI, BOOL SuppressPrompts);
   virtual void Initialize();

   void ApplyAppSettings();
   bool ShowDialog();
   virtual bool DoImport();

   bool ImportFromZip(const TCHAR *Zip, INodeTab& bones, bool reorder=true, bool faceMode=false);

   // Generic IniFile reading routine
   template<typename T>
   T GetIniValue(LPCTSTR Section, LPCTSTR Setting, T Default){
	   if (!iniFileValid) 
		   return Default;
	   return ::GetIniValue<T>(Section, Setting, Default, iniFileName.c_str());
   }

   // Generic IniFile reading routine
   template<typename T>
   void SetIniValue(LPCTSTR Section, LPCTSTR Setting, T value){
	   if (!iniFileValid) 
		   return;
	   ::SetIniValue<T>(Section, Setting, value, iniFileName.c_str());
   }

   protected: 
      GR2Importer();
};

#endif
