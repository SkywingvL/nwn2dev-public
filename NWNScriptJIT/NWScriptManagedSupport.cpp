/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptManagedSupport.cpp

Abstract:

	This module houses the representation for supporting data for managed
	language "scripts".

--*/

#include "Precomp.h"
#include "NWNScriptJIT.h"
#include "NWScriptCodeGenerator.h"
#include "NWScriptManagedSupport.h"
#include "NWScriptUtilities.h"

NWScriptManagedSupport::NWScriptManagedSupport(
	__in const NWScriptAnalyzer * Analyzer,
	__in_opt IDebugTextOut * TextOut,
	__in ULONG DebugLevel,
	__in INWScriptActions * ActionHandler,
	__in NWN::OBJECTID ObjectInvalid,
	__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams
	)
/*++

Routine Description:

	This routine constructs a new NWScriptManagedSupport.  It bears
	responsibility for generating code for the managed interface layer, and for
	maintaining a reference to the generated assembly.

Arguments:

	Analyzer - Supplies the analysis context that describes the NWScript action
	           service table to generate code for.

	TextOut - Optionally supplies an IDebugTextOut interface that receives text
	          debug output from the execution environment.

	DebugLevel - Supplies the debug output level.  Legal values are drawn from
	             the NWScriptVM::ExecDebugLevel family of enumerations.

	ActionHandler - Supplies the engine actions implementation handler.

	ObjectInvalid - Supplies the object id to reference for the 'object
	                invalid' manifest constant.

	CodeGenParams - Optionally supplies extension code generation parameters.

Return Value:

	None.  Raises a System::Exception on failure.

Environment:

	User mode, C++/CLI.

--*/
{
	try
	{
		NWScriptCodeGenerator              Generator(
			TextOut,
			DebugLevel,
			INWScriptProgram::typeid,
			IGeneratedScriptProgram::typeid);
		NWScriptCodeGenerator::ProgramInfo Program;
		IntPtr                             UniqueId;

		UniqueId = (IntPtr) ActionHandler;

		Generator.GenerateInterfaceLayerCode(
			Analyzer,
			ActionHandler,
			ObjectInvalid,
			CodeGenParams,
			String::Format( "NWScriptManagedInterface_{0}", UniqueId ),
			NWScriptUtilities::NWUTF8Encoding,
			Program);

		m_Assembly             = Program.Assembly;
		m_EngineStructureTypes = Program.EngineStructureTypes;
	}
	catch (Exception ^ e)
	{
		ErrorException( e, TextOut, DebugLevel );
		throw;
	}
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

NWScriptManagedSupport::!NWScriptManagedSupport(
	)
/*++

Routine Description:

	This routine deletes the current NWScriptManagedSupport object and its
	associated members.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode, C++/CLI.

--*/
{
}

void
NWScriptManagedSupport::ErrorException(
	__in Exception ^ Excpt,
	__in_opt IDebugTextOut * TextOut,
	__in ULONG DebugLevel
	)
/*++

Routine Description:

	This routine issues an error diagnostic for an error-level exception.

Arguments:

	Excpt - Supplies the exception to issue the diagnostic for.

	TextOut - Optionally supplies an IDebugTextOut interface that receives text
	          debug output from the execution environment.

	DebugLevel - Supplies the debug output level.  Legal values are drawn from
	             the NWScriptVM::ExecDebugLevel family of enumerations.

Return Value:

	None.  This routine does not fail.

Environment:

	User mode, C++/CLI.

--*/
{
	if (TextOut == NULL)
		return;

	if (DebugLevel < NWScriptVM::EDL_Errors)
		return;

	//
	// Issue the diagnostic warning to the user.
	//

	try
	{
		StringBuilder ^ Diagnostic = gcnew StringBuilder( "NWScriptManagedSupport: Exception: '" );
		Exception     ^ InnerException;

		Diagnostic->Append( Excpt->Message );
		Diagnostic->Append( "', stack: " );
		Diagnostic->Append( Excpt->StackTrace );

		if ((InnerException = Excpt->InnerException) != nullptr)
		{
			Diagnostic->Append( ", InnerException: '" );
			Diagnostic->Append( InnerException->Message );
			Diagnostic->Append( "', stack: " );
			Diagnostic->Append( InnerException->StackTrace );
		}

		Diagnostic->Append( "." );

		std::string ConvStr = NWScriptUtilities::ConvertString( Diagnostic->ToString( ) );

		TextOut->WriteText(
			"%s\n",
			ConvStr.c_str( ));
	}
	catch (Exception ^)
	{
		//
		// Silently consume any exceptions without allowing them to propagate
		// upwards, so that the original failure is not obscured.
		//
	}
}



