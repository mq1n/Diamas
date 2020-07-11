#pragma once
#include "../../../common/packets.h"

#include "../../common/length.h"
#include "../../common/tables.h"
#include <cstdint>

#if 0

/* 로그인 (1) */
typedef struct command_handshake
{
	uint8_t	bHeader;
	uint32_t	dwHandshake;
	uint32_t	dwTime;
	int32_t	lDelta;
} TPacketCGHandshake;

typedef struct command_login
{
	uint8_t	header;
	char	login[LOGIN_MAX_LEN + 1];
	char	passwd[PASSWD_MAX_LEN + 1];
} TPacketCGLogin;

typedef struct command_login2
{
	uint8_t	header;
	char	login[LOGIN_MAX_LEN + 1];
	uint32_t	dwLoginKey;
	uint32_t	adwClientKey[4];
} TPacketCGLogin2;

typedef struct command_login3
{
	uint8_t	header;
	char	login[LOGIN_MAX_LEN + 1];
	char	passwd[PASSWD_MAX_LEN + 1];
	uint32_t	adwClientKey[4];
	uint32_t	version;
} TPacketCGLogin3;

typedef struct packet_login_key
{
	uint8_t	bHeader;
	uint32_t	dwLoginKey;
} TPacketGCLoginKey;

typedef struct command_player_select
{
	uint8_t	header;
	uint8_t	index;
} TPacketCGPlayerSelect;

typedef struct command_player_delete
{
	uint8_t	header;
	uint8_t	index;
	char	private_code[8];
} TPacketCGPlayerDelete;

typedef struct command_player_create
{
	uint8_t        header;
	uint8_t        index;
	char        name[CHARACTER_NAME_MAX_LEN + 1];
	uint16_t        job;
	uint8_t	shape;
	uint8_t	Con;
	uint8_t	Int;
	uint8_t	Str;
	uint8_t	Dex;
} TPacketCGPlayerCreate;

typedef struct command_player_create_success
{
	uint8_t		header;
	uint8_t		bAccountCharacterIndex;
	TSimplePlayer	player;
} TPacketGCPlayerCreateSuccess;

// 공격
typedef struct command_attack
{
	uint8_t	bHeader;
	uint8_t	bType;
	uint32_t	dwVID;
} TPacketCGAttack;

enum EMoveFuncType
{
	FUNC_WAIT,
	FUNC_MOVE,
	FUNC_ATTACK,
	FUNC_COMBO,
	FUNC_MOB_SKILL,
	FUNC_MAX_NUM,
	FUNC_SKILL = 0x80,
};

// 이동
typedef struct command_move
{
	uint8_t	bHeader;
	uint8_t	bFunc;
	uint8_t	bArg;
	float	rot;
	int32_t	lX;
	int32_t	lY;
	uint32_t	dwTime;
} TPacketCGMove;

typedef struct command_sync_position_element
{
	uint32_t	dwVID;
	int32_t	lX;
	int32_t	lY;
} TPacketCGSyncPositionElement;

// 위치 동기화
typedef struct command_sync_position	// 가변 패킷
{
	uint8_t	bHeader;
	uint16_t	wSize;
} TPacketCGSyncPosition;

/* 채팅 (3) */
typedef struct command_chat	// 가변 패킷
{
	uint8_t	header;
	uint16_t	size;
	uint8_t	type;
} TPacketCGChat;

