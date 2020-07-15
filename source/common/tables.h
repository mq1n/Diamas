#pragma once
#include <cstdint>
#include "defines.h"
#include "constants.h"
#include "item_data.h"

#pragma pack(push)
#pragma pack(1)

typedef struct SQuickSlot
{
    uint8_t Type;
    uint8_t Position;
} TQuickSlot;

typedef struct TPlayerItemAttribute
{
    uint8_t        bType;
    int16_t       sValue;
} TPlayerItemAttribute;

typedef struct SPlayerItem
{
    uint32_t	id;
    uint8_t	window;
    uint16_t	pos;
    uint32_t	count;

    uint32_t	vnum;
    int32_t	alSockets[ITEM_SOCKET_MAX_NUM];	// ¼ÒÄÏ¹øÈ£

    TPlayerItemAttribute    aAttr[ITEM_ATTRIBUTE_MAX_NUM];

    uint32_t	owner;
    bool	is_gm_owner;
} TPlayerItem;

typedef struct packet_item
{
    uint32_t       vnum;
    uint8_t        count;
    uint32_t		flags;
    uint32_t		anti_flags;
    int32_t		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
} TItemData;

typedef struct packet_shop_item
{
    uint32_t       vnum;
    uint32_t       price;
    uint8_t        count;
    uint8_t		display_pos;
    int32_t		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
} TShopItemData;

struct	THorseInfo
{
    uint8_t	bLevel;
    char	szName[CHARACTER_NAME_MAX_LEN + 1];
    bool	bRiding;
    int16_t	sStamina;
    int16_t	sHealth;
    uint32_t	dwHorseHealthDropTime;
};

typedef struct SPlayerSkill
{
	uint8_t bMasterType;
	uint8_t bLevel;
	uint32_t tNextRead;
} TPlayerSkill;

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

typedef struct SChestDropInfoTable
{
    uint8_t	bPageIndex;
    uint8_t	bSlotIndex;
    uint32_t	dwItemVnum;
    uint8_t	bItemCount;
} TChestDropInfoTable;

typedef struct
{
    uint32_t       dwID;
    int32_t        x, y;
    int32_t        width, height;
    uint32_t       dwGuildID;
} TLandPacketElement;

typedef struct TPacketAffectElement
{
    uint32_t	dwType;
    uint8_t	bApplyOn;
    int32_t	lApplyValue;
    uint32_t	dwFlag;
    int32_t	lDuration;
    int32_t	lSPCost;
} TPacketAffectElement;

struct TNPCPosition
{
    uint8_t bType;
    char name[CHARACTER_NAME_MAX_LEN + 1];
    int32_t x;
    int32_t y;
};

struct TMaterial
{
    uint32_t vnum;
    uint32_t count;
};

typedef struct SRefineTable
{
    uint32_t id;
    uint32_t src_vnum;
    uint32_t result_vnum;
    uint8_t material_count;
    int32_t cost; // ¼Ò¿ä ºñ¿ë
    int32_t prob; // È®·ü
    TMaterial materials[REFINE_MATERIAL_MAX_NUM];
} TRefineTable;

typedef struct SEquipmentItemSet
{
    uint32_t   vnum;
    uint8_t    count;
    int32_t    alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
} TEquipmentItemSet;

typedef struct SShopItemTable
{
    uint32_t		vnum;
    uint8_t		count;

    TItemPos	pos;			// PC »óÁ¡¿¡¸¸ ÀÌ¿ë
    uint32_t		price;			// PC »óÁ¡¿¡¸¸ ÀÌ¿ë
    uint8_t		display_pos;	//	PC »óÁ¡¿¡¸¸ ÀÌ¿ë, º¸ÀÏ À§Ä¡.
} TShopItemTable;

typedef struct SShopTable
{
    uint32_t		dwVnum;
    uint32_t		dwNPCVnum;

    uint8_t		byItemCount;
    TShopItemTable	items[SHOP_HOST_ITEM_MAX_NUM];
} TShopTable;

typedef struct SRequestChargeCash
{
    uint32_t		dwAID;		// id(primary key) - Account Table
    uint32_t		dwAmount;
    ERequestChargeType	eChargeType;
} TRequestChargeCash;

typedef struct SSimplePlayerInformation
{
    uint32_t               dwID;
    char                szName[CHARACTER_NAME_MAX_LEN + 1];
    uint8_t                byJob;
    uint8_t                byLevel;
    uint32_t               dwPlayMinutes;
    uint8_t                byST, byHT, byDX, byIQ;
    //	uint16_t				wParts[CRaceData::PART_MAX_NUM];
    uint16_t                wMainPart;
    uint8_t                bChangeName;
    uint16_t				wHairPart;
#ifdef ENABLE_ACCE_SYSTEM
    uint32_t				dwAccePart;
#endif
    int32_t				x, y;
    int32_t				lAddr;
    uint16_t				wPort;
    uint8_t				bySkillGroup;
} TSimplePlayerInformation;

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
	uint8_t	account_index;	/* account ¿¡¼­ÀÇ À§Ä¡ */
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

typedef struct SPacketDGCreateSuccess
{
	uint8_t		bAccountCharacterIndex;
	TSimplePlayerInformation	player;
} TPacketDGCreateSuccess;

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
	uint8_t	bChannel;	// Ã¤³Î
	uint16_t	wListenPort;	// Å¬¶óÀÌ¾ðÆ®°¡ Á¢¼ÓÇÏ´Â Æ÷Æ® ¹øÈ£
	uint16_t	wP2PPort;	// ¼­¹ö³¢¸® ¿¬°á ½ÃÅ°´Â P2P Æ÷Æ® ¹øÈ£
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

