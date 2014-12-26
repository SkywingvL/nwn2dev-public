/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ResourceManager.cpp

Abstract:

	This module houses the implementation of the resource manager, which is
	used to provide access to data files and string entries for the current
	module.

--*/

#include "Precomp.h"
#include "ResourceManager.h"
#include "TextOut.h"



//
// Define to 2 to enable very verbose output.
// Define to 1 to enable performance statistics tracing.
//

#define RES_DEBUG 0



#if RES_DEBUG
#include "TextOut.h"
#define ResDebug m_TextWriter->WriteText
#else
#define ResDebug  __noop
#endif // RES_DEBUG

#if RES_DEBUG >= 1
#define ResDebug1 ResDebug
#else
#define ResDebug1 __noop
#endif // RES_DEBUG >= 1

#if RES_DEBUG >= 2
#define ResDebug2 ResDebug
#else
#define ResDebug2 __noop
#endif // RES_DEBUG >= 2

//
// Define to use optimized indexed file access.  This is necessary to support
// files that are multi-versioned or nested in subdirectories.
//

#define USE_INDEX 1

ResourceManager::ResourceManager(
	__in IDebugTextOut * TextWriter,
	__in unsigned long CreateFlags /* = 0 */
	)
/*++

Routine Description:

	This routine constructs a new ResourceManager object.

Arguments:

	TextWriter - Supplies the text output implementation that is used to
	             indicate debug log messages upwards.

	CreateFlags - Supplies instance creation control flags.  Legal values are
	              drawn from the ResManCreateFlags enumeration.

Return Value:

	The newly constructed object.

Environment:

	User mode.

--*/
: m_TextWriter( TextWriter ),
  m_NextFileHandle( 0 ),
  m_Gr2Accessor( NULL ),
  m_ResManFlags( 0 )
{
	CHAR TempPath[ MAX_PATH + 1 ];
	CHAR TempUnique[ 32 ];

	if (CreateFlags & ResManCreateFlagNoInstanceSetup)
		return;

	if (!GetTempPathA( MAX_PATH, TempPath ))
		TempPath[ 0 ] = '\0';

	StringCbPrintfA(
		TempUnique,
		sizeof( TempUnique ),
		"NWN2CliExt_%lu",
		GetCurrentProcessId( ) );

	m_TempUnique = TempUnique;

	//
	// A named event is used to communicate to other instances that they should
	// not clean up our temp directory.
	//

	m_InstanceEvent = CreateEventA( NULL, TRUE, FALSE, TempUnique );

	//
	// Prepare the temporary storage path for use.
	//

	ChangeTemporaryDirectory( TempPath );
}

ResourceManager::~ResourceManager(
	)
/*++

Routine Description:

	This routine cleans up an already-existing ResourceManager object.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	CleanDemandLoadedFiles( );

	RemoveDirectoryA( m_TempPath.c_str( ) );

	if (m_InstanceEvent != NULL)
	{
		CloseHandle( m_InstanceEvent );
		m_InstanceEvent = NULL;
	}
}

void
ResourceManager::LoadModuleResources(
	__in const std::string & ModuleResName,
	__in const std::string & AltTlkFileName,
	__in const std::string & HomeDir,
	__in const std::string & InstallDir,
	__in const std::vector< NWN::ResRef32 > & HAKs,
	__in_opt ModuleLoadParams * LoadParams /* = NULL */
	)
/*++

Routine Description:

	This routine is invoked when all module resources have been downloaded and
	are available.  It bears responsibility for loading always-load resources
	such as TLK files, as well as setting up other state necessary for the
	proper servicing of demand-load requests.

Arguments:

	ModuleResName - Supplies the resource name of the current module.

	AltTlkFileName - Optionally supplies the name of an alternate tlk file.

	HomeDir - Supplies the game home directory, i.e. Docs\Neverwinter Nights 2.

	InstallDir - Supplies the installation directory of the game.

	HAKs - Supplies the list of HAK files to load.

	LoadParams - Optionally supplies extended module load parameters, such as
	             the campaign ID.

Return Value:

	The routine raises an std::exception on catastrophic failure.

Environment:

	User mode.

--*/
{
	LoadModuleResourcesInternal(
		ModuleResName,
		AltTlkFileName,
		HomeDir,
		InstallDir,
		HAKs,
		LoadParams,
		false);
}

void
ResourceManager::LoadModuleResourcesLite(
	__in const std::string & ModuleResName,
	__in const std::string & HomeDir,
	__in const std::string & InstallDir
	)
/*++

Routine Description:

	This routine is invoked to bring up just enough of the resource manager
	system to parse data out of a module.  The caller is expected to perform a
	subsequent call to LoadModuleResources once critical parameters such as the
	HAK list have been discovered by processing the module resources.

Arguments:

	ModuleResName - Supplies the resource name of the current module.

	HomeDir - Supplies the game home directory, i.e. Docs\Neverwinter Nights 2.

	InstallDir - Supplies the installation directory of the game.

Return Value:

	The routine raises an std::exception on catastrophic failure.

Environment:

	User mode.

--*/
{
	LoadModuleResourcesInternal(
		ModuleResName,
		"",
		HomeDir,
		InstallDir,
		std::vector< NWN::ResRef32 >( ),
		NULL,
		true);
}

void
ResourceManager::UnloadAllResources(
	)
/*++

Routine Description:

	This routine is invoked to deregister all resource providers and unload all
	still loaded resources.  All open demanded resource references and file
	handles become invalidated.  A subsequent call to LoadModuleResources must
	be made before the resource system will service new requests again.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	CleanDemandLoadedFiles( );
}

size_t
ResourceManager::CloseOpenResourceFileHandles(
	)
/*++

Routine Description:

	This routine forcibly closes all open files.  Any file handles that were
	still open are invalidated (which would cause them to operate incorrectly
	on the next usage).

Arguments:

	None.

Return Value:

	The routine returns the number of outstanding file handles that had to be
	forcibly closed.

Environment:

	User mode.

--*/
{
	return ForceCloseOpenFileHandles( );
}

void
ResourceManager::ChangeTemporaryDirectory(
	__in const std::string & TempDirectory
	)
