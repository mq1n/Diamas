#include "stdafx.h"
#include "Peer.h"
#include "DBManager.h"
#include "ClientManager.h"
#include "GuildManager.h"
#include "ItemAwardManager.h"
#include "QID.h"
#include "PrivManager.h"
#include "Marriage.h"
#include "ItemIDRangeManager.h"
#include <signal.h>
#include "../../common/json_helper.h"
#include "../../libthecore/include/winminidump.h"

void SetPlayerDBName(const char* c_pszPlayerDBName);
void SetAccountDBName(const char* c_pszAccountDBName);
bool Start();

std::string g_stLocaleNameColumn = "name";
std::string g_stLocaleName = "turkey";
std::string g_stLocale = "latin5"; // default: euckr
std::string g_stPlayerDBName = "";
std::string g_stAccountDBName = "";

bool g_test_server = false;
bool g_log = true;

//단위 초
int32_t g_iPlayerCacheFlushSeconds = 60*7;
int32_t g_iItemCacheFlushSeconds = 60*5;
int32_t g_gwTimeDivisor = 1;
int32_t g_iLogoutSeconds = 60*10;
int32_t g_iItemPriceListTableCacheFlushSeconds = 360;
int32_t g_iActivityCacheFlushSeconds = 600;



extern void WriteVersion();

void emergency_sig(int32_t sig)
{
	if (sig == SIGSEGV)
		sys_log(0, "SIGNAL: SIGSEGV");
	else if (sig == SIGUSR1)
		sys_log(0, "SIGNAL: SIGUSR1");

	if (sig == SIGSEGV)
		abort();
}

int32_t main()
{
	if (setup_minidump_generator() == false)
		return 1;

	WriteVersion();

	CConfigManager configManager;
	CNetPoller poller;
	CDBManager DBManager; 
	CClientManager ClientManager;
	CGuildManager GuildManager;
	CPrivManager PrivManager;
	ItemAwardManager ItemAwardManager;
	marriage::CManager MarriageManager;
	CItemIDRangeManager ItemIDRangeManager;

#ifdef _DEBUG
	log_set_level(1);
#endif
	if (!Start())
		return 1;

	GuildManager.Initialize();
	MarriageManager.Initialize();
	ItemIDRangeManager.Build();

	sys_log(0, "DBCacheServer Start\n");

	CClientManager::Instance().MainLoop();

	signal_timer_disable();

	DBManager.Quit();
	int32_t iCount;

	for (;;)
	{
		iCount = 0;

		iCount += CDBManager::Instance().CountReturnQuery(SQL_PLAYER);
		iCount += CDBManager::Instance().CountAsyncQuery(SQL_PLAYER);

		if (iCount == 0)
			break;

		usleep(1000);
		fprintf(stderr, "WAITING_QUERY_COUNT %d\n", iCount);
	}

	return 1;
}

void emptybeat(LPHEART heart, int32_t pulse)
{
	if (!(pulse % heart->passes_per_sec))	// 1초에 한번
	{
	}
}

