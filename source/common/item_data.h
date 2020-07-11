#pragma once
#include <cstdint>
#include "defines.h"
#include "constants.h"

enum EItemMisc
{
	ITEM_NAME_MAX_LEN			= 24,
	ITEM_VALUES_MAX_NUM			= 6,
	ITEM_SMALL_DESCR_MAX_LEN	= 256,
	ITEM_LIMIT_MAX_NUM			= 2,
	ITEM_APPLY_MAX_NUM			= 3,
	ITEM_SOCKET_MAX_NUM			= 3,
	ITEM_MAX_COUNT				= 200,

	ITEM_ATTRIBUTE_NORM_NUM		= 5,
	ITEM_ATTRIBUTE_RARE_NUM		= 2,

	ITEM_ATTRIBUTE_NORM_START	= 0,
	ITEM_ATTRIBUTE_NORM_END		= ITEM_ATTRIBUTE_NORM_START + ITEM_ATTRIBUTE_NORM_NUM,

	ITEM_ATTRIBUTE_RARE_START	= ITEM_ATTRIBUTE_NORM_END,
	ITEM_ATTRIBUTE_RARE_END		= ITEM_ATTRIBUTE_RARE_START + ITEM_ATTRIBUTE_RARE_NUM,

	ITEM_ATTRIBUTE_MAX_NUM		= 7,
	ITEM_ATTRIBUTE_MAX_LEVEL	= 5,
	ITEM_AWARD_WHY_MAX_LEN		= 50,

	ITEM_ELK_VNUM				= 50026,

	ITEM_SOCKET_REMAIN_SEC = 0,
};

enum EItemValueIdice
{
	ITEM_VALUE_DRAGON_SOUL_POLL_OUT_BONUS_IDX = 0,
	ITEM_VALUE_CHARGING_AMOUNT_IDX = 0,
	ITEM_VALUE_SECONDARY_COIN_UNIT_IDX = 0,
};

enum EItemDragonSoulSockets
{
	ITEM_SOCKET_DRAGON_SOUL_ACTIVE_IDX = 2,
	ITEM_SOCKET_CHARGING_AMOUNT_IDX = 2,
};

enum EItemUniqueSockets
{
	ITEM_SOCKET_UNIQUE_SAVE_TIME = ITEM_SOCKET_MAX_NUM - 2,
	ITEM_SOCKET_UNIQUE_REMAIN_TIME = ITEM_SOCKET_MAX_NUM - 1
};

enum EDragonSoulGradeTypes
{
	DRAGON_SOUL_GRADE_NORMAL,
	DRAGON_SOUL_GRADE_BRILLIANT,
	DRAGON_SOUL_GRADE_RARE,
	DRAGON_SOUL_GRADE_ANCIENT,
	DRAGON_SOUL_GRADE_LEGENDARY,
	DRAGON_SOUL_GRADE_MYTH,
	DRAGON_SOUL_GRADE_MAX,
};

enum EDragonSoulStepTypes
{
	DRAGON_SOUL_STEP_LOWEST,
	DRAGON_SOUL_STEP_LOW,
	DRAGON_SOUL_STEP_MID,
	DRAGON_SOUL_STEP_HIGH,
	DRAGON_SOUL_STEP_HIGHEST,
	DRAGON_SOUL_STEP_MAX,
};
#define DRAGON_SOUL_STRENGTH_MAX 7

enum EDragonSoulSubType
{
	DS_SLOT1,
	DS_SLOT2,
	DS_SLOT3,
	DS_SLOT4,
	DS_SLOT5,
	DS_SLOT6,
	DS_SLOT_MAX,
};

enum EFishSubTypes
{
	FISH_ALIVE,
	FISH_DEAD,
};

enum EResourceSubTypes
{
	RESOURCE_FISHBONE,
	RESOURCE_WATERSTONEPIECE,
	RESOURCE_WATERSTONE,
	RESOURCE_BLOOD_PEARL,
	RESOURCE_BLUE_PEARL,
	RESOURCE_WHITE_PEARL,
	RESOURCE_BUCKET,
	RESOURCE_CRYSTAL,
	RESOURCE_GEM,
	RESOURCE_STONE,
	RESOURCE_METIN,
	RESOURCE_ORE,
};

