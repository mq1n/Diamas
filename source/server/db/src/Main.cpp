#include "stdafx.h"
#include "Config.h"
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

#include "../../libthecore/include/winminidump.h"

void SetPlayerDBName(const char* c_pszPlayerDBName);
void SetAccountDBName(const char* c_pszAccountDBName);
bool Start();

std::string g_stLocaleNameColumn = "name";
std::string g_stLocale = "latin1"; // default: euckr
std::string g_stPlayerDBName = "";
std::string g_stAccountDBName = "";

bool g_test_server = false;
bool g_log = true;

//단위 초
int32_t g_iPlayerCacheFlushSeconds = 60*7;
int32_t g_iItemCacheFlushSeconds = 60*5;

//g_iLogoutSeconds 수치는 g_iPlayerCacheFlushSeconds 와 g_iItemCacheFlushSeconds 보다 길어야 한다.
int32_t g_iLogoutSeconds = 60*10;
// MYSHOP_PRICE_LIST
int32_t g_iItemPriceListTableCacheFlushSeconds = 360;
int32_t g_iActivityCacheFlushSeconds = 600;

// END_OF_MYSHOP_PRICE_LIST


extern void WriteVersion();

//#ifndef _DEBUG
void emergency_sig(int32_t sig)
{
	if (sig == SIGSEGV)
		sys_log(0, "SIGNAL: SIGSEGV");
	else if (sig == SIGUSR1)
		sys_log(0, "SIGNAL: SIGUSR1");

	if (sig == SIGSEGV)
		abort();
}
//#endif

