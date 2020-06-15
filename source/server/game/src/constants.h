#ifndef __INC_METIN_II_GAME_CONSTANTS_H__
#define __INC_METIN_II_GAME_CONSTANTS_H__

#include "../../common/tables.h"

// MAP_INDEX
enum EMapIndexList
{
	HOME_MAP_INDEX_RED_1 = 1,
	HOME_MAP_INDEX_RED_2 = 3,
	GUILD_MAP_INDEX_RED = 5,
	HOME_MAP_INDEX_YELLOW_1 = 21,
	HOME_MAP_INDEX_YELLOW_2 = 23,
	GUILD_MAP_INDEX_YELLOW = 25,
	HOME_MAP_INDEX_BLUE_1 = 41,
	HOME_MAP_INDEX_BLUE_2 = 43,
	GUILD_MAP_INDEX_BLUE = 45,

	SNOW_MAP_INDEX = 61,
	FLAME_MAP_INDEX = 62,
	DESERT_MAP_INDEX = 63,
	THREEWAY_MAP_INDEX = 64,
	MILGYO_MAP_INDEX = 65,
	TRENT_1_MAP_INDEX = 66,
	TRENT_2_MAP_INDEX = 67,
	NUSLUCK_MAP_INDEX = 68,
	DUEL_MAP_INDEX = 69,

	MONKEY_EASY_MAP_INDEX_1 = 71,
	MONKEY_EASY_MAP_INDEX_2 = 72,
	MONKEY_EASY_MAP_INDEX_3 = 73,
	MONKEY_MEDIUM_MAP_INDEX = 74,
	MONKEY_EXPERT_MAP_INDEX = 75,
	DEVILTOWER_MAP_INDEX = 76,
	DEVILSCATACOMB_MAP_INDEX = 77,
	SPIDER_MAP_INDEX_1 = 78,
	SPIDER_MAP_INDEX_2 = 79,
	SKIPIA_MAP_INDEX_1 = 80,
	SKIPIA_MAP_INDEX_2 = 81,
	SKIPIA_MAP_INDEX_BOSS = 82,

	GUILD_WAR_MAP_INDEX = 91,
	GUILD_FLAG_WAR_MAP_INDEX = 92,
	WEDDING_MAP_INDEX = 93,
	OXEVENT_MAP_INDEX = 94,
};
// MAP_INDEX_END

typedef struct SNPCMovingPosition
{
	int32_t	lX;
	int32_t	lY;
	bool	bRun;
} TNPCMovingPosition;

enum EMonsterChatState
{
	MONSTER_CHAT_WAIT		= 0,
	MONSTER_CHAT_ATTACK		= 1,
	MONSTER_CHAT_ATTACKED	= 2,
};

typedef struct SMobRankStat
{
	int32_t iGoldPercent;   // ���� ���� Ȯ��
} TMobRankStat;

typedef struct SMobStat
{
	uint8_t	byLevel;
	uint16_t	HP;
	uint32_t	dwExp;
	uint16_t	wDefGrade;
} TMobStat;

typedef struct SBattleTypeStat
{
	int32_t		AttGradeBias;
	int32_t		DefGradeBias;
	int32_t		MagicAttGradeBias;
	int32_t		MagicDefGradeBias;
} TBattleTypeStat;

typedef struct SJobInitialPoints
{
	int32_t		st, ht, dx, iq;
	int32_t		max_hp, max_sp;
	int32_t		hp_per_ht, sp_per_iq;
	int32_t		hp_per_lv_begin, hp_per_lv_end;
	int32_t		sp_per_lv_begin, sp_per_lv_end;
	int32_t		max_stamina;
	int32_t		stamina_per_con;
	int32_t		stamina_per_lv_begin, stamina_per_lv_end;
} TJobInitialPoints;

typedef struct __coord
{
	int32_t		x, y;
} Coord;

typedef struct SApplyInfo
{
	uint8_t	bPointType;                          // APPLY -> POINT
} TApplyInfo;

const int32_t STONE_INFO_MAX_NUM = 10;
const int32_t STONE_LEVEL_MAX_NUM = 4;

struct SStoneDropInfo
{
	uint32_t dwMobVnum;
	int32_t iDropPct;
	int32_t iLevelPct[STONE_LEVEL_MAX_NUM+1];
};

inline bool operator < (const SStoneDropInfo& l, uint32_t r)
{
	return l.dwMobVnum < r;
}

inline bool operator < (uint32_t l, const SStoneDropInfo& r)
{
	return l < r.dwMobVnum;
}

inline bool operator < (const SStoneDropInfo& l, const SStoneDropInfo& r)
{
	return l.dwMobVnum < r.dwMobVnum;
}

extern const TApplyInfo		aApplyInfo[MAX_APPLY_NUM];
extern const TMobRankStat       MobRankStats[MOB_RANK_MAX_NUM];

extern TBattleTypeStat		BattleTypeStats[BATTLE_TYPE_MAX_NUM];

extern const uint32_t		party_exp_distribute_table[PLAYER_EXP_TABLE_MAX + 1];