enum EItemType
{
	ITEM_TYPE_NONE,					//0
	ITEM_TYPE_WEAPON,				//1//¹«±â
	ITEM_TYPE_ARMOR,				//2//°©¿Ê
	ITEM_TYPE_USE,					//3//¾ÆÀÌÅÛ »ç¿ë
	ITEM_TYPE_AUTOUSE,				//4
	ITEM_TYPE_MATERIAL,				//5
	ITEM_TYPE_SPECIAL,				//6 //½ºÆä¼È ¾ÆÀÌÅÛ
	ITEM_TYPE_TOOL,					//7
	ITEM_TYPE_LOTTERY,				//8//º¹±Ç
	ITEM_TYPE_ELK,					//9//µ·
	ITEM_TYPE_METIN,				//10
	ITEM_TYPE_CONTAINER,			//11
	ITEM_TYPE_FISH,					//12//³¬½Ã
	ITEM_TYPE_ROD,					//13
	ITEM_TYPE_RESOURCE,				//14
	ITEM_TYPE_CAMPFIRE,				//15
	ITEM_TYPE_UNIQUE,				//16
	ITEM_TYPE_SKILLBOOK,			//17
	ITEM_TYPE_QUEST,				//18
	ITEM_TYPE_POLYMORPH,			//19
	ITEM_TYPE_TREASURE_BOX,			//20//º¸¹°»óÀÚ
	ITEM_TYPE_TREASURE_KEY,			//21//º¸¹°»óÀÚ ¿­¼è
	ITEM_TYPE_SKILLFORGET,			//22
	ITEM_TYPE_GIFTBOX,				//23
	ITEM_TYPE_PICK,					//24
	ITEM_TYPE_HAIR,					//25//¸Ó¸®
	ITEM_TYPE_TOTEM,				//26//ÅäÅÛ
	ITEM_TYPE_BLEND,				//27//»ý¼ºµÉ¶§ ·£´ýÇÏ°Ô ¼Ó¼ºÀÌ ºÙ´Â ¾à¹°
	ITEM_TYPE_COSTUME,				//28//ÄÚ½ºÃõ ¾ÆÀÌÅÛ (2011³â 8¿ù Ãß°¡µÈ ÄÚ½ºÃõ ½Ã½ºÅÛ¿ë ¾ÆÀÌÅÛ)
	ITEM_TYPE_DS,					//29 //¿ëÈ¥¼®
	ITEM_TYPE_SPECIAL_DS,			//30 // Æ¯¼öÇÑ ¿ëÈ¥¼® (DS_SLOT¿¡ Âø¿ëÇÏ´Â UNIQUE ¾ÆÀÌÅÛÀÌ¶ó »ý°¢ÇÏ¸é µÊ)
	ITEM_TYPE_EXTRACT,				//31 ÃßÃâµµ±¸.
	ITEM_TYPE_SECONDARY_COIN,		//32 ¸íµµÀü.
	ITEM_TYPE_RING,					//33 ¹ÝÁö (À¯´ÏÅ© ½½·ÔÀÌ ¾Æ´Ñ ¼ø¼ö ¹ÝÁö ½½·Ô)
	ITEM_TYPE_BELT,					//34 º§Æ®
	ITEM_TYPE_MAX_NUM,
};

enum EWeaponSubTypes
{
	WEAPON_SWORD,
	WEAPON_DAGGER,
	WEAPON_BOW,
	WEAPON_TWO_HANDED,
	WEAPON_BELL,
	WEAPON_FAN,
	WEAPON_ARROW,
	WEAPON_MOUNT_SPEAR,
#ifdef ENABLE_WOLFMAN_CHARACTER
	WEAPON_CLAW = 8, //8
#endif
	WEAPON_NUM_TYPES,
	WEAPON_NONE = WEAPON_NUM_TYPES+1,
};

enum EMaterialSubTypes
{
	MATERIAL_LEATHER,
	MATERIAL_BLOOD,
	MATERIAL_ROOT,
	MATERIAL_NEEDLE,
	MATERIAL_JEWEL,
	MATERIAL_DS_REFINE_NORMAL, 
	MATERIAL_DS_REFINE_BLESSED, 
	MATERIAL_DS_REFINE_HOLLY,
};
		
enum EUniqueSubTypes
{
	UNIQUE_NONE,
	UNIQUE_BOOK,
	UNIQUE_SPECIAL_RIDE,
	UNIQUE_SPECIAL_MOUNT_RIDE,
};

enum EExtractSubTypes
{
	EXTRACT_DRAGON_SOUL,
	EXTRACT_DRAGON_HEART,
};

