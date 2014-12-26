/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptCodeGenerator.cpp

Abstract:

	This module houses the code generator for NWScript.  Its purpose is to
	construct an assembly given a source NWScript program in IR representation.

--*/

#include "Precomp.h"
#include "NWNScriptJIT.h"
#include "NWScriptCodeGenerator.h"
#include "../NWNScriptLib/NWScriptInternal.h"
#include "NWScriptUtilities.h"

//
// N.B.  Additional switches in NWScriptProgram.h.
//

//
// Define to 1 to enable saving the assembly to disk (for debug purposes only).
//

#define NWSCRIPT_SAVE_ASSEMBLY        0

//
// Define to 1 to use casts and virtual method calls to work around an access
// issue in the CLR.
//

#define NWSCRIPT_VIRT_TYPE_WORKAROUND 0

//
// Define to 1 to enable loop and call depth checks to prevent denial of
// service agains the script host.
//

#define NWSCRIPT_EXECUTION_GUARDS     1

//
// Define to 1 to improve performance of execution guards by counting down to
// zero instead of counting up.
//

#define NWSCRIPT_FAST_EXEC_GUARDS     1

//
// Define to 1 to debug fast action service handler calls.
//

#define NWSCRIPT_DEBUG_ACTION_CALLS   0

//
// Define to 1 to use the fast action service handler call mechanism.
//

#define NWSCRIPT_FAST_ACTION_CALLS    1

//
// Define to tune the fast action call parameter list threshold.  Calls with
// more parameters use the fast (bulk) call mechanism, otherwise making a
// series of individual calls is considered to be faster and will be preferred.
//

#define NWSCRIPT_FAST_CALL_THRESHOLD  6

//
// Define to 1 to test save to stack (if enabled for compilation) for all saved
// states.
//

#define NWSCRIPT_TEST_SAVE_VMSTACK   0

//
// Define to 1 to use action invocation stubs for non-fast action service calls
// instead of inlining the VMStack* calls.
//

#define NWSCRIPT_SLOW_CALLS_VIA_STUB 1

//
// Define to disable usage of collectable assemblies due to an internal flaw in
// collection of LoaderAllocator objects associated with a collectable
// assembly.
//

#define NWSCRIPT_COLLECT_ASM_GC_BUG  1

//
// Define to 1 to make subroutines public (for managed script
// interoperability).
//

#define NWSCRIPT_PUBLIC_SUBROUTINES  1

//
// Define to 1 to enable generation of special intrinsics for NWN-compatible
// action service handlers.
//

#define NWSCRIPT_OPT_NWN_ACTIONS     1


NWScriptCodeGenerator::NWScriptCodeGenerator(
	__in_opt IDebugTextOut * TextOut,
	__in ULONG DebugLevel,
	__in Type ^ HostInterfaceType,
	__in Type ^ ScriptInterfaceType
	)
/*++

Routine Description:

	This routine constructs a new NWScirptCodeGenerator.

Arguments:

	TextOut - Optionally supplies an IDebugTextOut interface that receives text
	          debug output from the execution environment.

	DebugLevel - Supplies the debug output level.  Legal values are drawn from
	             the NWScriptVM::ExecDebugLevel family of enumerations.

	HostInterfaceType - Supplies the type of the interface class that is used
	                    to communicate with the script host.  Typically, this
	                    would be INWScriptProgram::typeid.

	ScriptInterfaceType - Supplies the type of the interface class that the
	                      generated script program implements.  Typically, this
	                      would be IGeneratedScriptProgram.

Return Value:

	None.  Raises a System::Exception on failure.

Environment:

	User mode, C++/CLI.

--*/
: m_TextOut( TextOut ),
  m_DebugLevel( DebugLevel ),
  m_HostInterfaceType( HostInterfaceType ),
  m_ScriptInterfaceType( ScriptInterfaceType ),
  m_ActionHandler( NULL ),
  m_ActionDefs( NULL ),
  m_ActionCount( 0 ),
  m_ILGenCtx( nullptr ),
  m_EngineStructureTypes( nullptr )
{
	C_ASSERT( NUM_ENGINE_STRUCTURE_TYPES == LAST_ENGINE_STRUCTURE + 1 );
}

NWScriptCodeGenerator::!NWScriptCodeGenerator(
	)
