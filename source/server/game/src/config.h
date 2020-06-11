
#ifndef __INC_METIN_II_GAME_CONFIG_H__
#define __INC_METIN_II_GAME_CONFIG_H__

enum
{
	ADDRESS_MAX_LEN = 15
};

enum ItemDestroyTime {ITEM_DESTROY_TIME_AUTOGIVE, ITEM_DESTROY_TIME_DROPGOLD, ITEM_DESTROY_TIME_DROPITEM, ITEM_DESTROY_TIME_MAX};

void config_init(const std::string& st_localeServiceName); // default "" is CONFIG

extern char sql_addr[256];

extern uint16_t mother_port;
extern uint16_t p2p_port;

extern char db_addr[ADDRESS_MAX_LEN + 1];
extern uint16_t db_port;

extern int32_t passes_per_sec;
extern int32_t save_event_second_cycle;
extern int32_t ping_event_second_cycle;
extern int32_t test_server;
extern bool	guild_mark_server;
extern uint8_t guild_mark_min_level;
extern bool	distribution_test_server;

extern bool	g_bNoMoreClient;
extern bool	g_bNoRegen;

// #ifdef ENABLE_NEWSTUFF
extern bool	g_bEmpireShopPriceTripleDisable;
extern bool g_bShoutAddonEnable;
extern bool g_bGlobalShoutEnable;
extern bool g_bDisablePrismNeed;
extern bool g_bDisableEmotionMask;
extern uint8_t g_bItemCountLimit;
extern uint32_t g_dwItemBonusChangeTime;
extern bool	g_bAllMountAttack;
extern bool	g_bEnableBootaryCheck;
extern bool	g_bGMHostCheck;
extern bool	g_bGuildInviteLimit;
extern bool	g_bGuildInfiniteMembers;
extern bool	g_bEnableSpeedHackCrash;
extern int32_t g_iStatusPointGetLevelLimit;
extern int32_t g_iStatusPointSetMaxValue;
extern int32_t g_iShoutLimitLevel;
extern uint32_t g_dwSkillBookNextReadMin;
extern uint32_t g_dwSkillBookNextReadMax;
extern std::string g_stProxyIP;
// extern int32_t g_iShoutLimitTime;
extern int32_t g_iDbLogLevel;
extern int32_t g_iSysLogLevel;
extern int32_t g_aiItemDestroyTime[ITEM_DESTROY_TIME_MAX];
extern bool	g_bDisableEmpireLanguageCheck;
// #endif

extern bool	g_bTrafficProfileOn;		///< true �̸� TrafficProfiler �� �Ҵ�.

extern uint8_t	g_bChannel;

extern bool	map_allow_find(int32_t index);
extern void	map_allow_copy(int32_t * pl, int32_t size);
extern bool	no_wander;

extern int32_t	g_iUserLimit;
extern time_t	g_global_time;

const char *	get_table_postfix();

extern std::string	g_stHostname;
extern std::string	g_stLocale;
extern std::string	g_stLocaleFilename;

extern char		g_szPublicIP[16];
extern char		g_szInternalIP[16];

extern int32_t (*check_name) (const char * str);

extern bool		g_bSkillDisable;

extern int32_t		g_iFullUserCount;
extern int32_t		g_iBusyUserCount;
extern void		LoadStateUserCount();

extern bool	g_bEmpireWhisper;

extern uint8_t	g_bAuthServer;

extern uint8_t	PK_PROTECT_LEVEL;

extern std::string	g_stAuthMasterIP;
extern uint16_t		g_wAuthMasterPort;

extern uint32_t	g_dwClientVersion;

extern std::string	g_stQuestDir;
//extern std::string	g_stQuestObjectDir;
extern std::set<std::string> g_setQuestObjectDir;


extern std::vector<std::string>	g_stAdminPageIP;
extern std::string	g_stAdminPagePassword;

extern int32_t	SPEEDHACK_LIMIT_COUNT;
extern int32_t 	SPEEDHACK_LIMIT_BONUS;

extern int32_t g_iSyncHackLimitCount;

extern int32_t g_server_id;
extern std::string g_strWebMallURL;

extern int32_t VIEW_RANGE;
extern int32_t VIEW_BONUS_RANGE;

extern bool g_bCheckMultiHack;
extern bool g_protectNormalPlayer;      // �����ڰ� "��ȭ���" �� �Ϲ������� �������� ����
extern bool g_noticeBattleZone;         // �߸����뿡 �����ϸ� �ȳ��޼����� �˷���

extern uint32_t g_GoldDropTimeLimitValue;
// #ifdef ENABLE_NEWSTUFF
extern uint32_t g_ItemDropTimeLimitValue;
extern uint32_t g_BoxUseTimeLimitValue;
extern uint32_t g_BuySellTimeLimitValue;
extern bool g_NoDropMetinStone;
extern bool g_NoMountAtGuildWar;
extern bool g_NoPotionsOnPVP;
// #endif

extern int32_t gPlayerMaxLevel;
extern int32_t gShutdownAge;
extern int32_t gShutdownEnable;	

extern bool gHackCheckEnable;
extern bool g_BlockCharCreation;

// missing begin
extern std::string g_stBlockDate;

extern int32_t g_iSpamBlockMaxLevel;
extern uint32_t g_uiSpamBlockScore;
extern uint32_t g_uiSpamBlockDuration;
extern uint32_t g_uiSpamReloadCycle;

extern void map_allow_log();
// missing end

#endif /* __INC_METIN_II_GAME_CONFIG_H__ */