enum EAutoUseSubTypes
{
	AUTOUSE_POTION,
	AUTOUSE_ABILITY_UP,
	AUTOUSE_BOMB,
	AUTOUSE_GOLD,
	AUTOUSE_MONEYBAG,
	AUTOUSE_TREASURE_BOX
};

enum ESpecialSubTypes
{
	SPECIAL_MAP,
	SPECIAL_KEY,
	SPECIAL_DOC,
	SPECIAL_SPIRIT,
};

enum EToolSubTypes
{
	TOOL_FISHING_ROD
};

enum ELotterySubTypes
{
	LOTTERY_TICKET,
	LOTTERY_INSTANT
};

enum EArmorSubTypes
{
	ARMOR_BODY,
	ARMOR_HEAD,
	ARMOR_SHIELD,
	ARMOR_WRIST,
	ARMOR_FOOTS,
	ARMOR_NECK,
	ARMOR_EAR,
	ARMOR_NUM_TYPES
};

enum ECostumeSubTypes
{
	COSTUME_BODY,				//0	°©¿Ê(main look)
	COSTUME_HAIR,				//1	Çì¾î(Å»Âø°¡´É)
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	COSTUME_MOUNT = 2,		//2
#endif
#ifdef ENABLE_ACCE_SYSTEM
	COSTUME_ACCE = 3,		//3
#endif
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	COSTUME_WEAPON = 4,		//4
#endif
	COSTUME_NUM_TYPES,
};

enum EUseSubTypes
{
	USE_POTION,							// 0
	USE_TALISMAN,
	USE_TUNING,
	USE_MOVE,
	USE_TREASURE_BOX,
	USE_MONEYBAG,
	USE_BAIT,
	USE_ABILITY_UP,
	USE_AFFECT,
	USE_CREATE_STONE,
	USE_SPECIAL,						// 10
	USE_POTION_NODELAY,
	USE_CLEAR,
	USE_INVISIBILITY,
	USE_DETACHMENT,
	USE_BUCKET,
	USE_POTION_CONTINUE,
	USE_CLEAN_SOCKET,
	USE_CHANGE_ATTRIBUTE,
	USE_ADD_ATTRIBUTE,
	USE_ADD_ACCESSORY_SOCKET,			// 20
	USE_PUT_INTO_ACCESSORY_SOCKET,
	USE_ADD_ATTRIBUTE2,
	USE_RECIPE,
	USE_CHANGE_ATTRIBUTE2,
	USE_BIND,
	USE_UNBIND,
	USE_TIME_CHARGE_PER,
	USE_TIME_CHARGE_FIX,				// 28
	USE_PUT_INTO_BELT_SOCKET,			// 29
	USE_PUT_INTO_RING_SOCKET,			// 30
#ifdef ENABLE_USE_COSTUME_ATTR
	USE_CHANGE_COSTUME_ATTR,			// 31
	USE_RESET_COSTUME_ATTR,				// 32
#endif
};

enum EMetinSubTypes
{
	METIN_NORMAL,
	METIN_GOLD
};

enum ELimitTypes
{
	LIMIT_NONE,

	LIMIT_LEVEL,
	LIMIT_STR,
	LIMIT_DEX,
	LIMIT_INT,
	LIMIT_CON,
	LIMIT_PCBANG,

	LIMIT_REAL_TIME,						

	LIMIT_REAL_TIME_START_FIRST_USE,

	LIMIT_TIMER_BASED_ON_WEAR,

	LIMIT_MAX_NUM
};

enum EItemAntiFlag
{
	ITEM_ANTIFLAG_FEMALE = (1 << 0),
	ITEM_ANTIFLAG_MALE = (1 << 1),
	ITEM_ANTIFLAG_WARRIOR = (1 << 2),
	ITEM_ANTIFLAG_ASSASSIN = (1 << 3),
	ITEM_ANTIFLAG_SURA = (1 << 4),
	ITEM_ANTIFLAG_SHAMAN = (1 << 5),
	ITEM_ANTIFLAG_GET = (1 << 6),
	ITEM_ANTIFLAG_DROP = (1 << 7),
	ITEM_ANTIFLAG_SELL = (1 << 8),
	ITEM_ANTIFLAG_EMPIRE_A = (1 << 9),
	ITEM_ANTIFLAG_EMPIRE_B = (1 << 10),
	ITEM_ANTIFLAG_EMPIRE_R = (1 << 11),
	ITEM_ANTIFLAG_SAVE = (1 << 12),
	ITEM_ANTIFLAG_GIVE = (1 << 13),
	ITEM_ANTIFLAG_PKDROP = (1 << 14),
	ITEM_ANTIFLAG_STACK = (1 << 15),
	ITEM_ANTIFLAG_MYSHOP = (1 << 16),
	ITEM_ANTIFLAG_SAFEBOX = (1 << 17),
#ifdef ENABLE_WOLFMAN_CHARACTER
	ITEM_ANTIFLAG_WOLFMAN = (1 << 18),
#endif
};

