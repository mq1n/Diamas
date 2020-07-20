#pragma once
#include <cstdint>
#include "packet_def.h"
#include "defines.h"
#include "tables.h"
#include "constants.h"
#include "item_data.h"

// Common
// =========================================================================================================================
enum ECommonHeaders
{
	HEADER_KEY_AGREEMENT	= 254,
	HEADER_HANDSHAKE		= 255
};
// =========================================================================================================================

// CG
// =========================================================================================================================
enum EClientToGameHeaders : TPacketHeader
{
	HEADER_CG_LOGIN = 1,
	HEADER_CG_ATTACK = 2,
	HEADER_CG_CHAT = 3,
	HEADER_CG_PLAYER_CREATE = 4,
	HEADER_CG_PLAYER_DESTROY = 5,
	HEADER_CG_PLAYER_SELECT = 6,
	HEADER_CG_CHARACTER_MOVE = 7,
	HEADER_CG_SYNC_POSITION = 8,
	HEADER_CG_ENTERGAME = 9,
	HEADER_CG_ITEM_USE = 10,

	HEADER_CG_ITEM_DROP = 11,
	HEADER_CG_ITEM_MOVE = 12,
	HEADER_CG_ITEM_PICKUP = 13,
	HEADER_CG_QUICKSLOT_ADD = 14,
	HEADER_CG_QUICKSLOT_DEL = 15,
	HEADER_CG_QUICKSLOT_SWAP = 16,
	HEADER_CG_WHISPER = 17,
	HEADER_CG_ITEM_DROP2 = 18,
	HEADER_CG_ON_CLICK = 19,
	HEADER_CG_EXCHANGE = 20,

	HEADER_CG_CHARACTER_POSITION = 21,
	HEADER_CG_SCRIPT_ANSWER = 22,
	HEADER_CG_QUEST_INPUT_STRING = 23,
	HEADER_CG_QUEST_CONFIRM = 24,
	HEADER_CG_SHOP = 25,
	HEADER_CG_FLY_TARGETING = 26,
	HEADER_CG_USE_SKILL = 27,
	HEADER_CG_ADD_FLY_TARGETING = 28,
	HEADER_CG_SHOOT = 29,
	HEADER_CG_MYSHOP = 30,

	HEADER_CG_ITEM_USE_TO_ITEM = 31,
	HEADER_CG_TARGET = 32,
	HEADER_CG_TARGET_DROP = 33,
	HEADER_CG_CHEST_DROP_INFO = 34,
	HEADER_CG_SCRIPT_BUTTON = 35,
	HEADER_CG_MESSENGER = 36,
	HEADER_CG_MALL_CHECKOUT = 37,
	HEADER_CG_SAFEBOX_CHECKIN = 38,
	HEADER_CG_SAFEBOX_CHECKOUT = 39,
	HEADER_CG_PARTY_INVITE = 40,

	HEADER_CG_PARTY_INVITE_ANSWER = 41,
	HEADER_CG_PARTY_REMOVE = 42,
	HEADER_CG_PARTY_SET_STATE = 43,
	HEADER_CG_PARTY_USE_SKILL = 44,
	HEADER_CG_SAFEBOX_ITEM_MOVE = 45,
	HEADER_CG_PARTY_PARAMETER = 46,
	HEADER_CG_GUILD = 47,
	HEADER_CG_ANSWER_MAKE_GUILD = 48,
	HEADER_CG_FISHING = 49,
	HEADER_CG_GIVE_ITEM = 50,

	HEADER_CG_EMPIRE = 51,
	HEADER_CG_REFINE = 52,
	HEADER_CG_MARK_LOGIN = 53,
	HEADER_CG_MARK_CRCLIST = 54,
	HEADER_CG_MARK_UPLOAD = 55,
	HEADER_CG_MARK_IDXLIST = 56,
	HEADER_CG_HACK = 57,
	HEADER_CG_CHANGE_NAME = 58,
	HEADER_CG_LOGIN2 = 59,
	HEADER_CG_LOGIN3 = 60,

	HEADER_CG_GUILD_SYMBOL_UPLOAD = 61,
	HEADER_CG_GUILD_SYMBOL_CRC = 62,
	HEADER_CG_SCRIPT_SELECT_ITEM = 63,
	HEADER_CG_DRAGON_SOUL_REFINE = 64,
	HEADER_CG_STATE_CHECKER = 65,
	HEADER_CG_ACCE = 66,

	HEADER_CG_PONG = 252,
	HEADER_CG_TIME_SYNC = 253,
	HEADER_CG_KEY_AGREEMENT = HEADER_KEY_AGREEMENT, // 254
	HEADER_CG_HANDSHAKE = HEADER_HANDSHAKE // 255
};
// =========================================================================================================================

// GC
// =========================================================================================================================
enum EGameToClientHeaders : TPacketHeader
{
	HEADER_GC_CHARACTER_ADD = 1,
	HEADER_GC_CHARACTER_DEL = 2,
	HEADER_GC_CHARACTER_MOVE = 3,
	HEADER_GC_CHAT = 4,
	HEADER_GC_SYNC_POSITION = 5,
	HEADER_GC_LOGIN_SUCCESS = 6,
	HEADER_GC_LOGIN_FAILURE = 7,
	HEADER_GC_PLAYER_CREATE_SUCCESS = 8,
	HEADER_GC_PLAYER_CREATE_FAILURE = 9,
	HEADER_GC_PLAYER_DELETE_SUCCESS = 10,

	HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID = 11,
	HEADER_GC_STUN = 12,
	HEADER_GC_DEAD = 13,
	HEADER_GC_PLAYER_POINTS = 14,
	HEADER_GC_PLAYER_POINT_CHANGE = 15,
	HEADER_GC_CHANGE_SPEED = 16,
	HEADER_GC_CHARACTER_UPDATE = 17,
	HEADER_GC_ITEM_SET = 18, // DEL
	HEADER_GC_ITEM_SET2 = 19, // SET_EXTENDED
	HEADER_GC_ITEM_USE = 20, 

	// 21 = NOT USED
	HEADER_GC_ITEM_UPDATE = 22, 
	HEADER_GC_ITEM_GROUND_ADD = 23, 
	HEADER_GC_ITEM_GROUND_DEL = 24, 
	HEADER_GC_QUICKSLOT_ADD = 25,
	HEADER_GC_QUICKSLOT_DEL = 26,
	HEADER_GC_QUICKSLOT_SWAP = 27,
	HEADER_GC_ITEM_OWNERSHIP = 28,
	HEADER_GC_WHISPER = 29,
	HEADER_GC_MOTION = 30,

	HEADER_GC_SHOP = 31,
	HEADER_GC_SHOP_SIGN = 32,
	HEADER_GC_DUEL_START = 33,
	HEADER_GC_PVP = 34,
	HEADER_GC_EXCHANGE = 35,
	HEADER_GC_CHARACTER_POSITION = 36,
	HEADER_GC_PING = 37,
	HEADER_GC_SCRIPT = 38,
	HEADER_GC_QUEST_CONFIRM = 39,
	HEADER_GC_OWNERSHIP = 40,

	HEADER_GC_TARGET = 41,
	HEADER_GC_WARP = 42,
	HEADER_GC_ADD_FLY_TARGETING = 43,
	HEADER_GC_CREATE_FLY = 44,
	HEADER_GC_FLY_TARGETING = 45,
	HEADER_GC_MESSENGER = 46,
	HEADER_GC_GUILD = 47,
	HEADER_GC_PARTY_INVITE = 48,
	HEADER_GC_PARTY_ADD = 49,
	HEADER_GC_PARTY_UPDATE = 50,

	HEADER_GC_PARTY_REMOVE = 51,
	HEADER_GC_QUEST_INFO = 52,
	HEADER_GC_REQUEST_MAKE_GUILD = 53,
	HEADER_GC_PARTY_PARAMETER = 54,
	HEADER_GC_SAFEBOX_SET = 55,
	HEADER_GC_SAFEBOX_DEL = 56,
	HEADER_GC_SAFEBOX_WRONG_PASSWORD = 57,
	HEADER_GC_SAFEBOX_SIZE = 58,
	HEADER_GC_FISHING = 59,
	HEADER_GC_EMPIRE = 60,

	HEADER_GC_PARTY_LINK = 61,
	HEADER_GC_PARTY_UNLINK = 62,
	HEADER_GC_VIEW_EQUIP = 63,
	HEADER_GC_MARK_BLOCK = 64,
	HEADER_GC_MARK_IDXLIST = 65,
	HEADER_GC_TIME = 66,
	HEADER_GC_CHANGE_NAME = 67,
	HEADER_GC_DUNGEON = 68,
	HEADER_GC_WALK_MODE = 69,
	HEADER_GC_CHANGE_SKILL_GROUP = 70,

	HEADER_GC_MAIN_CHARACTER = 71,
	HEADER_GC_SPECIAL_EFFECT = 72,
	HEADER_GC_NPC_POSITION = 73,
	HEADER_GC_LOGIN_KEY = 74,
	HEADER_GC_REFINE_INFORMATION = 75,
	HEADER_GC_CHANNEL = 76,
	HEADER_GC_TARGET_UPDATE = 77,
	HEADER_GC_TARGET_DELETE = 78,
	HEADER_GC_TARGET_CREATE = 79,
	HEADER_GC_AFFECT_ADD = 80,

	HEADER_GC_AFFECT_REMOVE = 81,
	HEADER_GC_MALL_SET = 82,
	HEADER_GC_MALL_DEL = 83,
	HEADER_GC_LAND_LIST = 84,
	HEADER_GC_LOVER_INFO = 85,
	HEADER_GC_LOVE_POINT_UPDATE = 86,
	HEADER_GC_GUILD_SYMBOL_DATA = 87,
	HEADER_GC_DIG_MOTION = 88,
	HEADER_GC_DAMAGE_INFO = 89,
	HEADER_GC_CHAR_ADDITIONAL_INFO = 90,

	HEADER_GC_MAIN_CHARACTER3_BGM = 91,
	HEADER_GC_MAIN_CHARACTER4_BGM_VOL = 92,
	HEADER_GC_AUTH_SUCCESS = 93,
	HEADER_GC_SPECIFIC_EFFECT = 94,
	HEADER_GC_DRAGON_SOUL_REFINE = 95,
	HEADER_GC_RESPOND_CHANNELSTATUS = 96,
	HEADER_GC_CHEAT_BLACKLIST = 97,
	HEADER_GC_TARGET_DROP = 98,
	HEADER_GC_CHEST_DROP_INFO = 99,
	HEADER_GC_MALL_OPEN = 100,

	HEADER_GC_SKILL_LEVEL = 101,
	HEADER_GC_ACCE = 102,
	HEADER_GC_GUILD_DCINFO = 103,

