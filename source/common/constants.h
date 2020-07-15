#pragma once
#include <cstdint>
#include "defines.h"

#define WORD_MAX 0xffff
#define GOLD_MAX 2000000000

enum EGameConstants
{
	MAX_HOST_LENGTH = 15,
	IP_ADDRESS_LENGTH = 15,

	LOGIN_MAX_LEN = 30,
	PASSWD_MAX_LEN = 16,
	HWID_MAX_HASH_LEN = 120,
	LANG_MAX_LEN = 3,
	CHAT_MAX_NUM = 128,
	PATH_NODE_MAX_NUM = 64,
	SHOP_SIGN_MAX_LEN = 32,

	MUSIC_NAME_MAX_LEN = 24,

	POINT_MAX_NUM = 255,
	CHARACTER_NAME_MAX_LEN = 24,
	PLAYER_NAME_MAX_LEN = 12,

#ifdef ENABLE_PLAYER_PER_ACCOUNT5
	PLAYER_PER_ACCOUNT = 5,
#else
	PLAYER_PER_ACCOUNT = 4,
#endif

	PLAYER_ITEM_SLOT_MAX_NUM = 20,		// ÇÃ·¡ÀÌ¾îÀÇ ½½·Ô´ç µé¾î°¡´Â °¹¼ö.

	QUICKSLOT_MAX_LINE = 4,
	QUICKSLOT_MAX_COUNT_PER_LINE = 8, // Å¬¶óÀÌ¾ðÆ® ÀÓÀÇ °áÞ¤°ª
	QUICKSLOT_MAX_COUNT = QUICKSLOT_MAX_LINE * QUICKSLOT_MAX_COUNT_PER_LINE,

	QUICKSLOT_MAX_NUM = 36, // ¼­¹ö¿Þ ¸ÂÃçÞ® ÀÖ´Â °ª

	METIN_SOCKET_COUNT = 6,

	PARTY_AFFECT_SLOT_MAX_NUM = 7,

	MARK_CRC_NUM = 8 * 8,
	MARK_DATA_SIZE = 16 * 12,
	SYMBOL_DATA_SIZE = 128 * 256,
	QUEST_INPUT_STRING_MAX_NUM = 64,

	PRIVATE_CODE_LENGTH = 8,

	REFINE_MATERIAL_MAX_NUM = 5,

	SHOP_TAB_NAME_MAX = 32,
	SHOP_TAB_COUNT_MAX = 3,

	LOGIN_STATUS_MAX_LEN = 8,

	MAX_EFFECT_FILE_NAME = 128,

	ACCOUNT_STATUS_MAX_LEN = 8,
	INVENTORY_PAGE_COLUMN = 5, // 5 (default)
	INVENTORY_PAGE_ROW = 9, // 9 (default)
	INVENTORY_PAGE_SIZE = INVENTORY_PAGE_COLUMN * INVENTORY_PAGE_ROW,
#ifdef ENABLE_EXTEND_INVEN_SYSTEM
	INVENTORY_PAGE_COUNT = 4, // 2 (default)
#else
	INVENTORY_PAGE_COUNT = 2, // 2 (default)
#endif
	INVENTORY_MAX_NUM = INVENTORY_PAGE_SIZE * INVENTORY_PAGE_COUNT, // 90 (default)
	ABILITY_MAX_NUM = 50,
	EMPIRE_MAX_NUM = 4,
	BANWORD_MAX_LEN = 24,
	SOCIAL_ID_MAX_LEN = 18,

	SHOP_HOST_ITEM_MAX_NUM = 40,	/* È£½ºÆ®ÀÇ ÃÖ´ë ¾ÆÀÌÅÛ °³¼ö */
	SHOP_GUEST_ITEM_MAX_NUM = 18,	/* °Ô½ºÆ®ÀÇ ÃÖ´ë ¾ÆÀÌÅÛ °³¼ö */

	SHOP_PRICELIST_MAX_NUM = 40,	///< °³ÀÞ»óÞ¡ °¡°ÞÞ¤º¸ ¸®½ºÆ®¿¡¼­ À¯ÞöÇÒ °¡°ÞÞ¤º¸ÀÇ ÃÖ´ë °¹¼ö

	CHAT_MAX_LEN = 512,

	JOURNAL_MAX_NUM = 2,

	//	QUERY_MAX_LEN			= 8192,

	FILE_MAX_LEN = 128,

	PLAYER_EXP_TABLE_MAX = 120,
	PLAYER_MAX_LEVEL_CONST = 120,

	GUILD_MAX_LEVEL = 20,
	MOB_MAX_LEVEL = 100,

	ATTRIBUTE_MAX_VALUE = 20,
	CHARACTER_PATH_MAX_NUM = 64,
	SKILL_MAX_NUM = 255,
	SKILLBOOK_DELAY_MIN = 64800,
	SKILLBOOK_DELAY_MAX = 108000,
	SKILL_MAX_LEVEL = 40,

	APPLY_NAME_MAX_LEN = 32,
	EVENT_FLAG_NAME_MAX_LEN = 32,

	MOB_SKILL_MAX_NUM = 5,

	DRAGON_SOUL_BOX_SIZE = 32,
	DRAGON_SOUL_BOX_COLUMN_NUM = 8,
	DRAGON_SOUL_BOX_ROW_NUM = DRAGON_SOUL_BOX_SIZE / DRAGON_SOUL_BOX_COLUMN_NUM,
	DRAGON_SOUL_REFINE_GRID_SIZE = 15,
	MAX_AMOUNT_OF_MALL_BONUS = 20,

