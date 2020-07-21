#pragma once

#define _PACKETDUMP

//#define ENABLE_ANTICHEAT
#ifdef ENABLE_ANTICHEAT
	#define ENABLE_ANTICHEAT_AGGRESSIVE_MODE false
	#define ENABLE_DEBUGGER_ALLOW_BY_ANTICHEAT
#endif

#define LSS_SECURITY_KEY	"1234abcd5678efgh"
#define CLIENT_VERSION_TIMESTAMP 1215955205
#define DISCORD_APP_CLIENT_ID	"734739162141294694"

// #define PYTHON_DYNAMIC_MODULE_NAME

//#define _IMPROVED_PACKET_ENCRYPTION_
//#define __PERFORMANCE_CHECK__



/* ---------------- COMMON -------------------*/
// ...
/* ---------------- COMMON -------------------*/

/* ---------------- CLIENT -------------------*/
// ...
/* ---------------- CLIENT -------------------*/

/* ---------------- SERVER -------------------*/
// ...
/* ---------------- SERVER -------------------*/




//////////////////////////////////////////////////////////////////////////
// ### Default Ymir Macros ###
#define ENABLE_COSTUME_SYSTEM
#define ENABLE_ENERGY_SYSTEM
#define ENABLE_DRAGON_SOUL_SYSTEM
#define ENABLE_NEW_EQUIPMENT_SYSTEM
// ### Default Ymir Macros ###
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// ### New From LocaleInc ###
#define ENABLE_CANSEEHIDDENTHING_FOR_GM

#define ENABLE_PLAYER_PER_ACCOUNT5
#define ENABLE_LEVEL_IN_TRADE
#define ENABLE_DICE_SYSTEM
//#define ENABLE_EXTEND_INVEN_SYSTEM
#define ENABLE_LVL115_ARMOR_EFFECT
#define ENABLE_SLOT_WINDOW_EX
#define ENABLE_TEXT_LEVEL_REFRESH
#define ENABLE_USE_COSTUME_ATTR

#define WJ_SHOW_MOB_INFO
#ifdef WJ_SHOW_MOB_INFO
#define ENABLE_SHOW_MOBAIFLAG
#define ENABLE_SHOW_MOBLEVEL
#endif
// ### New From LocaleInc ###
//////////////////////////////////////////////////////////////////////////

// #define ENABLE_NO_RECV_GAME_LIMIT

//////////////////////////////////////////////////////////////////////////
// ### From GameLib ###
//#define ENABLE_WOLFMAN_CHARACTER

// #define ENABLE_MAGIC_REDUCTION_SYSTEM
#define ENABLE_MOUNT_COSTUME_SYSTEM
#define ENABLE_WEAPON_COSTUME_SYSTEM
// ### From GameLib ###
//////////////////////////////////////////////////////////////////////////


/*
	###		New System Defines - Extended Version		###
*/

// if is define ENABLE_ACCE_SYSTEM the players can use shoulder sash 
// if you want to use object scaling function you must defined ENABLE_OBJ_SCALLING
#define ENABLE_ACCE_SYSTEM
#define ENABLE_OBJ_SCALLING

// if you want use SetMouseWheelScrollEvent or you want use mouse wheel to move the scrollbar
#define ENABLE_MOUSEWHEEL_EVENT

//if you want to see highlighted a new item when dropped or when exchanged
#define ENABLE_HIGHLIGHT_NEW_ITEM




/*
	###		New Debugging Defines
*/
// #define ENABLE_PRINT_RECV_PACKET_DEBUG





//---------------------------------------------------------




#define __PET_SYSTEM__

#define ENABLE_PROTO_FROM_DB

//////////////////////////////////////////////////////////////////////////
// ### General Features ###
//#define ENABLE_QUEST_CATEGORY
#define ENABLE_D_NJGUILD
#define ENABLE_FULL_NOTICE
#define ENABLE_NEWSTUFF
#define ENABLE_PORT_SECURITY
#define ENABLE_BELT_INVENTORY_EX
#define ENABLE_CMD_WARP_IN_DUNGEON
// #define ENABLE_ITEM_ATTR_COSTUME

enum ECommonDefines
{
	EVENT_MOB_RATE_LIMIT = 1000, // 1000 default

	MAP_ALLOW_LIMIT = 32, // 32 default