/*++

Routine Description:

	This routine deletes the current NWScriptCodeGenerator object and its
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
NWScriptCodeGenerator::GenerateProgramCode(
	__in const NWScriptAnalyzer * Analyzer,
	__in INWScriptActions * ActionHandler,
	__in NWN::OBJECTID ObjectInvalid,
	__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams,
	__in String ^ Name,
	__in Encoding ^ StringEncoding,
	__out ProgramInfo % Program
	)
/*++

Routine Description:

	This routine constructs an MSIL representation of the functional nature of
	a NWScript program.  The representation is saved for invocation by native
	code (through the NWScriptProgram::ExecuteScript C++/CLI adapter).

Arguments:

	Analyzer - Supplies the analysis context that describes the NWScript IR to
	           generate code for.

	ActionHandler - Supplies the engine actions implementation handler.

	ObjectInvalid - Supplies the object id to reference for the 'object
	                invalid' manifest constant.

	CodeGenParams - Optionally supplies extension code generation parameters.

	Name - Supplies the name prefix to apply to the type within which the
	       script program MSIL representation is placed.

	StringEncoding - Supplies the encoding object to use for string creation
	                 during the code generation phase.

	Program - Receives the assembly and type information for the generated
	          script program.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	AssemblyBuilder       ^ Assembly;
	ModuleBuilder         ^ Module;
	TypeBuilder           ^ ProgramType;
	bool                    SaveAsm;

	//
	// Perform common initialization.
	//

	SetupCodeGeneration(
		Analyzer,
		ActionHandler,
		CodeGenParams,
		Name,
		StringEncoding,
		false,
		Assembly,
		Module,
		SaveAsm);

	//
	// Next, prepare the overarching type that contains all the methods and
	// member variables (globals) of the script.
	//

	ProgramType = GenerateProgramType( Module, Name, ObjectInvalid );

#if NWSCRIPT_OPT_NWN_ACTIONS

	//
	// Generate optimized action service handlers.
	//

	GenerateOptimizedActionServiceStubs( );

#endif

	//
	// Now emit code for the program IR.
	//

	GenerateAllSubroutines( );

	//
	// Finalize the program type and create an instance that we can use for
	// execution.
	//

	Program.Type = ProgramType->CreateType( );

	if (SaveAsm)
		Assembly->Save( GenerateAsmName( Name, false ) + ".dll" );

	Program.Assembly             = Assembly;
	Program.EngineStructureTypes = m_EngineStructureTypes;

	//
	// Finally, release the IL generation context, as it's no longer necessary.
	//

	m_ActionHandler        = NULL;
	m_ActionDefs           = NULL;
	m_ActionCount          = 0;
	m_ILGenCtx             = nullptr;
	m_EngineStructureTypes = nullptr;
}


void
NWScriptCodeGenerator::GenerateInterfaceLayerCode(
	__in const NWScriptAnalyzer * Analyzer,
	__in INWScriptActions * ActionHandler,
	__in NWN::OBJECTID ObjectInvalid,
	__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams,
	__in String ^ Name,
	__in Encoding ^ StringEncoding,
	__out ProgramInfo % Program
	)
/*++

Routine Description:

	This routine constructs an MSIL representation of the interface DLL for CLR
	clients of the NWScript runtime environment.  The representation is saved
	for invocation by CLR code (through the NWScriptProgram::ExecuteScript
	C++/CLI adapter).

Arguments:

	Analyzer - Supplies the analysis context that describes the NWScript action
	           service table to generate code for.

	ActionHandler - Supplies the engine actions implementation handler.

	ObjectInvalid - Supplies the object id to reference for the 'object
	                invalid' manifest constant.

	CodeGenParams - Optionally supplies extension code generation parameters.

	Name - Supplies the name prefix to apply to the type within which the
	       interface layer MSIL representation is placed.

	StringEncoding - Supplies the encoding object to use for string creation
	                 during the code generation phase.

	Program - Receives the assembly and type information for the generated
	          interface layer.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	AssemblyBuilder       ^ Assembly;
	ModuleBuilder         ^ Module;
	TypeBuilder           ^ ProgramType;
	bool                    SaveAsm;

	//
	// Perform common initialization.
	//

	SetupCodeGeneration(
		Analyzer,
		ActionHandler,
		CodeGenParams,
		Name,
		StringEncoding,
		true,
		Assembly,
		Module,
		SaveAsm);

	m_ILGenCtx->ActionServiceStubAttributes = MethodAttributes::Public;

	//
	// Next, prepare the overarching type that contains all the methods for
	// the interface layer.
	//

	ProgramType = GenerateInterfaceLayerType( Module, Name, ObjectInvalid );

#if NWSCRIPT_OPT_NWN_ACTIONS

	//
	// Generate optimized action service handlers.
	//

	GenerateOptimizedActionServiceStubs( );

#endif

	//
	// Now emit code for the interface layer.
	//

	GenerateInterfaceLayerActionServiceSubroutines( );

	//
	// Finalize the program type and create an instance that we can use for
	// execution.
	//

	Program.Type = ProgramType->CreateType( );

	if (SaveAsm)
		Assembly->Save( GenerateAsmName( Name, true ) + ".dll" );

	Program.Assembly             = Assembly;
	Program.EngineStructureTypes = m_EngineStructureTypes;

	//
	// Finally, release the IL generation context, as it's no longer necessary.
	//

	m_ActionHandler        = NULL;
	m_ActionDefs           = NULL;
	m_ActionCount          = 0;
	m_ILGenCtx             = nullptr;
	m_EngineStructureTypes = nullptr;
}


void
NWScriptCodeGenerator::SetupCodeGeneration(
	__in const NWScriptAnalyzer * Analyzer,
	__in INWScriptActions * ActionHandler,
	__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams,
	__in String ^ Name,
	__in Encoding ^ StringEncoding,
	__in bool InterfaceLayer,
	__out AssemblyBuilder ^ % Assembly,
	__out ModuleBuilder ^ % Module,
	__out bool % SaveAsm
	)
/*++

Routine Description:

	This routine performs common initialization tasks for code generation
	setup.

Arguments:

	Analyzer - Supplies the analysis context that describes the NWScript action
	           service table to generate code for.

	ActionHandler - Supplies the engine actions implementation handler.

	CodeGenParams - Optionally supplies extension code generation parameters.

	Name - Supplies the name prefix to apply to the type within which the
	       interface layer MSIL representation is placed.

	StringEncoding - Supplies the encoding object to use for string creation
	                 during the code generation phase.

	InterfaceLayer - Supplies a Boolean value indicating true if setup is to
	                 be performed for an interface layer module, versus a
	                 script program module.

	Assembly - Receives the assembly builder for the new assembly.

	Module - Receives the module builder for the new module.

	SaveAsm - Receives a Boolean value indicating true if the assembly is to be
	          saved to disk.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	PCNWACTION_DEFINITION   ActionDefs;
	NWSCRIPT_ACTION         ActionCount;
	String                ^ OutputDir;

	m_ActionHandler = ActionHandler;

	Analyzer->GetActionDefs( ActionDefs, ActionCount );

	m_ActionDefs  = ActionDefs;
	m_ActionCount = ActionCount;

	//
	// Set up the ILGen context.
	//

	m_ILGenCtx = CreateILGenContext( Analyzer, CodeGenParams );

	m_ILGenCtx->StringEncoding = StringEncoding;

#if NWSCRIPT_SAVE_ASSEMBLY
	SaveAsm = true;
#else
	SaveAsm = false;
#endif

	try
	{
		OutputDir = Directory::GetCurrentDirectory( );
	}
	catch (NotSupportedException ^)
	{
		OutputDir = "";
	}

	if (m_ILGenCtx->CodeGenParams != NULL)
	{
		if (m_ILGenCtx->CodeGenParams->CodeGenFlags & NWCGF_SAVE_OUTPUT)
		{
			SaveAsm = true;

			if (m_ILGenCtx->CodeGenParams->CodeGenOutputDir != NULL)
				OutputDir = gcnew String( m_ILGenCtx->CodeGenParams->CodeGenOutputDir );
		}
	}

	//
	// First, generate the assembly for the target.
	//

	Assembly = GenerateProgramAssembly(
		AppDomain::CurrentDomain,
		Name,
		OutputDir,
		SaveAsm,
		InterfaceLayer);

	//
	// Now build a single module.
	//

	Module = GenerateProgramModule( Assembly );
}


NWScriptCodeGenerator::ILGenContext ^
NWScriptCodeGenerator::CreateILGenContext(
	__in const NWScriptAnalyzer * Analyzer,
	__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams
	)
/*++

Routine Description:

	This routine constructs a new ILGenContext object.

Arguments:

	Analyzer - Optionally supplies the analysis context that describes the
	           NWScript IR to generate code for.

	CodeGenParams - Optionally supplies extension code generation parameters.

Return Value:

	A handle to a new ILGenContext instance is returned.
	
	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	ILGenContext ^ ILGenCtx;

	ILGenCtx = gcnew ILGenContext;

	ILGenCtx->Analyzer                          = Analyzer;
	ILGenCtx->CodeGenParams                     = CodeGenParams;
	ILGenCtx->NextResumeMethodId                = 0;
	ILGenCtx->MethodMap                         = gcnew PCMethodMap;
	ILGenCtx->ActionServiceStubAttributes       = MethodAttributes::Private;
	ILGenCtx->MaxLoopIterations                 = MAX_LOOP_ITERATIONS;
	ILGenCtx->MaxCallDepth                      = MAX_CALL_DEPTH;

	if ((ARGUMENT_PRESENT( CodeGenParams )) &&
	    (CodeGenParams->Size >= NWSCRIPT_JIT_PARAMS_SIZE_V2))
	{
		ILGenCtx->MaxLoopIterations             = CodeGenParams->MaxLoopIterations;
		ILGenCtx->MaxCallDepth                  = CodeGenParams->MaxCallDepth;
	}

	if (ILGenCtx->MaxLoopIterations <= 0)
		ILGenCtx->MaxLoopIterations             = MAX_LOOP_ITERATIONS;
	if (ILGenCtx->MaxCallDepth <= 0)
		ILGenCtx->MaxCallDepth                  = MAX_CALL_DEPTH;

	//
	// Now cache some frequently looked up system-supplied intrinsic methods.
	//

	ILGenCtx->MthString_Equals                  = String::typeid->GetMethod( "Equals", gcnew array< Type ^ >{ String::typeid } );
	ILGenCtx->MthString_Concat                  = String::typeid->GetMethod( "Concat", gcnew array< Type ^ >{ String::typeid, String::typeid } );
	ILGenCtx->MthMethodBase_GetMethodFromHandle = MethodBase::typeid->GetMethod( "GetMethodFromHandle", gcnew array< Type ^ >{ RuntimeMethodHandle::typeid } );
	ILGenCtx->CtorException                     = Exception::typeid->GetConstructor( gcnew array< Type ^ >{ String::typeid } );
	ILGenCtx->FldVector3_x                      = NWScript::Vector3::typeid->GetField( "x" );
	ILGenCtx->FldVector3_y                      = NWScript::Vector3::typeid->GetField( "y" );
	ILGenCtx->FldVector3_z                      = NWScript::Vector3::typeid->GetField( "z" );

	if (ILGenCtx->MthString_Equals == nullptr)
		throw gcnew Exception( "Failed to locate System::String::Equals." );
	else if (ILGenCtx->MthString_Concat == nullptr)
		throw gcnew Exception( "Failed to locate System::String::Concat." );
	else if (ILGenCtx->MthMethodBase_GetMethodFromHandle == nullptr)
		throw gcnew Exception( "Failed to locate MethodBase::GetMethodFromHandle." );
	else if (ILGenCtx->CtorException == nullptr)
		throw gcnew Exception( "Failed to locate Exception::Exception." );

#if NWSCRIPT_DIRECT_FAST_ACTION_CALLS

	//
	// Create the direct fast action call stub array if we have direct stubs
	// turned on.  Each direct stub calls the native interface for a particular
	// action service handler directly.
	//

	ILGenCtx->MthActionServiceStubs = gcnew ActionMethodInfoArrArr( (Int32) m_ActionCount );

	//
	// Cache the native code address to call for direct action calls.  We can
	// devirtualize the call as we know that it will never actually change once
	// we generate code (as the generated code instance is already bound to a
	// particular INWScriptActions object).
	//

	ILGenCtx->PtrOnExecuteActionFromJITFast =
		((INWScriptActions_Raw *) m_ActionHandler)->Vtbl->OnExecuteActionFromJITFast;

#endif

	return ILGenCtx;
}


AssemblyBuilder ^
NWScriptCodeGenerator::GenerateProgramAssembly(
	__in AppDomain ^ Domain,
	__in String ^ Name,
	__in_opt String ^ OutputDir,
	__in bool SaveAsm,
	__in bool InterfaceLayer
	)
/*++

Routine Description:

	This routine constructs the dynamic assembly within which the script
	program's module is stored.

Arguments:

	Domain - Supplies the application domain to place the assembly within.

	Name - Supplies the name prefix to apply to the type within which the
	       script program MSIL representation is placed.

	OutputDir - Optionally supplies the directory to save the assembly to.

	SaveAsm - Supplies a Boolean value indicating true if the assembly is to be
	          saved to disk.

	InterfaceLayer - Supplies a Boolean value indicating true if the assembly
	                 to be generated is an interface layer module and not a
	                 script program.

Return Value:

	A handle to an AssemblyBuilder instance describing the program main
	assembly is returned.
	
	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	AssemblyBuilder       ^ ProgramAssembly;
	AssemblyName          ^ AsmName;
	AssemblyBuilderAccess   Access;

#if NWSCRIPT_SAVE_ASSEMBLY
	Access = AssemblyBuilderAccess::RunAndSave;
#else
#if NWSCRIPT_COLLECT_ASM_GC_BUG
	Access = AssemblyBuilderAccess::Run;
#else
	Access = AssemblyBuilderAccess::RunAndCollect;
#endif
#endif

	if (SaveAsm)
		Access = AssemblyBuilderAccess::RunAndSave;
	else
		OutputDir = nullptr;

	AsmName         = gcnew AssemblyName;
	AsmName->Name   = GenerateAsmName( Name, InterfaceLayer );
	ProgramAssembly = Domain->DefineDynamicAssembly(
		AsmName,
		Access,
		OutputDir);

	return ProgramAssembly;
}

ModuleBuilder ^
NWScriptCodeGenerator::GenerateProgramModule(
	__in AssemblyBuilder ^ Assembly
	)
/*++

Routine Description:

	This routine constructs the dynamic module within which the script
	program's type is stored.

Arguments:

	Assembly - Supplies the assembly to emit the program into.

Return Value:

	A handle to a ModuleBuilder instance describing the program main module is
	returned.
	
	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	ModuleBuilder ^ ProgramModule;
	AssemblyName  ^ AsmName;

	AsmName       = Assembly->GetName( );

#if NWSCRIPT_COLLECT_ASM_GC_BUG
	if ((m_ILGenCtx->CodeGenParams != NULL) &&
	    (m_ILGenCtx->CodeGenParams->CodeGenFlags & NWCGF_SAVE_OUTPUT))
	{
		ProgramModule = Assembly->DefineDynamicModule(
			AsmName->Name,
			AsmName->Name + ".dll");
	}
	else
	{
		ProgramModule = Assembly->DefineDynamicModule( AsmName->Name );
	}
#else
	ProgramModule = Assembly->DefineDynamicModule(
		AsmName->Name,
		AsmName->Name + ".dll");
#endif

	return ProgramModule;
}

TypeBuilder ^
NWScriptCodeGenerator::GenerateProgramType(
	__in ModuleBuilder ^ Module,
	__in String ^ Name,
	__in NWN::OBJECTID ObjectInvalid
	)
/*++

Routine Description:

	This routine constructs the dynamic type within which the script program's
	subroutine and global variables are stored.

Arguments:

	Module - Supplies the module to emit the program into.

	Name - Supplies the name prefix of the script.

	ObjectInvalid - Supplies the object id to reference for the 'object
	                invalid' manifest constant.

Return Value:

	A handle to a TypeBuilder instance describing the program main type is
	returned.
	
	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	TypeBuilder        ^ ProgramType;
	FieldBuilder       ^ FldCurrentActionObjectSelf;
	FieldBuilder       ^ FldProgram;
	FieldBuilder       ^ FldProgramInterface;
	FieldBuilder       ^ FldCallDepth;
	FieldBuilder       ^ FldLoopCounter;
	Type               ^ ObjType;
	ConstructorInfo    ^ ObjCtor;
	ConstructorBuilder ^ ProgCtor;
	ILGenerator        ^ ILGen;
	Type               ^ NWScriptProgramType;
	Type               ^ NWScriptJITIntrinsicsType;
	String             ^ Namespace;

	Namespace = GenerateProgramNamespace( Name );

	m_ILGenCtx->Namespace = Namespace;

	ProgramType = Module->DefineType(
		Namespace + "ScriptProgram",
		TypeAttributes::Public );

	//
	// All generated script programs export a uniform interface that is used to
	// invoke them.
	//
	
	ProgramType->AddInterfaceImplementation( m_ScriptInterfaceType );

	//
	// Now set up the standard fields of the program type.
	//

	FldCurrentActionObjectSelf = ProgramType->DefineField(
		"m_CurrentActionObjectSelf",
		NWN::OBJECTID::typeid,
		FieldAttributes::Private);

#if NWSCRIPT_VIRT_TYPE_WORKAROUND
	NWScriptProgramType = Object::typeid;
#else
	NWScriptProgramType = NWScriptJITIntrinsics::typeid;
#endif

	NWScriptJITIntrinsicsType = NWScriptJITIntrinsics::typeid;

	FldProgram = ProgramType->DefineField(
		"m_ProgramJITIntrinsics",
		NWScriptProgramType,
		FieldAttributes::Private);
	FldProgramInterface = ProgramType->DefineField(
		"m_ProgramInterface",
		m_HostInterfaceType,
		FieldAttributes::Private);
	FldCallDepth = ProgramType->DefineField(
		"m_CallDepth",
		UInt32::typeid,
		FieldAttributes::Private);
	FldLoopCounter = ProgramType->DefineField(
		"m_LoopCounter",
		UInt32::typeid,
		FieldAttributes::Private);

	//
	// Create the constructor, which takes two parameters (the NWScriptProgram
	// instance and the INWScriptJITIntrinsics  interface).
	//

	ObjType  = Object::typeid;
	ObjCtor  = ObjType->GetConstructor( Type::EmptyTypes );

	ProgCtor = ProgramType->DefineConstructor(
		MethodAttributes::Public,
		CallingConventions::Standard,
		gcnew array< Type ^ >{ NWScriptProgramType, m_HostInterfaceType });

	ILGen = ProgCtor->GetILGenerator( );

	//
	// System::Object::Object( this )
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Call, ObjCtor );

	//
	// m_CurrentActionObjectSelf = ObjectInvalid;
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldc_I4, (Int32) ObjectInvalid );
	ILGen->Emit( OpCodes::Stfld, FldCurrentActionObjectSelf );

	//
	// m_Program = Program
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldarg_1 );
	ILGen->Emit( OpCodes::Stfld, FldProgram );

	//
	// m_ProgramInterface = ProgramInterface
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldarg_2 );
	ILGen->Emit( OpCodes::Stfld, FldProgramInterface );

	//
	// Initialize the loop counter and call depth now.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
#if NWSCRIPT_FAST_EXEC_GUARDS
	ILGen->Emit( OpCodes::Ldc_I4, -m_ILGenCtx->MaxCallDepth );
#else
	ILGen->Emit( OpCodes::Ldc_I4_0 );
#endif
	ILGen->Emit( OpCodes::Stfld, FldCallDepth );

	ILGen->Emit( OpCodes::Ldarg_0 );
#if NWSCRIPT_FAST_EXEC_GUARDS
	ILGen->Emit( OpCodes::Ldc_I4, -m_ILGenCtx->MaxLoopIterations );
#else
	ILGen->Emit( OpCodes::Ldc_I4_0 );
#endif
	ILGen->Emit( OpCodes::Stfld, FldLoopCounter );

	ILGen->Emit( OpCodes::Ret );

	//
	// Now save fields away in the ILGenContext.
	//

	m_ILGenCtx->ObjectInvalid                            = ObjectInvalid;
	m_ILGenCtx->JITModule                                = NWScriptJITIntrinsicsType->Module;
	m_ILGenCtx->ProgramModule                            = Module;

	m_ILGenCtx->ProgramType                              = ProgramType;

	//
	// Prepare the engine structure type classes.
	//

	RetrieveJITIntrinsicsTypeInfo( NWScriptJITIntrinsicsType );

	m_ILGenCtx->FldCurrentActionObjectSelf              = FldCurrentActionObjectSelf;
	m_ILGenCtx->FldProgram                              = FldProgram;
	m_ILGenCtx->FldProgramInterface                     = FldProgramInterface;
	m_ILGenCtx->FldCallDepth                            = FldCallDepth;
	m_ILGenCtx->FldLoopCounter                          = FldLoopCounter;

	m_EngineStructureTypes = m_ILGenCtx->EngineStructureTypes;

	//
	// Prepare global variable storage within the program class type.
	//

	GenerateGlobals( ProgramType );

	//
	// Create the clone method.
	//

	m_ILGenCtx->MthCloneScriptProgram                   = GenerateCloneScriptProgram( ProgramType, ProgCtor );

	//
	// Create the load globals method.
	//

	m_ILGenCtx->MthLoadScriptGlobals                    = GenerateLoadScriptGlobals( ProgramType );

	return ProgramType;
}

TypeBuilder ^
NWScriptCodeGenerator::GenerateInterfaceLayerType(
	__in ModuleBuilder ^ Module,
	__in String ^ Name,
	__in NWN::OBJECTID ObjectInvalid
	)
/*++

Routine Description:

	This routine constructs the dynamic type within which the interface layer
	stores its state.

Arguments:

	Module - Supplies the module to emit the program into.

	Name - Supplies the name prefix of the interface layer module.

	ObjectInvalid - Supplies the object id to reference for the 'object
	                invalid' manifest constant.

Return Value:

	A handle to a TypeBuilder instance describing the program main type is
	returned.
	
	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	TypeBuilder        ^ ProgramType;
	FieldBuilder       ^ FldProgram;
	FieldBuilder       ^ FldProgramInterface;
	Type               ^ ObjType;
	ConstructorInfo    ^ ObjCtor;
	ConstructorBuilder ^ ProgCtor;
	ILGenerator        ^ ILGen;
	Type               ^ NWScriptProgramType;
	Type               ^ NWScriptJITIntrinsicsType;
	String             ^ Namespace;

	UNREFERENCED_PARAMETER( Name );

	Namespace  = "NWScript.ManagedInterfaceLayer.NWScriptManagedInterface.";

	m_ILGenCtx->Namespace = Namespace;

	ProgramType = Module->DefineType(
		Namespace + "ManagedNWScript",
		TypeAttributes::Public );

	//
	// Set up the standard fields of the interface type; the intrinsics and
	// script program interface backlinks.
	//

#if NWSCRIPT_VIRT_TYPE_WORKAROUND
	NWScriptProgramType = Object::typeid;
#else
	NWScriptProgramType = NWScriptJITIntrinsics::typeid;
#endif

	NWScriptJITIntrinsicsType = NWScriptJITIntrinsics::typeid;

	FldProgram = ProgramType->DefineField(
		"m_ProgramJITIntrinsics",
		NWScriptProgramType,
		FieldAttributes::Private);
	FldProgramInterface = ProgramType->DefineField(
		"m_ProgramInterface",
		m_HostInterfaceType,
		FieldAttributes::Private);

	//
	// Create properties to access the JIT intrinsics and program interface
	// members.
	//

	GenerateReadOnlyProperty( ProgramType, FldProgram, "Intrinsics" );
	GenerateReadOnlyProperty( ProgramType, FldProgramInterface, "Host" );
	GenerateLiteralField( ProgramType, "OBJECT_INVALID", (UInt32) ObjectInvalid );

	//
	// Create the constructor, which takes two parameters (the NWScriptProgram
	// instance and the INWScriptJITIntrinsics  interface).
	//

	ObjType  = Object::typeid;
	ObjCtor  = ObjType->GetConstructor( Type::EmptyTypes );

	ProgCtor = ProgramType->DefineConstructor(
		MethodAttributes::Public,
		CallingConventions::Standard,
		gcnew array< Type ^ >{ NWScriptProgramType, m_HostInterfaceType });

	ILGen = ProgCtor->GetILGenerator( );

	//
	// System::Object::Object( this )
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Call, ObjCtor );

	//
	// m_Program = Program
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldarg_1 );
	ILGen->Emit( OpCodes::Stfld, FldProgram );

	//
	// m_ProgramInterface = ProgramInterface
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldarg_2 );
	ILGen->Emit( OpCodes::Stfld, FldProgramInterface );

	ILGen->Emit( OpCodes::Ret );

	//
	// Now save fields away in the ILGenContext.
	//

	m_ILGenCtx->ObjectInvalid                            = ObjectInvalid;
	m_ILGenCtx->JITModule                                = NWScriptJITIntrinsicsType->Module;
	m_ILGenCtx->ProgramModule                            = Module;

	m_ILGenCtx->ProgramType                              = ProgramType;

	//
	// Prepare the engine structure type classes.
	//

	RetrieveJITIntrinsicsTypeInfo( NWScriptJITIntrinsicsType );

	m_ILGenCtx->FldProgram                              = FldProgram;
	m_ILGenCtx->FldProgramInterface                     = FldProgramInterface;

	m_EngineStructureTypes = m_ILGenCtx->EngineStructureTypes;

	return ProgramType;
}

void
NWScriptCodeGenerator::RetrieveJITIntrinsicsTypeInfo(
	__in Type ^ NWScriptJITIntrinsicsType
	)
/*++

Routine Description:

	This routine looks up and caches type information relating to the
	NWScriptJITIntrinsics type.  Type information is stored in the active
	ILGenContext.

Arguments:

	NWScriptJITIntrinsicsType - Supplies the type of the JITIntrinsics
	                            object.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	m_ILGenCtx->EngineStructureTypes                     = gcnew array< Type ^ >( NUM_ENGINE_STRUCTURE_TYPES );
	m_ILGenCtx->MthIntrinsic_VMStackPushEngineStructure  = gcnew array< MethodInfo ^ >( NUM_ENGINE_STRUCTURE_TYPES );
	m_ILGenCtx->MthIntrinsic_VMStackPopEngineStructure   = gcnew array< MethodInfo ^ >( NUM_ENGINE_STRUCTURE_TYPES );
	m_ILGenCtx->MthIntrinsic_CompareEngineStructure      = gcnew array< MethodInfo ^ >( NUM_ENGINE_STRUCTURE_TYPES );
	m_ILGenCtx->MthEngineStructure_DeleteEngineStructure = gcnew array< MethodInfo ^ >( NUM_ENGINE_STRUCTURE_TYPES );
	m_ILGenCtx->MthIntrinsic_CreateEngineStructure       = gcnew array< MethodInfo ^ >( NUM_ENGINE_STRUCTURE_TYPES );

	for (int i = 0; i < NUM_ENGINE_STRUCTURE_TYPES; i += 1)
	{
		Type       ^ EngineStructType;
		MethodInfo ^ Intrinsic_VMStackPush;
		MethodInfo ^ Intrinsic_VMStackPop;
		MethodInfo ^ Intrinsic_Compare;
		MethodInfo ^ Intrinsic_Delete;
		MethodInfo ^ Intrinsic_Create;

		EngineStructType = m_ILGenCtx->JITModule->GetType(
			"NWScript.NWScriptEngineStructure" + i,
			true,
			false);

		if (EngineStructType == nullptr)
			throw gcnew Exception( "Unable to retrieve engine structure type for structure " + i + "." );

		Intrinsic_VMStackPush = NWScriptJITIntrinsicsType->GetMethod(
			"Intrinsic_VMStackPushEngineStructure" + i
			);
		Intrinsic_VMStackPop  = NWScriptJITIntrinsicsType->GetMethod(
			"Intrinsic_VMStackPopEngineStructure" + i
			);
		Intrinsic_Compare     = NWScriptJITIntrinsicsType->GetMethod(
			"Intrinsic_CompareEngineStructure" + i
			);
		Intrinsic_Delete      = EngineStructType->GetMethod(
			"DeleteEngineStructure"
			);
		Intrinsic_Create      = NWScriptJITIntrinsicsType->GetMethod(
			"Intrinsic_CreateEngineStructure" + i
			);

		if (Intrinsic_VMStackPush == nullptr)
			throw gcnew Exception( "Unable to retrieve Intrinsic_VMStackPushEngineStructure" + i + "." );
		else if (Intrinsic_VMStackPop == nullptr)
			throw gcnew Exception( "Unable to retrieve Intrinsic_VMStackPopEngineStructure" + i + "." );
		else if (Intrinsic_Compare == nullptr)
			throw gcnew Exception( "Unable to retrieve Intrinsic_CompareEngineStructure" + i + "." );
		else if (Intrinsic_Delete == nullptr)
			throw gcnew Exception( "Unable to retrieve DeleteEngineStructure (" + i + ")." );
		else if (Intrinsic_Create == nullptr)
			throw gcnew Exception( "Unable to retrieve Intrinsic_CreateEngineStructure" + i + "." );

		m_ILGenCtx->EngineStructureTypes[ i ]                     = EngineStructType;
		m_ILGenCtx->MthIntrinsic_VMStackPushEngineStructure[ i ]  = Intrinsic_VMStackPush;
		m_ILGenCtx->MthIntrinsic_VMStackPopEngineStructure[ i ]   = Intrinsic_VMStackPop;
		m_ILGenCtx->MthIntrinsic_CompareEngineStructure[ i ]      = Intrinsic_Compare;
		m_ILGenCtx->MthEngineStructure_DeleteEngineStructure[ i ] = Intrinsic_Delete;
		m_ILGenCtx->MthIntrinsic_CreateEngineStructure[ i ]       = Intrinsic_Create;
	}

	//
	// Acquire the rest of the intrinsics.  Most intrinsics directly call the
	// INWScriptProgram interface for better performance now, except those that
	// really must go through the JIT intrinsics class due to actually
	// different calling conventions (engine structure types).
	//

	m_ILGenCtx->MthIntrinsic_VMStackPushInt             = m_HostInterfaceType->GetMethod( "Intrinsic_VMStackPushInt" );
	m_ILGenCtx->MthIntrinsic_VMStackPopInt              = m_HostInterfaceType->GetMethod( "Intrinsic_VMStackPopInt" );
	m_ILGenCtx->MthIntrinsic_VMStackPushFloat           = m_HostInterfaceType->GetMethod( "Intrinsic_VMStackPushFloat" );
	m_ILGenCtx->MthIntrinsic_VMStackPopFloat            = m_HostInterfaceType->GetMethod( "Intrinsic_VMStackPopFloat" );
	m_ILGenCtx->MthIntrinsic_VMStackPushString          = m_HostInterfaceType->GetMethod( "Intrinsic_VMStackPushString" );
	m_ILGenCtx->MthIntrinsic_VMStackPopString           = m_HostInterfaceType->GetMethod( "Intrinsic_VMStackPopString" );
	m_ILGenCtx->MthIntrinsic_VMStackPushObjectId        = m_HostInterfaceType->GetMethod( "Intrinsic_VMStackPushObjectId" );
	m_ILGenCtx->MthIntrinsic_VMStackPopObjectId         = m_HostInterfaceType->GetMethod( "Intrinsic_VMStackPopObjectId" );
	m_ILGenCtx->MthIntrinsic_ExecuteActionService       = m_HostInterfaceType->GetMethod( "Intrinsic_ExecuteActionService" );
	m_ILGenCtx->MthIntrinsic_StoreState                 = m_HostInterfaceType->GetMethod( "Intrinsic_StoreState" );
	m_ILGenCtx->MthIntrinsic_ExecuteActionServiceFast   = m_HostInterfaceType->GetMethod( "Intrinsic_ExecuteActionServiceFast" );
	m_ILGenCtx->MthIntrinsic_AllocateNeutralString      = m_HostInterfaceType->GetMethod( "Intrinsic_AllocateNeutralString" );
	m_ILGenCtx->MthIntrinsic_DeleteNeutralString        = m_HostInterfaceType->GetMethod( "Intrinsic_DeleteNeutralString" );
	m_ILGenCtx->MthIntrinsic_NeutralStringToString      = m_HostInterfaceType->GetMethod( "Intrinsic_NeutralStringToString" );
	m_ILGenCtx->MthIntrinsic_CheckScriptAbort           = m_HostInterfaceType->GetMethod( "Intrinsic_CheckScriptAbort" );
}

MethodInfo ^
NWScriptCodeGenerator::GenerateCloneScriptProgram(
	__in TypeBuilder ^ ProgramType,
	__in ConstructorBuilder ^ ProgCtor
	)
/*++

Routine Description:

	This routine constructs the 'CloneScriptProgram' subroutine on the script
	program.

Arguments:

	ProgramType - Supplies the type to add the method to.

	ProgCtor - Supplies the default initializer constructor to invoke for the
	           new ScriptProgram object.

Return Value:

	A handle to the MethodInfo describing the created method is returned.
	
	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	MethodBuilder ^ MthCloneScriptProgram;
	ILGenerator   ^ ILGen;
	LocalBuilder  ^ LocNewProg;
	int             NumGlobals;

	MthCloneScriptProgram = ProgramType->DefineMethod(
		"CloneScriptProgram",
		MethodAttributes::Public | MethodAttributes::Virtual,
		m_ScriptInterfaceType,
		Type::EmptyTypes);

	ILGen = MthCloneScriptProgram->GetILGenerator( );

	LocNewProg = ILGen->DeclareLocal( ProgramType );

	//
	// NewLoc = new ScriptProgram
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgram );
	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );

	ILGen->Emit( OpCodes::Newobj, ProgCtor );
	ILGen->Emit( OpCodes::Stloc, LocNewProg );

	//
	// Now copy all of the fields over.  First, do the fixed fields.
	//

	ILGen->Emit( OpCodes::Ldloc, LocNewProg );
	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldCurrentActionObjectSelf );
	ILGen->Emit( OpCodes::Stfld, m_ILGenCtx->FldCurrentActionObjectSelf );

	//
	// The JITIntrinsics instance has already been duplicated in the primary
	// constructor (m_ILGenCtx->FldProgram).  CallDepth and LoopCounter are
	// appropriately default initialized.
	//

	//
	// Now copy the globals.
	//

	if (m_ILGenCtx->Globals == nullptr)
		NumGlobals = 0;
	else
		NumGlobals = m_ILGenCtx->Globals->Length;

	for (int i = 0; i < NumGlobals; i += 1)
	{
		FieldInfo ^ FldGlobal;

		FldGlobal = m_ILGenCtx->Globals[ i ];

		ILGen->Emit( OpCodes::Ldloc, LocNewProg );
		ILGen->Emit( OpCodes::Ldarg_0 );
		ILGen->Emit( OpCodes::Ldfld, FldGlobal );
		ILGen->Emit( OpCodes::Stfld, FldGlobal );
	}

	//
	// Finally, return the cloned object.
	//

	ILGen->Emit( OpCodes::Ldloc, LocNewProg );
	ILGen->Emit( OpCodes::Ret );

	return MthCloneScriptProgram;
}

MethodInfo ^
NWScriptCodeGenerator::GenerateLoadScriptGlobals(
	__in TypeBuilder ^ ProgramType
	)
/*++

Routine Description:

	This routine constructs the 'LoadScriptGlobals' subroutine on the script
	program.

Arguments:

	ProgramType - Supplies the type to add the method to.

	ProgCtor - Supplies the default initializer constructor to invoke for the
	           new ScriptProgram object.

Return Value:

	A handle to the MethodInfo describing the created method is returned.
	
	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	PROGRAM_COUNTER            GlobalsPC;
	const NWScriptSubroutine * GlobalsSub;
	MethodBuilder            ^ MthLoadScriptGlobals;
	ILGenerator              ^ ILGen;
	int                        NumGlobals;
	int                        i;

	MthLoadScriptGlobals = ProgramType->DefineMethod(
		"LoadScriptGlobals",
		MethodAttributes::Public | MethodAttributes::Virtual,
		void::typeid,
		gcnew array< Type ^ >{ array< Object ^ >::typeid });

	ILGen = MthLoadScriptGlobals->GetILGenerator( );

	//
	// Create a dummy subroutine generation context for the unboxing generation
	// logic.
	//

	m_ILGenCtx->Sub = gcnew SubroutineGenContext;
	m_ILGenCtx->Sub->ILGen = ILGen;
	m_ILGenCtx->Sub->CurrentFlow = gcnew SubroutineControlFlow;
	m_ILGenCtx->Sub->CurrentFlow->LocalPool = gcnew FreeLocalMap;

	//
	// Now generate code to unpack the global array and copy it into the global
	// variables for the script.
	//
	// N.B.  The globals array is provided in order from the highest global to
	//       the lowest global, whereas the globals in the object (and the
	//       variable descriptor array) are ordered from the lowest global to
	//       the highest.  Hence, the argument array is designed to be
	//       traversed in reverse order.
	//

	if (m_ILGenCtx->Globals == nullptr)
		NumGlobals = 0;
	else
		NumGlobals = m_ILGenCtx->Globals->Length;

	if (NumGlobals != 0)
	{
		if ((GlobalsPC = m_ILGenCtx->Analyzer->GetGlobalsPC( )) == NWNScriptLib::INVALID_PC)
			throw gcnew Exception( "Script without #globals is declaring global variables" );

		if ((GlobalsSub = GetIRSubroutine( GlobalsPC )) == NULL)
			throw gcnew Exception( "Unable to locate NWScriptSubroutine for #globals" );

		i = 0;

		for (NWNScriptLib::VariablePtrVec::const_iterator it = GlobalsSub->GetLocals( ).begin( );
			 it != GlobalsSub->GetLocals( ).end( );
			 ++it)
		{
			NWScriptVariable * Var;

			//
			// Skip non-global variables in the frame of #globals.
			//

			Var     = it->get( )->GetHeadVariable( );

			if (Var->GetType( ) == ACTIONTYPE_VOID)
				continue;
			else if (Var->GetClass( ) != NWScriptVariable::Global)
				continue;
			else if (Var->GetType( ) == LASTACTIONTYPE)
				continue;

			//
			// Generate code to unpack this global from the argument array and
			// load it into the appropriate member variable field.
			//

			ILGen->Emit( OpCodes::Ldarg_1 );
			ILGen->Emit( OpCodes::Ldc_I4, (Int32) (NumGlobals - i) - 1 );
			ILGen->Emit( OpCodes::Ldelem_Ref );

			GenerateStoreVariable( Var, true );

			if ((int) Var->GetScope( ) != i)
				throw gcnew Exception( "Global variable index mismatch." );

			i += 1;
		}
	}

	ILGen->Emit( OpCodes::Ldnull );
	ILGen->Emit( OpCodes::Starg_S, (Byte) 1 );

	ILGen->Emit( OpCodes::Ret );

	return MthLoadScriptGlobals;
}

MethodInfo ^
NWScriptCodeGenerator::GenerateExecuteScript(
	__in TypeBuilder ^ ProgramType
	)
/*++

Routine Description:

	This routine constructs the 'ExecuteScript' subroutine on the script
	program.  This method conforms the the interface method
	IGeneratedScriptProgram::ExecuteScript.

Arguments:

	ProgramType - Supplies the type to add the method to.

Return Value:

	A handle to the MethodInfo describing the created method is returned.
	
	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	MethodBuilder      ^ MthExecuteScript;
	ILGenerator        ^ ILGen;
	NWScriptSubroutine * EntrySub;
	size_t               ParamCount;
	bool                 ReturnsInt;

	if (m_ILGenCtx->Analyzer->GetSubroutines( ).empty( ))
		throw gcnew Exception( "Script program has no entry point." );

	EntrySub = m_ILGenCtx->Analyzer->GetSubroutines( ).front( ).get( );

	//
	// N.B.  ACTIONTYPE_VOID returned if there were no return values.
	//

	ReturnsInt = EntrySub->GetFirstReturnType( ) == ACTIONTYPE_INT;
	ParamCount = EntrySub->GetParameters( ).size( );

	MthExecuteScript = ProgramType->DefineMethod(
		"ExecuteScript",
		MethodAttributes::Public | MethodAttributes::Virtual,
		Int32::typeid,
		gcnew array< Type ^ >{ UInt32::typeid, array< Object ^ >::typeid, Int32::typeid });

	ILGen = MthExecuteScript->GetILGenerator( );

	//
	// Create a dummy subroutine generation context for the unboxing generation
	// logic.
	//

	m_ILGenCtx->Sub = gcnew SubroutineGenContext;
	m_ILGenCtx->Sub->ILGen = ILGen;

	//
	// m_CurrentActionObjectSelf = ObjectSelf
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldarg_1 );
	ILGen->Emit( OpCodes::Stfld, m_ILGenCtx->FldCurrentActionObjectSelf );

	//
	// m_CallDepth = 0
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
#if NWSCRIPT_FAST_EXEC_GUARDS
	ILGen->Emit( OpCodes::Ldc_I4, -m_ILGenCtx->MaxCallDepth );
#else
	ILGen->Emit( OpCodes::Ldc_I4_0 );
#endif
	ILGen->Emit( OpCodes::Stfld, m_ILGenCtx->FldCallDepth );

	//
	// m_LoopCounter = 0
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
#if NWSCRIPT_FAST_EXEC_GUARDS
	ILGen->Emit( OpCodes::Ldc_I4, -m_ILGenCtx->MaxLoopIterations );
#else
	ILGen->Emit( OpCodes::Ldc_I4_0 );
#endif
	ILGen->Emit( OpCodes::Stfld, m_ILGenCtx->FldLoopCounter );

	//
	// Start us off with a call to #globals (if we had #globals).
	//

	if (m_ILGenCtx->MthGlobals != nullptr)
	{
		ILGen->Emit( OpCodes::Ldarg_0 );
		ILGen->Emit( OpCodes::Call, m_ILGenCtx->MthGlobals );

		//
		// The return value of #globals is thrown away.  It would be the actual
		// return value of the entry point in NWScript, but we do not structure
		// #globals to call into the entry point.
		//

		if (m_ILGenCtx->MthGlobals->ReturnType == Int32::typeid)
			ILGen->Emit( OpCodes::Pop );
		else if (m_ILGenCtx->MthGlobals->ReturnType != void::typeid)
			throw gcnew Exception( "#globals returns a type other than int32 or void which is illegal." );
	}

	//
	// Now generate code to unpack the array and turn it into subroutine parameters.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );

	for (size_t i = 0; i < ParamCount; i += 1)
	{
		NWScriptVariable * Var = &EntrySub->GetParameterVariable( i );

		ILGen->Emit( OpCodes::Ldarg_2 );
		ILGen->Emit( OpCodes::Ldc_I4, (Int32) i );
		ILGen->Emit( OpCodes::Ldelem_Ref );

		//
		// Now unbox the type if necessary.
		//

		GenerateUnbox( Var->GetType( ) );
	}

	ILGen->Emit( OpCodes::Ldnull );
	ILGen->Emit( OpCodes::Starg_S, (Byte) 2 );

	//
	// Now call the actual entry point symbol.
	//

	if (ReturnsInt)
		ILGen->Emit( OpCodes::Tailcall ); // Can tail call if we return int.

	ILGen->Emit( OpCodes::Call, m_ILGenCtx->MthNWScriptEntryPoint );

	//
	// If we did not return int, we need to push the dummy return code onto the
	// stack.
	//

	if (!ReturnsInt)
		ILGen->Emit( OpCodes::Ldarg_3 );

	ILGen->Emit( OpCodes::Ret );

	return MthExecuteScript;
}

MethodInfo ^
NWScriptCodeGenerator::GenerateExecuteScriptSituation(
	__in TypeBuilder ^ ProgramType
	)
/*++

Routine Description:

	This routine constructs the 'ExecuteScriptSituation' subroutine on the
	script program.  This method conforms the the interface method
	IGeneratedScriptProgram::ExecuteScriptSituation.

Arguments:

	ProgramType - Supplies the type to add the method to.

Return Value:

	A handle to the MethodInfo describing the created method is returned.
	
	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	MethodBuilder  ^ MthExecuteScriptSituation;
	ILGenerator    ^ ILGen;
	array< Label > ^ JumpTable;
	Label            DefaultCase;

	MthExecuteScriptSituation = ProgramType->DefineMethod(
		"ExecuteScriptSituation",
		MethodAttributes::Public | MethodAttributes::Virtual,
		void::typeid,
		gcnew array< Type ^ >{ UInt32::typeid, array< Object ^ >::typeid, UInt32::typeid });

	ILGen = MthExecuteScriptSituation->GetILGenerator( );

	//
	// Create a dummy subroutine generation context for the unboxing generation
	// logic.
	//

	m_ILGenCtx->Sub = gcnew SubroutineGenContext;
	m_ILGenCtx->Sub->ILGen = ILGen;

	//
	// Create the switch over the resume method id.
	//

	if (m_ILGenCtx->ResumeMethods.Count != 0)
	{
		DefaultCase = ILGen->DefineLabel( );
		JumpTable   = gcnew array< Label >( m_ILGenCtx->ResumeMethods.Count );

		for (UInt32 i = 0; i < (UInt32) JumpTable->Length; i += 1)
			JumpTable[ i ] = ILGen->DefineLabel( );

		//
		// Update m_CurrentActionObjectSelf to point to the new OBJECT_SELF.
		//

		ILGen->Emit( OpCodes::Ldarg_0 );
		ILGen->Emit( OpCodes::Ldarg_3 );
		ILGen->Emit( OpCodes::Stfld, m_ILGenCtx->FldCurrentActionObjectSelf );

		//
		// Generate the switch on resume method id.
		//

		ILGen->Emit( OpCodes::Ldarg_1 );
		ILGen->Emit( OpCodes::Switch, JumpTable );
		ILGen->Emit( OpCodes::Br, DefaultCase );

		//
		// Now emit the case blocks.
		//

		for (UInt32 i = 0; i < (UInt32) JumpTable->Length; i += 1)
		{
			SubroutineAttributes ^ Attributes;
			NWScriptSubroutine   * Sub;
			size_t                 ParamCount;

			Attributes = m_ILGenCtx->ResumeMethods[ i ];
			Sub        = Attributes->IRSub;
			ParamCount = Sub->GetParameters( ).size( );

			ILGen->MarkLabel( JumpTable[ i ] );
			ILGen->Emit( OpCodes::Ldarg_0 );

			//
			// Now generate code to unpack the parameter array and turn it into
			// subroutine parameters.
			//

			for (size_t j = 0; j < ParamCount; j += 1)
			{
				NWScriptVariable * Var = &Sub->GetParameterVariable( j );

				ILGen->Emit( OpCodes::Ldarg_2 );
				ILGen->Emit( OpCodes::Ldc_I4, (Int32) j );
				ILGen->Emit( OpCodes::Ldelem_Ref );

				//
				// Now unbox the type if necessary.
				//

				GenerateUnbox( Var->GetType( ) );
			}

			//
			// Finally, call the entry point and branch to the default case
			// afterwards.
			//

			ILGen->Emit( OpCodes::Ldnull );
			ILGen->Emit( OpCodes::Starg_S, (Byte) 2 );
			ILGen->Emit( OpCodes::Tailcall );
			ILGen->Emit( OpCodes::Call, Attributes->Method );
			ILGen->Emit( OpCodes::Ret );
		}

		//
		// Finally, mark the default case label.
		//

		ILGen->MarkLabel( DefaultCase );
	}

	//
	// All done.
	//

	GenerateThrowException( "Illegal resume method id." );

	ILGen->Emit( OpCodes::Ret );

	return MthExecuteScriptSituation;
}

PropertyBuilder ^
NWScriptCodeGenerator::GenerateReadOnlyProperty(
	__in TypeBuilder ^ ParentType,
	__in FieldBuilder ^ BackingField,
	__in String ^ PropertyName
	)
/*++

Routine Description:

	This routine constructs a read-only property attached to a given type.  The
	property returns the value of a given field as the 'get' accessor's return
	value.

Arguments:

	ParentType - Supplies the type to add the property to.

	BackingField - Supplies the field that the property should read.

	PropertyName - Supplies the name of the property.

Return Value:

	A handle to the PropertyBuilder describing the created property is
	returned.
	
	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	PropertyBuilder ^ Property;
	MethodBuilder   ^ Method;
	ILGenerator     ^ ILGen;

	Property = ParentType->DefineProperty(
		PropertyName,
		PropertyAttributes::None,
		BackingField->FieldType,
		nullptr);

	Method = ParentType->DefineMethod(
		"get_" + PropertyName,
		MethodAttributes::Public      |
		MethodAttributes::SpecialName |
		MethodAttributes::HideBySig,
		BackingField->FieldType,
		Type::EmptyTypes);

	ILGen = Method->GetILGenerator( );

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldfld, BackingField );
	ILGen->Emit( OpCodes::Ret );

	Property->SetGetMethod( Method );

	return Property;
}

FieldBuilder ^
NWScriptCodeGenerator::GenerateLiteralField(
	__in TypeBuilder ^ ParentType,
	__in String ^ FieldName,
	__in Object ^ FieldLiteralValue
	)
/*++

Routine Description:

	This routine constructs a literal (compile-time constant) field that is
	attached to a given type.

Arguments:

	ParentType - Supplies the type to add the field to.

	FieldName - Supplies the name of the field.

	FieldLiteralValue - Supplies the literal constant value of the field.

Return Value:

	A handle to the FieldBuilder describing the created field is returned.
	
	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	FieldBuilder ^ Field;

	Field = ParentType->DefineField(
		FieldName,
		FieldLiteralValue->GetType( ),
		FieldAttributes::Public | FieldAttributes::Literal | FieldAttributes::Static);

	Field->SetConstant( FieldLiteralValue );

	return Field;
}

void
NWScriptCodeGenerator::GenerateGlobals(
	__in TypeBuilder ^ ProgramType
	)
/*++

Routine Description:

	This routine generates declarations for all global variables used by the
	script program.

Arguments:

	None.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	PROGRAM_COUNTER            GlobalsPC;
	const NWScriptSubroutine * GlobalsSub;
	Int32                      i;
	Int32                      NumGlobals;

	if ((GlobalsPC = m_ILGenCtx->Analyzer->GetGlobalsPC( )) == NWNScriptLib::INVALID_PC)
	{
		m_ILGenCtx->Globals = nullptr;
		return;
	}

	if ((GlobalsSub = GetIRSubroutine( GlobalsPC )) == NULL)
		throw gcnew Exception( "Unable to locate NWScriptSubroutine for #globals" );

	NumGlobals = 0;

	for (NWNScriptLib::VariablePtrVec::const_iterator it = GlobalsSub->GetLocals( ).begin( );
	     it != GlobalsSub->GetLocals( ).end( );
	     ++it)
	{
		NWScriptVariable * Var;

		Var = it->get( )->GetHeadVariable( );

		if (Var->GetType( ) == ACTIONTYPE_VOID)
			continue;
		else if (Var->GetClass( ) != NWScriptVariable::Global)
			continue;
		else if (Var->GetType( ) == LASTACTIONTYPE)
			continue;

		if (NumGlobals == LONG_MAX)
			throw gcnew Exception( "Too many global variables." );

		NumGlobals += 1;
	}

	//
	// Each global variable is stored as a field (member variable) on the
	// program class object.  Here, we iterate through each global in the IR,
	// emitting fields for that corresponding global and linking said fields
	// into the variable table.
	//
	// The "Scope" field in a NWScriptVariable is repurposed as a contextural
	// value to mean the variable index within its corresponding variable
	// table.
	//

	if (NumGlobals == 0)
	{
		m_ILGenCtx->Globals = nullptr;
		return;
	}

	m_ILGenCtx->Globals = gcnew GlobalVariableTable(
		NumGlobals );

	i = 0;

	for (NWNScriptLib::VariablePtrVec::const_iterator it = GlobalsSub->GetLocals( ).begin( );
	     it != GlobalsSub->GetLocals( ).end( );
	     ++it)
	{
		NWScriptVariable * Var;
		Type             ^ VarType;
		FieldBuilder     ^ Field;

		//
		// Skip non-global variables in the frame of #globals.
		//

		Var     = it->get( )->GetHeadVariable( );

		if (Var->GetType( ) == ACTIONTYPE_VOID)
			continue;
		else if (Var->GetClass( ) != NWScriptVariable::Global)
			continue;
		else if (Var->GetType( ) == LASTACTIONTYPE)
			continue;

		VarType = GetVariableType( Var );

		//
		// Generate a member variable field for this referenced global.
		//

		Field = ProgramType->DefineField(
			"m__NWScriptGlobal" + i,
			VarType,
			FieldAttributes::Private);

		m_ILGenCtx->Globals[ i ] = Field;

		Var->SetScope( i );

		i += 1;

		if (IsDebugLevel( NWScriptVM::EDL_Verbose ))
		{
			m_TextOut->WriteText(
				"[Sub=%s(%08X)] Create global %p (#%lu) as type %lu.\n",
				GlobalsSub->GetSymbolName( ).c_str( ),
				GlobalsSub->GetAddress( ),
				Var,
				i - 1,
				Var->GetType( ));
		}
	}
}

void
NWScriptCodeGenerator::GenerateAllSubroutines(
	)
/*++

Routine Description:

	This routine emits code for each subroutine in the program.

Arguments:

	None.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	const NWScriptAnalyzer * Analyzer;
	PROGRAM_COUNTER          LoaderPC;
	PROGRAM_COUNTER          GlobalsPC;
	bool                     First;
	SubroutineGenContext   ^ SubCtx;
	TypeBuilder            ^ ProgramType;

	Analyzer    = m_ILGenCtx->Analyzer;
	LoaderPC    = Analyzer->GetLoaderPC( );
	GlobalsPC   = Analyzer->GetGlobalsPC( );
	First       = true;
	ProgramType = m_ILGenCtx->ProgramType;

	if (Analyzer->GetSubroutines( ).empty( ))
		throw gcnew Exception( "Script program has no subroutines." );

	m_ILGenCtx->EntryPC = Analyzer->GetSubroutines( ).front( )->GetAddress( );

	//
	// First, construct a prototype method for each subroutine (and insert each
	// prototype into the mapping table).
	//

	for (NWNScriptLib::SubroutinePtrVec::const_iterator it = Analyzer->GetSubroutines( ).begin( );
	     it != Analyzer->GetSubroutines( ).end( );
	     ++it)
	{
		NWScriptSubroutine       * IRSub;
		unsigned long              Flags;
		MethodBuilder            ^ Method;
		Type                     ^ ReturnType;
		array< Type ^ >          ^ ParameterTypes;
		String                   ^ RoutineName;
		MethodAttributes           Attributes;
		MethodAttributes           DefSubroutineAccess;
		SubroutineAttributes     ^ SubAttributes;

#if NWSCRIPT_PUBLIC_SUBROUTINES
		DefSubroutineAccess = MethodAttributes::Public;
#else
		DefSubroutineAccess = MethodAttributes::Private;
#endif

		IRSub = it->get( );
		Flags = 0;

		//
		// Discover any special properties of this subroutine (such as whether
		// it is the entry point, #loader, or #globals).
		//

		if (First)
		{
			Flags |= (unsigned long) GSUB_FLAGS::GSF_ENTRY_POINT;
			First  = false;
		}
		else if (IRSub->GetAddress( ) == LoaderPC)
		{
			//
			// No code is actually emitted for the #loader subroutine.
			//

			continue;
		}
		else if (IRSub->GetAddress( ) == GlobalsPC)
		{
			Flags |= (unsigned long) GSUB_FLAGS::GSF_GLOBALS;
		}
		else
		{
			if (IRSub->GetFlags( ) & NWScriptSubroutine::SCRIPT_SITUATION)
				Flags |= (unsigned long) GSUB_FLAGS::GSF_SCRIPT_SITUATION;
		}

		//
		// Generate the prototype for the method.
		//

		ReturnType = GenerateSubroutineRetType( IRSub );

		ParameterTypes = gcnew array< Type ^ >( (int) IRSub->GetParameters( ).size( ) );

		for (size_t i = 0; i < IRSub->GetParameters( ).size( ); i += 1)
		{
			Type ^ T = GetVariableType( &IRSub->GetParameterVariable( i ) );

			//
			// TODO: Eliminate unreferenced parameters (instead of turning them
			//       into Int32's).
			//

			ParameterTypes[ (int) i ] = T;
		}

		//
		// Create the underlying method now.
		//

		if (Flags & (unsigned long) GSUB_FLAGS::GSF_ENTRY_POINT)
		{
			RoutineName  = "NWScriptEntryPoint";
			Attributes   = DefSubroutineAccess;
		}
		else if (Flags & (unsigned long) GSUB_FLAGS::GSF_GLOBALS)
		{
			RoutineName  = "NWScriptInitGlobals";
			Attributes   = DefSubroutineAccess;
		}
		else
		{
			if (Flags & (unsigned long) GSUB_FLAGS::GSF_SCRIPT_SITUATION)
			{
				Attributes  = MethodAttributes::Private;
				RoutineName = "NWScriptSituationResume_";
			}
			else
			{
				Attributes  = DefSubroutineAccess;
				RoutineName = "NWScriptSubroutine_";
			}

			if (IRSub->GetSymbolName( ).empty( ))
				RoutineName += IRSub->GetAddress( );
			else
			{
				RoutineName += gcnew String(
					IRSub->GetSymbolName( ).data( ),
					0,
					(int) IRSub->GetSymbolName( ).size( ),
					m_ILGenCtx->StringEncoding);
			}
		}

		Method = ProgramType->DefineMethod(
			RoutineName,
			Attributes,
			ReturnType,
			ParameterTypes);

		//
		// Record any state about the subroutine that we'll need to refer to
		// later, such as its script situation id.
		//

		SubAttributes = gcnew SubroutineAttributes;

		SubAttributes->Method = Method;
		SubAttributes->IRSub  = IRSub;

		if (Flags & (unsigned long) GSUB_FLAGS::GSF_SCRIPT_SITUATION)
		{
			SubAttributes->ResumeMethodId = m_ILGenCtx->NextResumeMethodId++;

			m_ILGenCtx->ResumeMethods.Add( SubAttributes );
		}
		else
		{
			SubAttributes->ResumeMethodId = UInt32::MaxValue;
		}

		m_ILGenCtx->MethodMap->Add( IRSub->GetAddress( ), SubAttributes );

		if (Flags & (unsigned long) GSUB_FLAGS::GSF_ENTRY_POINT)
			m_ILGenCtx->MthNWScriptEntryPoint = Method;
		else if (Flags & (unsigned long) GSUB_FLAGS::GSF_GLOBALS)
			m_ILGenCtx->MthGlobals = Method;
	}

	//
	// Now that we have prototyped each method, generate code for all methods
	// in turn, in a single pass.
	//

	First = true;

	for (NWNScriptLib::SubroutinePtrVec::const_iterator it = Analyzer->GetSubroutines( ).begin( );
	     it != Analyzer->GetSubroutines( ).end( );
	     ++it)
	{
		NWScriptSubroutine       * IRSub;
		unsigned long              Flags;
		MethodBuilder            ^ Method;

		IRSub = it->get( );

		Flags = 0;

		//
		// Discover any special properties of this subroutine (such as whether
		// it is the entry point, #loader, or #globals).
		//

		if (First)
		{
			Flags |= (unsigned long) GSUB_FLAGS::GSF_ENTRY_POINT;
			First  = false;
		}
		else if (IRSub->GetAddress( ) == LoaderPC)
		{
			//
			// No code is actually emitted for the #loader subroutine.
			//

			continue;
		}
		else if (IRSub->GetAddress( ) == GlobalsPC)
		{
			Flags |= (unsigned long) GSUB_FLAGS::GSF_GLOBALS;
		}
		else
		{
			if (IRSub->GetFlags( ) & NWScriptSubroutine::SCRIPT_SITUATION)
				Flags |= (unsigned long) GSUB_FLAGS::GSF_SCRIPT_SITUATION;
		}

		//
		// Finally, emit the MSIL code for this subroutine.
		//

		SubCtx = gcnew SubroutineGenContext;

		Method = GetMSILSubroutine( IRSub->GetAddress( ) );

		SubCtx->IRSub         = IRSub;
		SubCtx->IRInstruction = -1;
		SubCtx->IRFlow        = NULL;
		SubCtx->MSILSub       = Method;
		SubCtx->Flags         = Flags;
		SubCtx->ILGen         = Method->GetILGenerator( );
		SubCtx->Flows         = gcnew ControlFlowMap;
		SubCtx->CurrentFlow   = nullptr;
		SubCtx->FlowsToEmit   = gcnew ControlFlowStack;

		//
		// Prepare the control flow map for flow merging.
		//

		BuildControlFlowMap( SubCtx );

		m_ILGenCtx->Sub = SubCtx;

		//
		// Generate the MSIL.
		//

		GenerateSubroutineCode( SubCtx );
	}

	//
	// Now generate the ExecuteScript interface method.
	//

	GenerateExecuteScript( m_ILGenCtx->ProgramType );

	//
	// Now generate the ExecuteScriptSituation interface method, which is used
	// to dispatch script situations (saved state) without relying on a slow
	// Reflection-based invoke call.
	//

	GenerateExecuteScriptSituation( m_ILGenCtx->ProgramType );
}

void
NWScriptCodeGenerator::GenerateInterfaceLayerActionServiceSubroutines(
	)
/*++

Routine Description:

	This routine emits code for each action service routine associated with the
	interface layer.

Arguments:

	None.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	PCNWACTION_DEFINITION Action;
	NWSCRIPT_ACTION       ActionServiceNumber;
	size_t                PassedParams;
	bool                  FastAction;

	//
	// Generate action service call stubs for each action service.
	//

	for (ActionServiceNumber = 0;
	     ActionServiceNumber < m_ActionCount;
	     ActionServiceNumber += 1)
	{
		Action = &m_ActionDefs[ ActionServiceNumber ];

		//
		// Determine if we can make a fast call.  A fast call must not involve
		// any engine structure types.
		//

		PassedParams = 0;
		FastAction   = true;

		for (size_t i = 0; i < Action->NumParameters; i += 1)
		{
			NWACTION_TYPE VarType = Action->ParameterTypes[ i ];

			if ((VarType >= ACTIONTYPE_ENGINE_0) &&
			    (VarType <= ACTIONTYPE_ENGINE_9))
			{
				//
				// We have encountered an engine structure type, emit a slow
				// call.
				//

				FastAction = false;
			}

			//
			// Count the number of actual parameters that will be passed to the
			// action on the stack.  This is a count of stack slots.
			//

			switch (VarType)
			{

			case ACTIONTYPE_VECTOR:
				PassedParams += 3;
				break;

			case ACTIONTYPE_ACTION:
				PassedParams += 0;
				break;

			default:
				PassedParams += 1;
				break;

			}
		}

		if ((Action->ReturnType >= ACTIONTYPE_ENGINE_0) &&
		    (Action->ReturnType <= ACTIONTYPE_ENGINE_9))
		{
			//
			// We have encountered an engine structure type, emit a slow
			// call.
			//

			FastAction = false;
		}

#if NWSCRIPT_DIRECT_FAST_ACTION_CALLS

		if (FastAction)
		{
			AcquireFastDirectActionServiceStub(
				Action,
				PassedParams,
				Action->NumParameters);

			continue;
		}

#endif

		AcquireSlowActionServiceStub(
			Action,
			PassedParams,
			Action->NumParameters);
	}
}

void
NWScriptCodeGenerator::GenerateSubroutineCode(
	__in SubroutineGenContext ^ Sub
	)
/*++

Routine Description:

	This routine emits MSIL code for a single subroutine.

Arguments:

	Sub - Supplies the subroutine context for the subroutine to emit the code
	      for.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	SubroutineControlFlow ^ Flow;
	ILGenerator           ^ ILGen;
	ControlFlowStack      ^ FlowsToEmit;
	int                     Instrs;

	if (Sub->FlowsToEmit->Count != 0)
	{
		FlowsToEmit      = Sub->FlowsToEmit;
		Flow             = FlowsToEmit->Peek( );
		Sub->CurrentFlow = Flow;

		//
		// First, annotate special variables (such as return values and
		// parameters).  We must assign them slots now because we will not see
		// a series of creation or deletion events for these.
		//

		AnnotateSpecialVariables( Sub );

		//
		// Now generate the prolog for the subroutine.
		//

		GenerateProlog( );
	}

	//
	// Set up the initial flow.
	//

	ILGen = Sub->ILGen;

	while (FlowsToEmit->Count != 0)
	{
		Flow = FlowsToEmit->Pop( );

		//
		// If we are starting a new flow, mark the label and set it as under
		// processing.
		//

		Flow->Enqueued = false;

		if (!Flow->Processed)
			Flow->Processed = true;
		else
			throw gcnew Exception( "Emitting code for control flow twice." );

		Sub->CurrentFlow = Flow;

		//
		// Create the label for this flow as we are about to emit code for it.
		//

		ILGen->MarkLabel( Flow->MSILLabel );

		Instrs = Flow->Instructions != nullptr ? Flow->Instructions->Length : 0;

		for (Sub->IRInstruction = 0;
		     Sub->IRInstruction < Instrs;
		     Sub->IRInstruction += 1)
		{
			//
			// Translate the IR instruction into its MSIL equivalents.
			//

			GenerateMSILForIRInstruction(
				Sub,
				Flow,
				*Flow->Instructions[ Sub->IRInstruction ] );
		}

		//
		// Now enqueue any available children to the generation queue.
		//

		EnqueueChildFlows( Sub, Flow, Flow->IRFlow );

		//
		// If we have reached the end of the flow, then emit a jump to the next
		// flow section (should we have ended in a fall-through (merge) state).
		//

		switch (Flow->IRFlow->GetTerminationType( ))
		{

			//
			// This control flow exited the subroutine, we are guaranteed to
			// not need a jump as there can be no fall through.
			//

		case NWScriptControlFlow::Terminate:
			break;

			//
			// This control flow merged to another, we may need a jump.
			//

		case NWScriptControlFlow::Merge:
			if (Flow->IRFlow->GetChild( 0 )->GetStartPC( ) < Flow->IRFlow->GetEndPC( ))
				GenerateLoopCheck( );

			ILGen->Emit(
				OpCodes::Br,
				GetLabel( Flow->IRFlow->GetChild( 0 ).get( ) )); // Fallthrough
			break;

			//
			// The control flow is an unconditional jump, we must emit a jump
			// to the target.
			//

		case NWScriptControlFlow::Transfer:
			if (Flow->IRFlow->GetChild( 0 )->GetStartPC( ) < Flow->IRFlow->GetEndPC( ))
				GenerateLoopCheck( );

			ILGen->Emit(
				OpCodes::Br,
				GetLabel( Flow->IRFlow->GetChild( 0 ).get( ) )); // Target
			break;

			//
			// This control flow split into several, we need to jump to the
			// fallthrough label.
			//

		case NWScriptControlFlow::Split:         // Fallthrough intended
			if (Flow->IRFlow->GetChild( 0 )->GetStartPC( ) < Flow->IRFlow->GetEndPC( ))
				GenerateLoopCheck( );

			ILGen->Emit(
				OpCodes::Br,
				GetLabel( Flow->IRFlow->GetChild( 1 ).get( ) )); // Fallthrough
			break;

		default:
			throw gcnew Exception( "Unsupported control flow termination type." );
			break;

		}
	}
}

void
NWScriptCodeGenerator::AnnotateSpecialVariables(
	__in SubroutineGenContext ^ Sub
	)
/*++

Routine Description:

	This routine annotates special variables (those aside from globals which
	are not created by I_CREATE instructions) with their corresponding MSIL
	location data.

	Because special variables are assumed to exist up front, we must create
	them now instead of on demand.

Arguments:

	Sub - Supplies the subroutine context for the subroutine to annotate
	      special variables for.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	NWScriptSubroutine * IRSub;
	size_t               ParamCount;
	size_t               ReturnCount;

	IRSub       = Sub->IRSub;
	ParamCount  = IRSub->GetParameters( ).size( );
	ReturnCount = IRSub->GetNumReturnTypes( );

	//
	// First, annotate all parameters to the subroutine.  We store their MSIL
	// argument slot in the Scope field.  (Argument slot 0 is the 'this'
	// pointer.)
	//

	for (size_t i = 0; i < ParamCount; i += 1)
	{
		NWScriptVariable * Var = &IRSub->GetParameterVariable( i );

		Var = Var->GetHeadVariable( );

		if (Var->GetClass( ) != NWScriptVariable::Parameter)
			throw gcnew Exception( "Incorrectly classified subroutine parameter detected." );
		else if (Var->GetScope( ) != NWNScriptLib::INVALID_SCOPE)
			throw gcnew Exception( "Aliased subroutine parameter detected." );

		Var->SetScope( (NWNScriptLib::SCOPE_ID) (i + 1) );
	}

	//
	// Now annotate the return values.  For these, we simply create local
	// variables up front.
	//

	for (size_t i = 0; i < ReturnCount; i += 1)
	{
		NWScriptVariable * Var = &IRSub->GetReturnValueVariable( i );
		LocalBuilder     ^ Loc;

		Var = Var->GetHeadVariable( );

		if (Var->GetClass( ) != NWScriptVariable::ReturnValue)
			throw gcnew Exception( "Incorrectly classified subroutine return value detected." );
		else if (Var->GetScope( ) != NWNScriptLib::INVALID_SCOPE)
			throw gcnew Exception( "Aliased subroutine return value detected." );

		Loc = AcquireLocal( GetVariableType( Var ) );

		Var->SetScope( Sub->CurrentFlow->VarTable->Count );
		Sub->CurrentFlow->VarTable->Add( Loc );

		//
		// There should be no need to assign a default value to a return value
		// unless we return a value of type engine structure and we create a
		// script situation.  In this case, it's legitimately possible that the
		// script program might bequeath the uninitialized return value to the
		// script situation, and save to stack / restore from stack require
		// that all engine structure types not be NULL (for their types to be
		// discoverable at runtime).  Similarly, all string types must not be
		// NULL.
		// 

		if ((IRSub->GetFlags( ) & NWScriptSubroutine::SAVES_STATE))
		{
			if ((Var->GetType( ) >= ACTIONTYPE_ENGINE_0) &&
			    (Var->GetType( ) <= ACTIONTYPE_ENGINE_9))
			{
				GenerateSetDefaultValue( Loc );
			}
			else if (Var->GetType( ) == ACTIONTYPE_STRING)
			{
				GenerateSetDefaultValue( Loc );
			}
		}
	}

	//
	// The following is a workaround for a limitation in the frontend, where we
	// may issue duplicate I_CREATEs for the same variable in a non-exclusive
	// flow chain.  This can occur in cases where a variable is merged across
	// two flows, but is conditionally deleted and recreated in one of the flow
	// forks.
	//
	// When the frontend removes the extra I_CREATEs, this logic should be
	// removed.
	//

	for (NWNScriptLib::VariablePtrVec::iterator it = IRSub->GetLocals( ).begin( );
	     it != IRSub->GetLocals( ).end( );
	     ++it)
	{
		NWScriptVariable * Var = it->get( );
		LocalBuilder     ^ Loc;

		//
		// Pre-create any multiply created variables up front to work around
		// the issue described above.
		//

		if (Var->GetClass( ) != NWScriptVariable::Local)
			continue;
		else if (!(Var->GetFlags( ) & NWScriptVariable::MultiplyCreated))
			continue;

		Loc = CreateLocal( Var );
	}
}

void
NWScriptCodeGenerator::GenerateMSILForIRInstruction(
	__in SubroutineGenContext ^ Sub,
	__in SubroutineControlFlow ^ Flow,
	__in NWScriptInstruction & IRInstr
	)
/*++

Routine Description:

	This routine emits MSIL code for a single IR instruction (which may
	correspond to several MSIL instructions, or an intrinsic invocation).

	Note that there is not necessarily a one-to-one correspondence between IR
	instructions and NWScript virtual machine instruction codes.  For example,
	OP_JZ/OP_JNZ yield a combination of I_TEST and I_JZ/I_JNZ instructions.

Arguments:

	Sub - Supplies the subroutine context for the subroutine to emit the code
	      for.

	Flow - Supplies the control flow containing the current IR instruction.

	IRInstr - Supplies the high-level IR instruction to translate into MSIL.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	NWNScriptLib::VariableWeakPtrVec   ReadVars;
	NWNScriptLib::VariableWeakPtrVec   WriteVars;

	UNREFERENCED_PARAMETER( Flow );

	if (IsDebugLevel( NWScriptVM::EDL_Verbose ))
	{
		m_TextOut->WriteText(
			"[Sub=%s(%08X)] Instr%03lu=%s [@PC=%08X]\n",
			Sub->IRSub->GetSymbolName( ).c_str( ),
			Sub->IRSub->GetAddress( ),
			Sub->IRInstruction,
			GetIRInstructionName( IRInstr.GetType( ) ),
			IRInstr.GetAddress( ));
	}

	m_ILGenCtx->Analyzer->GetInstructionVariableLists(
		IRInstr,
		&ReadVars,
		&WriteVars);

	switch (IRInstr.GetType( ))
	{

	case NWScriptInstruction::I_CREATE:     // Create variable with type (no value)
		{
			NWScriptVariable * Var;
			LocalBuilder     ^ Loc;

			Var = IRInstr.GetVar( 0 )->GetHeadVariable( );

			//
			// If we are creating a variable that is later marked as a global,
			// then do nothing.  We do all manipulation through the instance
			// field in this case.  This is expected to occur in #globals,
			// where some of the local variables are aliased to the globals of
			// the program.
			//

			if (Var->GetClass( ) == NWScriptVariable::Global)
			{
				if (!(Sub->Flags & (unsigned long) GSUB_FLAGS::GSF_GLOBALS))
					throw gcnew Exception( "Creating global variable outside of #globals." );

				//
				// Globals are not actually instantiated as local slots, so we
				// have nothing to do here.
				//

				break;
			}
			else if (Var->GetClass( ) == NWScriptVariable::Constant)
			{
				//
				// Constants are never actually instantiated, so we can ignore them.
				//

				break;
			}
			else if ((Var->GetClass( ) != NWScriptVariable::Local)            &&
			         (Var->GetClass( ) != NWScriptVariable::CallParameter)    &&
			         (Var->GetClass( ) != NWScriptVariable::CallReturnValue))
			{
				//
				// This variable class is not actually a local (for example, a
				// parameter to the current subroutine).  We should not see a
				// normal create for it.
				//

				throw gcnew Exception( "Creating variable of illegal class with I_CREATE." );
			}

			//
			// If we have already allocated this variable, then it must be one
			// that is created across multiple flows -- otherwise there is a
			// bug.
			//

			if ((Var->GetScope( ) != NWNScriptLib::INVALID_SCOPE) &&
			    (!(Var->GetFlags( ) & NWScriptVariable::MultiplyCreated)))
			{
				throw gcnew Exception( "Discovered multiple creations for variable that is marked as no multiple creations." );
			}

			Loc = CreateLocal( Var );
		}
		break;

	case NWScriptInstruction::I_DELETE:     // Delete variable
		{
			NWScriptVariable * Var;
			LocalBuilder     ^ Loc;

			Var = IRInstr.GetVar( 0 )->GetHeadVariable( );

			//
			// If we are deleting a variable that is later marked as global,
			// then do nothing.  We do all manipulation through the instance
			// field in this case.  This is expected to occur in #globals,
			// where some of the local variables are aliased to the globals of
			// the program.
			//

			if (Var->GetClass( ) == NWScriptVariable::Global)
			{
				if (!(Sub->Flags & (unsigned long) GSUB_FLAGS::GSF_GLOBALS))
					throw gcnew Exception( "Deleting global variable outside of #globals." );

				break;
			}
			else if ((Var->GetClass( ) == NWScriptVariable::Constant) ||
			         (Var->GetClass( ) == NWScriptVariable::Parameter))
			{
				break;
			}
			else if ((Var->GetClass( ) != NWScriptVariable::Local)            &&
			         (Var->GetClass( ) != NWScriptVariable::CallParameter)    &&
			         (Var->GetClass( ) != NWScriptVariable::CallReturnValue))
			{
				throw gcnew Exception( "Deleting variable of illegal class with I_DELETE." );
			}

			//
			// Return the local back to the pool for future reuse, so that we
			// can keep the number of locals due to temporaries down to a
			// manageable number.
			//
			// Note that we can only release a local that is not created in
			// more than one flow.  Otherwise, a problematic scenario may
			// arise where we have two flow forks that create the same variable
			// and merge, but which assign a particular local to multiple
			// different variables (as the merged variable's local is fixed the
			// first time we encounter it, but that particular local remains
			// available for use in the pool for a (different) variable to pick
			// up in the alternate fork).
			//
			// TODO:  Currently, the variable is copied each time we follow a
			//        new control flow, which reduces complexity but causes a
			//        reduction in sharing should there be a large number of
			//        parallel control flows (i.e. in a switch statement).  We
			//        should investigate managing the pool in a shared fashion
			//        to conserve locals in these scenarios.
			//

			if (!(Var->GetFlags( ) & NWScriptVariable::MultiplyCreated))
			{
				Loc = GetLocalVariable( Sub, Var );

				ReleaseLocal( Loc );
			}
		}
		break;

	case NWScriptInstruction::I_ASSIGN:     // Copy (assign) variables
		{
			for (size_t i = 0; i < ReadVars.size( ); i += 1)
			{
				GenerateLoadVariable( ReadVars[ i ] );
				GenerateStoreVariable( WriteVars[ i ] );
			}
		}
		break;

	case NWScriptInstruction::I_JZ:         // Jump if zero
		{
			//
			// Emit the conditional jump.  Note that the argument has been cast
			// to a Boolean value and may take only logic 1 or logic 0 values.
			//

			Sub->ILGen->Emit(
				OpCodes::Brfalse,
				GetLabel( IRInstr.GetJumpTarget( ) ) );
		}
		break;

	case NWScriptInstruction::I_JNZ:        // Jump if not zero
		{
			//
			// Emit the conditional jump.  Note that the argument has been cast
			// to a Boolean value and may take only logic 1 or logic 0 values.
			//

			Sub->ILGen->Emit(
				OpCodes::Brtrue,
				GetLabel( IRInstr.GetJumpTarget( ) ) );
		}
		break;

	case NWScriptInstruction::I_JMP:        // Jump unconditionally [OBSOLETE]
		{
			//
			// N.B.  This IR instruction will be removed.  No operation is
			//       performed here as this case is handled by termination of
			//       the current flow.
			//
		}
		break;

	case NWScriptInstruction::I_CALL:       // Call subroutine
		{
			NWNScriptLib::VariableWeakPtrVec       * ParamList;
			NWScriptSubroutine                     * CalledSub;
			MethodBuilder                          ^ MSILSub;
			size_t                                   ParamCount;
			size_t                                   ReturnCount;
			bool                                     IgnoreCall;

			ParamList   = IRInstr.GetParamVarList( );
			CalledSub   = IRInstr.GetSubroutine( );
			MSILSub     = GetMSILSubroutine( CalledSub->GetAddress( ) );
			ParamCount  = CalledSub->GetParameters( ).size( );
			ReturnCount = CalledSub->GetNumReturnTypes( );
			IgnoreCall  = IsIgnoredCallInstr( Sub, CalledSub );

			if (!IgnoreCall)
			{
				//
				// Now load all of the arguments for the subroutine.
				//

				Sub->ILGen->Emit( OpCodes::Ldarg_0 );

				if (ReturnCount + ParamCount != ParamList->size( ))
					throw gcnew Exception( "Subroutine call parameter list size is inconsistent." );

				for (size_t i = ReturnCount; i < ReturnCount + ParamCount; i += 1)
					GenerateLoadVariable( ParamList->at( i ) );

				//
				// Issue the method call, then unpack the return value to our
				// associated working local variables (if any).
				//

				Sub->ILGen->Emit( OpCodes::Call, MSILSub );

				if (ReturnCount != 0)
				{
					GenerateUnpackReturnValue(
						CalledSub,
						MSILSub,
						&(*ParamList)[ 0 ],
						ReturnCount);
				}
			}
			else
			{
				//
				// Load default values for each return value.
				//
				// N.B.  For ignored call sites, we must only use the parameter
				//       list in the instruction itself, as it is permitted to
				//       mismatch with the prototype of the target (as for the
				//       case of #globals calling the entry point).
				//

				for (size_t i = 0; i < ReturnCount; i += 1)
					GenerateSetDefaultValue( ParamList->at( i ) );
			}
		}
		break;

	case NWScriptInstruction::I_RETN:       // Return from subroutine
		{
			//
			// First, write the standard epilog (before we place anything
			// further on the stack).
			//

			GenerateEpilog( );

			if (Sub->IRSub->GetNumReturnTypes( ) != 0)
			{
				std::vector< NWScriptVariable * > ReturnVars;

				ReturnVars.reserve( Sub->IRSub->GetNumReturnTypes( ) );

				for (size_t i = 0; i < Sub->IRSub->GetNumReturnTypes( ); i += 1)
				{
					ReturnVars.push_back(
						&Sub->IRSub->GetReturnValueVariable( i ) );
				}

				GeneratePackReturnValue(
					Sub->IRSub,
					&ReturnVars[ 0 ],
					ReturnVars.size( ) );
			}

			Sub->ILGen->Emit( OpCodes::Ret );
		}
		break;

	case NWScriptInstruction::I_ACTION:     // Call script action
		{
			NWNScriptLib::VariableWeakPtrVec       * ParamList;
			NWSCRIPT_ACTION                          CalledActionId;
			size_t                                   ParamCount;

			ParamList      = IRInstr.GetParamVarList( );
			CalledActionId = (NWSCRIPT_ACTION) IRInstr.GetActionIndex( );
			ParamCount     = (size_t) IRInstr.GetActionParameterCount( );

			GenerateExecuteActionService(
				CalledActionId,
				ParamList,
				ParamCount);
		}
		break;

	case NWScriptInstruction::I_SAVE_STATE: // Save state for script situation
		{
			NWNScriptLib::VariableWeakPtrVec    * ParamList;
			NWScriptSubroutine                  * IRSub;
			PROGRAM_COUNTER                       IRSubPC;
			uintptr_t                             NumGlobals;
			NWScriptVariable                  * * SaveLocalList;
			size_t                                SaveLocalCount;
			NWScriptVariable                  * * SaveGlobalList;
			SubroutineAttributes                ^ Attributes;

			ParamList   = IRInstr.GetParamVarList( );
			IRSub       = IRInstr.GetSubroutine( );
			IRSubPC     = IRSub->GetAddress( );
			NumGlobals  = IRInstr.GetStateNumGlobals( );
			Attributes  = GetMSILSubroutineAttributes( IRSubPC );

			//
			// The locals to save are stored after all globals to save.  For
			// purposes of minimizing redundant code size, we always save all
			// global variables.  The compiler always saves all active globals
			// in the scope so this doesn't really work out to any loss at the
			// end of the day.
			//
			// The only exception is if an OP_SAVE_STATE/OP_SAVE_STATEALL is
			// issued during the direct course of #globals setup.  This is not
			// really functional anyway, though, because references to globals
			// outside of the saved state assume that the entire #globals frame
			// was setup (i.e. so any references to globals don't even work in
			// that case anyway, even in the script VM).
			//
			// Hence, no particular special effort is made to support the case
			// of partial global variable frame saves.
			//

			if ((SaveLocalCount = (ParamList->size( ) - NumGlobals)) == 0)
				SaveLocalList = NULL;
			else
				SaveLocalList = &(*ParamList)[ NumGlobals ];

			if (NumGlobals == 0)
				SaveGlobalList = NULL;
			else
				SaveGlobalList = &(*ParamList)[ 0 ];

			//
			// Now generate instructions to save locals into the current saved
			// state object.
			//

			GenerateSaveState(
				Attributes->Method,
				Attributes->ResumeMethodId,
				IRSubPC,
				SaveLocalList,
				SaveLocalCount,
				SaveGlobalList,
				(size_t) NumGlobals);
		}
		break;

	case NWScriptInstruction::I_LOGAND:     // Logical AND (&&)
		{
			Label L1;
			Label L2;

			AssertTypeEqual( GetVariableType( ReadVars[ 0 ] ), Int32::typeid );
			AssertTypeEqual( GetVariableType( ReadVars[ 1 ] ), Int32::typeid );
			AssertTypeEqual( GetVariableType( WriteVars[ 0 ] ), Int32::typeid );

			//
			// Cast both arguments to Boolean values and perform the operation.
			//

			L1 = Sub->ILGen->DefineLabel( );
			L2 = Sub->ILGen->DefineLabel( );

			GenerateLoadVariable( ReadVars[ 1 ] );
			Sub->ILGen->Emit( OpCodes::Brfalse_S, L1 );
			GenerateLoadVariable( ReadVars[ 0 ] );
			Sub->ILGen->Emit( OpCodes::Brfalse_S, L1 );

			Sub->ILGen->Emit( OpCodes::Ldc_I4_1 );
			Sub->ILGen->Emit( OpCodes::Br_S, L2 );
			Sub->ILGen->MarkLabel( L1 );
			Sub->ILGen->Emit( OpCodes::Ldc_I4_0 );
			Sub->ILGen->MarkLabel( L2 );

			GenerateStoreVariable( WriteVars[ 0 ] );
		}
		break;

	case NWScriptInstruction::I_LOGOR:      // Logical OR (||)
		{
			Label L1;
			Label L2;

			AssertTypeEqual( GetVariableType( ReadVars[ 0 ] ), Int32::typeid );
			AssertTypeEqual( GetVariableType( ReadVars[ 1 ] ), Int32::typeid );
			AssertTypeEqual( GetVariableType( WriteVars[ 0 ] ), Int32::typeid );

			//
			// Cast both arguments to Boolean values and perform the operation.
			//

			L1 = Sub->ILGen->DefineLabel( );
			L2 = Sub->ILGen->DefineLabel( );

			GenerateLoadVariable( ReadVars[ 1 ] );
			Sub->ILGen->Emit( OpCodes::Brtrue_S, L1 );
			GenerateLoadVariable( ReadVars[ 0 ] );
			Sub->ILGen->Emit( OpCodes::Brtrue_S, L1 );

			Sub->ILGen->Emit( OpCodes::Ldc_I4_0 );
			Sub->ILGen->Emit( OpCodes::Br_S, L2 );
			Sub->ILGen->MarkLabel( L1 );
			Sub->ILGen->Emit( OpCodes::Ldc_I4_1 );
			Sub->ILGen->MarkLabel( L2 );

			GenerateStoreVariable( WriteVars[ 0 ] );
		}
		break;

	case NWScriptInstruction::I_INCOR:      // Bitwise OR (|)
		{
			GenerateBinaryOp(
				ReadVars[ 1 ],
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Or,
				ACTIONTYPE_INT);
		}
		break;

	case NWScriptInstruction::I_EXCOR:      // Bitwise XOR (^)
		{
			GenerateBinaryOp(
				ReadVars[ 1 ],
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Xor,
				ACTIONTYPE_INT);
		}
		break;

	case NWScriptInstruction::I_BOOLAND:    // Bitwise AND (&)
		{
			GenerateBinaryOp(
				ReadVars[ 1 ],
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::And,
				ACTIONTYPE_INT);
		}
		break;

	case NWScriptInstruction::I_EQUAL:      // Compare (==)
		{
			GenerateCompare(
				ReadVars[ 1 ],
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Ceq,
				false);
		}
		break;

	case NWScriptInstruction::I_NEQUAL:     // Compare (!=)
		{
			GenerateCompare(
				ReadVars[ 1 ],
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Ceq,
				true);
		}
		break;

	case NWScriptInstruction::I_GEQ:        // Compare (>=)
		{
			GenerateCompare(
				ReadVars[ 1 ],
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Clt,
				true);
		}
		break;

	case NWScriptInstruction::I_GT:         // Compare (>)
		{
			GenerateCompare(
				ReadVars[ 1 ],
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Cgt,
				false);
		}
		break;

	case NWScriptInstruction::I_LT:         // Compare (<)
		{
			GenerateCompare(
				ReadVars[ 1 ],
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Clt,
				false);
		}
		break;

	case NWScriptInstruction::I_LEQ:        // Compare (<=)
		{
			GenerateCompare(
				ReadVars[ 1 ],
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Cgt,
				true);
		}
		break;

	case NWScriptInstruction::I_SHLEFT:     // Shift left (<<)
		{
			GenerateBinaryOp(
				ReadVars[ 1 ],
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Shl,
				ACTIONTYPE_INT);
		}
		break;

	case NWScriptInstruction::I_SHRIGHT:    // Shift right (>>, SAR)
		{
			Label L1;
			Label L2;

			AssertTypeEqual( GetVariableType( ReadVars[ 0 ] ), Int32::typeid );
			AssertTypeEqual( GetVariableType( ReadVars[ 1 ] ), Int32::typeid );

			//
			// N.B.  The canonical VM implements a complex operation that first
			//       negates the amount to shift if it is negative.
			//

			L1 = Sub->ILGen->DefineLabel( );
			L2 = Sub->ILGen->DefineLabel( );

			GenerateLoadVariable( ReadVars[ 1 ] );  // Load for final sign test
			GenerateLoadVariable( ReadVars[ 1 ] );  // Load first shift operand
			GenerateLoadVariable( ReadVars[ 1 ] );  // Load again to test sign
			Sub->ILGen->Emit( OpCodes::Ldc_I4_0 );  //
			Sub->ILGen->Emit( OpCodes::Bge_S, L1 ); // If positive, skip
			Sub->ILGen->Emit( OpCodes::Neg );       // Negate sign of operand
			Sub->ILGen->MarkLabel( L1 );            // Skip branch target

			GenerateUnaryOp(    // Not really a unary op (uses above operand)
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Shr,
				ACTIONTYPE_INT);

			Sub->ILGen->Emit( OpCodes::Ldc_I4_0 );  //
			Sub->ILGen->Emit( OpCodes::Bge_S, L2 ); // If positive, skip
			GenerateUnaryOp(                        // Negate if negative
				WriteVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Neg,
				ACTIONTYPE_INT);
			Sub->ILGen->MarkLabel( L2 );            // Skip branch target
		}
		break;

	case NWScriptInstruction::I_USHRIGHT:   // Shift unsigned right (>>)
		{
			//
			// N.B.  The canonical VM implements a signed right shift for this
			//       instruction.  While this appears to be a bug, it needs to
			//       be implemented in a compatible fashion.
			//

			GenerateBinaryOp(
				ReadVars[ 1 ],
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Shr,
				ACTIONTYPE_INT);
		}
		break;

	case NWScriptInstruction::I_ADD:        // Add (+), concatenate strings
		{
			//
			// Handle string concatenate separately; otherwise, dispatch to the
			// common upcast binary op generator.
			//

			if (ReadVars[ 0 ]->GetType( ) == ACTIONTYPE_STRING)
			{
				if (ReadVars[ 1 ]->GetType( ) != ACTIONTYPE_STRING)
					throw gcnew Exception( "Type mismatch for string concatenate in IR." );

				GenerateLoadVariable( ReadVars[ 1 ] );
				GenerateLoadVariable( ReadVars[ 0 ] );

				Sub->ILGen->Emit( OpCodes::Call, m_ILGenCtx->MthString_Concat );

				GenerateStoreVariable( WriteVars[ 0 ] );

				break;
			}

			GenerateUpcastBinaryOp(
				ReadVars[ 1 ],
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Add);
		}
		break;

	case NWScriptInstruction::I_SUB:        // Subtract (-)
		{
			GenerateUpcastBinaryOp(
				ReadVars[ 1 ],
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Sub);
		}
		break;

	case NWScriptInstruction::I_MUL:        // Multiply (*)
		{
			GenerateUpcastBinaryOp(
				ReadVars[ 1 ],
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Mul);
		}
		break;

	case NWScriptInstruction::I_DIV:        // Divide (/)
		{
			GenerateUpcastBinaryOp(
				ReadVars[ 1 ],
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Div);
		}
		break;

	case NWScriptInstruction::I_MOD:        // Modulus (%)
		{
			GenerateBinaryOp(
				ReadVars[ 1 ],
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Rem,
				ACTIONTYPE_INT);
		}
		break;

	case NWScriptInstruction::I_NEG:        // Negation (-)
		{
			NWACTION_TYPE ActionType;

			if (ReadVars[ 0 ]->GetType( ) == ACTIONTYPE_INT)
				ActionType = ACTIONTYPE_INT;
			else
				ActionType = ACTIONTYPE_FLOAT;

			GenerateUnaryOp(
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Neg,
				ActionType);
		}
		break;

	case NWScriptInstruction::I_COMP:       // Complement (~)
		{
			GenerateUnaryOp(
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Not,
				ACTIONTYPE_INT);
		}
		break;

	case NWScriptInstruction::I_NOT:        // Logical NOT (!)
		{
			AssertTypeEqual( GetVariableType( ReadVars[ 0 ] ), Int32::typeid );
			AssertTypeEqual( GetVariableType( WriteVars[ 0 ] ), Int32::typeid );

			//
			// Cast the argument to a Boolean value and invert it.
			//

			GenerateLoadVariable( ReadVars[ 0 ] );

			Sub->ILGen->Emit( OpCodes::Ldc_I4_0 );
			Sub->ILGen->Emit( OpCodes::Ceq );

			GenerateStoreVariable( WriteVars[ 0 ] );
		}
		break;

	case NWScriptInstruction::I_INC:        // Increment
		{
			Sub->ILGen->Emit( OpCodes::Ldc_I4_1 );

			GenerateUnaryOp(   // Not really a unary op (uses above constant)
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Add,
				ACTIONTYPE_INT);
		}
		break;

	case NWScriptInstruction::I_DEC:        // Decrement
		{
			Sub->ILGen->Emit( OpCodes::Ldc_I4_M1 );

			GenerateUnaryOp(   // Not really a unary op (uses above constant)
				ReadVars[ 0 ],
				WriteVars[ 0 ],
				OpCodes::Add,
				ACTIONTYPE_INT);
		}
		break;

	case NWScriptInstruction::I_TEST:       // Set zero/not zero based on variable value
		{
			AssertTypeEqual( GetVariableType( ReadVars[ 0 ] ), Int32::typeid );

			//
			// Cast the argument to a Boolean value.  The I_TEST instruction is
			// synthesized internally by the IR raising process in order to
			// generate a logic 1 or logic 0 for an upcoming I_JZ/I_JNZ IR
			// instruction.
			//

			GenerateLoadVariable( ReadVars[ 0 ] );

#if 0
			Sub->ILGen->Emit( OpCodes::Ldc_I4_0 );
			Sub->ILGen->Emit( OpCodes::Ceq );
			Sub->ILGen->Emit( OpCodes::Ldc_I4_0 );
			Sub->ILGen->Emit( OpCodes::Ceq );
#else
//			GenerateCastToBool( );
#endif

			//
			// Note that the result of the test is left on the evaluation stack
			// for pick-up by the next instruction, which is required to be an
			// I_JZ/I_JNZ.
			//
		}
		break;

	case NWScriptInstruction::I_INITIALIZE: // Set variable to default value
		{
			GenerateSetDefaultValue( WriteVars[ 0 ] );
		}
		break;

	default:
		throw gcnew Exception( "Unrecognized IR instruction." );

	}
}




void
NWScriptCodeGenerator::GenerateVMStackPush(
	__in ILGenerator ^ ILGen,
	__in Int32 i
	)
/*++

Routine Description:

	This routine generates an intrinsic call to Intrinsic_VMStackPushInt.

Arguments:

	ILGen - Supplies the IL generator context to emit the call into.

	i - Supplies the value to push onto the VM stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	//
	// Fetch NWScriptProgram ^ T::m_Program.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );
#if NWSCRIPT_VIRT_TYPE_WORKAROUND
	ILGen->Emit( OpCodes::Castclass, m_HostInterfaceType );
#endif

	//
	// Now set up for a call to NWScriptCodeGenerator::Intrinsic_VMStackPushInt.
	//

	ILGen->Emit( OpCodes::Ldc_I4, i );
	ILGen->Emit( OpCodes::Callvirt, m_ILGenCtx->MthIntrinsic_VMStackPushInt );
}

void
NWScriptCodeGenerator::GenerateVMStackPush(
	__in ILGenerator ^ ILGen,
	__in UInt32 o
	)
/*++

Routine Description:

	This routine generates an intrinsic call to Intrinsic_VMStackPushInt.

Arguments:

	ILGen - Supplies the IL generator context to emit the call into.

	o - Supplies the value to push onto the VM stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	switch (o)
	{

	default:
		if (o != (UInt32) m_ILGenCtx->ObjectInvalid)
		{
			throw gcnew Exception(
				"Illegal object id literal for GenerateVMStackPush/ObjectId");
		}

		//
		// Fallthrough.
		//

	case OBJECTID_INVALID:
		{
			//
			// Fetch NWScriptProgram ^ T::m_Program.
			//

			ILGen->Emit( OpCodes::Ldarg_0 );
			ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );

			//
			// Now set up for a call to NWScriptCodeGenerator::Intrinsic_VMStackPushObjectId.
			//

			ILGen->Emit( OpCodes::Ldc_I4, (Int32) m_ILGenCtx->ObjectInvalid );
			ILGen->Emit( OpCodes::Conv_U4 );
			ILGen->Emit( OpCodes::Callvirt, m_ILGenCtx->MthIntrinsic_VMStackPushObjectId );
		}
		break;

	case OBJECTID_SELF:
		{
			//
			// Fetch NWScriptProgram ^ T::m_Program.
			//

			ILGen->Emit( OpCodes::Ldarg_0 );
			ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );
#if NWSCRIPT_VIRT_TYPE_WORKAROUND
			ILGen->Emit( OpCodes::Castclass, m_HostInterfaceType );
#endif

			//
			// Fetch NWScriptProgram ^ T::m_CurrentActionObjectSelf.
			//

			ILGen->Emit( OpCodes::Ldarg_0 );
			ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldCurrentActionObjectSelf );

			//
			// Now set up for a call to NWScriptCodeGenerator::Intrinsic_VMStackPushObjectId.
			//

			ILGen->Emit( OpCodes::Callvirt, m_ILGenCtx->MthIntrinsic_VMStackPushObjectId );
		}
		break;

	}
}

void
NWScriptCodeGenerator::GenerateVMStackPush(
	__in ILGenerator ^ ILGen,
	__in Single f
	)
/*++

Routine Description:

	This routine generates an intrinsic call to Intrinsic_VMStackPushFloat.

Arguments:

	ILGen - Supplies the IL generator context to emit the call into.

	f - Supplies the value to push onto the VM stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	//
	// Fetch NWScriptProgram ^ T::m_Program.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );
#if NWSCRIPT_VIRT_TYPE_WORKAROUND
	ILGen->Emit( OpCodes::Castclass, m_HostInterfaceType );
#endif

	//
	// Now set up for a call to NWScriptCodeGenerator::Intrinsic_VMStackPushFloat.
	//

	ILGen->Emit( OpCodes::Ldc_R4, f );
	ILGen->Emit( OpCodes::Callvirt, m_ILGenCtx->MthIntrinsic_VMStackPushFloat );
}

void
NWScriptCodeGenerator::GenerateVMStackPush(
	__in ILGenerator ^ ILGen,
	__in String ^ s
	)
/*++

Routine Description:

	This routine generates an intrinsic call to Intrinsic_VMStackPushString.

Arguments:

	ILGen - Supplies the IL generator context to emit the call into.

	s - Supplies the value to push onto the VM stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	//
	// Fetch NWScriptProgram ^ T::m_Program.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );
#if NWSCRIPT_VIRT_TYPE_WORKAROUND
	ILGen->Emit( OpCodes::Castclass, m_HostInterfaceType );
#endif

	//
	// Now set up for a call to NWScriptCodeGenerator::Intrinsic_VMStackPushString.
	//

	ILGen->Emit( OpCodes::Ldstr, s );
	ILGen->Emit( OpCodes::Callvirt, m_ILGenCtx->MthIntrinsic_VMStackPushString );
}

void
NWScriptCodeGenerator::GenerateVMStackPush(
	__in ILGenerator ^ ILGen,
	__in LocalBuilder ^ Local
	)
/*++

Routine Description:

	This routine generates an intrinsic call to Intrinsic_VMStackPush< T >.

Arguments:

	ILGen - Supplies the IL generator context to emit the call into.

	Local - Supplies the local to push onto the stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	Type       ^ LocalType;
	MethodInfo ^ Intrinsic;
	bool         EngineStruct;

	//
	// Determine the intrinsic to invoke.  First, handle the base types, and
	// then check for extension (engine structure) types.
	//

	LocalType    = Local->LocalType;
	EngineStruct = false;

	if (LocalType == Int32::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPushInt;
	else if (LocalType == Single::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPushFloat;
	else if (LocalType == String::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPushString;
	else if (LocalType == UInt32::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPushObjectId;
	else
	{
		Intrinsic = nullptr;

		for (int i = 0; i < NUM_ENGINE_STRUCTURE_TYPES; i += 1)
		{
			if (LocalType != m_ILGenCtx->EngineStructureTypes[ i ])
				continue;

			Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPushEngineStructure[ i ];
			break;
		}

		if (Intrinsic == nullptr)
			throw gcnew Exception( "GenerateVMStackPush: Attempted to push unsupported local type " + LocalType->Name + " onto VM stack." );

		EngineStruct = true;
	}

	//
	// Fetch NWScriptProgram ^ T::m_Program.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );

	if (EngineStruct)
		ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgram );
	else
		ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );

#if NWSCRIPT_VIRT_TYPE_WORKAROUND
	if (EngineStruct)
		ILGen->Emit( OpCodes::Castclass, NWScriptJITIntrinsics::typeid );
	else
		ILGen->Emit( OpCodes::Castclass, m_HostInterfaceType );
#endif

	//
	// Now set up for a call to NWScriptCodeGenerator::Intrinsic_VMStackPush< T >
	//

	ILGen->Emit( OpCodes::Ldloc, Local );
	ILGen->Emit( OpCodes::Callvirt, Intrinsic );
}

void
NWScriptCodeGenerator::GenerateVMStackPush(
	__in ILGenerator ^ ILGen,
	__in NWScriptVariable * Var
	)
/*++

Routine Description:

	This routine generates an intrinsic call to Intrinsic_VMStackPush< T >.

Arguments:

	ILGen - Supplies the IL generator context to emit the call into.

	Var - Supplies the IR variable to push onto the stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	Type         ^ LocalType;
	MethodInfo   ^ Intrinsic;
	bool           EngineStruct;

	//
	// Determine the intrinsic to invoke.  First, handle the base types, and
	// then check for extension (engine structure) types.
	//

	Var          = Var->GetHeadVariable( );
	LocalType    = GetVariableType( Var );
	EngineStruct = false;

	if (LocalType == Int32::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPushInt;
	else if (LocalType == Single::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPushFloat;
	else if (LocalType == String::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPushString;
	else if (LocalType == UInt32::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPushObjectId;
	else
	{
		Intrinsic = nullptr;

		for (int i = 0; i < NUM_ENGINE_STRUCTURE_TYPES; i += 1)
		{
			if (LocalType != m_ILGenCtx->EngineStructureTypes[ i ])
				continue;

			Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPushEngineStructure[ i ];
			break;
		}

		if (Intrinsic == nullptr)
			throw gcnew Exception( "GenerateVMStackPush: Attempted to push unsupported local type " + LocalType->Name + " onto VM stack." );

		EngineStruct = true;
	}

	//
	// Fetch NWScriptProgram ^ T::m_Program.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );

	if (EngineStruct)
		ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgram );
	else
		ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );

#if NWSCRIPT_VIRT_TYPE_WORKAROUND
	if (EngineStruct)
		ILGen->Emit( OpCodes::Castclass, NWScriptJITIntrinsics::typeid );
	else
		ILGen->Emit( OpCodes::Castclass, m_HostInterfaceType );
#endif

	//
	// Now set up for a call to NWScriptCodeGenerator::Intrinsic_VMStackPush< T >
	//

	GenerateLoadVariable( Var );

	ILGen->Emit( OpCodes::Callvirt, Intrinsic );
}

void
NWScriptCodeGenerator::GenerateVMStackPush(
	__in ILGenerator ^ ILGen,
	__in Type ^ ArgType,
	__in short ArgSlot
	)
/*++

Routine Description:

	This routine generates an intrinsic call to Intrinsic_VMStackPush< T >.

Arguments:

	ILGen - Supplies the IL generator context to emit the call into.

	ArgType - Supplies the MSIL type of the variable.

	ArgSlot - Supplies the MSIL argument slot of the variable to push onto the
	          stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	MethodInfo   ^ Intrinsic;
	bool           EngineStruct;

	//
	// Determine the intrinsic to invoke.  First, handle the base types, and
	// then check for extension (engine structure) types.
	//

	EngineStruct = false;

	if (ArgType == Int32::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPushInt;
	else if (ArgType == Single::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPushFloat;
	else if (ArgType == String::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPushString;
	else if (ArgType == UInt32::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPushObjectId;
	else
	{
		Intrinsic = nullptr;

		for (int i = 0; i < NUM_ENGINE_STRUCTURE_TYPES; i += 1)
		{
			if (ArgType != m_ILGenCtx->EngineStructureTypes[ i ])
				continue;

			Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPushEngineStructure[ i ];
			break;
		}

		if (Intrinsic == nullptr)
			throw gcnew Exception( "GenerateVMStackPush: Attempted to push unsupported local type " + ArgType->Name + " onto VM stack." );

		EngineStruct = true;
	}

	//
	// Fetch NWScriptProgram ^ T::m_Program.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );

	if (EngineStruct)
		ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgram );
	else
		ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );

#if NWSCRIPT_VIRT_TYPE_WORKAROUND
	if (EngineStruct)
		ILGen->Emit( OpCodes::Castclass, NWScriptJITIntrinsics::typeid );
	else
		ILGen->Emit( OpCodes::Castclass, m_HostInterfaceType );
#endif

	//
	// Now set up for a call to NWScriptCodeGenerator::Intrinsic_VMStackPush< T >
	//

	ILGen->Emit( OpCodes::Ldarg, ArgSlot );

	ILGen->Emit( OpCodes::Callvirt, Intrinsic );
}



void
NWScriptCodeGenerator::GenerateVMStackPop(
	__in ILGenerator ^ ILGen,
	__in LocalBuilder ^ Local
	)
/*++

Routine Description:

	This routine generates an intrinsic call to Intrinsic_VMStackPop< T >.

Arguments:

	ILGen - Supplies the IL generator context to emit the call into.

	Local - Supplies the local to pop from the stack.  The local is assigned to
	        the contents at the top of the VM stack (if the type matched).

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	Type       ^ LocalType;
	MethodInfo ^ Intrinsic;
	bool         EngineStruct;

	//
	// Determine the intrinsic to invoke.  First, handle the base types, and
	// then check for extension (engine structure) types.
	//

	LocalType    = Local->LocalType;
	EngineStruct = false;

	if (LocalType == Int32::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPopInt;
	else if (LocalType == Single::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPopFloat;
	else if (LocalType == String::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPopString;
	else if (LocalType == UInt32::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPopObjectId;
	else
	{
		Intrinsic = nullptr;

		for (int i = 0; i < NUM_ENGINE_STRUCTURE_TYPES; i += 1)
		{
			if (LocalType != m_ILGenCtx->EngineStructureTypes[ i ])
				continue;

			Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPopEngineStructure[ i ];
			break;
		}

		if (Intrinsic == nullptr)
			throw gcnew Exception( "GenerateVMStackPop: Attempted to pop unsupported local type " + LocalType->Name + " onto VM stack." );

		EngineStruct = true;
	}

	//
	// Fetch NWScriptProgram ^ T::m_Program.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );

	if (EngineStruct)
		ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgram );
	else
		ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );

#if NWSCRIPT_VIRT_TYPE_WORKAROUND
	if (EngineStruct)
		ILGen->Emit( OpCodes::Castclass, NWScriptJITIntrinsics::typeid );
	else
		ILGen->Emit( OpCodes::Castclass, m_HostInterfaceType );
#endif

	//
	// Now set up for a call to NWScriptCodeGenerator::Intrinsic_VMStackPop< T >
	//

	ILGen->Emit( OpCodes::Callvirt, Intrinsic );
	ILGen->Emit( OpCodes::Stloc, Local );
}

void
NWScriptCodeGenerator::GenerateVMStackPop(
	__in ILGenerator ^ ILGen,
	__in NWScriptVariable * Var
	)
/*++

Routine Description:

	This routine generates an intrinsic call to Intrinsic_VMStackPop< T >.

Arguments:

	ILGen - Supplies the IL generator context to emit the call into.

	Var - Supplies the IR variable to pop from the stack.  The local is
	      assigned to the contents at the top of the VM stack (if the type
	      matched).

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	Type       ^ LocalType;
	MethodInfo ^ Intrinsic;
	bool         EngineStruct;

	//
	// Determine the intrinsic to invoke.  First, handle the base types, and
	// then check for extension (engine structure) types.
	//

	Var          = Var->GetHeadVariable( );
	LocalType    = GetVariableType( Var );
	EngineStruct = false;

	if (LocalType == Int32::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPopInt;
	else if (LocalType == Single::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPopFloat;
	else if (LocalType == String::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPopString;
	else if (LocalType == UInt32::typeid)
		Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPopObjectId;
	else
	{
		Intrinsic = nullptr;

		for (int i = 0; i < NUM_ENGINE_STRUCTURE_TYPES; i += 1)
		{
			if (LocalType != m_ILGenCtx->EngineStructureTypes[ i ])
				continue;

			Intrinsic = m_ILGenCtx->MthIntrinsic_VMStackPopEngineStructure[ i ];
			break;
		}

		if (Intrinsic == nullptr)
			throw gcnew Exception( "GenerateVMStackPop: Attempted to pop unsupported local type " + LocalType->Name + " onto VM stack." );

		EngineStruct = true;
	}

	//
	// Fetch NWScriptProgram ^ T::m_Program.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );

	if (EngineStruct)
		ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgram );
	else
		ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );

#if NWSCRIPT_VIRT_TYPE_WORKAROUND
	if (EngineStruct)
		ILGen->Emit( OpCodes::Castclass, NWScriptJITIntrinsics::typeid );
	else
		ILGen->Emit( OpCodes::Castclass, m_HostInterfaceType );
#endif

	//
	// Now set up for a call to NWScriptCodeGenerator::Intrinsic_VMStackPop< T >
	//

	ILGen->Emit( OpCodes::Callvirt, Intrinsic );

	GenerateStoreVariable( Var );
}


LocalBuilder ^
NWScriptCodeGenerator::AcquireLocal(
	__in Type ^ LocalType
	)
/*++

Routine Description:

	This routine acquires a local variable of a given type for usage within the
	current subroutine.

	If there are no free locals of the given type within the current
	subroutine, then a new local is allocated.  Otherwise, an existing, free
	local is reused.

	N.B.  It is the responsibility of the caller to link the local to a high
	      level IR variable.

Arguments:

	LocalType - Supplies the type of the local.

Return Value:

	A LocalBuilder handle is returned to the caller, representing an available
	local variable.  The caller bears responsibility for returning the local to
	the free pool via a call to ReleaseLocal when the local has been deleted by
	the high level IR.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	LocalStack           ^ FreeStack;
	SubroutineGenContext ^ Sub;
	ILGenerator          ^ ILGen;

	Sub   = m_ILGenCtx->Sub;
	ILGen = Sub->ILGen;

	//
	// If we haven't created the free pool for this type yet, or the free pool
	// is empty, then we must declare a new local entirely.
	//

	if (!Sub->CurrentFlow->LocalPool->TryGetValue( LocalType, FreeStack ))
		return ILGen->DeclareLocal( LocalType );

	if (FreeStack->Count == 0)
		return ILGen->DeclareLocal( LocalType );

	//
	// Otherwise, simply return a previous local that is no longer in scope, so
	// as to conserve local variable usage within the subroutine.
	//

	return FreeStack->Pop( );
}

void
NWScriptCodeGenerator::ReleaseLocal(
	__in LocalBuilder ^ Local
	)
/*++

Routine Description:

	This routine releases usage of a local variable with in the current
	subroutine.  The released local may then be reused freely, as within the
	context of the IR, it is considered to have been deleted.

Arguments:

	Local - Supplies the local to release.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	LocalStack            ^ FreeStack;
	Type                  ^ LocalType;
	SubroutineControlFlow ^ Flow;

	LocalType = Local->LocalType;
	Flow      = m_ILGenCtx->Sub->CurrentFlow;

	//
	// If we haven't created the free pool for this type yet, or the free pool
	// is empty, then we must declare a new local entirely.
	//

	if (!Flow->LocalPool->TryGetValue( LocalType, FreeStack ))
	{
		FreeStack = gcnew LocalStack;

		Flow->LocalPool->Add( LocalType, FreeStack );
	}

#if 0
	//
	// If we are releasing a non-basic type, then we'll need to delete the
	// underlying object now.
	//
	// N.B.  Other types, such as basic types or marshal objects, are not
	//       automatically deleted by this routine.  It is the responsibility
	//       of the caller to perform these tasks.
	//

	for (int i = 0; i < NUM_ENGINE_STRUCTURE_TYPES; i += 1)
	{
		if (m_ILGenCtx->EngineStructureTypes[ i ] != LocalType)
			continue;

		ILGenerator ^ ILGen;
		MethodInfo  ^ DeleteEngineStructure;

		ILGen = m_ILGenCtx->Sub->ILGen;

		//
		// Call the NWScriptEngineStructure<i>::DeleteEngineStructure
		// intrinsic to release the underlying script host's engine structure
		// object.
		//

		DeleteEngineStructure = m_ILGenCtx->MthEngineStructure_DeleteEngineStructure[ i ];

		ILGen->Emit( OpCodes::Ldloc, Local );
		ILGen->Emit( OpCodes::Callvirt, DeleteEngineStructure );

		//
		// Now release the managed wrapper object too.  (We don't want to wait
		// for GC in order to preserve correct program behavior, hence the
		// explicit deletion call above.)
		//

		ILGen->Emit( OpCodes::Ldnull );
		ILGen->Emit( OpCodes::Stloc, Local );

		break;
	}
#endif

	//
	// Finally, push the local back onto the free stack.
	//

	for each (LocalBuilder ^ Loc in FreeStack)
	{
		if (Loc == Local)
			throw gcnew Exception( "Freeing local to free pool twice." );
	}

	FreeStack->Push( Local );
}

LocalBuilder ^
NWScriptCodeGenerator::CreateLocal(
	__in NWScriptVariable * Var
	)
/*++

Routine Description:

	This routine acquires a local variable of a given type for usage within the
	current subroutine.

	If the variable is created in multiple flows, a non-shareable local is
	allocated for it.  Otherwise, a local from the current flow's pool is
	allocated.

	The routine also registers the link between the NWScriptVariable object and
	the MSIL local.

	N.B.  The caller bears responsibility for initializing a default value for
	      the local (if desired).

Arguments:

	Var - Supplies the variable to instantiate as a local.

Return Value:

	A LocalBuilder handle is returned to the caller, representing an available
	local variable.  The caller bears responsibility for returning the local to
	the free pool via a call to ReleaseLocal when the local has been deleted by
	the high level IR (if the variable was not multiply created).

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	SubroutineGenContext ^ Sub;
	ILGenerator          ^ ILGen;
	Type                 ^ LocalType;
	LocalBuilder         ^ Loc;

	Var       = Var->GetHeadVariable( );
	Sub       = m_ILGenCtx->Sub;
	ILGen     = Sub->ILGen;
	LocalType = GetVariableType( Var );

	//
	// If we have already allocated a local for this IR variable, then use it.
	//

	if (Var->GetScope( ) != NWNScriptLib::INVALID_SCOPE)
		return GetLocalVariable( Sub, Var );

	if (!(Var->GetFlags( ) & NWScriptVariable::MultiplyCreated))
	{
		//
		// The variable is poolable, acquire a slot from the free variable
		// pool now.
		//

		Loc = AcquireLocal( LocalType );

		Var->SetScope( Sub->CurrentFlow->VarTable->Count );
		Sub->CurrentFlow->VarTable->Add( Loc );
	}
	else
	{
		//
		// The variable will be created by multiple distinct control flows and
		// cannot come from the pool local to this flow.  Allocate a
		// non-shareable local for it.
		//

		Loc = ILGen->DeclareLocal( LocalType );

		Var->SetScope( Sub->MultipleCreatedVarTable.Count );
		Sub->MultipleCreatedVarTable.Add( Loc );
	}

	return Loc;
}




Type ^
NWScriptCodeGenerator::GenerateSubroutineRetType(
	__in const NWScriptSubroutine * IRSub
	)
/*++

Routine Description:

	This routine creates an MSIL type descriptor for the return type of a
	subroutine.

	Should the subroutine return a scalar value, its underlying type is
	returned to the calle.  Otherwise, should the subroutine return an
	aggregate type, a new type to contain all of the return values is created
	and returned.

Arguments:

	IRSub - Supplies the IR-level subroutine descriptor.

Return Value:

	A Type handle is returned on success.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	//
	// First, handle simple return types.
	//

	switch (IRSub->GetNumReturnTypes( ))
	{

	case 0:
		return void::typeid;

	case 1:
		return GetVariableType( IRSub->GetReturnTypes( ).front( ) );

	}

	//
	// The routine must return an aggregate.  Because MSIL only supports a
	// single return value, we must return a pointer to a structure that
	// represents the actual return value set.
	//

	return BuildCompositeReturnType( IRSub );
}

Type ^
NWScriptCodeGenerator::BuildCompositeReturnType(
	__in const NWScriptSubroutine * IRSub
	)
/*++

Routine Description:

	This routine creates an MSIL type descriptor for the return type of a
	subroutine which returns an aggregate (i.e. composite) type.

Arguments:

	IRSub - Supplies the IR-level subroutine descriptor.

Return Value:

	A Type handle is returned on success.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	TypeBuilder        ^ RetType;
	size_t               FldIndex;
//	Type               ^ ObjType;
//	ConstructorInfo    ^ ObjCtor;
	ConstructorBuilder ^ TypeCtor;
	ILGenerator        ^ ILGen;

	RetType = m_ILGenCtx->ProgramModule->DefineType(
		m_ILGenCtx->Namespace + "ReturnTypes.RetSub_" + IRSub->GetAddress( ),
		TypeAttributes::NotPublic | TypeAttributes::Sealed,
		ValueType::typeid);

	//
	// Now create fields for each of the scalar members of the aggregate.  We
	// are guaranteed that each of the members aren't sub-aggregates (i.e. if
	// they were, they have already been decomposed to flat values).
	//

	FldIndex = 0;

	for (NWNScriptLib::ReturnTypeList::const_iterator it = IRSub->GetReturnTypes( ).begin( );
	     it != IRSub->GetReturnTypes( ).end( );
	     ++it)
	{
		RetType->DefineField(
			"ReturnValue_" + FldIndex,
			GetVariableType( *it ),
			FieldAttributes::Public);

		FldIndex += 1;
	}

	//
	// Create the constructor, which takes no parameters.
	//

//	ObjType  = Object::typeid;
//	ObjCtor  = ObjType->GetConstructor( Type::EmptyTypes );

	TypeCtor = RetType->DefineConstructor(
		MethodAttributes::Public,
		CallingConventions::Standard,
		Type::EmptyTypes);

	ILGen = TypeCtor->GetILGenerator( );

	//
	// System::Object::Object( this )
	//

//	ILGen->Emit( OpCodes::Ldarg_0 );
//	ILGen->Emit( OpCodes::Call, ObjCtor );

	ILGen->Emit( OpCodes::Ret );

	return RetType->CreateType( );
}

LocalBuilder ^
NWScriptCodeGenerator::GeneratePackReturnValue(
	__in const NWScriptSubroutine * IRSub,
	__in_ecount( ReturnValueCount ) NWScriptVariable * * ReturnValues,
	__in size_t ReturnValueCount
	)
/*++

Routine Description:

	This routine emits any code necessary to prepare the return value on the
	MSIL evaluation stack in preparation for a subroutine return.  On return
	from this routine, MSIL has been emitted to arrange for the return value to
	be at the current position on the evaluation stack.

	Typically, this routine is invoked just before a subroutine returns to its
	caller.

Arguments:

	IRSub - Supplies the IR-level subroutine descriptor.

	ReturnValues - Supplies the list of working variables to copy the return
	               value of the just called subroutine from (many be zero or
	               more).

	ReturnValueCount - Supplies the count of return values.

Return Value:

	A handle to the local variable containing the real return value (which may
	be nullptr if there were no return values, the first entry in ReturnValues
	should there be a signle return value, or a new, temporary aggregate object
	should the return value be an anggregate type) is returned.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	if (IsDebugLevel( NWScriptVM::EDL_Verbose ))
	{
		m_TextOut->WriteText(
			"[Sub=%s(%08X)] Packing %lu return values.\n",
			IRSub->GetSymbolName( ).c_str( ),
			IRSub->GetAddress( ),
			(unsigned long) ReturnValueCount);
	}

	if (ReturnValueCount == 0)
	{
		//
		// If we were returning nothing, then there is nothing to do here.
		//

		return nullptr;
	}
	else if (ReturnValueCount == 1)
	{
		GenerateLoadVariable( ReturnValues[ 0 ] );

		return nullptr;
	}
	else
	{
		ILGenerator     ^ ILGen;
		Type            ^ RetType;
		FieldInfo       ^ Fld;
		size_t            FldIndex;
		LocalBuilder    ^ RetLocal;
		ConstructorInfo ^ Ctor;

		//
		// We're returning multiple return values.  We need to create a
		// temporary aggregate in order to marshal the data back to the caller.
		//

		ILGen     = m_ILGenCtx->Sub->ILGen;
		RetType   = m_ILGenCtx->Sub->MSILSub->ReturnType;
		FldIndex  = 0;
		RetLocal  = ILGen->DeclareLocal( RetType );
		Ctor      = RetType->GetConstructor( Type::EmptyTypes );

		//
		// Create a new marshal object to contain the aggregate return type.
		//

//		ILGen->Emit( OpCodes::Newobj, Ctor );
//		ILGen->Emit( OpCodes::Stloc, RetLocal );

		//
		// Now copy all of the return locals into the aggregate.
		//

		for (NWNScriptLib::ReturnTypeList::const_iterator it = IRSub->GetReturnTypes( ).begin( );
		     it != IRSub->GetReturnTypes( ).end( );
		     ++it)
		{
			Fld = RetType->GetField( "ReturnValue_" + FldIndex );

			ILGen->Emit( OpCodes::Ldloca, RetLocal );

			GenerateLoadVariable( ReturnValues[ (int) FldIndex ] );

			ILGen->Emit( OpCodes::Stfld, Fld );

			FldIndex += 1;
		}

		//
		// Finally, push the marshal object onto the evaluation stack and
		// return to our caller.
		//

		ILGen->Emit( OpCodes::Ldloc, RetLocal );

		return RetLocal;
	}
}

void
NWScriptCodeGenerator::GenerateUnpackReturnValue(
	__in const NWScriptSubroutine * IRSub,
	__in MethodBuilder ^ MSILSub,
	__in_ecount( ReturnValueCount ) NWScriptVariable * * ReturnValues,
	__in size_t ReturnValueCount
	)
/*++

Routine Description:

	This routine emits any code necessary to copy the return value of a called
	subroutine to its working local variables in the callee.

	Typically, this routine is invoked just after a subroutine call site that
	may (or may not) have returned a value to its caller.  The return values of
	the prior callee, if any, are copied to their working locals in the
	caller's local frame.

Arguments:

	IRSub - Supplies the IR-level subroutine descriptor.  This is the
	        subroutine that was just called, NOT the current subroutine.

	MSILSub - Supplies the MSIL-level subroutine descriptor associated with the
	          given IR subroutine.

	ReturnValues - Supplies the list of working variables to store the return
	               value of the just called subroutine to (many be zero or
	               more).

	ReturnValueCount - Supplies the count of return values.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	if (ReturnValueCount == 0)
	{
		//
		// If we were returning nothing, then there is nothing to do here.
		//
	}
	else if (ReturnValueCount == 1)
	{
		//
		// We're returning a single scalar value, simply copy it directly from
		// the top of the evaluation stack.
		//

		GenerateStoreVariable( ReturnValues[ 0 ] );
	}
	else
	{
		ILGenerator     ^ ILGen;
		Type            ^ RetType;
		FieldInfo       ^ Fld;
		size_t            FldIndex;
		LocalBuilder    ^ RetLocal;
		ConstructorInfo ^ Ctor;

		//
		// We're returning multiple return values.  We need to create a
		// temporary aggregate in order to marshal the data back to the caller.
		//

		ILGen     = m_ILGenCtx->Sub->ILGen;
		RetType   = MSILSub->ReturnType;
		FldIndex  = 0;
		RetLocal  = AcquireLocal( RetType );
		Ctor      = RetType->GetConstructor( Type::EmptyTypes );

		if (Ctor == nullptr)
			throw gcnew Exception( "No constructor exists for return value struct." );

		//
		// Save the called subroutine's marshal object so that it can be
		// unpacked.
		//

		ILGen->Emit( OpCodes::Stloc, RetLocal );

		//
		// Now copy all of the return locals into the aggregate.
		//

		for (NWNScriptLib::ReturnTypeList::const_iterator it = IRSub->GetReturnTypes( ).begin( );
		     it != IRSub->GetReturnTypes( ).end( );
		     ++it)
		{
			Fld = RetType->GetField( "ReturnValue_" + FldIndex );

			ILGen->Emit( OpCodes::Ldloca, RetLocal );
			ILGen->Emit( OpCodes::Ldfld, Fld );

			GenerateStoreVariable( ReturnValues[ (int) FldIndex ] );

			FldIndex += 1;
		}

		//
		// Finally, release the marshal object and return the local back to its
		// pool (in case we call the same subroutine twice).
		//

//		ILGen->Emit( OpCodes::Ldnull );
//		ILGen->Emit( OpCodes::Stloc, RetLocal );

		ReleaseLocal( RetLocal );
	}
}


void
NWScriptCodeGenerator::GenerateExecuteActionService(
	__in NWSCRIPT_ACTION CalledActionId,
	__in NWNScriptLib::VariableWeakPtrVec * ParamList,
	__in size_t ParamCount
	)
/*++

Routine Description:

	This routine emits any code necessary to invoke an action service handler.

Arguments:

	CalledActionId - Supplies the action id of the action to execute.

	ParamList - Supplies the list of actual parameters (and return values) for
	            the action.

	ParamCount - Supplies the count of source-level parameters to the action,
	             which does not need to match the parameter list size.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	PCNWACTION_DEFINITION CalledAction;
	size_t                ReturnCount;
	size_t                ActualParams;
	NWACTION_TYPE         ReturnTypes[ 3 ];
	PCNWACTION_TYPE       ParameterTypes;

	//
	// Unpack the action data from the data table.
	//
	// Note that we may have supplied fewer than the actual maximum
	// argument count here; we must only remove items from the variable
	// list based on the parameter list itself.
	//
	// Note also that the parameter list only includes single stack
	// cell sized items (i.e. an 'action' type does not appear at all,
	// and a 'vector' type appears as three floats).  There is no
	// direct connection here between the parameter list size and the
	// count of parameters that the script says it has provided.  If
	// there is an inconsistency, then the action service handler will
	// cause the stack to be offset (and the error will eventually
	// result in the program failing at a subsequent action call).
	//

	if (CalledActionId > m_ActionCount)
		throw gcnew Exception( "Invoked out of range action." );

	CalledAction   = &m_ActionDefs[ CalledActionId ];

	if (ParamCount < CalledAction->MinParameters)
		throw gcnew Exception( "Invoked action with too few parameters." );
	else if (ParamCount > CalledAction->NumParameters)
		throw gcnew Exception( "Invoked action with too many parameters." );

	ReturnTypes[ 0 ] = CalledAction->ReturnType;
	ParameterTypes   = CalledAction->ParameterTypes;

	switch (ReturnTypes[ 0 ])
	{

	case ACTIONTYPE_VECTOR:
		ReturnCount      = 3;
		ReturnTypes[ 0 ] = ACTIONTYPE_FLOAT;
		ReturnTypes[ 1 ] = ACTIONTYPE_FLOAT;
		ReturnTypes[ 2 ] = ACTIONTYPE_FLOAT;
		break;

	case ACTIONTYPE_VOID:
		ReturnCount = 0;
		break;

	default:
		ReturnCount = 1;
		break;

	}

	ActualParams = ParamList->size( );

	if (ReturnCount > ActualParams)
		throw gcnew Exception( "Return count exceeds real action parameter list size." );

#if NWSCRIPT_FAST_ACTION_CALLS

#if !NWSCRIPT_DIRECT_FAST_ACTION_CALLS
	//
	// Determine whether it is even optimal to use the 'fast' call mechanism.
	// If there are only a small number of VM stack operations, then performing
	// them individually has a lower overhead than the additional native to
	// managed transitions.
	//

	if (ParamList->size( ) < NWSCRIPT_FAST_CALL_THRESHOLD)
	{
		GenerateExecuteActionServiceSlow(
			CalledAction,
			ParamList,
			ParamCount,
			ReturnCount,
			ReturnTypes);

		return;
	}
#endif // !NWSCRIPT_DIRECT_FAST_ACTION_CALLS

	//
	// Now determine if we can make a fast or a slow call.  A fast call must
	// not involve any engine structure types.
	//

	for (size_t i = 0; i < ParamList->size( ); i += 1)
	{
		NWACTION_TYPE VarType = ParamList->at( i )->GetType( );

		if ((VarType >= ACTIONTYPE_ENGINE_0) &&
		    (VarType <= ACTIONTYPE_ENGINE_9))
		{
			//
			// We have encountered an engine structure type, emit a slow call.
			//

			GenerateExecuteActionServiceSlow(
				CalledAction,
				ParamList,
				ParamCount,
				ReturnCount,
				ReturnTypes);

			return;
		}
	}

	for (size_t i = 0; i < ReturnCount; i += 1)
	{
		NWACTION_TYPE VarType = ReturnTypes[ i ];

		if ((VarType >= ACTIONTYPE_ENGINE_0) &&
		    (VarType <= ACTIONTYPE_ENGINE_9))
		{
			//
			// We have encountered an engine structure type, emit a slow call.
			//

			GenerateExecuteActionServiceSlow(
				CalledAction,
				ParamList,
				ParamCount,
				ReturnCount,
				ReturnTypes);

			return;
		}
	}

	//
	// At this point, we have not found a variable type that would prevent us
	// from making a fast call.  Do so now.
	//

#if !NWSCRIPT_DIRECT_FAST_ACTION_CALLS

	GenerateExecuteActionServiceFast(
		CalledAction,
		ParamList,
		ParamCount,
		ReturnCount,
		ReturnTypes);

#else // !NWSCRIPT_DIRECT_FAST_ACTION_CALLS

	GenerateExecuteActionServiceFastDirect(
		CalledAction,
		ParamList,
		ParamCount,
		ReturnCount,
		ReturnTypes);


#endif // !NWSCRIPT_DIRECT_FAST_ACTION_CALLS

#else // NWSCRIPT_FAST_ACTION_CALLS

	GenerateExecuteActionServiceSlow(
		CalledAction,
		ParamList,
		ParamCount,
		ReturnCount,
		ReturnTypes);

#endif
}

void
NWScriptCodeGenerator::GenerateExecuteActionServiceSlow(
	__in PCNWACTION_DEFINITION CalledAction,
	__in NWNScriptLib::VariableWeakPtrVec * ParamList,
	__in size_t ParamCount,
	__in size_t ReturnCount,
	__in_ecount( ReturnCount ) PCNWACTION_TYPE ReturnTypes
	)
/*++

Routine Description:

	This routine emits any code necessary to invoke an action service handler.

	The generated code takes the slow path (with distinct calls to the push
	and pop intrinsics).

Arguments:

	CalledAction - Supplies the action descriptor of the action to invoke.

	ParamList - Supplies the list of actual parameters (and return values) for
	            the action.

	ParamCount - Supplies the count of source-level parameters to the action,
	             which does not need to match the parameter list size.

	ReturnCount - Supplies the count of return variables affected by the action
	              call.

	ReturnTypes - Supplies the list of return types.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
#if NWSCRIPT_SLOW_CALLS_VIA_STUB
	GenerateExecuteActionServiceSlowViaStub(
		CalledAction,
		ParamList,
		ParamCount,
		ReturnCount,
		ReturnTypes);
#else
	size_t        ActualParams;
	ILGenerator ^ ILGen;

	ILGen = m_ILGenCtx->Sub->ILGen;

	//
	// Now load all of the arguments for the subroutine.
	//
	// N.B.  We call a dispatch routine on the NWScriptProgram object,
	//       and not a method on the ProgramType.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );
#if NWSCRIPT_VIRT_TYPE_WORKAROUND
	ILGen->Emit( OpCodes::Castclass, m_HostInterfaceType );
#endif

	ActualParams = ParamList->size( );

	for (size_t i = ActualParams; i != ReturnCount; i -= 1)
		GenerateVMStackPush( ILGen, ParamList->at( i - 1 ) );

	//
	// Issue the action service dispatcher call, then unpack the
	// return value to our associated working local variables (if any).
	//

	ILGen->Emit( OpCodes::Ldc_I4, (Int32) CalledAction->ActionId );
	ILGen->Emit( OpCodes::Conv_U4 );
	ILGen->Emit( OpCodes::Ldc_I4, (Int32) ParamCount );
	ILGen->Emit( OpCodes::Conv_U4 );

	ILGen->Emit( OpCodes::Callvirt, m_ILGenCtx->MthIntrinsic_ExecuteActionService );

	for (size_t i = ReturnCount; i != 0; i -= 1)
	{
		if (ParamList->at( i - 1 )->GetType( ) != ReturnTypes[ i - 1 ])
			throw gcnew Exception( "Return type mismatch for action service routine invocation." );

		GenerateVMStackPop( ILGen, ParamList->at( i - 1 ) );
	}
#endif
}

void
NWScriptCodeGenerator::GenerateExecuteActionServiceFast(
	__in PCNWACTION_DEFINITION CalledAction,
	__in NWNScriptLib::VariableWeakPtrVec * ParamList,
	__in size_t ParamCount,
	__in size_t ReturnCount,
	__in_ecount( ReturnCount ) PCNWACTION_TYPE ReturnTypes
	)
/*++

Routine Description:

	This routine emits any code necessary to invoke an action service handler.

	The generated code takes the fast path (compressed to a single transition
	to native code).

Arguments:

	CalledAction - Supplies the action descriptor of the action to invoke.

	ParamList - Supplies the list of actual parameters (and return values) for
	            the action.

	ParamCount - Supplies the count of source-level parameters to the action,
	             which does not need to match the parameter list size.

	ReturnCount - Supplies the count of return variables affected by the action
	              call.

	ReturnTypes - Supplies the list of return types.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	size_t            ActualParams;
	ILGenerator     ^ ILGen;
	array< Type ^ > ^ ArgTypes;
	Int32             VarIdx;
	LocalBuilder    ^ Loc;

	ILGen = m_ILGenCtx->Sub->ILGen;

	//
	// Now load all of the arguments for the subroutine.
	//
	// N.B.  We call a dispatch routine on the NWScriptProgram object,
	//       and not a method on the ProgramType.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );
#if NWSCRIPT_VIRT_TYPE_WORKAROUND
	ILGen->Emit( OpCodes::Castclass, m_HostInterfaceType );
#endif

	//
	// Issue the action service dispatcher call, then unpack the
	// return value to our associated working local variables (if any).
	//

	ILGen->Emit( OpCodes::Ldc_I4, (Int32) CalledAction->ActionId );
	ILGen->Emit( OpCodes::Conv_U4 );
	ILGen->Emit( OpCodes::Ldc_I4, (Int32) ParamCount );
	ILGen->Emit( OpCodes::Conv_U4 );

	ActualParams = ParamList->size( );
	ArgTypes     = gcnew array< Type ^ >( (Int32) (ActualParams - ReturnCount) );
	VarIdx       = 0;
	Loc          = AcquireLocal( array< Object ^ >::typeid );

	ILGen->Emit( OpCodes::Ldc_I4, (Int32) (ActualParams - ReturnCount) );
	ILGen->Emit( OpCodes::Newarr, Object::typeid );
	ILGen->Emit( OpCodes::Stloc, Loc );

	for (size_t i = ReturnCount; i < ActualParams; i += 1)
	{
		NWScriptVariable * Var = ParamList->at( i );
		ArgTypes[ VarIdx ]     = GetVariableType( Var );

		ILGen->Emit( OpCodes::Ldloc, Loc );
		ILGen->Emit( OpCodes::Ldc_I4, (Int32) VarIdx++ );
		GenerateLoadVariable( Var, true );
		ILGen->Emit( OpCodes::Stelem_Ref );
	}

	ILGen->Emit( OpCodes::Ldloc, Loc );

	ILGen->EmitCall(
		OpCodes::Callvirt,
		m_ILGenCtx->MthIntrinsic_ExecuteActionServiceFast,
		ArgTypes);

	ILGen->Emit( OpCodes::Ldnull );
	ILGen->Emit( OpCodes::Stloc, Loc );

	ReleaseLocal( Loc );

	//
	// Now unpack the return value.
	//

	for (size_t i = ReturnCount; i != 0; i -= 1)
	{
		if (ParamList->at( i - 1 )->GetType( ) != ReturnTypes[ i - 1 ])
			throw gcnew Exception( "Return type mismatch for action service routine invocation." );

		switch (ReturnTypes[ i - 1 ])
		{

		case ACTIONTYPE_INT:
			GenerateStoreVariable( ParamList->at( i - 1 ), true );
			break;

		case ACTIONTYPE_FLOAT:
			if (ReturnCount == 1)
				GenerateStoreVariable( ParamList->at( i - 1 ), true );
			else
			{
				//
				// We have a vector3, unpack it.
				//

				switch (i)
				{

				case 3:
					Loc = AcquireLocal( NWScript::Vector3::typeid );

					ILGen->Emit( OpCodes::Unbox_Any, NWScript::Vector3::typeid );
					ILGen->Emit( OpCodes::Stloc, Loc );
					ILGen->Emit( OpCodes::Ldloc, Loc );
					ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldVector3_z );
					GenerateStoreVariable( ParamList->at( i - 1 ) );
					break;

				case 2:
					ILGen->Emit( OpCodes::Ldloc, Loc );
					ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldVector3_y );
					GenerateStoreVariable( ParamList->at( i - 1 ) );
					break;

				case 1:
					ILGen->Emit( OpCodes::Ldloc, Loc );
					ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldVector3_x );
					GenerateStoreVariable( ParamList->at( i - 1 ) );
					ReleaseLocal( Loc );
					break;

				default:
					throw gcnew Exception( "Invalid vector (too many components) returned from action service handler." );

				}
			}
			break;

		case ACTIONTYPE_OBJECT:
			GenerateStoreVariable( ParamList->at( i - 1 ), true );
			break;

		case ACTIONTYPE_STRING:
			GenerateStoreVariable( ParamList->at( i - 1 ), true );
			break;

		default:
			throw gcnew Exception( "Illegal return type for action service handler." );

		}
	}

	//
	// Remove the dummy return value from the stack if there was no actual
	// return value.
	//

	if (ReturnCount == 0)
		ILGen->Emit( OpCodes::Pop );
}

#if NWSCRIPT_DIRECT_FAST_ACTION_CALLS

void
NWScriptCodeGenerator::GenerateExecuteActionServiceFastDirect(
	__in PCNWACTION_DEFINITION CalledAction,
	__in NWNScriptLib::VariableWeakPtrVec * ParamList,
	__in size_t ParamCount,
	__in size_t ReturnCount,
	__in_ecount( ReturnCount ) PCNWACTION_TYPE ReturnTypes
	)
/*++

Routine Description:

	This routine emits any code necessary to invoke an action service handler.

	The generated code takes the fast path (compressed to a single transition
	to native code).  A direct call to the raw interface method is made, unlike
	via GenerateExecuteActionServiceFast (which requires boxing and indirection
	through a GC array).

	N.B.  This routine invokes non-verifiable emitted code !

Arguments:

	CalledAction - Supplies the action descriptor of the action to invoke.

	ParamList - Supplies the list of actual parameters (and return values) for
	            the action.

	ParamCount - Supplies the count of source-level parameters to the action,
	             which does not need to match the parameter list size.

	ReturnCount - Supplies the count of return variables affected by the action
	              call.

	ReturnTypes - Supplies the list of return types.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	size_t            ActualParams;
	ILGenerator     ^ ILGen;
	LocalBuilder    ^ Loc;
	MethodInfo      ^ DirectCallStub;

	ILGen        = m_ILGenCtx->Sub->ILGen;
	ActualParams = ParamList->size( );

	//
	// Acquire the direct call stub to use here.
	//

	DirectCallStub = AcquireFastDirectActionServiceStub(
		CalledAction,
		ActualParams - ReturnCount,
		ParamCount);

	//
	// Now load all of the arguments for the subroutine.
	//
	// N.B.  We call a dispatch routine on the ProgramType object,
	//       and not a method on the NWScriptProgram.  The dispatch routine
	//       (the stub) will call the raw native interface after packaging the
	///      parameter list appropriately.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );

	for (size_t i = ReturnCount; i < ActualParams; i += 1)
	{
		NWScriptVariable * Var = ParamList->at( i );

		GenerateLoadVariable( Var );
	}

	ILGen->Emit( OpCodes::Call, DirectCallStub );

	//
	// Now unpack the return value.
	//

	for (size_t i = ReturnCount; i != 0; i -= 1)
	{
		if (ParamList->at( i - 1 )->GetType( ) != ReturnTypes[ i - 1 ])
			throw gcnew Exception( "Return type mismatch for action service routine invocation." );

		switch (ReturnTypes[ i - 1 ])
		{

		case ACTIONTYPE_INT:
			GenerateStoreVariable( ParamList->at( i - 1 ) );
			break;

		case ACTIONTYPE_FLOAT:
			if (ReturnCount == 1)
				GenerateStoreVariable( ParamList->at( i - 1 ) );
			else
			{
				//
				// We have a vector3, unpack it.
				//

				switch (i)
				{

				case 3:
					Loc = AcquireLocal( NWScript::Vector3::typeid );

					ILGen->Emit( OpCodes::Stloc, Loc );
					ILGen->Emit( OpCodes::Ldloc, Loc );
					ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldVector3_z );
					GenerateStoreVariable( ParamList->at( i - 1 ) );
					break;

				case 2:
					ILGen->Emit( OpCodes::Ldloc, Loc );
					ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldVector3_y );
					GenerateStoreVariable( ParamList->at( i - 1 ) );
					break;

				case 1:
					ILGen->Emit( OpCodes::Ldloc, Loc );
					ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldVector3_x );
					GenerateStoreVariable( ParamList->at( i - 1 ) );
					ReleaseLocal( Loc );
					break;

				default:
					throw gcnew Exception( "Invalid vector (too many components) returned from action service handler." );

				}
			}
			break;

		case ACTIONTYPE_OBJECT:
			GenerateStoreVariable( ParamList->at( i - 1 ) );
			break;

		case ACTIONTYPE_STRING:
			GenerateStoreVariable( ParamList->at( i - 1 ) );
			break;

		default:
			throw gcnew Exception( "Illegal return type for action service handler." );

		}
	}
}

MethodInfo ^
NWScriptCodeGenerator::AcquireFastDirectActionServiceStub(
	__in PCNWACTION_DEFINITION CalledAction,
	__in size_t ParamCount,
	__in size_t SrcParams
	)
/*++

Routine Description:

	This routine returns a method descriptor for a method that can be invoked
	in order to execute a particular action service with a given number of
	arguments.  The method will contain code to directly call the devirtualized
	fast JIT action service dispatcher in the native INWScriptActions
	interface.

	If such a method has already been generated for a given action service
	number and parameter count combination, it is returned, else a new method
	is created.

	N.B.  Any methods created are not verifiable in the emitted code !

Arguments:

	CalledAction - Supplies the action descriptor of the action to invoke.

	ParamCount - Supplies the count of actual parameters to the action, which
	             includes one float for each vector component.

	SrcParams - Supplies the count of source level arguments to the action,
	            which counts 1 for each vector and 1 for each action.

Return Value:

	The method descriptor is returned.
	
	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	array< ActionServiceMethodInfo > ^ StubArray;
	array< ActionServiceMethodInfo > ^ NewArray;
	MethodInfo                       ^ Mth;

	//
	// First, check if an existing method matched.
	//

	StubArray = m_ILGenCtx->MthActionServiceStubs[ CalledAction->ActionId ];

	if (StubArray != nullptr)
	{
		for (Int32 i = 0; i < StubArray->Length; i += 1)
		{
			if (StubArray[ i ].NumParams == (Int32) ParamCount)
				return StubArray[ i ].Method;
		}
	}

	//
	// No existing method will fit, we'll need to create one.
	//

	Mth = GenerateFastDirectActionServiceStub(
		CalledAction,
		ParamCount,
		SrcParams);

	//
	// Cache it for future use and return to the caller.
	//
	// N.B.  Most script programs only have one action service handler at most
	//       per action id.  Current compilers always supply all default
	//       arguments at compile time, but for generality's sake, we allow a
	//       mix.
	//

	NewArray = gcnew array< ActionServiceMethodInfo >( StubArray == nullptr ? 1 : StubArray->Length + 1 );

	if (StubArray != nullptr)
		StubArray->CopyTo( NewArray, 0 );

	NewArray[ NewArray->Length - 1 ].Method    = Mth;
	NewArray[ NewArray->Length - 1 ].NumParams = (Int32) ParamCount;

	m_ILGenCtx->MthActionServiceStubs[ CalledAction->ActionId ] = NewArray;

	return Mth;
}

MethodInfo ^
NWScriptCodeGenerator::GenerateFastDirectActionServiceStub(
	__in PCNWACTION_DEFINITION CalledAction,
	__in size_t ParamCount,
	__in size_t SrcParams
	)
/*++

Routine Description:

	This routine generates an direct action service handler call stub method
	for a given action and parameter count mix.  The stub can be invoked to
	transfer control to the action service handler with a minimum of overhead.

	First, a value type describing the command array is created on the stack,
	and then initialized.  Then, underlying
	INWScriptAction::OnExecuteActionFromJITFast interface function is called
	directly in a devirtualized fashion.

	N.B.  Any methods created are not verifiable in the emitted code !

Arguments:

	CalledAction - Supplies the action descriptor of the action to invoke.

	ParamCount - Supplies the count of actual parameters to the action, which
	             includes one float for each vector component.

	SrcParams - Supplies the count of source level arguments to the action,
	            which counts 1 for each vector and 1 for each action.

Return Value:

	The method descriptor is returned.
	
	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	MethodBuilder           ^ MthActionService;
	TypeBuilder             ^ CmdDescriptorTypeBuilder;
	Type                    ^ CmdDescriptorType;
	String                  ^ Name;
	array< Type ^ >         ^ ArgTypes;
	array< FieldBuilder ^ > ^ ParamFields;
	array< FieldBuilder ^ > ^ ReturnFields;
	array< FieldBuilder ^ > ^ ReturnStorageFields;
	array< FieldBuilder ^ > ^ CmdFields;
	array< FieldBuilder ^ > ^ StringFields;
	array< NWACTION_TYPE >  ^ ParamRawTypes;
	FieldInfo               ^ ParamArrayBaseField;
	size_t                    ParamIndex;
	Int32                     i;
	Int32                     StringCount;
	Int32                     CurString;
	size_t                    ReturnCount;
	NWACTION_TYPE             ReturnType;
	Type                    ^ MSILReturnType;
	ILGenerator             ^ ILGen;
	LocalBuilder            ^ RetLocal;
	LocalBuilder            ^ CmdDescLocal;
	LocalBuilder            ^ CmdDescLocalPinPtr;
	Label                     ContinueLabel;
	bool                      NeedExceptBlock;
	SubroutineGenContext    ^ Sub;

	//
	// Determine the actual number of slots in the command descriptor array for
	// the return type.
	//

	ReturnType = CalledAction->ReturnType;

	switch (ReturnType)
	{

	case ACTIONTYPE_VOID:
		ReturnCount    = 0;
		MSILReturnType = void::typeid;
		break;

	case ACTIONTYPE_VECTOR:
		ReturnCount    = 3;
		MSILReturnType = NWScript::Vector3::typeid;
		ReturnType     = ACTIONTYPE_FLOAT;
		break;

	default:
		ReturnCount    = 1;
		MSILReturnType = GetVariableType( ReturnType );
		break;

	}

	//
	// Name the action service and its internal descriptor type.
	//

	Name  = "ExecuteActionService_" + (Int32) CalledAction->ActionId;
	Name += "_";
	Name += (Int32) ParamCount;

	__if_exists( NWACTION_DEFINITION::Name )
	{
		Name += "_";
		Name += gcnew System::String( CalledAction->Name );
	}

	//
	// Now create the command descriptor type.  The command descriptor type is
	// actually a pair of arrays comprising storage for the Cmds and CmdParams
	// arguments for a call to INWScriptActions::OnExecuteActionFromJITFast.
	//
	// As such, it it is a sequential layout value type whose fields are to be
	// interpreted as the Cmds and then CmdParams arrays.
	//

	CmdDescriptorTypeBuilder = m_ILGenCtx->ProgramModule->DefineType(
		m_ILGenCtx->Namespace + "DirectActionServiceCmdDescriptors." + Name,
		TypeAttributes::NotPublic | TypeAttributes::SequentialLayout | TypeAttributes::Sealed,
		ValueType::typeid);

	ArgTypes            = gcnew array< Type ^ >( (Int32) ParamCount );
	ParamFields         = gcnew array< FieldBuilder ^ >( (Int32) ParamCount );
	ReturnFields        = gcnew array< FieldBuilder ^ >( (Int32) ReturnCount );
	ReturnStorageFields = gcnew array< FieldBuilder ^ >( (Int32) ReturnCount );
	CmdFields           = gcnew array< FieldBuilder ^ >( 1 + (Int32) ParamCount + (Int32) ReturnCount );
	ParamRawTypes       = gcnew array< NWACTION_TYPE >( (Int32) ParamCount );

	//
	// Create the command fields, which are actually the Cmds array.
	//

	for (i = 0; i < CmdFields->Length; i += 1)
	{
		CmdFields[ i ] = CmdDescriptorTypeBuilder->DefineField(
			"Cmd_" + i,
			UInt32::typeid,
			FieldAttributes::Public);
	}

#if defined(_WIN64)
	//
	// If we had an odd number of commands, we'll need to create a padding
	// field between the first command and the start of the remaining
	// fields.  This keeps the remainder of the structure natural aligned on a
	// pointer-sized value, which is critical as the structure will then be
	// treated as an array by native code.
	//
	// If we did not insert the padding field here and we had an odd number of
	// items, one might be created between the first non-pointer member of the
	// remainder of the structure and the first pointer member, which would
	// offset the 'array' by 4 bytes.
	//

	if (CmdFields->Length % 2)
	{
		CmdDescriptorTypeBuilder->DefineField(
			"CmdPadding_Tail",
			UInt32::typeid,
			FieldAttributes::Public);
	}
#endif

	//
	// Now prepare to create the command parameter fields, which are actually
	// the CmdParams array.
	//
	// N.B.  The argument order has already been swapped in the argument array
	//       for ease of matching with the action descriptor.
	//

	//
	// First, build the array of raw types, expanding aggregates.  The array is
	// constructed from end to start, such that it is in the order that the
	// commands must be.
	//
	// Also, construct the argument array to the stub while we're at it.  The
	// stub receives arguments in their usual left to right order, and reverses
	// that order when filling out the command array.
	//

	ParamIndex  = 0;
	i           = (Int32) ParamCount;

	for (Int32 n = 0; n < (Int32) ParamCount; n += 1)
	{
		NWACTION_TYPE ParamType;

		if (ParamIndex >= CalledAction->NumParameters)
			throw gcnew Exception( "Invoked action with too many parameters." );

		ParamType = CalledAction->ParameterTypes[ ParamIndex ];

		switch (ParamType)
		{

		case ACTIONTYPE_ACTION:
			ParamIndex += 1;
			n -= 1;

			continue;

		case ACTIONTYPE_VECTOR:
			if (i - 3 < 0)
				throw gcnew Exception( "Passed partial vector to action service handler." );

			for (Int32 ii = 0; ii < 3; ii += 1)
			{
				i -= 1;

				ParamRawTypes[ i ] = ACTIONTYPE_FLOAT;

				ArgTypes[ n + ii ] = GetVariableType( ACTIONTYPE_FLOAT );
			}

			n          += 2; // +1 after loop
			ParamIndex += 1;
			break;

		default:
			i -= 1;
			ParamIndex += 1;

			ParamRawTypes[ i ] = ParamType;

			ArgTypes[ n ] = GetVariableType( ParamType );
			break;

		}
	}

	//
	// Now create the underlying parameter fields on the descriptor type.
	//

	StringCount = 0;
	CurString   = 0;

	for (i = 0; i < (Int32) ParamCount; i += 1)
	{
		Type ^ NativeType;
		bool   Padding;

		Padding = false;

		switch (ParamRawTypes[ i ])
		{

		case ACTIONTYPE_INT:
			NativeType = uintptr_t::typeid;
			break;

		case ACTIONTYPE_FLOAT:
			NativeType = float::typeid;

			//
			// We need 32 bits of padding on 64-bit after each float so that
			// the array is correctly aligned.
			//

#ifdef _WIN64
			Padding = true;
#endif
			break;

		case ACTIONTYPE_OBJECT:
			NativeType = uintptr_t::typeid;
			break;

		case ACTIONTYPE_STRING:
			NativeType = NWScript::NeutralStringStorage::typeid->MakePointerType( );

			StringCount += 1;
			break;

		default:
			throw gcnew Exception( "Illegal raw parameter type for direct fast action service call." );

		}

		ParamFields[ i ] = CmdDescriptorTypeBuilder->DefineField(
			"CmdParam_" + i,
			NativeType,
			FieldAttributes::Public);

		//
		// If we need padding to align on a uintptr_t boundary, create it now.
		//

		if (Padding)
		{
			CmdDescriptorTypeBuilder->DefineField(
				"CmdParamPad_" + i,
				Int32::typeid,
				FieldAttributes::Public);
		}
	}

	//
	// Now create the return type pointers.
	//

	for (i = 0; i < (Int32) ReturnCount; i += 1)
	{
		Type ^ NativeType;

		switch (ReturnType)
		{

		case ACTIONTYPE_VECTOR:
		case ACTIONTYPE_FLOAT:
			NativeType = float::typeid->MakePointerType( );
			break;

		case ACTIONTYPE_INT:
		case ACTIONTYPE_OBJECT:
			NativeType = uintptr_t::typeid->MakePointerType( );
			break;

		case ACTIONTYPE_STRING:
			NativeType = NWScript::NeutralStringStorage::typeid->MakePointerType( );

			StringCount += 1;
			break;

		}

		ReturnFields[ i ] = CmdDescriptorTypeBuilder->DefineField(
			"CmdParam_Ret_" + (i + (Int32) ParamCount),
			NativeType,
			FieldAttributes::Public);
	}

	//
	// Now we are done building the CmdParams array.  Miscellaneous fields can
	// follow.
	//

	//
	// Generate storage fields for return values.  We emitted pointers to them
	// in the command descriptor array above, now we need to emit their actual
	// backing storage.
	//

	for (i = 0; i < (Int32) ReturnCount; i += 1)
	{
		Type ^ NativeType;

		switch (ReturnType)
		{

		case ACTIONTYPE_VECTOR:
		case ACTIONTYPE_FLOAT:
			NativeType = float::typeid;
			break;

		case ACTIONTYPE_INT:
		case ACTIONTYPE_OBJECT:
			NativeType = uintptr_t::typeid;
			break;

		case ACTIONTYPE_STRING:
			//
			// If we return a string, we place it with the rest of the string
			// fields as we need to do auto cleanup for it.
			//

			if (i != 0)
				throw gcnew Exception( "Only single string return values are supported." );

			continue;

		}

		ReturnStorageFields[ i ] = CmdDescriptorTypeBuilder->DefineField(
			"CmdParam_RetStorage_" + i,
			NativeType,
			FieldAttributes::Public);
	}

	NeedExceptBlock = false;

	//
	// Now set up the actual string storage.  The string parameters will point
	// to these fields.  We keep it all in one value structure for simplicity.
	//

	if (StringCount != 0)
	{
		StringFields = gcnew array< FieldBuilder ^ >( StringCount );

		for (i = 0; i < StringCount; i += 1)
		{
			StringFields[ i ] = CmdDescriptorTypeBuilder->DefineField(
				"StringStorage_" + i,
				NWScript::NeutralStringStorage::typeid,
				FieldAttributes::Public);
		}

		//
		// If we returned a string, then the last string field provides the
		// storage for the return type.  We keep the return string with the
		// storage for the parameter strings, so that we can clean all of the
		// strings up in the exception handler easily.
		//

		if ((ReturnCount == 1) && (ReturnType == ACTIONTYPE_STRING))
			ReturnStorageFields[ 0 ] = StringFields[ StringCount - 1 ];

		//
		// If we are dealing with strings, then we need to emit an exception
		// block that can release the native resources properly.
		//

		NeedExceptBlock = true;
	}

	//
	// We're done creating the command descriptor type, finalize it and start
	// constructing the stub method itself.
	//

	CmdDescriptorType = CmdDescriptorTypeBuilder->CreateType( );
	Sub               = m_ILGenCtx->Sub;
	MthActionService  = BeginNewSubroutine(
		Name,
		m_ILGenCtx->ActionServiceStubAttributes,
		MSILReturnType,
		ArgTypes);

	ILGen = MthActionService->GetILGenerator( );

	if (ReturnType != ACTIONTYPE_VOID)
	{
		RetLocal = ILGen->DeclareLocal( MSILReturnType );

		//
		// If we returned a NWScript::Vector3, then initialize it.
		//

		if ((ReturnCount == 3) && (ReturnType == ACTIONTYPE_FLOAT))
		{
//			ILGen->Emit( OpCodes::Ldloca, RetLocal );
//			ILGen->Emit( OpCodes::Initobj, MSILReturnType );
		}
	}

	CmdDescLocal = ILGen->DeclareLocal( CmdDescriptorType );
	CmdDescLocalPinPtr = ILGen->DeclareLocal(
		CmdDescriptorType->MakeByRefType( ),
		true );

	ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );
	ILGen->Emit( OpCodes::Stloc, CmdDescLocalPinPtr );

//	ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );
//	ILGen->Emit( OpCodes::Initobj, CmdDescriptorType );

	//
	// Create the exception block if we need one for cleanup here.
	//

	if (NeedExceptBlock)
		ILGen->BeginExceptionBlock( );

	//
	// Now initialize the input parameter command descriptor and command code
	// arrays.
	//

	for (i = 0; i < (Int32) ParamCount; i += 1)
	{
		Int32 Cmd;
		short ArgSlot;

		switch (ParamRawTypes[ i ])
		{

		case ACTIONTYPE_FLOAT:
			Cmd = NWFASTACTION_PUSHFLOAT;
			break;

		case ACTIONTYPE_INT:
			Cmd = NWFASTACTION_PUSHINT;
			break;

		case ACTIONTYPE_OBJECT:
			Cmd = NWFASTACTION_PUSHOBJECTID;
			break;

		case ACTIONTYPE_STRING:
			Cmd = NWFASTACTION_PUSHSTRING;
			break;

		default:
			throw gcnew Exception( "Illegal raw parameter type for setting up direct fast action service command descriptor." );

		}

		//
		// Assign the command for this array position.
		//

		ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );

		if (Cmd <= 255)
			ILGen->Emit( OpCodes::Ldc_I4_S, (Byte) Cmd );
		else
			ILGen->Emit( OpCodes::Ldc_I4, Cmd );

		ILGen->Emit( OpCodes::Conv_U4 );
		ILGen->Emit( OpCodes::Stfld, CmdFields[ i ] );

		//
		// Now prepare the corresponding parameter.  Note that the first entry
		// pushed onto the stack is the last parameter, and our real parameters
		// start at 1 (0 being this).
		//

		ArgSlot = (short) (ParamCount - i);

		ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );

		if (ParamRawTypes[ i ] == ACTIONTYPE_STRING)
		{
			ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );
			ILGen->Emit( OpCodes::Ldarg_0 );
			ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );
		}

		if (ArgSlot <= 255)
			ILGen->Emit( OpCodes::Ldarg_S, (Byte) ArgSlot );
		else
			ILGen->Emit( OpCodes::Ldarg, ArgSlot );

		switch (ParamRawTypes[ i ])
		{

		case ACTIONTYPE_INT:
		case ACTIONTYPE_OBJECT:
			//
			// Map to unsigned of the right type.
			//

#ifdef _WIN64
			ILGen->Emit( OpCodes::Conv_U8 );
#else
			ILGen->Emit( OpCodes::Conv_U4 );
#endif
			break;

		case ACTIONTYPE_FLOAT:
			//
			// No conversion is needed.  We don't bother to initialize the
			// padding adjacent (if there was any).
			//

			break;

		case ACTIONTYPE_STRING:
			//
			// Call the conversion function.
			//

			ILGen->Emit( OpCodes::Callvirt, m_ILGenCtx->MthIntrinsic_AllocateNeutralString );
			ILGen->Emit( OpCodes::Stfld, StringFields[ CurString ] );
			ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );
			ILGen->Emit( OpCodes::Ldflda, StringFields[ CurString ] );

			CurString += 1;
			break;

		default:
			throw gcnew Exception( "Illegal raw parameter type for setting up direct fast action service command descriptor." );

		}

		ILGen->Emit( OpCodes::Stfld, ParamFields[ i ] );
	}

	//
	// Emit the call directive into the command list.
	//

	ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4127) // warning C4127: conditional expression is constant
#endif

	if (NWFASTACTION_CALL <= 255)
		ILGen->Emit( OpCodes::Ldc_I4_S, (Byte) NWFASTACTION_CALL );
	else
		ILGen->Emit( OpCodes::Ldc_I4, (Int32) NWFASTACTION_CALL );

#ifdef _MSC_VER
#pragma warning(pop)
#endif

	ILGen->Emit( OpCodes::Stfld, CmdFields[ (Int32) ParamCount ] );

	//
	// Now initialize the return value command descriptor and command code
	// arrays.
	//

	for (i = 0; i < (Int32) ReturnCount; i += 1)
	{
		Int32 Cmd;

		switch (ReturnType)
		{

		case ACTIONTYPE_FLOAT:
			Cmd = NWFASTACTION_POPFLOAT;
			break;

		case ACTIONTYPE_INT:
			Cmd = NWFASTACTION_POPINT;
			break;

		case ACTIONTYPE_OBJECT:
			Cmd = NWFASTACTION_POPOBJECTID;
			break;

		case ACTIONTYPE_STRING:
			Cmd = NWFASTACTION_POPSTRING;
			break;

		default:
			throw gcnew Exception( "Illegal return type for setting up direct fast action service command descriptor." );

		}

		//
		// Assign the command for this array position.
		//

		ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );

		if (Cmd <= 255)
			ILGen->Emit( OpCodes::Ldc_I4_S, (Byte) Cmd );
		else
			ILGen->Emit( OpCodes::Ldc_I4, Cmd );

		ILGen->Emit( OpCodes::Conv_U4 );
		ILGen->Emit( OpCodes::Stfld, CmdFields[ (Int32) ParamCount + 1 + i ] );

		//
		// Now prepare the corresponding return value pointers.  These reside
		// within the command parameter array and need to point to the actual
		// storage that we have set aside for the return values.
		//

		ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );
		ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );
		ILGen->Emit( OpCodes::Ldflda, ReturnStorageFields[ i ] );
#ifdef _WIN64
		ILGen->Emit( OpCodes::Conv_U8 );
#else
		ILGen->Emit( OpCodes::Conv_U4 );
#endif
		ILGen->Emit( OpCodes::Stfld, ReturnFields[ i ] );
	}

	//
	// Determine which field we should take the address of to form the base
	// parameter array (CmdParams).
	//
	// If we have any parameters, it will be the first parameter field.  If
	// we have no parameters but have a return value, it will be the first
	// return pointer field.  Otherwise, null is supplied.
	//

	if (ParamCount != 0)
		ParamArrayBaseField = ParamFields[ 0 ];
	else if (ReturnCount != 0)
		ParamArrayBaseField = ReturnFields[ 0 ];
	else
		ParamArrayBaseField = nullptr;

	//
	// Finally, we are ready to emit the call to the action service handler's
	// underlying implementation.  Create a devirtualized indirect call to the
	// native method.
	//
	// Note that we can hardcode both 'this' and the call target because the
	// INWScriptActions interface cannot change while the NWScriptProgram
	// object is live.
	//

#ifdef _WIN64
	ILGen->Emit( OpCodes::Ldc_I8, (Int64) m_ActionHandler );             // this
#else
	ILGen->Emit( OpCodes::Ldc_I4, (Int32) m_ActionHandler );    
#endif

	ILGen->Emit( OpCodes::Ldc_I4, (Int32) CalledAction->ActionId );      // ActionId
	ILGen->Emit( OpCodes::Conv_U4 );
#ifdef _WIN64
	ILGen->Emit( OpCodes::Ldc_I8, (Int64) SrcParams );                   // NumArguments
	ILGen->Emit( OpCodes::Conv_U8 );
#else
	ILGen->Emit( OpCodes::Ldc_I4, (Int32) SrcParams );
	ILGen->Emit( OpCodes::Conv_U4 );
#endif
	ILGen->Emit( OpCodes::Ldloc, CmdDescLocalPinPtr );                   // Cmds
#ifdef _WIN64
	ILGen->Emit( OpCodes::Conv_U8 );
#else
	ILGen->Emit( OpCodes::Conv_U4 );
#endif
//	ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );
//	ILGen->Emit( OpCodes::Ldflda, CmdFields[ 0 ] );                      // Cmds
#ifdef _WIN64
	ILGen->Emit( OpCodes::Ldc_I8, (Int64) CmdFields->Length );           // NumCmds
	ILGen->Emit( OpCodes::Conv_U8 );
#else
	ILGen->Emit( OpCodes::Ldc_I4, (Int32) CmdFields->Length );
	ILGen->Emit( OpCodes::Conv_U4 );
#endif

	if (ParamArrayBaseField != nullptr)
	{
		ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );                    // CmdParams
		ILGen->Emit( OpCodes::Ldflda, ParamArrayBaseField );
	}
	else
	{
		ILGen->Emit( OpCodes::Ldnull );
	}

	//
	// Now load the (devirtualized) call site.
	//

#ifdef _WIN64
	ILGen->Emit( OpCodes::Ldc_I8, (Int64) m_ILGenCtx->PtrOnExecuteActionFromJITFast );
	ILGen->Emit( OpCodes::Conv_U8 );
#else
	ILGen->Emit( OpCodes::Ldc_I4, (Int32) m_ILGenCtx->PtrOnExecuteActionFromJITFast );
	ILGen->Emit( OpCodes::Conv_U4 );
#endif

	//
	// Finally, emit the call.  Note that the virtual interface is declared as
	// stdcall, so this is not passed using the thiscall convention (even on
	// x86).
	//

	ILGen->EmitCalli(
		OpCodes::Calli,
		CallingConvention::StdCall,
		bool::typeid,
		gcnew array< Type ^ > {
#ifdef _WIN64
			Int64::typeid,
#else
			Int32::typeid,
#endif
			NWSCRIPT_ACTION::typeid,
			size_t::typeid,
			void::typeid->MakePointerType( ),
			size_t::typeid,
			uintptr_t::typeid->MakePointerType( )
			}
		);

	ContinueLabel = ILGen->DefineLabel( );

	//
	// We must abort the script program directly if the invocation failed.  In
	// that case, emit the code to generate the exception here (if the action
	// service handler dispatcher returned false).
	//

	ILGen->Emit( OpCodes::Brtrue_S, ContinueLabel );

	GenerateThrowException( "Action service handler invocation failed." );
	ILGen->MarkLabel( ContinueLabel );

	//
	// Now push the (MSIL) return value onto the stack.
	//

	switch (ReturnCount)
	{

	case 0:
		break;

	case 1:
		//
		// If it's a string, we must convert it to the CLR form.  Otherwise, we
		// directly pass through (though with a minor conversion in the case of
		// integral types).
		//

		if (ReturnType == ACTIONTYPE_STRING)
		{
			ILGen->Emit( OpCodes::Ldarg_0 );
			ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );
			ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );
			ILGen->Emit( OpCodes::Ldflda, ReturnStorageFields[ 0 ] );
			ILGen->Emit( OpCodes::Callvirt, m_ILGenCtx->MthIntrinsic_NeutralStringToString );
		}
		else
		{
			ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );
			ILGen->Emit( OpCodes::Ldfld, ReturnStorageFields[ 0 ] );

			if (ReturnType == ACTIONTYPE_INT)
				ILGen->Emit( OpCodes::Conv_I4 );
			else if (ReturnType == ACTIONTYPE_OBJECT)
				ILGen->Emit( OpCodes::Conv_U4 );
		}

		ILGen->Emit( OpCodes::Stloc, RetLocal );

		break;

	case 3:
		//
		// We returned a vector.  Package it up into a NWScript::Vector3 value
		// type and return that.
		//

		ILGen->Emit( OpCodes::Ldloca, RetLocal );
		ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );
		ILGen->Emit( OpCodes::Ldfld, ReturnStorageFields[ 0 ] );
		ILGen->Emit( OpCodes::Stfld, m_ILGenCtx->FldVector3_z );

		ILGen->Emit( OpCodes::Ldloca, RetLocal );
		ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );
		ILGen->Emit( OpCodes::Ldfld, ReturnStorageFields[ 1 ] );
		ILGen->Emit( OpCodes::Stfld, m_ILGenCtx->FldVector3_y );

		ILGen->Emit( OpCodes::Ldloca, RetLocal );
		ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );
		ILGen->Emit( OpCodes::Ldfld, ReturnStorageFields[ 2 ] );
		ILGen->Emit( OpCodes::Stfld, m_ILGenCtx->FldVector3_x );

		break;
	}

	//
	// Now emit code to clean up any strings (if we had any).
	//

	for (i = 0; i < StringCount; i += 1)
	{
		ILGen->Emit( OpCodes::Ldarg_0 );
		ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );
		ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );
		ILGen->Emit( OpCodes::Ldflda, StringFields[ i ] );
		ILGen->Emit( OpCodes::Callvirt, m_ILGenCtx->MthIntrinsic_DeleteNeutralString );
	}

	//
	// If we need an exception block to perform resource cleanup on our native
	// resources, create it here and now.
	//
	// Currently, this is only the case for strings, which are actually native
	// heap pointers that must be released.
	//

	if (NeedExceptBlock)
	{
		ILGen->BeginCatchBlock( Object::typeid );
		ILGen->Emit( OpCodes::Pop );

		//
		// Now delete any strings we've created.  Note that calling the delete
		// intrinsic is safe even if the string was zero initialized and never
		// allocated.  If we were to free it, the free intrinsic automagically
		// sets the string to be zeroed so that a double free is harmless.
		//

		for (i = 0; i < StringCount; i += 1)
		{
			ILGen->Emit( OpCodes::Ldarg_0 );
			ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );
			ILGen->Emit( OpCodes::Ldloca, CmdDescLocal );
			ILGen->Emit( OpCodes::Ldflda, StringFields[ i ] );
			ILGen->Emit( OpCodes::Callvirt, m_ILGenCtx->MthIntrinsic_DeleteNeutralString );
		}

		//
		// Finally, let the exception continue upwards now that we have cleaned
		// up.
		//

		ILGen->Emit( OpCodes::Rethrow );

		//
		// This code is unreachable, but we still need to push a dummy return
		// type on the stack for the IL to be legal.  Prepare a default
		// initializer for the return value in this case (except if we were
		// returning a Vector3, which we already initialized at declaration
		// time).
		//

		if (ReturnCount == 1)
		{
			switch (ReturnType)
			{

			case ACTIONTYPE_INT:
				ILGen->Emit( OpCodes::Ldc_I4_0 );
				break;

			case ACTIONTYPE_FLOAT:
				ILGen->Emit( OpCodes::Ldc_R4, 0.0f );
				break;

			case ACTIONTYPE_OBJECT:
				ILGen->Emit( OpCodes::Ldc_I4, (Int32) m_ILGenCtx->ObjectInvalid );
				ILGen->Emit( OpCodes::Conv_U4 );
				break;

			case ACTIONTYPE_STRING:
				ILGen->Emit( OpCodes::Ldnull );
				break;

			default:
				throw gcnew Exception( "Illegal return type for setting up direct fast action service exception handler dummy return value." );
				break;

			}

			ILGen->Emit( OpCodes::Stloc, RetLocal );
		}

		ILGen->EndExceptionBlock( );
	}

	//
	// We're done, emit the return instruction.
	//

	if (ReturnType != ACTIONTYPE_VOID)
		ILGen->Emit( OpCodes::Ldloc, RetLocal );

	ILGen->Emit( OpCodes::Ret );

	//
	// Restore the caller's subroutine generation context and we're done.
	//

	m_ILGenCtx->Sub = Sub;

	return MthActionService;
}

#endif // NWSCRIPT_DIRECT_FAST_ACTION_CALLS

void
NWScriptCodeGenerator::GenerateExecuteActionServiceSlowViaStub(
	__in PCNWACTION_DEFINITION CalledAction,
	__in NWNScriptLib::VariableWeakPtrVec * ParamList,
	__in size_t ParamCount,
	__in size_t ReturnCount,
	__in_ecount( ReturnCount ) PCNWACTION_TYPE ReturnTypes
	)
/*++

Routine Description:

	This routine emits any code necessary to invoke an action service handler.

	The generated code takes the slow path (individual VMStack operation calls
	made to native code).  A service handler stub is invoked instead of using
	an inline call mechanism.

Arguments:

	CalledAction - Supplies the action descriptor of the action to invoke.

	ParamList - Supplies the list of actual parameters (and return values) for
	            the action.

	ParamCount - Supplies the count of source-level parameters to the action,
	             which does not need to match the parameter list size.

	ReturnCount - Supplies the count of return variables affected by the action
	              call.

	ReturnTypes - Supplies the list of return types.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	size_t            ActualParams;
	ILGenerator     ^ ILGen;
	LocalBuilder    ^ Loc;
	MethodInfo      ^ DirectCallStub;

	ILGen        = m_ILGenCtx->Sub->ILGen;
	ActualParams = ParamList->size( );

	//
	// Acquire the direct call stub to use here.
	//

	DirectCallStub = AcquireSlowActionServiceStub(
		CalledAction,
		ActualParams - ReturnCount,
		ParamCount);

	//
	// Now load all of the arguments for the subroutine.
	//
	// N.B.  We call a dispatch routine on the ProgramType object,
	//       and not a method on the NWScriptProgram.  The dispatch routine
	//       (the stub) will call the raw native interface after packaging the
	///      parameter list appropriately.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );

	for (size_t i = ReturnCount; i < ActualParams; i += 1)
	{
		NWScriptVariable * Var = ParamList->at( i );

		GenerateLoadVariable( Var );
	}

	ILGen->Emit( OpCodes::Call, DirectCallStub );

	//
	// Now unpack the return value.
	//

	for (size_t i = ReturnCount; i != 0; i -= 1)
	{
		if (ParamList->at( i - 1 )->GetType( ) != ReturnTypes[ i - 1 ])
			throw gcnew Exception( "Return type mismatch for action service routine invocation." );

		switch (ReturnTypes[ i - 1 ])
		{

		case ACTIONTYPE_INT:
			GenerateStoreVariable( ParamList->at( i - 1 ) );
			break;

		case ACTIONTYPE_FLOAT:
			if (ReturnCount == 1)
				GenerateStoreVariable( ParamList->at( i - 1 ) );
			else
			{
				//
				// We have a vector3, unpack it.
				//

				switch (i)
				{

				case 3:
					Loc = AcquireLocal( NWScript::Vector3::typeid );

					ILGen->Emit( OpCodes::Stloc, Loc );
					ILGen->Emit( OpCodes::Ldloc, Loc );
					ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldVector3_z );
					GenerateStoreVariable( ParamList->at( i - 1 ) );
					break;

				case 2:
					ILGen->Emit( OpCodes::Ldloc, Loc );
					ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldVector3_y );
					GenerateStoreVariable( ParamList->at( i - 1 ) );
					break;

				case 1:
					ILGen->Emit( OpCodes::Ldloc, Loc );
					ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldVector3_x );
					GenerateStoreVariable( ParamList->at( i - 1 ) );
					ReleaseLocal( Loc );
					break;

				default:
					throw gcnew Exception( "Invalid vector (too many components) returned from action service handler." );

				}
			}
			break;

		case ACTIONTYPE_OBJECT:
			GenerateStoreVariable( ParamList->at( i - 1 ) );
			break;

		case ACTIONTYPE_STRING:
			GenerateStoreVariable( ParamList->at( i - 1 ) );
			break;

		default:
			if ((ReturnTypes[ i - 1 ] >= ACTIONTYPE_ENGINE_0) &&
			    (ReturnTypes[ i - 1 ] <= ACTIONTYPE_ENGINE_9))
			{
				GenerateStoreVariable( ParamList->at( i - 1 ) );
				break;
			}

			throw gcnew Exception( "Illegal return type for action service handler." );

		}
	}
}

MethodInfo ^
NWScriptCodeGenerator::AcquireSlowActionServiceStub(
	__in PCNWACTION_DEFINITION CalledAction,
	__in size_t ParamCount,
	__in size_t SrcParams
	)
/*++

Routine Description:

	This routine returns a method descriptor for a method that can be invoked
	in order to execute a particular action service with a given number of
	arguments.  The method will contain code to execute individual VMStack
	intrinsic operations necessary to effect the call.

	If such a method has already been generated for a given action service
	number and parameter count combination, it is returned, else a new method
	is created.

Arguments:

	CalledAction - Supplies the action descriptor of the action to invoke.

	ParamCount - Supplies the count of actual parameters to the action, which
	             includes one float for each vector component.

	SrcParams - Supplies the count of source level arguments to the action,
	            which counts 1 for each vector and 1 for each action.

Return Value:

	The method descriptor is returned.
	
	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	array< ActionServiceMethodInfo > ^ StubArray;
	array< ActionServiceMethodInfo > ^ NewArray;
	MethodInfo                       ^ Mth;

	//
	// First, check if an existing method matched.
	//

	StubArray = m_ILGenCtx->MthActionServiceStubs[ CalledAction->ActionId ];

	if (StubArray != nullptr)
	{
		for (Int32 i = 0; i < StubArray->Length; i += 1)
		{
			if (StubArray[ i ].NumParams == (Int32) ParamCount)
				return StubArray[ i ].Method;
		}
	}

	//
	// No existing method will fit, we'll need to create one.
	//

	Mth = GenerateSlowActionServiceStub(
		CalledAction,
		ParamCount,
		SrcParams);

	//
	// Cache it for future use and return to the caller.
	//
	// N.B.  Most script programs only have one action service handler at most
	//       per action id.  Current compilers always supply all default
	//       arguments at compile time, but for generality's sake, we allow a
	//       mix.
	//

	NewArray = gcnew array< ActionServiceMethodInfo >( StubArray == nullptr ? 1 : StubArray->Length + 1 );

	if (StubArray != nullptr)
		StubArray->CopyTo( NewArray, 0 );

	NewArray[ NewArray->Length - 1 ].Method    = Mth;
	NewArray[ NewArray->Length - 1 ].NumParams = (Int32) ParamCount;

	m_ILGenCtx->MthActionServiceStubs[ CalledAction->ActionId ] = NewArray;

	return Mth;
}

MethodInfo ^
NWScriptCodeGenerator::GenerateSlowActionServiceStub(
	__in PCNWACTION_DEFINITION CalledAction,
	__in size_t ParamCount,
	__in size_t SrcParams
	)
/*++

Routine Description:

	This routine generates a slow action service handler call stub method for
	a given action and parameter count mix.  The stub can be invoked to
	transfer control to the action service handler with a minimum of overhead.

	Logic is emitted to directly push arguments onto the VM stack for the call,
	make the action service handler call, and then pop any return values off of
	the VM stack.

Arguments:

	CalledAction - Supplies the action descriptor of the action to invoke.

	ParamCount - Supplies the count of actual parameters to the action, which
	             includes one float for each vector component.

	SrcParams - Supplies the count of source level arguments to the action,
	            which counts 1 for each vector and 1 for each action.

Return Value:

	The method descriptor is returned.
	
	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	MethodBuilder           ^ MthActionService;
	String                  ^ Name;
	array< Type ^ >         ^ ArgTypes;
	array< NWACTION_TYPE >  ^ ParamRawTypes;
	size_t                    ParamIndex;
	Int32                     i;
	size_t                    ReturnCount;
	NWACTION_TYPE             ReturnType;
	Type                    ^ MSILReturnType;
	ILGenerator             ^ ILGen;
	LocalBuilder            ^ RetLocal;
	LocalBuilder            ^ RetTmpFloat;
	SubroutineGenContext    ^ Sub;

	//
	// Determine the MSIL-level types for arguments and return values.
	//

	ReturnType = CalledAction->ReturnType;

	switch (ReturnType)
	{

	case ACTIONTYPE_VOID:
		ReturnCount    = 0;
		MSILReturnType = void::typeid;
		break;

	case ACTIONTYPE_VECTOR:
		ReturnCount    = 3;
		MSILReturnType = NWScript::Vector3::typeid;
		ReturnType     = ACTIONTYPE_FLOAT;
		break;

	default:
		ReturnCount    = 1;
		MSILReturnType = GetVariableType( ReturnType );
		break;

	}

	//
	// Name the action service and its internal descriptor type.
	//

	Name  = "ExecuteActionService_" + (Int32) CalledAction->ActionId;
	Name += "_";
	Name += (Int32) ParamCount;

	__if_exists( NWACTION_DEFINITION::Name )
	{
		Name += "_";
		Name += gcnew System::String( CalledAction->Name );
	}

	ArgTypes            = gcnew array< Type ^ >( (Int32) ParamCount );
	ParamRawTypes       = gcnew array< NWACTION_TYPE >( (Int32) ParamCount );

	//
	// First, build the array of raw types, expanding aggregates.  The array is
	// constructed from end to start, such that it is in the order that the
	// commands must be.
	//

	ParamIndex  = 0;
	i           = (Int32) ParamCount;

	for (Int32 n = 0; n < (Int32) ParamCount; n += 1)
	{
		NWACTION_TYPE ParamType;

		if (ParamIndex >= CalledAction->NumParameters)
			throw gcnew Exception( "Invoked action with too many parameters." );

		ParamType = CalledAction->ParameterTypes[ ParamIndex ];

		switch (ParamType)
		{

		case ACTIONTYPE_ACTION:
			ParamIndex += 1;
			n -= 1;

			continue;

		case ACTIONTYPE_VECTOR:
			if (i - 3 < 0)
				throw gcnew Exception( "Passed partial vector to action service handler." );

			for (Int32 ii = 0; ii < 3; ii += 1)
			{
				i -= 1;

				ParamRawTypes[ i ] = ACTIONTYPE_FLOAT;

				ArgTypes[ n + ii ] = GetVariableType( ACTIONTYPE_FLOAT );
			}

			n          += 2; // +1 after loop
			ParamIndex += 1;
			break;

		default:
			i -= 1;
			ParamIndex += 1;

			ParamRawTypes[ i ] = ParamType;

			ArgTypes[ n ] = GetVariableType( ParamType );
			break;

		}
	}

	Sub               = m_ILGenCtx->Sub;
	MthActionService  = BeginNewSubroutine(
		Name,
		m_ILGenCtx->ActionServiceStubAttributes,
		MSILReturnType,
		ArgTypes);

	ILGen = MthActionService->GetILGenerator( );

	if (ReturnType != ACTIONTYPE_VOID)
	{
		RetLocal = ILGen->DeclareLocal( MSILReturnType );

		//
		// If we returned a NWScript::Vector3, then initialize it.
		//

		if ((ReturnCount == 3) && (ReturnType == ACTIONTYPE_FLOAT))
		{
//			ILGen->Emit( OpCodes::Ldloca, RetLocal );
//			ILGen->Emit( OpCodes::Initobj, MSILReturnType );
		}
	}

	//
	// Now load all of the arguments for the subroutine.
	//
	// N.B.  We call a dispatch routine on the NWScriptProgram object,
	//       and not a method on the ProgramType.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );
#if NWSCRIPT_VIRT_TYPE_WORKAROUND
	ILGen->Emit( OpCodes::Castclass, m_HostInterfaceType );
#endif

	//
	// Push the parameters on to the VM stack.
	//

	for (i = 0; i < (Int32) ParamCount; i += 1)
	{
		short ArgSlot;

		//
		// Now prepare the corresponding parameter.  Note that the first entry
		// pushed onto the stack is the last parameter, and our real parameters
		// start at 1 (0 being this).
		//

		ArgSlot = (short) (ParamCount - i);

		GenerateVMStackPush(
			ILGen,
			GetVariableType( ParamRawTypes[ i ] ),
			ArgSlot );
	}

	// Issue the action service dispatcher call, then unpack the
	// return value to our associated working local variables (if any).
	//

	ILGen->Emit( OpCodes::Ldc_I4, (Int32) CalledAction->ActionId );
	ILGen->Emit( OpCodes::Conv_U4 );
	ILGen->Emit( OpCodes::Ldc_I4, (Int32) SrcParams );
	ILGen->Emit( OpCodes::Conv_U4 );

	ILGen->Emit( OpCodes::Callvirt, m_ILGenCtx->MthIntrinsic_ExecuteActionService );

	//
	// Now push the (MSIL) return value onto the stack.
	//

	switch (ReturnCount)
	{

	case 0:
		break;

	case 1:
		GenerateVMStackPop( ILGen, RetLocal );
		break;

	case 3:
		//
		// We returned a vector.  Package it up into a NWScript::Vector3 value
		// type and return that.
		//

		RetTmpFloat = AcquireLocal( Single::typeid );

		GenerateVMStackPop( ILGen, RetTmpFloat );
		ILGen->Emit( OpCodes::Ldloca, RetLocal );
		ILGen->Emit( OpCodes::Ldloc, RetTmpFloat );
		ILGen->Emit( OpCodes::Stfld, m_ILGenCtx->FldVector3_z );

		GenerateVMStackPop( ILGen, RetTmpFloat );
		ILGen->Emit( OpCodes::Ldloca, RetLocal );
		ILGen->Emit( OpCodes::Ldloc, RetTmpFloat );
		ILGen->Emit( OpCodes::Stfld, m_ILGenCtx->FldVector3_y );

		GenerateVMStackPop( ILGen, RetTmpFloat );
		ILGen->Emit( OpCodes::Ldloca, RetLocal );
		ILGen->Emit( OpCodes::Ldloc, RetTmpFloat );
		ILGen->Emit( OpCodes::Stfld, m_ILGenCtx->FldVector3_x );

		break;
	}

	//
	// We're done, emit the return instruction.
	//

	if (ReturnType != ACTIONTYPE_VOID)
		ILGen->Emit( OpCodes::Ldloc, RetLocal );

	ILGen->Emit( OpCodes::Ret );

	//
	// Restore the caller's subroutine generation context and we're done.
	//

	m_ILGenCtx->Sub = Sub;

	return MthActionService;
}

void
NWScriptCodeGenerator::GenerateOptimizedActionServiceStubs(
	)
/*++

Routine Description:

	This routine identifies whether optimized action service stubs can be
	created for the action service set in use, and, if so, generates the
	optimized stubs.

Arguments:

	None.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
#if NWSCRIPT_DIRECT_FAST_ACTION_CALLS
	PCNWACTION_DEFINITION Action;
	NWSCRIPT_ACTION       ActionServiceNumber;
	size_t                PassedParams;

	if (m_ILGenCtx->CodeGenParams == NULL)
		return;

	if ((m_ILGenCtx->CodeGenParams->CodeGenFlags & NWCGF_NWN_COMPATIBLE_ACTIONS) == 0)
		return;

	__if_exists( NWACTION_DEFINITION::Name )
	{
		for (ActionServiceNumber = 0;
			 ActionServiceNumber < m_ActionCount;
			 ActionServiceNumber += 1)
		{
			array< ActionServiceMethodInfo > ^ StubArray;
			array< ActionServiceMethodInfo > ^ NewArray;
			MethodInfo                       ^ Mth;
			String                           ^ StubName;

			Action       = &m_ActionDefs[ ActionServiceNumber ];
			PassedParams = 0;

			for (size_t i = 0; i < Action->NumParameters; i += 1)
			{
				NWACTION_TYPE VarType = Action->ParameterTypes[ i ];

				//
				// Count the number of actual parameters that will be passed to
				// the action on the stack.  This is a count of stack slots.
				//

				switch (VarType)
				{

				case ACTIONTYPE_VECTOR:
					PassedParams += 3;
					break;

				case ACTIONTYPE_ACTION:
					PassedParams += 0;
					break;

				default:
					PassedParams += 1;
					break;

				}
			}

			//
			// First, check if an existing method matched.
			//

			StubArray = m_ILGenCtx->MthActionServiceStubs[ Action->ActionId ];

			if (StubArray != nullptr)
			{
				for (Int32 i = 0; i < StubArray->Length; i += 1)
				{
					if (StubArray[ i ].NumParams == (Int32) PassedParams)
						continue;
				}
			}

			//
			// Name the action service and its internal descriptor type.
			//

			StubName  = "ExecuteActionService_" + (Int32) Action->ActionId;
			StubName += "_";
			StubName += (Int32) PassedParams;
			StubName += "_";
			StubName += gcnew System::String( Action->Name );

			//
			// If the action service handler is one that has an optimized
			// implementation, generate code for the optimized version.
			//

			Mth = GenerateOptimizedNWNCompatibleActionServiceStub(
				gcnew String( Action->Name ),
				StubName
				);

			if (Mth == nullptr)
				continue;

			//
			// Cache it for future use.
			//
			// N.B.  Most script programs only have one action service handler
			//       at most per action id.  Current compilers always supply
			//       all default arguments at compile time, but for
			//       generality's sake, we allow a mix.
			//

			NewArray = gcnew array< ActionServiceMethodInfo >( StubArray == nullptr ? 1 : StubArray->Length + 1 );

			if (StubArray != nullptr)
				StubArray->CopyTo( NewArray, 0 );

			NewArray[ NewArray->Length - 1 ].Method    = Mth;
			NewArray[ NewArray->Length - 1 ].NumParams = (Int32) PassedParams;

			m_ILGenCtx->MthActionServiceStubs[ Action->ActionId ] = NewArray;
		}
	}
#endif
}

MethodInfo ^
NWScriptCodeGenerator::GenerateOptimizedNWNCompatibleActionServiceStub(
	__in String ^ ActionName,
	__in String ^ StubName
	)
/*++

Routine Description:

	This routine determines whether a NWN-compatible action service handler has
	an accelerated intrinsic implementation.  If so, the accelerated version is
	generated.

Arguments:

	ActionName - Supplies the NWN-compatible action service name.

	StubName - Supplies the canonical name of the action service stub.

Return Value:

	The method descriptor is returned, if an accelerated implementation was
	available.  Otherwise, nullptr is returned.
	
	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	if (ActionName == "GetStringLength")
		return GenerateOptAction_GetStringLength( StubName );
	else if (ActionName == "GetStringLeft")
		return GenerateOptAction_GetStringLeft( StubName );
	else if (ActionName == "GetStringRight")
		return GenerateOptAction_GetStringRight( StubName );
	else if (ActionName == "IntToString")
		return GenerateOptAction_IntToString( StubName );
	else if (ActionName == "GetSubString")
		return GenerateOptAction_GetSubString( StubName );
	else
		return nullptr;
}

MethodInfo ^
NWScriptCodeGenerator::GenerateOptAction_GetStringLength(
	__in String ^ StubName
	)
/*++

Routine Description:

	This routine generates accelerated code to implement the action logic for
	GetStringLength.

Arguments:

	StubName - Supplies the canonical name of the action service stub.

Return Value:

	The method descriptor is returned.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	MethodBuilder ^ Method = BeginNewSubroutine(
		StubName,
		m_ILGenCtx->ActionServiceStubAttributes,
		Int32::typeid,
		gcnew array< Type ^ > { String::typeid });
	ILGenerator   ^ ILGen = Method->GetILGenerator( );

	//
	// Operation:
	//
	// return s.Length;
	//

	ILGen->Emit( OpCodes::Ldarg_1 );
	ILGen->Emit(
		OpCodes::Call,
		String::typeid->GetProperty( "Length" )->GetGetMethod( ) );
	ILGen->Emit( OpCodes::Ret );

	return Method;
}

MethodInfo ^
NWScriptCodeGenerator::GenerateOptAction_GetStringLeft(
	__in String ^ StubName
	)
/*++

Routine Description:

	This routine generates accelerated code to implement the action logic for
	GetStringLeft.

Arguments:

	StubName - Supplies the canonical name of the action service stub.

Return Value:

	The method descriptor is returned.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	MethodBuilder ^ Method = BeginNewSubroutine(
		StubName,
		m_ILGenCtx->ActionServiceStubAttributes,
		String::typeid,
		gcnew array< Type ^ > { String::typeid, Int32::typeid });
	ILGenerator   ^ ILGen = Method->GetILGenerator( );
	LocalBuilder  ^ LocStrLen;
	Label           L1;
	Label           L2;

	LocStrLen = AcquireLocal( Int32::typeid );
	L1        = ILGen->DefineLabel( );
	L2        = ILGen->DefineLabel( );

	//
	// Operation:
	//
	// Int32 StrLen = s.Length;
	// if (l < 0)
	//   return "";
	// else if (l > StrLen)
	//   l = StrLen;
	// return s.Substring( 0, l );
	//

	ILGen->Emit( OpCodes::Ldarg_1 );
	ILGen->Emit(
		OpCodes::Call,
		String::typeid->GetProperty( "Length" )->GetGetMethod( ) );
	ILGen->Emit( OpCodes::Stloc_S, LocStrLen );

	ILGen->Emit( OpCodes::Ldarg_2 );
	ILGen->Emit( OpCodes::Ldc_I4_0 );
	ILGen->Emit( OpCodes::Blt_S, L1 );

	ILGen->Emit( OpCodes::Ldarg_2 );
	ILGen->Emit( OpCodes::Ldloc_S, LocStrLen );
	ILGen->Emit( OpCodes::Ble_S, L2 );

	ILGen->Emit( OpCodes::Ldloc_S, LocStrLen );
	ILGen->Emit( OpCodes::Starg_S, (Byte)2 );

	ILGen->MarkLabel( L2 );
	ILGen->Emit( OpCodes::Ldarg_1 );
	ILGen->Emit( OpCodes::Ldc_I4_0 );
	ILGen->Emit( OpCodes::Ldarg_2 );
	ILGen->Emit(
		OpCodes::Call,
		String::typeid->GetMethod( "Substring", gcnew array< Type ^ > { Int32::typeid, Int32::typeid } )
		);

	ILGen->Emit( OpCodes::Ret );

	ILGen->MarkLabel( L1 );
	ILGen->Emit( OpCodes::Ldstr, "" );
	ILGen->Emit( OpCodes::Ret );

	ReleaseLocal( LocStrLen );

	return Method;
}

MethodInfo ^
NWScriptCodeGenerator::GenerateOptAction_GetStringRight(
	__in String ^ StubName
	)
/*++

Routine Description:

	This routine generates accelerated code to implement the action logic for
	GetStringRight.

Arguments:

	StubName - Supplies the canonical name of the action service stub.

Return Value:

	The method descriptor is returned.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	MethodBuilder ^ Method = BeginNewSubroutine(
		StubName,
		m_ILGenCtx->ActionServiceStubAttributes,
		String::typeid,
		gcnew array< Type ^ > { String::typeid, Int32::typeid });
	ILGenerator   ^ ILGen = Method->GetILGenerator( );
	LocalBuilder  ^ LocStrLen;
	Label           L1;
	Label           L2;

	LocStrLen = AcquireLocal( Int32::typeid );
	L1        = ILGen->DefineLabel( );
	L2        = ILGen->DefineLabel( );

	//
	// Operation:
	//
	// Int32 StrLen = s.Length;
	// if (l < 0)
	//   return "";
	// else if (l > StrLen)
	//   l = StrLen;
	// return s.Substring( StrLen - l );
	//

	ILGen->Emit( OpCodes::Ldarg_1 );
	ILGen->Emit(
		OpCodes::Call,
		String::typeid->GetProperty( "Length" )->GetGetMethod( ) );
	ILGen->Emit( OpCodes::Stloc_S, LocStrLen );

	ILGen->Emit( OpCodes::Ldarg_2 );
	ILGen->Emit( OpCodes::Ldc_I4_0 );
	ILGen->Emit( OpCodes::Blt_S, L1 );

	ILGen->Emit( OpCodes::Ldarg_2 );
	ILGen->Emit( OpCodes::Ldloc_S, LocStrLen );
	ILGen->Emit( OpCodes::Ble_S, L2 );

	ILGen->Emit( OpCodes::Ldloc_S, LocStrLen );
	ILGen->Emit( OpCodes::Starg_S, (Byte)2 );

	ILGen->MarkLabel( L2 );
	ILGen->Emit( OpCodes::Ldarg_1 );
	ILGen->Emit( OpCodes::Ldloc_S, LocStrLen );
	ILGen->Emit( OpCodes::Ldarg_2 );
	ILGen->Emit( OpCodes::Sub );
	ILGen->Emit(
		OpCodes::Call,
		String::typeid->GetMethod( "Substring", gcnew array< Type ^ > { Int32::typeid } )
		);

	ILGen->Emit( OpCodes::Ret );

	ILGen->MarkLabel( L1 );
	ILGen->Emit( OpCodes::Ldstr, "" );
	ILGen->Emit( OpCodes::Ret );

	ReleaseLocal( LocStrLen );

	return Method;
}

MethodInfo ^
NWScriptCodeGenerator::GenerateOptAction_IntToString(
	__in String ^ StubName
	)
/*++

Routine Description:

	This routine generates accelerated code to implement the action logic for
	IntToString.

Arguments:

	StubName - Supplies the canonical name of the action service stub.

Return Value:

	The method descriptor is returned.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	MethodBuilder ^ Method = BeginNewSubroutine(
		StubName,
		m_ILGenCtx->ActionServiceStubAttributes,
		String::typeid,
		gcnew array< Type ^ > { Int32::typeid });
	ILGenerator   ^ ILGen = Method->GetILGenerator( );

	//
	// Operation:
	//
	// return Convert.ToString( i );
	//

	ILGen->Emit( OpCodes::Ldarg_1 );
	ILGen->Emit(
		OpCodes::Call,
		Convert::typeid->GetMethod( "ToString", gcnew array< Type ^ > { Int32::typeid } )
		);
	ILGen->Emit( OpCodes::Ret );

	return Method;
}

MethodInfo ^
NWScriptCodeGenerator::GenerateOptAction_GetSubString(
	__in String ^ StubName
	)
/*++

Routine Description:

	This routine generates accelerated code to implement the action logic for
	GetSubString.

Arguments:

	StubName - Supplies the canonical name of the action service stub.

Return Value:

	The method descriptor is returned.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	MethodBuilder ^ Method = BeginNewSubroutine(
		StubName,
		m_ILGenCtx->ActionServiceStubAttributes,
		String::typeid,
		gcnew array< Type ^ > { String::typeid, Int32::typeid, Int32::typeid });
	ILGenerator   ^ ILGen = Method->GetILGenerator( );
	LocalBuilder  ^ LocStrLen;
	LocalBuilder  ^ LocMax;
	Label           L1;
	Label           L2;

	LocStrLen = AcquireLocal( Int32::typeid );
	LocMax    = AcquireLocal( Int32::typeid );
	L1        = ILGen->DefineLabel( );
	L2        = ILGen->DefineLabel( );

	//
	// Operation:
	//
	// Int32 StrLen = s.Length;
	// if (start < 0)
	//   return "";
	// else if (start > StrLen)
	//   return "";
	// Int32 Max = StrLen - start;
	// if ((UInt32)l > (UInt32)Max)
	//   l = Max;
	// return s.Substring( start, l );
	//

	ILGen->Emit( OpCodes::Ldarg_1 );
	ILGen->Emit(
		OpCodes::Call,
		String::typeid->GetProperty( "Length" )->GetGetMethod( ) );
	ILGen->Emit( OpCodes::Stloc, LocStrLen );

	ILGen->Emit( OpCodes::Ldarg_2 );
	ILGen->Emit( OpCodes::Ldc_I4_0 );
	ILGen->Emit( OpCodes::Blt_S, L1 );

	ILGen->Emit( OpCodes::Ldarg_2 );
	ILGen->Emit( OpCodes::Ldloc_S, LocStrLen );
	ILGen->Emit( OpCodes::Bgt_S, L1 );

	ILGen->Emit( OpCodes::Ldloc_S, LocStrLen );
	ILGen->Emit( OpCodes::Ldarg_2 );
	ILGen->Emit( OpCodes::Sub );
	ILGen->Emit( OpCodes::Stloc_S, LocMax );

	ILGen->Emit( OpCodes::Ldarg_3 );
	ILGen->Emit( OpCodes::Conv_U4 );
	ILGen->Emit( OpCodes::Ldloc_S, LocMax );
	ILGen->Emit( OpCodes::Conv_U4 );
	ILGen->Emit( OpCodes::Ble_Un_S, L2 );

	ILGen->Emit( OpCodes::Ldloc_S, LocMax );
	ILGen->Emit( OpCodes::Starg_S, (Byte)3 );

	ILGen->MarkLabel( L2 );
	ILGen->Emit( OpCodes::Ldarg_1 );
	ILGen->Emit( OpCodes::Ldarg_2 );
	ILGen->Emit( OpCodes::Ldarg_3 );
	ILGen->Emit(
		OpCodes::Call,
		String::typeid->GetMethod( "Substring", gcnew array< Type ^ > { Int32::typeid, Int32::typeid } )
		);

	ILGen->Emit( OpCodes::Ret );

	ILGen->MarkLabel( L1 );
	ILGen->Emit( OpCodes::Ldstr, "" );
	ILGen->Emit( OpCodes::Ret );

	ReleaseLocal( LocMax );
	ReleaseLocal( LocStrLen );

	return Method;
}


MethodBuilder ^
NWScriptCodeGenerator::BeginNewSubroutine(
	__in String ^ Name,
	__in MethodAttributes Attributes,
	__in Type ^ ReturnType,
	__in_opt array< Type ^ > ^ ParameterTypes
	)
/*++

Routine Description:

	This routine begins generation of a new subroutine on the program main
	class type.

Arguments:

	Name - Supplies the subroutine name.

	Attributes - Supplies the subroutine method attributes.

	ReturnType - Supplies the subroutine return type.

	ParameterTypes - Optionally supplies the subroutine parameter type list.

Return Value:

	On success, a method builder for the method is returned.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	MethodBuilder ^ Method = m_ILGenCtx->ProgramType->DefineMethod(
		Name,
		Attributes,
		ReturnType,
		ParameterTypes);

	//
	// Set up a SubroutineGenContext so that we can use the generate helpers.
	//
	
	m_ILGenCtx->Sub = gcnew SubroutineGenContext;
	m_ILGenCtx->Sub->ILGen = Method->GetILGenerator( );
	m_ILGenCtx->Sub->CurrentFlow = gcnew SubroutineControlFlow;
	m_ILGenCtx->Sub->CurrentFlow->LocalPool = gcnew FreeLocalMap;

	return Method;
}


void
NWScriptCodeGenerator::GenerateLoadVariable(
	__in NWScriptVariable * Var
	)
/*++

Routine Description:

	This routine emits any code necessary to copy an IR variable's value to the
	top of the execution stack.

Arguments:

	Var - Supplies the variable to copy to the top of the execution stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	GenerateLoadVariable( Var, false );
}

void
NWScriptCodeGenerator::GenerateLoadVariable(
	__in NWScriptVariable * Var,
	__in bool Box
	)
/*++

Routine Description:

	This routine emits any code necessary to copy an IR variable's value to the
	top of the execution stack.

Arguments:

	Var - Supplies the variable to copy to the top of the execution stack.

	Box - Supplies a Boolean value that indicates true if a boxed reference
	      should be pushed onto the stack if the type was a value type, else
	      false if the type should be pushed "as-is".

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	ILGenerator ^ ILGen;

	Var   = Var->GetHeadVariable( );
	ILGen = m_ILGenCtx->Sub->ILGen;

	if (IsDebugLevel( NWScriptVM::EDL_Verbose ))
	{
		m_TextOut->WriteText(
			"LOAD %p:%08X (%lu)\n",
			Var,
			Var->GetFlags( ),
			(unsigned long) Var->GetClass( ) );
	}

	switch (Var->GetClass( ))
	{

	case NWScriptVariable::Global:
		{
			//
			// We're to read a global variable.  Retrieve it from the instance
			// field.
			//

			ILGen->Emit( OpCodes::Ldarg_0 );
			ILGen->Emit( OpCodes::Ldfld, GetGlobalVariable( Var ) );
		}
		break;

	case NWScriptVariable::Local:
	case NWScriptVariable::CallParameter:
	case NWScriptVariable::CallReturnValue:
	case NWScriptVariable::ReturnValue:
		{
			//
			// We're to read a local variable.  Find the local slot and retrieve
			// the contents from it.
			//

			ILGen->Emit( OpCodes::Ldloc, GetLocalVariable( m_ILGenCtx->Sub, Var ) );
		}

		break;

	case NWScriptVariable::Parameter:
		{
			short ArgSlot = (short) Var->GetScope( );

			ILGen->Emit( OpCodes::Ldarg, ArgSlot );
		}
		break;

	case NWScriptVariable::Constant:
		{
			const NWScriptAnalyzer::VARIABLE_VALUE & Value = m_ILGenCtx->Analyzer->GetConstantValue( Var );

			if (Value.Type != Var->GetType( ))
			{
				String ^ Str = "Inconsistent constant variable type (";
				int      i;

				i    = (int) Value.Type;
				Str += i;
				Str += " != ";
				i    = (int) Var->GetType( );
				Str += i;
				Str += ")";

				throw gcnew Exception( Str );
			}

			switch (Var->GetType( ))
			{

			case ACTIONTYPE_INT:
				ILGen->Emit( OpCodes::Ldc_I4, (Int32) Value.Int );
				break;

			case ACTIONTYPE_FLOAT:
				ILGen->Emit( OpCodes::Ldc_R4, Value.Float );
				break;

			case ACTIONTYPE_STRING:
				ILGen->Emit(
					OpCodes::Ldstr,
					gcnew String(
						Value.StringPtr->data( ),
						0,
						(Int32) Value.StringPtr->size( ),
						m_ILGenCtx->StringEncoding
						)
					);
				break;

			case ACTIONTYPE_OBJECT:
				if ((Value.Object == OBJECTID_INVALID) ||
					(Value.Object == (ULONG) m_ILGenCtx->ObjectInvalid))
				{
					ILGen->Emit(
						OpCodes::Ldc_I4,
						(Int32) m_ILGenCtx->ObjectInvalid);
				}
				else if (Value.Object == OBJECTID_SELF)
				{
					//
					// Fetch ScriptProgram ^ T::m_CurrentActionObjectSelf.
					//

					ILGen->Emit( OpCodes::Ldarg_0 );
					ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldCurrentActionObjectSelf );
				}
				else
				{
					throw gcnew Exception(
						"Illegal object id literal for GenerateLoadVariable");
				}
				break;

			default:
				throw gcnew Exception( "Illegal constant type." );

			}
		}
		break;

	default:
		throw gcnew Exception( "Loading from a variable of an unsupported type." );
	}

	if (Box)
		GenerateBox( Var->GetType( ) );
}

void
NWScriptCodeGenerator::GenerateStoreVariable(
	__in NWScriptVariable * Var
	)
/*++

Routine Description:

	This routine emits any code necessary to assign an IR variable's value from the
	top of the execution stack.

Arguments:

	Var - Supplies the variable whose value should be assigned to that on the
	      top of the execution stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	GenerateStoreVariable( Var, false );
}

void
NWScriptCodeGenerator::GenerateStoreVariable(
	__in NWScriptVariable * Var,
	__in bool Unbox
	)
/*++

Routine Description:

	This routine emits any code necessary to assign an IR variable's value from the
	top of the execution stack.

Arguments:

	Var - Supplies the variable whose value should be assigned to that on the
	      top of the execution stack.

	Unbox - Supplies a Boolean value that indicates true if a boxed reference
	        should be unboxed from the stack if the type was a value type, else
	        false if the type should be popped "as-is".

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	ILGenerator ^ ILGen;

	Var   = Var->GetHeadVariable( );
	ILGen = m_ILGenCtx->Sub->ILGen;

	if (IsDebugLevel( NWScriptVM::EDL_Verbose ))
	{
		m_TextOut->WriteText(
			"STORE %p:%08X (%lu)\n",
			Var,
			Var->GetFlags( ),
			(unsigned long) Var->GetClass( ) );
	}

	if (Unbox)
		GenerateUnbox( Var->GetType( ) );

	switch (Var->GetClass( ))
	{

	case NWScriptVariable::Global:
		{
			LocalBuilder ^ Loc = AcquireLocal( GetVariableType( Var ) );

			if (Loc == nullptr)
				throw gcnew Exception( "Couldn't acquire local!\n" );

			//
			// We're to write a global variable.  Store it to the instance
			// field.
			//

			ILGen->Emit( OpCodes::Stloc, Loc );
			ILGen->Emit( OpCodes::Ldarg_0 );
			ILGen->Emit( OpCodes::Ldloc, Loc );
			ILGen->Emit( OpCodes::Stfld, GetGlobalVariable( Var ) );

			ReleaseLocal( Loc );
		}
		break;

	case NWScriptVariable::Local:
	case NWScriptVariable::CallParameter:
	case NWScriptVariable::CallReturnValue:
	case NWScriptVariable::ReturnValue:
		{
			//
			// We're to write a local variable.  Find the local slot and store
			// the contents to it.
			//

			ILGen->Emit( OpCodes::Stloc, GetLocalVariable( m_ILGenCtx->Sub, Var ) );
		}
		break;

	case NWScriptVariable::Constant:
		throw gcnew Exception( "Attempting to store to a constant variable." );
		break;

	case NWScriptVariable::Parameter:
		{
			short ArgSlot = (short) Var->GetScope( );

			ILGen->Emit( OpCodes::Starg, ArgSlot );
		}
		break;

	default:
		throw gcnew Exception( "Storing to variable of an unsupported type." );

	}
}


void
NWScriptCodeGenerator::GeneratePushDefaultValue(
	__in Type ^ LocalType
	)
/*++

Routine Description:

	This routine emits any code necessary to push the default value for a given
	type onto the stack.

Arguments:

	LocalType - Supplies the type to push a default initializer for.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	ILGenerator    ^ ILGen;
	NWACTION_TYPE    NWScriptType;

	ILGen        = m_ILGenCtx->Sub->ILGen;
	NWScriptType = GetNWScriptType( LocalType );

	switch (NWScriptType)
	{

	case ACTIONTYPE_INT:
		ILGen->Emit( OpCodes::Ldc_I4_0 );
		break;

	case ACTIONTYPE_FLOAT:
		ILGen->Emit( OpCodes::Ldc_R4, 0.0f );
		break;

	case ACTIONTYPE_STRING:
		ILGen->Emit( OpCodes::Ldstr, "" );
		break;

	case ACTIONTYPE_OBJECT:
		ILGen->Emit( OpCodes::Ldc_I4, (Int32) m_ILGenCtx->ObjectInvalid );
		ILGen->Emit( OpCodes::Conv_U4 );
		break;

	default:
		if ((NWScriptType >= ACTIONTYPE_ENGINE_0) &&
		    (NWScriptType <= ACTIONTYPE_ENGINE_9))
		{
			int EType = (int) (NWScriptType - ACTIONTYPE_ENGINE_0);

			//
			// Call the creation intrinsic.
			//

			ILGen->Emit( OpCodes::Ldarg_0 );
			ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgram );

			ILGen->Emit(
				OpCodes::Callvirt,
				m_ILGenCtx->MthIntrinsic_CreateEngineStructure[ EType ] );
			break;
		}

		throw gcnew Exception( "Attempted to initialize default value of object of unknown type." );

	}
}

void
NWScriptCodeGenerator::GenerateSetDefaultValue(
	__in LocalBuilder ^ Local
	)
/*++

Routine Description:

	This routine emits any code necessary to assign a default value to a local
	variable.

Arguments:

	Local - Supplies the local to perform default initialization for.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	ILGenerator ^ ILGen;
	Type        ^ LocalType;

	ILGen     = m_ILGenCtx->Sub->ILGen;
	LocalType = Local->LocalType;

	GeneratePushDefaultValue( LocalType );

	ILGen->Emit( OpCodes::Stloc, Local );
}

void
NWScriptCodeGenerator::GenerateSetDefaultValue(
	__in NWScriptVariable * Var
	)
/*++

Routine Description:

	This routine emits any code necessary to assign a default value to an IR
	variable.

Arguments:

	Var - Supplies the IR variable to perform default initialization for.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	ILGenerator ^ ILGen;
	Type        ^ LocalType;

	ILGen     = m_ILGenCtx->Sub->ILGen;
	LocalType = GetVariableType( Var );

	GeneratePushDefaultValue( LocalType );
	GenerateStoreVariable( Var );
}


void
NWScriptCodeGenerator::GenerateCompare(
	__in NWScriptVariable * Var1,
	__in NWScriptVariable * Var2,
	__in NWScriptVariable * Result,
	__in OpCode Op,
	__in bool Negate
	)
/*++

Routine Description:

	This routine emits any code necessary to compare two variables for a given
	style of comparison.

Arguments:

	Var1 - Supplies the first variable to compare.

	Var2 - Supplies the second variable to compare.

	Result - Supplies the variable that receives the result of the comparison.

	Op - Supplies the opcode to emit.

	Negate - Supplies a Boolean value that indicates true if the comparision is
	         to be negated, else false if it should not be.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	ILGenerator ^ ILGen;

	ILGen     = m_ILGenCtx->Sub->ILGen;

	if (Var1->GetType( ) != Var2->GetType( ))
		throw gcnew Exception( "Comparing variables of incompatible types." );

	//
	// Generate the type-specific comparison code.
	//

	switch (Var1->GetType( ))
	{

	case ACTIONTYPE_OBJECT:
		if (Op != OpCodes::Ceq)
			throw gcnew Exception( "Illegal comparison opcode for 'object'." );

	case ACTIONTYPE_INT:
	case ACTIONTYPE_FLOAT:
		GenerateLoadVariable( Var1 );
		GenerateLoadVariable( Var2 );

		ILGen->Emit( Op );
		break;

	case ACTIONTYPE_STRING:
		if (Op != OpCodes::Ceq)
			throw gcnew Exception( "Illegal comparison opcode for 'string'." );

		GenerateLoadVariable( Var1 );
		GenerateLoadVariable( Var2 );

		ILGen->Emit( OpCodes::Callvirt, m_ILGenCtx->MthString_Equals );
		break;

	default:
		if ((Var1->GetType( ) >= ACTIONTYPE_ENGINE_0) &&
		    (Var1->GetType( ) <= ACTIONTYPE_ENGINE_9))
		{
			int EType = (int) (Var1->GetType( ) - ACTIONTYPE_ENGINE_0);

			if (Op != OpCodes::Ceq)
				throw gcnew Exception( "Illegal comparison opcode for 'engine structure'." );

			//
			// Call the comparison intrinsic.
			//

			ILGen->Emit( OpCodes::Ldarg_0 );
			ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgram );

			GenerateLoadVariable( Var1 );
			GenerateLoadVariable( Var2 );

			ILGen->Emit(
				OpCodes::Callvirt,
				m_ILGenCtx->MthIntrinsic_CompareEngineStructure[ EType ] );

			break;
		}

		throw gcnew Exception( "Comparing equality of unknown typed variable." );

	}

	//
	// Now save the result.
	//

	if (Negate)
	{
		ILGen->Emit( OpCodes::Ldc_I4_0 );
		ILGen->Emit( OpCodes::Ceq );
	}

	GenerateStoreVariable( Result );
}


void
NWScriptCodeGenerator::GenerateBinaryOp(
	__in NWScriptVariable * Var1,
	__in NWScriptVariable * Var2,
	__in NWScriptVariable * Result,
	__in OpCode Op,
	__in NWACTION_TYPE ReqType
	)
/*++

Routine Description:

	This routine emits any code necessary to perform a general purpose binary
	operation with a restricted set of types.

Arguments:

	Var1 - Supplies the first variable operand.

	Var2 - Supplies the second variable operand.

	Result - Supplies the variable that receives the result of the operation.

	Op - Supplies the opcode to emit.

	ReqType - Supplies the required type of all of the variables.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	//
	// Check types, then perform the operation and store the result.
	//

	if (Var1->GetType( ) != Var2->GetType( ))
		throw gcnew Exception( "Executing BinaryOp on variables of incompatible types." );
	else if (Var1->GetType( ) != ReqType)
		throw gcnew Exception( "Operand type mismatch for BinaryOp." );
	else if (Result->GetType( ) != ReqType)
		throw gcnew Exception( "Result type mismatch for BinaryOp." );

	GenerateLoadVariable( Var1 );
	GenerateLoadVariable( Var2 );

	m_ILGenCtx->Sub->ILGen->Emit( Op );

	GenerateStoreVariable( Result );
}

void
NWScriptCodeGenerator::GenerateBinaryOp(
	__in NWScriptVariable * Var1,
	__in NWScriptVariable * Var2,
	__in NWScriptVariable * Result,
	__in OpCode Op,
	__in NWACTION_TYPE ReqType,
	__in NWACTION_TYPE ReqRetType
	)
/*++

Routine Description:

	This routine emits any code necessary to perform a general purpose binary
	operation with a restricted set of types.

Arguments:

	Var1 - Supplies the first variable operand.

	Var2 - Supplies the second variable operand.

	Result - Supplies the variable that receives the result of the operation.

	Op - Supplies the opcode to emit.

	ReqType - Supplies the required type of the operand variables.

	ReqRetType - Supplies the required type of the return variable.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	//
	// Check types, then perform the operation and store the result.
	//

	if (Var1->GetType( ) != Var2->GetType( ))
		throw gcnew Exception( "Executing BinaryOp on variables of incompatible types." );
	else if (Var1->GetType( ) != ReqType)
		throw gcnew Exception( "Operand type mismatch for BinaryOp." );
	else if (Result->GetType( ) != ReqRetType)
		throw gcnew Exception( "Result type mismatch for BinaryOp." );

	GenerateLoadVariable( Var1 );
	GenerateLoadVariable( Var2 );

	m_ILGenCtx->Sub->ILGen->Emit( Op );

	GenerateStoreVariable( Result );
}


void
NWScriptCodeGenerator::GenerateUnaryOp(
	__in NWScriptVariable * Var,
	__in NWScriptVariable * Result,
	__in OpCode Op,
	__in NWACTION_TYPE ReqType
	)
/*++

Routine Description:

	This routine emits any code necessary to perform a general purpose unary
	operation with a restricted set of types.

Arguments:

	Var - Supplies the variable operand.

	Result - Supplies the variable that receives the result of the operation.

	Op - Supplies the opcode to emit.

	ReqType - Supplies the required type of all of the variables.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	//
	// Check types, then perform the operation and store the result.
	//

	if (Var->GetType( ) != ReqType)
		throw gcnew Exception( "Operand type mismatch for UnaryOp." );
	else if (Result->GetType( ) != ReqType)
		throw gcnew Exception( "Result type mismatch for UnaryOp." );

	GenerateLoadVariable( Var );

	m_ILGenCtx->Sub->ILGen->Emit( Op );

	GenerateStoreVariable( Result );
}

void
NWScriptCodeGenerator::GenerateUnaryOp(
	__in NWScriptVariable * Var,
	__in NWScriptVariable * Result,
	__in OpCode Op,
	__in NWACTION_TYPE ReqType,
	__in NWACTION_TYPE ReqRetType
	)
/*++

Routine Description:

	This routine emits any code necessary to perform a general purpose unary
	operation with a restricted set of types.

Arguments:

	Var - Supplies the variable operand.

	Result - Supplies the variable that receives the result of the operation.

	Op - Supplies the opcode to emit.

	ReqType - Supplies the required type of the operand variables.

	ReqRetType - Supplies the required type of the return variable.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	//
	// Check types, then perform the operation and store the result.
	//

	if (Var->GetType( ) != ReqType)
		throw gcnew Exception( "Operand type mismatch for UnaryOp." );
	else if (Result->GetType( ) != ReqRetType)
		throw gcnew Exception( "Result type mismatch for UnaryOp." );

	GenerateLoadVariable( Var );

	m_ILGenCtx->Sub->ILGen->Emit( Op );

	GenerateStoreVariable( Result );
}


void
NWScriptCodeGenerator::GenerateUpcastBinaryOp(
	__in NWScriptVariable * Var1,
	__in NWScriptVariable * Var2,
	__in NWScriptVariable * Result,
	__in OpCode Op
	)
/*++

Routine Description:

	This routine emits any code necessary to perform an upcasting binary
	operation.  Any mix of int32 and float parameters are permitted, with
	conversion to float if there is a parameter type mix.

Arguments:

	Var1 - Supplies the first variable operand.

	Var2 - Supplies the second variable operand.

	Result - Supplies the variable that receives the result of the operation.

	Op - Supplies the opcode to emit.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	bool            HaveFloat;
	NWACTION_TYPE   EffectiveType;
	ILGenerator   ^ ILGen;

	//
	// Determine the effective type of the operation.  Int32/int32 operations
	// yield an int32, all other mixes yield a float.

	HaveFloat = false;

	switch (Var1->GetType( ))
	{

	case ACTIONTYPE_INT:
		break;

	case ACTIONTYPE_FLOAT:
		HaveFloat = true;
		break;

	default:
		throw gcnew Exception( "Illegal operand 1 type for UpcastBinaryOp." );

	}

	switch (Var2->GetType( ))
	{

	case ACTIONTYPE_INT:
		break;

	case ACTIONTYPE_FLOAT:
		HaveFloat = true;
		break;

	default:
		throw gcnew Exception( "Illegal operand 2 type for UpcastBinaryOp." );

	}

	if (HaveFloat)
		EffectiveType = ACTIONTYPE_FLOAT;
	else
		EffectiveType = ACTIONTYPE_INT;

	ILGen = m_ILGenCtx->Sub->ILGen;

	//
	// Check types, then perform the operation (with upcasting to float32).
	//

	if (Result->GetType( ) != EffectiveType)
		throw gcnew Exception( "Result type mismatch for UpcastBinaryOp." );

	GenerateLoadVariable( Var1 );

	if (Var1->GetType( ) != EffectiveType)
		ILGen->Emit( OpCodes::Conv_R4 );

	GenerateLoadVariable( Var2 );

	if (Var2->GetType( ) != EffectiveType)
		ILGen->Emit( OpCodes::Conv_R4 );

	m_ILGenCtx->Sub->ILGen->Emit( Op );

	//
	// Finally, save the result away.
	//

	GenerateStoreVariable( Result );
}


void
NWScriptCodeGenerator::GenerateSaveState(
	__in MethodInfo ^ ResumeSub,
	__in UInt32 ResumeSubId,
	__in PROGRAM_COUNTER ResumeSubPC,
	__in_ecount( SaveLocalCount ) NWScriptVariable * * SaveLocals,
	__in size_t SaveLocalCount,
	__in_ecount( SaveGlobalCount ) NWScriptVariable * * SaveGlobals,
	__in size_t SaveGlobalCount
	)
/*++

Routine Description:

	This routine emits any code necessary to duplicate any variables that are
	referenced by a saved state resume subroutine.

	Typically, this routine is invoked to support an I_SAVE_STATE request.  It
	captures the values of all referenced locals, clones the 'this' pointer
	(that is, the ScriptProgram object), and then sets up an invocation to the
	store state intrinsic.

Arguments:

	ResumeSub - Supplies the MSIL-level subroutine descriptor associated with the
	            save state request.  This is the subroutine that is invoked
	            when the saved state is resumed.

	ResumeSubId - Supplies the resme subroutine identifier of the ResumeSub.
	              For a fast resume, this is used as an index into a dispatch
	              switch statement for ExecuteScriptSituation.

	ResumeSubPC - Supplies the NWScript program counter of the resume
	              subroutine (for correlation with the script interpreter).

	SaveLocals - Supplies the list of working variables (locals only) to copy
	             into the saved state object.

	SaveLocalCount - Supplies the count of locals in the SaveLocals list.

	SaveGlobals - Supplies the list of working variables (globals only) to copy
	              into the saved state object.

	              N.B.  For JIT purposes, all globals are duplicated presently.

	                    SaveGlobals is only used to allow tracking of which
	                    globals should be saved to an INWScriptStack if the
	                    NWCGF_ENABLE_SAVESTATE_TO_VMSTACK code generation flag
	                    was set.

	SaveGlobalCount - Supplies the actual number of NWScript global variables
	                  that the subroutine requires.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	ILGenerator  ^ ILGen;
	LocalBuilder ^ LocLocals;
	LocalBuilder ^ LocGlobals;
	bool           EnableSaveGlobals;

	ILGen     = m_ILGenCtx->Sub->ILGen;
	LocLocals = AcquireLocal( array< Object ^ >::typeid );

	if (m_ILGenCtx->CodeGenParams != NULL)
	{
		if (m_ILGenCtx->CodeGenParams->CodeGenFlags & NWCGF_ENABLE_SAVESTATE_TO_VMSTACK)
			EnableSaveGlobals = true;
		else
			EnableSaveGlobals = false;
	}
	else
	{
		EnableSaveGlobals = false;
	}

	if (EnableSaveGlobals)
	{
		LocGlobals = AcquireLocal( array< Object ^ >::typeid );

		if (SaveGlobalCount == 0)
		{
			//
			// If we have no globals to save, then set the global array to
			// null.
			//

			ILGen->Emit( OpCodes::Ldnull );
			ILGen->Emit( OpCodes::Stloc, LocGlobals );
		}
		else
		{
			//
			// We have global variables to save.  Emit code to save them into
			// the array.
			//

			ILGen->Emit( OpCodes::Ldc_I4, (Int32) SaveGlobalCount );
			ILGen->Emit( OpCodes::Newarr, Object::typeid );
			ILGen->Emit( OpCodes::Stloc, LocGlobals );

			//
			// Now generate a load of each global, boxing it if necessary, and
			// stash the global into the array.
			//

			for (size_t i = 0; i < SaveGlobalCount; i += 1)
			{
				ILGen->Emit( OpCodes::Ldloc, LocGlobals );
				ILGen->Emit( OpCodes::Ldc_I4, (Int32) i );
				GenerateLoadVariable( SaveGlobals[ i ], true );
				ILGen->Emit( OpCodes::Stelem_Ref );
			}
		}
	}

	if (SaveLocalCount == 0)
	{
		//
		// If we have no locals to save, then set the local array to null.
		//

		ILGen->Emit( OpCodes::Ldnull );
		ILGen->Emit( OpCodes::Stloc, LocLocals );
	}
	else
	{
		//
		// We have local variables to save.  Emit code to save them into the
		// array.
		//

		ILGen->Emit( OpCodes::Ldc_I4, (Int32) SaveLocalCount );
		ILGen->Emit( OpCodes::Newarr, Object::typeid );
		ILGen->Emit( OpCodes::Stloc, LocLocals );

		//
		// Now generate a load of each local, boxing it if necessary, and stash
		// the local into the array.
		//

		for (size_t i = 0; i < SaveLocalCount; i += 1)
		{
			ILGen->Emit( OpCodes::Ldloc, LocLocals );
			ILGen->Emit( OpCodes::Ldc_I4, (Int32) i );
			GenerateLoadVariable( SaveLocals[ i ], true );
			ILGen->Emit( OpCodes::Stelem_Ref );
		}
	}

	//
	// Now duplicate the ScriptProgram (ProgramType) object, and set up for the
	// call to Intrinsic_StoreState.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldProgramInterface );   // this

	if (EnableSaveGlobals)                                            // Globals
		ILGen->Emit( OpCodes::Ldloc, LocGlobals);
	else
		ILGen->Emit( OpCodes::Ldnull );

	ILGen->Emit( OpCodes::Ldloc, LocLocals );                         // Locals
	ILGen->Emit( OpCodes::Ldc_I4, (Int32) ResumeSubPC );              // ResumeSubPC
	ILGen->Emit( OpCodes::Conv_U4 );
	ILGen->Emit( OpCodes::Ldc_I4, (Int32) ResumeSubId );              // ResumeMethodId
	ILGen->Emit( OpCodes::Conv_U4 );
	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Call, m_ILGenCtx->MthCloneScriptProgram );  // ProgramObject
	ILGen->Emit( OpCodes::Callvirt, m_ILGenCtx->MthIntrinsic_StoreState );

	//
	// Now clean up the local array and return.
	//

	if (EnableSaveGlobals)
	{
		ILGen->Emit( OpCodes::Ldnull );
		ILGen->Emit( OpCodes::Stloc, LocGlobals );

		ReleaseLocal( LocGlobals );
	}

	ILGen->Emit( OpCodes::Ldnull );
	ILGen->Emit( OpCodes::Stloc, LocLocals );

	ReleaseLocal( LocLocals );

	UNREFERENCED_PARAMETER( ResumeSub );
}

void
NWScriptCodeGenerator::GenerateCastToBool(
	)
/*++

Routine Description:

	This routine emits any code necessary to cast the value at the top of the
	stack to a Boolean value (logical zero or one).  An I4 is pushed onto the
	stack as the Boolean representation of the previous top of stack.

Arguments:

	None.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	ILGenerator  ^ ILGen;
	Label          L1;
	Label          L2;

	ILGen = m_ILGenCtx->Sub->ILGen;
	L1    = ILGen->DefineLabel( );
	L2    = ILGen->DefineLabel( );

	ILGen->Emit( OpCodes::Brfalse_S, L1 );
	ILGen->Emit( OpCodes::Ldc_I4_1 );
	ILGen->Emit( OpCodes::Br_S, L2 );
	ILGen->MarkLabel( L1 );
	ILGen->Emit( OpCodes::Ldc_I4_0 );
	ILGen->MarkLabel( L2 );
}


void
NWScriptCodeGenerator::GenerateThrowException(
	__in String ^ Description
	)
/*++

Routine Description:

	This routine emits necessary code to support the raising of an exception in
	order to terminate the script program's execution.  A generic
	System::Exception is raised with the given description.

Arguments:

	Description - Supplies the exception description.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	ILGenerator  ^ ILGen;

	ILGen = m_ILGenCtx->Sub->ILGen;

	//
	// Create a new System::Exception with the given description, and then throw it.
	//

	ILGen->Emit( OpCodes::Ldstr, Description );
	ILGen->Emit( OpCodes::Newobj, m_ILGenCtx->CtorException );
	ILGen->Emit( OpCodes::Throw );
}


void
NWScriptCodeGenerator::GenerateProlog(
	)
/*++

Routine Description:

	This routine emits necessary code to support the prolog of a script
	subroutine, such as recursion limit checks.

Arguments:

	None.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
#if NWSCRIPT_EXECUTION_GUARDS
	ILGenerator  ^ ILGen;
	Label          L1;
	LocalBuilder ^ Loc;

	if ((m_ILGenCtx->CodeGenParams != NULL) &&
	    (m_ILGenCtx->CodeGenParams->CodeGenFlags & NWCGF_DISABLE_EXECUTION_GUARDS))
	{
		return;
	}

	ILGen = m_ILGenCtx->Sub->ILGen;
	L1    = ILGen->DefineLabel( );
	Loc   = AcquireLocal( m_ILGenCtx->FldCallDepth->FieldType );

	//
	// Increment the call depth and throw an exception if it has grown too
	// large.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldCallDepth );
	ILGen->Emit( OpCodes::Ldc_I4_1 );
	ILGen->Emit( OpCodes::Add );
	ILGen->Emit( OpCodes::Dup );
	ILGen->Emit( OpCodes::Stloc, Loc );
	ILGen->Emit( OpCodes::Stfld, m_ILGenCtx->FldCallDepth );
	ILGen->Emit( OpCodes::Ldloc, Loc );
#if !NWSCRIPT_FAST_EXEC_GUARDS
	ILGen->Emit( OpCodes::Ldc_I4, m_ILGenCtx->MaxCallDepth );
	ILGen->Emit( OpCodes::Clt_Un );
#endif
	ILGen->Emit( OpCodes::Brtrue_S, L1 );
	GenerateThrowException( "Maximum call depth exceeded." );
	ILGen->MarkLabel( L1 );

	ReleaseLocal( Loc );
#endif
}

void
NWScriptCodeGenerator::GenerateEpilog(
	)
/*++

Routine Description:

	This routine emits necessary code to support the epilog of a script
	subroutine, such as recursion limit checks.

Arguments:

	None.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
#if NWSCRIPT_EXECUTION_GUARDS
	ILGenerator  ^ ILGen;

	if ((m_ILGenCtx->CodeGenParams != NULL) &&
	    (m_ILGenCtx->CodeGenParams->CodeGenFlags & NWCGF_DISABLE_EXECUTION_GUARDS))
	{
		return;
	}

	ILGen = m_ILGenCtx->Sub->ILGen;

	//
	// Decrement the call depth.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldCallDepth );
	ILGen->Emit( OpCodes::Ldc_I4_M1 );
	ILGen->Emit( OpCodes::Add );
	ILGen->Emit( OpCodes::Stfld, m_ILGenCtx->FldCallDepth );
#endif
}


void
NWScriptCodeGenerator::GenerateLoopCheck(
	)
/*++

Routine Description:

	This routine emits necessary code to support a loop check, which increments
	the current loop iterations counter and aborts the script if it has grown
	to be too large.

Arguments:

	None.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
#if NWSCRIPT_EXECUTION_GUARDS
	ILGenerator  ^ ILGen;
	Label          L1;
	LocalBuilder ^ Loc;

	if ((m_ILGenCtx->CodeGenParams != NULL) &&
	    (m_ILGenCtx->CodeGenParams->CodeGenFlags & NWCGF_DISABLE_EXECUTION_GUARDS))
	{
		return;
	}

	ILGen = m_ILGenCtx->Sub->ILGen;
	L1    = ILGen->DefineLabel( );
	Loc   = AcquireLocal( m_ILGenCtx->FldLoopCounter->FieldType );

	//
	// Increment the loop counter and throw an exception if it has grown too
	// large.
	//

	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldarg_0 );
	ILGen->Emit( OpCodes::Ldfld, m_ILGenCtx->FldLoopCounter );
	ILGen->Emit( OpCodes::Ldc_I4_1 );
	ILGen->Emit( OpCodes::Add );
	ILGen->Emit( OpCodes::Dup );
	ILGen->Emit( OpCodes::Stloc, Loc );
	ILGen->Emit( OpCodes::Stfld, m_ILGenCtx->FldLoopCounter );
	ILGen->Emit( OpCodes::Ldloc, Loc );
#if !NWSCRIPT_FAST_EXEC_GUARDS
	ILGen->Emit( OpCodes::Ldc_I4, m_ILGenCtx->MaxLoopIterations );
	ILGen->Emit( OpCodes::Clt_Un );
#endif
	ILGen->Emit( OpCodes::Brtrue_S, L1 );
	GenerateThrowException( "Maximum loop iterations exceeded." );
	ILGen->MarkLabel( L1 );

	ReleaseLocal( Loc );
#endif
}


void
NWScriptCodeGenerator::GenerateBox(
	__in NWACTION_TYPE Type
	)
/*++

Routine Description:

	This routine emits necessary code to box a type should a reference be
	needed on the stack.  The value is only boxed if it was actually a value
	type.

Arguments:

	Type - Supplies the type of the value at the top of the stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	//
	// If we were to store a boxed reference to the stack, then convert the
	// value type to a boxed type if it was a value type.  Currently, the
	// only value types are the intrinsic int/float/object types.
	//

	switch (Type)
	{

	case ACTIONTYPE_INT:
	case ACTIONTYPE_FLOAT:
	case ACTIONTYPE_OBJECT:
	case ACTIONTYPE_VOID:
		m_ILGenCtx->Sub->ILGen->Emit( OpCodes::Box, GetVariableType( Type ) );
		break;

	}
}

void
NWScriptCodeGenerator::GenerateUnbox(
	__in NWACTION_TYPE Type
	)
/*++

Routine Description:

	This routine emits necessary code to unbox a type should a value type be
	needed on the stack.  The value is only unboxed if it was actually a value
	type.

	Reference types are casted to their appropriate types.

Arguments:

	Type - Supplies the type of the value at the top of the stack.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	ILGenerator ^ ILGen;

	ILGen = m_ILGenCtx->Sub->ILGen;

	//
	// If we were to load a value type from a boxed reference on the stack,
	// then unbox the boxed value if it was a value type.  Currently, the
	// only value types are the intrinsic int/float/object types.
	//

	switch (Type)
	{

	case ACTIONTYPE_INT:
	case ACTIONTYPE_FLOAT:
	case ACTIONTYPE_OBJECT:
	case ACTIONTYPE_VOID:
		ILGen->Emit( OpCodes::Unbox_Any, GetVariableType( Type ) );
		break;

	case ACTIONTYPE_STRING:
		ILGen->Emit( OpCodes::Castclass, String::typeid );
		break;

	default:
		if ((Type >= ACTIONTYPE_ENGINE_0) &&
		    (Type <= ACTIONTYPE_ENGINE_9))
		{
			int EType = (int) (Type - ACTIONTYPE_ENGINE_0);

			ILGen->Emit(
				OpCodes::Castclass,
				m_ILGenCtx->EngineStructureTypes[ EType ] );
		}
		break;

	}
}


void
NWScriptCodeGenerator::BuildControlFlowMap(
	__in SubroutineGenContext ^ Sub
	)
/*++

Routine Description:

	This routine generates a mapping of program counter to control flow
	descriptors.  The mapping is used to track which flows have had code
	generated yet, as well as to link jump targets within a subroutine at the
	MSIL level.

Arguments:

	Sub - Supplies the subroutine context for the subroutine to generate the
	      control flow map for.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	const NWScriptSubroutine * IRSub;
	ILGenerator              ^ ILGen;
	ControlFlowMap           ^ FlowMap;
	ControlFlowStack         ^ FlowStack;
	SubroutineControlFlow    ^ EntryFlow;

	IRSub     = Sub->IRSub;
	ILGen     = Sub->ILGen;
	FlowMap   = Sub->Flows;
	FlowStack = Sub->FlowsToEmit;
	EntryFlow = nullptr;

	for (NWNScriptLib::ControlFlowSet::const_iterator it = IRSub->GetControlFlows( ).begin( );
	     it != IRSub->GetControlFlows( ).end( );
	     ++it)
	{
		SubroutineControlFlow ^ Flow = gcnew SubroutineControlFlow;
		int                     i;

		//
		// Create an MSIL label for this flow and link it into the tracking
		// list.  We'll refer back to the label to resolve jump targets in the
		// MSIL generation phase.
		//

		Flow->IRFlow       = it->second.get( );
		Flow->MSILLabel    = ILGen->DefineLabel( );
		Flow->Processed    = false;
		Flow->Enqueued     = false;
		Flow->LocalPool    = nullptr;
		Flow->VarTable     = nullptr;

		if (Flow->IRFlow->GetIR( ).size( ) > LONG_MAX)
			throw gcnew Exception( "Too many IR instructions in control flow." );

		//
		// Index all of the IR instructions in the flow.
		//

		if (!Flow->IRFlow->GetIR( ).empty( ))
		{
			Flow->Instructions = gcnew IRInstructionArr(
				(int) Flow->IRFlow->GetIR( ).size( ) );
		}

		i = 0;

		for (IRInstructionList::iterator iit = Flow->IRFlow->GetIR( ).begin( );
		     iit != Flow->IRFlow->GetIR( ).end( );
		     ++iit)
		{
			Flow->Instructions[ i++ ] = &*iit;
		}

		FlowMap->Add( it->first, Flow );

		if (IsDebugLevel( NWScriptVM::EDL_Verbose ))
		{
			m_TextOut->WriteText(
				"[Sub=%s(%08X)] Flow at %08X discovered.\n",
				IRSub->GetSymbolName( ).c_str( ),
				IRSub->GetAddress( ),
				it->first);
		}

		if (it->first == IRSub->GetAddress( ))
		{
			if (EntryFlow == nullptr)
				EntryFlow = Flow;
			else
				throw gcnew Exception( "Multiple control flows for subroutine entry." );
		}
	}

	//
	// Ensure that the first analyzed flow is the entry point flow for the
	// subroutine.
	//

	if (EntryFlow == nullptr)
		throw gcnew Exception( "No entry flow for subroutine." );

	EntryFlow->Enqueued  = true;
	EntryFlow->LocalPool = gcnew FreeLocalMap;
	EntryFlow->VarTable  = gcnew LocalVariableTable;

	FlowStack->Push( EntryFlow );
}

void
NWScriptCodeGenerator::EnqueueChildFlows(
	__in SubroutineGenContext ^ Sub,
	__in SubroutineControlFlow ^ Flow,
	__in NWScriptControlFlow * IRFlow
	)
/*++

Routine Description:

	This routine enqueues any children of a control flow to the code generation
	queue (if said children were not already enqueued or processed).  Typically
	this routine would be invoked when a flow has been fully emitted.

	The state of the current subroutine generation context is duplicated as
	follows:

	- The free local pool and the variable table are copied.
	- All other portions of the generation context are set to point to their
	  parent.

Arguments:

	Sub - Supplies the subroutine context for the subroutine to generate the
	      cloned queue entry for.

	Flow - Supplies the MSIL-level flow to duplicate.

	IRFlow - Supplies the IR-level flow to duplicate children for.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	for (size_t i = 0; i < 2; i += 1)
	{
		NWScriptControlFlow   * ChildIRFlow;
		SubroutineControlFlow ^ ChildMSILFlow;

		ChildIRFlow = IRFlow->GetChild( i ).get( );

		if (ChildIRFlow == NULL)
			break;

		ChildMSILFlow = GetControlFlow( Sub, ChildIRFlow->GetStartPC( ) );

		//
		// If this control flow has already been emitted, or it is already in
		// the queue, then do not enqueue it again.
		//

		if ((ChildMSILFlow->Processed) || (ChildMSILFlow->Enqueued))
			continue;

		//
		// Otherwise add the flow to the pending queue.  The flow inherits the
		// local variable pool and local variable table of its parent.  (If the
		// flow has multiple parents, it is required that the variable tables
		// are consistent.)
		//

		ChildMSILFlow->Enqueued  = true;
		ChildMSILFlow->LocalPool = gcnew FreeLocalMap;
		ChildMSILFlow->VarTable  = gcnew LocalVariableTable( Flow->VarTable );

		for each (FreeLocalMapKvp Item in Flow->LocalPool)
		{
			ChildMSILFlow->LocalPool->Add(
				Item.Key,
				gcnew LocalStack( Item.Value ) );
		}

		Sub->FlowsToEmit->Push( ChildMSILFlow );
	}
}


Type ^
NWScriptCodeGenerator::GetVariableType(
	__in NWScriptVariable * Var
	)
/*++

Routine Description:

	This routine returns the Type handle for an IR variable.

Arguments:

	Var - Supplies the variable to return the type descriptor for.

Return Value:

	The routine returns a Type handle for the appropriate type.  If the
	variable is not used, nullptr is returned.

Environment:

	User mode, C++/CLI.

--*/
{
	return GetVariableType( Var->GetType( ) );
}