enum EItemFlag
{
	ITEM_FLAG_REFINEABLE		= (1 << 0),
	ITEM_FLAG_SAVE			= (1 << 1),
	ITEM_FLAG_STACKABLE		= (1 << 2),
	ITEM_FLAG_COUNT_PER_1GOLD	= (1 << 3),
	ITEM_FLAG_SLOW_QUERY		= (1 << 4),
	ITEM_FLAG_RARE		= (1 << 5),
	ITEM_FLAG_UNIQUE		= (1 << 6),
	ITEM_FLAG_MAKECOUNT		= (1 << 7),
	ITEM_FLAG_IRREMOVABLE		= (1 << 8),
	ITEM_FLAG_CONFIRM_WHEN_USE	= (1 << 9),
	ITEM_FLAG_QUEST_USE		= (1 << 10),
	ITEM_FLAG_QUEST_USE_MULTIPLE	= (1 << 11),
	ITEM_FLAG_QUEST_GIVE		= (1 << 12),
	ITEM_FLAG_LOG			= (1 << 13),
	ITEM_FLAG_APPLICABLE		= (1 << 14),
};

enum EWearPositions
{
	WEAR_BODY,			// 0
	WEAR_HEAD,			// 1
	WEAR_FOOTS,			// 2
	WEAR_WRIST,			// 3
	WEAR_WEAPON,		// 4
	WEAR_NECK,			// 5
	WEAR_EAR,			// 6
	WEAR_UNIQUE1,		// 7
	WEAR_UNIQUE2,		// 8
	WEAR_ARROW,			// 9
	WEAR_SHIELD,		// 10
	WEAR_ABILITY1,		// 11
	WEAR_ABILITY2,		// 12
	WEAR_ABILITY3,		// 13
	WEAR_ABILITY4,		// 14
	WEAR_ABILITY5,		// 15
	WEAR_ABILITY6,		// 16
	WEAR_ABILITY7,		// 17
	WEAR_ABILITY8,		// 18
	WEAR_COSTUME_BODY,	// 19
	WEAR_COSTUME_HAIR,	// 20

	WEAR_RING1,			// 21	: ½Å±Ô ¹İÁö½½·Ô1 (¿ŞÂÊ)
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	WEAR_COSTUME_MOUNT = WEAR_RING1, // costume_mount == ring1
#endif

	WEAR_RING2,			// 22	: ½Å±Ô ¹İÁö½½·Ô2 (¿À¸¥ÂÊ)
#ifdef ENABLE_ACCE_SYSTEM
	WEAR_COSTUME_ACCE = WEAR_RING2, // costume_acce == ring2
#endif

	WEAR_BELT,			// 23	: ½Å±Ô º§Æ®½½·Ô

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	WEAR_COSTUME_WEAPON,// 24
#endif
};

enum EItemWearableFlag
{
	WEARABLE_BODY			= (1 << 0),
	WEARABLE_HEAD			= (1 << 1),
	WEARABLE_FOOTS			= (1 << 2),
	WEARABLE_WRIST			= (1 << 3),
	WEARABLE_WEAPON			= (1 << 4),
	WEARABLE_NECK			= (1 << 5),
	WEARABLE_EAR			= (1 << 6),
	WEARABLE_UNIQUE			= (1 << 7),
	WEARABLE_SHIELD			= (1 << 8),
	WEARABLE_ARROW			= (1 << 9),
	WEARABLE_HAIR			= (1 << 10),
	WEARABLE_ABILITY		= (1 << 11),
	WEARABLE_COSTUME_BODY	= (1 << 12),
};

enum EAttrAddonTypes
{
	ATTR_ADDON_NONE,
	ATTR_DAMAGE_ADDON = -1,
};