int32_t main()
{
	if (setup_minidump_generator() == false)
		return 1;

	WriteVersion();

	CConfig Config;
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

	sys_log(0, "Metin2DBCacheServer Start\n");

	CClientManager::instance().MainLoop();

	signal_timer_disable();

	DBManager.Quit();
	int32_t iCount;

	for (;;)
	{
		iCount = 0;

		iCount += CDBManager::instance().CountReturnQuery(SQL_PLAYER);
		iCount += CDBManager::instance().CountAsyncQuery(SQL_PLAYER);

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
	if (!CConfig::instance().LoadFile("conf.txt"))
	{
		fprintf(stderr, "Loading conf.txt failed.\n");
		return false;
	}

	int test_srv = 0;
	if (!CConfig::instance().GetValue("TEST_SERVER", &test_srv))
	{
		fprintf(stderr, "Real Server\n");
	}
	else
		fprintf(stderr, "Test Server\n");
	g_test_server = !!test_srv;

	int logon = 0;
	if (!CConfig::instance().GetValue("LOG", &logon))
	{
		fprintf(stderr, "Log Off");
		logon = 0;
	}
	else
	{
		logon = 1;
		fprintf(stderr, "Log On");
	}
	g_log = !!logon;
	
	int32_t tmpValue;

	int32_t heart_beat = 50;
	if (!CConfig::instance().GetValue("CLIENT_HEART_FPS", &heart_beat))
	{
		fprintf(stderr, "Cannot find CLIENT_HEART_FPS configuration.\n");
		return false;
	}

	log_set_expiration_days(3);

	if (CConfig::instance().GetValue("LOG_KEEP_DAYS", &tmpValue))
	{
		tmpValue = MINMAX(3, tmpValue, 30);
		log_set_expiration_days(tmpValue);
		fprintf(stderr, "Setting log keeping days to %d\n", tmpValue);
	}

	thecore_init();
	thecore_set(heart_beat, emptybeat);
	signal_timer_enable(60);

	char szBuf[256+1];

	if (CConfig::instance().GetValue("LOCALE", szBuf, 256))
	{
		g_stLocale = szBuf;
		sys_log(0, "LOCALE set to %s", g_stLocale.c_str());
	}

	if (CConfig::instance().GetValue("PLAYER_CACHE_FLUSH_SECONDS", szBuf, 256))
	{
		str_to_number(g_iPlayerCacheFlushSeconds, szBuf);
		sys_log(0, "PLAYER_CACHE_FLUSH_SECONDS: %d", g_iPlayerCacheFlushSeconds);
	}

	if (CConfig::instance().GetValue("ITEM_CACHE_FLUSH_SECONDS", szBuf, 256))
	{
		str_to_number(g_iItemCacheFlushSeconds, szBuf);
		sys_log(0, "ITEM_CACHE_FLUSH_SECONDS: %d", g_iItemCacheFlushSeconds);
	}

	// MYSHOP_PRICE_LIST
	if (CConfig::instance().GetValue("ITEM_PRICELIST_CACHE_FLUSH_SECONDS", szBuf, 256)) 
	{
		str_to_number(g_iItemPriceListTableCacheFlushSeconds, szBuf);
		sys_log(0, "ITEM_PRICELIST_CACHE_FLUSH_SECONDS: %d", g_iItemPriceListTableCacheFlushSeconds);
	}
	// END_OF_MYSHOP_PRICE_LIST
	//

	if (CConfig::instance().GetValue("ACTIVITY_CACHE_FLUSH_SECONDS", szBuf, 256))
	{
		str_to_number(g_iActivityCacheFlushSeconds, szBuf);
		sys_log(0, "ACTIVITY_CACHE_FLUSH_SECONDS: %d", g_iActivityCacheFlushSeconds);
	}

	if (CConfig::instance().GetValue("CACHE_FLUSH_LIMIT_PER_SECOND", szBuf, 256))
	{
		uint32_t dwVal = 0; str_to_number(dwVal, szBuf);
		CClientManager::instance().SetCacheFlushCountLimit(dwVal);
	}

	int32_t iIDStart;
	if (!CConfig::instance().GetValue("PLAYER_ID_START", &iIDStart))
	{
		sys_err("PLAYER_ID_START not configured");
		return false;
	}

	CClientManager::instance().SetPlayerIDStart(iIDStart);

	if (CConfig::instance().GetValue("NAME_COLUMN", szBuf, 256))
	{
		fprintf(stderr, "%s %s", g_stLocaleNameColumn.c_str(), szBuf);
		g_stLocaleNameColumn = szBuf;
	}

	char szAddr[64], szDB[64], szUser[64], szPassword[64];
	int32_t iPort;
	char line[256+1];

	std::string databases[] = { "SQL_PLAYER", "SQL_ACCOUNT", "SQL_COMMON" };

	for (size_t dataBase = 0; dataBase < sizeof(databases) / sizeof(databases[0]); dataBase++)
	{
		const char * DB_NAME = databases[dataBase].c_str();

		if (!CConfig::instance().GetValue(DB_NAME, line, 256))
		{
			sys_err("%s not configured",DB_NAME);
			return false;
		}

		sscanf(line, " %s %s %s %s %d ", szAddr, szDB, szUser, szPassword, &iPort);

		for (int32_t retries = 1; retries < 4; ++retries)
		{
			if (CDBManager::instance().Connect(dataBase, szAddr, iPort, szDB, szUser, szPassword))
				break;

			fprintf(stderr, "Connection to %s failed, try: [%d|3]\n", szDB, retries);

			if (retries == 3) 
				return false;

			sleep(2);
		}
		fprintf(stderr, "Established connection with database [%s]\n", szDB);

		if (dataBase == 0)
			SetPlayerDBName(szDB);
		else if (dataBase == 1)
			SetAccountDBName(szDB);
	}
	
	if (!CNetPoller::instance().Create())
	{
		sys_err("Cannot create network poller");
		return false;
	}

	sys_log(0, "ClientManager initialization.. ");

	if (!CClientManager::instance().Initialize())
	{
		sys_log(0, "ClientManager initialization failed"); 
		return false;
	}

	sys_log(0, "DB initialization OK");

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