	WEAR_MAX_NUM = 32,

	BELT_INVENTORY_SLOT_WIDTH = 4,
	BELT_INVENTORY_SLOT_HEIGHT = 4,

	BELT_INVENTORY_SLOT_COUNT = BELT_INVENTORY_SLOT_WIDTH * BELT_INVENTORY_SLOT_HEIGHT
};

enum EGameStage : uint8_t
{
	STAGE_NULL,
	STAGE_DEV_GAME,
	STAGE_BETA_GAME,
	STAGE_LIVE_GAME,
};

enum EGamePhase : uint8_t
{
	PHASE_CLOSE,
	PHASE_HANDSHAKE,
	PHASE_LOGIN,
	PHASE_SELECT,
	PHASE_LOADING,
	PHASE_GAME,
	PHASE_DEAD,

	PHASE_DBCLIENT_CONNECTING,
	PHASE_DBCLIENT,
	PHASE_P2P,
	PHASE_AUTH,
};

enum EJobs
{
	JOB_WARRIOR,
	JOB_ASSASSIN,
	JOB_SURA,
	JOB_SHAMAN,
#ifdef ENABLE_WOLFMAN_CHARACTER
	JOB_WOLFMAN,
#endif
	JOB_MAX_NUM
};

enum EWindows
{
	RESERVED_WINDOW,
	INVENTORY,				// ±âº» ÀÞº¥Åä¸®. (45Ä­ Â¥¸®°¡ 2ÆäÀÌÞö Þ¸Àç = 90Ä­)
	EQUIPMENT,
	SAFEBOX,
	MALL,
	DRAGON_SOUL_INVENTORY,
	BELT_INVENTORY,			// NOTE: W2.1 ¹öÀü¿¡ »õ·Þ Ãß°¡µÇ´Â º§Æ® ½½·Ô ¾ÆÀÌÅÛÀÌ Þ¦°øÇÞ´Â º§Æ® ÀÞº¥Åä¸®
	GROUND,					// NOTE: 2013³â 2¿ù5ÀÞ ÇöÀç±îÞö unused.. ¿Ö ÀÖ´Â°ÅÞö???

	WINDOW_TYPE_MAX,
};

enum
{
	GUILD_NAME_MAX_LEN = 12,
	GUILD_GRADE_NAME_MAX_LEN = 8,
	GUILD_GRADE_COUNT = 15,
	GUILD_COMMENT_MAX_COUNT = 12,
	GUILD_COMMENT_MAX_LEN = 50,
	GUILD_LEADER_GRADE = 1,
	GUILD_BASE_POWER = 400,
	GUILD_POWER_PER_SKILL_LEVEL = 200,
	GUILD_POWER_PER_LEVEL = 100,
	GUILD_MINIMUM_LEADERSHIP = 40,
	GUILD_WAR_MIN_MEMBER_COUNT = 8,
	GUILD_LADDER_POINT_PER_LEVEL = 1000,
	GUILD_CREATE_ITEM_VNUM = 70101,
};

enum
{
	ITEM_SOCKET_SLOT_MAX_NUM = 3,
	// refactored attribute slot begin
	ITEM_ATTRIBUTE_SLOT_NORM_NUM = 5,
	ITEM_ATTRIBUTE_SLOT_RARE_NUM = 2,

	ITEM_ATTRIBUTE_SLOT_NORM_START = 0,
	ITEM_ATTRIBUTE_SLOT_NORM_END = ITEM_ATTRIBUTE_SLOT_NORM_START + ITEM_ATTRIBUTE_SLOT_NORM_NUM,

	ITEM_ATTRIBUTE_SLOT_RARE_START = ITEM_ATTRIBUTE_SLOT_NORM_END,
	ITEM_ATTRIBUTE_SLOT_RARE_END = ITEM_ATTRIBUTE_SLOT_RARE_START + ITEM_ATTRIBUTE_SLOT_RARE_NUM,

	ITEM_ATTRIBUTE_SLOT_MAX_NUM = ITEM_ATTRIBUTE_SLOT_RARE_END, // 7
	// refactored attribute slot end
};

enum EFunc
{
	FUNC_WAIT,
	FUNC_MOVE,
	FUNC_ATTACK,
	FUNC_COMBO,
	FUNC_MOB_SKILL,
	FUNC_EMOTION,
	FUNC_MAX_NUM,
	FUNC_SKILL = 0x80
};

enum EPartySkillType
{
	PARTY_SKILL_HEAL = 1,
	PARTY_SKILL_WARP = 2
};

enum EDragonSoulRefineWindowRefineType
{
	DragonSoulRefineWindow_UPGRADE,
	DragonSoulRefineWindow_IMPROVEMENT,
	DragonSoulRefineWindow_REFINE,
};

enum EPartyExpDistributionType
{
	PARTY_EXP_DISTRIBUTION_NON_PARITY,
	PARTY_EXP_DISTRIBUTION_PARITY,
	PARTY_EXP_DISTRIBUTION_MAX_NUM
};

enum ERequestChargeType
{
	ERequestCharge_Cash = 0,
	ERequestCharge_Mileage,
};

enum ESafeboxState
{
	SAFEBOX_MONEY_STATE_SAVE,
	SAFEBOX_MONEY_STATE_WITHDRAW,
};

enum EBattleMode
{
	BATTLEMODE_ATTACK,
	BATTLEMODE_DEFENSE
};

enum EMessengerState
{
	MESSENGER_CONNECTED_STATE_OFFLINE,
	MESSENGER_CONNECTED_STATE_ONLINE
};