Type ^
NWScriptCodeGenerator::GetVariableType(
	__in NWACTION_TYPE VarType
	)
/*++

Routine Description:

	This routine returns the Type handle for an IR variable.

Arguments:

	VarType - Supplies the variable type to return the type descriptor for.

Return Value:

	The routine returns a Type handle for the appropriate type.  If the
	variable is not used, nullptr is returned.

Environment:

	User mode, C++/CLI.

--*/
{
	switch (VarType)
	{

	case ACTIONTYPE_VOID:
		return Int32::typeid; // Just treat unused values as 'int' for now.

	case ACTIONTYPE_INT:
		return Int32::typeid;

	case ACTIONTYPE_FLOAT:
		return Single::typeid;

	case ACTIONTYPE_STRING:
		return String::typeid;

	case ACTIONTYPE_OBJECT:
		return UInt32::typeid;

	case ACTIONTYPE_VECTOR:
		throw gcnew Exception( "Vector types are not supported in IR." );

	case ACTIONTYPE_ACTION:
		throw gcnew Exception( "Action types are not supported in IR." );

	default:
		{
			String ^ Str;
			int      Vt;

			if ((VarType >= ACTIONTYPE_ENGINE_0) &&
				(VarType <= ACTIONTYPE_ENGINE_9))
			{
				int EngIdx = (VarType - ACTIONTYPE_ENGINE_0);

				return m_ILGenCtx->EngineStructureTypes[ EngIdx ];
			}

			Vt = (int) VarType;

			Str  = "Illegal IR variable type ";
			Str += Vt;
			Str += ".";

			throw gcnew Exception( Str );
		}
		break;

	}
}