extern const uint32_t		exp_table_common[PLAYER_MAX_LEVEL_CONST + 1];

extern const uint32_t*		exp_table;

extern const uint32_t		guild_exp_table[GUILD_MAX_LEVEL + 1];
extern const uint32_t		guild_exp_table2[GUILD_MAX_LEVEL + 1];

#define MAX_EXP_DELTA_OF_LEV	31
#define PERCENT_LVDELTA(me, victim) aiPercentByDeltaLev[MINMAX(0, (victim + 15) - me, MAX_EXP_DELTA_OF_LEV - 1)]
#define PERCENT_LVDELTA_BOSS(me, victim) aiPercentByDeltaLevForBoss[MINMAX(0, (victim + 15) - me, MAX_EXP_DELTA_OF_LEV - 1)]
#define CALCULATE_VALUE_LVDELTA(me, victim, val) ((val * PERCENT_LVDELTA(me, victim)) / 100)
extern const int32_t		aiPercentByDeltaLev_euckr[MAX_EXP_DELTA_OF_LEV];
extern const int32_t		aiPercentByDeltaLevForBoss_euckr[MAX_EXP_DELTA_OF_LEV];
extern const int32_t *		aiPercentByDeltaLev;
extern const int32_t *		aiPercentByDeltaLevForBoss;

#define ARROUND_COORD_MAX_NUM	161
extern Coord			aArroundCoords[ARROUND_COORD_MAX_NUM];
extern TJobInitialPoints	JobInitialPoints[JOB_MAX_NUM];

extern const int32_t		aiMobEnchantApplyIdx[MOB_ENCHANTS_MAX_NUM];
extern const int32_t		aiMobResistsApplyIdx[MOB_RESISTS_MAX_NUM];

extern const int32_t		aSkillAttackAffectProbByRank[MOB_RANK_MAX_NUM];

extern const int32_t aiItemMagicAttributePercentHigh[ITEM_ATTRIBUTE_MAX_LEVEL]; // 1������
extern const int32_t aiItemMagicAttributePercentLow[ITEM_ATTRIBUTE_MAX_LEVEL];

extern const int32_t aiItemAttributeAddPercent[ITEM_ATTRIBUTE_MAX_NUM];

extern const int32_t aiWeaponSocketQty[WEAPON_NUM_TYPES];
extern const int32_t aiArmorSocketQty[ARMOR_NUM_TYPES];
extern const int32_t aiSocketPercentByQty[5][4];

extern const int32_t aiExpLossPercents[PLAYER_EXP_TABLE_MAX + 1];

extern const int32_t * aiSkillPowerByLevel;
extern const int32_t aiSkillPowerByLevel_euckr[SKILL_MAX_LEVEL + 1];

extern const int32_t aiPolymorphPowerByLevel[SKILL_MAX_LEVEL + 1];

extern const int32_t aiSkillBookCountForLevelUp[10];
extern const int32_t aiGrandMasterSkillBookCountForLevelUp[10];
extern const int32_t aiGrandMasterSkillBookMinCount[10];
extern const int32_t aiGrandMasterSkillBookMaxCount[10];
extern const int32_t CHN_aiPartyBonusExpPercentByMemberCount[9];
extern const int32_t KOR_aiPartyBonusExpPercentByMemberCount[9];
extern const int32_t KOR_aiUniqueItemPartyBonusExpPercentByMemberCount[9];

typedef std::map<uint32_t, TItemAttrTable> TItemAttrMap;
extern TItemAttrMap g_map_itemAttr;
extern TItemAttrMap g_map_itemRare;

extern const int32_t * aiChainLightningCountBySkillLevel;
extern const int32_t aiChainLightningCountBySkillLevel_euckr[SKILL_MAX_LEVEL + 1];

extern const char * c_apszEmpireNames[EMPIRE_MAX_NUM];
extern const char * c_apszEmpireNamesAlt[EMPIRE_MAX_NUM];
extern const char * c_apszPrivNames[MAX_PRIV_NUM];
extern const SStoneDropInfo aStoneDrop[STONE_INFO_MAX_NUM];

typedef struct
{
	int32_t lMapIndex;
	int32_t iWarPrice;
	int32_t iWinnerPotionRewardPctToWinner;
	int32_t iLoserPotionRewardPctToWinner;
	int32_t iInitialScore;
	int32_t iEndScore;
} TGuildWarInfo;

extern TGuildWarInfo KOR_aGuildWarInfo[GUILD_WAR_TYPE_MAX_NUM];

// ACCESSORY_REFINE
enum 
{
	ITEM_ACCESSORY_SOCKET_MAX_NUM = 3
};

extern const int32_t aiAccessorySocketAddPct[ITEM_ACCESSORY_SOCKET_MAX_NUM];
extern const int32_t aiAccessorySocketEffectivePct[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1];
extern const int32_t aiAccessorySocketDegradeTime[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1];
extern const int32_t aiAccessorySocketPutPct[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1];
int32_t FN_get_apply_type(const char *apply_type_string);

// END_OF_ACCESSORY_REFINE

int32_t FN_get_apply_type(const char *apply_type_string);
#endif

