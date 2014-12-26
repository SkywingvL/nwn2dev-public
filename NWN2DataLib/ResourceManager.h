/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ResourceManager.h

Abstract:

	This module defines the interface to the module resource manager, which
	provides access to data files and string entries for the current module.

--*/

#ifndef _PROGRAMS_NWN2DATALIB_RESOURCEMANAGER_H
#define _PROGRAMS_NWN2DATALIB_RESOURCEMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "MeshManager.h"

#include "ResourceAccessor.h"
#include "ErfFileReader.h"
#include "DirectoryFileReader.h"
#include "ZipFileReader.h"
#include "KeyFileReader.h"

#include "TlkFileReader.h"
#include "2DAFileReader.h"
#include "GffFileReader.h"

#include "Gr2FileReader.h"

struct IDebugTextOut;

class ResourceManager : public IResourceAccessor< NWN::ResRef32 >
{

public:

	enum ModuleSearchOrder
	{
		//
		// Take the first matching module, either as an ERF (preferred), or as a
		// directory (if there was no ERF).  This is the default.
		//

		ModSearch_Automatic        = 0,

		//
		// Prefer a directory source for the module, even if there were ERFs
		// present.  If the load fails, the automatic search order is used.
		//

		ModSearch_PrefDirectory    = 1,

		//
		// Prefere an ERF for the module, even if there were directories
		// present.  If the load fails, the automatic search order is used.
		//

		ModSearch_PrefEncapsulated = 2,

		LastModSearchOrder
	};

	//
	// Define resource manager flags (for LoadModuleResources).
	//

	typedef enum _ResManFlags
	{
		//
		// Encapsulated resources use 16-byte instead of 32-byte format, i.e.
		// the caller is loading NWN1-style resources instead of NWN2-style
		// resources.  Note that the interface continues to use 32-byte resrefs
		// in its APIs.
		//

		ResManFlagErf16              = 0x00000001,

		//
		// External granny2 support should not be loaded because the caller
		// does not intend to use it.
		//

		ResManFlagNoGranny2          = 0x00000002,

		//
		// Only load the module core resource set itself, not any other game
		// resources (such as in-box resources), useful for loading just enough
		// of the module up to read the HAK and TLK list.
		//
		// This flag provides analogous functionality to the
		// LoadModuleResourcesLite function.
		//

		ResManFlagLoadCoreModuleOnly = 0x00000004,

		//
		// Prefer to invoke Granny2 out of process (to protect against bugs in
		// the granny2.dll core logic that might bring down the process).
		//

		ResManFlagGr2DllOutOfProcess = 0x00000008,

		//
		// Only load base resources, not module resources.
		//

		ResManFlagBaseResourcesOnly  = 0x00000010,

		//
		// Disable all built-in resource provider discovery.  This can be
		// used to force the resource system to clean up without loading a new
		// module, or to prepare a resource manager instance that is entirely
		// backed by a custom set of resource accessors (only).
		//

		ResManFlagNoBuiltinProviders = 0x00000020,

		//
		// Require module.ifo to be present before a module is accepted.  This
		// flag should only be used for server applications.
		//

		ResManFlagRequireModuleIfo   = 0x00000040,

		LastResManFlag
	} ResManFlags;

	//
	// Define resource manager create flags (for constructor).
	//

	typedef enum _ResManCreateFlags
	{
		//
		// Do not perform instance setup for the temporary directory.
		//
		// N.B.  Most functionality is unavailable in such a resource manager
		//       instance.
		//

		ResManCreateFlagNoInstanceSetup = 0x00000001,

		LastResManCreateFlag
	} ResManCreateFlags;

	typedef std::vector< std::string > StringVec;

	//
	// Define extended module load parameters for use with LoadModuleResources.
	//
	// Users should zero the structure when declaring it for future
	// compatibility with new fields.  Zeroing the structure enacts the default
	// options, which are the same as though no ModuleLoadParams was specified.
	//
	// N.B.  Unless explicitly specified otherwise, any pointers passed are
	//       assumed to only remain valid until LoadModuleResources returns to
	//       its caller.
	//

	struct ModuleLoadParams
	{
		//
		// Supply the campaign ID to use (or NULL).
		//

		const GUID                  * CampaignID;     // Optional

		//
		// Receive the actual campaign ID that was used, if any.
		//

		GUID                          CampaignIDUsed; // Out

		//
		// Supply the module search preference.
		//

		ModuleSearchOrder             SearchOrder;

		//
		// Supply a custom search path to use as a directory store for loading
		// resources (or NULL if unused).  The search path need not have a
		// trailing backslash, and should be fully qualified.  The custom
		// search path is the first directory path searched after the module
		// itself.
		//

		const char                  * CustomSearchPath;

		//
		// Supply custom load flags.  Legal values are drawn from ResManFlags.
		//

		unsigned long                 ResManFlags;

		//
		// Supply an array of *.key files to load (for NWN1-style resource load
		// operations).  The first key file in the list is searched first (with
		// respect to all key files).  If NULL, no key files are loaded.
		//