	HEADER_GC_PHASE = 251,
	HEADER_GC_KEY_AGREEMENT_COMPLETED = 252,
	HEADER_GC_HANDSHAKE_OK = 253,
	HEADER_GC_KEY_AGREEMENT = HEADER_KEY_AGREEMENT,
	HEADER_GC_HANDSHAKE = HEADER_HANDSHAKE,
};
// =========================================================================================================================

// GG
// =========================================================================================================================
enum EGameToGameHeaders
{
	HEADER_GG_LOGIN = 1,
	HEADER_GG_LOGOUT = 2,
	HEADER_GG_RELAY = 3,
	HEADER_GG_NOTICE = 4,
	HEADER_GG_SHUTDOWN = 5,

	HEADER_GG_GUILD = 6,
	HEADER_GG_DISCONNECT = 7,
	HEADER_GG_SHOUT = 8,
	HEADER_GG_SETUP = 9,
	HEADER_GG_MESSENGER_ADD = 10,

	HEADER_GG_MESSENGER_REMOVE = 11,
	HEADER_GG_FIND_POSITION = 12,
	HEADER_GG_WARP_CHARACTER = 13,
	HEADER_GG_GUILD_WAR_ZONE_MAP_INDEX = 14,
	HEADER_GG_TRANSFER = 15,

	HEADER_GG_XMAS_WARP_SANTA = 16,
	HEADER_GG_XMAS_WARP_SANTA_REPLY = 17,
	HEADER_GG_LOGIN_PING = 18,
	HEADER_GG_BLOCK_CHAT = 19,
	HEADER_GG_CHECK_AWAKENESS = 20,

	HEADER_GG_BIG_NOTICE = 21,
	HEADER_GG_UPDATE_RIGHTS = 22,
	HEADER_GG_RELOAD_COMMAND = 23,
	HEADER_GG_MESSENGER_REQUEST = 24,
	HEADER_GG_MESSENGER_REQUEST_FAIL = 25,
};
// =========================================================================================================================

// GD
// =========================================================================================================================
enum EGameToDbHeaders
{
	HEADER_GD_LOGIN = 1,
	HEADER_GD_LOGOUT = 2,
	HEADER_GD_PLAYER_LOAD = 3,
	HEADER_GD_PLAYER_SAVE = 4,
	HEADER_GD_PLAYER_CREATE = 5,
	HEADER_GD_PLAYER_DELETE = 6,
	HEADER_GD_LOGIN_KEY = 7,
	HEADER_GD_BOOT = 9,
	HEADER_GD_QUEST_SAVE = 11,
	HEADER_GD_SAFEBOX_LOAD = 12,
	HEADER_GD_SAFEBOX_SAVE = 13,
	HEADER_GD_SAFEBOX_CHANGE_SIZE = 14,
	HEADER_GD_EMPIRE_SELECT = 15,

	HEADER_GD_SAFEBOX_CHANGE_PASSWORD = 16,
	HEADER_GD_SAFEBOX_CHANGE_PASSWORD_SECOND = 17, // Not really a packet, used internal
	HEADER_GD_DIRECT_ENTER = 18,

	HEADER_GD_GUILD_SKILL_UPDATE = 19,
	HEADER_GD_GUILD_EXP_UPDATE = 20,
	HEADER_GD_GUILD_ADD_MEMBER = 21,
	HEADER_GD_GUILD_REMOVE_MEMBER = 22,
	HEADER_GD_GUILD_CHANGE_GRADE = 23,
	HEADER_GD_GUILD_CHANGE_MEMBER_DATA = 24,
	HEADER_GD_GUILD_DISBAND = 25,
	HEADER_GD_GUILD_WAR = 26,
	HEADER_GD_GUILD_WAR_SCORE = 27,
	HEADER_GD_GUILD_CREATE = 28,

	HEADER_GD_ITEM_SAVE = 30,
	HEADER_GD_ITEM_DESTROY = 31,

	HEADER_GD_ADD_AFFECT = 32,
	HEADER_GD_REMOVE_AFFECT = 33,

	HEADER_GD_ITEM_FLUSH = 35,

	HEADER_GD_PARTY_CREATE = 36,
	HEADER_GD_PARTY_DELETE = 37,
	HEADER_GD_PARTY_ADD = 38,
	HEADER_GD_PARTY_REMOVE = 39,
	HEADER_GD_PARTY_STATE_CHANGE = 40,
	HEADER_GD_PARTY_HEAL_USE = 41,

	HEADER_GD_FLUSH_CACHE = 42,
	HEADER_GD_RELOAD_PROTO = 43,

	HEADER_GD_CHANGE_NAME = 44,

	HEADER_GD_GUILD_CHANGE_LADDER_POINT = 46,
	HEADER_GD_GUILD_USE_SKILL = 47,

	HEADER_GD_REQUEST_EMPIRE_PRIV = 48,
	HEADER_GD_REQUEST_GUILD_PRIV = 49,

	HEADER_GD_GUILD_DEPOSIT_MONEY = 51,
	HEADER_GD_GUILD_WITHDRAW_MONEY = 52,
	HEADER_GD_GUILD_WITHDRAW_MONEY_GIVE_REPLY = 53,

	HEADER_GD_REQUEST_CHARACTER_PRIV = 54,

	HEADER_GD_SET_EVENT_FLAG = 55,

	HEADER_GD_PARTY_SET_MEMBER_LEVEL = 56,

	HEADER_GD_GUILD_WAR_BET = 57,

	HEADER_GD_CREATE_OBJECT = 60,
	HEADER_GD_DELETE_OBJECT = 61,

	HEADER_GD_MARRIAGE_ADD = 70,
	HEADER_GD_MARRIAGE_UPDATE = 71,
	HEADER_GD_MARRIAGE_REMOVE = 72,

	HEADER_GD_WEDDING_REQUEST = 73,
	HEADER_GD_WEDDING_READY = 74,
	HEADER_GD_WEDDING_END = 75,

	HEADER_GD_SAVE_ACTIVITY = 76,

	HEADER_GD_AUTH_LOGIN = 100,
	HEADER_GD_LOGIN_BY_KEY = 101,
	HEADER_GD_MALL_LOAD = 107,

	HEADER_GD_MYSHOP_PRICELIST_UPDATE = 108,		///< 가격정보 갱신 요청
	HEADER_GD_MYSHOP_PRICELIST_REQ = 109,		///< 가격정보 리스트 요청

	HEADER_GD_BLOCK_CHAT = 110,

	HEADER_GD_RELOAD_ADMIN = 115,			///<운영자 정보 요청
	HEADER_GD_BREAK_MARRIAGE = 116,			///< 결혼 파기

	HEADER_GD_REQ_CHANGE_GUILD_MASTER = 129,

	HEADER_GD_REQ_SPARE_ITEM_ID_RANGE = 130,

	HEADER_GD_DC = 133,		// Login Key를 지움

	HEADER_GD_VALID_LOGOUT = 134,

	HEADER_GD_REQUEST_CHARGE_CASH = 137,

	HEADER_GD_DELETE_AWARDID = 138,	// delete gift notify icon

	HEADER_GD_UPDATE_CHANNELSTATUS = 139,
	HEADER_GD_REQUEST_CHANNELSTATUS = 140,

	HEADER_GD_SETUP = 0xff,
};
// =========================================================================================================================

// DG
// =========================================================================================================================

enum EDbToGameHeaders
{
	///////////////////////////////////////////////
	HEADER_DG_NOTICE = 1,

	HEADER_DG_LOGIN_SUCCESS = 30,
	HEADER_DG_LOGIN_NOT_EXIST = 31,
	HEADER_DG_LOGIN_WRONG_PASSWD = 33,
	HEADER_DG_LOGIN_ALREADY = 34,

	HEADER_DG_PLAYER_LOAD_SUCCESS = 35,
	HEADER_DG_PLAYER_LOAD_FAILED = 36,
	HEADER_DG_PLAYER_CREATE_SUCCESS = 37,
	HEADER_DG_PLAYER_CREATE_ALREADY = 38,
	HEADER_DG_PLAYER_CREATE_FAILED = 39,
	HEADER_DG_PLAYER_DELETE_SUCCESS = 40,
	HEADER_DG_PLAYER_DELETE_FAILED = 41,

	HEADER_DG_ITEM_LOAD = 42,

	HEADER_DG_BOOT = 43,
	HEADER_DG_QUEST_LOAD = 44,

	HEADER_DG_SAFEBOX_LOAD = 45,
	HEADER_DG_SAFEBOX_CHANGE_SIZE = 46,
	HEADER_DG_SAFEBOX_WRONG_PASSWORD = 47,
	HEADER_DG_SAFEBOX_CHANGE_PASSWORD_ANSWER = 48,

	HEADER_DG_EMPIRE_SELECT = 49,

	HEADER_DG_AFFECT_LOAD = 50,
	HEADER_DG_MALL_LOAD = 51,

	HEADER_DG_ACTIVITY_LOAD = 54,
	HEADER_DG_DIRECT_ENTER = 55,

	HEADER_DG_GUILD_SKILL_UPDATE = 56,
	HEADER_DG_GUILD_SKILL_RECHARGE = 57,
	HEADER_DG_GUILD_EXP_UPDATE = 58,

	HEADER_DG_PARTY_CREATE = 59,
	HEADER_DG_PARTY_DELETE = 60,
	HEADER_DG_PARTY_ADD = 61,
	HEADER_DG_PARTY_REMOVE = 62,
	HEADER_DG_PARTY_STATE_CHANGE = 63,
	HEADER_DG_PARTY_HEAL_USE = 64,
	HEADER_DG_PARTY_SET_MEMBER_LEVEL = 65,

	HEADER_DG_TIME = 90,
	HEADER_DG_ITEM_ID_RANGE = 91,

	HEADER_DG_GUILD_ADD_MEMBER = 92,
	HEADER_DG_GUILD_REMOVE_MEMBER = 93,
	HEADER_DG_GUILD_CHANGE_GRADE = 94,
	HEADER_DG_GUILD_CHANGE_MEMBER_DATA = 95,
	HEADER_DG_GUILD_DISBAND = 96,
	HEADER_DG_GUILD_WAR = 97,
	HEADER_DG_GUILD_WAR_SCORE = 98,
	HEADER_DG_GUILD_TIME_UPDATE = 99,
	HEADER_DG_GUILD_LOAD = 100,
	HEADER_DG_GUILD_LADDER = 101,
	HEADER_DG_GUILD_SKILL_USABLE_CHANGE = 102,
	HEADER_DG_GUILD_MONEY_CHANGE = 103,
	HEADER_DG_GUILD_WITHDRAW_MONEY_GIVE = 104,

	HEADER_DG_SET_EVENT_FLAG = 105,

	HEADER_DG_GUILD_WAR_RESERVE_ADD = 106,
	HEADER_DG_GUILD_WAR_RESERVE_DEL = 107,
	HEADER_DG_GUILD_WAR_BET = 108,

	HEADER_DG_RELOAD_PROTO = 120,
	HEADER_DG_CHANGE_NAME = 121,