/*++

Routine Description:

	This routine sets the temporary storage location for the resource manager.
	If there were any previous data items in the temporary storage location,
	they are removed.

	The caller bears responsibility for only invoking this routine prior to the
	resource manager having loaded any data resources, as the resource temp
	path being invalidated and no attempt is made to update opened resource
	files.

Arguments:

	TempDirectory - Supplies the directory to store temporary data in.  The
	                resource manager uses a uniquely named subdirectory to
	                hold any temporary files.  Any previous instances that are
	                now defunct under this path are also cleaned up.

Return Value:

	The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	if (!TempDirectory.empty( ))
		CreateDirectoryA( TempDirectory.c_str( ), NULL );

	m_TempPath  = TempDirectory;
	m_TempPath += m_TempUnique;
	m_TempPath += "\\";

	//
	// A previous instance might have had the same path as us, so delete it.
	//

	DeleteDirectoryFiles( m_TempPath.c_str( ) );

	CleanOldTempFiles( TempDirectory.c_str( ) );

	//
	// Initialize the temp directory.
	//

	CreateDirectoryA( m_TempPath.c_str( ), NULL );
}

std::string
ResourceManager::Demand(
	__in const std::string & ResRef,
	__in ResType Type
	)
/*++

Routine Description:

	This routine demand-loads a resource and returns a filesystem path that may
	be used to access the resource.

	The returned path may point to the raw resource for directory resources, or
	it may point to a temporary file if the resource needed to be unpacked.

	Because demand-loading to a filesystem path may involve a full copy of the
	entire content encapsulated file in question, callers should only use this
	API if they are dealing with short files, or expect to access the entire
	file contents.

	Otherwise, it is advisable to use the built-in IResourceAccessor APIs which
	provide direct access to the resource file (optimized for sequential scan
	access).  The IResourceAccessor APIs do not require a copy in the
	encapsulated file case.

	The caller should release the resource with a call to Release after
	resource utilization is finished.

Arguments:

	ResRef - Supplies the resource reference identifying the name of the
	         resource to load.

	Type - Supplies the type of the resource to load.

Return Value:

	A fully qualified file name that may be used to access the demanded file is
	returned on success.  The file name is only valid until a matching call to
	Release is made.

	The routine raises an std::exception on failure.

Environment:

	User mode.

--*/
{
#if USE_INDEX
	NWN::ResRef32                    ResRef32;
	std::string                      ResPath;
	HANDLE                           ResFile;
	char                             Msg[ 512 ];
	ResRefNameMap::iterator          nit;
	ResourceEntryMap::const_iterator eit;
	std::string                      LookupName;

	LookupName  = _itoa( (int) Type, Msg, 10 );
	LookupName.push_back( 'T' );
	LookupName += ResRef;

	ResFile = INVALID_HANDLE_VALUE;

	//
	// First, check the cache to see if we've already located this one.
	//

	if ((nit = m_NameMap.find( LookupName )) != m_NameMap.end( ))
	{
		nit->second.Refs++;

		try
		{
			return nit->second.ResourceFileName;
		}
		catch (...)
		{
			nit->second.Refs--;
			throw;
		}
	}

	CheckResFileName( ResRef );

	ZeroMemory( ResRef32.RefStr, sizeof( ResRef32.RefStr ) );

	if (ResRef.empty( ))
	{
		throw std::runtime_error(
			"Attempted to demand load the null resource." );
	}

	//
	// Look up the file in our index mapping.
	//

	if ((eit = m_NameIdMap.find( LookupName )) != m_NameIdMap.end( ))
	{
		DemandResourceRef     Ref;
		FileHandle            Handle;
		const ResourceEntry * Entry;

		Entry = &m_ResourceEntries[ eit->second ];

		//
		// Pull the file and return it to the caller.
		//

		if (Entry->Tier == TIER_DIRECTORY)
		{
			size_t DirIndex;

			//
			// As a special optimization, directly use the source file for
			// directory tiers.
			//

			//
			// Pick the right source directory as we were traversing in
			// reverse order.
			//

			DirIndex = m_DirFiles.size( ) - Entry->TierIndex;

			ResPath  = m_DirFiles[ DirIndex ]->GetRealFileName(
				Entry->FileIndex );

			//
			// Add the file to the quick lookup list.
			//

			Ref.ResourceFileName = ResPath;
			Ref.Refs             = 1;
			Ref.Delete           = false;

			m_NameMap.insert(
				ResRefNameMap::value_type( LookupName, Ref ) );

			return ResPath;
		}

		Handle = INVALID_FILE;

		//
		// Copy the file to a temp location.
		//

		try
		{
			size_t FileSize;
			size_t BytesLeft;
			size_t Offset;
			LONG   DistHigh;

			//
			// Open a handle to the file.
			//

			Handle = Entry->Accessor->OpenFileByIndex( Entry->FileIndex );

			if (Handle == INVALID_FILE)
			{
				StringCbPrintfA(
					Msg,
					sizeof( Msg ),
					"Failed to open RESREF '%s'",
					ResRef.c_str( ) );
				throw std::runtime_error( Msg );
			}

			//
			// First, acquire a resource filename for the resource file.
			//

			ResPath  = m_TempPath;
			ResPath += ResRef;
			ResPath += ".";
			ResPath += ResTypeToExt( Type );

			//
			// Open the temp file.
			//

			ResFile = CreateFileA(
				ResPath.c_str( ),
				GENERIC_WRITE,
				FILE_SHARE_WRITE,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_TEMPORARY,
				NULL);

			if (ResFile == INVALID_HANDLE_VALUE)
			{
				StringCbPrintfA(
					Msg,
					sizeof( Msg ),
					"CreateFile( %s ) failed",
					ResPath.c_str( ) );

				throw std::runtime_error( Msg );
			}

			//
			// Copy contents over.
			//

			FileSize = Entry->Accessor->GetEncapsulatedFileSize( Handle );

#ifdef _WIN64
			DistHigh = (LONG) (FileSize >> 32);
#else
			DistHigh = 0L;
#endif

			//
			// Allocate the entire file up-front so that we don't fragment
			// the file system.
			//

			if (SetFilePointer(
				ResFile,
				(LONG) (FileSize & 0xFFFFFFFF),
				&DistHigh,
				FILE_BEGIN ))
			{
				SetEndOfFile( ResFile );
				SetFilePointer( ResFile, 0, NULL, FILE_BEGIN );
			}

			BytesLeft = FileSize;
			Offset    = 0;

			while (BytesLeft)
			{
				enum { CHUNK_SIZE = 4096 };

				unsigned char Buffer[ CHUNK_SIZE ];
				size_t        Read;
				DWORD         Written;

				if (!Entry->Accessor->ReadEncapsulatedFile(
					Handle,
					Offset,
					min( BytesLeft, CHUNK_SIZE ),
					&Read,
					Buffer))
				{
					throw std::runtime_error(
						"ReadEncapsulatedFile failed" );
				}

				if (!WriteFile(
					ResFile,
					Buffer,
					(DWORD) Read,
					&Written,
					NULL))
				{
					throw std::runtime_error( "WriteFile failed" );
				}

				if (Written != (DWORD) Read)
					throw std::runtime_error( "Short write" );

				Offset    += Read;
				BytesLeft -= Read;
			}

			Ref.ResourceFileName = ResPath;
			Ref.Refs             = 1;
			Ref.Delete           = true;

			m_NameMap.insert(
				ResRefNameMap::value_type( LookupName, Ref ) );
		}
		catch (std::exception &e)
		{
			if (Handle != INVALID_FILE)
				Entry->Accessor->CloseFile( Handle );

			if (ResFile != INVALID_HANDLE_VALUE)
				CloseHandle( ResFile );

			if (!ResPath.empty( ))
				DeleteFileA( ResPath.c_str( ) );

			m_TextWriter->WriteText(
				"WARNING: Exception '%s' loading resource '%s' (type %04X).\n",
				e.what( ),
				ResRef.c_str( ),
				(unsigned short) Type);

			throw;
		}
		catch (...)
		{
			if (Handle != INVALID_FILE)
				Entry->Accessor->CloseFile( Handle );

			if (ResFile != INVALID_HANDLE_VALUE)
				CloseHandle( ResFile );

			if (!ResPath.empty( ))
				DeleteFileA( ResPath.c_str( ) );

			throw;
		}

		//
		// Close out both files and call it done.
		//

		CloseHandle( ResFile );
		Entry->Accessor->CloseFile( Handle );

		//
		// Hand the temporary path out to the caller.  It will persist
		// until all module content is unloaded.
		//

		return ResPath;
	}

#else
	NWN::ResRef32           ResRef32;
	std::string             ResPath;
	HANDLE                  ResFile;
	char                    Msg[ 512 ];
	ResRefNameMap::iterator nit;
	std::string             LookupName;

	LookupName  = _itoa( (int) Type, Msg, 10 );
	LookupName.push_back( 'T' );
	LookupName += ResRef;

	ResFile = INVALID_HANDLE_VALUE;

	//
	// First, check the cache to see if we've already located this one.
	//

	if ((nit = m_NameMap.find( LookupName )) != m_NameMap.end( ))
	{
		try
		{
			nit->second.Refs++;

			return nit->second.ResourceFileName;
		}
		catch (...)
		{
			nit->second.Refs--;
			throw;
		}
	}

	CheckResFileName( ResRef );

	ZeroMemory( ResRef32.RefStr, sizeof( ResRef32.RefStr ) );

	if (ResRef.empty( ))
	{
		throw std::runtime_error(
			"Attempted to demand load the null resource." );
	}

	memcpy(
		ResRef32.RefStr,
		&ResRef[ 0 ],
		min( ResRef.size( ), sizeof( ResRef32.RefStr ) ) );

	//
	// Search each tier in turn.
	//

	for (size_t i = 0; i < MAX_TIERS; i += 1)
	{
		size_t j;

		//
		// Search each tier in order, based on the defined behavior of the
		// BioWare resource manager.  The most recently added resource provider
		// is searched first.
		//

		j = 0;

		for (ResourceAccessorVec::reverse_iterator it = m_ResourceFiles[ i ].rbegin( );
		     it != m_ResourceFiles[ i ].rend( );
		     ++it)
		{
			FileHandle Handle;

			j += 1;

			//
			// Try and open the file.
			//

			Handle = (*it)->OpenFile( ResRef32, Type );

			if (Handle == INVALID_FILE)
				continue;

			//
			// We got the file, let's go with it.
			//

			if (i == TIER_DIRECTORY)
			{
				size_t DirIndex;

				//
				// As a special optimization, directly use the source file for
				// directory tiers.
				//

				(*it)->CloseFile( Handle );

				//
				// Pick the right source directory as we were traversing in
				// reverse order.
				//

				DirIndex = m_DirFiles.size( ) - j;

				ResPath  = m_DirFiles[ DirIndex ]->GetDirectoryName( );
				ResPath += ResRef;
				ResPath += ".";
				ResPath += ResTypeToExt( Type );

				//
				// Add the file to the quick lookup list.
				//

				try
				{
					DemandResourceRef Ref;

					Ref.ResourceFileName = ResPath;
					Ref.Refs             = 1;
					Ref.Delete           = false;

					m_NameMap.insert(
						ResRefNameMap::value_type( LookupName, Ref ) );
				}
				catch (std::exception)
				{
				}

				return ResPath;
			}

			//
			// Copy the file to a temp location.
			//

			try
			{
				size_t FileSize;
				size_t BytesLeft;
				size_t Offset;
				LONG   DistHigh;

				//
				// First, acquire a resource filename for the resource file.
				//

				ResPath  = m_TempPath;
				ResPath += ResRef;
				ResPath += ".";
				ResPath += ResTypeToExt( Type );

				//
				// Open the temp file.
				//

				ResFile = CreateFileA(
					ResPath.c_str( ),
					GENERIC_WRITE,
					FILE_SHARE_WRITE,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_TEMPORARY,
					NULL);

				if (ResFile == INVALID_HANDLE_VALUE)
				{
					StringCbPrintfA(
						Msg,
						sizeof( Msg ),
						"CreateFile( %s ) failed",
						ResPath.c_str( ) );

					throw std::runtime_error( Msg );
				}

				//
				// Copy contents over.
				//

				FileSize = (*it)->GetEncapsulatedFileSize( Handle );

#ifdef _WIN64
				DistHigh = (LONG) (FileSize >> 32);
#else
				DistHigh = 0L;
#endif

				//
				// Allocate the entire file up-front so that we don't fragment
				// the file system.
				//

				if (SetFilePointer(
					ResFile,
					(LONG) (FileSize & 0xFFFFFFFF),
					&DistHigh,
					FILE_BEGIN ))
				{
					SetEndOfFile( ResFile );
					SetFilePointer( ResFile, 0, NULL, FILE_BEGIN );
				}

				BytesLeft = FileSize;
				Offset    = 0;

				while (BytesLeft)
				{
					enum { CHUNK_SIZE = 4096 };

					unsigned char Buffer[ CHUNK_SIZE ];
					size_t        Read;
					DWORD         Written;

					if (!(*it)->ReadEncapsulatedFile(
						Handle,
						Offset,
						min( BytesLeft, CHUNK_SIZE ),
						&Read,
						Buffer))
					{
						throw std::runtime_error(
							"ReadEncapsulatedFile failed" );
					}

					if (!WriteFile(
						ResFile,
						Buffer,
						(DWORD) Read,
						&Written,
						NULL))
					{
						throw std::runtime_error( "WriteFile failed" );
					}

					if (Written != (DWORD) Read)
						throw std::runtime_error( "Short write" );

					Offset    += Read;
					BytesLeft -= Read;
				}

				DemandResourceRef Ref;

				Ref.ResourceFileName = ResPath;
				Ref.Refs             = 1;
				Ref.Delete           = true;

				m_NameMap.insert(
					ResRefNameMap::value_type( LookupName, Ref ) );
			}
			catch (std::exception &e)
			{
				(*it)->CloseFile( Handle );

				if (ResFile != INVALID_HANDLE_VALUE)
					CloseHandle( ResFile );

				if (!ResPath.empty( ))
					DeleteFileA( ResPath.c_str( ) );

				m_TextWriter->WriteText(
					"WARNING: Exception '%s' loading resource '%s'.\n",
					e.what( ),
					ResRef.c_str( ) );

				throw;
			}
			catch (...)
			{
				(*it)->CloseFile( Handle );

				if (ResFile != INVALID_HANDLE_VALUE)
					CloseHandle( ResFile );

				if (!ResPath.empty( ))
					DeleteFileA( ResPath.c_str( ) );

				throw;
			}

			//
			// Close out both files and call it done.
			//

			CloseHandle( ResFile );
			(*it)->CloseFile( Handle );

			//
			// Hand the temporary path out to the caller.  It will persist
			// until all module content is unloaded.
			//

			return ResPath;
		}
	}
#endif

	StringCbPrintfA(
		Msg,
		sizeof( Msg ),
		"Failed to locate RESREF '%s'",
		ResRef.c_str( ) );
	throw std::runtime_error( Msg );
}

bool
ResourceManager::ResourceExists(
	__in const NWN::ResRef32 & ResRef,
	__in NWN::ResType Type
	)
/*++

Routine Description:

	This routine determines whether a resource exists in the resource index,
	without attempting to open it.  It can be used to avoid the penalty of an
	exception in cases where resources are likely to not exist in non-error
	cases.

Arguments:

	ResRef - Supplies the resource reference identifying the name of the
	         resource to check.

	Type - Supplies the resource type.

Return Value:

	The routine returns a Boolean value indicating true if the resource could
	be located, else false.

Environment:

	User mode.

--*/
{
#if USE_INDEX
	char        TypeStr[ 32 ];
	std::string LookupName;

	try
	{
		LookupName  = _itoa( (int) Type, TypeStr, 10 );
		LookupName.push_back( 'T' );
		LookupName += StrFromResRef( ResRef );
	}
	catch (std::exception)
	{
		return false;
	}

	return (m_NameIdMap.find( LookupName ) != m_NameIdMap.end( ));
#else
	FileHandle Handle;

	Handle = OpenFile( ResRef, Type );

	if (Handle == INVALID_FILE)
		return false;

	CloseFile( Handle );

	return true;
#endif
}

void
ResourceManager::Release(
	__in const std::string & ResourceFileName
	)
