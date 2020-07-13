#ifndef __INC_TABLES_H__
#define __INC_TABLES_H__

#include "../../common/common_incl.hpp"
#include <cstdint>
#include <cstring>


/* ----------------------------------------------
 * table
 * ----------------------------------------------
 */

/* game Server -> DB Server */
#pragma pack(1)


typedef struct SAccountTable
{
	uint32_t		id;
	char		login[LOGIN_MAX_LEN + 1];
	char		passwd[PASSWD_MAX_LEN + 1];
	char		social_id[SOCIAL_ID_MAX_LEN + 1];
	char		status[ACCOUNT_STATUS_MAX_LEN + 1];
	uint8_t		bEmpire;
	TSimplePlayerInformation	players[PLAYER_PER_ACCOUNT];
} TAccountTable;

typedef struct SPacketDGCreateSuccess
{
	uint8_t		bAccountCharacterIndex;
	TSimplePlayerInformation	player;
} TPacketDGCreateSuccess;

typedef struct SPlayerItem
{
	uint32_t	id;
	uint8_t	window;
	uint16_t	pos;
	uint32_t	count;

	uint32_t	vnum;
	int32_t	alSockets[ITEM_SOCKET_MAX_NUM];	// 소켓번호

	TPlayerItemAttribute    aAttr[ITEM_ATTRIBUTE_MAX_NUM];

	uint32_t	owner;
	bool	is_gm_owner;
} TPlayerItem;


struct	THorseInfo
{
	uint8_t	bLevel;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	bool	bRiding;
	int16_t	sStamina;
	int16_t	sHealth;
	uint32_t	dwHorseHealthDropTime;
};

typedef struct SPlayerTable
{
	uint32_t	id;

	char	name[CHARACTER_NAME_MAX_LEN + 1];
	char	ip[IP_ADDRESS_LENGTH + 1];

	uint16_t	job;
	uint8_t	voice;

	uint8_t	level;
	uint8_t	level_step;
	int16_t	st, ht, dx, iq;

	uint32_t	exp;
	int32_t		gold;

	uint8_t	dir;
	int32_t		x, y, z;
	int32_t		lMapIndex;

	int32_t	lExitX, lExitY;
	int32_t	lExitMapIndex;

	int32_t		hp;
	int32_t		sp;

	int16_t	sRandomHP;
	int16_t	sRandomSP;

	int32_t         playtime;

	int16_t	stat_point;
	int16_t	skill_point;
	int16_t	sub_skill_point;
	int16_t	horse_skill_point;

	TPlayerSkill skills[SKILL_MAX_NUM];

	TQuickSlot  quickslot[QUICKSLOT_MAX_NUM];

	uint8_t	part_base;
	uint16_t	parts[PART_MAX_NUM];

	int16_t	stamina;

	uint8_t	skill_group;
	int32_t	lAlignment;

	int16_t	stat_reset_count;

	THorseInfo	horse;

	uint32_t	logoff_interval;

	int32_t		aiPremiumTimes[PREMIUM_MAX_NUM];

	bool	is_gm_invisible;
} TPlayerTable;

enum EAttTypes
{
	ATT_ELEC,
	ATT_FIRE,
	ATT_ICE,
	ATT_WIND,
	ATT_EARTH,
	ATT_DARK,
	ATT_MAX
};

enum EResistTypes
{
	RESIST_DARK,
	RESIST_ICE,
	RESIST_EARTH,
	RESIST_MAX
};

typedef struct SMobSkillLevel
{
	uint32_t	dwVnum;
	uint8_t	bLevel;
} TMobSkillLevel;

typedef struct SEntityTable
{
	uint32_t dwVnum;
} TEntityTable;