		const StringVec             * KeyFiles;

		//
		// Supply an array of first chance custom resource accessors.  These
		// are searched before any other component of the resource system, and
		// a reference is maintained until the resource system is unloaded.
		// The last provider in the array provided is searched first.  If the
		// accessor count is zero, no custom providers are used.
		//

		IResourceAccessor   * const * CustomFirstChanceAccessors;
		size_t                        NumCustomFirstChanceAccessors;

		//
		// Supply an array of last chance custom resource accessors.  These
		// are searched after every other component of the resource system, and
		// a reference is maintained until the resource system is unloaded.
		// The last provider in the array provided is searched first.  If the
		// accessor count is zeor, no custom providers are used.
		//

		IResourceAccessor   * const * CustomLastChanceAccessors;
		size_t                        NumCustomLastChanceAccessors;

		//
		// Supply a custom raw path to where the module may be loaded from.
		// The path must point to a directory with a module.ifo file, or an ERF
		// file with a module.ifo file.  If NULL, the default search logic is
		// used.
		//

		const char                  * CustomModuleSourcePath;
	};

	typedef NWN::ResRef32 ResRefT;

	//
	// Constructor.  Raises an std::exception on parse failure.
	//

	ResourceManager(
		__in IDebugTextOut * TextWriter,
		__in unsigned long CreateFlags = 0
		);

	//
	// Destructor.
	//

	virtual
	~ResourceManager(
		);

	//
	// Called to load resources for a module after resources are available,
	// i.e. after autodownload completes.
	//

	void
	LoadModuleResources(
		__in const std::string & ModuleResName,
		__in const std::string & AltTlkFileName,
		__in const std::string & HomeDir,
		__in const std::string & InstallDir,
		__in const std::vector< NWN::ResRef32 > & HAKs,
		__in_opt ModuleLoadParams * LoadParams = NULL
		);

	//
	// Called to pre-load module resources.  This routine is used by the server
	// as the module must be loaded to discover parameters such as the HAK list
	// and the talk file list.  The server then calls LoadModuleResources to
	// do a full load with that discovered data.
	//
		
	void
	LoadModuleResourcesLite(
		__in const std::string & ModuleResName,
		__in const std::string & HomeDir,
		__in const std::string & InstallDir
		);

	//
	// (Forcibly) unload all still-loaded resources and deregister all active
	// resource providers.  A call to LoadModuleResources must be made before
	// the resource manager may be used again once this routine is invoked.
	//

	void
	UnloadAllResources(
		);

	//
	// (Forcibly) unload all still-loaded resources, but do not clear the index
	// table or deregister any active resource providers.  The resource manager
	// will still be able to serve new requests.
	//
	// The number of handles that had to be forcibly closed is returned.
	//

	size_t
	CloseOpenResourceFileHandles(
		);

	//
	// Change the temporary directory for the resource manager.  This routine
	// should only be called before the resource manager has loaded any data
	// resources.
	//

	void
	ChangeTemporaryDirectory(
		__in const std::string & TempDirectory
		);

	//
	// Look up a string based on STRREF.  Returns false on failure, i.e. if the
	// string could not be found.
	//

	bool
	GetTalkString(
		__in unsigned long StringId,
		__out std::string & String
		) const;

	//
	// Look up the value of a particular column at a given row index in a given
	// .2DA file.
	//
	// The routine returns false if no such column or row existed, or if the
	// column value was the empty value ("****").
	//
	// The caller assumes responsibility for using canonical resource names
	// (all lowercase) for ResourceName.
	//

	bool
	Get2DAString(
		__in const std::string & ResourceName,
		__in const std::string & Column,
		__in size_t Row,
		__out std::string & Value
		);

	//
	// Various datatype wrappers around Get2DAString.
	//

	inline
	bool
	Get2DAInt(
		__in const std::string & ResourceName,
		__in const std::string & Column,
		__in size_t Row,
		__out int & Value,
		__in int Radix = 0
		)
	{
		std::string V;

		if (!Get2DAString( ResourceName, Column, Row, V ))
			return false;

		Value = (int) strtol( V.c_str( ), NULL, Radix );

		return true;
	}

	inline
	bool
	Get2DAUlong(
		__in const std::string & ResourceName,
		__in const std::string & Column,
		__in size_t Row,
		__out unsigned long & Value,
		__in int Radix = 0
		)
	{
		std::string V;

		if (!Get2DAString( ResourceName, Column, Row, V ))
			return false;

		Value = strtoul( V.c_str( ), NULL, Radix );

		return true;
	}

	inline
	bool
	Get2DABool(
		__in const std::string & ResourceName,
		__in const std::string & Column,
		__in size_t Row,
		__out bool & Value
		)
	{
		std::string V;

		if (!Get2DAString( ResourceName, Column, Row, V ))
			return false;

		if (V.empty( ))
			return false;

		if ((V[ 0 ] == 't') || (V[ 0 ] == 'T') || (V[ 0 ] == '1'))
			Value = true;
		else
			Value = false;

		return true;
	}

