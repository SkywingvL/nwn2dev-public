/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    Main.cpp

Abstract:

    This module houses the main entry point of the compiler driver.  The
    compiler driver provides a user interface to compile scripts under user
    control.

--*/

#include "Precomp.h"
#include "../NWN2DataLib/TextOut.h"
#include "../NWN2DataLib/ResourceManager.h"
#include "../NWNScriptCompilerLib/Nsc.h"
#include "NWNScriptCompilerDll.h"

using namespace NscExt;

namespace NscExt
{

class NullTextOut : public IDebugTextOut
{

public:

	inline
	NullTextOut(
		)
	{
	}

	inline
	~NullTextOut(
		)
	{
	}

	enum { STD_COLOR = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE };

	inline
	virtual
	void
	WriteText(
		__in __format_string const char* fmt,
		...
		)
	{
		va_list ap;

		va_start( ap, fmt );
		WriteTextV( STD_COLOR, fmt, ap );
		va_end( ap );
	}

	inline
	virtual
	void
	WriteText(
		__in WORD Attributes,
		__in __format_string const char* fmt,
		...
		)
	{
		va_list ap;

		va_start( ap, fmt );
		WriteTextV( Attributes, fmt, ap );
		va_end( ap );

		UNREFERENCED_PARAMETER( Attributes );
	}

	inline
	virtual
	void
	WriteTextV(
		__in __format_string const char* fmt,
		__in va_list ap
		)
	{
		WriteTextV( STD_COLOR, fmt, ap );
	}

	inline
	virtual
	void
	WriteTextV(
		__in WORD Attributes,
		__in const char *fmt,
		__in va_list argptr
		)
	/*++

	Routine Description:

		This routine displays text to the output file associated with the output
		object.

		The console output may have color attributes supplied, as per the standard
		SetConsoleTextAttribute API.

	Arguments:

		Attributes - Supplies color attributes for the text as per the standard
					 SetConsoleTextAttribute API (e.g. FOREGROUND_RED).

		fmt - Supplies the printf-style format string to use to display text.

		argptr - Supplies format inserts.

	Return Value:

		None.

	Environment:

		User mode.

	--*/
	{
		UNREFERENCED_PARAMETER( Attributes );
		UNREFERENCED_PARAMETER( fmt );
		UNREFERENCED_PARAMETER( argptr );
	}

private:

};

class StringTextOut : public IDebugTextOut
{

public:

	inline
	StringTextOut(
		)
	{
	}

	inline
	~StringTextOut(
		)
	{
	}

	//
	// Return the currently captured output buffer.
	//

	inline
	const std::string &
	GetTextOutput(
		)
	{
		return m_TextOut;
	}

	enum { STD_COLOR = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE };

	inline
	virtual
	void
	WriteText(
		__in __format_string const char* fmt,
		...
		)
	{
		va_list ap;

		va_start( ap, fmt );
		WriteTextV( STD_COLOR, fmt, ap );
		va_end( ap );
	}

	inline
	virtual
	void
	WriteText(
		__in WORD Attributes,
		__in __format_string const char* fmt,
		...
		)
	{
		va_list ap;

		va_start( ap, fmt );
		WriteTextV( Attributes, fmt, ap );
		va_end( ap );

		UNREFERENCED_PARAMETER( Attributes );
	}

	inline
	virtual
	void
	WriteTextV(
		__in __format_string const char* fmt,
		__in va_list ap
		)
	{
		WriteTextV( STD_COLOR, fmt, ap );
	}

	inline
	virtual
	void
	WriteTextV(
		__in WORD Attributes,
		__in const char *fmt,
		__in va_list argptr
		)
	/*++

	Routine Description:

		This routine displays text to the output file associated with the output
		object.

		The console output may have color attributes supplied, as per the standard
		SetConsoleTextAttribute API.

	Arguments:

		Attributes - Supplies color attributes for the text as per the standard
					 SetConsoleTextAttribute API (e.g. FOREGROUND_RED).

		fmt - Supplies the printf-style format string to use to display text.

		argptr - Supplies format inserts.

	Return Value:

		None.

	Environment:

		User mode.

	--*/
	{
		char buf[8193];
		StringCbVPrintfA(buf, sizeof( buf ), fmt, argptr);

		m_TextOut += buf;

		UNREFERENCED_PARAMETER( Attributes );
	}

private:

