/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	Hooks.cpp

Abstract:

	This module defines the hook code for the NWScript VM plugin.

--*/

#include "Precomp.h"
#include "NWNX4Plugin.h"
#include "NWScriptJITPolicy.h"
#include "ServerNWScript.h"
#include "Offsets.h"
#include "NWN2Def.h"
#include "hdlcommon.h"

using namespace NWN2Server;

BOOL
CVirtualMachine_StackPopCommandInternal_Hook(
	);

Patch _patches[ ] =
{
	Patch( OFFS_VM_ExecuteCode, "\xe9", 1 ),
	Patch( OFFS_VM_ExecuteCode+1, (relativefunc) CVirtualMachine::GetAddressOf_ExecuteCode( ) ),
	Patch( OFFS_VM_StackPopCommand_Internal, "\xe9", 1 ),
	Patch( OFFS_VM_StackPopCommand_Internal+1, (relativefunc) CVirtualMachine_StackPopCommandInternal_Hook ),
	Patch( OFFS_VMCheckParameterValidation, "\x90\x90", 2 ),

	Patch( )
};

Patch * patches = _patches;


SCRIPT_STATUS
__thiscall
CVirtualMachine::ExecuteCode(
	__inout int * PC,
	__inout_ecount( CodeSize ) unsigned char * InstructionStream,
	__in int CodeSize,
	__in struct CVirtualMachineDebuggingContext * DebugContext
	)
/*++

Routine Description:

	This routine is invoked when the server virtual machine attempts to execute
	a script.  Its purpose is to hand over control to the virtual machine
	replacement.

Arguments:

	PC - Supplies a pointer to the initial program counter for the script.  On
	     return, the final program counter is returned.

	InstructionStream - Supplies a pointer to the instruction stream to execute,
	                    representing the entire script.

	CodeSize - Supplies the length, in bytes, of the instruction stream.

	DebugContext - Supplies the script debugger context.

Return Value:

	On success, zero is returned.  On failure, a negative value that is a talk
	table reference is returned (i.e., a script VM error code).

Environment:

	User mode, invoked from NWN2Server logic.

--*/
{
	ServerNWScriptPlugin * Plugin;

	UNREFERENCED_PARAMETER( DebugContext );

	//
	// Hand the request off to the plugin for processing.
	//

	if ((Plugin = ServerNWScriptPlugin::GetPlugin( )) == NULL)
		return NWSCRIPT_ERR_FAKE_SHORTCUT_LOGICAL_OPERATION;

	if (!Plugin->ExecuteScriptForServer(
		PC,
		InstructionStream,
		CodeSize,
		this))
	{
		return NWSCRIPT_ERR_FAKE_ABORT_SCRIPT;
	}

	return 0;
}

extern void * NWN2_Heap_Deallocate;

__declspec( naked )
VOID
__cdecl
NWN2Server::FreeNwn2(
	__in PVOID P
	)
/*++

Routine Description:

	This routine releases memory to an NWN2 heap.  This memory must have come
	from an NWN2 heap allocator.

Arguments:

	P - Supplies the address of an NWN2 heap allocation.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	UNREFERENCED_PARAMETER( P );

#ifdef _M_IX86
	__asm
	{
		jmp     dword ptr [NWN2_Heap_Deallocate]
	}
#else
	if (P != NULL)
		HeapFree( GetProcessHeap( ), 0, P );
#endif
}

unsigned long
__fastcall
SaveStateToServerVMStack(
	__in NWN2Server::CVirtualMachine * ServerVM
	)
/*++

Routine Description:

	This routine is invoked to save the current saved state to the server's VM
	stack so that it may be packaged into a saved script situation.

Arguments:

	ServerVM - Supplies a pointer to the server's CVirtualMachine instance.

Return Value:

	The routine returns the number of elements placed on the VM stack.

Environment:

	User mode.

--*/
{
	ServerNWScriptPlugin * Plugin;

	//
	// Hand the request off to the plugin for processing.
	//

	if ((Plugin = ServerNWScriptPlugin::GetPlugin( )) == NULL)
		return 0;

	return Plugin->SaveStateToServerVMStack( ServerVM );
}

void
__fastcall
RemoveSavedStateFromServerVMStack(
	__in NWN2Server::CVirtualMachine * ServerVM,
	__in unsigned long StackElements
	)