	HEADER_DG_AUTH_LOGIN = 122,

	HEADER_DG_CHANGE_EMPIRE_PRIV = 124,
	HEADER_DG_CHANGE_GUILD_PRIV = 125,

	HEADER_DG_CHANGE_CHARACTER_PRIV = 127,

	HEADER_DG_CREATE_OBJECT = 140,
	HEADER_DG_DELETE_OBJECT = 141,

	HEADER_DG_MARRIAGE_ADD = 150,
	HEADER_DG_MARRIAGE_UPDATE = 151,
	HEADER_DG_MARRIAGE_REMOVE = 152,

	HEADER_DG_WEDDING_REQUEST = 153,
	HEADER_DG_WEDDING_READY = 154,
	HEADER_DG_WEDDING_START = 155,
	HEADER_DG_WEDDING_END = 156,

	HEADER_DG_MYSHOP_PRICELIST_RES = 157,		///< 가격정보 리스트 응답
	HEADER_DG_RELOAD_ADMIN = 158, 				///< 운영자 정보 리로드 
	HEADER_DG_BREAK_MARRIAGE = 159,				///< 결혼 파기

	HEADER_DG_ACK_CHANGE_GUILD_MASTER = 173,

	HEADER_DG_ACK_SPARE_ITEM_ID_RANGE = 174,

	HEADER_DG_UPDATE_HORSE_NAME = 175,
	HEADER_DG_ACK_HORSE_NAME = 176,

	HEADER_DG_NEED_LOGIN_LOG = 177,
	HEADER_DG_RESULT_CHARGE_CASH = 179,
	HEADER_DG_ITEMAWARD_INFORMER = 180,	//gift notify
	HEADER_DG_RESPOND_CHANNELSTATUS = 181,

	HEADER_DG_MAP_LOCATIONS = 0xfe,
	HEADER_DG_P2P = 0xff,
};

// =========================================================================================================================

// Sub headers
// =========================================================================================================================
enum EPacketCGDragonSoulSubHeaderType
{
	DS_SUB_HEADER_OPEN,
	DS_SUB_HEADER_CLOSE,
	DS_SUB_HEADER_DO_UPGRADE,
	DS_SUB_HEADER_DO_IMPROVEMENT,
	DS_SUB_HEADER_DO_REFINE,
	DS_SUB_HEADER_REFINE_FAIL,
	DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE,
	DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL,
	DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY,
	DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL,
	DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL,
	DS_SUB_HEADER_REFINE_SUCCEED,
};

enum EPacketCGGuildSubHeaderType
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
	GUILD_SUBHEADER_CG_WITHDRAW_MONEY
};

enum EPacketGCGuildSubHeaderType
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

enum EPacketGGGuildSubHeaderType
{
	GUILD_SUBHEADER_GG_CHAT,
	GUILD_SUBHEADER_GG_SET_MEMBER_COUNT_BONUS,
};

enum EPacketCGMessengerSubHeaderType
{
	MESSENGER_SUBHEADER_CG_ADD_BY_VID,
	MESSENGER_SUBHEADER_CG_ADD_BY_NAME,
	MESSENGER_SUBHEADER_CG_REMOVE
};

enum EPacketGCMessengerSubHeaderType
{
	MESSENGER_SUBHEADER_GC_LIST,
	MESSENGER_SUBHEADER_GC_LOGIN,
	MESSENGER_SUBHEADER_GC_LOGOUT,
	MESSENGER_SUBHEADER_GC_INVITE
};

enum EPacketCGShopSubHeaderType
{
	SHOP_SUBHEADER_CG_END,
	SHOP_SUBHEADER_CG_BUY,
	SHOP_SUBHEADER_CG_SELL,
	SHOP_SUBHEADER_CG_SELL2
};

enum EPacketGCShopSubHeaderType
{
	SHOP_SUBHEADER_GC_START,
	SHOP_SUBHEADER_GC_END,
	SHOP_SUBHEADER_GC_UPDATE_ITEM,
	SHOP_SUBHEADER_GC_UPDATE_PRICE,
	SHOP_SUBHEADER_GC_OK,
	SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY,
	SHOP_SUBHEADER_GC_INVENTORY_FULL,
	SHOP_SUBHEADER_GC_INVALID_POS,
	SHOP_SUBHEADER_GC_SOLD_OUT,
	SHOP_SUBHEADER_GC_START_EX,
	SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX,
};

enum EPacketCGExchangeSubHeaderType
{
	EXCHANGE_SUBHEADER_CG_START,			// arg1 == vid of target character
	EXCHANGE_SUBHEADER_CG_ITEM_ADD,		// arg1 == position of item
	EXCHANGE_SUBHEADER_CG_ITEM_DEL,		// arg1 == position of item
	EXCHANGE_SUBHEADER_CG_ELK_ADD,			// arg1 == amount of elk
	EXCHANGE_SUBHEADER_CG_ACCEPT,			// arg1 == not used
	EXCHANGE_SUBHEADER_CG_CANCEL,			// arg1 == not used
};

enum EPacketGCExchangeSubHeaderType
{
	EXCHANGE_SUBHEADER_GC_START,			// arg1 == vid
	EXCHANGE_SUBHEADER_GC_ITEM_ADD,		// arg1 == vnum  arg2 == pos  arg3 == count
	EXCHANGE_SUBHEADER_GC_ITEM_DEL,		// arg1 == pos
	EXCHANGE_SUBHEADER_GC_ELK_ADD,			// arg1 == elk
	EXCHANGE_SUBHEADER_GC_ACCEPT,			// arg1 == accept
	EXCHANGE_SUBHEADER_GC_END,				// arg1 == not used
	EXCHANGE_SUBHEADER_GC_ALREADY,			// arg1 == not used
	EXCHANGE_SUBHEADER_GC_LESS_ELK,		// arg1 == not used
};

enum EPacketGCFishingSubHeaderType
{
	FISHING_SUBHEADER_GC_START,
	FISHING_SUBHEADER_GC_STOP,
	FISHING_SUBHEADER_GC_REACT,
	FISHING_SUBHEADER_GC_SUCCESS,
	FISHING_SUBHEADER_GC_FAIL,
	FISHING_SUBHEADER_GC_FISH,
};

enum EPacketCGAcceSubHeaderType
{
	ACCE_SUBHEADER_CG_CLOSE,
	ACCE_SUBHEADER_CG_ADD,
	ACCE_SUBHEADER_CG_REMOVE,
	ACCE_SUBHEADER_CG_REFINE
};

enum EPacketGCAcceSubHeaderType
{
	ACCE_SUBHEADER_GC_OPEN,
	ACCE_SUBHEADER_GC_CLOSE,
	ACCE_SUBHEADER_GC_ADDED,
	ACCE_SUBHEADER_GC_REMOVED,
	ACCE_SUBHEADER_GC_REFINED
};

enum EPacketGCDungeonSubHeaderType
{
	DUNGEON_SUBHEADER_GC_TIME_ATTACK_START,
	DUNGEON_SUBHEADER_GC_DESTINATION_POSITION
};

// =========================================================================================================================


#pragma pack(push)
#pragma pack(1)

// Common
// =========================================================================================================================
struct SDynamicSizePacketHeader
{
	uint8_t	 header{};
	uint16_t size{};
};

struct SPacketSyncPositionElement
{
	uint32_t dwVID{};
	int32_t	 lX{};
	int32_t	 lY{};
};
// =========================================================================================================================

// Client To Game
// =========================================================================================================================

// Size calculation:
//	Header size(sizeof(TPacketHeader)) (default: 1)
//  Struct elements size
// Example:
//	NET_DECLARE_PACKET(42, { char x[12]; uint32_t y; });
//	sizeof(TPacketHeader) + /* sizeof(this->x) */ + /* sizeof(this->y) */
//	1					  +    12                 + 4                  = 17 
//	sizeof(packet) /* 17 */ should be equal to 17

struct SPacketCGLogin : SNetPacket
{
	char login[LOGIN_MAX_LEN + 1]{};
	char passwd[PASSWD_MAX_LEN + 1]{};

	NET_DECLARE_PACKET(HEADER_CG_LOGIN, SPacketCGLogin); // 1
};
static_assert((sizeof(SPacketCGLogin)) == (sizeof(TPacketHeader) + LOGIN_MAX_LEN + 1 + PASSWD_MAX_LEN + 1));
static_assert(sizeof(SPacketCGLogin) == 49);

struct SPacketCGAttack : SNetPacket
{
	uint8_t		bType{};
	uint32_t	dwVictimVID{};

	NET_DECLARE_PACKET(HEADER_CG_ATTACK, SPacketCGAttack); // 2
};
static_assert((sizeof(SPacketCGAttack)) == (sizeof(TPacketHeader) + (sizeof(uint8_t) + sizeof(uint32_t))));
static_assert(sizeof(SPacketCGAttack) == 6);

struct SPacketCGChat : SNetPacket
{
	uint16_t length{};
	uint8_t	 type{};

	NET_DECLARE_PACKET(HEADER_CG_CHAT, SPacketCGChat); // 3
};
static_assert((sizeof(SPacketCGChat)) == (sizeof(TPacketHeader) + (sizeof(uint16_t) + sizeof(uint8_t))));
static_assert(sizeof(SPacketCGChat) == 4);

struct SPacketCGCreateCharacter : SNetPacket
{
	uint8_t     index{};
	char        name[CHARACTER_NAME_MAX_LEN + 1]{};
	uint16_t    job{};
	uint8_t		shape{};
	uint8_t		CON{};
	uint8_t		IQ{};
	uint8_t		STR{};
	uint8_t		DEX{};

	NET_DECLARE_PACKET(HEADER_CG_PLAYER_CREATE, SPacketCGCreateCharacter); // 4
};
static_assert((sizeof(SPacketCGCreateCharacter)) == (sizeof(TPacketHeader) + (sizeof(uint8_t) + CHARACTER_NAME_MAX_LEN + 1 + sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t))));
static_assert(sizeof(SPacketCGCreateCharacter) == 34);

struct SPacketCGDestroyCharacter : SNetPacket
{
	uint8_t index{};
	char	szPrivateCode[PRIVATE_CODE_LENGTH]{};

	NET_DECLARE_PACKET(HEADER_CG_PLAYER_DESTROY, SPacketCGDestroyCharacter); // 5
};
static_assert((sizeof(SPacketCGDestroyCharacter)) == (sizeof(TPacketHeader) + sizeof(uint8_t) + PRIVATE_CODE_LENGTH));
static_assert(sizeof(SPacketCGDestroyCharacter) == 10);

struct SPacketCGSelectCharacter : SNetPacket
{
	uint8_t	player_index{};

	NET_DECLARE_PACKET(HEADER_CG_PLAYER_SELECT, SPacketCGSelectCharacter); // 6
};
static_assert((sizeof(SPacketCGSelectCharacter)) == (sizeof(TPacketHeader) + sizeof(uint8_t)));
static_assert(sizeof(SPacketCGSelectCharacter) == 2);