	std::string m_TextOut;

};

//
// Define the compiler context.  A NSC_COMPILER_HANDLE points to this
// structure.
//

typedef struct _NSC_COMPILER_CONTEXT
{
	PCNSC_COMPILER_DISPATCH_TABLE   DispatchTable;
	NscCompiler                   * Compiler;
} NSC_COMPILER_CONTEXT, * PNSC_COMPILER_CONTEXT;

typedef const struct _NSC_COMPILER_CONTEXT * PCNSC_COMPILER_CONTEXT;

//
// Define the external thunk resource accessor.
//

class ExternalResourceAccessor : public IResourceAccessor< NWN::ResRef32 >
{

public:

	//
	// Create a new resource accessor instance.
	//

	inline
	ExternalResourceAccessor(
		)
	: m_DispatchTable( NULL )
	{
	}

	//
	// Set the current I/O dispatch table.
	//

	inline
	void
	SetIoDispatchTable(
		__in PCNSC_COMPILER_DISPATCH_TABLE DispatchTable
		)
	{
		m_DispatchTable = DispatchTable;
	}

	//
	// Open an encapsulated file by resref.
	//

	inline
	virtual
	FileHandle
	OpenFile(
		__in const NWN::ResRef32 & ResRef,
		__in ResType Type
		)
	{
		return m_DispatchTable->ResOpenFile(
			ResRef,
			Type,
			m_DispatchTable->Context);
	}

	//
	// Open an encapsulated file by file index.
	//

	inline
	virtual
	FileHandle
	OpenFileByIndex(
		__in FileId FileIndex
		)
	{
		return m_DispatchTable->ResOpenFileByIndex(
			FileIndex,
			m_DispatchTable->Context);
	}

	//
	// Close an encapsulated file.
	//

	inline
	virtual
	bool
	CloseFile(
		__in FileHandle File
		)
	{
		return m_DispatchTable->ResCloseFile( File, m_DispatchTable->Context );
	}

	//
	// Read an encapsulated file by file handle.  The routine is optimized to
	// operate for sequential file reads.
	//

	inline
	virtual
	bool
	ReadEncapsulatedFile(
		__in FileHandle File,
		__in size_t Offset,
		__in size_t BytesToRead,
		__out size_t * BytesRead,
		__out_bcount( BytesToRead ) void * Buffer
		)
	{
		return m_DispatchTable->ResReadEncapsulatedFile(
			File,
			Offset,
			BytesToRead,
			BytesRead,
			Buffer,
			m_DispatchTable->Context);
	}

	//
	// Return the size of a file.
	//

	inline
	virtual
	size_t
	GetEncapsulatedFileSize(
		__in FileHandle File
		)
	{
		return m_DispatchTable->ResGetEncapsulatedFileSize(
			File,
			m_DispatchTable->Context);
	}

	//
	// Return the resource type of a file.
	//

	inline
	virtual
	ResType
	GetEncapsulatedFileType(
		__in FileHandle File
		)
	{
		return m_DispatchTable->ResGetEncapsulatedFileType(
			File,
			m_DispatchTable->Context);
	}

	//
	// Iterate through resources in this resource accessor.  The routine
	// returns false on failure.
	//

	inline
	virtual
	bool
	GetEncapsulatedFileEntry(
		__in FileId FileIndex,
		__out NWN::ResRef32 & ResRef,
		__out ResType & Type
		)
	{
		return m_DispatchTable->ResGetEncapsulatedFileEntry(
			FileIndex,
			ResRef,
			Type,
			m_DispatchTable->Context);
	}

	//
	// Return the count of encapsulated files in this accessor.
	//

	inline
	virtual
	FileId
	GetEncapsulatedFileCount(
		)
	{
		return m_DispatchTable->ResGetEncapsulatedFileCount(
			m_DispatchTable->Context);
	}

	//
	// Get the logical name of this accessor.
	//

