/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptAnalyzer.cpp

Abstract:

	This module houses the NWScriptAnalyzer object.  The NWScript analyzer
	staticially analyzes a script in order to raise it to a higher level
	intermediate representation (IR).  The higher order IR allows the script
	program to be compiled into a different form (such as native code).

--*/

#include "Precomp.h"
#include "NWScriptVM.h"
#include "NWScriptStack.h"
#include "NWScriptInternal.h"
#include "NWScriptInterfaces.h"
#include "NWScriptAnalyzer.h"


//
// Define to 1 to enable verbose analysis debug output.
//

#define ANALYZE_DEBUG 0


#if ANALYZE_DEBUG

void
DebugPrintOutStdErr(
	__in __format_string const char * Fmt,
	...
	)
/*++

Routine Description:

	This routine prints output to stderr.

Arguments:

	Fmt - Supplies the output format string.

	... -  Supplies output inserts.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	va_list Ap;

	va_start( Ap, Fmt );
	vfprintf( stderr, Fmt, Ap );
	va_end( Ap );
}

// #define AnalyzeDebug if (m_TextOut != NULL) m_TextOut->WriteText
#define AnalyzeDebug DebugPrintOutStdErr
#else
#define AnalyzeDebug __noop
#endif

#define PrintIROut if (m_TextOut != NULL) m_TextOut->WriteText



NWScriptAnalyzer::NWScriptAnalyzer(
	__in IDebugTextOut * TextOut,
	__in_ecount( ActionCount ) PCNWACTION_DEFINITION ActionDefs,
	__in NWSCRIPT_ACTION ActionCount
	)
/*++

Routine Description:

	This routine constructs a new NWScriptAnalyzer.

Arguments:

	TextOut - Supplies the text output system for debug prints.

	ActionDefs - Supplies the action table to use when analyzing the script.

	ActionCount - Supplies the count of entries in the action table.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
: m_TextOut( TextOut ),
  m_ActionDefs( ActionDefs ),
  m_ActionCount( ActionCount ),
  m_ProgramName( "" ),
  m_LoaderPC( INVALID_PC ),
  m_GlobalsPC( INVALID_PC ),
  m_EntryPC( INVALID_PC ),
  m_EntryReturnType( ACTIONTYPE_VOID )
{
}

NWScriptAnalyzer::~NWScriptAnalyzer(
	)
/*++

Routine Description:

	This routine deletes the current NWScriptAnalyzer object and its associated
	members.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	// Free any strings allocated as variable values
	for (VariableValueMap::iterator It = m_ConstantValueMap.begin( );
		It != m_ConstantValueMap.end( ); It++)
	{
		if (It->second.Type == ACTIONTYPE_STRING && It->second.StringPtr)
			delete It->second.StringPtr;
	}
}

bool
NWScriptAnalyzer::IsPlatformNativeScript(
	__in NWScriptReader * Script,
	__in const char * PlatformSignature,
	__out NWNScriptLib::PROGRAM_COUNTER & PlatformBinaryOffset,
	__out size_t & PlatformBinarySize
	)
/*++

Routine Description:

	This routine determines whether a script program is really a platform
	native script (e.g. a managed script).

	N.B.  The binary size is not validated on successful return other than that
	      it must be a nonzero quantity.

Arguments:

	Script - Supplies a pointer to the script to analyze.

	PlatformSignature - Supplies the signature local to this platform.

	PlatformBinaryOffset - Receives the PC offset of the binary data, if the
	                       routine returns true.

	PlatformBinarySize - Receives the size of the binary data, if the routine
	                     returns true.

Return Value:

	The routine returns a Boolean value indicating true if the script was a
	platform native script, else false if it was a regular script.

	On failure due to a malformed script or other exceptional conditions, an
	std::exception is raised.

Environment:

	User mode.

--*/
{
	UCHAR           Opcode;
	UCHAR           TypeOpcode;
	ULONG           Len;
	ULONG           Offset;
	PROGRAM_COUNTER PC;

	PlatformBinaryOffset = 0;
	PlatformBinarySize   = 0;
	PC                   = 0;

	Script->SetInstructionPointer( 0 );

	//
	// A platform native script consists of the following:
	//
	// JSR <main> +8
	// main: RETN
	// RETN
	// CONSTS "NWScript Platform Native Script v1.0"
	// CONSTS <PlatformSignature> (e.g. "NWScript Managed Script v1.0")
	// CONSTI <Total size of BinaryData>
	// CONSTS <BinaryData chunk 1>
	// CONSTS <BinaryData chunk N>
	//

	Len = Disassemble( Script, Opcode, TypeOpcode, Offset );

	if (Opcode != OP_JSR)
		return false;

	if (Script->ReadINT32( ) != 8)
		return false;

	PC += Len;

	Len = Disassemble( Script, Opcode, TypeOpcode, Offset );

	if (Opcode != OP_RETN || TypeOpcode != TYPE_UNARY_NONE)
		return false;

	Script->AdvanceInstructionPointer( Len - Offset );
	PC += Len;

	Len = Disassemble( Script, Opcode, TypeOpcode, Offset );

	if (Opcode != OP_RETN || TypeOpcode != TYPE_UNARY_NONE)
		return false;

	Script->AdvanceInstructionPointer( Len - Offset );
	PC += Len;

	Len = Disassemble( Script, Opcode, TypeOpcode, Offset );

	if (Opcode != OP_CONST || TypeOpcode != TYPE_UNARY_STRING)
		return false;

	if (Script->ReadString( Len - 4 ) != "NWScript Platform Native Script v1.0")
		return false;

	PC += Len;

	Len = Disassemble( Script, Opcode, TypeOpcode, Offset );

	if (Opcode != OP_CONST || TypeOpcode != TYPE_UNARY_STRING)
		return false;

	if (Script->ReadString( Len - 4 ) != PlatformSignature)
		return false;

	PC += Len;

	Len = Disassemble( Script, Opcode, TypeOpcode, Offset );

	if (Opcode != OP_CONST || TypeOpcode != TYPE_UNARY_INT)
		return false;

	PlatformBinarySize = (size_t) Script->ReadINT32( );
	PC += Len;

	if (PlatformBinarySize == 0)
		return false;

	PlatformBinaryOffset = PC;

	return true;
}

void
NWScriptAnalyzer::ReadPlatformNativeScript(
	__in NWScriptReader * Script,
	__in NWNScriptLib::PROGRAM_COUNTER PlatformBinaryOffset,
	__in_bcount( PlatformBinarySize ) void * PlatformBinary,
	__in size_t PlatformBinarySize
	)
/*++

Routine Description:

	This routine decodes a platform native binary from a platform native script
	and returns the platform binary contents.

Arguments:

	Script - Supplies a pointer to the script to analyze.

	PlatformBinaryOffset - Supplies the PC offset of the binary data.

	PlatformBinary - Supplies the buffer that receives the contents of the
	                 platform binary.

	PlatformBinarySize - Supplies the size, in bytes, of the PlatformBinary
	                     buffer.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	UCHAR             Opcode;
	UCHAR             TypeOpcode;
	ULONG             Len;
	ULONG             Offset;
	unsigned char   * PlatformBinaryData;
	std::string       Chunk;

	Script->SetInstructionPointer( PlatformBinaryOffset );

	PlatformBinaryData = reinterpret_cast< unsigned char * >( PlatformBinary );

	//
	// Loop reading CONSTS instructions containing chunks of the platform
	// binary.
	//

	while (PlatformBinarySize != 0)
	{
		Len = Disassemble( Script, Opcode, TypeOpcode, Offset );

		if (Opcode != OP_CONST || TypeOpcode != TYPE_UNARY_STRING)
			throw std::runtime_error( "invalid opcode for platform native binary" );

		Chunk = Script->ReadString( Len - 4 );

		if (Chunk.empty( ))
			throw std::runtime_error( "invalid empty chunk for platform native binary" );

		if (Chunk.size( ) > PlatformBinarySize)
			throw std::runtime_error( "invalid chunk length for platform native binary" );

		memcpy( PlatformBinaryData, Chunk.data( ), Chunk.size( ) );
		PlatformBinaryData += Chunk.size( );
		PlatformBinarySize -= Chunk.size( );
	}
}

void
NWScriptAnalyzer::Analyze(
	__in NWScriptReader * Script,
	__in unsigned long Flags /* = 0 */
	)
/*++

Routine Description:

	This routine analyzes a script program, creating the high level
	intermediate representation (IR).  The IR can be used to compile the
	script into a different form (such as native code).

Arguments:

	Script - Supplies a pointer to the script to analyze.

	Flags - Supplies flags that control the program analysis.  Legal values are
	        drawn from the ANALYZE_FLAGS enumeration:

	        AF_STRUCTURE_ONLY - Only the program structure is analyzed.

	        AF_NO_OPTIMIZATIONS - Skip the optimization pass.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	AnalysisQueueEntry   Entry;
	bool                 Optimize;
	Subroutine         * Entrypoint;

	CreateValidOpcodeTypeMap( );

	m_ProgramName = Script->GetScriptName( );

	//
	// First, analyze #loader and #globals, as they are not typical subroutines
	// but follow special rules.
	//

	m_EntryPC = AnalyzeLoader( Script );

	//
	// Create the first subroutine entry for the entry point symbol.
	//

	if (m_EntryReturnType != ACTIONTYPE_VOID)
	{
		m_Subroutines.push_back( new Subroutine( 
			m_EntryPC, m_EntryReturnType, ParameterList( ), false ) );
	}
	else
	{
		m_Subroutines.push_back( new Subroutine( m_EntryPC, false ) );
	}

	Entrypoint = m_Subroutines.back( ).get( );

	if (!Script->GetSymbolName(
		(ULONG) Entrypoint->GetAddress( ),
		Entrypoint->GetSymbolName( ) ))
	{
		if (m_EntryReturnType != ACTIONTYPE_VOID)
			Entrypoint->SetSymbolName( "StartingConditional" );
		else
			Entrypoint->SetSymbolName( "main" );
	}

	//
	// Temporarily mark the entry point subroutine as fully analyzed and keep
	// it with an empty parameter list while we analyze #globals.  Although the
	// entry point subroutine may actually have parameters, #globals invokes it
	// as though it had none, and we must analyze under this assumption.
	//

	if (m_GlobalsPC != INVALID_PC)
	{
		Subroutine * Globals;

		Entrypoint = m_Subroutines.front( ).get( );

		m_Subroutines.push_back( new Subroutine( 
			m_GlobalsPC, m_EntryReturnType, ParameterList( ), false ));

		Globals   = m_Subroutines.back( ).get( );

		if (!Script->GetSymbolName(
			(ULONG) Entrypoint->GetAddress( ),
			Entrypoint->GetSymbolName( ) ))
		{
			Globals->SetSymbolName( "#globals" );
		}

		Entrypoint->SetIsAnalyzed( true );

		if (m_EntryReturnType != ACTIONTYPE_VOID)
			Entrypoint->SetReturnSize( CELL_SIZE );

		//
		// Analyze the subroutine tree of #globals.
		//

		Entry.PC         = m_GlobalsPC;
		Entry.SP         = 0;
		Entry.Flow       = NULL;
		Entry.Function   = Globals;
		Entry.BlockedOn  = NULL;
		Entry.LabelFlags = 0;

		AnalyzeSubroutineStructure( Entry, Script );

		Entrypoint->SetIsAnalyzed( false );

		if (m_EntryReturnType != ACTIONTYPE_VOID)
			Entrypoint->SetReturnSize( 0 );

		AnalyzeDebug( "Structural analysis for #globals completed.\n" );
	}

	//
	// Now analyze the subroutine tree of the main program.
	//

	Entry.PC         = m_EntryPC;
	Entry.SP         = 0;
	Entry.Flow       = NULL;
	Entry.Function   = m_Subroutines.front( ).get( );
	Entry.BlockedOn  = NULL;
	Entry.LabelFlags = 0;

	AnalyzeSubroutineStructure( Entry, Script );

	//
	// Finalize the return sizes.  Up until this point, the
	// 'return size' has been simply accounting for the maximum
	// stack write below SP.
	//
	// If the maximum stack write was less than the parameter
	// size of the routine, then we were simply assigning to
	// the parameters themselves.
	//
	// Otherwise, any space written to beyond the parameter
	// size region is return value space.
	//

	for (SubroutinePtrVec::iterator SubIt = m_Subroutines.begin( );
		SubIt != m_Subroutines.end( ); SubIt++)
	{
		Subroutine * Sub = SubIt->get( );

		AnalyzeDebug(
			"Inspecting subroutine %p (ReturnSize %d ParameterSize %d ReturnTypes %d Parameters %d)...\n",
			Sub,
			(int) Sub->GetReturnSize( ),
			(int) Sub->GetParameterSize( ),
			(int) Sub->GetReturnTypes( ).size( ),
			(int) Sub->GetParameters( ).size( ));

		if (Sub->GetReturnSize( ) <= Sub->GetParameterSize( ))
			Sub->SetReturnSize( 0 );
		else
			Sub->SetReturnSize( Sub->GetReturnSize( ) - 
				Sub->GetParameterSize( ) );

		while (Sub->GetReturnTypes( ).size( ) <
			(size_t)Sub->GetReturnSize( ) / CELL_SIZE)
			Sub->GetReturnTypes( ).push_back( 
				ACTIONTYPE_VOID );

		while (Sub->GetParameters( ).size( ) <
			(size_t)Sub->GetParameterSize( ) / CELL_SIZE)
			Sub->GetParameters( ).push_back( 
				ACTIONTYPE_VOID );

		Sub->CreateParameterReturnVariables( );
	}

#if ANALYZE_DEBUG
	// Print out all functions, flows, and labels in order
	for (SubroutinePtrVec::iterator SubIt = m_Subroutines.begin( );
		SubIt != m_Subroutines.end( ); SubIt++)
	{
		Subroutine * Sub = SubIt->get( );

		AnalyzeDebug(
			"Found function at address %X%s. %d bytes parameters, %d bytes return value\n",
			Sub->GetAddress( ),
			(Sub->GetFlags( ) & Subroutine::SCRIPT_SITUATION) ? " (script situation)" : "",
			Sub->GetParameterSize( ),
			Sub->GetReturnSize( ) );

		if (!Sub->GetIsAnalyzed( ))
			AnalyzeDebug( "Error: Function not fully analyzed!" );

		for (ControlFlowSet::iterator FlowIt = Sub->GetControlFlows( ).begin( );
			FlowIt != Sub->GetControlFlows( ).end( ); FlowIt++)
		{
			ControlFlowPtr Flow = FlowIt->second;
			const char *TerminationType = "Unknown";

			switch (Flow->GetTerminationType( ))
			{
			case ControlFlow::Terminate:
				TerminationType = "Terminate";
				break;

			case ControlFlow::Merge:
				TerminationType = "Merge";
				break;

			case ControlFlow::Transfer:
				TerminationType = "Transfer";
				break;

			case ControlFlow::Split:
				TerminationType = "Split";
				break;
			}

			AnalyzeDebug(
				"Found control flow from %X/%X to %X/%X, %s, targets at %X and %X\n",
				Flow->GetStartPC( ),
				Flow->GetStartSP( ),
				Flow->GetEndPC( ),
				Flow->GetEndSP( ),
				TerminationType,
				Flow->GetChild( 0 ).get( ) ? Flow->GetChild( 0 )->GetStartPC( ) : 0,
				Flow->GetChild( 1 ).get( ) ? Flow->GetChild( 1 )->GetStartPC( ) : 0);

			// Verify every child link has a corresponding parent link
			for (size_t ChildIdx = 0; ChildIdx < 2; ChildIdx++)
			{
				ControlFlowPtr Child = Flow->GetChild( ChildIdx );
				if (!Child.get( ))
					continue;
				
				if (Child->GetParents( ).count( Flow.get( ) ) == 0)
					AnalyzeDebug(
						"Error: Flow %X child %d %X does not link back to parent!\n",
						Flow->GetStartPC( ),
						ChildIdx,
						Child->GetStartPC( ) );
				else if (Flow->GetEndSP( ) != Child->GetStartSP( ))
					AnalyzeDebug(
						"Error: Flow %X child %d stack is inconsistent: %X vs %X!\n",
						Flow->GetStartPC( ),
						ChildIdx,
						Flow->GetEndSP( ),
						Child->GetStartSP( ) );
			}

			// Verify each parent link has a corresponding child link
			for (ControlFlowWeakPtrSet::iterator ParentIt = 
				Flow->GetParents( ).begin( );
				ParentIt != Flow->GetParents( ).end( ); ParentIt++)
			{
				if ((*ParentIt)->GetChild( 0 ) != Flow.get( ) &&
					(*ParentIt)->GetChild( 1 ) != Flow.get( ))
					AnalyzeDebug(
						"Error: Flow %X parent %X does not link back to child!\n",
						Flow->GetStartPC( ),
						(*ParentIt)->GetStartPC( ) );
			}
		}

		for (LabelVec::iterator LabelIt = Sub->GetBranchTargets( ).begin( );
			LabelIt != Sub->GetBranchTargets( ).end( ); LabelIt++)
		{
			ControlFlowPtr Flow = LabelIt->GetControlFlow( );

			AnalyzeDebug(
				"Found label %X/%X with flags %X and flow @%X\n",
				LabelIt->GetAddress( ),
				LabelIt->GetSP( ),
				LabelIt->GetFlags( ),
				Flow.get( ) ? Flow->GetStartPC( ) : INVALID_PC);
		}
	}
#endif

	//
	// If we were to only analyze the basic program structure, halt here.
	//

	if (Flags & AF_STRUCTURE_ONLY)
		return;

	//
	// Before we start doing the actual analysis, we need to generate some 
	// global data necessary for the analysis - specifically, the stack maps 
	// of the parameters and return types of action handlers.
	//

	m_ActionParameters.assign( m_ActionCount, ParameterList( ) );

	for (NWSCRIPT_ACTION ActionIdx = 0; 
		ActionIdx < m_ActionCount; ActionIdx++)
	{
		PCNWACTION_DEFINITION Action = m_ActionDefs + ActionIdx;
		ParameterList & Parameters = m_ActionParameters[ ActionIdx ];

		for (unsigned long Param = 0; 
			Param < Action->NumParameters; Param++)
		{
			NWACTION_TYPE Type = Action->ParameterTypes[ Param ];

			if (Type == ACTIONTYPE_VECTOR)
			{
				for (size_t i = 0; i < 3; i++)
					Parameters.push_back( ACTIONTYPE_FLOAT );
			}
			else if (Type != ACTIONTYPE_ACTION)
				Parameters.push_back( Type );
			// Else: ACTIONTYPE_ACTION is 0 size
		}
	}

	AnalyzeSubroutineCode( Script );

	Optimize = !(Flags & AF_NO_OPTIMIZATIONS);

#if ANALYZE_DEBUG
	if (Optimize)
	{
		// Mark it up and print it out
		PostProcessIR( false );

		AnalyzeDebug( "\n\n" );
		PrintIR( );
	}
#endif

	// Now optimize it and print out the optimized code
	PostProcessIR( Optimize );

#if ANALYZE_DEBUG
	AnalyzeDebug( "\n\n" );
	AnalyzeDebug( "Printing optimized IR:\n\n" );
	PrintIR( );
#endif
}




ULONG
NWScriptAnalyzer::Disassemble(
	__in NWScriptReader * Script,
	__out UCHAR & Opcode,
	__out UCHAR & TypeOpcode,
	__out ULONG & PCOffset
	)
/*++

Routine Description:

	This routine determines the length of the instruction at the current PC,
	including any parameters to the instruction.

Arguments:

	Script - Supplies the script to decode the instruction for.  The current
	         program counter of the script is used.

	Opcode - Receives the instruction opcode.

	TypeOpcode - Receives the instruction type opcode.

	PCOffset - Receives the offset of the new PC relative to the start of the
	           instruction when the routine returns.  Generally, the routine
	           returns 2 bytes into the instruction, but not always.

Return Value:

	The routine returns the length of the instruction (including the opcode
	bytes).  On failure, an std::exception is returned.

Environment:

	User mode.

--*/
{
	return NWScriptVM::Disassemble( Script, Opcode, TypeOpcode, PCOffset );
}

