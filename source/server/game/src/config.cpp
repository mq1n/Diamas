#include "stdafx.h"
#include <sstream>
#ifndef _WIN32
#include <ifaddrs.h>
#endif

#include "constants.h"
#include "utils.h"
#include "log.h"
#include "desc.h"
#include "desc_manager.h"
#include "item_manager.h"
#include "p2p.h"
#include "char.h"
#include "ip_ban.h"
#include "war_map.h"
#include "locale_service.h"
#include "config.h"
#include "dev_log.h"
#include "db.h"
#include "skill_power.h"

using std::string;

uint8_t	g_bChannel = 0;
uint16_t	mother_port = 50080;
int32_t		passes_per_sec = 25;
uint16_t	db_port = 0;
uint16_t	p2p_port = 50900;
char	db_addr[ADDRESS_MAX_LEN + 1];
int32_t		save_event_second_cycle = passes_per_sec * 120;	// 3분
int32_t		ping_event_second_cycle = passes_per_sec * 60;
bool	g_bNoMoreClient = false;
bool	g_bNoRegen = false;
// #ifdef ENABLE_NEWSTUFF
bool	g_bEmpireShopPriceTripleDisable = false;
bool	g_bShoutAddonEnable = false;
bool	g_bGlobalShoutEnable = false;
bool	g_bDisablePrismNeed = false;
bool	g_bDisableEmotionMask = false;
uint8_t	g_bItemCountLimit = 200;
uint32_t	g_dwItemBonusChangeTime = 60;
bool	g_bAllMountAttack = false;
bool	g_bEnableBootaryCheck = false;
bool	g_bGMHostCheck = false;
bool	g_bGuildInfiniteMembers = false;
bool	g_bEnableSpeedHackCrash = false;
int32_t		g_iStatusPointGetLevelLimit = 90;
int32_t		g_iStatusPointSetMaxValue = 90;
int32_t		g_iShoutLimitLevel = 15;
// int32_t		g_iShoutLimitTime = 15;
int32_t		g_iDbLogLevel = 3;
int32_t		g_iSysLogLevel = 3;
int32_t		g_aiItemDestroyTime[ITEM_DESTROY_TIME_MAX] = {300, 150, 300}; // autoitem, dropgold, dropitem
bool	g_bDisableEmpireLanguageCheck = false;
uint32_t	g_dwSkillBookNextReadMin = 28800;
uint32_t	g_dwSkillBookNextReadMax = 43200;
std::string	g_stProxyIP = "";
// #endif

int32_t			g_bIsTestServer = 0;
int32_t			game_stage = STAGE_NULL;
bool		guild_mark_server = false;
uint8_t		guild_mark_min_level = 3;
bool		no_wander = false;
int32_t		g_iUserLimit = 32768;

char		g_szPublicIP[16] = "0";
char		g_szInternalIP[16] = "0";
bool		g_bSkillDisable = false;
int32_t			g_iFullUserCount = 1200;
int32_t			g_iBusyUserCount = 650;
bool		g_bEmpireWhisper = true;
uint8_t		g_bAuthServer = false;

uint32_t	g_dwClientVersion = 1215955205;

uint8_t		g_bPKProtectLevel = 15;

string	g_stAuthMasterIP;
uint16_t		g_wAuthMasterPort = 0;

string g_stHostname = "";

string g_stQuestDir = "./quest";
//string g_stQuestObjectDir = "./quest/object";
string g_stDefaultQuestObjectDir = "./quest/object";
std::set<string> g_setQuestObjectDir;

string g_stBlockDate = "30000705";

extern string g_stLocale;
int32_t SPEEDHACK_LIMIT_COUNT   = 50;
int32_t SPEEDHACK_LIMIT_BONUS   = 80;
int32_t g_iSyncHackLimitCount = 10;

bool	g_bDisableMovspeedHacklog = false;
double	g_dMovspeedHackThreshold = 0.75;

//시야 = VIEW_RANGE + VIEW_BONUS_RANGE
//VIEW_BONUSE_RANGE : 클라이언트와 시야 처리에서너무 딱 떨어질경우 문제가 발생할수있어 500CM의 여분을 항상준다.
int32_t VIEW_RANGE = 5000;
int32_t VIEW_BONUS_RANGE = 500;

int32_t g_server_id = 0;
string g_strWebMallURL = "www.metin2.de";

uint32_t g_uiSpamBlockDuration = 60 * 15; // 기본 15분
uint32_t g_uiSpamBlockScore = 100; // 기본 100점
uint32_t g_uiSpamReloadCycle = 60 * 10; // 기본 10분

bool		g_bCheckMultiHack = true;

int32_t			g_iSpamBlockMaxLevel = 10;

bool            g_protectNormalPlayer   = false;        // 범법자가 "평화모드" 인 일반유저를 공격하지 못함
bool            g_noticeBattleZone      = false;        // 중립지대에 입장하면 안내메세지를 알려줌

int32_t gPlayerMaxLevel = 99;
int32_t gShutdownAge = 0;
int32_t gShutdownEnable = 0;

/*
 * NOTE : 핵 체크 On/Off. CheckIn할때 false로 수정했으면 반드시 확인하고 고쳐놓을것!
 * 이걸로 생길수있는 똥은 책임안짐 ~ ity ~
 */