/*++

Routine Description:

	This routine is invoked to remove the current saved state from the server's
	VM stack, so that execution may continue normally after the saved state has
	been captured.

Arguments:

	ServerVM - Supplies a pointer to the server's CVirtualMachine instance.

	StackElements - Supplies the number of elements that need to be removed from
	                the server's VM stack.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	ServerNWScriptPlugin * Plugin;

	//
	// Hand the request off to the plugin for processing.
	//

	if ((Plugin = ServerNWScriptPlugin::GetPlugin( )) == NULL)
		return;

	return Plugin->RemoveSavedStateFromServerVMStack( ServerVM, StackElements );
}

__declspec( naked )
BOOL
CVirtualMachine_StackPopCommandInternal_Hook(
	)
/*++

Routine Description:

	This routine is invoked when the server attempts to retrieve the current
	saved state from the VM instance.  Its purpose is to push the contents of
	the current execution environment state onto the VM stack, call the original
	implementation, and then remove the saved state from the VM stack.

Arguments:

	ecx - Supplies the CVirtualMachine instance.

	[esp+4] - Receives a pointer to a CVirtualMachineScript instance that
	          contains the saved script situation state.

Return Value:

	The routine returns TRUE on success, else FALSE on failure.

Environment:

	User mode.

--*/
{
	enum
	{
		//
		// Locals (negative offset).
		//

		SPCmd_This = 0x04,
		SPCmd_Elms = 0x08,
		SPCmd_RetC = 0x0C,

		//
		// Arguments (positive offset).
		//

		SPCmd_SSit = 0x08,

		//
		// Total local variable space required.
		//

		LocalSize  = 0x0C
	};

	__asm
	{
		push    ebp                        ; establish stack frame
		mov     ebp, esp                   ;
		sub     esp, LocalSize             ;
		mov     [ebp-SPCmd_This], ecx      ; save CVirtualMachine this pointer

		call    SaveStateToServerVMStack   ; place saved state state on VM stack
		mov     ecx, [ebp-SPCmd_This]      ; reload this pointer
		mov     [ebp-SPCmd_Elms], eax      ; save # of elements saved to stack

		push    [ebp+SPCmd_SSit]           ; script situation pointer
		call    RunStackPopCommand_Internal ; call VM to copy stack contents
		mov     [ebp-SPCmd_RetC], eax      ; save return code

		mov     edx, [ebp-SPCmd_Elms]      ; get # of elements saved to stack
		mov     ecx, [ebp-SPCmd_This]      ; reload this pointer
		call    RemoveSavedStateFromServerVMStack ;

		mov     eax, [ebp-SPCmd_RetC]      ; reload return code
		mov     esp, ebp                   ; de-establish stack frame
		pop     ebp                        ;
		ret     04h                        ; return, one thiscall argument cleaned

RunStackPopCommand_Internal:
		push    ebx                        ; run prologue code we patched over
		push    ebp                        ;
		push    esi                        ;
		push    edi                        ;
		push    28h                        ;
		mov     eax, OFFS_VM_StackPopCommand_Internal+6 ;
		jmp     eax                        ; rejoin server implementation

	}
}

__declspec( naked )
BOOL
CVirtualMachineDebugLoader::DemandDebugInfo(
	__in CExoString * FileName
	)
/*++

Routine Description:

	This routine loads debug symbols for a script.

Arguments:

	ecx - Supplies the CVirtualMachineDebugLoader instance.

	[esp+4] - Supplies a pointer to the file name of the script.

Return Value:

	The routine returns TRUE on success, else FALSE on failure.

Environment:

	User mode.

--*/
{
	UNREFERENCED_PARAMETER( FileName );

	__asm
	{
#if OFFS_VMDbgLdr_DemandDebugInfo
		mov     eax, OFFS_VMDbgLdr_DemandDebugInfo ;
		jmp     eax                        ; jump to implementation
#else
		xor     eax, eax                   ; clear return value
		ret     04h                        ; return, stdcall, one argument
#endif
	}
}

__declspec( naked )
BOOL
CVirtualMachineDebugLoader::ReleaseDebugInfo(
	)
/*++

Routine Description:

	This routine unloads debug symbols for a script.

Arguments:

	ecx - Supplies the CVirtualMachineDebugLoader instance.

Return Value:

	The routine returns TRUE on success, else FALSE on failure.

Environment:

	User mode.

--*/
{
	__asm
	{
#if OFFS_VMDbgLdr_ReleaseDebugInfo
		mov     eax, OFFS_VMDbgLdr_ReleaseDebugInfo ;
		jmp     eax                        ; jump to implementation
#else
		xor     eax, eax                   ; clear return value
		ret                                ; return, stdcall, zero arguments
#endif
	}
}