/*++

Routine Description:

	This routine releases a reference to a resource previously loaded by a call
	to Demand.

Arguments:

	ResourceFileName - Supplies the resource file name, which must h ave been
	                   returned by a prior call to Demand.

Return Value:

	The routine raises an std::exception on catastrophic failure, such as an
	attempt to release an already-unloaded resource.

Environment:

	User mode.

--*/
{
	char                    Ext[ 32 ];
	char                    Name[ MAX_PATH ];
	NWN::ResType            ResType;
	ResRefNameMap::iterator nit;

	if (_splitpath_s(
		ResourceFileName.c_str( ),
		NULL,
		0,
		NULL,
		0,
		Name,
		sizeof( Name ),
		Ext,
		sizeof( Ext )) || (Ext[ 0 ] == '\0'))
	{
		try
		{
			std::string ErrorStr;

			ErrorStr  = "Illegal resource path '";
			ErrorStr += ResourceFileName;
			ErrorStr += "'.";

			throw std::runtime_error( ErrorStr );
		}
		catch (std::bad_alloc)
		{
			throw std::runtime_error( "Illegal resource path." );
		}
	}

	//
	// Look up the name in the demand-loaded file list.
	//

	ResType = ExtToResType( Ext + 1 );

	try
	{
		std::string RefName;

		_strlwr( Name );

		_itoa( (int) ResType, Ext, 10 );

		RefName  = Ext;
		RefName.push_back( 'T' );
		RefName += Name;

		nit = m_NameMap.find( RefName );
	}
	catch (std::exception)
	{
		return;
	}

	if (nit == m_NameMap.end( ))
	{
		if (m_TextWriter != NULL)
		{
			m_TextWriter->WriteText(
				"ERROR: ResourceManager::Release: Attempted to release resource '%s' after it was already released (or it was never demanded in the first place).\n"
				"Check also that a resource extension mapping exists in ExtToResType for this resource type.\n",
				ResourceFileName.c_str( ));
		}

		throw std::runtime_error( "Attempted to release unknown resource!" );
	}

	//
	// Drop the reference to us and perform the appropriate action once the
	// reference count goes to zero.
	//

	if (--nit->second.Refs == 0)
	{
		if (nit->second.Delete)
			DeleteFileA( nit->second.ResourceFileName.c_str( ) );

		m_NameMap.erase( nit );
	}
}

ResourceManager::FileHandle
ResourceManager::OpenFile(
	__in const ResRefT & FileName,
	__in ResType Type
	)
/*++

Routine Description:

	This routine logically opens an encapsulated sub-file.

Arguments:

	FileName - Supplies the name of the resource file to open.

	Type - Supplies the type of file to open (i.e. ResTRN, ResARE).

Return Value:

	The routine returns a new file handle on success.  The file handle must be
	closed by a call to CloseFile on successful return.

	On failure, the routine returns the manifest constant INVALID_FILE, which
	should not be closed.

Environment:

	User mode.

--*/
{
#if USE_INDEX
	ResourceEntryMap::const_iterator eit;
	char                             TypeStr[ 32 ];
	std::string                      LookupName;

	LookupName  = _itoa( (int) Type, TypeStr, 10 );
	LookupName.push_back( 'T' );
	LookupName += StrFromResRef( FileName );

	//
	// Look up the file in our index mapping.
	//

	if ((eit = m_NameIdMap.find( LookupName )) != m_NameIdMap.end( ))
	{
		const ResourceEntry * Entry;
		FileHandle            AccessorHandle;

		//
		// Open it up via the accessor.
		//

		Entry          = &m_ResourceEntries[ eit->second ];
		AccessorHandle = Entry->Accessor->OpenFileByIndex( Entry->FileIndex );

		if (AccessorHandle == INVALID_FILE)
			return INVALID_FILE;

		//
		// We've found a match, build a resource manager handle and return
		// it to the caller.
		//

		try
		{
			FileHandle ResManHandle;
			ResHandle  HandleEntry;

			//
			// Allocate a resource manager handle table entry.
			//

			ResManHandle = AllocateFileHandle( );

			if (ResManHandle == INVALID_FILE)
				throw std::runtime_error( "Failed to build FileHandle" );

			HandleEntry.Accessor = Entry->Accessor;
			HandleEntry.Handle   = AccessorHandle;
			HandleEntry.Type     = Type;

			//
			// Link the handle table entry up.
			//

			m_ResFileHandles.insert(
				ResHandleMap::value_type( ResManHandle, HandleEntry ) );

			//
			// All done.
			//

			return ResManHandle;
		}
		catch (std::exception &e)
		{
			Entry->Accessor->CloseFile( AccessorHandle );

			m_TextWriter->WriteText(
				"WARNING: Exception '%s' loading resource '%.32s' (type %04X).\n",
				e.what( ),
				FileName.RefStr,
				(unsigned short) Type );

			throw;
		}
		catch (...)
		{
			Entry->Accessor->CloseFile( AccessorHandle );

			throw;
		}

	}
#else
	//
	// Search through all providers, finding a match if we can.
	//

	for (size_t i = 0; i < MAX_TIERS; i += 1)
	{
		for (ResourceAccessorVec::reverse_iterator it = m_ResourceFiles[ i ].rbegin( );
		     it != m_ResourceFiles[ i ].rend( );
		     ++it)
		{
			FileHandle AccessorHandle;

			//
			// Have each accessor attempt to open the file in turn.
			//

			AccessorHandle = (*it)->OpenFile( FileName, Type );

			if (AccessorHandle == INVALID_FILE)
				continue;

			//
			// We've found a match, build a resource manager handle and return
			// it to the caller.
			//

			try
			{
				FileHandle ResManHandle;
				ResHandle  HandleEntry;

				//
				// Allocate a resource manager handle table entry.
				//

				ResManHandle = AllocateFileHandle( );

				if (ResManHandle == INVALID_FILE)
					throw std::runtime_error( "Failed to build FileHandle" );

				HandleEntry.Accessor = (*it);
				HandleEntry.Handle   = AccessorHandle;
				HandleEntry.Type     = Type;

				//
				// Link the handle table entry up.
				//

				m_ResFileHandles.insert(
					ResHandleMap::value_type( ResManHandle, HandleEntry ) );

				//
				// All done.
				//

				return ResManHandle;
			}
			catch (std::exception &e)
			{
				(*it)->CloseFile( AccessorHandle );

				m_TextWriter->WriteText(
					"WARNING: Exception '%s' loading resource '%.32s'.\n",
					e.what( ),
					FileName.RefStr );

				throw;
			}
			catch (...)
			{
				(*it)->CloseFile( AccessorHandle );

				throw;
			}
		}
	}
#endif

	//
	// No resource providers recognize it, bail.
	//

	return INVALID_FILE;
}

ResourceManager::FileHandle
ResourceManager::OpenFileByIndex(
	__in FileId FileIndex
	)
/*++

Routine Description:

	This routine logically opens an encapsulated sub-file.

Arguments:

	FileIndex - Supplies the directory index of the file to open.

Return Value:

	The routine returns a new file handle on success.  The file handle must be
	closed by a call to CloseFile on successful return.

	On failure, the routine returns the manifest constant INVALID_FILE, which
	should not be closed.

Environment:

	User mode.

--*/
{

	const ResourceEntry * Entry;
	FileHandle            AccessorHandle;
	NWN::ResRef32         FileName;
	NWN::ResType          Type;

	if ((size_t) FileIndex >= m_ResourceEntries.size( ))
		return INVALID_FILE;

	if (!GetEncapsulatedFileEntry( FileIndex, FileName, Type ))
		return INVALID_FILE;

	//
	// Open it up via the accessor.
	//

	Entry          = &m_ResourceEntries[ (size_t) FileIndex ];
	AccessorHandle = Entry->Accessor->OpenFileByIndex( Entry->FileIndex );

	if (AccessorHandle == INVALID_FILE)
		return INVALID_FILE;

	//
	// We've found a match, build a resource manager handle and return
	// it to the caller.
	//

	try
	{
		FileHandle ResManHandle;
		ResHandle  HandleEntry;

		//
		// Allocate a resource manager handle table entry.
		//

		ResManHandle = AllocateFileHandle( );

		if (ResManHandle == INVALID_FILE)
			throw std::runtime_error( "Failed to build FileHandle" );

		HandleEntry.Accessor = Entry->Accessor;
		HandleEntry.Handle   = AccessorHandle;
		HandleEntry.Type     = Type;

		//
		// Link the handle table entry up.
		//

		m_ResFileHandles.insert(
			ResHandleMap::value_type( ResManHandle, HandleEntry ) );

		//
		// All done.
		//

		return ResManHandle;
	}
	catch (std::exception &e)
	{
		Entry->Accessor->CloseFile( AccessorHandle );

		m_TextWriter->WriteText(
			"WARNING: Exception '%s' loading resource '%.32s' (type %04X).\n",
			e.what( ),
			FileName.RefStr,
			(unsigned short) Type );

		throw;
	}
	catch (...)
	{
		Entry->Accessor->CloseFile( AccessorHandle );

		throw;
	}
}

bool
ResourceManager::CloseFile(
	__in FileHandle File
	)
/*++

Routine Description:

	This routine logically closes an encapsulated sub-file.

Arguments:

	File - Supplies the file handle to close.

Return Value:

	The routine returns a Boolean value indicating true on success, else false
	on failure.  A return value of false typically indicates a serious
	programming error upon the caller (i.e. reuse of a closed file handle).

Environment:

	User mode.

--*/
{
	ResHandleMap::iterator it = m_ResFileHandles.find( File );
	bool                   Res;

	if (it == m_ResFileHandles.end( ))
		return false;

	//
	// Delegate the request to the underlying accessor's implementation.
	//

	Res = it->second.Accessor->CloseFile( it->second.Handle );

	//
	// Invalidate the resource manager handle.
	//

	m_ResFileHandles.erase( it );

	return Res;
}

bool
ResourceManager::ReadEncapsulatedFile(
	__in FileHandle File,
	__in size_t Offset,
	__in size_t BytesToRead,
	__out size_t * BytesRead,
	__out_bcount( BytesToRead ) void * Buffer
	)
/*++

Routine Description:

	This routine logically reads an encapsulated sub-file.

	File reading is optimized for sequential scan.

Arguments:

	File - Supplies a file handle to the desired sub-file to read.

	Offset - Supplies the offset into the desired sub-file to read from.

	BytesToRead - Supplies the requested count of bytes to read.

	BytesRead - Receives the count of bytes transferred.

	Buffer - Supplies the address of a buffer to transfer raw encapsulated file
	         contents to.

Return Value:

	The routine returns a Boolean value indicating true on success, else false
	on failure.  An attempt to read from an invalid file handle, or an attempt
	to read beyond the end of file would be examples of failure conditions.

Environment:

	User mode.

--*/
{
	ResHandleMap::const_iterator it = m_ResFileHandles.find( File );

	if (it == m_ResFileHandles.end( ))
		return 0;

	//
	// Delegate the request to the underlying accessor's implementation.
	//

	return it->second.Accessor->ReadEncapsulatedFile(
		it->second.Handle,
		Offset,
		BytesToRead,
		BytesRead,
		Buffer);
}

size_t
ResourceManager::GetEncapsulatedFileSize(
	__in FileHandle File
	)
/*++

Routine Description:

	This routine returns the size, in bytes, of an encapsulated file.

Arguments:

	File - Supplies the file handle to query the size of.

Return Value:

	The routine returns the size of the given file.  If a valid file handle is
	supplied, then the routine never fails.

	Should an illegal file handle be supplied, the routine returns zero.  There
	is no way to distinguish this condition from legal file handle to a file
	with zero length.  Only a serious programming error results in a caller
	supplying an illegal file handle.

Environment:

	User mode.

--*/
{
	ResHandleMap::const_iterator it = m_ResFileHandles.find( File );

	if (it == m_ResFileHandles.end( ))
		return 0;

	//
	// Delegate the request to the underlying accessor's implementation.
	//

	return it->second.Accessor->GetEncapsulatedFileSize( it->second.Handle );
}