enum EImmuneFlags
{
	IMMUNE_STUN = (1 << 0),
	IMMUNE_SLOW = (1 << 1),
	IMMUNE_FALL = (1 << 2),
	IMMUNE_CURSE = (1 << 3),
	IMMUNE_POISON = (1 << 4),
	IMMUNE_TERROR = (1 << 5),
	IMMUNE_REFLECT = (1 << 6)
};

enum ERefineType
{
	REFINE_TYPE_NORMAL,
	REFINE_TYPE_NOT_USED1,
	REFINE_TYPE_SCROLL,
	REFINE_TYPE_HYUNIRON,
	REFINE_TYPE_MONEY_ONLY,
	REFINE_TYPE_MUSIN,
	REFINE_TYPE_BDRAGON,
};

enum EDragonSoulDeckType
{
	DRAGON_SOUL_DECK_0,
	DRAGON_SOUL_DECK_1,
	DRAGON_SOUL_DECK_MAX_NUM = 2,

	DRAGON_SOUL_DECK_RESERVED_MAX_NUM = 3,
};

enum EItemTypes
{
	ITEM_NONE,              //0
	ITEM_WEAPON,            //1//¹«±â
	ITEM_ARMOR,             //2//°©¿Ê
	ITEM_USE,               //3//¾ÆÀÌÅÛ »ç¿ë
	ITEM_AUTOUSE,           //4
	ITEM_MATERIAL,          //5
	ITEM_SPECIAL,           //6 //½ºÆä¼È ¾ÆÀÌÅÛ
	ITEM_TOOL,              //7
	ITEM_LOTTERY,           //8//º¹±Ç
	ITEM_ELK,               //9//µ·
	ITEM_METIN,             //10
	ITEM_CONTAINER,         //11
	ITEM_FISH,              //12//³¬½Ã
	ITEM_ROD,               //13
	ITEM_RESOURCE,          //14
	ITEM_CAMPFIRE,          //15
	ITEM_UNIQUE,            //16
	ITEM_SKILLBOOK,         //17
	ITEM_QUEST,             //18
	ITEM_POLYMORPH,         //19
	ITEM_TREASURE_BOX,      //20//º¸¹°»óÀÚ
	ITEM_TREASURE_KEY,      //21//º¸¹°»óÀÚ ¿­¼è
	ITEM_SKILLFORGET,       //22
	ITEM_GIFTBOX,           //23
	ITEM_PICK,              //24
	ITEM_HAIR,              //25//¸Ó¸®
	ITEM_TOTEM,             //26//ÅäÅÛ
	ITEM_BLEND,				//27//»ı¼ºµÉ¶§ ·£´ıÇÏ°Ô ¼Ó¼ºÀÌ ºÙ´Â ¾à¹°
	ITEM_COSTUME,			//28//ÄÚ½ºÃõ ¾ÆÀÌÅÛ (2011³â 8¿ù Ãß°¡µÈ ÄÚ½ºÃõ ½Ã½ºÅÛ¿ë ¾ÆÀÌÅÛ)
	ITEM_DS,				//29 //¿ëÈ¥¼®
	ITEM_SPECIAL_DS,		//30 // Æ¯¼öÇÑ ¿ëÈ¥¼® (DS_SLOT¿¡ Âø¿ëÇÏ´Â UNIQUE ¾ÆÀÌÅÛÀÌ¶ó »ı°¢ÇÏ¸é µÊ)
	ITEM_EXTRACT,			//31 ÃßÃâµµ±¸.
	ITEM_SECONDARY_COIN_UNUSED,	//32 ?? ¸íµµÀü??
	ITEM_RING,				//33 ¹İÁö
	ITEM_BELT,				//34 º§Æ®
	ITEM_MAX_NUM
};


#ifdef ENABLE_ACCE_SYSTEM
enum EAcceInfo
{
	ACCE_GRADE_VALUE_FIELD = 0,
	ACCE_ABSORPTION_SOCKET = 0,
	ACCE_ABSORBED_SOCKET = 1,
	ACCE_GRADE_1_ABS = 1,
	ACCE_GRADE_2_ABS = 5,
	ACCE_GRADE_3_ABS = 10,
	ACCE_GRADE_4_ABS_MIN = 11,
	ACCE_GRADE_4_ABS_MAX = 25,
	ACCE_GRADE_4_ABS_MAX_COMB = 19,
	ACCE_GRADE_4_ABS_RANGE = 5,
	ACCE_EFFECT_FROM_ABS = 19,
	ACCE_CLEAN_ATTR_VALUE0 = 7,
	ACCE_WINDOW_MAX_MATERIALS = 2,
	ACCE_GRADE_1_PRICE = 100000,
	ACCE_GRADE_2_PRICE = 200000,
	ACCE_GRADE_3_PRICE = 300000,
	ACCE_GRADE_4_PRICE = 500000,
	ACCE_COMBINE_GRADE_1 = 80,
	ACCE_COMBINE_GRADE_2 = 70,
	ACCE_COMBINE_GRADE_3 = 50,
	ACCE_COMBINE_GRADE_4 = 30,
};
#endif