NWACTION_TYPE
NWScriptCodeGenerator::GetNWScriptType(
	__in Type ^ MSILType
	)
/*++

Routine Description:

	This routine returns the NWScript type code for an MSIL variable by its
	Type handle.

Arguments:

	MSILType - Supplies the MSIL type handle to return the NWScript type code
	           for.

Return Value:

	The routine returns the NWScript type code (drawn from the NWACTION_TYPE
	family of enumerations) for the given MSIL type.  If there is no mapping to
	NWScript types, i.e. the type is not a valid NWScript type, then a
	System::Exception is raised.

Environment:

	User mode, C++/CLI, invoked at program construction time ONLY.

--*/
{
	if (MSILType == Int32::typeid)
		return ACTIONTYPE_INT;
	else if (MSILType == Single::typeid)
		return ACTIONTYPE_FLOAT;
	else if (MSILType == String::typeid)
		return ACTIONTYPE_STRING;
	else if (MSILType == UInt32::typeid)
		return ACTIONTYPE_OBJECT;
	else
	{
		for (int i = 0; i < NUM_ENGINE_STRUCTURE_TYPES; i += 1)
		{
			if (MSILType == m_ILGenCtx->EngineStructureTypes[ i ])
				return (NWACTION_TYPE) (ACTIONTYPE_ENGINE_0 + i);
		}

		throw gcnew ArgumentException(
			"Invalid type argument to GetNWScriptType." );
	}
}


