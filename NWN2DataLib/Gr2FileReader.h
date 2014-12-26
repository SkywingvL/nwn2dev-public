/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	Gr2FileReader.h

Abstract:

	This module define the granny2 (*.gr2) file reader.  Granny2 files supply
	skeletal and animation data for game models.

	This granny2 file reader is based on data from tazpn (Theo)'s Neverwinter
	Nights 2 Model Importer/Exporter tool.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_GR2FILEREADER_H
#define _PROGRAMS_NWN2DATALIB_GR2FILEREADER_H

#ifdef _MSC_VER
#pragma once
#endif

class ModelSkeleton;

namespace granny2
{
	class Granny2Reader;

	namespace granny2_impl
	{
		struct GR2Info;
	}
}

class Gr2Accessor;

//
// Define the GR2 file reader object, used to access GR2 files.
//

class Gr2FileReader
{

public:

	typedef swutil::SharedPtr< Gr2FileReader > Ptr;

	//
	// Constructor.  Raises an std::exception on parse failure.
	//

	Gr2FileReader(
		__in Gr2Accessor * Accessor,
		__in const std::string & FileName,
		__in bool PreferOutOfProcess
		);

	//
	// Destructor.
	//

	virtual
	~Gr2FileReader(
		);

	//
	// Load a model's skeleton from the current GR2 file.
	//

	ModelSkeleton *
	LoadModelSkeleton(
		);

private:

	//
	// Define the underlying granny data storage.
	//

	typedef swutil::SharedPtr< granny2::granny2_impl::GR2Info > GR2InfoPtr;

	GR2InfoPtr m_GR2Info;

};


//
// Define the GR2 accessor.  Typically, one GR2 accessor exists per
// ResourceManager.
//
// The Gr2Accessor wraps granny2::Granny2Reader.
//

class Gr2Accessor
{

public:

	typedef swutil::SharedPtr< Gr2Accessor > Ptr;
	typedef swutil::SharedPtr< granny2::granny2_impl::GR2Info > GR2InfoPtr;

	//
	// Constructor.  Raises an std::excepton on failure to load granny2.
	//

	Gr2Accessor(
		__in const std::string & Granny2Dll,
		__in const std::string & TempPath
		);

	virtual
	~Gr2Accessor(
		);

	//
	// Load a gr2 file and return a GR2InfoPtr.
	//

	GR2InfoPtr
	LoadGranny2File(
		__in const std::string & FileName,
		__in bool PreferOutOfProcess
		);

private:

	typedef swutil::SharedPtr< granny2::Granny2Reader > Granny2ReaderPtr;

	Granny2ReaderPtr m_Granny2Reader;
	std::string      m_TempPath;
	UINT             m_Unique;

};


#endif
