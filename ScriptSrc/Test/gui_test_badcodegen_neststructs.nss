/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	gui_test_badcodegen_neststructs.nss

Abstract:

	This module houses test code for a code generation and parsing issue
	in the OEI/Bioware script compiler.

--*/

struct leaf
{
	int leafmember;
};

struct nested
{
	struct leaf nestedstruct;
};

void
testfunc1(
	struct nested n
	)
/*++

Routine Description:

	This routine references a nested struct in a context that allows the
	invalid generated code to easily be inspected.

Arguments:

	n - Supplies the nested structure to reference.

Return Value:

	None.

Environment:

	Any script caller.

--*/
{
	//
	// The following fails to parse in the OEI/Bioware compiler:
	//
	// "ERROR: VOID EXPRESSION WHERE NON VOID REQUIRED"
	//
	 
	// PrintInteger(n.nestedstruct.leafmember);

	//
	// The following emits illegal code in the OEI/Bioware compiler:
	//
	// 'n.nestedstruct' is pushed and then popped from the stack,
	// but then emit a CPDOWNSP that assumes that 'n.nestedstruct'
	// was still at the top of the stack.
	//
	// If whatever local variable was at the top of the stack at
	// this time has a compatible type, then the invalid stack
	// reference goes undetected by the VM (however the
	// NWScriptAnalyzer may emit a
	// 'CPDOWNSP source/destination overlap' if the last local on
	// the stack was the on the lefth and side of the expression).
	//

	//
	// If 'local' is the last local declared before the reference,
	// then the NWScript VM does not detect the corruption.  The
	// NWScriptAnalyzer raises a 'CPDOWNSP source/destination
	// overlap' verifier error.
	//
	// The actual compiled program behavior with the buggy compiler
	// is to assign 'local' to itself with an overlapping memcpy that
	// may produce propagation.
	//
	
	struct leaf local;
	
	//
	// If 'local2' is the last local declared before the
	// reference, then the NWScript VM and the NWScriptAnalyzer
	// both raise a type mismatch error.
	//
	// The actual compiled program behavior is an unexecutable
	// program that violates type safety.
	//
	
	string local2;
	
	//
	// If 'local3' is the last local declared before the
	// reference, then both the NWScript VM and NWScriptAnalyzer
	// are unable to detect a structural deficiency with the
	// program.
	//
	// The actual compiled program behavior is to assign 'local3'
	// to 'local'.
	// 

	struct leaf local3;
	
	//
	// The following line causes bad code generation when the
	// OEI/Bioware script compiler is used.
	//
	// Depending on which of the previous line are uncommented,
	// a variety of illegal effective behaviors are produced.
	//

	local = n.nestedstruct;
}

void
main(
	)
/*++

Routine Description:

	This script is invoked to test bad code generation.

Arguments:

	None.

Return Value:

	None.

Environment:

	GUI script.

--*/
{
	struct nested n;

	testfunc1(n);
}