enum GuildWarStatus
{
	GUILD_WAR_RESULT_OK,
	GUILD_WAR_RESULT_UNDETERMINED,
	GUILD_WAR_RESULT_TIMEOUT = 99,
};

enum ESkillGroups
{
	SKILL_GROUP_MAX_NUM = 2
};

enum ESex
{
	SEX_MALE,
	SEX_FEMALE
};

enum ERaceFlags
{
	RACE_FLAG_ANIMAL = (1 << 0),
	RACE_FLAG_UNDEAD = (1 << 1),
	RACE_FLAG_DEVIL = (1 << 2),
	RACE_FLAG_HUMAN = (1 << 3),
	RACE_FLAG_ORC = (1 << 4),
	RACE_FLAG_MILGYO = (1 << 5),
	RACE_FLAG_INSECT = (1 << 6),
	RACE_FLAG_FIRE = (1 << 7),
	RACE_FLAG_ICE = (1 << 8),
	RACE_FLAG_DESERT = (1 << 9),
	RACE_FLAG_TREE = (1 << 10),
	RACE_FLAG_ATT_ELEC = (1 << 11),
	RACE_FLAG_ATT_FIRE = (1 << 12),
	RACE_FLAG_ATT_ICE = (1 << 13),
	RACE_FLAG_ATT_WIND = (1 << 14),
	RACE_FLAG_ATT_EARTH = (1 << 15),
	RACE_FLAG_ATT_DARK = (1 << 16)
};

enum EPVPModes
{
	PVP_MODE_NONE,
	PVP_MODE_AGREE,
	PVP_MODE_FIGHT,
	PVP_MODE_REVENGE
};

enum ECharAddState
{
	ADD_CHARACTER_STATE_DEAD = (1 << 0),
	ADD_CHARACTER_STATE_SPAWN = (1 << 1),
	ADD_CHARACTER_STATE_GUNGON = (1 << 2),
	ADD_CHARACTER_STATE_KILLER = (1 << 3),
	ADD_CHARACTER_STATE_PARTY = (1 << 4),
};

enum EPKModes
{
	PK_MODE_PEACE,
	PK_MODE_REVENGE,
	PK_MODE_FREE,
	PK_MODE_PROTECT,
	PK_MODE_GUILD,
	PK_MODE_MAX_NUM,
};

enum ECharacterEquipmentPart
{
	CHR_EQUIPPART_ARMOR,
	CHR_EQUIPPART_WEAPON,
	CHR_EQUIPPART_HEAD,
	CHR_EQUIPPART_HAIR,

#ifdef ENABLE_ACCE_SYSTEM
	CHR_EQUIPPART_ACCE,
#endif
	CHR_EQUIPPART_NUM,
};

enum EChatType
{
	CHAT_TYPE_TALKING,  /* ±×³É Ã¤ÆÃ */
	CHAT_TYPE_INFO,     /* Þ¤º¸ (¾ÆÀÌÅÛÀ» Þý¾ú´Ù, °æÇèÄ¡¸¦ ¾ò¾ú´Ù. µî) */
	CHAT_TYPE_NOTICE,   /* °øÞö»çÇ× */
	CHAT_TYPE_PARTY,    /* ÆÄÆ¼¸» */
	CHAT_TYPE_GUILD,    /* ±æµå¸» */
	CHAT_TYPE_COMMAND,	/* ¸í·É */
	CHAT_TYPE_SHOUT,	/* ¿ÜÄ¡±â */
	CHAT_TYPE_WHISPER,	// ¼­¹ö¿Þ´Â ¿¬µ¿µÇÞö ¾Ê´Â Only Client Enum
	CHAT_TYPE_BIG_NOTICE,
#ifdef ENABLE_DICE_SYSTEM
	CHAT_TYPE_DICE_INFO, //11
#endif
	CHAT_TYPE_MAX_NUM,
};

enum EWhisperType
{
	WHISPER_TYPE_NORMAL = 0,
	WHISPER_TYPE_NOT_EXIST = 1,
	WHISPER_TYPE_TARGET_BLOCKED = 2,
	WHISPER_TYPE_SENDER_BLOCKED = 3,
	WHISPER_TYPE_ERROR = 4,
	WHISPER_TYPE_GM = 5,
	WHISPER_TYPE_SYSTEM = 0xFF
};

enum
{
	QUEST_SEND_IS_BEGIN = 1 << 0,
	QUEST_SEND_TITLE = 1 << 1,  // 28ÀÚ ±îÞö
	QUEST_SEND_CLOCK_NAME = 1 << 2,  // 16ÀÚ ±îÞö
	QUEST_SEND_CLOCK_VALUE = 1 << 3,
	QUEST_SEND_COUNTER_NAME = 1 << 4,  // 16ÀÚ ±îÞö
	QUEST_SEND_COUNTER_VALUE = 1 << 5,
	QUEST_SEND_ICON_FILE = 1 << 6,  // 24ÀÚ ±îÞö 
};

enum EGuildAuth
{
	GUILD_AUTH_ADD_MEMBER = (1 << 0),
	GUILD_AUTH_REMOVE_MEMBER = (1 << 1),
	GUILD_AUTH_NOTICE = (1 << 2),
	GUILD_AUTH_SKILL = (1 << 3),
};

enum EGuildWarState
{
	GUILD_WAR_NONE,
	GUILD_WAR_SEND_DECLARE,
	GUILD_WAR_REFUSE,
	GUILD_WAR_RECV_DECLARE,
	GUILD_WAR_WAIT_START,
	GUILD_WAR_CANCEL,
	GUILD_WAR_ON_WAR,
	GUILD_WAR_END,
	GUILD_WAR_OVER,
	GUILD_WAR_RESERVE,