/* 귓속말 */
typedef struct command_whisper
{
	uint8_t	bHeader;
	uint16_t	wSize;
	char 	szNameTo[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGWhisper;

typedef struct command_entergame
{
	uint8_t	header;
} TPacketCGEnterGame;

typedef struct command_item_use
{
	uint8_t 	header;
	TItemPos 	Cell;
} TPacketCGItemUse;

typedef struct command_item_use_to_item
{
	uint8_t	header;
	TItemPos	Cell;
	TItemPos	TargetCell;
} TPacketCGItemUseToItem;

typedef struct command_item_drop
{
	uint8_t 	header;
	TItemPos 	Cell;
	uint32_t	gold;
} TPacketCGItemDrop;

typedef struct command_item_drop2
{
	uint8_t 	header;
	TItemPos 	Cell;
	uint32_t	gold;
	uint8_t	count;
} TPacketCGItemDrop2;

typedef struct command_item_move
{
	uint8_t 	header;
	TItemPos	Cell;
	TItemPos	CellTo;
	uint8_t	count;
} TPacketCGItemMove;

typedef struct command_item_pickup
{
	uint8_t 	header;
	uint32_t	vid;
} TPacketCGItemPickup;

typedef struct command_quickslot_swap
{
	uint8_t	header;
	uint8_t	pos;
	uint8_t	change_pos;
} TPacketCGQuickslotSwap;

enum EShopSubheaderClientGame
{
	SHOP_SUBHEADER_CG_END,
	SHOP_SUBHEADER_CG_BUY,
	SHOP_SUBHEADER_CG_SELL,
	SHOP_SUBHEADER_CG_SELL2
};

typedef struct command_shop
{
	uint8_t	header;
	uint8_t	subheader;
} TPacketCGShop;

typedef struct command_on_click
{
	uint8_t	header;
	uint32_t	vid;
} TPacketCGOnClick;

enum ExchangeSubheader
{
	EXCHANGE_SUBHEADER_CG_START,	/* arg1 == vid of target character */
	EXCHANGE_SUBHEADER_CG_ITEM_ADD,	/* arg1 == position of item */
	EXCHANGE_SUBHEADER_CG_ITEM_DEL,	/* arg1 == position of item */
	EXCHANGE_SUBHEADER_CG_ELK_ADD,	/* arg1 == amount of gold */
	EXCHANGE_SUBHEADER_CG_ACCEPT,	/* arg1 == not used */
	EXCHANGE_SUBHEADER_CG_CANCEL,	/* arg1 == not used */
};

typedef struct command_exchange
{
	uint8_t	header;
	uint8_t	sub_header;
	uint32_t	arg1;
	uint8_t	arg2;
	TItemPos	Pos;
} TPacketCGExchange;

typedef struct command_position
{
	uint8_t	header;
	uint8_t	position;
} TPacketCGPosition;

typedef struct command_script_answer
{
	uint8_t	header;
	uint8_t	answer;
	//char	file[32 + 1];
	//uint8_t	answer[16 + 1];
} TPacketCGScriptAnswer;


typedef struct command_script_button
{
	uint8_t        header;
	uint32_t	idx;
} TPacketCGScriptButton;

typedef struct command_quest_input_string
{
	uint8_t header;
	char msg[64+1];
} TPacketCGQuestInputString;

typedef struct command_quest_confirm
{
	uint8_t header;
	uint8_t answer;
	uint32_t requestPID;
} TPacketCGQuestConfirm;

/*
 * 서버 측에서 보내는 패킷 
 */
typedef struct packet_quest_confirm
{
	uint8_t header;
	char msg[64+1]; 
	int32_t timeout;
	uint32_t requestPID;
} TPacketGCQuestConfirm;

typedef struct packet_handshake
{
	uint8_t	bHeader;
	uint32_t	dwHandshake;
	uint32_t	dwTime;
	int32_t	lDelta;
} TPacketGCHandshake;

enum EPhase
{
	PHASE_CLOSE,
	PHASE_HANDSHAKE,
	PHASE_LOGIN,
	PHASE_SELECT,
	PHASE_LOADING,
	PHASE_GAME,
	PHASE_DEAD,

	PHASE_CLIENT_CONNECTING,
	PHASE_DBCLIENT,
	PHASE_P2P,
	PHASE_AUTH,
};

typedef struct packet_phase
{
	uint8_t	header;
	uint8_t	phase;
	uint8_t stage;
} TPacketGCPhase;

typedef struct packet_login_success
{
	uint8_t		bHeader;
	TSimplePlayer	players[PLAYER_PER_ACCOUNT];
	uint32_t		guild_id[PLAYER_PER_ACCOUNT];
	char		guild_name[PLAYER_PER_ACCOUNT][GUILD_NAME_MAX_LEN+1];

	uint32_t		handle;
	uint32_t		random_key;
} TPacketGCLoginSuccess;

typedef struct packet_auth_success
{
	uint8_t	bHeader;
	uint32_t	dwLoginKey;
	uint8_t	bResult;
} TPacketGCAuthSuccess;

typedef struct packet_login_failure
{
	uint8_t	header;
	char	szStatus[ACCOUNT_STATUS_MAX_LEN + 1];
} TPacketGCLoginFailure;

typedef struct packet_create_failure
{
	uint8_t	header;
	uint8_t	bType;
} TPacketGCCreateFailure;

enum
{
	ADD_CHARACTER_STATE_DEAD		= (1 << 0),
	ADD_CHARACTER_STATE_SPAWN		= (1 << 1),
	ADD_CHARACTER_STATE_GUNGON		= (1 << 2),
	ADD_CHARACTER_STATE_KILLER		= (1 << 3),
	ADD_CHARACTER_STATE_PARTY		= (1 << 4),
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

typedef struct packet_add_char
{
	uint8_t	header;
	uint32_t	dwVID;

	float	angle;
	int32_t	x;
	int32_t	y;
	int32_t	z;

	uint8_t	bType;
	uint32_t	dwRaceNum;
	uint8_t	bMovingSpeed;
	uint8_t	bAttackSpeed;

	uint8_t	bStateFlag;
	uint32_t	dwAffectFlag[2];	// 효과

	uint32_t guildID;
	uint32_t level;
} TPacketGCCharacterAdd;

typedef struct packet_char_additional_info
{
	uint8_t    header;
	uint32_t   dwVID;
	char    name[CHARACTER_NAME_MAX_LEN + 1];
	uint16_t    awPart[CHR_EQUIPPART_NUM];
	uint8_t	bEmpire;
	uint32_t   dwGuildID;
	uint32_t   dwLevel;
	int16_t	sAlignment;
	uint8_t	bPKMode;
	uint32_t	dwMountVnum;
} TPacketGCCharacterAdditionalInfo;

/*
   typedef struct packet_update_char_old
   {
   uint8_t	header;
   uint32_t	dwVID;

   uint16_t        awPart[CHR_EQUIPPART_NUM];
   uint8_t	bMovingSpeed;
   uint8_t	bAttackSpeed;

   uint8_t	bStateFlag;
   uint32_t	dwAffectFlag[2];

   uint32_t	dwGuildID;
   int16_t	sAlignment;
   uint8_t	bPKMode;
   uint32_t	dwMountVnum;
   } TPacketGCCharacterUpdateOld;
 */

typedef struct packet_update_char
{
	uint8_t	header;
	uint32_t	dwVID;

	uint16_t        awPart[CHR_EQUIPPART_NUM];
	uint8_t	bMovingSpeed;
	uint8_t	bAttackSpeed;

	uint8_t	bStateFlag;
	uint32_t	dwAffectFlag[2];

	uint32_t	dwGuildID;
	int16_t	sAlignment;
	uint8_t	bPKMode;
	uint32_t	dwMountVnum;
	//uint16_t	wRaceNum;
} TPacketGCCharacterUpdate;

typedef struct packet_del_char
{
	uint8_t	header;
	uint32_t	id;
} TPacketGCCharacterDelete;

typedef struct packet_chat	// 가변 패킷
{
	uint8_t	header;
	uint16_t	size;
	uint8_t	type;
	uint32_t	id;
	uint8_t	bEmpire;
} TPacketGCChat;

typedef struct packet_whisper	// 가변 패킷
{
	uint8_t	bHeader;
	uint16_t	wSize;
	uint8_t	bType;
	char	szNameFrom[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCWhisper;

typedef struct packet_main_character
{
	uint8_t        header;
	uint32_t	dwVID;
	uint16_t	wRaceNum;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	int32_t	lx, ly, lz;
	uint8_t	empire;
	uint8_t	skill_group;
} TPacketGCMainCharacter;

// SUPPORT_BGM
typedef struct packet_main_character3_bgm
{
	enum
	{
		MUSIC_NAME_LEN = 24,
	};

	uint8_t    header;
	uint32_t	dwVID;
	uint16_t	wRaceNum;
	char	szChrName[CHARACTER_NAME_MAX_LEN + 1];
	char	szBGMName[MUSIC_NAME_LEN + 1];
	int32_t	lx, ly, lz;
	uint8_t	empire;
	uint8_t	skill_group;
} TPacketGCMainCharacter3_BGM;

typedef struct packet_main_character4_bgm_vol
{
	enum
	{
		MUSIC_NAME_LEN = 24,
	};

	uint8_t    header;
	uint32_t	dwVID;
	uint16_t	wRaceNum;
	char	szChrName[CHARACTER_NAME_MAX_LEN + 1];
	char	szBGMName[MUSIC_NAME_LEN + 1];
	float	fBGMVol;
	int32_t	lx, ly, lz;
	uint8_t	empire;
	uint8_t	skill_group;
} TPacketGCMainCharacter4_BGM_VOL;
// END_OF_SUPPORT_BGM

typedef struct packet_points
{
	uint8_t	header;
	int32_t		points[POINT_MAX_NUM];
} TPacketGCPoints;

typedef struct packet_skill_level
{
	uint8_t		bHeader;
	TPlayerSkill	skills[SKILL_MAX_NUM];
} TPacketGCSkillLevel;

typedef struct packet_point_change
{
	int32_t		header;
	uint32_t	dwVID;
	uint8_t	type;
	int32_t	amount;
	int32_t	value;
} TPacketGCPointChange;

typedef struct packet_stun
{
	uint8_t	header;
	uint32_t	vid;
} TPacketGCStun;

typedef struct packet_dead
{
	uint8_t	header;
	uint32_t	vid;
} TPacketGCDead;

struct TPacketGCItemDelDeprecated
{
	uint8_t	header;
	TItemPos Cell;
	uint32_t	vnum;
	uint8_t	count;
	int32_t	alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
};

typedef struct packet_item_set
{
	uint8_t	header;
	TItemPos Cell;
	uint32_t	vnum;
	uint8_t	count;
	uint32_t	flags;
	uint32_t	anti_flags;
	bool	highlight;
	int32_t	alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
} TPacketGCItemSet;

typedef struct packet_item_del
{
	uint8_t	header;
	uint8_t	pos;
} TPacketGCItemDel;

struct packet_item_use
{
	uint8_t	header;
	TItemPos Cell;
	uint32_t	ch_vid;
	uint32_t	victim_vid;
	uint32_t	vnum;
};

typedef struct packet_item_update
{
	uint8_t	header;
	TItemPos Cell;
	uint8_t	count;
	int32_t	alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
} TPacketGCItemUpdate;

typedef struct packet_item_ground_add
{
	uint8_t	bHeader;
	int32_t 	x, y, z;
	uint32_t	dwVID;
	uint32_t	dwVnum;
} TPacketGCItemGroundAdd;

typedef struct packet_item_ownership
{
	uint8_t	bHeader;
	uint32_t	dwVID;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCItemOwnership;

typedef struct packet_item_ground_del
{
	uint8_t	bHeader;
	uint32_t	dwVID;
} TPacketGCItemGroundDel;


struct packet_quickslot_del
{
	uint8_t	header;
	uint8_t	pos;
};

struct packet_quickslot_swap
{
	uint8_t	header;
	uint8_t	pos;
	uint8_t	pos_to;
};

struct packet_motion
{
	uint8_t	header;
	uint32_t	vid;
	uint32_t	victim_vid;
	uint16_t	motion;
};

enum EPacketShopSubHeaders
{
	SHOP_SUBHEADER_GC_START,
	SHOP_SUBHEADER_GC_END,
	SHOP_SUBHEADER_GC_UPDATE_ITEM,
	SHOP_SUBHEADER_GC_UPDATE_PRICE,
	SHOP_SUBHEADER_GC_OK,
	SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY,
	SHOP_SUBHEADER_GC_SOLDOUT,
	SHOP_SUBHEADER_GC_INVENTORY_FULL,
	SHOP_SUBHEADER_GC_INVALID_POS,
	SHOP_SUBHEADER_GC_SOLD_OUT,
	SHOP_SUBHEADER_GC_START_EX,
	SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX,
};

struct packet_shop_item
{   
	uint32_t       vnum;
	uint32_t       price;
	uint8_t        count;
	uint8_t		display_pos;
	int32_t	alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
};

typedef struct packet_shop_start
{
	uint32_t   owner_vid;
	struct packet_shop_item	items[SHOP_HOST_ITEM_MAX_NUM];
} TPacketGCShopStart;

typedef struct packet_shop_start_ex // 다음에 TSubPacketShopTab* shop_tabs 이 따라옴.
{
	typedef struct sub_packet_shop_tab 
	{
		char name[SHOP_TAB_NAME_MAX];
		uint8_t coin_type;
		packet_shop_item items[SHOP_HOST_ITEM_MAX_NUM];
	} TSubPacketShopTab;
	uint32_t owner_vid;
	uint8_t shop_tab_count;
} TPacketGCShopStartEx;

typedef struct packet_shop_update_item
{
	uint8_t			pos;
	struct packet_shop_item	item;
} TPacketGCShopUpdateItem;

typedef struct packet_shop	// 가변 패킷
{
	uint8_t        header;
	uint16_t	size; 
	uint8_t        subheader;
} TPacketGCShop;

struct packet_exchange
{
	uint8_t	header;
	uint8_t	sub_header;
	uint8_t	is_me;
	uint32_t	arg1;	// vnum
	TItemPos	arg2;	// cell
	uint32_t	arg3;	// count
	int32_t	alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
};

enum EPacketTradeSubHeaders
{
	EXCHANGE_SUBHEADER_GC_START,	/* arg1 == vid */
	EXCHANGE_SUBHEADER_GC_ITEM_ADD,	/* arg1 == vnum  arg2 == pos  arg3 == count */
	EXCHANGE_SUBHEADER_GC_ITEM_DEL,
	EXCHANGE_SUBHEADER_GC_GOLD_ADD,	/* arg1 == gold */
	EXCHANGE_SUBHEADER_GC_ACCEPT,	/* arg1 == accept */
	EXCHANGE_SUBHEADER_GC_END,		/* arg1 == not used */
	EXCHANGE_SUBHEADER_GC_ALREADY,	/* arg1 == not used */
	EXCHANGE_SUBHEADER_GC_LESS_GOLD,	/* arg1 == not used */
};

struct packet_position
{
	uint8_t	header;
	uint32_t	vid;
	uint8_t	position;
};

typedef struct packet_ping
{
	uint8_t	header;
} TPacketGCPing;

struct packet_script
{
	uint8_t	header;
	uint16_t	size;
	uint8_t	skin;
	uint16_t	src_size;
};

typedef struct packet_move
{	
	uint8_t		bHeader;
	uint8_t		bFunc;
	uint8_t		bArg;
	float		rot;
	uint32_t		dwVID;
	int32_t		lX;
	int32_t		lY;
	uint32_t		dwTime;
	uint32_t		dwDuration;
} TPacketGCMove;

// 소유권
typedef struct packet_ownership
{
	uint8_t		bHeader;
	uint32_t		dwOwnerVID;
	uint32_t		dwVictimVID;
} TPacketGCOwnership;

// 위치 동기화
typedef struct packet_sync_position	// 가변 패킷
{
	uint8_t	bHeader;
	uint16_t	wSize;	// 개수 = (wSize - sizeof(TPacketGCSyncPosition)) / sizeof(TPacketGCSyncPositionElement) 
} TPacketGCSyncPosition;

typedef struct packet_fly
{
	uint8_t	bHeader;
	uint8_t	bType;
	uint32_t	dwStartVID;
	uint32_t	dwEndVID;
} TPacketGCCreateFly;

typedef struct command_fly_targeting
{
	uint8_t		bHeader;
	uint32_t		dwTargetVID;
	int32_t		x, y;
} TPacketCGFlyTargeting;

typedef struct packet_fly_targeting
{
	uint8_t		bHeader;
	uint32_t		dwShooterVID;
	uint32_t		dwTargetVID;
	int32_t		x, y;
} TPacketGCFlyTargeting;

typedef struct packet_shoot
{
	uint8_t		bHeader;
	uint8_t		bType;
} TPacketCGShoot;

typedef struct packet_duel_start
{
	uint8_t	header;
	uint16_t	wSize;	// uint32_t가 몇개? 개수 = (wSize - sizeof(TPacketGCPVPList)) / 4
} TPacketGCDuelStart;

enum EPVPModes
{
	PVP_MODE_NONE,
	PVP_MODE_AGREE,
	PVP_MODE_FIGHT,
	PVP_MODE_REVENGE
};

typedef struct packet_pvp
{
	uint8_t        bHeader;
	uint32_t       dwVIDSrc;
	uint32_t       dwVIDDst;
	uint8_t        bMode;	// 0 이면 끔, 1이면 켬
} TPacketGCPVP;

typedef struct command_use_skill
{
	uint8_t	bHeader;
	uint32_t	dwVnum;
	uint32_t	dwVID;
} TPacketCGUseSkill;

typedef struct command_target
{
	uint8_t	header;
	uint32_t	dwVID;
} TPacketCGTarget;

typedef struct packet_target
{
	uint8_t	header;
	uint32_t	dwVID;
	uint8_t	bHPPercent;
} TPacketGCTarget;

typedef struct packet_target_drop
{
	uint8_t			header;
	uint32_t		dwVID;
	uint8_t			size;
	int32_t			drop[60];
	int32_t			bonuses;
} TPacketGCTargetDrop;

typedef struct SPacketCGChestDropInfo 
{
	uint8_t		header;
	uint16_t	wInventoryCell;
} TPacketCGChestDropInfo;

typedef struct SChestDropInfoTable
{
	uint8_t	bPageIndex;
	uint8_t	bSlotIndex;
	uint32_t	dwItemVnum;
	uint8_t	bItemCount;
} TChestDropInfoTable;

typedef struct SPacketGCChestDropInfo 
{
	uint8_t	bHeader;
	uint16_t	wSize;
	uint32_t	dwChestVnum;
} TPacketGCChestDropInfo;

typedef struct packet_warp
{
	uint8_t	bHeader;
	int32_t	lX;
	int32_t	lY;
	int32_t	lAddr;
	uint16_t	wPort;
} TPacketGCWarp;

typedef struct command_warp
{
	uint8_t	bHeader;
} TPacketCGWarp;

struct packet_quest_info
{
	uint8_t header;
	uint16_t size;
	uint16_t index;
	uint8_t flag;
};

enum 
{
	MESSENGER_SUBHEADER_GC_LIST,
	MESSENGER_SUBHEADER_GC_LOGIN,
	MESSENGER_SUBHEADER_GC_LOGOUT,
	MESSENGER_SUBHEADER_GC_INVITE
};

typedef struct packet_messenger
{
	uint8_t header;
	uint16_t size;
	uint8_t subheader;
} TPacketGCMessenger;

typedef struct packet_messenger_guild_list
{
	uint8_t connected;
	uint8_t length;
	//char login[LOGIN_MAX_LEN+1];
} TPacketGCMessengerGuildList;

typedef struct packet_messenger_guild_login
{
	uint8_t length;
	//char login[LOGIN_MAX_LEN+1];
} TPacketGCMessengerGuildLogin;

typedef struct packet_messenger_guild_logout
{
	uint8_t length;

	//char login[LOGIN_MAX_LEN+1];
} TPacketGCMessengerGuildLogout;

typedef struct packet_messenger_list_offline
{
	uint8_t connected; // always 0
	uint8_t length;
} TPacketGCMessengerListOffline;

typedef struct packet_messenger_list_online
{
	uint8_t connected; // always 1
	uint8_t length;
} TPacketGCMessengerListOnline;

enum 
{
	MESSENGER_SUBHEADER_CG_ADD_BY_VID,
	MESSENGER_SUBHEADER_CG_ADD_BY_NAME,
	MESSENGER_SUBHEADER_CG_REMOVE,
	MESSENGER_SUBHEADER_CG_INVITE_ANSWER,
};

typedef struct command_messenger
{
	uint8_t header;
	uint8_t subheader;
} TPacketCGMessenger;

typedef struct command_messenger_add_by_vid
{
	uint32_t vid;
} TPacketCGMessengerAddByVID;

typedef struct command_messenger_add_by_name
{
	uint8_t length;
	//char login[LOGIN_MAX_LEN+1];
} TPacketCGMessengerAddByName;

typedef struct command_messenger_remove
{
	char login[LOGIN_MAX_LEN+1];
	//uint32_t account;
} TPacketCGMessengerRemove;

typedef struct command_safebox_checkout
{
	uint8_t	bHeader;
	uint8_t	bSafePos;
	TItemPos	ItemPos;
} TPacketCGSafeboxCheckout;

typedef struct command_safebox_checkin
{
	uint8_t	bHeader;
	uint8_t	bSafePos;
	TItemPos	ItemPos;
} TPacketCGSafeboxCheckin;

///////////////////////////////////////////////////////////////////////////////////
// Party

typedef struct command_party_parameter
{
	uint8_t	bHeader;
	uint8_t	bDistributeMode;
} TPacketCGPartyParameter;

typedef struct paryt_parameter
{
	uint8_t	bHeader;
	uint8_t	bDistributeMode;
} TPacketGCPartyParameter;

typedef struct packet_party_add
{
	uint8_t	header;
	uint32_t	pid;
	char	name[CHARACTER_NAME_MAX_LEN+1];
} TPacketGCPartyAdd;

typedef struct command_party_invite
{
	uint8_t	header;
	uint32_t	vid;
} TPacketCGPartyInvite;

typedef struct packet_party_invite
{
	uint8_t	header;
	uint32_t	leader_vid;
} TPacketGCPartyInvite;

typedef struct command_party_invite_answer
{
	uint8_t	header;
	uint32_t	leader_vid;
	uint8_t	accept;
} TPacketCGPartyInviteAnswer;

typedef struct packet_party_update
{
	uint8_t	header;
	uint32_t	pid;
	uint8_t	role;
	uint8_t	percent_hp;
	int16_t	affects[7];
} TPacketGCPartyUpdate;

typedef struct packet_party_remove
{
	uint8_t header;
	uint32_t pid;
} TPacketGCPartyRemove;

typedef struct packet_party_link
{
	uint8_t header;
	uint32_t pid;
	uint32_t vid;
} TPacketGCPartyLink;

typedef struct packet_party_unlink
{
	uint8_t header;
	uint32_t pid;
	uint32_t vid;
} TPacketGCPartyUnlink;

typedef struct command_party_remove
{
	uint8_t header;
	uint32_t pid;
} TPacketCGPartyRemove;

typedef struct command_party_set_state
{
	uint8_t header;
	uint32_t pid;
	uint8_t byRole;
	uint8_t flag;
} TPacketCGPartySetState;

enum EPartySkillType
{
	PARTY_SKILL_HEAL = 1,
	PARTY_SKILL_WARP = 2
};

typedef struct command_party_use_skill
{
	uint8_t header;
	uint8_t bySkillIndex;
	uint32_t vid;
} TPacketCGPartyUseSkill;

typedef struct packet_safebox_size
{
	uint8_t bHeader;
	uint8_t bSize;
} TPacketCGSafeboxSize;

typedef struct packet_safebox_wrong_password
{
	uint8_t	bHeader;
} TPacketCGSafeboxWrongPassword;

typedef struct command_empire
{
	uint8_t	bHeader;
	uint8_t	bEmpire;
} TPacketCGEmpire;

typedef struct packet_empire
{
	uint8_t	bHeader;
	uint8_t	bEmpire;
} TPacketGCEmpire;

enum
{
	SAFEBOX_MONEY_STATE_SAVE,
	SAFEBOX_MONEY_STATE_WITHDRAW,
};

typedef struct command_safebox_money
{
	uint8_t        bHeader;
	uint8_t        bState;
	int32_t	lMoney;
} TPacketCGSafeboxMoney;

typedef struct packet_safebox_money_change
{
	uint8_t	bHeader;
	int32_t	lMoney;
} TPacketGCSafeboxMoneyChange;

// Guild

enum EGuildSubheaderGameClient
{
	GUILD_SUBHEADER_GC_LOGIN,
	GUILD_SUBHEADER_GC_LOGOUT,
	GUILD_SUBHEADER_GC_LIST,
	GUILD_SUBHEADER_GC_GRADE,
	GUILD_SUBHEADER_GC_ADD,
	GUILD_SUBHEADER_GC_REMOVE,
	GUILD_SUBHEADER_GC_GRADE_NAME,
	GUILD_SUBHEADER_GC_GRADE_AUTH,
	GUILD_SUBHEADER_GC_INFO,
	GUILD_SUBHEADER_GC_COMMENTS,
	GUILD_SUBHEADER_GC_CHANGE_EXP,
	GUILD_SUBHEADER_GC_CHANGE_MEMBER_GRADE,
	GUILD_SUBHEADER_GC_SKILL_INFO,
	GUILD_SUBHEADER_GC_CHANGE_MEMBER_GENERAL,
	GUILD_SUBHEADER_GC_GUILD_INVITE,
	GUILD_SUBHEADER_GC_WAR,
	GUILD_SUBHEADER_GC_GUILD_NAME,
	GUILD_SUBHEADER_GC_GUILD_WAR_LIST,
	GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST,
	GUILD_SUBHEADER_GC_WAR_SCORE,
	GUILD_SUBHEADER_GC_MONEY_CHANGE,
};

enum GUILD_SUBHEADER_CG
{
	GUILD_SUBHEADER_CG_ADD_MEMBER,
	GUILD_SUBHEADER_CG_REMOVE_MEMBER,
	GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME,
	GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY,
	GUILD_SUBHEADER_CG_OFFER,
	GUILD_SUBHEADER_CG_POST_COMMENT,
	GUILD_SUBHEADER_CG_DELETE_COMMENT,    
	GUILD_SUBHEADER_CG_REFRESH_COMMENT,
	GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE,
	GUILD_SUBHEADER_CG_USE_SKILL,
	GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL,
	GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER,
	GUILD_SUBHEADER_CG_CHARGE_GSP,
	GUILD_SUBHEADER_CG_DEPOSIT_MONEY,
	GUILD_SUBHEADER_CG_WITHDRAW_MONEY,
};

typedef struct packet_guild
{
	uint8_t header;
	uint16_t size;
	uint8_t subheader;
} TPacketGCGuild;

typedef struct packet_guild_name_t
{
	uint8_t header;
	uint16_t size;
	uint8_t subheader;
	uint32_t	guildID;
	char	guildName[GUILD_NAME_MAX_LEN];
} TPacketGCGuildName;

typedef struct packet_guild_war
{
	uint32_t	dwGuildSelf;
	uint32_t	dwGuildOpp;
	uint8_t	bType;
	uint8_t 	bWarState;
} TPacketGCGuildWar;

typedef struct command_guild
{
	uint8_t header;
	uint8_t subheader;
} TPacketCGGuild;

typedef struct command_guild_answer_make_guild
{
	uint8_t header;
	char guild_name[GUILD_NAME_MAX_LEN+1];
} TPacketCGAnswerMakeGuild;

typedef struct command_guild_use_skill
{
	uint32_t	dwVnum;
	uint32_t	dwPID;
} TPacketCGGuildUseSkill;

// Guild Mark
typedef struct command_mark_login
{
	uint8_t    header;
	uint32_t   handle;
	uint32_t   random_key;
} TPacketCGMarkLogin;

typedef struct command_mark_upload
{
	uint8_t	header;
	uint32_t	gid;
	uint8_t	image[16*12*4];
} TPacketCGMarkUpload;

typedef struct command_mark_idxlist
{
	uint8_t	header;
} TPacketCGMarkIDXList;

typedef struct command_mark_crclist
{
	uint8_t	header;
	uint8_t	imgIdx;
	uint32_t	crclist[80];
} TPacketCGMarkCRCList;

typedef struct packet_mark_idxlist
{
	uint8_t    header;
	uint32_t	bufSize;
	uint16_t	count;
	//뒤에 size * (uint16_t + uint16_t)만큼 데이터 붙음
} TPacketGCMarkIDXList;

typedef struct packet_mark_block
{
	uint8_t	header;
	uint32_t	bufSize;
	uint8_t	imgIdx;
	uint32_t	count;
	// 뒤에 64 x 48 x 픽셀크기(4바이트) = 12288만큼 데이터 붙음
} TPacketGCMarkBlock;

typedef struct command_symbol_upload
{
	uint8_t	header;
	uint16_t	size;
	uint32_t	guild_id;
} TPacketCGGuildSymbolUpload;

typedef struct command_symbol_crc
{
	uint8_t header;
	uint32_t guild_id;
	uint32_t crc;
	uint32_t size;
} TPacketCGSymbolCRC;

typedef struct packet_symbol_data
{
	uint8_t header;
	uint16_t size;
	uint32_t guild_id;
} TPacketGCGuildSymbolData;

// Fishing

typedef struct command_fishing
{
	uint8_t header;
	uint8_t dir;
} TPacketCGFishing;

typedef struct packet_fishing
{
	uint8_t header;
	uint8_t subheader;
	uint32_t info;
	uint8_t dir;
} TPacketGCFishing;

enum FishingHeader
{
	FISHING_SUBHEADER_GC_START,
	FISHING_SUBHEADER_GC_STOP,
	FISHING_SUBHEADER_GC_REACT,
	FISHING_SUBHEADER_GC_SUCCESS,
	FISHING_SUBHEADER_GC_FAIL,
	FISHING_SUBHEADER_GC_FISH,
};

typedef struct command_give_item
{
	uint8_t byHeader;
	uint32_t dwTargetVID;
	TItemPos ItemPos;
	uint8_t byItemCount;
} TPacketCGGiveItem;

typedef struct SPacketCGHack
{
	uint8_t	bHeader;
	char	szBuf[255 + 1];
	char	szInfo[255 + 1];
} TPacketCGHack;

typedef struct packet_cheat_blacklist
{
	uint8_t	header;
	char	content[1024];
} TPacketGCCheatBlacklist;

// SubHeader - Dungeon
enum
{
	DUNGEON_SUBHEADER_GC_TIME_ATTACK_START = 0,
	DUNGEON_SUBHEADER_GC_DESTINATION_POSITION = 1,
};

typedef struct packet_dungeon
{
	uint8_t bHeader;
	uint16_t size;
	uint8_t subheader;
} TPacketGCDungeon;

typedef struct packet_dungeon_dest_position
{
	int32_t x;
	int32_t y;
} TPacketGCDungeonDestPosition;

typedef struct SPacketGCShopSign
{
	uint8_t	bHeader;
	uint32_t	dwVID;
	char	szSign[SHOP_SIGN_MAX_LEN + 1];
} TPacketGCShopSign;

typedef struct SPacketCGMyShop
{
	uint8_t	bHeader;
	char	szSign[SHOP_SIGN_MAX_LEN + 1];
	uint8_t	bCount;
} TPacketCGMyShop;

typedef struct SPacketGCTime
{
	uint8_t	bHeader;
	uint32_t	time;
} TPacketGCTime;

enum EWalkModes
{
	WALKMODE_RUN,
	WALKMODE_WALK,
};

typedef struct SPacketGCWalkMode
{
	uint8_t	header;
	uint32_t	vid;
	uint8_t	mode;
} TPacketGCWalkMode;

typedef struct SPacketGCChangeSkillGroup
{
	uint8_t        header;
	uint8_t        skill_group;
} TPacketGCChangeSkillGroup;

typedef struct SPacketCGRefine
{
	uint8_t	header;
	uint8_t	pos;
	uint8_t	type;
} TPacketCGRefine;
typedef struct SPacketGCRefineInformaion
{
	uint8_t	header;
	uint8_t	type;
	uint8_t	pos;
	uint32_t	src_vnum;
	uint32_t	result_vnum;
	uint8_t	material_count;
	int32_t		cost; // 소요 비용
	int32_t		prob; // 확률
	TRefineMaterial materials[REFINE_MATERIAL_MAX_NUM];
} TPacketGCRefineInformation;

struct TNPCPosition
{
	uint8_t bType;
	char name[CHARACTER_NAME_MAX_LEN+1];
	int32_t x;
	int32_t y;
};

typedef struct SPacketGCNPCPosition
{
	uint8_t header;
	uint16_t size;
	uint16_t count;

	// array of TNPCPosition
} TPacketGCNPCPosition;

typedef struct SPacketGCSpecialEffect
{
	uint8_t header;
	uint8_t type;
	uint32_t vid;
} TPacketGCSpecialEffect;

typedef struct SPacketCGChangeName
{
	uint8_t header;
	uint8_t index;
	char name[CHARACTER_NAME_MAX_LEN+1];
} TPacketCGChangeName;

typedef struct SPacketGCChangeName
{
	uint8_t header;
	uint32_t pid;
	char name[CHARACTER_NAME_MAX_LEN+1];
} TPacketGCChangeName;

typedef struct packet_channel
{
	uint8_t header;
	uint8_t channel;
} TPacketGCChannel;

typedef struct SEquipmentItemSet
{
	uint32_t   vnum;
	uint8_t    count;
	int32_t    alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
} TEquipmentItemSet;

typedef struct pakcet_view_equip
{
	uint8_t  header;
	uint32_t vid;
	TEquipmentItemSet equips[WEAR_MAX_NUM];
} TPacketViewEquip;

typedef struct 
{
	uint32_t	dwID;
	int32_t	x, y;
	int32_t	width, height;
	uint32_t	dwGuildID;
} TLandPacketElement;

typedef struct packet_land_list
{
	uint8_t	header;
	uint16_t	size;
} TPacketGCLandList;

typedef struct guild_land_update
{
	uint8_t header;
	uint32_t landID;
	uint32_t guildID;
} TPacketGCGuildLandUpdate;

typedef struct
{
	uint8_t	bHeader;
	int32_t	lID;
	char	szName[32+1];
	uint32_t	dwVID;
	uint8_t	bType;
} TPacketGCTargetCreate;

typedef struct
{
	uint8_t	bHeader;
	int32_t	lID;
	int32_t	lX, lY;
} TPacketGCTargetUpdate;

typedef struct
{
	uint8_t	bHeader;
	int32_t	lID;
} TPacketGCTargetDelete;

typedef struct
{
	uint8_t		bHeader;
	TPacketAffectElement elem;
} TPacketGCAffectAdd;

typedef struct
{
	uint8_t	bHeader;
	uint32_t	dwType;
	uint8_t	bApplyOn;
} TPacketGCAffectRemove;

typedef struct packet_lover_info
{
	uint8_t header;
	char name[CHARACTER_NAME_MAX_LEN + 1];
	uint8_t love_point;
} TPacketGCLoverInfo;

typedef struct packet_love_point_update
{
	uint8_t header;
	uint8_t love_point;
} TPacketGCLovePointUpdate;

// MINING
typedef struct packet_dig_motion
{
	uint8_t header;
	uint32_t vid;
	uint32_t target_vid;
	uint8_t count;
} TPacketGCDigMotion;
// END_OF_MINING

// SCRIPT_SELECT_ITEM
typedef struct command_script_select_item
{
	uint8_t header;
	uint32_t selection;
} TPacketCGScriptSelectItem;
// END_OF_SCRIPT_SELECT_ITEM

typedef struct packet_damage_info
{
	uint8_t header;
	uint32_t dwVID;
	uint8_t flag;
	int32_t damage;
} TPacketGCDamageInfo;


#ifdef _IMPROVED_PACKET_ENCRYPTION_
struct TPacketKeyAgreement
{
	static const int32_t MAX_DATA_LEN = 256;
	uint8_t bHeader;
	uint16_t wAgreedLength;
	uint16_t wDataLength;
	uint8_t data[MAX_DATA_LEN];
};

struct TPacketKeyAgreementCompleted
{
	uint8_t bHeader;
	uint8_t data[3]; // dummy (not used)
};

#endif // _IMPROVED_PACKET_ENCRYPTION_

#define MAX_EFFECT_FILE_NAME 128
typedef struct SPacketGCSpecificEffect
{
	uint8_t header;
	uint32_t vid;
	char effect_file[MAX_EFFECT_FILE_NAME];
} TPacketGCSpecificEffect;

enum EPacketCGDragonSoulSubHeaderType
{
	DS_SUB_HEADER_OPEN,
	DS_SUB_HEADER_CLOSE,
	DS_SUB_HEADER_DO_REFINE_GRADE,
	DS_SUB_HEADER_DO_REFINE_STEP,
	DS_SUB_HEADER_DO_REFINE_STRENGTH,
	DS_SUB_HEADER_REFINE_FAIL,
	DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE,
	DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL,
	DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY,
	DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL,
	DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL,
	DS_SUB_HEADER_REFINE_SUCCEED,
};
typedef struct SPacketCGDragonSoulRefine
{
	SPacketCGDragonSoulRefine() : header (HEADER_CG_DRAGON_SOUL_REFINE)
	{}
	uint8_t header;
	uint8_t bSubType;
	TItemPos ItemGrid[DRAGON_SOUL_REFINE_GRID_SIZE];
} TPacketCGDragonSoulRefine;

typedef struct SPacketGCDragonSoulRefine
{
	SPacketGCDragonSoulRefine() : header(HEADER_GC_DRAGON_SOUL_REFINE)
	{}
	uint8_t header;
	uint8_t bSubType;
	TItemPos Pos;
} TPacketGCDragonSoulRefine;

typedef struct SPacketCGStateCheck
{
	uint8_t header;
	uint32_t key;	
	uint32_t index;
} TPacketCGStateCheck;

typedef struct SPacketGCStateCheck
{
	uint8_t header;
	uint32_t key;
	uint32_t index;
	uint8_t state;
} TPacketGCStateCheck;

#ifdef ENABLE_ACCE_SYSTEM
enum
{
	HEADER_CG_ACCE = 211,
	HEADER_GC_ACCE = 215,
	ACCE_SUBHEADER_GC_OPEN = 0,
	ACCE_SUBHEADER_GC_CLOSE,
	ACCE_SUBHEADER_GC_ADDED,
	ACCE_SUBHEADER_GC_REMOVED,
	ACCE_SUBHEADER_CG_REFINED,
	ACCE_SUBHEADER_CG_CLOSE = 0,
	ACCE_SUBHEADER_CG_ADD,
	ACCE_SUBHEADER_CG_REMOVE,
	ACCE_SUBHEADER_CG_REFINE,
};

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
#endif
#endif