	inline
	bool
	Get2DAResRef(
		__in const std::string & ResourceName,
		__in const std::string & Column,
		__in size_t Row,
		__out NWN::ResRef32 & Value
		)
	{
		std::string V;

		if (!Get2DAString( ResourceName, Column, Row, V ))
			return false;

		if (V.empty( ))
			return false;

		ZeroMemory( &Value, sizeof( Value ) );
		memcpy( &Value, V.data( ), min( V.size( ), sizeof( Value ) ) );

		return true;
	}

	inline
	bool
	Get2DAResRef(
		__in const std::string & ResourceName,
		__in const std::string & Column,
		__in size_t Row,
		__out NWN::ResRef16 & Value
		)
	{
		std::string V;

		if (!Get2DAString( ResourceName, Column, Row, V ))
			return false;

		if (V.empty( ))
			return false;

		ZeroMemory( &Value, sizeof( Value ) );
		memcpy( &Value, V.data( ), min( V.size( ), sizeof( Value ) ) );

		return true;
	}

	inline
	bool
	Get2DAFloat(
		__in const std::string & ResourceName,
		__in const std::string & Column,
		__in size_t Row,
		__out float & Value
		)
	{
		std::string V;

		if (!Get2DAString( ResourceName, Column, Row, V ))
			return false;

		Value = (float) atof( V.c_str( ) );

		return true;
	}

	//
	// Return the count of valid rows in the .2DA.
	//

	inline
	size_t
	Get2DARowCount(
		__in const std::string & ResourceName
		)
	{
		const TwoDAFileReader * TwoDA;

		TwoDA = Get2DA( ResourceName );

		if (TwoDA == NULL)
			return 0;

		return TwoDA->GetRowCount( );
	}

	//
	// Return the count of valid columns in the .2DA.
	//

	inline
	size_t
	Get2DAColumnCount(
		__in const std::string & ResourceName
		)
	{
		const TwoDAFileReader * TwoDA;

		TwoDA = Get2DA( ResourceName );

		if (TwoDA == NULL)
			return 0;

		return TwoDA->GetColumnCount( );
	}

	//
	// Unload all cached 2DAs, causing them to be reloaded on the next
	// reference.
	//

	inline
	void
	Clear2DACache(
		)
	{
		m_2DAs.clear( );
	}

	//
	// Gr2 file access.  Raises an std::exception on failure.
	//
	// The caller is expected to provide an on-disk filename for the *.gr2
	// file to load, typically retrieved via a call to Demand.
	//

	inline
	Gr2FileReader *
	OpenGr2File(
		__in const std::string & FileName
		)
	{
		bool PreferOutOfProcess;

		if (m_Gr2Accessor.get( ) == NULL)
		{
			throw std::runtime_error(
				"Failed to load .gr2 file as granny2 is unavailable." );
		}

		PreferOutOfProcess = false;

		if (m_ResManFlags & ResManFlagGr2DllOutOfProcess)
			PreferOutOfProcess = true;

		return new Gr2FileReader(
			m_Gr2Accessor.get( ),
			FileName,
			PreferOutOfProcess );
	}

	//
	// Demand load a resource by resref.  The routine copies the entire
	// resource to a disk file (which may be the original file for a directory
	// resource or a temporary file for an encapsupated or ZIP resource).  Thus
	// all callers incur the expensive of a full file access for the entire
	// contents of the resource.
	//
	// If more granular access to individual, large resources is required, then
	// callers may use the IResourceAccessor APIs.
	//
	// The caller should release the demand-loaded resource with a call to
	// Release once the caller is finished with it.
	//

	std::string
	Demand(
		__in const std::string & ResRef,
		__in ResType Type
		);

	inline
	std::string
	Demand(
		__in const NWN::ResRef16 & ResRef,
		__in ResType Type
		)
	{
		std::string   R;
		const char  * p;

		p = (const char *) memchr(
			ResRef.RefStr,
			'\0',
			sizeof( ResRef.RefStr ) );

		if (p == NULL)
			R.assign( ResRef.RefStr, sizeof( ResRef.RefStr ) );
		else
			R.assign( ResRef.RefStr, p - ResRef.RefStr );

		for (size_t i = 0; i < R.size( ); i += 1)
		{
			R[ i ] = (char) tolower( (int) (unsigned char) R[ i ] );
		}

		return Demand( R, Type );
	}

	inline
	std::string
	Demand(
		__in const NWN::ResRef32 & ResRef,
		__in ResType Type
		)
	{
		std::string   R;
		const char  * p;

		p = (const char *) memchr(
			ResRef.RefStr,
			'\0',
			sizeof( ResRef.RefStr ) );

		if (p == NULL)
			R.assign( ResRef.RefStr, sizeof( ResRef.RefStr ) );
		else
			R.assign( ResRef.RefStr, p - ResRef.RefStr );

		for (size_t i = 0; i < R.size( ); i += 1)
		{
			R[ i ] = (char) tolower( (int) (unsigned char) R[ i ] );
		}

		return Demand( R, Type );
	}