struct SPacketCGMove : SNetPacket
{
	uint8_t		bFunc{};
	uint8_t		bArg{};
	float		rot{};
	int32_t		lX{};
	int32_t		lY{};
	uint32_t	dwTime{};

	NET_DECLARE_PACKET(HEADER_CG_CHARACTER_MOVE, SPacketCGMove); // 7
};

struct SPacketCGSyncPosition : SNetPacket
{
	uint16_t wSize{};

	NET_DECLARE_PACKET(HEADER_CG_SYNC_POSITION, SPacketCGSyncPosition); // 8
};

struct SPacketCGEnterFrontGame : SNetPacket
{
	NET_DECLARE_PACKET(HEADER_CG_ENTERGAME, SPacketCGEnterFrontGame); // 9
};

struct SPacketCGItemUse : SNetPacket
{
	TItemPos pos{};

	NET_DECLARE_PACKET(HEADER_CG_ITEM_USE, SPacketCGItemUse); // 10
};

struct SPacketCGItemDrop : SNetPacket
{
	TItemPos pos{};
	uint32_t elk{};

	NET_DECLARE_PACKET(HEADER_CG_ITEM_DROP, SPacketCGItemDrop); // 11
};

struct SPacketCGItemMove : SNetPacket
{
	TItemPos pos{};
	TItemPos change_pos{};
	uint8_t  num{};

	NET_DECLARE_PACKET(HEADER_CG_ITEM_MOVE, SPacketCGItemMove); // 12
};

struct SPacketCGItemPickUp : SNetPacket
{
	uint32_t vid{};

	NET_DECLARE_PACKET(HEADER_CG_ITEM_PICKUP, SPacketCGItemPickUp); // 13
};

struct SPacketCGQuickSlotAdd : SNetPacket
{
	uint8_t     pos{};
	TQuickSlot	slot{};

	NET_DECLARE_PACKET(HEADER_CG_QUICKSLOT_ADD, SPacketCGQuickSlotAdd); // 14
};

struct SPacketCGQuickSlotDel : SNetPacket
{
	uint8_t pos{};
	uint8_t change_pos{};

	NET_DECLARE_PACKET(HEADER_CG_QUICKSLOT_DEL, SPacketCGQuickSlotDel); // 15
};

struct SPacketCGQuickSlotSwap : SNetPacket
{
	uint8_t pos{};
	uint8_t change_pos{};

	NET_DECLARE_PACKET(HEADER_CG_QUICKSLOT_SWAP, SPacketCGQuickSlotSwap); // 16
};

struct SPacketCGWhisper : SNetPacket
{
	uint16_t wSize{};
	char     szNameTo[CHARACTER_NAME_MAX_LEN + 1]{};

	NET_DECLARE_PACKET(HEADER_CG_WHISPER, SPacketCGWhisper); // 17
};

struct SPacketCGItemDrop2 : SNetPacket
{
	TItemPos pos{};
	uint32_t gold{};
	uint8_t  count{};

	NET_DECLARE_PACKET(HEADER_CG_ITEM_DROP2, SPacketCGItemDrop2); // 18
};

struct SPacketCGOnClick : SNetPacket
{
	uint32_t vid{};

	NET_DECLARE_PACKET(HEADER_CG_ON_CLICK, SPacketCGOnClick); // 19
};

struct SPacketCGExchange : SNetPacket
{
	uint8_t		subheader{};
	uint32_t	arg1{};
	uint8_t		arg2{};
	TItemPos	Pos{};

	NET_DECLARE_PACKET(HEADER_CG_EXCHANGE, SPacketCGExchange); // 20
};

struct SPacketCGPosition : SNetPacket
{
	uint8_t position{};

	NET_DECLARE_PACKET(HEADER_CG_CHARACTER_POSITION, SPacketCGPosition); // 21
};

struct SPacketCGScriptAnswer : SNetPacket
{
	uint8_t	answer{};

	NET_DECLARE_PACKET(HEADER_CG_SCRIPT_ANSWER, SPacketCGScriptAnswer); // 22
};

struct SPacketCGQuestInputString : SNetPacket
{
	char szString[QUEST_INPUT_STRING_MAX_NUM + 1]{};

	NET_DECLARE_PACKET(HEADER_CG_QUEST_INPUT_STRING, SPacketCGQuestInputString); // 23
};

struct SPacketCGQuestConfirm : SNetPacket
{
	uint8_t answer{};
	uint32_t requestPID{};

	NET_DECLARE_PACKET(HEADER_CG_QUEST_CONFIRM, SPacketCGQuestConfirm); // 24
};

struct SPacketCGShop : SNetPacket
{
	uint8_t		subheader{};

	NET_DECLARE_PACKET(HEADER_CG_SHOP, SPacketCGShop); // 25
};

struct SPacketCGFlyTargeting : SNetPacket
{
	uint32_t	dwTargetVID{};
	int32_t		lX{};
	int32_t		lY{};

	NET_DECLARE_PACKET(HEADER_CG_FLY_TARGETING, SPacketCGFlyTargeting); // 26
};

struct SPacketCGUseSkill : SNetPacket
{
	uint32_t dwVnum{};
	uint32_t dwTargetVID{};

	NET_DECLARE_PACKET(HEADER_CG_USE_SKILL, SPacketCGUseSkill); // 27
};

struct SPacketCGAddFlyTargeting : SNetPacket
{
	uint32_t	dwTargetVID{};
	int32_t		lX{};
	int32_t		lY{};

	NET_DECLARE_PACKET(HEADER_CG_ADD_FLY_TARGETING, SPacketCGAddFlyTargeting); // 28
};

struct SPacketCGShoot : SNetPacket
{
	uint8_t	bType{};

	NET_DECLARE_PACKET(HEADER_CG_SHOOT, SPacketCGShoot); // 29
};

struct SPacketCGMyShop : SNetPacket
{
	char	szSign[SHOP_SIGN_MAX_LEN + 1]{};
	uint8_t	bCount{};	// count of TShopItemTable, max 39

	NET_DECLARE_PACKET(HEADER_CG_MYSHOP, SPacketCGMyShop); // 30
};

struct SPacketCGItemUseToItem : SNetPacket
{
	TItemPos source_pos{};
	TItemPos target_pos{};

	NET_DECLARE_PACKET(HEADER_CG_ITEM_USE_TO_ITEM, SPacketCGItemUseToItem); // 31
};

struct SPacketCGTarget : SNetPacket
{
	uint32_t dwVID{};

	NET_DECLARE_PACKET(HEADER_CG_TARGET, SPacketCGTarget); // 32
};

struct SPacketCGTargetDrop : SNetPacket
{
	uint32_t dwVID{};

	NET_DECLARE_PACKET(HEADER_CG_TARGET_DROP, SPacketCGTargetDrop); // 33
};

struct SPacketCGChestDropInfo : SNetPacket
{
	uint16_t wInventoryCell{};

	NET_DECLARE_PACKET(HEADER_CG_CHEST_DROP_INFO, SPacketCGChestDropInfo); // 34
};

struct SPacketCGScriptButton : SNetPacket
{
	uint32_t idx{};

	NET_DECLARE_PACKET(HEADER_CG_SCRIPT_BUTTON, SPacketCGScriptButton); // 35
};

struct SPacketCGMessenger : SNetPacket
{
	uint8_t subheader{};

	NET_DECLARE_PACKET(HEADER_CG_MESSENGER, SPacketCGMessenger); // 36
};

struct SPacketCGMallCheckout : SNetPacket
{
	uint8_t     bMallPos{};
	TItemPos	ItemPos{};

	NET_DECLARE_PACKET(HEADER_CG_MALL_CHECKOUT, SPacketCGMallCheckout); // 37
};

struct SPacketCGSafeboxCheckin : SNetPacket
{
	uint8_t     bSafePos{};
	TItemPos	ItemPos{};

	NET_DECLARE_PACKET(HEADER_CG_SAFEBOX_CHECKIN, SPacketCGSafeboxCheckin); // 38
};

struct SPacketCGSafeboxCheckout : SNetPacket
{
	uint8_t     bSafePos{};
	TItemPos	ItemPos{};

	NET_DECLARE_PACKET(HEADER_CG_SAFEBOX_CHECKOUT, SPacketCGSafeboxCheckout); // 39
};

struct SPacketCGPartyInvite : SNetPacket
{
	uint32_t vid{};

	NET_DECLARE_PACKET(HEADER_CG_PARTY_INVITE, SPacketCGPartyInvite); // 40
};

struct SPacketCGPartyInviteAnswer : SNetPacket
{
	uint32_t leader_vid{};
	uint8_t accept{};

	NET_DECLARE_PACKET(HEADER_CG_PARTY_INVITE_ANSWER, SPacketCGPartyInviteAnswer); // 41
};

struct SPacketCGPartyRemove : SNetPacket
{
	uint32_t pid{};

	NET_DECLARE_PACKET(HEADER_CG_PARTY_REMOVE, SPacketCGPartyRemove); // 42
};

struct SPacketCGPartySetState : SNetPacket
{
	uint32_t dwPID{};
	uint8_t byState{};
	uint8_t byFlag{};

	NET_DECLARE_PACKET(HEADER_CG_PARTY_SET_STATE, SPacketCGPartySetState); // 43
};

struct SPacketCGPartyUseSkill : SNetPacket
{
	uint8_t bySkillIndex{};
	uint32_t dwTargetVID{};

	NET_DECLARE_PACKET(HEADER_CG_PARTY_USE_SKILL, SPacketCGPartyUseSkill); // 44
};

struct SPacketCGSafeboxItemMove : SNetPacket
{
	TItemPos pos{};
	TItemPos change_pos{};
	uint8_t num{};

	NET_DECLARE_PACKET(HEADER_CG_SAFEBOX_ITEM_MOVE, SPacketCGSafeboxItemMove); // 45
};

struct SPacketCGPartyParameter : SNetPacket
{
	uint8_t bDistributeMode{};

	NET_DECLARE_PACKET(HEADER_CG_PARTY_PARAMETER, SPacketCGPartyParameter); // 46
};

struct SPacketCGGuild : SNetPacket
{
	uint8_t bySubHeader{};

	NET_DECLARE_PACKET(HEADER_CG_GUILD, SPacketCGGuild); // 47
};

struct SPacketCGAnswerMakeGuild : SNetPacket
{
	char guild_name[GUILD_NAME_MAX_LEN + 1]{};

	NET_DECLARE_PACKET(HEADER_CG_ANSWER_MAKE_GUILD, SPacketCGAnswerMakeGuild); // 48
};

struct SPacketCGFishing : SNetPacket
{
	uint8_t dir{};

	NET_DECLARE_PACKET(HEADER_CG_FISHING, SPacketCGFishing); // 49
};