	PRIV_EMPIRE_RATE_LIMIT = 200, // 200 default
	PRIV_EMPIRE_TIME_LIMIT = 60 * 60 * 24 * 7, // 1 week default

	PRIV_GUILD_RATE_LIMIT = 50, // 50 default
	PRIV_GUILD_TIME_LIMIT = 60 * 60 * 24 * 7, // 1 week default

	PRIV_PLAYER_RATE_LIMIT = 100, // 100 default
};
// ### General Features ###
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// ### CommonDefines Systems ###
#define ENABLE_WOLFMAN_CHARACTER
#ifdef ENABLE_WOLFMAN_CHARACTER
#define USE_MOB_BLEEDING_AS_POISON
#define USE_MOB_CLAW_AS_DAGGER
// #define USE_ITEM_BLEEDING_AS_POISON
// #define USE_ITEM_CLAW_AS_DAGGER
#define USE_WOLFMAN_STONES
#define USE_WOLFMAN_BOOKS
#endif

#define ENABLE_PLAYER_PER_ACCOUNT5
#define ENABLE_DICE_SYSTEM
//#define ENABLE_EXTEND_INVEN_SYSTEM

#define ENABLE_MOUNT_COSTUME_SYSTEM
#define ENABLE_WEAPON_COSTUME_SYSTEM

#define USE_MAGIC_REDUCTION_STONES


/*
	###		New Defines Extended Version		###
*/
// todo : coding GLOBAL GIFT SYSTEM
// if ENABLE_GLOBAL_GIFT is defined, the GMs can use the quest global_gift_management.quest to set a global gift
//#define ENABLE_GLOBAL_GIFT


#define DISABLE_STOP_RIDING_WHEN_DIE //	if DISABLE_TOP_RIDING_WHEN_DIE is defined , the player does not lose the horse after his death
#define ENABLE_ACCE_SYSTEM //fixed version
#define ENABLE_HIGHLIGHT_NEW_ITEM //if you want to see highlighted a new item when dropped or when exchanged
#define __ENABLE_KILL_EVENT_FIX__ //if you want to fix the 0 exp problem about the when kill lua event (recommended)




#if 0


enum ECommonDefines
{
	EVENT_MOB_RATE_LIMIT = 1000, // 1000 default

	MAP_ALLOW_LIMIT = 32, // 32 default

	PRIV_EMPIRE_RATE_LIMIT = 200, // 200 default
	PRIV_EMPIRE_TIME_LIMIT = 60 * 60 * 24 * 7, // 1 week default

	PRIV_GUILD_RATE_LIMIT = 50, // 50 default
	PRIV_GUILD_TIME_LIMIT = 60 * 60 * 24 * 7, // 1 week default

	PRIV_PLAYER_RATE_LIMIT = 100, // 100 default
};

#define USE_STACKTRACE
// #define DEBUG_ALLOC
// #define USE_DEBUG_PTR
// #define M2_USE_POOL
// #define RSA_NET_PACKET_ENCRYPTION
#ifdef RSA_NET_PACKET_ENCRYPTION
#define RSA_NET_PACKET_ENCRYPTION_AES_IV "000102030405060708090A0B0C0D0E0F"
#endif

// #define __OFFLINE_SHOP__
/* For the disable any option change 1 value as 0 */
#ifdef __OFFLINE_SHOP__
#define __OFFLINE_SHOP_ALLOW_COIN__		1
#define __OFFLINE_SHOP_ALLOW_GOLDBAR__	1
#define __OFFLINE_SHOP_ALLOW_WON__		1
#endif

// #define NET_PACKET_SEQUENCE

// #define ENABLE_JSON_GAME_FILES

#define ENABLE_PROTO_FROM_DB
#define ENABLE_EXPTABLE_FROMDB

// #define ADMIN_MANAGER

// #define ENABLE_WOLFMAN_CHARACTER
#ifdef ENABLE_WOLFMAN_CHARACTER
#define USE_WOLFMAN_STONES
#define USE_WOLFMAN_BOOKS
#endif

// #define ENABLE_DICE_SYSTEM
// #define ENABLE_MOUNT_COSTUME_SYSTEM // marty
// #define ENABLE_WEAPON_COSTUME_SYSTEM // marty

// #define ENABLE_AURA_SYSTEM


#endif






/*
	***  Defines To DEBUG  ***
*/
// #define ENABLE_SYSLOG_PACKET_SENT