	GUILD_WAR_DURATION = 2 * 60 * 60,
};

enum EWalkModes
{
	WALKMODE_RUN,
	WALKMODE_WALK,
};

enum SPECIAL_EFFECT
{
	SE_NONE,
	SE_HPUP_RED,
	SE_SPUP_BLUE,
	SE_SPEEDUP_GREEN,
	SE_DXUP_PURPLE,
	SE_CRITICAL,
	SE_PENETRATE,
	SE_BLOCK,
	SE_DODGE,
	SE_CHINA_FIREWORK,
	SE_SPIN_TOP,
	SE_SUCCESS,
	SE_FAIL,
	SE_FR_SUCCESS,
	SE_LEVELUP_ON_14_FOR_GERMANY,	//·¹º§¾÷ 14ÀÞ¶§ ( µ¶ÀÞÀü¿ë )
	SE_LEVELUP_UNDER_15_FOR_GERMANY,//·¹º§¾÷ 15ÀÞ¶§ ( µ¶ÀÞÀü¿ë )
	SE_PERCENT_DAMAGE1,
	SE_PERCENT_DAMAGE2,
	SE_PERCENT_DAMAGE3,
	SE_AUTO_HPUP,
	SE_AUTO_SPUP,
	SE_EQUIP_RAMADAN_RING,			// ÃÊ½Â´ÞÀÇ ¹ÞÞö¸¦ Âø¿ëÇÞ´Â ¼ø°£¿¡ ¹ßµ¿ÇÞ´Â ÀÌÆåÆ®
	SE_EQUIP_HALLOWEEN_CANDY,		// ÇÒ·ÞÀ© »çÅÞÀ» Âø¿ë(-_-;)ÇÑ ¼ø°£¿¡ ¹ßµ¿ÇÞ´Â ÀÌÆåÆ®
	SE_EQUIP_HAPPINESS_RING,		// Å©¸®½º¸¶½º Çàº¹ÀÇ ¹ÞÞö¸¦ Âø¿ëÇÞ´Â ¼ø°£¿¡ ¹ßµ¿ÇÞ´Â ÀÌÆåÆ®
	SE_EQUIP_LOVE_PENDANT,		// ¹ß·»Å¸ÀÞ »ç¶ûÀÇ ÆÒ´øÆ®(71145) Âø¿ëÇÒ ¶§ ÀÌÆåÆ® (¹ßµ¿ÀÌÆåÆ®ÀÓ, Þö¼ÓÀÌÆåÆ® ¾Æ´Ô)
#ifdef ENABLE_ACCE_SYSTEM
	SE_EFFECT_ACCE_SUCCEDED,
	SE_EFFECT_ACCE_EQUIP,
#endif
};

enum EBlockAction
{
	BLOCK_EXCHANGE = (1 << 0),
	BLOCK_PARTY_INVITE = (1 << 1),
	BLOCK_GUILD_INVITE = (1 << 2),
	BLOCK_WHISPER = (1 << 3),
	BLOCK_MESSENGER_INVITE = (1 << 4),
	BLOCK_PARTY_REQUEST = (1 << 5)
};

enum EInstantFlags
{
	INSTANT_FLAG_DEATH_PENALTY	= (1 << 0),
	INSTANT_FLAG_SHOP			= (1 << 1),
	INSTANT_FLAG_EXCHANGE		= (1 << 2),
	INSTANT_FLAG_STUN			= (1 << 3),
	INSTANT_FLAG_NO_REWARD		= (1 << 4),
};

enum EAiFlags
{
	AI_FLAG_NPC 			= (1 << 0),
	AI_FLAG_AGGRESSIVE		= (1 << 1),
	AI_FLAG_HELPER			= (1 << 2),
	AI_FLAG_STAYZONE		= (1 << 3),
};

enum
{
	MAIN_RACE_WARRIOR_M,
	MAIN_RACE_ASSASSIN_W,
	MAIN_RACE_SURA_M,
	MAIN_RACE_SHAMAN_W,
	MAIN_RACE_WARRIOR_W,
	MAIN_RACE_ASSASSIN_M,
	MAIN_RACE_SURA_W,
	MAIN_RACE_SHAMAN_M,
#ifdef ENABLE_WOLFMAN_CHARACTER
	MAIN_RACE_WOLFMAN_M,
#endif
	MAIN_RACE_MAX_NUM,
};

enum EOther
{
	POISON_LENGTH = 30,
#ifdef ENABLE_WOLFMAN_CHARACTER
	BLEEDING_LENGTH = 30,
#endif
	STAMINA_PER_STEP = 1,
	SAFEBOX_PAGE_SIZE = 9,
	AI_CHANGE_ATTACK_POISITION_TIME_NEAR = 10000,
	AI_CHANGE_ATTACK_POISITION_TIME_FAR = 1000,
	AI_CHANGE_ATTACK_POISITION_DISTANCE = 100,
	SUMMON_MONSTER_COUNT = 3,
};

enum FlyTypes
{
	FLY_NONE,
	FLY_EXP,
	FLY_HP_MEDIUM,
	FLY_HP_BIG,
	FLY_SP_SMALL,
	FLY_SP_MEDIUM,
	FLY_SP_BIG,
	FLY_FIREWORK1,
	FLY_FIREWORK2,
	FLY_FIREWORK3,
	FLY_FIREWORK4,
	FLY_FIREWORK5,
	FLY_FIREWORK6,
	FLY_FIREWORK_CHRISTMAS,
	FLY_CHAIN_LIGHTNING,
	FLY_HP_SMALL,
	FLY_SKILL_MUYEONG,
};

