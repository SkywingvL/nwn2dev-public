/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	AISystem.h

Abstract:

	This module defines types and constants used in the AI system.

--*/

#ifndef _SOURCE_PROGRAMS_NWNBASELIB_AISYSTEM_H
#define _SOURCE_PROGRAMS_NWNBASELIB_AISYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

namespace NWN
{
	//
	// AI Actions.
	//

	typedef enum _AIACTION
	{
		AIACTION_INVALID = 0,
		AIACTION_MOVETOPOINT = 1,
		AIACTION_CHECKMOVETOOBJECT = 2,
		AIACTION_CHECKMOVEAWAYFROMOBJECT = 3,
		AIACTION_UNUSED1 = 4,
		AIACTION_JUMPTOPOINT = 5,
		AIACTION_PLAYANIMATION = 6,
		AIACTION_PICKUPITEM = 7,
		AIACTION_EQUIPITEM = 8,
		AIACTION_DROPITEM = 9,
		AIACTION_CHECKMOVETOPOINT = 10,
		AIACTION_UNEQUIPITEM = 11,
		AIACTION_ATTACKOBJECT = 12,
		AIACTION_REPOSITORYMOVE = 13,
		AIACTION_SPEAK = 14,
		AIACTION_CASTSPELL = 15,
		AIACTION_WAITFORENDOFROUND = 16,
		AIACTION_CHECKMOVETOOBJECTRADIUS = 17,
		AIACTION_CHECKMOVETOPOINTRADIUS = 18,
		AIACTION_CHANGEFACINGOBJECT = 19,
		AIACTION_OPENDOOR = 20,
		AIACTION_CLOSEDOOR = 21,
		AIACTION_ORIENTCAMERA = 22,
		AIACTION_PLAYSOUND = 23,
		AIACTION_DIALOGOBJECT = 24,
		AIACTION_DISABLETRAP = 25,
		AIACTION_RECOVERTRAP = 26,
		AIACTION_FLAGTRAP = 27,
		AIACTION_EXAMINETRAP = 28,
		AIACTION_SETTRAP = 29,
		AIACTION_WAIT = 30,
		AIACTION_PAUSEDIALOG = 31,
		AIACTION_RESUMEDIALOG = 32,
		AIACTION_SPEAKSTRREF = 33,
		AIACTION_GIVEITEM = 34,
		AIACTION_TAKEITEM = 35,
		AIACTION_ENCOUNTERCREATUREDESTROYSELF = 36,
		AIACTION_DOCOMMAND = 37,
		AIACTION_OPENLOCK = 38,
		AIACTION_LOCK = 39,
		AIACTION_USEOBJECT = 40,
		AIACTION_ANIMALEMPATHY = 41,
		AIACTION_REST = 42,
		AIACTION_TAUNT = 43,
		AIACTION_CHECKMOVEAWAYFROMLOCATION = 44,
		AIACTION_RANDOMWALK = 45,
		AIACTION_ITEMCASTSPELL = 46,
		AIACTION_SETCOMMANDABLE = 47,
		AIACTION_JUMPTOOBJECT = 48,
		AIACTION_CHANGEFACINGPOINT = 49,
		AIACTION_COUNTERSPELL = 50,
		AIACTION_DRIVE = 51,
		AIACTION_APPEAR = 52,
		AIACTION_DISAPPEAR = 53,
		AIACTION_SLEIGHT_OF_HAND = 54,
		AIACTION_FORCEFOLLOWOBJECT = 55,
		AIACTION_HEAL = 56,
		AIACTION_SIT = 57,
		AIACTION_CHECKFORCEFOLLOWOBJECT = 58,
		AIACTION_BARTER = 59,
		AIACTION_AREAWAIT = 60,
		AIACTION_EXAMINE = 61,
		AIACTION_MOUNT = 62,
		AIACTION_DISMOUNT = 63,
		AIACTION_PAUSECUTSCENE = 64,
		AIACTION_COUNT = 65
	} AIACTION, * PAIACTION;

	typedef const enum _AIACTION * PCAIACTION;

	typedef enum _AISTATEACTION
	{
		AIStateAction_Ready                 = 0,
		AIStateAction_Movement,
		AIStateAction_Combat,
		AIStateAction_Counterspell,
		AIStateAction_Unknown0,
		AIStateAction_Unknown1,
		AIStateAction_Unknown2,
		AIStateAction_Unknown3,
		AIStateAction_Unknown4,
		AIStateAction_Unknown5,
		AIStateAction_PickUpItem,
		AIStateAction_DropItem,
		AIStateAction_EquipItem,
		AIStateAction_UnequipItem,
		AIStateAction_CastingSpell,
		AIStateAction_OpenObject,
		AIStateAction_CloseObject,
		AIStateAction_DisableTrap,
		AIStateAction_RecoverTrap,
		AIStateAction_FlagTrap,
		AIStateAction_ExamineTrap,
		AIStateAction_SetTrap,
		AIStateAction_GiveItem,
		AIStateAction_TakeItem,
		AIStateAction_OpenLock,
		AIStateAction_Lock,
		AIStateAction_UseObject,
		AIStateAction_AnimalEmpathy,
		AIStateAction_Rest,
		AIStateAction_SleightOfHand,
		AIStateAction_Heal,
		AIStateAction_Sit,
		AIStateAction_Barter,

		LastAIStateAction
	} AISTATE_ACTION, * PAISTATEACTION;