// Game -> DB : »ó´ëÀû º¯È­°ª
// DB -> Game : ÅäÅ»µÈ ÃÖÁ¾°ª
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
	char hwid[HWID_MAX_HASH_LEN + 1];
	char lang[LANG_MAX_LEN + 1];
} TPacketGDAuthLogin;

typedef struct SPacketGDLoginByKey
{
	char	szLogin[LOGIN_MAX_LEN + 1];
	uint32_t	dwLoginKey;
	uint32_t	adwClientKey[4];
	char	szIP[MAX_HOST_LENGTH + 1];
} TPacketGDLoginByKey;

/**
 * @version 05/06/08	Bang2ni - Áö¼Ó½Ã°£ Ãß°¡
 */
typedef struct SPacketGiveGuildPriv
{
	uint8_t type;
	int32_t value;
	uint32_t guild_id;
	uint32_t duration_sec;	///< Áö¼Ó½Ã°£
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
 * @version 05/06/08	Bang2ni - Áö¼Ó½Ã°£ Ãß°¡
 */
typedef struct SPacketDGChangeGuildPriv
{
	uint8_t type;
	int32_t value;
	uint32_t guild_id;
	uint8_t bLog;
	uint32_t end_time_sec;	///< Áö¼Ó½Ã°£
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

/// °³ÀÎ»óÁ¡ °¡°ÝÁ¤º¸ÀÇ Çì´õ. °¡º¯ ÆÐÅ¶À¸·Î ÀÌ µÚ¿¡ byCount ¸¸Å­ÀÇ TItemPriceInfo °¡ ¿Â´Ù.
typedef struct SPacketMyshopPricelistHeader
{
	uint32_t	dwOwnerID;	///< °¡°ÝÁ¤º¸¸¦ °¡Áø ÇÃ·¹ÀÌ¾î ID 
	uint8_t	byCount;	///< °¡°ÝÁ¤º¸ °¹¼ö
} TPacketMyshopPricelistHeader;

/// °³ÀÎ»óÁ¡ÀÇ ´ÜÀÏ ¾ÆÀÌÅÛ¿¡ ´ëÇÑ °¡°ÝÁ¤º¸
typedef struct SItemPriceInfo
{
	uint32_t	dwVnum;		///< ¾ÆÀÌÅÛ vnum
	uint32_t	dwPrice;	///< °¡°Ý
} TItemPriceInfo;

/// °³ÀÎ»óÁ¡ ¾ÆÀÌÅÛ °¡°ÝÁ¤º¸ ¸®½ºÆ® Å×ÀÌºí
typedef struct SItemPriceListTable
{
	uint32_t	dwOwnerID;	///< °¡°ÝÁ¤º¸¸¦ °¡Áø ÇÃ·¹ÀÌ¾î ID
	uint8_t	byCount;	///< °¡°ÝÁ¤º¸ ¸®½ºÆ®ÀÇ °¹¼ö

	TItemPriceInfo	aPriceInfo[SHOP_PRICELIST_MAX_NUM];	///< °¡°ÝÁ¤º¸ ¸®½ºÆ®
} TItemPriceListTable;

typedef struct
{
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	int32_t lDuration;
} TPacketBlockChat;


//ADMIN_MANAGER
typedef struct TAdminInfo
{
	int32_t m_ID;				//°íÀ¯ID
	char m_szAccount[32];	//°èÁ¤
	char m_szName[32];		//Ä³¸¯ÅÍÀÌ¸§
	int32_t m_Authority;		//±ÇÇÑ
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

//µ¶ÀÏ ¼±¹° ¾Ë¸² ±â´É Å×½ºÆ®¿ë ÆÐÅ¶ Á¤º¸
typedef struct tItemAwardInformer
{
	char	login[LOGIN_MAX_LEN + 1];
	char	command[20];		//¸í·É¾î
	uint32_t vnum;			//¾ÆÀÌÅÛ
} TPacketItemAwardInfromer;
// ¼±¹° ¾Ë¸² ±â´É »èÁ¦¿ë ÆÐÅ¶ Á¤º¸
typedef struct tDeleteAwardID
{
	uint32_t dwID;
} TPacketDeleteAwardID;

typedef struct TPlayerCount
{
	uint16_t nPort;
	uint32_t dwUserCount;
} TPlayerCount;

typedef struct sub_packet_shop_tab
{
    char name[SHOP_TAB_NAME_MAX];
    uint8_t coin_type;
    packet_shop_item items[SHOP_HOST_ITEM_MAX_NUM];
} TSubPacketShopTab;

typedef struct SPacketAcce
{
    uint8_t	header;
    uint8_t	subheader;
    bool	bWindow;
    uint32_t	dwPrice;
    uint8_t	bPos;
    TItemPos	tPos;
    uint32_t	dwItemVnum;
    uint32_t	dwMinAbs;
    uint32_t	dwMaxAbs;
} TPacketAcce;

typedef struct SAcceMaterial
{
    uint8_t	bHere;
    uint16_t	wCell;
} TAcceMaterial;

typedef struct SAcceResult
{
    uint32_t	dwItemVnum;
    uint32_t	dwMinAbs;
    uint32_t	dwMaxAbs;
} TAcceResult;

typedef struct SChannelStatus
{
    uint16_t nPort;
    uint8_t bStatus;
} TChannelStatus;

#pragma pack(pop)