	//
	// Check if a resource exists without opening it.
	//

	bool
	ResourceExists(
		__in const NWN::ResRef32 & ResType,
		__in NWN::ResType Type
		);

	//
	// Release a reference to a previously demand-loaded resource file.
	//

	void
	Release(
		__in const std::string & ResourceName
		);

	//
	// Retrieve the resource manager's temp file path.  Files created under
	// this path will be cleaned up by last-ditch cleanup when a new resource
	// manager instance starts.
	//
	// The returned file path shall have a trailing path separator character.
	//

	inline
	const std::string &
	GetResTempPath(
		) const
	{
		return m_TempPath;
	}

	//
	// Return the unique instance differentiator for this resource manager.
	//

	inline
	const std::string &
	GetInstanceUniqueIdentifier(
		) const
	{
		return m_TempUnique;
	}

	//
	// Convert a resref into a textural string.
	//

	inline
	std::string
	StrFromResRef(
		__in const NWN::ResRef32 & ResRef
		) const
	{
		std::string   R;
		const char  * p;

		p = (const char *) memchr(
			ResRef.RefStr,
			'\0' ,
			sizeof( ResRef.RefStr ) );

		if (p == NULL)
			R.assign( ResRef.RefStr, sizeof( ResRef.RefStr ) );
		else
			R.assign( ResRef.RefStr, p - ResRef.RefStr );

		for (size_t i = 0; i < R.size( ); i += 1)
		{
			R[ i ] = (char) tolower( (int) (unsigned char) R[ i ] );
		}

		return R;
	}

	inline
	std::string
	StrFromResRef(
		__in const NWN::ResRef16 & ResRef
		) const
	{
		std::string   R;
		const char  * p;

		p = (const char *) memchr(
			ResRef.RefStr,
			'\0',
			sizeof( ResRef.RefStr ) );

		if (p == NULL)
			R.assign( ResRef.RefStr, sizeof( ResRef.RefStr ) );
		else
			R.assign( ResRef.RefStr, p - ResRef.RefStr );

		return R;
	}

	//
	// Convert a textural string into a resref.
	//

	inline
	static
	NWN::ResRef32
	ResRef32FromStr(
		__in const std::string & Str
		)
	{
		NWN::ResRef32 ResRef;
		size_t        l;

		ZeroMemory( &ResRef, sizeof( ResRef ) ); // Satisfy overzealous compiler

		l = Str.length( );
		l = min( l, sizeof( ResRef ) );

		if (l != 0)
		{
			for (size_t i = 0; i < l; i += 1)
				ResRef.RefStr[ i ] = (char) tolower( Str[ i ] );
		}

		if (l < sizeof( ResRef ) )
			memset( &ResRef.RefStr[ l ], 0, sizeof( ResRef ) - l );

		return ResRef;
	}

	inline
	static
	NWN::ResRef16
	ResRef16FromStr(
		__in const std::string & Str
		)
	{
		NWN::ResRef16 ResRef;
		size_t        l;

		l = Str.length( );
		l = min( l, sizeof( ResRef ) );

		if (l != 0)
		{
			for (size_t i = 0; i < l; i += 1)
				ResRef.RefStr[ i ] = (char) tolower( Str[ i ] );
		}

		if (l < sizeof( ResRef ) )
			memset( &ResRef.RefStr[ l ], 0, sizeof( ResRef ) - l );

		return ResRef;
	}

	//
	// Check if a campaign ID represents a real campaign or the dummy campaign.
	//

	inline
	bool
	IsRealCampaignID(
		__in const GUID * CampaignID
		) const
	{
		GUID EmptyGuid;

		ZeroMemory( &EmptyGuid, sizeof( EmptyGuid ) );
		memcpy( &EmptyGuid, "AdHocGUID", 9 );

		if (!memcmp( CampaignID, &EmptyGuid, sizeof( EmptyGuid ) ))
			return false;
		else
			return true;
	}

	//
	// Access the mesh manager.
	//

	inline
	MeshManager &
	GetMeshManager(
		)
	{
		return m_MeshManager;
	}

	inline
	const MeshManager &
	GetMeshManager(
		) const
	{
		return m_MeshManager;
	}

	//
	// IResourceAccessor methods for granular access to a resource.  The APIs
	// are backed against the IResourceAccessor implementation for the resource
	// provider that claimed the resource.
	//
	// Not all resource accessor providers support multiple concurrent open
	// file handles nor random access within a file.  In general, users are
	// encouraged to avoid deviating from these guidelines for maximum
	// compatibility.
	//
	// In particular, compressed file backends typically do not support random
	// file access, and also might not support multiple concurrent open file
	// handles.
	//

	//
	// Open an encapsulated file by resref.
	//