void
NWScriptAnalyzer::GetInstructionNames(
	__in UCHAR Opcode,
	__in UCHAR TypeOpcode,
	__deref __out const char * * OpcodeName,
	__deref __out const char * * TypeOpcodeName
	)
/*++

Routine Description:

	This routine determines the names of the two opcode components of a script
	instruction.

Arguments:

	Opcode - Supplies the instruction opcode.

	TypeOpcode - Supplies the instruction type opcode.

	OpcodeName - Receives a pointer to the textural name of the opcode.

	TypeOpcodeName - Receives a pointer to the textural name of the type
	                 opcode.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	return NWScriptVM::GetInstructionNames(
		Opcode,
		TypeOpcode,
		OpcodeName,
		TypeOpcodeName);
}

NWNScriptLib::PROGRAM_COUNTER
NWScriptAnalyzer::AnalyzeLoader(
	__in NWScriptReader * Script
	)
/*++

Routine Description:

	This routine analyzes #loader and #globals.

Arguments:

	Script - Supplies the script to analyze.

Return Value:

	The program counter offset of the script entry point is returned.

	On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	UCHAR           Opcode;
	UCHAR           TypeOpcode;
	ULONG           Len;
	ULONG           Offset;
	PROGRAM_COUNTER PC;
	PROGRAM_COUNTER SAVEBPPC;

	//
	// #loader always begins at PC=0.
	//

	PC         = 0;
	m_LoaderPC = PC;

	AnalyzeDebug( "#loader at PC=%08X\n", m_LoaderPC );

	//
	// First, identify #loader and #globals.
	//

	Script->SetInstructionPointer( 0 );

	Len = Disassemble( Script, Opcode, TypeOpcode, Offset );

	switch (Opcode)
	{

	case OP_RSADD:
		if (TypeOpcode != TYPE_UNARY_INT)
			throw std::runtime_error( "#loader returns non-int/non-void type" );

		m_EntryReturnType = ACTIONTYPE_INT;
		break;

	case OP_JSR:
		m_EntryReturnType = ACTIONTYPE_VOID;
		break;

	case OP_NOP:
		if (Script->GetPatchState( ) == NWScriptReader::NCSPatchState_PatchReturnValue)
		{
			m_EntryReturnType = ACTIONTYPE_INT;
			break;
		}

	default:
		throw std::runtime_error( "unrecognized instruction pattern for #loader" );

	}

	//
	// Now, discover the control transfer out of #loader.  This will be either
	// to #globals (if the routine uses SAVEBP/RESTOREBP), else the actual
	// entry point itself.
	//
	// N.B.  We may already be at OP_JSR for a trivial #loader (i.e. if the
	//       entry symbol returns void).
	//
	// N.B.  The permissible instruction set for #loader is highly restricted.
	//

	while (Opcode != OP_JSR)
	{
		if (Script->ScriptIsEof( ))
			throw std::runtime_error( "reached eof while searching #loader control transfer" );

		Script->AdvanceInstructionPointer( Len - Offset );
		PC += Len;

		Len = Disassemble( Script, Opcode, TypeOpcode, Offset );

		if (Len < Offset)
			break;

		if (Opcode == OP_RETN)
			throw std::runtime_error( "reached RETN while searching #loader control transfer" );

		AnalyzeDebug(
			"Other instruction at PC=%08X while searching for JSR <next>\n",
			PC);
	}

	AnalyzeDebug( "Found JSR <next> at PC=%08X\n", PC );

	//
	// Now check for SAVEBP usage, which would indicate that we are in #globals
	// and not the entry point.
	//

	PC += (PROGRAM_COUNTER) Script->ReadINT32( );

	Script->SetInstructionPointer( PC );

	AnalyzeDebug( "Searching for SAVEBP starting at PC=%08X...\n", PC );

	if ((SAVEBPPC = FindInstructionInFlow( PC, Script, OP_SAVEBP )) == INVALID_PC)
	{
		//
		// No SAVEBP, this must be the real entry point we're transferring
		// control to.  Return a pointer to the first instruction.
		//

		m_GlobalsPC = INVALID_PC;

		AnalyzeDebug( "No #globals present.\n" );
	}
	else
	{
		//
		// SAVEBP was used, so this must be #globals.  Find the next subroutine
		// control transfer instruction, which must be to the real entry point.
		//
		// N.B.  #globals may have subroutine calls before the entry point, so
		//       we will need to find the first subroutine after SAVEBP.
		//

		Script->SetInstructionPointer( SAVEBPPC );

		m_GlobalsPC = PC;
		PC          = FindInstructionInFlow( SAVEBPPC, Script, OP_JSR );

		AnalyzeDebug( "#globals at PC=%08X\n", m_GlobalsPC );

		if (PC == INVALID_PC)
			throw std::runtime_error( "failed to discover JSR to entry point symbol" );

		//
		// Analyze the JSR and return the subroutine call target as the entry
		// point subroutine's first instruction.
		//

		Script->SetInstructionPointer( PC );

		Disassemble( Script, Opcode, TypeOpcode, Offset );

		PC += (PROGRAM_COUNTER) Script->ReadINT32( );
	}

	AnalyzeDebug( "Entry point symbol at PC=%08X\n", PC );

	return PC;
}

void
NWScriptAnalyzer::AnalyzeSubroutineStructure(
	__in AnalysisQueueEntry Entry,
	__in NWScriptReader * Script
	)
/*++

Routine Description:

	This routine analyzes the basic structure of all subroutines in the script
	program.  Analysis begins at the entry point symbol.  Once an unknown
	subroutine is discovered, analysis switches to the subroutine from the
	current subroutine (resuming once the depth traversal completes).

Arguments:

	Entry - Supplies the analysis parameters of the portion of the subroutine
	        that is to be analyzed.

	Script - Supplies the script to analyze.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	unsigned long        Scanned;
	UCHAR                Opcode;
	UCHAR                TypeOpcode;
	ULONG                Len;
	ULONG                Offset;
	PROGRAM_COUNTER      RelPC;
	PROGRAM_COUNTER      FlowPC;
	bool                 Continue;
	AnalysisQueueEntry   NewEntry;
	AnalysisQueueEntry * QueueEntry;
	Subroutine         * Sub;
	bool                 Found;
	bool                 FlowAssigned;
	ControlFlowPtr       SubseqFlow;
#if ANALYZE_DEBUG
	const char         * OpcodeName;
	const char         * TypeOpcodeName;
#endif

	Scanned  = 0;

	for (;;)
	{
		Continue = true;

		Script->SetInstructionPointer( Entry.PC );

		//
		// If we have not yet created a control flow descriptor for this 
		// function, we need to prepare this function.
		//

		if (Entry.Flow == NULL)
		{
			AnalyzeDebug(
				"Analyzing function @ PC=%08X (SP=%08X)...\n",
				Entry.PC,
				Entry.SP);

			Entry.Flow = new ControlFlow( Entry.PC, Entry.SP );
			Entry.Function->GetControlFlows( ).insert(
				ControlFlowSet::value_type( Entry.PC, Entry.Flow )
				);

			SubseqFlow = NULL;

			Entry.BlockedOn = NULL;
		}

		while (Continue)
		{
			if (Script->ScriptIsEof( ))
				throw std::runtime_error( "reached eof in AnalyzeSubroutineStructure" );

			//
			// Handle the case where we've reached the subsequent flow. But 
			// check for stack consistency.
			//

			if (SubseqFlow != NULL && Entry.PC >= SubseqFlow->GetStartPC( ))
			{
				// Verify that the stacks are consistent
				if (SubseqFlow->GetStartSP( ) != Entry.SP)
				{
					ThrowError(
						"mismatched stack on control flow at PC=%08X (SP=%08X, FlowSP=%08X)",
						SubseqFlow->GetStartPC( ),
						Entry.SP,
						SubseqFlow->GetStartSP( ));
				}

				// Need to terminate the current flow and link it to the next one
				Entry.Flow->SetEndPC( Entry.PC );
				Entry.Flow->SetEndSP( Entry.SP );
				Entry.Flow->SetTerminationType( ControlFlow::Merge );
				Entry.Flow->SetChild( 0, SubseqFlow );
				Entry.Flow->SetChild( 1, NULL );
				SubseqFlow->GetParents( ).insert( Entry.Flow.get( ) );

				// Get ourselves a new queue entry to execute
				Continue = false;
				break;
			}

			if (!Continue)
				break;	// Need to get out here if there's no next flow

			//
			// Decode and scan the instruction.
			//

			Len = Disassemble( Script, Opcode, TypeOpcode, Offset );

			if (Len < Offset)
				break;

			Scanned += 1;

			//
			// Update the current flow's end PC. This is necessary each 
			// iteration because if the end PC of each flow is not 
			// maintained, if we get suspended for a function call another 
			// thread might end up getting a jump into the middle of an 
			// existing flow without realizing it, and create a new,
			// duplicate flow.
			//

			//Entry.Flow->SetEndPC( Entry.PC + Len );

			if (Scanned > NWScriptVM::ANALYSIS_MAX_SCRIPT_INSTRUCTIONS)
				throw std::runtime_error( "too many script instructions in AnalyzeSubroutineStructure" );

#if ANALYZE_DEBUG
			GetInstructionNames(
				Opcode,
				TypeOpcode,
				&OpcodeName,
				&TypeOpcodeName);

			AnalyzeDebug(
				"%08X: %02X.%02X   %s%s   SP=%08X\n",
				Entry.PC,
				Opcode,
				TypeOpcode,
				OpcodeName,
				TypeOpcodeName,
				Entry.SP);
#endif

			CheckOpcodeType( Entry.PC, (NWACTION_TYPE) Opcode, TypeOpcode );

			switch (Opcode)
			{

			case OP_RETN: // Return from subroutine
				//
				// First, mark the function entry as analyzed, and discover the
				// count of parameters.  Note that the current SP will be
				// negative if there were any parameters, as the virtual SP at
				// entry to every analyzed function is zero (so when parameters
				// are cleaned off the stack, the SP goes negative).
				//

				if (!(Entry.Function->GetFlags( ) & Subroutine::SCRIPT_SITUATION))
				{
					if (!Entry.Function->GetIsAnalyzed( ))
					{
						if (Entry.SP > 0)
						{
							ThrowError(
								"illegal virtual SP on return at PC=%08X (SP=%08X)",
								Entry.PC,
								Entry.SP);
						}

						Entry.Function->SetParameterSize( -Entry.SP );
					}
					else if (Entry.Function->GetParameterSize( ) != -Entry.SP)
					{
						ThrowError(
							"unbalanced virtual SP on return at PC=%08X (expected %lu, actual %lu)",
							Entry.PC,
							Entry.Function->GetParameterSize( ),
							-Entry.SP);
					}

					AnalyzeDebug(
						"Initial analysis completed for function @ PC=%08X (parameter size = %lu, raw return size = %lu).\n",
						Entry.Function->GetAddress( ),
						Entry.Function->GetParameterSize( ),
						Entry.Function->GetReturnSize( ));
				}
				else
				{
					//
					// Return fallthrough from a script situation label cannot
					// terminate subroutine analysis, because a script
					// situation label does not constitute a proper call/return
					// pair.  (The stack pointer is not properly adjusted.)
					//

					AnalyzeDebug(
						"Analysis completed for script situation label in function @ PC=%08X.\n",
						Entry.Function->GetAddress( ));
				}

				Entry.Function->SetIsAnalyzed( true );

				//
				// Now close out the control flow.  Note that no children are
				// assigned as this is the end of the line.
				//

				Entry.Flow->SetEndPC( Entry.PC + Len);
				Entry.Flow->SetEndSP( Entry.SP );
				Entry.Flow->SetTerminationType( ControlFlow::Terminate );

				//
				// In all cases, OP_RETN signifies a nonlinear control
				// transfer, so we'll skip the normal fall thorugh.
				//

				Continue = false;
				continue;

			case OP_JSR: // Jump to subroutine (call)
			case OP_STORE_STATEALL: // Save a script situation state
			case OP_STORE_STATE:    // Save a script situation state
			{
				//
				// N.B. Script situations are treatest as special functions.
				// The saved locals are converted to parameters, and the 
				// value of the globals at the time of the op is saved.
				// Unlike normal functions, script situation parameter count 
				// is known up front, and they are not required to fully 
				// clean the stack on return.
				//

				//
				// Decode the instruction
				//

				unsigned long Flags = 0;
				STACK_POINTER LocalSaveSize = 0;

				if (Opcode == OP_JSR)
					RelPC = (PROGRAM_COUNTER) Script->ReadINT32( );
				else
				{
					RelPC = TypeOpcode;
					Flags = Subroutine::SCRIPT_SITUATION;

					if (Opcode == OP_STORE_STATE)
					{
						Script->ReadINT32( );
						LocalSaveSize = Script->ReadINT32( );
					}
					else
						LocalSaveSize = Entry.SP;
				}

				if (RelPC == 0)
					throw std::runtime_error( "trivial infinite loop (JSR) detected" );

				FlowPC = Entry.PC + RelPC;

				//
				// If this was a completely unknown subroutine, create a
				// new descriptor for it.
				//

				Sub = GetSubroutine( FlowPC );
				if (Sub == NULL)
				{
					m_Subroutines.push_back( new Subroutine( FlowPC, Flags ) );

					Sub = m_Subroutines.back( ).get( );

					if (Opcode == OP_JSR)
					{
						// TODO: What to do here with script situations?
						Script->GetSymbolName(
							(ULONG) Sub->GetAddress( ),
							Sub->GetSymbolName( ) );
					}
					else
					{
						//
						// If this is a script situtation, we already know 
						// the exact number of return values ( 0 ) and 
						// parameters.
						//

						Sub->SetReturnSize( 0 );
						Sub->SetParameterSize( LocalSaveSize );
					}
				}

				if (!Sub->GetIsAnalyzed( ))
				{
					QueueEntry = GetSubroutineQueueEntry( FlowPC );

					//
					// Be careful here. If it's a recursive function call, 
					// there may not already be a queue entry for it. Make 
					// sure this doesn't cause us to make a false positive.
					bool IsNewEntry = 
						(QueueEntry == NULL && Sub != Entry.Function);

					if (IsNewEntry)
					{
						//
						// We have reached a function entry that we don't have a
						// queue entry for (i.e. a completely new function).  Queue
						// a new analysis entry for the new function with no
						// blocking entries.
						//

						NewEntry.PC         = FlowPC;
						NewEntry.SP         = 0;
						NewEntry.Flow       = NULL;
						NewEntry.BlockedOn  = NULL;
						NewEntry.Function   = Sub;
						NewEntry.LabelFlags = 0;

						m_AnalysisQueue.push_back( NewEntry );
					}

					//
					// If this is a script situation, there's no need for us 
					// to block, as there is no stack displacement.
					//

					if ((Sub->GetFlags( ) & Subroutine::SCRIPT_SITUATION))
					{
						Entry.PC += Len;

						continue;
					}

					//
					// At this point, we have reached a subroutine call to a
					// subroutine that we've not analyzed (at least to the point of
					// understanding the parameter size).  Thus, we must block on
					// analysis until the stack displacement (parameter size) is
					// known.
					//

					if (IsNewEntry)
					{
						AnalyzeDebug(
							"Analysis at PC=%08X blocking on subroutine analysis for new function %08X.\n",
							Entry.PC,
							FlowPC);
					}
					else
					{
						AnalyzeDebug(
							"Analysis at PC=%08X blocking on subroutine analysis for partially analyzed function %08X (%08X).\n",
							Entry.PC,
							FlowPC,
							Sub);
					}

					//
					// Save the flow end PC and SP. We have to set the SP to 
					// INVALID_SP becase we won't know what the SP will be 
					// until the call returns.
					//

					Entry.Flow->SetEndPC( Entry.PC + Len);
					Entry.Flow->SetEndSP( INVALID_SP );

					NewEntry.PC         = Entry.PC;
					NewEntry.SP         = Entry.SP;
					NewEntry.Flow       = Entry.Flow;
					NewEntry.BlockedOn  = Sub;
					NewEntry.Function   = Entry.Function;
					NewEntry.LabelFlags = Entry.LabelFlags;

					m_AnalysisQueue.push_back( NewEntry );

					Continue = false;
					continue;
				}

				if (Opcode == OP_JSR)
				{
					//
					// We have already analyzed the subroutine, adjust SP
					// based on the arguments that the subroutine will
					// remove from the stack.
					//

					AnalyzeDebug(
						"Function %08X call at PC=%08X has SP displacement %d.\n",
						FlowPC,
						Entry.PC,
						-Sub->GetParameterSize( ));

					Entry.SP -= Sub->GetParameterSize( );
				}

				Entry.PC += Len;

				//
				// We've consumed the opcode parameters, so don't fall
				// through to the unhandled case but rather dispatch
				// the next opcode.
				//

				continue;
			}

			case OP_JZ:  // Jump if zero
			case OP_JNZ: // Jump if not zero
			case OP_JMP: // Unconditional jump
			{
				RelPC = (PROGRAM_COUNTER) Script->ReadINT32( );

				if (RelPC == 0)
					throw std::runtime_error( "trivial infinite loop detected" );

				if (Opcode != OP_JMP)
					Entry.SP -= CELL_SIZE;

				//
				// First of all we need to close the current flow. If we don't
				// do this first, PrepareNewControlFlow will fail to detect
				// cases where we loop back into the current flow. Though
				// obviously we can't set the child pointers until we actually
				// know them.
				//
				Entry.Flow->SetEndPC( Entry.PC + Len );
				Entry.Flow->SetEndSP( Entry.SP );
				if (Opcode == OP_JMP)
					Entry.Flow->SetTerminationType( ControlFlow::Transfer );
				else
					Entry.Flow->SetTerminationType( ControlFlow::Split );

				//
				// If we have already traced this flow, then don't scan through
				// it again.
				//

				FlowPC = Entry.PC + RelPC;
				Found  = false;

				ControlFlowPtr FlowBranch = NULL, FlowFallthrough = NULL;
				Label LabelBranch(0, 0, NULL), LabelFallthrough(0, 0, NULL);
				bool NewBranchFlow = PrepareNewControlFlow( 
					Entry, FlowPC, FlowBranch, LabelBranch );
				bool NewFallthroughFlow = false;

				FlowAssigned = false;

				if (Opcode != OP_JMP)
				{
					NewFallthroughFlow = PrepareNewControlFlow( 
						Entry, Entry.PC + Len, FlowFallthrough, LabelFallthrough );
				}

				//
				// The first flow is for the branch target, and the second
				// flow is (optionally) for the fall-through.  Note that a
				// fall-through flow isn't created for an unconditional
				// jump.
				//

				Entry.Flow->SetChild( 0, FlowBranch );
				Entry.Flow->SetChild( 1, FlowFallthrough );

				//
				// Push it onto the list of flows to follow. We need to also 
				// add it to the list of branch targets, but only if it's not 
				// already been examined.
				//

				NewEntry.BlockedOn  = NULL;
				NewEntry.Function   = Entry.Function;
				NewEntry.LabelFlags = Entry.LabelFlags;

				if (NewBranchFlow)
				{
					NewEntry.PC         = FlowBranch->GetStartPC( );
					NewEntry.SP         = FlowBranch->GetStartSP( );
					NewEntry.Flow       = FlowBranch;

					m_AnalysisQueue.push_back( NewEntry );

					AnalyzeDebug(
						"Scheduling trace of alternate flow path at PC=%08X SP=%08X.\n",
						FlowPC,
						Entry.SP);
				}

				if (NewFallthroughFlow)
				{
					NewEntry.PC         = FlowFallthrough->GetStartPC( );
					NewEntry.SP         = FlowFallthrough->GetStartSP( );
					NewEntry.Flow       = FlowFallthrough;

					m_AnalysisQueue.push_back( NewEntry );

					AnalyzeDebug(
						"Scheduling trace of alternate flow path at PC=%08X SP=%08X.\n",
						Entry.PC + Len,
						Entry.SP);
				}

				//
				// Take the fall through as the next flow, unless we are
				// an unconditional branch.
				//

				/*if (Opcode == OP_JMP)
					Entry.Flow = FlowBranch;
				else
					Entry.Flow = FlowFallthrough;

				FlowAssigned = true;*/

				Continue = false;
				continue;
				/*}
				else if (!Found)
				{
					ControlFlow * FlowBranch;
					ControlFlow * FlowFallthrough;

					//
					// Now close out the existing flow and create new flows.
					//
					// The first flow is for the branch target, and the second
					// flow is (optionally) for the fall-through.  Note that a
					// fall-through flow isn't created for an unconditional
					// jump.
					//

					pair< ControlFlowSet::iterator, bool > pr = 
						Entry.Function->GetControlFlows( ).insert(
						ControlFlowSet::value_type( FlowPC, 
						ControlFlow( FlowPC, Entry.SP ) ) );

					FlowBranch = pr.first;

					if (Opcode != OP_JMP)
					{
						Entry.Function->GetControlFlows( ).push_back(
							ControlFlow( Entry.PC + Len, Entry.SP ));

						FlowFallthrough = &Entry.Function->GetControlFlows( ).back( );
					}
					else
					{
						FlowFallthrough = NULL;
					}

					Entry.Flow->SetEndPC( Entry.PC );
					Entry.Flow->SetEndSP( Entry.SP );
					Entry.Flow->SetChild( 0, FlowBranch );
					Entry.Flow->SetChild( 1, FlowFallthrough );

					//
					// Mark the flow as visited and push it onto the list of flows
					// to follow.
					//

					Label Branch( FlowPC, Entry.SP, FlowBranch );

					Entry.Function->GetBranchTargets( ).push_back( Branch );

					if (Opcode != OP_JMP)
						Entry.Function->GetAnalyzeBranches( ).push_back( Branch );

					AnalyzeDebug(
						"Scheduling trace of alternate flow path at PC=%08X SP=%08X.\n",
						FlowPC,
						Entry.SP);

					//
					// Take the fall through as the next flow, unless we are
					// an unconditional branch.
					//

					if (Opcode == OP_JMP)
						Entry.Flow = FlowBranch;
					else
						Entry.Flow = FlowFallthrough;

					FlowAssigned = true;
				}
				else
				{
					//
					// This is a conditional jump that we have recognized
					// before and we are taking the fall-through path.
					//

					FlowAssigned = false;
				}

				if (Opcode != OP_JMP)
					Entry.PC += Len;
				else
					Entry.PC = FlowPC;

				//
				// We might not have assigned a new flow already if we have
				// seen the branch target before.  In this case, we'll need to
				// scan through our registered list for a matching flow.
				//

				if (FlowAssigned == false)
				{
					ControlFlow * ExistingFlow;

					AnalyzeDebug(
						"Scanning for pre-existing control flow at StartPC=%08X.\n",
						Entry.PC );

					ExistingFlow = Entry.Function->GetControlFlow( Entry.PC );

					if (ExistingFlow == NULL)
					{
						ControlFlow * NewFlow;

						AnalyzeDebug(
							"First fallthrough for this already seen branch at PC=%08X.\n",
							Entry.PC );

						//
						// If we got no matching flow back, we have seen the
						// branch target before (say a common cleanup label),
						// but we have never fallen through before.  Create a
						// new fallthrough flow.
						//

						if ((Entry.Flow->GetChild( 0 ) != NULL) &&
						    (Entry.Flow->GetChild( 1 ) != NULL))
						{
							//
							// We should always have room for another flow.
							//

							ThrowError(
								"no space to link new fallthrough control flow for new PC=%08X",
								Entry.PC);
						}

						Entry.Function->GetControlFlows( ).push_back(
							ControlFlow( Entry.PC, Entry.SP ));

						NewFlow = &Entry.Function->GetControlFlows( ).back( );

						if (Entry.Flow->GetChild( 0 ) == NULL)
							Entry.Flow->SetChild( 0, NewFlow );
						else
							Entry.Flow->SetChild( 1, NewFlow );

						if (Entry.Flow->GetEndPC( ) != INVALID_PC)
						{
							ThrowError(
								"closing already closed flow at PC=%08X",
								Entry.PC);
						}

						//
						// Close out the existing flow and switch to the new
						// flow.
						//

						Entry.Flow->SetEndPC( Entry.PC );
						Entry.Flow->SetEndSP( Entry.SP );

						Entry.Flow = NewFlow;
					}
					else
					{
						//
						// Ensure that our state matches up.  The existing
						// state must have the PC/SP for us.  It might not
						// have the same linkages as we could be reaching this
						// PC from a different path.
						//

						//
						if ((Entry.Flow->GetChild( 0 ) != ExistingFlow) &&
						    (Entry.Flow->GetChild( 1 ) != ExistingFlow))
						{
							ThrowError(
								"flow state mismatch at PC=%08X",
								Entry.PC);
						}
						//

						if ((ExistingFlow->GetStartPC( ) != Entry.PC) ||
						    (ExistingFlow->GetStartSP( ) != Entry.SP))
						{
							ThrowError(
								"flow state mismatch (PC/SP) at PC=%08X, SP=%08X (FlowPC=%08X, FlowSP=%08X)",
								Entry.PC,
								Entry.SP,
								ExistingFlow->GetStartPC( ),
								ExistingFlow->GetStartSP( ));
						}

						Entry.Flow = ExistingFlow;
					}
				}

				Script->SetInstructionPointer( Entry.PC );*/

				continue;
			}

			case OP_CPDOWNSP:   // Copy down SP (assignment operator)
				{
					STACK_POINTER Offset;
					STACK_POINTER Size;

					Offset = (STACK_POINTER) Script->ReadINT32( );
					Size   = (STACK_POINTER) Script->ReadINT16( );

					if ((Offset & CELL_UNALIGNED) ||
					    (Size & CELL_UNALIGNED))
					{
						throw std::runtime_error( "unaligned CPDOWNSP access" );
					}

					Entry.Function->UpdateReturnSize( Entry.SP + Offset );

					Entry.PC += Len;

					Script->SetInstructionPointer( Entry.PC );

					continue;
				}
				break;

			case OP_RSADD:      // Reserve uninitialized space on the stack
				Entry.SP += CELL_SIZE;

				//
				// N.B.  Variable types are validated later.
				//

				break;

			case OP_CPTOPSP:    // Read / duplicate local variables
				{
					STACK_POINTER Offset;
					STACK_POINTER Size;

					Offset = (STACK_POINTER) Script->ReadINT32( );
					Size   = (STACK_POINTER) Script->ReadINT16( );

					if ((Offset & CELL_UNALIGNED) ||
					    (Size & CELL_UNALIGNED))
					{
						throw std::runtime_error( "unaligned CPTOPSP access" );
					}

					Entry.SP += Size;
					Entry.PC += Len;

					Script->SetInstructionPointer( Entry.PC );

					continue;
				}
				break;

			case OP_CONST:      // Push a constant onto the stack
				Entry.SP += CELL_SIZE;
				break;

			case OP_ACTION:     // Call an engine action API
				{
					PCNWACTION_DEFINITION Action;
					NWSCRIPT_ACTION       ActionId;
					unsigned long         ArgumentCount;

					ActionId      = (NWSCRIPT_ACTION) Script->ReadINT16( );
					ArgumentCount = (unsigned long) Script->ReadINT8( );

					if (ActionId >= m_ActionCount)
						throw std::runtime_error( "out of range action call" );

					Action = &m_ActionDefs[ ActionId ];

					if (ArgumentCount < Action->MinParameters)
						throw std::runtime_error( "too few parameters for action call" );
					else if (ArgumentCount > Action->NumParameters)
						throw std::runtime_error( "too many parameters for action call" );

					//
					// Adjust the virtual SP based on the action's argument and
					// return value.
					//

					for (unsigned long i = 0; i < ArgumentCount; i += 1)
						Entry.SP -= GetTypeSize( Action->ParameterTypes[ i ] );

					Entry.SP += GetTypeSize( Action->ReturnType );
					Entry.PC += Len;

					Script->SetInstructionPointer( Entry.PC );
					
					continue;
				}
				break;

			case OP_LOGAND:     // Perform logical AND (&&)
			case OP_LOGOR:      // Perform logical OR (||)
			case OP_INCOR:      // Perform bitwise OR (|)
			case OP_EXCOR:      // Perform bitwise XOR (^)
			case OP_BOOLAND:    // Perform bitwise AND (&)
				Entry.SP -= CELL_SIZE; // +2, -1
				break;

			case OP_EQUAL:      // Compare (==)
			case OP_NEQUAL:     // Compare (!=)
				switch (TypeOpcode)
				{

				case TYPE_BINARY_INTINT:
				case TYPE_BINARY_FLOATFLOAT:
				case TYPE_BINARY_OBJECTIDOBJECTID:
				case TYPE_BINARY_STRINGSTRING:
					Entry.SP -= CELL_SIZE; // +2, -1
					break;

				case TYPE_BINARY_STRUCTSTRUCT:
					{
						STACK_POINTER Size;

						Size = (STACK_POINTER) Script->ReadINT16( );

						if (Size & CELL_UNALIGNED)
							throw std::runtime_error( "unaligned struct/struct comparison" );

						Entry.SP -= (2 * Size);
						Entry.SP += CELL_SIZE;
						Entry.PC += Len;

						Script->SetInstructionPointer( Entry.PC );

						continue; // Arguments already consumed.
					}
					break;

				default:
					if ((TypeOpcode >= TYPE_BINARY_ENGINE_FIRST) &&
					    (TypeOpcode <= TYPE_BINARY_ENGINE_LAST))
					{
						Entry.SP -= CELL_SIZE; // +2, -1
					}
					else
					{
						throw std::runtime_error( "illegal EQUAL/NEQUAL operand type" );
					}
					break;

				}
				break;

			case OP_GEQ:        // Compare (>=)
			case OP_GT:         // Compare (>)
			case OP_LT:         // Compare (<)
			case OP_LEQ:        // Compare (<=)
			case OP_SHLEFT:     // Shift left (<<)
			case OP_SHRIGHT:    // Shift right (>>, SAR)
			case OP_USHRIGHT:   // Shift unsigned right (>>)
				Entry.SP -= CELL_SIZE; // +2, -1
				break;

			case OP_ADD:        // Add (+), concatenate strings
			case OP_SUB:        // Subtract (-)
				if (TypeOpcode == TYPE_BINARY_VECTORVECTOR)
					Entry.SP -= 3 * CELL_SIZE;
				else
					Entry.SP -= CELL_SIZE; // +2, -1
				break;

			case OP_MUL:        // Multiply (*)
			case OP_DIV:        // Divide (/)
				Entry.SP -= CELL_SIZE; // +2, -1
				break;

			case OP_MOD:        // Modulus (%)
				Entry.SP -= CELL_SIZE;
				break;

			case OP_NEG:        // Negation (-); SP is unchanged (+1, -1)
			case OP_COMP:       // Complement (~); SP is unchanged (+1, -1)
				break;

			case OP_MOVSP:      // add sp, <n> (always dallocates stack, negative <n>)
				{
					STACK_POINTER Displacement;

					Displacement = (STACK_POINTER) Script->ReadINT32( );

					if (Displacement & CELL_UNALIGNED)
						throw std::runtime_error( "unaligned MOVSP" );
					else if (Displacement > 0)
						throw std::runtime_error( "positive MOVSP" );

					Entry.SP += Displacement;
					Entry.PC += Len;

					Script->SetInstructionPointer( Entry.PC );

					continue;
				}
				break;

			case OP_DESTRUCT:   // Deallocate stack space except for a 'hole'
				{
					STACK_POINTER Size;
					STACK_POINTER ExcludeOffset;
					STACK_POINTER ExcludeSize;

					Size          = (STACK_POINTER) Script->ReadINT16( );
					ExcludeOffset = (STACK_POINTER) Script->ReadINT16( );
					ExcludeSize   = (STACK_POINTER) Script->ReadINT16( );

					if ((Size & CELL_UNALIGNED)          ||
					    (ExcludeOffset & CELL_UNALIGNED) ||
					    (ExcludeSize & CELL_UNALIGNED))
					{
						throw std::runtime_error( "unaligned DESTRUCT" );
					}

					if (ExcludeSize > Size)
						throw std::runtime_error( "too large DESTRUCT.ExcludeSize" );

					Entry.SP -= Size;
					Entry.SP += ExcludeSize;
					Entry.PC += Len;

					Script->SetInstructionPointer( Entry.PC );

					continue;
				}
				break;

			case OP_NOT:        // Logical NOT (!); SP is unchanged (+1, -1)
				break;

			case OP_DECISP:     // Decrement local variable (sp-relative)
			case OP_INCISP:     // Increment local variable (sp-relative)
			case OP_CPDOWNBP:   // Assign to global variables
				break;

			case OP_CPTOPBP:    // Read / duplicate global variables
				{
					STACK_POINTER Offset;
					STACK_POINTER Size;

					Offset = (STACK_POINTER) Script->ReadINT32( );
					Size   = (STACK_POINTER) Script->ReadINT16( );

					if ((Offset & CELL_UNALIGNED) ||
					    (Size & CELL_UNALIGNED))
					{
						throw std::runtime_error( "unaligned CPTOPBP access" );
					}

					Entry.SP += Size;
					Entry.PC += Len;

					Script->SetInstructionPointer( Entry.PC );

					continue;
				}
				break;

			case OP_DECIBP:     // Decrement global variable (bp-relative)
			case OP_INCIBP:     // Increment global variable (bp-relative)
				break;

			case OP_SAVEBP:     // Set global variables
				Entry.SP += CELL_SIZE;
				break;

			case OP_RESTOREBP:  // Clear global variables
				Entry.SP -= CELL_SIZE;
				break;

			case OP_NOP:        // No operation (ignored)
				break;

			default:
				ThrowError( "unrecognized instruction at PC=%0X", Entry.PC );
				break;

			}

			//
			// If we fell through, then this was not a control transfer (jump),
			// and so the PC incremented linearly.  Account for this here.
			//

			Script->AdvanceInstructionPointer( Len - Offset );
			Entry.PC += Len;
		}

		Entry.Function = NULL;

		//
		// Now search through the analysis queue for an entry that is not
		// blocked.  Unless we have finished analyzing the entire program, we
		// should always have at least one remaining unblocked queue entry
		// unless we have discovered a case of infinite recursion (i.e. a
		// function with no flow paths that do not call into another function).
		//

		for (AnalysisQueueVec::iterator it = m_AnalysisQueue.begin( );
		     it != m_AnalysisQueue.end( );
		     ++it)
		{
			if (it->BlockedOn != NULL)
			{
				if (!it->BlockedOn->GetIsAnalyzed( ))
					continue;
			}

			AnalyzeDebug(
				"Analyze queue entry for function %08X (PC %08X, SP %08X) has no blocker (or blocker is not busy) [%p]\n",
				it->Function->GetAddress( ),
				it->PC,
				it->SP,
				it->BlockedOn);

			//
			// We've found a queue entry that is no longer blocked, process it
			// this time through.
			//
			// N.B.  Note that the LabelFlags are completely replaced by those
			//       of the target entry (as LabelFlags are not propagated
			//       cross-subroutine).
			//

			Entry = *it;
			m_AnalysisQueue.erase( it );
			break;
		}

		//
		// If we did not discover a function entry to analyze next, we must be
		// either done (if the queue is empty), or deadlocked (if the queue is
		// not empty).  We would only deadlock if there were an unbounded
		// recursion control flow path.
		//

		if (Entry.Function == NULL)
		{
			if (m_AnalysisQueue.empty( ))
				break;
			else
				throw std::runtime_error( "infinite recursion encountered; analysis aborted" );
		}

		SubseqFlow = FindSubseqControlFlow( Entry );
	}

	//
	// Analysis complete.
	//
}