bool gHackCheckEnable = false;

bool g_BlockCharCreation = false;


bool is_string_true(const char * string)
{
	bool result = false;
	if (isdigit(*string))
	{
		str_to_bool(result, string);
		return result;
	}

	return (LOWER(*string) == 't');
}

static std::set<int32_t> s_set_map_allows;

bool map_allow_find(int32_t index)
{
	if (g_bAuthServer)
		return false;

	if (s_set_map_allows.find(index) == s_set_map_allows.end())
		return false;

	return true;
}

void map_allow_log()
{
	std::set<int32_t>::iterator i;

	for (i = s_set_map_allows.begin(); i != s_set_map_allows.end(); ++i)
		sys_log(0, "MAP_ALLOW: %d", *i);
}

void map_allow_add(int32_t index)
{
	if (map_allow_find(index) == true)
	{
		fprintf(stdout, "!!! FATAL ERROR !!! multiple MAP_ALLOW setting!!\n");
		exit(1);
	}

	fprintf(stdout, "MAP ALLOW %d\n", index);
	s_set_map_allows.insert(index);
}

void map_allow_copy(int32_t * pl, int32_t size)
{
	int32_t iCount = 0;
	std::set<int32_t>::iterator it = s_set_map_allows.begin();

	while (it != s_set_map_allows.end())
	{
		int32_t i = *(it++);
		*(pl++) = i;

		if (++iCount > size)
			break;
	}
}