	virtual
	FileHandle
	OpenFile(
		__in const ResRefT & ResRef,
		__in ResType Type
		);

	//
	// Open an encapsulated file by file index.
	//

	virtual
	FileHandle
	OpenFileByIndex(
		__in FileId FileIndex
		);

	//
	// Close an encapsulated file.
	//

	virtual
	bool
	CloseFile(
		__in FileHandle File
		);

	//
	// Read an encapsulated file by file handle.  The routine is optimized to
	// operate for sequential file reads.
	//

	virtual
	bool
	ReadEncapsulatedFile(
		__in FileHandle File,
		__in size_t Offset,
		__in size_t BytesToRead,
		__out size_t * BytesRead,
		__out_bcount( BytesToRead ) void * Buffer
		);

	//
	// Return the size of a file.
	//

	virtual
	size_t
	GetEncapsulatedFileSize(
		__in FileHandle File
		);

	//
	// Return the resource type of a file.
	//

	virtual
	ResType
	GetEncapsulatedFileType(
		__in FileHandle File
		);

	//
	// Iterate through resources in this resource accessor.  The routine
	// returns false on failure.
	//

	virtual
	bool
	GetEncapsulatedFileEntry(
		__in FileId FileIndex,
		__out ResRefT & ResRef,
		__out ResType & Type
		);

	//
	// Return the count of encapsulated files in this accessor.
	//

	virtual
	FileId
	GetEncapsulatedFileCount(
		);

	//
	// Get the logical name of this accessor.
	//

	virtual
	AccessorType
	GetResourceAccessorName(
		__in FileHandle File,
		__out std::string & AccessorName
		);

	//
	// Check a resource file name to ensure it will not escape out of the
	// current directory.
	//

	void
	CheckResFileName(
		__in const std::string & ResFileName
		) const
	{
		if ((ResFileName.find( '\\' ) != std::string::npos) ||
			(ResFileName.find( '/'  ) != std::string::npos) ||
			(ResFileName.find( ".." ) != std::string::npos))
		{
			throw std::runtime_error(
				"Resource name has path characters (illegal).");
		}

		if ((ResFileName == "PRN") ||
		    (ResFileName == "AUX") ||
		    (ResFileName == "CON") ||
		    (ResFileName == "NUL") ||
		    (ResFileName == "CONIN$") ||
		    (ResFileName == "CONOUT$") ||
		    (ResFileName == "CLOCK$"))
		{
			throw std::runtime_error(
				"Resource name is a DOS device (illegal).");
		}

		if (ResFileName.size( ) == 4)
		{
			std::string Prefix = ResFileName.substr( 0, 3 );

			if ((Prefix == "COM") ||
			    (Prefix == "LPT"))
			{
				if (isdigit( (int) (unsigned char) ResFileName[ 3 ] ))
				{
					throw std::runtime_error(
						"Resource name is a DOS device (illegal).");
				}
			}
		}
	}

	//
	// Read a file into a vector, given a resource accessor and file index.
	//

	template< typename ResRefType >
	static
	void
	LoadEncapsulatedFile(
		__in IResourceAccessor< ResRefType > * Accessor,
		__in typename IResourceAccessor< ResRefType >::FileId FileIndex,
		__out std::vector< unsigned char > & FileContents
		);

private:

	inline
	std::string
	GetModulePath(
		__in bool Directory,
		__in bool UseHome
		) const
	{
		std::string Str( UseHome ? m_HomeDir : m_InstallDir );

		Str += "/Modules/";
		Str += m_ModuleResName;

		if (Directory)
			return Str;

		Str += ".mod";

		return Str;
	}

	inline
	std::string
	GetModuleNWMPath(
		) const
	{
		std::string Str( m_InstallDir );

		Str += "/nwm/";
		Str += m_ModuleResName;
		Str += ".nwm";

		return Str;
	}

	inline
	std::string
	GetCampaignPath(
		__in bool UseHome
		) const
	{
		std::string Str( UseHome ? m_HomeDir : m_InstallDir );

		Str += "/Campaigns/";

		return Str;
	}

	//
	// Called to perform the core work for LoadModuleResources and
	// LoadModuleResourcesLite.
	//

	void
	LoadModuleResourcesInternal(
		__in const std::string & ModuleResName,
		__in const std::string & AltTlkFileName,
		__in const std::string & HomeDir,
		__in const std::string & InstallDir,
		__in const std::vector< NWN::ResRef32 > & HAKs,
		__in_opt ModuleLoadParams * LoadParams,
		__in bool PartialLoadOnly
		);

	//
	// Load all .hak files.
	//

	template< typename ResRefLoadType, const size_t LoadTier >
	void
	LoadHAKFiles(
		__in const std::vector< NWN::ResRef32 > & HAKs
		);

	//
	// Load the base module.
	//

	void
	LoadModule(
		__in ModuleSearchOrder SearchOrder,
		__in_opt const char * CustomModuleSourcePath
		);

	//
	// Load the module campaign (if there was one).
	//