enum EDamageType
{
	DAMAGE_TYPE_NONE,
	DAMAGE_TYPE_NORMAL,
	DAMAGE_TYPE_NORMAL_RANGE,
	//��ų
	DAMAGE_TYPE_MELEE,
	DAMAGE_TYPE_RANGE,
	DAMAGE_TYPE_FIRE,
	DAMAGE_TYPE_ICE,
	DAMAGE_TYPE_ELEC,
	DAMAGE_TYPE_MAGIC,
	DAMAGE_TYPE_POISON,
	DAMAGE_TYPE_SPECIAL,
#ifdef ENABLE_WOLFMAN_CHARACTER
	DAMAGE_TYPE_BLEEDING,
#endif
	DAMAGE_TYPE_MAX_NUM,
};

enum DamageFlag
{
	DAMAGE_NORMAL	= (1 << 0),
	DAMAGE_POISON	= (1 << 1),
	DAMAGE_DODGE	= (1 << 2),
	DAMAGE_BLOCK	= (1 << 3),
	DAMAGE_PENETRATE= (1 << 4),
	DAMAGE_CRITICAL = (1 << 5),
#if defined(ENABLE_WOLFMAN_CHARACTER) && !defined(USE_MOB_BLEEDING_AS_POISON)
	DAMAGE_BLEEDING	= (1 << 6),
#endif
};

enum EPositions
{
	POS_DEAD,
	POS_SLEEPING,
	POS_RESTING,
	POS_SITTING,
	POS_FISHING,
	POS_FIGHTING,
	POS_MOUNTING,
	POS_STANDING
};

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

enum // unit : minute
{
	PARTY_ENOUGH_MINUTE_FOR_EXP_BONUS = 60, // ÆÄÆ¼ °á¼º ÈÄ 60ºÞ ÈÄ ºÞÅÞ Ãß°¡ °æÇèÄ¡ º¸³Ê½º
	PARTY_HEAL_COOLTIME_LONG = 60,
	PARTY_HEAL_COOLTIME_SHORT = 30,
	PARTY_MAX_MEMBER = 8,
	PARTY_DEFAULT_RANGE = 5000,
};

enum EPartyMessages
{
	PM_ATTACK,		// Attack him
	PM_RETURN,		// Return back to position
	PM_ATTACKED_BY,	// I was attacked by someone
	PM_AGGRO_INCREASE,	// My aggro is increased
};

enum EPartyRole
{   
	PARTY_ROLE_NORMAL,  
	PARTY_ROLE_LEADER,
	PARTY_ROLE_ATTACKER,
	PARTY_ROLE_TANKER,
	PARTY_ROLE_BUFFER,  
	PARTY_ROLE_SKILL_MASTER,
	PARTY_ROLE_HASTE,   
	PARTY_ROLE_DEFENDER,
	PARTY_ROLE_MAX_NUM, 
}; 

enum ECreateTarget
{
	CREATE_TARGET_TYPE_NONE,
	CREATE_TARGET_TYPE_LOCATION,
	CREATE_TARGET_TYPE_CHARACTER,
};

enum
{
	QUICKSLOT_TYPE_NONE,
	QUICKSLOT_TYPE_ITEM,
	QUICKSLOT_TYPE_SKILL,
	QUICKSLOT_TYPE_COMMAND,
	QUICKSLOT_TYPE_MAX_NUM
};

enum ESlotType
{
	SLOT_TYPE_NONE,
	SLOT_TYPE_INVENTORY,
	SLOT_TYPE_SKILL,
	SLOT_TYPE_EMOTION,
	SLOT_TYPE_SHOP,
	SLOT_TYPE_EXCHANGE_OWNER,
	SLOT_TYPE_EXCHANGE_TARGET,
	SLOT_TYPE_QUICK_SLOT,
	SLOT_TYPE_SAFEBOX,
	SLOT_TYPE_PRIVATE_SHOP,
	SLOT_TYPE_MALL,
	SLOT_TYPE_DRAGON_SOUL_INVENTORY,
	SLOT_TYPE_MAX,
};

enum EParts
{
	PART_MAIN,
	PART_WEAPON,
	PART_HEAD,
	PART_HAIR,

#ifdef ENABLE_ACCE_SYSTEM
	PART_ACCE,
#endif
	PART_MAX_NUM,
	PART_WEAPON_SUB,
};

enum EGMLevels
{
	GM_PLAYER,
	GM_LOW_WIZARD,
	GM_WIZARD,
	GM_HIGH_WIZARD,
	GM_GOD,
	GM_IMPLEMENTOR,
	GM_MAX_NUM,
};

enum EGMAllowFlags
{
	GM_ALLOW_DROP_PLAYER_ITEM = 1 << 0,
	GM_ALLOW_DROP_GM_ITEM = 1 << 1,
	GM_ALLOW_EXCHANGE_PLAYER_ITEM_TO_GM = 1 << 2,
	GM_ALLOW_EXCHANGE_PLAYER_ITEM_TO_PLAYER = 1 << 3,
	GM_ALLOW_EXCHANGE_GM_ITEM_TO_GM = 1 << 4,
	GM_ALLOW_EXCHANGE_GM_ITEM_TO_PLAYER = 1 << 5,
	GM_ALLOW_EXCHANGE_TO_GM = 1 << 6,
	GM_ALLOW_EXCHANGE_TO_PLAYER = 1 << 7,
	GM_ALLOW_BUY_PRIVATE_ITEM = 1 << 8,
	GM_ALLOW_CREATE_PRIVATE_SHOP = 1 << 9,
	GM_ALLOW_USE_SAFEBOX = 1 << 10,
	GM_ALLOW_CREATE_PLAYER = 1 << 11,
	GM_ALLOW_DELETE_PLAYER = 1 << 12,
};

