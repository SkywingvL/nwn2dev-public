/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    Offsets.h

Abstract:

    This module supplies the virtual address offsets that are used to patch the
	game server.  It abstracts these differences away from the main game server
	extension logic, so that in general, only changes to this module need be
	made to support a new game patch revision level.

--*/

#ifndef _PROGRAMS_AURORASERVERNWSCRIPT_OFFSETS_H
#define _PROGRAMS_AURORASERVERNWSCRIPT_OFFSETS_H

#ifdef _MSC_VER
#pragma once
#endif

/*
 * May be applied as a delta to all of the following for relocation if such
 * becomes necessary.
 */

#define NWN2_BASE                       (0x00400000)

#include "../ProjectGlobal/VersionConstants.h"

/*
 * Version specific offset list.
 */

#if NWN2MAIN_VERSION == 0x01231765

/*
 * nwn2server.exe 1.0.23.1765  English
 */

// #define OFFS_VMCmd_ExecuteCommand     (0x0067C8A0) // 1.23.1765: 0x0067C8A0 CNWVirtualMachineCommands::ExecuteCommand
#define OFFS_VM_ExecuteCode              (0x00728990) // 1.23.1765: 0x00728990 CVirtualMachine::ExecuteCode
#define OFFS_VM_StackPopCommand_Internal (0x007281E0) // 1.23.1765: 0x007281E0 CVirtualMachine::StackPopCommand_Internal

#define OFFS_VM_StackPopInteger         (0x0067CFD0) // 1.23.1763: 0x0067CFD0 CVirtualMachine::StackPopInteger
#define OFFS_VM_StackPopFloat           (0x0067D030) // 1.23.1765: 0x0067D030 CVirtualMachine::StackPopFloat
#define OFFS_VM_StackPopString          (0x0067D190) // 1.23.1765: 0x0067D190 CVirtualMachine::StackPopString
#define OFFS_VM_StackPopVector          (0x0067D090) // 1.23.1765: 0x0067D090 CVirtualMachine::StackPopVector
#define OFFS_VM_StackPopObject          (0x0067D2F0) // 1.23.1765: 0x0067D2F0 CVirtualMachine::StackPopObject
#define OFFS_VM_StackPopEngineStructure (0x0067D240) // 1.23.1765: 0x0067D240 CVirtualMachine::StackPopEngineStructure

#define OFFS_VM_StackPushInteger        (0x0067D000) // 1.23.1765: 0x0067D000 CVirtualMachine::StackPushInteger
#define OFFS_VM_StackPushFloat          (0x0067D060) // 1.23.1765: 0x0067D060 CVirtualMachine::StackPushFloat
#define OFFS_VM_StackPushString         (0x0067D1E0) // 1.23.1765: 0x0067D1E0 CVirtualMachine::StackPushString
#define OFFS_VM_StackPushVector         (0x0067D110) // 1.23.1765: 0x0067D110 CVirtualMachine::StackPushVector
#define OFFS_VM_StackPushObject         (0x0067D320) // 1.23.1765: 0x0067D320 CVirtualMachine::StackPushObject
#define OFFS_VM_StackPushEngineStructure (0x0067D2A0) // 1.23.1765: 0x0067D2A0 CVirtualMachine::StackPushEngineStructure

//
// Define offsets used for the run script export API.
//

#define OFFS_VM_RunScript               (0x0072B050) // 1.23.1765: 0x0072B050 CVirtualMachine::RunScript

//
// Debug symbols support, optional.
//

#define OFFS_VMDbgLdr_DemandDebugInfo   (0x00734320) // 1.23.1765: 0x00734320 CVirtualMachineDebugLoader::DemandDebugInfo
#define OFFS_VMDbgLdr_ReleaseDebugInfo  (0x00734400) // 1.23.1765: 0x00734400 CVirtualMachineDebugLoader::ReleaseDebugInfo

//
// Parameter checking patch, optional (disables server-side script parse).
//

#define OFFS_VMCheckParameterValidation (0x0072AC2D) // 1.23.1765: 0x0072AC2D CVirtualMachine::CheckParameterValidation+0xd

//
// Check data, *(PULONG) OFFS_VM_ExecuteCode should be CHECK_VM_ExecuteCode.
//

#define CHECK_VM_ExecuteCode            (0x015CEC81)


#else

#error Not a supported NWN2MAIN version!

#endif

#endif