NWScriptSubroutine *
NWScriptCodeGenerator::GetIRSubroutine(
	__in PROGRAM_COUNTER PC
	)
/*++

Routine Description:

	This routine returns the IR subroutine descriptor for the subroutine at a
	given NWScript program counter.

Arguments:

	PC - Supplies the program counter to look up in the subroutine table.

Return Value:

	On success, the routine returns the IR subroutine that begins at the
	specified program counter.

	On failure, NULL is returned.

Environment:

	User mode, C++/CLI.

--*/
{
	const NWScriptAnalyzer * Analyzer;

	Analyzer = m_ILGenCtx->Analyzer;

	for (NWNScriptLib::SubroutinePtrVec::const_iterator it = Analyzer->GetSubroutines( ).begin( );
	     it != Analyzer->GetSubroutines( ).end( );
	     ++it)
	{
		if (it->get( )->GetAddress( ) == PC)
			return it->get( );
	}

	return NULL;
}

MethodBuilder ^
NWScriptCodeGenerator::GetMSILSubroutine(
	__in PROGRAM_COUNTER PC
	)
/*++

Routine Description:

	This routine returns the MSIL subroutine descriptor for the subroutine at a
	given NWScript program counter.

Arguments:

	PC - Supplies the program counter to look up in the subroutine table.

Return Value:

	On success, the routine returns the MSIL subroutine that begins at the
	specified program counter.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	return m_ILGenCtx->MethodMap[ PC ]->Method;
}

NWScriptCodeGenerator::SubroutineAttributes ^
NWScriptCodeGenerator::GetMSILSubroutineAttributes(
	__in PROGRAM_COUNTER PC
	)
/*++

Routine Description:

	This routine returns the MSIL subroutine attributes for the subroutine at a
	given NWScript program counter.

Arguments:

	PC - Supplies the program counter to look up in the subroutine table.

Return Value:

	On success, the routine returns the MSIL subroutine attributes for the MSIL
	subroutine that begins at the specified program counter.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	return m_ILGenCtx->MethodMap[ PC ];
}

LocalBuilder ^
NWScriptCodeGenerator::GetLocalVariable(
	__in SubroutineGenContext ^ Sub,
	__in NWScriptVariable * Var
	)
/*++

Routine Description:

	This routine retrieves the MSIL local variable context for a given IR
	local variable.

Arguments:

	Sub - Supplies the subroutine to return the local for.

	Var - Supplies the IR variable to return the MSIL local for.

Return Value:

	A handle to the local variable is returned.  On failure, a
	System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	if (Var->GetScope( ) == NWNScriptLib::INVALID_SCOPE)
		throw gcnew Exception( "Attempting to retrieve non-allocated local variable." );

	if ((!(Var->GetFlags( ) & NWScriptVariable::MultiplyCreated)) ||
	    (Var->GetClass( ) == NWScriptVariable::ReturnValue))
		return Sub->CurrentFlow->VarTable[ Var->GetScope( ) ];
	else
		return Sub->MultipleCreatedVarTable[ Var->GetScope( ) ];
}

FieldInfo ^
NWScriptCodeGenerator::GetGlobalVariable(
	__in NWScriptVariable * Var
	)
/*++

Routine Description:

	This routine retrieves the MSIL global variable context for a given IR
	global variable.

Arguments:

	Var - Supplies the IR variable to return the MSIL global for.

Return Value:

	A handle to the global variable is returned.  On failure, a
	System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	if (Var->GetScope( ) == NWNScriptLib::INVALID_SCOPE)
		throw gcnew Exception( "Attempting to retrieve non-allocated global variable." );

	return m_ILGenCtx->Globals[ Var->GetScope( ) ];
}

NWScriptCodeGenerator::SubroutineControlFlow ^
NWScriptCodeGenerator::GetControlFlow(
	__in SubroutineGenContext ^ Sub,
	__in PROGRAM_COUNTER PC
	)
/*++

Routine Description:

	This routine retrieves the control flow descriptor for a given PC within
	a subroutine being built.

Arguments:

	Sub - Supplies the subroutine to return the control flow for.

	PC - Supplies the program counter value to look up the control flow for.

Return Value:

	A handle to the control flow descriptor is returned.  On failure, a
	System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	return Sub->Flows[ PC ];
}

Label
NWScriptCodeGenerator::GetLabel(
	__in NWScriptLabel * Label
	)
/*++

Routine Description:

	This routine retrieves the MSIL label for a given IR label.

Arguments:

	Label - Supplies the IR label to look up.

Return Value:

	The corresponding MSIL label is returned.  On failure, a System::Exception
	is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	return GetControlFlow(
		m_ILGenCtx->Sub,
		Label->GetControlFlow( )->GetStartPC( ) )->MSILLabel;
}

Label
NWScriptCodeGenerator::GetLabel(
	__in NWScriptControlFlow * Flow
	)
/*++

Routine Description:

	This routine retrieves the MSIL label for a given IR control flow.

Arguments:

	Flow - Supplies the IR control flow to look up.

Return Value:

	The corresponding MSIL label is returned.  On failure, a System::Exception
	is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	return GetControlFlow(
		m_ILGenCtx->Sub,
		Flow->GetStartPC( ) )->MSILLabel;
}

const char *
NWScriptCodeGenerator::GetIRInstructionName(
	__in NWScriptInstruction::INSTR Instr
	)
/*++

Routine Description:

	This routine retrieves the textural name of an IR instruction.

Arguments:

	Instr - Supplies the instruction to return the textural name for.

Return Value:

	The textural name for the given IR instruction is returned.

Environment:

	User mode, C++/CLI.

--*/
{
	switch (Instr)
	{

	case NWScriptInstruction::I_CREATE:
		return "I_CREATE";
	case NWScriptInstruction::I_DELETE:
		return "I_DELETE";
	case NWScriptInstruction::I_ASSIGN:
		return "I_ASSIGN";
	case NWScriptInstruction::I_JZ:
		return "I_JZ";
	case NWScriptInstruction::I_JNZ:
		return "I_JNZ";
	case NWScriptInstruction::I_JMP:
		return "I_JMP";
	case NWScriptInstruction::I_CALL:
		return "I_CALL";
	case NWScriptInstruction::I_RETN:
		return "I_RETN";
	case NWScriptInstruction::I_ACTION:
		return "I_ACTION";
	case NWScriptInstruction::I_SAVE_STATE:
		return "I_SAVE_STATE";
	case NWScriptInstruction::I_LOGAND:
		return "I_LOGAND";
	case NWScriptInstruction::I_LOGOR:
		return "I_LOGOR";
	case NWScriptInstruction::I_INCOR:
		return "I_INCOR";
	case NWScriptInstruction::I_EXCOR:
		return "I_EXCOR";
	case NWScriptInstruction::I_BOOLAND:
		return "I_BOOLAND";
	case NWScriptInstruction::I_EQUAL:
		return "I_EQUAL";
	case NWScriptInstruction::I_NEQUAL:
		return "I_NEQUAL";
	case NWScriptInstruction::I_GEQ:
		return "I_GEQ";
	case NWScriptInstruction::I_GT:
		return "I_GT";
	case NWScriptInstruction::I_LT:
		return "I_LT";
	case NWScriptInstruction::I_LEQ:
		return "I_LEQ";
	case NWScriptInstruction::I_SHLEFT:
		return "I_SHLEFT";
	case NWScriptInstruction::I_SHRIGHT:
		return "I_SHRIGHT";
	case NWScriptInstruction::I_USHRIGHT:
		return "I_USHRIGHT";
	case NWScriptInstruction::I_ADD:
		return "I_ADD";
	case NWScriptInstruction::I_SUB:
		return "I_SUB";
	case NWScriptInstruction::I_MUL:
		return "I_MUL";
	case NWScriptInstruction::I_DIV:
		return "I_DIV";
	case NWScriptInstruction::I_MOD:
		return "I_MOD";
	case NWScriptInstruction::I_NEG:
		return "I_NEG";
	case NWScriptInstruction::I_COMP:
		return "I_COMP";
	case NWScriptInstruction::I_NOT:
		return "I_NOT";
	case NWScriptInstruction::I_INC:
		return "I_INC";
	case NWScriptInstruction::I_DEC:
		return "I_DEC";
	case NWScriptInstruction::I_TEST:
		return "I_TEST";
	case NWScriptInstruction::I_INITIALIZE:
		return "I_INITIALIZE";
	default:
		return "???";
	}
}