void
NWScriptAnalyzer::AnalyzeSubroutineCode(
	__in NWScriptReader * Script,
	__in CodeAnalysisEntry & Entry
	)
/*++

Routine Description:

	This routine analyzes the functional semantics of a given subroutine.  Its
	purpose is to prepare a high level intermediate representation (IR) that
	allows the functional semantics of the subroutine to be translated into a
	different form (or otherwise operated upon).

	Local variables are created, types are propagated, and IR versions of each
	functional operation are emitted.

Arguments:

	Script - Supplies the script to analyze.

	Entry - Supplies the analysis parameters of the portion of the subroutine
	        that is to be analyzed.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	unsigned long            Scanned;
	UCHAR                    Opcode;
	UCHAR                    TypeOpcode;
	ULONG                    Len;
	ULONG                    Offset;
	PROGRAM_COUNTER          RelPC;
	PROGRAM_COUNTER          FlowPC;
	STACK_POINTER            SP;
	STACK_POINTER            ReturnSP;
	//STACK_POINTER            LocalSP;
	bool                     Continue;
	Subroutine             * Sub;
	//bool                     Found;
	//bool                     FlowAssigned;
	size_t                   Idx;
	size_t                   PrevIRInstrCount;
	//PROGRAM_COUNTER          NewPC;
	PCSet                    VisitedPCs;
	ControlFlowPtr           Flow;
	ControlFlowPtr           ChildFlow;
	Variable               * Var;
	FlowEndStackMap          StackMap;
	InstructionList       * IR;
	Instruction           * Instr;
#if ANALYZE_DEBUG
	const char            * OpcodeName;
	const char            * TypeOpcodeName;
#endif

	Sub = Entry.Function;

	Entry.PC         = Sub->GetAddress( );
	Entry.SP         = 0;
	Entry.LabelFlags = 0;

	Entry.VarStack.clear( );

	VisitedPCs.insert( Entry.PC );

	//
	// Prepare the initial stack on entry to the subroutine.
	//

	for (STACK_POINTER i = 0; 
		i < Sub->GetReturnSize( ); i += CELL_SIZE)
	{
		Entry.VarStack.push_back( 
			&Sub->GetReturnValueVariable( i / CELL_SIZE ) );
		Entry.SP += CELL_SIZE;
	}

	ReturnSP = Entry.SP;

	// Note the (0, x] range
	for (STACK_POINTER i = Sub->GetParameterSize( ); 
		i > 0; i -= CELL_SIZE)
	{
		Entry.VarStack.push_back( 
			&Sub->GetParameterVariable( (i / CELL_SIZE) - 1 ) );
		Entry.SP += CELL_SIZE;
	}

	//LocalSP = Entry.SP;

	Scanned  = 0;
	Continue = true;

	Script->SetInstructionPointer( Entry.PC );

	AnalyzeDebug(
		"Analyzing function code @ PC=%08X (SP=%08X)...\n",
		Entry.PC,
		Entry.SP);

	//
	// Set up initial flow
	//

	Flow = Sub->GetControlFlow(Entry.PC);
	IR = &Flow->GetIR( );

	while (Continue)
	{
		// Since we're within the bounds of the flows already computed, it 
		// should be impossible for us to run into EOF.
		assert( !Script->ScriptIsEof( ) );

		//
		// Decode and scan the instruction.
		//

		Len = Disassemble( Script, Opcode, TypeOpcode, Offset );

		if (Len < Offset)
			break;

		Scanned += 1;

		if (Scanned > NWScriptVM::ANALYSIS_MAX_SCRIPT_INSTRUCTIONS)
			throw script_error( Entry.PC, INVALID_SP, 
				"too many script instructions in AnalyzeSubroutineCode" );

#if ANALYZE_DEBUG
		GetInstructionNames(
			Opcode,
			TypeOpcode,
			&OpcodeName,
			&TypeOpcodeName);

		AnalyzeDebug(
			"%08X: %02X.%02X   %s%s   SP=%08X\n",
			Entry.PC,
			Opcode,
			TypeOpcode,
			OpcodeName,
			TypeOpcodeName,
			Entry.SP);
#endif

		PrevIRInstrCount = IR->size( );

		switch (Opcode)
		{

		case OP_RETN: // Return from subroutine

			// Nothing special to do as far as variable identification. Just 
			// emit the instruction and we're done.
			IR->push_back( Instruction( Entry.PC, Instruction::I_RETN) );

			break;

		case OP_JSR: // Jump to subroutine (call)
		{
			RelPC = (PROGRAM_COUNTER) Script->ReadINT32( );
			// This will be verified in the first pass
			assert( RelPC );

			FlowPC = Entry.PC + RelPC;
			Sub    = GetSubroutine( FlowPC );

			//
			// Mark parameter and return value variables as such for the
			// call, and begin to build the instruction.
			//

			STACK_POINTER ParameterSize = Sub->GetParameterSize( );

			// 
			// This right here is a little hack for the call to the entry 
			// point. If the entry point takes parameters, they will be 
			// pushed by VM voodoo, NOT by #globals. So we need to 
			// ignore the parameter displacement or we'll fubar the stack.
			// It's necessary that this come before the stack check.
			//
			// Note however that calls to the entry point symbol may be
			// observed outside of #globals (in which case the call will really
			// have the right number of arguments, unlike in #globals).
			//

			if (FlowPC == m_EntryPC && Entry.Function->GetAddress( ) == m_GlobalsPC)
				ParameterSize = 0;

			CheckStackAccess( Entry, ReturnSP, 
				Sub->GetReturnSize( ) + ParameterSize );

			SP  = Entry.SP;
			Idx = 0;

			IR->push_back( Instruction( Entry.PC, Instruction::I_CALL ) );
			Instr = &IR->back( );

			Instr->SetSubroutine( Sub );
			Instr->GetParamVarList( )->assign( 
				(Sub->GetReturnSize( ) + ParameterSize) / CELL_SIZE,
				(Variable *)NULL );

			for (Idx = 0; Idx * CELL_SIZE < (size_t)ParameterSize; Idx++)
			{
				Var = DeleteTopLocal( Entry, IR );
				
				Var->SetClass( Variable::CallParameter );
				Sub->GetParameterVariable( Idx ).LinkTypes( Var );

				Instr->GetParamVarList( )->at( Idx + 
					(Sub->GetReturnSize( ) / CELL_SIZE) ) = Var;
			}

			assert( Entry.SP >= Sub->GetReturnSize( ) );

			SP  = Entry.SP - Sub->GetReturnSize( );
			Idx = 0;

			for (STACK_POINTER RetSP = SP;
			     RetSP < Entry.SP; RetSP += CELL_SIZE, Idx++)
			{
				Var = GetLocalVariable( Entry, RetSP );

				Var->SetClass( Variable::CallReturnValue );
				Sub->GetReturnValueVariable( Idx ).LinkTypes( Var );

				Instr->GetParamVarList( )->at( Idx ) = Var;
			}

			AnalyzeDebug(
				"Function %08X call at PC=%08X has SP displacement %d (%d).\n",
				FlowPC,
				Entry.PC,
				-Sub->GetParameterSize( ),
				ParameterSize);

			break;
		}

		case OP_JZ:  // Jump if zero
		case OP_JNZ: // Jump if not zero
		case OP_JMP: // Unconditional jump
		{
			RelPC = (PROGRAM_COUNTER) Script->ReadINT32( );
			assert( RelPC );

			//
			// Because unconditional jumps correspond to flow termination 
			// (and we have the pointers to the next flow in the flow graph), 
			// there's no need to generate unconditional jumps in the IR 
			// at all. So we won't.
			//

			if (Opcode == OP_JMP)
				break;
			CheckStackAccess( Entry, ReturnSP, CELL_SIZE );

			//
			// The IR must be generated in this order: test, delete, then jump
			IR->push_back( Instruction( Entry.PC, Instruction::I_TEST ) );
			Instr = &IR->back( );

			Var = DeleteTopLocal( Entry, IR );

			Instr->SetConditionVariable( Var );

			// The condition parameter must be an integer
			if (Var->GetType( ) == ACTIONTYPE_VOID)
				Var->SetType( ACTIONTYPE_INT );
			else if (Var->GetType( ) != ACTIONTYPE_INT)
				throw script_error( Entry.PC, Entry.SP - CELL_SIZE, 
					"condition variable not integer" );

			IR->push_back( Instruction( Entry.PC, MapIROpcode( Opcode ) ) );
			Instr = &IR->back( );

			// Find the branch target
			for (LabelVec::iterator TargetIt = 
				Entry.Function->GetBranchTargets( ).begin( );
				TargetIt != Entry.Function->GetBranchTargets( ).end( ); 
				TargetIt++)
			{
				if (TargetIt->GetAddress( ) == Entry.PC + RelPC)
					Instr->SetJumpTarget( &(*TargetIt) );
			}

			assert( Instr->GetJumpTarget( ) );

			break;
		}

		case OP_STORE_STATEALL: // Save a script situation state
		case OP_STORE_STATE:    // Save a script situation state
		{
			RelPC = (PROGRAM_COUNTER) TypeOpcode;

			STACK_POINTER GlobalsSize = 0, LocalsSize = 0;
			if (Opcode == OP_STORE_STATE)
			{
				GlobalsSize = (STACK_POINTER) Script->ReadINT32( );
				LocalsSize = (STACK_POINTER) Script->ReadINT32( );

				CheckGlobalAccess( Entry, -GlobalsSize, GlobalsSize );
			}
			else
			{
				GlobalsSize = (STACK_POINTER) (CELL_SIZE * m_GlobalVariables.size( ));
				LocalsSize = Entry.SP;
			}

			FlowPC = Entry.PC + RelPC;
			Sub    = GetSubroutine( FlowPC );

			IR->push_back( Instruction( Entry.PC, Instruction::I_SAVE_STATE ) );
			Instr = &IR->back( );

			Instr->SetSubroutine( Sub );
			Instr->SetStateNumGlobals( uintptr_t( GlobalsSize / CELL_SIZE ) );

			// Add the globals
			for (STACK_POINTER Offset = 0;
				Offset < GlobalsSize; Offset += CELL_SIZE)
			{
				Var = &GetGlobalVariable( -Offset - CELL_SIZE );

				Instr->GetParamVarList( )->push_back( Var );
			}

			// Now add the locals
			for (STACK_POINTER Offset = 0;
				Offset < LocalsSize; Offset += CELL_SIZE)
			{
				Var = GetLocalVariable( Entry, Entry.SP - Offset - CELL_SIZE);

				Sub->GetParameterVariable( Offset / CELL_SIZE ).LinkTypes( Var );
				Instr->GetParamVarList( )->push_back( Var );
			}

			// Hint to the backend at that this function saves states
			Entry.Function->SetFlags( Entry.Function->GetFlags( ) |
				Subroutine::SAVES_STATE );

			break;
		}

		case OP_CPDOWNSP:   // Copy down SP (assignment operator)
		{
			STACK_POINTER Offset;
			STACK_POINTER Size;

			Offset = (STACK_POINTER) Script->ReadINT32( );
			Size   = (STACK_POINTER) Script->ReadINT16( );

			CheckStackAccess( Entry, 0, Offset, Size );
			// TODO: Verify this is correct behavior:
			if (Offset + Size > -Size)
				throw script_error( 
					Entry.PC, "CPDOWNSP source/destination overlap" );

			for (Idx = 0; Idx < (size_t) Size; Idx += CELL_SIZE)
			{
				Var = GetLocalVariable( 
					Entry, Entry.SP + Offset + (STACK_POINTER) Idx );
				Variable *Source = GetLocalVariable( 
					Entry, Entry.SP - Size + (STACK_POINTER) Idx );

				Var->LinkTypes ( Source );

				IR->push_back( Instruction( 
					Entry.PC, Instruction::I_ASSIGN, Var, Source ) );
			}

			break;
		}

		case OP_RSADD:      // Reserve uninitialized space on the stack

			Var = CreateLocal( Entry, IR, GetOperandType( TypeOpcode ) );

			//
			// Default initialize the variable.
			IR->push_back( Instruction( Entry.PC, Instruction::I_INITIALIZE ) );
			Instr = &IR->back( );

			Instr->SetResultVar( Var );

			break;

		case OP_CPTOPSP:    // Read / duplicate local variables
		{
			STACK_POINTER Offset;
			STACK_POINTER Size;

			Offset = (STACK_POINTER) Script->ReadINT32( );
			Size   = (STACK_POINTER) Script->ReadINT16( );

			CheckStackAccess( Entry, ReturnSP, Offset, Size );

			for (Idx = 0; Idx < (size_t) Size; Idx += CELL_SIZE)
			{
				// N.B. We don't need to add Idx because SP changes 
				// each time we insert a new variable
				Variable * Source = 
					GetLocalVariable( Entry, Entry.SP + Offset );

				Var = CreateLocal( Entry, IR, Source->GetType( ) );
				if (Source->GetType( ) == ACTIONTYPE_VOID)
					Var->LinkTypes( Source );

				IR->push_back( Instruction(
					Entry.PC, Instruction::I_ASSIGN ) );
				Instr = &IR->back( );

				Instr->GetVar( 0 ) = Source;
				Instr->SetResultVar( Var );
			}

			break;
		}

		case OP_CONST:      // Push a constant onto the stack
		{
			//
			// For reasons related to variable merging, we have to generate 
			// two variables here: one constant, which we'll assign a value, 
			// and a variable, which is what actually gets put on the stack.
			// We'll then assign the constant to the variable.
			// 
			// To be precise, it's possible that a value is pushed in two 
			// separate flows that converge to form a single variable, and 
			// it's possible one path might push a constant while one copies 
			// something from the stack (a non-constant).
			//

			NWACTION_TYPE VarType = GetOperandType( TypeOpcode );

			Entry.Function->AddLocal( 
				new Variable( Entry.SP, Variable::Constant, VarType ) );
			Variable * Constant = Entry.Function->GetLocals( ).back( ).get( );

			// Read the constant value and add it to the constant table
			VARIABLE_VALUE Value;
			switch (VarType)
			{
			case ACTIONTYPE_INT:
				Value.Int = ( LONG )Script->ReadINT32( );
				break;
			case ACTIONTYPE_FLOAT:
				Value.Float = Script->ReadFLOAT( );
				break;
			case ACTIONTYPE_STRING:
				Value.StringPtr = new std::string( Script->ReadString( Len - 4 ) );
				break;

			default:
				assert( VarType != ACTIONTYPE_VOID &&
					VarType != ACTIONTYPE_VECTOR &&
					VarType < LASTACTIONTYPE );

				// It's a handle of some kind
				Value.Object = ( UINT )Script->ReadINT32( );
			}
			Value.Type = VarType;

			Var = CreateLocal( Entry, IR, VarType );

			try
			{
				m_ConstantValueMap[Constant] = Value;
			}
			catch (...)
			{
				if (VarType == ACTIONTYPE_STRING)
					delete Value.StringPtr;

				throw;
			}

			IR->push_back( Instruction( Entry.PC, Instruction::I_ASSIGN ) );
			Instr = &IR->back( );
			Instr->GetVar( 0 ) = Constant;
			Instr->SetResultVar( Var );

			break;
		}

		case OP_ACTION:     // Call an engine action API
			{
				PCNWACTION_DEFINITION Action;
				NWSCRIPT_ACTION       ActionId;
				unsigned long         ArgumentCount;

				ActionId      = (NWSCRIPT_ACTION) Script->ReadINT16( );
				ArgumentCount = (unsigned long) Script->ReadINT8( );

				if (ActionId >= m_ActionCount)
					throw script_error( Entry.PC, 0, "out of range action call" );

				Action = &m_ActionDefs[ ActionId ];

				if (ArgumentCount < Action->MinParameters)
					throw script_error( 1, "too few parameters for action call" );
				else if (ArgumentCount > Action->NumParameters)
					throw script_error( 1, "too many parameters for action call" );

				//
				// Set the types of the parameters and pop them, then 
				// push the return values. The generation if the IR will be 
				// a bit messy because there's really no easy way to do it 
				// without recomputing something. So we're gonna do it this 
				// way, which is easy but ugly. We also have to save 
				// a pointer to the IR position before the ACTION, because
				// we must allocate the return value prior to the ACTION.
				//

				InstructionList::iterator CreateIt = IR->insert( 
					IR->end( ), Instruction( Entry.PC, Instruction::I_ACTION ) );
				Instr = &IR->back( );

				Instr->SetActionIndex( ActionId );
				Instr->SetActionParameterCount( ArgumentCount );

				VariableWeakPtrVec * ParamList = Instr->GetParamVarList( );

				ParameterList & Parameters = m_ActionParameters[ ActionId ];
				STACK_POINTER TotalOffset = 0;
				STACK_POINTER TotalStackSize = 0;
				STACK_POINTER ReturnSize = GetTypeSize( Action->ReturnType );

				//
				// Compute the total size for all parameters and check if 
				// there's room for them all.
				//

				for (size_t Arg = 0; Arg < ArgumentCount; Arg++)
					TotalStackSize += GetTypeSize( Action->ParameterTypes[ Arg ] );

				CheckStackAccess( Entry, ReturnSP, TotalStackSize );

				ParamList->assign( 
					size_t( TotalStackSize + ReturnSize ) / CELL_SIZE, NULL );

				//
				// Iterate through the parameters and set the types as appropriate
				//

				SP = Entry.SP;
				for (size_t Arg = 0; Arg < ArgumentCount; Arg++)
				{
					STACK_POINTER ParamSize = 
						GetTypeSize( Action->ParameterTypes[ Arg ] );

					for (STACK_POINTER Offset = 0; 
						Offset < ParamSize; Offset += CELL_SIZE)
					{
						// Now points to the current variable
						SP -= CELL_SIZE;

						Variable * Var = GetLocalVariable( Entry, SP );
						NWACTION_TYPE Type = Parameters[ 
								((Entry.SP - SP) / CELL_SIZE) - 1 ],
							StackType = Var->GetType( );

						if (StackType != ACTIONTYPE_VOID)
						{
							if (StackType != Type)
								throw script_error( Entry.PC, 
									-(TotalOffset + Offset) / CELL_SIZE,
									"argument type mismatch",
									"%s should be %s",
									GetTypeName( StackType ),
									GetTypeName( Type ) );
						}
						else
							Var->SetType( Type );
						Var->SetClass( Variable::CallParameter );

						ParamList->at( (ReturnSize + TotalOffset + Offset) 
							/ CELL_SIZE ) = Var;
					}

					TotalOffset += ParamSize;
				}

				// Pop and delete the parameters in order
				DeleteTopLocals( Entry, TotalOffset, IR );

				if (Action->ReturnType == ACTIONTYPE_VECTOR)
				{
					for (size_t i = 0; i < 3; i++)
					{
						Var = CreateLocal( Entry, IR, CreateIt, 
							ACTIONTYPE_FLOAT, Variable::CallReturnValue );

						ParamList->at( i ) = Var;
					}
				}
				else if (GetTypeSize( Action->ReturnType ) == CELL_SIZE)
				{
					Var = CreateLocal( Entry, IR, CreateIt, 
						Action->ReturnType, Variable::CallReturnValue );

					ParamList->at( 0 ) = Var;
				}
				// Else it's ACTION or void
			}
			break;

		case OP_LOGAND:     // Perform logical AND (&&)
		case OP_LOGOR:      // Perform logical OR (||)
		case OP_INCOR:      // Perform bitwise OR (|)
		case OP_EXCOR:      // Perform bitwise XOR (^)
		case OP_BOOLAND:    // Perform bitwise AND (&)
		{
			CheckStackAccess( Entry, ReturnSP, CELL_SIZE * 2 );

			InstructionList::iterator CreateIt = IR->insert( IR->end( ), 
				Instruction( Entry.PC, MapIROpcode( Opcode ) ) );
			Instr = &IR->back( );

			for (size_t i = 0; i < 2; i++)
			{
				Var = DeleteTopLocal( Entry, IR );

				if (Var->GetType( ) != ACTIONTYPE_INT)
					Var->SetType( ACTIONTYPE_INT );

				Instr->GetVar( i ) = Var;
			}

			Var = CreateLocal( Entry, IR, CreateIt, ACTIONTYPE_INT );
			Instr->SetResultVar( Var );

			break;
		}

		case OP_EQUAL:      // Compare (==)
		case OP_NEQUAL:     // Compare (!=)
		{
			STACK_POINTER TypeSize;

			if (TypeOpcode != TYPE_BINARY_STRUCTSTRUCT)
				TypeSize = GetTypeSize( GetOperandType( TypeOpcode ) );
			else
				TypeSize = (STACK_POINTER) Script->ReadINT16( );

			CheckStackAccess( Entry, ReturnSP, TypeSize * 2 );

			//
			// This is another case where there's no elegant way to do 
			// this, so we're not going to do it elegantly. Create the 
			// result variables (including temporaries), generate the 
			// comparison instructions, then delete the parameters and 
			// temporaries and relocate the final result.
			//

			SP = Entry.SP - (TypeSize * 2);

			// Go through the list and for each variable create a result 
			// variable, perform the operation, and then merge temporary 
			// values into current result.
			Variable * PrevResult = NULL;
			Instruction::INSTR MergeOpcode = (Opcode == OP_EQUAL) 
				? Instruction::I_LOGAND : Instruction::I_INCOR;

			for (STACK_POINTER Offset = 0; 
				Offset < TypeSize; Offset += CELL_SIZE)
			{
				// Create the result variable
				Variable * Result = CreateLocal( Entry, IR, ACTIONTYPE_INT ),
					* Left = GetLocalVariable( Entry, SP + Offset ),
					* Right = GetLocalVariable( Entry, SP + TypeSize + Offset );

				// We only know the exact variable type if we're not doing
				// struct-struct comparison.
				if (TypeOpcode != TYPE_BINARY_STRUCTSTRUCT)
				{
					NWACTION_TYPE Type = GetOperandType( TypeOpcode );

					if (Left->GetType( ) != Type)
						Left->SetType( Type );
					if (Right->GetType( ) != Type)
						Right->SetType( Type );
				}
				else
					Left->LinkTypes( Right );

				// Perform the operation
				IR->push_back( Instruction( 
					Entry.PC, MapIROpcode( Opcode ), 
					Result, Left, Right ) );

				if (Offset == 0)
					PrevResult = Result;
				else
				{
					// Perform the merge. More precisely, 
					// create a merge variable (deferred), perform the 
					// merge with the previous result, and delete the 
					// temporaries (including the previous result), 
					// updating the previous result pointer.
					InstructionList::iterator CreateIt = IR->insert( 
						IR->end( ), Instruction(
						Entry.PC, MergeOpcode, 
						NULL, PrevResult, Result ) );
					Instr = &IR->back( );

					DeleteTopLocals( Entry, CELL_SIZE * 2, IR );

					// N.B. PrevResult is currently dangling
					PrevResult = CreateLocal( 
						Entry, IR, CreateIt, ACTIONTYPE_INT );
					Instr->SetResultVar( PrevResult );
				}
			}

			// Save and pop the final result variable (but don't delete 
			// it), then delete the parameters themselves.
			PrevResult = DeleteTopLocal( Entry, NULL );
			DeleteTopLocals( Entry, TypeSize * 2, IR );

			// Lastly, push the result on the top
			Entry.VarStack.push_back( PrevResult );
			Entry.SP += CELL_SIZE;

			break;
		}

		case OP_GEQ:        // Compare (>=)
		case OP_GT:         // Compare (>)
		case OP_LT:         // Compare (<)
		case OP_LEQ:        // Compare (<=)
		case OP_SHLEFT:     // Shift left (<<)
		case OP_SHRIGHT:    // Shift right (>>, SAR)
		case OP_USHRIGHT:   // Shift unsigned right (>>)
		case OP_MOD:        // Modulus (%)
		{
			CheckStackAccess( Entry, ReturnSP, CELL_SIZE  * 2 );

			InstructionList::iterator CreateIt = IR->insert( IR->end( ), 
				Instruction( Entry.PC, MapIROpcode( Opcode ) ) );
			Instr = &IR->back( );

			// Both parameters will be of the same type. The allowed types 
			// vary by instruction, but will always be specified.
			NWACTION_TYPE VarType = GetOperandType( TypeOpcode );

			for (size_t i = 0; i < 2; i++)
			{
				Var = DeleteTopLocal( Entry, IR );

				if (Var->GetType( ) != VarType)
					Var->SetType( VarType );

				Instr->GetVar( 1 - i ) = Var;
			}

			Var = CreateLocal( Entry, IR, CreateIt, ACTIONTYPE_INT );
			Instr->SetResultVar( Var );

			break;
		}

		case OP_ADD:        // Add (+), concatenate strings
		case OP_SUB:        // Subtract (-)
		case OP_MUL:        // Multiply (*)
		case OP_DIV:        // Divide (/)
		{
			NWACTION_TYPE RightType, 
				LeftType = GetOperandType( TypeOpcode, &RightType );
			STACK_POINTER LeftSize = GetTypeSize( LeftType ),
				RightSize = GetTypeSize( RightType );

			CheckStackAccess( Entry, ReturnSP, LeftSize + RightSize );

			if (TypeOpcode == TYPE_BINARY_VECTORFLOAT ||
				TypeOpcode == TYPE_BINARY_FLOATVECTOR ||
				TypeOpcode == TYPE_BINARY_VECTORVECTOR)
			{
				// Hard path. One or both of the parameters is a vector, 
				// so we're gonna have to generate 3 separate return values.

				// Calculate where the parameters start
				STACK_POINTER RightSP = Entry.SP - RightSize,
					LeftSP = RightSP - LeftSize;
				// So that we only need one loop, we're going to calculate 
				// the stride of both parameters. The stride will be 0 for 
				// atomic types, CELL_SIZE for vectors.
				STACK_POINTER LeftStride = 
						(LeftType == ACTIONTYPE_VECTOR) ? CELL_SIZE : 0, 
					RightStride = 
						(RightType == ACTIONTYPE_VECTOR) ? CELL_SIZE : 0;

				// Generate the result variables and ops
				for (size_t i = 0; i < 3; i++)
				{
					Variable * Result = CreateLocal( Entry, IR, ACTIONTYPE_FLOAT ),
						* Left = GetLocalVariable( 
							Entry, LeftSP + (LeftStride * (STACK_POINTER) i) ),
						* Right = GetLocalVariable( 
							Entry, RightSP + (RightStride * (STACK_POINTER) i) );

					IR->push_back( Instruction( 
						Entry.PC, MapIROpcode( Opcode ), Result, Left, Right ) );
				}

				// Same old: delete the input variables, copy the results
				// to where they should be, then clean the stack.
				for (STACK_POINTER Offset = LeftSize + RightSize; 
					Offset > 0; Offset -= CELL_SIZE)
				{
					Var = GetLocalVariable( Entry, LeftSP + Offset - CELL_SIZE );
					IR->push_back( Instruction( 
						Entry.PC, Instruction::I_DELETE, NULL, Var ) );

					// Now's a convenient time to set the types, as well
					if (Var->GetType( ) != ACTIONTYPE_FLOAT)
						Var->SetType( ACTIONTYPE_FLOAT );
				}

				for (size_t i = 3; i > 0; i--)
				{
					Var = DeleteTopLocal( Entry, NULL );
					Entry.VarStack[ (i - 1) + (LeftSP / CELL_SIZE) ] = Var;

					if ((LeftStride == 0) && (RightStride == 0))
						break;
				}

				DeleteTopLocals( Entry, LeftSize + RightSize - (CELL_SIZE * 3), NULL );
			}
			else
			{
				// Easy path. Simply generate a create, an op, and a delete.
				// Don't forget to watch for int/float ops.
				InstructionList::iterator CreateIt = IR->insert( IR->end( ), 
					Instruction( Entry.PC, MapIROpcode( Opcode ) ) );
				Instr = &IR->back( );

				NWACTION_TYPE Types[2];
				NWACTION_TYPE ResultType;

				// Pop the parameters and add them to the instruction
				Types[ 0 ] = GetOperandType( TypeOpcode, &Types[ 1 ] );
				for (size_t i = 0; i < 2; i++)
				{
					Var = DeleteTopLocal( Entry, IR );

					if (Var->GetType( ) != Types[ 1 - i ])
						Var->SetType( Types[ 1 - i ] );

					Instr->GetVar( 1 - i ) = Var;
				}

				// INTFLOAT and FLOATINT operations always return type FLOAT.
				// Ensure that we assign the result type appropriately; we have
				// to pick it from the right side if it was an INTFLOAT in
				// order to assign type FLOAT to the result.
				if (TypeOpcode == TYPE_BINARY_INTFLOAT)
					ResultType = Types[ 1 ];
				else
					ResultType = Types[ 0 ];

				Var = CreateLocal( Entry, IR, CreateIt, ResultType );
				Instr->SetResultVar( Var );
			}

			break;
		}

		case OP_NEG:        // Negation (-); SP is unchanged (+1, -1)
		case OP_COMP:       // Complement (~); SP is unchanged (+1, -1)
		case OP_NOT:        // Logical NOT (!); SP is unchanged (+1, -1)
		{
			CheckStackAccess( Entry, ReturnSP, CELL_SIZE );

			NWACTION_TYPE VarType = GetOperandType( TypeOpcode );

			InstructionList::iterator CreateIt = IR->insert( IR->end( ), 
				Instruction( Entry.PC, MapIROpcode( Opcode ) ) );
			Instr = &IR->back( );

			Variable * Source = DeleteTopLocal( Entry, IR );
			if (Source->GetType( ) != VarType)
				Source->SetType( VarType );

			Var = CreateLocal( Entry, IR, CreateIt, VarType );
			Instr->GetVar( 0 ) = Source;
			Instr->SetResultVar( Var );

			break;
		}

		case OP_MOVSP:      // add sp, <n> (always dallocates stack, negative <n>)
			{
				STACK_POINTER Displacement;

				Displacement = (STACK_POINTER) Script->ReadINT32( );

				if (Displacement & CELL_UNALIGNED)
					throw script_error( Entry.PC, "unaligned MOVSP" );
				else if (Displacement > 0)
					throw script_error( Entry.PC, "positive MOVSP" );
				else if (Displacement + Entry.SP < 0)
					throw script_error( Entry.PC, 
						Entry.SP / CELL_SIZE,
						"stack underflow",
						"%X bytes to pop, stack size %X bytes",
						-Displacement,
						Entry.SP );

				DeleteTopLocals( Entry, -Displacement, IR );
			}
			break;

		case OP_DESTRUCT:   // Deallocate stack space except for a 'hole'
			{
				STACK_POINTER Size;
				STACK_POINTER ExcludeOffset;
				STACK_POINTER ExcludeSize;

				Size          = (STACK_POINTER) Script->ReadINT16( );
				ExcludeOffset = (STACK_POINTER) Script->ReadINT16( );
				ExcludeSize   = (STACK_POINTER) Script->ReadINT16( );

				CheckStackAccess( Entry, ReturnSP, Size );

				if (ExcludeOffset > Size)
					throw script_error( Entry.PC, "invalid exclude offset" );
				else if (ExcludeSize > Size ||
					ExcludeOffset + ExcludeSize > Size)
					throw script_error( Entry.PC, "too large exclude size" );

				// Move the portion we want to save to where it will be after
				for (STACK_POINTER CurOffset = 0; 
					CurOffset < ExcludeSize; CurOffset += CELL_SIZE)
				{
					Variable *&Source = Entry.VarStack[ 
							(Entry.SP - Size + ExcludeOffset + CurOffset) 
							/ CELL_SIZE ],
						*&Dest = Entry.VarStack[ 
							(Entry.SP - Size + CurOffset) / CELL_SIZE ];

					std::swap( Source, Dest );
				}

				// Release the rest
				DeleteTopLocals( Entry, Size - ExcludeSize, IR );
			}
			break;

		case OP_DECISP:     // Decrement local variable (sp-relative)
		case OP_INCISP:     // Increment local variable (sp-relative)
			STACK_POINTER Displacement;

			Displacement = (STACK_POINTER) Script->ReadINT32( );

			CheckStackAccess( Entry, 0, Displacement, CELL_SIZE );

			Var = GetLocalVariable( Entry, Entry.SP + Displacement );
			if (Var->GetType( ) != ACTIONTYPE_INT)
				Var->SetType( ACTIONTYPE_INT );

			IR->push_back( Instruction( Entry.PC, MapIROpcode( Opcode ), Var, Var ) );

			break;

		case OP_CPDOWNBP:   // Assign to global variables
		{
			STACK_POINTER Offset;
			STACK_POINTER Size;

			Offset = (STACK_POINTER) Script->ReadINT32( );
			Size   = (STACK_POINTER) Script->ReadINT16( );

			CheckStackAccess( Entry, ReturnSP, Size );
			CheckGlobalAccess( Entry, Offset, Size );

			for (STACK_POINTER RelOffset = 0; 
				RelOffset < Size; RelOffset += CELL_SIZE)
			{
				Var = &GetGlobalVariable( Offset + RelOffset );
				Variable * StackVar = GetLocalVariable( 
					Entry, Entry.SP + RelOffset - Size );

				Var->LinkTypes( StackVar );

				IR->push_back( Instruction( 
					Entry.PC, Instruction::I_ASSIGN, Var, StackVar ) );
			}

			break;
		}

		case OP_CPTOPBP:    // Read / duplicate global variables
			{
				STACK_POINTER Offset;
				STACK_POINTER Size;

				Offset = (STACK_POINTER) Script->ReadINT32( );
				Size   = (STACK_POINTER) Script->ReadINT16( );

				CheckGlobalAccess( Entry, Offset, Size );

				for (STACK_POINTER RelOffset = 0; 
					RelOffset < Size; RelOffset += CELL_SIZE)
				{
					Variable * Source = &GetGlobalVariable( Offset + RelOffset ); 

					Var = CreateLocal( Entry, IR, Source->GetType( ) );
					IR->push_back( Instruction(
						Entry.PC, Instruction::I_ASSIGN, Var, Source ) );
				}
			}
			break;

		case OP_DECIBP:     // Decrement global variable (bp-relative)
		case OP_INCIBP:     // Increment global variable (bp-relative)
		{
			STACK_POINTER Offset;

			Offset = (STACK_POINTER) Script->ReadINT32( );

			CheckGlobalAccess( Entry, Offset, CELL_SIZE );

			Var = &GetGlobalVariable( Offset );

			if (Var->GetType( ) != ACTIONTYPE_INT)
				ThrowError( "Global variable %d is not of type int",
					Offset );

			IR->push_back( Instruction( 
				Entry.PC, MapIROpcode( Opcode ), Var, Var ) );

			break;
		}

		case OP_SAVEBP:     // Set global variables

			//
			// Here comes the black magic. For now, we don't support 
			// multiple global frames in the same program, so this must be 
			// inside #globals. At this point the set of global variables 
			// has been created and all types are known. Add these to the 
			// global variable list.
			//

			if (Entry.Function->GetAddress( ) != m_GlobalsPC)
				throw std::runtime_error( "SAVEBP used outside #globals" );
			else if (m_GlobalVariables.size( ) > 0)
				throw std::runtime_error( "SAVEBP after global creation" );

			for (size_t Idx = (Entry.Function->GetReturnSize( ) + 
				Entry.Function->GetParameterSize( )) / CELL_SIZE; 
				Idx < Entry.VarStack.size( ); Idx++)
			{
				// TODO: Mark variable as global, or create a new variable?
				Variable * Source = Entry.VarStack[ Idx ];

				Source->SetClass( Variable::Global );
				m_GlobalVariables.push_back( Source );
			}

			// Create the stack entry for the previous BP
			Var = CreateLocal( Entry );
			Var->SetType( LASTACTIONTYPE );
			break;

		case OP_RESTOREBP:  // Clear global variables
			//
			// It would be nice if we could verify that RESTOREBP matches 
			// up with its SAVEBP, but because there might be some 
			// parameters to the entry point that aren't pushed by #globals, 
			// this is tedious at best. Don't check it for now.
			//

			if (Entry.Function->GetAddress( ) != m_GlobalsPC)
				throw std::runtime_error( "OP_RESTOREBP used outside #globals" );
			/*else if (Entry.VarStack.back( )->GetType( ) != LASTACTIONTYPE)
				throw std::runtime_error( "RESTOREBP does not match SAVEBP" );*/

			//m_GlobalVariables.clear( );

			if (Entry.VarStack.empty( ))
				throw std::runtime_error( "RESTOREBP without global variable frame" );
			Entry.VarStack.pop_back( );
			Entry.SP -= CELL_SIZE;
			break;

		case OP_NOP:        // No operation (ignored)
			break;

		default:
			throw script_error( Entry.PC, "unrecognized instruction" );
			break;

		}

		//
		// Fill in the sequence numbers. We could have done it when we 
		// created all the instructions, but it's more convenient to do it 
		// here after all instructions have been created, as we don't 
		// always generate the instructions in sequence order.
		//

		Idx = IR->size( ) - PrevIRInstrCount;
		for (InstructionList::reverse_iterator InstrIt = IR->rbegin( );
			Idx > 0; Idx--, InstrIt++)
			InstrIt->SetSeqIndex( (unsigned) (Idx - 1) );

		//
		// Increment the PC. Instructions will take care of the script offset.
		//

		Entry.PC += Len;

		if (Entry.PC < Flow->GetEndPC( ))
			// Yeah, continues here are messy, but it seems preferrable
			// to making a huge if-block here.
			continue;

		// If this flow leads to another, save our stack for future use
		if (Flow->GetTerminationType( ) != ControlFlow::Terminate)
			StackMap[ Flow->GetStartPC( ) ] = Entry.VarStack;

		//
		// Verify stack consistency with existing and next flows. Remember 
		// that the SPs in the flow graph do not take into account the 
		// initial function displacement, and correct for that.
		//

		STACK_POINTER Displacement = Entry.Function->GetParameterSize( ) +
			Entry.Function->GetReturnSize( );

		if ((STACK_POINTER)Entry.VarStack.size( ) * CELL_SIZE != Entry.SP ||
			Flow->GetEndPC( ) != Entry.PC ||
			Flow->GetEndSP( ) + Displacement != Entry.SP)
		{
			ThrowError(
				"flow end state mismatch (PC/SP) at PC=%08X, SP=%08X (FlowPC=%08X, FlowSP=%08X)",
				Entry.PC,
				Entry.SP,
				Flow->GetEndPC( ),
				Flow->GetEndSP( ) + Displacement);
		}

		for (size_t Child = 0; Child < 2; Child++)
		{
			ControlFlowPtr NextFlow = Flow->GetChild( Child );

			if (NextFlow.get( ) &&
				NextFlow->GetStartSP( ) + Displacement != Entry.SP)
			{
				ThrowError(
					"flow start state mismatch (PC/SP) at PC=%08X, SP=%08X (FlowPC=%08X, FlowSP=%08X)",
					Entry.PC,
					Entry.SP,
					NextFlow->GetStartPC( ),
					NextFlow->GetStartSP( ) + Displacement);
			}
		}

		//
		// We've reached the end of the current flow. From here there 
		// are three options: continue to the next instruction (the 
		// last valid child of the flow), take the conditional branch
		// (the second last valid child), or pick up a new flow from
		// the queue (current flow has no children that haven't been
		// visited yet). In the latter case, if no queued flows exist, 
		// we're done with this function. What we're gonna do is a bit
		// simpler: we'll queue the branch flow if unvisited, and 
		// continue to the fallthrough flow if unvisited, falling 
		// through to the queue when necessary.
		//

		bool HaveNewFlow = false;
		LabelVec & AnalyzeBranches = Entry.Function->GetAnalyzeBranches( );

		if (Flow->GetTerminationType( ) != ControlFlow::Terminate)
		{
			// We have at least one child flow
			ControlFlowPtr FallthroughFlow = Flow->GetChild( 1 ), 
				BranchFlow;
			if (FallthroughFlow.get( ))
			{
				// We have two flows
				BranchFlow = Flow->GetChild( 0 );

				// Queue the branch flow if unvisited
				if (VisitedPCs.count( BranchFlow->GetStartPC( ) ) == 0)
				{
					AnalyzeBranches.push_back( Label( 
						BranchFlow->GetStartPC( ), Entry.SP, BranchFlow ) );

					VisitedPCs.insert( BranchFlow->GetStartPC( ) );
				}
			}
			else
				// We have one flow
				FallthroughFlow = Flow->GetChild( 0 );

			// Continue to the fallthrough flow if unvisited
			if (VisitedPCs.count( FallthroughFlow->GetStartPC( ) ) == 0)
			{
				Entry.PC = FallthroughFlow->GetStartPC( );
				// Can continue using the same stack
				Flow = FallthroughFlow;
				IR = &Flow->GetIR( );
				// Continue using the same flags

				Script->SetInstructionPointer( Entry.PC );

				VisitedPCs.insert( Entry.PC );

				HaveNewFlow = true;

				AnalyzeDebug(
					"Tracing alternate flow path at PC=%08X SP=%08X.\n",
					Entry.PC,
					Entry.SP);
			}
		}

		// If we found a new flow, start processing it
		if (HaveNewFlow)
			continue;

		// We haven't found a new flow yet. Pick one up from the queue if any.
		if (AnalyzeBranches.empty( ))
		{
			//
			// If we have no more flows to trace, then we're finished.
			//

			Continue = false;

			AnalyzeDebug(
				"Finished code analysis for function %08X at PC=%08X.\n",
				Entry.Function->GetAddress( ),
				Entry.PC);

			Entry.Function->SetIsTypeAnalyzed( true );

			continue;
		}
		//
		// Otherwise, we'll examine the next flow on our list.
		//

		Label Target = AnalyzeBranches.back( );
		AnalyzeBranches.pop_back( );

		Entry.PC          = Target.GetAddress( );
		Entry.SP          = Target.GetSP( );
		Flow              = Target.GetControlFlow( );
		IR                = &Flow->GetIR( );
		//Entry.LabelFlags |= Target.GetFlags( ) & Label::SCRIPT_SITUATION;

		//
		// Find a copy of the stack that we can use for this flow. There 
		// will always be at least one parent flow that has been traced, 
		// by this point, so there will be at least one saved stack we 
		// can use here.
		//

		ControlFlowWeakPtrSet & Parents = 
			Target.GetControlFlow( )->GetParents( );
		for (ControlFlowWeakPtrSet::iterator Parent = Parents.begin( );
			Parent != Parents.end( ); Parent++)
		{
			FlowEndStackMap::iterator StackEntry = 
				StackMap.find( (*Parent)->GetStartPC( ) );
			if (StackEntry != StackMap.end( ))
			{
				// Found one
				Entry.VarStack = StackEntry->second;
				break;
			}
		}

		Script->SetInstructionPointer( Target.GetAddress( ) );

		AnalyzeDebug(
			"Tracing alternate flow path at PC=%08X SP=%08X.\n",
			Entry.PC,
			Entry.SP);

		VisitedPCs.insert( Entry.PC );

		Continue = true;
	}

	//
	// We've finished the instruction-by-instruction analysis. Now we 
	// do the stuff that requires all portions of the function to be 
	// processed.
	//
	// We're now going to look through the list of flows in search of 
	// variables which are created in more than one place but are in fact 
	// the same variable, e.g. in the following pseudo-code:
	//
	// if something
	//		a = 1 [as an optimization, a was not previously allocated]
	// else
	//		a = 2
	// do something with a
	//
	// This will be accomplished by merging variables between different 
	// parent stacks at the beginning of each flow.
	//

	ControlFlowSet & FlowSet = Entry.Function->GetControlFlows( );
	for (ControlFlowSet::iterator FlowIt = FlowSet.begin( );
		FlowIt != FlowSet.end( ); FlowIt++)
	{
		if (FlowIt->second->GetParents( ).size( ) == 0)
			continue;	// Flow has no parents

		ControlFlowWeakPtrSet & ParentList = FlowIt->second->GetParents( );
		ControlFlow * FirstParent = *ParentList.begin( );
		VariableWeakPtrVec & FirstStack =
			StackMap[ FirstParent->GetStartPC( ) ];

		ControlFlowWeakPtrSet::iterator ParentIt = ParentList.begin( );
		ParentIt++;

		for (; ParentIt != ParentList.end( ); ParentIt++)
		{
			ControlFlow * Parent = *ParentIt;
			VariableWeakPtrVec & ParentStack = 
				StackMap[ Parent->GetStartPC( ) ];

			if (FirstParent->GetEndSP( ) != Parent->GetEndSP( ))
				ThrowError(
					"flow end state mismatch (PC/SP) at PC=%08X, SP=%08X (FlowPC=%08X, FlowSP=%08X)",
					FirstParent->GetEndPC( ),
					FirstParent->GetEndSP( ),
					Parent->GetEndPC( ),
					Parent->GetEndSP( ) );

			for (size_t Idx = 0; Idx < FirstStack.size( ); Idx++)
			{
				Variable * Left = FirstStack[ Idx ]->GetHeadVariable( ),
					* Right = ParentStack[ Idx ]->GetHeadVariable( );

				if (Left != Right)
				{
					if (Right == NULL)
						throw std::runtime_error( "merging nonexistant variables" );

					Right->SetMergedWith( Left );
					Left->SetFlag( Variable::MultiplyCreated );
				}
			}
		}
	}
}