	void
	LoadCampaign(
		__in const GUID & CampaignID,
		__out GUID & CampaignIDUsed
		);

	//
	// Load all other directories.
	//

	void
	LoadDirectories(
		__in_opt const char * CustomSearchPath
		);

	//
	// Load all in-box .zip archives.
	//

	void
	LoadZipArchives(
		);

	//
	// Load all in-box .key/.bif archives.
	//

	void
	LoadFixedKeyFiles(
		__in const StringVec & KeyFiles
		);

	//
	// Load custom resource providers.
	//

	void
	LoadCustomResourceProviders(
		__in_ecount( ProviderCount ) IResourceAccessor * const * Providers,
		__in size_t ProviderCount,
		__in bool FirstChance
		);

	//
	// Load all .zip archives in a directory, using the canonical order, which is
	// to load in filesystem enumeration order (assumed to be alpha order).
	// 

	void
	LoadDirectoryZipFiles(
		__in const std::string & DirName
		);

	//
	// Load talk tables from our existing resource pool.
	//

	void
	LoadTalkTables(
		__in const std::string & AltTlkFileName
		);

	//
	// Locate the file path for a resource using a relative path name, for
	// resource types that may be in either the home or install dir, using the
	// standard search order.
	//

	std::string
	GetResourceFilePath(
		__in const std::string & RelPathName
		);

	//
	// Check that module.ifo exists in a path (or that module.ifo checking is
	// disabled).
	//

	bool
	CheckModuleIfo(
		__in const std::string & ModulePath,
		__in bool Erf
		) const;

	//
	// Check whether a file exists in a resource accessor.  Note that it is
	// assumed that no files are open in the accessor.
	//

	bool
	DoesFileExistInResourceAccessor(
		__in IResourceAccessor & Accessor,
		__in const NWN::ResRef32 & ResRef,
		__in NWN::ResType ResType
		) const;

	//
	// Forcibly close open file handles.
	//

	size_t
	ForceCloseOpenFileHandles(
		);

	//
	// Clean up demand-loaded files.
	//

	void
	CleanDemandLoadedFiles(
		);

	//
	// Clean up old instance temp files that weren't removed cleanly.
	//

	void
	CleanOldTempFiles(
		__in const char * TempPath
		);

	//
	// Remove all files in a directory.
	//

	void
	DeleteDirectoryFiles(
		__in const char * DirPath
		);

	//
	// Scan all resource accessors and create the master resource id list.
	//

	void
	DiscoverResources(
		);

	//
	// Allocate a file handle for the overarching resource manager file
	// accessor interface.  This file handle may be used with the direct
	// implementation of IResourceAccessor that is provided by the resource
	// manager itself.
	//
	// The routine returns INVALID_FILE should there be no available file
	// handles.
	//

	FileHandle
	AllocateFileHandle(
		);

	//
	// Acquire a pointer to a TwoDAFileReader given a 2DA Resref name.  If the
	// 2DA has not already been cached, it will be made cached by the routine.
	//

	const TwoDAFileReader *
	Get2DA(
		__in const std::string & ResourceName
		);

	//
	// Return the appropriate HAK vector for a given ResRef type.
	//
	// N.B.  The generalized template is a dummy and is never used, only the
	//       specializations may be invoked.
	//

	template< typename ResRefLoadType >
	inline
	std::vector< swutil::SharedPtr< ::ErfFileReader< ResRefLoadType > > > &
	GetHakFiles(
		)
	{
		C_ASSERT( sizeof( ResRefLoadType ) != sizeof( ResRefLoadType ) );

		NWN_ASSERT( 0 );

		return *new std::vector< swutil::SharedPtr< ResRefLoadType > >( );
	}

	template< >
	inline
	std::vector< swutil::SharedPtr< ::ErfFileReader< NWN::ResRef32 > > > &
	GetHakFiles(
		)
	{
		return m_HakFiles;
	}

	template< >
	inline
	std::vector< swutil::SharedPtr< ::ErfFileReader< NWN::ResRef16 > > > &
	GetHakFiles(
		)
	{
		return m_HakFiles16;
	}

	//
	// Push a HAK onto the HAK list (using the appropriate tier based on what
	// format of HAKs we're loading).
	//

	IResourceAccessor *
	LoadEncapsulatedFile(
		__in const std::string & Path
		)
	{
		IResourceAccessor * Accessor;

		if ((m_ResManFlags & ResManFlagErf16) == 0)
		{
			swutil::SharedPtr< ::ErfFileReader32 > Erf;

			Erf = new ::ErfFileReader32( Path );
			m_HakFiles.push_back( Erf );

			Accessor = Erf.get( );

			m_ResourceFiles[ TIER_ENCAPSULATED ].push_back( Accessor );
		}
		else
		{
			swutil::SharedPtr< ::ErfFileReader16 > Erf;

			Erf = new ::ErfFileReader16( Path );
			m_HakFiles16.push_back( Erf );

			Accessor = Erf.get( );

			m_ResourceFiles[ TIER_ENCAPSULAT16 ].push_back( Accessor );
		}

		return Accessor;
	}