bool
NWScriptCodeGenerator::IsIgnoredCallInstr(
	__in SubroutineGenContext ^ Sub,
	__in NWScriptSubroutine * CalledSub
	)
/*++

Routine Description:

	This routine determines whether a subroutine call should be ignored instead
	of actually emitted.  This is to support the behavior of ignoring the call
	to the entry point from #globals, as the generated IR will not contain the
	correct actual instructions to successfully invoke the entry point (and in
	fact we will not want it to as we really call #globals from the very start
	of the entry point).

Arguments:

	Sub - Supplies the subroutine being generated.

	CalledSub - Supplies the IR subroutine being invoked.

Return Value:

	The corresponding MSIL label is returned.  On failure, a System::Exception
	is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	if (Sub->Flags & (unsigned long) GSUB_FLAGS::GSF_GLOBALS)
	{
		//
		// If we are #globals calling the entry point, do not emit the call.
		// Instead, fill the return value with default values.  We will have
		// restructured the program such that the entry point calls #globals,
		// and not the other way around.  The generated IR does not match the
		// signature anyway for this one call.
		//

		if (CalledSub->GetAddress( ) == m_ILGenCtx->EntryPC)
			return true;
	}

	return false;
}



String ^
NWScriptCodeGenerator::GenerateAsmName(
	__in String ^ Name,
	__in bool InterfaceLayer
	)
/*++

Routine Description:

	This routine builds a name for the dynamic assembly to store the script
	code in.

Arguments:

	Name - Supplies the name prefix to apply to the type within which the
	       script program MSIL representation is placed.

	InterfaceLayer - Supplies a Boolean value indicating true if the assembly
	                 to be generated is an interface layer module and not a
	                 script program.

Return Value:

	On success, a handle to the assembly name string is returned.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	if (InterfaceLayer == true)
		return Name;

	StringBuilder ^ AsmNameBuilder = gcnew StringBuilder( Name->Length + 12 );

	AsmNameBuilder->Append( "NWScriptAsm_" );

	for (int i = 0; i < Name->Length; i += 1)
	{
		switch (Name[ i ])
		{

		case ':':
		case '/':
		case '\\':
		case ' ':
		case '\r':
		case '\n':
		case '\t':
			AsmNameBuilder->Append( '_' );
			break;

		default:
			AsmNameBuilder->Append( Name[ i ] );
			break;

		}
	}

	return AsmNameBuilder->ToString( );
}

String ^
NWScriptCodeGenerator::GenerateProgramNamespace(
	__in String ^ Name
	)
/*++

Routine Description:

	This routine builds a name for the dynamic namespace to store the script
	code in.

Arguments:

	Name - Supplies the name prefix to apply to the type within which the
	       script program MSIL representation is placed.

Return Value:

	On success, a handle to the generated namespace string is returned.

	On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	StringBuilder ^ ProgNameBuilder = gcnew StringBuilder( Name->Length + 17 + 1 );

	ProgNameBuilder->Append( "NWScript.JITCode." );

	for (int i = 0; i < Name->Length; i += 1)
	{
		switch (Name[ i ])
		{

		case ':':
		case '/':
		case '\\':
		case ' ':
		case '\r':
		case '\n':
		case '\t':
			ProgNameBuilder->Append( "_" );
			break;

		default:
			ProgNameBuilder->Append( Name[ i ] );
			break;

		}
	}

	ProgNameBuilder->Append( "." );

	return ProgNameBuilder->ToString( );
}


void
NWScriptCodeGenerator::AssertTypeEqual(
	__in Type ^ Type1,
	__in Type ^ Type2
	)
/*++

Routine Description:

	This routine issues an error diagnostic if two types are not equal.

Arguments:

	Type1 - Supplies the first type.

	Type2 - Supplies the second type.

Return Value:

	None.  On failure, a System::Exception is raised.

Environment:

	User mode, C++/CLI.

--*/
{
	if (Type1 == Type2)
		return;

	throw gcnew Exception( "Discovered type mismatch in IR during code generation." );
}