ResourceManager::ResType
ResourceManager::GetEncapsulatedFileType(
	__in FileHandle File
	)
/*++

Routine Description:

	This routine returns the type of an encapsulated file.

Arguments:

	File - Supplies the file handle to query the size of.

Return Value:

	The routine returns the type of the given file.  If a valid file handle is
	supplied, then the routine never fails.

	Should an illegal file handle be supplied, the routine returns ResINVALID.
	There is no way to distinguish this condition from legal file handle to a
	file of type ResINVALID.  Only a serious programming error results in a
	caller supplying an illegal file handle.

Environment:

	User mode.

--*/
{
	ResHandleMap::const_iterator it = m_ResFileHandles.find( File );

	if (it == m_ResFileHandles.end( ))
		return NWN::ResINVALID;

	return it->second.Type;
}

bool
ResourceManager::GetEncapsulatedFileEntry(
	__in FileId FileIndex,
	__out ResRefT & ResRef,
	__out ResType & Type
	)
/*++

Routine Description:

	This routine reads an encapsulated file directory entry, returning the name
	and type of a particular resource.  The enumeration is stable across calls.

Arguments:

	FileIndex - Supplies the index into the logical directory entry to reutrn.

	ResRef - Receives the resource name.

	Type - Receives the resource type.

Return Value:

	The routine returns a Boolean value indicating success or failure.  The
	routine always succeeds as long as the caller provides a legal file index.

Environment:

	User mode.

--*/
{
	IResourceAccessor * Accessor;

	if ((size_t) FileIndex >= m_ResourceEntries.size( ))
		return false;

	Accessor = m_ResourceEntries[ (size_t) FileIndex ].Accessor;

	return Accessor->GetEncapsulatedFileEntry(
		m_ResourceEntries[ (size_t) FileIndex ].FileIndex,
		ResRef,
		Type);
}

ResourceManager::FileId
ResourceManager::GetEncapsulatedFileCount(
	)
/*++

Routine Description:

	This routine returns the count of files in this resource accessor.  The
	highest valid file index is the returned count minus one, unless there are
	zero files, in which case no file index values are legal.

Arguments:

	None.

Return Value:

	The routine returns the count of files present.

Environment:

	User mode.

--*/
{
	return m_ResourceEntries.size( );
}

ResourceManager::AccessorType
ResourceManager::GetResourceAccessorName(
	__in FileHandle File,
	__out std::string & AccessorName
	)
/*++

Routine Description:

	This routine returns the logical name of the resource accessor.

Arguments:

	File - Supplies the file handle to inquire about.

	AccessorName - Receives the logical name of the resource accessor.

Return Value:

	The routine returns the accessor type.  An std::exception is raised on
	failure.

Environment:

	User mode.

--*/
{
	ResHandleMap::const_iterator it;

	if (File == INVALID_FILE)
	{
		AccessorName = "Resource Manager";
		return AccessorTypeResourceManager;
	}

	it = m_ResFileHandles.find( File );

	if (it == m_ResFileHandles.end( ))
		throw std::runtime_error( "invalid file handle passed to ResourceManager::GetResourceAccessorName" );

	//
	// Delegate the request to the underlying accessor's implementation.
	//

	return it->second.Accessor->GetResourceAccessorName(
			it->second.Handle,
			AccessorName);
}

template< typename ResRefType >
void
ResourceManager::LoadEncapsulatedFile(
	__in IResourceAccessor< ResRefType > * Accessor,
	__in typename IResourceAccessor< ResRefType >::FileId FileIndex,
	__out std::vector< unsigned char > & FileContents
	)
/*++

Routine Description:

	This helper routine loads a file from a resource accessor into an
	std::vector.  The entire file is loaded at once.

Arguments:

	Accessor - Supplies the resource accessor instance to open the file from.

	FileIndex - Supplies the index of the file in the resource accessor that is
	            to be loaded.

	FileContents - Receives the loaded contents of the file.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	IResourceAccessor< ResRefType >::FileHandle   Handle;
	size_t                                        FileSize;
	size_t                                        BytesLeft;
	size_t                                        Offset;
	size_t                                        Read;

	//
	// Open the file via the resource system.
	//

	Handle = Accessor->OpenFileByIndex( FileIndex );

	if (Handle == IResourceAccessor< ResRefType >::INVALID_FILE)
		throw std::runtime_error( "OpenFileByIndex failed." );

	//
	// Read the whole contents into memory up front.
	//

	FileSize = Accessor->GetEncapsulatedFileSize( Handle );

	try
	{
		if (FileSize != 0)
		{
			FileContents.resize( FileSize );

			BytesLeft = FileSize;
			Offset    = 0;

			while (BytesLeft != 0)
			{
				if (!Accessor->ReadEncapsulatedFile(
					Handle,
					Offset,
					BytesLeft,
					&Read,
					&FileContents[ Offset ]))
				{
					throw std::runtime_error( "ReadEncapsulatedFile failed." );
				}

				if (Read == 0)
					throw std::runtime_error( "Read zero bytes." );

				Offset    += Read;
				BytesLeft -= Read;
			}
		}
		else
		{
			FileContents.clear( );
		}
	}
	catch (std::exception)
	{
		Accessor->CloseFile( Handle );

		throw;
	}

	//
	// Close the file out and we're done.
	//

	Accessor->CloseFile( Handle );
}

bool
ResourceManager::GetTalkString(
	__in unsigned long StringId,
	__out std::string & String
	) const
/*++

Routine Description:

	This routine retrieves a localized string from the string tables.

Arguments:

	StringId - Supplies the STRREF to look up.

	String - Receives the localized string, on success.

Return Value:

	The routine returns a Boolean value indicating whether the string could be
	successfully located or not.
	
	On catastrophic failure, the routine raises an std::exception.

Environment:

	User mode.

--*/
{
	unsigned long RefId;

	if (StringId == STRREF_INVALID)
	{
		String = "";
		return true;
	}

	RefId = (StringId & STRREF_IDMASK);

	//
	// If requested, try the alternate table first.
	//

	if (StringId & STRREF_TABLEMASK)
	{
		if ((m_AlternateTlk.get( ) != NULL) &&
		    (m_AlternateTlk->GetTalkString( RefId, String )))
		{
			return true;
		}
	}

	//
	// Always fall back to the base table if we've found nothing so far.
	//

	if ((m_BaseTlk.get( ) != NULL) &&
	    (m_BaseTlk->GetTalkString( RefId, String )))
	{
		return true;
	}

	return false;
}

bool
ResourceManager::Get2DAString(
	__in const std::string & ResourceName,
	__in const std::string & Column,
	__in size_t Row,
	__out std::string & Value
	)
/*++

Routine Description:

	This routine fetches the string value of a column at a particular row index
	into the given 2DA file.

Arguments:

	ResourceName - Supplies the RESREF identifier of the .2DA to read.

	Column - Supplies the column name to look up.

	Row - Supplies the row index to pull the column from.

	Value - Receives the string contents of the particular row.

Return Value:

	The routine returns a Boolean value indicating true on success, else false
	on failure (i.e. unknown string).  On catastrophic failure, an
	std::exception is raised.

Environment:

	User mode.

--*/
{
	TwoDANameMap::const_iterator it;

	it = m_2DAs.find( ResourceName );

	if (it == m_2DAs.end( ))
	{
		//
		// Try and load the .2DA on demand using the resource manager's search
		// hierarchy for locating the .2DA file.
		//
		// If successful, cache the 2DA object in-memory, drop the demanded
		// resource reference (as the TwoDAFileReader does not require
		// continual file access), and delegate the Get2DAString requets to the
		// newly-instantiated TwoDAReader object.
		//

		try
		{
			TwoDAFileReaderPtr Reader;
			DemandResourceStr  Res( *this, ResourceName, NWN::Res2DA );

			Reader = new TwoDAFileReader( Res );

			m_2DAs.insert( TwoDANameMap::value_type( ResourceName, Reader ) );

			return Reader->Get2DAString( Column, Row, Value );
		}
		catch (std::exception &e)
		{
			m_TextWriter->WriteText(
				"WARNING: Failed to access 2DA '%s' ('%s'/%lu): exception '%s'.\n",
				ResourceName.c_str( ),
				Column.c_str( ),
				Row,
				e.what( ));

			//
			// Cache a NULL reader pointer so that we don't try and hit the
			// resource list each time from now on.  Failures to load a 2DA are
			// not temporary failures and are typically symptomatic of a critical
			// condition such as a missing or malformed .2DA on-disk.
			//

			m_2DAs.insert( TwoDANameMap::value_type( ResourceName, (TwoDAFileReader *) NULL ) );
		}

		return false;
	}

	//
	// If we cached this .2DA as not present, fail all attempts to access it.
	//

	if (it->second.get( ) == NULL)
		return false;

	//
	// Delegate the request to the actual .2DA reader implementation.
	//

	try
	{
		return it->second->Get2DAString( Column, Row, Value );
	}
	catch (std::exception &e)
	{
		m_TextWriter->WriteText(
			"WARNING: Failed to retrieve 2DA value '%s'/%lu from '%s': exception '%s'.\n",
			Column.c_str( ),
			Row,
			ResourceName.c_str( ),
			e.what( ));

		return false;
	}
}


void
ResourceManager::LoadModuleResourcesInternal(
	__in const std::string & ModuleResName,
	__in const std::string & AltTlkFileName,
	__in const std::string & HomeDir,
	__in const std::string & InstallDir,
	__in const std::vector< NWN::ResRef32 > & HAKs,
	__in_opt ModuleLoadParams * LoadParams,
	__in bool PartialLoadOnly
	)