const uint32_t c_Inventory_Page_Column = 5;
const uint32_t c_Inventory_Page_Row = 9;
const uint32_t c_Inventory_Page_Size = c_Inventory_Page_Column*c_Inventory_Page_Row; // x*y
#ifdef ENABLE_EXTEND_INVEN_SYSTEM
const uint32_t c_Inventory_Page_Count = 4;
#else
const uint32_t c_Inventory_Page_Count = 2;
#endif
const uint32_t c_ItemSlot_Count = c_Inventory_Page_Size * c_Inventory_Page_Count;
const uint32_t c_Equipment_Count = 12;

const uint32_t c_Equipment_Start = c_ItemSlot_Count;

const uint32_t c_Equipment_Body	= c_Equipment_Start + 0;
const uint32_t c_Equipment_Head	= c_Equipment_Start + 1;
const uint32_t c_Equipment_Shoes	= c_Equipment_Start + 2;
const uint32_t c_Equipment_Wrist	= c_Equipment_Start + 3;
const uint32_t c_Equipment_Weapon	= c_Equipment_Start + 4;
const uint32_t c_Equipment_Neck	= c_Equipment_Start + 5;
const uint32_t c_Equipment_Ear		= c_Equipment_Start + 6;
const uint32_t c_Equipment_Unique1	= c_Equipment_Start + 7;
const uint32_t c_Equipment_Unique2	= c_Equipment_Start + 8;
const uint32_t c_Equipment_Arrow	= c_Equipment_Start + 9;
const uint32_t c_Equipment_Shield	= c_Equipment_Start + 10;

// »õ·Î Ãß°¡µÈ ½Å±Ô ¹İÁö & º§Æ®
// ÀåÂøÇü ¾ÆÀÌÅÛ¿¡ ÇÒ´çÇÒ ¼ö ÀÖ´Â À§Ä¡°¡ ±âÁ¸ Àåºñ, Ã¤±â¶ø Äù½ºÆ® º¸»ó, ÄÚ½ºÆ¬ ½Ã½ºÅÛ µîÀ¸·Î ÀÎÇØ¼­ °ø°£ÀÌ Àß·ÁÀÖ´Ù.
// ÀÌ°Ô ´Ù Ã¤±â¶ø º¸»ó ¹öÇÁ¸¦ ÀåÂø¾ÆÀÌÅÛÃ³·³ ±¸ÇöÇÑ ¤µ¤¢ ¶§¹®¿¡ ³­¸®³µµû... ¤¶¤²
// 
// Á¤¸®ÇÏ¸é, ±âÁ¸ ÀåºñÃ¢µéÀº ¼­¹öDB»ó ¾ÆÀÌÅÛ Æ÷Áö¼ÇÀÌ 90 ~ 102 ÀÌ°í,
// 2013³â ÃÊ¿¡ »õ·Î Ãß°¡µÇ´Â ½½·ÔµéÀº 111 ~ ºÎÅÍ ½ÃÀÛÇÑ´Ù. Âø¿ë Àåºñ¿¡¼­ ÃÖ´ë·Î »ç¿ëÇÒ ¼ö ÀÖ´Â °ªÀº 121 ±îÁöÀÌ°í, 122ºÎÅÍ´Â ¿ëÈ¥¼®¿¡¼­ »ç¿ëÇÑ´Ù.
#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	const uint32_t c_New_Equipment_Start = c_Equipment_Start + 21;
	const uint32_t c_New_Equipment_Count = 3;
	const uint32_t c_Equipment_Ring1 = c_New_Equipment_Start + 0;
	const uint32_t c_Equipment_Ring2 = c_New_Equipment_Start + 1;
	const uint32_t c_Equipment_Belt  = c_New_Equipment_Start + 2;
#endif