void
NWScriptAnalyzer::AnalyzeSubroutineCode(
	__in NWScriptReader * Script
	)
/*++

Routine Description:

	This routine analyzes the functional logic of all subroutines in the script
	program.  Analysis begins at the first discovered subroutine and is
	performed in sequence for all following subroutines.

	High level intermediate representation (IR) instructions are generated for
	each subroutine in this analysis phase.  Additionally, variables are 
	created and tracked, and types are propagated.

Arguments:

	Script - Supplies the script to analyze.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, subroutine structural analysis completed.

--*/
{
	CodeAnalysisEntry        Entry;

	//
	// Perform analysis of the #globals function to discover type of all 
	// global variables. For this, we'll use exactly the same trick we did
	// when determining the structure of #globals: mark the entry point as 
	// already analyzed, analyze #globals like any other function, then go 
	// back and analyze the entry point and descendents.
	//

	if (m_GlobalsPC != INVALID_PC)
	{
		Subroutine * Entrypoint = GetSubroutine( m_EntryPC );
		Entrypoint->SetIsTypeAnalyzed( true );

		//
		// This is the globals function. This will, hackishly, save us 
		// a copy of the stack when SAVEBP is executed. As these must 
		// all be explicitely pushed or reserved, all of the types will 
		// be known. Then we just save that info for future use.
		//

		Entry.Function = GetSubroutine( m_GlobalsPC );

		AnalyzeSubroutineCode( Script, Entry );

		Entrypoint->SetIsTypeAnalyzed( false );
	}

	//
	// Now, analyze all other subroutines in order.  We do not need to perform
	// the analysis in any particular order, as data flow dependency chains are
	// generated cross-subroutine.  Once any portion of a data flow dependency
	// chain has a known type, the entire chain's types are set.
	//

	for (SubroutinePtrVec::iterator it = m_Subroutines.begin( );
	     it != m_Subroutines.end( ); it++)
	{
		//
		// Set up for analysis on this subroutine.
		//

		Subroutine *Sub = it->get( );

		if (Sub->GetIsTypeAnalyzed( ))
			continue;

		Entry.Function   = Sub;

		AnalyzeSubroutineCode( Script, Entry );
	}

	//
	// Almost done. But we have one more thing to do before we return:
	// populate the parameter type list for each function with its types 
	// that we figured out for the Variables of.
	//

	for (SubroutinePtrVec::iterator it = m_Subroutines.begin( );
     it != m_Subroutines.end( ); it++)
	{
		Subroutine *Sub = it->get( );

		for (size_t i = 0; i < Sub->GetParameters( ).size( ); i++)
			Sub->GetParameters( )[ i ] = 
				Sub->GetParameterVariable( i ).GetType( );

		for (size_t i = 0; i < Sub->GetNumReturnTypes( ); i++)
			Sub->GetReturnTypes( )[ i ] = 
				Sub->GetReturnValueVariable( i ).GetType( );
	}

	//
	// Analysis complete. 
	//

#if ANALYZE_DEBUG
	char OutBuffer[4096];
	int BufferSize;

	for (SubroutinePtrVec::iterator it = m_Subroutines.begin( );
	     it != m_Subroutines.end( ); it++)
	{
		*OutBuffer = '\0';
		BufferSize = 0;

		Subroutine * Sub = it->get( );

		BufferSize += sprintf_s( OutBuffer + BufferSize, 
			sizeof(OutBuffer) - BufferSize,
			"Function %08X%s returns {",
			Sub->GetAddress( ),
			(Sub->GetFlags( ) & Subroutine::SCRIPT_SITUATION) ? " (script situation)" : "" );

		size_t NumReturns = Sub->GetReturnTypes( ).size( );
		for (size_t Return = 0; Return < NumReturns; Return++)
		{
			BufferSize += sprintf_s( OutBuffer + BufferSize, 
				sizeof(OutBuffer) - BufferSize,
				 " %s%s", 
				GetTypeName( Sub->GetReturnValueVariable( Return ).GetType( ) ),
				Return + 1 < NumReturns ? "," : "" );
		}

		BufferSize += sprintf_s( OutBuffer + BufferSize, 
			sizeof(OutBuffer) - BufferSize,
			  " }, takes {" );

		size_t NumParams = Sub->GetParameters( ).size( );
		for (size_t Param = 0; Param < NumParams; Param++)
		{
			BufferSize += sprintf_s( OutBuffer + BufferSize, 
				sizeof(OutBuffer) - BufferSize,
				 " %s%s", 
				GetTypeName( Sub->GetParameterVariable( Param ).GetType( ) ),
				Param + 1 < NumParams ? "," : "" );
		}

		BufferSize += sprintf_s( OutBuffer + BufferSize, 
			sizeof(OutBuffer) - BufferSize,
			" }\n" );
		AnalyzeDebug( "%s", OutBuffer );

		// Print out whether there are any unidentified vars
		size_t NumUntypedLocals = 0, NumUntypedParams = 0, NumUntypedRets = 0;
		for (size_t Idx = 0; Idx < Sub->GetLocals( ).size( ); Idx++)
		{
			Variable *Var = Sub->GetLocals( )[ Idx ].get( );

			if (Var->GetType( ) == ACTIONTYPE_VOID)
			{
				if (Var->GetClass( ) == Variable::Parameter)
					NumUntypedParams++;
				else if (Var->GetClass( ) == Variable::ReturnValue)
					NumUntypedRets++;
				else
					NumUntypedLocals++;
			}
		}

		if (NumUntypedParams + NumUntypedRets + NumUntypedLocals > 0)
			AnalyzeDebug( "Warning: Function %08X has %d parameters, %d return values, and %d local variables of unknown type\n", 
				Sub->GetAddress( ),
				NumUntypedParams,
				NumUntypedRets,
				NumUntypedLocals);
	}

	// Print out the number of variables for each function. It's a lot 
	// easier to see if we do it separately from the last output.
	for (SubroutinePtrVec::iterator it = m_Subroutines.begin( );
	     it != m_Subroutines.end( ); it++)
	{
		AnalyzeDebug( "Function %08X has %d total variable references\n",
			(*it)->GetAddress( ),
			(*it)->GetLocals( ).size( ) );
	}

#endif
}