//
// @version	05/06/13 Bang2ni - 아이템 가격정보 캐시 flush timeout 설정 추가.
//
bool Start()
{
	int32_t nHeartBeatFPS = 50;
	int8_t cbDatabaseConnectRetries = 5;
	std::vector <std::string> pkVecDatabases = { "SQL_PLAYER", "SQL_ACCOUNT", "SQL_COMMON" };
	std::vector <std::string> pkVecConnectedDatabases;

	// Initialize lib-thecore
	thecore_init();

	// Parse config file
	if (!CConfigManager::Instance().ParseFile("CONFIG.json"))
	{
		sys_err("Loading CONFIG.json failed.");
		return false;		
	}

	auto stConfigBuffer = CConfigManager::Instance().GetConfigFileContent();
	if (stConfigBuffer.empty())
	{
		sys_err("nullptr CONFIG.json content.");
		return false;				
	}
//	sys_log(0, "%s", stConfigBuffer.c_str());

	Document document;
	document.Parse<rapidjson::kParseStopWhenDoneFlag>(stConfigBuffer.c_str());

	if (document.HasParseError())
	{
		sys_err("CONFIG.json parse failed. Error: %s offset: %u", GetParseError_En(document.GetParseError()), document.GetErrorOffset());
		return false;
	}

	if (!document.IsObject())
	{
		sys_err("CONFIG.json file base is NOT object");
		return false;
	}

	auto& pkStage = document["stage"];
	if (!pkStage.IsString())
		return false;
//	sys_log(0, "stage: %s", pkStage.GetString());

	auto& pkConfigContent = document[pkStage];
	if (!pkConfigContent.IsObject() || pkConfigContent.IsNull())
		return false;
//	sys_log(0, "config: %s", pkConfigContent.GetString());

	// Register config values
	auto& pkIsTestServer = pkConfigContent["test_server"];
	if (!pkIsTestServer.IsNull() && pkIsTestServer.IsBool())
	{
		g_test_server = pkIsTestServer.GetBool();
		sys_log(0, "%s Server", g_test_server ? "Test" : "Real");
	}

	auto& pkIsLogsEnabled = pkConfigContent["logs_enable"];
	if (!pkIsLogsEnabled.IsNull() && pkIsLogsEnabled.IsBool())
	{
		g_log = pkIsLogsEnabled.GetBool();
		sys_log(0, "Logs: %s", g_log ? "Enabled" : "Disabled");
	}

	auto& pkNumHeartbeat = pkConfigContent["heartbeat"];
	if (pkNumHeartbeat.IsNull() || !pkNumHeartbeat.IsNumber())
	{
		sys_err("Cannot find heartbeat configuration.");
		return false;
	}
	nHeartBeatFPS = pkNumHeartbeat.GetInt();

	auto& pkNumLogKeepDays = pkConfigContent["log_keep_days"];
	if (!pkNumLogKeepDays.IsNull() && pkNumLogKeepDays.IsNumber())
		log_set_expiration_days(pkNumLogKeepDays.GetInt());
	else
		log_set_expiration_days(3);

	thecore_set(nHeartBeatFPS, emptybeat);
	signal_timer_enable(60);

	auto& pkStrCharset = pkConfigContent["charset"];
	if (!pkStrCharset.IsNull() && pkStrCharset.IsString())
	{
		g_stLocale = pkStrCharset.GetString();
		sys_log(0, "LOCALE set to %s", g_stLocale.c_str());
	}

	// Quick gw time division for test server
	auto& pkNumGwTimeDivisor = pkConfigContent["guildwar_time_divisor"];
	if (!pkNumGwTimeDivisor.IsNull() && pkNumGwTimeDivisor.IsNumber())
	{
		g_gwTimeDivisor = pkNumGwTimeDivisor.GetInt();
		sys_log(0, "GUILDWAR_TIME_DIVISOR: %d", g_gwTimeDivisor);
	}

	auto& pkNumPlayerCacheDelay = pkConfigContent["player_cache_flush_seconds"];
	if (!pkNumPlayerCacheDelay.IsNull() && pkNumPlayerCacheDelay.IsNumber())
	{
		g_iPlayerCacheFlushSeconds = pkNumPlayerCacheDelay.GetInt();
		sys_log(0, "PLAYER_CACHE_FLUSH_SECONDS: %d", g_iPlayerCacheFlushSeconds);
	}

	auto& pkNumItemCacheDelay = pkConfigContent["item_cache_flush_seconds"];
	if (!pkNumItemCacheDelay.IsNull() && pkNumItemCacheDelay.IsNumber())
	{
		g_iItemCacheFlushSeconds = pkNumItemCacheDelay.GetInt();
		sys_log(0, "ITEM_CACHE_FLUSH_SECONDS: %d", g_iItemCacheFlushSeconds);
	}

	auto& pkNumPricelistCacheDelay = pkConfigContent["item_pricelist_cache_flush_seconds"];
	if (!pkNumPricelistCacheDelay.IsNull() && pkNumPricelistCacheDelay.IsNumber())
	{
		g_iItemPriceListTableCacheFlushSeconds = pkNumPricelistCacheDelay.GetInt();
		sys_log(0, "ITEM_PRICELIST_CACHE_FLUSH_SECONDS: %d", g_iItemPriceListTableCacheFlushSeconds);
	}

	auto& pkNumActivityCacheDelay = pkConfigContent["activity_cache_flush_seconds"];
	if (!pkNumActivityCacheDelay.IsNull() && pkNumActivityCacheDelay.IsNumber())
	{
		g_iActivityCacheFlushSeconds = pkNumActivityCacheDelay.GetInt();
		sys_log(0, "ACTIVITY_CACHE_FLUSH_SECONDS: %d", g_iActivityCacheFlushSeconds);
	}

	auto& pkNumCacheFlushLimit = pkConfigContent["cache_flush_limit_per_second"];
	if (!pkNumCacheFlushLimit.IsNull() && pkNumCacheFlushLimit.IsNumber())
		CClientManager::Instance().SetCacheFlushCountLimit(pkNumCacheFlushLimit.GetUint());

	auto& pkStrLocalName = pkConfigContent["locale_name"];
	if (!pkStrLocalName.IsNull() && pkStrLocalName.IsString())
	{
		g_stLocaleName = pkStrLocalName.GetString();
		sys_log(0, "LOCALE_NAME set to %s", g_stLocaleName.c_str());
	}

	auto& pkObjDatabaseTree = pkConfigContent["databases"];
	if (pkObjDatabaseTree.IsNull() || !pkObjDatabaseTree.IsObject())
	{
		sys_err("DATABASES not configured");
		return false;
	}

	for (auto i = 0U; i < pkVecDatabases.size(); ++i)
	{
		auto stDatabase = pkVecDatabases.at(i);

		auto& pkObjDatabase = pkObjDatabaseTree[stDatabase.c_str()];
		if (pkObjDatabase.IsNull() || !pkObjDatabase.IsObject())
		{
			sys_err("Database: %s not well configured.", stDatabase.c_str());
			return false;
		}
		sys_log(0, "connecting to MySQL server (%s)", stDatabase.c_str());

		auto& pkStrHost = pkObjDatabase["address"];
		auto& pkNumPort = pkObjDatabase["port"];
		auto& pkStrDatabase = pkObjDatabase["database"];
		auto& pkStrUsername = pkObjDatabase["user"];
		auto& pkStrPassword = pkObjDatabase["password"];

		if (pkStrHost.IsNull() || pkNumPort.IsNull() || pkStrDatabase.IsNull() || pkStrUsername.IsNull() || pkStrPassword.IsNull())
		{
			sys_err("Database: %s context not well configured.", stDatabase.c_str());
			return false;			
		}

		do
		{
			if (CDBManager::Instance().Connect(i, pkStrHost.GetString(), pkNumPort.GetInt(), pkStrDatabase.GetString(),
				pkStrUsername.GetString(), pkStrPassword.GetString()))
			{
				sys_log(0, "Connection to: %s: OK", stDatabase.c_str());
				pkVecConnectedDatabases.emplace_back(stDatabase);
				break;
			}

			sys_log(0, "Connection to: %s failed, retrying in 5 seconds", stDatabase.c_str());
			sleep(5);
		} while (cbDatabaseConnectRetries--);

		if (stDatabase == "SQL_PLAYER")
			SetPlayerDBName(pkStrDatabase.GetString());
		else if (stDatabase == "SQL_ACCOUNT")
			SetAccountDBName(pkStrDatabase.GetString());
	}

	if (pkVecConnectedDatabases.size() != pkVecDatabases.size())
	{
		sys_err("Cannot connected to a database");
		return false;		
	}
	
	if (!CNetPoller::Instance().Create())
	{
		sys_err("Cannot create network poller");
		return false;
	}

	sys_log(0, "ClientManager initializing...");

	if (!CClientManager::Instance().Initialize(stConfigBuffer))
	{
		sys_log(0, "ClientManager initialization failed.");
		return false;
	}

	sys_log(0, "ClientManager initialized.");

//#ifndef _DEBUG
	#ifndef _WIN32
		signal(SIGUSR1, emergency_sig);
	#endif
		signal(SIGSEGV, emergency_sig);
//#endif
	return true;
}

void SetPlayerDBName(const char* c_pszPlayerDBName)
{
	if (! c_pszPlayerDBName || ! *c_pszPlayerDBName)
		g_stPlayerDBName.clear();
	else
		g_stPlayerDBName = c_pszPlayerDBName;
}

const char * GetPlayerDBName()
{
	return g_stPlayerDBName.c_str();
}

void SetAccountDBName(const char* c_pszAccountDBName)
{
	if (!c_pszAccountDBName || !*c_pszAccountDBName)
		g_stAccountDBName.clear();
	else
		g_stAccountDBName = c_pszAccountDBName;
}
const char * GetAccountDBName()
{
	return g_stAccountDBName.c_str();
}

