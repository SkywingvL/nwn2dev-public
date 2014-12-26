/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptCodeGenerator.h

Abstract:

	This module defines the code generator for NWScript.  Its purpose is to
	construct an assembly given a source NWScript program in IR representation.

--*/

#ifndef _SOURCE_PROGRAMS_NWNSCRIPTJIT_NWSCRIPTCODEGENERATOR_H
#define _SOURCE_PROGRAMS_NWNSCRIPTJIT_NWSCRIPTCODEGENERATOR_H

#ifdef _MSC_VER
#pragma once
#endif

//
// N.B.  Additional switches in NWScriptCodeGenerator.cpp.
//

//
// Define to 1 to enable direct fast action service calls.
//

#define NWSCRIPT_DIRECT_FAST_ACTION_CALLS    1


namespace NWScript
{

ref class NWScriptSavedState;
ref class NWScriptEngineStructure;
ref class NWScriptProgram;

//
// Define state for a generated action service method.
//

value struct ActionServiceMethodInfo
{
	System::Reflection::MethodInfo ^ Method;
	Int32                            NumParams;
};

//
// Define the representation of the NWScript code generator object.
//

public ref class NWScriptCodeGenerator
{

public:

	//
	// Define state about a generated script program.
	//

	typedef array< NWACTION_TYPE > ActionTypeArr;

	value struct ProgramInfo
	{
		//
		// Define the assembly containing the generated program.
		//

		Assembly        ^ Assembly;

		//
		// Define the types for engine structures.
		//

		array< Type ^ > ^ EngineStructureTypes;

		//
		// Define the main type for the generated program, which conforms to
		// the interface type passed as ScriptInterfaceType, canonically
		// specified as IGeneratedScriptProgram::typeid.
		//

		Type            ^ Type;
	};

	//
	// Construct a new script code generator.
	//

	NWScriptCodeGenerator(
		__in_opt IDebugTextOut * TextOut,
		__in ULONG DebugLevel,
		__in Type ^ HostInterfaceType,
		__in Type ^ ScriptInterfaceType
		);

	//
	// Destruct a NWScriptCodeGenerator instance.
	//

	inline
	~NWScriptCodeGenerator(
		)
	{
		this->!NWScriptCodeGenerator( );
	}

	!NWScriptCodeGenerator(
		);

	//
	// Generate code for a script.
	//

	void
	GenerateProgramCode(
		__in const NWScriptAnalyzer * Analyzer,
		__in INWScriptActions * ActionHandler,
		__in NWN::OBJECTID ObjectInvalid,
		__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams,
		__in String ^ Name,
		__in Encoding ^ StringEncoding,
		__out ProgramInfo % Program
		);

	//
	// Generate code for the CLR interface DLL.
	//
	
	void
	GenerateInterfaceLayerCode(
		__in const NWScriptAnalyzer * Analyzer,
		__in INWScriptActions * ActionHandler,
		__in NWN::OBJECTID ObjectInvalid,
		__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams,
		__in String ^ Name,
		__in Encoding ^ StringEncoding,
		__out ProgramInfo % Program
		);

private:

	typedef array< NWACTION_TYPE > ActionTypeArr;

	ref struct SubroutineGenContext;
	ref struct SubroutineControlFlow;
	ref struct ILGenContext;

	typedef System::Collections::Generic::List< SubroutineGenContext ^ > SubroutineGenQueue;
	typedef System::Collections::Generic::List< MethodBuilder ^ > MethodBuilderList;

	typedef NWNScriptLib::PROGRAM_COUNTER PROGRAM_COUNTER;

	//
	// Define attributes about a generated subroutine.
	//

	ref struct SubroutineAttributes
	{
		//
		// Define the associated MSIL method.
		//

		MethodBuilder      ^ Method;

		//
		// Define the associated IR subroutine.
		//

		NWScriptSubroutine * IRSub;

		//
		// Define the resume method id (if any).
		//

		UInt32               ResumeMethodId;
	};

	//
	// Define a mapping of PC addresses to subroutines to invoke.
	//

	typedef System::Collections::Generic::Dictionary< PROGRAM_COUNTER, SubroutineAttributes ^ > PCMethodMap;
	typedef System::Collections::Generic::List< SubroutineAttributes ^ > SubroutineAttrList;

	enum class GSUB_FLAGS
	{
		//
		// We are generating #globals.
		//

		GSF_GLOBALS          = 0x00000001,

		//
		// We are generating the actual entry point symbol.
		//

		GSF_ENTRY_POINT      = 0x00000002,

		//
		// We're generating a save state resume subroutine that will be called
		// as a script situation.
		//

		GSF_SCRIPT_SITUATION = 0x00000004,

		GSF_LAST
	};

	//
	// Perform overall common setup for code generation.
	//

	void
	SetupCodeGeneration(
		__in const NWScriptAnalyzer * Analyzer,
		__in INWScriptActions * ActionHandler,
		__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams,
		__in String ^ Name,
		__in Encoding ^ StringEncoding,
		__in bool InterfaceLayer,
		__out AssemblyBuilder ^ % Assembly,
		__out ModuleBuilder ^ % Module,
		__out bool % SaveAsm
		);

	//
	// Create and initialize a new ILGenContext.
	//

	ILGenContext ^
	CreateILGenContext(
		__in const NWScriptAnalyzer * Analyzer,
		__in_opt PCNWSCRIPT_JIT_PARAMS CodeGenParams
		);

	//
	// Generate the overarching assembly for the script program.
	//

	AssemblyBuilder ^
	GenerateProgramAssembly(
		__in AppDomain ^ Domain,
		__in String ^ Name,
		__in_opt String ^ OutputDir,
		__in bool SaveAsm,
		__in bool InterfaceLayer
		);

	//
	// Generate the overarching module for the script program.
	//

	ModuleBuilder ^
	GenerateProgramModule(
		__in AssemblyBuilder ^ Assembly
		);

	//
	// Generate the overarching class type for the script program.
	//

	TypeBuilder ^
	GenerateProgramType(
		__in ModuleBuilder ^ Module,
		__in String ^ Name,
		__in NWN::OBJECTID ObjectInvalid
		);

	//
	// Generate the overarching class type for the interface layer module.
	//

	TypeBuilder ^
	GenerateInterfaceLayerType(
		__in ModuleBuilder ^ Module,
		__in String ^ Name,
		__in NWN::OBJECTID ObjectInvalid
		);

	//
	// Retrieve type members relating to JIT intrinsics.
	//

	void
	RetrieveJITIntrinsicsTypeInfo(
		__in Type ^ NWScriptJITIntrinsicsType
		);

	//
	// Generate the CloneScriptProgram method on the script program class type.
	//

	MethodInfo ^
	GenerateCloneScriptProgram(
		__in TypeBuilder ^ ProgramType,
		__in ConstructorBuilder ^ ProgCtor
		);

	//
	// Generate the LoadScriptGlobals method on the script program class type.
	//

	MethodInfo ^
	GenerateLoadScriptGlobals(
		__in TypeBuilder ^ ProgramType
		);

	//
	// Generate the ExecuteScript method on the script program class type.
	//

	MethodInfo ^
	GenerateExecuteScript(
		__in TypeBuilder ^ ProgramType
		);

	//
	// Generate the ExecuteScriptSituation method on the script program class
	// type.
	//

	MethodInfo ^
	GenerateExecuteScriptSituation(
		__in TypeBuilder ^ ProgramType
		);

	//
	// Generate a readonly property.
	//

	PropertyBuilder ^
	GenerateReadOnlyProperty(
		__in TypeBuilder ^ ParentType,
		__in FieldBuilder ^ BackingField,
		__in String ^ PropertyName
		);

	//
	// Generate a literal compile-time constant field.
	//

	FieldBuilder ^
	GenerateLiteralField(
		__in TypeBuilder ^ ParentType,
		__in String ^ FieldName,
		__in Object ^ FieldLiteralValue
		);

	//
	// Generate global variable storage for the script program.  Only storage
	// within the class type is created; initialization code is not emitted.
	//

	void
	GenerateGlobals(
		__in TypeBuilder ^ ProgramType
		);

	//
	// Generate code for all subroutines in the program IR.
	//

	void
	GenerateAllSubroutines(
		);

	//
	// Generate code for all subroutines in the interface layer module.
	//

	void
	GenerateInterfaceLayerActionServiceSubroutines(
		);

	//
	// Generate code for a subroutine.
	//

	void
	GenerateSubroutineCode(
		__in SubroutineGenContext ^ Sub
		);

	//
	// Annotate special variables (e.g. return values and parameters) that are
	// not created by I_CREATE instructions.
	//

	void
	AnnotateSpecialVariables(
		__in SubroutineGenContext ^ Sub
		);

	//
	// Generate the MSIL equivalent of a single IR instruction.
	//

	void
	GenerateMSILForIRInstruction(
		__in SubroutineGenContext ^ Sub,
		__in SubroutineControlFlow ^ Flow,
		__in NWScriptInstruction & IRInstr
		);


	//
	// Stack push helpers.  These routines emit IL code to store a value onto
	// the VM stack for an action service handler call.
	//

	void
	GenerateVMStackPush(
		__in ILGenerator ^ ILGen,
		__in Int32 i
		);

	void
	GenerateVMStackPush(
		__in ILGenerator ^ ILGen,
		__in UInt32 o // NWN::OBJECTID
		);

	void
	GenerateVMStackPush(
		__in ILGenerator ^ ILGen,
		__in Single f
		);

	void
	GenerateVMStackPush(
		__in ILGenerator ^ ILGen,
		__in String ^ s
		);

	void
	GenerateVMStackPush(
		__in ILGenerator ^ ILGen,
		__in LocalBuilder ^ Local
		);

	void
	GenerateVMStackPush(
		__in ILGenerator ^ ILGen,
		__in NWScriptVariable * Var
		);

	void
	GenerateVMStackPush(
		__in ILGenerator ^ ILGen,
		__in Type ^ ArgType,
		__in short ArgSlot
		);

	//
	// Stack pop helpers.  These routine emit IL code to retrieve a value from
	// the VM stack for an action service handler call.
	//

	void
	GenerateVMStackPop(
		__in ILGenerator ^ ILGen,
		__in LocalBuilder ^ Local
		);

	void
	GenerateVMStackPop(
		__in ILGenerator ^ ILGen,
		__in NWScriptVariable * Var
		);


	//
	// Acquire a local variable for use within the current subroutine.  The
	// local may be drawn from the free pool, or created anew (if necessary).
	//

	LocalBuilder ^
	AcquireLocal(
		__in Type ^ LocalType
		);

	//
	// Release a local variable back to the current subroutine's local variable
	// pool.
	//

	void
	ReleaseLocal(
		__in LocalBuilder ^ Local
		);

	//
	// Create (and register) a local in the appropriate place.
	//

	LocalBuilder ^
	CreateLocal(
		__in NWScriptVariable * Var
		);

	
	//
	// Generate a return type for an IR subroutine.  Should the subroutine
	// return an aggregate type, an aggregate type descriptor is manufactured
	// and returned.
	//

	Type ^
	GenerateSubroutineRetType(
		__in const NWScriptSubroutine * IRSub
		);

	//
	// Create a composite return type for an IR subroutine that returns an
	// aggregate value.
	//

	Type ^
	BuildCompositeReturnType(
		__in const NWScriptSubroutine * IRSub
		);

	//
	// Generate code to store variables to the return value of a subroutine.
	//

	LocalBuilder ^
	GeneratePackReturnValue(
		__in const NWScriptSubroutine * IRSub,
		__in_ecount( ReturnValueCount ) NWScriptVariable * * ReturnValues,
		__in size_t ReturnValueCount
		);

	//
	// Generate code to unpack a return value from a subroutine.
	//

	void
	GenerateUnpackReturnValue(
		__in const NWScriptSubroutine * IRSub,
		__in MethodBuilder ^ MSILSub,
		__in_ecount( ReturnValueCount ) NWScriptVariable * * ReturnValues,
		__in size_t ReturnValueCount
		);


	//
	// Generate code to call an action service handler.
	//

	void
	GenerateExecuteActionService(
		__in NWSCRIPT_ACTION CalledActionId,
		__in NWNScriptLib::VariableWeakPtrVec * ParamList,
		__in size_t ParamCount
		);

	//
	// Generate a slow call to an action service handler.
	//

	void
	GenerateExecuteActionServiceSlow(
		__in PCNWACTION_DEFINITION CalledAction,
		__in NWNScriptLib::VariableWeakPtrVec * ParamList,
		__in size_t ParamCount,
		__in size_t ReturnCount,
		__in_ecount( ReturnCount ) PCNWACTION_TYPE ReturnTypes
		);

	//
	// Generate a fast call to an action service handler.  The fast call must
	// not involved an engine structure.
	//

	void
	GenerateExecuteActionServiceFast(
		__in PCNWACTION_DEFINITION CalledAction,
		__in NWNScriptLib::VariableWeakPtrVec * ParamList,
		__in size_t ParamCount,
		__in size_t ReturnCount,
		__in_ecount( ReturnCount ) PCNWACTION_TYPE ReturnTypes
		);

#if NWSCRIPT_DIRECT_FAST_ACTION_CALLS

	//
	// Generate a direct call to an action service handler.
	//

	void
	GenerateExecuteActionServiceFastDirect(
		__in PCNWACTION_DEFINITION CalledAction,
		__in NWNScriptLib::VariableWeakPtrVec * ParamList,
		__in size_t ParamCount,
		__in size_t ReturnCount,
		__in_ecount( ReturnCount ) PCNWACTION_TYPE ReturnTypes
		);

	//
	// Create a fast action service handler stub (if it did not already exist).
	//

	MethodInfo ^
	AcquireFastDirectActionServiceStub(
		__in PCNWACTION_DEFINITION CalledAction,
		__in size_t ParamCount,
		__in size_t SrcParams
		);

	//
	// Create a fast direct action service handler stub.
	//

	MethodInfo ^
	GenerateFastDirectActionServiceStub(
		__in PCNWACTION_DEFINITION CalledAction,
		__in size_t ParamCount,
		__in size_t SrcParams
		);

#endif // NWSCRIPT_DIRECT_FAST_ACTION_CALLS

	//
	// Generate a slow call to an action service handler via a stub.
	//

	void
	GenerateExecuteActionServiceSlowViaStub(
		__in PCNWACTION_DEFINITION CalledAction,
		__in NWNScriptLib::VariableWeakPtrVec * ParamList,
		__in size_t ParamCount,
		__in size_t ReturnCount,
		__in_ecount( ReturnCount ) PCNWACTION_TYPE ReturnTypes
		);

	//
	// Create a slow action service handler stub (if it did not already exist).
	//

	MethodInfo ^
	AcquireSlowActionServiceStub(
		__in PCNWACTION_DEFINITION CalledAction,
		__in size_t ParamCount,
		__in size_t SrcParams
		);

	//
	// Create a slow action service handler stub.
	//

	MethodInfo ^
	GenerateSlowActionServiceStub(
		__in PCNWACTION_DEFINITION CalledAction,
		__in size_t ParamCount,
		__in size_t SrcParams
		);


	//
	// Generate action service handlers for optimized action services that are
	// promoted to intrinsics.
	//

	void
	GenerateOptimizedActionServiceStubs(
		);

	//
	// Generate an optimized action service handler for a given standard NWN
	// action.
	//

	MethodInfo ^
	GenerateOptimizedNWNCompatibleActionServiceStub(
		__in String ^ ActionName,
		__in String ^ StubName
		);

	//
	// Generate optimized code for the GetStringLength action.
	//

	MethodInfo ^
	GenerateOptAction_GetStringLength(
		__in String ^ StubName
		);

	//
	// Generate optimized code for the GetStringLeft action.
	//

	MethodInfo ^
	GenerateOptAction_GetStringLeft(
		__in String ^ StubName
		);

	//
	// Generate optimized code for the GetStringRight action.
	//

	MethodInfo ^
	GenerateOptAction_GetStringRight(
		__in String ^ StubName
		);

	//
	// Generate optimized code for the IntToString action.
	//

	MethodInfo ^
	GenerateOptAction_IntToString(
		__in String ^ StubName
		);

	//
	// Generate optimized code for the GetSubString action.
	//

	MethodInfo ^
	GenerateOptAction_GetSubString(
		__in String ^ StubName
		);

	//
	// Execution stack variable manipulation.  These helpers emit MSIL
	// instructions to load an IR variable's value to the execution stack, or
	// to store the top of the execution stack to an IR variable.
	//

	void
	GenerateLoadVariable(
		__in NWScriptVariable * Var
		);

	void
	GenerateLoadVariable(
		__in NWScriptVariable * Var,
		__in bool Box
		);

	void
	GenerateStoreVariable(
		__in NWScriptVariable * Var
		);

	void
	GenerateStoreVariable(
		__in NWScriptVariable * Var,
		__in bool Unbox
		);


	//
	// Begin generation of a new subroutine.
	//

	MethodBuilder ^
	BeginNewSubroutine(
		__in String ^ Name,
		__in MethodAttributes Attributes,
		__in Type ^ ReturnType,
		__in_opt array< Type ^ > ^ ParameterTypes
		);


	//
	// Generate instructions to assign the default value to a local.
	//

	void
	GeneratePushDefaultValue(
		__in Type ^ LocalType
		);

	void
	GenerateSetDefaultValue(
		__in LocalBuilder ^ Local
		);

	void
	GenerateSetDefaultValue(
		__in NWScriptVariable * Var
		);

	
	//
	// Generate instructions to compare equality of two variables.
	//

	void
	GenerateCompare(
		__in NWScriptVariable * Var1,
		__in NWScriptVariable * Var2,
		__in NWScriptVariable * Result,
		__in OpCode Op,
		__in bool Negate
		);


	//
	// Generate instructions to perform a binary operation with a restricted
	// type id.
	//

	void
	GenerateBinaryOp(
		__in NWScriptVariable * Var1,
		__in NWScriptVariable * Var2,
		__in NWScriptVariable * Result,
		__in OpCode Op,
		__in NWACTION_TYPE ReqType
		);

	void
	GenerateBinaryOp(
		__in NWScriptVariable * Var1,
		__in NWScriptVariable * Var2,
		__in NWScriptVariable * Result,
		__in OpCode Op,
		__in NWACTION_TYPE ReqType,
		__in NWACTION_TYPE ReqRetType
		);


	//
	// Generate instructions to perform a unary operation with a restricted type
	// id.
	//

	void
	GenerateUnaryOp(
		__in NWScriptVariable * Var,
		__in NWScriptVariable * Result,
		__in OpCode Op,
		__in NWACTION_TYPE ReqType
		);

	void
	GenerateUnaryOp(
		__in NWScriptVariable * Var,
		__in NWScriptVariable * Result,
		__in OpCode Op,
		__in NWACTION_TYPE ReqType,
		__in NWACTION_TYPE ReqRetType
		);


	//
	// Generate instructions to perform an upcast binary operation, such as a
	// multiply, divide, add, or subtract.
	//
	// Upcast binary operations allow mixing of int32/float values, with
	// automatic upcast conversion to float.
	//

	void
	GenerateUpcastBinaryOp(
		__in NWScriptVariable * Var1,
		__in NWScriptVariable * Var2,
		__in NWScriptVariable * Result,
		__in OpCode Op
		);


	//
	// Generate instructions to save the current visible VM state for an
	// I_SAVE_STATE instruction.
	//

	void
	GenerateSaveState(
		__in MethodInfo ^ ResumeSub,
		__in UInt32 ResumeSubId,
		__in PROGRAM_COUNTER ResumeSubPC,
		__in_ecount( SaveLocalCount ) NWScriptVariable * * SaveLocals,
		__in size_t SaveLocalCount,
		__in_ecount( SaveGlobalCount ) NWScriptVariable * * SaveGlobals,
		__in size_t SaveGlobalCount
		);


	//
	// Generate instructions to cast the top of stack to a Boolean value.
	//

	void
	GenerateCastToBool(
		);


	//
	// Generate instructions to throw a string as a System::Exception.
	//

	void
	GenerateThrowException(
		__in String ^ Description
		);


	//
	// Generate prologue instructions for a subroutine.
	//

	void
	GenerateProlog(
		);

	//
	// Generate epilogue instructions for a subroutine.
	//

	void
	GenerateEpilog(
		);


	//
	// Generate loop counter checks for a potential loop (to avoid forever
	// loops hanging the host indefinitely).
	//

	void
	GenerateLoopCheck(
		);


	//
	// Generate instructions to box a type, if the type was a value type.
	//

	void
	GenerateBox(
		__in NWACTION_TYPE Type
		);

	//
	// Generate instructions to unbox a type, if the type was a value type.
	//

	void
	GenerateUnbox(
		__in NWACTION_TYPE Type
		);


	//
	// Generate the control flow map for a subroutine.
	//

	void
	BuildControlFlowMap(
		__in SubroutineGenContext ^ Sub
		);

	//
	// Enqueue any not-yet-discovered child flows of a given flow to the
	// analysis queue.
	//

	void
	EnqueueChildFlows(
		__in SubroutineGenContext ^ Sub,
		__in SubroutineControlFlow ^ Flow,
		__in NWScriptControlFlow * IRFlow
		);


	//
	// Return the type for a variable.
	//

	Type ^
	GetVariableType(
		__in NWScriptVariable * Var
		);

	Type ^
	GetVariableType(
		__in NWACTION_TYPE ActionType
		);

	//
	// Discern the NWScript type of an object given its MSIL type.
	//

	NWACTION_TYPE
	GetNWScriptType(
		__in Type ^ MSILType
		);

	//
	// Return an IR subroutine for a particular PC.
	//

	NWScriptSubroutine *
	GetIRSubroutine(
		__in PROGRAM_COUNTER PC
		);

	//
	// Return an MSIL subroutine for a particular PC.
	//

	MethodBuilder ^
	GetMSILSubroutine(
		__in PROGRAM_COUNTER PC
		);

	//
	// Return the subroutine attributes for a particular PC.
	//

	SubroutineAttributes ^
	GetMSILSubroutineAttributes(
		__in PROGRAM_COUNTER PC
		);

	//
	// Retrieve a local variable within a subroutine.
	//

	LocalBuilder ^
	GetLocalVariable(
		__in SubroutineGenContext ^ Sub,
		__in NWScriptVariable * Var
		);

	//
	// Retrieve a global variable.
	//

	FieldInfo ^
	GetGlobalVariable(
		__in NWScriptVariable * Var
		);

	//
	// Retrieve a control flow for a particular PC.
	//

	SubroutineControlFlow ^
	GetControlFlow(
		__in SubroutineGenContext ^ Sub,
		__in PROGRAM_COUNTER PC
		);

	//
	// Retrieve an MSIL label for a given IR label.
	//

	Label
	GetLabel(
		__in NWScriptLabel * Label
		);

	//
	// Retrieve an MSIL label for a given IR control flow.
	//

	Label
	GetLabel(
		__in NWScriptControlFlow * Flow
		);

	//
	// Return the name for an IR instruction.
	//

	const char *
	GetIRInstructionName(
		__in NWScriptInstruction::INSTR Instr
		);

	//
	// Determine whether a subroutine call should be skipped during IR
	// generation.  This is used to allow us to effectively nop out the call to
	// the entry point in #globals.
	//

	bool
	IsIgnoredCallInstr(
		__in SubroutineGenContext ^ Sub,
		__in NWScriptSubroutine * CalledSub
		);


	//
	// Generate the assembly name for a script.
	//

	String ^
	GenerateAsmName(
		__in String ^ Name,
		__in bool InterfaceLayer
		);

	//
	// Generate the main program namespace for the script.
	//

	String ^
	GenerateProgramNamespace(
		__in String ^ Name
		);



	//
	// Emit an error diagnostic if variable types do not match (for code
	// generation purposes).
	//

	void
	AssertTypeEqual(
		__in Type ^ Type1,
		__in Type ^ Type2
		);
	

	//
	// Determine whether a debug level is enabled.
	//


	inline
	bool
	IsDebugLevel(
		__in ULONG DebugLevel
		)
	{
		return ((m_TextOut != NULL) && (m_DebugLevel >= DebugLevel));
	}


	//
	// Define the count of supported engine structure types.
	//

	static const int NUM_ENGINE_STRUCTURE_TYPES = 10;

	//
	// Define the maximum supported call depth.
	//

	static const int MAX_CALL_DEPTH             = 128;

	//
	// Define the maximum number of backwards jumps (probable loop iterations)
	// permitted.
	//

	static const int MAX_LOOP_ITERATIONS        = 100000;

	//
	// Define a pool of local slots available for use.
	//

	typedef System::Collections::Generic::Stack< LocalBuilder ^ > LocalStack;

	//
	// Define a mapping of types to free variable stacks.
	//

	typedef System::Collections::Generic::Dictionary< Type ^, LocalStack ^ > FreeLocalMap;
	typedef System::Collections::Generic::KeyValuePair< Type ^, LocalStack ^ > FreeLocalMapKvp;

	//
	// Define a local variable table (locals within a subroutine).
	//

	typedef System::Collections::Generic::List< LocalBuilder ^ > LocalVariableTable;

	//
	// Define a global variable table (fields within the program class type).
	//

	typedef array< FieldInfo ^ > GlobalVariableTable;

	typedef array< NWScriptInstruction * > IRInstructionArr;

	typedef array< array < ActionServiceMethodInfo > ^ > ActionMethodInfoArrArr;

	//
	// Define control flow state.
	//

	ref struct SubroutineControlFlow
	{
		//
		// Define the underlying IR-level control flow.
		//

		NWScriptControlFlow * IRFlow;

		//
		// Define the associated MSIL label for the start of the control flow.
		//

		Label                 MSILLabel;

		//
		// Define whether the label has had code generation processed (or is
		// currently processing), or whether it has not yet been analyzed.
		//

		bool                  Processed;

		//
		// Define whether the label is already enqueued for processing.
		//

		bool                  Enqueued;

		//
		// Define the IR instructions for this flow.
		//

		IRInstructionArr    ^ Instructions;

		//
		// Define the pool of free local variable slots (per type) within this
		// flow.
		//

		FreeLocalMap        ^ LocalPool;

		//
		// Define the variable table for the subroutine.  The variable table is
		// indexed by the 'scope' field in a NWScriptVariable in order to yield
		// a LocalBuilder ^ for the IL-level variable.
		//

		LocalVariableTable  ^ VarTable;
	};

	//
	// Define a mapping of control flow start PC to control flow descriptor.
	//

	typedef System::Collections::Generic::Dictionary< PROGRAM_COUNTER, SubroutineControlFlow ^ > ControlFlowMap;
	typedef System::Collections::Generic::Stack< SubroutineControlFlow ^ > ControlFlowStack;

	typedef std::list< NWScriptInstruction > IRInstructionList;

	//
	// Define the context state for the currently being constructed NWScript
	// subroutine.
	//

	ref struct SubroutineGenContext
	{
		//
		// Define the IR subroutine that we are working on.
		//

		NWScriptSubroutine                * IRSub;

		//
		// Define the IR instruction index that we are working on.
		//

		int                                 IRInstruction;

		//
		// Define the IR control flow that we are working on.
		//

		const NWScriptControlFlow         * IRFlow;

		//
		// Define the MSIL subroutine that we are emitting.
		//

		MethodBuilder                    ^ MSILSub;

		//
		// Define the control flags (GSF_*) for this subroutine.
		//

		unsigned long                       Flags;

		//
		// Define the current ILGenerator for the active subroutine.
		//

		ILGenerator                       ^ ILGen;

		//
		// Define the control flow being emitted presently.
		//

		SubroutineControlFlow             ^ CurrentFlow;

		//
		// Define the control flow map for this subroutine.
		//

		ControlFlowMap                    ^ Flows;

		//
		// Define the stack of control flows to emit code for.
		//

		ControlFlowStack                  ^ FlowsToEmit;

		//
		// Define the variable table for variables that are created across
		// multiple control flows.
		//

		LocalVariableTable                  MultipleCreatedVarTable;
	};

	//
	// Define the context state for the IL generation phase.
	//

	ref struct ILGenContext
	{
		//
		// Define the analyzer (IR) for the program.
		//

		const NWScriptAnalyzer * Analyzer;

		//
		// Define extended code generation parameters (optional).
		//

		PCNWSCRIPT_JIT_PARAMS    CodeGenParams;

		//
		// Define the ObjectInvalid literal.
		//

		NWN::OBJECTID            ObjectInvalid;

		//
		// Define the maximum loop iterations (backwards jumps) that are
		// permitted.
		//

		int                      MaxLoopIterations;

		//
		// Define the maximum call depth in the call stack that is permitted,
		// in terms of script functions.
		//

		int                      MaxCallDepth;

		//
		// Define the overarching NWNScriptJITIntrinsics module.
		//

		Module                 ^ JITModule;

		//
		// Define the module being created (for the JIT code).
		//

		ModuleBuilder          ^ ProgramModule;

		//
		// Define the namespace to use for generated types.
		//

		String                 ^ Namespace;

		//
		// Define the program type itself.
		//

		TypeBuilder            ^ ProgramType;

		//
		// Define the types for engine structures.
		//

		array< Type ^ >        ^ EngineStructureTypes;

		//
		// Define fields on the main program type.
		//

		FieldBuilder           ^ FldCurrentActionObjectSelf;  // m_CurrentActionObjectSelf
		FieldBuilder           ^ FldProgram;                  // m_Program
		FieldBuilder           ^ FldProgramInterface;         // m_ProgramInterface;
		FieldBuilder           ^ FldCallDepth;                // m_CallDepth  
		FieldBuilder           ^ FldLoopCounter;              // m_LoopCounter

		//
		// Define the global variable table (set of member fields on the main
		// program type).
		//

		GlobalVariableTable    ^ Globals;

		//
		// Define methods on the main program type.
		//

		//
		// Define the emitted subroutine for #globals (if it exists).
		//

		MethodBuilder          ^ MthGlobals;

		//
		// Define the emitted subroutine for the entry point.
		//

		MethodBuilder          ^ MthNWScriptEntryPoint;

		//
		// Define the emitted subroutine to clone the subroutine globals state.
		//

		MethodInfo             ^ MthCloneScriptProgram;

		//
		// Define the emitted subroutine to load global variables from a boxed
		// array.
		//

		MethodInfo             ^ MthLoadScriptGlobals;

		//
		// Define the mapping of NWScript program counter values to methods.
		//

		PCMethodMap            ^ MethodMap;

		//
		// Define methods on the NWScriptProgram type.  (i.e. intrinsics that
		// are supplied by the generator and are not customized for the given
		// program)
		//

		MethodInfo             ^ MthIntrinsic_VMStackPushInt;
		MethodInfo             ^ MthIntrinsic_VMStackPopInt;
		MethodInfo             ^ MthIntrinsic_VMStackPushFloat;
		MethodInfo             ^ MthIntrinsic_VMStackPopFloat;
		MethodInfo             ^ MthIntrinsic_VMStackPushString;
		MethodInfo             ^ MthIntrinsic_VMStackPopString;
		MethodInfo             ^ MthIntrinsic_VMStackPushObjectId;
		MethodInfo             ^ MthIntrinsic_VMStackPopObjectId;
		array< MethodInfo ^ >  ^ MthIntrinsic_VMStackPushEngineStructure;
		array< MethodInfo ^ >  ^ MthIntrinsic_VMStackPopEngineStructure;
		array< MethodInfo ^ >  ^ MthIntrinsic_CompareEngineStructure;
		array< MethodInfo ^ >  ^ MthIntrinsic_CreateEngineStructure;
		MethodInfo             ^ MthIntrinsic_ExecuteActionService;
		MethodInfo             ^ MthIntrinsic_StoreState;
		MethodInfo             ^ MthIntrinsic_ExecuteActionServiceFast;
		MethodInfo             ^ MthIntrinsic_AllocateNeutralString;
		MethodInfo             ^ MthIntrinsic_DeleteNeutralString;
		MethodInfo             ^ MthIntrinsic_NeutralStringToString;
		MethodInfo             ^ MthIntrinsic_CheckScriptAbort;

		//
		// Define methods on the NWScriptEngineStructure* family of types.
		//

		array< MethodInfo ^ >  ^ MthEngineStructure_DeleteEngineStructure;

		//
		// Define the generation context for the subroutine that is currently
		// under construction.
		//

		SubroutineGenContext   ^ Sub;

		//
		// Define the code generation queue.
		//

		SubroutineGenQueue       CodeGenQueue;

		//
		// Define the NWScript address of the entry point symbol.
		//

		PROGRAM_COUNTER          EntryPC;

		//
		// Define the next resume method id for a script situation.
		//

		UInt32                   NextResumeMethodId;

		//
		// Define the list of resume methods.
		//

		SubroutineAttrList       ResumeMethods;

#if NWSCRIPT_DIRECT_FAST_ACTION_CALLS

		//
		// Define the helper routines to invoke action service handlers.
		//

		ActionMethodInfoArrArr ^ MthActionServiceStubs;

		//
		// Define the direct action service call routine, fetched from the
		// INWScriptActions interface at code generation time.
		//

		void                   * PtrOnExecuteActionFromJITFast;

#endif

		//
		// Define other miscellaneous state.
		//

		MethodInfo             ^ MthString_Equals; // System::String::Equals
		MethodInfo             ^ MthString_Concat; // System::String::Concat
		MethodInfo             ^ MthMethodBase_GetMethodFromHandle; // System::Reflection::MethodBase::GetMethodFromHandle
		ConstructorInfo        ^ CtorException;    // System::Exception::Exception(String)
		FieldInfo              ^ FldVector3_x;     // NWScript::Vector3::x
		FieldInfo              ^ FldVector3_y;     // NWScript::Vector3::y
		FieldInfo              ^ FldVector3_z;     // NWScript::Vector3::z

		//
		// Define the default access attributes for action service stub routines.
		//

		MethodAttributes         ActionServiceStubAttributes;

		//
		// Define the encoding to use with text strings.
		//

		Encoding               ^ StringEncoding;
	};

	//
	// Define the optional text out interface, for debug prints.
	//

	IDebugTextOut            * m_TextOut;

	//
	// Define the debug level for debug output.  Legal values are drawn from
	// the NWScriptVM::ExecDebugLevel family of enumerations.
	//

	ULONG                      m_DebugLevel;

	//
	// Define the host interface type, which is called through to communicate
	// with the script host (from the generated program).
	//

	Type                     ^ m_HostInterfaceType;

	//
	// Define the script interface type, which is called through to communicate
	// with the generated script program (from the script host).
	//

	Type                     ^ m_ScriptInterfaceType;

	//
	// Define the action implementation interface.
	//

	INWScriptActions         * m_ActionHandler;

	//
	// Define the action definition table.
	//

	PCNWACTION_DEFINITION      m_ActionDefs;
	NWSCRIPT_ACTION            m_ActionCount;

	//
	// Define the invalid object id (for use in parameter conversions).
	//

	NWN::OBJECTID              m_InvalidObjId;

	//
	// Define the IL generation context for the currently-being-built program.
	//
	// The generation context is only kept valid while the program is being
	// constructed.  It is discarded once construction is completed.
	//

	ILGenContext             ^ m_ILGenCtx;

	//
	// Define the types for engine structures.
	//

	array< Type ^ >          ^ m_EngineStructureTypes;


};

}

#endif