bool NWScriptAnalyzer::IsMCVarInDoubleCreatePair( 
	__in VariableData & VarData
	)
{
	// By this point the DELETE address will be set
	assert( VarData.HaveDeleteAddr );

	// Must have CREATE and DELETE in same flow
	if (!VarData.HaveCreateAddr)
		return false;

	// CREATE and DELETE must be in same bytecode instruction
	if (VarData.CreateAddr->GetAddress( ) != VarData.DeleteAddr->GetAddress( ))
		return false;

	// Looks like a duck, quacks like a duck
	return true;
}

void
NWScriptAnalyzer::PostProcessIR(
	__in NWScriptControlFlow & Flow,
	__in IRAnalysisData & Data,
	__in bool Optimize
	)
{
	Data.VarDataMap.clear( );
	Data.VarCopiedToMap.clear( );

	Data.InstrsToErase.clear( );

	// Scan through the list of instructions looking for the ones we're 
	// interested in.
	InstructionList & IR = Flow.GetIR( );

	for (InstructionList::iterator InstrIt = IR.begin( );
		InstrIt != IR.end( ); InstrIt++)
	{
		Instruction & Instr = *InstrIt;

		Data.ReadVars.clear( );
		Data.WriteVars.clear( );

		// If the instruction reads from any variables, remember it
		GetInstructionVariableLists( Instr, &Data.ReadVars, &Data.WriteVars );
		for (VariableWeakPtrVec::iterator VarIt = Data.ReadVars.begin( );
			VarIt != Data.ReadVars.end( ); VarIt++)
		{
			VariableData & VarData = 
				Data.VarDataMap[ (*VarIt)->GetHeadVariable( ) ];
			assert( !VarData.HaveDeleteAddr );

			VarData.ReadAddrs.insert( Instr.GetExtAddress( ) );
		}

		// If the instruction writes to any variables, remember it
		for (VariableWeakPtrVec::iterator VarIt = Data.WriteVars.begin( );
			VarIt != Data.WriteVars.end( ); VarIt++)
		{
			VariableData & VarData = 
				Data.VarDataMap[ (*VarIt)->GetHeadVariable( ) ];
			assert( !VarData.HaveDeleteAddr );

			VarData.WriteAddrs.insert( Instr.GetExtAddress( ) );

			// If the variable was previously a copy of something, it won't 
			// be a copy of it anymore. It is, however, possible that this 
			// is an I_ASSIGN, which will make a new copy; in this case, the 
			// below I_ASSIGN handler will take care of that part.
			if (VarData.CopiedFrom != NULL)
			{
				CopiedToMapEqualRange EqualRange = 
					Data.VarCopiedToMap.equal_range( VarData.CopiedFrom );
				for (VariableCopiedToMap::iterator CopyIt = EqualRange.first;
					CopyIt != EqualRange.second; CopyIt++)
				{
					if (CopyIt->second == *VarIt)
					{
						Data.VarCopiedToMap.erase( CopyIt );
						break;
					}
				}

				VarData.CopiedFrom = NULL;
			}

			// If the variable was previously INITIALIZEd but never read 
			// from, we can eliminate the INITIALIZE as redundant.
			if (Optimize && VarData.HaveInitAddr)
			{
				// We delete the INITIALIZE, so if we have one now that 
				// means that this is the first write since CREATE.

				// Was it read from since INITIALIZE?
				EXT_PROGRAM_COUNTER EPC = EXT_PROGRAM_COUNTER(
					VarData.InitAddr->GetAddress( ), 
					VarData.InitAddr->GetSeqIndex( ) + 1);
				IRPCSet::iterator LowerBound = 
					VarData.ReadAddrs.lower_bound( EPC );
				IRPCSet::iterator UpperBound = 
					VarData.ReadAddrs.upper_bound( Instr.GetExtAddress( ) );

				if (LowerBound == UpperBound)
				{
					// It hasn't been read from. Delete the INITIALIZE.
					Data.InstrsToErase.push_back( VarData.InitAddr );

					VarData.HaveInitAddr = false;
				}
			}
		}

		if (Instr.GetType( ) == Instruction::I_CREATE)
		{
			// If the instruction creates a variable, note the location
			Variable * Var = Instr.GetVar( 0 )->GetHeadVariable( );
			VariableData & VarData = Data.VarDataMap[ Var ];

			// There should never be two CREATEs or an INITIALIZE, CREATE
			// in the same flow, but a CREATE may follow a DELETE or ASSIGN 
			// for multiply created variables.
			assert( !VarData.HaveCreateAddr );
			assert( !VarData.HaveInitAddr );
			if (!Var->IsFlagSet( Variable::MultiplyCreated ) )
			{
				assert( !VarData.HaveAssignAddr );
				assert( !VarData.HaveDeleteAddr );
			}

			VarData.CreateAddr = InstrIt;
			VarData.HaveCreateAddr = true;
		}
		else if (Instr.GetType( ) == Instruction::I_INITIALIZE)
		{
			Variable * Var = Instr.GetResultVar( )->GetHeadVariable( );
			VariableData & VarData = Data.VarDataMap[ Var ];

			// Prior to our optimization INITIALIZE must follow a CREATE. 
			// It can only follow a DELETE if there's a subsequent CREATE.
			assert( VarData.HaveCreateAddr );
			if (!Var->IsFlagSet( Variable::MultiplyCreated ) )
				assert( !VarData.HaveDeleteAddr );
			else if (VarData.HaveDeleteAddr)
				assert(VarData.DeleteAddr->GetExtAddress( ) < 
					VarData.CreateAddr->GetExtAddress( ));

			VarData.InitAddr = InstrIt;
			VarData.HaveInitAddr = true;

			// Whether we count INITIALIZE as a write is context-dependant, 
			// so don't set it here.
		}
		else if (Instr.GetType( ) == Instruction::I_ASSIGN)
		{
			// Remember the last assignment for this variable
			Variable * SourceVar = Instr.GetVar( 0 )->GetHeadVariable( ),
				* DestVar = Instr.GetResultVar( )->GetHeadVariable( );
			VariableData & DestData = 
				Data.VarDataMap[ DestVar ];

			// Same conditions as INITIALIZE
			if (!DestVar->IsFlagSet( Variable::MultiplyCreated ) )
				assert( !DestData.HaveDeleteAddr );
			else if (DestData.HaveDeleteAddr)
				assert(DestData.HaveCreateAddr && 
					(DestData.DeleteAddr->GetExtAddress( ) < 
					DestData.CreateAddr->GetExtAddress( )));

			DestData.AssignAddr = InstrIt;
			DestData.HaveAssignAddr = true;

			// By now any existing copied from link will be severed. Create 
			// a new one.
			DestData.CopiedFrom = Instr.GetVar( 0 );
			Data.VarCopiedToMap.insert( 
				VariableCopiedToMap::value_type( SourceVar, DestVar ) );
		}
		else if (Instr.GetType( ) == Instruction::I_DELETE)
		{
			// TODO: Make sure this all works with already merged variable
			Variable * Var = Instr.GetVar( 0 )->GetHeadVariable( );
			VariableData & VarData = Data.VarDataMap[ Var ];

			assert( !VarData.HaveDeleteAddr );
			VarData.DeleteAddr = InstrIt;
			VarData.HaveDeleteAddr = true;

			// We don't want to mess with it if it's not local to this flow.
			// This is true even for multiply created variables
			if (!VarData.HaveCreateAddr)
				continue;

			// At this point we have a CREATE and DELETE in the same flow. 
			// This means either that we have a flow-local variable or we 
			// have one out-of-order pair of a multiply created variable.
			if (Var->IsFlagSet( Variable::MultiplyCreated ) &&
				IsMCVarInDoubleCreatePair( VarData ))
			{
				// Delete the excess CREATE and DELETE
				Data.InstrsToErase.push_back( VarData.CreateAddr );
				Data.InstrsToErase.push_back( VarData.DeleteAddr );

				VarData.HaveCreateAddr = false;
				VarData.HaveDeleteAddr = false;

				// We've taken care of it here; do not proceed with optimization
				continue;
			}

			// Note that when we scan the #globals function we'll detect
			// that globals are flow-local (among other things). We don't 
			// want to make globals this way, so check for that.
			if (Var->GetClass( ) != Variable::Global)
			{
				Var->SetFlag( Variable::LocalToFlow );

				if (VarData.WriteAddrs.size( ) == 1)
					Var->SetFlag( Variable::SingleAssignment );
				if (VarData.ReadAddrs.size( ) == 0)
					Var->SetFlag( Variable::WriteOnly );
			}

			// Okay, we've marked it for the various properties. If we don't 
			// need to optimize, we're done here.
			if (!Optimize)
				continue;

			// If it's not single-assignment, it's probably not a temporary
			// TODO: Eliminate unused vars
			if (VarData.WriteAddrs.size( ) != 1 ||
				VarData.ReadAddrs.size( ) == 0)
				continue;

			// If the variable requires manually managed storage and isn't made
			// with a single creation CREATE, then avoid optimizing it so that
			// merged versions of the variable aren't included in variable
			// lists twice.
			// TODO: It would be best to avoid having the duplicate entries to
			//       begin with rather than excluding optimization of these.
			if (Var->GetRequiresExplicitStorage( ))
				continue;
			
			if (VarData.CopiedFrom)
			{
				// The variable is a copy of something, and it's in SSA form. 
				// That means we can potientiallye eliminate it as an input 
				// temporary. However, this requires that the source variable 
				// not be modified between the time when this variable was 
				// assigned and when it was read. Verify that now.
				VariableData & SourceData = 
					Data.VarDataMap[ VarData.CopiedFrom ];

				IRPCSet::iterator BeginIt = SourceData.WriteAddrs.lower_bound( 
					*VarData.WriteAddrs.rbegin( ) );
				IRPCSet::iterator EndIt = SourceData.WriteAddrs.upper_bound(
					*VarData.ReadAddrs.rbegin( ) );

				if (BeginIt != EndIt)
					continue;	// It was modified
				// TODO: Allow extension of lifetime
				else if (SourceData.HaveDeleteAddr &&
					SourceData.DeleteAddr->GetExtAddress( )
					< *VarData.ReadAddrs.rbegin( ) )
					continue; // It was deleted prior to where we need it
				else if (VarData.CopiedFrom->GetRequiresExplicitStorage( ))
					continue; // TODO:  Fix duplicate local lists instead

				// All systems are go. Merge it.
				Var->SetMergedWith( VarData.CopiedFrom );
				Var->SetFlag( Variable::OptimizerEliminated );

				Data.InstrsToErase.push_back( VarData.CreateAddr );
				Data.InstrsToErase.push_back( VarData.AssignAddr );
				Data.InstrsToErase.push_back( VarData.DeleteAddr );

				SourceData.ReadAddrs.insert( 
					VarData.ReadAddrs.begin( ),
					VarData.ReadAddrs.end( ) );
				// Eliminate the write address where the copy was made

				CopiedToMapEqualRange EqualRange = 
					Data.VarCopiedToMap.equal_range( VarData.CopiedFrom );
				for (VariableCopiedToMap::iterator CopyIt = EqualRange.first;
					CopyIt != EqualRange.second; CopyIt++)
				{
					if (CopyIt->second == Var)
					{
						Data.VarCopiedToMap.erase( CopyIt );
						break;
					}
				}

				VarData.CopiedFrom = NULL;
			}
			else
			{
				// The variable isn't a copy of anything. However, if another 
				// variable is a copy of this, we might be able to merge this 
				// into the other variable. This requires, however, that the 
				// copy not be read between when this variable acquired its
				// value and when the copy is made.

				CopiedToMapEqualRange EqualRange = 
					Data.VarCopiedToMap.equal_range( Var );

				Variable * AcceptVar = NULL, * GoodVar = NULL;
				for (VariableCopiedToMap::iterator CopyIt = EqualRange.first;
					CopyIt != EqualRange.second; CopyIt++)
				{
					// There are some variables that are a copy of this. Look for 
					// one that would make a good candidate to merge with.
					Variable * CopyVar = CopyIt->second;
					VariableData & CopyData = 
						Data.VarDataMap[ CopyVar ];

					assert( CopyData.HaveAssignAddr );
					if (CopyData.HaveCreateAddr &&
						CopyData.CreateAddr->GetExtAddress( ) >
						*VarData.WriteAddrs.begin( ))
						continue;	// It didn't exist when this variable was assigned
					// TODO: Support life extension

					IRPCSet::iterator BeginIt = CopyData.ReadAddrs.lower_bound( 
						*VarData.WriteAddrs.rbegin( ) );
					IRPCSet::iterator EndIt = CopyData.ReadAddrs.upper_bound(
						CopyData.AssignAddr->GetExtAddress( ) );

					if (BeginIt != EndIt)
						continue;	// No good. It was read.

					// We have found a candidate
					// TODO: See whether it's a good one
					AcceptVar = CopyVar;
					break;
				}

				if (!AcceptVar)
					continue;	// No variable we can merge with
				if (!GoodVar)
					GoodVar = AcceptVar;
				if (GoodVar->GetRequiresExplicitStorage( ))
					continue; // TODO:  Fix duplicate local lists instead

				VariableData & CopyData = 
					Data.VarDataMap[ GoodVar ];

				// We have a variable, now we're gonna merge with it
				Var->SetMergedWith( GoodVar );
				Var->SetFlag( Variable::OptimizerEliminated );

				Data.InstrsToErase.push_back( VarData.CreateAddr );
				Data.InstrsToErase.push_back( CopyData.AssignAddr );
				Data.InstrsToErase.push_back( VarData.DeleteAddr );

				CopyData.ReadAddrs.insert( 
					VarData.ReadAddrs.begin( ),
					VarData.ReadAddrs.end( ) );
				CopyData.WriteAddrs.insert( 
					VarData.WriteAddrs.begin( ),
					VarData.WriteAddrs.end( ) );
				// Delete the reference where the copy is made
				CopyData.WriteAddrs.erase( 
					CopyData.AssignAddr->GetExtAddress( ) );

				EqualRange = Data.VarCopiedToMap.equal_range( Var );
				for (VariableCopiedToMap::iterator CopyIt = EqualRange.first;
					CopyIt != EqualRange.second; CopyIt++)
				{
					if (CopyIt->second == GoodVar)
					{
						Data.VarCopiedToMap.erase( CopyIt );
						break;
					}
				}

				CopyData.CopiedFrom = NULL;
			}
		}
	}

	// Now that we've finished scanning through the IR, we need to make 
	// any scheduled deletions.
	for (InstructionItVec::iterator EraseIt = Data.InstrsToErase.begin( );
		EraseIt != Data.InstrsToErase.end( ); EraseIt++)
		IR.erase( *EraseIt );
}