struct SPacketCGGiveItem : SNetPacket
{
	uint32_t dwTargetVID{};
	TItemPos ItemPos{};
	uint8_t  byItemCount{};

	NET_DECLARE_PACKET(HEADER_CG_GIVE_ITEM, SPacketCGGiveItem); // 50
};

struct SPacketCGEmpire : SNetPacket
{
	uint8_t bEmpire{};

	NET_DECLARE_PACKET(HEADER_CG_EMPIRE, SPacketCGEmpire); // 51
};

struct SPacketCGRefine : SNetPacket
{
	uint8_t pos{};
	uint8_t	type{};

	NET_DECLARE_PACKET(HEADER_CG_REFINE, SPacketCGRefine); // 52
};

struct SPacketCGMarkLogin : SNetPacket
{
	uint32_t handle{};
	uint32_t random_key{};

	NET_DECLARE_PACKET(HEADER_CG_MARK_LOGIN, SPacketCGMarkLogin); // 53
};

struct SPacketCGMarkCRCList : SNetPacket
{
	uint8_t  imgIdx{};
	uint32_t crclist[80]{};

	NET_DECLARE_PACKET(HEADER_CG_MARK_CRCLIST, SPacketCGMarkCRCList); // 54
};

struct SPacketCGMarkUpload : SNetPacket
{
	uint32_t gid{};
	uint8_t  image[16 * 12 * 4]{};

	NET_DECLARE_PACKET(HEADER_CG_MARK_UPLOAD, SPacketCGMarkUpload); // 55
};

struct SPacketCGMarkIDXList : SNetPacket
{
	NET_DECLARE_PACKET(HEADER_CG_MARK_LOGIN, SPacketCGMarkIDXList); // 56
};

struct SPacketCGHack : SNetPacket
{
	char szBuf[255 + 1]{};
	char szInfo[255 + 1]{};

	NET_DECLARE_PACKET(HEADER_CG_HACK, SPacketCGHack); // 57
};

struct SPacketCGChangeName : SNetPacket
{
	uint8_t index{};
	char name[CHARACTER_NAME_MAX_LEN + 1]{};

	NET_DECLARE_PACKET(HEADER_CG_CHANGE_NAME, SPacketCGChangeName); // 58
};

struct SPacketCGLogin2 : SNetPacket
{
	char		name[LOGIN_MAX_LEN + 1]{};
	uint32_t	login_key{};
	uint32_t	adwClientKey[4]{};

	NET_DECLARE_PACKET(HEADER_CG_LOGIN2, SPacketCGLogin2); // 59
};

struct SPacketCGLogin3 : SNetPacket
{
	char		name[LOGIN_MAX_LEN + 1]{};
	char		pwd[PASSWD_MAX_LEN + 1]{};
	uint32_t	adwClientKey[4]{};
	uint32_t	version{};
	char		hwid[HWID_MAX_HASH_LEN + 1]{};
	char		lang[LANG_MAX_LEN + 1]{};

	NET_DECLARE_PACKET(HEADER_CG_LOGIN3, SPacketCGLogin3); // 60
};

struct SPacketCGGuildSymbolUpload : SNetPacket
{
	uint16_t	size{};
	uint32_t	guild_id{};

	NET_DECLARE_PACKET(HEADER_CG_GUILD_SYMBOL_UPLOAD, SPacketCGGuildSymbolUpload); // 61
};

struct SPacketCGGuildSymbolCRC : SNetPacket
{
	uint32_t	dwGuildID{};
	uint32_t	dwCRC{};
	uint32_t	dwSize{};
	uint8_t		isLastEntry{};

	NET_DECLARE_PACKET(HEADER_CG_GUILD_SYMBOL_CRC, SPacketCGGuildSymbolCRC); // 62
};

struct SPacketCGScriptSelectItem : SNetPacket
{
	uint32_t selection{};

	NET_DECLARE_PACKET(HEADER_CG_SCRIPT_SELECT_ITEM, SPacketCGScriptSelectItem); // 63
};

struct SPacketCGDragonSoulRefine : SNetPacket
{
	uint8_t bSubType{};
	TItemPos ItemGrid[DS_REFINE_WINDOW_MAX_NUM]{};

	NET_DECLARE_PACKET(HEADER_CG_DRAGON_SOUL_REFINE, SPacketCGDragonSoulRefine); // 64
};

struct SPacketCGStateChecker : SNetPacket
{
	NET_DECLARE_PACKET(HEADER_CG_STATE_CHECKER, SPacketCGStateChecker); // 65
};

struct SPacketCGPong : SNetPacket
{
	NET_DECLARE_PACKET(HEADER_CG_PONG, SPacketCGPong); // 252
};

struct SPacketCGTimeSync : SNetPacket
{
	uint32_t	dwHandshake{};
	uint32_t	dwTime{};
	int32_t		lDelta{};

	NET_DECLARE_PACKET(HEADER_CG_TIME_SYNC, SPacketCGTimeSync); // 253
};
// =========================================================================================================================


// Game To Client
// =========================================================================================================================
struct SPacketGCCharacterAdd : SNetPacket
{
	uint32_t dwVID{};

	float    angle{};
	int32_t  x{};
	int32_t  y{};
	int32_t  z{};

	uint8_t	 bType{};
	uint32_t dwRaceNum{};
	uint8_t  bMovingSpeed{};
	uint8_t  bAttackSpeed{};

	uint8_t   bStateFlag{};
	uint32_t  dwAffectFlag[2]{};
	uint32_t  dwGuild{};
	uint32_t  dwLevel{};

	NET_DECLARE_PACKET(HEADER_GC_CHARACTER_ADD, SPacketGCCharacterAdd); // 1
};

struct SPacketGCCharacterDelete : SNetPacket
{
	uint32_t	dwVID{};

	NET_DECLARE_PACKET(HEADER_GC_CHARACTER_DEL, SPacketGCCharacterDelete); // 2
};

struct SPacketGCMove : SNetPacket
{
	uint8_t		bFunc{};
	uint8_t		bArg{};
	float		rot{};
	uint32_t		dwVID{};
	int32_t		lX{};
	int32_t		lY{};
	uint32_t		dwTime{};
	uint32_t		dwDuration{};

	NET_DECLARE_PACKET(HEADER_GC_CHARACTER_MOVE, SPacketGCMove); // 3
};

struct SPacketGCChat : SNetPacket
{
	uint16_t	size{};
	uint8_t	type{};
	uint32_t	dwVID{};
	uint8_t	bEmpire{};

	NET_DECLARE_PACKET(HEADER_GC_CHAT, SPacketGCChat); // 4
};

struct SPacketGCSyncPosition : SNetPacket
{
	uint16_t		wSize{};

	NET_DECLARE_PACKET(HEADER_GC_SYNC_POSITION, SPacketGCSyncPosition); // 5
};

struct SPacketGCLoginSuccess : SNetPacket
{
	TSimplePlayerInformation	akSimplePlayerInformation[PLAYER_PER_ACCOUNT]{};
	uint32_t						guild_id[PLAYER_PER_ACCOUNT]{};
	char						guild_name[PLAYER_PER_ACCOUNT][GUILD_NAME_MAX_LEN + 1]{};
	uint32_t handle{};
	uint32_t random_key{};

	NET_DECLARE_PACKET(HEADER_GC_LOGIN_SUCCESS, SPacketGCLoginSuccess); // 6
};

struct SPacketGCLoginFailure : SNetPacket
{
	char	szStatus[LOGIN_STATUS_MAX_LEN + 1]{};

	NET_DECLARE_PACKET(HEADER_GC_LOGIN_FAILURE, SPacketGCLoginFailure); // 7
};

struct SPacketGCPlayerCreateSuccess : SNetPacket
{
	uint8_t						bAccountCharacterSlot{};
	TSimplePlayerInformation	kSimplePlayerInfomation{};

	NET_DECLARE_PACKET(HEADER_GC_PLAYER_CREATE_SUCCESS, SPacketGCPlayerCreateSuccess); // 8
};

struct SPacketGCCreateFailure : SNetPacket
{
	uint8_t	bType{};

	NET_DECLARE_PACKET(HEADER_GC_PLAYER_CREATE_FAILURE, SPacketGCCreateFailure); // 9
};

struct SPacketGCPlayerDeleteSuccess : SNetPacket
{
	NET_DECLARE_PACKET(HEADER_GC_PLAYER_DELETE_SUCCESS, SPacketGCPlayerDeleteSuccess); // 10
};

struct SPacketGCPlayerDeleteWrongSocialID : SNetPacket
{
	NET_DECLARE_PACKET(HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID, SPacketGCPlayerDeleteWrongSocialID); // 11
};

struct SPacketGCStun : SNetPacket
{
	uint32_t		vid{};

	NET_DECLARE_PACKET(HEADER_GC_STUN, SPacketGCStun); // 12
};

struct SPacketGCDead : SNetPacket
{
	uint32_t		vid{};

	NET_DECLARE_PACKET(HEADER_GC_DEAD, SPacketGCDead); // 13
};

struct SPacketGCPoints : SNetPacket
{
	int32_t        points[POINT_MAX_NUM]{};

	NET_DECLARE_PACKET(HEADER_GC_PLAYER_POINTS, SPacketGCPoints); // 14
};

struct SPacketGCPointChange : SNetPacket
{
	uint32_t		dwVID{};
	uint8_t		Type{};
	int32_t        amount{};
	int32_t        value{};

	NET_DECLARE_PACKET(HEADER_GC_PLAYER_POINT_CHANGE, SPacketGCPointChange); // 15
};

struct SPacketGCChangeSpeed : SNetPacket
{
	uint32_t		vid{};
	uint16_t		moving_speed{};

	NET_DECLARE_PACKET(HEADER_GC_CHANGE_SPEED, SPacketGCChangeSpeed); // 16
};

struct SPacketGCCharacterUpdate : SNetPacket
{
	uint32_t       dwVID{};

	uint16_t        awPart[CHR_EQUIPPART_NUM]{};
	uint8_t        bMovingSpeed{};
	uint8_t		bAttackSpeed{};

	uint8_t        bStateFlag{};
	uint32_t       dwAffectFlag[2]{};

	uint32_t		dwGuildID{};
	int16_t       sAlignment{};
	uint8_t		bPKMode{};
	uint32_t		dwMountVnum{};

	NET_DECLARE_PACKET(HEADER_GC_CHARACTER_UPDATE, SPacketGCCharacterUpdate); // 17
};

struct SPacketGCItemSet : SNetPacket
{
	TItemPos	Cell{};
	uint32_t		vnum{};
	uint8_t		count{};
	int32_t		alSockets[ITEM_SOCKET_SLOT_MAX_NUM]{};
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM]{};

	NET_DECLARE_PACKET(HEADER_GC_ITEM_SET, SPacketGCItemSet); // 18
};