typedef enum
{
	SHOP_COIN_TYPE_GOLD, // DEFAULT VALUE
	SHOP_COIN_TYPE_SECONDARY_COIN
} EShopCoinType;

enum EMobRank
{
	MOB_RANK_PAWN,
	MOB_RANK_S_PAWN,
	MOB_RANK_KNIGHT,
	MOB_RANK_S_KNIGHT,
	MOB_RANK_BOSS,
	MOB_RANK_KING,
	MOB_RANK_MAX_NUM
};

enum ECharType
{
	CHAR_TYPE_MONSTER,
	CHAR_TYPE_NPC,
	CHAR_TYPE_STONE,
	CHAR_TYPE_WARP,
	CHAR_TYPE_DOOR,
	CHAR_TYPE_BUILDING,
	CHAR_TYPE_PC,
	CHAR_TYPE_POLYMORPH_PC,
	CHAR_TYPE_HORSE,
	CHAR_TYPE_GOTO
};

enum EBattleType
{
	BATTLE_TYPE_MELEE,
	BATTLE_TYPE_RANGE,
	BATTLE_TYPE_MAGIC,
	BATTLE_TYPE_SPECIAL,
	BATTLE_TYPE_POWER,
	BATTLE_TYPE_TANKER,
	BATTLE_TYPE_SUPER_POWER,
	BATTLE_TYPE_SUPER_TANKER,
	BATTLE_TYPE_MAX_NUM
};

enum
{
	SKILL_ATTR_TYPE_NORMAL = 1,
	SKILL_ATTR_TYPE_MELEE,
	SKILL_ATTR_TYPE_RANGE,
	SKILL_ATTR_TYPE_MAGIC,
	SKILL_ATTR_TYPE_FIRE,
	SKILL_ATTR_TYPE_ICE,
	SKILL_ATTR_TYPE_ELEC,
	SKILL_ATTR_TYPE_DARK
};

enum
{
	SKILL_NORMAL,
	SKILL_MASTER,
	SKILL_GRAND_MASTER,
	SKILL_PERFECT_MASTER
};

enum EGuildWarType
{
	GUILD_WAR_TYPE_FIELD,
	GUILD_WAR_TYPE_BATTLE,
	GUILD_WAR_TYPE_FLAG,
	GUILD_WAR_TYPE_MAX_NUM
};
enum EAttributeSet
{
	ATTRIBUTE_SET_WEAPON,
	ATTRIBUTE_SET_BODY,
	ATTRIBUTE_SET_WRIST,
	ATTRIBUTE_SET_FOOTS,
	ATTRIBUTE_SET_NECK,
	ATTRIBUTE_SET_HEAD,
	ATTRIBUTE_SET_SHIELD,
	ATTRIBUTE_SET_EAR,
#ifdef ENABLE_ITEM_ATTR_COSTUME
	ATTRIBUTE_SET_COSTUME_BODY,
	ATTRIBUTE_SET_COSTUME_HAIR,
#if defined(ENABLE_ITEM_ATTR_COSTUME) && defined(ENABLE_WEAPON_COSTUME_SYSTEM)
	ATTRIBUTE_SET_COSTUME_WEAPON,
#endif
#endif
	ATTRIBUTE_SET_MAX_NUM
};

enum EPrivType
{
	PRIV_NONE,
	PRIV_ITEM_DROP,
	PRIV_GOLD_DROP,
	PRIV_GOLD10_DROP,
	PRIV_EXP_PCT,
	MAX_PRIV_NUM
};

enum EMoneyLogType
{
	MONEY_LOG_RESERVED,
	MONEY_LOG_MONSTER,
	MONEY_LOG_SHOP,
	MONEY_LOG_REFINE,
	MONEY_LOG_QUEST,
	MONEY_LOG_GUILD,
	MONEY_LOG_MISC,
	MONEY_LOG_MONSTER_KILL,
	MONEY_LOG_DROP,
	MONEY_LOG_TYPE_MAX_NUM
};

enum EPremiumTypes
{
	PREMIUM_EXP,		// °æÇèÄ¡°¡ 1.2¹è
	PREMIUM_ITEM,		// ¾ÆÀÌÅÛ µå·ÓÀ²ÀÌ 2¹è
	PREMIUM_SAFEBOX,		// Ã¢°í°¡ 1Ä­¿¡¼­ 3Ä­
	PREMIUM_AUTOLOOT,		// µ· ÀÚµ¿ ÞÞ±â
	PREMIUM_FISH_MIND,		// °í±Þ ¹°°í±â ³¬ÀÞ È®·ü »ó½Â
	PREMIUM_MARRIAGE_FAST,	// ±Þ½Ç Þõ°¡ ¾çÀ» ºü¸£°ÔÇÕ´Þ´Ù.
	PREMIUM_GOLD,		// µ· µå·ÓÀ²ÀÌ 1.5¹è
	PREMIUM_MAX_NUM = 9
};

enum ECharacterPosition
{
	POSITION_GENERAL,
	POSITION_BATTLE,
	POSITION_SITTING_CHAIR,
	POSITION_SITTING_GROUND,
	POSITION_MAX_NUM
};