void
NWScriptAnalyzer::PostProcessIR(
	__in bool Optimize
	)
/*++

Routine Description:

	This routine postprocesses the generated IR in order to perform high level
	tasks such as optimization.

Arguments:

	Optimize - Supplies a Boolean value that indicates true if optimizations
	           are to be enabled, else false if they are to be disabled.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, IR generation completed.

--*/
{
	IRAnalysisData Data;

	for (SubroutinePtrVec::iterator SubIt = m_Subroutines.begin( );
		SubIt != m_Subroutines.end( ); SubIt++)
	{
		for (ControlFlowSet::iterator FlowIt = 
			(*SubIt)->GetControlFlows( ).begin( );
			FlowIt != (*SubIt)->GetControlFlows( ).end( );
			FlowIt++)
		{
			PostProcessIR( *FlowIt->second, Data, Optimize );
		}
	}

#if ANALYZE_DEBUG
	for (SubroutinePtrVec::iterator SubIt = m_Subroutines.begin( );
		SubIt != m_Subroutines.end( ); SubIt++)
	{
		unsigned int NumLocals = 0, NumConsts = 0, 
			NumTemps = 0, NumSSAs = 0, NumMerged = 0;

		for (VariablePtrVec::iterator VarIt = (*SubIt)->GetLocals( ).begin( );
			VarIt != (*SubIt)->GetLocals( ).end( ); VarIt++)
		{
			VariablePtr Var = *VarIt;

			switch (Var->GetClass( ))
			{
			case Variable::Constant:
				NumConsts++;
				break;

			case Variable::CallParameter:
			case Variable::CallReturnValue:
			case Variable::Local:
				if (Var->IsFlagSet( Variable:: OptimizerEliminated ) )
				{
					NumMerged++;
					break;
				}

				NumLocals++;

				if (Var->IsFlagSet( Variable::LocalToFlow ))
					NumTemps++;
				if (Var->IsFlagSet( Variable:: SingleAssignment ))
					NumSSAs++;

				break;
			}
		}

		AnalyzeDebug( "Function %08X%s has %d constants, %d merged, %d locals, %d temporaries, %d single-assigns\n",
			(*SubIt)->GetAddress( ),
			((*SubIt)->GetFlags( ) & Subroutine::SCRIPT_SITUATION) ? 
			" (script situation)" : "",
			NumConsts,
			NumMerged,
			NumLocals,
			NumTemps,
			NumSSAs );
	}
#endif
}