	typedef const enum _AISTATEACTION * PCAISTATEACTION;

	typedef enum _AISTATE_FLAG
	{
		AISTATE_FLAG_ALIVE = 0x0002,

		AISTATE_FLAG_ALL   = 0xFFFF
	} AISTATE_FLAG, * PAISTATE_FLAG;

	typedef const enum _AISTATEFLAG * PCAISTATEFLAG;

	//
	// Action queue action codes.  These must match actions.2da.
	//

	typedef enum _NWACTION
	{
		NWACTION_MOVETOPOINT                   = 0,
		NWACTION_PICKUPITEM                    = 1,
		NWACTION_DROPITEM                      = 2,
		NWACTION_ATTACKOBJECT                  = 3,
		NWACTION_CASTSPELL                     = 4,
		NWACTION_OPENDOOR                      = 5,
		NWACTION_CLOSEDOOR                     = 6,
		NWACTION_DIALOGOBJECT                  = 7,
		NWACTION_DISABLETRAP                   = 8,
		NWACTION_RECOVERTRAP                   = 9,
		NWACTION_FLAGTRAP                      = 10,
		NWACTION_EXAMINETRAP                   = 11,
		NWACTION_SETTRAP                       = 12,
		NWACTION_OPENLOCK                      = 13,
		NWACTION_LOCK                          = 14,
		NWACTION_USEOBJECT                     = 15,
		NWACTION_ANIMALEMPATHY                 = 16,
		NWACTION_REST                          = 17,
		NWACTION_TAUNT                         = 18,
		NWACTION_ITEMCASTSPELL                 = 19,
		NWACTION_DISARM                        = 20,
		NWACTION_IMPROVEDDISARM                = 21,
		NWACTION_KNOCKDOWN                     = 22,
		NWACTION_IMPROVEDKNOCKDOWN             = 23,
		NWACTION_MANYSHOT                      = 24,
		NWACTION_SAP                           = 25,
		NWACTION_STUNNINGFIST                  = 26,
		NWACTION_FLURRYOFBLOWS                 = 27,
		NWACTION_QUIVERINGPALM                 = 28,
		NWACTION_CALLEDSHOTLEG                 = 29,
		NWACTION_CALLEDSHOTARM                 = 30,
		NWACTION_COUNTERSPELL                  = 31,
		NWACTION_SMITEEVIL_1                   = 32,
		NWACTION_HEAL                          = 33,
		NWACTION_PICKPOCKET                    = 34,
		NWACTION_FOLLOW                        = 35,
		NWACTION_WAIT                          = 36,
		NWACTION_SIT                           = 37,
		NWACTION_BARTER                        = 38,
		NWACTION_DIRTYFIGHTING                 = 39,
		NWACTION_SMITEGOOD                     = 40,
		NWACTION_KIDAMAGE                      = 41,
		NWACTION_DEFENSIVESTANCE               = 42,
		NWACTION_RANDOMWALK                    = 43,
		NWACTION_MOUNT                         = 44,
		NWACTION_DISMOUNT                      = 45,
		NWACTION_FEINT                         = 46,
		NWACTION_SMITEEVIL_2                   = 47,
		NWACTION_SMITEEVIL_3                   = 48,
		NWACTION_SMITEEVIL_4                   = 49,
		NWACTION_SMITEEVIL_5                   = 50,
		NWACTION_SMITEINFIDEL                  = 51,
		NWACTION_FLOURISH                      = 52,
		NWACTION_IMPROMPTU_SNEAK_ATTACK_1      = 53,
		NWACTION_IMPROMPTU_SNEAK_ATTACK_2      = 54,
		NWACTION_CIRCLE_KICK                   = 55,

		NWACTION_WAITFORENDOFROUND             = 60,

		NWACTION_LAST
	} NWACTION, * PNWACTION;

	typedef const enum _NWACTION * PCNWACTION;

	//
	// Define the action group id type.  Action group ids are incrementing
	// serial numbers (may wrap) that are used to group related actions
	// together.  For example, an attempt to open a door may involve both a
	// move to point and open door action, which would share the same action
	// group id.
	//

	typedef unsigned short NWACTION_GROUP_ID;

}

#endif