typedef struct SMobTable : public SEntityTable
{
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	char	szLocaleName[CHARACTER_NAME_MAX_LEN + 1];

	uint8_t	bType;			// Monster, NPC
	uint8_t	bRank;			// PAWN, KNIGHT, KING
	uint8_t	bBattleType;		// MELEE, etc..
	uint8_t	bLevel;			// Level
	uint8_t	bSize;

	uint32_t	dwGoldMin;
	uint32_t	dwGoldMax;
	uint32_t	dwExp;
	uint32_t	dwMaxHP;
	uint8_t	bRegenCycle;
	uint8_t	bRegenPercent;
	uint16_t	wDef;

	uint32_t	dwAIFlag;
	uint32_t	dwRaceFlag;
	uint32_t	dwImmuneFlag;

	uint8_t	bStr, bDex, bCon, bInt;
	uint32_t	dwDamageRange[2];

	int16_t	sAttackSpeed;
	int16_t	sMovingSpeed;
	uint8_t	bAggresiveHPPct;
	uint16_t	wAggressiveSight;
	uint16_t	wAttackRange;

	char	cEnchants[MOB_ENCHANTS_MAX_NUM];
	char	cResists[MOB_RESISTS_MAX_NUM];

	uint32_t	dwResurrectionVnum;
	uint32_t	dwDropItemVnum;

	uint8_t	bMountCapacity;
	uint8_t	bOnClickType;

	uint8_t	bEmpire;
	char	szFolder[64 + 1];

	float	fDamMultiply;

	uint32_t	dwSummonVnum;
	uint32_t	dwDrainSP;
	uint32_t	dwMobColor;
	uint32_t	dwPolymorphItemVnum;

	TMobSkillLevel Skills[MOB_SKILL_MAX_NUM];

	uint8_t	bBerserkPoint;
	uint8_t	bStoneSkinPoint;
	uint8_t	bGodSpeedPoint;
	uint8_t	bDeathBlowPoint;
	uint8_t	bRevivePoint;
} TMobTable;

typedef struct SSkillTable
{
	uint32_t	dwVnum;
	char	szName[32 + 1];
	uint8_t	bType;
	uint8_t	bMaxLevel;
	uint32_t	dwSplashRange;

	char	szPointOn[64];
	char	szPointPoly[100 + 1];
	char	szSPCostPoly[100 + 1];
	char	szDurationPoly[100 + 1];
	char	szDurationSPCostPoly[100 + 1];
	char	szCooldownPoly[100 + 1];
	char	szMasterBonusPoly[100 + 1];
	//char	szAttackGradePoly[100 + 1];
	char	szGrandMasterAddSPCostPoly[100 + 1];
	uint32_t	dwFlag;
	uint32_t	dwAffectFlag;

	// Data for secondary skill
	char 	szPointOn2[64];
	char 	szPointPoly2[100 + 1];
	char 	szDurationPoly2[100 + 1];
	uint32_t 	dwAffectFlag2;

	// Data for grand master point
	char 	szPointOn3[64];
	char 	szPointPoly3[100 + 1];
	char 	szDurationPoly3[100 + 1];

	uint8_t	bLevelStep;
	uint8_t	bLevelLimit;
	uint32_t	preSkillVnum;
	uint8_t	preSkillLevel;

	int32_t	lMaxHit; 
	char	szSplashAroundDamageAdjustPoly[100 + 1];

	uint8_t	bSkillAttrType;

	uint32_t	dwTargetRange;
} TSkillTable;

typedef struct command_messenger_add_by_vid
{
	uint32_t vid;
} TPacketMessengerAddByVID;

#define QUEST_NAME_MAX_LEN	32
#define QUEST_STATE_MAX_LEN	64

typedef struct SQuestTable
{
	uint32_t		dwPID;
	char		szName[QUEST_NAME_MAX_LEN + 1];
	char		szState[QUEST_STATE_MAX_LEN + 1];
	int32_t		lValue;
} TQuestTable;

typedef struct SItemLimit
{
	uint8_t	bType;
	int32_t	lValue;
} TItemLimit;

typedef struct SItemApply
{
	uint8_t	bType;
	int32_t	lValue;
} TItemApply;