void
NWScriptAnalyzer::PrintIR(
	)
/*++

Routine Description:

	This routine displays the contents of the IR tree to the debugger console.

Arguments:

	None.

Return Value:

	None.  On failure, an std::exception is raised.

Environment:

	User mode, IR postprocessing completed.

--*/
{
	char OutBuffer[4096];
	size_t BufferSize;

	for (SubroutinePtrVec::iterator it = m_Subroutines.begin( );
	     it != m_Subroutines.end( ); it++)
	{
		Subroutine * Sub = it->get( );

		PrintIROut( "Printing IR for function %08X%s (%s)\n",
			Sub->GetAddress( ),
			(Sub->GetFlags( ) & Subroutine::SCRIPT_SITUATION) ? 
			" (script situation)" : "",
			Sub->GetSymbolName( ).c_str( ));

		for (ControlFlowSet::iterator FlowIt = Sub->GetControlFlows( ).begin( );
			FlowIt != Sub->GetControlFlows( ).end( ); FlowIt++)
		{
			PrintIROut( "label %08X:\n",
				FlowIt->second->GetStartPC( ) );

			InstructionList * IR = &FlowIt->second->GetIR( );

			for (InstructionList::iterator InstrIt = IR->begin( );
				InstrIt != IR->end( ); InstrIt++)
			{
				Instruction &Instr = *InstrIt;

				*OutBuffer = '\0';
				BufferSize = 0;

				BufferSize += sprintf_s( OutBuffer + BufferSize,
					sizeof(OutBuffer) - BufferSize,
					"%08X: %-6s ",
					Instr.GetAddress( ),
					GetIROpcodeName( Instr.GetType( ) ) );

				switch (Instr.GetType( ))
				{
				case Instruction::I_JZ: 
				case Instruction::I_JNZ:
				case Instruction::I_JMP:
					BufferSize += sprintf_s( OutBuffer + BufferSize,
						sizeof(OutBuffer) - BufferSize,
						"%08X",
						Instr.GetLabel( 0 )->GetAddress( ) );
					break;

				case Instruction::I_RETN: 
					break;

				// The solitary instruction with a return but no parameter
				case Instruction::I_INITIALIZE:
					AppendVarString( 
						OutBuffer, 
						BufferSize, 
						sizeof(OutBuffer), 
						Instr.GetResultVar( ) );

					break;

				// Unary
				case Instruction::I_CREATE: 
				case Instruction::I_DELETE:
				case Instruction::I_TEST:
					AppendVarString( 
						OutBuffer, 
						BufferSize, 
						sizeof(OutBuffer), 
						Instr.GetVar( 0 ) );

					break;

				// Unary with return
				case Instruction::I_ASSIGN: 
				case Instruction::I_NEG:
				case Instruction::I_COMP: 
				case Instruction::I_NOT:
				case Instruction::I_INC:
				case Instruction::I_DEC:
				{
					AppendVarString( 
						OutBuffer, 
						BufferSize, 
						sizeof(OutBuffer), 
						Instr.GetResultVar( ) );

					AppendVarString( 
						OutBuffer, 
						BufferSize, 
						sizeof(OutBuffer), 
						Instr.GetVar( 0 ),
						true);

					break;
				}

				// Binary with return
				case Instruction::I_LOGAND: 
				case Instruction::I_LOGOR:
				case Instruction::I_INCOR:
				case Instruction::I_EXCOR:
				case Instruction::I_BOOLAND:
				case Instruction::I_EQUAL:
				//case Instruction::I_EQUALMULTI: 
				case Instruction::I_NEQUAL: 
				//case Instruction::I_NEQUALMULTI:
				case Instruction::I_GEQ:
				case Instruction::I_GT: 
				case Instruction::I_LT: 
				case Instruction::I_LEQ:
				case Instruction::I_SHLEFT: 
				case Instruction::I_SHRIGHT:
				case Instruction::I_USHRIGHT:
				case Instruction::I_ADD:
				case Instruction::I_SUB:
				case Instruction::I_MUL:
				case Instruction::I_DIV:
				case Instruction::I_MOD:
				{
					AppendVarString( 
						OutBuffer, 
						BufferSize, 
						sizeof(OutBuffer), 
						Instr.GetResultVar( ) );

					for (size_t i = 0; i < 2; i++)
					{
						AppendVarString( 
							OutBuffer, 
							BufferSize, 
							sizeof(OutBuffer), 
							Instr.GetVar( i ),
							true );
					}

					break;
				}

				case Instruction::I_CALL: 
				case Instruction::I_SAVE_STATE: 
					BufferSize += sprintf_s( OutBuffer + BufferSize,
						sizeof(OutBuffer) - BufferSize,
						"%08X (%s) ",
						Instr.GetSubroutine( )->GetAddress( ),
						Instr.GetSubroutine( )->GetSymbolName( ).c_str( ) );

					for (VariableWeakPtrVec::iterator VarIt = 
							Instr.GetParamVarList( )->begin( );
						VarIt != Instr.GetParamVarList( )->end( ); VarIt++)
					{
						AppendVarString( 
							OutBuffer, 
							BufferSize, 
							sizeof(OutBuffer), 
							*VarIt,
							VarIt != Instr.GetParamVarList( )->begin( ) );
					}

					break;

				case Instruction::I_ACTION: 
					BufferSize += sprintf_s( OutBuffer + BufferSize,
						sizeof(OutBuffer) - BufferSize,
						"%04X (%s) (%d) ",
						Instr.GetActionIndex( ),
#if NWACTION_DEF_INCLUDE_NAME
						m_ActionDefs[ Instr.GetActionIndex( ) ].Name,
#else
						"",
#endif
						Instr.GetActionParameterCount( )
						);

					for (VariableWeakPtrVec::iterator VarIt = 
							Instr.GetParamVarList( )->begin( );
						VarIt != Instr.GetParamVarList( )->end( ); VarIt++)
					{
						AppendVarString( 
							OutBuffer, 
							BufferSize, 
							sizeof(OutBuffer), 
							*VarIt,
							VarIt != Instr.GetParamVarList( )->begin( ) );
					}

					break;

				default:
					break;
				}

				PrintIROut( "%s\n",
					OutBuffer );
			}

			ControlFlowPtr NextFlow = FlowIt->second->GetChild( 1 );
			if (!NextFlow.get( ))
				NextFlow = FlowIt->second->GetChild( 0 );
			if (NextFlow.get( ))
			{
				PrintIROut( "          goto   %08X\n",
					NextFlow->GetStartPC( ) );
			}
		}
	}
}

NWNScriptLib::PROGRAM_COUNTER
NWScriptAnalyzer::FindInstructionInFlow(
	__in NWNScriptLib::PROGRAM_COUNTER PC,
	__in NWScriptReader * Script,
	__in UCHAR ScanOpcode
	)
/*++

Routine Description:

	This routine searches a subroutine control flow for the first occurance of
	a particular instruction.  The search terminates unsuccessfully at OP_RETN.

Arguments:

	PC - Supplies the starting program counter that the script is positioned
	     at.

	Script - Supplies the script to analyze.

	ScanOpcode - Supplies the instruction opcode to scan for.

Return Value:

	The absolute program counter offset of the first occurance of a given
	instruction is returned.  Should OP_RETN be reached first, INVALID_PC is
	returned.

	On catastrophic failure (including EOF), an std::exception is raised.

Environment:

	User mode.

--*/
{
	unsigned long   Scanned;
	PCStack         FlowStack;
	PCVec           VisitedFlows;
	UCHAR           Opcode;
	UCHAR           TypeOpcode;
	ULONG           Len;
	ULONG           Offset;
	PROGRAM_COUNTER RelPC;
	PROGRAM_COUNTER FlowPC;
	bool            Continue;

	Scanned  = 0;
	Continue = true;

	while (Continue)
	{
		if (Script->ScriptIsEof( ))
			throw std::runtime_error( "reached eof in FindInstructionInFlow" );

		//
		// Decode and scan the instruction.
		//

		Len = Disassemble( Script, Opcode, TypeOpcode, Offset );

		if (Len < Offset)
			break;

		Scanned += 1;

		if (Scanned > NWScriptVM::ANALYSIS_MAX_SCRIPT_INSTRUCTIONS)
			throw std::runtime_error( "too many script instructions in FindInstructionInFlow" );

		//
		// If we've found the desired opcode, halt here and return its address
		// to the caller.
		//

		if (Opcode == ScanOpcode)
			return PC;

		switch (Opcode)
		{

		case OP_RETN: // Return from subroutine
			if (FlowStack.empty( ))
			{
				//
				// If we have more flows to trace, then we've not found our
				// target, so we'll need to abort here.
				//

				Continue = false;
			}
			else
			{
				//
				// Otherwise, we'll examine the next flow on our list.
				//

				PC = FlowStack.top( );
				FlowStack.pop( );

				Script->SetInstructionPointer( PC );
			}

			//
			// In all cases, OP_RETN signifies a nonlinear control transfer, so
			// we'll skip the normal fall thorugh.
			//

			continue;

		case OP_JZ:  // Jump if zero
		case OP_JNZ: // Jump if not zero
		case OP_JMP: // Unconditional jump
			RelPC = (PROGRAM_COUNTER) Script->ReadINT32( );

			if (RelPC == 0)
				throw std::runtime_error( "trivial infinite loop detected" );

			//
			// If we have already traced this flow, then don't scan through it
			// again.
			//

			FlowPC = PC + RelPC;

			if (std::find(
				VisitedFlows.begin( ),
				VisitedFlows.end( ),
				FlowPC) != VisitedFlows.end( ))
			{
				if (Opcode == OP_JMP)
				{
					if (FlowStack.empty( ))
					{
						//
						// If we have more flows to trace, then we've not found
						// our target, so we'll need to abort here.
						//

						Continue = false;
					}
					else
					{
						//
						// Otherwise, we'll examine the next flow on our list.
						//

						PC = FlowStack.top( );
						FlowStack.pop( );

						Script->SetInstructionPointer( PC );
					}

					continue;
				}

				PC += Len;
				continue; // We've already consumed the operand from the istream
			}

			//
			// Mark the flow as visited and push it onto the list of flows to
			// follow.
			//

			VisitedFlows.push_back( FlowPC );

			if (Opcode != OP_JMP)
				FlowStack.push( FlowPC );

			if (Opcode != OP_JMP)
				PC += Len;
			else
				PC = FlowPC;

			Script->SetInstructionPointer( PC );
			continue;

		case OP_STORE_STATEALL: // Save a script situation state
		case OP_STORE_STATE:    // Save a script situation state
			RelPC = (PROGRAM_COUNTER) TypeOpcode;

			//
			// If we have already traced this flow, then don't scan through it
			// again.
			//

			FlowPC = PC + RelPC;

			if (std::find(
				VisitedFlows.begin( ),
				VisitedFlows.end( ),
				FlowPC) != VisitedFlows.end( ))
			{
				break;
			}

			//
			// Mark the flow as visited and push it onto the list of flows to
			// follow.
			//

			VisitedFlows.push_back( FlowPC );
			FlowStack.push( FlowPC );
			break;

		}

		//
		// If we fell through, then this was not a control transfer (jump), and
		// so the PC incremented linearly.  Account for this here.
		//

		Script->AdvanceInstructionPointer( Len - Offset );
		PC += Len;
	}

	//
	// The desired instruction wasn't found before we reached the end of all
	// flows; return failure.
	//

	return INVALID_PC;
}