#ifdef ENABLE_COSTUME_SYSTEM
	const uint32_t c_Costume_Slot_Start	= c_Equipment_Start + 19;	// [ÁÖÀÇ] ¼ıÀÚ(19) ÇÏµåÄÚµù ÁÖÀÇ. ÇöÀç ¼­¹ö¿¡¼­ ÄÚ½ºÃõ ½½·ÔÀº 19ºÎÅÍÀÓ. ¼­¹ö common/length.h ÆÄÀÏÀÇ EWearPositions ¿­°ÅÇü Âü°í.
	const uint32_t	c_Costume_Slot_Body		= c_Costume_Slot_Start + 0;
	const uint32_t	c_Costume_Slot_Hair		= c_Costume_Slot_Start + 1;
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	const uint32_t	c_Costume_Slot_Mount	= c_Costume_Slot_Start + 2;
#endif
#ifdef ENABLE_ACCE_SYSTEM
	const uint32_t	c_Costume_Slot_Acce		= c_Costume_Slot_Start + 3;
#endif

#if defined(ENABLE_WEAPON_COSTUME_SYSTEM) || defined(ENABLE_ACCE_SYSTEM)
	const uint32_t c_Costume_Slot_Count	= 4;
#elif defined(ENABLE_MOUNT_COSTUME_SYSTEM)
	const uint32_t c_Costume_Slot_Count	= 3;
#else
	const uint32_t c_Costume_Slot_Count	= 2;
#endif

	const uint32_t c_Costume_Slot_End		= c_Costume_Slot_Start + c_Costume_Slot_Count;

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	const uint32_t	c_Costume_Slot_Weapon	= c_Costume_Slot_End + 1;
#endif

#endif

// [ÁÖÀÇ] ¼ıÀÚ(32) ÇÏµåÄÚµù ÁÖÀÇ. ÇöÀç ¼­¹ö¿¡¼­ ¿ëÈ¥¼® ½½·ÔÀº 32ºÎÅÍÀÓ. 
// ¼­¹ö common/length.h ÆÄÀÏÀÇ EWearPositions ¿­°ÅÇüÀÌ 32±îÁö È®ÀåµÉ °ÍÀ» ¿°µÎÇÏ°í(32 ÀÌ»óÀº È®Àå ÇÏ±â Èûµé°Ô µÇ¾îÀÖÀ½.), 
// ±× ÀÌÈÄºÎÅÍ¸¦ ¿ëÈ¥¼® ÀåÂø ½½·ÔÀ¸·Î »ç¿ë.
const uint32_t c_Wear_Max = 32;
const uint32_t c_DragonSoul_Equip_Start = c_ItemSlot_Count + c_Wear_Max;
const uint32_t c_DragonSoul_Equip_Slot_Max = 6;
const uint32_t c_DragonSoul_Equip_End = c_DragonSoul_Equip_Start + c_DragonSoul_Equip_Slot_Max * DRAGON_SOUL_DECK_MAX_NUM;

// NOTE: 2013³â 2¿ù 5ÀÏ ÇöÀç... ¿ëÈ¥¼® µ¥Å©´Â 2°³°¡ Á¸ÀçÇÏ´Âµ¥, ÇâÈÄ È®Àå °¡´É¼ºÀÌ ÀÖ¾î¼­ 3°³ µ¥Å© ¿©À¯ºĞÀ» ÇÒ´ç ÇØ µÒ. ±× µÚ °ø°£Àº º§Æ® ÀÎº¥Åä¸®·Î »ç¿ë
const uint32_t c_DragonSoul_Equip_Reserved_Count = c_DragonSoul_Equip_Slot_Max * 3;		

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	// º§Æ® ¾ÆÀÌÅÛÀÌ Á¦°øÇÏ´Â ÀÎº¥Åä¸®
	const uint32_t c_Belt_Inventory_Slot_Start = c_DragonSoul_Equip_End + c_DragonSoul_Equip_Reserved_Count;
	const uint32_t c_Belt_Inventory_Width = 4;
	const uint32_t c_Belt_Inventory_Height= 4;
	const uint32_t c_Belt_Inventory_Slot_Count = c_Belt_Inventory_Width * c_Belt_Inventory_Height;
	const uint32_t c_Belt_Inventory_Slot_End = c_Belt_Inventory_Slot_Start + c_Belt_Inventory_Slot_Count;

	const uint32_t c_Inventory_Count	= c_Belt_Inventory_Slot_End;
#else
	const uint32_t c_Inventory_Count	= c_DragonSoul_Equip_End;
#endif

