/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWScriptDataTables.h

Abstract:

	This module defines the data tables that are utilized by the NWScript
	analyzer system.  The built-in data tables support the canonical script
	API.  The user may provide custom data tables if so desired.

--*/

#include "Precomp.h"
#include "NWScriptVM.h"
#include "NWScriptStack.h"
#include "NWScriptInternal.h"
#include "NWScriptInterfaces.h"
#include "NWScriptAnalyzer.h"


//
// Adjust these macros to remove name and/or prototype from the table.
//

/*
#define NWSCRIPT_ACTIONNAME(TEXT) TEXT,
#define NWSCRIPT_ACTIONPROTOTYPE(TEXT) TEXT,
#define NWSCRIPT_ACTIONPARAMETERSIZES(TEXT) , TEXT 
*/

#if NWACTION_DEF_INCLUDE_NAME
#define NWSCRIPT_ACTIONNAME(TEXT) TEXT,
#else
#define NWSCRIPT_ACTIONNAME(TEXT) 
#endif

#if NWACTION_DEF_INCLUDE_PROTOTYPE
#define NWSCRIPT_ACTIONPROTOTYPE(TEXT) TEXT,
#else
#define NWSCRIPT_ACTIONPROTOTYPE(TEXT) 
#endif

#define NWSCRIPT_ACTIONPARAMETERSIZES(TEXT) 

//
// Define the NWN2 data tables for nwscript.nss.
//


static const NWACTION_TYPE NWN2_NWActionParameterTypes_Random[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_PrintString[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_PrintFloat[ 3 ] = { ACTIONTYPE_FLOAT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_FloatToString[ 3 ] = { ACTIONTYPE_FLOAT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_PrintInteger[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_PrintObject[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AssignCommand[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_ACTION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DelayCommand[ 2 ] = { ACTIONTYPE_FLOAT, ACTIONTYPE_ACTION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ExecuteScript[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ClearAllActions[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetFacing[ 2 ] = { ACTIONTYPE_FLOAT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCalendar[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetTime[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetCalendarYear = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetCalendarMonth = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetCalendarDay = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetTimeHour = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetTimeMinute = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetTimeSecond = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetTimeMillisecond = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_ActionRandomWalk = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionMoveToLocation[ 2 ] = { ACTIONTYPE_LOCATION, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionMoveToObject[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionMoveAwayFromObject[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetArea[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetEnteringObject = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetExitingObject = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetPosition[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFacing[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemPossessor[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemPossessedBy[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_CreateItemOnObject[ 5 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionEquipItem[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionUnequipItem[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionPickUpItem[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionPutDownItem[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLastAttacker[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionAttack[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNearestCreature[ 8 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionSpeakString[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionPlayAnimation[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetDistanceToObject[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsObjectValid[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionOpenDoor[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionCloseDoor[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCameraFacing[ 4 ] = { ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_PlaySound[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetSpellTargetObject = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionCastSpellAtObject[ 7 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCurrentHitPoints[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetMaxHitPoints[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLocalInt[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLocalFloat[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLocalString[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLocalObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetLocalInt[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetLocalFloat[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetLocalString[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetLocalObject[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetStringLength[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetStringUpperCase[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetStringLowerCase[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetStringRight[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetStringLeft[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_InsertString[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetSubString[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_FindSubString[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_fabs[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_cos[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_sin[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_tan[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_acos[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_asin[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_atan[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_log[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_pow[ 2 ] = { ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_sqrt[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_abs[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectHeal[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectDamage[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectAbilityIncrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectDamageResistance[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectResurrection = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectSummonCreature[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCasterLevel[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFirstEffect[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNextEffect[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_RemoveEffect[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsEffectValid[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetEffectDurationType[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetEffectSubType[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetEffectCreator[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_IntToString[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFirstObjectInArea[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNextObjectInArea[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_d2[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_d3[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_d4[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_d6[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_d8[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_d10[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_d12[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_d20[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_d100[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_VectorMagnitude[ 1 ] = { ACTIONTYPE_VECTOR };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetMetaMagicFeat = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetObjectType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetRacialType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_FortitudeSave[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ReflexSave[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_WillSave[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetSpellSaveDC = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_MagicalEffect[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SupernaturalEffect[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ExtraordinaryEffect[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectACIncrease[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAC[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectSavingThrowIncrease[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectAttackIncrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectDamageReduction[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectDamageIncrease[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_RoundsToSeconds[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_HoursToSeconds[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_TurnsToSeconds[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLawChaosValue[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetGoodEvilValue[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAlignmentLawChaos[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAlignmentGoodEvil[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFirstObjectInShape[ 6 ] = { ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_VECTOR };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNextObjectInShape[ 6 ] = { ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_VECTOR };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectEntangle = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SignalEvent[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_EVENT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EventUserDefined[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectDeath[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectKnockdown = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionGiveItem[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionTakeItem[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_VectorNormalize[ 1 ] = { ACTIONTYPE_VECTOR };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectCurse[ 6 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAbilityScore[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsDead[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_PrintVector[ 2 ] = { ACTIONTYPE_VECTOR, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_Vector[ 3 ] = { ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetFacingPoint[ 2 ] = { ACTIONTYPE_VECTOR, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AngleToVector[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_VectorToAngle[ 1 ] = { ACTIONTYPE_VECTOR };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_TouchAttackMelee[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_TouchAttackRanged[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectParalyze[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectSpellImmunity[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectDeaf = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetDistanceBetween[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetLocalLocation[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLocalLocation[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectSleep = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemInSlot[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectCharmed = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectConfused = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectFrightened = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectDominated = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectDazed = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectStunned = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCommandable[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCommandable[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectRegenerate[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectMovementSpeedIncrease[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetHitDice[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionForceFollowObject[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ResistSpell[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetEffectType[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectAreaOfEffect[ 5 ] = { ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFactionEqual[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ChangeFaction[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsListening[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetListening[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetListenPattern[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_TestStringAgainstPattern[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetMatchedSubstring[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetMatchedSubstringsCount = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectVisualEffect[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFactionWeakestMember[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFactionStrongestMember[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFactionMostDamagedMember[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFactionLeastDamagedMember[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFactionGold[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFactionAverageReputation[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFactionAverageGoodEvilAlignment[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFactionAverageLawChaosAlignment[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFactionAverageLevel[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFactionAverageXP[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFactionMostFrequentClass[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFactionWorstAC[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFactionBestAC[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionSit[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetListenPatternNumber = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionJumpToObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetWaypointByTag[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTransitionTarget[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectLinkEffects[ 2 ] = { ACTIONTYPE_EFFECT, ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetObjectByTag[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AdjustAlignment[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionWait[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetAreaTransitionBMP[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionStartConversation[ 6 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_ActionPauseConversation = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_ActionResumeConversation = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectBeam[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetReputation[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AdjustReputation[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetSittingCreature[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetGoingToBeAttackedBy[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectSpellResistanceIncrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLocation[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionJumpToLocation[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_Location[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_VECTOR, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ApplyEffectAtLocation[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_EFFECT, ACTIONTYPE_LOCATION, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsPC[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_FeetToMeters[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_YardsToMeters[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ApplyEffectToObject[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_EFFECT, ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SpeakString[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetSpellTargetLocation = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetPositionFromLocation[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAreaFromLocation[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFacingFromLocation[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNearestCreatureToLocation[ 8 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNearestObject[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNearestObjectToLocation[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_LOCATION, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNearestObjectByTag[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_IntToFloat[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_FloatToInt[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_StringToInt[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_StringToFloat[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionCastSpellAtLocation[ 7 ] = { ACTIONTYPE_INT, ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsEnemy[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsFriend[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsNeutral[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetPCSpeaker = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetStringByStrRef[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionSpeakStringByStrRef[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DestroyObject[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetModule = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_CreateObject[ 5 ] = { ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EventSpellCastAt[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastSpellCaster = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastSpell = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetUserDefinedEventNumber = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetSpellId = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_RandomName = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectPoison[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectDisease[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectSilence = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetName[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastSpeaker = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_BeginConversation[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastPerceived = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastPerceptionHeard = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastPerceptionInaudible = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastPerceptionSeen = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastClosedBy = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastPerceptionVanished = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFirstInPersistentObject[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNextInPersistentObject[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAreaOfEffectCreator[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DeleteLocalInt[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DeleteLocalFloat[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DeleteLocalString[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DeleteLocalObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DeleteLocalLocation[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectHaste = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectSlow = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ObjectToString[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectImmunity[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsImmune[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectDamageImmunityIncrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetEncounterActive[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetEncounterActive[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetEncounterSpawnsMax[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetEncounterSpawnsMax[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetEncounterSpawnsCurrent[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetEncounterSpawnsCurrent[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetModuleItemAcquired = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetModuleItemAcquiredFrom = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCustomToken[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetHasFeat[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetHasSkill[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionUseFeat[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionUseSkill[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetObjectSeen[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetObjectHeard[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastPlayerDied = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetModuleItemLost = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetModuleItemLostBy = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionDoCommand[ 1 ] = { ACTIONTYPE_ACTION };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EventConversation = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetEncounterDifficulty[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetEncounterDifficulty[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetDistanceBetweenLocations[ 2 ] = { ACTIONTYPE_LOCATION, ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetReflexAdjustedDamage[ 5 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_PlayAnimation[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_TalentSpell[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_TalentFeat[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_TalentSkill[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetHasSpellEffect[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetEffectSpellId[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCreatureHasTalent[ 2 ] = { ACTIONTYPE_TALENT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCreatureTalentRandom[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCreatureTalentBest[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionUseTalentOnObject[ 2 ] = { ACTIONTYPE_TALENT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionUseTalentAtLocation[ 2 ] = { ACTIONTYPE_TALENT, ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetGoldPieceValue[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsPlayableRacialType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_JumpToLocation[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectTemporaryHitpoints[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetSkillRank[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAttackTarget[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLastAttackType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLastAttackMode[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetMaster[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsInCombat[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLastAssociateCommand[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GiveGoldToCreature[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetIsDestroyable[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetLocked[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLocked[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetClickingObject = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetAssociateListenPatterns[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLastWeaponUsed[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionInteractObject[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastUsedBy = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAbilityModifier[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIdentified[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetIdentified[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SummonAnimalCompanion[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SummonFamiliar[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetBlockingDoor = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsDoorActionPossible[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DoDoorAction[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFirstItemInInventory[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNextItemInInventory[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetClassByPosition[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLevelByPosition[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLevelByClass[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetDamageDealtByType[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetTotalDamageDealt = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLastDamager[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastDisarmed = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastDisturbed = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastLocked = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastUnlocked = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectSkillIncrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetInventoryDisturbType = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetInventoryDisturbItem = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetHenchman[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_VersusAlignmentEffect[ 3 ] = { ACTIONTYPE_EFFECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_VersusRacialTypeEffect[ 2 ] = { ACTIONTYPE_EFFECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_VersusTrapEffect[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetGender[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsTalentValid[ 1 ] = { ACTIONTYPE_TALENT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionMoveAwayFromLocation[ 3 ] = { ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetAttemptedAttackTarget = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTypeFromTalent[ 1 ] = { ACTIONTYPE_TALENT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIdFromTalent[ 1 ] = { ACTIONTYPE_TALENT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAssociate[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AddHenchman[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_RemoveHenchman[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AddJournalQuestEntry[ 6 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_RemoveJournalQuestEntry[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetPCPublicCDKey[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetPCIPAddress[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetPCPlayerName[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetPCLike[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetPCDislike[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SendMessageToPC[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetAttemptedSpellTarget = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastOpenedBy = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetHasSpell[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_OpenStore[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectTurned = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFirstFactionMember[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNextFactionMember[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionForceMoveToLocation[ 3 ] = { ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionForceMoveToObject[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetJournalQuestExperience[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_JumpToObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetMapPinEnabled[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectHitPointChangeWhenDying[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_PopUpGUIPanel[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ClearPersonalReputation[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetIsTemporaryFriend[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetIsTemporaryEnemy[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetIsTemporaryNeutral[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GiveXPToCreature[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetXP[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetXP[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_IntToHexString[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetBaseItemType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemHasItemProperty[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionEquipMostDamagingMelee[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionEquipMostDamagingRanged[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemACValue[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionRest[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ExploreAreaForPlayer[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_ActionEquipMostEffectiveArmor = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetIsDay = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetIsNight = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetIsDawn = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetIsDusk = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsEncounterCreature[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastPlayerDying = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetStartingLocation = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ChangeToStandardFaction[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SoundObjectPlay[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SoundObjectStop[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SoundObjectSetVolume[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SoundObjectSetPosition[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_VECTOR };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SpeakOneLinerConversation[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetGold[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastRespawnButtonPresser = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsDM[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_PlayVoiceChat[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsWeaponEffective[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastSpellHarmful = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EventActivateItem[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_LOCATION, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_MusicBackgroundPlay[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_MusicBackgroundStop[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_MusicBackgroundSetDelay[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_MusicBackgroundChangeDay[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_MusicBackgroundChangeNight[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_MusicBattlePlay[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_MusicBattleStop[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_MusicBattleChange[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AmbientSoundPlay[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AmbientSoundStop[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AmbientSoundChangeDay[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AmbientSoundChangeNight[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastKiller = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetSpellCastItem = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetItemActivated = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetItemActivator = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetItemActivatedTargetLocation = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetItemActivatedTarget = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsOpen[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_TakeGoldFromCreature[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_IsInConversation[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectAbilityDecrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectAttackDecrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectDamageDecrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectDamageImmunityDecrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectACDecrease[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectMovementSpeedDecrease[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectSavingThrowDecrease[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectSkillDecrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectSpellResistanceDecrease[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetPlotFlag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetPlotFlag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectInvisibility[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectConcealment[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectDarkness = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectDispelMagicAll[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_ACTION };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectUltravision = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectNegativeLevel[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectPolymorph[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectSanctuary[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectTrueSeeing = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectSeeInvisible = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectTimeStop = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectBlindness = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsReactionTypeFriendly[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsReactionTypeNeutral[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsReactionTypeHostile[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectSpellLevelAbsorption[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectDispelMagicBest[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_ACTION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActivatePortal[ 5 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNumStackedItems[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_SurrenderToEnemies = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectMissChance[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTurnResistanceHD[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCreatureSize[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectDisappearAppear[ 2 ] = { ACTIONTYPE_LOCATION, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectDisappear[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectAppear[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionUnlockObject[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionLockObject[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectModifyAttacks[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLastTrapDetected[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectDamageShield[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNearestTrapToObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetDeity[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetSubRace[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFortitudeSavingThrow[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetWillSavingThrow[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetReflexSavingThrow[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetChallengeRating[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAge[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetMovementRate[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAnimalCompanionCreatureType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFamiliarCreatureType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAnimalCompanionName[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFamiliarName[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionCastFakeSpellAtObject[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionCastFakeSpellAtLocation[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_LOCATION, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_RemoveSummonedAssociate[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCameraMode[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsResting[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastPCRested = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetWeather[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastRestEventType = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_StartNewModule[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectSwarm[ 5 ] = { ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetWeaponRanged[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_DoSinglePlayerAutoSave = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetGameDifficulty = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetTileMainLightColor[ 3 ] = { ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetTileSourceLightColor[ 3 ] = { ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_RecomputeStaticLighting[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTileMainLight1Color[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTileMainLight2Color[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTileSourceLight1Color[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTileSourceLight2Color[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetPanelButtonFlash[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCurrentAction[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetStandardFactionReputation[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetStandardFactionReputation[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_FloatingTextStrRefOnCreature[ 8 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_VECTOR };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_FloatingTextStringOnCreature[ 8 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_VECTOR };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTrapDisarmable[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTrapDetectable[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTrapDetectedBy[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTrapFlagged[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTrapBaseType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTrapOneShot[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTrapCreator[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTrapKeyTag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTrapDisarmDC[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTrapDetectDC[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLockKeyRequired[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLockKeyTag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLockLockable[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLockUnlockDC[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLockLockDC[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetPCLevellingUp = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetHasFeatEffect[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetPlaceableIllumination[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetPlaceableIllumination[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsPlaceableObjectActionPossible[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DoPlaceableObjectAction[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFirstPC[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNextPC[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetTrapDetectedBy[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsTrapped[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectTurnResistanceDecrease[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectTurnResistanceIncrease[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_PopUpDeathGUIPanel[ 5 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetTrapDisabled[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLastHostileActor[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_ExportAllCharacters = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_MusicBackgroundGetDayTrack[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_MusicBackgroundGetNightTrack[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_WriteTimestampedLogEntry[ 1 ] = { ACTIONTYPE_STRING };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetModuleName = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFactionLeader[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SendMessageToAllDMs[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EndGame[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_BootPC[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionCounterSpell[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AmbientSoundSetDayVolume[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AmbientSoundSetNightVolume[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_MusicBackgroundGetBattleTrack[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetHasInventory[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetStrRefSoundDuration[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AddToParty[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_RemoveFromParty[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetStealthMode[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetDetectMode[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetDefensiveCastingMode[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAppearanceType[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_SpawnScriptDebugger = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetModuleItemAcquiredStackSize = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DecrementRemainingFeatUses[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DecrementRemainingSpellUses[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetResRef[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectPetrify = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_CopyItem[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectCutsceneParalyze = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetDroppableFlag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetUseableFlag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetStolenFlag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCampaignFloat[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_FLOAT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCampaignInt[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCampaignVector[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_VECTOR, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCampaignLocation[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_LOCATION, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCampaignString[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DestroyCampaignDatabase[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCampaignFloat[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCampaignInt[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCampaignVector[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCampaignLocation[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCampaignString[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_CopyObject[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_LOCATION, ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DeleteCampaignVariable[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_StoreCampaignObject[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_RetrieveCampaignObject[ 5 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_LOCATION, ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectCutsceneDominated = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemStackSize[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetItemStackSize[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemCharges[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetItemCharges[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AddItemProperty[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_ITEMPROPERTY, ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_RemoveItemProperty[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsItemPropertyValid[ 1 ] = { ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFirstItemProperty[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNextItemProperty[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemPropertyType[ 1 ] = { ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemPropertyDurationType[ 1 ] = { ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyAbilityBonus[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyACBonus[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyACBonusVsAlign[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyACBonusVsDmgType[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyACBonusVsRace[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyACBonusVsSAlign[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyEnhancementBonus[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyEnhancementBonusVsAlign[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyEnhancementBonusVsRace[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyEnhancementBonusVsSAlign[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyEnhancementPenalty[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyWeightReduction[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyBonusFeat[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyBonusLevelSpell[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyCastSpell[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyDamageBonus[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyDamageBonusVsAlign[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyDamageBonusVsRace[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyDamageBonusVsSAlign[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyDamageImmunity[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyDamagePenalty[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyDamageReduction[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyDamageResistance[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyDamageVulnerability[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyDarkvision = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyDecreaseAbility[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyDecreaseAC[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyDecreaseSkill[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyContainerReducedWeight[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyExtraMeleeDamageType[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyExtraRangeDamageType[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyHaste = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyHolyAvenger = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyImmunityMisc[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyImprovedEvasion = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyBonusSpellResistance[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyBonusSavingThrowVsX[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyBonusSavingThrow[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyKeen = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyLight[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyMaxRangeStrengthMod[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyNoDamage = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyOnHitProps[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyReducedSavingThrowVsX[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyReducedSavingThrow[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyRegeneration[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertySkillBonus[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertySpellImmunitySpecific[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertySpellImmunitySchool[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyThievesTools[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyAttackBonus[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyAttackBonusVsAlign[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyAttackBonusVsRace[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyAttackBonusVsSAlign[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyAttackPenalty[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyUnlimitedAmmo[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyLimitUseByAlign[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyLimitUseByClass[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyLimitUseByRace[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyLimitUseBySAlign[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyBonusHitpoints[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyVampiricRegeneration[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyTrap[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyTrueSeeing = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyOnMonsterHitProperties[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyTurnResistance[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyMassiveCritical[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyFreeAction = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyMonsterDamage[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyImmunityToSpellLevel[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertySpecialWalk[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyHealersKit[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyWeightIncrease[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsSkillSuccessful[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectSpellFailure[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SpeakStringByStrRef[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCutsceneMode[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastPCToCancelCutscene = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetDialogSoundLength[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_FadeFromBlack[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_FadeToBlack[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_StopFade[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_BlackScreen[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetBaseAttackBonus[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetImmortal[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_OpenInventory[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_StoreCameraFacing = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_RestoreCameraFacing = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_LevelUpHenchman[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetDroppableFlag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetWeight[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetModuleItemAcquiredBy = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetImmortal[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DoWhirlwindAttack[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_Get2DAString[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectEthereal = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAILevel[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetAILevel[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsPossessedFamiliar[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_UnpossessFamiliar[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsAreaInterior[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SendMessageToPCByStrRef[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_IncrementRemainingFeatUses[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ExportSingleCharacter[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_PlaySoundByStrRef[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetSubRace[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetDeity[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsDMPossessed[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetWeather[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsAreaNatural[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsAreaAboveGround[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetPCItemLastEquipped = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetPCItemLastEquippedBy = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetPCItemLastUnequipped = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetPCItemLastUnequippedBy = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_CopyItemAndModify[ 5 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemAppearance[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyOnHitCastSpell[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemPropertySubType[ 1 ] = { ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetActionMode[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetActionMode[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetArcaneSpellFailure[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionExamine[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyVisualEffect[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetLootable[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLootable[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCutsceneCameraMoveRate[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCutsceneCameraMoveRate[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemCursedFlag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetItemCursedFlag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetMaxHenchmen[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetMaxHenchmen = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAssociateType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetSpellResistance[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DayToNight[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_NightToDay[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_LineOfSightObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_LineOfSightVector[ 2 ] = { ACTIONTYPE_VECTOR, ACTIONTYPE_VECTOR };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLastSpellCastClass = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetBaseAttackBonus[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_RestoreBaseAttackBonus[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectCutsceneGhost = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ItemPropertyArcaneSpellFailure[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetStoreGold[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetStoreGold[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetStoreMaxBuyPrice[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetStoreMaxBuyPrice[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetStoreIdentifyCost[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetStoreIdentifyCost[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCreatureAppearanceType[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCreatureStartingPackage[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectCutsceneImmobilize = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsInSubArea[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemPropertyCostTable[ 1 ] = { ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemPropertyCostTableValue[ 1 ] = { ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemPropertyParam1[ 1 ] = { ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemPropertyParam1Value[ 1 ] = { ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsCreatureDisarmable[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetStolenFlag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ForceRest[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCameraHeight[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetGlobalInt[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetGlobalBool[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetGlobalString[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetGlobalFloat[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetGlobalInt[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetGlobalBool[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetGlobalString[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetGlobalFloat[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SaveGlobalVariables[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_LoadGlobalVariables[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_MountObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DismountObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetJournalEntry[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectNWN2ParticleEffect = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectNWN2ParticleEffectFile[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectNWN2SpecialEffectFile[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT, ACTIONTYPE_VECTOR };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetSpellLevel[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_RemoveSEFFromObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ActionPauseCutscene[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AssignCutsceneActionToObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_ACTION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCharBackground[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetTrapActive[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetOrientOnDialog[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectDetectUndead = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectLowLightVision = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectSetScale[ 3 ] = { ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectShareDamage[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectAssayResistance[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectSeeTrueHPs = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetNumCutsceneActionsPending = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectDamageOverTime[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectAbsorbDamage[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectHideousBlow[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectMesmerize[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetSpellFeatId = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetFog[ 6 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectDarkVision = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DebugPostString[ 6 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetHasAnySpellEffect[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectArmorCheckPenaltyIncrease[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectDisintegrate[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectHealOnZeroHP[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectBreakEnchantment[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetFirstEnteringPC = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetNextEnteringPC = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AddRosterMemberByTemplate[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AddRosterMemberByCharacter[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_RemoveRosterMember[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsRosterMemberAvailable[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsRosterMemberSelectable[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetIsRosterMemberSelectable[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetObjectFromRosterName[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetRosterNameFromObject[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SpawnRosterMember[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DespawnRosterMember[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AddRosterMemberToParty[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_RemoveRosterMemberFromParty[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetFirstRosterMember = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetNextRosterMember = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SpawnSpellProjectile[ 6 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_LOCATION, ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SpawnItemProjectile[ 8 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_LOCATION, ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsOwnedByPlayer[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetOwnersControlledCompanion[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCreatureScriptsToSet[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetProjectileTravelTime[ 4 ] = { ACTIONTYPE_LOCATION, ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetRosterNPCPartyLimit[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetRosterNPCPartyLimit = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetIsRosterMemberCampaignNPC[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsRosterMemberCampaignNPC[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsRosterMember[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ShowWorldMap[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_TriggerEncounter[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetIsSinglePlayer = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DisplayGuiScreen[ 5 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectOnDispel[ 2 ] = { ACTIONTYPE_FLOAT, ACTIONTYPE_ACTION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_LoadNewModule[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetScriptHidden[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetIsCompanionPossessionBlocked[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetEventHandler[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetEventHandler[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsPartyTransition[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_JumpPartyToArea[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNumActions[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DisplayMessageBox[ 11 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_StringCompare[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_CharToASCII[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetOwnedCharacter[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetControlledCharacter[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_FeatAdd[ 5 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_FeatRemove[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCanTalkToNonPlayerOwnedCreatures[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCanTalkToNonPlayerOwnedCreatures[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetLevelUpPackage[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLevelUpPackage[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCombatOverrides[ 11 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ClearCombatOverrides[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ResetCreatureLevelForXP[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_CalcPointAwayFromPoint[ 5 ] = { ACTIONTYPE_LOCATION, ACTIONTYPE_LOCATION, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_CalcSafeLocation[ 5 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_LOCATION, ACTIONTYPE_FLOAT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTotalLevels[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ResetFeatUses[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetNWN2Fog[ 5 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ResetNWN2Fog[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectBonusHitpoints[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetGUIObjectHidden[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_CloseGUIScreen[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_FiredFromPartyTransition = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetScriptHidden[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetNodeSpeaker = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetLocalGUIVariable[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetGUIObjectDisabled[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetGUIObjectText[ 5 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsCompanionPossessionBlocked[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectBardSongSinging[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectJarring = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetEffectInteger[ 2 ] = { ACTIONTYPE_EFFECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_RefreshSpellEffectDurations[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetEffectSpellId[ 2 ] = { ACTIONTYPE_EFFECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectBABMinimum[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTRUEBaseAttackBonus[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetFirstName[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetLastName[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetDescription[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFirstName[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetLastName[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetDescription[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_IsInMultiplayerConversation[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_PlayCustomAnimation[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectMaxDamage = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_DisplayInputBox[ 13 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetWeaponVisibility[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetLookAtTarget[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_VECTOR, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetBumpState[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetBumpState[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetOnePartyMode = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetPrimaryPlayer = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectArcaneSpellFailure[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SpawnBloodHit[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetFirstArea = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetNextArea = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetArmorRank[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetWeaponType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetPlayerCurrentTarget[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectWildshape = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetEncumbranceState[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_PackCampaignDatabase[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_UnlinkDoor[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetPlayerCreatureExamineTarget[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_Clear2DACache[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_NWNXGetInt[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_NWNXGetFloat[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_NWNXGetString[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_NWNXSetInt[ 5 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_NWNXSetFloat[ 5 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_NWNXSetString[ 5 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectEffectIcon[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetGUIProgressBarPosition[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetGUITexture[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectRescue[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_IntToObject[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ObjectToInt[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_StringToObject[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsSpirit[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectDetectSpirits = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectDamageReductionNegated = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectConcealmentNegated = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetInfiniteFlag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetKeyRequiredFeedbackMessage[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetKeyRequiredFeedbackMessage[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetInfiniteFlag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetPickpocketableFlag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetPickpocketableFlag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTrapActive[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetWillSavingThrow[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetReflexSavingThrow[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetFortitudeSavingThrow[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetLockKeyRequired[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetLockKeyTag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetLockLockDC[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetLockUnlockDC[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetLockLockable[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetHardness[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetHardness[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetModuleXPScale = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetModuleXPScale[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetTrapDetectable[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetTrapDetectDC[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetTrapDisarmable[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetTrapDisarmDC[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetTrapKeyTag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetTrapOneShot[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_CreateTrapAtLocation[ 7 ] = { ACTIONTYPE_INT, ACTIONTYPE_LOCATION, ACTIONTYPE_FLOAT, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_CreateTrapOnObject[ 5 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAreaSize[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetTrapRecoverable[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetTrapRecoverable[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetUseableFlag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetPlaceableLastClickedBy = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetRenderWaterInArea[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_EffectInsane = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetPlayerGUIHidden[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetSelectedMapPointTag = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetNoticeText[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetLightActive[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_EffectSummonCopy[ 6 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetPolymorphLocked[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetSoundSet[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetScale[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetScale[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNum2DARows[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNum2DAColumns[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCustomHeartbeat[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCustomHeartbeat[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetScrollBarRanges[ 7 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ClearListBox[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AddListBoxRow[ 8 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_RemoveListBoxRow[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemPropActivation[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetItemPropActivation[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ModifyListBoxRow[ 8 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetFactionLeader[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetFirstSubArea[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_VECTOR };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetNextSubArea[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetMovementRateFactor[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetMovementRateFactor[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetBicFileName[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCollision[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetCollision[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemIcon[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetVariableName[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetVariableType[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAreaOfEffectDuration[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsPlayerCreated[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetPartyName = NULL;
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetPartyMotto = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsOverlandMap[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetUnrestrictedLevelUp[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetSoundFileDuration[ 1 ] = { ACTIONTYPE_STRING };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetPartyMembersDyingFlag = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetListBoxRowSelected[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetTalkTableLanguage = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetScrollBarValue[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetPause[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetPause = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetAreaOfEffectSpellId[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetGlobalGUIVariable[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_CreateInstancedAreaFromSource[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetVariableValueInt[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetVariableValueString[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetVariableValueFloat[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetVariableValueLocation[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetVariableValueObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetVariableCount[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetBaseAbilityScore[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetBaseSkillRank[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SendChatMessage[ 5 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetIsLocationValid[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetSurfaceMaterialsAtLocation[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetSpellKnown[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetItemBaseMaterialType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetItemBaseMaterialType[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetSpellKnown[ 5 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_GetLimboCreatureCount = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCreatureInLimbo[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SendCreatureToLimbo[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AddScriptParameterInt[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AddScriptParameterString[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AddScriptParameterFloat[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_AddScriptParameterObject[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_ExecuteScriptEnhanced[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN2_NWActionParameterTypes_ClearScriptParams = NULL;
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetSkillPointsRemaining[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetSkillPointsRemaining[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetCasterClassSpellSchool[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetGender[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetTag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetArmorRulesType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetArmorRulesType[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_SetItemIcon[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_GetObjectByTagAndType[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN2_NWActionParameterTypes_RecallCreatureFromLimboToLocation[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_LOCATION };



static const unsigned long NWN2_NWActionTotalParameterSizes_Random[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_PrintString[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_PrintFloat[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_FloatToString[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_PrintInteger[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_PrintObject[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AssignCommand[ 2 ] = { 4, 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_DelayCommand[ 2 ] = { 4, 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ExecuteScript[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ClearAllActions[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetFacing[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCalendar[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetTime[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetCalendarYear = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetCalendarMonth = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetCalendarDay = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetTimeHour = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetTimeMinute = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetTimeSecond = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetTimeMillisecond = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_ActionRandomWalk = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionMoveToLocation[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionMoveToObject[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionMoveAwayFromObject[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetArea[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetEnteringObject = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetExitingObject = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetPosition[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFacing[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemPossessor[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemPossessedBy[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_CreateItemOnObject[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionEquipItem[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionUnequipItem[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionPickUpItem[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionPutDownItem[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLastAttacker[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionAttack[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNearestCreature[ 8 ] = { 4, 8, 12, 16, 20, 24, 28, 32 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionSpeakString[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionPlayAnimation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetDistanceToObject[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsObjectValid[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionOpenDoor[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionCloseDoor[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCameraFacing[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_PlaySound[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetSpellTargetObject = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionCastSpellAtObject[ 7 ] = { 4, 8, 12, 16, 20, 24, 28 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCurrentHitPoints[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetMaxHitPoints[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLocalInt[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLocalFloat[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLocalString[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLocalObject[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetLocalInt[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetLocalFloat[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetLocalString[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetLocalObject[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetStringLength[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetStringUpperCase[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetStringLowerCase[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetStringRight[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetStringLeft[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_InsertString[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetSubString[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_FindSubString[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_fabs[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_cos[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_sin[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_tan[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_acos[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_asin[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_atan[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_log[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_pow[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_sqrt[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_abs[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectHeal[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectDamage[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectAbilityIncrease[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectDamageResistance[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectResurrection = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectSummonCreature[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCasterLevel[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFirstEffect[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNextEffect[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_RemoveEffect[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsEffectValid[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetEffectDurationType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetEffectSubType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetEffectCreator[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_IntToString[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFirstObjectInArea[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNextObjectInArea[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_d2[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_d3[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_d4[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_d6[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_d8[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_d10[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_d12[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_d20[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_d100[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_VectorMagnitude[ 1 ] = { 12 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetMetaMagicFeat = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetObjectType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetRacialType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_FortitudeSave[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ReflexSave[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_WillSave[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetSpellSaveDC = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_MagicalEffect[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SupernaturalEffect[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ExtraordinaryEffect[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectACIncrease[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAC[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectSavingThrowIncrease[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectAttackIncrease[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectDamageReduction[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectDamageIncrease[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_RoundsToSeconds[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_HoursToSeconds[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_TurnsToSeconds[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLawChaosValue[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetGoodEvilValue[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAlignmentLawChaos[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAlignmentGoodEvil[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFirstObjectInShape[ 6 ] = { 4, 8, 12, 16, 20, 32 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNextObjectInShape[ 6 ] = { 4, 8, 12, 16, 20, 32 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectEntangle = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SignalEvent[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EventUserDefined[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectDeath[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectKnockdown = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionGiveItem[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionTakeItem[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_VectorNormalize[ 1 ] = { 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectCurse[ 6 ] = { 4, 8, 12, 16, 20, 24 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAbilityScore[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsDead[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_PrintVector[ 2 ] = { 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_Vector[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetFacingPoint[ 2 ] = { 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AngleToVector[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_VectorToAngle[ 1 ] = { 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_TouchAttackMelee[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_TouchAttackRanged[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectParalyze[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectSpellImmunity[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectDeaf = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetDistanceBetween[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetLocalLocation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLocalLocation[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectSleep = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemInSlot[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectCharmed = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectConfused = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectFrightened = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectDominated = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectDazed = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectStunned = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCommandable[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCommandable[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectRegenerate[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectMovementSpeedIncrease[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetHitDice[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionForceFollowObject[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTag[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ResistSpell[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetEffectType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectAreaOfEffect[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFactionEqual[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ChangeFaction[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsListening[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetListening[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetListenPattern[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_TestStringAgainstPattern[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetMatchedSubstring[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetMatchedSubstringsCount = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectVisualEffect[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFactionWeakestMember[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFactionStrongestMember[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFactionMostDamagedMember[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFactionLeastDamagedMember[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFactionGold[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFactionAverageReputation[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFactionAverageGoodEvilAlignment[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFactionAverageLawChaosAlignment[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFactionAverageLevel[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFactionAverageXP[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFactionMostFrequentClass[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFactionWorstAC[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFactionBestAC[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionSit[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetListenPatternNumber = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionJumpToObject[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetWaypointByTag[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTransitionTarget[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectLinkEffects[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetObjectByTag[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AdjustAlignment[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionWait[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetAreaTransitionBMP[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionStartConversation[ 6 ] = { 4, 8, 12, 16, 20, 24 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_ActionPauseConversation = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_ActionResumeConversation = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectBeam[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetReputation[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AdjustReputation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetSittingCreature[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetGoingToBeAttackedBy[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectSpellResistanceIncrease[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLocation[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionJumpToLocation[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_Location[ 3 ] = { 4, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ApplyEffectAtLocation[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsPC[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_FeetToMeters[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_YardsToMeters[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ApplyEffectToObject[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SpeakString[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetSpellTargetLocation = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetPositionFromLocation[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAreaFromLocation[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFacingFromLocation[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNearestCreatureToLocation[ 8 ] = { 4, 8, 12, 16, 20, 24, 28, 32 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNearestObject[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNearestObjectToLocation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNearestObjectByTag[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_IntToFloat[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_FloatToInt[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_StringToInt[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_StringToFloat[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionCastSpellAtLocation[ 7 ] = { 4, 8, 12, 16, 20, 24, 28 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsEnemy[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsFriend[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsNeutral[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetPCSpeaker = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetStringByStrRef[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionSpeakStringByStrRef[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_DestroyObject[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetModule = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_CreateObject[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EventSpellCastAt[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastSpellCaster = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastSpell = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetUserDefinedEventNumber = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetSpellId = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_RandomName = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectPoison[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectDisease[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectSilence = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetName[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastSpeaker = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_BeginConversation[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastPerceived = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastPerceptionHeard = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastPerceptionInaudible = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastPerceptionSeen = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastClosedBy = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastPerceptionVanished = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFirstInPersistentObject[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNextInPersistentObject[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAreaOfEffectCreator[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_DeleteLocalInt[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_DeleteLocalFloat[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_DeleteLocalString[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_DeleteLocalObject[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_DeleteLocalLocation[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectHaste = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectSlow = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_ObjectToString[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectImmunity[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsImmune[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectDamageImmunityIncrease[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetEncounterActive[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetEncounterActive[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetEncounterSpawnsMax[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetEncounterSpawnsMax[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetEncounterSpawnsCurrent[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetEncounterSpawnsCurrent[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetModuleItemAcquired = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetModuleItemAcquiredFrom = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCustomToken[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetHasFeat[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetHasSkill[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionUseFeat[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionUseSkill[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetObjectSeen[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetObjectHeard[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastPlayerDied = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetModuleItemLost = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetModuleItemLostBy = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionDoCommand[ 1 ] = { 0 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EventConversation = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SetEncounterDifficulty[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetEncounterDifficulty[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetDistanceBetweenLocations[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetReflexAdjustedDamage[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_PlayAnimation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_TalentSpell[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_TalentFeat[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_TalentSkill[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetHasSpellEffect[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetEffectSpellId[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCreatureHasTalent[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCreatureTalentRandom[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCreatureTalentBest[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionUseTalentOnObject[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionUseTalentAtLocation[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetGoldPieceValue[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsPlayableRacialType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_JumpToLocation[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectTemporaryHitpoints[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetSkillRank[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAttackTarget[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLastAttackType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLastAttackMode[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetMaster[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsInCombat[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLastAssociateCommand[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GiveGoldToCreature[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetIsDestroyable[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetLocked[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLocked[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetClickingObject = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SetAssociateListenPatterns[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLastWeaponUsed[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionInteractObject[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastUsedBy = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAbilityModifier[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIdentified[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetIdentified[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SummonAnimalCompanion[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SummonFamiliar[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetBlockingDoor = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsDoorActionPossible[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_DoDoorAction[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFirstItemInInventory[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNextItemInInventory[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetClassByPosition[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLevelByPosition[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLevelByClass[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetDamageDealtByType[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetTotalDamageDealt = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLastDamager[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastDisarmed = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastDisturbed = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastLocked = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastUnlocked = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectSkillIncrease[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetInventoryDisturbType = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetInventoryDisturbItem = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetHenchman[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_VersusAlignmentEffect[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_VersusRacialTypeEffect[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_VersusTrapEffect[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetGender[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsTalentValid[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionMoveAwayFromLocation[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetAttemptedAttackTarget = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTypeFromTalent[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIdFromTalent[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAssociate[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AddHenchman[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_RemoveHenchman[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AddJournalQuestEntry[ 6 ] = { 4, 8, 12, 16, 20, 24 };
static const unsigned long NWN2_NWActionTotalParameterSizes_RemoveJournalQuestEntry[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetPCPublicCDKey[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetPCIPAddress[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetPCPlayerName[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetPCLike[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetPCDislike[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SendMessageToPC[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetAttemptedSpellTarget = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastOpenedBy = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetHasSpell[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_OpenStore[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectTurned = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFirstFactionMember[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNextFactionMember[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionForceMoveToLocation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionForceMoveToObject[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetJournalQuestExperience[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_JumpToObject[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetMapPinEnabled[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectHitPointChangeWhenDying[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_PopUpGUIPanel[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ClearPersonalReputation[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetIsTemporaryFriend[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetIsTemporaryEnemy[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetIsTemporaryNeutral[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GiveXPToCreature[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetXP[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetXP[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_IntToHexString[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetBaseItemType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemHasItemProperty[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionEquipMostDamagingMelee[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionEquipMostDamagingRanged[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemACValue[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionRest[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ExploreAreaForPlayer[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_ActionEquipMostEffectiveArmor = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetIsDay = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetIsNight = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetIsDawn = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetIsDusk = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsEncounterCreature[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastPlayerDying = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetStartingLocation = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_ChangeToStandardFaction[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SoundObjectPlay[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SoundObjectStop[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SoundObjectSetVolume[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SoundObjectSetPosition[ 2 ] = { 4, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SpeakOneLinerConversation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetGold[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastRespawnButtonPresser = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsDM[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_PlayVoiceChat[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsWeaponEffective[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastSpellHarmful = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_EventActivateItem[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_MusicBackgroundPlay[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_MusicBackgroundStop[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_MusicBackgroundSetDelay[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_MusicBackgroundChangeDay[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_MusicBackgroundChangeNight[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_MusicBattlePlay[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_MusicBattleStop[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_MusicBattleChange[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AmbientSoundPlay[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AmbientSoundStop[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AmbientSoundChangeDay[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AmbientSoundChangeNight[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastKiller = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetSpellCastItem = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetItemActivated = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetItemActivator = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetItemActivatedTargetLocation = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetItemActivatedTarget = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsOpen[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_TakeGoldFromCreature[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_IsInConversation[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectAbilityDecrease[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectAttackDecrease[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectDamageDecrease[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectDamageImmunityDecrease[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectACDecrease[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectMovementSpeedDecrease[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectSavingThrowDecrease[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectSkillDecrease[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectSpellResistanceDecrease[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetPlotFlag[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetPlotFlag[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectInvisibility[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectConcealment[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectDarkness = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectDispelMagicAll[ 2 ] = { 4, 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectUltravision = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectNegativeLevel[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectPolymorph[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectSanctuary[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectTrueSeeing = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectSeeInvisible = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectTimeStop = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectBlindness = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsReactionTypeFriendly[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsReactionTypeNeutral[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsReactionTypeHostile[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectSpellLevelAbsorption[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectDispelMagicBest[ 2 ] = { 4, 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActivatePortal[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNumStackedItems[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_SurrenderToEnemies = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectMissChance[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTurnResistanceHD[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCreatureSize[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectDisappearAppear[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectDisappear[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectAppear[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionUnlockObject[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionLockObject[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectModifyAttacks[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLastTrapDetected[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectDamageShield[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNearestTrapToObject[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetDeity[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetSubRace[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFortitudeSavingThrow[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetWillSavingThrow[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetReflexSavingThrow[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetChallengeRating[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAge[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetMovementRate[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAnimalCompanionCreatureType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFamiliarCreatureType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAnimalCompanionName[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFamiliarName[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionCastFakeSpellAtObject[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionCastFakeSpellAtLocation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_RemoveSummonedAssociate[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCameraMode[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsResting[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastPCRested = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SetWeather[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastRestEventType = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_StartNewModule[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectSwarm[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetWeaponRanged[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_DoSinglePlayerAutoSave = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetGameDifficulty = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SetTileMainLightColor[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetTileSourceLightColor[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_RecomputeStaticLighting[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTileMainLight1Color[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTileMainLight2Color[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTileSourceLight1Color[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTileSourceLight2Color[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetPanelButtonFlash[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCurrentAction[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetStandardFactionReputation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetStandardFactionReputation[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_FloatingTextStrRefOnCreature[ 8 ] = { 4, 8, 12, 16, 20, 24, 28, 40 };
static const unsigned long NWN2_NWActionTotalParameterSizes_FloatingTextStringOnCreature[ 8 ] = { 4, 8, 12, 16, 20, 24, 28, 40 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTrapDisarmable[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTrapDetectable[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTrapDetectedBy[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTrapFlagged[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTrapBaseType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTrapOneShot[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTrapCreator[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTrapKeyTag[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTrapDisarmDC[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTrapDetectDC[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLockKeyRequired[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLockKeyTag[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLockLockable[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLockUnlockDC[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLockLockDC[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetPCLevellingUp = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetHasFeatEffect[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetPlaceableIllumination[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetPlaceableIllumination[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsPlaceableObjectActionPossible[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_DoPlaceableObjectAction[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFirstPC[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNextPC[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetTrapDetectedBy[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsTrapped[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectTurnResistanceDecrease[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectTurnResistanceIncrease[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_PopUpDeathGUIPanel[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetTrapDisabled[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLastHostileActor[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_ExportAllCharacters = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_MusicBackgroundGetDayTrack[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_MusicBackgroundGetNightTrack[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_WriteTimestampedLogEntry[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetModuleName = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFactionLeader[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SendMessageToAllDMs[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EndGame[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_BootPC[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionCounterSpell[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AmbientSoundSetDayVolume[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AmbientSoundSetNightVolume[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_MusicBackgroundGetBattleTrack[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetHasInventory[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetStrRefSoundDuration[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AddToParty[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_RemoveFromParty[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetStealthMode[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetDetectMode[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetDefensiveCastingMode[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAppearanceType[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_SpawnScriptDebugger = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetModuleItemAcquiredStackSize = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_DecrementRemainingFeatUses[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_DecrementRemainingSpellUses[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetResRef[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectPetrify = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_CopyItem[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectCutsceneParalyze = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetDroppableFlag[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetUseableFlag[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetStolenFlag[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCampaignFloat[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCampaignInt[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCampaignVector[ 4 ] = { 4, 8, 20, 24 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCampaignLocation[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCampaignString[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_DestroyCampaignDatabase[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCampaignFloat[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCampaignInt[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCampaignVector[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCampaignLocation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCampaignString[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_CopyObject[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_DeleteCampaignVariable[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_StoreCampaignObject[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_RetrieveCampaignObject[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectCutsceneDominated = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemStackSize[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetItemStackSize[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemCharges[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetItemCharges[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AddItemProperty[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_RemoveItemProperty[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsItemPropertyValid[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFirstItemProperty[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNextItemProperty[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemPropertyType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemPropertyDurationType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyAbilityBonus[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyACBonus[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyACBonusVsAlign[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyACBonusVsDmgType[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyACBonusVsRace[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyACBonusVsSAlign[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyEnhancementBonus[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyEnhancementBonusVsAlign[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyEnhancementBonusVsRace[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyEnhancementBonusVsSAlign[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyEnhancementPenalty[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyWeightReduction[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyBonusFeat[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyBonusLevelSpell[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyCastSpell[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyDamageBonus[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyDamageBonusVsAlign[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyDamageBonusVsRace[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyDamageBonusVsSAlign[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyDamageImmunity[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyDamagePenalty[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyDamageReduction[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyDamageResistance[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyDamageVulnerability[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_ItemPropertyDarkvision = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyDecreaseAbility[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyDecreaseAC[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyDecreaseSkill[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyContainerReducedWeight[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyExtraMeleeDamageType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyExtraRangeDamageType[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_ItemPropertyHaste = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_ItemPropertyHolyAvenger = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyImmunityMisc[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_ItemPropertyImprovedEvasion = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyBonusSpellResistance[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyBonusSavingThrowVsX[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyBonusSavingThrow[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_ItemPropertyKeen = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyLight[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyMaxRangeStrengthMod[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_ItemPropertyNoDamage = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyOnHitProps[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyReducedSavingThrowVsX[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyReducedSavingThrow[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyRegeneration[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertySkillBonus[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertySpellImmunitySpecific[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertySpellImmunitySchool[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyThievesTools[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyAttackBonus[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyAttackBonusVsAlign[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyAttackBonusVsRace[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyAttackBonusVsSAlign[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyAttackPenalty[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyUnlimitedAmmo[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyLimitUseByAlign[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyLimitUseByClass[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyLimitUseByRace[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyLimitUseBySAlign[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyBonusHitpoints[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyVampiricRegeneration[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyTrap[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_ItemPropertyTrueSeeing = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyOnMonsterHitProperties[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyTurnResistance[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyMassiveCritical[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_ItemPropertyFreeAction = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyMonsterDamage[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyImmunityToSpellLevel[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertySpecialWalk[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyHealersKit[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyWeightIncrease[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsSkillSuccessful[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectSpellFailure[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SpeakStringByStrRef[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCutsceneMode[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastPCToCancelCutscene = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetDialogSoundLength[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_FadeFromBlack[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_FadeToBlack[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_StopFade[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_BlackScreen[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetBaseAttackBonus[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetImmortal[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_OpenInventory[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_StoreCameraFacing = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_RestoreCameraFacing = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_LevelUpHenchman[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetDroppableFlag[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetWeight[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetModuleItemAcquiredBy = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetImmortal[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_DoWhirlwindAttack[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_Get2DAString[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectEthereal = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAILevel[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetAILevel[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsPossessedFamiliar[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_UnpossessFamiliar[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsAreaInterior[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SendMessageToPCByStrRef[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_IncrementRemainingFeatUses[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ExportSingleCharacter[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_PlaySoundByStrRef[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetSubRace[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetDeity[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsDMPossessed[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetWeather[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsAreaNatural[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsAreaAboveGround[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetPCItemLastEquipped = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetPCItemLastEquippedBy = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetPCItemLastUnequipped = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetPCItemLastUnequippedBy = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_CopyItemAndModify[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemAppearance[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyOnHitCastSpell[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemPropertySubType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetActionMode[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetActionMode[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetArcaneSpellFailure[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionExamine[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyVisualEffect[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetLootable[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLootable[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCutsceneCameraMoveRate[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCutsceneCameraMoveRate[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemCursedFlag[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetItemCursedFlag[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetMaxHenchmen[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetMaxHenchmen = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAssociateType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetSpellResistance[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_DayToNight[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_NightToDay[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_LineOfSightObject[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_LineOfSightVector[ 2 ] = { 12, 24 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLastSpellCastClass = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SetBaseAttackBonus[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_RestoreBaseAttackBonus[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectCutsceneGhost = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_ItemPropertyArcaneSpellFailure[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetStoreGold[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetStoreGold[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetStoreMaxBuyPrice[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetStoreMaxBuyPrice[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetStoreIdentifyCost[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetStoreIdentifyCost[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCreatureAppearanceType[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCreatureStartingPackage[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectCutsceneImmobilize = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsInSubArea[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemPropertyCostTable[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemPropertyCostTableValue[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemPropertyParam1[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemPropertyParam1Value[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsCreatureDisarmable[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetStolenFlag[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ForceRest[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCameraHeight[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetGlobalInt[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetGlobalBool[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetGlobalString[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetGlobalFloat[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetGlobalInt[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetGlobalBool[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetGlobalString[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetGlobalFloat[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SaveGlobalVariables[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_LoadGlobalVariables[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_MountObject[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_DismountObject[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetJournalEntry[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectNWN2ParticleEffect = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectNWN2ParticleEffectFile[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectNWN2SpecialEffectFile[ 3 ] = { 4, 8, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetSpellLevel[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_RemoveSEFFromObject[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ActionPauseCutscene[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AssignCutsceneActionToObject[ 2 ] = { 4, 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCharBackground[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetTrapActive[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetOrientOnDialog[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectDetectUndead = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectLowLightVision = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectSetScale[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectShareDamage[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectAssayResistance[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectSeeTrueHPs = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetNumCutsceneActionsPending = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectDamageOverTime[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectAbsorbDamage[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectHideousBlow[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectMesmerize[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetSpellFeatId = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SetFog[ 6 ] = { 4, 8, 12, 16, 20, 24 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectDarkVision = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_DebugPostString[ 6 ] = { 4, 8, 12, 16, 20, 24 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetHasAnySpellEffect[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectArmorCheckPenaltyIncrease[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectDisintegrate[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectHealOnZeroHP[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectBreakEnchantment[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetFirstEnteringPC = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetNextEnteringPC = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_AddRosterMemberByTemplate[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AddRosterMemberByCharacter[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_RemoveRosterMember[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsRosterMemberAvailable[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsRosterMemberSelectable[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetIsRosterMemberSelectable[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetObjectFromRosterName[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetRosterNameFromObject[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SpawnRosterMember[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_DespawnRosterMember[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AddRosterMemberToParty[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_RemoveRosterMemberFromParty[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetFirstRosterMember = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetNextRosterMember = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SpawnSpellProjectile[ 6 ] = { 4, 8, 12, 16, 20, 24 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SpawnItemProjectile[ 8 ] = { 4, 8, 12, 16, 20, 24, 28, 32 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsOwnedByPlayer[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetOwnersControlledCompanion[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCreatureScriptsToSet[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetProjectileTravelTime[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetRosterNPCPartyLimit[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetRosterNPCPartyLimit = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SetIsRosterMemberCampaignNPC[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsRosterMemberCampaignNPC[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsRosterMember[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ShowWorldMap[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_TriggerEncounter[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetIsSinglePlayer = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_DisplayGuiScreen[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectOnDispel[ 2 ] = { 4, 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_LoadNewModule[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetScriptHidden[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetIsCompanionPossessionBlocked[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetEventHandler[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetEventHandler[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsPartyTransition[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_JumpPartyToArea[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNumActions[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_DisplayMessageBox[ 11 ] = { 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44 };
static const unsigned long NWN2_NWActionTotalParameterSizes_StringCompare[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_CharToASCII[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetOwnedCharacter[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetControlledCharacter[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_FeatAdd[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_FeatRemove[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCanTalkToNonPlayerOwnedCreatures[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCanTalkToNonPlayerOwnedCreatures[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetLevelUpPackage[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLevelUpPackage[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCombatOverrides[ 11 ] = { 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ClearCombatOverrides[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ResetCreatureLevelForXP[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_CalcPointAwayFromPoint[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_CalcSafeLocation[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTotalLevels[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ResetFeatUses[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetNWN2Fog[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ResetNWN2Fog[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectBonusHitpoints[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetGUIObjectHidden[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_CloseGUIScreen[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_FiredFromPartyTransition = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetScriptHidden[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetNodeSpeaker = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SetLocalGUIVariable[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetGUIObjectDisabled[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetGUIObjectText[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsCompanionPossessionBlocked[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectBardSongSinging[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectJarring = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetEffectInteger[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_RefreshSpellEffectDurations[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetEffectSpellId[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectBABMinimum[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTRUEBaseAttackBonus[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetFirstName[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetLastName[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetDescription[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFirstName[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetLastName[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetDescription[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_IsInMultiplayerConversation[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_PlayCustomAnimation[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectMaxDamage = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_DisplayInputBox[ 13 ] = { 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetWeaponVisibility[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetLookAtTarget[ 3 ] = { 4, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetBumpState[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetBumpState[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetOnePartyMode = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetPrimaryPlayer = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectArcaneSpellFailure[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SpawnBloodHit[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetFirstArea = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetNextArea = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetArmorRank[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetWeaponType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetPlayerCurrentTarget[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectWildshape = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetEncumbranceState[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_PackCampaignDatabase[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_UnlinkDoor[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetPlayerCreatureExamineTarget[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_Clear2DACache[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_NWNXGetInt[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_NWNXGetFloat[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_NWNXGetString[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_NWNXSetInt[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_NWNXSetFloat[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_NWNXSetString[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectEffectIcon[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetGUIProgressBarPosition[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetGUITexture[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectRescue[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_IntToObject[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ObjectToInt[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_StringToObject[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsSpirit[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectDetectSpirits = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectDamageReductionNegated = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectConcealmentNegated = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetInfiniteFlag[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetKeyRequiredFeedbackMessage[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetKeyRequiredFeedbackMessage[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetInfiniteFlag[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetPickpocketableFlag[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetPickpocketableFlag[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTrapActive[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetWillSavingThrow[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetReflexSavingThrow[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetFortitudeSavingThrow[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetLockKeyRequired[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetLockKeyTag[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetLockLockDC[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetLockUnlockDC[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetLockLockable[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetHardness[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetHardness[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetModuleXPScale = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SetModuleXPScale[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetTrapDetectable[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetTrapDetectDC[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetTrapDisarmable[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetTrapDisarmDC[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetTrapKeyTag[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetTrapOneShot[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_CreateTrapAtLocation[ 7 ] = { 4, 8, 12, 16, 20, 24, 28 };
static const unsigned long NWN2_NWActionTotalParameterSizes_CreateTrapOnObject[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAreaSize[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetTrapRecoverable[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetTrapRecoverable[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetUseableFlag[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetPlaceableLastClickedBy = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SetRenderWaterInArea[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_EffectInsane = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SetPlayerGUIHidden[ 2 ] = { 4, 8 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetSelectedMapPointTag = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SetNoticeText[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetLightActive[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_EffectSummonCopy[ 6 ] = { 4, 8, 12, 16, 20, 24 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetPolymorphLocked[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetSoundSet[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetScale[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetScale[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNum2DARows[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNum2DAColumns[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCustomHeartbeat[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCustomHeartbeat[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetScrollBarRanges[ 7 ] = { 4, 8, 12, 16, 20, 24, 28 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ClearListBox[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AddListBoxRow[ 8 ] = { 4, 8, 12, 16, 20, 24, 28, 32 };
static const unsigned long NWN2_NWActionTotalParameterSizes_RemoveListBoxRow[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemPropActivation[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetItemPropActivation[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ModifyListBoxRow[ 8 ] = { 4, 8, 12, 16, 20, 24, 28, 32 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetFactionLeader[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetFirstSubArea[ 2 ] = { 4, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetNextSubArea[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetMovementRateFactor[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetMovementRateFactor[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetBicFileName[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCollision[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetCollision[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemIcon[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetVariableName[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetVariableType[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAreaOfEffectDuration[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsPlayerCreated[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetPartyName = NULL;
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetPartyMotto = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsOverlandMap[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetUnrestrictedLevelUp[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetSoundFileDuration[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetPartyMembersDyingFlag = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SetListBoxRowSelected[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetTalkTableLanguage = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SetScrollBarValue[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetPause[ 1 ] = { 4 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetPause = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetAreaOfEffectSpellId[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetGlobalGUIVariable[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_CreateInstancedAreaFromSource[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetVariableValueInt[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetVariableValueString[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetVariableValueFloat[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetVariableValueLocation[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetVariableValueObject[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetVariableCount[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetBaseAbilityScore[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetBaseSkillRank[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SendChatMessage[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetIsLocationValid[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetSurfaceMaterialsAtLocation[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetSpellKnown[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetItemBaseMaterialType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetItemBaseMaterialType[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetSpellKnown[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_GetLimboCreatureCount = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCreatureInLimbo[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SendCreatureToLimbo[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AddScriptParameterInt[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AddScriptParameterString[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AddScriptParameterFloat[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_AddScriptParameterObject[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_ExecuteScriptEnhanced[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN2_NWActionTotalParameterSizes_ClearScriptParams = NULL;
static const unsigned long NWN2_NWActionTotalParameterSizes_SetSkillPointsRemaining[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetSkillPointsRemaining[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetCasterClassSpellSchool[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetGender[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetTag[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetArmorRulesType[ 1 ] = { 4 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetArmorRulesType[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_SetItemIcon[ 2 ] = { 4, 8 };
static const unsigned long NWN2_NWActionTotalParameterSizes_GetObjectByTagAndType[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN2_NWActionTotalParameterSizes_RecallCreatureFromLimboToLocation[ 2 ] = { 4, 8 };



const NWACTION_DEFINITION NWActions_NWN2[ 1058 ] =
{
	{ NWSCRIPT_ACTIONNAME("Random") NWSCRIPT_ACTIONPROTOTYPE("int Random(int nMaxInteger);") 0, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_Random NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_Random) },
	{ NWSCRIPT_ACTIONNAME("PrintString") NWSCRIPT_ACTIONPROTOTYPE("void PrintString(string sString);") 1, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_PrintString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_PrintString) },
	{ NWSCRIPT_ACTIONNAME("PrintFloat") NWSCRIPT_ACTIONPROTOTYPE("void PrintFloat(float fFloat, int nWidth=18, int nDecimals=9);") 2, 1, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_PrintFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_PrintFloat) },
	{ NWSCRIPT_ACTIONNAME("FloatToString") NWSCRIPT_ACTIONPROTOTYPE("string FloatToString(float fFloat, int nWidth=18, int nDecimals=9);") 3, 1, 3, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_FloatToString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_FloatToString) },
	{ NWSCRIPT_ACTIONNAME("PrintInteger") NWSCRIPT_ACTIONPROTOTYPE("void PrintInteger(int nInteger);") 4, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_PrintInteger NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_PrintInteger) },
	{ NWSCRIPT_ACTIONNAME("PrintObject") NWSCRIPT_ACTIONPROTOTYPE("void PrintObject(object oObject);") 5, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_PrintObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_PrintObject) },
	{ NWSCRIPT_ACTIONNAME("AssignCommand") NWSCRIPT_ACTIONPROTOTYPE("void AssignCommand(object oActionSubject,action aActionToAssign);") 6, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AssignCommand NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AssignCommand) },
	{ NWSCRIPT_ACTIONNAME("DelayCommand") NWSCRIPT_ACTIONPROTOTYPE("void DelayCommand(float fSeconds, action aActionToDelay);") 7, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DelayCommand NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DelayCommand) },
	{ NWSCRIPT_ACTIONNAME("ExecuteScript") NWSCRIPT_ACTIONPROTOTYPE("void ExecuteScript(string sScript, object oTarget);") 8, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ExecuteScript NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ExecuteScript) },
	{ NWSCRIPT_ACTIONNAME("ClearAllActions") NWSCRIPT_ACTIONPROTOTYPE("void ClearAllActions(int nClearCombatState=FALSE);") 9, 0, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ClearAllActions NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ClearAllActions) },
	{ NWSCRIPT_ACTIONNAME("SetFacing") NWSCRIPT_ACTIONPROTOTYPE("void SetFacing(float fDirection, int bLockToThisOrientation = FALSE);") 10, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetFacing NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetFacing) },
	{ NWSCRIPT_ACTIONNAME("SetCalendar") NWSCRIPT_ACTIONPROTOTYPE("void SetCalendar(int nYear,int nMonth, int nDay);") 11, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCalendar NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCalendar) },
	{ NWSCRIPT_ACTIONNAME("SetTime") NWSCRIPT_ACTIONPROTOTYPE("void SetTime(int nHour,int nMinute,int nSecond,int nMillisecond);") 12, 4, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetTime NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetTime) },
	{ NWSCRIPT_ACTIONNAME("GetCalendarYear") NWSCRIPT_ACTIONPROTOTYPE("int GetCalendarYear();") 13, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetCalendarYear NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCalendarYear) },
	{ NWSCRIPT_ACTIONNAME("GetCalendarMonth") NWSCRIPT_ACTIONPROTOTYPE("int GetCalendarMonth();") 14, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetCalendarMonth NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCalendarMonth) },
	{ NWSCRIPT_ACTIONNAME("GetCalendarDay") NWSCRIPT_ACTIONPROTOTYPE("int GetCalendarDay();") 15, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetCalendarDay NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCalendarDay) },
	{ NWSCRIPT_ACTIONNAME("GetTimeHour") NWSCRIPT_ACTIONPROTOTYPE("int GetTimeHour();") 16, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTimeHour NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTimeHour) },
	{ NWSCRIPT_ACTIONNAME("GetTimeMinute") NWSCRIPT_ACTIONPROTOTYPE("int GetTimeMinute();") 17, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTimeMinute NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTimeMinute) },
	{ NWSCRIPT_ACTIONNAME("GetTimeSecond") NWSCRIPT_ACTIONPROTOTYPE("int GetTimeSecond();") 18, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTimeSecond NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTimeSecond) },
	{ NWSCRIPT_ACTIONNAME("GetTimeMillisecond") NWSCRIPT_ACTIONPROTOTYPE("int GetTimeMillisecond();") 19, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTimeMillisecond NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTimeMillisecond) },
	{ NWSCRIPT_ACTIONNAME("ActionRandomWalk") NWSCRIPT_ACTIONPROTOTYPE("void ActionRandomWalk();") 20, 0, 0, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionRandomWalk NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionRandomWalk) },
	{ NWSCRIPT_ACTIONNAME("ActionMoveToLocation") NWSCRIPT_ACTIONPROTOTYPE("void ActionMoveToLocation(location lDestination, int bRun=FALSE);") 21, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionMoveToLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionMoveToLocation) },
	{ NWSCRIPT_ACTIONNAME("ActionMoveToObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionMoveToObject(object oMoveTo, int bRun=FALSE, float fRange=1.0f);") 22, 1, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionMoveToObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionMoveToObject) },
	{ NWSCRIPT_ACTIONNAME("ActionMoveAwayFromObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionMoveAwayFromObject(object oFleeFrom, int bRun=FALSE, float fMoveAwayRange=40.0f);") 23, 1, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionMoveAwayFromObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionMoveAwayFromObject) },
	{ NWSCRIPT_ACTIONNAME("GetArea") NWSCRIPT_ACTIONPROTOTYPE("object GetArea(object oTarget);") 24, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetArea NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetArea) },
	{ NWSCRIPT_ACTIONNAME("GetEnteringObject") NWSCRIPT_ACTIONPROTOTYPE("object GetEnteringObject();") 25, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetEnteringObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetEnteringObject) },
	{ NWSCRIPT_ACTIONNAME("GetExitingObject") NWSCRIPT_ACTIONPROTOTYPE("object GetExitingObject();") 26, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetExitingObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetExitingObject) },
	{ NWSCRIPT_ACTIONNAME("GetPosition") NWSCRIPT_ACTIONPROTOTYPE("vector GetPosition(object oTarget);") 27, 1, 1, ACTIONTYPE_VECTOR, NWN2_NWActionParameterTypes_GetPosition NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPosition) },
	{ NWSCRIPT_ACTIONNAME("GetFacing") NWSCRIPT_ACTIONPROTOTYPE("float GetFacing(object oTarget);") 28, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_GetFacing NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFacing) },
	{ NWSCRIPT_ACTIONNAME("GetItemPossessor") NWSCRIPT_ACTIONPROTOTYPE("object GetItemPossessor(object oItem);") 29, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetItemPossessor NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemPossessor) },
	{ NWSCRIPT_ACTIONNAME("GetItemPossessedBy") NWSCRIPT_ACTIONPROTOTYPE("object GetItemPossessedBy(object oCreature, string sItemTag);") 30, 2, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetItemPossessedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemPossessedBy) },
	{ NWSCRIPT_ACTIONNAME("CreateItemOnObject") NWSCRIPT_ACTIONPROTOTYPE("object CreateItemOnObject(string sItemTemplate, object oTarget=OBJECT_SELF, int nStackSize=1, string sNewTag="", int bDisplayFeedback=1);") 31, 1, 5, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_CreateItemOnObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_CreateItemOnObject) },
	{ NWSCRIPT_ACTIONNAME("ActionEquipItem") NWSCRIPT_ACTIONPROTOTYPE("void ActionEquipItem(object oItem, int nInventorySlot);") 32, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionEquipItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionEquipItem) },
	{ NWSCRIPT_ACTIONNAME("ActionUnequipItem") NWSCRIPT_ACTIONPROTOTYPE("void ActionUnequipItem(object oItem);") 33, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionUnequipItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionUnequipItem) },
	{ NWSCRIPT_ACTIONNAME("ActionPickUpItem") NWSCRIPT_ACTIONPROTOTYPE("void ActionPickUpItem(object oItem);") 34, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionPickUpItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionPickUpItem) },
	{ NWSCRIPT_ACTIONNAME("ActionPutDownItem") NWSCRIPT_ACTIONPROTOTYPE("void ActionPutDownItem(object oItem);") 35, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionPutDownItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionPutDownItem) },
	{ NWSCRIPT_ACTIONNAME("GetLastAttacker") NWSCRIPT_ACTIONPROTOTYPE("object GetLastAttacker(object oAttackee=OBJECT_SELF);") 36, 0, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastAttacker NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastAttacker) },
	{ NWSCRIPT_ACTIONNAME("ActionAttack") NWSCRIPT_ACTIONPROTOTYPE("void ActionAttack(object oAttackee, int bPassive=FALSE);") 37, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionAttack NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionAttack) },
	{ NWSCRIPT_ACTIONNAME("GetNearestCreature") NWSCRIPT_ACTIONPROTOTYPE("object GetNearestCreature(int nFirstCriteriaType, int nFirstCriteriaValue, object oTarget=OBJECT_SELF, int nNth=1, int nSecondCriteriaType=-1, int nSecondCriteriaValue=-1, int nThirdCriteriaType=-1,  int nThirdCriteriaValue=-1 );") 38, 2, 8, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetNearestCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNearestCreature) },
	{ NWSCRIPT_ACTIONNAME("ActionSpeakString") NWSCRIPT_ACTIONPROTOTYPE("void ActionSpeakString(string sStringToSpeak, int nTalkVolume=TALKVOLUME_TALK);") 39, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionSpeakString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionSpeakString) },
	{ NWSCRIPT_ACTIONNAME("ActionPlayAnimation") NWSCRIPT_ACTIONPROTOTYPE("void ActionPlayAnimation(int nAnimation, float fSpeed=1.0, float fDurationSeconds=0.0);") 40, 1, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionPlayAnimation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionPlayAnimation) },
	{ NWSCRIPT_ACTIONNAME("GetDistanceToObject") NWSCRIPT_ACTIONPROTOTYPE("float GetDistanceToObject(object oObject);") 41, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_GetDistanceToObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetDistanceToObject) },
	{ NWSCRIPT_ACTIONNAME("GetIsObjectValid") NWSCRIPT_ACTIONPROTOTYPE("int GetIsObjectValid(object oObject);") 42, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsObjectValid NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsObjectValid) },
	{ NWSCRIPT_ACTIONNAME("ActionOpenDoor") NWSCRIPT_ACTIONPROTOTYPE("void ActionOpenDoor(object oDoor);") 43, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionOpenDoor NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionOpenDoor) },
	{ NWSCRIPT_ACTIONNAME("ActionCloseDoor") NWSCRIPT_ACTIONPROTOTYPE("void ActionCloseDoor(object oDoor);") 44, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionCloseDoor NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionCloseDoor) },
	{ NWSCRIPT_ACTIONNAME("SetCameraFacing") NWSCRIPT_ACTIONPROTOTYPE("void SetCameraFacing(float fDirection, float fDistance = -1.0f, float fPitch = -1.0, int nTransitionType=CAMERA_TRANSITION_TYPE_SNAP);") 45, 1, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCameraFacing NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCameraFacing) },
	{ NWSCRIPT_ACTIONNAME("PlaySound") NWSCRIPT_ACTIONPROTOTYPE("void PlaySound(string sSoundName, int bPlayAs2D=FALSE);") 46, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_PlaySound NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_PlaySound) },
	{ NWSCRIPT_ACTIONNAME("GetSpellTargetObject") NWSCRIPT_ACTIONPROTOTYPE("object GetSpellTargetObject();") 47, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetSpellTargetObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetSpellTargetObject) },
	{ NWSCRIPT_ACTIONNAME("ActionCastSpellAtObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionCastSpellAtObject(int nSpell, object oTarget, int nMetaMagic=METAMAGIC_ANY, int bCheat=FALSE, int nDomainLevel=0, int nProjectilePathType=PROJECTILE_PATH_TYPE_DEFAULT, int bInstantSpell=FALSE);") 48, 2, 7, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionCastSpellAtObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionCastSpellAtObject) },
	{ NWSCRIPT_ACTIONNAME("GetCurrentHitPoints") NWSCRIPT_ACTIONPROTOTYPE("int GetCurrentHitPoints(object oObject=OBJECT_SELF);") 49, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetCurrentHitPoints NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCurrentHitPoints) },
	{ NWSCRIPT_ACTIONNAME("GetMaxHitPoints") NWSCRIPT_ACTIONPROTOTYPE("int GetMaxHitPoints(object oObject=OBJECT_SELF);") 50, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetMaxHitPoints NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetMaxHitPoints) },
	{ NWSCRIPT_ACTIONNAME("GetLocalInt") NWSCRIPT_ACTIONPROTOTYPE("int GetLocalInt(object oObject, string sVarName);") 51, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLocalInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLocalInt) },
	{ NWSCRIPT_ACTIONNAME("GetLocalFloat") NWSCRIPT_ACTIONPROTOTYPE("float GetLocalFloat(object oObject, string sVarName);") 52, 2, 2, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_GetLocalFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLocalFloat) },
	{ NWSCRIPT_ACTIONNAME("GetLocalString") NWSCRIPT_ACTIONPROTOTYPE("string GetLocalString(object oObject, string sVarName);") 53, 2, 2, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetLocalString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLocalString) },
	{ NWSCRIPT_ACTIONNAME("GetLocalObject") NWSCRIPT_ACTIONPROTOTYPE("object GetLocalObject(object oObject, string sVarName);") 54, 2, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLocalObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLocalObject) },
	{ NWSCRIPT_ACTIONNAME("SetLocalInt") NWSCRIPT_ACTIONPROTOTYPE("void SetLocalInt(object oObject, string sVarName, int nValue);") 55, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetLocalInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetLocalInt) },
	{ NWSCRIPT_ACTIONNAME("SetLocalFloat") NWSCRIPT_ACTIONPROTOTYPE("void SetLocalFloat(object oObject, string sVarName, float fValue);") 56, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetLocalFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetLocalFloat) },
	{ NWSCRIPT_ACTIONNAME("SetLocalString") NWSCRIPT_ACTIONPROTOTYPE("void SetLocalString(object oObject, string sVarName, string sValue);") 57, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetLocalString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetLocalString) },
	{ NWSCRIPT_ACTIONNAME("SetLocalObject") NWSCRIPT_ACTIONPROTOTYPE("void SetLocalObject(object oObject, string sVarName, object oValue);") 58, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetLocalObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetLocalObject) },
	{ NWSCRIPT_ACTIONNAME("GetStringLength") NWSCRIPT_ACTIONPROTOTYPE("int GetStringLength(string sString);") 59, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetStringLength NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetStringLength) },
	{ NWSCRIPT_ACTIONNAME("GetStringUpperCase") NWSCRIPT_ACTIONPROTOTYPE("string GetStringUpperCase(string sString);") 60, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetStringUpperCase NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetStringUpperCase) },
	{ NWSCRIPT_ACTIONNAME("GetStringLowerCase") NWSCRIPT_ACTIONPROTOTYPE("string GetStringLowerCase(string sString);") 61, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetStringLowerCase NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetStringLowerCase) },
	{ NWSCRIPT_ACTIONNAME("GetStringRight") NWSCRIPT_ACTIONPROTOTYPE("string GetStringRight(string sString, int nCount);") 62, 2, 2, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetStringRight NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetStringRight) },
	{ NWSCRIPT_ACTIONNAME("GetStringLeft") NWSCRIPT_ACTIONPROTOTYPE("string GetStringLeft(string sString, int nCount);") 63, 2, 2, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetStringLeft NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetStringLeft) },
	{ NWSCRIPT_ACTIONNAME("InsertString") NWSCRIPT_ACTIONPROTOTYPE("string InsertString(string sDestination, string sString, int nPosition);") 64, 3, 3, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_InsertString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_InsertString) },
	{ NWSCRIPT_ACTIONNAME("GetSubString") NWSCRIPT_ACTIONPROTOTYPE("string GetSubString(string sString, int nStart, int nCount);") 65, 3, 3, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetSubString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetSubString) },
	{ NWSCRIPT_ACTIONNAME("FindSubString") NWSCRIPT_ACTIONPROTOTYPE("int FindSubString(string sString, string sSubString, int nStart = 0);") 66, 2, 3, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_FindSubString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_FindSubString) },
	{ NWSCRIPT_ACTIONNAME("fabs") NWSCRIPT_ACTIONPROTOTYPE("float fabs(float fValue);") 67, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_fabs NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_fabs) },
	{ NWSCRIPT_ACTIONNAME("cos") NWSCRIPT_ACTIONPROTOTYPE("float cos(float fValue);") 68, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_cos NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_cos) },
	{ NWSCRIPT_ACTIONNAME("sin") NWSCRIPT_ACTIONPROTOTYPE("float sin(float fValue);") 69, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_sin NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_sin) },
	{ NWSCRIPT_ACTIONNAME("tan") NWSCRIPT_ACTIONPROTOTYPE("float tan(float fValue);") 70, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_tan NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_tan) },
	{ NWSCRIPT_ACTIONNAME("acos") NWSCRIPT_ACTIONPROTOTYPE("float acos(float fValue);") 71, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_acos NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_acos) },
	{ NWSCRIPT_ACTIONNAME("asin") NWSCRIPT_ACTIONPROTOTYPE("float asin(float fValue);") 72, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_asin NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_asin) },
	{ NWSCRIPT_ACTIONNAME("atan") NWSCRIPT_ACTIONPROTOTYPE("float atan(float fValue);") 73, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_atan NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_atan) },
	{ NWSCRIPT_ACTIONNAME("log") NWSCRIPT_ACTIONPROTOTYPE("float log(float fValue);") 74, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_log NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_log) },
	{ NWSCRIPT_ACTIONNAME("pow") NWSCRIPT_ACTIONPROTOTYPE("float pow(float fValue, float fExponent);") 75, 2, 2, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_pow NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_pow) },
	{ NWSCRIPT_ACTIONNAME("sqrt") NWSCRIPT_ACTIONPROTOTYPE("float sqrt(float fValue);") 76, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_sqrt NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_sqrt) },
	{ NWSCRIPT_ACTIONNAME("abs") NWSCRIPT_ACTIONPROTOTYPE("int abs(int nValue);") 77, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_abs NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_abs) },
	{ NWSCRIPT_ACTIONNAME("EffectHeal") NWSCRIPT_ACTIONPROTOTYPE("effect EffectHeal(int nDamageToHeal);") 78, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectHeal NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectHeal) },
	{ NWSCRIPT_ACTIONNAME("EffectDamage") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamage(int nDamageAmount, int nDamageType=DAMAGE_TYPE_MAGICAL, int nDamagePower=DAMAGE_POWER_NORMAL, int nIgnoreResistances=FALSE);") 79, 1, 4, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDamage NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDamage) },
	{ NWSCRIPT_ACTIONNAME("EffectAbilityIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectAbilityIncrease(int nAbilityToIncrease, int nModifyBy);") 80, 2, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectAbilityIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectAbilityIncrease) },
	{ NWSCRIPT_ACTIONNAME("EffectDamageResistance") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamageResistance(int nDamageType, int nAmount, int nLimit=0);") 81, 2, 3, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDamageResistance NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDamageResistance) },
	{ NWSCRIPT_ACTIONNAME("EffectResurrection") NWSCRIPT_ACTIONPROTOTYPE("effect EffectResurrection();") 82, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectResurrection NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectResurrection) },
	{ NWSCRIPT_ACTIONNAME("EffectSummonCreature") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSummonCreature(string sCreatureResref, int nVisualEffectId=VFX_NONE, float fDelaySeconds=0.0f, int nUseAppearAnimation=0);") 83, 1, 4, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSummonCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSummonCreature) },
	{ NWSCRIPT_ACTIONNAME("GetCasterLevel") NWSCRIPT_ACTIONPROTOTYPE("int GetCasterLevel(object oCreature);") 84, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetCasterLevel NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCasterLevel) },
	{ NWSCRIPT_ACTIONNAME("GetFirstEffect") NWSCRIPT_ACTIONPROTOTYPE("effect GetFirstEffect(object oCreature);") 85, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_GetFirstEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFirstEffect) },
	{ NWSCRIPT_ACTIONNAME("GetNextEffect") NWSCRIPT_ACTIONPROTOTYPE("effect GetNextEffect(object oCreature);") 86, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_GetNextEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNextEffect) },
	{ NWSCRIPT_ACTIONNAME("RemoveEffect") NWSCRIPT_ACTIONPROTOTYPE("void RemoveEffect(object oCreature, effect eEffect);") 87, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_RemoveEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RemoveEffect) },
	{ NWSCRIPT_ACTIONNAME("GetIsEffectValid") NWSCRIPT_ACTIONPROTOTYPE("int GetIsEffectValid(effect eEffect);") 88, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsEffectValid NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsEffectValid) },
	{ NWSCRIPT_ACTIONNAME("GetEffectDurationType") NWSCRIPT_ACTIONPROTOTYPE("int GetEffectDurationType(effect eEffect);") 89, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetEffectDurationType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetEffectDurationType) },
	{ NWSCRIPT_ACTIONNAME("GetEffectSubType") NWSCRIPT_ACTIONPROTOTYPE("int GetEffectSubType(effect eEffect);") 90, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetEffectSubType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetEffectSubType) },
	{ NWSCRIPT_ACTIONNAME("GetEffectCreator") NWSCRIPT_ACTIONPROTOTYPE("object GetEffectCreator(effect eEffect);") 91, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetEffectCreator NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetEffectCreator) },
	{ NWSCRIPT_ACTIONNAME("IntToString") NWSCRIPT_ACTIONPROTOTYPE("string IntToString(int nInteger);") 92, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_IntToString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_IntToString) },
	{ NWSCRIPT_ACTIONNAME("GetFirstObjectInArea") NWSCRIPT_ACTIONPROTOTYPE("object GetFirstObjectInArea(object oArea=OBJECT_INVALID);") 93, 0, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetFirstObjectInArea NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFirstObjectInArea) },
	{ NWSCRIPT_ACTIONNAME("GetNextObjectInArea") NWSCRIPT_ACTIONPROTOTYPE("object GetNextObjectInArea(object oArea=OBJECT_INVALID);") 94, 0, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetNextObjectInArea NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNextObjectInArea) },
	{ NWSCRIPT_ACTIONNAME("d2") NWSCRIPT_ACTIONPROTOTYPE("int d2(int nNumDice=1);") 95, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_d2 NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_d2) },
	{ NWSCRIPT_ACTIONNAME("d3") NWSCRIPT_ACTIONPROTOTYPE("int d3(int nNumDice=1);") 96, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_d3 NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_d3) },
	{ NWSCRIPT_ACTIONNAME("d4") NWSCRIPT_ACTIONPROTOTYPE("int d4(int nNumDice=1);") 97, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_d4 NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_d4) },
	{ NWSCRIPT_ACTIONNAME("d6") NWSCRIPT_ACTIONPROTOTYPE("int d6(int nNumDice=1);") 98, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_d6 NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_d6) },
	{ NWSCRIPT_ACTIONNAME("d8") NWSCRIPT_ACTIONPROTOTYPE("int d8(int nNumDice=1);") 99, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_d8 NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_d8) },
	{ NWSCRIPT_ACTIONNAME("d10") NWSCRIPT_ACTIONPROTOTYPE("int d10(int nNumDice=1);") 100, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_d10 NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_d10) },
	{ NWSCRIPT_ACTIONNAME("d12") NWSCRIPT_ACTIONPROTOTYPE("int d12(int nNumDice=1);") 101, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_d12 NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_d12) },
	{ NWSCRIPT_ACTIONNAME("d20") NWSCRIPT_ACTIONPROTOTYPE("int d20(int nNumDice=1);") 102, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_d20 NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_d20) },
	{ NWSCRIPT_ACTIONNAME("d100") NWSCRIPT_ACTIONPROTOTYPE("int d100(int nNumDice=1);") 103, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_d100 NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_d100) },
	{ NWSCRIPT_ACTIONNAME("VectorMagnitude") NWSCRIPT_ACTIONPROTOTYPE("float VectorMagnitude(vector vVector);") 104, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_VectorMagnitude NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_VectorMagnitude) },
	{ NWSCRIPT_ACTIONNAME("GetMetaMagicFeat") NWSCRIPT_ACTIONPROTOTYPE("int GetMetaMagicFeat();") 105, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetMetaMagicFeat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetMetaMagicFeat) },
	{ NWSCRIPT_ACTIONNAME("GetObjectType") NWSCRIPT_ACTIONPROTOTYPE("int GetObjectType(object oTarget);") 106, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetObjectType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetObjectType) },
	{ NWSCRIPT_ACTIONNAME("GetRacialType") NWSCRIPT_ACTIONPROTOTYPE("int GetRacialType(object oCreature);") 107, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetRacialType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetRacialType) },
	{ NWSCRIPT_ACTIONNAME("FortitudeSave") NWSCRIPT_ACTIONPROTOTYPE("int FortitudeSave(object oCreature, int nDC, int nSaveType=SAVING_THROW_TYPE_NONE, object oSaveVersus=OBJECT_SELF);") 108, 2, 4, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_FortitudeSave NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_FortitudeSave) },
	{ NWSCRIPT_ACTIONNAME("ReflexSave") NWSCRIPT_ACTIONPROTOTYPE("int ReflexSave(object oCreature, int nDC, int nSaveType=SAVING_THROW_TYPE_NONE, object oSaveVersus=OBJECT_SELF);") 109, 2, 4, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_ReflexSave NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ReflexSave) },
	{ NWSCRIPT_ACTIONNAME("WillSave") NWSCRIPT_ACTIONPROTOTYPE("int WillSave(object oCreature, int nDC, int nSaveType=SAVING_THROW_TYPE_NONE, object oSaveVersus=OBJECT_SELF);") 110, 2, 4, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_WillSave NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_WillSave) },
	{ NWSCRIPT_ACTIONNAME("GetSpellSaveDC") NWSCRIPT_ACTIONPROTOTYPE("int GetSpellSaveDC();") 111, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetSpellSaveDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetSpellSaveDC) },
	{ NWSCRIPT_ACTIONNAME("MagicalEffect") NWSCRIPT_ACTIONPROTOTYPE("effect MagicalEffect(effect eEffect);") 112, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_MagicalEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_MagicalEffect) },
	{ NWSCRIPT_ACTIONNAME("SupernaturalEffect") NWSCRIPT_ACTIONPROTOTYPE("effect SupernaturalEffect(effect eEffect);") 113, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_SupernaturalEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SupernaturalEffect) },
	{ NWSCRIPT_ACTIONNAME("ExtraordinaryEffect") NWSCRIPT_ACTIONPROTOTYPE("effect ExtraordinaryEffect(effect eEffect);") 114, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_ExtraordinaryEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ExtraordinaryEffect) },
	{ NWSCRIPT_ACTIONNAME("EffectACIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectACIncrease(int nValue, int nModifyType=AC_DODGE_BONUS, int nDamageType=AC_VS_DAMAGE_TYPE_ALL, int bVsSpiritsOnly=FALSE);") 115, 1, 4, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectACIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectACIncrease) },
	{ NWSCRIPT_ACTIONNAME("GetAC") NWSCRIPT_ACTIONPROTOTYPE("int GetAC(object oObject, int nForFutureUse=0);") 116, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetAC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAC) },
	{ NWSCRIPT_ACTIONNAME("EffectSavingThrowIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSavingThrowIncrease(int nSave, int nValue, int nSaveType=SAVING_THROW_TYPE_ALL, int bVsSpiritsOnly=FALSE);") 117, 2, 4, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSavingThrowIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSavingThrowIncrease) },
	{ NWSCRIPT_ACTIONNAME("EffectAttackIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectAttackIncrease(int nBonus, int nModifierType=ATTACK_BONUS_MISC);") 118, 1, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectAttackIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectAttackIncrease) },
	{ NWSCRIPT_ACTIONNAME("EffectDamageReduction") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamageReduction(int nAmount, int nDRSubType=DAMAGE_POWER_NORMAL, int nLimit=0, int nDRType=DR_TYPE_MAGICBONUS);") 119, 1, 4, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDamageReduction NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDamageReduction) },
	{ NWSCRIPT_ACTIONNAME("EffectDamageIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamageIncrease(int nBonus, int nDamageType=DAMAGE_TYPE_MAGICAL, int nVersusRace=-1);") 120, 1, 3, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDamageIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDamageIncrease) },
	{ NWSCRIPT_ACTIONNAME("RoundsToSeconds") NWSCRIPT_ACTIONPROTOTYPE("float RoundsToSeconds(int nRounds);") 121, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_RoundsToSeconds NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RoundsToSeconds) },
	{ NWSCRIPT_ACTIONNAME("HoursToSeconds") NWSCRIPT_ACTIONPROTOTYPE("float HoursToSeconds(int nHours);") 122, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_HoursToSeconds NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_HoursToSeconds) },
	{ NWSCRIPT_ACTIONNAME("TurnsToSeconds") NWSCRIPT_ACTIONPROTOTYPE("float TurnsToSeconds(int nTurns);") 123, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_TurnsToSeconds NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_TurnsToSeconds) },
	{ NWSCRIPT_ACTIONNAME("GetLawChaosValue") NWSCRIPT_ACTIONPROTOTYPE("int GetLawChaosValue(object oCreature);") 124, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLawChaosValue NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLawChaosValue) },
	{ NWSCRIPT_ACTIONNAME("GetGoodEvilValue") NWSCRIPT_ACTIONPROTOTYPE("int GetGoodEvilValue(object oCreature);") 125, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetGoodEvilValue NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetGoodEvilValue) },
	{ NWSCRIPT_ACTIONNAME("GetAlignmentLawChaos") NWSCRIPT_ACTIONPROTOTYPE("int GetAlignmentLawChaos(object oCreature);") 126, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetAlignmentLawChaos NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAlignmentLawChaos) },
	{ NWSCRIPT_ACTIONNAME("GetAlignmentGoodEvil") NWSCRIPT_ACTIONPROTOTYPE("int GetAlignmentGoodEvil(object oCreature);") 127, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetAlignmentGoodEvil NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAlignmentGoodEvil) },
	{ NWSCRIPT_ACTIONNAME("GetFirstObjectInShape") NWSCRIPT_ACTIONPROTOTYPE("object GetFirstObjectInShape(int nShape, float fSize, location lTarget, int bLineOfSight=FALSE, int nObjectFilter=OBJECT_TYPE_CREATURE, vector vOrigin=[0.0,0.0,0.0]);") 128, 3, 6, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetFirstObjectInShape NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFirstObjectInShape) },
	{ NWSCRIPT_ACTIONNAME("GetNextObjectInShape") NWSCRIPT_ACTIONPROTOTYPE("object GetNextObjectInShape(int nShape, float fSize, location lTarget, int bLineOfSight=FALSE, int nObjectFilter=OBJECT_TYPE_CREATURE, vector vOrigin=[0.0,0.0,0.0]);") 129, 3, 6, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetNextObjectInShape NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNextObjectInShape) },
	{ NWSCRIPT_ACTIONNAME("EffectEntangle") NWSCRIPT_ACTIONPROTOTYPE("effect EffectEntangle();") 130, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectEntangle NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectEntangle) },
	{ NWSCRIPT_ACTIONNAME("SignalEvent") NWSCRIPT_ACTIONPROTOTYPE("void SignalEvent(object oObject, event evToRun);") 131, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SignalEvent NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SignalEvent) },
	{ NWSCRIPT_ACTIONNAME("EventUserDefined") NWSCRIPT_ACTIONPROTOTYPE("event EventUserDefined(int nUserDefinedEventNumber);") 132, 1, 1, ACTIONTYPE_EVENT, NWN2_NWActionParameterTypes_EventUserDefined NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EventUserDefined) },
	{ NWSCRIPT_ACTIONNAME("EffectDeath") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDeath(int nSpectacularDeath=FALSE, int nDisplayFeedback=TRUE, int nIgnoreDeathImmunity=FALSE, int bPurgeEffects=TRUE);") 133, 0, 4, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDeath NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDeath) },
	{ NWSCRIPT_ACTIONNAME("EffectKnockdown") NWSCRIPT_ACTIONPROTOTYPE("effect EffectKnockdown();") 134, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectKnockdown NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectKnockdown) },
	{ NWSCRIPT_ACTIONNAME("ActionGiveItem") NWSCRIPT_ACTIONPROTOTYPE("void ActionGiveItem(object oItem, object oGiveTo, int bDisplayFeedback=TRUE);") 135, 2, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionGiveItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionGiveItem) },
	{ NWSCRIPT_ACTIONNAME("ActionTakeItem") NWSCRIPT_ACTIONPROTOTYPE("void ActionTakeItem(object oItem, object oTakeFrom, int bDisplayFeedback=TRUE);") 136, 2, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionTakeItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionTakeItem) },
	{ NWSCRIPT_ACTIONNAME("VectorNormalize") NWSCRIPT_ACTIONPROTOTYPE("vector VectorNormalize(vector vVector);") 137, 1, 1, ACTIONTYPE_VECTOR, NWN2_NWActionParameterTypes_VectorNormalize NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_VectorNormalize) },
	{ NWSCRIPT_ACTIONNAME("EffectCurse") NWSCRIPT_ACTIONPROTOTYPE("effect EffectCurse(int nStrMod=1, int nDexMod=1, int nConMod=1, int nIntMod=1, int nWisMod=1, int nChaMod=1);") 138, 0, 6, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectCurse NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectCurse) },
	{ NWSCRIPT_ACTIONNAME("GetAbilityScore") NWSCRIPT_ACTIONPROTOTYPE("int GetAbilityScore(object oCreature, int nAbilityType, int nBaseAttribute=FALSE);") 139, 2, 3, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetAbilityScore NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAbilityScore) },
	{ NWSCRIPT_ACTIONNAME("GetIsDead") NWSCRIPT_ACTIONPROTOTYPE("int GetIsDead(object oCreature, int bIgnoreDying=FALSE);") 140, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsDead NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsDead) },
	{ NWSCRIPT_ACTIONNAME("PrintVector") NWSCRIPT_ACTIONPROTOTYPE("void PrintVector(vector vVector, int bPrepend);") 141, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_PrintVector NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_PrintVector) },
	{ NWSCRIPT_ACTIONNAME("Vector") NWSCRIPT_ACTIONPROTOTYPE("vector Vector(float x=0.0f, float y=0.0f, float z=0.0f);") 142, 0, 3, ACTIONTYPE_VECTOR, NWN2_NWActionParameterTypes_Vector NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_Vector) },
	{ NWSCRIPT_ACTIONNAME("SetFacingPoint") NWSCRIPT_ACTIONPROTOTYPE("void SetFacingPoint(vector vTarget, int bLockToThisOrientation = FALSE);") 143, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetFacingPoint NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetFacingPoint) },
	{ NWSCRIPT_ACTIONNAME("AngleToVector") NWSCRIPT_ACTIONPROTOTYPE("vector AngleToVector(float fAngle);") 144, 1, 1, ACTIONTYPE_VECTOR, NWN2_NWActionParameterTypes_AngleToVector NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AngleToVector) },
	{ NWSCRIPT_ACTIONNAME("VectorToAngle") NWSCRIPT_ACTIONPROTOTYPE("float VectorToAngle(vector vVector);") 145, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_VectorToAngle NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_VectorToAngle) },
	{ NWSCRIPT_ACTIONNAME("TouchAttackMelee") NWSCRIPT_ACTIONPROTOTYPE("int TouchAttackMelee(object oTarget, int bDisplayFeedback=TRUE, int nBonus=0);") 146, 1, 3, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_TouchAttackMelee NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_TouchAttackMelee) },
	{ NWSCRIPT_ACTIONNAME("TouchAttackRanged") NWSCRIPT_ACTIONPROTOTYPE("int TouchAttackRanged(object oTarget, int bDisplayFeedback=TRUE, int nBonus=0);") 147, 1, 3, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_TouchAttackRanged NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_TouchAttackRanged) },
	{ NWSCRIPT_ACTIONNAME("EffectParalyze") NWSCRIPT_ACTIONPROTOTYPE("effect EffectParalyze(int nSaveDC=-1, int nSave=SAVING_THROW_WILL, int bSaveEveryRound = TRUE);") 148, 0, 3, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectParalyze NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectParalyze) },
	{ NWSCRIPT_ACTIONNAME("EffectSpellImmunity") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSpellImmunity(int nImmunityToSpell=SPELL_ALL_SPELLS);") 149, 0, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSpellImmunity NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSpellImmunity) },
	{ NWSCRIPT_ACTIONNAME("EffectDeaf") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDeaf();") 150, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDeaf NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDeaf) },
	{ NWSCRIPT_ACTIONNAME("GetDistanceBetween") NWSCRIPT_ACTIONPROTOTYPE("float GetDistanceBetween(object oObjectA, object oObjectB);") 151, 2, 2, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_GetDistanceBetween NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetDistanceBetween) },
	{ NWSCRIPT_ACTIONNAME("SetLocalLocation") NWSCRIPT_ACTIONPROTOTYPE("void SetLocalLocation(object oObject, string sVarName, location lValue);") 152, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetLocalLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetLocalLocation) },
	{ NWSCRIPT_ACTIONNAME("GetLocalLocation") NWSCRIPT_ACTIONPROTOTYPE("location GetLocalLocation(object oObject, string sVarName);") 153, 2, 2, ACTIONTYPE_LOCATION, NWN2_NWActionParameterTypes_GetLocalLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLocalLocation) },
	{ NWSCRIPT_ACTIONNAME("EffectSleep") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSleep();") 154, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSleep NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSleep) },
	{ NWSCRIPT_ACTIONNAME("GetItemInSlot") NWSCRIPT_ACTIONPROTOTYPE("object GetItemInSlot(int nInventorySlot, object oCreature=OBJECT_SELF);") 155, 1, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetItemInSlot NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemInSlot) },
	{ NWSCRIPT_ACTIONNAME("EffectCharmed") NWSCRIPT_ACTIONPROTOTYPE("effect EffectCharmed();") 156, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectCharmed NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectCharmed) },
	{ NWSCRIPT_ACTIONNAME("EffectConfused") NWSCRIPT_ACTIONPROTOTYPE("effect EffectConfused();") 157, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectConfused NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectConfused) },
	{ NWSCRIPT_ACTIONNAME("EffectFrightened") NWSCRIPT_ACTIONPROTOTYPE("effect EffectFrightened();") 158, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectFrightened NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectFrightened) },
	{ NWSCRIPT_ACTIONNAME("EffectDominated") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDominated();") 159, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDominated NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDominated) },
	{ NWSCRIPT_ACTIONNAME("EffectDazed") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDazed();") 160, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDazed NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDazed) },
	{ NWSCRIPT_ACTIONNAME("EffectStunned") NWSCRIPT_ACTIONPROTOTYPE("effect EffectStunned();") 161, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectStunned NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectStunned) },
	{ NWSCRIPT_ACTIONNAME("SetCommandable") NWSCRIPT_ACTIONPROTOTYPE("void SetCommandable(int bCommandable, object oTarget=OBJECT_SELF);") 162, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCommandable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCommandable) },
	{ NWSCRIPT_ACTIONNAME("GetCommandable") NWSCRIPT_ACTIONPROTOTYPE("int GetCommandable(object oTarget=OBJECT_SELF);") 163, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetCommandable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCommandable) },
	{ NWSCRIPT_ACTIONNAME("EffectRegenerate") NWSCRIPT_ACTIONPROTOTYPE("effect EffectRegenerate(int nAmount, float fIntervalSeconds);") 164, 2, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectRegenerate NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectRegenerate) },
	{ NWSCRIPT_ACTIONNAME("EffectMovementSpeedIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectMovementSpeedIncrease(int nPercentChange);") 165, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectMovementSpeedIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectMovementSpeedIncrease) },
	{ NWSCRIPT_ACTIONNAME("GetHitDice") NWSCRIPT_ACTIONPROTOTYPE("int GetHitDice(object oCreature);") 166, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetHitDice NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetHitDice) },
	{ NWSCRIPT_ACTIONNAME("ActionForceFollowObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionForceFollowObject(object oFollow, float fFollowDistance=0.5f, int iFollowPosition = 0);") 167, 1, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionForceFollowObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionForceFollowObject) },
	{ NWSCRIPT_ACTIONNAME("GetTag") NWSCRIPT_ACTIONPROTOTYPE("string GetTag(object oObject);") 168, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTag) },
	{ NWSCRIPT_ACTIONNAME("ResistSpell") NWSCRIPT_ACTIONPROTOTYPE("int ResistSpell(object oCaster, object oTarget);") 169, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_ResistSpell NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ResistSpell) },
	{ NWSCRIPT_ACTIONNAME("GetEffectType") NWSCRIPT_ACTIONPROTOTYPE("int GetEffectType(effect eEffect);") 170, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetEffectType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetEffectType) },
	{ NWSCRIPT_ACTIONNAME("EffectAreaOfEffect") NWSCRIPT_ACTIONPROTOTYPE("effect EffectAreaOfEffect(int nAreaEffectId, string sOnEnterScript="", string sHeartbeatScript="", string sOnExitScript="", string sEffectTag="" );") 171, 1, 5, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectAreaOfEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectAreaOfEffect) },
	{ NWSCRIPT_ACTIONNAME("GetFactionEqual") NWSCRIPT_ACTIONPROTOTYPE("int GetFactionEqual(object oFirstObject, object oSecondObject=OBJECT_SELF);") 172, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetFactionEqual NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFactionEqual) },
	{ NWSCRIPT_ACTIONNAME("ChangeFaction") NWSCRIPT_ACTIONPROTOTYPE("void ChangeFaction(object oObjectToChangeFaction, object oMemberOfFactionToJoin);") 173, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ChangeFaction NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ChangeFaction) },
	{ NWSCRIPT_ACTIONNAME("GetIsListening") NWSCRIPT_ACTIONPROTOTYPE("int GetIsListening(object oObject);") 174, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsListening NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsListening) },
	{ NWSCRIPT_ACTIONNAME("SetListening") NWSCRIPT_ACTIONPROTOTYPE("void SetListening(object oObject, int bValue);") 175, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetListening NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetListening) },
	{ NWSCRIPT_ACTIONNAME("SetListenPattern") NWSCRIPT_ACTIONPROTOTYPE("void SetListenPattern(object oObject, string sPattern, int nNumber=0);") 176, 2, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetListenPattern NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetListenPattern) },
	{ NWSCRIPT_ACTIONNAME("TestStringAgainstPattern") NWSCRIPT_ACTIONPROTOTYPE("int TestStringAgainstPattern(string sPattern, string sStringToTest);") 177, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_TestStringAgainstPattern NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_TestStringAgainstPattern) },
	{ NWSCRIPT_ACTIONNAME("GetMatchedSubstring") NWSCRIPT_ACTIONPROTOTYPE("string GetMatchedSubstring(int nString);") 178, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetMatchedSubstring NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetMatchedSubstring) },
	{ NWSCRIPT_ACTIONNAME("GetMatchedSubstringsCount") NWSCRIPT_ACTIONPROTOTYPE("int GetMatchedSubstringsCount();") 179, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetMatchedSubstringsCount NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetMatchedSubstringsCount) },
	{ NWSCRIPT_ACTIONNAME("EffectVisualEffect") NWSCRIPT_ACTIONPROTOTYPE("effect EffectVisualEffect(int nVisualEffectId, int nMissEffect=FALSE);") 180, 1, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectVisualEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectVisualEffect) },
	{ NWSCRIPT_ACTIONNAME("GetFactionWeakestMember") NWSCRIPT_ACTIONPROTOTYPE("object GetFactionWeakestMember(object oFactionMember=OBJECT_SELF, int bMustBeVisible=TRUE);") 181, 0, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetFactionWeakestMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFactionWeakestMember) },
	{ NWSCRIPT_ACTIONNAME("GetFactionStrongestMember") NWSCRIPT_ACTIONPROTOTYPE("object GetFactionStrongestMember(object oFactionMember=OBJECT_SELF, int bMustBeVisible=TRUE);") 182, 0, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetFactionStrongestMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFactionStrongestMember) },
	{ NWSCRIPT_ACTIONNAME("GetFactionMostDamagedMember") NWSCRIPT_ACTIONPROTOTYPE("object GetFactionMostDamagedMember(object oFactionMember=OBJECT_SELF, int bMustBeVisible=TRUE);") 183, 0, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetFactionMostDamagedMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFactionMostDamagedMember) },
	{ NWSCRIPT_ACTIONNAME("GetFactionLeastDamagedMember") NWSCRIPT_ACTIONPROTOTYPE("object GetFactionLeastDamagedMember(object oFactionMember=OBJECT_SELF, int bMustBeVisible=TRUE);") 184, 0, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetFactionLeastDamagedMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFactionLeastDamagedMember) },
	{ NWSCRIPT_ACTIONNAME("GetFactionGold") NWSCRIPT_ACTIONPROTOTYPE("int GetFactionGold(object oFactionMember);") 185, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetFactionGold NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFactionGold) },
	{ NWSCRIPT_ACTIONNAME("GetFactionAverageReputation") NWSCRIPT_ACTIONPROTOTYPE("int GetFactionAverageReputation(object oSourceFactionMember, object oTarget);") 186, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetFactionAverageReputation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFactionAverageReputation) },
	{ NWSCRIPT_ACTIONNAME("GetFactionAverageGoodEvilAlignment") NWSCRIPT_ACTIONPROTOTYPE("int GetFactionAverageGoodEvilAlignment(object oFactionMember);") 187, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetFactionAverageGoodEvilAlignment NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFactionAverageGoodEvilAlignment) },
	{ NWSCRIPT_ACTIONNAME("GetFactionAverageLawChaosAlignment") NWSCRIPT_ACTIONPROTOTYPE("int GetFactionAverageLawChaosAlignment(object oFactionMember);") 188, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetFactionAverageLawChaosAlignment NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFactionAverageLawChaosAlignment) },
	{ NWSCRIPT_ACTIONNAME("GetFactionAverageLevel") NWSCRIPT_ACTIONPROTOTYPE("int GetFactionAverageLevel(object oFactionMember);") 189, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetFactionAverageLevel NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFactionAverageLevel) },
	{ NWSCRIPT_ACTIONNAME("GetFactionAverageXP") NWSCRIPT_ACTIONPROTOTYPE("int GetFactionAverageXP(object oFactionMember);") 190, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetFactionAverageXP NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFactionAverageXP) },
	{ NWSCRIPT_ACTIONNAME("GetFactionMostFrequentClass") NWSCRIPT_ACTIONPROTOTYPE("int GetFactionMostFrequentClass(object oFactionMember);") 191, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetFactionMostFrequentClass NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFactionMostFrequentClass) },
	{ NWSCRIPT_ACTIONNAME("GetFactionWorstAC") NWSCRIPT_ACTIONPROTOTYPE("object GetFactionWorstAC(object oFactionMember=OBJECT_SELF, int bMustBeVisible=TRUE);") 192, 0, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetFactionWorstAC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFactionWorstAC) },
	{ NWSCRIPT_ACTIONNAME("GetFactionBestAC") NWSCRIPT_ACTIONPROTOTYPE("object GetFactionBestAC(object oFactionMember=OBJECT_SELF, int bMustBeVisible=TRUE);") 193, 0, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetFactionBestAC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFactionBestAC) },
	{ NWSCRIPT_ACTIONNAME("ActionSit") NWSCRIPT_ACTIONPROTOTYPE("void ActionSit(object oChair);") 194, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionSit NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionSit) },
	{ NWSCRIPT_ACTIONNAME("GetListenPatternNumber") NWSCRIPT_ACTIONPROTOTYPE("int GetListenPatternNumber();") 195, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetListenPatternNumber NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetListenPatternNumber) },
	{ NWSCRIPT_ACTIONNAME("ActionJumpToObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionJumpToObject(object oToJumpTo, int bWalkStraightLineToPoint=TRUE);") 196, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionJumpToObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionJumpToObject) },
	{ NWSCRIPT_ACTIONNAME("GetWaypointByTag") NWSCRIPT_ACTIONPROTOTYPE("object GetWaypointByTag(string sWaypointTag);") 197, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetWaypointByTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetWaypointByTag) },
	{ NWSCRIPT_ACTIONNAME("GetTransitionTarget") NWSCRIPT_ACTIONPROTOTYPE("object GetTransitionTarget(object oTransition);") 198, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetTransitionTarget NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTransitionTarget) },
	{ NWSCRIPT_ACTIONNAME("EffectLinkEffects") NWSCRIPT_ACTIONPROTOTYPE("effect EffectLinkEffects(effect eChildEffect, effect eParentEffect );") 199, 2, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectLinkEffects NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectLinkEffects) },
	{ NWSCRIPT_ACTIONNAME("GetObjectByTag") NWSCRIPT_ACTIONPROTOTYPE("object GetObjectByTag(string sTag, int nNth=0);") 200, 1, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetObjectByTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetObjectByTag) },
	{ NWSCRIPT_ACTIONNAME("AdjustAlignment") NWSCRIPT_ACTIONPROTOTYPE("void AdjustAlignment(object oSubject, int nAlignment, int nShift);") 201, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AdjustAlignment NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AdjustAlignment) },
	{ NWSCRIPT_ACTIONNAME("ActionWait") NWSCRIPT_ACTIONPROTOTYPE("void ActionWait(float fSeconds);") 202, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionWait NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionWait) },
	{ NWSCRIPT_ACTIONNAME("SetAreaTransitionBMP") NWSCRIPT_ACTIONPROTOTYPE("void SetAreaTransitionBMP(int nPredefinedAreaTransition, string sCustomAreaTransitionBMP="");") 203, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetAreaTransitionBMP NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetAreaTransitionBMP) },
	{ NWSCRIPT_ACTIONNAME("ActionStartConversation") NWSCRIPT_ACTIONPROTOTYPE("void ActionStartConversation(object oObjectToConverseWith, string sDialogResRef="", int bPrivateConversation=FALSE, int bPlayHello=TRUE, int bIgnoreStartDistance=FALSE, int bDisableCutsceneBars=FALSE);") 204, 1, 6, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionStartConversation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionStartConversation) },
	{ NWSCRIPT_ACTIONNAME("ActionPauseConversation") NWSCRIPT_ACTIONPROTOTYPE("void ActionPauseConversation();") 205, 0, 0, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionPauseConversation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionPauseConversation) },
	{ NWSCRIPT_ACTIONNAME("ActionResumeConversation") NWSCRIPT_ACTIONPROTOTYPE("void ActionResumeConversation();") 206, 0, 0, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionResumeConversation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionResumeConversation) },
	{ NWSCRIPT_ACTIONNAME("EffectBeam") NWSCRIPT_ACTIONPROTOTYPE("effect EffectBeam(int nBeamVisualEffect, object oEffector, int nBodyPart, int bMissEffect=FALSE);") 207, 3, 4, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectBeam NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectBeam) },
	{ NWSCRIPT_ACTIONNAME("GetReputation") NWSCRIPT_ACTIONPROTOTYPE("int GetReputation(object oSource, object oTarget);") 208, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetReputation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetReputation) },
	{ NWSCRIPT_ACTIONNAME("AdjustReputation") NWSCRIPT_ACTIONPROTOTYPE("void AdjustReputation(object oTarget, object oSourceFactionMember, int nAdjustment);") 209, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AdjustReputation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AdjustReputation) },
	{ NWSCRIPT_ACTIONNAME("GetSittingCreature") NWSCRIPT_ACTIONPROTOTYPE("object GetSittingCreature(object oChair);") 210, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetSittingCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetSittingCreature) },
	{ NWSCRIPT_ACTIONNAME("GetGoingToBeAttackedBy") NWSCRIPT_ACTIONPROTOTYPE("object GetGoingToBeAttackedBy(object oTarget);") 211, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetGoingToBeAttackedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetGoingToBeAttackedBy) },
	{ NWSCRIPT_ACTIONNAME("EffectSpellResistanceIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSpellResistanceIncrease(int nValue, int nUses = -1 );") 212, 1, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSpellResistanceIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSpellResistanceIncrease) },
	{ NWSCRIPT_ACTIONNAME("GetLocation") NWSCRIPT_ACTIONPROTOTYPE("location GetLocation(object oObject);") 213, 1, 1, ACTIONTYPE_LOCATION, NWN2_NWActionParameterTypes_GetLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLocation) },
	{ NWSCRIPT_ACTIONNAME("ActionJumpToLocation") NWSCRIPT_ACTIONPROTOTYPE("void ActionJumpToLocation(location lLocation);") 214, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionJumpToLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionJumpToLocation) },
	{ NWSCRIPT_ACTIONNAME("Location") NWSCRIPT_ACTIONPROTOTYPE("location Location(object oArea, vector vPosition, float fOrientation);") 215, 3, 3, ACTIONTYPE_LOCATION, NWN2_NWActionParameterTypes_Location NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_Location) },
	{ NWSCRIPT_ACTIONNAME("ApplyEffectAtLocation") NWSCRIPT_ACTIONPROTOTYPE("void ApplyEffectAtLocation(int nDurationType, effect eEffect, location lLocation, float fDuration=0.0f);") 216, 3, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ApplyEffectAtLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ApplyEffectAtLocation) },
	{ NWSCRIPT_ACTIONNAME("GetIsPC") NWSCRIPT_ACTIONPROTOTYPE("int GetIsPC(object oCreature);") 217, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsPC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsPC) },
	{ NWSCRIPT_ACTIONNAME("FeetToMeters") NWSCRIPT_ACTIONPROTOTYPE("float FeetToMeters(float fFeet);") 218, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_FeetToMeters NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_FeetToMeters) },
	{ NWSCRIPT_ACTIONNAME("YardsToMeters") NWSCRIPT_ACTIONPROTOTYPE("float YardsToMeters(float fYards);") 219, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_YardsToMeters NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_YardsToMeters) },
	{ NWSCRIPT_ACTIONNAME("ApplyEffectToObject") NWSCRIPT_ACTIONPROTOTYPE("void ApplyEffectToObject(int nDurationType, effect eEffect, object oTarget, float fDuration=0.0f);") 220, 3, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ApplyEffectToObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ApplyEffectToObject) },
	{ NWSCRIPT_ACTIONNAME("SpeakString") NWSCRIPT_ACTIONPROTOTYPE("void SpeakString(string sStringToSpeak, int nTalkVolume=TALKVOLUME_TALK);") 221, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SpeakString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SpeakString) },
	{ NWSCRIPT_ACTIONNAME("GetSpellTargetLocation") NWSCRIPT_ACTIONPROTOTYPE("location GetSpellTargetLocation();") 222, 0, 0, ACTIONTYPE_LOCATION, NWN2_NWActionParameterTypes_GetSpellTargetLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetSpellTargetLocation) },
	{ NWSCRIPT_ACTIONNAME("GetPositionFromLocation") NWSCRIPT_ACTIONPROTOTYPE("vector GetPositionFromLocation(location lLocation);") 223, 1, 1, ACTIONTYPE_VECTOR, NWN2_NWActionParameterTypes_GetPositionFromLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPositionFromLocation) },
	{ NWSCRIPT_ACTIONNAME("GetAreaFromLocation") NWSCRIPT_ACTIONPROTOTYPE("object GetAreaFromLocation(location lLocation);") 224, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetAreaFromLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAreaFromLocation) },
	{ NWSCRIPT_ACTIONNAME("GetFacingFromLocation") NWSCRIPT_ACTIONPROTOTYPE("float GetFacingFromLocation(location lLocation);") 225, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_GetFacingFromLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFacingFromLocation) },
	{ NWSCRIPT_ACTIONNAME("GetNearestCreatureToLocation") NWSCRIPT_ACTIONPROTOTYPE("object GetNearestCreatureToLocation(int nFirstCriteriaType, int nFirstCriteriaValue,  location lLocation, int nNth=1, int nSecondCriteriaType=-1, int nSecondCriteriaValue=-1, int nThirdCriteriaType=-1,  int nThirdCriteriaValue=-1 );") 226, 3, 8, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetNearestCreatureToLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNearestCreatureToLocation) },
	{ NWSCRIPT_ACTIONNAME("GetNearestObject") NWSCRIPT_ACTIONPROTOTYPE("object GetNearestObject(int nObjectType=OBJECT_TYPE_ALL, object oTarget=OBJECT_SELF, int nNth=1);") 227, 0, 3, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetNearestObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNearestObject) },
	{ NWSCRIPT_ACTIONNAME("GetNearestObjectToLocation") NWSCRIPT_ACTIONPROTOTYPE("object GetNearestObjectToLocation(int nObjectType, location lLocation, int nNth=1);") 228, 2, 3, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetNearestObjectToLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNearestObjectToLocation) },
	{ NWSCRIPT_ACTIONNAME("GetNearestObjectByTag") NWSCRIPT_ACTIONPROTOTYPE("object GetNearestObjectByTag(string sTag, object oTarget=OBJECT_SELF, int nNth=1);") 229, 1, 3, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetNearestObjectByTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNearestObjectByTag) },
	{ NWSCRIPT_ACTIONNAME("IntToFloat") NWSCRIPT_ACTIONPROTOTYPE("float IntToFloat(int nInteger);") 230, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_IntToFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_IntToFloat) },
	{ NWSCRIPT_ACTIONNAME("FloatToInt") NWSCRIPT_ACTIONPROTOTYPE("int FloatToInt(float fFloat);") 231, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_FloatToInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_FloatToInt) },
	{ NWSCRIPT_ACTIONNAME("StringToInt") NWSCRIPT_ACTIONPROTOTYPE("int StringToInt(string sNumber);") 232, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_StringToInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_StringToInt) },
	{ NWSCRIPT_ACTIONNAME("StringToFloat") NWSCRIPT_ACTIONPROTOTYPE("float StringToFloat(string sNumber);") 233, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_StringToFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_StringToFloat) },
	{ NWSCRIPT_ACTIONNAME("ActionCastSpellAtLocation") NWSCRIPT_ACTIONPROTOTYPE("void   ActionCastSpellAtLocation(int nSpell, location lTargetLocation, int nMetaMagic=METAMAGIC_ANY, int bCheat=FALSE, int nProjectilePathType=PROJECTILE_PATH_TYPE_DEFAULT, int bInstantSpell=FALSE, int nDomainLevel=0);") 234, 2, 7, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionCastSpellAtLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionCastSpellAtLocation) },
	{ NWSCRIPT_ACTIONNAME("GetIsEnemy") NWSCRIPT_ACTIONPROTOTYPE("int GetIsEnemy(object oTarget, object oSource=OBJECT_SELF);") 235, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsEnemy NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsEnemy) },
	{ NWSCRIPT_ACTIONNAME("GetIsFriend") NWSCRIPT_ACTIONPROTOTYPE("int GetIsFriend(object oTarget, object oSource=OBJECT_SELF);") 236, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsFriend NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsFriend) },
	{ NWSCRIPT_ACTIONNAME("GetIsNeutral") NWSCRIPT_ACTIONPROTOTYPE("int GetIsNeutral(object oTarget, object oSource=OBJECT_SELF);") 237, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsNeutral NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsNeutral) },
	{ NWSCRIPT_ACTIONNAME("GetPCSpeaker") NWSCRIPT_ACTIONPROTOTYPE("object GetPCSpeaker();") 238, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetPCSpeaker NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPCSpeaker) },
	{ NWSCRIPT_ACTIONNAME("GetStringByStrRef") NWSCRIPT_ACTIONPROTOTYPE("string GetStringByStrRef(int nStrRef, int nGender=GENDER_MALE);") 239, 1, 2, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetStringByStrRef NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetStringByStrRef) },
	{ NWSCRIPT_ACTIONNAME("ActionSpeakStringByStrRef") NWSCRIPT_ACTIONPROTOTYPE("void ActionSpeakStringByStrRef(int nStrRef, int nTalkVolume=TALKVOLUME_TALK);") 240, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionSpeakStringByStrRef NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionSpeakStringByStrRef) },
	{ NWSCRIPT_ACTIONNAME("DestroyObject") NWSCRIPT_ACTIONPROTOTYPE("void DestroyObject(object oDestroy, float fDelay=0.0f, int nDisplayFeedback=TRUE);") 241, 1, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DestroyObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DestroyObject) },
	{ NWSCRIPT_ACTIONNAME("GetModule") NWSCRIPT_ACTIONPROTOTYPE("object GetModule();") 242, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetModule NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetModule) },
	{ NWSCRIPT_ACTIONNAME("CreateObject") NWSCRIPT_ACTIONPROTOTYPE("object CreateObject(int nObjectType, string sTemplate, location lLocation, int bUseAppearAnimation=FALSE, string sNewTag="");") 243, 3, 5, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_CreateObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_CreateObject) },
	{ NWSCRIPT_ACTIONNAME("EventSpellCastAt") NWSCRIPT_ACTIONPROTOTYPE("event EventSpellCastAt(object oCaster, int nSpell, int bHarmful=TRUE);") 244, 2, 3, ACTIONTYPE_EVENT, NWN2_NWActionParameterTypes_EventSpellCastAt NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EventSpellCastAt) },
	{ NWSCRIPT_ACTIONNAME("GetLastSpellCaster") NWSCRIPT_ACTIONPROTOTYPE("object GetLastSpellCaster();") 245, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastSpellCaster NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastSpellCaster) },
	{ NWSCRIPT_ACTIONNAME("GetLastSpell") NWSCRIPT_ACTIONPROTOTYPE("int GetLastSpell();") 246, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLastSpell NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastSpell) },
	{ NWSCRIPT_ACTIONNAME("GetUserDefinedEventNumber") NWSCRIPT_ACTIONPROTOTYPE("int GetUserDefinedEventNumber();") 247, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetUserDefinedEventNumber NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetUserDefinedEventNumber) },
	{ NWSCRIPT_ACTIONNAME("GetSpellId") NWSCRIPT_ACTIONPROTOTYPE("int GetSpellId();") 248, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetSpellId NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetSpellId) },
	{ NWSCRIPT_ACTIONNAME("RandomName") NWSCRIPT_ACTIONPROTOTYPE("string RandomName();") 249, 0, 0, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_RandomName NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RandomName) },
	{ NWSCRIPT_ACTIONNAME("EffectPoison") NWSCRIPT_ACTIONPROTOTYPE("effect EffectPoison(int nPoisonType);") 250, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectPoison NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectPoison) },
	{ NWSCRIPT_ACTIONNAME("EffectDisease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDisease(int nDiseaseType);") 251, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDisease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDisease) },
	{ NWSCRIPT_ACTIONNAME("EffectSilence") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSilence();") 252, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSilence NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSilence) },
	{ NWSCRIPT_ACTIONNAME("GetName") NWSCRIPT_ACTIONPROTOTYPE("string GetName(object oObject);") 253, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetName NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetName) },
	{ NWSCRIPT_ACTIONNAME("GetLastSpeaker") NWSCRIPT_ACTIONPROTOTYPE("object GetLastSpeaker();") 254, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastSpeaker NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastSpeaker) },
	{ NWSCRIPT_ACTIONNAME("BeginConversation") NWSCRIPT_ACTIONPROTOTYPE("int BeginConversation(string sResRef="", object oObjectToDialog=OBJECT_INVALID, int bPreventHello=FALSE);") 255, 0, 3, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_BeginConversation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_BeginConversation) },
	{ NWSCRIPT_ACTIONNAME("GetLastPerceived") NWSCRIPT_ACTIONPROTOTYPE("object GetLastPerceived();") 256, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastPerceived NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastPerceived) },
	{ NWSCRIPT_ACTIONNAME("GetLastPerceptionHeard") NWSCRIPT_ACTIONPROTOTYPE("int GetLastPerceptionHeard();") 257, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLastPerceptionHeard NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastPerceptionHeard) },
	{ NWSCRIPT_ACTIONNAME("GetLastPerceptionInaudible") NWSCRIPT_ACTIONPROTOTYPE("int GetLastPerceptionInaudible();") 258, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLastPerceptionInaudible NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastPerceptionInaudible) },
	{ NWSCRIPT_ACTIONNAME("GetLastPerceptionSeen") NWSCRIPT_ACTIONPROTOTYPE("int GetLastPerceptionSeen();") 259, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLastPerceptionSeen NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastPerceptionSeen) },
	{ NWSCRIPT_ACTIONNAME("GetLastClosedBy") NWSCRIPT_ACTIONPROTOTYPE("object GetLastClosedBy();") 260, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastClosedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastClosedBy) },
	{ NWSCRIPT_ACTIONNAME("GetLastPerceptionVanished") NWSCRIPT_ACTIONPROTOTYPE("int GetLastPerceptionVanished();") 261, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLastPerceptionVanished NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastPerceptionVanished) },
	{ NWSCRIPT_ACTIONNAME("GetFirstInPersistentObject") NWSCRIPT_ACTIONPROTOTYPE("object GetFirstInPersistentObject(object oPersistentObject=OBJECT_SELF, int nResidentObjectType=OBJECT_TYPE_CREATURE, int nPersistentZone=PERSISTENT_ZONE_ACTIVE);") 262, 0, 3, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetFirstInPersistentObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFirstInPersistentObject) },
	{ NWSCRIPT_ACTIONNAME("GetNextInPersistentObject") NWSCRIPT_ACTIONPROTOTYPE("object GetNextInPersistentObject(object oPersistentObject=OBJECT_SELF, int nResidentObjectType=OBJECT_TYPE_CREATURE, int nPersistentZone=PERSISTENT_ZONE_ACTIVE);") 263, 0, 3, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetNextInPersistentObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNextInPersistentObject) },
	{ NWSCRIPT_ACTIONNAME("GetAreaOfEffectCreator") NWSCRIPT_ACTIONPROTOTYPE("object GetAreaOfEffectCreator(object oAreaOfEffectObject=OBJECT_SELF);") 264, 0, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetAreaOfEffectCreator NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAreaOfEffectCreator) },
	{ NWSCRIPT_ACTIONNAME("DeleteLocalInt") NWSCRIPT_ACTIONPROTOTYPE("void DeleteLocalInt(object oObject, string sVarName);") 265, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DeleteLocalInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DeleteLocalInt) },
	{ NWSCRIPT_ACTIONNAME("DeleteLocalFloat") NWSCRIPT_ACTIONPROTOTYPE("void DeleteLocalFloat(object oObject, string sVarName);") 266, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DeleteLocalFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DeleteLocalFloat) },
	{ NWSCRIPT_ACTIONNAME("DeleteLocalString") NWSCRIPT_ACTIONPROTOTYPE("void DeleteLocalString(object oObject, string sVarName);") 267, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DeleteLocalString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DeleteLocalString) },
	{ NWSCRIPT_ACTIONNAME("DeleteLocalObject") NWSCRIPT_ACTIONPROTOTYPE("void DeleteLocalObject(object oObject, string sVarName);") 268, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DeleteLocalObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DeleteLocalObject) },
	{ NWSCRIPT_ACTIONNAME("DeleteLocalLocation") NWSCRIPT_ACTIONPROTOTYPE("void DeleteLocalLocation(object oObject, string sVarName);") 269, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DeleteLocalLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DeleteLocalLocation) },
	{ NWSCRIPT_ACTIONNAME("EffectHaste") NWSCRIPT_ACTIONPROTOTYPE("effect EffectHaste();") 270, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectHaste NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectHaste) },
	{ NWSCRIPT_ACTIONNAME("EffectSlow") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSlow();") 271, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSlow NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSlow) },
	{ NWSCRIPT_ACTIONNAME("ObjectToString") NWSCRIPT_ACTIONPROTOTYPE("string ObjectToString(object oObject);") 272, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_ObjectToString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ObjectToString) },
	{ NWSCRIPT_ACTIONNAME("EffectImmunity") NWSCRIPT_ACTIONPROTOTYPE("effect EffectImmunity(int nImmunityType);") 273, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectImmunity NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectImmunity) },
	{ NWSCRIPT_ACTIONNAME("GetIsImmune") NWSCRIPT_ACTIONPROTOTYPE("int GetIsImmune(object oCreature, int nImmunityType, object oVersus=OBJECT_INVALID);") 274, 2, 3, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsImmune NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsImmune) },
	{ NWSCRIPT_ACTIONNAME("EffectDamageImmunityIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamageImmunityIncrease(int nDamageType, int nPercentImmunity);") 275, 2, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDamageImmunityIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDamageImmunityIncrease) },
	{ NWSCRIPT_ACTIONNAME("GetEncounterActive") NWSCRIPT_ACTIONPROTOTYPE("int  GetEncounterActive(object oEncounter=OBJECT_SELF);") 276, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetEncounterActive NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetEncounterActive) },
	{ NWSCRIPT_ACTIONNAME("SetEncounterActive") NWSCRIPT_ACTIONPROTOTYPE("void SetEncounterActive(int nNewValue, object oEncounter=OBJECT_SELF);") 277, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetEncounterActive NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetEncounterActive) },
	{ NWSCRIPT_ACTIONNAME("GetEncounterSpawnsMax") NWSCRIPT_ACTIONPROTOTYPE("int GetEncounterSpawnsMax(object oEncounter=OBJECT_SELF);") 278, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetEncounterSpawnsMax NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetEncounterSpawnsMax) },
	{ NWSCRIPT_ACTIONNAME("SetEncounterSpawnsMax") NWSCRIPT_ACTIONPROTOTYPE("void SetEncounterSpawnsMax(int nNewValue, object oEncounter=OBJECT_SELF);") 279, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetEncounterSpawnsMax NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetEncounterSpawnsMax) },
	{ NWSCRIPT_ACTIONNAME("GetEncounterSpawnsCurrent") NWSCRIPT_ACTIONPROTOTYPE("int  GetEncounterSpawnsCurrent(object oEncounter=OBJECT_SELF);") 280, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetEncounterSpawnsCurrent NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetEncounterSpawnsCurrent) },
	{ NWSCRIPT_ACTIONNAME("SetEncounterSpawnsCurrent") NWSCRIPT_ACTIONPROTOTYPE("void SetEncounterSpawnsCurrent(int nNewValue, object oEncounter=OBJECT_SELF);") 281, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetEncounterSpawnsCurrent NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetEncounterSpawnsCurrent) },
	{ NWSCRIPT_ACTIONNAME("GetModuleItemAcquired") NWSCRIPT_ACTIONPROTOTYPE("object GetModuleItemAcquired();") 282, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetModuleItemAcquired NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetModuleItemAcquired) },
	{ NWSCRIPT_ACTIONNAME("GetModuleItemAcquiredFrom") NWSCRIPT_ACTIONPROTOTYPE("object GetModuleItemAcquiredFrom();") 283, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetModuleItemAcquiredFrom NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetModuleItemAcquiredFrom) },
	{ NWSCRIPT_ACTIONNAME("SetCustomToken") NWSCRIPT_ACTIONPROTOTYPE("void SetCustomToken(int nCustomTokenNumber, string sTokenValue);") 284, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCustomToken NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCustomToken) },
	{ NWSCRIPT_ACTIONNAME("GetHasFeat") NWSCRIPT_ACTIONPROTOTYPE("int GetHasFeat(int nFeat, object oCreature=OBJECT_SELF, int nIgnoreUses=FALSE);") 285, 1, 3, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetHasFeat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetHasFeat) },
	{ NWSCRIPT_ACTIONNAME("GetHasSkill") NWSCRIPT_ACTIONPROTOTYPE("int GetHasSkill(int nSkill, object oCreature=OBJECT_SELF);") 286, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetHasSkill NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetHasSkill) },
	{ NWSCRIPT_ACTIONNAME("ActionUseFeat") NWSCRIPT_ACTIONPROTOTYPE("void ActionUseFeat(int nFeat, object oTarget);") 287, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionUseFeat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionUseFeat) },
	{ NWSCRIPT_ACTIONNAME("ActionUseSkill") NWSCRIPT_ACTIONPROTOTYPE("int ActionUseSkill(int nSkill, object oTarget, int nSubSkill=0, object oItemUsed=OBJECT_INVALID );") 288, 2, 4, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_ActionUseSkill NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionUseSkill) },
	{ NWSCRIPT_ACTIONNAME("GetObjectSeen") NWSCRIPT_ACTIONPROTOTYPE("int GetObjectSeen(object oTarget, object oSource=OBJECT_SELF);") 289, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetObjectSeen NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetObjectSeen) },
	{ NWSCRIPT_ACTIONNAME("GetObjectHeard") NWSCRIPT_ACTIONPROTOTYPE("int GetObjectHeard(object oTarget, object oSource=OBJECT_SELF);") 290, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetObjectHeard NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetObjectHeard) },
	{ NWSCRIPT_ACTIONNAME("GetLastPlayerDied") NWSCRIPT_ACTIONPROTOTYPE("object GetLastPlayerDied();") 291, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastPlayerDied NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastPlayerDied) },
	{ NWSCRIPT_ACTIONNAME("GetModuleItemLost") NWSCRIPT_ACTIONPROTOTYPE("object GetModuleItemLost();") 292, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetModuleItemLost NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetModuleItemLost) },
	{ NWSCRIPT_ACTIONNAME("GetModuleItemLostBy") NWSCRIPT_ACTIONPROTOTYPE("object GetModuleItemLostBy();") 293, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetModuleItemLostBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetModuleItemLostBy) },
	{ NWSCRIPT_ACTIONNAME("ActionDoCommand") NWSCRIPT_ACTIONPROTOTYPE("void ActionDoCommand(action aActionToDo);") 294, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionDoCommand NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionDoCommand) },
	{ NWSCRIPT_ACTIONNAME("EventConversation") NWSCRIPT_ACTIONPROTOTYPE("event EventConversation();") 295, 0, 0, ACTIONTYPE_EVENT, NWN2_NWActionParameterTypes_EventConversation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EventConversation) },
	{ NWSCRIPT_ACTIONNAME("SetEncounterDifficulty") NWSCRIPT_ACTIONPROTOTYPE("void SetEncounterDifficulty(int nEncounterDifficulty, object oEncounter=OBJECT_SELF);") 296, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetEncounterDifficulty NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetEncounterDifficulty) },
	{ NWSCRIPT_ACTIONNAME("GetEncounterDifficulty") NWSCRIPT_ACTIONPROTOTYPE("int GetEncounterDifficulty(object oEncounter=OBJECT_SELF);") 297, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetEncounterDifficulty NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetEncounterDifficulty) },
	{ NWSCRIPT_ACTIONNAME("GetDistanceBetweenLocations") NWSCRIPT_ACTIONPROTOTYPE("float GetDistanceBetweenLocations(location lLocationA, location lLocationB);") 298, 2, 2, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_GetDistanceBetweenLocations NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetDistanceBetweenLocations) },
	{ NWSCRIPT_ACTIONNAME("GetReflexAdjustedDamage") NWSCRIPT_ACTIONPROTOTYPE("int GetReflexAdjustedDamage(int nDamage, object oTarget, int nDC, int nSaveType=SAVING_THROW_TYPE_NONE, object oSaveVersus=OBJECT_SELF);") 299, 3, 5, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetReflexAdjustedDamage NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetReflexAdjustedDamage) },
	{ NWSCRIPT_ACTIONNAME("PlayAnimation") NWSCRIPT_ACTIONPROTOTYPE("void PlayAnimation(int nAnimation, float fSpeed=1.0, float fSeconds=0.0);") 300, 1, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_PlayAnimation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_PlayAnimation) },
	{ NWSCRIPT_ACTIONNAME("TalentSpell") NWSCRIPT_ACTIONPROTOTYPE("talent TalentSpell(int nSpell);") 301, 1, 1, ACTIONTYPE_TALENT, NWN2_NWActionParameterTypes_TalentSpell NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_TalentSpell) },
	{ NWSCRIPT_ACTIONNAME("TalentFeat") NWSCRIPT_ACTIONPROTOTYPE("talent TalentFeat(int nFeat);") 302, 1, 1, ACTIONTYPE_TALENT, NWN2_NWActionParameterTypes_TalentFeat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_TalentFeat) },
	{ NWSCRIPT_ACTIONNAME("TalentSkill") NWSCRIPT_ACTIONPROTOTYPE("talent TalentSkill(int nSkill);") 303, 1, 1, ACTIONTYPE_TALENT, NWN2_NWActionParameterTypes_TalentSkill NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_TalentSkill) },
	{ NWSCRIPT_ACTIONNAME("GetHasSpellEffect") NWSCRIPT_ACTIONPROTOTYPE("int GetHasSpellEffect(int nSpell, object oObject=OBJECT_SELF);") 304, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetHasSpellEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetHasSpellEffect) },
	{ NWSCRIPT_ACTIONNAME("GetEffectSpellId") NWSCRIPT_ACTIONPROTOTYPE("int GetEffectSpellId(effect eSpellEffect);") 305, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetEffectSpellId NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetEffectSpellId) },
	{ NWSCRIPT_ACTIONNAME("GetCreatureHasTalent") NWSCRIPT_ACTIONPROTOTYPE("int GetCreatureHasTalent(talent tTalent, object oCreature=OBJECT_SELF);") 306, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetCreatureHasTalent NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCreatureHasTalent) },
	{ NWSCRIPT_ACTIONNAME("GetCreatureTalentRandom") NWSCRIPT_ACTIONPROTOTYPE("talent GetCreatureTalentRandom(int nCategory, object oCreature=OBJECT_SELF, int iExcludedTalentsFlag = 0);") 307, 1, 3, ACTIONTYPE_TALENT, NWN2_NWActionParameterTypes_GetCreatureTalentRandom NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCreatureTalentRandom) },
	{ NWSCRIPT_ACTIONNAME("GetCreatureTalentBest") NWSCRIPT_ACTIONPROTOTYPE("talent GetCreatureTalentBest(int nCategory, int nCRMax, object oCreature=OBJECT_SELF, int iExcludedTalentsFlag = 0);") 308, 2, 4, ACTIONTYPE_TALENT, NWN2_NWActionParameterTypes_GetCreatureTalentBest NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCreatureTalentBest) },
	{ NWSCRIPT_ACTIONNAME("ActionUseTalentOnObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionUseTalentOnObject(talent tChosenTalent, object oTarget);") 309, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionUseTalentOnObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionUseTalentOnObject) },
	{ NWSCRIPT_ACTIONNAME("ActionUseTalentAtLocation") NWSCRIPT_ACTIONPROTOTYPE("void ActionUseTalentAtLocation(talent tChosenTalent, location lTargetLocation);") 310, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionUseTalentAtLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionUseTalentAtLocation) },
	{ NWSCRIPT_ACTIONNAME("GetGoldPieceValue") NWSCRIPT_ACTIONPROTOTYPE("int GetGoldPieceValue(object oItem);") 311, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetGoldPieceValue NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetGoldPieceValue) },
	{ NWSCRIPT_ACTIONNAME("GetIsPlayableRacialType") NWSCRIPT_ACTIONPROTOTYPE("int GetIsPlayableRacialType(object oCreature);") 312, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsPlayableRacialType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsPlayableRacialType) },
	{ NWSCRIPT_ACTIONNAME("JumpToLocation") NWSCRIPT_ACTIONPROTOTYPE("void JumpToLocation(location lDestination);") 313, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_JumpToLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_JumpToLocation) },
	{ NWSCRIPT_ACTIONNAME("EffectTemporaryHitpoints") NWSCRIPT_ACTIONPROTOTYPE("effect EffectTemporaryHitpoints(int nHitPoints);") 314, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectTemporaryHitpoints NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectTemporaryHitpoints) },
	{ NWSCRIPT_ACTIONNAME("GetSkillRank") NWSCRIPT_ACTIONPROTOTYPE("int GetSkillRank(int nSkill, object oTarget=OBJECT_SELF, int bBaseOnly=FALSE );") 315, 1, 3, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetSkillRank NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetSkillRank) },
	{ NWSCRIPT_ACTIONNAME("GetAttackTarget") NWSCRIPT_ACTIONPROTOTYPE("object GetAttackTarget(object oCreature=OBJECT_SELF);") 316, 0, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetAttackTarget NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAttackTarget) },
	{ NWSCRIPT_ACTIONNAME("GetLastAttackType") NWSCRIPT_ACTIONPROTOTYPE("int GetLastAttackType(object oCreature=OBJECT_SELF);") 317, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLastAttackType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastAttackType) },
	{ NWSCRIPT_ACTIONNAME("GetLastAttackMode") NWSCRIPT_ACTIONPROTOTYPE("int GetLastAttackMode(object oCreature=OBJECT_SELF);") 318, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLastAttackMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastAttackMode) },
	{ NWSCRIPT_ACTIONNAME("GetMaster") NWSCRIPT_ACTIONPROTOTYPE("object GetMaster(object oAssociate=OBJECT_SELF);") 319, 0, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetMaster NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetMaster) },
	{ NWSCRIPT_ACTIONNAME("GetIsInCombat") NWSCRIPT_ACTIONPROTOTYPE("int GetIsInCombat(object oCreature=OBJECT_SELF);") 320, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsInCombat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsInCombat) },
	{ NWSCRIPT_ACTIONNAME("GetLastAssociateCommand") NWSCRIPT_ACTIONPROTOTYPE("int GetLastAssociateCommand(object oAssociate=OBJECT_SELF);") 321, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLastAssociateCommand NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastAssociateCommand) },
	{ NWSCRIPT_ACTIONNAME("GiveGoldToCreature") NWSCRIPT_ACTIONPROTOTYPE("void GiveGoldToCreature(object oCreature, int nGP, int bDisplayFeedback=TRUE );") 322, 2, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_GiveGoldToCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GiveGoldToCreature) },
	{ NWSCRIPT_ACTIONNAME("SetIsDestroyable") NWSCRIPT_ACTIONPROTOTYPE("void SetIsDestroyable(int bDestroyable, int bRaiseable=TRUE, int bSelectableWhenDead=FALSE);") 323, 1, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetIsDestroyable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetIsDestroyable) },
	{ NWSCRIPT_ACTIONNAME("SetLocked") NWSCRIPT_ACTIONPROTOTYPE("void SetLocked(object oTarget, int bLocked);") 324, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetLocked NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetLocked) },
	{ NWSCRIPT_ACTIONNAME("GetLocked") NWSCRIPT_ACTIONPROTOTYPE("int GetLocked(object oTarget);") 325, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLocked NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLocked) },
	{ NWSCRIPT_ACTIONNAME("GetClickingObject") NWSCRIPT_ACTIONPROTOTYPE("object GetClickingObject();") 326, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetClickingObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetClickingObject) },
	{ NWSCRIPT_ACTIONNAME("SetAssociateListenPatterns") NWSCRIPT_ACTIONPROTOTYPE("void SetAssociateListenPatterns( object oTarget=OBJECT_SELF );") 327, 0, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetAssociateListenPatterns NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetAssociateListenPatterns) },
	{ NWSCRIPT_ACTIONNAME("GetLastWeaponUsed") NWSCRIPT_ACTIONPROTOTYPE("object GetLastWeaponUsed(object oCreature);") 328, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastWeaponUsed NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastWeaponUsed) },
	{ NWSCRIPT_ACTIONNAME("ActionInteractObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionInteractObject(object oPlaceable);") 329, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionInteractObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionInteractObject) },
	{ NWSCRIPT_ACTIONNAME("GetLastUsedBy") NWSCRIPT_ACTIONPROTOTYPE("object GetLastUsedBy();") 330, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastUsedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastUsedBy) },
	{ NWSCRIPT_ACTIONNAME("GetAbilityModifier") NWSCRIPT_ACTIONPROTOTYPE("int GetAbilityModifier(int nAbility, object oCreature=OBJECT_SELF);") 331, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetAbilityModifier NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAbilityModifier) },
	{ NWSCRIPT_ACTIONNAME("GetIdentified") NWSCRIPT_ACTIONPROTOTYPE("int GetIdentified(object oItem);") 332, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIdentified NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIdentified) },
	{ NWSCRIPT_ACTIONNAME("SetIdentified") NWSCRIPT_ACTIONPROTOTYPE("void SetIdentified(object oItem, int bIdentified);") 333, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetIdentified NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetIdentified) },
	{ NWSCRIPT_ACTIONNAME("SummonAnimalCompanion") NWSCRIPT_ACTIONPROTOTYPE("void SummonAnimalCompanion(object oMaster=OBJECT_SELF, string sResRef = "");") 334, 0, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SummonAnimalCompanion NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SummonAnimalCompanion) },
	{ NWSCRIPT_ACTIONNAME("SummonFamiliar") NWSCRIPT_ACTIONPROTOTYPE("void SummonFamiliar(object oMaster=OBJECT_SELF, string sResRef = "");") 335, 0, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SummonFamiliar NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SummonFamiliar) },
	{ NWSCRIPT_ACTIONNAME("GetBlockingDoor") NWSCRIPT_ACTIONPROTOTYPE("object GetBlockingDoor();") 336, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetBlockingDoor NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetBlockingDoor) },
	{ NWSCRIPT_ACTIONNAME("GetIsDoorActionPossible") NWSCRIPT_ACTIONPROTOTYPE("int GetIsDoorActionPossible(object oTargetDoor, int nDoorAction);") 337, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsDoorActionPossible NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsDoorActionPossible) },
	{ NWSCRIPT_ACTIONNAME("DoDoorAction") NWSCRIPT_ACTIONPROTOTYPE("void DoDoorAction(object oTargetDoor, int nDoorAction);") 338, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DoDoorAction NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DoDoorAction) },
	{ NWSCRIPT_ACTIONNAME("GetFirstItemInInventory") NWSCRIPT_ACTIONPROTOTYPE("object GetFirstItemInInventory(object oTarget=OBJECT_SELF);") 339, 0, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetFirstItemInInventory NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFirstItemInInventory) },
	{ NWSCRIPT_ACTIONNAME("GetNextItemInInventory") NWSCRIPT_ACTIONPROTOTYPE("object GetNextItemInInventory(object oTarget=OBJECT_SELF);") 340, 0, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetNextItemInInventory NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNextItemInInventory) },
	{ NWSCRIPT_ACTIONNAME("GetClassByPosition") NWSCRIPT_ACTIONPROTOTYPE("int GetClassByPosition(int nClassPosition, object oCreature=OBJECT_SELF);") 341, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetClassByPosition NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetClassByPosition) },
	{ NWSCRIPT_ACTIONNAME("GetLevelByPosition") NWSCRIPT_ACTIONPROTOTYPE("int GetLevelByPosition(int nClassPosition, object oCreature=OBJECT_SELF);") 342, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLevelByPosition NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLevelByPosition) },
	{ NWSCRIPT_ACTIONNAME("GetLevelByClass") NWSCRIPT_ACTIONPROTOTYPE("int GetLevelByClass(int nClassType, object oCreature=OBJECT_SELF);") 343, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLevelByClass NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLevelByClass) },
	{ NWSCRIPT_ACTIONNAME("GetDamageDealtByType") NWSCRIPT_ACTIONPROTOTYPE("int GetDamageDealtByType(int nDamageType);") 344, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetDamageDealtByType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetDamageDealtByType) },
	{ NWSCRIPT_ACTIONNAME("GetTotalDamageDealt") NWSCRIPT_ACTIONPROTOTYPE("int GetTotalDamageDealt();") 345, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTotalDamageDealt NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTotalDamageDealt) },
	{ NWSCRIPT_ACTIONNAME("GetLastDamager") NWSCRIPT_ACTIONPROTOTYPE("object GetLastDamager(object oObject=OBJECT_SELF);") 346, 0, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastDamager NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastDamager) },
	{ NWSCRIPT_ACTIONNAME("GetLastDisarmed") NWSCRIPT_ACTIONPROTOTYPE("object GetLastDisarmed();") 347, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastDisarmed NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastDisarmed) },
	{ NWSCRIPT_ACTIONNAME("GetLastDisturbed") NWSCRIPT_ACTIONPROTOTYPE("object GetLastDisturbed();") 348, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastDisturbed NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastDisturbed) },
	{ NWSCRIPT_ACTIONNAME("GetLastLocked") NWSCRIPT_ACTIONPROTOTYPE("object GetLastLocked();") 349, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastLocked NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastLocked) },
	{ NWSCRIPT_ACTIONNAME("GetLastUnlocked") NWSCRIPT_ACTIONPROTOTYPE("object GetLastUnlocked();") 350, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastUnlocked NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastUnlocked) },
	{ NWSCRIPT_ACTIONNAME("EffectSkillIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSkillIncrease(int nSkill, int nValue);") 351, 2, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSkillIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSkillIncrease) },
	{ NWSCRIPT_ACTIONNAME("GetInventoryDisturbType") NWSCRIPT_ACTIONPROTOTYPE("int GetInventoryDisturbType();") 352, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetInventoryDisturbType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetInventoryDisturbType) },
	{ NWSCRIPT_ACTIONNAME("GetInventoryDisturbItem") NWSCRIPT_ACTIONPROTOTYPE("object GetInventoryDisturbItem();") 353, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetInventoryDisturbItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetInventoryDisturbItem) },
	{ NWSCRIPT_ACTIONNAME("GetHenchman") NWSCRIPT_ACTIONPROTOTYPE("object GetHenchman(object oMaster=OBJECT_SELF,int nNth=1);") 354, 0, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetHenchman NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetHenchman) },
	{ NWSCRIPT_ACTIONNAME("VersusAlignmentEffect") NWSCRIPT_ACTIONPROTOTYPE("effect VersusAlignmentEffect(effect eEffect, int nLawChaos=ALIGNMENT_ALL, int nGoodEvil=ALIGNMENT_ALL);") 355, 1, 3, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_VersusAlignmentEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_VersusAlignmentEffect) },
	{ NWSCRIPT_ACTIONNAME("VersusRacialTypeEffect") NWSCRIPT_ACTIONPROTOTYPE("effect VersusRacialTypeEffect(effect eEffect, int nRacialType);") 356, 2, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_VersusRacialTypeEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_VersusRacialTypeEffect) },
	{ NWSCRIPT_ACTIONNAME("VersusTrapEffect") NWSCRIPT_ACTIONPROTOTYPE("effect VersusTrapEffect(effect eEffect);") 357, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_VersusTrapEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_VersusTrapEffect) },
	{ NWSCRIPT_ACTIONNAME("GetGender") NWSCRIPT_ACTIONPROTOTYPE("int GetGender(object oCreature);") 358, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetGender NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetGender) },
	{ NWSCRIPT_ACTIONNAME("GetIsTalentValid") NWSCRIPT_ACTIONPROTOTYPE("int GetIsTalentValid(talent tTalent);") 359, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsTalentValid NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsTalentValid) },
	{ NWSCRIPT_ACTIONNAME("ActionMoveAwayFromLocation") NWSCRIPT_ACTIONPROTOTYPE("void ActionMoveAwayFromLocation(location lMoveAwayFrom, int bRun=FALSE, float fMoveAwayRange=40.0f);") 360, 1, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionMoveAwayFromLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionMoveAwayFromLocation) },
	{ NWSCRIPT_ACTIONNAME("GetAttemptedAttackTarget") NWSCRIPT_ACTIONPROTOTYPE("object GetAttemptedAttackTarget();") 361, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetAttemptedAttackTarget NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAttemptedAttackTarget) },
	{ NWSCRIPT_ACTIONNAME("GetTypeFromTalent") NWSCRIPT_ACTIONPROTOTYPE("int GetTypeFromTalent(talent tTalent);") 362, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTypeFromTalent NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTypeFromTalent) },
	{ NWSCRIPT_ACTIONNAME("GetIdFromTalent") NWSCRIPT_ACTIONPROTOTYPE("int GetIdFromTalent(talent tTalent);") 363, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIdFromTalent NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIdFromTalent) },
	{ NWSCRIPT_ACTIONNAME("GetAssociate") NWSCRIPT_ACTIONPROTOTYPE("object GetAssociate(int nAssociateType, object oMaster=OBJECT_SELF, int nTh=1);") 364, 1, 3, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetAssociate NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAssociate) },
	{ NWSCRIPT_ACTIONNAME("AddHenchman") NWSCRIPT_ACTIONPROTOTYPE("void AddHenchman(object oMaster, object oHenchman=OBJECT_SELF);") 365, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AddHenchman NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AddHenchman) },
	{ NWSCRIPT_ACTIONNAME("RemoveHenchman") NWSCRIPT_ACTIONPROTOTYPE("void RemoveHenchman(object oMaster, object oHenchman=OBJECT_SELF);") 366, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_RemoveHenchman NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RemoveHenchman) },
	{ NWSCRIPT_ACTIONNAME("AddJournalQuestEntry") NWSCRIPT_ACTIONPROTOTYPE("void AddJournalQuestEntry(string szPlotID, int nState, object oCreature, int bAllPartyMembers=TRUE, int bAllPlayers=FALSE, int bAllowOverrideHigher=FALSE);") 367, 3, 6, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AddJournalQuestEntry NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AddJournalQuestEntry) },
	{ NWSCRIPT_ACTIONNAME("RemoveJournalQuestEntry") NWSCRIPT_ACTIONPROTOTYPE("void RemoveJournalQuestEntry(string szPlotID, object oCreature, int bAllPartyMembers=TRUE, int bAllPlayers=FALSE);") 368, 2, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_RemoveJournalQuestEntry NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RemoveJournalQuestEntry) },
	{ NWSCRIPT_ACTIONNAME("GetPCPublicCDKey") NWSCRIPT_ACTIONPROTOTYPE("string GetPCPublicCDKey(object oPlayer);") 369, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetPCPublicCDKey NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPCPublicCDKey) },
	{ NWSCRIPT_ACTIONNAME("GetPCIPAddress") NWSCRIPT_ACTIONPROTOTYPE("string GetPCIPAddress(object oPlayer);") 370, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetPCIPAddress NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPCIPAddress) },
	{ NWSCRIPT_ACTIONNAME("GetPCPlayerName") NWSCRIPT_ACTIONPROTOTYPE("string GetPCPlayerName(object oPlayer);") 371, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetPCPlayerName NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPCPlayerName) },
	{ NWSCRIPT_ACTIONNAME("SetPCLike") NWSCRIPT_ACTIONPROTOTYPE("void SetPCLike(object oPlayer, object oTarget);") 372, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetPCLike NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetPCLike) },
	{ NWSCRIPT_ACTIONNAME("SetPCDislike") NWSCRIPT_ACTIONPROTOTYPE("void SetPCDislike(object oPlayer, object oTarget);") 373, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetPCDislike NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetPCDislike) },
	{ NWSCRIPT_ACTIONNAME("SendMessageToPC") NWSCRIPT_ACTIONPROTOTYPE("void SendMessageToPC(object oPlayer, string szMessage);") 374, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SendMessageToPC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SendMessageToPC) },
	{ NWSCRIPT_ACTIONNAME("GetAttemptedSpellTarget") NWSCRIPT_ACTIONPROTOTYPE("object GetAttemptedSpellTarget();") 375, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetAttemptedSpellTarget NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAttemptedSpellTarget) },
	{ NWSCRIPT_ACTIONNAME("GetLastOpenedBy") NWSCRIPT_ACTIONPROTOTYPE("object GetLastOpenedBy();") 376, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastOpenedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastOpenedBy) },
	{ NWSCRIPT_ACTIONNAME("GetHasSpell") NWSCRIPT_ACTIONPROTOTYPE("int GetHasSpell(int nSpell, object oCreature=OBJECT_SELF);") 377, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetHasSpell NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetHasSpell) },
	{ NWSCRIPT_ACTIONNAME("OpenStore") NWSCRIPT_ACTIONPROTOTYPE("void OpenStore(object oStore, object oPC, int nBonusMarkUp=0, int nBonusMarkDown=0);") 378, 2, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_OpenStore NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_OpenStore) },
	{ NWSCRIPT_ACTIONNAME("EffectTurned") NWSCRIPT_ACTIONPROTOTYPE("effect EffectTurned();") 379, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectTurned NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectTurned) },
	{ NWSCRIPT_ACTIONNAME("GetFirstFactionMember") NWSCRIPT_ACTIONPROTOTYPE("object GetFirstFactionMember(object oMemberOfFaction, int bPCOnly=TRUE);") 380, 1, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetFirstFactionMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFirstFactionMember) },
	{ NWSCRIPT_ACTIONNAME("GetNextFactionMember") NWSCRIPT_ACTIONPROTOTYPE("object GetNextFactionMember(object oMemberOfFaction, int bPCOnly=TRUE);") 381, 1, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetNextFactionMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNextFactionMember) },
	{ NWSCRIPT_ACTIONNAME("ActionForceMoveToLocation") NWSCRIPT_ACTIONPROTOTYPE("void ActionForceMoveToLocation(location lDestination, int bRun=FALSE, float fTimeout=30.0f);") 382, 1, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionForceMoveToLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionForceMoveToLocation) },
	{ NWSCRIPT_ACTIONNAME("ActionForceMoveToObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionForceMoveToObject(object oMoveTo, int bRun=FALSE, float fRange=1.0f, float fTimeout=30.0f);") 383, 1, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionForceMoveToObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionForceMoveToObject) },
	{ NWSCRIPT_ACTIONNAME("GetJournalQuestExperience") NWSCRIPT_ACTIONPROTOTYPE("int GetJournalQuestExperience(string szPlotID);") 384, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetJournalQuestExperience NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetJournalQuestExperience) },
	{ NWSCRIPT_ACTIONNAME("JumpToObject") NWSCRIPT_ACTIONPROTOTYPE("void JumpToObject(object oToJumpTo, int nWalkStraightLineToPoint=1);") 385, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_JumpToObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_JumpToObject) },
	{ NWSCRIPT_ACTIONNAME("SetMapPinEnabled") NWSCRIPT_ACTIONPROTOTYPE("void SetMapPinEnabled(object oMapPin, int nEnabled);") 386, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetMapPinEnabled NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetMapPinEnabled) },
	{ NWSCRIPT_ACTIONNAME("EffectHitPointChangeWhenDying") NWSCRIPT_ACTIONPROTOTYPE("effect EffectHitPointChangeWhenDying(float fHitPointChangePerRound);") 387, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectHitPointChangeWhenDying NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectHitPointChangeWhenDying) },
	{ NWSCRIPT_ACTIONNAME("PopUpGUIPanel") NWSCRIPT_ACTIONPROTOTYPE("void PopUpGUIPanel(object oPC, int nGUIPanel);") 388, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_PopUpGUIPanel NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_PopUpGUIPanel) },
	{ NWSCRIPT_ACTIONNAME("ClearPersonalReputation") NWSCRIPT_ACTIONPROTOTYPE("void ClearPersonalReputation(object oTarget, object oSource=OBJECT_SELF);") 389, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ClearPersonalReputation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ClearPersonalReputation) },
	{ NWSCRIPT_ACTIONNAME("SetIsTemporaryFriend") NWSCRIPT_ACTIONPROTOTYPE("void SetIsTemporaryFriend(object oTarget, object oSource=OBJECT_SELF, int bDecays=FALSE, float fDurationInSeconds=180.0f);") 390, 1, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetIsTemporaryFriend NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetIsTemporaryFriend) },
	{ NWSCRIPT_ACTIONNAME("SetIsTemporaryEnemy") NWSCRIPT_ACTIONPROTOTYPE("void SetIsTemporaryEnemy(object oTarget, object oSource=OBJECT_SELF, int bDecays=FALSE, float fDurationInSeconds=180.0f);") 391, 1, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetIsTemporaryEnemy NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetIsTemporaryEnemy) },
	{ NWSCRIPT_ACTIONNAME("SetIsTemporaryNeutral") NWSCRIPT_ACTIONPROTOTYPE("void SetIsTemporaryNeutral(object oTarget, object oSource=OBJECT_SELF, int bDecays=FALSE, float fDurationInSeconds=180.0f);") 392, 1, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetIsTemporaryNeutral NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetIsTemporaryNeutral) },
	{ NWSCRIPT_ACTIONNAME("GiveXPToCreature") NWSCRIPT_ACTIONPROTOTYPE("void GiveXPToCreature(object oCreature, int nXpAmount);") 393, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_GiveXPToCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GiveXPToCreature) },
	{ NWSCRIPT_ACTIONNAME("SetXP") NWSCRIPT_ACTIONPROTOTYPE("void SetXP(object oCreature, int nXpAmount);") 394, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetXP NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetXP) },
	{ NWSCRIPT_ACTIONNAME("GetXP") NWSCRIPT_ACTIONPROTOTYPE("int GetXP(object oCreature);") 395, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetXP NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetXP) },
	{ NWSCRIPT_ACTIONNAME("IntToHexString") NWSCRIPT_ACTIONPROTOTYPE("string IntToHexString(int nInteger);") 396, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_IntToHexString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_IntToHexString) },
	{ NWSCRIPT_ACTIONNAME("GetBaseItemType") NWSCRIPT_ACTIONPROTOTYPE("int GetBaseItemType(object oItem);") 397, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetBaseItemType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetBaseItemType) },
	{ NWSCRIPT_ACTIONNAME("GetItemHasItemProperty") NWSCRIPT_ACTIONPROTOTYPE("int GetItemHasItemProperty(object oItem, int nProperty);") 398, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetItemHasItemProperty NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemHasItemProperty) },
	{ NWSCRIPT_ACTIONNAME("ActionEquipMostDamagingMelee") NWSCRIPT_ACTIONPROTOTYPE("void ActionEquipMostDamagingMelee(object oVersus=OBJECT_INVALID, int bOffHand=FALSE);") 399, 0, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionEquipMostDamagingMelee NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionEquipMostDamagingMelee) },
	{ NWSCRIPT_ACTIONNAME("ActionEquipMostDamagingRanged") NWSCRIPT_ACTIONPROTOTYPE("void ActionEquipMostDamagingRanged(object oVersus=OBJECT_INVALID);") 400, 0, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionEquipMostDamagingRanged NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionEquipMostDamagingRanged) },
	{ NWSCRIPT_ACTIONNAME("GetItemACValue") NWSCRIPT_ACTIONPROTOTYPE("int GetItemACValue(object oItem);") 401, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetItemACValue NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemACValue) },
	{ NWSCRIPT_ACTIONNAME("ActionRest") NWSCRIPT_ACTIONPROTOTYPE("void ActionRest(int bIgnoreNoRestFlag=0);") 402, 0, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionRest NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionRest) },
	{ NWSCRIPT_ACTIONNAME("ExploreAreaForPlayer") NWSCRIPT_ACTIONPROTOTYPE("void ExploreAreaForPlayer(object oArea, object oPlayer, int nExplored=TRUE);") 403, 2, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ExploreAreaForPlayer NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ExploreAreaForPlayer) },
	{ NWSCRIPT_ACTIONNAME("ActionEquipMostEffectiveArmor") NWSCRIPT_ACTIONPROTOTYPE("void ActionEquipMostEffectiveArmor();") 404, 0, 0, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionEquipMostEffectiveArmor NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionEquipMostEffectiveArmor) },
	{ NWSCRIPT_ACTIONNAME("GetIsDay") NWSCRIPT_ACTIONPROTOTYPE("int GetIsDay();") 405, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsDay NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsDay) },
	{ NWSCRIPT_ACTIONNAME("GetIsNight") NWSCRIPT_ACTIONPROTOTYPE("int GetIsNight();") 406, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsNight NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsNight) },
	{ NWSCRIPT_ACTIONNAME("GetIsDawn") NWSCRIPT_ACTIONPROTOTYPE("int GetIsDawn();") 407, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsDawn NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsDawn) },
	{ NWSCRIPT_ACTIONNAME("GetIsDusk") NWSCRIPT_ACTIONPROTOTYPE("int GetIsDusk();") 408, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsDusk NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsDusk) },
	{ NWSCRIPT_ACTIONNAME("GetIsEncounterCreature") NWSCRIPT_ACTIONPROTOTYPE("int GetIsEncounterCreature(object oCreature=OBJECT_SELF);") 409, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsEncounterCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsEncounterCreature) },
	{ NWSCRIPT_ACTIONNAME("GetLastPlayerDying") NWSCRIPT_ACTIONPROTOTYPE("object GetLastPlayerDying();") 410, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastPlayerDying NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastPlayerDying) },
	{ NWSCRIPT_ACTIONNAME("GetStartingLocation") NWSCRIPT_ACTIONPROTOTYPE("location GetStartingLocation();") 411, 0, 0, ACTIONTYPE_LOCATION, NWN2_NWActionParameterTypes_GetStartingLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetStartingLocation) },
	{ NWSCRIPT_ACTIONNAME("ChangeToStandardFaction") NWSCRIPT_ACTIONPROTOTYPE("void ChangeToStandardFaction(object oCreatureToChange, int nStandardFaction);") 412, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ChangeToStandardFaction NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ChangeToStandardFaction) },
	{ NWSCRIPT_ACTIONNAME("SoundObjectPlay") NWSCRIPT_ACTIONPROTOTYPE("void SoundObjectPlay(object oSound);") 413, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SoundObjectPlay NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SoundObjectPlay) },
	{ NWSCRIPT_ACTIONNAME("SoundObjectStop") NWSCRIPT_ACTIONPROTOTYPE("void SoundObjectStop(object oSound);") 414, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SoundObjectStop NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SoundObjectStop) },
	{ NWSCRIPT_ACTIONNAME("SoundObjectSetVolume") NWSCRIPT_ACTIONPROTOTYPE("void SoundObjectSetVolume(object oSound, int nVolume);") 415, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SoundObjectSetVolume NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SoundObjectSetVolume) },
	{ NWSCRIPT_ACTIONNAME("SoundObjectSetPosition") NWSCRIPT_ACTIONPROTOTYPE("void SoundObjectSetPosition(object oSound, vector vPosition);") 416, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SoundObjectSetPosition NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SoundObjectSetPosition) },
	{ NWSCRIPT_ACTIONNAME("SpeakOneLinerConversation") NWSCRIPT_ACTIONPROTOTYPE("void SpeakOneLinerConversation(string sDialogResRef="", object oTokenTarget=OBJECT_INVALID, int nTalkVolume=TALKVOLUME_TALK);") 417, 0, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SpeakOneLinerConversation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SpeakOneLinerConversation) },
	{ NWSCRIPT_ACTIONNAME("GetGold") NWSCRIPT_ACTIONPROTOTYPE("int GetGold(object oTarget=OBJECT_SELF);") 418, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetGold NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetGold) },
	{ NWSCRIPT_ACTIONNAME("GetLastRespawnButtonPresser") NWSCRIPT_ACTIONPROTOTYPE("object GetLastRespawnButtonPresser();") 419, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastRespawnButtonPresser NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastRespawnButtonPresser) },
	{ NWSCRIPT_ACTIONNAME("GetIsDM") NWSCRIPT_ACTIONPROTOTYPE("int GetIsDM(object oCreature);") 420, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsDM NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsDM) },
	{ NWSCRIPT_ACTIONNAME("PlayVoiceChat") NWSCRIPT_ACTIONPROTOTYPE("void PlayVoiceChat(int nVoiceChatID, object oTarget=OBJECT_SELF);") 421, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_PlayVoiceChat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_PlayVoiceChat) },
	{ NWSCRIPT_ACTIONNAME("GetIsWeaponEffective") NWSCRIPT_ACTIONPROTOTYPE("int GetIsWeaponEffective(object oVersus=OBJECT_INVALID, int bOffHand=FALSE);") 422, 0, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsWeaponEffective NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsWeaponEffective) },
	{ NWSCRIPT_ACTIONNAME("GetLastSpellHarmful") NWSCRIPT_ACTIONPROTOTYPE("int GetLastSpellHarmful();") 423, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLastSpellHarmful NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastSpellHarmful) },
	{ NWSCRIPT_ACTIONNAME("EventActivateItem") NWSCRIPT_ACTIONPROTOTYPE("event EventActivateItem(object oItem, location lTarget, object oTarget=OBJECT_INVALID);") 424, 2, 3, ACTIONTYPE_EVENT, NWN2_NWActionParameterTypes_EventActivateItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EventActivateItem) },
	{ NWSCRIPT_ACTIONNAME("MusicBackgroundPlay") NWSCRIPT_ACTIONPROTOTYPE("void MusicBackgroundPlay(object oArea);") 425, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_MusicBackgroundPlay NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_MusicBackgroundPlay) },
	{ NWSCRIPT_ACTIONNAME("MusicBackgroundStop") NWSCRIPT_ACTIONPROTOTYPE("void MusicBackgroundStop(object oArea);") 426, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_MusicBackgroundStop NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_MusicBackgroundStop) },
	{ NWSCRIPT_ACTIONNAME("MusicBackgroundSetDelay") NWSCRIPT_ACTIONPROTOTYPE("void MusicBackgroundSetDelay(object oArea, int nDelay);") 427, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_MusicBackgroundSetDelay NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_MusicBackgroundSetDelay) },
	{ NWSCRIPT_ACTIONNAME("MusicBackgroundChangeDay") NWSCRIPT_ACTIONPROTOTYPE("void MusicBackgroundChangeDay(object oArea, int nTrack);") 428, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_MusicBackgroundChangeDay NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_MusicBackgroundChangeDay) },
	{ NWSCRIPT_ACTIONNAME("MusicBackgroundChangeNight") NWSCRIPT_ACTIONPROTOTYPE("void MusicBackgroundChangeNight(object oArea, int nTrack);") 429, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_MusicBackgroundChangeNight NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_MusicBackgroundChangeNight) },
	{ NWSCRIPT_ACTIONNAME("MusicBattlePlay") NWSCRIPT_ACTIONPROTOTYPE("void MusicBattlePlay(object oArea);") 430, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_MusicBattlePlay NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_MusicBattlePlay) },
	{ NWSCRIPT_ACTIONNAME("MusicBattleStop") NWSCRIPT_ACTIONPROTOTYPE("void MusicBattleStop(object oArea);") 431, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_MusicBattleStop NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_MusicBattleStop) },
	{ NWSCRIPT_ACTIONNAME("MusicBattleChange") NWSCRIPT_ACTIONPROTOTYPE("void MusicBattleChange(object oArea, int nTrack);") 432, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_MusicBattleChange NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_MusicBattleChange) },
	{ NWSCRIPT_ACTIONNAME("AmbientSoundPlay") NWSCRIPT_ACTIONPROTOTYPE("void AmbientSoundPlay(object oArea);") 433, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AmbientSoundPlay NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AmbientSoundPlay) },
	{ NWSCRIPT_ACTIONNAME("AmbientSoundStop") NWSCRIPT_ACTIONPROTOTYPE("void AmbientSoundStop(object oArea);") 434, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AmbientSoundStop NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AmbientSoundStop) },
	{ NWSCRIPT_ACTIONNAME("AmbientSoundChangeDay") NWSCRIPT_ACTIONPROTOTYPE("void AmbientSoundChangeDay(object oArea, int nTrack);") 435, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AmbientSoundChangeDay NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AmbientSoundChangeDay) },
	{ NWSCRIPT_ACTIONNAME("AmbientSoundChangeNight") NWSCRIPT_ACTIONPROTOTYPE("void AmbientSoundChangeNight(object oArea, int nTrack);") 436, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AmbientSoundChangeNight NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AmbientSoundChangeNight) },
	{ NWSCRIPT_ACTIONNAME("GetLastKiller") NWSCRIPT_ACTIONPROTOTYPE("object GetLastKiller();") 437, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastKiller NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastKiller) },
	{ NWSCRIPT_ACTIONNAME("GetSpellCastItem") NWSCRIPT_ACTIONPROTOTYPE("object GetSpellCastItem();") 438, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetSpellCastItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetSpellCastItem) },
	{ NWSCRIPT_ACTIONNAME("GetItemActivated") NWSCRIPT_ACTIONPROTOTYPE("object GetItemActivated();") 439, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetItemActivated NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemActivated) },
	{ NWSCRIPT_ACTIONNAME("GetItemActivator") NWSCRIPT_ACTIONPROTOTYPE("object GetItemActivator();") 440, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetItemActivator NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemActivator) },
	{ NWSCRIPT_ACTIONNAME("GetItemActivatedTargetLocation") NWSCRIPT_ACTIONPROTOTYPE("location GetItemActivatedTargetLocation();") 441, 0, 0, ACTIONTYPE_LOCATION, NWN2_NWActionParameterTypes_GetItemActivatedTargetLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemActivatedTargetLocation) },
	{ NWSCRIPT_ACTIONNAME("GetItemActivatedTarget") NWSCRIPT_ACTIONPROTOTYPE("object GetItemActivatedTarget();") 442, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetItemActivatedTarget NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemActivatedTarget) },
	{ NWSCRIPT_ACTIONNAME("GetIsOpen") NWSCRIPT_ACTIONPROTOTYPE("int GetIsOpen(object oObject);") 443, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsOpen NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsOpen) },
	{ NWSCRIPT_ACTIONNAME("TakeGoldFromCreature") NWSCRIPT_ACTIONPROTOTYPE("void TakeGoldFromCreature(int nAmount, object oCreatureToTakeFrom, int bDestroy=FALSE, int bDisplayFeedback=TRUE);") 444, 2, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_TakeGoldFromCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_TakeGoldFromCreature) },
	{ NWSCRIPT_ACTIONNAME("IsInConversation") NWSCRIPT_ACTIONPROTOTYPE("int IsInConversation(object oObject);") 445, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_IsInConversation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_IsInConversation) },
	{ NWSCRIPT_ACTIONNAME("EffectAbilityDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectAbilityDecrease(int nAbility, int nModifyBy);") 446, 2, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectAbilityDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectAbilityDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectAttackDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectAttackDecrease(int nPenalty, int nModifierType=ATTACK_BONUS_MISC);") 447, 1, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectAttackDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectAttackDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectDamageDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamageDecrease(int nPenalty, int nDamageType=DAMAGE_TYPE_MAGICAL);") 448, 1, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDamageDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDamageDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectDamageImmunityDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamageImmunityDecrease(int nDamageType, int nPercentImmunity);") 449, 2, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDamageImmunityDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDamageImmunityDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectACDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectACDecrease(int nValue, int nModifyType=AC_DODGE_BONUS, int nDamageType=AC_VS_DAMAGE_TYPE_ALL);") 450, 1, 3, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectACDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectACDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectMovementSpeedDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectMovementSpeedDecrease(int nPercentChange);") 451, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectMovementSpeedDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectMovementSpeedDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectSavingThrowDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSavingThrowDecrease(int nSave, int nValue, int nSaveType=SAVING_THROW_TYPE_ALL);") 452, 2, 3, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSavingThrowDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSavingThrowDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectSkillDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSkillDecrease(int nSkill, int nValue);") 453, 2, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSkillDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSkillDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectSpellResistanceDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSpellResistanceDecrease(int nValue);") 454, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSpellResistanceDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSpellResistanceDecrease) },
	{ NWSCRIPT_ACTIONNAME("GetPlotFlag") NWSCRIPT_ACTIONPROTOTYPE("int GetPlotFlag(object oTarget=OBJECT_SELF);") 455, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetPlotFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPlotFlag) },
	{ NWSCRIPT_ACTIONNAME("SetPlotFlag") NWSCRIPT_ACTIONPROTOTYPE("void SetPlotFlag(object oTarget, int nPlotFlag);") 456, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetPlotFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetPlotFlag) },
	{ NWSCRIPT_ACTIONNAME("EffectInvisibility") NWSCRIPT_ACTIONPROTOTYPE("effect EffectInvisibility(int nInvisibilityType);") 457, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectInvisibility NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectInvisibility) },
	{ NWSCRIPT_ACTIONNAME("EffectConcealment") NWSCRIPT_ACTIONPROTOTYPE("effect EffectConcealment(int nPercentage, int nMissType=MISS_CHANCE_TYPE_NORMAL);") 458, 1, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectConcealment NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectConcealment) },
	{ NWSCRIPT_ACTIONNAME("EffectDarkness") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDarkness();") 459, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDarkness NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDarkness) },
	{ NWSCRIPT_ACTIONNAME("EffectDispelMagicAll") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDispelMagicAll(int nCasterLevel, action aOnDispelEffect );") 460, 2, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDispelMagicAll NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDispelMagicAll) },
	{ NWSCRIPT_ACTIONNAME("EffectUltravision") NWSCRIPT_ACTIONPROTOTYPE("effect EffectUltravision();") 461, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectUltravision NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectUltravision) },
	{ NWSCRIPT_ACTIONNAME("EffectNegativeLevel") NWSCRIPT_ACTIONPROTOTYPE("effect EffectNegativeLevel(int nNumLevels, int bHPBonus=FALSE);") 462, 1, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectNegativeLevel NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectNegativeLevel) },
	{ NWSCRIPT_ACTIONNAME("EffectPolymorph") NWSCRIPT_ACTIONPROTOTYPE("effect EffectPolymorph(int nPolymorphSelection, int nLocked=FALSE, int bWildshape=FALSE);") 463, 1, 3, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectPolymorph NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectPolymorph) },
	{ NWSCRIPT_ACTIONNAME("EffectSanctuary") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSanctuary(int nDifficultyClass);") 464, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSanctuary NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSanctuary) },
	{ NWSCRIPT_ACTIONNAME("EffectTrueSeeing") NWSCRIPT_ACTIONPROTOTYPE("effect EffectTrueSeeing();") 465, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectTrueSeeing NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectTrueSeeing) },
	{ NWSCRIPT_ACTIONNAME("EffectSeeInvisible") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSeeInvisible();") 466, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSeeInvisible NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSeeInvisible) },
	{ NWSCRIPT_ACTIONNAME("EffectTimeStop") NWSCRIPT_ACTIONPROTOTYPE("effect EffectTimeStop();") 467, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectTimeStop NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectTimeStop) },
	{ NWSCRIPT_ACTIONNAME("EffectBlindness") NWSCRIPT_ACTIONPROTOTYPE("effect EffectBlindness();") 468, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectBlindness NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectBlindness) },
	{ NWSCRIPT_ACTIONNAME("GetIsReactionTypeFriendly") NWSCRIPT_ACTIONPROTOTYPE("int GetIsReactionTypeFriendly(object oTarget, object oSource=OBJECT_SELF);") 469, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsReactionTypeFriendly NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsReactionTypeFriendly) },
	{ NWSCRIPT_ACTIONNAME("GetIsReactionTypeNeutral") NWSCRIPT_ACTIONPROTOTYPE("int GetIsReactionTypeNeutral(object oTarget, object oSource=OBJECT_SELF);") 470, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsReactionTypeNeutral NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsReactionTypeNeutral) },
	{ NWSCRIPT_ACTIONNAME("GetIsReactionTypeHostile") NWSCRIPT_ACTIONPROTOTYPE("int GetIsReactionTypeHostile(object oTarget, object oSource=OBJECT_SELF);") 471, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsReactionTypeHostile NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsReactionTypeHostile) },
	{ NWSCRIPT_ACTIONNAME("EffectSpellLevelAbsorption") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSpellLevelAbsorption(int nMaxSpellLevelAbsorbed, int nTotalSpellLevelsAbsorbed=0, int nSpellSchool=SPELL_SCHOOL_GENERAL );") 472, 1, 3, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSpellLevelAbsorption NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSpellLevelAbsorption) },
	{ NWSCRIPT_ACTIONNAME("EffectDispelMagicBest") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDispelMagicBest(int nCasterLevel, action aOnDispelEffect );") 473, 2, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDispelMagicBest NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDispelMagicBest) },
	{ NWSCRIPT_ACTIONNAME("ActivatePortal") NWSCRIPT_ACTIONPROTOTYPE("void ActivatePortal(object oTarget, string sIPaddress="", string sPassword="", string sWaypointTag="", int bSeemless=FALSE);") 474, 1, 5, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActivatePortal NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActivatePortal) },
	{ NWSCRIPT_ACTIONNAME("GetNumStackedItems") NWSCRIPT_ACTIONPROTOTYPE("int GetNumStackedItems(object oItem);") 475, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetNumStackedItems NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNumStackedItems) },
	{ NWSCRIPT_ACTIONNAME("SurrenderToEnemies") NWSCRIPT_ACTIONPROTOTYPE("void SurrenderToEnemies();") 476, 0, 0, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SurrenderToEnemies NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SurrenderToEnemies) },
	{ NWSCRIPT_ACTIONNAME("EffectMissChance") NWSCRIPT_ACTIONPROTOTYPE("effect EffectMissChance(int nPercentage, int nMissChanceType=MISS_CHANCE_TYPE_NORMAL);") 477, 1, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectMissChance NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectMissChance) },
	{ NWSCRIPT_ACTIONNAME("GetTurnResistanceHD") NWSCRIPT_ACTIONPROTOTYPE("int GetTurnResistanceHD(object oUndead=OBJECT_SELF);") 478, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTurnResistanceHD NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTurnResistanceHD) },
	{ NWSCRIPT_ACTIONNAME("GetCreatureSize") NWSCRIPT_ACTIONPROTOTYPE("int GetCreatureSize(object oCreature);") 479, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetCreatureSize NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCreatureSize) },
	{ NWSCRIPT_ACTIONNAME("EffectDisappearAppear") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDisappearAppear(location lLocation, int nAnimation=1);") 480, 1, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDisappearAppear NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDisappearAppear) },
	{ NWSCRIPT_ACTIONNAME("EffectDisappear") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDisappear(int nAnimation=1);") 481, 0, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDisappear NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDisappear) },
	{ NWSCRIPT_ACTIONNAME("EffectAppear") NWSCRIPT_ACTIONPROTOTYPE("effect EffectAppear(int nAnimation=1);") 482, 0, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectAppear NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectAppear) },
	{ NWSCRIPT_ACTIONNAME("ActionUnlockObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionUnlockObject(object oTarget);") 483, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionUnlockObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionUnlockObject) },
	{ NWSCRIPT_ACTIONNAME("ActionLockObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionLockObject(object oTarget);") 484, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionLockObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionLockObject) },
	{ NWSCRIPT_ACTIONNAME("EffectModifyAttacks") NWSCRIPT_ACTIONPROTOTYPE("effect EffectModifyAttacks(int nAttacks);") 485, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectModifyAttacks NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectModifyAttacks) },
	{ NWSCRIPT_ACTIONNAME("GetLastTrapDetected") NWSCRIPT_ACTIONPROTOTYPE("object GetLastTrapDetected(object oTarget=OBJECT_SELF);") 486, 0, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastTrapDetected NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastTrapDetected) },
	{ NWSCRIPT_ACTIONNAME("EffectDamageShield") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamageShield(int nDamageAmount, int nRandomAmount, int nDamageType);") 487, 3, 3, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDamageShield NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDamageShield) },
	{ NWSCRIPT_ACTIONNAME("GetNearestTrapToObject") NWSCRIPT_ACTIONPROTOTYPE("object GetNearestTrapToObject(object oTarget=OBJECT_SELF, int nTrapDetected=TRUE);") 488, 0, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetNearestTrapToObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNearestTrapToObject) },
	{ NWSCRIPT_ACTIONNAME("GetDeity") NWSCRIPT_ACTIONPROTOTYPE("string GetDeity(object oCreature);") 489, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetDeity NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetDeity) },
	{ NWSCRIPT_ACTIONNAME("GetSubRace") NWSCRIPT_ACTIONPROTOTYPE("int GetSubRace(object oTarget);") 490, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetSubRace NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetSubRace) },
	{ NWSCRIPT_ACTIONNAME("GetFortitudeSavingThrow") NWSCRIPT_ACTIONPROTOTYPE("int GetFortitudeSavingThrow(object oTarget);") 491, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetFortitudeSavingThrow NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFortitudeSavingThrow) },
	{ NWSCRIPT_ACTIONNAME("GetWillSavingThrow") NWSCRIPT_ACTIONPROTOTYPE("int GetWillSavingThrow(object oTarget);") 492, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetWillSavingThrow NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetWillSavingThrow) },
	{ NWSCRIPT_ACTIONNAME("GetReflexSavingThrow") NWSCRIPT_ACTIONPROTOTYPE("int GetReflexSavingThrow(object oTarget);") 493, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetReflexSavingThrow NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetReflexSavingThrow) },
	{ NWSCRIPT_ACTIONNAME("GetChallengeRating") NWSCRIPT_ACTIONPROTOTYPE("float GetChallengeRating(object oCreature);") 494, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_GetChallengeRating NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetChallengeRating) },
	{ NWSCRIPT_ACTIONNAME("GetAge") NWSCRIPT_ACTIONPROTOTYPE("int GetAge(object oCreature);") 495, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetAge NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAge) },
	{ NWSCRIPT_ACTIONNAME("GetMovementRate") NWSCRIPT_ACTIONPROTOTYPE("int GetMovementRate(object oCreature);") 496, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetMovementRate NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetMovementRate) },
	{ NWSCRIPT_ACTIONNAME("GetAnimalCompanionCreatureType") NWSCRIPT_ACTIONPROTOTYPE("int GetAnimalCompanionCreatureType(object oCreature);") 497, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetAnimalCompanionCreatureType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAnimalCompanionCreatureType) },
	{ NWSCRIPT_ACTIONNAME("GetFamiliarCreatureType") NWSCRIPT_ACTIONPROTOTYPE("int GetFamiliarCreatureType(object oCreature);") 498, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetFamiliarCreatureType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFamiliarCreatureType) },
	{ NWSCRIPT_ACTIONNAME("GetAnimalCompanionName") NWSCRIPT_ACTIONPROTOTYPE("string GetAnimalCompanionName(object oTarget);") 499, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetAnimalCompanionName NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAnimalCompanionName) },
	{ NWSCRIPT_ACTIONNAME("GetFamiliarName") NWSCRIPT_ACTIONPROTOTYPE("string GetFamiliarName(object oCreature);") 500, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetFamiliarName NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFamiliarName) },
	{ NWSCRIPT_ACTIONNAME("ActionCastFakeSpellAtObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionCastFakeSpellAtObject(int nSpell, object oTarget, int nProjectilePathType=PROJECTILE_PATH_TYPE_DEFAULT);") 501, 2, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionCastFakeSpellAtObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionCastFakeSpellAtObject) },
	{ NWSCRIPT_ACTIONNAME("ActionCastFakeSpellAtLocation") NWSCRIPT_ACTIONPROTOTYPE("void ActionCastFakeSpellAtLocation(int nSpell, location lTarget, int nProjectilePathType=PROJECTILE_PATH_TYPE_DEFAULT);") 502, 2, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionCastFakeSpellAtLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionCastFakeSpellAtLocation) },
	{ NWSCRIPT_ACTIONNAME("RemoveSummonedAssociate") NWSCRIPT_ACTIONPROTOTYPE("void RemoveSummonedAssociate(object oMaster, object oAssociate=OBJECT_SELF);") 503, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_RemoveSummonedAssociate NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RemoveSummonedAssociate) },
	{ NWSCRIPT_ACTIONNAME("SetCameraMode") NWSCRIPT_ACTIONPROTOTYPE("void SetCameraMode(object oPlayer, int nCameraMode);") 504, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCameraMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCameraMode) },
	{ NWSCRIPT_ACTIONNAME("GetIsResting") NWSCRIPT_ACTIONPROTOTYPE("int GetIsResting(object oCreature=OBJECT_SELF);") 505, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsResting NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsResting) },
	{ NWSCRIPT_ACTIONNAME("GetLastPCRested") NWSCRIPT_ACTIONPROTOTYPE("object GetLastPCRested();") 506, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastPCRested NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastPCRested) },
	{ NWSCRIPT_ACTIONNAME("SetWeather") NWSCRIPT_ACTIONPROTOTYPE("void SetWeather(object oTarget, int nWeatherType, int nPower = WEATHER_POWER_MEDIUM);") 507, 2, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetWeather NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetWeather) },
	{ NWSCRIPT_ACTIONNAME("GetLastRestEventType") NWSCRIPT_ACTIONPROTOTYPE("int GetLastRestEventType();") 508, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLastRestEventType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastRestEventType) },
	{ NWSCRIPT_ACTIONNAME("StartNewModule") NWSCRIPT_ACTIONPROTOTYPE("void StartNewModule(string sModuleName, string sWaypoint = "");") 509, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_StartNewModule NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_StartNewModule) },
	{ NWSCRIPT_ACTIONNAME("EffectSwarm") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSwarm(int nLooping, string sCreatureTemplate1, string sCreatureTemplate2="", string sCreatureTemplate3="", string sCreatureTemplate4="");") 510, 2, 5, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSwarm NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSwarm) },
	{ NWSCRIPT_ACTIONNAME("GetWeaponRanged") NWSCRIPT_ACTIONPROTOTYPE("int GetWeaponRanged(object oItem);") 511, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetWeaponRanged NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetWeaponRanged) },
	{ NWSCRIPT_ACTIONNAME("DoSinglePlayerAutoSave") NWSCRIPT_ACTIONPROTOTYPE("void DoSinglePlayerAutoSave();") 512, 0, 0, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DoSinglePlayerAutoSave NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DoSinglePlayerAutoSave) },
	{ NWSCRIPT_ACTIONNAME("GetGameDifficulty") NWSCRIPT_ACTIONPROTOTYPE("int GetGameDifficulty();") 513, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetGameDifficulty NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetGameDifficulty) },
	{ NWSCRIPT_ACTIONNAME("SetTileMainLightColor") NWSCRIPT_ACTIONPROTOTYPE("void SetTileMainLightColor(location lTileLocation, int nMainLight1Color, int nMainLight2Color);") 514, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetTileMainLightColor NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetTileMainLightColor) },
	{ NWSCRIPT_ACTIONNAME("SetTileSourceLightColor") NWSCRIPT_ACTIONPROTOTYPE("void SetTileSourceLightColor(location lTileLocation, int nSourceLight1Color, int nSourceLight2Color);") 515, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetTileSourceLightColor NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetTileSourceLightColor) },
	{ NWSCRIPT_ACTIONNAME("RecomputeStaticLighting") NWSCRIPT_ACTIONPROTOTYPE("void RecomputeStaticLighting(object oArea);") 516, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_RecomputeStaticLighting NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RecomputeStaticLighting) },
	{ NWSCRIPT_ACTIONNAME("GetTileMainLight1Color") NWSCRIPT_ACTIONPROTOTYPE("int GetTileMainLight1Color(location lTile);") 517, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTileMainLight1Color NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTileMainLight1Color) },
	{ NWSCRIPT_ACTIONNAME("GetTileMainLight2Color") NWSCRIPT_ACTIONPROTOTYPE("int GetTileMainLight2Color(location lTile);") 518, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTileMainLight2Color NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTileMainLight2Color) },
	{ NWSCRIPT_ACTIONNAME("GetTileSourceLight1Color") NWSCRIPT_ACTIONPROTOTYPE("int GetTileSourceLight1Color(location lTile);") 519, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTileSourceLight1Color NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTileSourceLight1Color) },
	{ NWSCRIPT_ACTIONNAME("GetTileSourceLight2Color") NWSCRIPT_ACTIONPROTOTYPE("int GetTileSourceLight2Color(location lTile);") 520, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTileSourceLight2Color NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTileSourceLight2Color) },
	{ NWSCRIPT_ACTIONNAME("SetPanelButtonFlash") NWSCRIPT_ACTIONPROTOTYPE("void SetPanelButtonFlash(object oPlayer, int nButton, int nEnableFlash);") 521, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetPanelButtonFlash NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetPanelButtonFlash) },
	{ NWSCRIPT_ACTIONNAME("GetCurrentAction") NWSCRIPT_ACTIONPROTOTYPE("int GetCurrentAction(object oObject=OBJECT_SELF);") 522, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetCurrentAction NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCurrentAction) },
	{ NWSCRIPT_ACTIONNAME("SetStandardFactionReputation") NWSCRIPT_ACTIONPROTOTYPE("void SetStandardFactionReputation(int nStandardFaction, int nNewReputation, object oCreature=OBJECT_SELF);") 523, 2, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetStandardFactionReputation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetStandardFactionReputation) },
	{ NWSCRIPT_ACTIONNAME("GetStandardFactionReputation") NWSCRIPT_ACTIONPROTOTYPE("int GetStandardFactionReputation(int nStandardFaction, object oCreature=OBJECT_SELF);") 524, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetStandardFactionReputation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetStandardFactionReputation) },
	{ NWSCRIPT_ACTIONNAME("FloatingTextStrRefOnCreature") NWSCRIPT_ACTIONPROTOTYPE("void FloatingTextStrRefOnCreature(int nStrRefToDisplay, object oCreatureToFloatAbove, int bBroadcastToFaction=TRUE, float fDuration=5.0, int nStartColor=4294967295, int nEndColor=4294967295, float fSpeed=0.0, vector vDirection=[0.0,0.0,0.0]);") 525, 2, 8, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_FloatingTextStrRefOnCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_FloatingTextStrRefOnCreature) },
	{ NWSCRIPT_ACTIONNAME("FloatingTextStringOnCreature") NWSCRIPT_ACTIONPROTOTYPE("void FloatingTextStringOnCreature(string sStringToDisplay, object oCreatureToFloatAbove, int bBroadcastToFaction=TRUE, float fDuration=5.0, int nStartColor=4294967295, int nEndColor=4294967295, float fSpeed=0.0, vector vDirection=[0.0,0.0,0.0]);") 526, 2, 8, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_FloatingTextStringOnCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_FloatingTextStringOnCreature) },
	{ NWSCRIPT_ACTIONNAME("GetTrapDisarmable") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapDisarmable(object oTrapObject);") 527, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTrapDisarmable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTrapDisarmable) },
	{ NWSCRIPT_ACTIONNAME("GetTrapDetectable") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapDetectable(object oTrapObject);") 528, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTrapDetectable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTrapDetectable) },
	{ NWSCRIPT_ACTIONNAME("GetTrapDetectedBy") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapDetectedBy(object oTrapObject, object oCreature);") 529, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTrapDetectedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTrapDetectedBy) },
	{ NWSCRIPT_ACTIONNAME("GetTrapFlagged") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapFlagged(object oTrapObject);") 530, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTrapFlagged NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTrapFlagged) },
	{ NWSCRIPT_ACTIONNAME("GetTrapBaseType") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapBaseType(object oTrapObject);") 531, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTrapBaseType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTrapBaseType) },
	{ NWSCRIPT_ACTIONNAME("GetTrapOneShot") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapOneShot(object oTrapObject);") 532, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTrapOneShot NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTrapOneShot) },
	{ NWSCRIPT_ACTIONNAME("GetTrapCreator") NWSCRIPT_ACTIONPROTOTYPE("object GetTrapCreator(object oTrapObject);") 533, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetTrapCreator NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTrapCreator) },
	{ NWSCRIPT_ACTIONNAME("GetTrapKeyTag") NWSCRIPT_ACTIONPROTOTYPE("string GetTrapKeyTag(object oTrapObject);") 534, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetTrapKeyTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTrapKeyTag) },
	{ NWSCRIPT_ACTIONNAME("GetTrapDisarmDC") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapDisarmDC(object oTrapObject);") 535, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTrapDisarmDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTrapDisarmDC) },
	{ NWSCRIPT_ACTIONNAME("GetTrapDetectDC") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapDetectDC(object oTrapObject);") 536, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTrapDetectDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTrapDetectDC) },
	{ NWSCRIPT_ACTIONNAME("GetLockKeyRequired") NWSCRIPT_ACTIONPROTOTYPE("int GetLockKeyRequired(object oObject);") 537, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLockKeyRequired NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLockKeyRequired) },
	{ NWSCRIPT_ACTIONNAME("GetLockKeyTag") NWSCRIPT_ACTIONPROTOTYPE("string GetLockKeyTag(object oObject);") 538, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetLockKeyTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLockKeyTag) },
	{ NWSCRIPT_ACTIONNAME("GetLockLockable") NWSCRIPT_ACTIONPROTOTYPE("int GetLockLockable(object oObject);") 539, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLockLockable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLockLockable) },
	{ NWSCRIPT_ACTIONNAME("GetLockUnlockDC") NWSCRIPT_ACTIONPROTOTYPE("int GetLockUnlockDC(object oObject);") 540, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLockUnlockDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLockUnlockDC) },
	{ NWSCRIPT_ACTIONNAME("GetLockLockDC") NWSCRIPT_ACTIONPROTOTYPE("int GetLockLockDC(object oObject);") 541, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLockLockDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLockLockDC) },
	{ NWSCRIPT_ACTIONNAME("GetPCLevellingUp") NWSCRIPT_ACTIONPROTOTYPE("object GetPCLevellingUp();") 542, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetPCLevellingUp NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPCLevellingUp) },
	{ NWSCRIPT_ACTIONNAME("GetHasFeatEffect") NWSCRIPT_ACTIONPROTOTYPE("int GetHasFeatEffect(int nFeat, object oObject=OBJECT_SELF);") 543, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetHasFeatEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetHasFeatEffect) },
	{ NWSCRIPT_ACTIONNAME("SetPlaceableIllumination") NWSCRIPT_ACTIONPROTOTYPE("void SetPlaceableIllumination(object oPlaceable=OBJECT_SELF, int bIlluminate=TRUE);") 544, 0, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetPlaceableIllumination NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetPlaceableIllumination) },
	{ NWSCRIPT_ACTIONNAME("GetPlaceableIllumination") NWSCRIPT_ACTIONPROTOTYPE("int GetPlaceableIllumination(object oPlaceable=OBJECT_SELF);") 545, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetPlaceableIllumination NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPlaceableIllumination) },
	{ NWSCRIPT_ACTIONNAME("GetIsPlaceableObjectActionPossible") NWSCRIPT_ACTIONPROTOTYPE("int GetIsPlaceableObjectActionPossible(object oPlaceable, int nPlaceableAction);") 546, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsPlaceableObjectActionPossible NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsPlaceableObjectActionPossible) },
	{ NWSCRIPT_ACTIONNAME("DoPlaceableObjectAction") NWSCRIPT_ACTIONPROTOTYPE("void DoPlaceableObjectAction(object oPlaceable, int nPlaceableAction);") 547, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DoPlaceableObjectAction NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DoPlaceableObjectAction) },
	{ NWSCRIPT_ACTIONNAME("GetFirstPC") NWSCRIPT_ACTIONPROTOTYPE("object GetFirstPC(int bOwnedCharacter=TRUE);") 548, 0, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetFirstPC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFirstPC) },
	{ NWSCRIPT_ACTIONNAME("GetNextPC") NWSCRIPT_ACTIONPROTOTYPE("object GetNextPC(int bOwnedCharacter=TRUE);") 549, 0, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetNextPC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNextPC) },
	{ NWSCRIPT_ACTIONNAME("SetTrapDetectedBy") NWSCRIPT_ACTIONPROTOTYPE("int SetTrapDetectedBy(object oTrap, object oDetector);") 550, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_SetTrapDetectedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetTrapDetectedBy) },
	{ NWSCRIPT_ACTIONNAME("GetIsTrapped") NWSCRIPT_ACTIONPROTOTYPE("int GetIsTrapped(object oObject);") 551, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsTrapped NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsTrapped) },
	{ NWSCRIPT_ACTIONNAME("EffectTurnResistanceDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectTurnResistanceDecrease(int nHitDice);") 552, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectTurnResistanceDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectTurnResistanceDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectTurnResistanceIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectTurnResistanceIncrease(int nHitDice);") 553, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectTurnResistanceIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectTurnResistanceIncrease) },
	{ NWSCRIPT_ACTIONNAME("PopUpDeathGUIPanel") NWSCRIPT_ACTIONPROTOTYPE("void PopUpDeathGUIPanel(object oPC, int bRespawnButtonEnabled=TRUE, int bWaitForHelpButtonEnabled=TRUE, int nHelpStringReference=0, string sHelpString="");") 554, 1, 5, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_PopUpDeathGUIPanel NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_PopUpDeathGUIPanel) },
	{ NWSCRIPT_ACTIONNAME("SetTrapDisabled") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapDisabled(object oTrap);") 555, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetTrapDisabled NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetTrapDisabled) },
	{ NWSCRIPT_ACTIONNAME("GetLastHostileActor") NWSCRIPT_ACTIONPROTOTYPE("object GetLastHostileActor(object oVictim=OBJECT_SELF);") 556, 0, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastHostileActor NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastHostileActor) },
	{ NWSCRIPT_ACTIONNAME("ExportAllCharacters") NWSCRIPT_ACTIONPROTOTYPE("void ExportAllCharacters();") 557, 0, 0, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ExportAllCharacters NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ExportAllCharacters) },
	{ NWSCRIPT_ACTIONNAME("MusicBackgroundGetDayTrack") NWSCRIPT_ACTIONPROTOTYPE("int MusicBackgroundGetDayTrack(object oArea);") 558, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_MusicBackgroundGetDayTrack NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_MusicBackgroundGetDayTrack) },
	{ NWSCRIPT_ACTIONNAME("MusicBackgroundGetNightTrack") NWSCRIPT_ACTIONPROTOTYPE("int MusicBackgroundGetNightTrack(object oArea);") 559, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_MusicBackgroundGetNightTrack NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_MusicBackgroundGetNightTrack) },
	{ NWSCRIPT_ACTIONNAME("WriteTimestampedLogEntry") NWSCRIPT_ACTIONPROTOTYPE("void WriteTimestampedLogEntry(string sLogEntry);") 560, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_WriteTimestampedLogEntry NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_WriteTimestampedLogEntry) },
	{ NWSCRIPT_ACTIONNAME("GetModuleName") NWSCRIPT_ACTIONPROTOTYPE("string GetModuleName();") 561, 0, 0, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetModuleName NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetModuleName) },
	{ NWSCRIPT_ACTIONNAME("GetFactionLeader") NWSCRIPT_ACTIONPROTOTYPE("object GetFactionLeader(object oMemberOfFaction);") 562, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetFactionLeader NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFactionLeader) },
	{ NWSCRIPT_ACTIONNAME("SendMessageToAllDMs") NWSCRIPT_ACTIONPROTOTYPE("void SendMessageToAllDMs(string szMessage);") 563, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SendMessageToAllDMs NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SendMessageToAllDMs) },
	{ NWSCRIPT_ACTIONNAME("EndGame") NWSCRIPT_ACTIONPROTOTYPE("void EndGame(string sEndMovie);") 564, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_EndGame NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EndGame) },
	{ NWSCRIPT_ACTIONNAME("BootPC") NWSCRIPT_ACTIONPROTOTYPE("void BootPC(object oPlayer);") 565, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_BootPC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_BootPC) },
	{ NWSCRIPT_ACTIONNAME("ActionCounterSpell") NWSCRIPT_ACTIONPROTOTYPE("void ActionCounterSpell(object oCounterSpellTarget);") 566, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionCounterSpell NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionCounterSpell) },
	{ NWSCRIPT_ACTIONNAME("AmbientSoundSetDayVolume") NWSCRIPT_ACTIONPROTOTYPE("void AmbientSoundSetDayVolume(object oArea, int nVolume);") 567, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AmbientSoundSetDayVolume NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AmbientSoundSetDayVolume) },
	{ NWSCRIPT_ACTIONNAME("AmbientSoundSetNightVolume") NWSCRIPT_ACTIONPROTOTYPE("void AmbientSoundSetNightVolume(object oArea, int nVolume);") 568, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AmbientSoundSetNightVolume NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AmbientSoundSetNightVolume) },
	{ NWSCRIPT_ACTIONNAME("MusicBackgroundGetBattleTrack") NWSCRIPT_ACTIONPROTOTYPE("int MusicBackgroundGetBattleTrack(object oArea);") 569, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_MusicBackgroundGetBattleTrack NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_MusicBackgroundGetBattleTrack) },
	{ NWSCRIPT_ACTIONNAME("GetHasInventory") NWSCRIPT_ACTIONPROTOTYPE("int GetHasInventory(object oObject);") 570, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetHasInventory NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetHasInventory) },
	{ NWSCRIPT_ACTIONNAME("GetStrRefSoundDuration") NWSCRIPT_ACTIONPROTOTYPE("float GetStrRefSoundDuration(int nStrRef);") 571, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_GetStrRefSoundDuration NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetStrRefSoundDuration) },
	{ NWSCRIPT_ACTIONNAME("AddToParty") NWSCRIPT_ACTIONPROTOTYPE("void AddToParty(object oPC, object oPartyLeader);") 572, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AddToParty NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AddToParty) },
	{ NWSCRIPT_ACTIONNAME("RemoveFromParty") NWSCRIPT_ACTIONPROTOTYPE("void RemoveFromParty(object oPC);") 573, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_RemoveFromParty NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RemoveFromParty) },
	{ NWSCRIPT_ACTIONNAME("GetStealthMode") NWSCRIPT_ACTIONPROTOTYPE("int GetStealthMode(object oCreature);") 574, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetStealthMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetStealthMode) },
	{ NWSCRIPT_ACTIONNAME("GetDetectMode") NWSCRIPT_ACTIONPROTOTYPE("int GetDetectMode(object oCreature);") 575, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetDetectMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetDetectMode) },
	{ NWSCRIPT_ACTIONNAME("GetDefensiveCastingMode") NWSCRIPT_ACTIONPROTOTYPE("int GetDefensiveCastingMode(object oCreature);") 576, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetDefensiveCastingMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetDefensiveCastingMode) },
	{ NWSCRIPT_ACTIONNAME("GetAppearanceType") NWSCRIPT_ACTIONPROTOTYPE("int GetAppearanceType(object oCreature);") 577, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetAppearanceType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAppearanceType) },
	{ NWSCRIPT_ACTIONNAME("SpawnScriptDebugger") NWSCRIPT_ACTIONPROTOTYPE("void SpawnScriptDebugger();") 578, 0, 0, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SpawnScriptDebugger NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SpawnScriptDebugger) },
	{ NWSCRIPT_ACTIONNAME("GetModuleItemAcquiredStackSize") NWSCRIPT_ACTIONPROTOTYPE("int GetModuleItemAcquiredStackSize();") 579, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetModuleItemAcquiredStackSize NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetModuleItemAcquiredStackSize) },
	{ NWSCRIPT_ACTIONNAME("DecrementRemainingFeatUses") NWSCRIPT_ACTIONPROTOTYPE("void DecrementRemainingFeatUses(object oCreature, int nFeat);") 580, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DecrementRemainingFeatUses NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DecrementRemainingFeatUses) },
	{ NWSCRIPT_ACTIONNAME("DecrementRemainingSpellUses") NWSCRIPT_ACTIONPROTOTYPE("void DecrementRemainingSpellUses(object oCreature, int nSpell);") 581, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DecrementRemainingSpellUses NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DecrementRemainingSpellUses) },
	{ NWSCRIPT_ACTIONNAME("GetResRef") NWSCRIPT_ACTIONPROTOTYPE("string GetResRef(object oObject);") 582, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetResRef NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetResRef) },
	{ NWSCRIPT_ACTIONNAME("EffectPetrify") NWSCRIPT_ACTIONPROTOTYPE("effect EffectPetrify();") 583, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectPetrify NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectPetrify) },
	{ NWSCRIPT_ACTIONNAME("CopyItem") NWSCRIPT_ACTIONPROTOTYPE("object CopyItem(object oItem, object oTargetInventory=OBJECT_INVALID, int bCopyVars=FALSE);") 584, 1, 3, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_CopyItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_CopyItem) },
	{ NWSCRIPT_ACTIONNAME("EffectCutsceneParalyze") NWSCRIPT_ACTIONPROTOTYPE("effect EffectCutsceneParalyze();") 585, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectCutsceneParalyze NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectCutsceneParalyze) },
	{ NWSCRIPT_ACTIONNAME("GetDroppableFlag") NWSCRIPT_ACTIONPROTOTYPE("int GetDroppableFlag(object oItem);") 586, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetDroppableFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetDroppableFlag) },
	{ NWSCRIPT_ACTIONNAME("GetUseableFlag") NWSCRIPT_ACTIONPROTOTYPE("int GetUseableFlag(object oObject=OBJECT_SELF);") 587, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetUseableFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetUseableFlag) },
	{ NWSCRIPT_ACTIONNAME("GetStolenFlag") NWSCRIPT_ACTIONPROTOTYPE("int GetStolenFlag(object oStolen);") 588, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetStolenFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetStolenFlag) },
	{ NWSCRIPT_ACTIONNAME("SetCampaignFloat") NWSCRIPT_ACTIONPROTOTYPE("void SetCampaignFloat(string sCampaignName, string sVarName, float flFloat, object oPlayer=OBJECT_INVALID);") 589, 3, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCampaignFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCampaignFloat) },
	{ NWSCRIPT_ACTIONNAME("SetCampaignInt") NWSCRIPT_ACTIONPROTOTYPE("void SetCampaignInt(string sCampaignName, string sVarName, int nInt, object oPlayer=OBJECT_INVALID);") 590, 3, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCampaignInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCampaignInt) },
	{ NWSCRIPT_ACTIONNAME("SetCampaignVector") NWSCRIPT_ACTIONPROTOTYPE("void SetCampaignVector(string sCampaignName, string sVarName, vector vVector, object oPlayer=OBJECT_INVALID);") 591, 3, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCampaignVector NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCampaignVector) },
	{ NWSCRIPT_ACTIONNAME("SetCampaignLocation") NWSCRIPT_ACTIONPROTOTYPE("void SetCampaignLocation(string sCampaignName, string sVarName, location locLocation, object oPlayer=OBJECT_INVALID);") 592, 3, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCampaignLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCampaignLocation) },
	{ NWSCRIPT_ACTIONNAME("SetCampaignString") NWSCRIPT_ACTIONPROTOTYPE("void SetCampaignString(string sCampaignName, string sVarName, string sString, object oPlayer=OBJECT_INVALID);") 593, 3, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCampaignString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCampaignString) },
	{ NWSCRIPT_ACTIONNAME("DestroyCampaignDatabase") NWSCRIPT_ACTIONPROTOTYPE("void DestroyCampaignDatabase(string sCampaignName);") 594, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DestroyCampaignDatabase NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DestroyCampaignDatabase) },
	{ NWSCRIPT_ACTIONNAME("GetCampaignFloat") NWSCRIPT_ACTIONPROTOTYPE("float GetCampaignFloat(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);") 595, 2, 3, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_GetCampaignFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCampaignFloat) },
	{ NWSCRIPT_ACTIONNAME("GetCampaignInt") NWSCRIPT_ACTIONPROTOTYPE("int GetCampaignInt(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);") 596, 2, 3, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetCampaignInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCampaignInt) },
	{ NWSCRIPT_ACTIONNAME("GetCampaignVector") NWSCRIPT_ACTIONPROTOTYPE("vector GetCampaignVector(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);") 597, 2, 3, ACTIONTYPE_VECTOR, NWN2_NWActionParameterTypes_GetCampaignVector NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCampaignVector) },
	{ NWSCRIPT_ACTIONNAME("GetCampaignLocation") NWSCRIPT_ACTIONPROTOTYPE("location GetCampaignLocation(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);") 598, 2, 3, ACTIONTYPE_LOCATION, NWN2_NWActionParameterTypes_GetCampaignLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCampaignLocation) },
	{ NWSCRIPT_ACTIONNAME("GetCampaignString") NWSCRIPT_ACTIONPROTOTYPE("string GetCampaignString(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);") 599, 2, 3, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetCampaignString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCampaignString) },
	{ NWSCRIPT_ACTIONNAME("CopyObject") NWSCRIPT_ACTIONPROTOTYPE("object CopyObject(object oSource, location locLocation, object oOwner = OBJECT_INVALID, string sNewTag = "");") 600, 2, 4, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_CopyObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_CopyObject) },
	{ NWSCRIPT_ACTIONNAME("DeleteCampaignVariable") NWSCRIPT_ACTIONPROTOTYPE("void DeleteCampaignVariable(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);") 601, 2, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DeleteCampaignVariable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DeleteCampaignVariable) },
	{ NWSCRIPT_ACTIONNAME("StoreCampaignObject") NWSCRIPT_ACTIONPROTOTYPE("int StoreCampaignObject(string sCampaignName, string sVarName, object oObject, object oPlayer=OBJECT_INVALID);") 602, 3, 4, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_StoreCampaignObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_StoreCampaignObject) },
	{ NWSCRIPT_ACTIONNAME("RetrieveCampaignObject") NWSCRIPT_ACTIONPROTOTYPE("object RetrieveCampaignObject(string sCampaignName, string sVarName, location locLocation, object oOwner = OBJECT_INVALID, object oPlayer=OBJECT_INVALID);") 603, 3, 5, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_RetrieveCampaignObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RetrieveCampaignObject) },
	{ NWSCRIPT_ACTIONNAME("EffectCutsceneDominated") NWSCRIPT_ACTIONPROTOTYPE("effect EffectCutsceneDominated();") 604, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectCutsceneDominated NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectCutsceneDominated) },
	{ NWSCRIPT_ACTIONNAME("GetItemStackSize") NWSCRIPT_ACTIONPROTOTYPE("int GetItemStackSize(object oItem);") 605, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetItemStackSize NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemStackSize) },
	{ NWSCRIPT_ACTIONNAME("SetItemStackSize") NWSCRIPT_ACTIONPROTOTYPE("void SetItemStackSize(object oItem, int nSize, int bDisplayFeedback=TRUE);") 606, 2, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetItemStackSize NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetItemStackSize) },
	{ NWSCRIPT_ACTIONNAME("GetItemCharges") NWSCRIPT_ACTIONPROTOTYPE("int GetItemCharges(object oItem);") 607, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetItemCharges NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemCharges) },
	{ NWSCRIPT_ACTIONNAME("SetItemCharges") NWSCRIPT_ACTIONPROTOTYPE("void SetItemCharges(object oItem, int nCharges);") 608, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetItemCharges NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetItemCharges) },
	{ NWSCRIPT_ACTIONNAME("AddItemProperty") NWSCRIPT_ACTIONPROTOTYPE("void AddItemProperty(int nDurationType, itemproperty ipProperty, object oItem, float fDuration=0.0f);") 609, 3, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AddItemProperty NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AddItemProperty) },
	{ NWSCRIPT_ACTIONNAME("RemoveItemProperty") NWSCRIPT_ACTIONPROTOTYPE("void RemoveItemProperty(object oItem, itemproperty ipProperty);") 610, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_RemoveItemProperty NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RemoveItemProperty) },
	{ NWSCRIPT_ACTIONNAME("GetIsItemPropertyValid") NWSCRIPT_ACTIONPROTOTYPE("int GetIsItemPropertyValid(itemproperty ipProperty);") 611, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsItemPropertyValid NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsItemPropertyValid) },
	{ NWSCRIPT_ACTIONNAME("GetFirstItemProperty") NWSCRIPT_ACTIONPROTOTYPE("itemproperty GetFirstItemProperty(object oItem);") 612, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_GetFirstItemProperty NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFirstItemProperty) },
	{ NWSCRIPT_ACTIONNAME("GetNextItemProperty") NWSCRIPT_ACTIONPROTOTYPE("itemproperty GetNextItemProperty(object oItem);") 613, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_GetNextItemProperty NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNextItemProperty) },
	{ NWSCRIPT_ACTIONNAME("GetItemPropertyType") NWSCRIPT_ACTIONPROTOTYPE("int GetItemPropertyType(itemproperty ip);") 614, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetItemPropertyType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemPropertyType) },
	{ NWSCRIPT_ACTIONNAME("GetItemPropertyDurationType") NWSCRIPT_ACTIONPROTOTYPE("int GetItemPropertyDurationType(itemproperty ip);") 615, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetItemPropertyDurationType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemPropertyDurationType) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyAbilityBonus") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyAbilityBonus(int nAbility, int nBonus);") 616, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyAbilityBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyAbilityBonus) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyACBonus") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyACBonus(int nBonus);") 617, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyACBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyACBonus) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyACBonusVsAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyACBonusVsAlign(int nAlignGroup, int nACBonus);") 618, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyACBonusVsAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyACBonusVsAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyACBonusVsDmgType") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyACBonusVsDmgType(int nDamageType, int nACBonus);") 619, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyACBonusVsDmgType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyACBonusVsDmgType) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyACBonusVsRace") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyACBonusVsRace(int nRace, int nACBonus);") 620, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyACBonusVsRace NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyACBonusVsRace) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyACBonusVsSAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyACBonusVsSAlign(int nAlign, int nACBonus);") 621, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyACBonusVsSAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyACBonusVsSAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyEnhancementBonus") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyEnhancementBonus(int nEnhancementBonus);") 622, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyEnhancementBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyEnhancementBonus) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyEnhancementBonusVsAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyEnhancementBonusVsAlign(int nAlignGroup, int nBonus);") 623, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyEnhancementBonusVsAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyEnhancementBonusVsAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyEnhancementBonusVsRace") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyEnhancementBonusVsRace(int nRace, int nBonus);") 624, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyEnhancementBonusVsRace NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyEnhancementBonusVsRace) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyEnhancementBonusVsSAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyEnhancementBonusVsSAlign(int nAlign, int nBonus);") 625, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyEnhancementBonusVsSAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyEnhancementBonusVsSAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyEnhancementPenalty") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyEnhancementPenalty(int nPenalty);") 626, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyEnhancementPenalty NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyEnhancementPenalty) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyWeightReduction") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyWeightReduction(int nReduction);") 627, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyWeightReduction NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyWeightReduction) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyBonusFeat") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyBonusFeat(int nFeat);") 628, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyBonusFeat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyBonusFeat) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyBonusLevelSpell") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyBonusLevelSpell(int nClass, int nSpellLevel);") 629, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyBonusLevelSpell NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyBonusLevelSpell) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyCastSpell") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyCastSpell(int nSpell, int nNumUses);") 630, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyCastSpell NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyCastSpell) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamageBonus") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamageBonus(int nDamageType, int nDamage);") 631, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyDamageBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyDamageBonus) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamageBonusVsAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamageBonusVsAlign(int nAlignGroup, int nDamageType, int nDamage);") 632, 3, 3, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyDamageBonusVsAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyDamageBonusVsAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamageBonusVsRace") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamageBonusVsRace(int nRace, int nDamageType, int nDamage);") 633, 3, 3, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyDamageBonusVsRace NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyDamageBonusVsRace) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamageBonusVsSAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamageBonusVsSAlign(int nAlign, int nDamageType, int nDamage);") 634, 3, 3, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyDamageBonusVsSAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyDamageBonusVsSAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamageImmunity") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamageImmunity(int nDamageType, int nImmuneBonus);") 635, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyDamageImmunity NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyDamageImmunity) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamagePenalty") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamagePenalty(int nPenalty);") 636, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyDamagePenalty NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyDamagePenalty) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamageReduction") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamageReduction(int nAmount, int nDRSubType, int nLimit=0, int nDRType=DR_TYPE_MAGICBONUS);") 637, 2, 4, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyDamageReduction NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyDamageReduction) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamageResistance") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamageResistance(int nDamageType, int nHPResist);") 638, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyDamageResistance NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyDamageResistance) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamageVulnerability") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamageVulnerability(int nDamageType, int nVulnerability);") 639, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyDamageVulnerability NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyDamageVulnerability) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDarkvision") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDarkvision();") 640, 0, 0, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyDarkvision NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyDarkvision) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDecreaseAbility") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDecreaseAbility(int nAbility, int nModifier);") 641, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyDecreaseAbility NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyDecreaseAbility) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDecreaseAC") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDecreaseAC(int nModifierType, int nPenalty);") 642, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyDecreaseAC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyDecreaseAC) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDecreaseSkill") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDecreaseSkill(int nSkill, int nPenalty);") 643, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyDecreaseSkill NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyDecreaseSkill) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyContainerReducedWeight") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyContainerReducedWeight(int nContainerType);") 644, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyContainerReducedWeight NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyContainerReducedWeight) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyExtraMeleeDamageType") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyExtraMeleeDamageType(int nDamageType);") 645, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyExtraMeleeDamageType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyExtraMeleeDamageType) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyExtraRangeDamageType") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyExtraRangeDamageType(int nDamageType);") 646, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyExtraRangeDamageType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyExtraRangeDamageType) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyHaste") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyHaste();") 647, 0, 0, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyHaste NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyHaste) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyHolyAvenger") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyHolyAvenger();") 648, 0, 0, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyHolyAvenger NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyHolyAvenger) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyImmunityMisc") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyImmunityMisc(int nImmunityType);") 649, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyImmunityMisc NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyImmunityMisc) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyImprovedEvasion") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyImprovedEvasion();") 650, 0, 0, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyImprovedEvasion NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyImprovedEvasion) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyBonusSpellResistance") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyBonusSpellResistance(int nBonus);") 651, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyBonusSpellResistance NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyBonusSpellResistance) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyBonusSavingThrowVsX") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyBonusSavingThrowVsX(int nBonusType, int nBonus);") 652, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyBonusSavingThrowVsX NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyBonusSavingThrowVsX) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyBonusSavingThrow") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyBonusSavingThrow(int nBaseSaveType, int nBonus);") 653, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyBonusSavingThrow NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyBonusSavingThrow) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyKeen") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyKeen();") 654, 0, 0, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyKeen NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyKeen) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyLight") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyLight(int nBrightness, int nColor);") 655, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyLight NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyLight) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyMaxRangeStrengthMod") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyMaxRangeStrengthMod(int nModifier);") 656, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyMaxRangeStrengthMod NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyMaxRangeStrengthMod) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyNoDamage") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyNoDamage();") 657, 0, 0, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyNoDamage NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyNoDamage) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyOnHitProps") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyOnHitProps(int nProperty, int nSaveDC, int nSpecial=0);") 658, 2, 3, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyOnHitProps NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyOnHitProps) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyReducedSavingThrowVsX") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyReducedSavingThrowVsX(int nBaseSaveType, int nPenalty);") 659, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyReducedSavingThrowVsX NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyReducedSavingThrowVsX) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyReducedSavingThrow") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyReducedSavingThrow(int nBonusType, int nPenalty);") 660, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyReducedSavingThrow NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyReducedSavingThrow) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyRegeneration") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyRegeneration(int nRegenAmount);") 661, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyRegeneration NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyRegeneration) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertySkillBonus") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertySkillBonus(int nSkill, int nBonus);") 662, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertySkillBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertySkillBonus) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertySpellImmunitySpecific") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertySpellImmunitySpecific(int nSpell);") 663, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertySpellImmunitySpecific NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertySpellImmunitySpecific) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertySpellImmunitySchool") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertySpellImmunitySchool(int nSchool);") 664, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertySpellImmunitySchool NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertySpellImmunitySchool) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyThievesTools") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyThievesTools(int nModifier);") 665, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyThievesTools NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyThievesTools) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyAttackBonus") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyAttackBonus(int nBonus);") 666, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyAttackBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyAttackBonus) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyAttackBonusVsAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyAttackBonusVsAlign(int nAlignGroup, int nBonus);") 667, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyAttackBonusVsAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyAttackBonusVsAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyAttackBonusVsRace") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyAttackBonusVsRace(int nRace, int nBonus);") 668, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyAttackBonusVsRace NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyAttackBonusVsRace) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyAttackBonusVsSAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyAttackBonusVsSAlign(int nAlignment, int nBonus);") 669, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyAttackBonusVsSAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyAttackBonusVsSAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyAttackPenalty") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyAttackPenalty(int nPenalty);") 670, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyAttackPenalty NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyAttackPenalty) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyUnlimitedAmmo") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyUnlimitedAmmo(int nAmmoDamage=IP_CONST_UNLIMITEDAMMO_BASIC);") 671, 0, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyUnlimitedAmmo NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyUnlimitedAmmo) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyLimitUseByAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyLimitUseByAlign(int nAlignGroup);") 672, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyLimitUseByAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyLimitUseByAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyLimitUseByClass") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyLimitUseByClass(int nClass);") 673, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyLimitUseByClass NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyLimitUseByClass) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyLimitUseByRace") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyLimitUseByRace(int nRace);") 674, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyLimitUseByRace NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyLimitUseByRace) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyLimitUseBySAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyLimitUseBySAlign(int nAlignment);") 675, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyLimitUseBySAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyLimitUseBySAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyBonusHitpoints") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyBonusHitpoints(int nBonusType);") 676, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyBonusHitpoints NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyBonusHitpoints) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyVampiricRegeneration") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyVampiricRegeneration(int nRegenAmount);") 677, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyVampiricRegeneration NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyVampiricRegeneration) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyTrap") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyTrap(int nTrapLevel, int nTrapType);") 678, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyTrap NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyTrap) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyTrueSeeing") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyTrueSeeing();") 679, 0, 0, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyTrueSeeing NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyTrueSeeing) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyOnMonsterHitProperties") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyOnMonsterHitProperties(int nProperty, int nSpecial=0);") 680, 1, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyOnMonsterHitProperties NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyOnMonsterHitProperties) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyTurnResistance") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyTurnResistance(int nModifier);") 681, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyTurnResistance NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyTurnResistance) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyMassiveCritical") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyMassiveCritical(int nDamage);") 682, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyMassiveCritical NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyMassiveCritical) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyFreeAction") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyFreeAction();") 683, 0, 0, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyFreeAction NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyFreeAction) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyMonsterDamage") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyMonsterDamage(int nDamage);") 684, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyMonsterDamage NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyMonsterDamage) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyImmunityToSpellLevel") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyImmunityToSpellLevel(int nLevel);") 685, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyImmunityToSpellLevel NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyImmunityToSpellLevel) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertySpecialWalk") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertySpecialWalk(int nWalkType=0);") 686, 0, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertySpecialWalk NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertySpecialWalk) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyHealersKit") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyHealersKit(int nModifier);") 687, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyHealersKit NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyHealersKit) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyWeightIncrease") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyWeightIncrease(int nWeight);") 688, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyWeightIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyWeightIncrease) },
	{ NWSCRIPT_ACTIONNAME("GetIsSkillSuccessful") NWSCRIPT_ACTIONPROTOTYPE("int GetIsSkillSuccessful(object oTarget, int nSkill, int nDifficulty, int bDisplayFeedback=TRUE);") 689, 3, 4, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsSkillSuccessful NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsSkillSuccessful) },
	{ NWSCRIPT_ACTIONNAME("EffectSpellFailure") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSpellFailure(int nPercent=100, int nSpellSchool=SPELL_SCHOOL_GENERAL);") 690, 0, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSpellFailure NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSpellFailure) },
	{ NWSCRIPT_ACTIONNAME("SpeakStringByStrRef") NWSCRIPT_ACTIONPROTOTYPE("void SpeakStringByStrRef(int nStrRef, int nTalkVolume=TALKVOLUME_TALK);") 691, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SpeakStringByStrRef NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SpeakStringByStrRef) },
	{ NWSCRIPT_ACTIONNAME("SetCutsceneMode") NWSCRIPT_ACTIONPROTOTYPE("void SetCutsceneMode(object oCreature, int nInCutscene=TRUE);") 692, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCutsceneMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCutsceneMode) },
	{ NWSCRIPT_ACTIONNAME("GetLastPCToCancelCutscene") NWSCRIPT_ACTIONPROTOTYPE("object GetLastPCToCancelCutscene();") 693, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetLastPCToCancelCutscene NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastPCToCancelCutscene) },
	{ NWSCRIPT_ACTIONNAME("GetDialogSoundLength") NWSCRIPT_ACTIONPROTOTYPE("float GetDialogSoundLength(int nStrRef);") 694, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_GetDialogSoundLength NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetDialogSoundLength) },
	{ NWSCRIPT_ACTIONNAME("FadeFromBlack") NWSCRIPT_ACTIONPROTOTYPE("void FadeFromBlack(object oCreature, float fSpeed=FADE_SPEED_MEDIUM);") 695, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_FadeFromBlack NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_FadeFromBlack) },
	{ NWSCRIPT_ACTIONNAME("FadeToBlack") NWSCRIPT_ACTIONPROTOTYPE("void FadeToBlack(object oCreature, float fSpeed=FADE_SPEED_MEDIUM, float fFailsafe=5.0, int nColor=0);") 696, 1, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_FadeToBlack NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_FadeToBlack) },
	{ NWSCRIPT_ACTIONNAME("StopFade") NWSCRIPT_ACTIONPROTOTYPE("void StopFade(object oCreature);") 697, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_StopFade NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_StopFade) },
	{ NWSCRIPT_ACTIONNAME("BlackScreen") NWSCRIPT_ACTIONPROTOTYPE("void BlackScreen(object oCreature, int nColor=0);") 698, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_BlackScreen NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_BlackScreen) },
	{ NWSCRIPT_ACTIONNAME("GetBaseAttackBonus") NWSCRIPT_ACTIONPROTOTYPE("int GetBaseAttackBonus(object oCreature);") 699, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetBaseAttackBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetBaseAttackBonus) },
	{ NWSCRIPT_ACTIONNAME("SetImmortal") NWSCRIPT_ACTIONPROTOTYPE("void SetImmortal(object oCreature, int bImmortal);") 700, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetImmortal NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetImmortal) },
	{ NWSCRIPT_ACTIONNAME("OpenInventory") NWSCRIPT_ACTIONPROTOTYPE("void OpenInventory(object oCreature, object oPlayer);") 701, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_OpenInventory NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_OpenInventory) },
	{ NWSCRIPT_ACTIONNAME("StoreCameraFacing") NWSCRIPT_ACTIONPROTOTYPE("void StoreCameraFacing();") 702, 0, 0, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_StoreCameraFacing NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_StoreCameraFacing) },
	{ NWSCRIPT_ACTIONNAME("RestoreCameraFacing") NWSCRIPT_ACTIONPROTOTYPE("void RestoreCameraFacing();") 703, 0, 0, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_RestoreCameraFacing NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RestoreCameraFacing) },
	{ NWSCRIPT_ACTIONNAME("LevelUpHenchman") NWSCRIPT_ACTIONPROTOTYPE("int LevelUpHenchman(object oCreature, int nClass = CLASS_TYPE_INVALID, int bReadyAllSpells = FALSE, int nPackage = PACKAGE_INVALID);") 704, 1, 4, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_LevelUpHenchman NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_LevelUpHenchman) },
	{ NWSCRIPT_ACTIONNAME("SetDroppableFlag") NWSCRIPT_ACTIONPROTOTYPE("void SetDroppableFlag(object oItem, int bDroppable);") 705, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetDroppableFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetDroppableFlag) },
	{ NWSCRIPT_ACTIONNAME("GetWeight") NWSCRIPT_ACTIONPROTOTYPE("int GetWeight(object oTarget=OBJECT_SELF);") 706, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetWeight NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetWeight) },
	{ NWSCRIPT_ACTIONNAME("GetModuleItemAcquiredBy") NWSCRIPT_ACTIONPROTOTYPE("object GetModuleItemAcquiredBy();") 707, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetModuleItemAcquiredBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetModuleItemAcquiredBy) },
	{ NWSCRIPT_ACTIONNAME("GetImmortal") NWSCRIPT_ACTIONPROTOTYPE("int GetImmortal(object oTarget=OBJECT_SELF);") 708, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetImmortal NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetImmortal) },
	{ NWSCRIPT_ACTIONNAME("DoWhirlwindAttack") NWSCRIPT_ACTIONPROTOTYPE("void DoWhirlwindAttack(int bDisplayFeedback=TRUE, int bImproved=FALSE);") 709, 0, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DoWhirlwindAttack NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DoWhirlwindAttack) },
	{ NWSCRIPT_ACTIONNAME("Get2DAString") NWSCRIPT_ACTIONPROTOTYPE("string Get2DAString(string s2DA, string sColumn, int nRow);") 710, 3, 3, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_Get2DAString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_Get2DAString) },
	{ NWSCRIPT_ACTIONNAME("EffectEthereal") NWSCRIPT_ACTIONPROTOTYPE("effect EffectEthereal();") 711, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectEthereal NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectEthereal) },
	{ NWSCRIPT_ACTIONNAME("GetAILevel") NWSCRIPT_ACTIONPROTOTYPE("int GetAILevel(object oTarget=OBJECT_SELF);") 712, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetAILevel NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAILevel) },
	{ NWSCRIPT_ACTIONNAME("SetAILevel") NWSCRIPT_ACTIONPROTOTYPE("void SetAILevel(object oTarget, int nAILevel);") 713, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetAILevel NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetAILevel) },
	{ NWSCRIPT_ACTIONNAME("GetIsPossessedFamiliar") NWSCRIPT_ACTIONPROTOTYPE("int GetIsPossessedFamiliar(object oCreature);") 714, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsPossessedFamiliar NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsPossessedFamiliar) },
	{ NWSCRIPT_ACTIONNAME("UnpossessFamiliar") NWSCRIPT_ACTIONPROTOTYPE("void UnpossessFamiliar(object oCreature);") 715, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_UnpossessFamiliar NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_UnpossessFamiliar) },
	{ NWSCRIPT_ACTIONNAME("GetIsAreaInterior") NWSCRIPT_ACTIONPROTOTYPE("int GetIsAreaInterior( object oArea = OBJECT_INVALID );") 716, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsAreaInterior NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsAreaInterior) },
	{ NWSCRIPT_ACTIONNAME("SendMessageToPCByStrRef") NWSCRIPT_ACTIONPROTOTYPE("void SendMessageToPCByStrRef(object oPlayer, int nStrRef);") 717, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SendMessageToPCByStrRef NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SendMessageToPCByStrRef) },
	{ NWSCRIPT_ACTIONNAME("IncrementRemainingFeatUses") NWSCRIPT_ACTIONPROTOTYPE("void IncrementRemainingFeatUses(object oCreature, int nFeat);") 718, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_IncrementRemainingFeatUses NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_IncrementRemainingFeatUses) },
	{ NWSCRIPT_ACTIONNAME("ExportSingleCharacter") NWSCRIPT_ACTIONPROTOTYPE("void ExportSingleCharacter(object oPlayer);") 719, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ExportSingleCharacter NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ExportSingleCharacter) },
	{ NWSCRIPT_ACTIONNAME("PlaySoundByStrRef") NWSCRIPT_ACTIONPROTOTYPE("void PlaySoundByStrRef(int nStrRef, int nRunAsAction = TRUE );") 720, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_PlaySoundByStrRef NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_PlaySoundByStrRef) },
	{ NWSCRIPT_ACTIONNAME("SetSubRace") NWSCRIPT_ACTIONPROTOTYPE("void SetSubRace(object oCreature, string sSubRace);") 721, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetSubRace NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetSubRace) },
	{ NWSCRIPT_ACTIONNAME("SetDeity") NWSCRIPT_ACTIONPROTOTYPE("void SetDeity(object oCreature, string sDeity);") 722, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetDeity NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetDeity) },
	{ NWSCRIPT_ACTIONNAME("GetIsDMPossessed") NWSCRIPT_ACTIONPROTOTYPE("int GetIsDMPossessed(object oCreature);") 723, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsDMPossessed NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsDMPossessed) },
	{ NWSCRIPT_ACTIONNAME("GetWeather") NWSCRIPT_ACTIONPROTOTYPE("int GetWeather(object oArea, int nWeatherType);") 724, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetWeather NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetWeather) },
	{ NWSCRIPT_ACTIONNAME("GetIsAreaNatural") NWSCRIPT_ACTIONPROTOTYPE("int GetIsAreaNatural(object oArea);") 725, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsAreaNatural NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsAreaNatural) },
	{ NWSCRIPT_ACTIONNAME("GetIsAreaAboveGround") NWSCRIPT_ACTIONPROTOTYPE("int GetIsAreaAboveGround(object oArea);") 726, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsAreaAboveGround NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsAreaAboveGround) },
	{ NWSCRIPT_ACTIONNAME("GetPCItemLastEquipped") NWSCRIPT_ACTIONPROTOTYPE("object GetPCItemLastEquipped();") 727, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetPCItemLastEquipped NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPCItemLastEquipped) },
	{ NWSCRIPT_ACTIONNAME("GetPCItemLastEquippedBy") NWSCRIPT_ACTIONPROTOTYPE("object GetPCItemLastEquippedBy();") 728, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetPCItemLastEquippedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPCItemLastEquippedBy) },
	{ NWSCRIPT_ACTIONNAME("GetPCItemLastUnequipped") NWSCRIPT_ACTIONPROTOTYPE("object GetPCItemLastUnequipped();") 729, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetPCItemLastUnequipped NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPCItemLastUnequipped) },
	{ NWSCRIPT_ACTIONNAME("GetPCItemLastUnequippedBy") NWSCRIPT_ACTIONPROTOTYPE("object GetPCItemLastUnequippedBy();") 730, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetPCItemLastUnequippedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPCItemLastUnequippedBy) },
	{ NWSCRIPT_ACTIONNAME("CopyItemAndModify") NWSCRIPT_ACTIONPROTOTYPE("object CopyItemAndModify(object oItem, int nType, int nIndex, int nNewValue, int bCopyVars=FALSE);") 731, 4, 5, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_CopyItemAndModify NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_CopyItemAndModify) },
	{ NWSCRIPT_ACTIONNAME("GetItemAppearance") NWSCRIPT_ACTIONPROTOTYPE("int GetItemAppearance(object oItem, int nType, int nIndex);") 732, 3, 3, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetItemAppearance NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemAppearance) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyOnHitCastSpell") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyOnHitCastSpell(int nSpell, int nLevel);") 733, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyOnHitCastSpell NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyOnHitCastSpell) },
	{ NWSCRIPT_ACTIONNAME("GetItemPropertySubType") NWSCRIPT_ACTIONPROTOTYPE("int GetItemPropertySubType(itemproperty iProperty);") 734, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetItemPropertySubType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemPropertySubType) },
	{ NWSCRIPT_ACTIONNAME("GetActionMode") NWSCRIPT_ACTIONPROTOTYPE("int GetActionMode(object oCreature, int nMode);") 735, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetActionMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetActionMode) },
	{ NWSCRIPT_ACTIONNAME("SetActionMode") NWSCRIPT_ACTIONPROTOTYPE("void SetActionMode(object oCreature, int nMode, int nStatus);") 736, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetActionMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetActionMode) },
	{ NWSCRIPT_ACTIONNAME("GetArcaneSpellFailure") NWSCRIPT_ACTIONPROTOTYPE("int GetArcaneSpellFailure(object oCreature);") 737, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetArcaneSpellFailure NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetArcaneSpellFailure) },
	{ NWSCRIPT_ACTIONNAME("ActionExamine") NWSCRIPT_ACTIONPROTOTYPE("void ActionExamine(object oExamine);") 738, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionExamine NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionExamine) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyVisualEffect") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyVisualEffect(int nEffect);") 739, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyVisualEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyVisualEffect) },
	{ NWSCRIPT_ACTIONNAME("SetLootable") NWSCRIPT_ACTIONPROTOTYPE("void SetLootable( object oCreature, int bLootable );") 740, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetLootable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetLootable) },
	{ NWSCRIPT_ACTIONNAME("GetLootable") NWSCRIPT_ACTIONPROTOTYPE("int GetLootable( object oCreature );") 741, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLootable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLootable) },
	{ NWSCRIPT_ACTIONNAME("GetCutsceneCameraMoveRate") NWSCRIPT_ACTIONPROTOTYPE("float GetCutsceneCameraMoveRate( object oCreature );") 742, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_GetCutsceneCameraMoveRate NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCutsceneCameraMoveRate) },
	{ NWSCRIPT_ACTIONNAME("SetCutsceneCameraMoveRate") NWSCRIPT_ACTIONPROTOTYPE("void SetCutsceneCameraMoveRate( object oCreature, float fRate );") 743, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCutsceneCameraMoveRate NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCutsceneCameraMoveRate) },
	{ NWSCRIPT_ACTIONNAME("GetItemCursedFlag") NWSCRIPT_ACTIONPROTOTYPE("int GetItemCursedFlag(object oItem);") 744, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetItemCursedFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemCursedFlag) },
	{ NWSCRIPT_ACTIONNAME("SetItemCursedFlag") NWSCRIPT_ACTIONPROTOTYPE("void SetItemCursedFlag(object oItem, int nCursed);") 745, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetItemCursedFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetItemCursedFlag) },
	{ NWSCRIPT_ACTIONNAME("SetMaxHenchmen") NWSCRIPT_ACTIONPROTOTYPE("void SetMaxHenchmen( int nNumHenchmen );") 746, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetMaxHenchmen NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetMaxHenchmen) },
	{ NWSCRIPT_ACTIONNAME("GetMaxHenchmen") NWSCRIPT_ACTIONPROTOTYPE("int GetMaxHenchmen();") 747, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetMaxHenchmen NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetMaxHenchmen) },
	{ NWSCRIPT_ACTIONNAME("GetAssociateType") NWSCRIPT_ACTIONPROTOTYPE("int GetAssociateType( object oAssociate );") 748, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetAssociateType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAssociateType) },
	{ NWSCRIPT_ACTIONNAME("GetSpellResistance") NWSCRIPT_ACTIONPROTOTYPE("int GetSpellResistance( object oCreature );") 749, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetSpellResistance NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetSpellResistance) },
	{ NWSCRIPT_ACTIONNAME("DayToNight") NWSCRIPT_ACTIONPROTOTYPE("void DayToNight(object oPlayer, float fTransitionTime=0.0f);") 750, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DayToNight NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DayToNight) },
	{ NWSCRIPT_ACTIONNAME("NightToDay") NWSCRIPT_ACTIONPROTOTYPE("void NightToDay(object oPlayer, float fTransitionTime=0.0f);") 751, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_NightToDay NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_NightToDay) },
	{ NWSCRIPT_ACTIONNAME("LineOfSightObject") NWSCRIPT_ACTIONPROTOTYPE("int LineOfSightObject( object oSource, object oTarget );") 752, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_LineOfSightObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_LineOfSightObject) },
	{ NWSCRIPT_ACTIONNAME("LineOfSightVector") NWSCRIPT_ACTIONPROTOTYPE("int LineOfSightVector( vector vSource, vector vTarget );") 753, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_LineOfSightVector NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_LineOfSightVector) },
	{ NWSCRIPT_ACTIONNAME("GetLastSpellCastClass") NWSCRIPT_ACTIONPROTOTYPE("int GetLastSpellCastClass();") 754, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLastSpellCastClass NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastSpellCastClass) },
	{ NWSCRIPT_ACTIONNAME("SetBaseAttackBonus") NWSCRIPT_ACTIONPROTOTYPE("void SetBaseAttackBonus( int nBaseAttackBonus, object oCreature = OBJECT_SELF );") 755, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetBaseAttackBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetBaseAttackBonus) },
	{ NWSCRIPT_ACTIONNAME("RestoreBaseAttackBonus") NWSCRIPT_ACTIONPROTOTYPE("void RestoreBaseAttackBonus( object oCreature = OBJECT_SELF );") 756, 0, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_RestoreBaseAttackBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RestoreBaseAttackBonus) },
	{ NWSCRIPT_ACTIONNAME("EffectCutsceneGhost") NWSCRIPT_ACTIONPROTOTYPE("effect EffectCutsceneGhost();") 757, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectCutsceneGhost NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectCutsceneGhost) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyArcaneSpellFailure") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyArcaneSpellFailure(int nModLevel);") 758, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN2_NWActionParameterTypes_ItemPropertyArcaneSpellFailure NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ItemPropertyArcaneSpellFailure) },
	{ NWSCRIPT_ACTIONNAME("GetStoreGold") NWSCRIPT_ACTIONPROTOTYPE("int GetStoreGold(object oidStore);") 759, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetStoreGold NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetStoreGold) },
	{ NWSCRIPT_ACTIONNAME("SetStoreGold") NWSCRIPT_ACTIONPROTOTYPE("void SetStoreGold(object oidStore, int nGold);") 760, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetStoreGold NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetStoreGold) },
	{ NWSCRIPT_ACTIONNAME("GetStoreMaxBuyPrice") NWSCRIPT_ACTIONPROTOTYPE("int GetStoreMaxBuyPrice(object oidStore);") 761, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetStoreMaxBuyPrice NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetStoreMaxBuyPrice) },
	{ NWSCRIPT_ACTIONNAME("SetStoreMaxBuyPrice") NWSCRIPT_ACTIONPROTOTYPE("void SetStoreMaxBuyPrice(object oidStore, int nMaxBuy);") 762, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetStoreMaxBuyPrice NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetStoreMaxBuyPrice) },
	{ NWSCRIPT_ACTIONNAME("GetStoreIdentifyCost") NWSCRIPT_ACTIONPROTOTYPE("int GetStoreIdentifyCost(object oidStore);") 763, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetStoreIdentifyCost NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetStoreIdentifyCost) },
	{ NWSCRIPT_ACTIONNAME("SetStoreIdentifyCost") NWSCRIPT_ACTIONPROTOTYPE("void SetStoreIdentifyCost(object oidStore, int nCost);") 764, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetStoreIdentifyCost NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetStoreIdentifyCost) },
	{ NWSCRIPT_ACTIONNAME("SetCreatureAppearanceType") NWSCRIPT_ACTIONPROTOTYPE("void SetCreatureAppearanceType(object oCreature, int nAppearanceType);") 765, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCreatureAppearanceType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCreatureAppearanceType) },
	{ NWSCRIPT_ACTIONNAME("GetCreatureStartingPackage") NWSCRIPT_ACTIONPROTOTYPE("int GetCreatureStartingPackage(object oCreature);") 766, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetCreatureStartingPackage NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCreatureStartingPackage) },
	{ NWSCRIPT_ACTIONNAME("EffectCutsceneImmobilize") NWSCRIPT_ACTIONPROTOTYPE("effect EffectCutsceneImmobilize();") 767, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectCutsceneImmobilize NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectCutsceneImmobilize) },
	{ NWSCRIPT_ACTIONNAME("GetIsInSubArea") NWSCRIPT_ACTIONPROTOTYPE("int GetIsInSubArea(object oCreature, object oSubArea=OBJECT_SELF);") 768, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsInSubArea NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsInSubArea) },
	{ NWSCRIPT_ACTIONNAME("GetItemPropertyCostTable") NWSCRIPT_ACTIONPROTOTYPE("int GetItemPropertyCostTable(itemproperty iProp);") 769, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetItemPropertyCostTable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemPropertyCostTable) },
	{ NWSCRIPT_ACTIONNAME("GetItemPropertyCostTableValue") NWSCRIPT_ACTIONPROTOTYPE("int GetItemPropertyCostTableValue(itemproperty iProp);") 770, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetItemPropertyCostTableValue NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemPropertyCostTableValue) },
	{ NWSCRIPT_ACTIONNAME("GetItemPropertyParam1") NWSCRIPT_ACTIONPROTOTYPE("int GetItemPropertyParam1(itemproperty iProp);") 771, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetItemPropertyParam1 NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemPropertyParam1) },
	{ NWSCRIPT_ACTIONNAME("GetItemPropertyParam1Value") NWSCRIPT_ACTIONPROTOTYPE("int GetItemPropertyParam1Value(itemproperty iProp);") 772, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetItemPropertyParam1Value NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemPropertyParam1Value) },
	{ NWSCRIPT_ACTIONNAME("GetIsCreatureDisarmable") NWSCRIPT_ACTIONPROTOTYPE("int GetIsCreatureDisarmable(object oCreature);") 773, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsCreatureDisarmable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsCreatureDisarmable) },
	{ NWSCRIPT_ACTIONNAME("SetStolenFlag") NWSCRIPT_ACTIONPROTOTYPE("void SetStolenFlag(object oItem, int nStolenFlag);") 774, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetStolenFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetStolenFlag) },
	{ NWSCRIPT_ACTIONNAME("ForceRest") NWSCRIPT_ACTIONPROTOTYPE("void ForceRest(object oCreature);") 775, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ForceRest NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ForceRest) },
	{ NWSCRIPT_ACTIONNAME("SetCameraHeight") NWSCRIPT_ACTIONPROTOTYPE("void SetCameraHeight(object oPlayer, float fHeight=0.0f);") 776, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCameraHeight NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCameraHeight) },
	{ NWSCRIPT_ACTIONNAME("SetGlobalInt") NWSCRIPT_ACTIONPROTOTYPE("int SetGlobalInt(string sName, int nValue);") 777, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_SetGlobalInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetGlobalInt) },
	{ NWSCRIPT_ACTIONNAME("SetGlobalBool") NWSCRIPT_ACTIONPROTOTYPE("int SetGlobalBool(string sName, int bValue);") 778, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_SetGlobalBool NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetGlobalBool) },
	{ NWSCRIPT_ACTIONNAME("SetGlobalString") NWSCRIPT_ACTIONPROTOTYPE("int SetGlobalString(string sName, string sValue);") 779, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_SetGlobalString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetGlobalString) },
	{ NWSCRIPT_ACTIONNAME("SetGlobalFloat") NWSCRIPT_ACTIONPROTOTYPE("int SetGlobalFloat(string sName, float fValue);") 780, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_SetGlobalFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetGlobalFloat) },
	{ NWSCRIPT_ACTIONNAME("GetGlobalInt") NWSCRIPT_ACTIONPROTOTYPE("int GetGlobalInt(string sName);") 781, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetGlobalInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetGlobalInt) },
	{ NWSCRIPT_ACTIONNAME("GetGlobalBool") NWSCRIPT_ACTIONPROTOTYPE("int GetGlobalBool(string sName);") 782, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetGlobalBool NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetGlobalBool) },
	{ NWSCRIPT_ACTIONNAME("GetGlobalString") NWSCRIPT_ACTIONPROTOTYPE("string GetGlobalString(string sName);") 783, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetGlobalString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetGlobalString) },
	{ NWSCRIPT_ACTIONNAME("GetGlobalFloat") NWSCRIPT_ACTIONPROTOTYPE("float GetGlobalFloat(string sName);") 784, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_GetGlobalFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetGlobalFloat) },
	{ NWSCRIPT_ACTIONNAME("SaveGlobalVariables") NWSCRIPT_ACTIONPROTOTYPE("int SaveGlobalVariables(string sSaveName="");") 785, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_SaveGlobalVariables NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SaveGlobalVariables) },
	{ NWSCRIPT_ACTIONNAME("LoadGlobalVariables") NWSCRIPT_ACTIONPROTOTYPE("int LoadGlobalVariables(string sLoadName="");") 786, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_LoadGlobalVariables NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_LoadGlobalVariables) },
	{ NWSCRIPT_ACTIONNAME("MountObject") NWSCRIPT_ACTIONPROTOTYPE("void MountObject(object oMountingObject, object oObjectToMount);") 787, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_MountObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_MountObject) },
	{ NWSCRIPT_ACTIONNAME("DismountObject") NWSCRIPT_ACTIONPROTOTYPE("void DismountObject(object oDismountingObject, object oObjectToDismount);") 788, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DismountObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DismountObject) },
	{ NWSCRIPT_ACTIONNAME("GetJournalEntry") NWSCRIPT_ACTIONPROTOTYPE("int GetJournalEntry(string szPlotID, object oObjectJournal);") 789, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetJournalEntry NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetJournalEntry) },
	{ NWSCRIPT_ACTIONNAME("EffectNWN2ParticleEffect") NWSCRIPT_ACTIONPROTOTYPE("effect EffectNWN2ParticleEffect();") 790, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectNWN2ParticleEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectNWN2ParticleEffect) },
	{ NWSCRIPT_ACTIONNAME("EffectNWN2ParticleEffectFile") NWSCRIPT_ACTIONPROTOTYPE("effect EffectNWN2ParticleEffectFile( string sDefinitionFile );") 791, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectNWN2ParticleEffectFile NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectNWN2ParticleEffectFile) },
	{ NWSCRIPT_ACTIONNAME("EffectNWN2SpecialEffectFile") NWSCRIPT_ACTIONPROTOTYPE("effect EffectNWN2SpecialEffectFile( string sFileName, object oTarget=OBJECT_INVALID, vector vTargetPosition=[0.0,0.0,0.0]  );") 792, 1, 3, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectNWN2SpecialEffectFile NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectNWN2SpecialEffectFile) },
	{ NWSCRIPT_ACTIONNAME("GetSpellLevel") NWSCRIPT_ACTIONPROTOTYPE("int GetSpellLevel(int nSpellID);") 793, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetSpellLevel NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetSpellLevel) },
	{ NWSCRIPT_ACTIONNAME("RemoveSEFFromObject") NWSCRIPT_ACTIONPROTOTYPE("void RemoveSEFFromObject( object oObject, string sSEFName );") 794, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_RemoveSEFFromObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RemoveSEFFromObject) },
	{ NWSCRIPT_ACTIONNAME("ActionPauseCutscene") NWSCRIPT_ACTIONPROTOTYPE("void ActionPauseCutscene( int nTimeoutSecs, int bPurgeCutsceneActionsOnTimeout=FALSE );") 795, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ActionPauseCutscene NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ActionPauseCutscene) },
	{ NWSCRIPT_ACTIONNAME("AssignCutsceneActionToObject") NWSCRIPT_ACTIONPROTOTYPE("void AssignCutsceneActionToObject( object oObject, action aAction );") 796, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AssignCutsceneActionToObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AssignCutsceneActionToObject) },
	{ NWSCRIPT_ACTIONNAME("GetCharBackground") NWSCRIPT_ACTIONPROTOTYPE("int GetCharBackground( object oCreature );") 797, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetCharBackground NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCharBackground) },
	{ NWSCRIPT_ACTIONNAME("SetTrapActive") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapActive( object oTrap, int bActive );") 798, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetTrapActive NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetTrapActive) },
	{ NWSCRIPT_ACTIONNAME("SetOrientOnDialog") NWSCRIPT_ACTIONPROTOTYPE("void SetOrientOnDialog( object oCreature, int bActive );") 799, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetOrientOnDialog NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetOrientOnDialog) },
	{ NWSCRIPT_ACTIONNAME("EffectDetectUndead") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDetectUndead();") 800, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDetectUndead NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDetectUndead) },
	{ NWSCRIPT_ACTIONNAME("EffectLowLightVision") NWSCRIPT_ACTIONPROTOTYPE("effect EffectLowLightVision();") 801, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectLowLightVision NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectLowLightVision) },
	{ NWSCRIPT_ACTIONNAME("EffectSetScale") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSetScale( float fScaleX, float fScaleY=-1.0, float fScaleZ=-1.0 );") 802, 1, 3, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSetScale NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSetScale) },
	{ NWSCRIPT_ACTIONNAME("EffectShareDamage") NWSCRIPT_ACTIONPROTOTYPE("effect EffectShareDamage( object oHelper, int nAmtShared=50, int nAmtCasterShared=50 );") 803, 1, 3, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectShareDamage NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectShareDamage) },
	{ NWSCRIPT_ACTIONNAME("EffectAssayResistance") NWSCRIPT_ACTIONPROTOTYPE("effect EffectAssayResistance( object oTarget );") 804, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectAssayResistance NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectAssayResistance) },
	{ NWSCRIPT_ACTIONNAME("EffectSeeTrueHPs") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSeeTrueHPs();") 805, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSeeTrueHPs NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSeeTrueHPs) },
	{ NWSCRIPT_ACTIONNAME("GetNumCutsceneActionsPending") NWSCRIPT_ACTIONPROTOTYPE("int GetNumCutsceneActionsPending();") 806, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetNumCutsceneActionsPending NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNumCutsceneActionsPending) },
	{ NWSCRIPT_ACTIONNAME("EffectDamageOverTime") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamageOverTime(int nAmount, float fIntervalSeconds, int nDamageType=DAMAGE_TYPE_MAGICAL, int nIgnoreResistances=FALSE);") 807, 2, 4, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDamageOverTime NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDamageOverTime) },
	{ NWSCRIPT_ACTIONNAME("EffectAbsorbDamage") NWSCRIPT_ACTIONPROTOTYPE("effect EffectAbsorbDamage(int nACTest);") 808, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectAbsorbDamage NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectAbsorbDamage) },
	{ NWSCRIPT_ACTIONNAME("EffectHideousBlow") NWSCRIPT_ACTIONPROTOTYPE("effect EffectHideousBlow(int nMetamagic);") 809, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectHideousBlow NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectHideousBlow) },
	{ NWSCRIPT_ACTIONNAME("EffectMesmerize") NWSCRIPT_ACTIONPROTOTYPE("effect EffectMesmerize( int nBreakFlags, float fBreakDist = 0.0f );") 810, 1, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectMesmerize NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectMesmerize) },
	{ NWSCRIPT_ACTIONNAME("GetSpellFeatId") NWSCRIPT_ACTIONPROTOTYPE("int GetSpellFeatId();") 811, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetSpellFeatId NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetSpellFeatId) },
	{ NWSCRIPT_ACTIONNAME("SetFog") NWSCRIPT_ACTIONPROTOTYPE("void SetFog( object oTarget, int nFogType, int nColor, float fFogStart, float fFogEnd, float fFarClipPlaneDistance);") 812, 6, 6, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetFog NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetFog) },
	{ NWSCRIPT_ACTIONNAME("EffectDarkVision") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDarkVision();") 813, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDarkVision NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDarkVision) },
	{ NWSCRIPT_ACTIONNAME("DebugPostString") NWSCRIPT_ACTIONPROTOTYPE("void DebugPostString( object oTarget, string sMesg, int nX, int nY, float fDuration, int nColor=4294901760 );") 814, 5, 6, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DebugPostString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DebugPostString) },
	{ NWSCRIPT_ACTIONNAME("GetHasAnySpellEffect") NWSCRIPT_ACTIONPROTOTYPE("int GetHasAnySpellEffect( object oObject );") 815, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetHasAnySpellEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetHasAnySpellEffect) },
	{ NWSCRIPT_ACTIONNAME("EffectArmorCheckPenaltyIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectArmorCheckPenaltyIncrease( object oTarget, int nPenaltyAmt );") 816, 2, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectArmorCheckPenaltyIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectArmorCheckPenaltyIncrease) },
	{ NWSCRIPT_ACTIONNAME("EffectDisintegrate") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDisintegrate( object oTarget );") 817, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDisintegrate NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDisintegrate) },
	{ NWSCRIPT_ACTIONNAME("EffectHealOnZeroHP") NWSCRIPT_ACTIONPROTOTYPE("effect EffectHealOnZeroHP( object oTarget, int nDmgToHeal );") 818, 2, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectHealOnZeroHP NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectHealOnZeroHP) },
	{ NWSCRIPT_ACTIONNAME("EffectBreakEnchantment") NWSCRIPT_ACTIONPROTOTYPE("effect EffectBreakEnchantment( int nLevel );") 819, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectBreakEnchantment NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectBreakEnchantment) },
	{ NWSCRIPT_ACTIONNAME("GetFirstEnteringPC") NWSCRIPT_ACTIONPROTOTYPE("object GetFirstEnteringPC();") 820, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetFirstEnteringPC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFirstEnteringPC) },
	{ NWSCRIPT_ACTIONNAME("GetNextEnteringPC") NWSCRIPT_ACTIONPROTOTYPE("object GetNextEnteringPC();") 821, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetNextEnteringPC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNextEnteringPC) },
	{ NWSCRIPT_ACTIONNAME("AddRosterMemberByTemplate") NWSCRIPT_ACTIONPROTOTYPE("int AddRosterMemberByTemplate( string sRosterName, string sTemplate );") 822, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_AddRosterMemberByTemplate NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AddRosterMemberByTemplate) },
	{ NWSCRIPT_ACTIONNAME("AddRosterMemberByCharacter") NWSCRIPT_ACTIONPROTOTYPE("int AddRosterMemberByCharacter( string sRosterName, object oCharacter );") 823, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_AddRosterMemberByCharacter NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AddRosterMemberByCharacter) },
	{ NWSCRIPT_ACTIONNAME("RemoveRosterMember") NWSCRIPT_ACTIONPROTOTYPE("int RemoveRosterMember( string sRosterName );") 824, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_RemoveRosterMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RemoveRosterMember) },
	{ NWSCRIPT_ACTIONNAME("GetIsRosterMemberAvailable") NWSCRIPT_ACTIONPROTOTYPE("int GetIsRosterMemberAvailable( string sRosterName );") 825, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsRosterMemberAvailable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsRosterMemberAvailable) },
	{ NWSCRIPT_ACTIONNAME("GetIsRosterMemberSelectable") NWSCRIPT_ACTIONPROTOTYPE("int GetIsRosterMemberSelectable( string sRosterName );") 826, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsRosterMemberSelectable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsRosterMemberSelectable) },
	{ NWSCRIPT_ACTIONNAME("SetIsRosterMemberSelectable") NWSCRIPT_ACTIONPROTOTYPE("int SetIsRosterMemberSelectable( string sRosterName, int bSelectable );") 827, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_SetIsRosterMemberSelectable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetIsRosterMemberSelectable) },
	{ NWSCRIPT_ACTIONNAME("GetObjectFromRosterName") NWSCRIPT_ACTIONPROTOTYPE("object GetObjectFromRosterName( string sRosterName );") 828, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetObjectFromRosterName NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetObjectFromRosterName) },
	{ NWSCRIPT_ACTIONNAME("GetRosterNameFromObject") NWSCRIPT_ACTIONPROTOTYPE("string GetRosterNameFromObject( object oCreature );") 829, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetRosterNameFromObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetRosterNameFromObject) },
	{ NWSCRIPT_ACTIONNAME("SpawnRosterMember") NWSCRIPT_ACTIONPROTOTYPE("object SpawnRosterMember( string sRosterName, location lLocation );") 830, 2, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_SpawnRosterMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SpawnRosterMember) },
	{ NWSCRIPT_ACTIONNAME("DespawnRosterMember") NWSCRIPT_ACTIONPROTOTYPE("int DespawnRosterMember( string sRosterName );") 831, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_DespawnRosterMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DespawnRosterMember) },
	{ NWSCRIPT_ACTIONNAME("AddRosterMemberToParty") NWSCRIPT_ACTIONPROTOTYPE("int AddRosterMemberToParty( string sRosterName, object oPC );") 832, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_AddRosterMemberToParty NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AddRosterMemberToParty) },
	{ NWSCRIPT_ACTIONNAME("RemoveRosterMemberFromParty") NWSCRIPT_ACTIONPROTOTYPE("void RemoveRosterMemberFromParty( string sRosterName, object oPC, int bDespawnNPC=TRUE );") 833, 2, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_RemoveRosterMemberFromParty NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RemoveRosterMemberFromParty) },
	{ NWSCRIPT_ACTIONNAME("GetFirstRosterMember") NWSCRIPT_ACTIONPROTOTYPE("string GetFirstRosterMember();") 834, 0, 0, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetFirstRosterMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFirstRosterMember) },
	{ NWSCRIPT_ACTIONNAME("GetNextRosterMember") NWSCRIPT_ACTIONPROTOTYPE("string GetNextRosterMember();") 835, 0, 0, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetNextRosterMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNextRosterMember) },
	{ NWSCRIPT_ACTIONNAME("SpawnSpellProjectile") NWSCRIPT_ACTIONPROTOTYPE("void SpawnSpellProjectile( object oSource, object oTaget, location lSource, location lTarget, int nSpellID, int nProjectilePathType );") 836, 6, 6, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SpawnSpellProjectile NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SpawnSpellProjectile) },
	{ NWSCRIPT_ACTIONNAME("SpawnItemProjectile") NWSCRIPT_ACTIONPROTOTYPE("void SpawnItemProjectile( object oSource, object oTaget, location lSource, location lTarget, int nBaseItemID, int nProjectilePathType, int nAttackType, int nDamageTypeFlag );") 837, 8, 8, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SpawnItemProjectile NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SpawnItemProjectile) },
	{ NWSCRIPT_ACTIONNAME("GetIsOwnedByPlayer") NWSCRIPT_ACTIONPROTOTYPE("int GetIsOwnedByPlayer( object oCreature );") 838, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsOwnedByPlayer NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsOwnedByPlayer) },
	{ NWSCRIPT_ACTIONNAME("SetOwnersControlledCompanion") NWSCRIPT_ACTIONPROTOTYPE("object SetOwnersControlledCompanion( object oCurrentCreature, object oTargetCreature=OBJECT_INVALID );") 839, 1, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_SetOwnersControlledCompanion NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetOwnersControlledCompanion) },
	{ NWSCRIPT_ACTIONNAME("SetCreatureScriptsToSet") NWSCRIPT_ACTIONPROTOTYPE("void SetCreatureScriptsToSet( object oCreature, int nScriptSet );") 840, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCreatureScriptsToSet NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCreatureScriptsToSet) },
	{ NWSCRIPT_ACTIONNAME("GetProjectileTravelTime") NWSCRIPT_ACTIONPROTOTYPE("float GetProjectileTravelTime( location lSource, location lTarget, int nProjectilePathType, int nSpellID=-1 );") 841, 3, 4, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_GetProjectileTravelTime NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetProjectileTravelTime) },
	{ NWSCRIPT_ACTIONNAME("SetRosterNPCPartyLimit") NWSCRIPT_ACTIONPROTOTYPE("void SetRosterNPCPartyLimit( int nLimit );") 842, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetRosterNPCPartyLimit NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetRosterNPCPartyLimit) },
	{ NWSCRIPT_ACTIONNAME("GetRosterNPCPartyLimit") NWSCRIPT_ACTIONPROTOTYPE("int GetRosterNPCPartyLimit();") 843, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetRosterNPCPartyLimit NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetRosterNPCPartyLimit) },
	{ NWSCRIPT_ACTIONNAME("SetIsRosterMemberCampaignNPC") NWSCRIPT_ACTIONPROTOTYPE("int SetIsRosterMemberCampaignNPC( string sRosterName, int nCampaignNPC );") 844, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_SetIsRosterMemberCampaignNPC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetIsRosterMemberCampaignNPC) },
	{ NWSCRIPT_ACTIONNAME("GetIsRosterMemberCampaignNPC") NWSCRIPT_ACTIONPROTOTYPE("int GetIsRosterMemberCampaignNPC( string sRosterName );") 845, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsRosterMemberCampaignNPC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsRosterMemberCampaignNPC) },
	{ NWSCRIPT_ACTIONNAME("GetIsRosterMember") NWSCRIPT_ACTIONPROTOTYPE("int GetIsRosterMember( object oMember );") 846, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsRosterMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsRosterMember) },
	{ NWSCRIPT_ACTIONNAME("ShowWorldMap") NWSCRIPT_ACTIONPROTOTYPE("void ShowWorldMap( string sWorldMap, object oPlayer, string sTag);") 847, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ShowWorldMap NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ShowWorldMap) },
	{ NWSCRIPT_ACTIONNAME("TriggerEncounter") NWSCRIPT_ACTIONPROTOTYPE("void TriggerEncounter(object oEncounter, object oPlayer, int iCRFlag, float fCR);") 848, 4, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_TriggerEncounter NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_TriggerEncounter) },
	{ NWSCRIPT_ACTIONNAME("GetIsSinglePlayer") NWSCRIPT_ACTIONPROTOTYPE("int GetIsSinglePlayer();") 849, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsSinglePlayer NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsSinglePlayer) },
	{ NWSCRIPT_ACTIONNAME("DisplayGuiScreen") NWSCRIPT_ACTIONPROTOTYPE("void DisplayGuiScreen( object oPlayer, string sScreenName, int bModal, string sFileName = "", int bOverrideOptions = FALSE);") 850, 3, 5, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DisplayGuiScreen NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DisplayGuiScreen) },
	{ NWSCRIPT_ACTIONNAME("EffectOnDispel") NWSCRIPT_ACTIONPROTOTYPE("effect EffectOnDispel( float fDelay, action aOnDispelEffect );") 851, 2, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectOnDispel NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectOnDispel) },
	{ NWSCRIPT_ACTIONNAME("LoadNewModule") NWSCRIPT_ACTIONPROTOTYPE("void LoadNewModule(string sModuleName, string sWaypoint = "");") 852, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_LoadNewModule NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_LoadNewModule) },
	{ NWSCRIPT_ACTIONNAME("SetScriptHidden") NWSCRIPT_ACTIONPROTOTYPE("void SetScriptHidden( object oCreature, int bHidden, int bDisableUI=TRUE );") 853, 2, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetScriptHidden NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetScriptHidden) },
	{ NWSCRIPT_ACTIONNAME("SetIsCompanionPossessionBlocked") NWSCRIPT_ACTIONPROTOTYPE("void SetIsCompanionPossessionBlocked( object oCreature, int bBlocked );") 854, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetIsCompanionPossessionBlocked NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetIsCompanionPossessionBlocked) },
	{ NWSCRIPT_ACTIONNAME("SetEventHandler") NWSCRIPT_ACTIONPROTOTYPE("void SetEventHandler( object oObject, int iEventID, string sScriptName );") 855, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetEventHandler NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetEventHandler) },
	{ NWSCRIPT_ACTIONNAME("GetEventHandler") NWSCRIPT_ACTIONPROTOTYPE("string GetEventHandler( object oObject, int iEventID );") 856, 2, 2, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetEventHandler NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetEventHandler) },
	{ NWSCRIPT_ACTIONNAME("GetIsPartyTransition") NWSCRIPT_ACTIONPROTOTYPE("int GetIsPartyTransition( object oObject );") 857, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsPartyTransition NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsPartyTransition) },
	{ NWSCRIPT_ACTIONNAME("JumpPartyToArea") NWSCRIPT_ACTIONPROTOTYPE("void JumpPartyToArea( object oPartyMember, object oDestination );") 858, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_JumpPartyToArea NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_JumpPartyToArea) },
	{ NWSCRIPT_ACTIONNAME("GetNumActions") NWSCRIPT_ACTIONPROTOTYPE("int GetNumActions( object oObject );") 859, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetNumActions NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNumActions) },
	{ NWSCRIPT_ACTIONNAME("DisplayMessageBox") NWSCRIPT_ACTIONPROTOTYPE("void DisplayMessageBox( object oPC, int nMessageStrRef, string sMessage, string sOkCB="", string sCancelCB="", int bShowCancel=FALSE, string sScreenName="", int nOkStrRef=0, string sOkString="", int nCancelStrRef=0, string sCancelString="" );") 860, 3, 11, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DisplayMessageBox NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DisplayMessageBox) },
	{ NWSCRIPT_ACTIONNAME("StringCompare") NWSCRIPT_ACTIONPROTOTYPE("int StringCompare( string sString1, string sString2, int nCaseSensitive=FALSE );") 861, 2, 3, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_StringCompare NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_StringCompare) },
	{ NWSCRIPT_ACTIONNAME("CharToASCII") NWSCRIPT_ACTIONPROTOTYPE("int CharToASCII( string sString );") 862, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_CharToASCII NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_CharToASCII) },
	{ NWSCRIPT_ACTIONNAME("GetOwnedCharacter") NWSCRIPT_ACTIONPROTOTYPE("object GetOwnedCharacter( object oControlled );") 863, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetOwnedCharacter NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetOwnedCharacter) },
	{ NWSCRIPT_ACTIONNAME("GetControlledCharacter") NWSCRIPT_ACTIONPROTOTYPE("object GetControlledCharacter( object oCreature );") 864, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetControlledCharacter NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetControlledCharacter) },
	{ NWSCRIPT_ACTIONNAME("FeatAdd") NWSCRIPT_ACTIONPROTOTYPE("int FeatAdd( object oCreature, int iFeatId, int bCheckRequirements, int bFeedback=FALSE, int bNotice=FALSE );") 865, 3, 5, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_FeatAdd NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_FeatAdd) },
	{ NWSCRIPT_ACTIONNAME("FeatRemove") NWSCRIPT_ACTIONPROTOTYPE("void FeatRemove( object oCreature, int iFeatIds );") 866, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_FeatRemove NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_FeatRemove) },
	{ NWSCRIPT_ACTIONNAME("SetCanTalkToNonPlayerOwnedCreatures") NWSCRIPT_ACTIONPROTOTYPE("void SetCanTalkToNonPlayerOwnedCreatures( object oObject, int bCanTalk );") 867, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCanTalkToNonPlayerOwnedCreatures NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCanTalkToNonPlayerOwnedCreatures) },
	{ NWSCRIPT_ACTIONNAME("GetCanTalkToNonPlayerOwnedCreatures") NWSCRIPT_ACTIONPROTOTYPE("int GetCanTalkToNonPlayerOwnedCreatures( object oObject );") 868, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetCanTalkToNonPlayerOwnedCreatures NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCanTalkToNonPlayerOwnedCreatures) },
	{ NWSCRIPT_ACTIONNAME("SetLevelUpPackage") NWSCRIPT_ACTIONPROTOTYPE("void SetLevelUpPackage( object oCreature, int nPackage );") 869, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetLevelUpPackage NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetLevelUpPackage) },
	{ NWSCRIPT_ACTIONNAME("GetLevelUpPackage") NWSCRIPT_ACTIONPROTOTYPE("int GetLevelUpPackage( object oCreature );") 870, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLevelUpPackage NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLevelUpPackage) },
	{ NWSCRIPT_ACTIONNAME("SetCombatOverrides") NWSCRIPT_ACTIONPROTOTYPE("void SetCombatOverrides( object oCreature, object oTarget, int nOnHandAttacks, int nOffHandAttacks, int nAttackResult, int nMinDamage, int nMaxDamage, int bSuppressBroadcastAOO, int bSuppressMakeAOO, int bIgnoreTargetReaction, int bSuppressFeedbackText );") 871, 11, 11, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCombatOverrides NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCombatOverrides) },
	{ NWSCRIPT_ACTIONNAME("ClearCombatOverrides") NWSCRIPT_ACTIONPROTOTYPE("void ClearCombatOverrides( object oCreature );") 872, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ClearCombatOverrides NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ClearCombatOverrides) },
	{ NWSCRIPT_ACTIONNAME("ResetCreatureLevelForXP") NWSCRIPT_ACTIONPROTOTYPE("void ResetCreatureLevelForXP( object oTargetCreature, int nExperience, int bUseXPMods );") 873, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ResetCreatureLevelForXP NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ResetCreatureLevelForXP) },
	{ NWSCRIPT_ACTIONNAME("CalcPointAwayFromPoint") NWSCRIPT_ACTIONPROTOTYPE("location CalcPointAwayFromPoint( location lPoint, location lAwayFromPoint, float fDistance, float fAngularVariance, int bComputeDistFromStart );") 874, 5, 5, ACTIONTYPE_LOCATION, NWN2_NWActionParameterTypes_CalcPointAwayFromPoint NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_CalcPointAwayFromPoint) },
	{ NWSCRIPT_ACTIONNAME("CalcSafeLocation") NWSCRIPT_ACTIONPROTOTYPE("location CalcSafeLocation( object oCreature, location lTestPosition, float fSearchRadius, int bWalkStraighLineRequired, int bIgnoreTestPosition );") 875, 5, 5, ACTIONTYPE_LOCATION, NWN2_NWActionParameterTypes_CalcSafeLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_CalcSafeLocation) },
	{ NWSCRIPT_ACTIONNAME("GetTotalLevels") NWSCRIPT_ACTIONPROTOTYPE("int GetTotalLevels( object oCreature, int bIncludeNegativeLevels );") 876, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTotalLevels NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTotalLevels) },
	{ NWSCRIPT_ACTIONNAME("ResetFeatUses") NWSCRIPT_ACTIONPROTOTYPE("void ResetFeatUses( object oCreature, int nFeatID, int bResetDailyUses, int bResetLastUseTime );") 877, 4, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ResetFeatUses NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ResetFeatUses) },
	{ NWSCRIPT_ACTIONNAME("SetNWN2Fog") NWSCRIPT_ACTIONPROTOTYPE("void SetNWN2Fog( object oTarget, int nFogType, int nColor, float fFogStart, float fFogEnd);") 878, 5, 5, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetNWN2Fog NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetNWN2Fog) },
	{ NWSCRIPT_ACTIONNAME("ResetNWN2Fog") NWSCRIPT_ACTIONPROTOTYPE("void ResetNWN2Fog(object oTarget, int nFogType);") 879, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ResetNWN2Fog NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ResetNWN2Fog) },
	{ NWSCRIPT_ACTIONNAME("EffectBonusHitpoints") NWSCRIPT_ACTIONPROTOTYPE("effect EffectBonusHitpoints( int nHitpoints );") 880, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectBonusHitpoints NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectBonusHitpoints) },
	{ NWSCRIPT_ACTIONNAME("SetGUIObjectHidden") NWSCRIPT_ACTIONPROTOTYPE("void SetGUIObjectHidden( object oPlayer, string sScreenName, string sUIObjectName, int bHidden );") 881, 4, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetGUIObjectHidden NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetGUIObjectHidden) },
	{ NWSCRIPT_ACTIONNAME("CloseGUIScreen") NWSCRIPT_ACTIONPROTOTYPE("void CloseGUIScreen( object oPlayer, string sScreenName );") 882, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_CloseGUIScreen NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_CloseGUIScreen) },
	{ NWSCRIPT_ACTIONNAME("FiredFromPartyTransition") NWSCRIPT_ACTIONPROTOTYPE("int FiredFromPartyTransition();") 883, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_FiredFromPartyTransition NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_FiredFromPartyTransition) },
	{ NWSCRIPT_ACTIONNAME("GetScriptHidden") NWSCRIPT_ACTIONPROTOTYPE("int GetScriptHidden( object oObject );") 884, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetScriptHidden NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetScriptHidden) },
	{ NWSCRIPT_ACTIONNAME("GetNodeSpeaker") NWSCRIPT_ACTIONPROTOTYPE("string GetNodeSpeaker();") 885, 0, 0, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetNodeSpeaker NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNodeSpeaker) },
	{ NWSCRIPT_ACTIONNAME("SetLocalGUIVariable") NWSCRIPT_ACTIONPROTOTYPE("void SetLocalGUIVariable( object oPlayer, string sScreenName, int nVarIndex, string sVarValue );") 886, 4, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetLocalGUIVariable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetLocalGUIVariable) },
	{ NWSCRIPT_ACTIONNAME("SetGUIObjectDisabled") NWSCRIPT_ACTIONPROTOTYPE("void SetGUIObjectDisabled( object oPlayer, string sScreenName, string sUIObjectName, int bDisabled );") 887, 4, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetGUIObjectDisabled NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetGUIObjectDisabled) },
	{ NWSCRIPT_ACTIONNAME("SetGUIObjectText") NWSCRIPT_ACTIONPROTOTYPE("void SetGUIObjectText( object oPlayer, string sScreenName, string sUIObjectName, int nStrRef, string sText );") 888, 5, 5, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetGUIObjectText NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetGUIObjectText) },
	{ NWSCRIPT_ACTIONNAME("GetIsCompanionPossessionBlocked") NWSCRIPT_ACTIONPROTOTYPE("int GetIsCompanionPossessionBlocked( object oCreature );") 889, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsCompanionPossessionBlocked NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsCompanionPossessionBlocked) },
	{ NWSCRIPT_ACTIONNAME("EffectBardSongSinging") NWSCRIPT_ACTIONPROTOTYPE("effect EffectBardSongSinging( int nSpellId );") 890, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectBardSongSinging NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectBardSongSinging) },
	{ NWSCRIPT_ACTIONNAME("EffectJarring") NWSCRIPT_ACTIONPROTOTYPE("effect EffectJarring();") 891, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectJarring NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectJarring) },
	{ NWSCRIPT_ACTIONNAME("GetEffectInteger") NWSCRIPT_ACTIONPROTOTYPE("int GetEffectInteger( effect eTest, int nIdx );") 892, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetEffectInteger NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetEffectInteger) },
	{ NWSCRIPT_ACTIONNAME("RefreshSpellEffectDurations") NWSCRIPT_ACTIONPROTOTYPE("void RefreshSpellEffectDurations( object oTarget, int nSpellId, float fDuration );") 893, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_RefreshSpellEffectDurations NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RefreshSpellEffectDurations) },
	{ NWSCRIPT_ACTIONNAME("SetEffectSpellId") NWSCRIPT_ACTIONPROTOTYPE("effect SetEffectSpellId( effect eTest, int nSpellId );") 894, 2, 2, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_SetEffectSpellId NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetEffectSpellId) },
	{ NWSCRIPT_ACTIONNAME("EffectBABMinimum") NWSCRIPT_ACTIONPROTOTYPE("effect EffectBABMinimum( int nBABMin );") 895, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectBABMinimum NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectBABMinimum) },
	{ NWSCRIPT_ACTIONNAME("GetTRUEBaseAttackBonus") NWSCRIPT_ACTIONPROTOTYPE("int GetTRUEBaseAttackBonus( object oTarget );") 896, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTRUEBaseAttackBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTRUEBaseAttackBonus) },
	{ NWSCRIPT_ACTIONNAME("SetFirstName") NWSCRIPT_ACTIONPROTOTYPE("void SetFirstName(object oTarget, string sFirstName);") 897, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetFirstName NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetFirstName) },
	{ NWSCRIPT_ACTIONNAME("SetLastName") NWSCRIPT_ACTIONPROTOTYPE("void SetLastName(object oTarget, string sLastName);") 898, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetLastName NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetLastName) },
	{ NWSCRIPT_ACTIONNAME("SetDescription") NWSCRIPT_ACTIONPROTOTYPE("void SetDescription(object oTarget, string sDescription);") 899, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetDescription NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetDescription) },
	{ NWSCRIPT_ACTIONNAME("GetFirstName") NWSCRIPT_ACTIONPROTOTYPE("string GetFirstName(object oTarget);") 900, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetFirstName NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFirstName) },
	{ NWSCRIPT_ACTIONNAME("GetLastName") NWSCRIPT_ACTIONPROTOTYPE("string GetLastName(object oTarget);") 901, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetLastName NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLastName) },
	{ NWSCRIPT_ACTIONNAME("GetDescription") NWSCRIPT_ACTIONPROTOTYPE("string GetDescription(object oTarget);") 902, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetDescription NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetDescription) },
	{ NWSCRIPT_ACTIONNAME("IsInMultiplayerConversation") NWSCRIPT_ACTIONPROTOTYPE("int IsInMultiplayerConversation( object oObject );") 903, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_IsInMultiplayerConversation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_IsInMultiplayerConversation) },
	{ NWSCRIPT_ACTIONNAME("PlayCustomAnimation") NWSCRIPT_ACTIONPROTOTYPE("int PlayCustomAnimation( object oObject, string sAnimationName, int nLooping, float fSpeed = 1.0f );") 904, 3, 4, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_PlayCustomAnimation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_PlayCustomAnimation) },
	{ NWSCRIPT_ACTIONNAME("EffectMaxDamage") NWSCRIPT_ACTIONPROTOTYPE("effect EffectMaxDamage();") 905, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectMaxDamage NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectMaxDamage) },
	{ NWSCRIPT_ACTIONNAME("DisplayInputBox") NWSCRIPT_ACTIONPROTOTYPE("void DisplayInputBox( object oPC, int nMessageStrRef, string sMessage, string sOkCB="", string sCancelCB="", int bShowCancel=FALSE, string sScreenName="", int nOkStrRef=0, string sOkString="", int nCancelStrRef=0, string sCancelString="", string sDefaultString="", string sUnusedString="" );") 906, 3, 13, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_DisplayInputBox NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_DisplayInputBox) },
	{ NWSCRIPT_ACTIONNAME("SetWeaponVisibility") NWSCRIPT_ACTIONPROTOTYPE("int SetWeaponVisibility( object oObject, int nVisibile, int nType=0 );") 907, 2, 3, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_SetWeaponVisibility NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetWeaponVisibility) },
	{ NWSCRIPT_ACTIONNAME("SetLookAtTarget") NWSCRIPT_ACTIONPROTOTYPE("void SetLookAtTarget( object oObject, vector vTarget, int nType=0 );") 908, 2, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetLookAtTarget NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetLookAtTarget) },
	{ NWSCRIPT_ACTIONNAME("GetBumpState") NWSCRIPT_ACTIONPROTOTYPE("int GetBumpState(object oCreature);") 909, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetBumpState NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetBumpState) },
	{ NWSCRIPT_ACTIONNAME("SetBumpState") NWSCRIPT_ACTIONPROTOTYPE("void SetBumpState(object oCreature, int nBumpState);") 910, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetBumpState NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetBumpState) },
	{ NWSCRIPT_ACTIONNAME("GetOnePartyMode") NWSCRIPT_ACTIONPROTOTYPE("int GetOnePartyMode();") 911, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetOnePartyMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetOnePartyMode) },
	{ NWSCRIPT_ACTIONNAME("GetPrimaryPlayer") NWSCRIPT_ACTIONPROTOTYPE("object GetPrimaryPlayer();") 912, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetPrimaryPlayer NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPrimaryPlayer) },
	{ NWSCRIPT_ACTIONNAME("EffectArcaneSpellFailure") NWSCRIPT_ACTIONPROTOTYPE("effect EffectArcaneSpellFailure(int nPercent);") 913, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectArcaneSpellFailure NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectArcaneSpellFailure) },
	{ NWSCRIPT_ACTIONNAME("SpawnBloodHit") NWSCRIPT_ACTIONPROTOTYPE("void SpawnBloodHit( object oCreature, int bCriticalHit, object oAttacker );") 914, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SpawnBloodHit NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SpawnBloodHit) },
	{ NWSCRIPT_ACTIONNAME("GetFirstArea") NWSCRIPT_ACTIONPROTOTYPE("object GetFirstArea();") 915, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetFirstArea NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFirstArea) },
	{ NWSCRIPT_ACTIONNAME("GetNextArea") NWSCRIPT_ACTIONPROTOTYPE("object GetNextArea();") 916, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetNextArea NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNextArea) },
	{ NWSCRIPT_ACTIONNAME("GetArmorRank") NWSCRIPT_ACTIONPROTOTYPE("int GetArmorRank(object oItem);") 917, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetArmorRank NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetArmorRank) },
	{ NWSCRIPT_ACTIONNAME("GetWeaponType") NWSCRIPT_ACTIONPROTOTYPE("int GetWeaponType(object oItem);") 918, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetWeaponType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetWeaponType) },
	{ NWSCRIPT_ACTIONNAME("GetPlayerCurrentTarget") NWSCRIPT_ACTIONPROTOTYPE("object GetPlayerCurrentTarget( object oCreature );") 919, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetPlayerCurrentTarget NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPlayerCurrentTarget) },
	{ NWSCRIPT_ACTIONNAME("EffectWildshape") NWSCRIPT_ACTIONPROTOTYPE("effect EffectWildshape();") 920, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectWildshape NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectWildshape) },
	{ NWSCRIPT_ACTIONNAME("GetEncumbranceState") NWSCRIPT_ACTIONPROTOTYPE("int GetEncumbranceState( object oCreature );") 921, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetEncumbranceState NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetEncumbranceState) },
	{ NWSCRIPT_ACTIONNAME("PackCampaignDatabase") NWSCRIPT_ACTIONPROTOTYPE("void PackCampaignDatabase(string sCampaignName);") 922, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_PackCampaignDatabase NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_PackCampaignDatabase) },
	{ NWSCRIPT_ACTIONNAME("UnlinkDoor") NWSCRIPT_ACTIONPROTOTYPE("void UnlinkDoor( object oDoor );") 923, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_UnlinkDoor NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_UnlinkDoor) },
	{ NWSCRIPT_ACTIONNAME("GetPlayerCreatureExamineTarget") NWSCRIPT_ACTIONPROTOTYPE("object GetPlayerCreatureExamineTarget( object oCreature );") 924, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetPlayerCreatureExamineTarget NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPlayerCreatureExamineTarget) },
	{ NWSCRIPT_ACTIONNAME("Clear2DACache") NWSCRIPT_ACTIONPROTOTYPE("void Clear2DACache(string s2DAName="");") 925, 0, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_Clear2DACache NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_Clear2DACache) },
	{ NWSCRIPT_ACTIONNAME("NWNXGetInt") NWSCRIPT_ACTIONPROTOTYPE("int NWNXGetInt( string sPlugin, string sFunction, string sParam1, int nParam2 );") 926, 4, 4, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_NWNXGetInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_NWNXGetInt) },
	{ NWSCRIPT_ACTIONNAME("NWNXGetFloat") NWSCRIPT_ACTIONPROTOTYPE("float NWNXGetFloat( string sPlugin, string sFunction, string sParam1, int nParam2 );") 927, 4, 4, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_NWNXGetFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_NWNXGetFloat) },
	{ NWSCRIPT_ACTIONNAME("NWNXGetString") NWSCRIPT_ACTIONPROTOTYPE("string NWNXGetString( string sPlugin, string sFunction, string sParam1, int nParam2 );") 928, 4, 4, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_NWNXGetString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_NWNXGetString) },
	{ NWSCRIPT_ACTIONNAME("NWNXSetInt") NWSCRIPT_ACTIONPROTOTYPE("void NWNXSetInt( string sPlugin, string sFunction, string sParam1, int nParam2, int nValue );") 929, 5, 5, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_NWNXSetInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_NWNXSetInt) },
	{ NWSCRIPT_ACTIONNAME("NWNXSetFloat") NWSCRIPT_ACTIONPROTOTYPE("void NWNXSetFloat( string sPlugin, string sFunction, string sParam1, int nParam2, float fValue );") 930, 5, 5, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_NWNXSetFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_NWNXSetFloat) },
	{ NWSCRIPT_ACTIONNAME("NWNXSetString") NWSCRIPT_ACTIONPROTOTYPE("void NWNXSetString( string sPlugin, string sFunction, string sParam1, int nParam2, string sValue );") 931, 5, 5, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_NWNXSetString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_NWNXSetString) },
	{ NWSCRIPT_ACTIONNAME("EffectEffectIcon") NWSCRIPT_ACTIONPROTOTYPE("effect EffectEffectIcon(int nEffectIconId);") 932, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectEffectIcon NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectEffectIcon) },
	{ NWSCRIPT_ACTIONNAME("SetGUIProgressBarPosition") NWSCRIPT_ACTIONPROTOTYPE("void SetGUIProgressBarPosition( object oPlayer, string sScreenName, string sUIObjectName, float fPosition );") 933, 4, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetGUIProgressBarPosition NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetGUIProgressBarPosition) },
	{ NWSCRIPT_ACTIONNAME("SetGUITexture") NWSCRIPT_ACTIONPROTOTYPE("void SetGUITexture( object oPlayer, string sScreenName, string sUIObjectName, string sTexture );") 934, 4, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetGUITexture NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetGUITexture) },
	{ NWSCRIPT_ACTIONNAME("EffectRescue") NWSCRIPT_ACTIONPROTOTYPE("effect EffectRescue( int nSpellId );") 935, 1, 1, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectRescue NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectRescue) },
	{ NWSCRIPT_ACTIONNAME("IntToObject") NWSCRIPT_ACTIONPROTOTYPE("object IntToObject( int nInt );") 936, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_IntToObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_IntToObject) },
	{ NWSCRIPT_ACTIONNAME("ObjectToInt") NWSCRIPT_ACTIONPROTOTYPE("int ObjectToInt( object oObj );") 937, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_ObjectToInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ObjectToInt) },
	{ NWSCRIPT_ACTIONNAME("StringToObject") NWSCRIPT_ACTIONPROTOTYPE("object StringToObject( string sString );") 938, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_StringToObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_StringToObject) },
	{ NWSCRIPT_ACTIONNAME("GetIsSpirit") NWSCRIPT_ACTIONPROTOTYPE("int GetIsSpirit( object oCreature );") 939, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsSpirit NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsSpirit) },
	{ NWSCRIPT_ACTIONNAME("EffectDetectSpirits") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDetectSpirits();") 940, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDetectSpirits NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDetectSpirits) },
	{ NWSCRIPT_ACTIONNAME("EffectDamageReductionNegated") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamageReductionNegated();") 941, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectDamageReductionNegated NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectDamageReductionNegated) },
	{ NWSCRIPT_ACTIONNAME("EffectConcealmentNegated") NWSCRIPT_ACTIONPROTOTYPE("effect EffectConcealmentNegated();") 942, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectConcealmentNegated NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectConcealmentNegated) },
	{ NWSCRIPT_ACTIONNAME("GetInfiniteFlag") NWSCRIPT_ACTIONPROTOTYPE("int GetInfiniteFlag( object oItem );") 943, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetInfiniteFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetInfiniteFlag) },
	{ NWSCRIPT_ACTIONNAME("GetKeyRequiredFeedbackMessage") NWSCRIPT_ACTIONPROTOTYPE("string GetKeyRequiredFeedbackMessage( object oObject );") 944, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetKeyRequiredFeedbackMessage NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetKeyRequiredFeedbackMessage) },
	{ NWSCRIPT_ACTIONNAME("SetKeyRequiredFeedbackMessage") NWSCRIPT_ACTIONPROTOTYPE("void SetKeyRequiredFeedbackMessage( object oObject, string sFeedback );") 945, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetKeyRequiredFeedbackMessage NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetKeyRequiredFeedbackMessage) },
	{ NWSCRIPT_ACTIONNAME("SetInfiniteFlag") NWSCRIPT_ACTIONPROTOTYPE("void SetInfiniteFlag( object oItem, int bInfinite=TRUE );") 946, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetInfiniteFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetInfiniteFlag) },
	{ NWSCRIPT_ACTIONNAME("GetPickpocketableFlag") NWSCRIPT_ACTIONPROTOTYPE("int GetPickpocketableFlag( object oItem );") 947, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetPickpocketableFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPickpocketableFlag) },
	{ NWSCRIPT_ACTIONNAME("SetPickpocketableFlag") NWSCRIPT_ACTIONPROTOTYPE("void SetPickpocketableFlag( object oItem, int bPickpocketable );") 948, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetPickpocketableFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetPickpocketableFlag) },
	{ NWSCRIPT_ACTIONNAME("GetTrapActive") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapActive( object oObject );") 949, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTrapActive NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTrapActive) },
	{ NWSCRIPT_ACTIONNAME("SetWillSavingThrow") NWSCRIPT_ACTIONPROTOTYPE("void SetWillSavingThrow( object oObject, int nNewWillSave );") 950, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetWillSavingThrow NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetWillSavingThrow) },
	{ NWSCRIPT_ACTIONNAME("SetReflexSavingThrow") NWSCRIPT_ACTIONPROTOTYPE("void SetReflexSavingThrow( object oObject, int nNewReflexSave );") 951, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetReflexSavingThrow NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetReflexSavingThrow) },
	{ NWSCRIPT_ACTIONNAME("SetFortitudeSavingThrow") NWSCRIPT_ACTIONPROTOTYPE("void SetFortitudeSavingThrow( object oObject, int nNewFortSave );") 952, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetFortitudeSavingThrow NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetFortitudeSavingThrow) },
	{ NWSCRIPT_ACTIONNAME("SetLockKeyRequired") NWSCRIPT_ACTIONPROTOTYPE("void SetLockKeyRequired( object oObject, int nKeyRequired=TRUE );") 953, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetLockKeyRequired NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetLockKeyRequired) },
	{ NWSCRIPT_ACTIONNAME("SetLockKeyTag") NWSCRIPT_ACTIONPROTOTYPE("void SetLockKeyTag( object oObject, string sKeyTag );") 954, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetLockKeyTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetLockKeyTag) },
	{ NWSCRIPT_ACTIONNAME("SetLockLockDC") NWSCRIPT_ACTIONPROTOTYPE("void SetLockLockDC( object oObject, int nNewLockDC );") 955, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetLockLockDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetLockLockDC) },
	{ NWSCRIPT_ACTIONNAME("SetLockUnlockDC") NWSCRIPT_ACTIONPROTOTYPE("void SetLockUnlockDC( object oObject, int nNewLockDC );") 956, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetLockUnlockDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetLockUnlockDC) },
	{ NWSCRIPT_ACTIONNAME("SetLockLockable") NWSCRIPT_ACTIONPROTOTYPE("void SetLockLockable( object oObject, int nLockable=TRUE );") 957, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetLockLockable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetLockLockable) },
	{ NWSCRIPT_ACTIONNAME("SetHardness") NWSCRIPT_ACTIONPROTOTYPE("void SetHardness( int nHardness, object oObject );") 958, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetHardness NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetHardness) },
	{ NWSCRIPT_ACTIONNAME("GetHardness") NWSCRIPT_ACTIONPROTOTYPE("int GetHardness( object oObject );") 959, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetHardness NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetHardness) },
	{ NWSCRIPT_ACTIONNAME("GetModuleXPScale") NWSCRIPT_ACTIONPROTOTYPE("int GetModuleXPScale();") 960, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetModuleXPScale NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetModuleXPScale) },
	{ NWSCRIPT_ACTIONNAME("SetModuleXPScale") NWSCRIPT_ACTIONPROTOTYPE("void SetModuleXPScale( int nXPScale );") 961, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetModuleXPScale NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetModuleXPScale) },
	{ NWSCRIPT_ACTIONNAME("SetTrapDetectable") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapDetectable( object oTrap, int nDetectable=TRUE );") 962, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetTrapDetectable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetTrapDetectable) },
	{ NWSCRIPT_ACTIONNAME("SetTrapDetectDC") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapDetectDC( object oTrap, int nDetectDC );") 963, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetTrapDetectDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetTrapDetectDC) },
	{ NWSCRIPT_ACTIONNAME("SetTrapDisarmable") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapDisarmable( object oTrap, int nDisarmable=TRUE );") 964, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetTrapDisarmable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetTrapDisarmable) },
	{ NWSCRIPT_ACTIONNAME("SetTrapDisarmDC") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapDisarmDC( object oTrap, int nDisarmDC );") 965, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetTrapDisarmDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetTrapDisarmDC) },
	{ NWSCRIPT_ACTIONNAME("SetTrapKeyTag") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapKeyTag( object oTrap, string sKeyTag );") 966, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetTrapKeyTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetTrapKeyTag) },
	{ NWSCRIPT_ACTIONNAME("SetTrapOneShot") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapOneShot( object oTrap, int nOneShot=TRUE );") 967, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetTrapOneShot NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetTrapOneShot) },
	{ NWSCRIPT_ACTIONNAME("CreateTrapAtLocation") NWSCRIPT_ACTIONPROTOTYPE("object CreateTrapAtLocation( int nTrapType, location lLocation, float fSize=2.0f, string sTag="", int nFaction=STANDARD_FACTION_HOSTILE, string sOnDisarmScript="", string sOnTrapTriggeredScript="" );") 968, 2, 7, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_CreateTrapAtLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_CreateTrapAtLocation) },
	{ NWSCRIPT_ACTIONNAME("CreateTrapOnObject") NWSCRIPT_ACTIONPROTOTYPE("void CreateTrapOnObject( int nTrapType, object oObject, int nFaction=STANDARD_FACTION_HOSTILE, string sOnDisarmScript="",string sOnTrapTriggeredScript="" );") 969, 2, 5, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_CreateTrapOnObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_CreateTrapOnObject) },
	{ NWSCRIPT_ACTIONNAME("GetAreaSize") NWSCRIPT_ACTIONPROTOTYPE("int GetAreaSize( int nAreaDimension, object oArea=OBJECT_INVALID );") 970, 1, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetAreaSize NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAreaSize) },
	{ NWSCRIPT_ACTIONNAME("GetTrapRecoverable") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapRecoverable( object oTrap );") 971, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTrapRecoverable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTrapRecoverable) },
	{ NWSCRIPT_ACTIONNAME("SetTrapRecoverable") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapRecoverable( object oTrap, int nRecoverable=TRUE );") 972, 1, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetTrapRecoverable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetTrapRecoverable) },
	{ NWSCRIPT_ACTIONNAME("SetUseableFlag") NWSCRIPT_ACTIONPROTOTYPE("void SetUseableFlag( object oPlaceable, int nUseableFlag );") 973, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetUseableFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetUseableFlag) },
	{ NWSCRIPT_ACTIONNAME("GetPlaceableLastClickedBy") NWSCRIPT_ACTIONPROTOTYPE("object GetPlaceableLastClickedBy();") 974, 0, 0, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetPlaceableLastClickedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPlaceableLastClickedBy) },
	{ NWSCRIPT_ACTIONNAME("SetRenderWaterInArea") NWSCRIPT_ACTIONPROTOTYPE("void SetRenderWaterInArea( object oArea, int bRender );") 975, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetRenderWaterInArea NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetRenderWaterInArea) },
	{ NWSCRIPT_ACTIONNAME("EffectInsane") NWSCRIPT_ACTIONPROTOTYPE("effect EffectInsane();") 976, 0, 0, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectInsane NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectInsane) },
	{ NWSCRIPT_ACTIONNAME("SetPlayerGUIHidden") NWSCRIPT_ACTIONPROTOTYPE("void SetPlayerGUIHidden( object oPlayer, int bHidden );") 977, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetPlayerGUIHidden NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetPlayerGUIHidden) },
	{ NWSCRIPT_ACTIONNAME("GetSelectedMapPointTag") NWSCRIPT_ACTIONPROTOTYPE("string GetSelectedMapPointTag();") 978, 0, 0, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetSelectedMapPointTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetSelectedMapPointTag) },
	{ NWSCRIPT_ACTIONNAME("SetNoticeText") NWSCRIPT_ACTIONPROTOTYPE("void SetNoticeText( object oPlayer, string sText );") 979, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetNoticeText NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetNoticeText) },
	{ NWSCRIPT_ACTIONNAME("SetLightActive") NWSCRIPT_ACTIONPROTOTYPE("void SetLightActive( object oLight, int bActive );") 980, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetLightActive NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetLightActive) },
	{ NWSCRIPT_ACTIONNAME("EffectSummonCopy") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSummonCopy(object oSource, int nVisualEffectId=VFX_NONE, float fDelaySeconds=0.0f, string sNewTag="", int nNewHP=0, string sScript="");") 981, 1, 6, ACTIONTYPE_EFFECT, NWN2_NWActionParameterTypes_EffectSummonCopy NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_EffectSummonCopy) },
	{ NWSCRIPT_ACTIONNAME("GetPolymorphLocked") NWSCRIPT_ACTIONPROTOTYPE("int GetPolymorphLocked(object oCreature);") 982, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetPolymorphLocked NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPolymorphLocked) },
	{ NWSCRIPT_ACTIONNAME("SetSoundSet") NWSCRIPT_ACTIONPROTOTYPE("void SetSoundSet( object oCreature, int nSoundSet );") 983, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetSoundSet NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetSoundSet) },
	{ NWSCRIPT_ACTIONNAME("SetScale") NWSCRIPT_ACTIONPROTOTYPE("void SetScale( object oObject, float fX, float fY, float fZ );") 984, 4, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetScale NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetScale) },
	{ NWSCRIPT_ACTIONNAME("GetScale") NWSCRIPT_ACTIONPROTOTYPE("float GetScale( object oObject, int nAxis );") 985, 2, 2, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_GetScale NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetScale) },
	{ NWSCRIPT_ACTIONNAME("GetNum2DARows") NWSCRIPT_ACTIONPROTOTYPE("int GetNum2DARows( string s2DAName );") 986, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetNum2DARows NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNum2DARows) },
	{ NWSCRIPT_ACTIONNAME("GetNum2DAColumns") NWSCRIPT_ACTIONPROTOTYPE("int GetNum2DAColumns( string s2DAName );") 987, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetNum2DAColumns NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNum2DAColumns) },
	{ NWSCRIPT_ACTIONNAME("SetCustomHeartbeat") NWSCRIPT_ACTIONPROTOTYPE("void SetCustomHeartbeat( object oTarget, int nMSeconds );") 988, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCustomHeartbeat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCustomHeartbeat) },
	{ NWSCRIPT_ACTIONNAME("GetCustomHeartbeat") NWSCRIPT_ACTIONPROTOTYPE("int GetCustomHeartbeat( object oTarget );") 989, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetCustomHeartbeat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCustomHeartbeat) },
	{ NWSCRIPT_ACTIONNAME("SetScrollBarRanges") NWSCRIPT_ACTIONPROTOTYPE("void SetScrollBarRanges( object oPlayer, string sScreenName, string sScrollBarName, int nMinSize, int nMaxSize, int nMinValue, int nMaxValue );") 990, 7, 7, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetScrollBarRanges NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetScrollBarRanges) },
	{ NWSCRIPT_ACTIONNAME("ClearListBox") NWSCRIPT_ACTIONPROTOTYPE("void ClearListBox( object oPlayer, string sScreenName, string sListBox );") 991, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ClearListBox NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ClearListBox) },
	{ NWSCRIPT_ACTIONNAME("AddListBoxRow") NWSCRIPT_ACTIONPROTOTYPE("void AddListBoxRow( object oPlayer, string sScreenName, string sListBox, string sRowName, string sTextFields, string sTextures, string sVariables, string sHideUnhide );") 992, 8, 8, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AddListBoxRow NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AddListBoxRow) },
	{ NWSCRIPT_ACTIONNAME("RemoveListBoxRow") NWSCRIPT_ACTIONPROTOTYPE("void RemoveListBoxRow( object oPlayer, string sScreenName, string sListBox, string sRowName );") 993, 4, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_RemoveListBoxRow NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RemoveListBoxRow) },
	{ NWSCRIPT_ACTIONNAME("GetItemPropActivation") NWSCRIPT_ACTIONPROTOTYPE("int GetItemPropActivation( object oItem );") 994, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetItemPropActivation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemPropActivation) },
	{ NWSCRIPT_ACTIONNAME("SetItemPropActivation") NWSCRIPT_ACTIONPROTOTYPE("void SetItemPropActivation( object oItem, int nPref );") 995, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetItemPropActivation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetItemPropActivation) },
	{ NWSCRIPT_ACTIONNAME("ModifyListBoxRow") NWSCRIPT_ACTIONPROTOTYPE("void ModifyListBoxRow( object oPlayer, string sScreenName, string sListBox, string sRowName, string sTextFields, string sTextures, string sVariables, string sHideUnhide );") 996, 8, 8, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ModifyListBoxRow NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ModifyListBoxRow) },
	{ NWSCRIPT_ACTIONNAME("SetFactionLeader") NWSCRIPT_ACTIONPROTOTYPE("void SetFactionLeader( object oNewLeader );") 997, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetFactionLeader NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetFactionLeader) },
	{ NWSCRIPT_ACTIONNAME("GetFirstSubArea") NWSCRIPT_ACTIONPROTOTYPE("object GetFirstSubArea( object oArea, vector vPosition );") 998, 2, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetFirstSubArea NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetFirstSubArea) },
	{ NWSCRIPT_ACTIONNAME("GetNextSubArea") NWSCRIPT_ACTIONPROTOTYPE("object GetNextSubArea( object oArea );") 999, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetNextSubArea NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetNextSubArea) },
	{ NWSCRIPT_ACTIONNAME("GetMovementRateFactor") NWSCRIPT_ACTIONPROTOTYPE("float GetMovementRateFactor( object oCreature );") 1000, 1, 1, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_GetMovementRateFactor NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetMovementRateFactor) },
	{ NWSCRIPT_ACTIONNAME("SetMovementRateFactor") NWSCRIPT_ACTIONPROTOTYPE("void SetMovementRateFactor( object oCreature, float fFactor );") 1001, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetMovementRateFactor NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetMovementRateFactor) },
	{ NWSCRIPT_ACTIONNAME("GetBicFileName") NWSCRIPT_ACTIONPROTOTYPE("string GetBicFileName( object oPC );") 1002, 1, 1, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetBicFileName NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetBicFileName) },
	{ NWSCRIPT_ACTIONNAME("GetCollision") NWSCRIPT_ACTIONPROTOTYPE("int GetCollision(object oTarget);") 1003, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetCollision NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCollision) },
	{ NWSCRIPT_ACTIONNAME("SetCollision") NWSCRIPT_ACTIONPROTOTYPE("void SetCollision(object oTarget, int bCollision);") 1004, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetCollision NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetCollision) },
	{ NWSCRIPT_ACTIONNAME("GetItemIcon") NWSCRIPT_ACTIONPROTOTYPE("int GetItemIcon(object oTarget);") 1005, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetItemIcon NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemIcon) },
	{ NWSCRIPT_ACTIONNAME("GetVariableName") NWSCRIPT_ACTIONPROTOTYPE("string GetVariableName(object oTarget, int nPosition);") 1006, 2, 2, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetVariableName NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetVariableName) },
	{ NWSCRIPT_ACTIONNAME("GetVariableType") NWSCRIPT_ACTIONPROTOTYPE("int GetVariableType(object oTarget, int nPosition);") 1007, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetVariableType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetVariableType) },
	{ NWSCRIPT_ACTIONNAME("GetAreaOfEffectDuration") NWSCRIPT_ACTIONPROTOTYPE("int GetAreaOfEffectDuration( object oAreaOfEffectObject=OBJECT_SELF );") 1008, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetAreaOfEffectDuration NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAreaOfEffectDuration) },
	{ NWSCRIPT_ACTIONNAME("GetIsPlayerCreated") NWSCRIPT_ACTIONPROTOTYPE("int GetIsPlayerCreated( object oCreature );") 1009, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsPlayerCreated NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsPlayerCreated) },
	{ NWSCRIPT_ACTIONNAME("GetPartyName") NWSCRIPT_ACTIONPROTOTYPE("string GetPartyName();") 1010, 0, 0, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetPartyName NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPartyName) },
	{ NWSCRIPT_ACTIONNAME("GetPartyMotto") NWSCRIPT_ACTIONPROTOTYPE("string GetPartyMotto();") 1011, 0, 0, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetPartyMotto NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPartyMotto) },
	{ NWSCRIPT_ACTIONNAME("GetIsOverlandMap") NWSCRIPT_ACTIONPROTOTYPE("int GetIsOverlandMap(object oArea);") 1012, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsOverlandMap NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsOverlandMap) },
	{ NWSCRIPT_ACTIONNAME("SetUnrestrictedLevelUp") NWSCRIPT_ACTIONPROTOTYPE("int SetUnrestrictedLevelUp(object oCreature);") 1013, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_SetUnrestrictedLevelUp NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetUnrestrictedLevelUp) },
	{ NWSCRIPT_ACTIONNAME("GetSoundFileDuration") NWSCRIPT_ACTIONPROTOTYPE("int GetSoundFileDuration( string sSoundFile );") 1014, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetSoundFileDuration NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetSoundFileDuration) },
	{ NWSCRIPT_ACTIONNAME("GetPartyMembersDyingFlag") NWSCRIPT_ACTIONPROTOTYPE("int GetPartyMembersDyingFlag();") 1015, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetPartyMembersDyingFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPartyMembersDyingFlag) },
	{ NWSCRIPT_ACTIONNAME("SetListBoxRowSelected") NWSCRIPT_ACTIONPROTOTYPE("void SetListBoxRowSelected( object oPlayer, string sScreenName, string sListBox, string sRowName );") 1016, 4, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetListBoxRowSelected NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetListBoxRowSelected) },
	{ NWSCRIPT_ACTIONNAME("GetTalkTableLanguage") NWSCRIPT_ACTIONPROTOTYPE("int GetTalkTableLanguage();") 1017, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetTalkTableLanguage NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetTalkTableLanguage) },
	{ NWSCRIPT_ACTIONNAME("SetScrollBarValue") NWSCRIPT_ACTIONPROTOTYPE("void SetScrollBarValue( object oPlayer, string sScreenName, string sScrollBarName, int nValue );") 1018, 4, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetScrollBarValue NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetScrollBarValue) },
	{ NWSCRIPT_ACTIONNAME("SetPause") NWSCRIPT_ACTIONPROTOTYPE("void SetPause( int bState );") 1019, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetPause NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetPause) },
	{ NWSCRIPT_ACTIONNAME("GetPause") NWSCRIPT_ACTIONPROTOTYPE("int GetPause();") 1020, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetPause NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetPause) },
	{ NWSCRIPT_ACTIONNAME("GetAreaOfEffectSpellId") NWSCRIPT_ACTIONPROTOTYPE("int GetAreaOfEffectSpellId( object oAreaOfEffectObject=OBJECT_SELF );") 1021, 0, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetAreaOfEffectSpellId NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetAreaOfEffectSpellId) },
	{ NWSCRIPT_ACTIONNAME("SetGlobalGUIVariable") NWSCRIPT_ACTIONPROTOTYPE("void SetGlobalGUIVariable( object oPlayer, int nVarIndex, string sVarValue );") 1022, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetGlobalGUIVariable NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetGlobalGUIVariable) },
	{ NWSCRIPT_ACTIONNAME("CreateInstancedAreaFromSource") NWSCRIPT_ACTIONPROTOTYPE("object CreateInstancedAreaFromSource(object oArea);") 1023, 1, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_CreateInstancedAreaFromSource NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_CreateInstancedAreaFromSource) },
	{ NWSCRIPT_ACTIONNAME("GetVariableValueInt") NWSCRIPT_ACTIONPROTOTYPE("int GetVariableValueInt(object oObject, int nIndex);") 1024, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetVariableValueInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetVariableValueInt) },
	{ NWSCRIPT_ACTIONNAME("GetVariableValueString") NWSCRIPT_ACTIONPROTOTYPE("string GetVariableValueString(object oObject, int nIndex);") 1025, 2, 2, ACTIONTYPE_STRING, NWN2_NWActionParameterTypes_GetVariableValueString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetVariableValueString) },
	{ NWSCRIPT_ACTIONNAME("GetVariableValueFloat") NWSCRIPT_ACTIONPROTOTYPE("float  GetVariableValueFloat(object oObject, int nIndex);") 1026, 2, 2, ACTIONTYPE_FLOAT, NWN2_NWActionParameterTypes_GetVariableValueFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetVariableValueFloat) },
	{ NWSCRIPT_ACTIONNAME("GetVariableValueLocation") NWSCRIPT_ACTIONPROTOTYPE("location GetVariableValueLocation(object oObject, int nIndex);") 1027, 2, 2, ACTIONTYPE_LOCATION, NWN2_NWActionParameterTypes_GetVariableValueLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetVariableValueLocation) },
	{ NWSCRIPT_ACTIONNAME("GetVariableValueObject") NWSCRIPT_ACTIONPROTOTYPE("object GetVariableValueObject(object oObject, int nIndex);") 1028, 2, 2, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetVariableValueObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetVariableValueObject) },
	{ NWSCRIPT_ACTIONNAME("GetVariableCount") NWSCRIPT_ACTIONPROTOTYPE("int GetVariableCount(object oObject);") 1029, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetVariableCount NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetVariableCount) },
	{ NWSCRIPT_ACTIONNAME("SetBaseAbilityScore") NWSCRIPT_ACTIONPROTOTYPE("void SetBaseAbilityScore(object oCreature, int nAbilityType, int nScore);") 1030, 3, 3, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetBaseAbilityScore NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetBaseAbilityScore) },
	{ NWSCRIPT_ACTIONNAME("SetBaseSkillRank") NWSCRIPT_ACTIONPROTOTYPE("void SetBaseSkillRank(object oCreature, int nSkill, int nRank, int bTrackWithLevel = TRUE);") 1031, 3, 4, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetBaseSkillRank NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetBaseSkillRank) },
	{ NWSCRIPT_ACTIONNAME("SendChatMessage") NWSCRIPT_ACTIONPROTOTYPE("void SendChatMessage(object oSender, object oReceiver, int nChannel, string sMessage, int bInvokeCallback = FALSE);") 1032, 4, 5, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SendChatMessage NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SendChatMessage) },
	{ NWSCRIPT_ACTIONNAME("GetIsLocationValid") NWSCRIPT_ACTIONPROTOTYPE("int GetIsLocationValid(location lLocation);") 1033, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetIsLocationValid NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetIsLocationValid) },
	{ NWSCRIPT_ACTIONNAME("GetSurfaceMaterialsAtLocation") NWSCRIPT_ACTIONPROTOTYPE("int GetSurfaceMaterialsAtLocation(location lLocation);") 1034, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetSurfaceMaterialsAtLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetSurfaceMaterialsAtLocation) },
	{ NWSCRIPT_ACTIONNAME("GetSpellKnown") NWSCRIPT_ACTIONPROTOTYPE("int GetSpellKnown(object oCreature, int nSpell) ;") 1035, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetSpellKnown NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetSpellKnown) },
	{ NWSCRIPT_ACTIONNAME("GetItemBaseMaterialType") NWSCRIPT_ACTIONPROTOTYPE("int GetItemBaseMaterialType(object oItem);") 1036, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetItemBaseMaterialType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetItemBaseMaterialType) },
	{ NWSCRIPT_ACTIONNAME("SetItemBaseMaterialType") NWSCRIPT_ACTIONPROTOTYPE("void SetItemBaseMaterialType(object oItem, int nMaterialType);") 1037, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetItemBaseMaterialType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetItemBaseMaterialType) },
	{ NWSCRIPT_ACTIONNAME("SetSpellKnown") NWSCRIPT_ACTIONPROTOTYPE("void SetSpellKnown(object oCreature, int nClassPosition, int nSpell, int bKnown = TRUE, int bTrackWithLevel = TRUE);") 1038, 3, 5, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetSpellKnown NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetSpellKnown) },
	{ NWSCRIPT_ACTIONNAME("GetLimboCreatureCount") NWSCRIPT_ACTIONPROTOTYPE("int GetLimboCreatureCount();") 1039, 0, 0, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetLimboCreatureCount NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetLimboCreatureCount) },
	{ NWSCRIPT_ACTIONNAME("GetCreatureInLimbo") NWSCRIPT_ACTIONPROTOTYPE("object GetCreatureInLimbo(int nTh = 0);") 1040, 0, 1, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetCreatureInLimbo NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCreatureInLimbo) },
	{ NWSCRIPT_ACTIONNAME("SendCreatureToLimbo") NWSCRIPT_ACTIONPROTOTYPE("void SendCreatureToLimbo(object oCreature);") 1041, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SendCreatureToLimbo NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SendCreatureToLimbo) },
	{ NWSCRIPT_ACTIONNAME("AddScriptParameterInt") NWSCRIPT_ACTIONPROTOTYPE("void AddScriptParameterInt(int nParam);") 1042, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AddScriptParameterInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AddScriptParameterInt) },
	{ NWSCRIPT_ACTIONNAME("AddScriptParameterString") NWSCRIPT_ACTIONPROTOTYPE("void AddScriptParameterString(string sParam);") 1043, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AddScriptParameterString NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AddScriptParameterString) },
	{ NWSCRIPT_ACTIONNAME("AddScriptParameterFloat") NWSCRIPT_ACTIONPROTOTYPE("void AddScriptParameterFloat(float fParam);") 1044, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AddScriptParameterFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AddScriptParameterFloat) },
	{ NWSCRIPT_ACTIONNAME("AddScriptParameterObject") NWSCRIPT_ACTIONPROTOTYPE("void AddScriptParameterObject(object oParam);") 1045, 1, 1, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_AddScriptParameterObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_AddScriptParameterObject) },
	{ NWSCRIPT_ACTIONNAME("ExecuteScriptEnhanced") NWSCRIPT_ACTIONPROTOTYPE("int  ExecuteScriptEnhanced(string sScript, object oTarget, int bClearParams = TRUE);") 1046, 2, 3, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_ExecuteScriptEnhanced NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ExecuteScriptEnhanced) },
	{ NWSCRIPT_ACTIONNAME("ClearScriptParams") NWSCRIPT_ACTIONPROTOTYPE("void  ClearScriptParams();") 1047, 0, 0, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_ClearScriptParams NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_ClearScriptParams) },
	{ NWSCRIPT_ACTIONNAME("SetSkillPointsRemaining") NWSCRIPT_ACTIONPROTOTYPE("void SetSkillPointsRemaining(object oPC, int nPoints);") 1048, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetSkillPointsRemaining NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetSkillPointsRemaining) },
	{ NWSCRIPT_ACTIONNAME("GetSkillPointsRemaining") NWSCRIPT_ACTIONPROTOTYPE("int GetSkillPointsRemaining(object oPC);") 1049, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetSkillPointsRemaining NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetSkillPointsRemaining) },
	{ NWSCRIPT_ACTIONNAME("GetCasterClassSpellSchool") NWSCRIPT_ACTIONPROTOTYPE("int GetCasterClassSpellSchool(object oPC, int nClassPos);") 1050, 2, 2, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetCasterClassSpellSchool NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetCasterClassSpellSchool) },
	{ NWSCRIPT_ACTIONNAME("SetGender") NWSCRIPT_ACTIONPROTOTYPE("void SetGender(object oCreature, int nGender);") 1051, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetGender NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetGender) },
	{ NWSCRIPT_ACTIONNAME("SetTag") NWSCRIPT_ACTIONPROTOTYPE("void SetTag(object oObject, string sNewTag);") 1052, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetTag) },
	{ NWSCRIPT_ACTIONNAME("GetArmorRulesType") NWSCRIPT_ACTIONPROTOTYPE("int GetArmorRulesType(object oItem);") 1053, 1, 1, ACTIONTYPE_INT, NWN2_NWActionParameterTypes_GetArmorRulesType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetArmorRulesType) },
	{ NWSCRIPT_ACTIONNAME("SetArmorRulesType") NWSCRIPT_ACTIONPROTOTYPE("void SetArmorRulesType(object oItem, int nType);") 1054, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetArmorRulesType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetArmorRulesType) },
	{ NWSCRIPT_ACTIONNAME("SetItemIcon") NWSCRIPT_ACTIONPROTOTYPE("void SetItemIcon(object oItem, int nIcon);") 1055, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_SetItemIcon NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_SetItemIcon) },
	{ NWSCRIPT_ACTIONNAME("GetObjectByTagAndType") NWSCRIPT_ACTIONPROTOTYPE("object GetObjectByTagAndType(string sTag, int nObjectType, int nTh);") 1056, 3, 3, ACTIONTYPE_OBJECT, NWN2_NWActionParameterTypes_GetObjectByTagAndType NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_GetObjectByTagAndType) },
	{ NWSCRIPT_ACTIONNAME("RecallCreatureFromLimboToLocation") NWSCRIPT_ACTIONPROTOTYPE("void RecallCreatureFromLimboToLocation(object oCreature, location loc);") 1057, 2, 2, ACTIONTYPE_VOID, NWN2_NWActionParameterTypes_RecallCreatureFromLimboToLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN2_NWActionTotalParameterSizes_RecallCreatureFromLimboToLocation) }
};


//
// Define the NWN1 data tables for nwscript.nss.
//



static const NWACTION_TYPE NWN1_NWActionParameterTypes_Random[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_PrintString[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_PrintFloat[ 3 ] = { ACTIONTYPE_FLOAT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_FloatToString[ 3 ] = { ACTIONTYPE_FLOAT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_PrintInteger[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_PrintObject[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_AssignCommand[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_ACTION };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_DelayCommand[ 2 ] = { ACTIONTYPE_FLOAT, ACTIONTYPE_ACTION };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ExecuteScript[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ClearAllActions[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetFacing[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetCalendar[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetTime[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetCalendarYear = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetCalendarMonth = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetCalendarDay = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetTimeHour = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetTimeMinute = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetTimeSecond = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetTimeMillisecond = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_ActionRandomWalk = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionMoveToLocation[ 2 ] = { ACTIONTYPE_LOCATION, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionMoveToObject[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionMoveAwayFromObject[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetArea[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetEnteringObject = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetExitingObject = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetPosition[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFacing[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetItemPossessor[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetItemPossessedBy[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_CreateItemOnObject[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionEquipItem[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionUnequipItem[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionPickUpItem[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionPutDownItem[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLastAttacker[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionAttack[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetNearestCreature[ 8 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionSpeakString[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionPlayAnimation[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetDistanceToObject[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsObjectValid[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionOpenDoor[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionCloseDoor[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetCameraFacing[ 4 ] = { ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_PlaySound[ 1 ] = { ACTIONTYPE_STRING };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetSpellTargetObject = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionCastSpellAtObject[ 7 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCurrentHitPoints[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetMaxHitPoints[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLocalInt[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLocalFloat[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLocalString[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLocalObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetLocalInt[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetLocalFloat[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetLocalString[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetLocalObject[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetStringLength[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetStringUpperCase[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetStringLowerCase[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetStringRight[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetStringLeft[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_InsertString[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetSubString[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_FindSubString[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_fabs[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_cos[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_sin[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_tan[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_acos[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_asin[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_atan[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_log[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_pow[ 2 ] = { ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_sqrt[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_abs[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectHeal[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectDamage[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectAbilityIncrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectDamageResistance[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectResurrection = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectSummonCreature[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCasterLevel[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFirstEffect[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetNextEffect[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_RemoveEffect[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsEffectValid[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetEffectDurationType[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetEffectSubType[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetEffectCreator[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_IntToString[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFirstObjectInArea[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetNextObjectInArea[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_d2[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_d3[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_d4[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_d6[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_d8[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_d10[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_d12[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_d20[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_d100[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_VectorMagnitude[ 1 ] = { ACTIONTYPE_VECTOR };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetMetaMagicFeat = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetObjectType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetRacialType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_FortitudeSave[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ReflexSave[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_WillSave[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetSpellSaveDC = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_MagicalEffect[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SupernaturalEffect[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ExtraordinaryEffect[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectACIncrease[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetAC[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectSavingThrowIncrease[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectAttackIncrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectDamageReduction[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectDamageIncrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_RoundsToSeconds[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_HoursToSeconds[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_TurnsToSeconds[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLawChaosValue[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetGoodEvilValue[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetAlignmentLawChaos[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetAlignmentGoodEvil[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFirstObjectInShape[ 6 ] = { ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_VECTOR };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetNextObjectInShape[ 6 ] = { ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_VECTOR };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectEntangle = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SignalEvent[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_EVENT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EventUserDefined[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectDeath[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectKnockdown = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionGiveItem[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionTakeItem[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_VectorNormalize[ 1 ] = { ACTIONTYPE_VECTOR };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectCurse[ 6 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetAbilityScore[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsDead[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_PrintVector[ 2 ] = { ACTIONTYPE_VECTOR, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_Vector[ 3 ] = { ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetFacingPoint[ 1 ] = { ACTIONTYPE_VECTOR };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_AngleToVector[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_VectorToAngle[ 1 ] = { ACTIONTYPE_VECTOR };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_TouchAttackMelee[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_TouchAttackRanged[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectParalyze = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectSpellImmunity[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectDeaf = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetDistanceBetween[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetLocalLocation[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLocalLocation[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectSleep = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetItemInSlot[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectCharmed = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectConfused = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectFrightened = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectDominated = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectDazed = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectStunned = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetCommandable[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCommandable[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectRegenerate[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectMovementSpeedIncrease[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetHitDice[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionForceFollowObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ResistSpell[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetEffectType[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectAreaOfEffect[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFactionEqual[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ChangeFaction[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsListening[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetListening[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetListenPattern[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_TestStringAgainstPattern[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetMatchedSubstring[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetMatchedSubstringsCount = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectVisualEffect[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFactionWeakestMember[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFactionStrongestMember[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFactionMostDamagedMember[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFactionLeastDamagedMember[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFactionGold[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFactionAverageReputation[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFactionAverageGoodEvilAlignment[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFactionAverageLawChaosAlignment[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFactionAverageLevel[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFactionAverageXP[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFactionMostFrequentClass[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFactionWorstAC[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFactionBestAC[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionSit[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetListenPatternNumber = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionJumpToObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetWaypointByTag[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTransitionTarget[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectLinkEffects[ 2 ] = { ACTIONTYPE_EFFECT, ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetObjectByTag[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_AdjustAlignment[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionWait[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetAreaTransitionBMP[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionStartConversation[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_ActionPauseConversation = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_ActionResumeConversation = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectBeam[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetReputation[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_AdjustReputation[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetSittingCreature[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetGoingToBeAttackedBy[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectSpellResistanceIncrease[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLocation[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionJumpToLocation[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_Location[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_VECTOR, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ApplyEffectAtLocation[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_EFFECT, ACTIONTYPE_LOCATION, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsPC[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_FeetToMeters[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_YardsToMeters[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ApplyEffectToObject[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_EFFECT, ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SpeakString[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetSpellTargetLocation = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetPositionFromLocation[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetAreaFromLocation[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFacingFromLocation[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetNearestCreatureToLocation[ 8 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetNearestObject[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetNearestObjectToLocation[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_LOCATION, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetNearestObjectByTag[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_IntToFloat[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_FloatToInt[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_StringToInt[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_StringToFloat[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionCastSpellAtLocation[ 6 ] = { ACTIONTYPE_INT, ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsEnemy[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsFriend[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsNeutral[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetPCSpeaker = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetStringByStrRef[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionSpeakStringByStrRef[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_DestroyObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetModule = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_CreateObject[ 5 ] = { ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EventSpellCastAt[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastSpellCaster = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastSpell = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetUserDefinedEventNumber = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetSpellId = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_RandomName[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectPoison[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectDisease[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectSilence = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetName[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastSpeaker = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_BeginConversation[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastPerceived = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastPerceptionHeard = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastPerceptionInaudible = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastPerceptionSeen = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastClosedBy = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastPerceptionVanished = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFirstInPersistentObject[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetNextInPersistentObject[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetAreaOfEffectCreator[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_DeleteLocalInt[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_DeleteLocalFloat[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_DeleteLocalString[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_DeleteLocalObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_DeleteLocalLocation[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectHaste = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectSlow = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ObjectToString[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectImmunity[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsImmune[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectDamageImmunityIncrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetEncounterActive[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetEncounterActive[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetEncounterSpawnsMax[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetEncounterSpawnsMax[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetEncounterSpawnsCurrent[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetEncounterSpawnsCurrent[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetModuleItemAcquired = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetModuleItemAcquiredFrom = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetCustomToken[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetHasFeat[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetHasSkill[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionUseFeat[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionUseSkill[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetObjectSeen[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetObjectHeard[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastPlayerDied = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetModuleItemLost = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetModuleItemLostBy = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionDoCommand[ 1 ] = { ACTIONTYPE_ACTION };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EventConversation = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetEncounterDifficulty[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetEncounterDifficulty[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetDistanceBetweenLocations[ 2 ] = { ACTIONTYPE_LOCATION, ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetReflexAdjustedDamage[ 5 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_PlayAnimation[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_TalentSpell[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_TalentFeat[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_TalentSkill[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetHasSpellEffect[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetEffectSpellId[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCreatureHasTalent[ 2 ] = { ACTIONTYPE_TALENT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCreatureTalentRandom[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCreatureTalentBest[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionUseTalentOnObject[ 2 ] = { ACTIONTYPE_TALENT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionUseTalentAtLocation[ 2 ] = { ACTIONTYPE_TALENT, ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetGoldPieceValue[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsPlayableRacialType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_JumpToLocation[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectTemporaryHitpoints[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetSkillRank[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetAttackTarget[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLastAttackType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLastAttackMode[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetMaster[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsInCombat[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLastAssociateCommand[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GiveGoldToCreature[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetIsDestroyable[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetLocked[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLocked[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetClickingObject = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetAssociateListenPatterns[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLastWeaponUsed[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionInteractObject[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastUsedBy = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetAbilityModifier[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIdentified[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetIdentified[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SummonAnimalCompanion[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SummonFamiliar[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetBlockingDoor = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsDoorActionPossible[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_DoDoorAction[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFirstItemInInventory[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetNextItemInInventory[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetClassByPosition[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLevelByPosition[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLevelByClass[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetDamageDealtByType[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetTotalDamageDealt = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLastDamager[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastDisarmed = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastDisturbed = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastLocked = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastUnlocked = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectSkillIncrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetInventoryDisturbType = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetInventoryDisturbItem = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetHenchman[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_VersusAlignmentEffect[ 3 ] = { ACTIONTYPE_EFFECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_VersusRacialTypeEffect[ 2 ] = { ACTIONTYPE_EFFECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_VersusTrapEffect[ 1 ] = { ACTIONTYPE_EFFECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetGender[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsTalentValid[ 1 ] = { ACTIONTYPE_TALENT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionMoveAwayFromLocation[ 3 ] = { ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetAttemptedAttackTarget = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTypeFromTalent[ 1 ] = { ACTIONTYPE_TALENT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIdFromTalent[ 1 ] = { ACTIONTYPE_TALENT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetAssociate[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_AddHenchman[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_RemoveHenchman[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_AddJournalQuestEntry[ 6 ] = { ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_RemoveJournalQuestEntry[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetPCPublicCDKey[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetPCIPAddress[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetPCPlayerName[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetPCLike[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetPCDislike[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SendMessageToPC[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetAttemptedSpellTarget = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastOpenedBy = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetHasSpell[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_OpenStore[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectTurned = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFirstFactionMember[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetNextFactionMember[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionForceMoveToLocation[ 3 ] = { ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionForceMoveToObject[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetJournalQuestExperience[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_JumpToObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetMapPinEnabled[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectHitPointChangeWhenDying[ 1 ] = { ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_PopUpGUIPanel[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ClearPersonalReputation[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetIsTemporaryFriend[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetIsTemporaryEnemy[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetIsTemporaryNeutral[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GiveXPToCreature[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetXP[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetXP[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_IntToHexString[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetBaseItemType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetItemHasItemProperty[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionEquipMostDamagingMelee[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionEquipMostDamagingRanged[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetItemACValue[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionRest[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ExploreAreaForPlayer[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_ActionEquipMostEffectiveArmor = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetIsDay = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetIsNight = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetIsDawn = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetIsDusk = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsEncounterCreature[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastPlayerDying = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetStartingLocation = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ChangeToStandardFaction[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SoundObjectPlay[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SoundObjectStop[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SoundObjectSetVolume[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SoundObjectSetPosition[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_VECTOR };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SpeakOneLinerConversation[ 2 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetGold[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastRespawnButtonPresser = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsDM[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_PlayVoiceChat[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsWeaponEffective[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastSpellHarmful = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EventActivateItem[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_LOCATION, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_MusicBackgroundPlay[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_MusicBackgroundStop[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_MusicBackgroundSetDelay[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_MusicBackgroundChangeDay[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_MusicBackgroundChangeNight[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_MusicBattlePlay[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_MusicBattleStop[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_MusicBattleChange[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_AmbientSoundPlay[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_AmbientSoundStop[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_AmbientSoundChangeDay[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_AmbientSoundChangeNight[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastKiller = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetSpellCastItem = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetItemActivated = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetItemActivator = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetItemActivatedTargetLocation = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetItemActivatedTarget = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsOpen[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_TakeGoldFromCreature[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_IsInConversation[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectAbilityDecrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectAttackDecrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectDamageDecrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectDamageImmunityDecrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectACDecrease[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectMovementSpeedDecrease[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectSavingThrowDecrease[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectSkillDecrease[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectSpellResistanceDecrease[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetPlotFlag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetPlotFlag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectInvisibility[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectConcealment[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectDarkness = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectDispelMagicAll[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectUltravision = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectNegativeLevel[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectPolymorph[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectSanctuary[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectTrueSeeing = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectSeeInvisible = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectTimeStop = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectBlindness = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsReactionTypeFriendly[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsReactionTypeNeutral[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsReactionTypeHostile[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectSpellLevelAbsorption[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectDispelMagicBest[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActivatePortal[ 5 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetNumStackedItems[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_SurrenderToEnemies = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectMissChance[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTurnResistanceHD[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCreatureSize[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectDisappearAppear[ 2 ] = { ACTIONTYPE_LOCATION, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectDisappear[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectAppear[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionUnlockObject[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionLockObject[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectModifyAttacks[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLastTrapDetected[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectDamageShield[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetNearestTrapToObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetDeity[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetSubRace[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFortitudeSavingThrow[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetWillSavingThrow[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetReflexSavingThrow[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetChallengeRating[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetAge[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetMovementRate[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFamiliarCreatureType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetAnimalCompanionCreatureType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFamiliarName[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetAnimalCompanionName[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionCastFakeSpellAtObject[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionCastFakeSpellAtLocation[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_LOCATION, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_RemoveSummonedAssociate[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetCameraMode[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsResting[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastPCRested = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetWeather[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastRestEventType = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_StartNewModule[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectSwarm[ 5 ] = { ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetWeaponRanged[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_DoSinglePlayerAutoSave = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetGameDifficulty = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetTileMainLightColor[ 3 ] = { ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetTileSourceLightColor[ 3 ] = { ACTIONTYPE_LOCATION, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_RecomputeStaticLighting[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTileMainLight1Color[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTileMainLight2Color[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTileSourceLight1Color[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTileSourceLight2Color[ 1 ] = { ACTIONTYPE_LOCATION };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetPanelButtonFlash[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCurrentAction[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetStandardFactionReputation[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetStandardFactionReputation[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_FloatingTextStrRefOnCreature[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_FloatingTextStringOnCreature[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTrapDisarmable[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTrapDetectable[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTrapDetectedBy[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTrapFlagged[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTrapBaseType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTrapOneShot[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTrapCreator[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTrapKeyTag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTrapDisarmDC[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTrapDetectDC[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLockKeyRequired[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLockKeyTag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLockLockable[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLockUnlockDC[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLockLockDC[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetPCLevellingUp = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetHasFeatEffect[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetPlaceableIllumination[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetPlaceableIllumination[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsPlaceableObjectActionPossible[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_DoPlaceableObjectAction[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetFirstPC = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetNextPC = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetTrapDetectedBy[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsTrapped[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectTurnResistanceDecrease[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectTurnResistanceIncrease[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_PopUpDeathGUIPanel[ 5 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetTrapDisabled[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLastHostileActor[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_ExportAllCharacters = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_MusicBackgroundGetDayTrack[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_MusicBackgroundGetNightTrack[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_WriteTimestampedLogEntry[ 1 ] = { ACTIONTYPE_STRING };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetModuleName = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFactionLeader[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SendMessageToAllDMs[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EndGame[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_BootPC[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionCounterSpell[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_AmbientSoundSetDayVolume[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_AmbientSoundSetNightVolume[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_MusicBackgroundGetBattleTrack[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetHasInventory[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetStrRefSoundDuration[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_AddToParty[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_RemoveFromParty[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetStealthMode[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetDetectMode[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetDefensiveCastingMode[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetAppearanceType[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_SpawnScriptDebugger = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetModuleItemAcquiredStackSize = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_DecrementRemainingFeatUses[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_DecrementRemainingSpellUses[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetResRef[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectPetrify = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_CopyItem[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectCutsceneParalyze = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetDroppableFlag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetUseableFlag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetStolenFlag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetCampaignFloat[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_FLOAT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetCampaignInt[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetCampaignVector[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_VECTOR, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetCampaignLocation[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_LOCATION, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetCampaignString[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_DestroyCampaignDatabase[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCampaignFloat[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCampaignInt[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCampaignVector[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCampaignLocation[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCampaignString[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_CopyObject[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_LOCATION, ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_DeleteCampaignVariable[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_StoreCampaignObject[ 4 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_RetrieveCampaignObject[ 5 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_LOCATION, ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectCutsceneDominated = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetItemStackSize[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetItemStackSize[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetItemCharges[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetItemCharges[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_AddItemProperty[ 4 ] = { ACTIONTYPE_INT, ACTIONTYPE_ITEMPROPERTY, ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_RemoveItemProperty[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsItemPropertyValid[ 1 ] = { ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFirstItemProperty[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetNextItemProperty[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetItemPropertyType[ 1 ] = { ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetItemPropertyDurationType[ 1 ] = { ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyAbilityBonus[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyACBonus[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyACBonusVsAlign[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyACBonusVsDmgType[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyACBonusVsRace[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyACBonusVsSAlign[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyEnhancementBonus[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyEnhancementBonusVsAlign[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyEnhancementBonusVsRace[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyEnhancementBonusVsSAlign[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyEnhancementPenalty[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyWeightReduction[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyBonusFeat[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyBonusLevelSpell[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyCastSpell[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyDamageBonus[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyDamageBonusVsAlign[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyDamageBonusVsRace[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyDamageBonusVsSAlign[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyDamageImmunity[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyDamagePenalty[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyDamageReduction[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyDamageResistance[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyDamageVulnerability[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyDarkvision = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyDecreaseAbility[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyDecreaseAC[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyDecreaseSkill[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyContainerReducedWeight[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyExtraMeleeDamageType[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyExtraRangeDamageType[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyHaste = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyHolyAvenger = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyImmunityMisc[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyImprovedEvasion = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyBonusSpellResistance[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyBonusSavingThrowVsX[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyBonusSavingThrow[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyKeen = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyLight[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyMaxRangeStrengthMod[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyNoDamage = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyOnHitProps[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyReducedSavingThrowVsX[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyReducedSavingThrow[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyRegeneration[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertySkillBonus[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertySpellImmunitySpecific[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertySpellImmunitySchool[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyThievesTools[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyAttackBonus[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyAttackBonusVsAlign[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyAttackBonusVsRace[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyAttackBonusVsSAlign[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyAttackPenalty[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyUnlimitedAmmo[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyLimitUseByAlign[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyLimitUseByClass[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyLimitUseByRace[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyLimitUseBySAlign[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_BadBadReplaceMeThisDoesNothing = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyVampiricRegeneration[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyTrap[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyTrueSeeing = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyOnMonsterHitProperties[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyTurnResistance[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyMassiveCritical[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyFreeAction = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyMonsterDamage[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyImmunityToSpellLevel[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertySpecialWalk[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyHealersKit[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyWeightIncrease[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsSkillSuccessful[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_EffectSpellFailure[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SpeakStringByStrRef[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetCutsceneMode[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastPCToCancelCutscene = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetDialogSoundLength[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_FadeFromBlack[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_FadeToBlack[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_StopFade[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_BlackScreen[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetBaseAttackBonus[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetImmortal[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_OpenInventory[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_StoreCameraFacing = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_RestoreCameraFacing = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_LevelUpHenchman[ 4 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetDroppableFlag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetWeight[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetModuleItemAcquiredBy = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetImmortal[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_DoWhirlwindAttack[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_Get2DAString[ 3 ] = { ACTIONTYPE_STRING, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectEthereal = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetAILevel[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetAILevel[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsPossessedFamiliar[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_UnpossessFamiliar[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsAreaInterior[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SendMessageToPCByStrRef[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_IncrementRemainingFeatUses[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ExportSingleCharacter[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_PlaySoundByStrRef[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetSubRace[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetDeity[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsDMPossessed[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetWeather[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsAreaNatural[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsAreaAboveGround[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetPCItemLastEquipped = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetPCItemLastEquippedBy = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetPCItemLastUnequipped = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetPCItemLastUnequippedBy = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_CopyItemAndModify[ 5 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetItemAppearance[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyOnHitCastSpell[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetItemPropertySubType[ 1 ] = { ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetActionMode[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetActionMode[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetArcaneSpellFailure[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ActionExamine[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyVisualEffect[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetLootable[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetLootable[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCutsceneCameraMoveRate[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetCutsceneCameraMoveRate[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetItemCursedFlag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetItemCursedFlag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetMaxHenchmen[ 1 ] = { ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetMaxHenchmen = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetAssociateType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetSpellResistance[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_DayToNight[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_NightToDay[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_LineOfSightObject[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_LineOfSightVector[ 2 ] = { ACTIONTYPE_VECTOR, ACTIONTYPE_VECTOR };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetLastSpellCastClass = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetBaseAttackBonus[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_RestoreBaseAttackBonus[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectCutsceneGhost = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyArcaneSpellFailure[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetStoreGold[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetStoreGold[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetStoreMaxBuyPrice[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetStoreMaxBuyPrice[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetStoreIdentifyCost[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetStoreIdentifyCost[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetCreatureAppearanceType[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCreatureStartingPackage[ 1 ] = { ACTIONTYPE_OBJECT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_EffectCutsceneImmobilize = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsInSubArea[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetItemPropertyCostTable[ 1 ] = { ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetItemPropertyCostTableValue[ 1 ] = { ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetItemPropertyParam1[ 1 ] = { ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetItemPropertyParam1Value[ 1 ] = { ACTIONTYPE_ITEMPROPERTY };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetIsCreatureDisarmable[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetStolenFlag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ForceRest[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetCameraHeight[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_FLOAT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetSkyBox[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetPhenoType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetPhenoType[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetFogColor[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCutsceneMode[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetSkyBox[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFogColor[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetFogAmount[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFogAmount[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetPickpocketableFlag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetPickpocketableFlag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetFootstepType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetFootstepType[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCreatureWingType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetCreatureWingType[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCreatureBodyPart[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetCreatureBodyPart[ 3 ] = { ACTIONTYPE_INT, ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetCreatureTailType[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetCreatureTailType[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetHardness[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetHardness[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetLockKeyRequired[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetLockKeyTag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetLockLockable[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetLockUnlockDC[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetLockLockDC[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetTrapDisarmable[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetTrapDetectable[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetTrapOneShot[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetTrapKeyTag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetTrapDisarmDC[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetTrapDetectDC[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_CreateTrapAtLocation[ 7 ] = { ACTIONTYPE_INT, ACTIONTYPE_LOCATION, ACTIONTYPE_FLOAT, ACTIONTYPE_STRING, ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_CreateTrapOnObject[ 5 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_STRING, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetWillSavingThrow[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetReflexSavingThrow[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetFortitudeSavingThrow[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTilesetResRef[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTrapRecoverable[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetTrapRecoverable[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetModuleXPScale = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetModuleXPScale[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetKeyRequiredFeedback[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetKeyRequiredFeedback[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetTrapActive[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetTrapActive[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_LockCameraPitch[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_LockCameraDistance[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_LockCameraDirection[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetPlaceableLastClickedBy = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetInfiniteFlag[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetInfiniteFlag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetAreaSize[ 2 ] = { ACTIONTYPE_INT, ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetName[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetPortraitId[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetPortraitId[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetPortraitResRef[ 1 ] = { ACTIONTYPE_OBJECT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetPortraitResRef[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetUseableFlag[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetDescription[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetDescription[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_STRING, ACTIONTYPE_INT };
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetPCChatSpeaker = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetPCChatMessage = NULL;
static PCNWACTION_TYPE NWN1_NWActionParameterTypes_GetPCChatVolume = NULL;
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetPCChatMessage[ 1 ] = { ACTIONTYPE_STRING };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetPCChatVolume[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_GetColor[ 2 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_SetColor[ 3 ] = { ACTIONTYPE_OBJECT, ACTIONTYPE_INT, ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyMaterial[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyQuality[ 1 ] = { ACTIONTYPE_INT };
static const NWACTION_TYPE NWN1_NWActionParameterTypes_ItemPropertyAdditional[ 1 ] = { ACTIONTYPE_INT };



static const unsigned long NWN1_NWActionTotalParameterSizes_Random[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_PrintString[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_PrintFloat[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_FloatToString[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_PrintInteger[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_PrintObject[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_AssignCommand[ 2 ] = { 4, 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_DelayCommand[ 2 ] = { 4, 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ExecuteScript[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ClearAllActions[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetFacing[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetCalendar[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetTime[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetCalendarYear = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetCalendarMonth = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetCalendarDay = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetTimeHour = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetTimeMinute = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetTimeSecond = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetTimeMillisecond = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_ActionRandomWalk = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionMoveToLocation[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionMoveToObject[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionMoveAwayFromObject[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetArea[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetEnteringObject = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetExitingObject = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetPosition[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFacing[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetItemPossessor[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetItemPossessedBy[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_CreateItemOnObject[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionEquipItem[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionUnequipItem[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionPickUpItem[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionPutDownItem[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLastAttacker[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionAttack[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetNearestCreature[ 8 ] = { 4, 8, 12, 16, 20, 24, 28, 32 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionSpeakString[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionPlayAnimation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetDistanceToObject[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsObjectValid[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionOpenDoor[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionCloseDoor[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetCameraFacing[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_PlaySound[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetSpellTargetObject = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionCastSpellAtObject[ 7 ] = { 4, 8, 12, 16, 20, 24, 28 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCurrentHitPoints[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetMaxHitPoints[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLocalInt[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLocalFloat[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLocalString[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLocalObject[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetLocalInt[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetLocalFloat[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetLocalString[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetLocalObject[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetStringLength[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetStringUpperCase[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetStringLowerCase[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetStringRight[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetStringLeft[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_InsertString[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetSubString[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_FindSubString[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_fabs[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_cos[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_sin[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_tan[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_acos[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_asin[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_atan[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_log[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_pow[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_sqrt[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_abs[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectHeal[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectDamage[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectAbilityIncrease[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectDamageResistance[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectResurrection = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectSummonCreature[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCasterLevel[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFirstEffect[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetNextEffect[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_RemoveEffect[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsEffectValid[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetEffectDurationType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetEffectSubType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetEffectCreator[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_IntToString[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFirstObjectInArea[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetNextObjectInArea[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_d2[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_d3[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_d4[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_d6[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_d8[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_d10[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_d12[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_d20[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_d100[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_VectorMagnitude[ 1 ] = { 12 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetMetaMagicFeat = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetObjectType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetRacialType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_FortitudeSave[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ReflexSave[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_WillSave[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetSpellSaveDC = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_MagicalEffect[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SupernaturalEffect[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ExtraordinaryEffect[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectACIncrease[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetAC[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectSavingThrowIncrease[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectAttackIncrease[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectDamageReduction[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectDamageIncrease[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_RoundsToSeconds[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_HoursToSeconds[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_TurnsToSeconds[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLawChaosValue[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetGoodEvilValue[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetAlignmentLawChaos[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetAlignmentGoodEvil[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFirstObjectInShape[ 6 ] = { 4, 8, 12, 16, 20, 32 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetNextObjectInShape[ 6 ] = { 4, 8, 12, 16, 20, 32 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectEntangle = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_SignalEvent[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EventUserDefined[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectDeath[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectKnockdown = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionGiveItem[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionTakeItem[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_VectorNormalize[ 1 ] = { 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectCurse[ 6 ] = { 4, 8, 12, 16, 20, 24 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetAbilityScore[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsDead[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_PrintVector[ 2 ] = { 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_Vector[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetFacingPoint[ 1 ] = { 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_AngleToVector[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_VectorToAngle[ 1 ] = { 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_TouchAttackMelee[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_TouchAttackRanged[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectParalyze = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectSpellImmunity[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectDeaf = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetDistanceBetween[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetLocalLocation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLocalLocation[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectSleep = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetItemInSlot[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectCharmed = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectConfused = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectFrightened = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectDominated = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectDazed = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectStunned = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_SetCommandable[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCommandable[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectRegenerate[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectMovementSpeedIncrease[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetHitDice[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionForceFollowObject[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTag[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ResistSpell[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetEffectType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectAreaOfEffect[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFactionEqual[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ChangeFaction[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsListening[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetListening[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetListenPattern[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_TestStringAgainstPattern[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetMatchedSubstring[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetMatchedSubstringsCount = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectVisualEffect[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFactionWeakestMember[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFactionStrongestMember[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFactionMostDamagedMember[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFactionLeastDamagedMember[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFactionGold[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFactionAverageReputation[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFactionAverageGoodEvilAlignment[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFactionAverageLawChaosAlignment[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFactionAverageLevel[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFactionAverageXP[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFactionMostFrequentClass[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFactionWorstAC[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFactionBestAC[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionSit[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetListenPatternNumber = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionJumpToObject[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetWaypointByTag[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTransitionTarget[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectLinkEffects[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetObjectByTag[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_AdjustAlignment[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionWait[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetAreaTransitionBMP[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionStartConversation[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_ActionPauseConversation = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_ActionResumeConversation = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectBeam[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetReputation[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_AdjustReputation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetSittingCreature[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetGoingToBeAttackedBy[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectSpellResistanceIncrease[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLocation[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionJumpToLocation[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_Location[ 3 ] = { 4, 16, 20 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ApplyEffectAtLocation[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsPC[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_FeetToMeters[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_YardsToMeters[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ApplyEffectToObject[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SpeakString[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetSpellTargetLocation = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetPositionFromLocation[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetAreaFromLocation[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFacingFromLocation[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetNearestCreatureToLocation[ 8 ] = { 4, 8, 12, 16, 20, 24, 28, 32 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetNearestObject[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetNearestObjectToLocation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetNearestObjectByTag[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_IntToFloat[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_FloatToInt[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_StringToInt[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_StringToFloat[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionCastSpellAtLocation[ 6 ] = { 4, 8, 12, 16, 20, 24 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsEnemy[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsFriend[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsNeutral[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetPCSpeaker = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetStringByStrRef[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionSpeakStringByStrRef[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_DestroyObject[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetModule = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_CreateObject[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EventSpellCastAt[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastSpellCaster = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastSpell = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetUserDefinedEventNumber = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetSpellId = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_RandomName[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectPoison[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectDisease[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectSilence = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetName[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastSpeaker = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_BeginConversation[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastPerceived = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastPerceptionHeard = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastPerceptionInaudible = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastPerceptionSeen = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastClosedBy = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastPerceptionVanished = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFirstInPersistentObject[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetNextInPersistentObject[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetAreaOfEffectCreator[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_DeleteLocalInt[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_DeleteLocalFloat[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_DeleteLocalString[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_DeleteLocalObject[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_DeleteLocalLocation[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectHaste = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectSlow = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_ObjectToString[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectImmunity[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsImmune[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectDamageImmunityIncrease[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetEncounterActive[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetEncounterActive[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetEncounterSpawnsMax[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetEncounterSpawnsMax[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetEncounterSpawnsCurrent[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetEncounterSpawnsCurrent[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetModuleItemAcquired = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetModuleItemAcquiredFrom = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_SetCustomToken[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetHasFeat[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetHasSkill[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionUseFeat[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionUseSkill[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetObjectSeen[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetObjectHeard[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastPlayerDied = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetModuleItemLost = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetModuleItemLostBy = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionDoCommand[ 1 ] = { 0 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EventConversation = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_SetEncounterDifficulty[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetEncounterDifficulty[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetDistanceBetweenLocations[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetReflexAdjustedDamage[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN1_NWActionTotalParameterSizes_PlayAnimation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_TalentSpell[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_TalentFeat[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_TalentSkill[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetHasSpellEffect[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetEffectSpellId[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCreatureHasTalent[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCreatureTalentRandom[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCreatureTalentBest[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionUseTalentOnObject[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionUseTalentAtLocation[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetGoldPieceValue[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsPlayableRacialType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_JumpToLocation[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectTemporaryHitpoints[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetSkillRank[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetAttackTarget[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLastAttackType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLastAttackMode[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetMaster[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsInCombat[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLastAssociateCommand[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GiveGoldToCreature[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetIsDestroyable[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetLocked[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLocked[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetClickingObject = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_SetAssociateListenPatterns[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLastWeaponUsed[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionInteractObject[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastUsedBy = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetAbilityModifier[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIdentified[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetIdentified[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SummonAnimalCompanion[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SummonFamiliar[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetBlockingDoor = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsDoorActionPossible[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_DoDoorAction[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFirstItemInInventory[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetNextItemInInventory[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetClassByPosition[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLevelByPosition[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLevelByClass[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetDamageDealtByType[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetTotalDamageDealt = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLastDamager[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastDisarmed = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastDisturbed = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastLocked = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastUnlocked = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectSkillIncrease[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetInventoryDisturbType = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetInventoryDisturbItem = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetHenchman[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_VersusAlignmentEffect[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_VersusRacialTypeEffect[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_VersusTrapEffect[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetGender[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsTalentValid[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionMoveAwayFromLocation[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetAttemptedAttackTarget = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTypeFromTalent[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIdFromTalent[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetAssociate[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_AddHenchman[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_RemoveHenchman[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_AddJournalQuestEntry[ 6 ] = { 4, 8, 12, 16, 20, 24 };
static const unsigned long NWN1_NWActionTotalParameterSizes_RemoveJournalQuestEntry[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetPCPublicCDKey[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetPCIPAddress[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetPCPlayerName[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetPCLike[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetPCDislike[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SendMessageToPC[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetAttemptedSpellTarget = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastOpenedBy = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetHasSpell[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_OpenStore[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectTurned = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFirstFactionMember[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetNextFactionMember[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionForceMoveToLocation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionForceMoveToObject[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetJournalQuestExperience[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_JumpToObject[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetMapPinEnabled[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectHitPointChangeWhenDying[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_PopUpGUIPanel[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ClearPersonalReputation[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetIsTemporaryFriend[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetIsTemporaryEnemy[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetIsTemporaryNeutral[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GiveXPToCreature[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetXP[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetXP[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_IntToHexString[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetBaseItemType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetItemHasItemProperty[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionEquipMostDamagingMelee[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionEquipMostDamagingRanged[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetItemACValue[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionRest[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ExploreAreaForPlayer[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_ActionEquipMostEffectiveArmor = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetIsDay = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetIsNight = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetIsDawn = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetIsDusk = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsEncounterCreature[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastPlayerDying = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetStartingLocation = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_ChangeToStandardFaction[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SoundObjectPlay[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SoundObjectStop[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SoundObjectSetVolume[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SoundObjectSetPosition[ 2 ] = { 4, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SpeakOneLinerConversation[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetGold[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastRespawnButtonPresser = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsDM[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_PlayVoiceChat[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsWeaponEffective[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastSpellHarmful = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_EventActivateItem[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_MusicBackgroundPlay[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_MusicBackgroundStop[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_MusicBackgroundSetDelay[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_MusicBackgroundChangeDay[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_MusicBackgroundChangeNight[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_MusicBattlePlay[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_MusicBattleStop[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_MusicBattleChange[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_AmbientSoundPlay[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_AmbientSoundStop[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_AmbientSoundChangeDay[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_AmbientSoundChangeNight[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastKiller = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetSpellCastItem = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetItemActivated = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetItemActivator = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetItemActivatedTargetLocation = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetItemActivatedTarget = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsOpen[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_TakeGoldFromCreature[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_IsInConversation[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectAbilityDecrease[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectAttackDecrease[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectDamageDecrease[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectDamageImmunityDecrease[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectACDecrease[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectMovementSpeedDecrease[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectSavingThrowDecrease[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectSkillDecrease[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectSpellResistanceDecrease[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetPlotFlag[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetPlotFlag[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectInvisibility[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectConcealment[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectDarkness = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectDispelMagicAll[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectUltravision = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectNegativeLevel[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectPolymorph[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectSanctuary[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectTrueSeeing = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectSeeInvisible = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectTimeStop = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectBlindness = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsReactionTypeFriendly[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsReactionTypeNeutral[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsReactionTypeHostile[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectSpellLevelAbsorption[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectDispelMagicBest[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActivatePortal[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetNumStackedItems[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_SurrenderToEnemies = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectMissChance[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTurnResistanceHD[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCreatureSize[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectDisappearAppear[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectDisappear[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectAppear[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionUnlockObject[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionLockObject[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectModifyAttacks[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLastTrapDetected[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectDamageShield[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetNearestTrapToObject[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetDeity[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetSubRace[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFortitudeSavingThrow[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetWillSavingThrow[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetReflexSavingThrow[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetChallengeRating[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetAge[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetMovementRate[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFamiliarCreatureType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetAnimalCompanionCreatureType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFamiliarName[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetAnimalCompanionName[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionCastFakeSpellAtObject[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionCastFakeSpellAtLocation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_RemoveSummonedAssociate[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetCameraMode[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsResting[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastPCRested = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_SetWeather[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastRestEventType = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_StartNewModule[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectSwarm[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetWeaponRanged[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_DoSinglePlayerAutoSave = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetGameDifficulty = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_SetTileMainLightColor[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetTileSourceLightColor[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_RecomputeStaticLighting[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTileMainLight1Color[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTileMainLight2Color[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTileSourceLight1Color[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTileSourceLight2Color[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetPanelButtonFlash[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCurrentAction[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetStandardFactionReputation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetStandardFactionReputation[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_FloatingTextStrRefOnCreature[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_FloatingTextStringOnCreature[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTrapDisarmable[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTrapDetectable[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTrapDetectedBy[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTrapFlagged[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTrapBaseType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTrapOneShot[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTrapCreator[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTrapKeyTag[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTrapDisarmDC[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTrapDetectDC[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLockKeyRequired[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLockKeyTag[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLockLockable[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLockUnlockDC[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLockLockDC[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetPCLevellingUp = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetHasFeatEffect[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetPlaceableIllumination[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetPlaceableIllumination[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsPlaceableObjectActionPossible[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_DoPlaceableObjectAction[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetFirstPC = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetNextPC = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_SetTrapDetectedBy[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsTrapped[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectTurnResistanceDecrease[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectTurnResistanceIncrease[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_PopUpDeathGUIPanel[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetTrapDisabled[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLastHostileActor[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_ExportAllCharacters = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_MusicBackgroundGetDayTrack[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_MusicBackgroundGetNightTrack[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_WriteTimestampedLogEntry[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetModuleName = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFactionLeader[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SendMessageToAllDMs[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EndGame[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_BootPC[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionCounterSpell[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_AmbientSoundSetDayVolume[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_AmbientSoundSetNightVolume[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_MusicBackgroundGetBattleTrack[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetHasInventory[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetStrRefSoundDuration[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_AddToParty[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_RemoveFromParty[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetStealthMode[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetDetectMode[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetDefensiveCastingMode[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetAppearanceType[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_SpawnScriptDebugger = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetModuleItemAcquiredStackSize = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_DecrementRemainingFeatUses[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_DecrementRemainingSpellUses[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetResRef[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectPetrify = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_CopyItem[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectCutsceneParalyze = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetDroppableFlag[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetUseableFlag[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetStolenFlag[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetCampaignFloat[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetCampaignInt[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetCampaignVector[ 4 ] = { 4, 8, 20, 24 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetCampaignLocation[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetCampaignString[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_DestroyCampaignDatabase[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCampaignFloat[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCampaignInt[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCampaignVector[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCampaignLocation[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCampaignString[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_CopyObject[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_DeleteCampaignVariable[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_StoreCampaignObject[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_RetrieveCampaignObject[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectCutsceneDominated = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetItemStackSize[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetItemStackSize[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetItemCharges[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetItemCharges[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_AddItemProperty[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_RemoveItemProperty[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsItemPropertyValid[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFirstItemProperty[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetNextItemProperty[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetItemPropertyType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetItemPropertyDurationType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyAbilityBonus[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyACBonus[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyACBonusVsAlign[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyACBonusVsDmgType[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyACBonusVsRace[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyACBonusVsSAlign[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyEnhancementBonus[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyEnhancementBonusVsAlign[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyEnhancementBonusVsRace[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyEnhancementBonusVsSAlign[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyEnhancementPenalty[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyWeightReduction[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyBonusFeat[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyBonusLevelSpell[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyCastSpell[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyDamageBonus[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyDamageBonusVsAlign[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyDamageBonusVsRace[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyDamageBonusVsSAlign[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyDamageImmunity[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyDamagePenalty[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyDamageReduction[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyDamageResistance[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyDamageVulnerability[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_ItemPropertyDarkvision = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyDecreaseAbility[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyDecreaseAC[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyDecreaseSkill[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyContainerReducedWeight[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyExtraMeleeDamageType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyExtraRangeDamageType[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_ItemPropertyHaste = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_ItemPropertyHolyAvenger = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyImmunityMisc[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_ItemPropertyImprovedEvasion = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyBonusSpellResistance[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyBonusSavingThrowVsX[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyBonusSavingThrow[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_ItemPropertyKeen = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyLight[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyMaxRangeStrengthMod[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_ItemPropertyNoDamage = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyOnHitProps[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyReducedSavingThrowVsX[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyReducedSavingThrow[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyRegeneration[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertySkillBonus[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertySpellImmunitySpecific[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertySpellImmunitySchool[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyThievesTools[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyAttackBonus[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyAttackBonusVsAlign[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyAttackBonusVsRace[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyAttackBonusVsSAlign[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyAttackPenalty[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyUnlimitedAmmo[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyLimitUseByAlign[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyLimitUseByClass[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyLimitUseByRace[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyLimitUseBySAlign[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_BadBadReplaceMeThisDoesNothing = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyVampiricRegeneration[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyTrap[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_ItemPropertyTrueSeeing = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyOnMonsterHitProperties[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyTurnResistance[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyMassiveCritical[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_ItemPropertyFreeAction = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyMonsterDamage[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyImmunityToSpellLevel[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertySpecialWalk[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyHealersKit[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyWeightIncrease[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsSkillSuccessful[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_EffectSpellFailure[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SpeakStringByStrRef[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetCutsceneMode[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastPCToCancelCutscene = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetDialogSoundLength[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_FadeFromBlack[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_FadeToBlack[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_StopFade[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_BlackScreen[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetBaseAttackBonus[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetImmortal[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_OpenInventory[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_StoreCameraFacing = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_RestoreCameraFacing = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_LevelUpHenchman[ 4 ] = { 4, 8, 12, 16 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetDroppableFlag[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetWeight[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetModuleItemAcquiredBy = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetImmortal[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_DoWhirlwindAttack[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_Get2DAString[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectEthereal = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetAILevel[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetAILevel[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsPossessedFamiliar[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_UnpossessFamiliar[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsAreaInterior[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SendMessageToPCByStrRef[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_IncrementRemainingFeatUses[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ExportSingleCharacter[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_PlaySoundByStrRef[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetSubRace[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetDeity[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsDMPossessed[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetWeather[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsAreaNatural[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsAreaAboveGround[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetPCItemLastEquipped = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetPCItemLastEquippedBy = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetPCItemLastUnequipped = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetPCItemLastUnequippedBy = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_CopyItemAndModify[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetItemAppearance[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyOnHitCastSpell[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetItemPropertySubType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetActionMode[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetActionMode[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetArcaneSpellFailure[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ActionExamine[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyVisualEffect[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetLootable[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetLootable[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCutsceneCameraMoveRate[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetCutsceneCameraMoveRate[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetItemCursedFlag[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetItemCursedFlag[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetMaxHenchmen[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetMaxHenchmen = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetAssociateType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetSpellResistance[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_DayToNight[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_NightToDay[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_LineOfSightObject[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_LineOfSightVector[ 2 ] = { 12, 24 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetLastSpellCastClass = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_SetBaseAttackBonus[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_RestoreBaseAttackBonus[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectCutsceneGhost = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyArcaneSpellFailure[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetStoreGold[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetStoreGold[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetStoreMaxBuyPrice[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetStoreMaxBuyPrice[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetStoreIdentifyCost[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetStoreIdentifyCost[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetCreatureAppearanceType[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCreatureStartingPackage[ 1 ] = { 4 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_EffectCutsceneImmobilize = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsInSubArea[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetItemPropertyCostTable[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetItemPropertyCostTableValue[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetItemPropertyParam1[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetItemPropertyParam1Value[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetIsCreatureDisarmable[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetStolenFlag[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ForceRest[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetCameraHeight[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetSkyBox[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetPhenoType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetPhenoType[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetFogColor[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCutsceneMode[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetSkyBox[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFogColor[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetFogAmount[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFogAmount[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetPickpocketableFlag[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetPickpocketableFlag[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetFootstepType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetFootstepType[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCreatureWingType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetCreatureWingType[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCreatureBodyPart[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetCreatureBodyPart[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetCreatureTailType[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetCreatureTailType[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetHardness[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetHardness[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetLockKeyRequired[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetLockKeyTag[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetLockLockable[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetLockUnlockDC[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetLockLockDC[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetTrapDisarmable[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetTrapDetectable[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetTrapOneShot[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetTrapKeyTag[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetTrapDisarmDC[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetTrapDetectDC[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_CreateTrapAtLocation[ 7 ] = { 4, 8, 12, 16, 20, 24, 28 };
static const unsigned long NWN1_NWActionTotalParameterSizes_CreateTrapOnObject[ 5 ] = { 4, 8, 12, 16, 20 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetWillSavingThrow[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetReflexSavingThrow[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetFortitudeSavingThrow[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTilesetResRef[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTrapRecoverable[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetTrapRecoverable[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetModuleXPScale = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_SetModuleXPScale[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetKeyRequiredFeedback[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetKeyRequiredFeedback[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetTrapActive[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetTrapActive[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_LockCameraPitch[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_LockCameraDistance[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_LockCameraDirection[ 2 ] = { 4, 8 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetPlaceableLastClickedBy = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_GetInfiniteFlag[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetInfiniteFlag[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetAreaSize[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetName[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetPortraitId[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetPortraitId[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetPortraitResRef[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetPortraitResRef[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetUseableFlag[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetDescription[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetDescription[ 3 ] = { 4, 8, 12 };
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetPCChatSpeaker = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetPCChatMessage = NULL;
static const unsigned long * NWN1_NWActionTotalParameterSizes_GetPCChatVolume = NULL;
static const unsigned long NWN1_NWActionTotalParameterSizes_SetPCChatMessage[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetPCChatVolume[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_GetColor[ 2 ] = { 4, 8 };
static const unsigned long NWN1_NWActionTotalParameterSizes_SetColor[ 3 ] = { 4, 8, 12 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyMaterial[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyQuality[ 1 ] = { 4 };
static const unsigned long NWN1_NWActionTotalParameterSizes_ItemPropertyAdditional[ 1 ] = { 4 };



const NWACTION_DEFINITION NWActions_NWN1[ 848 ] =
{
	{ NWSCRIPT_ACTIONNAME("Random") NWSCRIPT_ACTIONPROTOTYPE("int Random(int nMaxInteger);") 0, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_Random NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_Random) },
	{ NWSCRIPT_ACTIONNAME("PrintString") NWSCRIPT_ACTIONPROTOTYPE("void PrintString(string sString);") 1, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_PrintString NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_PrintString) },
	{ NWSCRIPT_ACTIONNAME("PrintFloat") NWSCRIPT_ACTIONPROTOTYPE("void PrintFloat(float fFloat, int nWidth=18, int nDecimals=9);") 2, 1, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_PrintFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_PrintFloat) },
	{ NWSCRIPT_ACTIONNAME("FloatToString") NWSCRIPT_ACTIONPROTOTYPE("string FloatToString(float fFloat, int nWidth=18, int nDecimals=9);") 3, 1, 3, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_FloatToString NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_FloatToString) },
	{ NWSCRIPT_ACTIONNAME("PrintInteger") NWSCRIPT_ACTIONPROTOTYPE("void PrintInteger(int nInteger);") 4, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_PrintInteger NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_PrintInteger) },
	{ NWSCRIPT_ACTIONNAME("PrintObject") NWSCRIPT_ACTIONPROTOTYPE("void PrintObject(object oObject);") 5, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_PrintObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_PrintObject) },
	{ NWSCRIPT_ACTIONNAME("AssignCommand") NWSCRIPT_ACTIONPROTOTYPE("void AssignCommand(object oActionSubject,action aActionToAssign);") 6, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_AssignCommand NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_AssignCommand) },
	{ NWSCRIPT_ACTIONNAME("DelayCommand") NWSCRIPT_ACTIONPROTOTYPE("void DelayCommand(float fSeconds, action aActionToDelay);") 7, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_DelayCommand NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_DelayCommand) },
	{ NWSCRIPT_ACTIONNAME("ExecuteScript") NWSCRIPT_ACTIONPROTOTYPE("void ExecuteScript(string sScript, object oTarget);") 8, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ExecuteScript NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ExecuteScript) },
	{ NWSCRIPT_ACTIONNAME("ClearAllActions") NWSCRIPT_ACTIONPROTOTYPE("void ClearAllActions(int nClearCombatState=FALSE);") 9, 0, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ClearAllActions NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ClearAllActions) },
	{ NWSCRIPT_ACTIONNAME("SetFacing") NWSCRIPT_ACTIONPROTOTYPE("void SetFacing(float fDirection);") 10, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetFacing NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetFacing) },
	{ NWSCRIPT_ACTIONNAME("SetCalendar") NWSCRIPT_ACTIONPROTOTYPE("void SetCalendar(int nYear,int nMonth, int nDay);") 11, 3, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetCalendar NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetCalendar) },
	{ NWSCRIPT_ACTIONNAME("SetTime") NWSCRIPT_ACTIONPROTOTYPE("void SetTime(int nHour,int nMinute,int nSecond,int nMillisecond);") 12, 4, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetTime NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetTime) },
	{ NWSCRIPT_ACTIONNAME("GetCalendarYear") NWSCRIPT_ACTIONPROTOTYPE("int GetCalendarYear();") 13, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetCalendarYear NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCalendarYear) },
	{ NWSCRIPT_ACTIONNAME("GetCalendarMonth") NWSCRIPT_ACTIONPROTOTYPE("int GetCalendarMonth();") 14, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetCalendarMonth NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCalendarMonth) },
	{ NWSCRIPT_ACTIONNAME("GetCalendarDay") NWSCRIPT_ACTIONPROTOTYPE("int GetCalendarDay();") 15, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetCalendarDay NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCalendarDay) },
	{ NWSCRIPT_ACTIONNAME("GetTimeHour") NWSCRIPT_ACTIONPROTOTYPE("int GetTimeHour();") 16, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTimeHour NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTimeHour) },
	{ NWSCRIPT_ACTIONNAME("GetTimeMinute") NWSCRIPT_ACTIONPROTOTYPE("int GetTimeMinute();") 17, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTimeMinute NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTimeMinute) },
	{ NWSCRIPT_ACTIONNAME("GetTimeSecond") NWSCRIPT_ACTIONPROTOTYPE("int GetTimeSecond();") 18, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTimeSecond NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTimeSecond) },
	{ NWSCRIPT_ACTIONNAME("GetTimeMillisecond") NWSCRIPT_ACTIONPROTOTYPE("int GetTimeMillisecond();") 19, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTimeMillisecond NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTimeMillisecond) },
	{ NWSCRIPT_ACTIONNAME("ActionRandomWalk") NWSCRIPT_ACTIONPROTOTYPE("void ActionRandomWalk();") 20, 0, 0, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionRandomWalk NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionRandomWalk) },
	{ NWSCRIPT_ACTIONNAME("ActionMoveToLocation") NWSCRIPT_ACTIONPROTOTYPE("void ActionMoveToLocation(location lDestination, int bRun=FALSE);") 21, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionMoveToLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionMoveToLocation) },
	{ NWSCRIPT_ACTIONNAME("ActionMoveToObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionMoveToObject(object oMoveTo, int bRun=FALSE, float fRange=1.0f);") 22, 1, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionMoveToObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionMoveToObject) },
	{ NWSCRIPT_ACTIONNAME("ActionMoveAwayFromObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionMoveAwayFromObject(object oFleeFrom, int bRun=FALSE, float fMoveAwayRange=40.0f);") 23, 1, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionMoveAwayFromObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionMoveAwayFromObject) },
	{ NWSCRIPT_ACTIONNAME("GetArea") NWSCRIPT_ACTIONPROTOTYPE("object GetArea(object oTarget);") 24, 1, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetArea NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetArea) },
	{ NWSCRIPT_ACTIONNAME("GetEnteringObject") NWSCRIPT_ACTIONPROTOTYPE("object GetEnteringObject();") 25, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetEnteringObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetEnteringObject) },
	{ NWSCRIPT_ACTIONNAME("GetExitingObject") NWSCRIPT_ACTIONPROTOTYPE("object GetExitingObject();") 26, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetExitingObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetExitingObject) },
	{ NWSCRIPT_ACTIONNAME("GetPosition") NWSCRIPT_ACTIONPROTOTYPE("vector GetPosition(object oTarget);") 27, 1, 1, ACTIONTYPE_VECTOR, NWN1_NWActionParameterTypes_GetPosition NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPosition) },
	{ NWSCRIPT_ACTIONNAME("GetFacing") NWSCRIPT_ACTIONPROTOTYPE("float GetFacing(object oTarget);") 28, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_GetFacing NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFacing) },
	{ NWSCRIPT_ACTIONNAME("GetItemPossessor") NWSCRIPT_ACTIONPROTOTYPE("object GetItemPossessor(object oItem);") 29, 1, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetItemPossessor NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemPossessor) },
	{ NWSCRIPT_ACTIONNAME("GetItemPossessedBy") NWSCRIPT_ACTIONPROTOTYPE("object GetItemPossessedBy(object oCreature, string sItemTag);") 30, 2, 2, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetItemPossessedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemPossessedBy) },
	{ NWSCRIPT_ACTIONNAME("CreateItemOnObject") NWSCRIPT_ACTIONPROTOTYPE("object CreateItemOnObject(string sItemTemplate, object oTarget=OBJECT_SELF, int nStackSize=1, string sNewTag="");") 31, 1, 4, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_CreateItemOnObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_CreateItemOnObject) },
	{ NWSCRIPT_ACTIONNAME("ActionEquipItem") NWSCRIPT_ACTIONPROTOTYPE("void ActionEquipItem(object oItem, int nInventorySlot);") 32, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionEquipItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionEquipItem) },
	{ NWSCRIPT_ACTIONNAME("ActionUnequipItem") NWSCRIPT_ACTIONPROTOTYPE("void ActionUnequipItem(object oItem);") 33, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionUnequipItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionUnequipItem) },
	{ NWSCRIPT_ACTIONNAME("ActionPickUpItem") NWSCRIPT_ACTIONPROTOTYPE("void ActionPickUpItem(object oItem);") 34, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionPickUpItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionPickUpItem) },
	{ NWSCRIPT_ACTIONNAME("ActionPutDownItem") NWSCRIPT_ACTIONPROTOTYPE("void ActionPutDownItem(object oItem);") 35, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionPutDownItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionPutDownItem) },
	{ NWSCRIPT_ACTIONNAME("GetLastAttacker") NWSCRIPT_ACTIONPROTOTYPE("object GetLastAttacker(object oAttackee=OBJECT_SELF);") 36, 0, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastAttacker NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastAttacker) },
	{ NWSCRIPT_ACTIONNAME("ActionAttack") NWSCRIPT_ACTIONPROTOTYPE("void ActionAttack(object oAttackee, int bPassive=FALSE);") 37, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionAttack NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionAttack) },
	{ NWSCRIPT_ACTIONNAME("GetNearestCreature") NWSCRIPT_ACTIONPROTOTYPE("object GetNearestCreature(int nFirstCriteriaType, int nFirstCriteriaValue, object oTarget=OBJECT_SELF, int nNth=1, int nSecondCriteriaType=-1, int nSecondCriteriaValue=-1, int nThirdCriteriaType=-1,  int nThirdCriteriaValue=-1 );") 38, 2, 8, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetNearestCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetNearestCreature) },
	{ NWSCRIPT_ACTIONNAME("ActionSpeakString") NWSCRIPT_ACTIONPROTOTYPE("void ActionSpeakString(string sStringToSpeak, int nTalkVolume=TALKVOLUME_TALK);") 39, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionSpeakString NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionSpeakString) },
	{ NWSCRIPT_ACTIONNAME("ActionPlayAnimation") NWSCRIPT_ACTIONPROTOTYPE("void ActionPlayAnimation(int nAnimation, float fSpeed=1.0, float fDurationSeconds=0.0);") 40, 1, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionPlayAnimation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionPlayAnimation) },
	{ NWSCRIPT_ACTIONNAME("GetDistanceToObject") NWSCRIPT_ACTIONPROTOTYPE("float GetDistanceToObject(object oObject);") 41, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_GetDistanceToObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetDistanceToObject) },
	{ NWSCRIPT_ACTIONNAME("GetIsObjectValid") NWSCRIPT_ACTIONPROTOTYPE("int GetIsObjectValid(object oObject);") 42, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsObjectValid NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsObjectValid) },
	{ NWSCRIPT_ACTIONNAME("ActionOpenDoor") NWSCRIPT_ACTIONPROTOTYPE("void ActionOpenDoor(object oDoor);") 43, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionOpenDoor NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionOpenDoor) },
	{ NWSCRIPT_ACTIONNAME("ActionCloseDoor") NWSCRIPT_ACTIONPROTOTYPE("void ActionCloseDoor(object oDoor);") 44, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionCloseDoor NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionCloseDoor) },
	{ NWSCRIPT_ACTIONNAME("SetCameraFacing") NWSCRIPT_ACTIONPROTOTYPE("void SetCameraFacing(float fDirection, float fDistance = -1.0f, float fPitch = -1.0, int nTransitionType=CAMERA_TRANSITION_TYPE_SNAP);") 45, 1, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetCameraFacing NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetCameraFacing) },
	{ NWSCRIPT_ACTIONNAME("PlaySound") NWSCRIPT_ACTIONPROTOTYPE("void PlaySound(string sSoundName);") 46, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_PlaySound NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_PlaySound) },
	{ NWSCRIPT_ACTIONNAME("GetSpellTargetObject") NWSCRIPT_ACTIONPROTOTYPE("object GetSpellTargetObject();") 47, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetSpellTargetObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetSpellTargetObject) },
	{ NWSCRIPT_ACTIONNAME("ActionCastSpellAtObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionCastSpellAtObject(int nSpell, object oTarget, int nMetaMagic=METAMAGIC_ANY, int bCheat=FALSE, int nDomainLevel=0, int nProjectilePathType=PROJECTILE_PATH_TYPE_DEFAULT, int bInstantSpell=FALSE);") 48, 2, 7, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionCastSpellAtObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionCastSpellAtObject) },
	{ NWSCRIPT_ACTIONNAME("GetCurrentHitPoints") NWSCRIPT_ACTIONPROTOTYPE("int GetCurrentHitPoints(object oObject=OBJECT_SELF);") 49, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetCurrentHitPoints NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCurrentHitPoints) },
	{ NWSCRIPT_ACTIONNAME("GetMaxHitPoints") NWSCRIPT_ACTIONPROTOTYPE("int GetMaxHitPoints(object oObject=OBJECT_SELF);") 50, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetMaxHitPoints NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetMaxHitPoints) },
	{ NWSCRIPT_ACTIONNAME("GetLocalInt") NWSCRIPT_ACTIONPROTOTYPE("int GetLocalInt(object oObject, string sVarName);") 51, 2, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLocalInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLocalInt) },
	{ NWSCRIPT_ACTIONNAME("GetLocalFloat") NWSCRIPT_ACTIONPROTOTYPE("float GetLocalFloat(object oObject, string sVarName);") 52, 2, 2, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_GetLocalFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLocalFloat) },
	{ NWSCRIPT_ACTIONNAME("GetLocalString") NWSCRIPT_ACTIONPROTOTYPE("string GetLocalString(object oObject, string sVarName);") 53, 2, 2, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetLocalString NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLocalString) },
	{ NWSCRIPT_ACTIONNAME("GetLocalObject") NWSCRIPT_ACTIONPROTOTYPE("object GetLocalObject(object oObject, string sVarName);") 54, 2, 2, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLocalObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLocalObject) },
	{ NWSCRIPT_ACTIONNAME("SetLocalInt") NWSCRIPT_ACTIONPROTOTYPE("void SetLocalInt(object oObject, string sVarName, int nValue);") 55, 3, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetLocalInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetLocalInt) },
	{ NWSCRIPT_ACTIONNAME("SetLocalFloat") NWSCRIPT_ACTIONPROTOTYPE("void SetLocalFloat(object oObject, string sVarName, float fValue);") 56, 3, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetLocalFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetLocalFloat) },
	{ NWSCRIPT_ACTIONNAME("SetLocalString") NWSCRIPT_ACTIONPROTOTYPE("void SetLocalString(object oObject, string sVarName, string sValue);") 57, 3, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetLocalString NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetLocalString) },
	{ NWSCRIPT_ACTIONNAME("SetLocalObject") NWSCRIPT_ACTIONPROTOTYPE("void SetLocalObject(object oObject, string sVarName, object oValue);") 58, 3, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetLocalObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetLocalObject) },
	{ NWSCRIPT_ACTIONNAME("GetStringLength") NWSCRIPT_ACTIONPROTOTYPE("int GetStringLength(string sString);") 59, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetStringLength NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetStringLength) },
	{ NWSCRIPT_ACTIONNAME("GetStringUpperCase") NWSCRIPT_ACTIONPROTOTYPE("string GetStringUpperCase(string sString);") 60, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetStringUpperCase NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetStringUpperCase) },
	{ NWSCRIPT_ACTIONNAME("GetStringLowerCase") NWSCRIPT_ACTIONPROTOTYPE("string GetStringLowerCase(string sString);") 61, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetStringLowerCase NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetStringLowerCase) },
	{ NWSCRIPT_ACTIONNAME("GetStringRight") NWSCRIPT_ACTIONPROTOTYPE("string GetStringRight(string sString, int nCount);") 62, 2, 2, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetStringRight NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetStringRight) },
	{ NWSCRIPT_ACTIONNAME("GetStringLeft") NWSCRIPT_ACTIONPROTOTYPE("string GetStringLeft(string sString, int nCount);") 63, 2, 2, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetStringLeft NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetStringLeft) },
	{ NWSCRIPT_ACTIONNAME("InsertString") NWSCRIPT_ACTIONPROTOTYPE("string InsertString(string sDestination, string sString, int nPosition);") 64, 3, 3, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_InsertString NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_InsertString) },
	{ NWSCRIPT_ACTIONNAME("GetSubString") NWSCRIPT_ACTIONPROTOTYPE("string GetSubString(string sString, int nStart, int nCount);") 65, 3, 3, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetSubString NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetSubString) },
	{ NWSCRIPT_ACTIONNAME("FindSubString") NWSCRIPT_ACTIONPROTOTYPE("int FindSubString(string sString, string sSubString, int nStart=0);") 66, 2, 3, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_FindSubString NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_FindSubString) },
	{ NWSCRIPT_ACTIONNAME("fabs") NWSCRIPT_ACTIONPROTOTYPE("float fabs(float fValue);") 67, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_fabs NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_fabs) },
	{ NWSCRIPT_ACTIONNAME("cos") NWSCRIPT_ACTIONPROTOTYPE("float cos(float fValue);") 68, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_cos NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_cos) },
	{ NWSCRIPT_ACTIONNAME("sin") NWSCRIPT_ACTIONPROTOTYPE("float sin(float fValue);") 69, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_sin NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_sin) },
	{ NWSCRIPT_ACTIONNAME("tan") NWSCRIPT_ACTIONPROTOTYPE("float tan(float fValue);") 70, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_tan NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_tan) },
	{ NWSCRIPT_ACTIONNAME("acos") NWSCRIPT_ACTIONPROTOTYPE("float acos(float fValue);") 71, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_acos NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_acos) },
	{ NWSCRIPT_ACTIONNAME("asin") NWSCRIPT_ACTIONPROTOTYPE("float asin(float fValue);") 72, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_asin NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_asin) },
	{ NWSCRIPT_ACTIONNAME("atan") NWSCRIPT_ACTIONPROTOTYPE("float atan(float fValue);") 73, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_atan NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_atan) },
	{ NWSCRIPT_ACTIONNAME("log") NWSCRIPT_ACTIONPROTOTYPE("float log(float fValue);") 74, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_log NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_log) },
	{ NWSCRIPT_ACTIONNAME("pow") NWSCRIPT_ACTIONPROTOTYPE("float pow(float fValue, float fExponent);") 75, 2, 2, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_pow NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_pow) },
	{ NWSCRIPT_ACTIONNAME("sqrt") NWSCRIPT_ACTIONPROTOTYPE("float sqrt(float fValue);") 76, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_sqrt NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_sqrt) },
	{ NWSCRIPT_ACTIONNAME("abs") NWSCRIPT_ACTIONPROTOTYPE("int abs(int nValue);") 77, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_abs NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_abs) },
	{ NWSCRIPT_ACTIONNAME("EffectHeal") NWSCRIPT_ACTIONPROTOTYPE("effect EffectHeal(int nDamageToHeal);") 78, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectHeal NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectHeal) },
	{ NWSCRIPT_ACTIONNAME("EffectDamage") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamage(int nDamageAmount, int nDamageType=DAMAGE_TYPE_MAGICAL, int nDamagePower=DAMAGE_POWER_NORMAL);") 79, 1, 3, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDamage NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDamage) },
	{ NWSCRIPT_ACTIONNAME("EffectAbilityIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectAbilityIncrease(int nAbilityToIncrease, int nModifyBy);") 80, 2, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectAbilityIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectAbilityIncrease) },
	{ NWSCRIPT_ACTIONNAME("EffectDamageResistance") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamageResistance(int nDamageType, int nAmount, int nLimit=0);") 81, 2, 3, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDamageResistance NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDamageResistance) },
	{ NWSCRIPT_ACTIONNAME("EffectResurrection") NWSCRIPT_ACTIONPROTOTYPE("effect EffectResurrection();") 82, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectResurrection NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectResurrection) },
	{ NWSCRIPT_ACTIONNAME("EffectSummonCreature") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSummonCreature(string sCreatureResref, int nVisualEffectId=VFX_NONE, float fDelaySeconds=0.0f, int nUseAppearAnimation=0);") 83, 1, 4, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectSummonCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectSummonCreature) },
	{ NWSCRIPT_ACTIONNAME("GetCasterLevel") NWSCRIPT_ACTIONPROTOTYPE("int GetCasterLevel(object oCreature);") 84, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetCasterLevel NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCasterLevel) },
	{ NWSCRIPT_ACTIONNAME("GetFirstEffect") NWSCRIPT_ACTIONPROTOTYPE("effect GetFirstEffect(object oCreature);") 85, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_GetFirstEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFirstEffect) },
	{ NWSCRIPT_ACTIONNAME("GetNextEffect") NWSCRIPT_ACTIONPROTOTYPE("effect GetNextEffect(object oCreature);") 86, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_GetNextEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetNextEffect) },
	{ NWSCRIPT_ACTIONNAME("RemoveEffect") NWSCRIPT_ACTIONPROTOTYPE("void RemoveEffect(object oCreature, effect eEffect);") 87, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_RemoveEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_RemoveEffect) },
	{ NWSCRIPT_ACTIONNAME("GetIsEffectValid") NWSCRIPT_ACTIONPROTOTYPE("int GetIsEffectValid(effect eEffect);") 88, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsEffectValid NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsEffectValid) },
	{ NWSCRIPT_ACTIONNAME("GetEffectDurationType") NWSCRIPT_ACTIONPROTOTYPE("int GetEffectDurationType(effect eEffect);") 89, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetEffectDurationType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetEffectDurationType) },
	{ NWSCRIPT_ACTIONNAME("GetEffectSubType") NWSCRIPT_ACTIONPROTOTYPE("int GetEffectSubType(effect eEffect);") 90, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetEffectSubType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetEffectSubType) },
	{ NWSCRIPT_ACTIONNAME("GetEffectCreator") NWSCRIPT_ACTIONPROTOTYPE("object GetEffectCreator(effect eEffect);") 91, 1, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetEffectCreator NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetEffectCreator) },
	{ NWSCRIPT_ACTIONNAME("IntToString") NWSCRIPT_ACTIONPROTOTYPE("string IntToString(int nInteger);") 92, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_IntToString NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_IntToString) },
	{ NWSCRIPT_ACTIONNAME("GetFirstObjectInArea") NWSCRIPT_ACTIONPROTOTYPE("object GetFirstObjectInArea(object oArea=OBJECT_INVALID);") 93, 0, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetFirstObjectInArea NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFirstObjectInArea) },
	{ NWSCRIPT_ACTIONNAME("GetNextObjectInArea") NWSCRIPT_ACTIONPROTOTYPE("object GetNextObjectInArea(object oArea=OBJECT_INVALID);") 94, 0, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetNextObjectInArea NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetNextObjectInArea) },
	{ NWSCRIPT_ACTIONNAME("d2") NWSCRIPT_ACTIONPROTOTYPE("int d2(int nNumDice=1);") 95, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_d2 NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_d2) },
	{ NWSCRIPT_ACTIONNAME("d3") NWSCRIPT_ACTIONPROTOTYPE("int d3(int nNumDice=1);") 96, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_d3 NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_d3) },
	{ NWSCRIPT_ACTIONNAME("d4") NWSCRIPT_ACTIONPROTOTYPE("int d4(int nNumDice=1);") 97, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_d4 NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_d4) },
	{ NWSCRIPT_ACTIONNAME("d6") NWSCRIPT_ACTIONPROTOTYPE("int d6(int nNumDice=1);") 98, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_d6 NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_d6) },
	{ NWSCRIPT_ACTIONNAME("d8") NWSCRIPT_ACTIONPROTOTYPE("int d8(int nNumDice=1);") 99, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_d8 NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_d8) },
	{ NWSCRIPT_ACTIONNAME("d10") NWSCRIPT_ACTIONPROTOTYPE("int d10(int nNumDice=1);") 100, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_d10 NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_d10) },
	{ NWSCRIPT_ACTIONNAME("d12") NWSCRIPT_ACTIONPROTOTYPE("int d12(int nNumDice=1);") 101, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_d12 NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_d12) },
	{ NWSCRIPT_ACTIONNAME("d20") NWSCRIPT_ACTIONPROTOTYPE("int d20(int nNumDice=1);") 102, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_d20 NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_d20) },
	{ NWSCRIPT_ACTIONNAME("d100") NWSCRIPT_ACTIONPROTOTYPE("int d100(int nNumDice=1);") 103, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_d100 NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_d100) },
	{ NWSCRIPT_ACTIONNAME("VectorMagnitude") NWSCRIPT_ACTIONPROTOTYPE("float VectorMagnitude(vector vVector);") 104, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_VectorMagnitude NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_VectorMagnitude) },
	{ NWSCRIPT_ACTIONNAME("GetMetaMagicFeat") NWSCRIPT_ACTIONPROTOTYPE("int GetMetaMagicFeat();") 105, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetMetaMagicFeat NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetMetaMagicFeat) },
	{ NWSCRIPT_ACTIONNAME("GetObjectType") NWSCRIPT_ACTIONPROTOTYPE("int GetObjectType(object oTarget);") 106, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetObjectType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetObjectType) },
	{ NWSCRIPT_ACTIONNAME("GetRacialType") NWSCRIPT_ACTIONPROTOTYPE("int GetRacialType(object oCreature);") 107, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetRacialType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetRacialType) },
	{ NWSCRIPT_ACTIONNAME("FortitudeSave") NWSCRIPT_ACTIONPROTOTYPE("int FortitudeSave(object oCreature, int nDC, int nSaveType=SAVING_THROW_TYPE_NONE, object oSaveVersus=OBJECT_SELF);") 108, 2, 4, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_FortitudeSave NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_FortitudeSave) },
	{ NWSCRIPT_ACTIONNAME("ReflexSave") NWSCRIPT_ACTIONPROTOTYPE("int ReflexSave(object oCreature, int nDC, int nSaveType=SAVING_THROW_TYPE_NONE, object oSaveVersus=OBJECT_SELF);") 109, 2, 4, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_ReflexSave NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ReflexSave) },
	{ NWSCRIPT_ACTIONNAME("WillSave") NWSCRIPT_ACTIONPROTOTYPE("int WillSave(object oCreature, int nDC, int nSaveType=SAVING_THROW_TYPE_NONE, object oSaveVersus=OBJECT_SELF);") 110, 2, 4, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_WillSave NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_WillSave) },
	{ NWSCRIPT_ACTIONNAME("GetSpellSaveDC") NWSCRIPT_ACTIONPROTOTYPE("int GetSpellSaveDC();") 111, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetSpellSaveDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetSpellSaveDC) },
	{ NWSCRIPT_ACTIONNAME("MagicalEffect") NWSCRIPT_ACTIONPROTOTYPE("effect MagicalEffect(effect eEffect);") 112, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_MagicalEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_MagicalEffect) },
	{ NWSCRIPT_ACTIONNAME("SupernaturalEffect") NWSCRIPT_ACTIONPROTOTYPE("effect SupernaturalEffect(effect eEffect);") 113, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_SupernaturalEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SupernaturalEffect) },
	{ NWSCRIPT_ACTIONNAME("ExtraordinaryEffect") NWSCRIPT_ACTIONPROTOTYPE("effect ExtraordinaryEffect(effect eEffect);") 114, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_ExtraordinaryEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ExtraordinaryEffect) },
	{ NWSCRIPT_ACTIONNAME("EffectACIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectACIncrease(int nValue, int nModifyType=AC_DODGE_BONUS, int nDamageType=AC_VS_DAMAGE_TYPE_ALL);") 115, 1, 3, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectACIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectACIncrease) },
	{ NWSCRIPT_ACTIONNAME("GetAC") NWSCRIPT_ACTIONPROTOTYPE("int GetAC(object oObject, int nForFutureUse=0);") 116, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetAC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAC) },
	{ NWSCRIPT_ACTIONNAME("EffectSavingThrowIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSavingThrowIncrease(int nSave, int nValue, int nSaveType=SAVING_THROW_TYPE_ALL);") 117, 2, 3, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectSavingThrowIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectSavingThrowIncrease) },
	{ NWSCRIPT_ACTIONNAME("EffectAttackIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectAttackIncrease(int nBonus, int nModifierType=ATTACK_BONUS_MISC);") 118, 1, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectAttackIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectAttackIncrease) },
	{ NWSCRIPT_ACTIONNAME("EffectDamageReduction") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamageReduction(int nAmount, int nDamagePower, int nLimit=0);") 119, 2, 3, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDamageReduction NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDamageReduction) },
	{ NWSCRIPT_ACTIONNAME("EffectDamageIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamageIncrease(int nBonus, int nDamageType=DAMAGE_TYPE_MAGICAL);") 120, 1, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDamageIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDamageIncrease) },
	{ NWSCRIPT_ACTIONNAME("RoundsToSeconds") NWSCRIPT_ACTIONPROTOTYPE("float RoundsToSeconds(int nRounds);") 121, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_RoundsToSeconds NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_RoundsToSeconds) },
	{ NWSCRIPT_ACTIONNAME("HoursToSeconds") NWSCRIPT_ACTIONPROTOTYPE("float HoursToSeconds(int nHours);") 122, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_HoursToSeconds NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_HoursToSeconds) },
	{ NWSCRIPT_ACTIONNAME("TurnsToSeconds") NWSCRIPT_ACTIONPROTOTYPE("float TurnsToSeconds(int nTurns);") 123, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_TurnsToSeconds NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_TurnsToSeconds) },
	{ NWSCRIPT_ACTIONNAME("GetLawChaosValue") NWSCRIPT_ACTIONPROTOTYPE("int GetLawChaosValue(object oCreature);") 124, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLawChaosValue NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLawChaosValue) },
	{ NWSCRIPT_ACTIONNAME("GetGoodEvilValue") NWSCRIPT_ACTIONPROTOTYPE("int GetGoodEvilValue(object oCreature);") 125, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetGoodEvilValue NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetGoodEvilValue) },
	{ NWSCRIPT_ACTIONNAME("GetAlignmentLawChaos") NWSCRIPT_ACTIONPROTOTYPE("int GetAlignmentLawChaos(object oCreature);") 126, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetAlignmentLawChaos NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAlignmentLawChaos) },
	{ NWSCRIPT_ACTIONNAME("GetAlignmentGoodEvil") NWSCRIPT_ACTIONPROTOTYPE("int GetAlignmentGoodEvil(object oCreature);") 127, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetAlignmentGoodEvil NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAlignmentGoodEvil) },
	{ NWSCRIPT_ACTIONNAME("GetFirstObjectInShape") NWSCRIPT_ACTIONPROTOTYPE("object GetFirstObjectInShape(int nShape, float fSize, location lTarget, int bLineOfSight=FALSE, int nObjectFilter=OBJECT_TYPE_CREATURE, vector vOrigin=[0.0,0.0,0.0]);") 128, 3, 6, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetFirstObjectInShape NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFirstObjectInShape) },
	{ NWSCRIPT_ACTIONNAME("GetNextObjectInShape") NWSCRIPT_ACTIONPROTOTYPE("object GetNextObjectInShape(int nShape, float fSize, location lTarget, int bLineOfSight=FALSE, int nObjectFilter=OBJECT_TYPE_CREATURE, vector vOrigin=[0.0,0.0,0.0]);") 129, 3, 6, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetNextObjectInShape NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetNextObjectInShape) },
	{ NWSCRIPT_ACTIONNAME("EffectEntangle") NWSCRIPT_ACTIONPROTOTYPE("effect EffectEntangle();") 130, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectEntangle NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectEntangle) },
	{ NWSCRIPT_ACTIONNAME("SignalEvent") NWSCRIPT_ACTIONPROTOTYPE("void SignalEvent(object oObject, event evToRun);") 131, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SignalEvent NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SignalEvent) },
	{ NWSCRIPT_ACTIONNAME("EventUserDefined") NWSCRIPT_ACTIONPROTOTYPE("event EventUserDefined(int nUserDefinedEventNumber);") 132, 1, 1, ACTIONTYPE_EVENT, NWN1_NWActionParameterTypes_EventUserDefined NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EventUserDefined) },
	{ NWSCRIPT_ACTIONNAME("EffectDeath") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDeath(int nSpectacularDeath=FALSE, int nDisplayFeedback=TRUE);") 133, 0, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDeath NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDeath) },
	{ NWSCRIPT_ACTIONNAME("EffectKnockdown") NWSCRIPT_ACTIONPROTOTYPE("effect EffectKnockdown();") 134, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectKnockdown NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectKnockdown) },
	{ NWSCRIPT_ACTIONNAME("ActionGiveItem") NWSCRIPT_ACTIONPROTOTYPE("void ActionGiveItem(object oItem, object oGiveTo);") 135, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionGiveItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionGiveItem) },
	{ NWSCRIPT_ACTIONNAME("ActionTakeItem") NWSCRIPT_ACTIONPROTOTYPE("void ActionTakeItem(object oItem, object oTakeFrom);") 136, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionTakeItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionTakeItem) },
	{ NWSCRIPT_ACTIONNAME("VectorNormalize") NWSCRIPT_ACTIONPROTOTYPE("vector VectorNormalize(vector vVector);") 137, 1, 1, ACTIONTYPE_VECTOR, NWN1_NWActionParameterTypes_VectorNormalize NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_VectorNormalize) },
	{ NWSCRIPT_ACTIONNAME("EffectCurse") NWSCRIPT_ACTIONPROTOTYPE("effect EffectCurse(int nStrMod=1, int nDexMod=1, int nConMod=1, int nIntMod=1, int nWisMod=1, int nChaMod=1);") 138, 0, 6, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectCurse NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectCurse) },
	{ NWSCRIPT_ACTIONNAME("GetAbilityScore") NWSCRIPT_ACTIONPROTOTYPE("int GetAbilityScore(object oCreature, int nAbilityType, int nBaseAbilityScore=FALSE);") 139, 2, 3, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetAbilityScore NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAbilityScore) },
	{ NWSCRIPT_ACTIONNAME("GetIsDead") NWSCRIPT_ACTIONPROTOTYPE("int GetIsDead(object oCreature);") 140, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsDead NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsDead) },
	{ NWSCRIPT_ACTIONNAME("PrintVector") NWSCRIPT_ACTIONPROTOTYPE("void PrintVector(vector vVector, int bPrepend);") 141, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_PrintVector NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_PrintVector) },
	{ NWSCRIPT_ACTIONNAME("Vector") NWSCRIPT_ACTIONPROTOTYPE("vector Vector(float x=0.0f, float y=0.0f, float z=0.0f);") 142, 0, 3, ACTIONTYPE_VECTOR, NWN1_NWActionParameterTypes_Vector NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_Vector) },
	{ NWSCRIPT_ACTIONNAME("SetFacingPoint") NWSCRIPT_ACTIONPROTOTYPE("void SetFacingPoint(vector vTarget);") 143, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetFacingPoint NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetFacingPoint) },
	{ NWSCRIPT_ACTIONNAME("AngleToVector") NWSCRIPT_ACTIONPROTOTYPE("vector AngleToVector(float fAngle);") 144, 1, 1, ACTIONTYPE_VECTOR, NWN1_NWActionParameterTypes_AngleToVector NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_AngleToVector) },
	{ NWSCRIPT_ACTIONNAME("VectorToAngle") NWSCRIPT_ACTIONPROTOTYPE("float VectorToAngle(vector vVector);") 145, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_VectorToAngle NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_VectorToAngle) },
	{ NWSCRIPT_ACTIONNAME("TouchAttackMelee") NWSCRIPT_ACTIONPROTOTYPE("int TouchAttackMelee(object oTarget, int bDisplayFeedback=TRUE);") 146, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_TouchAttackMelee NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_TouchAttackMelee) },
	{ NWSCRIPT_ACTIONNAME("TouchAttackRanged") NWSCRIPT_ACTIONPROTOTYPE("int TouchAttackRanged(object oTarget, int bDisplayFeedback=TRUE);") 147, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_TouchAttackRanged NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_TouchAttackRanged) },
	{ NWSCRIPT_ACTIONNAME("EffectParalyze") NWSCRIPT_ACTIONPROTOTYPE("effect EffectParalyze();") 148, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectParalyze NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectParalyze) },
	{ NWSCRIPT_ACTIONNAME("EffectSpellImmunity") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSpellImmunity(int nImmunityToSpell=SPELL_ALL_SPELLS);") 149, 0, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectSpellImmunity NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectSpellImmunity) },
	{ NWSCRIPT_ACTIONNAME("EffectDeaf") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDeaf();") 150, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDeaf NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDeaf) },
	{ NWSCRIPT_ACTIONNAME("GetDistanceBetween") NWSCRIPT_ACTIONPROTOTYPE("float GetDistanceBetween(object oObjectA, object oObjectB);") 151, 2, 2, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_GetDistanceBetween NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetDistanceBetween) },
	{ NWSCRIPT_ACTIONNAME("SetLocalLocation") NWSCRIPT_ACTIONPROTOTYPE("void SetLocalLocation(object oObject, string sVarName, location lValue);") 152, 3, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetLocalLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetLocalLocation) },
	{ NWSCRIPT_ACTIONNAME("GetLocalLocation") NWSCRIPT_ACTIONPROTOTYPE("location GetLocalLocation(object oObject, string sVarName);") 153, 2, 2, ACTIONTYPE_LOCATION, NWN1_NWActionParameterTypes_GetLocalLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLocalLocation) },
	{ NWSCRIPT_ACTIONNAME("EffectSleep") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSleep();") 154, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectSleep NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectSleep) },
	{ NWSCRIPT_ACTIONNAME("GetItemInSlot") NWSCRIPT_ACTIONPROTOTYPE("object GetItemInSlot(int nInventorySlot, object oCreature=OBJECT_SELF);") 155, 1, 2, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetItemInSlot NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemInSlot) },
	{ NWSCRIPT_ACTIONNAME("EffectCharmed") NWSCRIPT_ACTIONPROTOTYPE("effect EffectCharmed();") 156, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectCharmed NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectCharmed) },
	{ NWSCRIPT_ACTIONNAME("EffectConfused") NWSCRIPT_ACTIONPROTOTYPE("effect EffectConfused();") 157, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectConfused NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectConfused) },
	{ NWSCRIPT_ACTIONNAME("EffectFrightened") NWSCRIPT_ACTIONPROTOTYPE("effect EffectFrightened();") 158, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectFrightened NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectFrightened) },
	{ NWSCRIPT_ACTIONNAME("EffectDominated") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDominated();") 159, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDominated NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDominated) },
	{ NWSCRIPT_ACTIONNAME("EffectDazed") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDazed();") 160, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDazed NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDazed) },
	{ NWSCRIPT_ACTIONNAME("EffectStunned") NWSCRIPT_ACTIONPROTOTYPE("effect EffectStunned();") 161, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectStunned NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectStunned) },
	{ NWSCRIPT_ACTIONNAME("SetCommandable") NWSCRIPT_ACTIONPROTOTYPE("void SetCommandable(int bCommandable, object oTarget=OBJECT_SELF);") 162, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetCommandable NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetCommandable) },
	{ NWSCRIPT_ACTIONNAME("GetCommandable") NWSCRIPT_ACTIONPROTOTYPE("int GetCommandable(object oTarget=OBJECT_SELF);") 163, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetCommandable NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCommandable) },
	{ NWSCRIPT_ACTIONNAME("EffectRegenerate") NWSCRIPT_ACTIONPROTOTYPE("effect EffectRegenerate(int nAmount, float fIntervalSeconds);") 164, 2, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectRegenerate NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectRegenerate) },
	{ NWSCRIPT_ACTIONNAME("EffectMovementSpeedIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectMovementSpeedIncrease(int nPercentChange);") 165, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectMovementSpeedIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectMovementSpeedIncrease) },
	{ NWSCRIPT_ACTIONNAME("GetHitDice") NWSCRIPT_ACTIONPROTOTYPE("int GetHitDice(object oCreature);") 166, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetHitDice NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetHitDice) },
	{ NWSCRIPT_ACTIONNAME("ActionForceFollowObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionForceFollowObject(object oFollow, float fFollowDistance=0.0f);") 167, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionForceFollowObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionForceFollowObject) },
	{ NWSCRIPT_ACTIONNAME("GetTag") NWSCRIPT_ACTIONPROTOTYPE("string GetTag(object oObject);") 168, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTag) },
	{ NWSCRIPT_ACTIONNAME("ResistSpell") NWSCRIPT_ACTIONPROTOTYPE("int ResistSpell(object oCaster, object oTarget);") 169, 2, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_ResistSpell NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ResistSpell) },
	{ NWSCRIPT_ACTIONNAME("GetEffectType") NWSCRIPT_ACTIONPROTOTYPE("int GetEffectType(effect eEffect);") 170, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetEffectType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetEffectType) },
	{ NWSCRIPT_ACTIONNAME("EffectAreaOfEffect") NWSCRIPT_ACTIONPROTOTYPE("effect EffectAreaOfEffect(int nAreaEffectId, string sOnEnterScript="", string sHeartbeatScript="", string sOnExitScript="");") 171, 1, 4, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectAreaOfEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectAreaOfEffect) },
	{ NWSCRIPT_ACTIONNAME("GetFactionEqual") NWSCRIPT_ACTIONPROTOTYPE("int GetFactionEqual(object oFirstObject, object oSecondObject=OBJECT_SELF);") 172, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetFactionEqual NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFactionEqual) },
	{ NWSCRIPT_ACTIONNAME("ChangeFaction") NWSCRIPT_ACTIONPROTOTYPE("void ChangeFaction(object oObjectToChangeFaction, object oMemberOfFactionToJoin);") 173, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ChangeFaction NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ChangeFaction) },
	{ NWSCRIPT_ACTIONNAME("GetIsListening") NWSCRIPT_ACTIONPROTOTYPE("int GetIsListening(object oObject);") 174, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsListening NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsListening) },
	{ NWSCRIPT_ACTIONNAME("SetListening") NWSCRIPT_ACTIONPROTOTYPE("void SetListening(object oObject, int bValue);") 175, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetListening NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetListening) },
	{ NWSCRIPT_ACTIONNAME("SetListenPattern") NWSCRIPT_ACTIONPROTOTYPE("void SetListenPattern(object oObject, string sPattern, int nNumber=0);") 176, 2, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetListenPattern NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetListenPattern) },
	{ NWSCRIPT_ACTIONNAME("TestStringAgainstPattern") NWSCRIPT_ACTIONPROTOTYPE("int TestStringAgainstPattern(string sPattern, string sStringToTest);") 177, 2, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_TestStringAgainstPattern NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_TestStringAgainstPattern) },
	{ NWSCRIPT_ACTIONNAME("GetMatchedSubstring") NWSCRIPT_ACTIONPROTOTYPE("string GetMatchedSubstring(int nString);") 178, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetMatchedSubstring NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetMatchedSubstring) },
	{ NWSCRIPT_ACTIONNAME("GetMatchedSubstringsCount") NWSCRIPT_ACTIONPROTOTYPE("int GetMatchedSubstringsCount();") 179, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetMatchedSubstringsCount NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetMatchedSubstringsCount) },
	{ NWSCRIPT_ACTIONNAME("EffectVisualEffect") NWSCRIPT_ACTIONPROTOTYPE("effect EffectVisualEffect(int nVisualEffectId, int nMissEffect=FALSE);") 180, 1, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectVisualEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectVisualEffect) },
	{ NWSCRIPT_ACTIONNAME("GetFactionWeakestMember") NWSCRIPT_ACTIONPROTOTYPE("object GetFactionWeakestMember(object oFactionMember=OBJECT_SELF, int bMustBeVisible=TRUE);") 181, 0, 2, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetFactionWeakestMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFactionWeakestMember) },
	{ NWSCRIPT_ACTIONNAME("GetFactionStrongestMember") NWSCRIPT_ACTIONPROTOTYPE("object GetFactionStrongestMember(object oFactionMember=OBJECT_SELF, int bMustBeVisible=TRUE);") 182, 0, 2, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetFactionStrongestMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFactionStrongestMember) },
	{ NWSCRIPT_ACTIONNAME("GetFactionMostDamagedMember") NWSCRIPT_ACTIONPROTOTYPE("object GetFactionMostDamagedMember(object oFactionMember=OBJECT_SELF, int bMustBeVisible=TRUE);") 183, 0, 2, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetFactionMostDamagedMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFactionMostDamagedMember) },
	{ NWSCRIPT_ACTIONNAME("GetFactionLeastDamagedMember") NWSCRIPT_ACTIONPROTOTYPE("object GetFactionLeastDamagedMember(object oFactionMember=OBJECT_SELF, int bMustBeVisible=TRUE);") 184, 0, 2, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetFactionLeastDamagedMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFactionLeastDamagedMember) },
	{ NWSCRIPT_ACTIONNAME("GetFactionGold") NWSCRIPT_ACTIONPROTOTYPE("int GetFactionGold(object oFactionMember);") 185, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetFactionGold NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFactionGold) },
	{ NWSCRIPT_ACTIONNAME("GetFactionAverageReputation") NWSCRIPT_ACTIONPROTOTYPE("int GetFactionAverageReputation(object oSourceFactionMember, object oTarget);") 186, 2, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetFactionAverageReputation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFactionAverageReputation) },
	{ NWSCRIPT_ACTIONNAME("GetFactionAverageGoodEvilAlignment") NWSCRIPT_ACTIONPROTOTYPE("int GetFactionAverageGoodEvilAlignment(object oFactionMember);") 187, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetFactionAverageGoodEvilAlignment NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFactionAverageGoodEvilAlignment) },
	{ NWSCRIPT_ACTIONNAME("GetFactionAverageLawChaosAlignment") NWSCRIPT_ACTIONPROTOTYPE("int GetFactionAverageLawChaosAlignment(object oFactionMember);") 188, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetFactionAverageLawChaosAlignment NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFactionAverageLawChaosAlignment) },
	{ NWSCRIPT_ACTIONNAME("GetFactionAverageLevel") NWSCRIPT_ACTIONPROTOTYPE("int GetFactionAverageLevel(object oFactionMember);") 189, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetFactionAverageLevel NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFactionAverageLevel) },
	{ NWSCRIPT_ACTIONNAME("GetFactionAverageXP") NWSCRIPT_ACTIONPROTOTYPE("int GetFactionAverageXP(object oFactionMember);") 190, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetFactionAverageXP NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFactionAverageXP) },
	{ NWSCRIPT_ACTIONNAME("GetFactionMostFrequentClass") NWSCRIPT_ACTIONPROTOTYPE("int GetFactionMostFrequentClass(object oFactionMember);") 191, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetFactionMostFrequentClass NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFactionMostFrequentClass) },
	{ NWSCRIPT_ACTIONNAME("GetFactionWorstAC") NWSCRIPT_ACTIONPROTOTYPE("object GetFactionWorstAC(object oFactionMember=OBJECT_SELF, int bMustBeVisible=TRUE);") 192, 0, 2, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetFactionWorstAC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFactionWorstAC) },
	{ NWSCRIPT_ACTIONNAME("GetFactionBestAC") NWSCRIPT_ACTIONPROTOTYPE("object GetFactionBestAC(object oFactionMember=OBJECT_SELF, int bMustBeVisible=TRUE);") 193, 0, 2, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetFactionBestAC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFactionBestAC) },
	{ NWSCRIPT_ACTIONNAME("ActionSit") NWSCRIPT_ACTIONPROTOTYPE("void ActionSit(object oChair);") 194, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionSit NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionSit) },
	{ NWSCRIPT_ACTIONNAME("GetListenPatternNumber") NWSCRIPT_ACTIONPROTOTYPE("int GetListenPatternNumber();") 195, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetListenPatternNumber NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetListenPatternNumber) },
	{ NWSCRIPT_ACTIONNAME("ActionJumpToObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionJumpToObject(object oToJumpTo, int bWalkStraightLineToPoint=TRUE);") 196, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionJumpToObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionJumpToObject) },
	{ NWSCRIPT_ACTIONNAME("GetWaypointByTag") NWSCRIPT_ACTIONPROTOTYPE("object GetWaypointByTag(string sWaypointTag);") 197, 1, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetWaypointByTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetWaypointByTag) },
	{ NWSCRIPT_ACTIONNAME("GetTransitionTarget") NWSCRIPT_ACTIONPROTOTYPE("object GetTransitionTarget(object oTransition);") 198, 1, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetTransitionTarget NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTransitionTarget) },
	{ NWSCRIPT_ACTIONNAME("EffectLinkEffects") NWSCRIPT_ACTIONPROTOTYPE("effect EffectLinkEffects(effect eChildEffect, effect eParentEffect );") 199, 2, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectLinkEffects NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectLinkEffects) },
	{ NWSCRIPT_ACTIONNAME("GetObjectByTag") NWSCRIPT_ACTIONPROTOTYPE("object GetObjectByTag(string sTag, int nNth=0);") 200, 1, 2, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetObjectByTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetObjectByTag) },
	{ NWSCRIPT_ACTIONNAME("AdjustAlignment") NWSCRIPT_ACTIONPROTOTYPE("void AdjustAlignment(object oSubject, int nAlignment, int nShift, int bAllPartyMembers=TRUE);") 201, 3, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_AdjustAlignment NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_AdjustAlignment) },
	{ NWSCRIPT_ACTIONNAME("ActionWait") NWSCRIPT_ACTIONPROTOTYPE("void ActionWait(float fSeconds);") 202, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionWait NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionWait) },
	{ NWSCRIPT_ACTIONNAME("SetAreaTransitionBMP") NWSCRIPT_ACTIONPROTOTYPE("void SetAreaTransitionBMP(int nPredefinedAreaTransition, string sCustomAreaTransitionBMP="");") 203, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetAreaTransitionBMP NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetAreaTransitionBMP) },
	{ NWSCRIPT_ACTIONNAME("ActionStartConversation") NWSCRIPT_ACTIONPROTOTYPE("void ActionStartConversation(object oObjectToConverseWith, string sDialogResRef="", int bPrivateConversation=FALSE, int bPlayHello=TRUE);") 204, 1, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionStartConversation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionStartConversation) },
	{ NWSCRIPT_ACTIONNAME("ActionPauseConversation") NWSCRIPT_ACTIONPROTOTYPE("void ActionPauseConversation();") 205, 0, 0, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionPauseConversation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionPauseConversation) },
	{ NWSCRIPT_ACTIONNAME("ActionResumeConversation") NWSCRIPT_ACTIONPROTOTYPE("void ActionResumeConversation();") 206, 0, 0, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionResumeConversation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionResumeConversation) },
	{ NWSCRIPT_ACTIONNAME("EffectBeam") NWSCRIPT_ACTIONPROTOTYPE("effect EffectBeam(int nBeamVisualEffect, object oEffector, int nBodyPart, int bMissEffect=FALSE);") 207, 3, 4, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectBeam NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectBeam) },
	{ NWSCRIPT_ACTIONNAME("GetReputation") NWSCRIPT_ACTIONPROTOTYPE("int GetReputation(object oSource, object oTarget);") 208, 2, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetReputation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetReputation) },
	{ NWSCRIPT_ACTIONNAME("AdjustReputation") NWSCRIPT_ACTIONPROTOTYPE("void AdjustReputation(object oTarget, object oSourceFactionMember, int nAdjustment);") 209, 3, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_AdjustReputation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_AdjustReputation) },
	{ NWSCRIPT_ACTIONNAME("GetSittingCreature") NWSCRIPT_ACTIONPROTOTYPE("object GetSittingCreature(object oChair);") 210, 1, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetSittingCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetSittingCreature) },
	{ NWSCRIPT_ACTIONNAME("GetGoingToBeAttackedBy") NWSCRIPT_ACTIONPROTOTYPE("object GetGoingToBeAttackedBy(object oTarget);") 211, 1, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetGoingToBeAttackedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetGoingToBeAttackedBy) },
	{ NWSCRIPT_ACTIONNAME("EffectSpellResistanceIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSpellResistanceIncrease(int nValue);") 212, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectSpellResistanceIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectSpellResistanceIncrease) },
	{ NWSCRIPT_ACTIONNAME("GetLocation") NWSCRIPT_ACTIONPROTOTYPE("location GetLocation(object oObject);") 213, 1, 1, ACTIONTYPE_LOCATION, NWN1_NWActionParameterTypes_GetLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLocation) },
	{ NWSCRIPT_ACTIONNAME("ActionJumpToLocation") NWSCRIPT_ACTIONPROTOTYPE("void ActionJumpToLocation(location lLocation);") 214, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionJumpToLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionJumpToLocation) },
	{ NWSCRIPT_ACTIONNAME("Location") NWSCRIPT_ACTIONPROTOTYPE("location Location(object oArea, vector vPosition, float fOrientation);") 215, 3, 3, ACTIONTYPE_LOCATION, NWN1_NWActionParameterTypes_Location NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_Location) },
	{ NWSCRIPT_ACTIONNAME("ApplyEffectAtLocation") NWSCRIPT_ACTIONPROTOTYPE("void ApplyEffectAtLocation(int nDurationType, effect eEffect, location lLocation, float fDuration=0.0f);") 216, 3, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ApplyEffectAtLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ApplyEffectAtLocation) },
	{ NWSCRIPT_ACTIONNAME("GetIsPC") NWSCRIPT_ACTIONPROTOTYPE("int GetIsPC(object oCreature);") 217, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsPC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsPC) },
	{ NWSCRIPT_ACTIONNAME("FeetToMeters") NWSCRIPT_ACTIONPROTOTYPE("float FeetToMeters(float fFeet);") 218, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_FeetToMeters NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_FeetToMeters) },
	{ NWSCRIPT_ACTIONNAME("YardsToMeters") NWSCRIPT_ACTIONPROTOTYPE("float YardsToMeters(float fYards);") 219, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_YardsToMeters NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_YardsToMeters) },
	{ NWSCRIPT_ACTIONNAME("ApplyEffectToObject") NWSCRIPT_ACTIONPROTOTYPE("void ApplyEffectToObject(int nDurationType, effect eEffect, object oTarget, float fDuration=0.0f);") 220, 3, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ApplyEffectToObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ApplyEffectToObject) },
	{ NWSCRIPT_ACTIONNAME("SpeakString") NWSCRIPT_ACTIONPROTOTYPE("void SpeakString(string sStringToSpeak, int nTalkVolume=TALKVOLUME_TALK);") 221, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SpeakString NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SpeakString) },
	{ NWSCRIPT_ACTIONNAME("GetSpellTargetLocation") NWSCRIPT_ACTIONPROTOTYPE("location GetSpellTargetLocation();") 222, 0, 0, ACTIONTYPE_LOCATION, NWN1_NWActionParameterTypes_GetSpellTargetLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetSpellTargetLocation) },
	{ NWSCRIPT_ACTIONNAME("GetPositionFromLocation") NWSCRIPT_ACTIONPROTOTYPE("vector GetPositionFromLocation(location lLocation);") 223, 1, 1, ACTIONTYPE_VECTOR, NWN1_NWActionParameterTypes_GetPositionFromLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPositionFromLocation) },
	{ NWSCRIPT_ACTIONNAME("GetAreaFromLocation") NWSCRIPT_ACTIONPROTOTYPE("object GetAreaFromLocation(location lLocation);") 224, 1, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetAreaFromLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAreaFromLocation) },
	{ NWSCRIPT_ACTIONNAME("GetFacingFromLocation") NWSCRIPT_ACTIONPROTOTYPE("float GetFacingFromLocation(location lLocation);") 225, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_GetFacingFromLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFacingFromLocation) },
	{ NWSCRIPT_ACTIONNAME("GetNearestCreatureToLocation") NWSCRIPT_ACTIONPROTOTYPE("object GetNearestCreatureToLocation(int nFirstCriteriaType, int nFirstCriteriaValue,  location lLocation, int nNth=1, int nSecondCriteriaType=-1, int nSecondCriteriaValue=-1, int nThirdCriteriaType=-1,  int nThirdCriteriaValue=-1 );") 226, 3, 8, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetNearestCreatureToLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetNearestCreatureToLocation) },
	{ NWSCRIPT_ACTIONNAME("GetNearestObject") NWSCRIPT_ACTIONPROTOTYPE("object GetNearestObject(int nObjectType=OBJECT_TYPE_ALL, object oTarget=OBJECT_SELF, int nNth=1);") 227, 0, 3, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetNearestObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetNearestObject) },
	{ NWSCRIPT_ACTIONNAME("GetNearestObjectToLocation") NWSCRIPT_ACTIONPROTOTYPE("object GetNearestObjectToLocation(int nObjectType, location lLocation, int nNth=1);") 228, 2, 3, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetNearestObjectToLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetNearestObjectToLocation) },
	{ NWSCRIPT_ACTIONNAME("GetNearestObjectByTag") NWSCRIPT_ACTIONPROTOTYPE("object GetNearestObjectByTag(string sTag, object oTarget=OBJECT_SELF, int nNth=1);") 229, 1, 3, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetNearestObjectByTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetNearestObjectByTag) },
	{ NWSCRIPT_ACTIONNAME("IntToFloat") NWSCRIPT_ACTIONPROTOTYPE("float IntToFloat(int nInteger);") 230, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_IntToFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_IntToFloat) },
	{ NWSCRIPT_ACTIONNAME("FloatToInt") NWSCRIPT_ACTIONPROTOTYPE("int FloatToInt(float fFloat);") 231, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_FloatToInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_FloatToInt) },
	{ NWSCRIPT_ACTIONNAME("StringToInt") NWSCRIPT_ACTIONPROTOTYPE("int StringToInt(string sNumber);") 232, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_StringToInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_StringToInt) },
	{ NWSCRIPT_ACTIONNAME("StringToFloat") NWSCRIPT_ACTIONPROTOTYPE("float StringToFloat(string sNumber);") 233, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_StringToFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_StringToFloat) },
	{ NWSCRIPT_ACTIONNAME("ActionCastSpellAtLocation") NWSCRIPT_ACTIONPROTOTYPE("void   ActionCastSpellAtLocation(int nSpell, location lTargetLocation, int nMetaMagic=METAMAGIC_ANY, int bCheat=FALSE, int nProjectilePathType=PROJECTILE_PATH_TYPE_DEFAULT, int bInstantSpell=FALSE);") 234, 2, 6, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionCastSpellAtLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionCastSpellAtLocation) },
	{ NWSCRIPT_ACTIONNAME("GetIsEnemy") NWSCRIPT_ACTIONPROTOTYPE("int GetIsEnemy(object oTarget, object oSource=OBJECT_SELF);") 235, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsEnemy NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsEnemy) },
	{ NWSCRIPT_ACTIONNAME("GetIsFriend") NWSCRIPT_ACTIONPROTOTYPE("int GetIsFriend(object oTarget, object oSource=OBJECT_SELF);") 236, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsFriend NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsFriend) },
	{ NWSCRIPT_ACTIONNAME("GetIsNeutral") NWSCRIPT_ACTIONPROTOTYPE("int GetIsNeutral(object oTarget, object oSource=OBJECT_SELF);") 237, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsNeutral NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsNeutral) },
	{ NWSCRIPT_ACTIONNAME("GetPCSpeaker") NWSCRIPT_ACTIONPROTOTYPE("object GetPCSpeaker();") 238, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetPCSpeaker NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPCSpeaker) },
	{ NWSCRIPT_ACTIONNAME("GetStringByStrRef") NWSCRIPT_ACTIONPROTOTYPE("string GetStringByStrRef(int nStrRef, int nGender=GENDER_MALE);") 239, 1, 2, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetStringByStrRef NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetStringByStrRef) },
	{ NWSCRIPT_ACTIONNAME("ActionSpeakStringByStrRef") NWSCRIPT_ACTIONPROTOTYPE("void ActionSpeakStringByStrRef(int nStrRef, int nTalkVolume=TALKVOLUME_TALK);") 240, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionSpeakStringByStrRef NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionSpeakStringByStrRef) },
	{ NWSCRIPT_ACTIONNAME("DestroyObject") NWSCRIPT_ACTIONPROTOTYPE("void DestroyObject(object oDestroy, float fDelay=0.0f);") 241, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_DestroyObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_DestroyObject) },
	{ NWSCRIPT_ACTIONNAME("GetModule") NWSCRIPT_ACTIONPROTOTYPE("object GetModule();") 242, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetModule NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetModule) },
	{ NWSCRIPT_ACTIONNAME("CreateObject") NWSCRIPT_ACTIONPROTOTYPE("object CreateObject(int nObjectType, string sTemplate, location lLocation, int bUseAppearAnimation=FALSE, string sNewTag="");") 243, 3, 5, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_CreateObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_CreateObject) },
	{ NWSCRIPT_ACTIONNAME("EventSpellCastAt") NWSCRIPT_ACTIONPROTOTYPE("event EventSpellCastAt(object oCaster, int nSpell, int bHarmful=TRUE);") 244, 2, 3, ACTIONTYPE_EVENT, NWN1_NWActionParameterTypes_EventSpellCastAt NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EventSpellCastAt) },
	{ NWSCRIPT_ACTIONNAME("GetLastSpellCaster") NWSCRIPT_ACTIONPROTOTYPE("object GetLastSpellCaster();") 245, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastSpellCaster NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastSpellCaster) },
	{ NWSCRIPT_ACTIONNAME("GetLastSpell") NWSCRIPT_ACTIONPROTOTYPE("int GetLastSpell();") 246, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLastSpell NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastSpell) },
	{ NWSCRIPT_ACTIONNAME("GetUserDefinedEventNumber") NWSCRIPT_ACTIONPROTOTYPE("int GetUserDefinedEventNumber();") 247, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetUserDefinedEventNumber NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetUserDefinedEventNumber) },
	{ NWSCRIPT_ACTIONNAME("GetSpellId") NWSCRIPT_ACTIONPROTOTYPE("int GetSpellId();") 248, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetSpellId NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetSpellId) },
	{ NWSCRIPT_ACTIONNAME("RandomName") NWSCRIPT_ACTIONPROTOTYPE("string RandomName(int nNameType=NAME_FIRST_GENERIC_MALE);") 249, 0, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_RandomName NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_RandomName) },
	{ NWSCRIPT_ACTIONNAME("EffectPoison") NWSCRIPT_ACTIONPROTOTYPE("effect EffectPoison(int nPoisonType);") 250, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectPoison NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectPoison) },
	{ NWSCRIPT_ACTIONNAME("EffectDisease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDisease(int nDiseaseType);") 251, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDisease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDisease) },
	{ NWSCRIPT_ACTIONNAME("EffectSilence") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSilence();") 252, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectSilence NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectSilence) },
	{ NWSCRIPT_ACTIONNAME("GetName") NWSCRIPT_ACTIONPROTOTYPE("string GetName(object oObject, int bOriginalName=FALSE);") 253, 1, 2, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetName NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetName) },
	{ NWSCRIPT_ACTIONNAME("GetLastSpeaker") NWSCRIPT_ACTIONPROTOTYPE("object GetLastSpeaker();") 254, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastSpeaker NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastSpeaker) },
	{ NWSCRIPT_ACTIONNAME("BeginConversation") NWSCRIPT_ACTIONPROTOTYPE("int BeginConversation(string sResRef="", object oObjectToDialog=OBJECT_INVALID);") 255, 0, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_BeginConversation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_BeginConversation) },
	{ NWSCRIPT_ACTIONNAME("GetLastPerceived") NWSCRIPT_ACTIONPROTOTYPE("object GetLastPerceived();") 256, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastPerceived NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastPerceived) },
	{ NWSCRIPT_ACTIONNAME("GetLastPerceptionHeard") NWSCRIPT_ACTIONPROTOTYPE("int GetLastPerceptionHeard();") 257, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLastPerceptionHeard NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastPerceptionHeard) },
	{ NWSCRIPT_ACTIONNAME("GetLastPerceptionInaudible") NWSCRIPT_ACTIONPROTOTYPE("int GetLastPerceptionInaudible();") 258, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLastPerceptionInaudible NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastPerceptionInaudible) },
	{ NWSCRIPT_ACTIONNAME("GetLastPerceptionSeen") NWSCRIPT_ACTIONPROTOTYPE("int GetLastPerceptionSeen();") 259, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLastPerceptionSeen NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastPerceptionSeen) },
	{ NWSCRIPT_ACTIONNAME("GetLastClosedBy") NWSCRIPT_ACTIONPROTOTYPE("object GetLastClosedBy();") 260, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastClosedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastClosedBy) },
	{ NWSCRIPT_ACTIONNAME("GetLastPerceptionVanished") NWSCRIPT_ACTIONPROTOTYPE("int GetLastPerceptionVanished();") 261, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLastPerceptionVanished NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastPerceptionVanished) },
	{ NWSCRIPT_ACTIONNAME("GetFirstInPersistentObject") NWSCRIPT_ACTIONPROTOTYPE("object GetFirstInPersistentObject(object oPersistentObject=OBJECT_SELF, int nResidentObjectType=OBJECT_TYPE_CREATURE, int nPersistentZone=PERSISTENT_ZONE_ACTIVE);") 262, 0, 3, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetFirstInPersistentObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFirstInPersistentObject) },
	{ NWSCRIPT_ACTIONNAME("GetNextInPersistentObject") NWSCRIPT_ACTIONPROTOTYPE("object GetNextInPersistentObject(object oPersistentObject=OBJECT_SELF, int nResidentObjectType=OBJECT_TYPE_CREATURE, int nPersistentZone=PERSISTENT_ZONE_ACTIVE);") 263, 0, 3, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetNextInPersistentObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetNextInPersistentObject) },
	{ NWSCRIPT_ACTIONNAME("GetAreaOfEffectCreator") NWSCRIPT_ACTIONPROTOTYPE("object GetAreaOfEffectCreator(object oAreaOfEffectObject=OBJECT_SELF);") 264, 0, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetAreaOfEffectCreator NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAreaOfEffectCreator) },
	{ NWSCRIPT_ACTIONNAME("DeleteLocalInt") NWSCRIPT_ACTIONPROTOTYPE("void DeleteLocalInt(object oObject, string sVarName);") 265, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_DeleteLocalInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_DeleteLocalInt) },
	{ NWSCRIPT_ACTIONNAME("DeleteLocalFloat") NWSCRIPT_ACTIONPROTOTYPE("void DeleteLocalFloat(object oObject, string sVarName);") 266, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_DeleteLocalFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_DeleteLocalFloat) },
	{ NWSCRIPT_ACTIONNAME("DeleteLocalString") NWSCRIPT_ACTIONPROTOTYPE("void DeleteLocalString(object oObject, string sVarName);") 267, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_DeleteLocalString NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_DeleteLocalString) },
	{ NWSCRIPT_ACTIONNAME("DeleteLocalObject") NWSCRIPT_ACTIONPROTOTYPE("void DeleteLocalObject(object oObject, string sVarName);") 268, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_DeleteLocalObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_DeleteLocalObject) },
	{ NWSCRIPT_ACTIONNAME("DeleteLocalLocation") NWSCRIPT_ACTIONPROTOTYPE("void DeleteLocalLocation(object oObject, string sVarName);") 269, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_DeleteLocalLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_DeleteLocalLocation) },
	{ NWSCRIPT_ACTIONNAME("EffectHaste") NWSCRIPT_ACTIONPROTOTYPE("effect EffectHaste();") 270, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectHaste NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectHaste) },
	{ NWSCRIPT_ACTIONNAME("EffectSlow") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSlow();") 271, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectSlow NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectSlow) },
	{ NWSCRIPT_ACTIONNAME("ObjectToString") NWSCRIPT_ACTIONPROTOTYPE("string ObjectToString(object oObject);") 272, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_ObjectToString NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ObjectToString) },
	{ NWSCRIPT_ACTIONNAME("EffectImmunity") NWSCRIPT_ACTIONPROTOTYPE("effect EffectImmunity(int nImmunityType);") 273, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectImmunity NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectImmunity) },
	{ NWSCRIPT_ACTIONNAME("GetIsImmune") NWSCRIPT_ACTIONPROTOTYPE("int GetIsImmune(object oCreature, int nImmunityType, object oVersus=OBJECT_INVALID);") 274, 2, 3, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsImmune NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsImmune) },
	{ NWSCRIPT_ACTIONNAME("EffectDamageImmunityIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamageImmunityIncrease(int nDamageType, int nPercentImmunity);") 275, 2, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDamageImmunityIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDamageImmunityIncrease) },
	{ NWSCRIPT_ACTIONNAME("GetEncounterActive") NWSCRIPT_ACTIONPROTOTYPE("int  GetEncounterActive(object oEncounter=OBJECT_SELF);") 276, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetEncounterActive NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetEncounterActive) },
	{ NWSCRIPT_ACTIONNAME("SetEncounterActive") NWSCRIPT_ACTIONPROTOTYPE("void SetEncounterActive(int nNewValue, object oEncounter=OBJECT_SELF);") 277, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetEncounterActive NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetEncounterActive) },
	{ NWSCRIPT_ACTIONNAME("GetEncounterSpawnsMax") NWSCRIPT_ACTIONPROTOTYPE("int GetEncounterSpawnsMax(object oEncounter=OBJECT_SELF);") 278, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetEncounterSpawnsMax NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetEncounterSpawnsMax) },
	{ NWSCRIPT_ACTIONNAME("SetEncounterSpawnsMax") NWSCRIPT_ACTIONPROTOTYPE("void SetEncounterSpawnsMax(int nNewValue, object oEncounter=OBJECT_SELF);") 279, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetEncounterSpawnsMax NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetEncounterSpawnsMax) },
	{ NWSCRIPT_ACTIONNAME("GetEncounterSpawnsCurrent") NWSCRIPT_ACTIONPROTOTYPE("int  GetEncounterSpawnsCurrent(object oEncounter=OBJECT_SELF);") 280, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetEncounterSpawnsCurrent NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetEncounterSpawnsCurrent) },
	{ NWSCRIPT_ACTIONNAME("SetEncounterSpawnsCurrent") NWSCRIPT_ACTIONPROTOTYPE("void SetEncounterSpawnsCurrent(int nNewValue, object oEncounter=OBJECT_SELF);") 281, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetEncounterSpawnsCurrent NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetEncounterSpawnsCurrent) },
	{ NWSCRIPT_ACTIONNAME("GetModuleItemAcquired") NWSCRIPT_ACTIONPROTOTYPE("object GetModuleItemAcquired();") 282, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetModuleItemAcquired NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetModuleItemAcquired) },
	{ NWSCRIPT_ACTIONNAME("GetModuleItemAcquiredFrom") NWSCRIPT_ACTIONPROTOTYPE("object GetModuleItemAcquiredFrom();") 283, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetModuleItemAcquiredFrom NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetModuleItemAcquiredFrom) },
	{ NWSCRIPT_ACTIONNAME("SetCustomToken") NWSCRIPT_ACTIONPROTOTYPE("void SetCustomToken(int nCustomTokenNumber, string sTokenValue);") 284, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetCustomToken NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetCustomToken) },
	{ NWSCRIPT_ACTIONNAME("GetHasFeat") NWSCRIPT_ACTIONPROTOTYPE("int GetHasFeat(int nFeat, object oCreature=OBJECT_SELF);") 285, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetHasFeat NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetHasFeat) },
	{ NWSCRIPT_ACTIONNAME("GetHasSkill") NWSCRIPT_ACTIONPROTOTYPE("int GetHasSkill(int nSkill, object oCreature=OBJECT_SELF);") 286, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetHasSkill NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetHasSkill) },
	{ NWSCRIPT_ACTIONNAME("ActionUseFeat") NWSCRIPT_ACTIONPROTOTYPE("void ActionUseFeat(int nFeat, object oTarget);") 287, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionUseFeat NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionUseFeat) },
	{ NWSCRIPT_ACTIONNAME("ActionUseSkill") NWSCRIPT_ACTIONPROTOTYPE("void ActionUseSkill(int nSkill, object oTarget, int nSubSkill=0, object oItemUsed=OBJECT_INVALID );") 288, 2, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionUseSkill NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionUseSkill) },
	{ NWSCRIPT_ACTIONNAME("GetObjectSeen") NWSCRIPT_ACTIONPROTOTYPE("int GetObjectSeen(object oTarget, object oSource=OBJECT_SELF);") 289, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetObjectSeen NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetObjectSeen) },
	{ NWSCRIPT_ACTIONNAME("GetObjectHeard") NWSCRIPT_ACTIONPROTOTYPE("int GetObjectHeard(object oTarget, object oSource=OBJECT_SELF);") 290, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetObjectHeard NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetObjectHeard) },
	{ NWSCRIPT_ACTIONNAME("GetLastPlayerDied") NWSCRIPT_ACTIONPROTOTYPE("object GetLastPlayerDied();") 291, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastPlayerDied NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastPlayerDied) },
	{ NWSCRIPT_ACTIONNAME("GetModuleItemLost") NWSCRIPT_ACTIONPROTOTYPE("object GetModuleItemLost();") 292, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetModuleItemLost NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetModuleItemLost) },
	{ NWSCRIPT_ACTIONNAME("GetModuleItemLostBy") NWSCRIPT_ACTIONPROTOTYPE("object GetModuleItemLostBy();") 293, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetModuleItemLostBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetModuleItemLostBy) },
	{ NWSCRIPT_ACTIONNAME("ActionDoCommand") NWSCRIPT_ACTIONPROTOTYPE("void ActionDoCommand(action aActionToDo);") 294, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionDoCommand NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionDoCommand) },
	{ NWSCRIPT_ACTIONNAME("EventConversation") NWSCRIPT_ACTIONPROTOTYPE("event EventConversation();") 295, 0, 0, ACTIONTYPE_EVENT, NWN1_NWActionParameterTypes_EventConversation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EventConversation) },
	{ NWSCRIPT_ACTIONNAME("SetEncounterDifficulty") NWSCRIPT_ACTIONPROTOTYPE("void SetEncounterDifficulty(int nEncounterDifficulty, object oEncounter=OBJECT_SELF);") 296, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetEncounterDifficulty NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetEncounterDifficulty) },
	{ NWSCRIPT_ACTIONNAME("GetEncounterDifficulty") NWSCRIPT_ACTIONPROTOTYPE("int GetEncounterDifficulty(object oEncounter=OBJECT_SELF);") 297, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetEncounterDifficulty NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetEncounterDifficulty) },
	{ NWSCRIPT_ACTIONNAME("GetDistanceBetweenLocations") NWSCRIPT_ACTIONPROTOTYPE("float GetDistanceBetweenLocations(location lLocationA, location lLocationB);") 298, 2, 2, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_GetDistanceBetweenLocations NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetDistanceBetweenLocations) },
	{ NWSCRIPT_ACTIONNAME("GetReflexAdjustedDamage") NWSCRIPT_ACTIONPROTOTYPE("int GetReflexAdjustedDamage(int nDamage, object oTarget, int nDC, int nSaveType=SAVING_THROW_TYPE_NONE, object oSaveVersus=OBJECT_SELF);") 299, 3, 5, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetReflexAdjustedDamage NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetReflexAdjustedDamage) },
	{ NWSCRIPT_ACTIONNAME("PlayAnimation") NWSCRIPT_ACTIONPROTOTYPE("void PlayAnimation(int nAnimation, float fSpeed=1.0, float fSeconds=0.0);") 300, 1, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_PlayAnimation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_PlayAnimation) },
	{ NWSCRIPT_ACTIONNAME("TalentSpell") NWSCRIPT_ACTIONPROTOTYPE("talent TalentSpell(int nSpell);") 301, 1, 1, ACTIONTYPE_TALENT, NWN1_NWActionParameterTypes_TalentSpell NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_TalentSpell) },
	{ NWSCRIPT_ACTIONNAME("TalentFeat") NWSCRIPT_ACTIONPROTOTYPE("talent TalentFeat(int nFeat);") 302, 1, 1, ACTIONTYPE_TALENT, NWN1_NWActionParameterTypes_TalentFeat NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_TalentFeat) },
	{ NWSCRIPT_ACTIONNAME("TalentSkill") NWSCRIPT_ACTIONPROTOTYPE("talent TalentSkill(int nSkill);") 303, 1, 1, ACTIONTYPE_TALENT, NWN1_NWActionParameterTypes_TalentSkill NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_TalentSkill) },
	{ NWSCRIPT_ACTIONNAME("GetHasSpellEffect") NWSCRIPT_ACTIONPROTOTYPE("int GetHasSpellEffect(int nSpell, object oObject=OBJECT_SELF);") 304, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetHasSpellEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetHasSpellEffect) },
	{ NWSCRIPT_ACTIONNAME("GetEffectSpellId") NWSCRIPT_ACTIONPROTOTYPE("int GetEffectSpellId(effect eSpellEffect);") 305, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetEffectSpellId NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetEffectSpellId) },
	{ NWSCRIPT_ACTIONNAME("GetCreatureHasTalent") NWSCRIPT_ACTIONPROTOTYPE("int GetCreatureHasTalent(talent tTalent, object oCreature=OBJECT_SELF);") 306, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetCreatureHasTalent NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCreatureHasTalent) },
	{ NWSCRIPT_ACTIONNAME("GetCreatureTalentRandom") NWSCRIPT_ACTIONPROTOTYPE("talent GetCreatureTalentRandom(int nCategory, object oCreature=OBJECT_SELF);") 307, 1, 2, ACTIONTYPE_TALENT, NWN1_NWActionParameterTypes_GetCreatureTalentRandom NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCreatureTalentRandom) },
	{ NWSCRIPT_ACTIONNAME("GetCreatureTalentBest") NWSCRIPT_ACTIONPROTOTYPE("talent GetCreatureTalentBest(int nCategory, int nCRMax, object oCreature=OBJECT_SELF);") 308, 2, 3, ACTIONTYPE_TALENT, NWN1_NWActionParameterTypes_GetCreatureTalentBest NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCreatureTalentBest) },
	{ NWSCRIPT_ACTIONNAME("ActionUseTalentOnObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionUseTalentOnObject(talent tChosenTalent, object oTarget);") 309, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionUseTalentOnObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionUseTalentOnObject) },
	{ NWSCRIPT_ACTIONNAME("ActionUseTalentAtLocation") NWSCRIPT_ACTIONPROTOTYPE("void ActionUseTalentAtLocation(talent tChosenTalent, location lTargetLocation);") 310, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionUseTalentAtLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionUseTalentAtLocation) },
	{ NWSCRIPT_ACTIONNAME("GetGoldPieceValue") NWSCRIPT_ACTIONPROTOTYPE("int GetGoldPieceValue(object oItem);") 311, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetGoldPieceValue NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetGoldPieceValue) },
	{ NWSCRIPT_ACTIONNAME("GetIsPlayableRacialType") NWSCRIPT_ACTIONPROTOTYPE("int GetIsPlayableRacialType(object oCreature);") 312, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsPlayableRacialType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsPlayableRacialType) },
	{ NWSCRIPT_ACTIONNAME("JumpToLocation") NWSCRIPT_ACTIONPROTOTYPE("void JumpToLocation(location lDestination);") 313, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_JumpToLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_JumpToLocation) },
	{ NWSCRIPT_ACTIONNAME("EffectTemporaryHitpoints") NWSCRIPT_ACTIONPROTOTYPE("effect EffectTemporaryHitpoints(int nHitPoints);") 314, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectTemporaryHitpoints NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectTemporaryHitpoints) },
	{ NWSCRIPT_ACTIONNAME("GetSkillRank") NWSCRIPT_ACTIONPROTOTYPE("int GetSkillRank(int nSkill, object oTarget=OBJECT_SELF, int nBaseSkillRank=FALSE);") 315, 1, 3, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetSkillRank NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetSkillRank) },
	{ NWSCRIPT_ACTIONNAME("GetAttackTarget") NWSCRIPT_ACTIONPROTOTYPE("object GetAttackTarget(object oCreature=OBJECT_SELF);") 316, 0, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetAttackTarget NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAttackTarget) },
	{ NWSCRIPT_ACTIONNAME("GetLastAttackType") NWSCRIPT_ACTIONPROTOTYPE("int GetLastAttackType(object oCreature=OBJECT_SELF);") 317, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLastAttackType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastAttackType) },
	{ NWSCRIPT_ACTIONNAME("GetLastAttackMode") NWSCRIPT_ACTIONPROTOTYPE("int GetLastAttackMode(object oCreature=OBJECT_SELF);") 318, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLastAttackMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastAttackMode) },
	{ NWSCRIPT_ACTIONNAME("GetMaster") NWSCRIPT_ACTIONPROTOTYPE("object GetMaster(object oAssociate=OBJECT_SELF);") 319, 0, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetMaster NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetMaster) },
	{ NWSCRIPT_ACTIONNAME("GetIsInCombat") NWSCRIPT_ACTIONPROTOTYPE("int GetIsInCombat(object oCreature=OBJECT_SELF);") 320, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsInCombat NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsInCombat) },
	{ NWSCRIPT_ACTIONNAME("GetLastAssociateCommand") NWSCRIPT_ACTIONPROTOTYPE("int GetLastAssociateCommand(object oAssociate=OBJECT_SELF);") 321, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLastAssociateCommand NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastAssociateCommand) },
	{ NWSCRIPT_ACTIONNAME("GiveGoldToCreature") NWSCRIPT_ACTIONPROTOTYPE("void GiveGoldToCreature(object oCreature, int nGP);") 322, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_GiveGoldToCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GiveGoldToCreature) },
	{ NWSCRIPT_ACTIONNAME("SetIsDestroyable") NWSCRIPT_ACTIONPROTOTYPE("void SetIsDestroyable(int bDestroyable, int bRaiseable=TRUE, int bSelectableWhenDead=FALSE);") 323, 1, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetIsDestroyable NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetIsDestroyable) },
	{ NWSCRIPT_ACTIONNAME("SetLocked") NWSCRIPT_ACTIONPROTOTYPE("void SetLocked(object oTarget, int bLocked);") 324, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetLocked NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetLocked) },
	{ NWSCRIPT_ACTIONNAME("GetLocked") NWSCRIPT_ACTIONPROTOTYPE("int GetLocked(object oTarget);") 325, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLocked NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLocked) },
	{ NWSCRIPT_ACTIONNAME("GetClickingObject") NWSCRIPT_ACTIONPROTOTYPE("object GetClickingObject();") 326, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetClickingObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetClickingObject) },
	{ NWSCRIPT_ACTIONNAME("SetAssociateListenPatterns") NWSCRIPT_ACTIONPROTOTYPE("void SetAssociateListenPatterns(object oTarget=OBJECT_SELF);") 327, 0, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetAssociateListenPatterns NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetAssociateListenPatterns) },
	{ NWSCRIPT_ACTIONNAME("GetLastWeaponUsed") NWSCRIPT_ACTIONPROTOTYPE("object GetLastWeaponUsed(object oCreature);") 328, 1, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastWeaponUsed NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastWeaponUsed) },
	{ NWSCRIPT_ACTIONNAME("ActionInteractObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionInteractObject(object oPlaceable);") 329, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionInteractObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionInteractObject) },
	{ NWSCRIPT_ACTIONNAME("GetLastUsedBy") NWSCRIPT_ACTIONPROTOTYPE("object GetLastUsedBy();") 330, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastUsedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastUsedBy) },
	{ NWSCRIPT_ACTIONNAME("GetAbilityModifier") NWSCRIPT_ACTIONPROTOTYPE("int GetAbilityModifier(int nAbility, object oCreature=OBJECT_SELF);") 331, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetAbilityModifier NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAbilityModifier) },
	{ NWSCRIPT_ACTIONNAME("GetIdentified") NWSCRIPT_ACTIONPROTOTYPE("int GetIdentified(object oItem);") 332, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIdentified NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIdentified) },
	{ NWSCRIPT_ACTIONNAME("SetIdentified") NWSCRIPT_ACTIONPROTOTYPE("void SetIdentified(object oItem, int bIdentified);") 333, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetIdentified NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetIdentified) },
	{ NWSCRIPT_ACTIONNAME("SummonAnimalCompanion") NWSCRIPT_ACTIONPROTOTYPE("void SummonAnimalCompanion(object oMaster=OBJECT_SELF);") 334, 0, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SummonAnimalCompanion NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SummonAnimalCompanion) },
	{ NWSCRIPT_ACTIONNAME("SummonFamiliar") NWSCRIPT_ACTIONPROTOTYPE("void SummonFamiliar(object oMaster=OBJECT_SELF);") 335, 0, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SummonFamiliar NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SummonFamiliar) },
	{ NWSCRIPT_ACTIONNAME("GetBlockingDoor") NWSCRIPT_ACTIONPROTOTYPE("object GetBlockingDoor();") 336, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetBlockingDoor NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetBlockingDoor) },
	{ NWSCRIPT_ACTIONNAME("GetIsDoorActionPossible") NWSCRIPT_ACTIONPROTOTYPE("int GetIsDoorActionPossible(object oTargetDoor, int nDoorAction);") 337, 2, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsDoorActionPossible NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsDoorActionPossible) },
	{ NWSCRIPT_ACTIONNAME("DoDoorAction") NWSCRIPT_ACTIONPROTOTYPE("void DoDoorAction(object oTargetDoor, int nDoorAction);") 338, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_DoDoorAction NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_DoDoorAction) },
	{ NWSCRIPT_ACTIONNAME("GetFirstItemInInventory") NWSCRIPT_ACTIONPROTOTYPE("object GetFirstItemInInventory(object oTarget=OBJECT_SELF);") 339, 0, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetFirstItemInInventory NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFirstItemInInventory) },
	{ NWSCRIPT_ACTIONNAME("GetNextItemInInventory") NWSCRIPT_ACTIONPROTOTYPE("object GetNextItemInInventory(object oTarget=OBJECT_SELF);") 340, 0, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetNextItemInInventory NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetNextItemInInventory) },
	{ NWSCRIPT_ACTIONNAME("GetClassByPosition") NWSCRIPT_ACTIONPROTOTYPE("int GetClassByPosition(int nClassPosition, object oCreature=OBJECT_SELF);") 341, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetClassByPosition NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetClassByPosition) },
	{ NWSCRIPT_ACTIONNAME("GetLevelByPosition") NWSCRIPT_ACTIONPROTOTYPE("int GetLevelByPosition(int nClassPosition, object oCreature=OBJECT_SELF);") 342, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLevelByPosition NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLevelByPosition) },
	{ NWSCRIPT_ACTIONNAME("GetLevelByClass") NWSCRIPT_ACTIONPROTOTYPE("int GetLevelByClass(int nClassType, object oCreature=OBJECT_SELF);") 343, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLevelByClass NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLevelByClass) },
	{ NWSCRIPT_ACTIONNAME("GetDamageDealtByType") NWSCRIPT_ACTIONPROTOTYPE("int GetDamageDealtByType(int nDamageType);") 344, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetDamageDealtByType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetDamageDealtByType) },
	{ NWSCRIPT_ACTIONNAME("GetTotalDamageDealt") NWSCRIPT_ACTIONPROTOTYPE("int GetTotalDamageDealt();") 345, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTotalDamageDealt NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTotalDamageDealt) },
	{ NWSCRIPT_ACTIONNAME("GetLastDamager") NWSCRIPT_ACTIONPROTOTYPE("object GetLastDamager(object oObject=OBJECT_SELF);") 346, 0, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastDamager NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastDamager) },
	{ NWSCRIPT_ACTIONNAME("GetLastDisarmed") NWSCRIPT_ACTIONPROTOTYPE("object GetLastDisarmed();") 347, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastDisarmed NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastDisarmed) },
	{ NWSCRIPT_ACTIONNAME("GetLastDisturbed") NWSCRIPT_ACTIONPROTOTYPE("object GetLastDisturbed();") 348, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastDisturbed NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastDisturbed) },
	{ NWSCRIPT_ACTIONNAME("GetLastLocked") NWSCRIPT_ACTIONPROTOTYPE("object GetLastLocked();") 349, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastLocked NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastLocked) },
	{ NWSCRIPT_ACTIONNAME("GetLastUnlocked") NWSCRIPT_ACTIONPROTOTYPE("object GetLastUnlocked();") 350, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastUnlocked NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastUnlocked) },
	{ NWSCRIPT_ACTIONNAME("EffectSkillIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSkillIncrease(int nSkill, int nValue);") 351, 2, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectSkillIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectSkillIncrease) },
	{ NWSCRIPT_ACTIONNAME("GetInventoryDisturbType") NWSCRIPT_ACTIONPROTOTYPE("int GetInventoryDisturbType();") 352, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetInventoryDisturbType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetInventoryDisturbType) },
	{ NWSCRIPT_ACTIONNAME("GetInventoryDisturbItem") NWSCRIPT_ACTIONPROTOTYPE("object GetInventoryDisturbItem();") 353, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetInventoryDisturbItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetInventoryDisturbItem) },
	{ NWSCRIPT_ACTIONNAME("GetHenchman") NWSCRIPT_ACTIONPROTOTYPE("object GetHenchman(object oMaster=OBJECT_SELF,int nNth=1);") 354, 0, 2, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetHenchman NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetHenchman) },
	{ NWSCRIPT_ACTIONNAME("VersusAlignmentEffect") NWSCRIPT_ACTIONPROTOTYPE("effect VersusAlignmentEffect(effect eEffect, int nLawChaos=ALIGNMENT_ALL, int nGoodEvil=ALIGNMENT_ALL);") 355, 1, 3, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_VersusAlignmentEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_VersusAlignmentEffect) },
	{ NWSCRIPT_ACTIONNAME("VersusRacialTypeEffect") NWSCRIPT_ACTIONPROTOTYPE("effect VersusRacialTypeEffect(effect eEffect, int nRacialType);") 356, 2, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_VersusRacialTypeEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_VersusRacialTypeEffect) },
	{ NWSCRIPT_ACTIONNAME("VersusTrapEffect") NWSCRIPT_ACTIONPROTOTYPE("effect VersusTrapEffect(effect eEffect);") 357, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_VersusTrapEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_VersusTrapEffect) },
	{ NWSCRIPT_ACTIONNAME("GetGender") NWSCRIPT_ACTIONPROTOTYPE("int GetGender(object oCreature);") 358, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetGender NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetGender) },
	{ NWSCRIPT_ACTIONNAME("GetIsTalentValid") NWSCRIPT_ACTIONPROTOTYPE("int GetIsTalentValid(talent tTalent);") 359, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsTalentValid NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsTalentValid) },
	{ NWSCRIPT_ACTIONNAME("ActionMoveAwayFromLocation") NWSCRIPT_ACTIONPROTOTYPE("void ActionMoveAwayFromLocation(location lMoveAwayFrom, int bRun=FALSE, float fMoveAwayRange=40.0f);") 360, 1, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionMoveAwayFromLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionMoveAwayFromLocation) },
	{ NWSCRIPT_ACTIONNAME("GetAttemptedAttackTarget") NWSCRIPT_ACTIONPROTOTYPE("object GetAttemptedAttackTarget();") 361, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetAttemptedAttackTarget NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAttemptedAttackTarget) },
	{ NWSCRIPT_ACTIONNAME("GetTypeFromTalent") NWSCRIPT_ACTIONPROTOTYPE("int GetTypeFromTalent(talent tTalent);") 362, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTypeFromTalent NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTypeFromTalent) },
	{ NWSCRIPT_ACTIONNAME("GetIdFromTalent") NWSCRIPT_ACTIONPROTOTYPE("int GetIdFromTalent(talent tTalent);") 363, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIdFromTalent NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIdFromTalent) },
	{ NWSCRIPT_ACTIONNAME("GetAssociate") NWSCRIPT_ACTIONPROTOTYPE("object GetAssociate(int nAssociateType, object oMaster=OBJECT_SELF, int nTh=1);") 364, 1, 3, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetAssociate NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAssociate) },
	{ NWSCRIPT_ACTIONNAME("AddHenchman") NWSCRIPT_ACTIONPROTOTYPE("void AddHenchman(object oMaster, object oHenchman=OBJECT_SELF);") 365, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_AddHenchman NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_AddHenchman) },
	{ NWSCRIPT_ACTIONNAME("RemoveHenchman") NWSCRIPT_ACTIONPROTOTYPE("void RemoveHenchman(object oMaster, object oHenchman=OBJECT_SELF);") 366, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_RemoveHenchman NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_RemoveHenchman) },
	{ NWSCRIPT_ACTIONNAME("AddJournalQuestEntry") NWSCRIPT_ACTIONPROTOTYPE("void AddJournalQuestEntry(string szPlotID, int nState, object oCreature, int bAllPartyMembers=TRUE, int bAllPlayers=FALSE, int bAllowOverrideHigher=FALSE);") 367, 3, 6, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_AddJournalQuestEntry NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_AddJournalQuestEntry) },
	{ NWSCRIPT_ACTIONNAME("RemoveJournalQuestEntry") NWSCRIPT_ACTIONPROTOTYPE("void RemoveJournalQuestEntry(string szPlotID, object oCreature, int bAllPartyMembers=TRUE, int bAllPlayers=FALSE);") 368, 2, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_RemoveJournalQuestEntry NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_RemoveJournalQuestEntry) },
	{ NWSCRIPT_ACTIONNAME("GetPCPublicCDKey") NWSCRIPT_ACTIONPROTOTYPE("string GetPCPublicCDKey(object oPlayer, int nSinglePlayerCDKey=FALSE);") 369, 1, 2, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetPCPublicCDKey NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPCPublicCDKey) },
	{ NWSCRIPT_ACTIONNAME("GetPCIPAddress") NWSCRIPT_ACTIONPROTOTYPE("string GetPCIPAddress(object oPlayer);") 370, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetPCIPAddress NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPCIPAddress) },
	{ NWSCRIPT_ACTIONNAME("GetPCPlayerName") NWSCRIPT_ACTIONPROTOTYPE("string GetPCPlayerName(object oPlayer);") 371, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetPCPlayerName NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPCPlayerName) },
	{ NWSCRIPT_ACTIONNAME("SetPCLike") NWSCRIPT_ACTIONPROTOTYPE("void SetPCLike(object oPlayer, object oTarget);") 372, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetPCLike NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetPCLike) },
	{ NWSCRIPT_ACTIONNAME("SetPCDislike") NWSCRIPT_ACTIONPROTOTYPE("void SetPCDislike(object oPlayer, object oTarget);") 373, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetPCDislike NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetPCDislike) },
	{ NWSCRIPT_ACTIONNAME("SendMessageToPC") NWSCRIPT_ACTIONPROTOTYPE("void SendMessageToPC(object oPlayer, string szMessage);") 374, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SendMessageToPC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SendMessageToPC) },
	{ NWSCRIPT_ACTIONNAME("GetAttemptedSpellTarget") NWSCRIPT_ACTIONPROTOTYPE("object GetAttemptedSpellTarget();") 375, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetAttemptedSpellTarget NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAttemptedSpellTarget) },
	{ NWSCRIPT_ACTIONNAME("GetLastOpenedBy") NWSCRIPT_ACTIONPROTOTYPE("object GetLastOpenedBy();") 376, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastOpenedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastOpenedBy) },
	{ NWSCRIPT_ACTIONNAME("GetHasSpell") NWSCRIPT_ACTIONPROTOTYPE("int GetHasSpell(int nSpell, object oCreature=OBJECT_SELF);") 377, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetHasSpell NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetHasSpell) },
	{ NWSCRIPT_ACTIONNAME("OpenStore") NWSCRIPT_ACTIONPROTOTYPE("void OpenStore(object oStore, object oPC, int nBonusMarkUp=0, int nBonusMarkDown=0);") 378, 2, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_OpenStore NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_OpenStore) },
	{ NWSCRIPT_ACTIONNAME("EffectTurned") NWSCRIPT_ACTIONPROTOTYPE("effect EffectTurned();") 379, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectTurned NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectTurned) },
	{ NWSCRIPT_ACTIONNAME("GetFirstFactionMember") NWSCRIPT_ACTIONPROTOTYPE("object GetFirstFactionMember(object oMemberOfFaction, int bPCOnly=TRUE);") 380, 1, 2, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetFirstFactionMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFirstFactionMember) },
	{ NWSCRIPT_ACTIONNAME("GetNextFactionMember") NWSCRIPT_ACTIONPROTOTYPE("object GetNextFactionMember(object oMemberOfFaction, int bPCOnly=TRUE);") 381, 1, 2, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetNextFactionMember NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetNextFactionMember) },
	{ NWSCRIPT_ACTIONNAME("ActionForceMoveToLocation") NWSCRIPT_ACTIONPROTOTYPE("void ActionForceMoveToLocation(location lDestination, int bRun=FALSE, float fTimeout=30.0f);") 382, 1, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionForceMoveToLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionForceMoveToLocation) },
	{ NWSCRIPT_ACTIONNAME("ActionForceMoveToObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionForceMoveToObject(object oMoveTo, int bRun=FALSE, float fRange=1.0f, float fTimeout=30.0f);") 383, 1, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionForceMoveToObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionForceMoveToObject) },
	{ NWSCRIPT_ACTIONNAME("GetJournalQuestExperience") NWSCRIPT_ACTIONPROTOTYPE("int GetJournalQuestExperience(string szPlotID);") 384, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetJournalQuestExperience NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetJournalQuestExperience) },
	{ NWSCRIPT_ACTIONNAME("JumpToObject") NWSCRIPT_ACTIONPROTOTYPE("void JumpToObject(object oToJumpTo, int nWalkStraightLineToPoint=1);") 385, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_JumpToObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_JumpToObject) },
	{ NWSCRIPT_ACTIONNAME("SetMapPinEnabled") NWSCRIPT_ACTIONPROTOTYPE("void SetMapPinEnabled(object oMapPin, int nEnabled);") 386, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetMapPinEnabled NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetMapPinEnabled) },
	{ NWSCRIPT_ACTIONNAME("EffectHitPointChangeWhenDying") NWSCRIPT_ACTIONPROTOTYPE("effect EffectHitPointChangeWhenDying(float fHitPointChangePerRound);") 387, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectHitPointChangeWhenDying NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectHitPointChangeWhenDying) },
	{ NWSCRIPT_ACTIONNAME("PopUpGUIPanel") NWSCRIPT_ACTIONPROTOTYPE("void PopUpGUIPanel(object oPC, int nGUIPanel);") 388, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_PopUpGUIPanel NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_PopUpGUIPanel) },
	{ NWSCRIPT_ACTIONNAME("ClearPersonalReputation") NWSCRIPT_ACTIONPROTOTYPE("void ClearPersonalReputation(object oTarget, object oSource=OBJECT_SELF);") 389, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ClearPersonalReputation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ClearPersonalReputation) },
	{ NWSCRIPT_ACTIONNAME("SetIsTemporaryFriend") NWSCRIPT_ACTIONPROTOTYPE("void SetIsTemporaryFriend(object oTarget, object oSource=OBJECT_SELF, int bDecays=FALSE, float fDurationInSeconds=180.0f);") 390, 1, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetIsTemporaryFriend NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetIsTemporaryFriend) },
	{ NWSCRIPT_ACTIONNAME("SetIsTemporaryEnemy") NWSCRIPT_ACTIONPROTOTYPE("void SetIsTemporaryEnemy(object oTarget, object oSource=OBJECT_SELF, int bDecays=FALSE, float fDurationInSeconds=180.0f);") 391, 1, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetIsTemporaryEnemy NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetIsTemporaryEnemy) },
	{ NWSCRIPT_ACTIONNAME("SetIsTemporaryNeutral") NWSCRIPT_ACTIONPROTOTYPE("void SetIsTemporaryNeutral(object oTarget, object oSource=OBJECT_SELF, int bDecays=FALSE, float fDurationInSeconds=180.0f);") 392, 1, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetIsTemporaryNeutral NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetIsTemporaryNeutral) },
	{ NWSCRIPT_ACTIONNAME("GiveXPToCreature") NWSCRIPT_ACTIONPROTOTYPE("void GiveXPToCreature(object oCreature, int nXpAmount);") 393, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_GiveXPToCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GiveXPToCreature) },
	{ NWSCRIPT_ACTIONNAME("SetXP") NWSCRIPT_ACTIONPROTOTYPE("void SetXP(object oCreature, int nXpAmount);") 394, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetXP NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetXP) },
	{ NWSCRIPT_ACTIONNAME("GetXP") NWSCRIPT_ACTIONPROTOTYPE("int GetXP(object oCreature);") 395, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetXP NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetXP) },
	{ NWSCRIPT_ACTIONNAME("IntToHexString") NWSCRIPT_ACTIONPROTOTYPE("string IntToHexString(int nInteger);") 396, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_IntToHexString NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_IntToHexString) },
	{ NWSCRIPT_ACTIONNAME("GetBaseItemType") NWSCRIPT_ACTIONPROTOTYPE("int GetBaseItemType(object oItem);") 397, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetBaseItemType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetBaseItemType) },
	{ NWSCRIPT_ACTIONNAME("GetItemHasItemProperty") NWSCRIPT_ACTIONPROTOTYPE("int GetItemHasItemProperty(object oItem, int nProperty);") 398, 2, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetItemHasItemProperty NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemHasItemProperty) },
	{ NWSCRIPT_ACTIONNAME("ActionEquipMostDamagingMelee") NWSCRIPT_ACTIONPROTOTYPE("void ActionEquipMostDamagingMelee(object oVersus=OBJECT_INVALID, int bOffHand=FALSE);") 399, 0, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionEquipMostDamagingMelee NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionEquipMostDamagingMelee) },
	{ NWSCRIPT_ACTIONNAME("ActionEquipMostDamagingRanged") NWSCRIPT_ACTIONPROTOTYPE("void ActionEquipMostDamagingRanged(object oVersus=OBJECT_INVALID);") 400, 0, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionEquipMostDamagingRanged NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionEquipMostDamagingRanged) },
	{ NWSCRIPT_ACTIONNAME("GetItemACValue") NWSCRIPT_ACTIONPROTOTYPE("int GetItemACValue(object oItem);") 401, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetItemACValue NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemACValue) },
	{ NWSCRIPT_ACTIONNAME("ActionRest") NWSCRIPT_ACTIONPROTOTYPE("void ActionRest(int bCreatureToEnemyLineOfSightCheck=FALSE);") 402, 0, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionRest NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionRest) },
	{ NWSCRIPT_ACTIONNAME("ExploreAreaForPlayer") NWSCRIPT_ACTIONPROTOTYPE("void ExploreAreaForPlayer(object oArea, object oPlayer, int bExplored=TRUE);") 403, 2, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ExploreAreaForPlayer NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ExploreAreaForPlayer) },
	{ NWSCRIPT_ACTIONNAME("ActionEquipMostEffectiveArmor") NWSCRIPT_ACTIONPROTOTYPE("void ActionEquipMostEffectiveArmor();") 404, 0, 0, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionEquipMostEffectiveArmor NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionEquipMostEffectiveArmor) },
	{ NWSCRIPT_ACTIONNAME("GetIsDay") NWSCRIPT_ACTIONPROTOTYPE("int GetIsDay();") 405, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsDay NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsDay) },
	{ NWSCRIPT_ACTIONNAME("GetIsNight") NWSCRIPT_ACTIONPROTOTYPE("int GetIsNight();") 406, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsNight NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsNight) },
	{ NWSCRIPT_ACTIONNAME("GetIsDawn") NWSCRIPT_ACTIONPROTOTYPE("int GetIsDawn();") 407, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsDawn NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsDawn) },
	{ NWSCRIPT_ACTIONNAME("GetIsDusk") NWSCRIPT_ACTIONPROTOTYPE("int GetIsDusk();") 408, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsDusk NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsDusk) },
	{ NWSCRIPT_ACTIONNAME("GetIsEncounterCreature") NWSCRIPT_ACTIONPROTOTYPE("int GetIsEncounterCreature(object oCreature=OBJECT_SELF);") 409, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsEncounterCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsEncounterCreature) },
	{ NWSCRIPT_ACTIONNAME("GetLastPlayerDying") NWSCRIPT_ACTIONPROTOTYPE("object GetLastPlayerDying();") 410, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastPlayerDying NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastPlayerDying) },
	{ NWSCRIPT_ACTIONNAME("GetStartingLocation") NWSCRIPT_ACTIONPROTOTYPE("location GetStartingLocation();") 411, 0, 0, ACTIONTYPE_LOCATION, NWN1_NWActionParameterTypes_GetStartingLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetStartingLocation) },
	{ NWSCRIPT_ACTIONNAME("ChangeToStandardFaction") NWSCRIPT_ACTIONPROTOTYPE("void ChangeToStandardFaction(object oCreatureToChange, int nStandardFaction);") 412, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ChangeToStandardFaction NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ChangeToStandardFaction) },
	{ NWSCRIPT_ACTIONNAME("SoundObjectPlay") NWSCRIPT_ACTIONPROTOTYPE("void SoundObjectPlay(object oSound);") 413, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SoundObjectPlay NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SoundObjectPlay) },
	{ NWSCRIPT_ACTIONNAME("SoundObjectStop") NWSCRIPT_ACTIONPROTOTYPE("void SoundObjectStop(object oSound);") 414, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SoundObjectStop NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SoundObjectStop) },
	{ NWSCRIPT_ACTIONNAME("SoundObjectSetVolume") NWSCRIPT_ACTIONPROTOTYPE("void SoundObjectSetVolume(object oSound, int nVolume);") 415, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SoundObjectSetVolume NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SoundObjectSetVolume) },
	{ NWSCRIPT_ACTIONNAME("SoundObjectSetPosition") NWSCRIPT_ACTIONPROTOTYPE("void SoundObjectSetPosition(object oSound, vector vPosition);") 416, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SoundObjectSetPosition NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SoundObjectSetPosition) },
	{ NWSCRIPT_ACTIONNAME("SpeakOneLinerConversation") NWSCRIPT_ACTIONPROTOTYPE("void SpeakOneLinerConversation(string sDialogResRef="", object oTokenTarget=OBJECT_TYPE_INVALID);") 417, 0, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SpeakOneLinerConversation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SpeakOneLinerConversation) },
	{ NWSCRIPT_ACTIONNAME("GetGold") NWSCRIPT_ACTIONPROTOTYPE("int GetGold(object oTarget=OBJECT_SELF);") 418, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetGold NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetGold) },
	{ NWSCRIPT_ACTIONNAME("GetLastRespawnButtonPresser") NWSCRIPT_ACTIONPROTOTYPE("object GetLastRespawnButtonPresser();") 419, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastRespawnButtonPresser NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastRespawnButtonPresser) },
	{ NWSCRIPT_ACTIONNAME("GetIsDM") NWSCRIPT_ACTIONPROTOTYPE("int GetIsDM(object oCreature);") 420, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsDM NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsDM) },
	{ NWSCRIPT_ACTIONNAME("PlayVoiceChat") NWSCRIPT_ACTIONPROTOTYPE("void PlayVoiceChat(int nVoiceChatID, object oTarget=OBJECT_SELF);") 421, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_PlayVoiceChat NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_PlayVoiceChat) },
	{ NWSCRIPT_ACTIONNAME("GetIsWeaponEffective") NWSCRIPT_ACTIONPROTOTYPE("int GetIsWeaponEffective(object oVersus=OBJECT_INVALID, int bOffHand=FALSE);") 422, 0, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsWeaponEffective NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsWeaponEffective) },
	{ NWSCRIPT_ACTIONNAME("GetLastSpellHarmful") NWSCRIPT_ACTIONPROTOTYPE("int GetLastSpellHarmful();") 423, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLastSpellHarmful NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastSpellHarmful) },
	{ NWSCRIPT_ACTIONNAME("EventActivateItem") NWSCRIPT_ACTIONPROTOTYPE("event EventActivateItem(object oItem, location lTarget, object oTarget=OBJECT_INVALID);") 424, 2, 3, ACTIONTYPE_EVENT, NWN1_NWActionParameterTypes_EventActivateItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EventActivateItem) },
	{ NWSCRIPT_ACTIONNAME("MusicBackgroundPlay") NWSCRIPT_ACTIONPROTOTYPE("void MusicBackgroundPlay(object oArea);") 425, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_MusicBackgroundPlay NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_MusicBackgroundPlay) },
	{ NWSCRIPT_ACTIONNAME("MusicBackgroundStop") NWSCRIPT_ACTIONPROTOTYPE("void MusicBackgroundStop(object oArea);") 426, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_MusicBackgroundStop NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_MusicBackgroundStop) },
	{ NWSCRIPT_ACTIONNAME("MusicBackgroundSetDelay") NWSCRIPT_ACTIONPROTOTYPE("void MusicBackgroundSetDelay(object oArea, int nDelay);") 427, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_MusicBackgroundSetDelay NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_MusicBackgroundSetDelay) },
	{ NWSCRIPT_ACTIONNAME("MusicBackgroundChangeDay") NWSCRIPT_ACTIONPROTOTYPE("void MusicBackgroundChangeDay(object oArea, int nTrack);") 428, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_MusicBackgroundChangeDay NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_MusicBackgroundChangeDay) },
	{ NWSCRIPT_ACTIONNAME("MusicBackgroundChangeNight") NWSCRIPT_ACTIONPROTOTYPE("void MusicBackgroundChangeNight(object oArea, int nTrack);") 429, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_MusicBackgroundChangeNight NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_MusicBackgroundChangeNight) },
	{ NWSCRIPT_ACTIONNAME("MusicBattlePlay") NWSCRIPT_ACTIONPROTOTYPE("void MusicBattlePlay(object oArea);") 430, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_MusicBattlePlay NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_MusicBattlePlay) },
	{ NWSCRIPT_ACTIONNAME("MusicBattleStop") NWSCRIPT_ACTIONPROTOTYPE("void MusicBattleStop(object oArea);") 431, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_MusicBattleStop NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_MusicBattleStop) },
	{ NWSCRIPT_ACTIONNAME("MusicBattleChange") NWSCRIPT_ACTIONPROTOTYPE("void MusicBattleChange(object oArea, int nTrack);") 432, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_MusicBattleChange NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_MusicBattleChange) },
	{ NWSCRIPT_ACTIONNAME("AmbientSoundPlay") NWSCRIPT_ACTIONPROTOTYPE("void AmbientSoundPlay(object oArea);") 433, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_AmbientSoundPlay NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_AmbientSoundPlay) },
	{ NWSCRIPT_ACTIONNAME("AmbientSoundStop") NWSCRIPT_ACTIONPROTOTYPE("void AmbientSoundStop(object oArea);") 434, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_AmbientSoundStop NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_AmbientSoundStop) },
	{ NWSCRIPT_ACTIONNAME("AmbientSoundChangeDay") NWSCRIPT_ACTIONPROTOTYPE("void AmbientSoundChangeDay(object oArea, int nTrack);") 435, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_AmbientSoundChangeDay NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_AmbientSoundChangeDay) },
	{ NWSCRIPT_ACTIONNAME("AmbientSoundChangeNight") NWSCRIPT_ACTIONPROTOTYPE("void AmbientSoundChangeNight(object oArea, int nTrack);") 436, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_AmbientSoundChangeNight NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_AmbientSoundChangeNight) },
	{ NWSCRIPT_ACTIONNAME("GetLastKiller") NWSCRIPT_ACTIONPROTOTYPE("object GetLastKiller();") 437, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastKiller NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastKiller) },
	{ NWSCRIPT_ACTIONNAME("GetSpellCastItem") NWSCRIPT_ACTIONPROTOTYPE("object GetSpellCastItem();") 438, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetSpellCastItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetSpellCastItem) },
	{ NWSCRIPT_ACTIONNAME("GetItemActivated") NWSCRIPT_ACTIONPROTOTYPE("object GetItemActivated();") 439, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetItemActivated NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemActivated) },
	{ NWSCRIPT_ACTIONNAME("GetItemActivator") NWSCRIPT_ACTIONPROTOTYPE("object GetItemActivator();") 440, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetItemActivator NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemActivator) },
	{ NWSCRIPT_ACTIONNAME("GetItemActivatedTargetLocation") NWSCRIPT_ACTIONPROTOTYPE("location GetItemActivatedTargetLocation();") 441, 0, 0, ACTIONTYPE_LOCATION, NWN1_NWActionParameterTypes_GetItemActivatedTargetLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemActivatedTargetLocation) },
	{ NWSCRIPT_ACTIONNAME("GetItemActivatedTarget") NWSCRIPT_ACTIONPROTOTYPE("object GetItemActivatedTarget();") 442, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetItemActivatedTarget NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemActivatedTarget) },
	{ NWSCRIPT_ACTIONNAME("GetIsOpen") NWSCRIPT_ACTIONPROTOTYPE("int GetIsOpen(object oObject);") 443, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsOpen NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsOpen) },
	{ NWSCRIPT_ACTIONNAME("TakeGoldFromCreature") NWSCRIPT_ACTIONPROTOTYPE("void TakeGoldFromCreature(int nAmount, object oCreatureToTakeFrom, int bDestroy=FALSE);") 444, 2, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_TakeGoldFromCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_TakeGoldFromCreature) },
	{ NWSCRIPT_ACTIONNAME("IsInConversation") NWSCRIPT_ACTIONPROTOTYPE("int IsInConversation(object oObject);") 445, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_IsInConversation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_IsInConversation) },
	{ NWSCRIPT_ACTIONNAME("EffectAbilityDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectAbilityDecrease(int nAbility, int nModifyBy);") 446, 2, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectAbilityDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectAbilityDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectAttackDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectAttackDecrease(int nPenalty, int nModifierType=ATTACK_BONUS_MISC);") 447, 1, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectAttackDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectAttackDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectDamageDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamageDecrease(int nPenalty, int nDamageType=DAMAGE_TYPE_MAGICAL);") 448, 1, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDamageDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDamageDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectDamageImmunityDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamageImmunityDecrease(int nDamageType, int nPercentImmunity);") 449, 2, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDamageImmunityDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDamageImmunityDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectACDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectACDecrease(int nValue, int nModifyType=AC_DODGE_BONUS, int nDamageType=AC_VS_DAMAGE_TYPE_ALL);") 450, 1, 3, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectACDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectACDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectMovementSpeedDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectMovementSpeedDecrease(int nPercentChange);") 451, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectMovementSpeedDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectMovementSpeedDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectSavingThrowDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSavingThrowDecrease(int nSave, int nValue, int nSaveType=SAVING_THROW_TYPE_ALL);") 452, 2, 3, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectSavingThrowDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectSavingThrowDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectSkillDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSkillDecrease(int nSkill, int nValue);") 453, 2, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectSkillDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectSkillDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectSpellResistanceDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSpellResistanceDecrease(int nValue);") 454, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectSpellResistanceDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectSpellResistanceDecrease) },
	{ NWSCRIPT_ACTIONNAME("GetPlotFlag") NWSCRIPT_ACTIONPROTOTYPE("int GetPlotFlag(object oTarget=OBJECT_SELF);") 455, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetPlotFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPlotFlag) },
	{ NWSCRIPT_ACTIONNAME("SetPlotFlag") NWSCRIPT_ACTIONPROTOTYPE("void SetPlotFlag(object oTarget, int nPlotFlag);") 456, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetPlotFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetPlotFlag) },
	{ NWSCRIPT_ACTIONNAME("EffectInvisibility") NWSCRIPT_ACTIONPROTOTYPE("effect EffectInvisibility(int nInvisibilityType);") 457, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectInvisibility NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectInvisibility) },
	{ NWSCRIPT_ACTIONNAME("EffectConcealment") NWSCRIPT_ACTIONPROTOTYPE("effect EffectConcealment(int nPercentage, int nMissType=MISS_CHANCE_TYPE_NORMAL);") 458, 1, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectConcealment NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectConcealment) },
	{ NWSCRIPT_ACTIONNAME("EffectDarkness") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDarkness();") 459, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDarkness NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDarkness) },
	{ NWSCRIPT_ACTIONNAME("EffectDispelMagicAll") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDispelMagicAll(int nCasterLevel=USE_CREATURE_LEVEL);") 460, 0, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDispelMagicAll NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDispelMagicAll) },
	{ NWSCRIPT_ACTIONNAME("EffectUltravision") NWSCRIPT_ACTIONPROTOTYPE("effect EffectUltravision();") 461, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectUltravision NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectUltravision) },
	{ NWSCRIPT_ACTIONNAME("EffectNegativeLevel") NWSCRIPT_ACTIONPROTOTYPE("effect EffectNegativeLevel(int nNumLevels, int bHPBonus=FALSE);") 462, 1, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectNegativeLevel NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectNegativeLevel) },
	{ NWSCRIPT_ACTIONNAME("EffectPolymorph") NWSCRIPT_ACTIONPROTOTYPE("effect EffectPolymorph(int nPolymorphSelection, int nLocked=FALSE);") 463, 1, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectPolymorph NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectPolymorph) },
	{ NWSCRIPT_ACTIONNAME("EffectSanctuary") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSanctuary(int nDifficultyClass);") 464, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectSanctuary NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectSanctuary) },
	{ NWSCRIPT_ACTIONNAME("EffectTrueSeeing") NWSCRIPT_ACTIONPROTOTYPE("effect EffectTrueSeeing();") 465, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectTrueSeeing NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectTrueSeeing) },
	{ NWSCRIPT_ACTIONNAME("EffectSeeInvisible") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSeeInvisible();") 466, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectSeeInvisible NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectSeeInvisible) },
	{ NWSCRIPT_ACTIONNAME("EffectTimeStop") NWSCRIPT_ACTIONPROTOTYPE("effect EffectTimeStop();") 467, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectTimeStop NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectTimeStop) },
	{ NWSCRIPT_ACTIONNAME("EffectBlindness") NWSCRIPT_ACTIONPROTOTYPE("effect EffectBlindness();") 468, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectBlindness NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectBlindness) },
	{ NWSCRIPT_ACTIONNAME("GetIsReactionTypeFriendly") NWSCRIPT_ACTIONPROTOTYPE("int GetIsReactionTypeFriendly(object oTarget, object oSource=OBJECT_SELF);") 469, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsReactionTypeFriendly NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsReactionTypeFriendly) },
	{ NWSCRIPT_ACTIONNAME("GetIsReactionTypeNeutral") NWSCRIPT_ACTIONPROTOTYPE("int GetIsReactionTypeNeutral(object oTarget, object oSource=OBJECT_SELF);") 470, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsReactionTypeNeutral NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsReactionTypeNeutral) },
	{ NWSCRIPT_ACTIONNAME("GetIsReactionTypeHostile") NWSCRIPT_ACTIONPROTOTYPE("int GetIsReactionTypeHostile(object oTarget, object oSource=OBJECT_SELF);") 471, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsReactionTypeHostile NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsReactionTypeHostile) },
	{ NWSCRIPT_ACTIONNAME("EffectSpellLevelAbsorption") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSpellLevelAbsorption(int nMaxSpellLevelAbsorbed, int nTotalSpellLevelsAbsorbed=0, int nSpellSchool=SPELL_SCHOOL_GENERAL );") 472, 1, 3, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectSpellLevelAbsorption NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectSpellLevelAbsorption) },
	{ NWSCRIPT_ACTIONNAME("EffectDispelMagicBest") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDispelMagicBest(int nCasterLevel=USE_CREATURE_LEVEL);") 473, 0, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDispelMagicBest NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDispelMagicBest) },
	{ NWSCRIPT_ACTIONNAME("ActivatePortal") NWSCRIPT_ACTIONPROTOTYPE("void ActivatePortal(object oTarget, string sIPaddress="", string sPassword="", string sWaypointTag="", int bSeemless=FALSE);") 474, 1, 5, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActivatePortal NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActivatePortal) },
	{ NWSCRIPT_ACTIONNAME("GetNumStackedItems") NWSCRIPT_ACTIONPROTOTYPE("int GetNumStackedItems(object oItem);") 475, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetNumStackedItems NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetNumStackedItems) },
	{ NWSCRIPT_ACTIONNAME("SurrenderToEnemies") NWSCRIPT_ACTIONPROTOTYPE("void SurrenderToEnemies();") 476, 0, 0, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SurrenderToEnemies NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SurrenderToEnemies) },
	{ NWSCRIPT_ACTIONNAME("EffectMissChance") NWSCRIPT_ACTIONPROTOTYPE("effect EffectMissChance(int nPercentage, int nMissChanceType=MISS_CHANCE_TYPE_NORMAL);") 477, 1, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectMissChance NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectMissChance) },
	{ NWSCRIPT_ACTIONNAME("GetTurnResistanceHD") NWSCRIPT_ACTIONPROTOTYPE("int GetTurnResistanceHD(object oUndead=OBJECT_SELF);") 478, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTurnResistanceHD NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTurnResistanceHD) },
	{ NWSCRIPT_ACTIONNAME("GetCreatureSize") NWSCRIPT_ACTIONPROTOTYPE("int GetCreatureSize(object oCreature);") 479, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetCreatureSize NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCreatureSize) },
	{ NWSCRIPT_ACTIONNAME("EffectDisappearAppear") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDisappearAppear(location lLocation, int nAnimation=1);") 480, 1, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDisappearAppear NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDisappearAppear) },
	{ NWSCRIPT_ACTIONNAME("EffectDisappear") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDisappear(int nAnimation=1);") 481, 0, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDisappear NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDisappear) },
	{ NWSCRIPT_ACTIONNAME("EffectAppear") NWSCRIPT_ACTIONPROTOTYPE("effect EffectAppear(int nAnimation=1);") 482, 0, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectAppear NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectAppear) },
	{ NWSCRIPT_ACTIONNAME("ActionUnlockObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionUnlockObject(object oTarget);") 483, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionUnlockObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionUnlockObject) },
	{ NWSCRIPT_ACTIONNAME("ActionLockObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionLockObject(object oTarget);") 484, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionLockObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionLockObject) },
	{ NWSCRIPT_ACTIONNAME("EffectModifyAttacks") NWSCRIPT_ACTIONPROTOTYPE("effect EffectModifyAttacks(int nAttacks);") 485, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectModifyAttacks NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectModifyAttacks) },
	{ NWSCRIPT_ACTIONNAME("GetLastTrapDetected") NWSCRIPT_ACTIONPROTOTYPE("object GetLastTrapDetected(object oTarget=OBJECT_SELF);") 486, 0, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastTrapDetected NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastTrapDetected) },
	{ NWSCRIPT_ACTIONNAME("EffectDamageShield") NWSCRIPT_ACTIONPROTOTYPE("effect EffectDamageShield(int nDamageAmount, int nRandomAmount, int nDamageType);") 487, 3, 3, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectDamageShield NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectDamageShield) },
	{ NWSCRIPT_ACTIONNAME("GetNearestTrapToObject") NWSCRIPT_ACTIONPROTOTYPE("object GetNearestTrapToObject(object oTarget=OBJECT_SELF, int nTrapDetected=TRUE);") 488, 0, 2, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetNearestTrapToObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetNearestTrapToObject) },
	{ NWSCRIPT_ACTIONNAME("GetDeity") NWSCRIPT_ACTIONPROTOTYPE("string GetDeity(object oCreature);") 489, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetDeity NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetDeity) },
	{ NWSCRIPT_ACTIONNAME("GetSubRace") NWSCRIPT_ACTIONPROTOTYPE("string GetSubRace(object oTarget);") 490, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetSubRace NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetSubRace) },
	{ NWSCRIPT_ACTIONNAME("GetFortitudeSavingThrow") NWSCRIPT_ACTIONPROTOTYPE("int GetFortitudeSavingThrow(object oTarget);") 491, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetFortitudeSavingThrow NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFortitudeSavingThrow) },
	{ NWSCRIPT_ACTIONNAME("GetWillSavingThrow") NWSCRIPT_ACTIONPROTOTYPE("int GetWillSavingThrow(object oTarget);") 492, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetWillSavingThrow NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetWillSavingThrow) },
	{ NWSCRIPT_ACTIONNAME("GetReflexSavingThrow") NWSCRIPT_ACTIONPROTOTYPE("int GetReflexSavingThrow(object oTarget);") 493, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetReflexSavingThrow NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetReflexSavingThrow) },
	{ NWSCRIPT_ACTIONNAME("GetChallengeRating") NWSCRIPT_ACTIONPROTOTYPE("float GetChallengeRating(object oCreature);") 494, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_GetChallengeRating NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetChallengeRating) },
	{ NWSCRIPT_ACTIONNAME("GetAge") NWSCRIPT_ACTIONPROTOTYPE("int GetAge(object oCreature);") 495, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetAge NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAge) },
	{ NWSCRIPT_ACTIONNAME("GetMovementRate") NWSCRIPT_ACTIONPROTOTYPE("int GetMovementRate(object oCreature);") 496, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetMovementRate NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetMovementRate) },
	{ NWSCRIPT_ACTIONNAME("GetFamiliarCreatureType") NWSCRIPT_ACTIONPROTOTYPE("int GetFamiliarCreatureType(object oCreature);") 497, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetFamiliarCreatureType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFamiliarCreatureType) },
	{ NWSCRIPT_ACTIONNAME("GetAnimalCompanionCreatureType") NWSCRIPT_ACTIONPROTOTYPE("int GetAnimalCompanionCreatureType(object oCreature);") 498, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetAnimalCompanionCreatureType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAnimalCompanionCreatureType) },
	{ NWSCRIPT_ACTIONNAME("GetFamiliarName") NWSCRIPT_ACTIONPROTOTYPE("string GetFamiliarName(object oCreature);") 499, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetFamiliarName NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFamiliarName) },
	{ NWSCRIPT_ACTIONNAME("GetAnimalCompanionName") NWSCRIPT_ACTIONPROTOTYPE("string GetAnimalCompanionName(object oTarget);") 500, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetAnimalCompanionName NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAnimalCompanionName) },
	{ NWSCRIPT_ACTIONNAME("ActionCastFakeSpellAtObject") NWSCRIPT_ACTIONPROTOTYPE("void ActionCastFakeSpellAtObject(int nSpell, object oTarget, int nProjectilePathType=PROJECTILE_PATH_TYPE_DEFAULT);") 501, 2, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionCastFakeSpellAtObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionCastFakeSpellAtObject) },
	{ NWSCRIPT_ACTIONNAME("ActionCastFakeSpellAtLocation") NWSCRIPT_ACTIONPROTOTYPE("void ActionCastFakeSpellAtLocation(int nSpell, location lTarget, int nProjectilePathType=PROJECTILE_PATH_TYPE_DEFAULT);") 502, 2, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionCastFakeSpellAtLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionCastFakeSpellAtLocation) },
	{ NWSCRIPT_ACTIONNAME("RemoveSummonedAssociate") NWSCRIPT_ACTIONPROTOTYPE("void RemoveSummonedAssociate(object oMaster, object oAssociate=OBJECT_SELF);") 503, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_RemoveSummonedAssociate NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_RemoveSummonedAssociate) },
	{ NWSCRIPT_ACTIONNAME("SetCameraMode") NWSCRIPT_ACTIONPROTOTYPE("void SetCameraMode(object oPlayer, int nCameraMode);") 504, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetCameraMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetCameraMode) },
	{ NWSCRIPT_ACTIONNAME("GetIsResting") NWSCRIPT_ACTIONPROTOTYPE("int GetIsResting(object oCreature=OBJECT_SELF);") 505, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsResting NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsResting) },
	{ NWSCRIPT_ACTIONNAME("GetLastPCRested") NWSCRIPT_ACTIONPROTOTYPE("object GetLastPCRested();") 506, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastPCRested NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastPCRested) },
	{ NWSCRIPT_ACTIONNAME("SetWeather") NWSCRIPT_ACTIONPROTOTYPE("void SetWeather(object oTarget, int nWeather);") 507, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetWeather NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetWeather) },
	{ NWSCRIPT_ACTIONNAME("GetLastRestEventType") NWSCRIPT_ACTIONPROTOTYPE("int GetLastRestEventType();") 508, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLastRestEventType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastRestEventType) },
	{ NWSCRIPT_ACTIONNAME("StartNewModule") NWSCRIPT_ACTIONPROTOTYPE("void StartNewModule(string sModuleName);") 509, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_StartNewModule NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_StartNewModule) },
	{ NWSCRIPT_ACTIONNAME("EffectSwarm") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSwarm(int nLooping, string sCreatureTemplate1, string sCreatureTemplate2="", string sCreatureTemplate3="", string sCreatureTemplate4="");") 510, 2, 5, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectSwarm NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectSwarm) },
	{ NWSCRIPT_ACTIONNAME("GetWeaponRanged") NWSCRIPT_ACTIONPROTOTYPE("int GetWeaponRanged(object oItem);") 511, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetWeaponRanged NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetWeaponRanged) },
	{ NWSCRIPT_ACTIONNAME("DoSinglePlayerAutoSave") NWSCRIPT_ACTIONPROTOTYPE("void DoSinglePlayerAutoSave();") 512, 0, 0, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_DoSinglePlayerAutoSave NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_DoSinglePlayerAutoSave) },
	{ NWSCRIPT_ACTIONNAME("GetGameDifficulty") NWSCRIPT_ACTIONPROTOTYPE("int GetGameDifficulty();") 513, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetGameDifficulty NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetGameDifficulty) },
	{ NWSCRIPT_ACTIONNAME("SetTileMainLightColor") NWSCRIPT_ACTIONPROTOTYPE("void SetTileMainLightColor(location lTileLocation, int nMainLight1Color, int nMainLight2Color);") 514, 3, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetTileMainLightColor NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetTileMainLightColor) },
	{ NWSCRIPT_ACTIONNAME("SetTileSourceLightColor") NWSCRIPT_ACTIONPROTOTYPE("void SetTileSourceLightColor(location lTileLocation, int nSourceLight1Color, int nSourceLight2Color);") 515, 3, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetTileSourceLightColor NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetTileSourceLightColor) },
	{ NWSCRIPT_ACTIONNAME("RecomputeStaticLighting") NWSCRIPT_ACTIONPROTOTYPE("void RecomputeStaticLighting(object oArea);") 516, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_RecomputeStaticLighting NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_RecomputeStaticLighting) },
	{ NWSCRIPT_ACTIONNAME("GetTileMainLight1Color") NWSCRIPT_ACTIONPROTOTYPE("int GetTileMainLight1Color(location lTile);") 517, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTileMainLight1Color NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTileMainLight1Color) },
	{ NWSCRIPT_ACTIONNAME("GetTileMainLight2Color") NWSCRIPT_ACTIONPROTOTYPE("int GetTileMainLight2Color(location lTile);") 518, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTileMainLight2Color NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTileMainLight2Color) },
	{ NWSCRIPT_ACTIONNAME("GetTileSourceLight1Color") NWSCRIPT_ACTIONPROTOTYPE("int GetTileSourceLight1Color(location lTile);") 519, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTileSourceLight1Color NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTileSourceLight1Color) },
	{ NWSCRIPT_ACTIONNAME("GetTileSourceLight2Color") NWSCRIPT_ACTIONPROTOTYPE("int GetTileSourceLight2Color(location lTile);") 520, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTileSourceLight2Color NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTileSourceLight2Color) },
	{ NWSCRIPT_ACTIONNAME("SetPanelButtonFlash") NWSCRIPT_ACTIONPROTOTYPE("void SetPanelButtonFlash(object oPlayer, int nButton, int nEnableFlash);") 521, 3, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetPanelButtonFlash NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetPanelButtonFlash) },
	{ NWSCRIPT_ACTIONNAME("GetCurrentAction") NWSCRIPT_ACTIONPROTOTYPE("int GetCurrentAction(object oObject=OBJECT_SELF);") 522, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetCurrentAction NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCurrentAction) },
	{ NWSCRIPT_ACTIONNAME("SetStandardFactionReputation") NWSCRIPT_ACTIONPROTOTYPE("void SetStandardFactionReputation(int nStandardFaction, int nNewReputation, object oCreature=OBJECT_SELF);") 523, 2, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetStandardFactionReputation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetStandardFactionReputation) },
	{ NWSCRIPT_ACTIONNAME("GetStandardFactionReputation") NWSCRIPT_ACTIONPROTOTYPE("int GetStandardFactionReputation(int nStandardFaction, object oCreature=OBJECT_SELF);") 524, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetStandardFactionReputation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetStandardFactionReputation) },
	{ NWSCRIPT_ACTIONNAME("FloatingTextStrRefOnCreature") NWSCRIPT_ACTIONPROTOTYPE("void FloatingTextStrRefOnCreature(int nStrRefToDisplay, object oCreatureToFloatAbove, int bBroadcastToFaction=TRUE);") 525, 2, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_FloatingTextStrRefOnCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_FloatingTextStrRefOnCreature) },
	{ NWSCRIPT_ACTIONNAME("FloatingTextStringOnCreature") NWSCRIPT_ACTIONPROTOTYPE("void FloatingTextStringOnCreature(string sStringToDisplay, object oCreatureToFloatAbove, int bBroadcastToFaction=TRUE);") 526, 2, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_FloatingTextStringOnCreature NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_FloatingTextStringOnCreature) },
	{ NWSCRIPT_ACTIONNAME("GetTrapDisarmable") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapDisarmable(object oTrapObject);") 527, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTrapDisarmable NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTrapDisarmable) },
	{ NWSCRIPT_ACTIONNAME("GetTrapDetectable") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapDetectable(object oTrapObject);") 528, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTrapDetectable NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTrapDetectable) },
	{ NWSCRIPT_ACTIONNAME("GetTrapDetectedBy") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapDetectedBy(object oTrapObject, object oCreature);") 529, 2, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTrapDetectedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTrapDetectedBy) },
	{ NWSCRIPT_ACTIONNAME("GetTrapFlagged") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapFlagged(object oTrapObject);") 530, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTrapFlagged NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTrapFlagged) },
	{ NWSCRIPT_ACTIONNAME("GetTrapBaseType") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapBaseType(object oTrapObject);") 531, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTrapBaseType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTrapBaseType) },
	{ NWSCRIPT_ACTIONNAME("GetTrapOneShot") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapOneShot(object oTrapObject);") 532, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTrapOneShot NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTrapOneShot) },
	{ NWSCRIPT_ACTIONNAME("GetTrapCreator") NWSCRIPT_ACTIONPROTOTYPE("object GetTrapCreator(object oTrapObject);") 533, 1, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetTrapCreator NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTrapCreator) },
	{ NWSCRIPT_ACTIONNAME("GetTrapKeyTag") NWSCRIPT_ACTIONPROTOTYPE("string GetTrapKeyTag(object oTrapObject);") 534, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetTrapKeyTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTrapKeyTag) },
	{ NWSCRIPT_ACTIONNAME("GetTrapDisarmDC") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapDisarmDC(object oTrapObject);") 535, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTrapDisarmDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTrapDisarmDC) },
	{ NWSCRIPT_ACTIONNAME("GetTrapDetectDC") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapDetectDC(object oTrapObject);") 536, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTrapDetectDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTrapDetectDC) },
	{ NWSCRIPT_ACTIONNAME("GetLockKeyRequired") NWSCRIPT_ACTIONPROTOTYPE("int GetLockKeyRequired(object oObject);") 537, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLockKeyRequired NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLockKeyRequired) },
	{ NWSCRIPT_ACTIONNAME("GetLockKeyTag") NWSCRIPT_ACTIONPROTOTYPE("string GetLockKeyTag(object oObject);") 538, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetLockKeyTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLockKeyTag) },
	{ NWSCRIPT_ACTIONNAME("GetLockLockable") NWSCRIPT_ACTIONPROTOTYPE("int GetLockLockable(object oObject);") 539, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLockLockable NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLockLockable) },
	{ NWSCRIPT_ACTIONNAME("GetLockUnlockDC") NWSCRIPT_ACTIONPROTOTYPE("int GetLockUnlockDC(object oObject);") 540, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLockUnlockDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLockUnlockDC) },
	{ NWSCRIPT_ACTIONNAME("GetLockLockDC") NWSCRIPT_ACTIONPROTOTYPE("int GetLockLockDC(object oObject);") 541, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLockLockDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLockLockDC) },
	{ NWSCRIPT_ACTIONNAME("GetPCLevellingUp") NWSCRIPT_ACTIONPROTOTYPE("object GetPCLevellingUp();") 542, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetPCLevellingUp NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPCLevellingUp) },
	{ NWSCRIPT_ACTIONNAME("GetHasFeatEffect") NWSCRIPT_ACTIONPROTOTYPE("int GetHasFeatEffect(int nFeat, object oObject=OBJECT_SELF);") 543, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetHasFeatEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetHasFeatEffect) },
	{ NWSCRIPT_ACTIONNAME("SetPlaceableIllumination") NWSCRIPT_ACTIONPROTOTYPE("void SetPlaceableIllumination(object oPlaceable=OBJECT_SELF, int bIlluminate=TRUE);") 544, 0, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetPlaceableIllumination NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetPlaceableIllumination) },
	{ NWSCRIPT_ACTIONNAME("GetPlaceableIllumination") NWSCRIPT_ACTIONPROTOTYPE("int GetPlaceableIllumination(object oPlaceable=OBJECT_SELF);") 545, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetPlaceableIllumination NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPlaceableIllumination) },
	{ NWSCRIPT_ACTIONNAME("GetIsPlaceableObjectActionPossible") NWSCRIPT_ACTIONPROTOTYPE("int GetIsPlaceableObjectActionPossible(object oPlaceable, int nPlaceableAction);") 546, 2, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsPlaceableObjectActionPossible NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsPlaceableObjectActionPossible) },
	{ NWSCRIPT_ACTIONNAME("DoPlaceableObjectAction") NWSCRIPT_ACTIONPROTOTYPE("void DoPlaceableObjectAction(object oPlaceable, int nPlaceableAction);") 547, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_DoPlaceableObjectAction NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_DoPlaceableObjectAction) },
	{ NWSCRIPT_ACTIONNAME("GetFirstPC") NWSCRIPT_ACTIONPROTOTYPE("object GetFirstPC();") 548, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetFirstPC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFirstPC) },
	{ NWSCRIPT_ACTIONNAME("GetNextPC") NWSCRIPT_ACTIONPROTOTYPE("object GetNextPC();") 549, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetNextPC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetNextPC) },
	{ NWSCRIPT_ACTIONNAME("SetTrapDetectedBy") NWSCRIPT_ACTIONPROTOTYPE("int SetTrapDetectedBy(object oTrap, object oDetector, int bDetected=TRUE);") 550, 2, 3, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_SetTrapDetectedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetTrapDetectedBy) },
	{ NWSCRIPT_ACTIONNAME("GetIsTrapped") NWSCRIPT_ACTIONPROTOTYPE("int GetIsTrapped(object oObject);") 551, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsTrapped NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsTrapped) },
	{ NWSCRIPT_ACTIONNAME("EffectTurnResistanceDecrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectTurnResistanceDecrease(int nHitDice);") 552, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectTurnResistanceDecrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectTurnResistanceDecrease) },
	{ NWSCRIPT_ACTIONNAME("EffectTurnResistanceIncrease") NWSCRIPT_ACTIONPROTOTYPE("effect EffectTurnResistanceIncrease(int nHitDice);") 553, 1, 1, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectTurnResistanceIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectTurnResistanceIncrease) },
	{ NWSCRIPT_ACTIONNAME("PopUpDeathGUIPanel") NWSCRIPT_ACTIONPROTOTYPE("void PopUpDeathGUIPanel(object oPC, int bRespawnButtonEnabled=TRUE, int bWaitForHelpButtonEnabled=TRUE, int nHelpStringReference=0, string sHelpString="");") 554, 1, 5, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_PopUpDeathGUIPanel NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_PopUpDeathGUIPanel) },
	{ NWSCRIPT_ACTIONNAME("SetTrapDisabled") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapDisabled(object oTrap);") 555, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetTrapDisabled NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetTrapDisabled) },
	{ NWSCRIPT_ACTIONNAME("GetLastHostileActor") NWSCRIPT_ACTIONPROTOTYPE("object GetLastHostileActor(object oVictim=OBJECT_SELF);") 556, 0, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastHostileActor NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastHostileActor) },
	{ NWSCRIPT_ACTIONNAME("ExportAllCharacters") NWSCRIPT_ACTIONPROTOTYPE("void ExportAllCharacters();") 557, 0, 0, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ExportAllCharacters NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ExportAllCharacters) },
	{ NWSCRIPT_ACTIONNAME("MusicBackgroundGetDayTrack") NWSCRIPT_ACTIONPROTOTYPE("int MusicBackgroundGetDayTrack(object oArea);") 558, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_MusicBackgroundGetDayTrack NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_MusicBackgroundGetDayTrack) },
	{ NWSCRIPT_ACTIONNAME("MusicBackgroundGetNightTrack") NWSCRIPT_ACTIONPROTOTYPE("int MusicBackgroundGetNightTrack(object oArea);") 559, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_MusicBackgroundGetNightTrack NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_MusicBackgroundGetNightTrack) },
	{ NWSCRIPT_ACTIONNAME("WriteTimestampedLogEntry") NWSCRIPT_ACTIONPROTOTYPE("void WriteTimestampedLogEntry(string sLogEntry);") 560, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_WriteTimestampedLogEntry NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_WriteTimestampedLogEntry) },
	{ NWSCRIPT_ACTIONNAME("GetModuleName") NWSCRIPT_ACTIONPROTOTYPE("string GetModuleName();") 561, 0, 0, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetModuleName NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetModuleName) },
	{ NWSCRIPT_ACTIONNAME("GetFactionLeader") NWSCRIPT_ACTIONPROTOTYPE("object GetFactionLeader(object oMemberOfFaction);") 562, 1, 1, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetFactionLeader NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFactionLeader) },
	{ NWSCRIPT_ACTIONNAME("SendMessageToAllDMs") NWSCRIPT_ACTIONPROTOTYPE("void SendMessageToAllDMs(string szMessage);") 563, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SendMessageToAllDMs NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SendMessageToAllDMs) },
	{ NWSCRIPT_ACTIONNAME("EndGame") NWSCRIPT_ACTIONPROTOTYPE("void EndGame(string sEndMovie);") 564, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_EndGame NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EndGame) },
	{ NWSCRIPT_ACTIONNAME("BootPC") NWSCRIPT_ACTIONPROTOTYPE("void BootPC(object oPlayer);") 565, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_BootPC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_BootPC) },
	{ NWSCRIPT_ACTIONNAME("ActionCounterSpell") NWSCRIPT_ACTIONPROTOTYPE("void ActionCounterSpell(object oCounterSpellTarget);") 566, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionCounterSpell NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionCounterSpell) },
	{ NWSCRIPT_ACTIONNAME("AmbientSoundSetDayVolume") NWSCRIPT_ACTIONPROTOTYPE("void AmbientSoundSetDayVolume(object oArea, int nVolume);") 567, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_AmbientSoundSetDayVolume NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_AmbientSoundSetDayVolume) },
	{ NWSCRIPT_ACTIONNAME("AmbientSoundSetNightVolume") NWSCRIPT_ACTIONPROTOTYPE("void AmbientSoundSetNightVolume(object oArea, int nVolume);") 568, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_AmbientSoundSetNightVolume NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_AmbientSoundSetNightVolume) },
	{ NWSCRIPT_ACTIONNAME("MusicBackgroundGetBattleTrack") NWSCRIPT_ACTIONPROTOTYPE("int MusicBackgroundGetBattleTrack(object oArea);") 569, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_MusicBackgroundGetBattleTrack NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_MusicBackgroundGetBattleTrack) },
	{ NWSCRIPT_ACTIONNAME("GetHasInventory") NWSCRIPT_ACTIONPROTOTYPE("int GetHasInventory(object oObject);") 570, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetHasInventory NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetHasInventory) },
	{ NWSCRIPT_ACTIONNAME("GetStrRefSoundDuration") NWSCRIPT_ACTIONPROTOTYPE("float GetStrRefSoundDuration(int nStrRef);") 571, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_GetStrRefSoundDuration NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetStrRefSoundDuration) },
	{ NWSCRIPT_ACTIONNAME("AddToParty") NWSCRIPT_ACTIONPROTOTYPE("void AddToParty(object oPC, object oPartyLeader);") 572, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_AddToParty NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_AddToParty) },
	{ NWSCRIPT_ACTIONNAME("RemoveFromParty") NWSCRIPT_ACTIONPROTOTYPE("void RemoveFromParty(object oPC);") 573, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_RemoveFromParty NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_RemoveFromParty) },
	{ NWSCRIPT_ACTIONNAME("GetStealthMode") NWSCRIPT_ACTIONPROTOTYPE("int GetStealthMode(object oCreature);") 574, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetStealthMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetStealthMode) },
	{ NWSCRIPT_ACTIONNAME("GetDetectMode") NWSCRIPT_ACTIONPROTOTYPE("int GetDetectMode(object oCreature);") 575, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetDetectMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetDetectMode) },
	{ NWSCRIPT_ACTIONNAME("GetDefensiveCastingMode") NWSCRIPT_ACTIONPROTOTYPE("int GetDefensiveCastingMode(object oCreature);") 576, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetDefensiveCastingMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetDefensiveCastingMode) },
	{ NWSCRIPT_ACTIONNAME("GetAppearanceType") NWSCRIPT_ACTIONPROTOTYPE("int GetAppearanceType(object oCreature);") 577, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetAppearanceType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAppearanceType) },
	{ NWSCRIPT_ACTIONNAME("SpawnScriptDebugger") NWSCRIPT_ACTIONPROTOTYPE("void SpawnScriptDebugger();") 578, 0, 0, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SpawnScriptDebugger NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SpawnScriptDebugger) },
	{ NWSCRIPT_ACTIONNAME("GetModuleItemAcquiredStackSize") NWSCRIPT_ACTIONPROTOTYPE("int GetModuleItemAcquiredStackSize();") 579, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetModuleItemAcquiredStackSize NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetModuleItemAcquiredStackSize) },
	{ NWSCRIPT_ACTIONNAME("DecrementRemainingFeatUses") NWSCRIPT_ACTIONPROTOTYPE("void DecrementRemainingFeatUses(object oCreature, int nFeat);") 580, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_DecrementRemainingFeatUses NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_DecrementRemainingFeatUses) },
	{ NWSCRIPT_ACTIONNAME("DecrementRemainingSpellUses") NWSCRIPT_ACTIONPROTOTYPE("void DecrementRemainingSpellUses(object oCreature, int nSpell);") 581, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_DecrementRemainingSpellUses NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_DecrementRemainingSpellUses) },
	{ NWSCRIPT_ACTIONNAME("GetResRef") NWSCRIPT_ACTIONPROTOTYPE("string GetResRef(object oObject);") 582, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetResRef NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetResRef) },
	{ NWSCRIPT_ACTIONNAME("EffectPetrify") NWSCRIPT_ACTIONPROTOTYPE("effect EffectPetrify();") 583, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectPetrify NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectPetrify) },
	{ NWSCRIPT_ACTIONNAME("CopyItem") NWSCRIPT_ACTIONPROTOTYPE("object CopyItem(object oItem, object oTargetInventory=OBJECT_INVALID, int bCopyVars=FALSE);") 584, 1, 3, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_CopyItem NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_CopyItem) },
	{ NWSCRIPT_ACTIONNAME("EffectCutsceneParalyze") NWSCRIPT_ACTIONPROTOTYPE("effect EffectCutsceneParalyze();") 585, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectCutsceneParalyze NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectCutsceneParalyze) },
	{ NWSCRIPT_ACTIONNAME("GetDroppableFlag") NWSCRIPT_ACTIONPROTOTYPE("int GetDroppableFlag(object oItem);") 586, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetDroppableFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetDroppableFlag) },
	{ NWSCRIPT_ACTIONNAME("GetUseableFlag") NWSCRIPT_ACTIONPROTOTYPE("int GetUseableFlag(object oObject=OBJECT_SELF);") 587, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetUseableFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetUseableFlag) },
	{ NWSCRIPT_ACTIONNAME("GetStolenFlag") NWSCRIPT_ACTIONPROTOTYPE("int GetStolenFlag(object oStolen);") 588, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetStolenFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetStolenFlag) },
	{ NWSCRIPT_ACTIONNAME("SetCampaignFloat") NWSCRIPT_ACTIONPROTOTYPE("void SetCampaignFloat(string sCampaignName, string sVarName, float flFloat, object oPlayer=OBJECT_INVALID);") 589, 3, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetCampaignFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetCampaignFloat) },
	{ NWSCRIPT_ACTIONNAME("SetCampaignInt") NWSCRIPT_ACTIONPROTOTYPE("void SetCampaignInt(string sCampaignName, string sVarName, int nInt, object oPlayer=OBJECT_INVALID);") 590, 3, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetCampaignInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetCampaignInt) },
	{ NWSCRIPT_ACTIONNAME("SetCampaignVector") NWSCRIPT_ACTIONPROTOTYPE("void SetCampaignVector(string sCampaignName, string sVarName, vector vVector, object oPlayer=OBJECT_INVALID);") 591, 3, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetCampaignVector NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetCampaignVector) },
	{ NWSCRIPT_ACTIONNAME("SetCampaignLocation") NWSCRIPT_ACTIONPROTOTYPE("void SetCampaignLocation(string sCampaignName, string sVarName, location locLocation, object oPlayer=OBJECT_INVALID);") 592, 3, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetCampaignLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetCampaignLocation) },
	{ NWSCRIPT_ACTIONNAME("SetCampaignString") NWSCRIPT_ACTIONPROTOTYPE("void SetCampaignString(string sCampaignName, string sVarName, string sString, object oPlayer=OBJECT_INVALID);") 593, 3, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetCampaignString NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetCampaignString) },
	{ NWSCRIPT_ACTIONNAME("DestroyCampaignDatabase") NWSCRIPT_ACTIONPROTOTYPE("void DestroyCampaignDatabase(string sCampaignName);") 594, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_DestroyCampaignDatabase NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_DestroyCampaignDatabase) },
	{ NWSCRIPT_ACTIONNAME("GetCampaignFloat") NWSCRIPT_ACTIONPROTOTYPE("float GetCampaignFloat(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);") 595, 2, 3, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_GetCampaignFloat NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCampaignFloat) },
	{ NWSCRIPT_ACTIONNAME("GetCampaignInt") NWSCRIPT_ACTIONPROTOTYPE("int GetCampaignInt(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);") 596, 2, 3, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetCampaignInt NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCampaignInt) },
	{ NWSCRIPT_ACTIONNAME("GetCampaignVector") NWSCRIPT_ACTIONPROTOTYPE("vector GetCampaignVector(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);") 597, 2, 3, ACTIONTYPE_VECTOR, NWN1_NWActionParameterTypes_GetCampaignVector NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCampaignVector) },
	{ NWSCRIPT_ACTIONNAME("GetCampaignLocation") NWSCRIPT_ACTIONPROTOTYPE("location GetCampaignLocation(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);") 598, 2, 3, ACTIONTYPE_LOCATION, NWN1_NWActionParameterTypes_GetCampaignLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCampaignLocation) },
	{ NWSCRIPT_ACTIONNAME("GetCampaignString") NWSCRIPT_ACTIONPROTOTYPE("string GetCampaignString(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);") 599, 2, 3, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetCampaignString NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCampaignString) },
	{ NWSCRIPT_ACTIONNAME("CopyObject") NWSCRIPT_ACTIONPROTOTYPE("object CopyObject(object oSource, location locLocation, object oOwner = OBJECT_INVALID, string sNewTag = "");") 600, 2, 4, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_CopyObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_CopyObject) },
	{ NWSCRIPT_ACTIONNAME("DeleteCampaignVariable") NWSCRIPT_ACTIONPROTOTYPE("void DeleteCampaignVariable(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);") 601, 2, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_DeleteCampaignVariable NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_DeleteCampaignVariable) },
	{ NWSCRIPT_ACTIONNAME("StoreCampaignObject") NWSCRIPT_ACTIONPROTOTYPE("int StoreCampaignObject(string sCampaignName, string sVarName, object oObject, object oPlayer=OBJECT_INVALID);") 602, 3, 4, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_StoreCampaignObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_StoreCampaignObject) },
	{ NWSCRIPT_ACTIONNAME("RetrieveCampaignObject") NWSCRIPT_ACTIONPROTOTYPE("object RetrieveCampaignObject(string sCampaignName, string sVarName, location locLocation, object oOwner = OBJECT_INVALID, object oPlayer=OBJECT_INVALID);") 603, 3, 5, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_RetrieveCampaignObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_RetrieveCampaignObject) },
	{ NWSCRIPT_ACTIONNAME("EffectCutsceneDominated") NWSCRIPT_ACTIONPROTOTYPE("effect EffectCutsceneDominated();") 604, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectCutsceneDominated NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectCutsceneDominated) },
	{ NWSCRIPT_ACTIONNAME("GetItemStackSize") NWSCRIPT_ACTIONPROTOTYPE("int GetItemStackSize(object oItem);") 605, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetItemStackSize NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemStackSize) },
	{ NWSCRIPT_ACTIONNAME("SetItemStackSize") NWSCRIPT_ACTIONPROTOTYPE("void SetItemStackSize(object oItem, int nSize);") 606, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetItemStackSize NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetItemStackSize) },
	{ NWSCRIPT_ACTIONNAME("GetItemCharges") NWSCRIPT_ACTIONPROTOTYPE("int GetItemCharges(object oItem);") 607, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetItemCharges NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemCharges) },
	{ NWSCRIPT_ACTIONNAME("SetItemCharges") NWSCRIPT_ACTIONPROTOTYPE("void SetItemCharges(object oItem, int nCharges);") 608, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetItemCharges NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetItemCharges) },
	{ NWSCRIPT_ACTIONNAME("AddItemProperty") NWSCRIPT_ACTIONPROTOTYPE("void AddItemProperty(int nDurationType, itemproperty ipProperty, object oItem, float fDuration=0.0f);") 609, 3, 4, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_AddItemProperty NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_AddItemProperty) },
	{ NWSCRIPT_ACTIONNAME("RemoveItemProperty") NWSCRIPT_ACTIONPROTOTYPE("void RemoveItemProperty(object oItem, itemproperty ipProperty);") 610, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_RemoveItemProperty NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_RemoveItemProperty) },
	{ NWSCRIPT_ACTIONNAME("GetIsItemPropertyValid") NWSCRIPT_ACTIONPROTOTYPE("int GetIsItemPropertyValid(itemproperty ipProperty);") 611, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsItemPropertyValid NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsItemPropertyValid) },
	{ NWSCRIPT_ACTIONNAME("GetFirstItemProperty") NWSCRIPT_ACTIONPROTOTYPE("itemproperty GetFirstItemProperty(object oItem);") 612, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_GetFirstItemProperty NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFirstItemProperty) },
	{ NWSCRIPT_ACTIONNAME("GetNextItemProperty") NWSCRIPT_ACTIONPROTOTYPE("itemproperty GetNextItemProperty(object oItem);") 613, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_GetNextItemProperty NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetNextItemProperty) },
	{ NWSCRIPT_ACTIONNAME("GetItemPropertyType") NWSCRIPT_ACTIONPROTOTYPE("int GetItemPropertyType(itemproperty ip);") 614, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetItemPropertyType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemPropertyType) },
	{ NWSCRIPT_ACTIONNAME("GetItemPropertyDurationType") NWSCRIPT_ACTIONPROTOTYPE("int GetItemPropertyDurationType(itemproperty ip);") 615, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetItemPropertyDurationType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemPropertyDurationType) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyAbilityBonus") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyAbilityBonus(int nAbility, int nBonus);") 616, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyAbilityBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyAbilityBonus) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyACBonus") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyACBonus(int nBonus);") 617, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyACBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyACBonus) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyACBonusVsAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyACBonusVsAlign(int nAlignGroup, int nACBonus);") 618, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyACBonusVsAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyACBonusVsAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyACBonusVsDmgType") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyACBonusVsDmgType(int nDamageType, int nACBonus);") 619, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyACBonusVsDmgType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyACBonusVsDmgType) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyACBonusVsRace") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyACBonusVsRace(int nRace, int nACBonus);") 620, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyACBonusVsRace NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyACBonusVsRace) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyACBonusVsSAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyACBonusVsSAlign(int nAlign, int nACBonus);") 621, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyACBonusVsSAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyACBonusVsSAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyEnhancementBonus") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyEnhancementBonus(int nEnhancementBonus);") 622, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyEnhancementBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyEnhancementBonus) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyEnhancementBonusVsAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyEnhancementBonusVsAlign(int nAlignGroup, int nBonus);") 623, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyEnhancementBonusVsAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyEnhancementBonusVsAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyEnhancementBonusVsRace") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyEnhancementBonusVsRace(int nRace, int nBonus);") 624, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyEnhancementBonusVsRace NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyEnhancementBonusVsRace) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyEnhancementBonusVsSAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyEnhancementBonusVsSAlign(int nAlign, int nBonus);") 625, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyEnhancementBonusVsSAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyEnhancementBonusVsSAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyEnhancementPenalty") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyEnhancementPenalty(int nPenalty);") 626, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyEnhancementPenalty NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyEnhancementPenalty) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyWeightReduction") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyWeightReduction(int nReduction);") 627, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyWeightReduction NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyWeightReduction) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyBonusFeat") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyBonusFeat(int nFeat);") 628, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyBonusFeat NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyBonusFeat) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyBonusLevelSpell") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyBonusLevelSpell(int nClass, int nSpellLevel);") 629, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyBonusLevelSpell NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyBonusLevelSpell) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyCastSpell") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyCastSpell(int nSpell, int nNumUses);") 630, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyCastSpell NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyCastSpell) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamageBonus") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamageBonus(int nDamageType, int nDamage);") 631, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyDamageBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyDamageBonus) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamageBonusVsAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamageBonusVsAlign(int nAlignGroup, int nDamageType, int nDamage);") 632, 3, 3, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyDamageBonusVsAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyDamageBonusVsAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamageBonusVsRace") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamageBonusVsRace(int nRace, int nDamageType, int nDamage);") 633, 3, 3, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyDamageBonusVsRace NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyDamageBonusVsRace) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamageBonusVsSAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamageBonusVsSAlign(int nAlign, int nDamageType, int nDamage);") 634, 3, 3, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyDamageBonusVsSAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyDamageBonusVsSAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamageImmunity") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamageImmunity(int nDamageType, int nImmuneBonus);") 635, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyDamageImmunity NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyDamageImmunity) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamagePenalty") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamagePenalty(int nPenalty);") 636, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyDamagePenalty NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyDamagePenalty) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamageReduction") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamageReduction(int nEnhancement, int nHPSoak);") 637, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyDamageReduction NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyDamageReduction) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamageResistance") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamageResistance(int nDamageType, int nHPResist);") 638, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyDamageResistance NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyDamageResistance) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDamageVulnerability") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDamageVulnerability(int nDamageType, int nVulnerability);") 639, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyDamageVulnerability NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyDamageVulnerability) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDarkvision") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDarkvision();") 640, 0, 0, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyDarkvision NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyDarkvision) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDecreaseAbility") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDecreaseAbility(int nAbility, int nModifier);") 641, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyDecreaseAbility NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyDecreaseAbility) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDecreaseAC") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDecreaseAC(int nModifierType, int nPenalty);") 642, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyDecreaseAC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyDecreaseAC) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyDecreaseSkill") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyDecreaseSkill(int nSkill, int nPenalty);") 643, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyDecreaseSkill NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyDecreaseSkill) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyContainerReducedWeight") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyContainerReducedWeight(int nContainerType);") 644, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyContainerReducedWeight NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyContainerReducedWeight) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyExtraMeleeDamageType") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyExtraMeleeDamageType(int nDamageType);") 645, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyExtraMeleeDamageType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyExtraMeleeDamageType) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyExtraRangeDamageType") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyExtraRangeDamageType(int nDamageType);") 646, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyExtraRangeDamageType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyExtraRangeDamageType) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyHaste") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyHaste();") 647, 0, 0, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyHaste NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyHaste) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyHolyAvenger") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyHolyAvenger();") 648, 0, 0, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyHolyAvenger NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyHolyAvenger) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyImmunityMisc") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyImmunityMisc(int nImmunityType);") 649, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyImmunityMisc NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyImmunityMisc) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyImprovedEvasion") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyImprovedEvasion();") 650, 0, 0, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyImprovedEvasion NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyImprovedEvasion) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyBonusSpellResistance") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyBonusSpellResistance(int nBonus);") 651, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyBonusSpellResistance NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyBonusSpellResistance) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyBonusSavingThrowVsX") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyBonusSavingThrowVsX(int nBonusType, int nBonus);") 652, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyBonusSavingThrowVsX NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyBonusSavingThrowVsX) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyBonusSavingThrow") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyBonusSavingThrow(int nBaseSaveType, int nBonus);") 653, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyBonusSavingThrow NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyBonusSavingThrow) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyKeen") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyKeen();") 654, 0, 0, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyKeen NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyKeen) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyLight") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyLight(int nBrightness, int nColor);") 655, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyLight NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyLight) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyMaxRangeStrengthMod") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyMaxRangeStrengthMod(int nModifier);") 656, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyMaxRangeStrengthMod NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyMaxRangeStrengthMod) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyNoDamage") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyNoDamage();") 657, 0, 0, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyNoDamage NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyNoDamage) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyOnHitProps") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyOnHitProps(int nProperty, int nSaveDC, int nSpecial=0);") 658, 2, 3, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyOnHitProps NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyOnHitProps) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyReducedSavingThrowVsX") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyReducedSavingThrowVsX(int nBaseSaveType, int nPenalty);") 659, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyReducedSavingThrowVsX NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyReducedSavingThrowVsX) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyReducedSavingThrow") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyReducedSavingThrow(int nBonusType, int nPenalty);") 660, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyReducedSavingThrow NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyReducedSavingThrow) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyRegeneration") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyRegeneration(int nRegenAmount);") 661, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyRegeneration NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyRegeneration) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertySkillBonus") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertySkillBonus(int nSkill, int nBonus);") 662, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertySkillBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertySkillBonus) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertySpellImmunitySpecific") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertySpellImmunitySpecific(int nSpell);") 663, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertySpellImmunitySpecific NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertySpellImmunitySpecific) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertySpellImmunitySchool") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertySpellImmunitySchool(int nSchool);") 664, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertySpellImmunitySchool NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertySpellImmunitySchool) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyThievesTools") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyThievesTools(int nModifier);") 665, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyThievesTools NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyThievesTools) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyAttackBonus") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyAttackBonus(int nBonus);") 666, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyAttackBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyAttackBonus) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyAttackBonusVsAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyAttackBonusVsAlign(int nAlignGroup, int nBonus);") 667, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyAttackBonusVsAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyAttackBonusVsAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyAttackBonusVsRace") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyAttackBonusVsRace(int nRace, int nBonus);") 668, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyAttackBonusVsRace NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyAttackBonusVsRace) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyAttackBonusVsSAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyAttackBonusVsSAlign(int nAlignment, int nBonus);") 669, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyAttackBonusVsSAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyAttackBonusVsSAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyAttackPenalty") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyAttackPenalty(int nPenalty);") 670, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyAttackPenalty NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyAttackPenalty) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyUnlimitedAmmo") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyUnlimitedAmmo(int nAmmoDamage=IP_CONST_UNLIMITEDAMMO_BASIC);") 671, 0, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyUnlimitedAmmo NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyUnlimitedAmmo) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyLimitUseByAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyLimitUseByAlign(int nAlignGroup);") 672, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyLimitUseByAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyLimitUseByAlign) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyLimitUseByClass") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyLimitUseByClass(int nClass);") 673, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyLimitUseByClass NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyLimitUseByClass) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyLimitUseByRace") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyLimitUseByRace(int nRace);") 674, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyLimitUseByRace NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyLimitUseByRace) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyLimitUseBySAlign") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyLimitUseBySAlign(int nAlignment);") 675, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyLimitUseBySAlign NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyLimitUseBySAlign) },
	{ NWSCRIPT_ACTIONNAME("BadBadReplaceMeThisDoesNothing") NWSCRIPT_ACTIONPROTOTYPE("itemproperty BadBadReplaceMeThisDoesNothing();") 676, 0, 0, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_BadBadReplaceMeThisDoesNothing NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_BadBadReplaceMeThisDoesNothing) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyVampiricRegeneration") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyVampiricRegeneration(int nRegenAmount);") 677, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyVampiricRegeneration NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyVampiricRegeneration) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyTrap") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyTrap(int nTrapLevel, int nTrapType);") 678, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyTrap NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyTrap) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyTrueSeeing") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyTrueSeeing();") 679, 0, 0, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyTrueSeeing NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyTrueSeeing) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyOnMonsterHitProperties") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyOnMonsterHitProperties(int nProperty, int nSpecial=0);") 680, 1, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyOnMonsterHitProperties NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyOnMonsterHitProperties) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyTurnResistance") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyTurnResistance(int nModifier);") 681, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyTurnResistance NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyTurnResistance) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyMassiveCritical") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyMassiveCritical(int nDamage);") 682, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyMassiveCritical NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyMassiveCritical) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyFreeAction") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyFreeAction();") 683, 0, 0, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyFreeAction NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyFreeAction) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyMonsterDamage") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyMonsterDamage(int nDamage);") 684, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyMonsterDamage NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyMonsterDamage) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyImmunityToSpellLevel") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyImmunityToSpellLevel(int nLevel);") 685, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyImmunityToSpellLevel NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyImmunityToSpellLevel) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertySpecialWalk") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertySpecialWalk(int nWalkType=0);") 686, 0, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertySpecialWalk NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertySpecialWalk) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyHealersKit") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyHealersKit(int nModifier);") 687, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyHealersKit NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyHealersKit) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyWeightIncrease") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyWeightIncrease(int nWeight);") 688, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyWeightIncrease NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyWeightIncrease) },
	{ NWSCRIPT_ACTIONNAME("GetIsSkillSuccessful") NWSCRIPT_ACTIONPROTOTYPE("int GetIsSkillSuccessful(object oTarget, int nSkill, int nDifficulty);") 689, 3, 3, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsSkillSuccessful NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsSkillSuccessful) },
	{ NWSCRIPT_ACTIONNAME("EffectSpellFailure") NWSCRIPT_ACTIONPROTOTYPE("effect EffectSpellFailure(int nPercent=100, int nSpellSchool=SPELL_SCHOOL_GENERAL);") 690, 0, 2, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectSpellFailure NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectSpellFailure) },
	{ NWSCRIPT_ACTIONNAME("SpeakStringByStrRef") NWSCRIPT_ACTIONPROTOTYPE("void SpeakStringByStrRef(int nStrRef, int nTalkVolume=TALKVOLUME_TALK);") 691, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SpeakStringByStrRef NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SpeakStringByStrRef) },
	{ NWSCRIPT_ACTIONNAME("SetCutsceneMode") NWSCRIPT_ACTIONPROTOTYPE("void SetCutsceneMode(object oCreature, int nInCutscene=TRUE, int nLeftClickingEnabled=FALSE);") 692, 1, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetCutsceneMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetCutsceneMode) },
	{ NWSCRIPT_ACTIONNAME("GetLastPCToCancelCutscene") NWSCRIPT_ACTIONPROTOTYPE("object GetLastPCToCancelCutscene();") 693, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetLastPCToCancelCutscene NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastPCToCancelCutscene) },
	{ NWSCRIPT_ACTIONNAME("GetDialogSoundLength") NWSCRIPT_ACTIONPROTOTYPE("float GetDialogSoundLength(int nStrRef);") 694, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_GetDialogSoundLength NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetDialogSoundLength) },
	{ NWSCRIPT_ACTIONNAME("FadeFromBlack") NWSCRIPT_ACTIONPROTOTYPE("void FadeFromBlack(object oCreature, float fSpeed=FADE_SPEED_MEDIUM);") 695, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_FadeFromBlack NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_FadeFromBlack) },
	{ NWSCRIPT_ACTIONNAME("FadeToBlack") NWSCRIPT_ACTIONPROTOTYPE("void FadeToBlack(object oCreature, float fSpeed=FADE_SPEED_MEDIUM);") 696, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_FadeToBlack NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_FadeToBlack) },
	{ NWSCRIPT_ACTIONNAME("StopFade") NWSCRIPT_ACTIONPROTOTYPE("void StopFade(object oCreature);") 697, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_StopFade NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_StopFade) },
	{ NWSCRIPT_ACTIONNAME("BlackScreen") NWSCRIPT_ACTIONPROTOTYPE("void BlackScreen(object oCreature);") 698, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_BlackScreen NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_BlackScreen) },
	{ NWSCRIPT_ACTIONNAME("GetBaseAttackBonus") NWSCRIPT_ACTIONPROTOTYPE("int GetBaseAttackBonus(object oCreature);") 699, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetBaseAttackBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetBaseAttackBonus) },
	{ NWSCRIPT_ACTIONNAME("SetImmortal") NWSCRIPT_ACTIONPROTOTYPE("void SetImmortal(object oCreature, int bImmortal);") 700, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetImmortal NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetImmortal) },
	{ NWSCRIPT_ACTIONNAME("OpenInventory") NWSCRIPT_ACTIONPROTOTYPE("void OpenInventory(object oCreature, object oPlayer);") 701, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_OpenInventory NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_OpenInventory) },
	{ NWSCRIPT_ACTIONNAME("StoreCameraFacing") NWSCRIPT_ACTIONPROTOTYPE("void StoreCameraFacing();") 702, 0, 0, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_StoreCameraFacing NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_StoreCameraFacing) },
	{ NWSCRIPT_ACTIONNAME("RestoreCameraFacing") NWSCRIPT_ACTIONPROTOTYPE("void RestoreCameraFacing();") 703, 0, 0, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_RestoreCameraFacing NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_RestoreCameraFacing) },
	{ NWSCRIPT_ACTIONNAME("LevelUpHenchman") NWSCRIPT_ACTIONPROTOTYPE("int LevelUpHenchman(object oCreature, int nClass = CLASS_TYPE_INVALID, int bReadyAllSpells = FALSE, int nPackage = PACKAGE_INVALID);") 704, 1, 4, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_LevelUpHenchman NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_LevelUpHenchman) },
	{ NWSCRIPT_ACTIONNAME("SetDroppableFlag") NWSCRIPT_ACTIONPROTOTYPE("void SetDroppableFlag(object oItem, int bDroppable);") 705, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetDroppableFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetDroppableFlag) },
	{ NWSCRIPT_ACTIONNAME("GetWeight") NWSCRIPT_ACTIONPROTOTYPE("int GetWeight(object oTarget=OBJECT_SELF);") 706, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetWeight NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetWeight) },
	{ NWSCRIPT_ACTIONNAME("GetModuleItemAcquiredBy") NWSCRIPT_ACTIONPROTOTYPE("object GetModuleItemAcquiredBy();") 707, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetModuleItemAcquiredBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetModuleItemAcquiredBy) },
	{ NWSCRIPT_ACTIONNAME("GetImmortal") NWSCRIPT_ACTIONPROTOTYPE("int GetImmortal(object oTarget=OBJECT_SELF);") 708, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetImmortal NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetImmortal) },
	{ NWSCRIPT_ACTIONNAME("DoWhirlwindAttack") NWSCRIPT_ACTIONPROTOTYPE("void DoWhirlwindAttack(int bDisplayFeedback=TRUE, int bImproved=FALSE);") 709, 0, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_DoWhirlwindAttack NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_DoWhirlwindAttack) },
	{ NWSCRIPT_ACTIONNAME("Get2DAString") NWSCRIPT_ACTIONPROTOTYPE("string Get2DAString(string s2DA, string sColumn, int nRow);") 710, 3, 3, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_Get2DAString NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_Get2DAString) },
	{ NWSCRIPT_ACTIONNAME("EffectEthereal") NWSCRIPT_ACTIONPROTOTYPE("effect EffectEthereal();") 711, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectEthereal NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectEthereal) },
	{ NWSCRIPT_ACTIONNAME("GetAILevel") NWSCRIPT_ACTIONPROTOTYPE("int GetAILevel(object oTarget=OBJECT_SELF);") 712, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetAILevel NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAILevel) },
	{ NWSCRIPT_ACTIONNAME("SetAILevel") NWSCRIPT_ACTIONPROTOTYPE("void SetAILevel(object oTarget, int nAILevel);") 713, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetAILevel NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetAILevel) },
	{ NWSCRIPT_ACTIONNAME("GetIsPossessedFamiliar") NWSCRIPT_ACTIONPROTOTYPE("int GetIsPossessedFamiliar(object oCreature);") 714, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsPossessedFamiliar NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsPossessedFamiliar) },
	{ NWSCRIPT_ACTIONNAME("UnpossessFamiliar") NWSCRIPT_ACTIONPROTOTYPE("void UnpossessFamiliar(object oCreature);") 715, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_UnpossessFamiliar NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_UnpossessFamiliar) },
	{ NWSCRIPT_ACTIONNAME("GetIsAreaInterior") NWSCRIPT_ACTIONPROTOTYPE("int GetIsAreaInterior( object oArea = OBJECT_INVALID );") 716, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsAreaInterior NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsAreaInterior) },
	{ NWSCRIPT_ACTIONNAME("SendMessageToPCByStrRef") NWSCRIPT_ACTIONPROTOTYPE("void SendMessageToPCByStrRef(object oPlayer, int nStrRef);") 717, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SendMessageToPCByStrRef NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SendMessageToPCByStrRef) },
	{ NWSCRIPT_ACTIONNAME("IncrementRemainingFeatUses") NWSCRIPT_ACTIONPROTOTYPE("void IncrementRemainingFeatUses(object oCreature, int nFeat);") 718, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_IncrementRemainingFeatUses NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_IncrementRemainingFeatUses) },
	{ NWSCRIPT_ACTIONNAME("ExportSingleCharacter") NWSCRIPT_ACTIONPROTOTYPE("void ExportSingleCharacter(object oPlayer);") 719, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ExportSingleCharacter NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ExportSingleCharacter) },
	{ NWSCRIPT_ACTIONNAME("PlaySoundByStrRef") NWSCRIPT_ACTIONPROTOTYPE("void PlaySoundByStrRef(int nStrRef, int nRunAsAction = TRUE );") 720, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_PlaySoundByStrRef NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_PlaySoundByStrRef) },
	{ NWSCRIPT_ACTIONNAME("SetSubRace") NWSCRIPT_ACTIONPROTOTYPE("void SetSubRace(object oCreature, string sSubRace);") 721, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetSubRace NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetSubRace) },
	{ NWSCRIPT_ACTIONNAME("SetDeity") NWSCRIPT_ACTIONPROTOTYPE("void SetDeity(object oCreature, string sDeity);") 722, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetDeity NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetDeity) },
	{ NWSCRIPT_ACTIONNAME("GetIsDMPossessed") NWSCRIPT_ACTIONPROTOTYPE("int GetIsDMPossessed(object oCreature);") 723, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsDMPossessed NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsDMPossessed) },
	{ NWSCRIPT_ACTIONNAME("GetWeather") NWSCRIPT_ACTIONPROTOTYPE("int GetWeather(object oArea);") 724, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetWeather NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetWeather) },
	{ NWSCRIPT_ACTIONNAME("GetIsAreaNatural") NWSCRIPT_ACTIONPROTOTYPE("int GetIsAreaNatural(object oArea);") 725, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsAreaNatural NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsAreaNatural) },
	{ NWSCRIPT_ACTIONNAME("GetIsAreaAboveGround") NWSCRIPT_ACTIONPROTOTYPE("int GetIsAreaAboveGround(object oArea);") 726, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsAreaAboveGround NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsAreaAboveGround) },
	{ NWSCRIPT_ACTIONNAME("GetPCItemLastEquipped") NWSCRIPT_ACTIONPROTOTYPE("object GetPCItemLastEquipped();") 727, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetPCItemLastEquipped NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPCItemLastEquipped) },
	{ NWSCRIPT_ACTIONNAME("GetPCItemLastEquippedBy") NWSCRIPT_ACTIONPROTOTYPE("object GetPCItemLastEquippedBy();") 728, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetPCItemLastEquippedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPCItemLastEquippedBy) },
	{ NWSCRIPT_ACTIONNAME("GetPCItemLastUnequipped") NWSCRIPT_ACTIONPROTOTYPE("object GetPCItemLastUnequipped();") 729, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetPCItemLastUnequipped NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPCItemLastUnequipped) },
	{ NWSCRIPT_ACTIONNAME("GetPCItemLastUnequippedBy") NWSCRIPT_ACTIONPROTOTYPE("object GetPCItemLastUnequippedBy();") 730, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetPCItemLastUnequippedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPCItemLastUnequippedBy) },
	{ NWSCRIPT_ACTIONNAME("CopyItemAndModify") NWSCRIPT_ACTIONPROTOTYPE("object CopyItemAndModify(object oItem, int nType, int nIndex, int nNewValue, int bCopyVars=FALSE);") 731, 4, 5, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_CopyItemAndModify NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_CopyItemAndModify) },
	{ NWSCRIPT_ACTIONNAME("GetItemAppearance") NWSCRIPT_ACTIONPROTOTYPE("int GetItemAppearance(object oItem, int nType, int nIndex);") 732, 3, 3, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetItemAppearance NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemAppearance) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyOnHitCastSpell") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyOnHitCastSpell(int nSpell, int nLevel);") 733, 2, 2, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyOnHitCastSpell NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyOnHitCastSpell) },
	{ NWSCRIPT_ACTIONNAME("GetItemPropertySubType") NWSCRIPT_ACTIONPROTOTYPE("int GetItemPropertySubType(itemproperty iProperty);") 734, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetItemPropertySubType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemPropertySubType) },
	{ NWSCRIPT_ACTIONNAME("GetActionMode") NWSCRIPT_ACTIONPROTOTYPE("int GetActionMode(object oCreature, int nMode);") 735, 2, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetActionMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetActionMode) },
	{ NWSCRIPT_ACTIONNAME("SetActionMode") NWSCRIPT_ACTIONPROTOTYPE("void SetActionMode(object oCreature, int nMode, int nStatus);") 736, 3, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetActionMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetActionMode) },
	{ NWSCRIPT_ACTIONNAME("GetArcaneSpellFailure") NWSCRIPT_ACTIONPROTOTYPE("int GetArcaneSpellFailure(object oCreature);") 737, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetArcaneSpellFailure NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetArcaneSpellFailure) },
	{ NWSCRIPT_ACTIONNAME("ActionExamine") NWSCRIPT_ACTIONPROTOTYPE("void ActionExamine(object oExamine);") 738, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ActionExamine NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ActionExamine) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyVisualEffect") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyVisualEffect(int nEffect);") 739, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyVisualEffect NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyVisualEffect) },
	{ NWSCRIPT_ACTIONNAME("SetLootable") NWSCRIPT_ACTIONPROTOTYPE("void SetLootable( object oCreature, int bLootable );") 740, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetLootable NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetLootable) },
	{ NWSCRIPT_ACTIONNAME("GetLootable") NWSCRIPT_ACTIONPROTOTYPE("int GetLootable( object oCreature );") 741, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLootable NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLootable) },
	{ NWSCRIPT_ACTIONNAME("GetCutsceneCameraMoveRate") NWSCRIPT_ACTIONPROTOTYPE("float GetCutsceneCameraMoveRate( object oCreature );") 742, 1, 1, ACTIONTYPE_FLOAT, NWN1_NWActionParameterTypes_GetCutsceneCameraMoveRate NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCutsceneCameraMoveRate) },
	{ NWSCRIPT_ACTIONNAME("SetCutsceneCameraMoveRate") NWSCRIPT_ACTIONPROTOTYPE("void SetCutsceneCameraMoveRate( object oCreature, float fRate );") 743, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetCutsceneCameraMoveRate NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetCutsceneCameraMoveRate) },
	{ NWSCRIPT_ACTIONNAME("GetItemCursedFlag") NWSCRIPT_ACTIONPROTOTYPE("int GetItemCursedFlag(object oItem);") 744, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetItemCursedFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemCursedFlag) },
	{ NWSCRIPT_ACTIONNAME("SetItemCursedFlag") NWSCRIPT_ACTIONPROTOTYPE("void SetItemCursedFlag(object oItem, int nCursed);") 745, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetItemCursedFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetItemCursedFlag) },
	{ NWSCRIPT_ACTIONNAME("SetMaxHenchmen") NWSCRIPT_ACTIONPROTOTYPE("void SetMaxHenchmen( int nNumHenchmen );") 746, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetMaxHenchmen NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetMaxHenchmen) },
	{ NWSCRIPT_ACTIONNAME("GetMaxHenchmen") NWSCRIPT_ACTIONPROTOTYPE("int GetMaxHenchmen();") 747, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetMaxHenchmen NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetMaxHenchmen) },
	{ NWSCRIPT_ACTIONNAME("GetAssociateType") NWSCRIPT_ACTIONPROTOTYPE("int GetAssociateType( object oAssociate );") 748, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetAssociateType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAssociateType) },
	{ NWSCRIPT_ACTIONNAME("GetSpellResistance") NWSCRIPT_ACTIONPROTOTYPE("int GetSpellResistance( object oCreature );") 749, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetSpellResistance NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetSpellResistance) },
	{ NWSCRIPT_ACTIONNAME("DayToNight") NWSCRIPT_ACTIONPROTOTYPE("void DayToNight(object oPlayer, float fTransitionTime=0.0f);") 750, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_DayToNight NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_DayToNight) },
	{ NWSCRIPT_ACTIONNAME("NightToDay") NWSCRIPT_ACTIONPROTOTYPE("void NightToDay(object oPlayer, float fTransitionTime=0.0f);") 751, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_NightToDay NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_NightToDay) },
	{ NWSCRIPT_ACTIONNAME("LineOfSightObject") NWSCRIPT_ACTIONPROTOTYPE("int LineOfSightObject( object oSource, object oTarget );") 752, 2, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_LineOfSightObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_LineOfSightObject) },
	{ NWSCRIPT_ACTIONNAME("LineOfSightVector") NWSCRIPT_ACTIONPROTOTYPE("int LineOfSightVector( vector vSource, vector vTarget );") 753, 2, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_LineOfSightVector NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_LineOfSightVector) },
	{ NWSCRIPT_ACTIONNAME("GetLastSpellCastClass") NWSCRIPT_ACTIONPROTOTYPE("int GetLastSpellCastClass();") 754, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetLastSpellCastClass NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetLastSpellCastClass) },
	{ NWSCRIPT_ACTIONNAME("SetBaseAttackBonus") NWSCRIPT_ACTIONPROTOTYPE("void SetBaseAttackBonus( int nBaseAttackBonus, object oCreature = OBJECT_SELF );") 755, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetBaseAttackBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetBaseAttackBonus) },
	{ NWSCRIPT_ACTIONNAME("RestoreBaseAttackBonus") NWSCRIPT_ACTIONPROTOTYPE("void RestoreBaseAttackBonus( object oCreature = OBJECT_SELF );") 756, 0, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_RestoreBaseAttackBonus NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_RestoreBaseAttackBonus) },
	{ NWSCRIPT_ACTIONNAME("EffectCutsceneGhost") NWSCRIPT_ACTIONPROTOTYPE("effect EffectCutsceneGhost();") 757, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectCutsceneGhost NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectCutsceneGhost) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyArcaneSpellFailure") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyArcaneSpellFailure(int nModLevel);") 758, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyArcaneSpellFailure NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyArcaneSpellFailure) },
	{ NWSCRIPT_ACTIONNAME("GetStoreGold") NWSCRIPT_ACTIONPROTOTYPE("int GetStoreGold(object oidStore);") 759, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetStoreGold NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetStoreGold) },
	{ NWSCRIPT_ACTIONNAME("SetStoreGold") NWSCRIPT_ACTIONPROTOTYPE("void SetStoreGold(object oidStore, int nGold);") 760, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetStoreGold NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetStoreGold) },
	{ NWSCRIPT_ACTIONNAME("GetStoreMaxBuyPrice") NWSCRIPT_ACTIONPROTOTYPE("int GetStoreMaxBuyPrice(object oidStore);") 761, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetStoreMaxBuyPrice NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetStoreMaxBuyPrice) },
	{ NWSCRIPT_ACTIONNAME("SetStoreMaxBuyPrice") NWSCRIPT_ACTIONPROTOTYPE("void SetStoreMaxBuyPrice(object oidStore, int nMaxBuy);") 762, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetStoreMaxBuyPrice NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetStoreMaxBuyPrice) },
	{ NWSCRIPT_ACTIONNAME("GetStoreIdentifyCost") NWSCRIPT_ACTIONPROTOTYPE("int GetStoreIdentifyCost(object oidStore);") 763, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetStoreIdentifyCost NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetStoreIdentifyCost) },
	{ NWSCRIPT_ACTIONNAME("SetStoreIdentifyCost") NWSCRIPT_ACTIONPROTOTYPE("void SetStoreIdentifyCost(object oidStore, int nCost);") 764, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetStoreIdentifyCost NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetStoreIdentifyCost) },
	{ NWSCRIPT_ACTIONNAME("SetCreatureAppearanceType") NWSCRIPT_ACTIONPROTOTYPE("void SetCreatureAppearanceType(object oCreature, int nAppearanceType);") 765, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetCreatureAppearanceType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetCreatureAppearanceType) },
	{ NWSCRIPT_ACTIONNAME("GetCreatureStartingPackage") NWSCRIPT_ACTIONPROTOTYPE("int GetCreatureStartingPackage(object oCreature);") 766, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetCreatureStartingPackage NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCreatureStartingPackage) },
	{ NWSCRIPT_ACTIONNAME("EffectCutsceneImmobilize") NWSCRIPT_ACTIONPROTOTYPE("effect EffectCutsceneImmobilize();") 767, 0, 0, ACTIONTYPE_EFFECT, NWN1_NWActionParameterTypes_EffectCutsceneImmobilize NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_EffectCutsceneImmobilize) },
	{ NWSCRIPT_ACTIONNAME("GetIsInSubArea") NWSCRIPT_ACTIONPROTOTYPE("int GetIsInSubArea(object oCreature, object oSubArea=OBJECT_SELF);") 768, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsInSubArea NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsInSubArea) },
	{ NWSCRIPT_ACTIONNAME("GetItemPropertyCostTable") NWSCRIPT_ACTIONPROTOTYPE("int GetItemPropertyCostTable(itemproperty iProp);") 769, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetItemPropertyCostTable NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemPropertyCostTable) },
	{ NWSCRIPT_ACTIONNAME("GetItemPropertyCostTableValue") NWSCRIPT_ACTIONPROTOTYPE("int GetItemPropertyCostTableValue(itemproperty iProp);") 770, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetItemPropertyCostTableValue NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemPropertyCostTableValue) },
	{ NWSCRIPT_ACTIONNAME("GetItemPropertyParam1") NWSCRIPT_ACTIONPROTOTYPE("int GetItemPropertyParam1(itemproperty iProp);") 771, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetItemPropertyParam1 NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemPropertyParam1) },
	{ NWSCRIPT_ACTIONNAME("GetItemPropertyParam1Value") NWSCRIPT_ACTIONPROTOTYPE("int GetItemPropertyParam1Value(itemproperty iProp);") 772, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetItemPropertyParam1Value NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetItemPropertyParam1Value) },
	{ NWSCRIPT_ACTIONNAME("GetIsCreatureDisarmable") NWSCRIPT_ACTIONPROTOTYPE("int GetIsCreatureDisarmable(object oCreature);") 773, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetIsCreatureDisarmable NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetIsCreatureDisarmable) },
	{ NWSCRIPT_ACTIONNAME("SetStolenFlag") NWSCRIPT_ACTIONPROTOTYPE("void SetStolenFlag(object oItem, int nStolenFlag);") 774, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetStolenFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetStolenFlag) },
	{ NWSCRIPT_ACTIONNAME("ForceRest") NWSCRIPT_ACTIONPROTOTYPE("void ForceRest(object oCreature);") 775, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_ForceRest NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ForceRest) },
	{ NWSCRIPT_ACTIONNAME("SetCameraHeight") NWSCRIPT_ACTIONPROTOTYPE("void SetCameraHeight(object oPlayer, float fHeight=0.0f);") 776, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetCameraHeight NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetCameraHeight) },
	{ NWSCRIPT_ACTIONNAME("SetSkyBox") NWSCRIPT_ACTIONPROTOTYPE("void SetSkyBox(int nSkyBox, object oArea=OBJECT_INVALID);") 777, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetSkyBox NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetSkyBox) },
	{ NWSCRIPT_ACTIONNAME("GetPhenoType") NWSCRIPT_ACTIONPROTOTYPE("int GetPhenoType(object oCreature);") 778, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetPhenoType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPhenoType) },
	{ NWSCRIPT_ACTIONNAME("SetPhenoType") NWSCRIPT_ACTIONPROTOTYPE("void SetPhenoType(int nPhenoType, object oCreature=OBJECT_SELF);") 779, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetPhenoType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetPhenoType) },
	{ NWSCRIPT_ACTIONNAME("SetFogColor") NWSCRIPT_ACTIONPROTOTYPE("void SetFogColor(int nFogType, int nFogColor, object oArea=OBJECT_INVALID);") 780, 2, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetFogColor NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetFogColor) },
	{ NWSCRIPT_ACTIONNAME("GetCutsceneMode") NWSCRIPT_ACTIONPROTOTYPE("int GetCutsceneMode(object oCreature=OBJECT_SELF);") 781, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetCutsceneMode NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCutsceneMode) },
	{ NWSCRIPT_ACTIONNAME("GetSkyBox") NWSCRIPT_ACTIONPROTOTYPE("int GetSkyBox(object oArea=OBJECT_INVALID);") 782, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetSkyBox NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetSkyBox) },
	{ NWSCRIPT_ACTIONNAME("GetFogColor") NWSCRIPT_ACTIONPROTOTYPE("int GetFogColor(int nFogType, object oArea=OBJECT_INVALID);") 783, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetFogColor NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFogColor) },
	{ NWSCRIPT_ACTIONNAME("SetFogAmount") NWSCRIPT_ACTIONPROTOTYPE("void SetFogAmount(int nFogType, int nFogAmount, object oArea=OBJECT_INVALID);") 784, 2, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetFogAmount NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetFogAmount) },
	{ NWSCRIPT_ACTIONNAME("GetFogAmount") NWSCRIPT_ACTIONPROTOTYPE("int GetFogAmount(int nFogType, object oArea=OBJECT_INVALID);") 785, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetFogAmount NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFogAmount) },
	{ NWSCRIPT_ACTIONNAME("GetPickpocketableFlag") NWSCRIPT_ACTIONPROTOTYPE("int GetPickpocketableFlag(object oItem);") 786, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetPickpocketableFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPickpocketableFlag) },
	{ NWSCRIPT_ACTIONNAME("SetPickpocketableFlag") NWSCRIPT_ACTIONPROTOTYPE("void SetPickpocketableFlag(object oItem, int bPickpocketable);") 787, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetPickpocketableFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetPickpocketableFlag) },
	{ NWSCRIPT_ACTIONNAME("GetFootstepType") NWSCRIPT_ACTIONPROTOTYPE("int GetFootstepType(object oCreature=OBJECT_SELF);") 788, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetFootstepType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetFootstepType) },
	{ NWSCRIPT_ACTIONNAME("SetFootstepType") NWSCRIPT_ACTIONPROTOTYPE("void SetFootstepType(int nFootstepType, object oCreature=OBJECT_SELF);") 789, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetFootstepType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetFootstepType) },
	{ NWSCRIPT_ACTIONNAME("GetCreatureWingType") NWSCRIPT_ACTIONPROTOTYPE("int GetCreatureWingType(object oCreature=OBJECT_SELF);") 790, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetCreatureWingType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCreatureWingType) },
	{ NWSCRIPT_ACTIONNAME("SetCreatureWingType") NWSCRIPT_ACTIONPROTOTYPE("void SetCreatureWingType(int nWingType, object oCreature=OBJECT_SELF);") 791, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetCreatureWingType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetCreatureWingType) },
	{ NWSCRIPT_ACTIONNAME("GetCreatureBodyPart") NWSCRIPT_ACTIONPROTOTYPE("int GetCreatureBodyPart(int nPart, object oCreature=OBJECT_SELF);") 792, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetCreatureBodyPart NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCreatureBodyPart) },
	{ NWSCRIPT_ACTIONNAME("SetCreatureBodyPart") NWSCRIPT_ACTIONPROTOTYPE("void SetCreatureBodyPart(int nPart, int nModelNumber, object oCreature=OBJECT_SELF);") 793, 2, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetCreatureBodyPart NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetCreatureBodyPart) },
	{ NWSCRIPT_ACTIONNAME("GetCreatureTailType") NWSCRIPT_ACTIONPROTOTYPE("int GetCreatureTailType(object oCreature=OBJECT_SELF);") 794, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetCreatureTailType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetCreatureTailType) },
	{ NWSCRIPT_ACTIONNAME("SetCreatureTailType") NWSCRIPT_ACTIONPROTOTYPE("void SetCreatureTailType(int nTailType, object oCreature=OBJECT_SELF);") 795, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetCreatureTailType NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetCreatureTailType) },
	{ NWSCRIPT_ACTIONNAME("GetHardness") NWSCRIPT_ACTIONPROTOTYPE("int GetHardness(object oObject=OBJECT_SELF);") 796, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetHardness NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetHardness) },
	{ NWSCRIPT_ACTIONNAME("SetHardness") NWSCRIPT_ACTIONPROTOTYPE("void SetHardness(int nHardness, object oObject=OBJECT_SELF);") 797, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetHardness NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetHardness) },
	{ NWSCRIPT_ACTIONNAME("SetLockKeyRequired") NWSCRIPT_ACTIONPROTOTYPE("void SetLockKeyRequired(object oObject, int nKeyRequired=TRUE);") 798, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetLockKeyRequired NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetLockKeyRequired) },
	{ NWSCRIPT_ACTIONNAME("SetLockKeyTag") NWSCRIPT_ACTIONPROTOTYPE("void SetLockKeyTag(object oObject, string sNewKeyTag);") 799, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetLockKeyTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetLockKeyTag) },
	{ NWSCRIPT_ACTIONNAME("SetLockLockable") NWSCRIPT_ACTIONPROTOTYPE("void SetLockLockable(object oObject, int nLockable=TRUE);") 800, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetLockLockable NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetLockLockable) },
	{ NWSCRIPT_ACTIONNAME("SetLockUnlockDC") NWSCRIPT_ACTIONPROTOTYPE("void SetLockUnlockDC(object oObject, int nNewUnlockDC);") 801, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetLockUnlockDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetLockUnlockDC) },
	{ NWSCRIPT_ACTIONNAME("SetLockLockDC") NWSCRIPT_ACTIONPROTOTYPE("void SetLockLockDC(object oObject, int nNewLockDC);") 802, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetLockLockDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetLockLockDC) },
	{ NWSCRIPT_ACTIONNAME("SetTrapDisarmable") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapDisarmable(object oTrapObject, int nDisarmable=TRUE);") 803, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetTrapDisarmable NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetTrapDisarmable) },
	{ NWSCRIPT_ACTIONNAME("SetTrapDetectable") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapDetectable(object oTrapObject, int nDetectable=TRUE);") 804, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetTrapDetectable NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetTrapDetectable) },
	{ NWSCRIPT_ACTIONNAME("SetTrapOneShot") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapOneShot(object oTrapObject, int nOneShot=TRUE);") 805, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetTrapOneShot NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetTrapOneShot) },
	{ NWSCRIPT_ACTIONNAME("SetTrapKeyTag") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapKeyTag(object oTrapObject, string sKeyTag);") 806, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetTrapKeyTag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetTrapKeyTag) },
	{ NWSCRIPT_ACTIONNAME("SetTrapDisarmDC") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapDisarmDC(object oTrapObject, int nDisarmDC);") 807, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetTrapDisarmDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetTrapDisarmDC) },
	{ NWSCRIPT_ACTIONNAME("SetTrapDetectDC") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapDetectDC(object oTrapObject, int nDetectDC);") 808, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetTrapDetectDC NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetTrapDetectDC) },
	{ NWSCRIPT_ACTIONNAME("CreateTrapAtLocation") NWSCRIPT_ACTIONPROTOTYPE("object CreateTrapAtLocation(int nTrapType, location lLocation, float fSize=2.0f, string sTag="", int nFaction=STANDARD_FACTION_HOSTILE, string sOnDisarmScript="", string sOnTrapTriggeredScript="");") 809, 2, 7, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_CreateTrapAtLocation NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_CreateTrapAtLocation) },
	{ NWSCRIPT_ACTIONNAME("CreateTrapOnObject") NWSCRIPT_ACTIONPROTOTYPE("void CreateTrapOnObject(int nTrapType, object oObject, int nFaction=STANDARD_FACTION_HOSTILE, string sOnDisarmScript="", string sOnTrapTriggeredScript="");") 810, 2, 5, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_CreateTrapOnObject NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_CreateTrapOnObject) },
	{ NWSCRIPT_ACTIONNAME("SetWillSavingThrow") NWSCRIPT_ACTIONPROTOTYPE("void SetWillSavingThrow(object oObject, int nWillSave);") 811, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetWillSavingThrow NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetWillSavingThrow) },
	{ NWSCRIPT_ACTIONNAME("SetReflexSavingThrow") NWSCRIPT_ACTIONPROTOTYPE("void SetReflexSavingThrow(object oObject, int nReflexSave);") 812, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetReflexSavingThrow NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetReflexSavingThrow) },
	{ NWSCRIPT_ACTIONNAME("SetFortitudeSavingThrow") NWSCRIPT_ACTIONPROTOTYPE("void SetFortitudeSavingThrow(object oObject, int nFortitudeSave);") 813, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetFortitudeSavingThrow NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetFortitudeSavingThrow) },
	{ NWSCRIPT_ACTIONNAME("GetTilesetResRef") NWSCRIPT_ACTIONPROTOTYPE("string GetTilesetResRef(object oArea);") 814, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetTilesetResRef NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTilesetResRef) },
	{ NWSCRIPT_ACTIONNAME("GetTrapRecoverable") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapRecoverable(object oTrapObject);") 815, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTrapRecoverable NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTrapRecoverable) },
	{ NWSCRIPT_ACTIONNAME("SetTrapRecoverable") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapRecoverable(object oTrapObject, int nRecoverable=TRUE);") 816, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetTrapRecoverable NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetTrapRecoverable) },
	{ NWSCRIPT_ACTIONNAME("GetModuleXPScale") NWSCRIPT_ACTIONPROTOTYPE("int GetModuleXPScale();") 817, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetModuleXPScale NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetModuleXPScale) },
	{ NWSCRIPT_ACTIONNAME("SetModuleXPScale") NWSCRIPT_ACTIONPROTOTYPE("void SetModuleXPScale(int nXPScale);") 818, 1, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetModuleXPScale NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetModuleXPScale) },
	{ NWSCRIPT_ACTIONNAME("GetKeyRequiredFeedback") NWSCRIPT_ACTIONPROTOTYPE("string GetKeyRequiredFeedback(object oObject);") 819, 1, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetKeyRequiredFeedback NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetKeyRequiredFeedback) },
	{ NWSCRIPT_ACTIONNAME("SetKeyRequiredFeedback") NWSCRIPT_ACTIONPROTOTYPE("void SetKeyRequiredFeedback(object oObject, string sFeedbackMessage);") 820, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetKeyRequiredFeedback NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetKeyRequiredFeedback) },
	{ NWSCRIPT_ACTIONNAME("GetTrapActive") NWSCRIPT_ACTIONPROTOTYPE("int GetTrapActive(object oTrapObject);") 821, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetTrapActive NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetTrapActive) },
	{ NWSCRIPT_ACTIONNAME("SetTrapActive") NWSCRIPT_ACTIONPROTOTYPE("void SetTrapActive(object oTrapObject, int nActive=TRUE);") 822, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetTrapActive NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetTrapActive) },
	{ NWSCRIPT_ACTIONNAME("LockCameraPitch") NWSCRIPT_ACTIONPROTOTYPE("void LockCameraPitch(object oPlayer, int bLocked=TRUE);") 823, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_LockCameraPitch NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_LockCameraPitch) },
	{ NWSCRIPT_ACTIONNAME("LockCameraDistance") NWSCRIPT_ACTIONPROTOTYPE("void LockCameraDistance(object oPlayer, int bLocked=TRUE);") 824, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_LockCameraDistance NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_LockCameraDistance) },
	{ NWSCRIPT_ACTIONNAME("LockCameraDirection") NWSCRIPT_ACTIONPROTOTYPE("void LockCameraDirection(object oPlayer, int bLocked=TRUE);") 825, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_LockCameraDirection NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_LockCameraDirection) },
	{ NWSCRIPT_ACTIONNAME("GetPlaceableLastClickedBy") NWSCRIPT_ACTIONPROTOTYPE("object GetPlaceableLastClickedBy();") 826, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetPlaceableLastClickedBy NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPlaceableLastClickedBy) },
	{ NWSCRIPT_ACTIONNAME("GetInfiniteFlag") NWSCRIPT_ACTIONPROTOTYPE("int GetInfiniteFlag(object oItem);") 827, 1, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetInfiniteFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetInfiniteFlag) },
	{ NWSCRIPT_ACTIONNAME("SetInfiniteFlag") NWSCRIPT_ACTIONPROTOTYPE("void SetInfiniteFlag(object oItem, int bInfinite=TRUE);") 828, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetInfiniteFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetInfiniteFlag) },
	{ NWSCRIPT_ACTIONNAME("GetAreaSize") NWSCRIPT_ACTIONPROTOTYPE("int GetAreaSize(int nAreaDimension, object oArea=OBJECT_INVALID);") 829, 1, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetAreaSize NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetAreaSize) },
	{ NWSCRIPT_ACTIONNAME("SetName") NWSCRIPT_ACTIONPROTOTYPE("void SetName(object oObject, string sNewName="");") 830, 1, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetName NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetName) },
	{ NWSCRIPT_ACTIONNAME("GetPortraitId") NWSCRIPT_ACTIONPROTOTYPE("int GetPortraitId(object oTarget=OBJECT_SELF);") 831, 0, 1, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetPortraitId NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPortraitId) },
	{ NWSCRIPT_ACTIONNAME("SetPortraitId") NWSCRIPT_ACTIONPROTOTYPE("void SetPortraitId(object oTarget, int nPortraitId);") 832, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetPortraitId NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetPortraitId) },
	{ NWSCRIPT_ACTIONNAME("GetPortraitResRef") NWSCRIPT_ACTIONPROTOTYPE("string GetPortraitResRef(object oTarget=OBJECT_SELF);") 833, 0, 1, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetPortraitResRef NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPortraitResRef) },
	{ NWSCRIPT_ACTIONNAME("SetPortraitResRef") NWSCRIPT_ACTIONPROTOTYPE("void SetPortraitResRef(object oTarget, string sPortraitResRef);") 834, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetPortraitResRef NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetPortraitResRef) },
	{ NWSCRIPT_ACTIONNAME("SetUseableFlag") NWSCRIPT_ACTIONPROTOTYPE("void SetUseableFlag(object oPlaceable, int nUseableFlag);") 835, 2, 2, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetUseableFlag NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetUseableFlag) },
	{ NWSCRIPT_ACTIONNAME("GetDescription") NWSCRIPT_ACTIONPROTOTYPE("string GetDescription(object oObject, int bOriginalDescription=FALSE, int bIdentifiedDescription=TRUE);") 836, 1, 3, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetDescription NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetDescription) },
	{ NWSCRIPT_ACTIONNAME("SetDescription") NWSCRIPT_ACTIONPROTOTYPE("void SetDescription(object oObject, string sNewDescription="", int bIdentifiedDescription=TRUE);") 837, 1, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetDescription NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetDescription) },
	{ NWSCRIPT_ACTIONNAME("GetPCChatSpeaker") NWSCRIPT_ACTIONPROTOTYPE("object GetPCChatSpeaker();") 838, 0, 0, ACTIONTYPE_OBJECT, NWN1_NWActionParameterTypes_GetPCChatSpeaker NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPCChatSpeaker) },
	{ NWSCRIPT_ACTIONNAME("GetPCChatMessage") NWSCRIPT_ACTIONPROTOTYPE("string GetPCChatMessage();") 839, 0, 0, ACTIONTYPE_STRING, NWN1_NWActionParameterTypes_GetPCChatMessage NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPCChatMessage) },
	{ NWSCRIPT_ACTIONNAME("GetPCChatVolume") NWSCRIPT_ACTIONPROTOTYPE("int GetPCChatVolume();") 840, 0, 0, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetPCChatVolume NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetPCChatVolume) },
	{ NWSCRIPT_ACTIONNAME("SetPCChatMessage") NWSCRIPT_ACTIONPROTOTYPE("void SetPCChatMessage(string sNewChatMessage="");") 841, 0, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetPCChatMessage NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetPCChatMessage) },
	{ NWSCRIPT_ACTIONNAME("SetPCChatVolume") NWSCRIPT_ACTIONPROTOTYPE("void SetPCChatVolume(int nTalkVolume=TALKVOLUME_TALK);") 842, 0, 1, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetPCChatVolume NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetPCChatVolume) },
	{ NWSCRIPT_ACTIONNAME("GetColor") NWSCRIPT_ACTIONPROTOTYPE("int GetColor(object oObject, int nColorChannel);") 843, 2, 2, ACTIONTYPE_INT, NWN1_NWActionParameterTypes_GetColor NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_GetColor) },
	{ NWSCRIPT_ACTIONNAME("SetColor") NWSCRIPT_ACTIONPROTOTYPE("void SetColor(object oObject, int nColorChannel, int nColorValue);") 844, 3, 3, ACTIONTYPE_VOID, NWN1_NWActionParameterTypes_SetColor NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_SetColor) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyMaterial") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyMaterial(int nMaterialType);") 845, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyMaterial NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyMaterial) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyQuality") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyQuality(int nQuality);") 846, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyQuality NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyQuality) },
	{ NWSCRIPT_ACTIONNAME("ItemPropertyAdditional") NWSCRIPT_ACTIONPROTOTYPE("itemproperty ItemPropertyAdditional(int nAdditionalProperty);") 847, 1, 1, ACTIONTYPE_ITEMPROPERTY, NWN1_NWActionParameterTypes_ItemPropertyAdditional NWSCRIPT_ACTIONPARAMETERSIZES(NWN1_NWActionTotalParameterSizes_ItemPropertyAdditional) }
};