typedef struct SItemTable : public SEntityTable
{
	char* GetOriginalName()
	{
		return szName;
	}

	char* GetName()
	{
		return szLocaleName;
	}

	uint8_t GetType() const
	{
		return bType;
	}

	uint8_t GetSubType() const
	{
		return bSubType;
	}

	uint8_t GetWeight() const
	{
		return bWeight;
	}

	uint8_t GetSize() const
	{
		return bSize;
	}

	uint32_t GetAntiFlags() const
	{
		return dwAntiFlags;
	}

	uint32_t GetFlags() const
	{
		return dwAntiFlags;
	}

	uint32_t GetWearFlags() const
	{
		return dwWearFlags;
	}

	uint32_t GetImmuneFlags() const
	{
		return dwImmuneFlag;
	}

	uint32_t GetBuyPrice() const
	{
		return dwGold;
	}

	uint32_t GetSellPrice() const
	{
		return dwShopBuyPrice;
	}

	int32_t GetValue(uint32_t index)
	{
		return alValues[index];
	}

	uint8_t GetLimitType(uint32_t idx) const
	{
		return aLimits[idx].bType;
	}

	int32_t GetLimitValue(uint32_t idx) const
	{
		return aLimits[idx].lValue;
	}

	// Weapon
	bool IsWeapon() const
	{
		return GetType() == ITEM_WEAPON;
	}

	bool IsSword() const
	{
		return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_SWORD;
	}

	bool IsDagger() const
	{
		return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_DAGGER;
	}

	bool IsBow() const
	{
		return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_BOW;
	}

	bool IsTwoHandSword() const
	{
		return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_TWO_HANDED;
	}

	bool IsBell() const
	{
		return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_BELL;
	}

	bool IsFan() const
	{
		return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_FAN;
	}

	bool IsArrow() const
	{
		return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_ARROW;
	}

	bool IsMountSpear() const
	{
		return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_MOUNT_SPEAR;
	}

	bool IsClaw() const
	{
#ifdef ENABLE_WOLFMAN_CHARACTER
		return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_CLAW;
#else
		return false;
#endif
	}

	bool IsQuiver() const
	{
		return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_NUM_TYPES;
	}

	// Armor
	bool IsArmor() const
	{
		return GetType() == ITEM_ARMOR;
	}

	bool IsArmorBody() const
	{
		return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_BODY;
	}

	bool IsHelmet() const
	{
		return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_HEAD;
	}

	bool IsShield() const
	{
		return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_SHIELD;
	}

	bool IsWrist() const
	{
		return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_WRIST;
	}

	bool IsShoe() const
	{
		return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_FOOTS;
	}

	bool IsNecklace() const
	{
		return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_NECK;
	}

	bool IsEarRing() const
	{
		return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_EAR;
	}


	bool IsRing() const
	{
		return GetType() == ITEM_RING;
	}

	bool IsCostume() const
	{
		return GetType() == ITEM_COSTUME;
	}

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	bool IsCostumeMount() const
	{
		return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_MOUNT;
	}
#endif

	bool IsCostumeHair() const
	{
		return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_HAIR;
	}

	bool IsCostumeBody() const
	{
		return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_BODY;
	}

   bool IsCostumeAcce() const
	{
#ifdef ENABLE_ACCE_SYSTEM
		return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_ACCE;
#else
		return false;
#endif
	}

	bool IsCostumeAura() const
	{
#ifdef ENABLE_AURA_SYSTEM
		return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_AURA;
#else
		return false;
#endif
	}

	bool IsCostumeWeapon() const
	{
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
		return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_WEAPON;
#else
		return false;
#endif
	}

	bool IsCostumeModifyItem() const
	{
		return GetType() == ITEM_USE && (GetSubType() == USE_CHANGE_COSTUME_ATTR || GetSubType() == USE_RESET_COSTUME_ATTR);
	}

	bool IsBelt() const
	{
		return GetType() == ITEM_BELT;
	}

	int32_t GetApplyValue(uint32_t i) const
	{
		return aApplies[i].lValue;
	}

	int32_t GetApplyType(uint32_t i) const
	{
		return aApplies[i].bType;
	}

	int32_t FindApplyValue(uint32_t applyType) const
	{
		for (int32_t i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
		{
			if (aApplies[i].bType == applyType)
				return aApplies[i].lValue;
		}

		return 0;
	}
	uint32_t		dwVnumRange;
	char        szName[ITEM_NAME_MAX_LEN + 1];
	char	szLocaleName[ITEM_NAME_MAX_LEN + 1];
	uint8_t	bType;
	uint8_t	bSubType;

	uint8_t        bWeight;
	uint8_t	bSize;

	uint32_t	dwAntiFlags;
	uint32_t	dwFlags;
	uint32_t	dwWearFlags;
	uint32_t	dwImmuneFlag;

	uint32_t       dwGold;
	uint32_t       dwShopBuyPrice;

	TItemLimit	aLimits[ITEM_LIMIT_MAX_NUM];
	TItemApply	aApplies[ITEM_APPLY_MAX_NUM];
	int32_t        alValues[ITEM_VALUES_MAX_NUM];
	int32_t	alSockets[ITEM_SOCKET_MAX_NUM];
	uint32_t	dwRefinedVnum;
	uint16_t	wRefineSet;
	uint8_t	bAlterToMagicItemPct;
	uint8_t	bSpecular;
	uint8_t	bGainSocketPct;

	int16_t	sAddonType; // 기본 속성

	// 아래 limit flag들은 realtime에 체크 할 일이 많고, 아이템 VNUM당 고정된 값인데,
	// 현재 구조대로 매번 아이템마다 필요한 경우에 LIMIT_MAX_NUM까지 루프돌면서 체크하는 부하가 커서 미리 저장 해 둠.
	char		cLimitRealTimeFirstUseIndex;		// 아이템 limit 필드값 중에서 LIMIT_REAL_TIME_FIRST_USE 플래그의 위치 (없으면 -1)
	char		cLimitTimerBasedOnWearIndex;		// 아이템 limit 필드값 중에서 LIMIT_TIMER_BASED_ON_WEAR 플래그의 위치 (없으면 -1) 

} TItemTable;

struct TItemAttrTable
{
	TItemAttrTable() :
		dwApplyIndex(0),
		dwProb(0)
	{
		szApply[0] = 0;
		memset(&lValues, 0, sizeof(lValues));
		memset(&bMaxLevelBySet, 0, sizeof(bMaxLevelBySet));
	}

	char    szApply[APPLY_NAME_MAX_LEN + 1];
	uint32_t   dwApplyIndex;
	uint32_t   dwProb;
	int32_t    lValues[ITEM_ATTRIBUTE_MAX_LEVEL];
	uint8_t    bMaxLevelBySet[ATTRIBUTE_SET_MAX_NUM];
};

typedef struct SConnectTable
{
	char	login[LOGIN_MAX_LEN + 1];
	uint32_t	ident;
} TConnectTable;

typedef struct SLoginPacket
{
	char	login[LOGIN_MAX_LEN + 1];
	char	passwd[PASSWD_MAX_LEN + 1];
} TLoginPacket;

typedef struct SPlayerLoadPacket
{
	uint32_t	account_id;
	uint32_t	player_id;
	uint8_t	account_index;	/* account 에서의 위치 */
} TPlayerLoadPacket;

typedef struct SPlayerCreatePacket
{
	char		login[LOGIN_MAX_LEN + 1];
	char		passwd[PASSWD_MAX_LEN + 1];
	uint32_t		account_id;
	uint8_t		account_index;
	TPlayerTable	player_table;
} TPlayerCreatePacket;

typedef struct SPlayerDeletePacket
{
	char	login[LOGIN_MAX_LEN + 1];
	uint32_t	player_id;
	uint8_t	account_index;
	//char	name[CHARACTER_NAME_MAX_LEN + 1];
	char	private_code[8];
} TPlayerDeletePacket;

typedef struct SLogoutPacket
{
	char	login[LOGIN_MAX_LEN + 1];
	char	passwd[PASSWD_MAX_LEN + 1];
} TLogoutPacket;

typedef struct SPlayerCountPacket
{
	uint32_t	dwCount;
} TPlayerCountPacket;

#define SAFEBOX_MAX_NUM			135
#define SAFEBOX_PASSWORD_MAX_LEN	6

typedef struct SSafeboxTable
{
	uint32_t	dwID;
	uint8_t	bSize;
	uint32_t	dwGold;
	uint16_t	wItemCount;
} TSafeboxTable;

typedef struct SSafeboxChangeSizePacket
{
	uint32_t	dwID;
	uint8_t	bSize;
} TSafeboxChangeSizePacket;

typedef struct SSafeboxLoadPacket
{
	uint32_t	dwID;
	char	szLogin[LOGIN_MAX_LEN + 1];
	char	szPassword[SAFEBOX_PASSWORD_MAX_LEN + 1];
} TSafeboxLoadPacket;

typedef struct SSafeboxChangePasswordPacket
{
	uint32_t	dwID;
	char	szOldPassword[SAFEBOX_PASSWORD_MAX_LEN + 1];
	char	szNewPassword[SAFEBOX_PASSWORD_MAX_LEN + 1];
} TSafeboxChangePasswordPacket;

typedef struct SSafeboxChangePasswordPacketAnswer
{
	uint8_t	flag;
} TSafeboxChangePasswordPacketAnswer;

typedef struct SEmpireSelectPacket
{
	uint32_t	dwAccountID;
	uint8_t	bEmpire;
} TEmpireSelectPacket;

typedef struct SPacketGDSetup
{
	char	szPublicIP[16];	// Public IP which listen to users
	uint8_t	bChannel;	// 채널
	uint16_t	wListenPort;	// 클라이언트가 접속하는 포트 번호
	uint16_t	wP2PPort;	// 서버끼리 연결 시키는 P2P 포트 번호
	int32_t	alMaps[MAP_ALLOW_LIMIT];
	uint32_t	dwLoginCount;
	uint8_t	bAuthServer;
} TPacketGDSetup;

typedef struct SPacketDGMapLocations
{
	uint8_t	bCount;
} TPacketDGMapLocations;

typedef struct SMapLocation
{
	int32_t	alMaps[MAP_ALLOW_LIMIT];
	char	szHost[MAX_HOST_LENGTH + 1];
	uint16_t	wPort;
} TMapLocation;

typedef struct SPacketDGP2P
{
	char	szHost[MAX_HOST_LENGTH + 1];
	uint16_t	wPort;
	uint16_t	wListenPort;
	uint8_t	bChannel;
} TPacketDGP2P;

typedef struct SPacketGDDirectEnter
{
	char	login[LOGIN_MAX_LEN + 1];
	char	passwd[PASSWD_MAX_LEN + 1];
	uint8_t	index;
} TPacketGDDirectEnter;

typedef struct SPacketDGDirectEnter
{
	TAccountTable accountTable;
	TPlayerTable playerTable;
} TPacketDGDirectEnter;

typedef struct SPacketGuildSkillUpdate
{
	uint32_t guild_id;
	int32_t amount;
	uint8_t skill_levels[12];
	uint8_t skill_point;
	uint8_t save;
} TPacketGuildSkillUpdate;

typedef struct SPacketGuildExpUpdate
{
	uint32_t guild_id;
	int32_t amount;
} TPacketGuildExpUpdate;

typedef struct SPacketGuildChangeMemberData
{
	uint32_t guild_id;
	uint32_t pid;
	uint32_t offer;
	uint8_t level;
	uint8_t grade;
} TPacketGuildChangeMemberData;

typedef struct packet_dungeon_dest_position
{
	int32_t x;
	int32_t y;
} TPacketDungeonDestPosition;

typedef struct SPacketDGLoginAlready
{
	char	szLogin[LOGIN_MAX_LEN + 1];
} TPacketDGLoginAlready;


typedef struct SPacketGDAddAffect
{
	uint32_t			dwPID;
	TPacketAffectElement	elem;
} TPacketGDAddAffect;

typedef struct SPacketGDRemoveAffect
{
	uint32_t	dwPID;
	uint32_t	dwType;
	uint8_t	bApplyOn;
} TPacketGDRemoveAffect;

typedef struct SPacketPartyCreate
{
	uint32_t	dwLeaderPID;
} TPacketPartyCreate;

typedef struct SPacketPartyDelete
{
	uint32_t	dwLeaderPID;
} TPacketPartyDelete;

typedef struct SPacketPartyAdd
{
	uint32_t	dwLeaderPID;
	uint32_t	dwPID;
	uint8_t	bState;
} TPacketPartyAdd;

typedef struct SPacketPartyRemove
{
	uint32_t	dwLeaderPID;
	uint32_t	dwPID;
} TPacketPartyRemove;

typedef struct SPacketPartyStateChange
{
	uint32_t	dwLeaderPID;
	uint32_t	dwPID;
	uint8_t	bRole;
	uint8_t	bFlag;
} TPacketPartyStateChange;

typedef struct SPacketPartySetMemberLevel
{
	uint32_t	dwLeaderPID;
	uint32_t	dwPID;
	uint8_t	bLevel;
} TPacketPartySetMemberLevel;

typedef struct SPacketGDBoot
{
    uint32_t	dwItemIDRange[2];
	char	szIP[16];
} TPacketGDBoot;

typedef struct SPacketGuild
{
	uint32_t	dwGuild;
	uint32_t	dwInfo;
} TPacketGuild;

typedef struct SPacketGDGuildAddMember
{
	uint32_t	dwPID;
	uint32_t	dwGuild;
	uint8_t	bGrade;
} TPacketGDGuildAddMember;

typedef struct SPacketDGGuildMember
{
	uint32_t	dwPID;
	uint32_t	dwGuild;
	uint8_t	bGrade;
	uint8_t	isGeneral;
	uint8_t	bJob;
	uint8_t	bLevel;
	uint32_t	dwOffer;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketDGGuildMember;

typedef struct SPacketGuildWar
{
	uint8_t	bType;
	uint8_t	bWar;
	uint32_t	dwGuildFrom;
	uint32_t	dwGuildTo;
	int32_t	lWarPrice;
	int32_t	lInitialScore;
} TPacketGuildWar;

// Game -> DB : 상대적 변화값
// DB -> Game : 토탈된 최종값
typedef struct SPacketGuildWarScore
{
	uint32_t dwGuildGainPoint;
	uint32_t dwGuildOpponent;
	int32_t lScore;
	int32_t lBetScore;
} TPacketGuildWarScore;

typedef struct SRefineMaterial
{
	uint32_t vnum;
	int32_t count;
} TRefineMaterial;

typedef struct SBanwordTable
{
	char szWord[BANWORD_MAX_LEN + 1];
} TBanwordTable;

typedef struct SPacketGDChangeName
{
	uint32_t pid;
	char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGDChangeName;

typedef struct SPacketDGChangeName
{
	uint32_t pid;
	char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketDGChangeName;

typedef struct SPacketGuildLadder
{
	uint32_t dwGuild;
	int32_t lLadderPoint;
	int32_t lWin;
	int32_t lDraw;
	int32_t lLoss;
} TPacketGuildLadder;

typedef struct SPacketGuildLadderPoint
{
	uint32_t dwGuild;
	int32_t lChange;
} TPacketGuildLadderPoint;

typedef struct SPacketGuildUseSkill
{
	uint32_t dwGuild;
	uint32_t dwSkillVnum;
	uint32_t dwCooltime;
} TPacketGuildUseSkill;

typedef struct SPacketGuildSkillUsableChange
{
	uint32_t dwGuild;
	uint32_t dwSkillVnum;
	uint8_t bUsable;
} TPacketGuildSkillUsableChange;

typedef struct SPacketGDLoginKey
{
	uint32_t dwAccountID;
	uint32_t dwLoginKey;
} TPacketGDLoginKey;

typedef struct SPacketGDAuthLogin
{
	uint32_t	dwID;
	uint32_t	dwLoginKey;
	char	szLogin[LOGIN_MAX_LEN + 1];
	char	szSocialID[SOCIAL_ID_MAX_LEN + 1];
	uint32_t	adwClientKey[4];
	int32_t		iPremiumTimes[PREMIUM_MAX_NUM];
} TPacketGDAuthLogin;

typedef struct SPacketGDLoginByKey
{
	char	szLogin[LOGIN_MAX_LEN + 1];
	uint32_t	dwLoginKey;
	uint32_t	adwClientKey[4];
	char	szIP[MAX_HOST_LENGTH + 1];
} TPacketGDLoginByKey;

/**
 * @version 05/06/08	Bang2ni - 지속시간 추가
 */
typedef struct SPacketGiveGuildPriv
{
	uint8_t type;
	int32_t value;
	uint32_t guild_id;
	uint32_t duration_sec;	///< 지속시간
} TPacketGiveGuildPriv;
typedef struct SPacketGiveEmpirePriv
{
	uint8_t type;
	int32_t value;
	uint8_t empire;
	uint32_t duration_sec;
} TPacketGiveEmpirePriv;
typedef struct SPacketGiveCharacterPriv
{
	uint8_t type;
	int32_t value;
	uint32_t pid;
} TPacketGiveCharacterPriv;
typedef struct SPacketRemoveGuildPriv
{
	uint8_t type;
	uint32_t guild_id;
} TPacketRemoveGuildPriv;
typedef struct SPacketRemoveEmpirePriv
{
	uint8_t type;
	uint8_t empire;
} TPacketRemoveEmpirePriv;

typedef struct SPacketDGChangeCharacterPriv
{
	uint8_t type;
	int32_t value;
	uint32_t pid;
	uint8_t bLog;
} TPacketDGChangeCharacterPriv;

typedef struct SActivityTable
{
	uint32_t pid;
	struct
	{
		int32_t pvp;
		int32_t gk;
		int32_t pve;
		int32_t other;
	} today;
	int32_t		total;
	uint32_t	lastUpdate;
} TActivityTable;

/**
 * @version 05/06/08	Bang2ni - 지속시간 추가
 */
typedef struct SPacketDGChangeGuildPriv
{
	uint8_t type;
	int32_t value;
	uint32_t guild_id;
	uint8_t bLog;
	uint32_t end_time_sec;	///< 지속시간
} TPacketDGChangeGuildPriv;

typedef struct SPacketDGChangeEmpirePriv
{
	uint8_t type;
	int32_t value;
	uint8_t empire;
	uint8_t bLog;
	uint32_t end_time_sec;
} TPacketDGChangeEmpirePriv;

typedef struct SPacketGDGuildMoney
{
	uint32_t dwGuild;
	int32_t iGold;
} TPacketGDGuildMoney;

typedef struct SPacketDGGuildMoneyChange
{
	uint32_t dwGuild;
	int32_t iTotalGold;
} TPacketDGGuildMoneyChange;

typedef struct SPacketDGGuildMoneyWithdraw
{
	uint32_t dwGuild;
	int32_t iChangeGold;
} TPacketDGGuildMoneyWithdraw;

typedef struct SPacketGDGuildMoneyWithdrawGiveReply
{
	uint32_t dwGuild;
	int32_t iChangeGold;
	uint8_t bGiveSuccess;
} TPacketGDGuildMoneyWithdrawGiveReply;

typedef struct SPacketSetEventFlag
{
	char	szFlagName[EVENT_FLAG_NAME_MAX_LEN + 1];
	int32_t	lValue;
} TPacketSetEventFlag;

typedef struct SPacketLoginOnSetup
{
	uint32_t   dwID;
	char    szLogin[LOGIN_MAX_LEN + 1];
	char    szSocialID[SOCIAL_ID_MAX_LEN + 1];
	char    szHost[MAX_HOST_LENGTH + 1];
	uint32_t   dwLoginKey;
	uint32_t   adwClientKey[4];
} TPacketLoginOnSetup;

typedef struct SPacketGDCreateObject
{
	uint32_t	dwVnum;
	uint32_t	dwLandID;
	int32_t		lMapIndex;
	int32_t	 	x, y;
	float	xRot;
	float	yRot;
	float	zRot;
} TPacketGDCreateObject;

typedef struct SGuildReserve
{
	uint32_t       dwID;
	uint32_t       dwGuildFrom;
	uint32_t       dwGuildTo;
	uint32_t       dwTime;
	uint8_t        bType;
	int32_t        lWarPrice;
	int32_t        lInitialScore;
	bool        bStarted;
	uint32_t	dwBetFrom;
	uint32_t	dwBetTo;
	int32_t	lPowerFrom;
	int32_t	lPowerTo;
	int32_t	lHandicap;
} TGuildWarReserve;

typedef struct
{
	uint32_t	dwWarID;
	char	szLogin[LOGIN_MAX_LEN + 1];
	uint32_t	dwGold;
	uint32_t	dwGuild;
} TPacketGDGuildWarBet;

// Marriage

typedef struct
{
	uint32_t dwPID1;
	uint32_t dwPID2;
	uint32_t tMarryTime;
	char szName1[CHARACTER_NAME_MAX_LEN + 1];
	char szName2[CHARACTER_NAME_MAX_LEN + 1];
} TPacketMarriageAdd;

typedef struct
{
	uint32_t dwPID1;
	uint32_t dwPID2;
	int32_t  iLovePoint;
	uint8_t  byMarried;
} TPacketMarriageUpdate;

typedef struct
{
	uint32_t dwPID1;
	uint32_t dwPID2;
} TPacketMarriageRemove;

typedef struct
{
	uint32_t dwPID1;
	uint32_t dwPID2;
} TPacketWeddingRequest;

typedef struct
{
	uint32_t dwPID1;
	uint32_t dwPID2;
	uint32_t dwMapIndex;
} TPacketWeddingReady;

typedef struct
{
	uint32_t dwPID1;
	uint32_t dwPID2;
} TPacketWeddingStart;

typedef struct
{
	uint32_t dwPID1;
	uint32_t dwPID2;
} TPacketWeddingEnd;

/// 개인상점 가격정보의 헤더. 가변 패킷으로 이 뒤에 byCount 만큼의 TItemPriceInfo 가 온다.
typedef struct SPacketMyshopPricelistHeader
{ 
	uint32_t	dwOwnerID;	///< 가격정보를 가진 플레이어 ID 
	uint8_t	byCount;	///< 가격정보 갯수
} TPacketMyshopPricelistHeader;

/// 개인상점의 단일 아이템에 대한 가격정보
typedef struct SItemPriceInfo
{
	uint32_t	dwVnum;		///< 아이템 vnum
	uint32_t	dwPrice;	///< 가격
} TItemPriceInfo;

/// 개인상점 아이템 가격정보 리스트 테이블
typedef struct SItemPriceListTable
{
	uint32_t	dwOwnerID;	///< 가격정보를 가진 플레이어 ID
	uint8_t	byCount;	///< 가격정보 리스트의 갯수

	TItemPriceInfo	aPriceInfo[SHOP_PRICELIST_MAX_NUM];	///< 가격정보 리스트
} TItemPriceListTable;

typedef struct
{
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	int32_t lDuration;
} TPacketBlockChat;


//ADMIN_MANAGER
typedef struct TAdminInfo
{
	int32_t m_ID;				//고유ID
	char m_szAccount[32];	//계정
	char m_szName[32];		//캐릭터이름
	int32_t m_Authority;		//권한
} tAdminInfo;

typedef struct SPacketGGUpdateRights
{
	uint8_t	header;
	char	name[CHARACTER_NAME_MAX_LEN + 1];
	uint8_t	gm_level;
} TPacketGGUpdateRights;
//END_ADMIN_MANAGER

//BOOT_LOCALIZATION
struct tLocale
{
	char szValue[32];
	char szKey[32];
};
//BOOT_LOCALIZATION

//RELOAD_ADMIN
typedef struct SPacketReloadAdmin
{
	char szIP[16];
} TPacketReloadAdmin;
//END_RELOAD_ADMIN

typedef struct tChangeGuildMaster
{
	uint32_t dwGuildID;
	uint32_t idFrom;
	uint32_t idTo;
} TPacketChangeGuildMaster;

typedef struct tItemIDRange
{
	uint32_t dwMin;
	uint32_t dwMax;
	uint32_t dwUsableItemIDMin;
} TItemIDRangeTable;

typedef struct tDC
{
	char	login[LOGIN_MAX_LEN + 1];
} TPacketDC;

typedef struct tNeedLoginLogInfo
{
	uint32_t dwPlayerID;
} TPacketNeedLoginLogInfo;

//독일 선물 알림 기능 테스트용 패킷 정보
typedef struct tItemAwardInformer
{
	char	login[LOGIN_MAX_LEN + 1];
	char	command[20];		//명령어
	uint32_t vnum;			//아이템
} TPacketItemAwardInfromer;
// 선물 알림 기능 삭제용 패킷 정보
typedef struct tDeleteAwardID
{
	uint32_t dwID;
} TPacketDeleteAwardID;

typedef struct TPlayerCount
{
	uint16_t nPort;
	uint32_t dwUserCount;
} TPlayerCount;

#pragma pack()
#endif
