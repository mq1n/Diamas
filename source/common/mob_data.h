#pragma once
#include <cstdint>
#include "defines.h"
#include "constants.h"

enum EOnClickEvents
{
	ON_CLICK_NONE,
	ON_CLICK_SHOP,
	ON_CLICK_TALK,
	ON_CLICK_MAX_NUM
};

enum EMobSizes
{
	MOBSIZE_RESERVED,
	MOBSIZE_SMALL,
	MOBSIZE_MEDIUM,
	MOBSIZE_BIG
};

enum EAIFlags
{
	AIFLAG_AGGRESSIVE = (1 << 0),
	AIFLAG_NOMOVE = (1 << 1),
	AIFLAG_COWARD = (1 << 2),
	AIFLAG_NOATTACKSHINSU = (1 << 3),
	AIFLAG_NOATTACKJINNO = (1 << 4),
	AIFLAG_NOATTACKCHUNJO = (1 << 5),
	AIFLAG_ATTACKMOB = (1 << 6),
	AIFLAG_BERSERK = (1 << 7),
	AIFLAG_STONESKIN = (1 << 8),
	AIFLAG_GODSPEED = (1 << 9),
	AIFLAG_DEATHBLOW = (1 << 10),
	AIFLAG_REVIVE = (1 << 11),
};

enum EMobStatType
{
	MOB_STATTYPE_POWER,
	MOB_STATTYPE_TANKER,
	MOB_STATTYPE_SUPER_POWER,
	MOB_STATTYPE_SUPER_TANKER,
	MOB_STATTYPE_RANGE,
	MOB_STATTYPE_MAGIC,
	MOB_STATTYPE_MAX_NUM
};

enum EMobEnchants
{
	MOB_ENCHANT_CURSE,
	MOB_ENCHANT_SLOW,
	MOB_ENCHANT_POISON,
	MOB_ENCHANT_STUN,
	MOB_ENCHANT_CRITICAL,
	MOB_ENCHANT_PENETRATE,
#if defined(ENABLE_WOLFMAN_CHARACTER) && !defined(USE_MOB_BLEEDING_AS_POISON)
	MOB_ENCHANT_BLEEDING,
#endif
	MOB_ENCHANTS_MAX_NUM
};

enum EMobResists
{
	MOB_RESIST_SWORD,
	MOB_RESIST_TWOHAND,
	MOB_RESIST_DAGGER,
	MOB_RESIST_BELL,
	MOB_RESIST_FAN,
	MOB_RESIST_BOW,
	MOB_RESIST_FIRE,
	MOB_RESIST_ELECT,
	MOB_RESIST_MAGIC,
	MOB_RESIST_WIND,
	MOB_RESIST_POISON,
#if defined(ENABLE_WOLFMAN_CHARACTER) && !defined(USE_MOB_CLAW_AS_DAGGER)
	MOB_RESIST_CLAW,
#endif
#if defined(ENABLE_WOLFMAN_CHARACTER) && !defined(USE_MOB_BLEEDING_AS_POISON)
	MOB_RESIST_BLEEDING,
#endif
	MOB_RESISTS_MAX_NUM
};

enum EMobTypes
{
	MONSTER,
	NPC,
	STONE,
	WARP,
	DOOR,
	BUILDING,
	PC,
	POLYMORPH_PC,
	HORSE,
	GOTO
};


#pragma pack(push)
#pragma pack(1)
typedef struct SMobSkillLevel
{
	uint32_t       dwVnum;
	uint8_t        bLevel;
} TMobSkillLevel;

typedef struct SMobTable
{
	uint32_t       dwVnum;
	char        szName[CHARACTER_NAME_MAX_LEN + 1];
	char        szLocaleName[CHARACTER_NAME_MAX_LEN + 1];

	uint8_t        bType;                  // Monster, NPC
	uint8_t        bRank;                  // PAWN, KNIGHT, KING
	uint8_t        bBattleType;            // MELEE, etc..
	uint8_t        bLevel;                 // Level
	uint8_t        bSize;

	uint32_t       dwGoldMin;
	uint32_t       dwGoldMax;
	uint32_t       dwExp;
	uint32_t       dwMaxHP;
	uint8_t        bRegenCycle;
	uint8_t        bRegenPercent;
	uint16_t        wDef;

	uint32_t       dwAIFlag;
	uint32_t       dwRaceFlag;
	uint32_t       dwImmuneFlag;

	uint8_t        bStr, bDex, bCon, bInt;
	uint32_t       dwDamageRange[2];

	int16_t       sAttackSpeed;
	int16_t       sMovingSpeed;
	uint8_t        bAggresiveHPPct;
	uint16_t        wAggressiveSight;
	uint16_t        wAttackRange;

	char        cEnchants[MOB_ENCHANTS_MAX_NUM];
	char        cResists[MOB_RESISTS_MAX_NUM];

	uint32_t       dwResurrectionVnum;
	uint32_t       dwDropItemVnum;

	uint8_t        bMountCapacity;
	uint8_t        bOnClickType;

	uint8_t        bEmpire;
	char        szFolder[64 + 1];

	float       fDamMultiply;

	uint32_t       dwSummonVnum;
	uint32_t       dwDrainSP;
	uint32_t		dwMonsterColor;
	uint32_t       dwPolymorphItemVnum;

	TMobSkillLevel	Skills[MOB_SKILL_MAX_NUM];

	uint8_t		bBerserkPoint;
	uint8_t		bStoneSkinPoint;
	uint8_t		bGodSpeedPoint;
	uint8_t		bDeathBlowPoint;
	uint8_t		bRevivePoint;

	bool operator<(const SMobTable& rhs)
	{
		return dwVnum < rhs.dwVnum;
	}
} TMobTable;

#pragma pack(pop)