bool GetIPInfo()
{
#ifndef _WIN32
	struct ifaddrs* ifaddrp = nullptr;

	if (0 != getifaddrs(&ifaddrp))
		return false;

	for( struct ifaddrs* ifap=ifaddrp ; nullptr != ifap ; ifap = ifap->ifa_next )
	{
		struct sockaddr_in * sai = (struct sockaddr_in *) ifap->ifa_addr;

		if (!ifap->ifa_netmask ||  // ignore if no netmask
				sai->sin_addr.s_addr == 0 || // ignore if address is 0.0.0.0
				sai->sin_addr.s_addr == 16777343) // ignore if address is 127.0.0.1
			continue;
#else
	WSADATA wsa_data;
	char host_name[100];
	HOSTENT* host_ent;
	int32_t n = 0;

	if (WSAStartup(0x0101, &wsa_data))
		return false;

	gethostname(host_name, sizeof(host_name));
	host_ent = gethostbyname(host_name);
	if (host_ent == nullptr)
		return false;

	for (; host_ent->h_addr_list[n] != nullptr; ++n)
	{
		struct sockaddr_in addr;
		struct sockaddr_in* sai = &addr;
		memcpy(&sai->sin_addr.s_addr, host_ent->h_addr_list[n], host_ent->h_length);
#endif

		char * netip = inet_ntoa(sai->sin_addr);

		if (g_szPublicIP[0] == '0')
		{
			strlcpy(g_szPublicIP, netip, sizeof(g_szPublicIP));
#ifndef _WIN32
			fprintf(stderr, "PUBLIC_IP: %s interface %s\n", netip, ifap->ifa_name);
#else
			fprintf(stderr, "PUBLIC_IP: %s\n", netip);
#endif
		}
	}

#ifndef _WIN32
	freeifaddrs( ifaddrp );
#else
	WSACleanup();
#endif

	if (g_szPublicIP[0] != '0')
		return true;

	return false;
}

static bool __LoadConnectConfigFile(const char* configName)
	{
	char	buf[256];
	char	token_string[256];
	char	value_string[256];

	char db_host[2][64], db_user[2][64], db_pwd[2][64], db_db[2][64];
	// ... 아... db_port는 이미 있는데... 네이밍 어찌해야함...
	int32_t mysql_db_port[2];

	for (int32_t n = 0; n < 2; ++n)
	{
		*db_host[n]	= '\0';
		*db_user[n] = '\0';
		*db_pwd[n]= '\0';
		*db_db[n]= '\0';
		mysql_db_port[n] = 0;
	}

	char log_host[64], log_user[64], log_pwd[64], log_db[64];
	int32_t log_port = 0;

	*log_host = '\0';
	*log_user = '\0';
	*log_pwd = '\0';
	*log_db = '\0';


	// DB에서 로케일정보를 세팅하기위해서는 다른 세팅값보다 선행되어서
	// DB정보만 읽어와 로케일 세팅을 한후 다른 세팅을 적용시켜야한다.
	// 이유는 로케일관련된 초기화 루틴이 곳곳에 존재하기 때문.

	bool isCommonSQL = false;	
	bool isPlayerSQL = false;

	FILE* fpOnlyForDB;

	if (!(fpOnlyForDB = fopen(configName, "r")))
	{
		fprintf(stderr, "Can not open [%s]\n", configName);
		exit(1);
	}

	while (fgets(buf, 256, fpOnlyForDB))
	{
		parse_token(buf, token_string, value_string);

		TOKEN("hostname")
		{
			g_stHostname = value_string;
			fprintf(stdout, "HOSTNAME: %s\n", g_stHostname.c_str());
			continue;
		}

		TOKEN("channel")
		{
			str_to_number(g_bChannel, value_string);
			continue;
		}

		TOKEN("player_sql")
		{
			const char * line = two_arguments(value_string, db_host[0], sizeof(db_host[0]), db_user[0], sizeof(db_user[0]));
			line = two_arguments(line, db_pwd[0], sizeof(db_pwd[0]), db_db[0], sizeof(db_db[0]));

			if ('\0' != line[0])
			{
				char buf[256];
				one_argument(line, buf, sizeof(buf));
				str_to_number(mysql_db_port[0], buf);
			}

			if (!*db_host[0] || !*db_user[0] || !*db_pwd[0] || !*db_db[0])
			{
				fprintf(stderr, "PLAYER_SQL syntax: logsql <host user password db>\n");
				exit(1);
			}

			char buf[1024];
			snprintf(buf, sizeof(buf), "PLAYER_SQL: %s %s %s %s %d", db_host[0], db_user[0], db_pwd[0], db_db[0], mysql_db_port[0]);
			isPlayerSQL = true;
			continue;
		}

		TOKEN("common_sql")
		{
			const char * line = two_arguments(value_string, db_host[1], sizeof(db_host[1]), db_user[1], sizeof(db_user[1]));
			line = two_arguments(line, db_pwd[1], sizeof(db_pwd[1]), db_db[1], sizeof(db_db[1]));

			if ('\0' != line[0])
			{
				char buf[256];
				one_argument(line, buf, sizeof(buf));
				str_to_number(mysql_db_port[1], buf);
			}

			if (!*db_host[1] || !*db_user[1] || !*db_pwd[1] || !*db_db[1])
			{
				fprintf(stderr, "COMMON_SQL syntax: logsql <host user password db>\n");
				exit(1);
			}

			char buf[1024];
			snprintf(buf, sizeof(buf), "COMMON_SQL: %s %s %s %s %d", db_host[1], db_user[1], db_pwd[1], db_db[1], mysql_db_port[1]);
			isCommonSQL = true;
			continue;
		}

		TOKEN("log_sql")
		{
			const char * line = two_arguments(value_string, log_host, sizeof(log_host), log_user, sizeof(log_user));
			line = two_arguments(line, log_pwd, sizeof(log_pwd), log_db, sizeof(log_db));

			if ('\0' != line[0])
			{
				char buf[256];
				one_argument(line, buf, sizeof(buf));
				str_to_number(log_port, buf);
			}

			if (!*log_host || !*log_user || !*log_pwd || !*log_db)
			{
				fprintf(stderr, "LOG_SQL syntax: logsql <host user password db>\n");
				exit(1);
			}

			char buf[1024];
			snprintf(buf, sizeof(buf), "LOG_SQL: %s %s %s %s %d", log_host, log_user, log_pwd, log_db, log_port);
			continue;
		}
	}

	
	fclose(fpOnlyForDB);

	// CONFIG_SQL_INFO_ERROR
	if (!isCommonSQL)
	{
		puts("LOAD_COMMON_SQL_INFO_FAILURE:");
		puts("");
		puts("CONFIG:");
		puts("------------------------------------------------");
		puts("COMMON_SQL: HOST USER PASSWORD DATABASE");
		puts("");
		exit(1);
	}

	if (!isPlayerSQL)
	{
		puts("LOAD_PLAYER_SQL_INFO_FAILURE:");
		puts("");
		puts("CONFIG:");
		puts("------------------------------------------------");
		puts("PLAYER_SQL: HOST USER PASSWORD DATABASE");
		puts("");
		exit(1);
	}

	// Common DB 가 Locale 정보를 가지고 있기 때문에 가장 먼저 접속해야 한다.
	AccountDB::Instance().Connect(db_host[1], mysql_db_port[1], db_user[1], db_pwd[1], db_db[1]);

	if (false == AccountDB::Instance().IsConnected())
	{
		fprintf(stderr, "cannot start server while no common sql connected\n");
		exit(1);
	}

	fprintf(stdout, "CommonSQL connected\n");

	LocaleService_Init("turkey");

	// 로케일 정보를 COMMON SQL에 세팅해준다.
	// 참고로 g_stLocale 정보는 LocaleService_Init() 내부에서 세팅된다.
	fprintf(stdout, "Setting DB to locale %s\n", g_stLocale.c_str());

	AccountDB::Instance().SetLocale(g_stLocale);

	AccountDB::Instance().ConnectAsync(db_host[1], mysql_db_port[1], db_user[1], db_pwd[1], db_db[1], g_stLocale.c_str());

	// Player DB 접속
	DBManager::Instance().Connect(db_host[0], mysql_db_port[0], db_user[0], db_pwd[0], db_db[0]);

	if (!DBManager::Instance().IsConnected())
	{
		fprintf(stderr, "PlayerSQL.ConnectError\n");
		exit(1);
	}

	fprintf(stdout, "PlayerSQL connected\n");

	if (false == g_bAuthServer) // 인증 서버가 아닐 경우
	{
		// Log DB 접속
		LogManager::Instance().Connect(log_host, log_port, log_user, log_pwd, log_db);

		if (!LogManager::Instance().IsConnected())
		{
			fprintf(stderr, "LogSQL.ConnectError\n");
			exit(1);
		}

		fprintf(stdout, "LogSQL connected\n");

		LogManager::Instance().BootLog(g_stHostname.c_str(), g_bChannel);
	}

	// SKILL_POWER_BY_LEVEL
	// 스트링 비교의 문제로 인해서 AccountDB::Instance().SetLocale(g_stLocale) 후부터 한다.
	// 물론 국내는 별로 문제가 안된다(해외가 문제)
	{
		char szQuery[256];
		snprintf(szQuery, sizeof(szQuery), "SELECT mValue FROM locale WHERE mKey='SKILL_POWER_BY_LEVEL'");
		std::unique_ptr<SQLMsg> pMsg(AccountDB::Instance().DirectQuery(szQuery));

		if (pMsg->Get()->uiNumRows == 0)
		{
			fprintf(stderr, "[SKILL_PERCENT] Query failed: %s", szQuery);
			exit(1);
		}

		MYSQL_ROW row; 

		row = mysql_fetch_row(pMsg->Get()->pSQLResult);

		const char * p = row[0];
		int32_t cnt = 0;
		char num[128];
		int32_t aiBaseSkillPowerByLevelTable[SKILL_MAX_LEVEL+1];

		fprintf(stdout, "SKILL_POWER_BY_LEVEL %s\n", p);
		while (*p != '\0' && cnt < (SKILL_MAX_LEVEL + 1))
		{
			p = one_argument(p, num, sizeof(num));
			aiBaseSkillPowerByLevelTable[cnt++] = atoi(num);

			//fprintf(stdout, "%d %d\n", cnt - 1, aiBaseSkillPowerByLevelTable[cnt - 1]);
			if (*p == '\0')
			{
				if (cnt != (SKILL_MAX_LEVEL + 1))
				{
					fprintf(stderr, "[SKILL_PERCENT] locale table has not enough skill information! (count: %d query: %s)", cnt, szQuery);
					exit(1);
				}

				fprintf(stdout, "SKILL_POWER_BY_LEVEL: Done! (count %d)\n", cnt);
				break;
			}
		}

		// 종족별 스킬 세팅
		for (int32_t job = 0; job < JOB_MAX_NUM * 2; ++job)
		{
			snprintf(szQuery, sizeof(szQuery), "SELECT mValue from locale where mKey='SKILL_POWER_BY_LEVEL_TYPE%d' ORDER BY CAST(mValue AS unsigned)", job);
			std::unique_ptr<SQLMsg> pMsg(AccountDB::Instance().DirectQuery(szQuery));

			// 세팅이 안되어있으면 기본테이블을 사용한다.
			if (pMsg->Get()->uiNumRows == 0)
			{
				CTableBySkill::Instance().SetSkillPowerByLevelFromType(job, aiBaseSkillPowerByLevelTable);
				continue;
			}

			row = mysql_fetch_row(pMsg->Get()->pSQLResult);
			cnt = 0;
			p = row[0];
			int32_t aiSkillTable[SKILL_MAX_LEVEL + 1];

			fprintf(stdout, "SKILL_POWER_BY_JOB %d %s\n", job, p);
			while (*p != '\0' && cnt < (SKILL_MAX_LEVEL + 1))
			{			
				p = one_argument(p, num, sizeof(num));
				aiSkillTable[cnt++] = atoi(num);

				//fprintf(stdout, "%d %d\n", cnt - 1, aiBaseSkillPowerByLevelTable[cnt - 1]);
				if (*p == '\0')
				{
					if (cnt != (SKILL_MAX_LEVEL + 1))
					{
						fprintf(stderr, "[SKILL_PERCENT] locale table has not enough skill information! (count: %d query: %s)", cnt, szQuery);
						exit(1);
					}

					fprintf(stdout, "SKILL_POWER_BY_JOB: Done! (job: %d count: %d)\n", job, cnt);
					break;
				}
			}

			CTableBySkill::Instance().SetSkillPowerByLevelFromType(job, aiSkillTable);
		}		
	}
	// END_SKILL_POWER_BY_LEVEL

	// LOG_KEEP_DAYS_EXTEND
	log_set_expiration_days(2);
	// END_OF_LOG_KEEP_DAYS_EXTEND
	return true;
}

static bool __LoadDefaultConfigFile(const char* configName)
{
	FILE	*fp;

	char	buf[256];
	char	token_string[256];
	char	value_string[256];

	if (!(fp = fopen(configName, "r")))
		return false;

	while (fgets(buf, 256, fp))
	{
		parse_token(buf, token_string, value_string);

		TOKEN("port")
		{
			str_to_number(mother_port, value_string);
			continue;
		}

		TOKEN("p2p_port")
		{
			str_to_number(p2p_port, value_string);
			continue;
		}

		TOKEN("map_allow")
		{
			char * p = value_string;
			string stNum;

			for (; *p; p++)
			{
				if (isspace(*p))
				{
					if (stNum.length())
					{
						int32_t	index = 0;
						str_to_number(index, stNum.c_str());
						map_allow_add(index);
						stNum.clear();
					}
				}
				else
					stNum += *p;
			}

			if (stNum.length())
		{
				int32_t	index = 0;
				str_to_number(index, stNum.c_str());
				map_allow_add(index);
			}

			continue;
		}

		TOKEN("auth_server")
		{
			char szIP[32];
			char szPort[32];

			two_arguments(value_string, szIP, sizeof(szIP), szPort, sizeof(szPort));

			if (!*szIP || (!*szPort && strcasecmp(szIP, "master")))
			{
				fprintf(stderr, "AUTH_SERVER: syntax error: <ip|master> <port>\n");
				exit(1);
			}

			g_bAuthServer = true;

			LoadBanIP("BANIP");

			if (!strcasecmp(szIP, "master"))
				fprintf(stdout, "AUTH_SERVER: I am the master\n");
			else
		{
				g_stAuthMasterIP = szIP;
				str_to_number(g_wAuthMasterPort, szPort);

				fprintf(stdout, "AUTH_SERVER: master %s %u\n", g_stAuthMasterIP.c_str(), g_wAuthMasterPort);
			}
			continue;
		}
	}

	fclose(fp);
	return true;
}

static bool __LoadGeneralConfigFile(const char* configName)
{
	FILE	*fp;

	char	buf[256];
	char	token_string[256];
	char	value_string[256];

	if (!(fp = fopen(configName, "r")))
		return false;

	while (fgets(buf, 256, fp))
	{
		parse_token(buf, token_string, value_string);

		// DB_ONLY_BEGIN
		TOKEN("BLOCK_LOGIN")
		{
			g_stBlockDate = value_string;
		}
		// DB_ONLY_END

		// CONNECTION_BEGIN
		TOKEN("db_port")
		{
			str_to_number(db_port, value_string);
			continue;
		}

		TOKEN("db_addr")
		{
			strlcpy(db_addr, value_string, sizeof(db_addr));

			for (int32_t n =0; n < ADDRESS_MAX_LEN; ++n)
			{
				if (db_addr[n] == ' ')
					db_addr[n] = '\0';
			}

			continue;
		}
		// CONNECTION_END

		TOKEN("empire_whisper")
		{
			bool b_value = 0;
			str_to_number(b_value, value_string);
			g_bEmpireWhisper = !!b_value;
			continue;
		}

		TOKEN("mark_server")
		{
			guild_mark_server = is_string_true(value_string);
			continue;
		}

		TOKEN("mark_min_level")
		{
			str_to_number(guild_mark_min_level, value_string);
			guild_mark_min_level = MINMAX(0, guild_mark_min_level, GUILD_MAX_LEVEL);
			continue;
		}

		TOKEN("log_keep_days")
		{
			int32_t i = 0;
			str_to_number(i, value_string);
			log_set_expiration_days(MINMAX(1, i, 90));
			continue;
		}

		TOKEN("passes_per_sec")
		{
			str_to_number(passes_per_sec, value_string);
			continue;
		}

		TOKEN("save_event_second_cycle")
		{
			int32_t	cycle = 0;
			str_to_number(cycle, value_string);
			save_event_second_cycle = cycle * passes_per_sec;
			continue;
		}

		TOKEN("ping_event_second_cycle")
		{
			int32_t	cycle = 0;
			str_to_number(cycle, value_string);
			ping_event_second_cycle = cycle * passes_per_sec;
			continue;
		}

		TOKEN("test_server")
		{
			printf("-----------------------------------------------\n");
			printf("TEST_SERVER\n");
			printf("-----------------------------------------------\n");
			str_to_number(g_bIsTestServer, value_string);
			continue;
		}

		TOKEN("game_stage")
		{
			str_to_number(game_stage, value_string);
			continue;
		}

		TOKEN("shutdowned")
		{
			g_bNoMoreClient = true;
			continue;
		}

		TOKEN("no_regen")
		{
			g_bNoRegen = true;
			continue;
		}

		TOKEN("item_count_limit")
		{
			str_to_number(g_bItemCountLimit, value_string);
			fprintf(stdout, "ITEM_COUNT_LIMIT: %d\n", g_bItemCountLimit);
			continue;
		}

		TOKEN("disable_shop_price_3x")
		{
			g_bEmpireShopPriceTripleDisable = true;
			fprintf(stdout, "EMPIRE_SHOP_PRICE_3x: DISABLED\n");
			continue;
		}

		TOKEN("shop_price_3x_tax") //alternative
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bEmpireShopPriceTripleDisable = !flag;
			fprintf(stdout, "SHOP_PRICE_3X_TAX: %s\n", (!g_bEmpireShopPriceTripleDisable)?"ENABLED":"DISABLED");
			continue;
		}

		//unused
		TOKEN("enable_shout_addon")
		{
			g_bShoutAddonEnable = true;
			continue;
		}

		//unused
		TOKEN("enable_all_mount_attack")
		{
			g_bAllMountAttack = true;
			continue;
		}

		TOKEN("disable_change_attr_time")
		{
			g_dwItemBonusChangeTime = 0;
			fprintf(stdout, "CHANGE_ATTR_TIME_LIMIT: DISABLED\n");
			continue;
		}

		TOKEN("change_attr_time_limit") //alternative
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);
			g_dwItemBonusChangeTime = flag;
			fprintf(stdout, "CHANGE_ATTR_TIME_LIMIT: %u\n", g_dwItemBonusChangeTime);
			continue;
		}

		TOKEN("disable_prism_item")
		{
			g_bDisablePrismNeed = true;
			fprintf(stdout, "PRISM_ITEM_REQUIREMENT: DISABLED\n");
			continue;
		}

		TOKEN("prism_item_require") //alternative
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bDisablePrismNeed = !flag;
			fprintf(stdout, "PRISM_ITEM_REQUIRE: %s\n", (!g_bDisablePrismNeed)?"ENABLED":"DISABLED");
			continue;
		}

		TOKEN("enable_global_shout")
		{
			g_bGlobalShoutEnable = true;
			fprintf(stdout, "GLOBAL_SHOUT: ENABLED\n");
			continue;
		}

		TOKEN("global_shout") //alternative
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bGlobalShoutEnable = !!flag;
			fprintf(stdout, "GLOBAL_SHOUT: %s\n", (g_bGlobalShoutEnable)?"ENABLED":"DISABLED");
			continue;
		}

		TOKEN("disable_emotion_mask")
		{
			g_bDisableEmotionMask = true;
			fprintf(stdout, "EMOTION_MASK_REQUIREMENT: DISABLED\n");
			continue;
		}

		TOKEN("emotion_mask_require") //alternative
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bDisableEmotionMask = !flag;
			fprintf(stdout, "EMOTION_MASK_REQUIRE: %s\n", (g_bDisableEmotionMask)?"ENABLED":"DISABLED");
			continue;
		}

		TOKEN("enable_bootary_check")
		{
			g_bEnableBootaryCheck = true;
			fprintf(stdout, "ENABLE_BOOTARY_CHECK: ENABLED\n");
			continue;
		}

		TOKEN("bootary_check") //alternative
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bEnableBootaryCheck = !!flag;
			fprintf(stdout, "BOOTARY_CHECK: %s\n", (g_bEnableBootaryCheck)?"ENABLED":"DISABLED");
			continue;
		}

		TOKEN("status_point_get_level_limit")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			if (flag <= 0) continue;

			g_iStatusPointGetLevelLimit = MINMAX(0, flag, PLAYER_MAX_LEVEL_CONST);
			fprintf(stdout, "STATUS_POINT_GET_LEVEL_LIMIT: %d\n", g_iStatusPointGetLevelLimit);
			continue;
		}

		TOKEN("status_point_set_max_value")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			if (flag <= 0) continue;

			g_iStatusPointSetMaxValue = flag;
			fprintf(stdout, "STATUS_POINT_SET_MAX_VALUE: %d\n", g_iStatusPointSetMaxValue);
			continue;
		}

		TOKEN("shout_limit_level")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			if (flag <= 0) continue;

			g_iShoutLimitLevel = flag;
			fprintf(stdout, "SHOUT_LIMIT_LEVEL: %d\n", g_iShoutLimitLevel);
			continue;
		}

		TOKEN("db_log_level")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);

			g_iDbLogLevel = flag;
			fprintf(stdout, "DB_LOG_LEVEL: %d\n", g_iDbLogLevel);
			continue;
		}

		TOKEN("sys_log_level")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);

			g_iSysLogLevel = flag;
			fprintf(stdout, "SYS_LOG_LEVEL: %d\n", g_iSysLogLevel);
			continue;
		}

		TOKEN("item_destroy_time_autogive")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);

			g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE] = flag;
			fprintf(stdout, "ITEM_DESTROY_TIME_AUTOGIVE: %d\n", g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE]);
			continue;
		}

		TOKEN("item_destroy_time_dropgold")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);

			g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPGOLD] = flag;
			fprintf(stdout, "ITEM_DESTROY_TIME_DROPGOLD: %d\n", g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPGOLD]);
			continue;
		}

		TOKEN("item_destroy_time_dropitem")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);

			g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPITEM] = flag;
			fprintf(stdout, "ITEM_DESTROY_TIME_DROPITEM: %d\n", g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPITEM]);
			continue;
		}

		// TOKEN("shout_limit_time")
		// {
			// int32_t flag = 0;
			// str_to_number(flag, value_string);
			// if (flag <= 0) continue;

			// g_iShoutLimitTime = flag;
			// fprintf(stdout, "SHOUT_LIMIT_TIME: %d\n", g_iShoutLimitTime);
			// continue;
		// }

		TOKEN("check_version_value")
		{
			uint32_t version = 0;
			str_to_number(version, value_string);

			g_dwClientVersion = version;
			fprintf(stdout, "CHECK_VERSION_VALUE: %u\n", g_dwClientVersion);
			continue;
		}

		TOKEN("enable_hack_check")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);

			gHackCheckEnable = !!flag;
			fprintf(stdout, "ENABLE_HACK_CHECK: %d\n", gHackCheckEnable);
			continue;
		}

		TOKEN("gm_host_check")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);

			g_bGMHostCheck = !!flag;
			fprintf(stdout, "GM_HOST_CHECK: %d\n", g_bGMHostCheck);
			continue;
		}

		TOKEN("guild_infinite_members")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);

			g_bGuildInfiniteMembers = !!flag;
			fprintf(stdout, "GUILD_INFINITE_MEMBERS: %d\n", g_bGuildInfiniteMembers);
			continue;
		}

		TOKEN("disable_movspeed_hacklog")
		{
			str_to_number(g_bDisableMovspeedHacklog, value_string);
			continue;
		}

		TOKEN("movspeed_hack_threshold")
		{
			str_to_number(g_dMovspeedHackThreshold, value_string);
			continue;
		}

		TOKEN("empire_language_check")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bDisableEmpireLanguageCheck = !flag;
			fprintf(stdout, "EMPIRE_LANGUAGE_CHECK: %s\n", (g_bDisableEmpireLanguageCheck)?"DISABLED":"ENABLED");
			continue;
		}

		TOKEN("skillbook_nextread_min")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);
			g_dwSkillBookNextReadMin = flag;
			fprintf(stdout, "SKILLBOOK_NEXTREAD_MIN: %u\n", g_dwSkillBookNextReadMin);
			continue;
		}

		TOKEN("skillbook_nextread_max")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);
			g_dwSkillBookNextReadMax = flag;
			fprintf(stdout, "SKILLBOOK_NEXTREAD_MAX: %u\n", g_dwSkillBookNextReadMax);
			continue;
		}

		TOKEN("proxy_ip")
		{
			g_stProxyIP = value_string;
		}

		TOKEN("no_wander")
		{
			no_wander = true;
			continue;
		}

		TOKEN("user_limit")
		{
			str_to_number(g_iUserLimit, value_string);
			continue;
		}

		TOKEN("skill_disable")
		{
			str_to_number(g_bSkillDisable, value_string);
			continue;
		}

		TOKEN("quest_dir")
		{
			sys_log(0, "QUEST_DIR SETTING : %s", value_string);
			g_stQuestDir = value_string;
		}

		TOKEN("quest_object_dir")
		{
			//g_stQuestObjectDir = value_string;
			std::istringstream is(value_string);
			sys_log(0, "QUEST_OBJECT_DIR SETTING : %s", value_string);
			string dir;
			while (!is.eof())
			{
				is >> dir;
				if (is.fail())
					break;
				g_setQuestObjectDir.insert(dir);
				sys_log(0, "QUEST_OBJECT_DIR INSERT : %s", dir .c_str());
			}
		}

		TOKEN("synchack_limit_count")
		{
			str_to_number(g_iSyncHackLimitCount, value_string);
		}

		TOKEN("speedhack_limit_count")
		{
			str_to_number(SPEEDHACK_LIMIT_COUNT, value_string);
		}

		TOKEN("speedhack_limit_bonus")
		{
			str_to_number(SPEEDHACK_LIMIT_BONUS, value_string);
		}

		TOKEN("server_id")
		{
			str_to_number(g_server_id, value_string);
		}

		TOKEN("mall_url")
		{
			g_strWebMallURL = value_string;
		}

		TOKEN("bind_ip")
		{
			strlcpy(g_szPublicIP, value_string, sizeof(g_szPublicIP));
		}

		TOKEN("view_range")
		{
			str_to_number(VIEW_RANGE, value_string);
		}

		TOKEN("spam_block_duration")
		{
			str_to_number(g_uiSpamBlockDuration, value_string);
		}

		TOKEN("spam_block_score")
		{
			str_to_number(g_uiSpamBlockScore, value_string);
			g_uiSpamBlockScore = MAX(1, g_uiSpamBlockScore);
		}

		TOKEN("spam_block_reload_cycle")
		{
			str_to_number(g_uiSpamReloadCycle, value_string);
			g_uiSpamReloadCycle = MAX(60, g_uiSpamReloadCycle); // 최소 1분
		}

		TOKEN("check_multihack")
		{
			str_to_number(g_bCheckMultiHack, value_string);
		}

		TOKEN("spam_block_max_level")
		{
			str_to_number(g_iSpamBlockMaxLevel, value_string);
		}
		TOKEN("protect_normal_player")
		{
			str_to_number(g_protectNormalPlayer, value_string);
		}
		TOKEN("notice_battle_zone")
		{
			str_to_number(g_noticeBattleZone, value_string);
		}

		TOKEN("pk_protect_level")
		{
		    str_to_number(g_bPKProtectLevel, value_string);
		    fprintf(stderr, "PK_PROTECT_LEVEL: %d", g_bPKProtectLevel);
		}

		TOKEN("max_level")
		{
			str_to_number(gPlayerMaxLevel, value_string);

			gPlayerMaxLevel = MINMAX(1, gPlayerMaxLevel, PLAYER_MAX_LEVEL_CONST);

			fprintf(stderr, "PLAYER_MAX_LEVEL: %d\n", gPlayerMaxLevel);
		}

		TOKEN("shutdown_age")
		{
			str_to_number(gShutdownAge, value_string);
			fprintf(stderr, "SHUTDOWN_AGE: %d\n", gShutdownAge);

		}

		TOKEN("shutdown_enable")
		{
			str_to_number(gShutdownEnable, value_string);
			fprintf(stderr, "SHUTDOWN_ENABLE: %d\n", gShutdownEnable);
		}

		TOKEN("block_char_creation")
		{
			int32_t tmp = 0;

			str_to_number(tmp, value_string);

			if (0 == tmp)
				g_BlockCharCreation = false;
			else
				g_BlockCharCreation = true;

			continue;
		}
	}
	fclose(fp);
	return true;
	}