	typedef TlkFileReader16 TlkFileReader;
	typedef ErfFileReader32 ErfFileReader;
	typedef DirectoryFileReader32 DirectoryFileReader;
	typedef ZipFileReader32 ZipFileReader;
	typedef KeyFileReader16 KeyFileReader;

	typedef swutil::SharedPtr< TlkFileReader > TlkFileReaderPtr;
	typedef swutil::SharedPtr< TwoDAFileReader > TwoDAFileReaderPtr;

	typedef swutil::SharedPtr< ErfFileReader > ErfFileReaderPtr;
	typedef swutil::SharedPtr< ErfFileReader16 > ErfFileReader16Ptr;
	typedef swutil::SharedPtr< DirectoryFileReader > DirectoryFileReaderPtr;
	typedef swutil::SharedPtr< ZipFileReader > ZipFileReaderPtr;
	typedef swutil::SharedPtr< KeyFileReader > KeyFileReaderPtr;

	//
	// Define the resource handle type, to which a FileHandle refers to for the
	// overarching ResourceManager object.
	//

	struct ResHandle
	{
		IResourceAccessor * Accessor;
		FileHandle          Handle;
		ResType             Type;
	};

	//
	// Define book-keeping reference data for referring to a demanded file by
	// resref and type.  This allows multiple callers to demand the same file
	// and get a full filesystem path back, and furthermore allows us to only
	// keep the file on disk as long as we actually require it.
	//

	struct DemandResourceRef
	{
		std::string ResourceFileName;
		size_t      Refs;
		bool        Delete;
	};

	typedef const enum _PROVIDER_TYPE * PCPROVIDER_TYPE;

	//
	// Define the resource directory entry, used to provide quick access to
	// files across all resource accessors, in canonical order.
	//

	struct ResourceEntry
	{
		IResourceAccessor * Accessor;
		FileId              FileIndex;
		size_t              Tier;
		size_t              TierIndex; // From end
	};

	//
	// Priority order between resource types.
	//

	enum
	{
		TIER_CUSTOM_FIRST = 0, // First chance custom (external) accessors
		TIER_ENCAPSULATED = 1, // 32-byte ResRef ERFs
		TIER_ENCAPSULAT16 = 2, // 16-byte ResRef ERFs
		TIER_DIRECTORY    = 3, // Filesystem directory hierarchy
		TIER_INBOX        = 4, // Zip file in-box data
		TIER_INBOX_KEY    = 5, // KEY/BIF file in-box data
		TIER_CUSTOM_LAST  = 6, // Last chance custom (external) accessors

		MAX_TIERS
	};

	enum
	{
		STRREF_INVALID     = 0xFFFFFFFF,
		STRREF_IDMASK      = 0x00FFFFFF,
		STRREF_TABLEMASK   = 0x01000000,

		LAST_STRREF_MASK
	};

	//
	// Resref to file path mapping.
	//

	typedef std::map< std::string, DemandResourceRef > ResRefNameMap;

	//
	// Demand load hak list.
	//

	typedef std::vector< ErfFileReaderPtr > HakFileVec;
	typedef std::vector< ErfFileReader16Ptr > HakFile16Vec;

	//
	// Demand load directory list.
	//

	typedef std::vector< DirectoryFileReaderPtr > DirFileVec;

	//
	// Demand load zip file list.
	//

	typedef std::vector< ZipFileReaderPtr > ZipFileVec;

	//
	// Demand load key file list.
	//

	typedef std::vector< KeyFileReaderPtr > KeyFileVec;

	//
	// Global resource load list in priority order.
	//

	typedef std::vector< IResourceAccessor * > ResourceAccessorVec;

	//
	// Open handle mapping, used to redirect requests for service to their
	// underlying resource accessor implementations.
	//

	typedef std::map< FileHandle, ResHandle > ResHandleMap;

	//
	// Define the mapping of resource names to resource entries, used to open
	// by name quickly.
	//

	typedef stdext::hash_map< std::string, size_t > ResourceEntryMap;

	//
	// Define the array of all known resources.
	//

	typedef std::vector< ResourceEntry > ResourceEntryVec;

	//
	// Mapping type to map between 2DA RESREFs and TwoDAFileReader instances
	// that are used to access the underlying data for a particular 2DA.
	//

	typedef std::map< std::string, TwoDAFileReaderPtr > TwoDANameMap;

	//
	// Gr2Accessor shared pointer.
	//

	typedef swutil::SharedPtr< Gr2Accessor > Gr2AccessorPtr;

	//
	// Text output writer, used to display debug warnings to the user, or to a
	// log file.
	//

	IDebugTextOut           * m_TextWriter;

	//
	// Next free file handle.
	//

	FileHandle                m_NextFileHandle;

	//
	// Base resource data.
	//