/*++

Routine Description:

	This routine is invoked when all module resources have been downloaded and
	are available.  It bears responsibility for loading always-load resources
	such as TLK files, as well as setting up other state necessary for the
	proper servicing of demand-load requests.

Arguments:

	ModuleResName - Supplies the resource name of the current module.

	AltTlkFileName - Optionally supplies the name of an alternate tlk file.

	HomeDir - Supplies the game home directory, i.e. Docs\Neverwinter Nights 2.

	InstallDir - Supplies the installation directory of the game.

	HAKs - Supplies the list of HAK files to load.

	LoadParams - Optionally supplies extended module load parameters, such as
	             the campaign ID.

	PartialLoadOnly - Supplies a Boolean value that indicates whether only a
	                  partial load, enough to pull data out of the module GFF
	                  resources, should be performed.

Return Value:

	The routine raises an std::exception on catastrophic failure.

Environment:

	User mode.

--*/
{
	std::string Tlk;
	int         Cp;

	CleanDemandLoadedFiles( );

	m_ModuleResName = ModuleResName;
	m_HomeDir       = HomeDir;
	m_InstallDir    = InstallDir;

	//
	// Ensure that we don't get into DBCS comparisons which gets us a bit more
	// speed when figuring resrefs via splitpath_s.  Resrefs have a restricted
	// character set to begin with.
	//

	Cp = _getmbcp( );

	_setmbcp( _MB_CP_SBCS );

	if (LoadParams != NULL)
		m_ResManFlags = LoadParams->ResManFlags;
	else
		m_ResManFlags = 0;

	//
	// If all built-in providers are disabled, set the load module only and
	// load base resource only flags which turn off everything but directory
	// loads (which we'll explicitly test for later).
	//

	if (m_ResManFlags & ResManFlagNoBuiltinProviders)
	{
		m_ResManFlags |= ResManFlagLoadCoreModuleOnly |
		                 ResManFlagBaseResourcesOnly;
	}

	if (m_ResManFlags & ResManFlagLoadCoreModuleOnly)
		PartialLoadOnly = true;

	ResDebug2(
		"ResourceManager::LoadModuleResourcesInternal: Beginning resource load of module %s (HomeDir %s, InstallDir %s, AltTlk %s, NumHaks %lu, PartialLoad %lu)...\n",
		ModuleResName.c_str( ),
		HomeDir.c_str( ),
		InstallDir.c_str( ),
		AltTlkFileName.c_str( ),
		(unsigned long) HAKs.size( ),
		PartialLoadOnly ? 1 : 0);

	try
	{
		if (LoadParams != NULL)
		{
			LoadCustomResourceProviders(
				LoadParams->CustomFirstChanceAccessors,
				LoadParams->NumCustomFirstChanceAccessors,
				true);
		}

		//
		// Load all built-in resource providers.
		//

		if ((m_ResManFlags & ResManFlagBaseResourcesOnly) == 0)
		{
			LoadModule(
				LoadParams != NULL ? LoadParams->SearchOrder : ModSearch_Automatic,
				LoadParams != NULL ? LoadParams->CustomModuleSourcePath : NULL);

			if ((LoadParams != NULL) && (LoadParams->CampaignID != NULL))
				LoadCampaign( *LoadParams->CampaignID, LoadParams->CampaignIDUsed );

			if (!PartialLoadOnly)
			{
				if (!(m_ResManFlags & ResManFlagErf16))
					LoadHAKFiles< NWN::ResRef32, TIER_ENCAPSULATED >( HAKs );
				else
					LoadHAKFiles< NWN::ResRef16, TIER_ENCAPSULAT16 >( HAKs );
			}
		}

		if ((m_ResManFlags & ResManFlagNoBuiltinProviders) == 0)
		{
			LoadDirectories(
				LoadParams != NULL ? LoadParams->CustomSearchPath : NULL );
		}

		if (!PartialLoadOnly)
		{
			LoadZipArchives( );

			if (LoadParams != NULL && LoadParams->KeyFiles != NULL)
				LoadFixedKeyFiles( *LoadParams->KeyFiles );
		}

		if (LoadParams != NULL)
		{
			LoadCustomResourceProviders(
				LoadParams->CustomLastChanceAccessors,
				LoadParams->NumCustomLastChanceAccessors,
				false);
		}

#if USE_INDEX
		//
		// Now, discover and index all resources.
		//

		DiscoverResources( );
#endif

		//
		// Now load talk tables after we've initialized all resources.
		//

		LoadTalkTables( AltTlkFileName );

		//
		// Attempt to load GR2 support.
		//

		if ((m_Gr2Accessor.get( ) == NULL) &&
		    (!(m_ResManFlags & ResManFlagNoGranny2)))
		{
			try
			{
				std::string Gr2Path;

				Gr2Path  = m_InstallDir;
				Gr2Path += "/granny2.dll";

				if (_access( Gr2Path.c_str( ), 0 ))
					Gr2Path = "granny2.dll";

				m_Gr2Accessor = new Gr2Accessor( Gr2Path, m_TempPath );
			}
			catch (std::exception &e)
			{
				m_TextWriter->WriteText(
					"WARNING: Exception '%s' loading granny2.  Model skeletons will be unavailable.\n",
					e.what( ));
			}
		}
	}
	catch (...)
	{
		_setmbcp( Cp );
		throw;
	}

	_setmbcp( Cp );
}

template< typename ResRefLoadType, const size_t LoadTier >
void
ResourceManager::LoadHAKFiles(
	__in const std::vector< NWN::ResRef32 > & HAKs
	)