// ¿ëÈ¥¼® Àü¿ë ÀÎº¥Åä¸®
const uint32_t c_DragonSoul_Inventory_Start = 0;
const uint32_t c_DragonSoul_Inventory_Box_Size = 32;
const uint32_t c_DragonSoul_Inventory_Count = DS_SLOT_MAX * DRAGON_SOUL_GRADE_MAX * c_DragonSoul_Inventory_Box_Size;
const uint32_t c_DragonSoul_Inventory_End = c_DragonSoul_Inventory_Start + c_DragonSoul_Inventory_Count;


enum EDSInventoryMaxNum
{
	DRAGON_SOUL_INVENTORY_MAX_NUM = DS_SLOT_MAX * DRAGON_SOUL_GRADE_MAX * DRAGON_SOUL_BOX_SIZE,
	DS_REFINE_WINDOW_MAX_NUM = 15,
};

enum EDragonSoulRefineWindowSize
{
	DRAGON_SOUL_REFINE_GRID_MAX = 15,
};

enum EMisc2
{
	DRAGON_SOUL_EQUIP_SLOT_START = INVENTORY_MAX_NUM + WEAR_MAX_NUM,
	DRAGON_SOUL_EQUIP_SLOT_END = DRAGON_SOUL_EQUIP_SLOT_START + (DS_SLOT_MAX * DRAGON_SOUL_DECK_MAX_NUM),
	DRAGON_SOUL_EQUIP_RESERVED_SLOT_END = DRAGON_SOUL_EQUIP_SLOT_END + (DS_SLOT_MAX * DRAGON_SOUL_DECK_RESERVED_MAX_NUM),

	BELT_INVENTORY_SLOT_START = DRAGON_SOUL_EQUIP_RESERVED_SLOT_END,
	BELT_INVENTORY_SLOT_END = BELT_INVENTORY_SLOT_START + BELT_INVENTORY_SLOT_COUNT,

	INVENTORY_AND_EQUIP_SLOT_MAX = BELT_INVENTORY_SLOT_END,
};

#pragma pack (push, 1)
typedef struct SItemPos
{
	uint8_t window_type;
	uint16_t cell;

	SItemPos()
	{
		window_type = INVENTORY;
		cell = WORD_MAX;
	}
	SItemPos(uint8_t _window_type, uint16_t _cell)
	{
		window_type = _window_type;
		cell = _cell;
	}

	bool IsValidCell() const
	{
		switch (window_type)
		{
		case INVENTORY:
			return cell < c_Inventory_Count;

		case EQUIPMENT:
			return cell < c_DragonSoul_Equip_End;

		case DRAGON_SOUL_INVENTORY:
			return cell < (DRAGON_SOUL_INVENTORY_MAX_NUM);
			break;
		default:
			return false;
		}
	}

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	bool IsBeltInventoryCell()
	{
		bool bResult = c_Belt_Inventory_Slot_Start <= cell && c_Belt_Inventory_Slot_End > cell;
		return bResult;
	}
#endif

	bool IsEquipPosition() const
	{
		return (INVENTORY == window_type || EQUIPMENT == window_type) && ((cell >= INVENTORY_MAX_NUM && cell < INVENTORY_MAX_NUM + WEAR_MAX_NUM) || IsDragonSoulEquipPosition());
	}

	bool IsDragonSoulEquipPosition() const
	{
		return (DRAGON_SOUL_EQUIP_SLOT_START <= cell) && (DRAGON_SOUL_EQUIP_SLOT_END > cell);
	}

	bool IsBeltInventoryPosition() const
	{
		return (BELT_INVENTORY_SLOT_START <= cell) && (BELT_INVENTORY_SLOT_END > cell);
	}

	bool IsDefaultInventoryPosition() const
	{
		return INVENTORY == window_type && cell < INVENTORY_MAX_NUM;
	}

	bool operator==(const struct SItemPos& rhs) const
	{
		return (window_type == rhs.window_type) && (cell == rhs.cell);
	}
	bool operator!=(const struct SItemPos& rhs) const
	{
		return (window_type != rhs.window_type) || (cell != rhs.cell);
	}
	bool operator<(const struct SItemPos& rhs) const
	{
		return (window_type < rhs.window_type) || ((window_type == rhs.window_type) && (cell < rhs.cell));
	}

} TItemPos;

const SItemPos NPOS(RESERVED_WINDOW, WORD_MAX);

#pragma pack(pop)