	std::string               m_ModuleResName; // Module name for Modules directory
	std::string               m_HomeDir;    // User home directory, i.e. docs\NWN2
	std::string               m_InstallDir; // Game install directory with base data

	//
	// Talk tables.
	//

	TlkFileReaderPtr          m_BaseTlk;
	TlkFileReaderPtr          m_AlternateTlk;

	//
	// 2DA lookup mappings.
	//

	TwoDANameMap              m_2DAs;

	//
	// Resource load sources (in priority order).  These may be ERF or ZIP file
	// readers, or directory file readers.
	//

	ResourceAccessorVec       m_ResourceFiles[ MAX_TIERS ];

	//
	// Mapping of all demanded files to resrefs.
	//

	ResRefNameMap             m_NameMap;

	//
	// Hak files loaded.
	//

	HakFileVec                m_HakFiles;
	HakFile16Vec              m_HakFiles16;

	//
	// Directories loaded.
	//

	DirFileVec                m_DirFiles;

	//
	// Zip files loaded.
	//

	ZipFileVec                m_ZipFiles;

	//
	// Key files loaded.
	//

	KeyFileVec                m_KeyFiles;

	//
	// Active resource handles.
	//

	ResHandleMap              m_ResFileHandles;

	//
	// Mapping of all resource names (+types) to resource entry indicies.
	//
	// Keys are formed based on a convention of '<type>T<resref>', where
	// <type> is the base-10 string representation of the resource type code,
	// and 'resref' is the canonical (all-lowercase) form of the resource name.
	//
	// An uppercase character is never found in a canonical resource name, so
	// the search string is unambiguous.
	//

	ResourceEntryMap          m_NameIdMap;

	//
	// Array of all loaded resource identifiers with their associated accessor
	// objects.  Indicies into this array form ResourceManager FileIds.
	//

	ResourceEntryVec          m_ResourceEntries;

	//
	// Unique identifier for instance disambiguation in the temp storage path.
	//

	std::string               m_TempUnique;

	//
	// Temp storage path.
	//

	std::string               m_TempPath;

	//
	// Instance identification.
	//

	HANDLE                    m_InstanceEvent;

	//
	// Granny2 file accessor, used to convert *.gr2 files into a readable form
	// for Gr2FileReader instances.  There is typically one Gr2Accessor object
	// per program.
	//

	Gr2AccessorPtr            m_Gr2Accessor;

	//
	// Define the overarching mesh manager which maintains links to all mesh
	// objects for graphics related operations (i.e. device dependent reloads).
	//

	MeshManager               m_MeshManager;

	//
	// Define operational flags for the resource manager.  Legal values are
	// drawn from ResManFlags.
	//

	unsigned long             m_ResManFlags;

};

//
// Resource manager resref lookup reference class, to auto-release a demanded
// resource in a scoped fashion.
//

template< class T >
class DemandResource
{

public:

	//
	// Demand the resource and scope the reference for automatic cleanup on
	// object destructor.
	//
	// N.B.  The routine raises an std::exception should the underlying Demand
	//       call fail.
	//

	inline
	DemandResource(
		__in ResourceManager & RM,
		__in const T & ResRef,
		__in NWN::ResType Type
		)
	: m_RM( RM ),
	  m_ResRef( ResRef ),
	  m_ResType( Type )
	{
		m_DemandedFileName = m_RM.Demand( ResRef, Type );
	}

	//
	// Release the demanded resource.
	//

	inline
	~DemandResource(
		)
	{
		m_RM.Release( m_DemandedFileName );
	}

	//
	// Cast to const std::string & to retrieve the demanded filename.
	//

	inline
	operator const std::string &(
		) const
	{
		return m_DemandedFileName;
	}

	//
	// Retrieve the demanded filename.
	//

	inline
	const std::string &
	GetDemandedFileName(
		) const
	{
		return m_DemandedFileName;
	}

	//
	// Copy constructor and assignment.
	//

	inline
	DemandResource(
		__in const DemandResource & other
		)
	: m_RM( other.m_RM ),
	  m_ResRef( other.m_ResRef ),
	  m_ResType( other.m_ResType )
	{
		//
		// Re-demand the resource to acquire another reference to it.
		//

		m_DemandedFileName = m_RM.Demand( m_ResRef, m_ResType );
	}

	inline
	DemandResource &
	operator=(
		__in const DemandResource & other
		)
	{
		m_RM.Release( m_DemandedFileName );

		m_ResRef  = other.m_ResRef;
		m_ResType = other.m_ResType;

		//
		// Re-demand the resource to acquire another reference to it.
		//

		m_DemandedFileName = m_RM.Demand( m_ResRef, m_ResType );

		return *this;
	}

private:

	ResourceManager & m_RM;
	std::string       m_DemandedFileName;
	T                 m_ResRef;
	NWN::ResType      m_ResType;

};

typedef DemandResource< std::string >   DemandResourceStr;
typedef DemandResource< NWN::ResRef32 > DemandResource32;
typedef DemandResource< NWN::ResRef16 > DemandResource16;

#endif
