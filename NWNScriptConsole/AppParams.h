/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	AppParams.h

Abstract:

	This module defines the application parameter parsing logic of the script
	console application.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTCONSOLE_APPPARAMS_H
#define _SOURCE_PROGRAMS_NWNSCRIPTCONSOLE_APPPARAMS_H

#ifdef _MSC_VER
#pragma once
#endif

//
// Initialization parameters.  The AppParameters object maintains the
// configurable server parameter settings parsed from the command line.  It
// also communicates shared subsystem manager instance pointers that are to be
// communicated to the NWServer core.
//

class AppParameters
{

public:

	typedef std::vector< std::string > ScriptParamVec;

	inline
	AppParameters(
		__in IDebugTextOut * TextOut,
		__in int argc,
		__in_ecount( argc ) wchar_t * * argv
		)
	: m_TextOut( TextOut ),
	  m_argc( argc ),
	  m_argv( argv ),
	  m_InstallDir( "" ),
	  m_ModuleName( "" ),
	  m_ScriptName( "" ),
	  m_LogFile( "" ),
	  m_NoLogo( false ),
	  m_AllowManagedScripts( false ),
	  m_ScriptDebug( 1 ), // NWScriptVM::EDL_Errors
	  m_TestMode( 0 )
	{
		FindCriticalDirectories( );
		ParseArguments( m_argc, const_cast< const wchar_t * * >( m_argv ) );
	}

	inline IDebugTextOut * GetTextOut( ) { return m_TextOut; }
	inline void SetTextOut( __in IDebugTextOut * TextOut ) { m_TextOut = TextOut; }

	inline const std::string & GetInstallDir( ) const { return m_InstallDir; }
	inline void SetInstallDir( __in const std::string & InstallDir ) { m_InstallDir = InstallDir; }

	inline const std::string & GetHomeDir( ) const { return m_HomeDir; }
	inline void SetHomeDir( __in const std::string & HomeDir ) { m_HomeDir = HomeDir; }

	//
	// Configuration parameter access.
	//

	inline const std::string & GetModuleName( ) const { return m_ModuleName; }
	inline void SetModuleName( __in const std::string & ModuleName ) { m_ModuleName = ModuleName; }

	inline const std::string & GetScriptName( ) const { return m_ScriptName; }
	inline void SetScriptName( __in const std::string & ScriptName ) { m_ScriptName = ScriptName; }

	inline const ScriptParamVec & GetScriptParams( ) const { return m_ScriptParams; }
	inline void SetScriptParams( __in const ScriptParamVec & ScriptParams ) { m_ScriptParams = ScriptParams; }

	inline const std::string & GetLogFile( ) const { return m_LogFile; }
	inline void SetLogFile( __in const std::string & LogFile ) { m_LogFile = LogFile; }

	inline bool GetIsNoLogo( ) const { return m_NoLogo; }
	inline void SetIsNoLogo( __in bool NoLogo ) { m_NoLogo = NoLogo; }
	
	inline bool GetAllowManagedScripts( ) const { return m_AllowManagedScripts; }
	inline void SetAllowManagedScripts( __in bool AllowManagedScripts ) { m_AllowManagedScripts = AllowManagedScripts; }

	inline int GetScriptDebug( ) const { return m_ScriptDebug; }
	inline void SetScriptDebug( __in int ScriptDebug ) { m_ScriptDebug = ScriptDebug; }

	inline int GetTestMode( ) const { return m_TestMode; }
	inline void SetTestMode( __in int TestMode ) { m_TestMode = TestMode; }

private:

	void
	ParseArguments(
		__in int argc,
		__in_ecount( argc ) const wchar_t * * argv
		);

	void
	ParseResponseFile(
		__in const wchar_t * ResponseFileName
		);

	void
	FindCriticalDirectories(
		);

	std::string
	GetNwn2InstallPath(
		) const;

	std::string
	GetNwn2HomePath(
		) const;

	IDebugTextOut            * m_TextOut;
	bool                       m_FileParsed;
	int                        m_argc;
	wchar_t                * * m_argv;
	std::string                m_InstallDir;
	std::string                m_HomeDir;

	//
	// Configuration parameters.
	//

	std::string                m_ModuleName;
	std::string                m_ScriptName;
	ScriptParamVec             m_ScriptParams;
	std::string                m_LogFile;
	bool                       m_NoLogo;
	bool                       m_AllowManagedScripts;
	int                        m_ScriptDebug;
	int                        m_TestMode;

};

typedef const AppParameters * PCAppParameters;

#endif
