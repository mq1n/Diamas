#pragma once

#define CLIENT_VERSION_TIMESTAMP 1215955205
#define LSS_SECURITY_KEY	"1234abcd5678efgh"

#define _IMPROVED_PACKET_ENCRYPTION_
// #define _PACKETDUMP
//#define __PERFORMANCE_CHECK__

// #define ENABLE_ANTICHEAT
#ifdef ENABLE_ANTICHEAT
	#define ENABLE_ANTICHEAT_AGGRESSIVE_MODE false
	#define ENABLE_DEBUGGER_ALLOW_BY_ANTICHEAT
#endif
// #define PYTHON_DYNAMIC_MODULE_NAME



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
#define ENABLE_PROTOSTRUCT_AUTODETECT

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