struct SPacketGCItemSet2 : SNetPacket
{
	TItemPos	Cell{};
	uint32_t		vnum{};
	uint8_t		count{};
	uint32_t		flags{};
	uint32_t		anti_flags{};
	bool		highlight{};
	int32_t		alSockets[ITEM_SOCKET_SLOT_MAX_NUM]{};
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM]{};

	NET_DECLARE_PACKET(HEADER_GC_ITEM_SET2, SPacketGCItemSet2); // 19
};

struct SPacketGCItemUse : SNetPacket
{
	TItemPos Cell{};

	uint32_t ch_vid{};
	uint32_t victim_vid{};

	uint32_t vnum{};

	NET_DECLARE_PACKET(HEADER_GC_ITEM_USE, SPacketGCItemUse); // 20
};

// 21 = NULL

struct SPacketGCItemUpdate : SNetPacket
{
	TItemPos	Cell{};
	uint8_t		count{};
	int32_t		alSockets[ITEM_SOCKET_SLOT_MAX_NUM]{};
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM]{};

	NET_DECLARE_PACKET(HEADER_GC_ITEM_UPDATE, SPacketGCItemUpdate); // 22
};

struct SPacketGCItemGroundAdd : SNetPacket
{
	int32_t lX{};
	int32_t lY{};
	int32_t	lZ{};

	uint32_t dwVID{};
	uint32_t dwVnum{};

	NET_DECLARE_PACKET(HEADER_GC_ITEM_GROUND_ADD, SPacketGCItemGroundAdd); // 23
};

struct SPacketGCItemGroundDel : SNetPacket
{
	uint32_t vid{};

	NET_DECLARE_PACKET(HEADER_GC_ITEM_GROUND_DEL, SPacketGCItemGroundDel); // 24
};

struct SPacketGCQuickSlotAdd : SNetPacket
{
	uint8_t     pos{};
	TQuickSlot	slot{};

	NET_DECLARE_PACKET(HEADER_GC_QUICKSLOT_ADD, SPacketGCQuickSlotAdd); // 25
};

struct SPacketGCQuickSlotDel : SNetPacket
{
	uint8_t pos{};

	NET_DECLARE_PACKET(HEADER_GC_QUICKSLOT_DEL, SPacketGCQuickSlotDel); // 26
};

struct SPacketGCQuickSlotSwap : SNetPacket
{
	uint8_t pos{};
	uint8_t change_pos{};

	NET_DECLARE_PACKET(HEADER_GC_QUICKSLOT_SWAP, SPacketGCQuickSlotSwap); // 27
};

struct SPacketGCItemOwnership : SNetPacket
{
	uint32_t dwVID{};
	char     szName[CHARACTER_NAME_MAX_LEN + 1]{};

	NET_DECLARE_PACKET(HEADER_GC_ITEM_OWNERSHIP, SPacketGCItemOwnership); // 28
};

struct SPacketGCWhisper : SNetPacket
{
	uint16_t wSize{};
	uint8_t  bType{};
	char     szNameFrom[CHARACTER_NAME_MAX_LEN + 1]{};

	NET_DECLARE_PACKET(HEADER_GC_WHISPER, SPacketGCWhisper); // 29
};

struct SPacketGCMotion : SNetPacket
{
	uint32_t vid{};
	uint32_t victim_vid{};
	uint16_t motion{};

	NET_DECLARE_PACKET(HEADER_GC_MOTION, SPacketGCMotion); // 30
};

struct SPacketGCShop : SNetPacket
{
	uint16_t size{};
	uint8_t  subheader{};

	NET_DECLARE_PACKET(HEADER_GC_SHOP, SPacketGCShop); // 31
};

struct SPacketGCShopSign : SNetPacket
{
	uint32_t dwVID{};
	char     szSign[SHOP_SIGN_MAX_LEN + 1]{};

	NET_DECLARE_PACKET(HEADER_GC_SHOP_SIGN, SPacketGCShopSign); // 32
};

struct SPacketGCDuelStart : SNetPacket
{
	uint16_t wSize{};

	NET_DECLARE_PACKET(HEADER_GC_DUEL_START, SPacketGCDuelStart); // 33
};

struct SPacketGCPVP : SNetPacket
{
	uint32_t	dwVIDSrc{};
	uint32_t	dwVIDDst{};
	uint8_t		bMode{};

	NET_DECLARE_PACKET(HEADER_GC_PVP, SPacketGCPVP); // 34
};

struct SPacketGCExchange : SNetPacket
{
	uint8_t        subheader{};

	uint8_t        is_me{};

	uint32_t       arg1{};
	TItemPos       arg2{};
	uint32_t       arg3{};

	int32_t				 alSockets[ITEM_SOCKET_SLOT_MAX_NUM]{};
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM]{};

	NET_DECLARE_PACKET(HEADER_GC_EXCHANGE, SPacketGCExchange); // 35
};

struct SPacketGCPosition : SNetPacket
{
	uint32_t vid{};
	uint8_t  position{};

	NET_DECLARE_PACKET(HEADER_GC_CHARACTER_POSITION, SPacketGCPosition); // 36
};

struct SPacketGCPing : SNetPacket
{
	NET_DECLARE_PACKET(HEADER_GC_PING, SPacketGCPing); // 37
};

struct SPacketGCScript : SNetPacket
{
	uint16_t size{};
	uint8_t  skin{};
	uint16_t src_size{};

	NET_DECLARE_PACKET(HEADER_GC_SCRIPT, SPacketGCScript); // 38
};

struct SPacketGCQuestConfirm : SNetPacket
{
	char msg[64 + 1]{};
	int32_t timeout{};
	uint32_t requestPID{};

	NET_DECLARE_PACKET(HEADER_GC_QUEST_CONFIRM, SPacketGCQuestConfirm); // 39
};

struct SPacketGCOwnership : SNetPacket
{
	uint32_t dwOwnerVID{};
	uint32_t dwVictimVID{};

	NET_DECLARE_PACKET(HEADER_GC_OWNERSHIP, SPacketGCOwnership); // 40
};

struct SPacketGCTarget : SNetPacket
{
	uint32_t       dwVID{};
	uint8_t        bHPPercent{};

	NET_DECLARE_PACKET(HEADER_GC_TARGET, SPacketGCTarget); // 41
};

struct SPacketGCWarp : SNetPacket
{
	int32_t			lX{};
	int32_t			lY{};
	int32_t			lAddr{};
	uint16_t			wPort{};

	NET_DECLARE_PACKET(HEADER_GC_WARP, SPacketGCWarp); // 42
};

struct SPacketGCAddFlyTargeting : SNetPacket
{
	uint32_t		dwShooterVID{};
	uint32_t		dwTargetVID{};
	int32_t		lX{};
	int32_t		lY{};

	NET_DECLARE_PACKET(HEADER_GC_ADD_FLY_TARGETING, SPacketGCAddFlyTargeting); // 43
};

struct SPacketGCCreateFly : SNetPacket
{
	uint8_t        bType{};
	uint32_t       dwStartVID{};
	uint32_t       dwEndVID{};

	NET_DECLARE_PACKET(HEADER_GC_CREATE_FLY, SPacketGCCreateFly); // 44
};

struct SPacketGCFlyTargeting : SNetPacket
{
	uint32_t		dwShooterVID{};
	uint32_t		dwTargetVID{};
	int32_t		lX{};
	int32_t		lY{};

	NET_DECLARE_PACKET(HEADER_GC_FLY_TARGETING, SPacketGCFlyTargeting); // 45
};

struct SPacketGCMessenger : SNetPacket
{
	uint16_t size{};
	uint8_t subheader{};

	NET_DECLARE_PACKET(HEADER_GC_MESSENGER, SPacketGCMessenger); // 46
};

struct SPacketGCGuild : SNetPacket
{
	uint16_t size{};
	uint8_t subheader{};

	NET_DECLARE_PACKET(HEADER_GC_GUILD, SPacketGCGuild); // 47
};

struct SPacketGCPartyInvite : SNetPacket
{
	uint32_t leader_vid{};

	NET_DECLARE_PACKET(HEADER_GC_PARTY_INVITE, SPacketGCPartyInvite); // 48
};

struct SPacketGCPartyAdd : SNetPacket
{
	uint32_t pid{};
	char name[CHARACTER_NAME_MAX_LEN + 1]{};

	NET_DECLARE_PACKET(HEADER_GC_PARTY_ADD, SPacketGCPartyAdd); // 49
};

struct SPacketGCPartyUpdate : SNetPacket
{
	uint32_t pid{};
	uint8_t state{};
	uint8_t percent_hp{};
	int16_t affects[PARTY_AFFECT_SLOT_MAX_NUM]{};

	NET_DECLARE_PACKET(HEADER_GC_PARTY_UPDATE, SPacketGCPartyUpdate); // 50
};

struct SPacketGCPartyRemove : SNetPacket
{
	uint32_t pid{};

	NET_DECLARE_PACKET(HEADER_GC_PARTY_REMOVE, SPacketGCPartyRemove); // 51
};

struct SPacketGCQuestInfo : SNetPacket
{
	uint16_t size{};
	uint16_t index{};
	uint8_t flag{};

	NET_DECLARE_PACKET(HEADER_GC_QUEST_INFO, SPacketGCQuestInfo); // 52
};

struct SPacketGCRequestGuildMake : SNetPacket
{
	NET_DECLARE_PACKET(HEADER_GC_REQUEST_MAKE_GUILD, SPacketGCRequestGuildMake); // 53
};

struct SPacketGCPartyParameter : SNetPacket
{
	uint8_t bDistributeMode{};

	NET_DECLARE_PACKET(HEADER_GC_PARTY_PARAMETER, SPacketGCPartyParameter); // 54
};

struct SPacketGCSafeboxSet : SNetPacket
{
	TItemPos	Cell{};
	uint32_t		vnum{};
	uint8_t		count{};
	int32_t		alSockets[ITEM_SOCKET_SLOT_MAX_NUM]{};
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM]{};

	NET_DECLARE_PACKET(HEADER_GC_SAFEBOX_SET, SPacketGCSafeboxSet); // 55
};

struct SPacketGCSafeboxDel : SNetPacket
{
	uint8_t pos{};

	NET_DECLARE_PACKET(HEADER_GC_SAFEBOX_DEL, SPacketGCSafeboxDel); // 56
};

struct SPacketGCSafeboxWrongPassword : SNetPacket
{
	NET_DECLARE_PACKET(HEADER_GC_SAFEBOX_WRONG_PASSWORD, SPacketGCSafeboxWrongPassword); // 57
};

struct SPacketGCSafeboxSize : SNetPacket
{
	uint8_t bSize{};

	NET_DECLARE_PACKET(HEADER_GC_SAFEBOX_SIZE, SPacketGCSafeboxSize); // 58
};

struct SPacketGCFishing : SNetPacket
{
	uint8_t subheader{};
	uint32_t info{};
	uint8_t dir{};

	NET_DECLARE_PACKET(HEADER_GC_FISHING, SPacketGCFishing); // 59
};