	virtual
	AccessorType
	GetResourceAccessorName(
		__in FileHandle File,
		__out std::string & AccessorName
		)
	{
		UNREFERENCED_PARAMETER( File );

		AccessorName = "Custom";
		return AccessorTypeCustom;
	}


private:

	PCNSC_COMPILER_DISPATCH_TABLE m_DispatchTable;

};

//
// No reason these should be globals, except for ease of access to the debugger
// right now.
//

NullTextOut                 g_TextOut;
ResourceManager           * g_ResMan;
size_t                      g_ResManRefs;
ExternalResourceAccessor    g_ResAccessor;

NWACTION_TYPE
ConvertNscType(
	__in NscType Type
	)
/*++

Routine Description:

	This routine converts a compiler NscType to an analyzer NWACTION_TYPE.

Arguments:

	Type - Supplies the NscType-encoding type to convert.

Return Value:

	The routine returns the corresponding NWACTION_TYPE for the given NscType.
	If there was no matching conversion (i.e. a user defined type was in use),
	then an std::exception is raised.

Environment:

	User mode.

--*/
{
	switch (Type)
	{

	case NscType_Void:
		return ACTIONTYPE_VOID;
	case NscType_Integer:
		return ACTIONTYPE_INT;
	case NscType_Float:
		return ACTIONTYPE_FLOAT;
	case NscType_String:
		return ACTIONTYPE_STRING;
	case NscType_Object:
		return ACTIONTYPE_OBJECT;
	case NscType_Vector:
		return ACTIONTYPE_VECTOR;
	case NscType_Action:
		return ACTIONTYPE_ACTION;
	default:
		if ((Type >= NscType_Engine_0) && (Type < NscType_Engine_0 + 10))
			return (NWACTION_TYPE) (ACTIONTYPE_ENGINE_0 + (Type - NscType_Engine_0));
		else
			throw std::runtime_error( "Illegal NscType for action service handler." );
	}
}

bool
LoadFileFromDisk(
	__in const std::string & FileName,
	__out std::vector< unsigned char > & FileContents
	)
/*++

Routine Description:

	This routine canonicalizes an input file name to its resource name and
	resource type, and then loads the entire file contents into memory.

	The input file may be a short filename or a filename with a path.  It may be
	backed by the raw filesystem or by the resource system (in that order of
	precedence).

Arguments:

	ResMan - Supplies the resource manager to use to service file load requests.

	InFile - Supplies the filename of the file to load.

	FileResRef - Receives the canonical RESREF name of the input file.

	FileResType - Receives the canonical ResType (extension) of the input file.

	FileContents - Receives the contents of the input file.

Return Value:

	The routine returns a Boolean value indicating true on success, else false
	on failure.

Environment:

	User mode.

--*/
{
	FileWrapper FileWrap;
	HANDLE      SrcFile;

	FileContents.clear( );

	SrcFile = CreateFileA(
		FileName.c_str( ),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (SrcFile == INVALID_HANDLE_VALUE)
		return false;

	try
	{
		FileWrap.SetFileHandle( SrcFile, true );

		if ((size_t) FileWrap.GetFileSize( ) != 0)
		{
			FileContents.resize( (size_t) FileWrap.GetFileSize( ) );

			FileWrap.ReadFile(
				&FileContents[ 0 ],
				FileContents.size( ),
				"LoadFileFromDisk File Contents");
		}
	}
	catch (std::exception)
	{
		CloseHandle( SrcFile );
		SrcFile = INVALID_HANDLE_VALUE;
		return false;
	}

	CloseHandle( SrcFile );
	SrcFile = INVALID_HANDLE_VALUE;
	
	return true;
}

bool
LoadInputFile(
	__in ResourceManager & ResMan,
	__in IDebugTextOut * TextOut,
	__in PCNSC_COMPILER_DISPATCH_TABLE DispatchTable,
	__in const std::string & InFile,
	__out NWN::ResRef32 & FileResRef,
	__out NWN::ResType & FileResType,
	__out std::vector< unsigned char > & FileContents
	)
/*++

Routine Description:

	This routine canonicalizes an input file name to its resource name and
	resource type, and then loads the entire file contents into memory.

	The input file may be a short filename or a filename with a path.  It may be
	backed by the raw filesystem or by the resource system (in that order of
	precedence).

Arguments:

	ResMan - Supplies the resource manager to use to service file load requests.

	TextOut - Supplies the text out interface used to receive any diagnostics
	          issued.

	DispatchTable - Supplies the I/O dispatch table for the compiler.

	InFile - Supplies the filename of the file to load.

	FileResRef - Receives the canonical RESREF name of the input file.

	FileResType - Receives the canonical ResType (extension) of the input file.

	FileContents - Receives the contents of the input file.

Return Value:

	The routine returns a Boolean value indicating true on success, else false
	on failure.

	On catastrophic failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	char                   Drive[ _MAX_DRIVE ];
	char                   Dir[ _MAX_DIR ];
	char                   FileName[ _MAX_FNAME ];
	char                   Extension[ _MAX_EXT ];

	//
	// First, canonicalize the filename.
	//

	if (_splitpath_s(
		InFile.c_str( ),
		Drive,
		Dir,
		FileName,
		Extension))
	{
		TextOut->WriteText(
			"Error: Malformed file pathname \"%s\".\n", InFile.c_str( ));

		return false;
	}

	if (Extension[ 0 ] != '.')
		FileResType = NWN::ResINVALID;
	else
		FileResType = ResMan.ExtToResType( Extension + 1 );

	FileResRef = ResMan.ResRef32FromStr( FileName );

	//
	// Load the file directly if we can, otherwise attempt it via the resource
	// system.
	//

	if (!_access( InFile.c_str( ), 00 ))
	{
		return LoadFileFromDisk( InFile, FileContents );
	}
	else if (ResMan.ResourceExists( FileResRef, FileResType ))
	{
		DemandResource32 DemandRes( ResMan, FileResRef, FileResType );

		return LoadFileFromDisk( DemandRes, FileContents );
	}
	else if ((DispatchTable->Size >= sizeof( NSC_COMPILER_DISPATCH_TABLE_V2 )) &&
	         (DispatchTable->ResLoadFile != NULL)                              &&
	         (DispatchTable->ResUnloadFile != NULL))
	{
		void    * ExtFilePtr;
		size_t    ExtFileSize;

		if (!DispatchTable->ResLoadFile(
			FileResRef,
			FileResType,
			&ExtFilePtr,
			&ExtFileSize,
			DispatchTable->Context))
		{
			TextOut->WriteText(
				"Error: Failed to load input file \"%s\".\n", InFile.c_str( ));

			return false;
		}

		try
		{
			FileContents.resize( ExtFileSize );
		}
		catch (...)
		{
			DispatchTable->ResUnloadFile(
				ExtFilePtr,
				DispatchTable->Context);

			throw;
		}

		if (ExtFileSize != 0)
			memcpy( &FileContents[ 0 ], ExtFilePtr, ExtFileSize );

		DispatchTable->ResUnloadFile( ExtFilePtr, DispatchTable->Context );

		return true;
	}
	else
	{
		TextOut->WriteText(
			"Error: No resource providers accepted input file \"%s\".\n", InFile.c_str( ));

		return false;
	}
}


bool
CompileSourceFile(
	__in NscCompiler & Compiler,
	__in int CompilerVersion,
	__in bool Optimize,
	__in bool IgnoreIncludes,
	__in bool SuppressDebugSymbols,
	__in bool Quiet,
	__in IDebugTextOut * TextOut,
	__in const NWN::ResRef32 & InFile,
	__in const std::vector< unsigned char > & InFileContents,
	__in const std::string & OutBaseFile
	)
/*++

Routine Description:

	This routine compiles a single source file according to the specified set of
	compilation options.

Arguments:

	NscCompiler - Supplies the compiler context that will be used to process the
	              request.

	CompilerVersion - Supplies the BioWare-compatible compiler version number.

	Optimize - Supplies a Boolean value indicating true if the script should be
	           optimized.

	IgnoreIncludes - Supplies a Boolean value indicating true if include-only
	                 source files should be ignored.

	SuppressDebugSymbols - Supplies a Boolean value indicating true if debug
	                       symbol generation should be suppressed.

	Quiet - Supplies a Boolean value that indicates true if non-critical
	        messages should be silenced.

	TextOut - Supplies the text out interface used to receive any diagnostics
	          issued.

	InFile - Supplies the RESREF corresponding to the input file name.

	InFileContents - Supplies the contents of the input file.

	OutBaseFile - Supplies the base name (potentially including path) of the
	              output file.  No extension is present.

Return Value:

	The routine returns a Boolean value indicating true on success, else false
	on failure.

	On catastrophic failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	std::vector< unsigned char >   Code;
	std::vector< unsigned char >   Symbols;
	NscResult                      Result;
	std::string                    FileName;
	FILE                         * f;

	if (!Quiet)
	{
		TextOut->WriteText(
			"Compiling: %.32s.NSS\n",
			InFile.RefStr);
	}

	//
	// Execute the main compilation pass.
	//

	Result = Compiler.NscCompileScript(
		InFile,
		(!InFileContents.empty( )) ? &InFileContents[ 0 ] : NULL,
		InFileContents.size( ),
		CompilerVersion,
		Optimize,
		IgnoreIncludes,
		TextOut,
		0,
		Code,
		Symbols);

	switch (Result)
	{

	case NscResult_Failure:
		TextOut->WriteText(
			"Compilation aborted with errors.\n");

		return false;

	case NscResult_Include:
		if (!Quiet)
		{
			TextOut->WriteText(
				"%.32s.nss is an include file, ignored.\n",
				InFile.RefStr);
		}

		return true;

	case NscResult_Success:
		break;

	default:
		TextOut->WriteText(
			"Unknown compiler status code.\n");

		return false;

	}

	//
	// If we compiled successfully, write the results to disk.
	//

	FileName  = OutBaseFile;
	FileName += ".ncs";

	f = fopen( FileName.c_str( ), "wb" );

	if (f == NULL)
	{
		TextOut->WriteText(
			"Error: Unable to open output file \"%s\".\n",
			FileName.c_str( ));

		return false;
	}

	if (!Code.empty( ))
	{
		if (fwrite( &Code[ 0 ], Code.size( ), 1, f ) != 1)
		{
			fclose( f );

			TextOut->WriteText(
				"Error: Failed to write to output file \"%s\".\n",
				FileName.c_str( ));

			return false;
		}
	}

	fclose( f );

	if (!SuppressDebugSymbols)
	{
		FileName  = OutBaseFile;
		FileName += ".ndb";

		f = fopen( FileName.c_str( ), "wb" );

		if (f == NULL)
		{
			TextOut->WriteText(
				"Error: Failed to open debug symbols file \"%s\".\n",
				FileName.c_str( ));

			return false;
		}

		if (!Symbols.empty( ))
		{
			if (fwrite( &Symbols[ 0 ], Symbols.size( ), 1, f ) != 1)
			{
				fclose( f );

				TextOut->WriteText(
					"Error: Failed to write to debug symbols file \"%s\".\n",
					FileName.c_str( ));

				return false;
			}
		}

		fclose( f );
	}

	return true;
}

NSC_COMPILER_HANDLE
__stdcall
NscCreateCompiler(
	__in bool EnableExtensions
	)
/*++

Routine Description:

	This routine creates a new compiler context (and a new resource system if
	none exists yet).

Arguments:

	EnableExtension - Supplies a Boolean value indicating whether non-BioWare
	                  compiler extensions are to be enabled (true) or disabled
	                  (false).

Return Value:

	On success, the routine returns a pointer to a new compiler context.
	On failure, NULL is returned.

Environment:

	User mode, external entry point.

--*/
{
	PNSC_COMPILER_CONTEXT Compiler;

	Compiler = NULL;

	//
	// Currently, only a single instance is supported as the instances may not
	// share the same resource indexing topology and we don't want to have to
	// flush the resource system on each request.
	//

	if (g_ResManRefs != 0)
		return NULL;

	try
	{
		if (g_ResManRefs == 0)
		{
			g_ResMan = new ResourceManager( &g_TextOut );

			g_ResManRefs += 1;
		}

		Compiler = new NSC_COMPILER_CONTEXT;

		Compiler->DispatchTable = NULL;
		Compiler->Compiler = NULL;

		Compiler->Compiler = new NscCompiler(
			*g_ResMan,
			EnableExtensions,
			true);
	}
	catch (std::exception)
	{
		if (Compiler != NULL)
		{
			NscDeleteCompiler( Compiler );
			Compiler = NULL;
		}
	}

	return Compiler;
}

bool
__stdcall
NscDeleteCompiler(
	__in NSC_COMPILER_HANDLE Compiler
	)
/*++

Routine Description:

	This routine deletes a compiler context.

Arguments:

	EnableExtension - Supplies a Boolean value indicating whether non-BioWare
	                  compiler extensions are to be enabled (true) or disabled
	                  (false).

Return Value:

	On success, the routine returns a pointer to a new compiler context.
	On failure, NULL is returned.

Environment:

	User mode, external entry point.

--*/
{
	if (Compiler == NULL)
		return false;

	if (Compiler->Compiler != NULL)
		delete Compiler->Compiler;

	delete Compiler;

	//
	// Tear down the resource manager if this was our last user.
	//

	if (--g_ResManRefs == 0)
	{
		delete g_ResMan;
		g_ResMan = NULL;
	}

	return true;
}

bool
__stdcall
NscCompileScriptExternal(
	__in NSC_COMPILER_HANDLE Compiler,
	__in const char * ScriptFileName,
	__in const char * OutputDirectory,
	__in bool FlushResources,
	__in bool GenerateDebugInfo,
	__in bool Optimize,
	__in bool IgnoreIncludes,
	__in int CompilerVersion,
	__in PCNSC_COMPILER_DISPATCH_TABLE DispatchTable
	)
/*++

Routine Description:

	This routine compiles a single source file according to the specified set of
	compilation options.

Arguments:

	Compiler - Supplies the compiler context created by NscCreateCompiler.

	ScriptFileName - Supplies the file name of the script to compile.

	OutputDirectory - Supplies the directory where the compiled script should be
	                  placed (on success).

	FlushResources - Supplies a Boolean value indicating true if the resource
	                 system must be reindexed.  The caller must supply true for
	                 (at least) the first invocation for a given compiler.

	GenerateDebugInfo - Supplies a Boolean value indicating true if debug
	                    symbols should be saved.

	Optimize - Supplies a Boolean value indicating true if the script should be
	           optimized.

	IgnoreIncludes - Supplies a Boolean value indicating true if include-only
	                 source files should be ignored.

	CompilerVersion - Supplies the BioWare-compatible compiler version number.

	DispatchTable - Supplies the I/O dispatch table for the compiler.

Return Value:

	The routine returns a Boolean value indicating true on success, else false
	on failure.

Environment:

	User mode, external entry point.

--*/
{
	bool Status;

	switch (DispatchTable->Size)
	{

	case sizeof( NSC_COMPILER_DISPATCH_TABLE_V1 ):
		break;

	case sizeof( *DispatchTable ):
		break;

	default:
		return false;

	}

	try
	{
		std::vector< unsigned char > InFileContents;
		StringTextOut                CaptureOutput;
		NWN::ResRef32                InFileResRef;
		NWN::ResType                 InFileResType;
		std::string                  OutBaseFile;
		char                         FileName[ _MAX_FNAME ];

		if (DispatchTable->Size >= sizeof( NSC_COMPILER_DISPATCH_TABLE_V2 ))
		{
			if ((DispatchTable->ResLoadFile != NULL) &&
			    (DispatchTable->ResUnloadFile != NULL))
			{
				Compiler->Compiler->NscSetExternalResourceLoader(
					DispatchTable->Context,
					DispatchTable->ResLoadFile,
					DispatchTable->ResUnloadFile);
			}
		}

		if (_splitpath_s(
			ScriptFileName,
			NULL,
			0,
			NULL,
			0,
			FileName,
			_MAX_FNAME,
			NULL,
			0))
		{
			throw std::runtime_error( "Error: Invalid script source file path.");
		}

		OutBaseFile  = OutputDirectory;
		OutBaseFile += "/";
		OutBaseFile += FileName;

		//
		// Setup the resource system to point to the requestors I/O dispatch
		// table and fire off the compilation operation.
		//

		g_ResAccessor.SetIoDispatchTable( DispatchTable );

		if (FlushResources)
		{
			ResourceManager::ModuleLoadParams    LoadParams;
			IResourceAccessor< NWN::ResRef32 > * Accessor;

			Accessor = &g_ResAccessor;
	
			ZeroMemory( &LoadParams, sizeof( LoadParams ) );

			LoadParams.ResManFlags                   = ResourceManager::ResManFlagNoBuiltinProviders;
			LoadParams.CustomFirstChanceAccessors    = &Accessor;
			LoadParams.NumCustomFirstChanceAccessors = 1;

			g_ResMan->LoadModuleResources(
				"",
				"",
				"",
				"",
				std::vector< NWN::ResRef32 >( ),
				&LoadParams);
		}

		if (!LoadInputFile(
			*g_ResMan,
			&g_TextOut,
			DispatchTable,
			std::string( ScriptFileName ) + ".nss",
			InFileResRef,
			InFileResType,
			InFileContents))
		{
			throw std::runtime_error( "Error: Unable to access input file for compilation." );
		}

		Status = CompileSourceFile(
			*Compiler->Compiler,
			CompilerVersion,
			Optimize,
			IgnoreIncludes,
			!GenerateDebugInfo,
			true,
			&CaptureOutput,
			InFileResRef,
			InFileContents,
			OutBaseFile);

		//
		// If we have any accumulated diagnostics, issue them out now.
		//

		if (!CaptureOutput.GetTextOutput( ).empty( ))
		{
			DispatchTable->NscCompilerDiagnosticOutput(
				CaptureOutput.GetTextOutput( ).c_str( ),
				DispatchTable->Context);
		}
	}
	catch (std::exception &e)
	{
		DispatchTable->NscCompilerDiagnosticOutput(
			"Internal compiler error; compilation aborted (see below).\n",
			DispatchTable->Context);
		DispatchTable->NscCompilerDiagnosticOutput(
			e.what( ),
			DispatchTable->Context);

		Status = false;
	}

	//
	// Ensure that all resource references are closed as the dispatch table is
	// going away.
	//

	g_ResMan->CloseOpenResourceFileHandles( );

	g_ResAccessor.SetIoDispatchTable( NULL );

	Compiler->Compiler->NscSetExternalResourceLoader( NULL, NULL, NULL );

	return Status;
}

const char *
__stdcall
NscGetEntrypointSymbolName(
	__in NSC_COMPILER_HANDLE Compiler
	)
/*++

Routine Description:

	This routine returns the symbol name of a script program entry point.

Arguments:

	Compiler - Supplies the compiler context created by NscCreateCompiler.

Return Value:

	On success, the entry point symbol name is returned.
	On failure, NULL is returned.

Environment:

	User mode, external entry point.

--*/
{
	return Compiler->Compiler->NscGetEntrypointSymbolName( );
}

int
__stdcall
NscGetFunctionParameterCount(
	__in NSC_COMPILER_HANDLE Compiler,
	__in const char * FunctionName
	)
/*++

Routine Description:

	This routine returns the number of parameters that a script function takes.

Arguments:

	Compiler - Supplies the compiler context created by NscCreateCompiler.

	FunctionName - Supplies the symbol name of the function to inquire about.

Return Value:

	On success, the parameter count is returned (including default parameters).
	On failure, -1 is returned.

Environment:

	User mode, external entry point.

--*/
{
	try
	{
		NscPrototypeDefinition Prototype;

		if (!Compiler->Compiler->NscGetFunctionPrototype(
			FunctionName,
			Prototype))
		{
			return -1;
		}

		return (int) Prototype.NumParameters;
	}
	catch (std::exception)
	{
		return -1;
	}
}

const char *
__stdcall
NscGetCompilerBuildDate(
	)
/*++

Routine Description:

	This routine returns the build timestamp of the compiler module.

	N.B.  Only the build timestamp of this source file is returned.

Arguments:

	None.

Return Value:

	The routine returns a pointer to the build timestamp string, which remains
	valid as long as the compiler DLL is loaded.

Environment:

	User mode, external entry point.

--*/
{
	return __DATE__ " " __TIME__;
}

NWACTION_TYPE
__stdcall
NscGetFunctionParameterType(
	__in NSC_COMPILER_HANDLE Compiler,
	__in const char * FunctionName,
	__in int ParameterIndex
	)
/*++

Routine Description:

	This routine returns the type of an indexed parameter to a script function
	by name.

Arguments:

	Compiler - Supplies the compiler context created by NscCreateCompiler.

	FunctionName - Supplies the symbol name of the function to inquire about.

	ParameterIndex - Supplies the zero-based index of the parameter to query.

Return Value:

	On success, the parameter type is returned.
	On failure, ACTIONTYPE_VOID is returned.

Environment:

	User mode, external entry point.

--*/
{
	try
	{
		NscPrototypeDefinition Prototype;

		if (!Compiler->Compiler->NscGetFunctionPrototype(
			FunctionName,
			Prototype))
		{
			return ACTIONTYPE_VOID;
		}

		if ((size_t) ParameterIndex >= Prototype.ParameterTypes.size( ))
			return ACTIONTYPE_VOID;
		else
			return ConvertNscType(
				Prototype.ParameterTypes[ (size_t) ParameterIndex ]);
	}
	catch (std::exception)
	{
		return ACTIONTYPE_VOID;
	}
}

NWACTION_TYPE
__stdcall
NscGetFunctionReturnType(
	__in NSC_COMPILER_HANDLE Compiler,
	__in const char * FunctionName
	)
/*++

Routine Description:

	This routine returns the return type of a script function by name.

Arguments:

	Compiler - Supplies the compiler context created by NscCreateCompiler.

	FunctionName - Supplies the symbol name of the function to inquire about.

Return Value:

	On success, the return type is returned.
	On failure, ACTIONTYPE_VOID is returned.

Environment:

	User mode, external entry point.

--*/
{
	try
	{
		NscPrototypeDefinition Prototype;

		if (!Compiler->Compiler->NscGetFunctionPrototype(
			FunctionName,
			Prototype))
		{
			return ACTIONTYPE_VOID;
		}

		return ConvertNscType( Prototype.ReturnType );
	}
	catch (std::exception)
	{
		return ACTIONTYPE_VOID;
	}
}

} // namespace NscExt



const char *
__stdcall
I_NscGetEntrypointSymbolName(
	__in NSC_COMPILER_HANDLE Compiler
	)
/*++

Routine Description:

	This routine returns the symbol name of a script program entry point.

	N.B.  This wrapper is used to allow the VS2010 linker to be provided with
	      an unambiguous export symbol plain name.

Arguments:

	Compiler - Supplies the compiler context created by NscCreateCompiler.

Return Value:

	On success, the entry point symbol name is returned.
	On failure, NULL is returned.

Environment:

	User mode, external entry point.

--*/
{
	return NscExt::NscGetEntrypointSymbolName( Compiler );
}



BOOL
WINAPI
DllMain(
	__in HINSTANCE Module,
	__in ULONG Reason,
	__in PVOID Reserved
	)
/*++

Routine Description:

	This routine is the main program entry point symbol.  It bears
	responsibility for initializing the application.

Arguments:

	Module - Supplies the instance handle of the DLL.

	Reason - Supplies the DLL callout reason.  Legal values are drawn from the
	         DLL_PROCESS_* family of constants.

	Reserved - For DLL_PROCESS_ATTACH/DLL_PROCESS_DETACH, supplies a non-NULL
	           value if the module is loading statically, else NULL if the
	           module is loading dynamically.

Return Value:

	The routine always returns TRUE to indicate that the load was successful.

Environment:

	User mode.

--*/
{
	UNREFERENCED_PARAMETER( Module );
	UNREFERENCED_PARAMETER( Reason );
	UNREFERENCED_PARAMETER( Reserved );

	return TRUE;
}