enum ESkillFlags
{
	SKILL_FLAG_ATTACK			= (1 << 0),	// °ø°İ ±â¼ú
	SKILL_FLAG_USE_MELEE_DAMAGE		= (1 << 1),	// ±âº» ¹Ğ¸® Å¸°İÄ¡¸¦ b °ªÀ¸·Þ »ç¿ë
	SKILL_FLAG_COMPUTE_ATTGRADE		= (1 << 2),	// °ø°İµî±ŞÀ» °è»êÇÑ´Ù
	SKILL_FLAG_SELFONLY			= (1 << 3),	// ÀÚ½Å¿¡°Ô¸¸ ¾µ ¼ö ÀÖÀ½
	SKILL_FLAG_USE_MAGIC_DAMAGE		= (1 << 4),	// ±âº» ¸¶¹ı Å¸°İÄ¡¸¦ b °ªÀ¸·Þ »ç¿ë
	SKILL_FLAG_USE_HP_AS_COST		= (1 << 5),	// HP¸¦ SP´ë½Å ¾´´Ù
	SKILL_FLAG_COMPUTE_MAGIC_DAMAGE	= (1 << 6),
	SKILL_FLAG_SPLASH			= (1 << 7),
	SKILL_FLAG_GIVE_PENALTY		= (1 << 8),	// ¾²°í³ª¸é Àá½Ãµ¿¾È(3ÃÊ) 2¹è µ¥¹ÌÞö¸¦ ¹Ş´Â´Ù.
	SKILL_FLAG_USE_ARROW_DAMAGE		= (1 << 9),	// ±âº» È­»ì Å¸°İÄ¡¸¦ b °ªÀ¸·Þ »ç¿ë
	SKILL_FLAG_PENETRATE		= (1 << 10),	// ¹æ¾î¹«½Ã
	SKILL_FLAG_IGNORE_TARGET_RATING	= (1 << 11),	// »ó´ë ·¹ÀÌÆÃ ¹«½Ã
	SKILL_FLAG_SLOW			= (1 << 12),	// ½½·Þ¿ì °ø°İ
	SKILL_FLAG_STUN			= (1 << 13),	// ½ºÅÞ °ø°İ
	SKILL_FLAG_HP_ABSORB		= (1 << 14),	// HP Èí¼ö
	SKILL_FLAG_SP_ABSORB		= (1 << 15),	// SP Èí¼ö
	SKILL_FLAG_FIRE_CONT		= (1 << 16),	// FIRE Þö¼Ó µ¥¹ÌÞö
	SKILL_FLAG_REMOVE_BAD_AFFECT	= (1 << 17),	// ³ª»ÛÈ¿°ú Þ¦°Å
	SKILL_FLAG_REMOVE_GOOD_AFFECT	= (1 << 18),	// ³ª»ÛÈ¿°ú Þ¦°Å
	SKILL_FLAG_CRUSH			= (1 << 19),	// »ó´ë¹æÀ» ³¯¸²
	SKILL_FLAG_POISON			= (1 << 20),	// µ¶ °ø°İ
	SKILL_FLAG_TOGGLE			= (1 << 21),	// Åä±Û
	SKILL_FLAG_DISABLE_BY_POINT_UP	= (1 << 22),	// Âï¾î¼­ ¿Ã¸± ¼ö ¾ø´Ù.
	SKILL_FLAG_CRUSH_LONG		= (1 << 23),	// »ó´ë¹æÀ» ¸Ö¸® ³¯¸²
	SKILL_FLAG_WIND		= (1 << 24),	// ¹Ù¶÷ ¼Ó¼º 
	SKILL_FLAG_ELEC		= (1 << 25),	// Àü±â ¼Ó¼º
	SKILL_FLAG_FIRE		= (1 << 26),	// ºÒ ¼Ó¼º
#ifdef ENABLE_WOLFMAN_CHARACTER
	SKILL_FLAG_BLEEDING	= (1 << 27),
	SKILL_FLAG_PARTY	= (1 << 28),
#endif
};

enum
{
	SKILL_PENALTY_DURATION = 3,
	SKILL_TYPE_HORSE = 5,
};

enum ESkillIndexes
{
	SKILL_RESERVED = 0,

	// ©ö¡ì¡íc Au¡íc ¡Æe¢¯¡©		// - bedensel
	// A
	SKILL_SAMYEON = 1,		// 3 yonlu kesme
	SKILL_PALBANG,			// k©¥l©¥c cevirme
	// S
	SKILL_JEONGWI,			// ofke
	SKILL_GEOMKYUNG,		// hava
	SKILL_TANHWAN,			// hamle

	// ©ö¡ì¡íc ¡¾a¡Æ©ª ¡Æe¢¯¡©		// - zihinsel
	// A
	SKILL_GIGONGCHAM = 16,	// ruh
	SKILL_GYOKSAN,			// ?iddetli vurul
	SKILL_DAEJINGAK,		// guclu vuru?
	// S
	SKILL_CHUNKEON,			// guclu beden
	SKILL_GEOMPUNG,			// k©¥l©¥c darbesi

	// AU¡Æ¢¥ ¨úI¡íi ¡Æe¢¯¡©		// - yak©¥n dovu?
	// A
	SKILL_AMSEOP = 31,		// suikast
	SKILL_GUNGSIN,			// h©¥zl©¥ sald©¥r©¥
	SKILL_CHARYUN,			// b©¥cak cevirme
	// S
	SKILL_EUNHYUNG,			// kamuflaj 
	SKILL_SANGONG,			// zehirli bulut