/*++

Routine Description:

	This routine loads module HAK files.

Arguments:

	HAKs - Supplies the list of HAK files to load.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	typedef swutil::SharedPtr< typename ::ErfFileReader< typename ResRefLoadType > > ErfFileReaderPtr;
	typedef std::vector< typename ErfFileReaderPtr > HakVecType;

	HakVecType & HakFiles = GetHakFiles< ResRefLoadType >( );

#if defined(RES_DEBUG) && RES_DEBUG >= 1
	ULONG TimeSpent;

	TimeSpent = GetTickCount( );
#endif

	HakFiles.reserve( HakFiles.size( ) + HAKs.size( ) );
	m_ResourceFiles[ LoadTier ].reserve(
		m_ResourceFiles[ LoadTier ].size( ) + HAKs.size( ) );

	for (std::vector< NWN::ResRef32 >::const_reverse_iterator it = HAKs.rbegin( );
	     it != HAKs.rend( );
	     ++it)
	{
		try
		{
			std::string      HAKFile;
			std::string      HAKPath;
			ErfFileReaderPtr HAK;

			HAKFile = StrFromResRef( *it );

			CheckResFileName( HAKFile );

			for (size_t Pass = 0; Pass < 2; Pass += 1)
			{
				if (Pass == 0)
					HAKPath = m_HomeDir;
				else
					HAKPath = m_InstallDir;

				HAKPath += "/HAK/";
				HAKPath += HAKFile;
				HAKPath += ".hak";

				if ((Pass == 0) && _access( HAKPath.c_str( ), 00 ))
					continue;

				ResDebug2(
					"ResourceManager::LoadHAKFiles: Loading HAK '%s'...\n",
					HAKPath.c_str( ));

				//
				// Load it up.
				//

				HAK = new ::ErfFileReader< ResRefLoadType >( HAKPath );

				HakFiles.push_back( HAK );
				m_ResourceFiles[ LoadTier ].push_back( HAK.get( ) );
				break;
			}
		}
		catch (std::exception &e)
		{
			m_TextWriter->WriteText(
				"WARNING: Failed to load HAK file '%.32s' (exception '%s').  Certain module resources may be unavailable.\n",
				it->RefStr,
				e.what( ));
		}
	}

#if defined(RES_DEBUG) && RES_DEBUG >= 1
	m_TextWriter->WriteText( "HAK: %lu\n", GetTickCount( ) - TimeSpent );
#endif
}

void
ResourceManager::LoadModule(
	__in ModuleSearchOrder SearchOrder,
	__in_opt const char * CustomModuleSourcePath
	)
/*++

Routine Description:

	This routine registers module resources in the resource manager.

Arguments:

	SearchOrder - Supplies the module search order.  Legal values are drawn from
	              the ModuleSearchOrder enumeration.

	              ModSearch_Automatic - Try encapsulated, then directory.

	              ModSearch_PrefDirectory - Prefer directory modules.

	              ModSearch_PrefEncapsulated - Prefer encapsulated modules.

	CustomModuleSourcePath - Optionally supplies a custom search path for the
	                         module.  If provided, the default search logic is
	                         overridden.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	try
	{
		std::string ModulePath;

		CheckResFileName( m_ModuleResName );

		ResDebug2(
			"ResourceManager::LoadModule: Beginning module search (search order = %lu)...\n",
			(unsigned long) SearchOrder );

		//
		// If we have an override custom module source location, then load the
		// module directly from there.  The location is a full path and not a
		// resource name.
		//

		if (CustomModuleSourcePath != NULL)
		{
			struct _stat s;

			ResDebug2(
				"ResourceManager::LoadModule: Using custom module source path '%s'.\n",
				CustomModuleSourcePath );

			if ((_stat( CustomModuleSourcePath, &s ) == 0) &&
			    (s.st_mode & _S_IFDIR))
			{
				DirectoryFileReaderPtr ModuleRes;

				ResDebug2(
					"ResourceManager::LoadModule: Using custom directory '%s'.\n",
					CustomModuleSourcePath );

				if (!CheckModuleIfo( CustomModuleSourcePath, false ))
				{
					throw std::runtime_error(
							std::string( "No module.ifo in " ) +
							CustomModuleSourcePath );
				}

				ModuleRes  = new DirectoryFileReader( CustomModuleSourcePath );

				m_DirFiles.push_back( ModuleRes );
				m_ResourceFiles[ TIER_DIRECTORY ].push_back( ModuleRes.get( ) );
			}
			else
			{
				ResDebug2(
					"ResourceManager::LoadModule: Using custom ERF '%s'.\n",
					CustomModuleSourcePath );

				if (!CheckModuleIfo( CustomModuleSourcePath, true ))
				{
					throw std::runtime_error(
							std::string( "No module.ifo in " ) +
							CustomModuleSourcePath );
				}

				LoadEncapsulatedFile( CustomModuleSourcePath );
			}

			return;
		}

		//
		// Use two passes.  For the first, we'll go with the home directory
		// instead of the install directory, searching that 'Modules' and 'PWC'
		// directory set.  Priority is given to the home directory.
		//

		for (size_t i = 0; i < 2; i += 1)
		{
			bool UseHome;

			UseHome = (i == 0);

			ResDebug2(
				"ResourceManager::LoadModule: Searching for module in %s...\n",
				UseHome ? "home directory" : "install directory");

			//
			// Try it as an ERF (.mod) first.
			//

			ModulePath = GetModulePath( false, UseHome );

			if ((SearchOrder == ModSearch_PrefDirectory) ||
			    (_access( ModulePath.c_str( ), 00 ))     ||
			    (!CheckModuleIfo( ModulePath, true )))
			{
				//
				// Try as a PWC next.
				//

				std::string PWCName;

				PWCName  = "pwc/";
				PWCName += m_ModuleResName;
				PWCName += ".pwc";

				ModulePath = GetResourceFilePath( PWCName );

				if ((SearchOrder == ModSearch_PrefDirectory) ||
				    (_access( ModulePath.c_str( ), 00 ))     ||
				    (!CheckModuleIfo( ModulePath, true )))
				{
					DirectoryFileReaderPtr ModuleRes;

					//
					// Doesn't exist?  Try as a directory.
					//

					ModulePath = GetModulePath( true, UseHome );

					if ((SearchOrder == ModSearch_PrefEncapsulated) ||
					    (_access( ModulePath.c_str( ), 00 ))        ||
					    (!CheckModuleIfo( ModulePath, false )))
					{
						continue;
					}

					ResDebug2(
						"ResourceManager::LoadModule: Found module directory '%s'.\n",
						ModulePath.c_str( ));

					ModuleRes  = new DirectoryFileReader( ModulePath );

					m_DirFiles.push_back( ModuleRes );
					m_ResourceFiles[ TIER_DIRECTORY ].push_back( ModuleRes.get( ) );
					return;
				}
			}

			ResDebug2(
				"ResourceManager::LoadModule: Found module ERF file '%s'.\n",
				ModulePath.c_str( ));

			//
			// We've got a .mod or a .pwc, both of which are ERFs that include the
			// necessary client files.  Load them up.
			//

			LoadEncapsulatedFile( ModulePath );
			return;
		}

		//
		// If we failed completely to find a module, try automatic searching as a
		// next resort.
		//

		if (SearchOrder != ModSearch_Automatic)
			return LoadModule( ModSearch_Automatic, CustomModuleSourcePath );

		//
		// Finally, try as a NWM (legacy campaign module), should all else
		// fail.
		//

		ModulePath = GetModuleNWMPath( );

		if ((!_access( ModulePath.c_str( ), 00 )) &&
		    (CheckModuleIfo( ModulePath, true )))
		{
			ResDebug2(
				"ResourceManager::LoadModule: Found module NWM ERF file '%s'.\n",
				ModulePath.c_str( ));

			//
			// We've got a .nwm, which is a .ERF that includes the necessary
			// client files.  Load them up.
			//

			LoadEncapsulatedFile( ModulePath );
			return;
		}

		throw std::runtime_error( "Couldn't locate module file." );
	}
	catch (std::exception &e)
	{
		m_TextWriter->WriteText(
			"WARNING: Exception '%s' loading module resources.  The area map may not function.\n",
			e.what( ) );

		if (m_ResManFlags & ResManFlagRequireModuleIfo)
			throw;
	}
}

void
ResourceManager::LoadCampaign(
	__in const GUID & CampaignID,
	__out GUID & CampaignIDUsed
	)
/*++

Routine Description:

	This routine registers campaign resources in the resource manager.

	Not all modules have an associated campaign.

Arguments:

	CampaignID - Supplies the campaign ID of the campaign to load.

	CampaignIDUsed - Receives the ID of the campaign that was actually loaded.
	                 Generally, this is the input campaign ID, unless there was
	                 no matching GUID (in which case the default ID is used).

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	HANDLE Find;

	Find = INVALID_HANDLE_VALUE;

	//
	// Set to the default campaign ID first.  If we then find a matching
	// campaign, we return the real ID.
	//

	ZeroMemory( &CampaignIDUsed, sizeof( CampaignIDUsed ) );
	memcpy( &CampaignIDUsed, "AdHocGUID", 9 );

	try
	{
		std::string CampaignPath;
		ResRefT     ResRef;
		ResType     Type;

		//
		// Use two passes.  For the first, we'll go with the home directory
		// instead of the install directory.  Then we'll try the install
		// directory.
		//

		for (size_t i = 0; i < 2; i += 1)
		{
			bool              UseHome;
			FileId            ResourceCount;
			WIN32_FIND_DATAA  FindData;
			std::string       SearchedPath;

			UseHome = (i == 0);

			ResDebug2(
				"ResourceManager::LoadCampaign: Searching for campaign in %s...\n",
				UseHome ? "home directory" : "install directory");

			//
			// Try and find a campaign that matches our campaign ID.  We sweep
			// all directories in the campaigns directory for one with a .cam
			// resource and then choose the first matching one for our campaign
			// resource directory.
			//

			CampaignPath  = GetCampaignPath( UseHome );
			SearchedPath  = CampaignPath;
			SearchedPath += "*.";

			do
			{
				if ((!strcmp( FindData.cFileName, "." )) ||
					 (!strcmp( FindData.cFileName, ".." )))
					continue;

				if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					continue;

				DirectoryFileReaderPtr CampaignSearcher;

				//
				// We have a directory to search.  Inventory all subdirectories
				// for a ResCAM type file and pull IDs out until we find one
				// that matches (or we reach the end).
				//

				SearchedPath     = CampaignPath + FindData.cFileName;
				CampaignSearcher = new DirectoryFileReader( SearchedPath );
				ResourceCount    = CampaignSearcher->GetEncapsulatedFileCount( );

				for (FileId CurId = ResourceCount; CurId != 0; CurId -= 1)
				{
					//
					// Get the resource name and type at this index.
					//

					if (!CampaignSearcher->GetEncapsulatedFileEntry(
						CurId - 1,
						ResRef,
						Type))
					{
						//
						// It might be an unrecognized type, ignore it if so.
						//
						continue;
					}

					//
					// We are only searching for campaign descriptor files.
					//

					if (Type != NWN::ResCAM)
						continue;

					//
					// We only want actual campaign.cam files.
					//

					if (memcmp( &ResRef, "campaign", 8 ))
						continue;

					ResDebug2(
						"ResourceManager::LoadCampaign: Found campaign.cam (%s).\n",
						CampaignSearcher->GetRealFileName( CurId - 1 ).c_str( ));

					//
					// We have a campaign.cam file, check that the GUID matches.
					//

					try
					{
						std::vector< unsigned char > ID;
						GffFileReader                Campaign(
							CampaignSearcher->GetRealFileName( CurId - 1 ),
							*this);

						if (!Campaign.GetRootStruct( )->GetVOID( "GUID", ID ))
							throw std::runtime_error( "Failed to read GUID." );

						if (ID.size( ) != sizeof( CampaignID ))
							throw std::runtime_error( "Malformed GUID." );

						if (!memcmp( &ID[ 0 ], &CampaignID, sizeof( CampaignID ) ))
						{
							//
							// We've found our one.  Register it with the
							// resource search system and return the ID back to
							// the caller to let them know which campaign was
							// in use.
							//

							m_DirFiles.push_back( CampaignSearcher );
							m_ResourceFiles[ TIER_DIRECTORY ].push_back(
								CampaignSearcher.get( ) );

							CampaignIDUsed = CampaignID;

							FindClose( Find );
							Find = INVALID_HANDLE_VALUE;

							return;
						}
					}
					catch (std::exception &e)
					{
						m_TextWriter->WriteText(
							"WARNING: Failed to process campaign (FileId #%I64lu) in '%s': '%s'.\n",
							CurId,
							SearchedPath.c_str( ),
							e.what( ));
					}
				}

				//
				// "These are not the campaigns that you are looking for."
				//
				// Search the next directory if we had one.
				//

			} while (FindNextFileA( Find, &FindData ));

			//
			// Try the next directory hierarchy (i.e. the install directory).
			//
		}

		//
		// No campaign can be found.  This is acceptable as modules may be
		// created without any campaign linkage in effect.
		//
	}
	catch (std::exception &e)
	{
		m_TextWriter->WriteText(
			"WARNING: Exception '%s' loading campaign resources.\n",
			e.what( ) );
	}
	catch (...)
	{
		if (Find != INVALID_HANDLE_VALUE)
		{
			FindClose( Find );
			Find = INVALID_HANDLE_VALUE;
		}

		throw;
	}

	if (Find != INVALID_HANDLE_VALUE)
	{
		FindClose( Find );
		Find = INVALID_HANDLE_VALUE;
	}
}

void
ResourceManager::LoadDirectories(
	__in_opt const char * CustomSearchPath
	)
/*++

Routine Description:

	This routine registers other predefined directories with the resource
	management system.

Arguments:

	CustomSearchPath - Optionally supplies a path to a custom location that is
	                   searched first ahead of the standard directories, e.g to
	                   provide custom implementation-specific resources that
	                   are to be exposed via the standard resource system.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	DirectoryFileReaderPtr   DirFile;
	std::string              DirName;
	const char             * ResDirs[ ] =
	{
		"pwc",
		"override"
	};

	m_DirFiles.reserve(
		m_DirFiles.size( ) + sizeof( ResDirs ) / sizeof( ResDirs[ 0 ] ) + 1 );
	m_ResourceFiles[ TIER_DIRECTORY ].reserve(
		m_ResourceFiles[ TIER_DIRECTORY ].size( ) + sizeof( ResDirs ) / sizeof( ResDirs[ 0 ] ) + 1 );

	if (ARGUMENT_PRESENT( CustomSearchPath ))
	{
		DirName  = CustomSearchPath;
		DirName += "/";

		ResDebug2(
			"ResourceManager::LoadDirectories: Adding custom directory '%s'.\n",
			DirName.c_str( ));

		DirFile = new DirectoryFileReader( DirName );

		m_DirFiles.push_back( DirFile );
		m_ResourceFiles[ TIER_DIRECTORY ].push_back( DirFile.get( ) );
	}

	for (size_t i = 0;
	     i < sizeof( ResDirs ) / sizeof( ResDirs[ 0 ] );
	     i += 1)
	{
		DirName  = m_HomeDir;
		DirName += "/";
		DirName += ResDirs[ i ];

		ResDebug2(
			"ResourceManager::LoadDirectories: Adding home-based directory '%s'.\n",
			DirName.c_str( ));

		DirFile = new DirectoryFileReader( DirName );

		m_DirFiles.push_back( DirFile );
		m_ResourceFiles[ TIER_DIRECTORY ].push_back( DirFile.get( ) );

		DirName  = m_InstallDir;
		DirName += "/";
		DirName += ResDirs[ i ];

		ResDebug2(
			"ResourceManager::LoadDirectories: Adding install-based directory '%s'.\n",
			DirName.c_str( ));

		DirFile = new DirectoryFileReader( DirName );

		m_DirFiles.push_back( DirFile );
		m_ResourceFiles[ TIER_DIRECTORY ].push_back( DirFile.get( ) );
	}
}

void
ResourceManager::LoadZipArchives(
	)
/*++

Routine Description:

	This routine registers in-box zip archives with the resource management
	system.

Arguments:

	None.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	std::string              DirName;
	const char             * ResDirs[ ] =
	{
		"Data"
	};
#if PERF_TRACE
	ULONG                    TimeSpent;

	TimeSpent = GetTickCount( );
#endif

	//
	// Load all .zip archives in each zip-containing directory.
	//

	for (size_t i = 0;
	     i < sizeof( ResDirs ) / sizeof( ResDirs[ 0 ] );
	     i += 1)
	{
		DirName  = m_HomeDir;
		DirName += "/";
		DirName += ResDirs[ i ];

		ResDebug2(
			"ResourceManager::LoadZipArchives: Adding home-based zips from '%s'.\n",
			DirName.c_str( ));

		LoadDirectoryZipFiles( DirName );

		DirName  = m_InstallDir;
		DirName += "/";
		DirName += ResDirs[ i ];

		ResDebug2(
			"ResourceManager::LoadZipArchives: Adding install-based zips from '%s'.\n",
			DirName.c_str( ));

		LoadDirectoryZipFiles( DirName );
	}

#if PERF_TRACE
	m_TextWriter->WriteText( "ZIPLOAD: %lu\n", GetTickCount( ) - TimeSpent );
#endif
}

void
ResourceManager::LoadFixedKeyFiles(
	__in const StringVec & KeyFiles
	)
/*++

Routine Description:

	This routine registers in-box .key/.bif archives with the resource
	management system.

Arguments:

	KeyFiles - Supplies the list of key file RESREFs to load.  Earlier listed
	           RESREFs take priority over later listed RESREFs.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	std::string              KeyFileName;
#if PERF_TRACE
	ULONG                    TimeSpent;

	TimeSpent = GetTickCount( );
#endif

	//
	// Load all .key archives (and their associated .bif files) specified.
	//

	for (StringVec::const_reverse_iterator it = KeyFiles.rbegin( );
	     it != KeyFiles.rend( );
	     ++it)
	{
		KeyFileName =  m_InstallDir;
		KeyFileName += "/";
		KeyFileName += *it;
		KeyFileName += ".key";

		//
		// Create a .key reader context and register the resource file
		// provider in the master provider list.
		//

		try
		{
			KeyFileReaderPtr KeyRes;

			ResDebug2(
				"ResourceManager::LoadFixedKeyFiles: Loading key file '%s'...\n",
				KeyFileName.c_str( ));

			KeyRes = new KeyFileReader( KeyFileName, m_InstallDir );

			m_KeyFiles.push_back( KeyRes );
			m_ResourceFiles[ TIER_INBOX_KEY ].push_back( KeyRes.get( ) );
		}
		catch (std::exception &e)
		{
			e;
			ResDebug2(
				"WARNING: Failed to open .key archive '%s': exception '%s'.\n",
				KeyFileName.c_str( ),
				e.what( ));
		}
	}


#if PERF_TRACE
	m_TextWriter->WriteText( "FIXEDKEYLOAD: %lu\n", GetTickCount( ) - TimeSpent );
#endif
}

void
ResourceManager::LoadCustomResourceProviders(
	__in_ecount( ProviderCount ) IResourceAccessor * const * Providers,
	__in size_t ProviderCount,
	__in bool FirstChance
	)
/*++

Routine Description:

	This routine registers any custom resource providers that the user has
	established with the resource system.

Arguments:

	Providers - Optionally supplies a pointer to an array of resource accessor
	            providers to register.

	ProviderCount - Supplies the count of custom resource accessor providers.

	FirstChance - Supplies a Boolean value indicating true if the providers are
	              to be registered as first chance (true) or last chance
	              (false) resource providers with respect to the rest of the
	              built-in provider set.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	size_t                   Tier;
#if PERF_TRACE
	ULONG                    TimeSpent;

	TimeSpent = GetTickCount( );
#endif

	if (FirstChance)
		Tier = TIER_CUSTOM_FIRST;
	else
		Tier = TIER_CUSTOM_LAST;

	//
	// Register all custom providers specified with the resource system.
	//

	m_ResourceFiles[ Tier ].reserve( ProviderCount );

	for (size_t i = 0; i < ProviderCount; i += 1)
		m_ResourceFiles[ i ].push_back( Providers[ i ] );

#if PERF_TRACE
	m_TextWriter->WriteText( "CUSTOMLOAD%lu: %lu\n", (FirstChance) ? 0 : 1, GetTickCount( ) - TimeSpent );
#endif
}

void
ResourceManager::LoadDirectoryZipFiles(
	__in const std::string & DirName
	)
/*++

Routine Description:

	This routine enumerates all .zip files in a given directory hierarchy and
	creates a ZipFileReader context for each discovered .zip.

	Typically, "in-box" game data files are shipped as .zip archives, verus
	traditional custom content that is provided as ERFs or raw directories.

	N.B.  The order of filesystem enumeration is significant in that the
	      resource system assumes that alpha-sorted resources may be traversed
	      in reverse order in order to implement the canonical resource search
	      order.  This is required for "in-box" data files that are updated via
	      the patch distribution system.

Arguments:

	DirName - Supplies the directory to enumerate.  The directory name is not
	          required to end in a path separation character.

Return Value:

	None.  Raises an std::exception on catastrophic failure.

Environment:

	User mode.

--*/
{
	std::string       Mask;
	HANDLE            Find;
	WIN32_FIND_DATAA  FindData;

	Mask  = DirName;
	Mask += "\\*.zip";

	Find = FindFirstFileA( Mask.c_str( ), &FindData );

	if (Find == INVALID_HANDLE_VALUE)
		return;

	try
	{
		do
		{
			if ((!strcmp( FindData.cFileName, "." )) ||
				 (!strcmp( FindData.cFileName, ".." )))
				continue;

			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			//
			// Create a .zip reader context and register the resource file
			// provider in the master provider list.
			//

			try
			{
				ZipFileReaderPtr ZipRes;
				std::string      FileName;

				FileName  = DirName;
				FileName += "/";
				FileName += FindData.cFileName;

				ResDebug2(
					"ResourceManager::LoadDirectoryZipFiles: Loading zip file '%s'...\n",
					FileName.c_str( ));

				ZipRes = new ZipFileReader( FileName );

				m_ZipFiles.push_back( ZipRes );
				m_ResourceFiles[ TIER_INBOX ].push_back( ZipRes.get( ) );
			}
			catch (std::exception &e)
			{
				m_TextWriter->WriteText(
					"WARNING: Failed to open .zip archive '%s': exception '%s'.\n",
					FindData.cFileName,
					e.what( ));
			}
		} while (FindNextFileA( Find, &FindData )) ;
	}
	catch (...)
	{
		FindClose( Find );
		throw;
	}

	FindClose( Find );
}