static bool __LoadDefaultCMDFile(const char* cmdName)
	{
	FILE	*fp;
	char	buf[256];

	if ((fp = fopen(cmdName, "r")))
	{
		while (fgets(buf, 256, fp))
		{
			char cmd[32], levelname[32];
			int32_t level;

			two_arguments(buf, cmd, sizeof(cmd), levelname, sizeof(levelname));

			if (!*cmd || !*levelname)
			{
				fprintf(stderr, "CMD syntax error: <cmd> <PLAYER | LOW_WIZARD | WIZARD | HIGH_WIZARD | GOD | IMPLEMENTOR | DISABLE>\n");
				exit(1);
			}

			if (!strcasecmp(levelname, "LOW_WIZARD"))
				level = GM_LOW_WIZARD;
			else if (!strcasecmp(levelname, "WIZARD"))
				level = GM_WIZARD;
			else if (!strcasecmp(levelname, "HIGH_WIZARD"))
				level = GM_HIGH_WIZARD;
			else if (!strcasecmp(levelname, "GOD"))
				level = GM_GOD;
			else if (!strcasecmp(levelname, "IMPLEMENTOR"))
				level = GM_IMPLEMENTOR;
			else if (!strcasecmp(levelname, "PLAYER"))
				level = GM_PLAYER;
			else
			{
				fprintf(stderr, "CMD syntax error: <cmd> <PLAYER | LOW_WIZARD | WIZARD | HIGH_WIZARD | GOD | IMPLEMENTOR | DISABLE>\n");
				exit(1);
			}

			if (g_bIsTestServer)
				fprintf(stdout, "CMD_REWRITE: [%s] [%s:%d]\n", cmd, levelname, level);
			interpreter_set_privilege(cmd, level);
		}

		fclose(fp);
		return true;
	}
	return false;
}