bool
NWScriptAnalyzer::PrepareNewControlFlow(
	__inout AnalysisQueueEntry & Entry,
	__in NWNScriptLib::PROGRAM_COUNTER FlowPC,
	__out NWNScriptLib::ControlFlowPtr & TargetFlow,
	__out NWNScriptLib::NWScriptLabel & TargetLabel
	)
/*++

Routine Description:

	This routine returns a control flow descriptor for a given program counter.

	Typically, this routine would be invoked to acquire a flow descriptor for a
	branch target.

	If a prior flow matched, the pre-existing flow descriptor is returned (or
	split if necessary).  Otherwise, a completely new flow descriptor is
	created and returned.

Arguments:

	Entry - Supplies the analysis queue entry that is being processed.  The
	        current subroutine is updated with the new flow should one be
	        created.

	FlowPC - Supplies the starting PC to create (or return) a flow descriptor
	         for.

	TargetFlow - Receives the flow descriptor to use for the given PC.

	TargetLabel - Receives the label descriptor to use for the given PC.

Return Value:

	The routine returns a Boolean value indicating true if an entirely new
	control flow was created, else false if a pre-existing control flow was
	returned (or if a pre-existing control flow was split).

	On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	TargetFlow = Entry.Function->GetControlFlow(FlowPC);
	LabelVec &BranchTargets = Entry.Function->GetBranchTargets( );

	if (TargetFlow == NULL)
	{
		// Not yet examined
		TargetFlow = new ControlFlow( FlowPC, Entry.SP );
		TargetFlow->GetParents( ).insert( Entry.Flow.get( ) );

		Entry.Function->GetControlFlows( )[ FlowPC ] = TargetFlow;

		Label BranchTarget( FlowPC, Entry.SP, TargetFlow );
		BranchTargets.push_back( BranchTarget );
		TargetLabel = BranchTargets.back( );

		return true;
	}
	else if (TargetFlow->GetStartPC( ) == FlowPC)
	{
		//
		// We've already visited this flow
		//

		//
		// Check the consistency of SP at the other entry into this
		// flow.
		//

		if (TargetFlow->GetStartSP( ) != Entry.SP)
		{
			ThrowError(
				"mismatched stack on control flow at PC=%08X (SP=%08X, FlowSP=%08X)",
				FlowPC,
				Entry.SP,
				TargetFlow->GetStartSP( ));
		}

		TargetFlow->GetParents( ).insert( Entry.Flow.get( ) );

		for (LabelVec::iterator LabelIt = BranchTargets.begin( ); 
			LabelIt != BranchTargets.end( ); LabelIt++ )
		{
			if (LabelIt->GetAddress( ) == FlowPC)
				TargetLabel = *LabelIt;
		}
		
		return false;
	}
	else
	{
		//
		// Short stick: we have to split the existing flow. Allocate a new 
		// flow that will come after the previous flow.
		//
		// N.B. Because we save a pointer to the current flow in analysis 
		// queue entries, the new flow must come before the old flow, so as 
		// not to invalidate any analysis entries.

		AnalyzeDebug(
			"Splitting flow %X/%X-%X/%X due to branch to %X/%X\n",
			TargetFlow->GetStartPC( ),
			TargetFlow->GetStartSP( ),
			TargetFlow->GetEndPC( ),
			TargetFlow->GetEndSP( ),
			FlowPC,
			Entry.SP);

		ControlFlowPtr OldFlow = TargetFlow;
		ControlFlowPtr NewFlow = new ControlFlow( 
			OldFlow->GetStartPC( ), OldFlow->GetStartSP( ) );

		Entry.Function->GetControlFlows( )[ OldFlow->GetStartPC( ) ] = NewFlow;
		Entry.Function->GetControlFlows( )[ FlowPC ] = OldFlow;

		// Find the old label and switch it to point to the new flow
		for (LabelVec::iterator it = 
			Entry.Function->GetBranchTargets( ).begin( );
			it != Entry.Function->GetBranchTargets( ).end( ); it++)
		{
			if (it->GetAddress( ) == OldFlow->GetStartPC( ))
			{
				*it = Label( it->GetAddress( ), it->GetSP( ), 
					NewFlow, it->GetFlags( ) );
				break;
			}
		}

		Label BranchTarget( FlowPC, Entry.SP, TargetFlow );
		BranchTargets.push_back( BranchTarget );
		TargetLabel = BranchTargets.back( );

		//
		// The start of the target becomes the end of the new
		//

		NewFlow->SetEndPC( FlowPC );
		NewFlow->SetEndSP( Entry.SP );
		NewFlow->SetTerminationType( ControlFlow::Merge );
		NewFlow->SetChild( 0, TargetFlow );
		NewFlow->SetChild( 1, NULL );

		//
		// Update the existing parent links and child links of the parents
		//

		for (ControlFlowWeakPtrSet::iterator itParent = 
			OldFlow->GetParents( ).begin( );
			itParent != OldFlow->GetParents( ).end( ); itParent++)
		{
			for (size_t ChildIdx = 0; ChildIdx < 2; ChildIdx++)
			{
				ControlFlow * Parent = *itParent;
				if (Parent->GetChild( ChildIdx ).get( ) == OldFlow.get( ))
					Parent->SetChild( ChildIdx, NewFlow );
			}
		}
		NewFlow->GetParents( ).swap( OldFlow->GetParents( ) );

		//
		// The end of the new becomes the start of the old
		//

		OldFlow->SetStartPC( FlowPC );
		OldFlow->SetStartSP( Entry.SP );
		// Parents list has already been cleared by swap
		OldFlow->GetParents( ).insert( NewFlow.get( ) );
		OldFlow->GetParents( ).insert( Entry.Flow.get( ) );

		return false;
	}
}

bool
NWScriptAnalyzer::LoadNextAnalysisLabel(
	__inout AnalysisQueueEntry & Entry,
	__in NWScriptReader * Script
	)
/*++

Routine Description:

	This routine discovers the next analysis label to process from the code
	analysis queue.

Arguments:

	Entry - Supplies the current analysis queue entry being worked on.  On
	        successful return, the entry is updated to refer to the next item
	        available for processing in the queue.

Return Value:

	The routine returns a Boolean value indicating true if another analysis
	queue entry was pulled from the queue for processing, else false if there
	were no more analysis queue entries to process.

Environment:

	User mode.

--*/
{
	if (Entry.Function->GetAnalyzeBranches( ).empty( ))
	{
		//
		// If we have no more flows to trace, then we're finished.
		//

		return false;
	}
	else
	{
		//
		// Otherwise, we'll examine the next flow on our list.
		//

		Label Target = Entry.Function->GetAnalyzeBranches( ).back( );

		Entry.Function->GetAnalyzeBranches( ).pop_back( );

		Entry.PC          = Target.GetAddress( );
		Entry.SP          = Target.GetSP( );
		Entry.Flow        = Target.GetControlFlow( );
		//Entry.LabelFlags |= Target.GetFlags( ) & Label::SCRIPT_SITUATION;

		Script->SetInstructionPointer( Target.GetAddress( ) );

		AnalyzeDebug(
			"Tracing alternate flow path at PC=%08X SP=%08X.\n",
			Entry.PC,
			Entry.SP);

		return true;
	}
}

NWNScriptLib::ControlFlowPtr
NWScriptAnalyzer::FindSubseqControlFlow(
	__in AnalysisQueueEntry & Entry
	)
{
	if (Entry.Function == NULL)
		return NULL;

	ControlFlowSet &ControlFlows = Entry.Function->GetControlFlows( );
	ControlFlowSet::iterator it = 
		ControlFlows.upper_bound( Entry.PC );

	if (it != ControlFlows.end( ))
		return it->second;
	else
		return NULL;
}

NWACTION_TYPE
NWScriptAnalyzer::GetOperandType(
	__in UCHAR OperandType,
	__out_opt NWACTION_TYPE * SecondType
	)
/*++

Routine Description:

	This routine decodes an NWScript type opcode into its left-hand and right-
	hand fundamental type ordinals.

Arguments:

	OperandType - Supplies the NWScript type opcode to inspect.

	SecondType - Optionally receives the second (right-hand) type.

Return Value:

	The routine returns the left-hand type of the NWScript opcode.

	On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	NWACTION_TYPE LeftType = ACTIONTYPE_VOID,
		RightType = ACTIONTYPE_VOID;

	switch (OperandType)
	{
	case TYPE_BINARY_INTINT:
		RightType = ACTIONTYPE_INT;
	case TYPE_UNARY_INT:
		LeftType = ACTIONTYPE_INT;
		break;

	case TYPE_BINARY_FLOATFLOAT:
		RightType = ACTIONTYPE_FLOAT;
	case TYPE_UNARY_FLOAT:
		LeftType = ACTIONTYPE_FLOAT;
		break;

	case TYPE_BINARY_STRINGSTRING:
		RightType = ACTIONTYPE_STRING;
	case TYPE_UNARY_STRING:
		LeftType = ACTIONTYPE_STRING;
		break;

	case TYPE_BINARY_OBJECTIDOBJECTID:
		RightType = ACTIONTYPE_OBJECT;
	case TYPE_UNARY_OBJECTID:
		LeftType = ACTIONTYPE_OBJECT;
		break;

	// Conspicuously absent: TYPE_BINARY_STRUCTSTRUCT

	case TYPE_BINARY_INTFLOAT:
		LeftType = ACTIONTYPE_INT;
		RightType = ACTIONTYPE_FLOAT;
		break;

	case TYPE_BINARY_FLOATINT:
		LeftType = ACTIONTYPE_FLOAT;
		RightType = ACTIONTYPE_INT;
		break;

	case TYPE_BINARY_VECTORVECTOR:
		LeftType = RightType = ACTIONTYPE_VECTOR;
		break;

	case TYPE_BINARY_VECTORFLOAT:
		LeftType = ACTIONTYPE_VECTOR;
		RightType = ACTIONTYPE_FLOAT;
		break;

	case TYPE_BINARY_FLOATVECTOR:
		LeftType = ACTIONTYPE_FLOAT;
		RightType = ACTIONTYPE_VECTOR;
		break;

	default:
		if (OperandType >= TYPE_UNARY_ENGINE_FIRST && 
			OperandType <= TYPE_UNARY_ENGINE_LAST)
			LeftType = RightType = NWACTION_TYPE(OperandType - 
				TYPE_UNARY_ENGINE_FIRST + ACTIONTYPE_ENGINE_0);
		else if (OperandType >= TYPE_BINARY_ENGINE_FIRST &&
			OperandType <= TYPE_BINARY_ENGINE_LAST)
			LeftType = RightType = NWACTION_TYPE(OperandType - 
				TYPE_BINARY_ENGINE_FIRST + ACTIONTYPE_ENGINE_0);
		else
			throw std::runtime_error( "invalid operand type" );

	}

	if (SecondType)
		*SecondType = RightType;

	return LeftType;
}

void
NWScriptAnalyzer::CreateValidOpcodeTypeMap(
	)
{
	// The current set we're working on
	OpcodeTypeSet * Set;

	AddOpcodeTypesToMap( Set, OP_CPDOWNSP, { TYPE_UNARY_STACKOP } );

	CopyOpcodeTypes( OP_CPTOPSP, Set );
	CopyOpcodeTypes( OP_DESTRUCT, Set );
	CopyOpcodeTypes( OP_CPDOWNBP, Set );
	CopyOpcodeTypes( OP_CPTOPBP, Set );

	AddOpcodeTypesToMap( Set, OP_RSADD, 
		{ TYPE_UNARY_INT, 
		TYPE_UNARY_FLOAT, 
		TYPE_UNARY_STRING, 
		TYPE_UNARY_OBJECTID } );
	for (UCHAR i = TYPE_UNARY_ENGINE_FIRST; 
		i <= TYPE_UNARY_ENGINE_LAST; i++)
		Set->insert( i );

	AddOpcodeTypesToMap( Set, OP_CONST, 
		{ TYPE_UNARY_INT, 
		TYPE_UNARY_FLOAT, 
		TYPE_UNARY_STRING, 
		TYPE_UNARY_OBJECTID } );

	AddOpcodeTypesToMap( Set, OP_ACTION, { TYPE_UNARY_NONE } );

	CopyOpcodeTypes( OP_MOVSP, Set );
	CopyOpcodeTypes( OP_JMP, Set );
	CopyOpcodeTypes( OP_JSR, Set );
	CopyOpcodeTypes( OP_JZ, Set );
	CopyOpcodeTypes( OP_RETN, Set );
	CopyOpcodeTypes( OP_JNZ, Set );
	CopyOpcodeTypes( OP_SAVEBP, Set );
	CopyOpcodeTypes( OP_RESTOREBP, Set );
	CopyOpcodeTypes( OP_NOP, Set );

	AddOpcodeTypesToMap( Set, OP_LOGAND, { TYPE_BINARY_INTINT } );

	CopyOpcodeTypes( OP_LOGOR, Set );
	CopyOpcodeTypes( OP_INCOR, Set );
	CopyOpcodeTypes( OP_EXCOR, Set );
	CopyOpcodeTypes( OP_BOOLAND, Set );
	CopyOpcodeTypes( OP_SHLEFT, Set );
	CopyOpcodeTypes( OP_SHRIGHT, Set );
	CopyOpcodeTypes( OP_USHRIGHT, Set );
	CopyOpcodeTypes( OP_MOD, Set );

	AddOpcodeTypesToMap( Set, OP_EQUAL, {
		TYPE_BINARY_INTINT,
		TYPE_BINARY_FLOATFLOAT,
		TYPE_BINARY_STRINGSTRING,
		TYPE_BINARY_OBJECTIDOBJECTID,
		TYPE_BINARY_STRUCTSTRUCT } );
	for (UCHAR i = TYPE_BINARY_ENGINE_FIRST; 
		i <= TYPE_BINARY_ENGINE_LAST; i++)
		Set->insert( i );

	CopyOpcodeTypes( OP_NEQUAL, Set );

	AddOpcodeTypesToMap( Set, OP_GEQ, { 
		TYPE_BINARY_INTINT,
		TYPE_BINARY_FLOATFLOAT } );

	CopyOpcodeTypes( OP_GT, Set );
	CopyOpcodeTypes( OP_LT, Set );
	CopyOpcodeTypes( OP_LEQ, Set );

	AddOpcodeTypesToMap( Set, OP_SUB, {
		TYPE_BINARY_INTINT,
		TYPE_BINARY_INTFLOAT,
		TYPE_BINARY_FLOATINT,
		TYPE_BINARY_FLOATFLOAT,
		TYPE_BINARY_VECTORVECTOR } );

	CopyOpcodeTypes( OP_ADD, Set );
	AddOpcodeTypesToMap( Set, OP_ADD, { TYPE_BINARY_STRINGSTRING } );

	AddOpcodeTypesToMap( Set, OP_DIV, {
		TYPE_BINARY_INTINT,
		TYPE_BINARY_INTFLOAT,
		TYPE_BINARY_FLOATINT,
		TYPE_BINARY_FLOATFLOAT,
		TYPE_BINARY_VECTORFLOAT } );

	CopyOpcodeTypes( OP_MUL, Set );
	AddOpcodeTypesToMap( Set, OP_MUL, { TYPE_BINARY_FLOATVECTOR } );

	AddOpcodeTypesToMap( Set, OP_NEG, {
		TYPE_UNARY_INT,
		TYPE_UNARY_FLOAT } );

	AddOpcodeTypesToMap( Set, OP_COMP, { TYPE_UNARY_INT } );

	CopyOpcodeTypes( OP_NOT, Set );
	CopyOpcodeTypes( OP_DECISP, Set );
	CopyOpcodeTypes( OP_INCISP, Set );
	CopyOpcodeTypes( OP_DECIBP, Set );
	CopyOpcodeTypes( OP_INCIBP, Set );
}

NWScriptInstruction::INSTR
NWScriptAnalyzer::MapIROpcode(
	__in unsigned NWScriptOpcode
	)
/*++

Routine Description:

	This routine maps an NWScript instruction set opcode to an IR opcode.

Arguments:

	NWScriptOpcode - Supplies the NWScript instruction opcode to convert.

Return Value:

	The routine returns the IR encoding for the given NWScript instruction.

	On failure, an std::exception is raised.

Environment:

	User mode.

--*/
{
	switch (NWScriptOpcode)
	{
		case OP_CPDOWNSP:
		case OP_CPDOWNBP:
			return NWScriptInstruction::I_ASSIGN;
		case OP_RSADD:
			return NWScriptInstruction::I_CREATE;
		case OP_ACTION:
			return NWScriptInstruction::I_ACTION;
		case OP_LOGAND:
			return NWScriptInstruction::I_LOGAND;
		case OP_LOGOR:
			return NWScriptInstruction::I_LOGOR;
		case OP_INCOR:
			return NWScriptInstruction::I_INCOR;
		case OP_EXCOR:
			return NWScriptInstruction::I_EXCOR;
		case OP_BOOLAND:
			return NWScriptInstruction::I_BOOLAND;
		case OP_EQUAL:
			return NWScriptInstruction::I_EQUAL;
		case OP_NEQUAL:
			return NWScriptInstruction::I_NEQUAL;
		case OP_GEQ:
			return NWScriptInstruction::I_GEQ;
		case OP_GT:
			return NWScriptInstruction::I_GT;
		case OP_LT:
			return NWScriptInstruction::I_LT;
		case OP_LEQ:
			return NWScriptInstruction::I_LEQ;
		case OP_SHLEFT:
			return NWScriptInstruction::I_SHLEFT;
		case OP_SHRIGHT:
			return NWScriptInstruction::I_SHRIGHT;
		case OP_USHRIGHT:
			return NWScriptInstruction::I_USHRIGHT;
		case OP_ADD:
			return NWScriptInstruction::I_ADD;
		case OP_SUB:
			return NWScriptInstruction::I_SUB;
		case OP_MUL:
			return NWScriptInstruction::I_MUL;
		case OP_DIV:
			return NWScriptInstruction::I_DIV;
		case OP_MOD:
			return NWScriptInstruction::I_MOD;
		case OP_NEG:
			return NWScriptInstruction::I_NEG;
		case OP_COMP:
			return NWScriptInstruction::I_COMP;
		case OP_STORE_STATEALL:
		case OP_STORE_STATE:
			return NWScriptInstruction::I_SAVE_STATE;
		case OP_JMP:
			return NWScriptInstruction::I_JMP;
		case OP_JSR:
			return NWScriptInstruction::I_CALL;
		case OP_JZ:
			return NWScriptInstruction::I_JZ;
		case OP_RETN:
			return NWScriptInstruction::I_RETN;
		case OP_NOT:
			return NWScriptInstruction::I_NOT;
		case OP_DECISP:
		case OP_DECIBP:
			return NWScriptInstruction::I_DEC;
		case OP_INCISP:
		case OP_INCIBP:
			return NWScriptInstruction::I_INC;
		case OP_JNZ:
			return NWScriptInstruction::I_JNZ;
	}

	throw std::runtime_error( "Invalid opcode in MapIROpcode" );
}