void
ResourceManager::LoadTalkTables(
	__in const std::string & AltTlkFileName
	)
/*++

Routine Description:

	This routine loads all talk string tables.

Arguments:

	AltTlkFileName - Optionally supplies the name of an alternate tlk file.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
{
	std::string TlkFile;
#if PERF_TRACE
	ULONG       TimeSpent;

	TimeSpent = GetTickCount( );
#endif

	if (!AltTlkFileName.empty( ))
	{
		try
		{
			std::string RelPath;

			CheckResFileName( AltTlkFileName );

			RelPath  = "tlk/";
			RelPath += AltTlkFileName;
			RelPath += ".tlk";

			TlkFile  = GetResourceFilePath( RelPath );

			ResDebug2(
				"ResourceManager::LoadTalkTables: Loading module alternate tlk file '%s'....\n",
				TlkFile.c_str( ));

			m_AlternateTlk = new TlkFileReader( TlkFile );
		}
		catch (std::exception &e)
		{
			m_TextWriter->WriteText(
				"WARNING: Exception '%s' loading alternate TLK.  STRREF lookup may be unavailable.\n",
				e.what( ) );
		}
	}

	if (m_InstallDir.empty( ))
		return;

	try
	{
		TlkFile  = m_InstallDir;
		TlkFile += "\\dialog.tlk";

		ResDebug2(
			"ResourceManager::LoadTalkTables: Loading main tlk file '%s'...\n",
			TlkFile.c_str( ));

		m_BaseTlk = new TlkFileReader( TlkFile );
	}
	catch (std::exception &e)
	{
		m_TextWriter->WriteText(
			"WARNING: Exception '%s' loading base TLK.  STRREF lookup may be unavailable.\n",
			e.what( ) );
	}

#if PERF_TRACE
	m_TextWriter->WriteText( "TLK: %lu\n", GetTickCount( ) - TimeSpent );
#endif
}

std::string
ResourceManager::GetResourceFilePath(
	__in const std::string & RelPathName
	)
/*++

Routine Description:

	This routine utilizes the standard search order to locate a path to a
	given resource file.

Arguments:

	RelPathName - Supplies the relative path name to search for.

Return Value:

	The routine returns the fully qualified path name to use in the field.

Environment:

	User mode.

--*/
{
	std::string FullPath;

	//
	// Prefer the home directory, then fall back to the installation directory.
	//

	FullPath  = m_HomeDir;
	FullPath += "/";
	FullPath += RelPathName;

	if (_access( FullPath.c_str( ), 00 ))
	{
		FullPath  = m_InstallDir;
		FullPath += "/";
		FullPath += RelPathName;
	}

	return FullPath;
}

bool
ResourceManager::CheckModuleIfo(
	__in const std::string & ModulePath,
	__in bool Erf
	) const
/*++

Routine Description:

	This routine verifies that module.ifo is accessible in a given module
	fully qualified path.  If module.ifo verification is turned off, then the
	routine always returns true.

Arguments:

	ModulePath - Supplies the path to inquire about.

	Erf - Supples a Boolean value indicating true if the path refers to an ERF,
	      else false if the path refers to a directory.

Return Value:

	The routine returns true if the path is acceptable as a module load path,
	else false if the path was not acceptable.

	On catastrophic failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	static const NWN::ResRef32 ResRef = { "module" };

	if (!(m_ResManFlags & ResManFlagRequireModuleIfo))
		return true;

	if (!Erf)
	{
		DirectoryFileReader Reader( ModulePath );

		return DoesFileExistInResourceAccessor( Reader, ResRef, NWN::ResIFO );
	}
	else
	{
		if (m_ResManFlags & ResManFlagErf16)
		{
			ErfFileReader16 Reader( ModulePath );

			return DoesFileExistInResourceAccessor(
				Reader,
				ResRef,
				NWN::ResIFO);
		}
		else
		{
			ErfFileReader32 Reader( ModulePath );

			return DoesFileExistInResourceAccessor(
				Reader,
				ResRef,
				NWN::ResIFO);
		}
	}
}

bool
ResourceManager::DoesFileExistInResourceAccessor(
	__in IResourceAccessor & Accessor,
	__in const NWN::ResRef32 & ResRef,
	__in NWN::ResType ResType
	) const
/*++

Routine Description

	This routine checks whether a file exists in a given resource accessor.

	N.B.  It is assumed that the accessor allows a handle to be opened, which
	      for some accessors implies that no files may already be opened.

Arguments:

	Accessor - Supplies the resource accessor to query.

	ResRef - Supplies the RESREF of the file to query.

	ResType - Supplies the ResType of the file to query.

Return Value:

	The routine returns true if the given file existed in the resource accessor
	in question.

Environment:

	User mode.

--*/
{
	FileHandle    File;

	File = Accessor.OpenFile( ResRef, ResType );

	if (File == INVALID_FILE)
		return false;

	Accessor.CloseFile( File );

	return true;
}

size_t
ResourceManager::ForceCloseOpenFileHandles(
	)
/*++

Routine Description:

	This routine forcibly closes all open files.  Any file handles that were
	still open are invalidated (which would cause them to operate incorrectly
	on the next usage).

Arguments:

	None.

Return Value:

	The routine returns the number of outstanding file handles that had to be
	forcibly closed.

Environment:

	User mode.

--*/
{
	size_t FilesForceClosed;

	FilesForceClosed = 0;

	//
	// Unload 2DA files.
	//

	m_2DAs.clear( );

	//
	// Unload TLK files.
	//

	m_BaseTlk      = NULL;
	m_AlternateTlk = NULL;

	//
	// Close any still-open file handles.
	//

	FilesForceClosed += m_ResFileHandles.size( );

	for (ResHandleMap::iterator it = m_ResFileHandles.begin( );
	     it != m_ResFileHandles.end( );
	     ++it)
	{
		m_TextWriter->WriteText(
			"WARNING: Closing leaked ResourceManager handle %08X\n",
			it->first );
		it->second.Accessor->CloseFile( it->second.Handle );
	}

	m_ResFileHandles.clear( );

	FilesForceClosed += m_NameMap.size( );

	//
	// If we had demand loaded any files that had to be unpacked, then clear
	// them out now.
	//

	for (ResRefNameMap::const_iterator it = m_NameMap.begin( );
	     it != m_NameMap.end( );
	     ++it)
	{
		m_TextWriter->WriteText(
			"WARNING: Unloading leaked resource '%s' (%lu refs)...\n",
			it->second.ResourceFileName.c_str( ),
			it->second.Refs );

		if (it->second.Delete)
			DeleteFileA( it->second.ResourceFileName.c_str( ) );
	}

	m_NameMap.clear( );

	return FilesForceClosed;
}

void
ResourceManager::CleanDemandLoadedFiles(
	)