#ifdef ENABLE_EXPTABLE_FROMDB
static bool __LoadExpTableFromDB(void)
{
	std::unique_ptr<SQLMsg> pMsg(AccountDB::Instance().DirectQuery("SELECT level, exp FROM exp_table"));
	if (pMsg->Get()->uiNumRows == 0)
		return false;

	static uint32_t new_exp_table[PLAYER_MAX_LEVEL_CONST+1];
	if (exp_table != nullptr)
		memcpy(new_exp_table, exp_table, (PLAYER_MAX_LEVEL_CONST+1)*sizeof(uint32_t));

	MYSQL_ROW row = nullptr;
	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		uint32_t level = 0;
		uint32_t exp = 0;
		str_to_number(level, row[0]);
		str_to_number(exp, row[1]);
		if (level > PLAYER_MAX_LEVEL_CONST)
			continue;
		new_exp_table[level] = exp;
		// printf("new_exp_table[%u] = %u;\n", level, exp);
	}
	exp_table = new_exp_table;
	return true;
}
#endif

// #define ENABLE_GENERAL_CMD
// #define ENABLE_GENERAL_CONFIG
void config_init(const string& st_localeServiceName)
{
	// LOCALE_SERVICE
	string	st_configFileName;

	st_configFileName.reserve(32);
	st_configFileName = "CONFIG";

	if (!st_localeServiceName.empty())
	{
		st_configFileName += ".";
		st_configFileName += st_localeServiceName;
	}
	// END_OF_LOCALE_SERVICE

	
	
	if (!GetIPInfo())
	{
	//	fprintf(stderr, "Can not get public ip address\n");
	//	exit(1);
	}

	// default config load (REQUIRED)
	if (!__LoadConnectConfigFile(st_configFileName.c_str()) ||
		!__LoadDefaultConfigFile(st_configFileName.c_str()) ||
		!__LoadGeneralConfigFile(st_configFileName.c_str())
	)
	{
		fprintf(stderr, "Can not open [%s]\n", st_configFileName.c_str());
		exit(1);
	}

	if (g_setQuestObjectDir.empty())
		g_setQuestObjectDir.insert(g_stDefaultQuestObjectDir);

	if (0 == db_port)
	{
		fprintf(stderr, "DB_PORT not configured\n");
		exit(1);
	}

	if (0 == g_bChannel)
	{
		fprintf(stderr, "CHANNEL not configured\n");
		exit(1);
	}

	if (g_stHostname.empty())
	{
		fprintf(stderr, "HOSTNAME must be configured.\n");
		exit(1);
	}

	// LOCALE_SERVICE
	Locale_Init();
	LocaleService_TransferDefaultSetting();
	// END_OF_LOCALE_SERVICE

#ifdef ENABLE_EXPTABLE_FROMDB
	if (!__LoadExpTableFromDB())
	{
		// do as you please to manage this
		fprintf(stderr, "Failed to Load ExpTable from DB so exit\n");
		// exit(1);
	}
#endif

	std::string st_cmdFileName("CMD");
	__LoadDefaultCMDFile(st_cmdFileName.c_str());

	CWarMapManager::Instance().LoadWarMapInfo(nullptr);

	if (g_szPublicIP[0] == '0')
	{
		fprintf(stderr, "Can not get public ip address\n");
		exit(1);
	}
}