struct SPacketGCEmpire : SNetPacket
{
	uint8_t bEmpire{};

	NET_DECLARE_PACKET(HEADER_GC_EMPIRE, SPacketGCEmpire); // 60
};

struct SPacketGCPartyLink : SNetPacket
{
	uint32_t pid{};
	uint32_t vid{};

	NET_DECLARE_PACKET(HEADER_GC_PARTY_LINK, SPacketGCPartyLink); // 61
};

struct SPacketGCPartyUnlink : SNetPacket
{
	uint32_t pid{};
	uint32_t vid{};

	NET_DECLARE_PACKET(HEADER_GC_PARTY_UNLINK, SPacketGCPartyUnlink); // 62
};

struct SPacketGCViewEquip : SNetPacket
{
	uint32_t dwVID{};
	TEquipmentItemSet equips[WEAR_MAX_NUM]{};

	NET_DECLARE_PACKET(HEADER_GC_VIEW_EQUIP, SPacketGCViewEquip); // 63
};

struct SPacketGCMarkBlock : SNetPacket
{
	uint32_t   bufSize{};
	uint8_t	imgIdx{};
	uint32_t   count{};

	NET_DECLARE_PACKET(HEADER_GC_MARK_BLOCK, SPacketGCMarkBlock); // 64
};

struct SPacketGCMarkIDXList : SNetPacket
{
	uint32_t	bufSize{};
	uint16_t    count{};

	NET_DECLARE_PACKET(HEADER_GC_MARK_IDXLIST, SPacketGCMarkIDXList); // 65
};

struct SPacketGCTime : SNetPacket
{
	uint32_t      time{};

	NET_DECLARE_PACKET(HEADER_GC_TIME, SPacketGCTime); // 66
};

struct SPacketGCChangeName : SNetPacket
{
	uint32_t pid{};
	char name[CHARACTER_NAME_MAX_LEN + 1]{};

	NET_DECLARE_PACKET(HEADER_GC_CHANGE_NAME, SPacketGCChangeName); // 67
};

struct SPacketGCDungeon : SNetPacket
{
	uint16_t		size{};
	uint8_t		subheader{};

	NET_DECLARE_PACKET(HEADER_GC_DUNGEON, SPacketGCDungeon); // 68
};

struct SPacketGCWalkMode : SNetPacket
{
	uint32_t       vid{};
	uint8_t        mode{};

	NET_DECLARE_PACKET(HEADER_GC_WALK_MODE, SPacketGCWalkMode); // 69
};

struct SPacketGCChangeSkillGroup : SNetPacket
{
	uint8_t skill_group{};

	NET_DECLARE_PACKET(HEADER_GC_CHANGE_SKILL_GROUP, SPacketGCChangeSkillGroup); // 70
};

struct SPacketGCMainCharacter : SNetPacket
{
	uint32_t       dwVID{};
	uint16_t		wRaceNum{};
	char        szName[CHARACTER_NAME_MAX_LEN + 1]{};
	int32_t        lX{}, lY{}, lZ{};
	uint8_t		byEmpire{};
	uint8_t		bySkillGroup{};

	NET_DECLARE_PACKET(HEADER_GC_MAIN_CHARACTER, SPacketGCMainCharacter); // 71
};

struct SPacketGCSpecialEffect : SNetPacket
{
	uint8_t  type{};
	uint32_t vid{};

	NET_DECLARE_PACKET(HEADER_GC_SPECIAL_EFFECT, SPacketGCSpecialEffect); // 72
};

struct SPacketGCNPCPosition : SNetPacket
{
	uint16_t size{};
	uint16_t count{};

	NET_DECLARE_PACKET(HEADER_GC_NPC_POSITION, SPacketGCNPCPosition); // 73
};

struct SPacketGCLoginKey : SNetPacket
{
	uint32_t dwLoginKey{};

	NET_DECLARE_PACKET(HEADER_GC_LOGIN_KEY, SPacketGCLoginKey); // 74
};

struct SPacketGCRefineInformation : SNetPacket
{
	uint8_t			type{};
	uint8_t			pos{};
	TRefineTable	refine_table{};

	NET_DECLARE_PACKET(HEADER_GC_REFINE_INFORMATION, SPacketGCRefineInformation); // 75
};

struct SPacketGCChannel : SNetPacket
{
	uint8_t channel{};

	NET_DECLARE_PACKET(HEADER_GC_CHANNEL, SPacketGCChannel); // 76
};

struct SPacketGCTargetUpdate : SNetPacket
{
	int32_t lID{};
	int32_t lX{}, lY{};

	NET_DECLARE_PACKET(HEADER_GC_TARGET_UPDATE, SPacketGCTargetUpdate); // 77
};

struct SPacketGCTargetDelete : SNetPacket
{
	int32_t lID{};

	NET_DECLARE_PACKET(HEADER_GC_TARGET_DELETE, SPacketGCTargetDelete); // 78
};

struct SPacketGCTargetCreate : SNetPacket
{
	int32_t		lID{};
	char		szTargetName[32 + 1]{};
	uint32_t	dwVID{};
	uint8_t		byType{};

	NET_DECLARE_PACKET(HEADER_GC_TARGET_CREATE, SPacketGCTargetCreate); // 79
};

struct SPacketGCAffectAdd : SNetPacket
{
	TPacketAffectElement elem{};

	NET_DECLARE_PACKET(HEADER_GC_AFFECT_ADD, SPacketGCAffectAdd); // 80
};

struct SPacketGCAffectRemove : SNetPacket
{
	uint32_t dwType{};
	uint8_t  bApplyOn{};

	NET_DECLARE_PACKET(HEADER_GC_AFFECT_REMOVE, SPacketGCAffectRemove); // 81
};

struct SPacketGCMallSet : SNetPacket
{
	TItemPos	Cell{};
	uint32_t	vnum{};
	uint8_t		count{};
	int32_t		alSockets[ITEM_SOCKET_SLOT_MAX_NUM]{};
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM]{};

	NET_DECLARE_PACKET(HEADER_GC_MALL_SET, SPacketGCMallSet); // 82
};

struct SPacketGCMallDel : SNetPacket
{
	uint8_t pos{};

	NET_DECLARE_PACKET(HEADER_GC_MALL_DEL, SPacketGCMallDel); // 83
};

struct SPacketGCLandList : SNetPacket
{
	uint16_t size{};

	NET_DECLARE_PACKET(HEADER_GC_LAND_LIST, SPacketGCLandList); // 84
};

struct SPacketGCLoverInfo : SNetPacket
{
	char    szName[CHARACTER_NAME_MAX_LEN + 1]{};
	uint8_t byLovePoint{};

	NET_DECLARE_PACKET(HEADER_GC_LOVER_INFO, SPacketGCLoverInfo); // 85
};

struct SPacketGCLovePointUpdate : SNetPacket
{
	uint8_t byLovePoint{};

	NET_DECLARE_PACKET(HEADER_GC_LOVE_POINT_UPDATE, SPacketGCLovePointUpdate); // 86
};

struct SPacketGCGuildSymbolData : SNetPacket
{
	uint16_t size{};
	uint32_t guild_id{};

	NET_DECLARE_PACKET(HEADER_GC_GUILD_SYMBOL_DATA, SPacketGCGuildSymbolData); // 87
};

struct SPacketGCDigMotion : SNetPacket
{
	uint32_t vid{};
	uint32_t target_vid{};
	uint8_t count{};

	NET_DECLARE_PACKET(HEADER_GC_DIG_MOTION, SPacketGCDigMotion); // 88
};

struct SPacketGCDamageInfo : SNetPacket
{
	uint32_t dwVID{};
	uint8_t flag{};
	int32_t  damage{};

	NET_DECLARE_PACKET(HEADER_GC_DAMAGE_INFO, SPacketGCDamageInfo); // 89
};

struct SPacketGCCharacterAdditionalInfo : SNetPacket
{
	uint32_t   dwVID{};
	char    name[CHARACTER_NAME_MAX_LEN + 1]{};
	uint16_t    awPart[CHR_EQUIPPART_NUM]{};
	uint8_t	bEmpire{};
	uint32_t   dwGuildID{};
	uint32_t   dwLevel{};
	int16_t   sAlignment{};
	uint8_t    bPKMode{};
	uint32_t   dwMountVnum{};

	NET_DECLARE_PACKET(HEADER_GC_CHAR_ADDITIONAL_INFO, SPacketGCCharacterAdditionalInfo); // 90
};

struct SPacketGCMainCharacter3_BGM : SNetPacket
{
	uint32_t    dwVID{};
	uint16_t	wRaceNum{};
	char        szUserName[CHARACTER_NAME_MAX_LEN + 1]{};
	char        szBGMName[MUSIC_NAME_MAX_LEN + 1]{};
	int32_t     lX{}, lY{}, lZ{};
	uint8_t		byEmpire{};
	uint8_t		bySkillGroup{};

	NET_DECLARE_PACKET(HEADER_GC_MAIN_CHARACTER3_BGM, SPacketGCMainCharacter3_BGM); // 91
};

struct SPacketGCMainCharacter4_BGM_VOL : SNetPacket
{
	uint32_t    dwVID{};
	uint16_t	wRaceNum{};
	char        szUserName[CHARACTER_NAME_MAX_LEN + 1]{};
	char        szBGMName[MUSIC_NAME_MAX_LEN + 1]{};
	float		fBGMVol{};
	int32_t     lX{}, lY{}, lZ{};
	uint8_t		byEmpire{};
	uint8_t		bySkillGroup{};

	NET_DECLARE_PACKET(HEADER_GC_MAIN_CHARACTER4_BGM_VOL, SPacketGCMainCharacter4_BGM_VOL); // 92
};

struct SPacketGCAuthSuccess : SNetPacket
{
	uint32_t dwLoginKey{};
	uint8_t  bResult{};

	NET_DECLARE_PACKET(HEADER_GC_AUTH_SUCCESS, SPacketGCAuthSuccess); // 93
};

struct SPacketGCSpecificEffect : SNetPacket
{
	uint32_t vid{};
	char effect_file[MAX_EFFECT_FILE_NAME]{};

	NET_DECLARE_PACKET(HEADER_GC_SPECIFIC_EFFECT, SPacketGCSpecificEffect); // 94
};

struct SPacketGCDragonSoulRefine : SNetPacket
{
	uint8_t bSubType{};
	TItemPos Pos{};

	NET_DECLARE_PACKET(HEADER_GC_DRAGON_SOUL_REFINE, SPacketGCDragonSoulRefine); // 95
};

// HEADER_GC_RESPOND_CHANNELSTATUS = 96, has not contain a actual struct(buffered packet)

struct SPacketGCCheatBlacklist : SNetPacket
{
	char	 content[1024]{};
	uint32_t content_size{};

	NET_DECLARE_PACKET(HEADER_GC_CHEAT_BLACKLIST, SPacketGCCheatBlacklist); // 97
};