/*++

Routine Description:

	This routine unloads any demand-loaded resource data.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	//
	// Close out any open file references (internal or external).
	//

	ForceCloseOpenFileHandles( );

	//
	// Clear all resource id entries and the name (+type) to resource id map
	// table.
	//

	m_NameIdMap.clear( );
	m_ResourceEntries.clear( );

	//
	// Unload all resource providers.  First, sever the canonical search order
	// links.
	//

	for (size_t i = 0; i < MAX_TIERS; i += 1)
		m_ResourceFiles[ i ].clear( );

	//
	// Unload all HAK (and other ERF) files.
	//

	m_HakFiles.clear( );
	m_HakFiles16.clear( );

	//
	// Unload all directories.
	//

	m_DirFiles.clear( );

	//
	// Unload all zip files.
	//

	m_ZipFiles.clear( );
}

void
ResourceManager::CleanOldTempFiles(
	__in const char * TempPath
	)
/*++

Routine Description:

	This routine cleans up stale temporary files for instances of the client
	extension that did not cleanly exit.

Arguments:

	TempPath - Supplies the path to the temp directory.

Return Value:

	None.  Raises an std::exception on catastrophic failure.

Environment:

	User mode.

--*/
{
	std::string       FileName;
	HANDLE            Find;
	WIN32_FIND_DATAA  FindData;

	FileName  = TempPath;
	FileName += "\\NWN2CliExt_*.";

	Find = FindFirstFileA( FileName.c_str( ), &FindData );

	if (Find == INVALID_HANDLE_VALUE)
		return;

	try
	{
		do
		{
			HANDLE Event;

			if ((!strcmp( FindData.cFileName, "." )) ||
			    (!strcmp( FindData.cFileName, ".." )))
				continue;

			if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			if (strncmp( FindData.cFileName, "NWN2CliExt_", 11 ))
				continue;

			//
			// Check if the remote resource manager is still alive.
			//

			Event = OpenEventA( SYNCHRONIZE, FALSE, FindData.cFileName );

			if (Event != NULL)
			{
				CloseHandle( Event );
				continue;
			}

			//
			// It's gone, clean the files up.
			//

			FileName  = TempPath;
			FileName += "\\";
			FileName += FindData.cFileName;

			m_TextWriter->WriteText(
				"Cleaning up defunct resource manager instance '%s'...\n",
				FindData.cFileName );

			DeleteDirectoryFiles( FileName.c_str( ) );
			RemoveDirectoryA( FileName.c_str( ) );

		} while (FindNextFileA( Find, &FindData )) ;
	}
	catch (...)
	{
		FindClose( Find );
		throw;
	}

	FindClose( Find );
}

void
ResourceManager::DeleteDirectoryFiles(
	__in const char * DirPath
	)
/*++

Routine Description:

	This routine removes all files in a directory.  The removal is not a
	recursive remove.

Arguments:

	DirPath - Supplies the path to the directory to delete.

Return Value:

	None.  Raises an std::exception on catastrophic failure.

Environment:

	User mode.

--*/
{
	std::string       FileName;
	HANDLE            Find;
	WIN32_FIND_DATAA  FindData;

	FileName  = DirPath;
	FileName += "\\*.*";

	Find = FindFirstFileA( FileName.c_str( ), &FindData );

	if (Find == INVALID_HANDLE_VALUE)
		return;

	try
	{
		do
		{
			if ((!strcmp( FindData.cFileName, "." )) ||
			    (!strcmp( FindData.cFileName, "..")))
				continue;

			FileName  = DirPath;
			FileName += "\\";
			FileName += FindData.cFileName;

			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				RemoveDirectoryA( FileName.c_str( ) );
			else
				DeleteFileA( FileName.c_str( ) );
		} while (FindNextFileA( Find, &FindData )) ;
	}
	catch (...)
	{
		FindClose( Find );
		throw;
	}

	FindClose( Find );
}

void
ResourceManager::DiscoverResources(
	)
/*++

Routine Description:

	This routine discovers all resources across all loaded resource accessors,
	creating resource index entries for each resource.  The canonical order of
	resource providers and names within a provider is preserved.

Arguments:

	None.

Return Value:

	None.  Raises an std::exception on catastrophic failure.

Environment:

	User mode.

--*/
{
	ResourceEntry Entry;
	FileId        MaxId;
	ResRefT       ResRef;
	ResType       Type;
	char          TypeStr[ 32 ];
	std::string   LookupName;
	FileId        ResourceCount;
#if defined(RES_DEBUG) && RES_DEBUG >= 1
	DWORD         TimeSpent;
#endif

	//
	// First, total all files available to minimize reallocation.
	//

	ResourceCount = 0;

	for (size_t i = 0; i < MAX_TIERS; i += 1)
	{
		for (ResourceAccessorVec::reverse_iterator it = m_ResourceFiles[ i ].rbegin( );
		     it != m_ResourceFiles[ i ].rend( );
		     ++it)
		{
			ResourceCount += (*it)->GetEncapsulatedFileCount( );
		}
	}

	m_ResourceEntries.reserve( (size_t) ResourceCount );

#if defined(RES_DEBUG) && RES_DEBUG >= 1
	m_TextWriter->WriteText( "Indexing %lu resources...\n", ResourceCount );

	TimeSpent = GetTickCount( );
#endif

	//
	// Search each tier in turn.
	//

	for (size_t i = 0; i < MAX_TIERS; i += 1)
	{
		size_t j;

		//
		// Search each tier in order, based on the defined behavior of the
		// BioWare resource manager.  The most recently added resource provider
		// is searched first.
		//

		j = 0;

		for (ResourceAccessorVec::reverse_iterator it = m_ResourceFiles[ i ].rbegin( );
		     it != m_ResourceFiles[ i ].rend( );
		     ++it)
		{
			j += 1;

			MaxId = (*it)->GetEncapsulatedFileCount( );

			//
			// Iterate over each file, creating file entries for each resource
			// type in turn.
			//
			// We search in reverse order, taking the last entry.  This allows
			// us to preserve the order of the most recent entry of a
			// particular tier winning, used to ensure that we retrieve the
			// most precedent patched file for inbox datafiles. 
			//

			for (FileId CurId = MaxId; CurId != 0; CurId -= 1)
			{
				const char * p;

				//
				// Get the resource name and type at this index.
				//

				if (!(*it)->GetEncapsulatedFileEntry(
					CurId - 1,
					ResRef,
					Type))
				{
					//
					// It might be an unrecognized type, ignore it if so.
					//
					continue;
				}

				p = (const char *) memchr(
					ResRef.RefStr,
					'\0',
					sizeof( ResRef.RefStr ) );

				//
				// Ensure that we have not already claimed this name yet.  We
				// allow only one mapping for a particular name (+type), and it
				// is the most precedent one in the canonical search order.
				//

				LookupName = _itoa( (int) Type, TypeStr, 10 );
				LookupName.push_back( 'T' );

				if (p == NULL)
					LookupName.append( ResRef.RefStr, sizeof( ResRef.RefStr ) );
				else
					LookupName.append( ResRef.RefStr, p - ResRef.RefStr );

				//
				// Skip duplicate entry, we've already found the most precedent
				// version.
				//

				if (m_NameIdMap.find( LookupName ) != m_NameIdMap.end( ))
					continue;

				//
				// First one, add it as the most precedent.
				//

				Entry.Accessor  = (*it);
				Entry.FileIndex = CurId - 1;
				Entry.Tier      = i;
				Entry.TierIndex = j;

//				m_TextWriter->WriteText( "Found %s\n", LookupName.c_str( ) );

				m_ResourceEntries.push_back( Entry );

				m_NameIdMap.insert(
					ResourceEntryMap::value_type(
						LookupName,
						m_ResourceEntries.size( ) - 1
						)
					);
			}
		}
	}

#if defined(RES_DEBUG) && RES_DEBUG >= 1
	m_TextWriter->WriteText( "DISCOVER: %lu\n", GetTickCount( ) - TimeSpent );
#endif
}

ResourceManager::FileHandle
ResourceManager::AllocateFileHandle(
	)
/*++

Routine Description:

	This routine allocates a new file handle to return to a caller for purposes
	of accessing resource data.

	The returned handle value is only unique if the caller inserts it into the
	handle table before another call to AllocateFileHandle.

Arguments:

	None.

Return Value:

	The routine returns a legal FileHandle value on success, else it returns
	INVALID_FILE if there were no more file handles available to return.

Environment:

	User mode.

--*/
{
	FileHandle Handle;
	FileHandle FirstHandle;
	
	Handle = m_NextFileHandle++;

	if (Handle == INVALID_FILE)
		Handle = m_NextFileHandle++;

	FirstHandle = Handle;

	while (m_ResFileHandles.find( Handle ) != m_ResFileHandles.end( ) )
	{
		Handle = m_NextFileHandle++;

		if (Handle == INVALID_FILE)
			Handle = m_NextFileHandle++;

		//
		// If we've wrapped, then there are no handles left to give out.  Fail.
		//

		if (Handle == FirstHandle)
			return INVALID_FILE;
	}

	return Handle;
}

const TwoDAFileReader *
ResourceManager::Get2DA(
	__in const std::string & ResourceName
	)
/*++

Routine Description:

	This routine retrieves a cached 2DA reader context.  If the 2DA has not yet
	been cached, then it will be demand-loaded.  Should the 2DA load fail, then
	the 2DA will be negatively cached.

Arguments:

	ResourceName - Supplies the RESREF of the 2DA file.  It is the callers
	               responsibility to supply a canonical RESREF identifier.

Return Value:

	The routine returns a pointer to a TwoDAFileReader object on success, else
	it returns NULL on failure.  The returned pointer may be used until the
	module resources are unloaded.

Environment:

	User mode.

--*/
{
	TwoDANameMap::const_iterator it;

	it = m_2DAs.find( ResourceName );

	if (it == m_2DAs.end( ))
	{
		//
		// Try and load the .2DA on demand using the resource manager's search
		// hierarchy for locating the .2DA file.
		//
		// If successful, cache the 2DA object in-memory, drop the demanded
		// resource reference (as the TwoDAFileReader does not require
		// continual file access), and delegate the Get2DAString requets to the
		// newly-instantiated TwoDAReader object.
		//

		try
		{
			TwoDAFileReaderPtr Reader;
			DemandResourceStr  Res( *this, ResourceName, NWN::Res2DA );

			Reader = new TwoDAFileReader( Res );

			m_2DAs.insert( TwoDANameMap::value_type( ResourceName, Reader ) );

			return Reader.get( );
		}
		catch (std::exception &e)
		{
			m_TextWriter->WriteText(
				"WARNING: Failed to access 2DA '%s': exception '%s'.\n",
				ResourceName.c_str( ),
				e.what( ));

			//
			// Cache a NULL reader pointer so that we don't try and hit the
			// resource list each time from now on.  Failures to load a 2DA are
			// not temporary failures and are typically symptomatic of a critical
			// condition such as a missing or malformed .2DA on-disk.
			//

			m_2DAs.insert( TwoDANameMap::value_type( ResourceName, (TwoDAFileReader *) NULL ) );
		}

		return NULL;
	}

	//
	// Use the cached 2DA reader.
	//

	return it->second.get( );
}

template DemandResource< std::string >;
template DemandResource< NWN::ResRef16 >;
template DemandResource< NWN::ResRef32 >;

template
void
ResourceManager::LoadEncapsulatedFile< NWN::ResRef16 >(
	__in IResourceAccessor< NWN::ResRef16 > * Accessor,
	__in IResourceAccessor< NWN::ResRef16 >::FileId FileIndex,
	__out std::vector< unsigned char > & FileContents
	);

template
void
ResourceManager::LoadEncapsulatedFile< NWN::ResRef32 >(
	__in IResourceAccessor< NWN::ResRef32 > * Accessor,
	__in IResourceAccessor< NWN::ResRef32 >::FileId FileIndex,
	__out std::vector< unsigned char > & FileContents
	);