	// AU¡Æ¢¥ ¡¾A¨ùo ¡Æe¢¯¡©		// - uzak dovu?
	// A
	SKILL_YEONSA = 46,		// tekrarlanan at©¥?
	SKILL_KWANKYEOK,		// ok ya?muru
	SKILL_HWAJO,			// ate?li ok
	// S
	SKILL_GYEONGGONG,		// hafif ad©¥m
	SKILL_GIGUNG,			// zehirli ok

	// ¨ùo¢Òo ¡ÆE				// - buyulu silah
	// A
	SKILL_SWAERYUNG = 61,	// parmak darbesi
	SKILL_YONGKWON,			// ejderha donu?u
	// S
	SKILL_GWIGEOM,			// buyulu keskinlik
	SKILL_TERROR,			// deh?et
	SKILL_JUMAGAP,			// buyulu z©¥rh
	SKILL_PABEOB,			// buyu cozme

	// ¨ùo¢Òo ¢¬¢Ò©ö©¥			// - kara buyu
	// A
	SKILL_MARYUNG = 76,		// karanl©¥k vuru?
	SKILL_HWAYEOMPOK,		// ate? vuru?
	SKILL_MUYEONG,			// ate? hayaleti
	// S
	SKILL_MANASHIELD,		// karanl©¥k koruma
	SKILL_TUSOK,			// hayalet vuru?
	SKILL_MAHWAN,			// karanl©¥k kure

	// ©ö¡ì¢¥c ¢¯e¨öA
	// A					// - ejderha gucu
	SKILL_BIPABU = 91,		// ucan t©¥ls©¥m
	SKILL_YONGBI,			// ejderha at©¥?©¥
	SKILL_PAERYONG,			// ejderha kukremesi
	// S
	//SKILL_BUDONG,			// pasif yada eski ?aman skilli
	SKILL_HOSIN,			// kutsama
	SKILL_REFLECT,			// yans©¥tma
	SKILL_GICHEON,			// ejderha yard©¥m©¥

	// ©ö¡ì¢¥c ©øu¨öA
	// A					// - Iyile?tirme
	SKILL_NOEJEON = 106,	// ?im?ek atma
	SKILL_BYEURAK,			// ?im?ek Ca?©¥rma
	SKILL_CHAIN,			// ?im?ek pencesi
	// S
	SKILL_JEONGEOP,			// Iyile?tirme
	SKILL_KWAESOK,			// H©¥z
	SKILL_JEUNGRYEOK,		// Yuksek sald©¥r©¥

	// ¨¬¢¬A¢Ò ¨ö¨¬A©ø

	SKILL_LEADERSHIP	= 121,	// Liderlik
	SKILL_COMBO			= 122,	// Kombo
	SKILL_CREATE		= 123,	// ?? - Uzman recetesi ile yukseltilen skill
	SKILL_MINING		= 124,	// Madencilik

	SKILL_LANGUAGE1		= 126,	// Shinsoo lisan 
	SKILL_LANGUAGE2		= 127,	// Chunjo Lisan
	SKILL_LANGUAGE3		= 128,	// Jinno Lisan
	SKILL_POLYMORPH		= 129,	// Donu?me

	SKILL_HORSE						= 130,	// Binicilik
	SKILL_HORSE_SUMMON				= 131,	// At ca?©¥rma
	SKILL_HORSE_WILDATTACK			= 137,	// At uzerinde sald©¥r©¥
	SKILL_HORSE_CHARGE				= 138,	// At yuruyu?u
	SKILL_HORSE_ESCAPE				= 139,	// Guc dalgas©¥
	SKILL_HORSE_WILDATTACK_RANGE	= 140,	// Ok f©¥rt©¥nas©¥

	SKILL_ADD_HP			= 141,	// HP uretme - Pasif skill 		/ Aktif de?il
	SKILL_RESIST_PENETRATE	= 142,	// Delici vuru? - Pasif skill 	/ Aktif de?il

	GUILD_SKILL_START		= 151,			// [LONCA] Ba?lang©¥c
	GUILD_SKILL_EYE			= 151,			// Ejderha Gozleri 				/ Ejderha Hayaletleri
	GUILD_SKILL_BLOOD		= 152,			// Ejderha Tanr©¥s©¥ Kan©¥ 		/ Max HP
	GUILD_SKILL_BLESS		= 153,			// Ejderha Tanr©¥s©¥ Kutsamas©¥ 	/ Max SP
	GUILD_SKILL_SEONGHWI	= 154,			// Kutsal Z©¥rh 					/ Savunma
	GUILD_SKILL_ACCEL		= 155,			// H©¥zland©¥rma 					/ Hareket H©¥z©¥
	GUILD_SKILL_BUNNO		= 156,			// Ejderha Tanr©¥s©¥ ofkesi 		/ Kritik
	GUILD_SKILL_JUMUN		= 157,			// Buyu h©¥z©¥ deste?i 			/ Buyu h©¥z©¥
	GUILD_SKILL_END			= 162,			// [LONCA] Son

	GUILD_SKILL_COUNT		= GUILD_SKILL_END - GUILD_SKILL_START + 1,

#ifdef ENABLE_WOLFMAN_CHARACTER
	SKILL_CHAYEOL			= 170,			// Y©¥rtma	Parcalama
	SKILL_SALPOONG			= 171,			// Kurt Nefesi
	SKILL_GONGDAB			= 172,			// Kurt Atlay©¥?©¥
	SKILL_PASWAE			= 173,			// Kurt Pencesi
	SKILL_JEOKRANG			= 174,			// K©¥rm©¥z©¥ Kurt Ruhu
	SKILL_CHEONGRANG		= 175,			// Civit Kurt Ruhu
#endif
};