struct SPacketGCTargetDrop : SNetPacket
{
	uint32_t		dwVID{};
	uint8_t			size{};
	int32_t			drop[60]{};
	int32_t			bonuses{};

	NET_DECLARE_PACKET(HEADER_GC_TARGET_DROP, SPacketGCTargetDrop); // 98
};

struct SPacketGCChestDropInfo : SNetPacket
{
	uint16_t wSize{};
	uint32_t dwChestVnum{};

	NET_DECLARE_PACKET(HEADER_GC_CHEST_DROP_INFO, SPacketGCChestDropInfo); // 99
};

struct SPacketGCMallOpen : SNetPacket
{
	uint8_t bSize{};

	NET_DECLARE_PACKET(HEADER_GC_MALL_OPEN, SPacketGCMallOpen); // 100
};

struct SPacketGCSkillLevel : SNetPacket
{
	TPlayerSkill skills[SKILL_MAX_NUM]{};

	NET_DECLARE_PACKET(HEADER_GC_SKILL_LEVEL, SPacketGCSkillLevel); // 101
};

struct SPacketGCGuildDiscordInfo : SNetPacket
{
	uint32_t dwMemberCount{};
	uint32_t dwMaxMemberCount{};
	char szName[GUILD_NAME_MAX_LEN]{};
	
	NET_DECLARE_PACKET(HEADER_GC_GUILD_DCINFO, SPacketGCGuildDiscordInfo); // 103
};

struct SPacketGCPhase : SNetPacket
{
	uint8_t phase{};
	uint8_t stage{};

	NET_DECLARE_PACKET(HEADER_GC_PHASE, SPacketGCPhase); // 251
};

struct SPacketGCKeyAgreementCompleted : SNetPacket
{
	uint8_t data[3]{}; // dummy (not used)

	NET_DECLARE_PACKET(HEADER_GC_KEY_AGREEMENT_COMPLETED, SPacketGCKeyAgreementCompleted); // 252
};

struct SPacketGCHandshakeOK : SNetPacket
{
	NET_DECLARE_PACKET(HEADER_GC_HANDSHAKE_OK, SPacketGCHandshakeOK); // 253
};

// GC - Sub headers

struct TPacketCGGuildUseSkill
{
	uint32_t	dwVnum;
	uint32_t	dwPID;
};

// Messenger
struct TPacketGCMessengerListOffline
{
	uint8_t connected; // always 0
	uint8_t length;
};

struct TPacketGCMessengerListOnline
{
	uint8_t connected;
	uint8_t length;
	//uint8_t length_char_name;
};

struct TPacketGCMessengerLogin
{
	//uint8_t length_login;
	//uint8_t length_char_name;
	uint8_t length;
};

struct TPacketGCMessengerLogout
{
	uint8_t length;
};

// Player
struct TPacketGCDestroyCharacterSuccess
{
	uint8_t        header;
	uint8_t        account_index;
};

// Private Shop
struct TPacketGCShopStart
{
	uint32_t   owner_vid;
	struct packet_shop_item		items[SHOP_HOST_ITEM_MAX_NUM];
};

struct TPacketGCShopStartEx
{
//	TSubPacketShopTab tab;
	uint32_t owner_vid;
	uint8_t shop_tab_count;
};

struct TPacketGCShopUpdateItem
{
	uint8_t						pos;
	struct packet_shop_item		item;
};

struct TPacketGCShopUpdatePrice
{
	int32_t iElkAmount;
};

// Guild
struct TPacketGCGuildSubGrade
{
	char grade_name[GUILD_GRADE_NAME_MAX_LEN + 1]; // 8+1 길드장, 길드원 등의 이름
	uint8_t auth_flag;
};

struct TPacketGCGuildSubMember
{
	uint32_t pid;
	uint8_t byGrade;
	uint8_t byIsGeneral;
	uint8_t byJob;
	uint8_t byLevel;
	uint32_t dwOffer;
	uint8_t byNameFlag;
	// if NameFlag is TRUE, name is sent from server.
	//	char szName[CHARACTER_ME_MAX_LEN+1];
};

struct TPacketGCGuildInfo
{
	uint16_t member_count;
	uint16_t max_member_count;
	uint32_t guild_id;
	uint32_t master_pid;
	uint32_t exp;
	uint8_t level;
	char name[GUILD_NAME_MAX_LEN + 1];
	uint32_t gold;
	uint8_t hasLand;
};

struct TPacketGCGuildWar
{
	uint32_t       dwGuildSelf;
	uint32_t       dwGuildOpp;
	uint8_t        bType;
	uint8_t        bWarState;
};

struct SPacketGCGuildName
{
	uint8_t header;
	uint16_t size;
	uint8_t subheader;
	uint32_t	guildID;
	char	guildName[GUILD_NAME_MAX_LEN];
};

struct TPacketGuildWarPoint
{
	uint32_t dwGainGuildID;
	uint32_t dwOpponentGuildID;
	int32_t lPoint;
};
// =========================================================================================================================


// Game To Game
// =========================================================================================================================


typedef struct SPacketGGSetup
{
	uint8_t	bHeader;
	uint16_t	wPort;
	uint16_t	wListenPort;
	uint8_t	bChannel;
} TPacketGGSetup;

typedef struct SPacketGGLogin
{
	uint8_t	bHeader;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	uint32_t	dwPID;
	uint8_t	bEmpire;
	int32_t	lMapIndex;
	uint8_t	bChannel;
	int32_t		iLevel;
} TPacketGGLogin;

typedef struct SPacketGGLogout
{
	uint8_t	bHeader;
	uint32_t	dwPID;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGGLogout;

typedef struct SPacketGGRelay
{
	uint8_t	bHeader;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	int32_t	lSize;
} TPacketGGRelay;

typedef struct SPacketGGNotice
{
	uint8_t	bHeader;
	int32_t	lSize;
} TPacketGGNotice;

//FORKED_ROAD
typedef struct SPacketGGForkedMapInfo
{
	uint8_t	bHeader;
	uint8_t	bPass;
	uint8_t	bSungzi;
} TPacketGGForkedMapInfo;
//END_FORKED_ROAD
typedef struct SPacketGGShutdown
{
	uint8_t	bHeader;
} TPacketGGShutdown;

typedef struct SPacketGGGuild
{
	uint8_t	bHeader;
	uint8_t	bSubHeader;
	uint32_t	dwGuild;
} TPacketGGGuild;

typedef struct SPacketGGGuildChat
{
	uint8_t	bHeader;
	uint8_t	bSubHeader;
	uint32_t	dwGuild;
	char	szText[CHAT_MAX_LEN + 1];
} TPacketGGGuildChat;

typedef struct SPacketGGParty
{
	uint8_t	header;
	uint8_t	subheader;
	uint32_t	pid;
	uint32_t	leaderpid;
} TPacketGGParty;

typedef struct SPacketGGDisconnect
{
	uint8_t	bHeader;
	char	szLogin[LOGIN_MAX_LEN + 1];
} TPacketGGDisconnect;

typedef struct SPacketGGShout
{
	uint8_t	bHeader;
	uint8_t	bEmpire;
	char	szText[CHAT_MAX_LEN + 1];
} TPacketGGShout;

typedef struct SPacketGGXmasWarpSanta
{
	uint8_t	bHeader;
	uint8_t	bChannel;
	int32_t	lMapIndex;
} TPacketGGXmasWarpSanta;

typedef struct SPacketGGXmasWarpSantaReply
{
	uint8_t	bHeader;
	uint8_t	bChannel;
} TPacketGGXmasWarpSantaReply;

typedef struct SPacketGGMessenger
{
	uint8_t        bHeader;
	char        szAccount[CHARACTER_NAME_MAX_LEN + 1];
	char        szCompanion[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGGMessenger;

typedef struct SPacketGGFindPosition
{
	uint8_t header;
	uint32_t dwFromPID; // 저 위치로 워프하려는 사람
	uint32_t dwTargetPID; // 찾는 사람
} TPacketGGFindPosition;

typedef struct SPacketGGWarpCharacter
{
	uint8_t header;
	uint32_t pid;
	int32_t x;
	int32_t y;
	int32_t mapIndex;
} TPacketGGWarpCharacter;

typedef struct SPacketGGGuildWarMapIndex
{
	uint8_t bHeader;
	uint32_t dwGuildID1;
	uint32_t dwGuildID2;
	int32_t lMapIndex;
} TPacketGGGuildWarMapIndex;

typedef struct SPacketGGTransfer
{
	uint8_t	bHeader;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	int32_t	lX, lY;
} TPacketGGTransfer;

typedef struct SPacketGGLoginPing
{
	uint8_t	bHeader;
	char	szLogin[LOGIN_MAX_LEN + 1];
} TPacketGGLoginPing;

typedef struct SPacketGGBlockChat
{
	uint8_t	bHeader;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	int32_t	lBlockDuration;
} TPacketGGBlockChat;

typedef struct SPacketGGCheckAwakeness
{
	uint8_t bHeader;
} TPacketGGCheckAwakeness;

typedef struct SPacketGGReloadCommand
{
	uint8_t	header;
	char	argument[10];
} TPacketGGReloadCommand;

typedef struct SPacketGGMessengerRequest
{
	uint8_t		bHeader;
	char		szRequestor[CHARACTER_NAME_MAX_LEN + 1];
	uint32_t		dwTargetPID;
} TPacketGGMessengerRequest;
// =========================================================================================================================

// Common
// =========================================================================================================================

struct SPacketKeyAgreement : SNetPacket
{
	static const int32_t MAX_DATA_LEN = 256;

	uint16_t wAgreedLength{};
	uint16_t wDataLength{};
	uint8_t  data[MAX_DATA_LEN]{};

	NET_DECLARE_PACKET(HEADER_KEY_AGREEMENT, SPacketKeyAgreement); // 254
};

struct SPacketHandshake : SNetPacket
{
	uint32_t	dwHandshake{};
	uint32_t	dwTime{};
	int32_t		lDelta{};

	NET_DECLARE_PACKET(HEADER_HANDSHAKE, SPacketHandshake); // 255
};

// =========================================================================================================================
#if 0
static uint32_t GetPacketLength(TPacketHeader header, uint8_t type)
{
	if (type == PACKET_TYPE_CG)
	{
		switch (header)
		{
			case HEADER_CG_LOGIN:
				return sizeof(SPacketCGLogin);
			case HEADER_CG_ATTACK:
				return sizeof(SPacketCGAttack);
		}
	}

	else if (type == PACKET_TYPE_GC)
	{
	}

	else if (type == PACKET_TYPE_GG)
	{
	}

	else if (type == PACKET_TYPE_GD)
	{
	}

	else if (type == PACKET_TYPE_DG)
	{
	}

	else
	{
		assert(!"Unknown packet type");
	}

	assert(!"Unknown packet header");
	return 0U;
}
#endif

#pragma pack(pop)
