#include "stdafx.h"
#include "ClientManager.h"
#include "Main.h"
#include "DBManager.h"
#include "QID.h"
#include "GuildManager.h"
#include "PrivManager.h"
#include "ItemAwardManager.h"
#include "Marriage.h"
#include "ItemIDRangeManager.h"
#include "Cache.h"
#include "../../libgame/include/grid.h"

extern int32_t g_iPlayerCacheFlushSeconds;
extern int32_t g_iItemCacheFlushSeconds;
extern bool g_test_server;
extern bool g_log;
extern std::string g_stLocale;
extern std::string g_stLocaleNameColumn;
bool CreateItemTableFromRes(MYSQL_RES * res, std::vector<TPlayerItem> * pVec, uint32_t dwPID);

uint32_t g_dwUsageMax = 0;
uint32_t g_dwUsageAvg = 0;

CPacketInfo g_query_info;
CPacketInfo g_item_info;

#ifdef ENABLE_PROTO_FROM_DB
bool g_bMirror2DB = false;
#endif

CClientManager::CClientManager() :
	m_pkAuthPeer(nullptr),
	m_iPlayerIDStart(0),
	m_iPlayerDeleteLevelLimit(0),
	m_iPlayerDeleteLevelLimitLower(0),
	m_iShopTableSize(0),
	m_pShopTable(nullptr),
	m_iRefineTableSize(0),
	m_pRefineTable(nullptr),
	m_bShutdowned(false),
	m_iCacheFlushCount(0),
	m_iCacheFlushCountLimit(200)
{
	m_itemRange.dwMin = 0;
	m_itemRange.dwMax = 0;
	m_itemRange.dwUsableItemIDMin = 0;

#ifdef ENABLE_PROTO_FROM_DB
	bIsProtoReadFromDB = false;
#endif
}

CClientManager::~CClientManager()
{
	Destroy();
}

void CClientManager::SetPlayerIDStart(int32_t iIDStart)
{
	m_iPlayerIDStart = iIDStart;
}

void CClientManager::GetPeerP2PHostNames(std::string& peerHostNames)
{
	std::ostringstream oss(std::ostringstream::out);

	for (auto peer : m_peerList)
	{
		oss << peer->GetHost() << " " << peer->GetP2PPort() << " channel : " << static_cast<int32_t>(peer->GetChannel()) << "\n";
	}

	peerHostNames += oss.str();
}

void CClientManager::Destroy()
{
	m_mChannelStatus.clear();
	for (auto & i : m_peerList)
		i->Destroy();

	m_peerList.clear();

	if (m_fdAccept > 0)
	{
		socket_close(m_fdAccept);
		m_fdAccept = static_cast<socket_t>(-1);
	}
}

static bool bCleanOldPriv = true;
static bool __InitializeDefaultPriv()
{
	if (bCleanOldPriv)
	{
		std::unique_ptr<SQLMsg> pCleanStuff(CDBManager::Instance().DirectQuery("DELETE FROM priv_settings WHERE value <= 0 OR duration <= NOW();", SQL_COMMON));
		printf("DEFAULT_PRIV_EMPIRE: removed %u expired priv settings.\n", pCleanStuff->Get()->uiAffectedRows);
	}
	std::unique_ptr<SQLMsg> pMsg(CDBManager::Instance().DirectQuery("SELECT priv_type, id, type, value, UNIX_TIMESTAMP(duration) FROM priv_settings", SQL_COMMON));
	if (pMsg->Get()->uiNumRows == 0)
		return false;
	MYSQL_ROW row = nullptr;
	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		if (!strcmp(row[0], "EMPIRE"))
		{
			// init
			uint8_t empire = 0;
			uint8_t type = 1;
			int32_t value = 0;
			uint32_t duration_sec = 0;
			// set
			str_to_number(empire, row[1]);
			str_to_number(type, row[2]);
			str_to_number(value, row[3]);
			str_to_number(duration_sec, row[4]);
			// recalibrate time
			uint32_t now_time_sec = CClientManager::Instance().GetCurrentTime();
			if (now_time_sec>duration_sec)
				duration_sec = 0;
			else
				duration_sec -= now_time_sec;
			// send priv
			printf("DEFAULT_PRIV_EMPIRE: set empire(%u), type(%u), value(%d), duration(%u)\n", empire, type, value, duration_sec);
			CPrivManager::Instance().AddEmpirePriv(empire, type, value, duration_sec);
		}
		else if (!strcmp(row[0], "GUILD"))
		{
			// init
			uint32_t guild_id = 0;
			uint8_t type = 1;
			int32_t value = 0;
			uint32_t duration_sec = 0;
			// set
			str_to_number(guild_id, row[1]);
			str_to_number(type, row[2]);
			str_to_number(value, row[3]);
			str_to_number(duration_sec, row[4]);
			// recalibrate time
			uint32_t now_time_sec = CClientManager::Instance().GetCurrentTime();
			if (now_time_sec>duration_sec)
				duration_sec = 0;
			else
				duration_sec -= now_time_sec;
			// send priv
			if (guild_id)
			{
				printf("DEFAULT_PRIV_GUILD: set guild_id(%u), type(%u), value(%d), duration(%u)\n", guild_id, type, value, duration_sec);
				CPrivManager::Instance().AddGuildPriv(guild_id, type, value, duration_sec);
			}
		}
		else if (!strcmp(row[0], "PLAYER"))
		{
			// init
			uint32_t pid = 0;
			uint8_t type = 1;
			int32_t value = 0;
			// set
			str_to_number(pid, row[1]);
			str_to_number(type, row[2]);
			str_to_number(value, row[3]);
			// send priv
			if (pid)
			{
				printf("DEFAULT_PRIV_PLAYER: set pid(%u), type(%u), value(%d)\n", pid, type, value);
				CPrivManager::Instance().AddCharPriv(pid, type, value);
			}
		}
	}
	return true;
}

static bool __UpdateDefaultPriv(const char* priv_type, uint32_t id, uint8_t type, int32_t value, uint32_t duration_sec)
{
	char szQuery[1024];
	snprintf(szQuery, 1024,
		"REPLACE INTO priv_settings SET priv_type='%s', id=%u, type=%u, value=%d, duration=DATE_ADD(NOW(), INTERVAL %u SECOND);",
		priv_type, id, type, value, duration_sec
	);
	std::unique_ptr<SQLMsg> pMsg(CDBManager::Instance().DirectQuery(szQuery, SQL_COMMON));
	return pMsg->Get()->uiAffectedRows;
}

bool CClientManager::Initialize(const std::string& stConfigBuffer)
{	
	Document document;
	document.Parse<rapidjson::kParseStopWhenDoneFlag>(stConfigBuffer.c_str());

	auto& pkStage = document["stage"];
	auto& pkConfigContent = document[pkStage];

	auto& pkObjItemIDRange = pkConfigContent["item_id_range"];
	if (pkObjItemIDRange.IsNull() || !pkObjItemIDRange.IsObject())
	{
		sys_err("Cannot find item_id_range configuration.");
		return false;
	}	

	if (!InitializeItemIDRange(pkObjItemIDRange["min"].GetUint(), pkObjItemIDRange["max"].GetUint()))
	{
		sys_err("Item range Initialize Failed. Exit DBCache Server");
		return false;
	}

	if (!__InitializeDefaultPriv())
	{
		sys_err("Failed Default Priv Setting so exit\n");
		return false;
	}

#ifdef ENABLE_PROTO_FROM_DB
	auto& pkObjProtoFromDB = pkConfigContent["proto_from_db"];
	if (pkObjProtoFromDB.IsNull() || !pkObjProtoFromDB.IsBool())
	{
		sys_err("Cannot find proto_from_db configuration.");
		return false;
	}
	bIsProtoReadFromDB = pkObjProtoFromDB.GetBool();
	sys_log(0, "PROTO_FROM_DB: %s\n", (bIsProtoReadFromDB) ? "Enabled" : "Disabled");

	auto& pkObjMirrorToDB = pkConfigContent["mirror_to_db"];
	if (pkObjMirrorToDB.IsNull() || !pkObjMirrorToDB.IsBool())
	{
		sys_err("Cannot find proto_from_db configuration.");
		return false;
	}
	g_bMirror2DB = pkObjMirrorToDB.GetBool();
	sys_log(0, "MIRROR2DB: %s\n", (g_bMirror2DB) ? "Enabled" : "Disabled");
#endif

	if (!InitializeTables())
	{
		sys_err("Table Initialize FAILED");
		return false;
	}

	auto& pkNumPlayerDeleteLevelLimit = pkConfigContent["player_delete_level_limit"];
	if (!pkNumPlayerDeleteLevelLimit.IsNull() && pkNumPlayerDeleteLevelLimit.IsNumber())
		m_iPlayerDeleteLevelLimit = pkNumPlayerDeleteLevelLimit.GetInt();
	else
		m_iPlayerDeleteLevelLimitLower = PLAYER_MAX_LEVEL_CONST + 1;

	auto& pkNumPlayerDeleteLevelLimitLower = pkConfigContent["player_delete_level_limit_lower"];
	if (!pkNumPlayerDeleteLevelLimitLower.IsNull() && pkNumPlayerDeleteLevelLimitLower.IsNumber())
		m_iPlayerDeleteLevelLimitLower = pkNumPlayerDeleteLevelLimitLower.GetInt();
	else
		m_iPlayerDeleteLevelLimitLower = 0;

	sys_log(0, "PLAYER_DELETE_LEVEL_LIMIT set to %d", m_iPlayerDeleteLevelLimit);
	sys_log(0, "PLAYER_DELETE_LEVEL_LIMIT_LOWER set to %d", m_iPlayerDeleteLevelLimitLower);

	CGuildManager::Instance().BootReserveWar();

	auto& pkStrBindIP = pkConfigContent["bind_ip"];
	if (pkStrBindIP.IsNull() || !pkStrBindIP.IsString())
	{
		sys_err("Cannot find BIND_IP in config file; DB initialization failed");
		return false;		
	}
	std::string stBindIP = pkStrBindIP.GetString();

	auto& pkNumBindPort = pkConfigContent["bind_port"];
	if (pkNumBindPort.IsNull() || !pkNumBindPort.IsNumber())
	{
		sys_err("Cannot find BIND_PORT in config file; DB initialization failed");
		return false;		
	}
	auto nBindPort = pkNumBindPort.GetInt();

	m_fdAccept = socket_tcp_bind(stBindIP.c_str(), nBindPort);
	if (m_fdAccept < 0)
	{
		sys_err("Could not open socket at %s:%d", stBindIP.c_str(), nBindPort);
		perror("socket");
		return false;
	}

	sys_log(0, "ACCEPT_HANDLE: %u", m_fdAccept);
	fdwatch_add_fd(m_fdWatcher, m_fdAccept, nullptr, FDW_READ, false);

	m_looping = true;
	LoadEventFlag();
	return true;
}

void CClientManager::MainLoop()
{
	SQLMsg * tmp;

	sys_log(0, "ClientManager pointer is %p", this);

	// 메인루프
	while (!m_bShutdowned)
	{
		while ((tmp = CDBManager::Instance().PopResult()))
		{
			AnalyzeQueryResult(tmp);
			delete tmp;
		}

		if (!Process())
			break;

		log_rotate();
	}

	//
	// 메인루프 종료처리
	//
	sys_log(0, "MainLoop exited, Starting cache flushing");

	signal_timer_disable();

	/*
	*	Flush player cache
	*/
	for (auto & itPlayerCache : m_map_playerCache)
	{
		auto c = itPlayerCache.second;
		c->Flush();
		delete c;
	}
	m_map_playerCache.clear();

	/*
	*	Flush item cache
	*/
	for (auto & itItemCache : m_map_itemCache)
	{
		auto c = itItemCache.second;
		c->Flush();
		delete c;
	}
	m_map_itemCache.clear();

	/*
	*	Flush private shop pricelist cache
	*/
	for (auto & itPriceList : m_mapItemPriceListCache)
	{
		auto pCache = itPriceList.second;
		pCache->Flush();
		delete pCache;
	}

	m_mapItemPriceListCache.clear();
	// END_OF_MYSHOP_PRICE_LIST

	/*
	*	Flush activity cache
	*/
	for (const auto& activityCache : m_map_activityCache)
	{
		auto pCache = activityCache.second;
		pCache->Flush();
		delete pCache;
	}

	m_map_activityCache.clear();
}

void CClientManager::Quit()
{
	m_bShutdowned = TRUE;
}

void CClientManager::QUERY_BOOT(CPeer* peer, TPacketGDBoot * p)
{
	const uint8_t bPacketVersion = 6; // BOOT 패킷이 바뀔때마다 번호를 올리도록 한다.

	std::vector<tAdminInfo> vAdmin;
	__GetAdminInfo(vAdmin);
	uint32_t adwAdminConfig[GM_MAX_NUM];
	__GetAdminConfig(adwAdminConfig);

	sys_log(0, "QUERY_BOOT : AdminInfo %d (Request ServerIp %s) ", vAdmin.size(), p->szIP);

	uint32_t dwPacketSize = 
		sizeof(uint32_t) +
		sizeof(uint8_t) +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TMobTable) * m_vec_mobTable.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(SItemTable_Server) * m_vec_itemTable.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TShopTable) * m_iShopTableSize +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TSkillTable) * m_vec_skillTable.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TRefineTable) * m_iRefineTableSize +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TItemAttrTable) * m_vec_itemAttrTable.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TItemAttrTable) * m_vec_itemRareTable.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TBanwordTable) * m_vec_banwordTable.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(building::TLand) * m_vec_kLandTable.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(building::TObjectProto) * m_vec_kObjectProto.size() + 
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(building::TObject) * m_map_pkObjectTable.size() +
		sizeof(time_t) + 
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TItemIDRangeTable)*2 +
		//ADMIN_MANAGER
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(tAdminInfo) *  vAdmin.size() + sizeof(adwAdminConfig) +
		//END_ADMIN_MANAGER
		sizeof(uint16_t); 

	peer->EncodeHeader(HEADER_DG_BOOT, 0, dwPacketSize);
	peer->Encode(&dwPacketSize, sizeof(uint32_t));
	peer->Encode(&bPacketVersion, sizeof(uint8_t));

	sys_log(0, "BOOT: PACKET: %d", dwPacketSize);
	sys_log(0, "BOOT: VERSION: %d", bPacketVersion);

	sys_log(0, "sizeof(TMobTable) = %d", sizeof(TMobTable));
	sys_log(0, "sizeof(SItemTable_Server) = %d", sizeof(SItemTable_Server));
	sys_log(0, "sizeof(TShopTable) = %d", sizeof(TShopTable));
	sys_log(0, "sizeof(TSkillTable) = %d", sizeof(TSkillTable));
	sys_log(0, "sizeof(TRefineTable) = %d", sizeof(TRefineTable));
	sys_log(0, "sizeof(TItemAttrTable) = %d", sizeof(TItemAttrTable));
	sys_log(0, "sizeof(TItemRareTable) = %d", sizeof(TItemAttrTable));
	sys_log(0, "sizeof(TBanwordTable) = %d", sizeof(TBanwordTable));
	sys_log(0, "sizeof(TLand) = %d", sizeof(building::TLand));
	sys_log(0, "sizeof(TObjectProto) = %d", sizeof(building::TObjectProto));
	sys_log(0, "sizeof(TObject) = %d", sizeof(building::TObject));
	//ADMIN_MANAGER
	sys_log(0, "sizeof(tAdminInfo) = %d * %d ", sizeof(tAdminInfo), vAdmin.size());
	//END_ADMIN_MANAGER

	peer->EncodeWORD(sizeof(TMobTable));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_mobTable.size()));
	peer->Encode(&m_vec_mobTable[0], sizeof(TMobTable) * m_vec_mobTable.size());

	peer->EncodeWORD(sizeof(SItemTable_Server));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_itemTable.size()));
	peer->Encode(&m_vec_itemTable[0], sizeof(SItemTable_Server) * m_vec_itemTable.size());

	peer->EncodeWORD(sizeof(TShopTable));
	peer->EncodeWORD(static_cast<uint16_t>(m_iShopTableSize));
	peer->Encode(m_pShopTable, sizeof(TShopTable) * m_iShopTableSize);

	peer->EncodeWORD(sizeof(TSkillTable));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_skillTable.size()));
	peer->Encode(&m_vec_skillTable[0], sizeof(TSkillTable) * m_vec_skillTable.size());

	peer->EncodeWORD(sizeof(TRefineTable));
	peer->EncodeWORD(static_cast<uint16_t>(m_iRefineTableSize));
	peer->Encode(m_pRefineTable, sizeof(TRefineTable) * m_iRefineTableSize);

	peer->EncodeWORD(sizeof(TItemAttrTable));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_itemAttrTable.size()));
	peer->Encode(&m_vec_itemAttrTable[0], sizeof(TItemAttrTable) * m_vec_itemAttrTable.size());

	peer->EncodeWORD(sizeof(TItemAttrTable));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_itemRareTable.size()));
	peer->Encode(&m_vec_itemRareTable[0], sizeof(TItemAttrTable) * m_vec_itemRareTable.size());

	peer->EncodeWORD(sizeof(TBanwordTable));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_banwordTable.size()));
	if (m_vec_banwordTable.size())
		peer->Encode(&m_vec_banwordTable[0], sizeof(TBanwordTable) * m_vec_banwordTable.size());

	peer->EncodeWORD(sizeof(building::TLand));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_kLandTable.size()));
	if (m_vec_kLandTable.size())
		peer->Encode(&m_vec_kLandTable[0], sizeof(building::TLand) * m_vec_kLandTable.size());

	peer->EncodeWORD(sizeof(building::TObjectProto));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_kObjectProto.size()));
	peer->Encode(&m_vec_kObjectProto[0], sizeof(building::TObjectProto) * m_vec_kObjectProto.size());

	peer->EncodeWORD(sizeof(building::TObject));
	peer->EncodeWORD(static_cast<uint16_t>(m_map_pkObjectTable.size()));

	auto it = m_map_pkObjectTable.begin();

	while (it != m_map_pkObjectTable.end())
		peer->Encode((it++)->second, sizeof(building::TObject));

	time_t now = time(nullptr);
	peer->Encode(&now, sizeof(time_t));

	TItemIDRangeTable itemRange = CItemIDRangeManager::Instance().GetRange();
	TItemIDRangeTable itemRangeSpare = CItemIDRangeManager::Instance().GetRange();

	peer->EncodeWORD(sizeof(TItemIDRangeTable));
	peer->EncodeWORD(1);
	peer->Encode(&itemRange, sizeof(TItemIDRangeTable));
	peer->Encode(&itemRangeSpare, sizeof(TItemIDRangeTable));

	peer->SetItemIDRange(itemRange);
	peer->SetSpareItemIDRange(itemRangeSpare);

	//ADMIN_MANAGER
	peer->EncodeWORD(sizeof(tAdminInfo));
	peer->EncodeWORD(static_cast<uint16_t>(vAdmin.size()));

	for (size_t n = 0; n < vAdmin.size(); ++n)
	{
		peer->Encode(&vAdmin[n], sizeof(tAdminInfo));
		sys_log(0, "Admin name %s", vAdmin[n].m_szName);
	}

	peer->Encode(&adwAdminConfig[0], sizeof(adwAdminConfig));
	//END_ADMIN_MANAGER

	peer->EncodeWORD(0xffff);
}

void CClientManager::SendPartyOnSetup(CPeer* pkPeer)
{
	TPartyMap & pm = m_map_pkChannelParty[pkPeer->GetChannel()];

	for (auto & it_party : pm)
	{
		sys_log(0, "PARTY SendPartyOnSetup Party [%u]", it_party.first);
		pkPeer->EncodeHeader(HEADER_DG_PARTY_CREATE, 0, sizeof(TPacketPartyCreate));
		pkPeer->Encode(&it_party.first, sizeof(uint32_t));

		for (auto it_member = it_party.second.begin(); it_member != it_party.second.end(); ++it_member)
		{
			sys_log(0, "PARTY SendPartyOnSetup Party [%u] Member [%u]", it_party.first, it_member->first);
			pkPeer->EncodeHeader(HEADER_DG_PARTY_ADD, 0, sizeof(TPacketPartyAdd));
			pkPeer->Encode(&it_party.first, sizeof(uint32_t));
			pkPeer->Encode(&it_member->first, sizeof(uint32_t));
			pkPeer->Encode(&it_member->second.bRole, sizeof(uint8_t));

			pkPeer->EncodeHeader(HEADER_DG_PARTY_SET_MEMBER_LEVEL, 0, sizeof(TPacketPartySetMemberLevel));
			pkPeer->Encode(&it_party.first, sizeof(uint32_t));
			pkPeer->Encode(&it_member->first, sizeof(uint32_t));
			pkPeer->Encode(&it_member->second.bLevel, sizeof(uint8_t));
		}
	}
}

void CClientManager::QUERY_QUEST_SAVE(CPeer * pkPeer, TQuestTable * pTable, uint32_t dwLen)
{
	if (0 != (dwLen % sizeof(TQuestTable)))
	{
		sys_err("invalid packet size %d, sizeof(TQuestTable) == %d", dwLen, sizeof(TQuestTable));
		return;
	}

	int32_t iSize = dwLen / sizeof(TQuestTable);

	char szQuery[1024];

	for (int32_t i = 0; i < iSize; ++i, ++pTable)
	{
		if (pTable->lValue == 0)
		{
			snprintf(szQuery, sizeof(szQuery),
					"DELETE FROM quest WHERE dwPID=%d AND szName='%s' AND szState='%s'",
					pTable->dwPID, pTable->szName, pTable->szState);
		}
		else
		{
			snprintf(szQuery, sizeof(szQuery),
					"REPLACE INTO quest (dwPID, szName, szState, lValue) VALUES(%d, '%s', '%s', %d)",
					pTable->dwPID, pTable->szName, pTable->szState, pTable->lValue);
		}

		CDBManager::Instance().ReturnQuery(szQuery, QID_QUEST_SAVE, pkPeer->GetHandle(), nullptr);
	}
}

void CClientManager::QUERY_SAFEBOX_LOAD(CPeer * pkPeer, uint32_t dwHandle, TSafeboxLoadPacket * packet, bool bMall)
{
	auto pi = new ClientHandleInfo(dwHandle);
	strlcpy(pi->safebox_password, packet->szPassword, sizeof(pi->safebox_password));
	pi->account_id = packet->dwID;
	pi->account_index = 0;
	pi->ip[0] = bMall ? 1 : 0;
	strlcpy(pi->login, packet->szLogin, sizeof(pi->login));

	char szQuery[ASQL_QUERY_MAX_LEN];
	snprintf(szQuery, sizeof(szQuery), "SELECT account_id, size, password FROM safebox WHERE account_id=%u", packet->dwID);

	if (g_log)
		sys_log(0, "HEADER_GD_SAFEBOX_LOAD (handle: %d account.id %u is_mall %d)", dwHandle, packet->dwID, bMall ? 1 : 0);

	CDBManager::Instance().ReturnQuery(szQuery, QID_SAFEBOX_LOAD, pkPeer->GetHandle(), pi);
}

void CClientManager::RESULT_SAFEBOX_LOAD(CPeer * pkPeer, SQLMsg * msg)
{
	CQueryInfo * qi = (CQueryInfo *) msg->pvUserData;
	ClientHandleInfo * pi = (ClientHandleInfo *) qi->pvData;
	uint32_t dwHandle = pi->dwHandle;

	// 여기에서 사용하는 account_index는 쿼리 순서를 말한다.
	// 첫번째 패스워드 알아내기 위해 하는 쿼리가 0
	// 두번째 실제 데이터를 얻어놓는 쿼리가 1

	if (pi->account_index == 0)
	{
		char szSafeboxPassword[SAFEBOX_PASSWORD_MAX_LEN + 1];
		strlcpy(szSafeboxPassword, pi->safebox_password, sizeof(szSafeboxPassword));

		auto pSafebox = new TSafeboxTable;
		memset(pSafebox, 0, sizeof(TSafeboxTable));

		SQLResult * res = msg->Get();

		if (res->uiNumRows == 0)
		{
			if (strcmp("000000", szSafeboxPassword))
			{
				pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_WRONG_PASSWORD, dwHandle, 0);
				delete pSafebox;
				delete pi;
				return;
			}
		}
		else
		{
			MYSQL_ROW row = mysql_fetch_row(res->pSQLResult);

			// 비밀번호가 틀리면..
			if (((!row[2] || !*row[2]) && strcmp("000000", szSafeboxPassword)) ||
				((row[2] && *row[2]) && strcmp(row[2], szSafeboxPassword)))
			{
				pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_WRONG_PASSWORD, dwHandle, 0);
				delete pSafebox;
				delete pi;
				return;
			}

			if (!row[0])
				pSafebox->dwID = 0;
			else
				str_to_number(pSafebox->dwID, row[0]);

			if (!row[1])
				pSafebox->bSize = 0;
			else
				str_to_number(pSafebox->bSize, row[1]);
			/*
			   if (!row[3])
			   pSafebox->dwGold = 0;
			   else
			   pSafebox->dwGold = atoi(row[3]);
			   */
			if (pi->ip[0] == 1)
			{
				pSafebox->bSize = 1;
				sys_log(0, "MALL id[%d] size[%d]", pSafebox->dwID, pSafebox->bSize);
			}
			else
				sys_log(0, "SAFEBOX id[%d] size[%d]", pSafebox->dwID, pSafebox->bSize);
		}

		if (0 == pSafebox->dwID)
			pSafebox->dwID = pi->account_id;

		pi->pSafebox = pSafebox;

		char szQuery[512];
		snprintf(szQuery, sizeof(szQuery), 
				"SELECT id, `window`+0, pos, count, vnum, is_gm_owner, socket0, socket1, socket2, "
				"attrtype0, attrvalue0, "
				"attrtype1, attrvalue1, "
				"attrtype2, attrvalue2, "
				"attrtype3, attrvalue3, "
				"attrtype4, attrvalue4, "
				"attrtype5, attrvalue5, "
				"attrtype6, attrvalue6 "
				"FROM item WHERE owner_id=%d AND window='%s'",
				pi->account_id, pi->ip[0] == 0 ? "SAFEBOX" : "MALL");

		pi->account_index = 1;

		CDBManager::Instance().ReturnQuery(szQuery, QID_SAFEBOX_LOAD, pkPeer->GetHandle(), pi);
	}
	else
	{

		if (!pi->pSafebox)
		{
			sys_err("null safebox pointer!");
			delete pi;
			return;
		}


		// 쿼리에 에러가 있었으므로 응답할 경우 창고가 비어있는 것 처럼
		// 보이기 때문에 창고가 아얘 안열리는게 나음
		if (!msg->Get()->pSQLResult)
		{
			sys_err("null safebox result");
			delete pi;
			return;
		}

		static std::vector<TPlayerItem> s_items;
		CreateItemTableFromRes(msg->Get()->pSQLResult, &s_items, pi->account_id);

		std::set<TItemAward *> * pSet = ItemAwardManager::Instance().GetByLogin(pi->login);

		if (pSet && !m_vec_itemTable.empty())
		{

			CGrid grid(5, MAX(1, pi->pSafebox->bSize) * 9);
			bool bEscape = false;
			
			for (auto & r : s_items)
			{
				auto it = m_map_itemTableByVnum.find(r.vnum);

				if (it == m_map_itemTableByVnum.end())
				{
					bEscape = true;
					sys_err("invalid item vnum %u in safebox: login %s", r.vnum, pi->login);
					break;
				}

				grid.Put(r.pos, 1, it->second->bSize);
			}

			if (!bEscape)
			{
				std::vector<std::pair<uint32_t, uint32_t> > vec_dwFinishedAwardID;

				char szQuery[512];

				auto it = pSet->begin();
				while (it != pSet->end())
				{
					TItemAward * pItemAward = *(it++);
					const uint32_t& dwItemVnum = pItemAward->dwVnum;

					if (pItemAward->bTaken)
						continue;

					if (pi->ip[0] == 0 && pItemAward->bMall)
						continue;

					if (pi->ip[0] == 1 && !pItemAward->bMall)
						continue;

					auto it2 = m_map_itemTableByVnum.find(pItemAward->dwVnum);

					if (it2 == m_map_itemTableByVnum.end())
					{
						sys_err("invalid item vnum %u in item_award: login %s", pItemAward->dwVnum, pi->login);
						continue;
					}

					SItemTable_Server * pItemTable = it2->second;

					int32_t iPos;

					if ((iPos = grid.FindBlank(1, it2->second->bSize)) == -1)
						break;

					TPlayerItem item;
					memset(&item, 0, sizeof(TPlayerItem));

					uint32_t dwSocket2 = pItemAward->dwSocket2;

					if (pItemTable->bType == ITEM_UNIQUE)
					{
						if (pItemAward->dwSocket2 != 0)
							dwSocket2 = pItemAward->dwSocket2;
						else
							dwSocket2 = pItemTable->alValues[0];
					}
					else if ((dwItemVnum == 50300 || dwItemVnum == 70037) && pItemAward->dwSocket0 == 0)
					{
						uint32_t dwSkillIdx;
						uint32_t dwSkillVnum;

						do
						{
							dwSkillIdx = number(0, m_vec_skillTable.size()-1);

							dwSkillVnum = m_vec_skillTable[dwSkillIdx].dwVnum;

							if (!dwSkillVnum || dwSkillVnum > 120)
								continue;

							break;
						} while (1);

						pItemAward->dwSocket0 = dwSkillVnum;
					}
					else
					{
						switch (dwItemVnum)
						{
							case 72723: case 72724: case 72725: case 72726:
							case 72727: case 72728: case 72729: case 72730:
							// 무시무시하지만 이전에 하던 걸 고치기는 무섭고...
							// 그래서 그냥 하드 코딩. 선물 상자용 자동물약 아이템들.
							case 76004: case 76005: case 76021: case 76022:
							case 79012: case 79013:
								if (pItemAward->dwSocket2 == 0)
								{
									dwSocket2 = pItemTable->alValues[0];
								}
								else
								{
									dwSocket2 = pItemAward->dwSocket2;
								}
								break;
						}
					}

					if (GetItemID () > m_itemRange.dwMax)
					{
						sys_err("UNIQUE ID OVERFLOW!!");
						break;
					}

					{
						auto it3 = m_map_itemTableByVnum.find (dwItemVnum);
						if (it3 == m_map_itemTableByVnum.end())
						{
							sys_err ("Invalid item(vnum : %d). It is not in m_map_itemTableByVnum.", dwItemVnum);
							continue;
						}
						SItemTable_Server* item_table = it3->second;
						if (item_table == nullptr)
						{
							sys_err ("Invalid item_table (vnum : %d). It's value is nullptr in m_map_itemTableByVnum.", dwItemVnum);
							continue;
						}
						if (0 == pItemAward->dwSocket0)
						{
							for (int32_t i = 0; i < ITEM_LIMIT_MAX_NUM; i++)
							{
								if (LIMIT_REAL_TIME == item_table->aLimits[i].bType)
								{
									if (0 == item_table->aLimits[i].lValue)
										pItemAward->dwSocket0 = time(0) + 60 * 60 * 24 * 7;
									else
										pItemAward->dwSocket0 = time(0) + item_table->aLimits[i].lValue;

									break;
								}
								else if (LIMIT_REAL_TIME_START_FIRST_USE == item_table->aLimits[i].bType || LIMIT_TIMER_BASED_ON_WEAR == item_table->aLimits[i].bType)
								{
									if (0 == item_table->aLimits[i].lValue)
										pItemAward->dwSocket0 = 60 * 60 * 24 * 7;
									else
										pItemAward->dwSocket0 = item_table->aLimits[i].lValue;

									break;
								}
							}
						}

						snprintf(szQuery, sizeof(szQuery), 
								"INSERT INTO item (id, owner_id, window, pos, vnum, count, socket0, socket1, socket2) "
								"VALUES(%u, %u, '%s', %d, %u, %u, %u, %u, %u)",
								GainItemID(),
								pi->account_id,
								pi->ip[0] == 0 ? "SAFEBOX" : "MALL",
								iPos,
								pItemAward->dwVnum, pItemAward->dwCount, pItemAward->dwSocket0, pItemAward->dwSocket1, dwSocket2);
					}

					std::unique_ptr<SQLMsg> pmsg(CDBManager::Instance().DirectQuery(szQuery));
					SQLResult * pRes = pmsg->Get();
					sys_log(0, "SAFEBOX Query : [%s]", szQuery);

					if (pRes->uiAffectedRows == 0 || pRes->uiInsertID == 0 || pRes->uiAffectedRows == static_cast<uint32_t>(-1))
						break;

					item.id = pmsg->Get()->uiInsertID;
					item.window = pi->ip[0] == 0 ? SAFEBOX : MALL,
					item.pos = iPos;
					item.count = pItemAward->dwCount;
					item.vnum = pItemAward->dwVnum;
					item.alSockets[0] = pItemAward->dwSocket0;
					item.alSockets[1] = pItemAward->dwSocket1;
					item.alSockets[2] = dwSocket2;
					s_items.push_back(item);

					vec_dwFinishedAwardID.push_back(std::make_pair(pItemAward->dwID, item.id));
					grid.Put(iPos, 1, it2->second->bSize);
				}

				for (uint32_t i = 0; i < vec_dwFinishedAwardID.size(); ++i)
					ItemAwardManager::Instance().Taken(vec_dwFinishedAwardID[i].first, vec_dwFinishedAwardID[i].second);
			}
		}

		pi->pSafebox->wItemCount = static_cast<uint16_t>(s_items.size());

		pkPeer->EncodeHeader(pi->ip[0] == 0 ? HEADER_DG_SAFEBOX_LOAD : HEADER_DG_MALL_LOAD, dwHandle, sizeof(TSafeboxTable) + sizeof(TPlayerItem) * s_items.size());

		pkPeer->Encode(pi->pSafebox, sizeof(TSafeboxTable));

		if (!s_items.empty())
			pkPeer->Encode(&s_items[0], sizeof(TPlayerItem) * s_items.size());

		delete pi;
	}
}

void CClientManager::QUERY_SAFEBOX_CHANGE_SIZE(CPeer * pkPeer, uint32_t dwHandle, TSafeboxChangeSizePacket * p)
{
	ClientHandleInfo * pi = new ClientHandleInfo(dwHandle);
	pi->account_index = p->bSize;	// account_index를 사이즈로 임시로 사용

	char szQuery[ASQL_QUERY_MAX_LEN];

	if (p->bSize == 1)
	{
		snprintf(szQuery, sizeof(szQuery), "UPDATE safebox SET size=1 WHERE account_id=%u", p->dwID);

		std::unique_ptr<SQLMsg> pMsg(CDBManager::Instance().DirectQuery(szQuery, SQL_PLAYER));

		if (pMsg->Get()->uiAffectedRows == 0)
			snprintf(szQuery, sizeof(szQuery), "INSERT INTO safebox (account_id, size) VALUES(%u, 1)", p->dwID);
	}
	else {
		snprintf(szQuery, sizeof(szQuery), "UPDATE safebox SET size=%u WHERE account_id=%u", p->bSize, p->dwID);
	}
	CDBManager::Instance().ReturnQuery(szQuery, QID_SAFEBOX_CHANGE_SIZE, pkPeer->GetHandle(), pi);
}

void CClientManager::RESULT_SAFEBOX_CHANGE_SIZE(CPeer * pkPeer, SQLMsg * msg)
{
	CQueryInfo * qi = static_cast<CQueryInfo *>(msg->pvUserData);
	ClientHandleInfo * p = static_cast<ClientHandleInfo *>(qi->pvData);
	uint32_t dwHandle = p->dwHandle;
	uint8_t bSize = p->account_index;

	delete p;

	if (msg->Get()->uiNumRows > 0)
	{
		pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_CHANGE_SIZE, dwHandle, sizeof(uint8_t));
		pkPeer->EncodeBYTE(bSize);
	}
}

void CClientManager::QUERY_SAFEBOX_CHANGE_PASSWORD(CPeer * pkPeer, uint32_t dwHandle, TSafeboxChangePasswordPacket * p)
{
	ClientHandleInfo * pi = new ClientHandleInfo(dwHandle);
	strlcpy(pi->safebox_password, p->szNewPassword, sizeof(pi->safebox_password));
	strlcpy(pi->login, p->szOldPassword, sizeof(pi->login));
	pi->account_id = p->dwID;

	char szQuery[ASQL_QUERY_MAX_LEN];
	snprintf(szQuery, sizeof(szQuery), "SELECT password FROM safebox WHERE account_id=%u", p->dwID);

	CDBManager::Instance().ReturnQuery(szQuery, QID_SAFEBOX_CHANGE_PASSWORD, pkPeer->GetHandle(), pi);
}

void CClientManager::RESULT_SAFEBOX_CHANGE_PASSWORD(CPeer * pkPeer, SQLMsg * msg)
{
	CQueryInfo * qi = (CQueryInfo *) msg->pvUserData;
	ClientHandleInfo * p = (ClientHandleInfo *) qi->pvData;
	uint32_t dwHandle = p->dwHandle;

	if (msg->Get()->uiNumRows > 0)
	{
		MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);

		if ((row[0] && *row[0] && !strcasecmp(row[0], p->login)) || ((!row[0] || !*row[0]) && !strcmp("000000", p->login)))
		{
			char szQuery[ASQL_QUERY_MAX_LEN];
			char escape_pwd[64];
			CDBManager::Instance().EscapeString(escape_pwd, p->safebox_password, strlen(p->safebox_password));

			snprintf(szQuery, sizeof(szQuery), "UPDATE safebox SET password='%s' WHERE account_id=%u", escape_pwd, p->account_id);

			CDBManager::Instance().ReturnQuery(szQuery, QID_SAFEBOX_CHANGE_PASSWORD_SECOND, pkPeer->GetHandle(), p);
			return;
		}
	}

	delete p;

	// Wrong old password
	pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_CHANGE_PASSWORD_ANSWER, dwHandle, sizeof(uint8_t));
	pkPeer->EncodeBYTE(0);
}

void CClientManager::RESULT_SAFEBOX_CHANGE_PASSWORD_SECOND(CPeer * pkPeer, SQLMsg * msg)
{
	CQueryInfo * qi = (CQueryInfo *) msg->pvUserData;
	ClientHandleInfo * p = (ClientHandleInfo *) qi->pvData;
	uint32_t dwHandle = p->dwHandle;
	delete p;

	pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_CHANGE_PASSWORD_ANSWER, dwHandle, sizeof(uint8_t));
	pkPeer->EncodeBYTE(1);
}

// MYSHOP_PRICE_LIST
void CClientManager::RESULT_PRICELIST_LOAD(CPeer* peer, SQLMsg* pMsg)
{
	TItemPricelistReqInfo* pReqInfo = static_cast<TItemPricelistReqInfo*>(static_cast<CQueryInfo*>(pMsg->pvUserData)->pvData);

	//
	// DB 에서 로드한 정보를 Cache 에 저장
	//

	TItemPriceListTable table;
	table.dwOwnerID = pReqInfo->second;
	table.byCount = 0;
	
	MYSQL_ROW row;

	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		str_to_number(table.aPriceInfo[table.byCount].dwVnum, row[0]);
		str_to_number(table.aPriceInfo[table.byCount].dwPrice, row[1]);
		table.byCount++;
	}

	PutItemPriceListCache(&table);

	//
	// 로드한 데이터를 Game server 에 전송
	//

	TPacketMyshopPricelistHeader header;

	header.dwOwnerID = pReqInfo->second;
	header.byCount = table.byCount;

	size_t sizePriceListSize = sizeof(TItemPriceInfo) * header.byCount;

	peer->EncodeHeader(HEADER_DG_MYSHOP_PRICELIST_RES, pReqInfo->first, sizeof(header) + sizePriceListSize);
	peer->Encode(&header, sizeof(header));
	peer->Encode(table.aPriceInfo, sizePriceListSize);

	sys_log(0, "Load MyShopPricelist handle[%d] pid[%d] count[%d]", pReqInfo->first, pReqInfo->second, header.byCount);

	delete pReqInfo;
}

void CClientManager::RESULT_PRICELIST_LOAD_FOR_UPDATE(SQLMsg* pMsg)
{
	TItemPriceListTable* pUpdateTable = static_cast<TItemPriceListTable*>(static_cast<CQueryInfo*>(pMsg->pvUserData)->pvData);

	//
	// DB 에서 로드한 정보를 Cache 에 저장
	//

	TItemPriceListTable table;
	table.dwOwnerID = pUpdateTable->dwOwnerID;
	table.byCount = 0;
	
	MYSQL_ROW row;

	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		str_to_number(table.aPriceInfo[table.byCount].dwVnum, row[0]);
		str_to_number(table.aPriceInfo[table.byCount].dwPrice, row[1]);
		table.byCount++;
	}

	PutItemPriceListCache(&table);

	// Update cache
	GetItemPriceListCache(pUpdateTable->dwOwnerID)->UpdateList(pUpdateTable);

	delete pUpdateTable;
}
// END_OF_MYSHOP_PRICE_LIST

void CClientManager::QUERY_SAFEBOX_SAVE(CPeer * pkPeer, TSafeboxTable * pTable)
{
	char szQuery[ASQL_QUERY_MAX_LEN];
	snprintf(szQuery, sizeof(szQuery),
		"UPDATE safebox SET gold='%u' WHERE account_id=%u", 
		pTable->dwGold, pTable->dwID);

	CDBManager::Instance().ReturnQuery(szQuery, QID_SAFEBOX_SAVE, pkPeer->GetHandle(), nullptr);
}

void CClientManager::QUERY_EMPIRE_SELECT(CPeer * pkPeer, uint32_t dwHandle, TEmpireSelectPacket * p)
{
	if (p->bEmpire > 3)
		p->bEmpire = 3;
	char szQuery[ASQL_QUERY_MAX_LEN];

	snprintf(szQuery, sizeof(szQuery), "UPDATE player_index SET empire=%u WHERE id=%u", p->bEmpire, p->dwAccountID);
	CDBManager::Instance().AsyncQuery(szQuery);

	sys_log(0, "EmpireSelect: %s", szQuery);
	{
		snprintf(szQuery, sizeof(szQuery),
				"SELECT pid1, pid2, pid3, pid4, pid5 FROM player_index WHERE id=%u", p->dwAccountID);

		std::unique_ptr<SQLMsg> pmsg(CDBManager::Instance().DirectQuery(szQuery));

		SQLResult * pRes = pmsg->Get();

		if (pRes->uiNumRows)
		{
			sys_log(0, "EMPIRE %u", pRes->uiNumRows);

			MYSQL_ROW row = mysql_fetch_row(pRes->pSQLResult);
			uint32_t pids[PLAYER_PER_ACCOUNT];

			uint32_t g_start_map[4] =
			{
				0,  // reserved
				1,  // 신수국
				21, // 천조국
				41  // 진노국
			};

			// FIXME share with game
			uint32_t g_start_position[4][2]=
			{
				{      0,      0 },
				{ 469300, 964200 }, // 신수국
				{  55700, 157900 }, // 천조국
				{ 969600, 278400 }  // 진노국
			};

			for (int32_t i = 0; i < PLAYER_PER_ACCOUNT; ++i)
			{
				str_to_number(pids[i], row[i]);
				sys_log(0, "EMPIRE PIDS[%d]", pids[i]);

				if (pids[i])
				{
					sys_log(0, "EMPIRE move to pid[%d] to villiage of %u, map_index %d", 
							pids[i], p->bEmpire, g_start_map[p->bEmpire]);

					snprintf(szQuery, sizeof(szQuery), "UPDATE player SET map_index=%u,x=%u,y=%u WHERE id=%u",
							g_start_map[p->bEmpire],
							g_start_position[p->bEmpire][0],
							g_start_position[p->bEmpire][1],
							pids[i]);

					CDBManager::Instance().AsyncQuery(szQuery);
				}
			}
		}
	}

	pkPeer->EncodeHeader(HEADER_DG_EMPIRE_SELECT, dwHandle, sizeof(uint8_t));
	pkPeer->EncodeBYTE(p->bEmpire);
}

void CClientManager::QUERY_SETUP(CPeer * peer, uint32_t, const char * c_pData)
{
	const auto p = reinterpret_cast<const TPacketGDSetup *>(c_pData);
	c_pData += sizeof(TPacketGDSetup);

	if (p->bAuthServer)
	{
		sys_log(0, "AUTH_PEER ptr %p", peer);

		m_pkAuthPeer = peer;
		return;
	}

	peer->SetPublicIP(p->szPublicIP);
	peer->SetChannel(p->bChannel);
	peer->SetListenPort(p->wListenPort);
	peer->SetP2PPort(p->wP2PPort);
	peer->SetMaps(p->alMaps);

	//
	// 어떤 맵이 어떤 서버에 있는지 보내기
	//
	TMapLocation kMapLocations;

	strlcpy(kMapLocations.szHost, peer->GetPublicIP(), sizeof(kMapLocations.szHost));
	kMapLocations.wPort = peer->GetListenPort();
	memcpy(kMapLocations.alMaps, peer->GetMaps(), sizeof(kMapLocations.alMaps));

	uint8_t bMapCount;

	std::vector<TMapLocation> vec_kMapLocations;

	if (peer->GetChannel() == 1)
	{
		for (auto tmp : m_peerList)
		{
			if (tmp == peer)
				continue;

			if (!tmp->GetChannel())
				continue;

			if (tmp->GetChannel() == GUILD_WARP_WAR_CHANNEL || tmp->GetChannel() == peer->GetChannel())
			{
				TMapLocation kMapLocation2;
				strlcpy(kMapLocation2.szHost, tmp->GetPublicIP(), sizeof(kMapLocation2.szHost));
				kMapLocation2.wPort = tmp->GetListenPort();
				memcpy(kMapLocation2.alMaps, tmp->GetMaps(), sizeof(kMapLocation2.alMaps));
				vec_kMapLocations.push_back(kMapLocation2);

				tmp->EncodeHeader(HEADER_DG_MAP_LOCATIONS, 0, sizeof(uint8_t) + sizeof(TMapLocation));
				bMapCount = 1;
				tmp->EncodeBYTE(bMapCount);
				tmp->Encode(&kMapLocations, sizeof(TMapLocation));
			}
		}
	}
	else if (peer->GetChannel() == GUILD_WARP_WAR_CHANNEL)
	{
		for (auto tmp : m_peerList)
		{
			if (tmp == peer)
				continue;

			if (!tmp->GetChannel())
				continue;

			if (tmp->GetChannel() == 1 || tmp->GetChannel() == peer->GetChannel())
			{
				TMapLocation kMapLocation2;
				strlcpy(kMapLocation2.szHost, tmp->GetPublicIP(), sizeof(kMapLocation2.szHost));
				kMapLocation2.wPort = tmp->GetListenPort();
				memcpy(kMapLocation2.alMaps, tmp->GetMaps(), sizeof(kMapLocation2.alMaps));
				vec_kMapLocations.push_back(kMapLocation2);
			}

			tmp->EncodeHeader(HEADER_DG_MAP_LOCATIONS, 0, sizeof(uint8_t) + sizeof(TMapLocation));
			bMapCount = 1;
			tmp->EncodeBYTE(bMapCount);
			tmp->Encode(&kMapLocations, sizeof(TMapLocation));
		}
	}
	else
	{
		for (auto tmp : m_peerList)
		{
			if (tmp == peer)
				continue;

			if (!tmp->GetChannel())
				continue;

			if (tmp->GetChannel() == GUILD_WARP_WAR_CHANNEL || tmp->GetChannel() == peer->GetChannel())
			{
				TMapLocation kMapLocation2;

				strlcpy(kMapLocation2.szHost, tmp->GetPublicIP(), sizeof(kMapLocation2.szHost));
				kMapLocation2.wPort = tmp->GetListenPort();
				memcpy(kMapLocation2.alMaps, tmp->GetMaps(), sizeof(kMapLocation2.alMaps));

				vec_kMapLocations.push_back(kMapLocation2);
			}

			if (tmp->GetChannel() == peer->GetChannel())
			{
				tmp->EncodeHeader(HEADER_DG_MAP_LOCATIONS, 0, sizeof(uint8_t) + sizeof(TMapLocation));
				bMapCount = 1;
				tmp->EncodeBYTE(bMapCount);
				tmp->Encode(&kMapLocations, sizeof(TMapLocation));
			}
		}
	}

	vec_kMapLocations.push_back(kMapLocations);

	peer->EncodeHeader(HEADER_DG_MAP_LOCATIONS, 0, sizeof(uint8_t) + sizeof(TMapLocation) * vec_kMapLocations.size());
	bMapCount = static_cast<uint8_t>(vec_kMapLocations.size());
	peer->EncodeBYTE(bMapCount);
	peer->Encode(&vec_kMapLocations[0], sizeof(TMapLocation) * vec_kMapLocations.size());

	//
	// 셋업 : 접속한 피어에 다른 피어들이 접속하게 만든다. (P2P 컨넥션 생성)
	//
	sys_log(0, "SETUP: channel %u listen %u p2p %u count %u", peer->GetChannel(), p->wListenPort, p->wP2PPort, bMapCount);

	TPacketDGP2P p2pSetupPacket;
	p2pSetupPacket.wPort = peer->GetP2PPort();
	p2pSetupPacket.wListenPort = peer->GetListenPort();
	p2pSetupPacket.bChannel = peer->GetChannel();
	strlcpy(p2pSetupPacket.szHost, peer->GetPublicIP(), sizeof(p2pSetupPacket.szHost));

	for (auto tmp : m_peerList)
	{
		if (tmp == peer)
			continue;

		// 채널이 0이라면 아직 SETUP 패킷이 오지 않은 피어 또는 auth라고 간주할 수 있음
		if (0 == tmp->GetChannel())
			continue;

		tmp->EncodeHeader(HEADER_DG_P2P, 0, sizeof(TPacketDGP2P));
		tmp->Encode(&p2pSetupPacket, sizeof(TPacketDGP2P));
	}

	//
	// 로그인 및 빌링정보 보내기
	//
	auto pck = reinterpret_cast<const TPacketLoginOnSetup *>(c_pData);;
	for (uint32_t c = 0; c < p->dwLoginCount; ++c, ++pck)
	{
		auto pkLD = new CLoginData;

		pkLD->SetKey(pck->dwLoginKey);
		pkLD->SetClientKey(pck->adwClientKey);
		pkLD->SetIP(pck->szHost);

		TAccountTable & r = pkLD->GetAccountRef();

		r.id = pck->dwID;
		trim_and_lower(pck->szLogin, r.login, sizeof(r.login));
		strlcpy(r.social_id, pck->szSocialID, sizeof(r.social_id));
		strlcpy(r.passwd, "TEMP", sizeof(r.passwd));

		InsertLoginData(pkLD);

		if (InsertLogonAccount(pck->szLogin, peer->GetHandle(), pck->szHost))
		{
			sys_log(0, "SETUP: login %u %s login_key %u host %s", pck->dwID, pck->szLogin, pck->dwLoginKey, pck->szHost);
			pkLD->SetPlay(true);
		}
		else
			sys_log(0, "SETUP: login_fail %u %s login_key %u", pck->dwID, pck->szLogin, pck->dwLoginKey);
	}

	SendPartyOnSetup(peer);
	CGuildManager::Instance().OnSetup(peer);
	CPrivManager::Instance().SendPrivOnSetup(peer);
	SendEventFlagsOnSetup(peer);
	marriage::CManager::Instance().OnSetup(peer);
}

void CClientManager::QUERY_ITEM_FLUSH(CPeer *, const char * c_pData)
{
	uint32_t dwID = *reinterpret_cast<const uint32_t *>(c_pData);

	if (g_log)
		sys_log(0, "HEADER_GD_ITEM_FLUSH: %u", dwID);

	CItemCache * c = GetItemCache(dwID);

	if (c)
		c->Flush();
}

void CClientManager::QUERY_ITEM_SAVE(CPeer * pkPeer, const char * c_pData)
{
	TPlayerItem * p = (TPlayerItem *) c_pData;

	// 창고면 캐쉬하지 않고, 캐쉬에 있던 것도 빼버려야 한다.

	if (p->window == SAFEBOX || p->window == MALL)
	{
		CItemCache * c = GetItemCache(p->id);

		if (c)
		{
			auto it = m_map_pkItemCacheSetPtr.find(c->Get()->owner);

			if (it != m_map_pkItemCacheSetPtr.end())
			{
				if (g_test_server)
						sys_log(0, "ITEM_CACHE: window %d owner %u id %u", p->window, c->Get()->owner, c->Get()->id);

				it->second->erase(c);
			}

			m_map_itemCache.erase(p->id);

			delete c;
		}
		char szQuery[512];

		snprintf(szQuery, sizeof(szQuery), 
			"REPLACE INTO item (id, owner_id, `window`, pos, count, vnum, is_gm_owner, socket0, socket1, socket2, "
			"attrtype0, attrvalue0, "
			"attrtype1, attrvalue1, "
			"attrtype2, attrvalue2, "
			"attrtype3, attrvalue3, "
			"attrtype4, attrvalue4, "
			"attrtype5, attrvalue5, "
			"attrtype6, attrvalue6) "
			"VALUES(%u, %u, %d, %d, %u, %u, %u, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
			p->id,
			p->owner,
			p->window,
			p->pos,
			p->count,
			p->vnum,
			p->is_gm_owner,
			p->alSockets[0],
			p->alSockets[1],
			p->alSockets[2],
			p->aAttr[0].bType, p->aAttr[0].sValue,
			p->aAttr[1].bType, p->aAttr[1].sValue,
			p->aAttr[2].bType, p->aAttr[2].sValue,
			p->aAttr[3].bType, p->aAttr[3].sValue,
			p->aAttr[4].bType, p->aAttr[4].sValue,
			p->aAttr[5].bType, p->aAttr[5].sValue,
			p->aAttr[6].bType, p->aAttr[6].sValue);

		CDBManager::Instance().ReturnQuery(szQuery, QID_ITEM_SAVE, pkPeer->GetHandle(), nullptr);
	}
	else
	{
		if (g_test_server)
			sys_log(0, "QUERY_ITEM_SAVE => PutItemCache() owner %d id %d vnum %d ", p->owner, p->id, p->vnum);

		PutItemCache(p);
	}
}

CClientManager::TItemCacheSet * CClientManager::GetItemCacheSet(uint32_t pid)
{
	auto it = m_map_pkItemCacheSetPtr.find(pid);

	if (it == m_map_pkItemCacheSetPtr.end())
		return nullptr;

	return it->second;
}

void CClientManager::CreateItemCacheSet(uint32_t pid)
{
	if (m_map_pkItemCacheSetPtr.find(pid) != m_map_pkItemCacheSetPtr.end())
		return;

	auto pSet = new TItemCacheSet;
	m_map_pkItemCacheSetPtr.insert(TItemCacheSetPtrMap::value_type(pid, pSet));

	if (g_log)
		sys_log(0, "ITEM_CACHE: new cache %u", pid);
}

void CClientManager::FlushItemCacheSet(uint32_t pid)
{
	auto it = m_map_pkItemCacheSetPtr.find(pid);

	if (it == m_map_pkItemCacheSetPtr.end())
	{
		sys_log(0, "FLUSH_ITEMCACHESET : No ItemCacheSet pid(%d)", pid);
		return;
	}

	TItemCacheSet * pSet = it->second;
	auto it_set = pSet->begin();

	while (it_set != pSet->end())
	{
		CItemCache * c = *it_set++;
		c->Flush();

		m_map_itemCache.erase(c->Get()->id);
		delete c;
	}

	pSet->clear();
	delete pSet;

	m_map_pkItemCacheSetPtr.erase(it);

	if (g_log)
		sys_log(0, "FLUSH_ITEMCACHESET : Deleted pid(%d)", pid);
}

CItemCache * CClientManager::GetItemCache(uint32_t id)
{
	auto it = m_map_itemCache.find(id);

	if (it == m_map_itemCache.end())
		return nullptr;

	return it->second;
}

void CClientManager::PutItemCache(const TPlayerItem * pNew, bool bSkipQuery)
{       
	CItemCache * c;     

	c = GetItemCache(pNew->id);
	
	// 아이템 새로 생성
	if (!c)
	{
		if (g_log)
			sys_log(0, "ITEM_CACHE: PutItemCache ==> New CItemCache id%d vnum%d new owner%d", pNew->id, pNew->vnum, pNew->owner);

		c = new CItemCache;
		m_map_itemCache.insert(TItemCacheMap::value_type(pNew->id, c));
	}
	// 있을시
	else
	{
		if (g_log)
			sys_log(0, "ITEM_CACHE: PutItemCache ==> Have Cache");
		// 소유자가 틀리면
		if (pNew->owner != c->Get()->owner)
		{
			// 이미 이 아이템을 가지고 있었던 유저로 부터 아이템을 삭제한다.
			auto it = m_map_pkItemCacheSetPtr.find(c->Get()->owner);

			if (it != m_map_pkItemCacheSetPtr.end())
			{
				if (g_log)
				sys_log(0, "ITEM_CACHE: delete owner %u id %u new owner %u", c->Get()->owner, c->Get()->id, pNew->owner);
				it->second->erase(c);
			}
		}
	}

	// 새로운 정보 업데이트 
	c->Put(pNew, bSkipQuery);
	
	auto it = m_map_pkItemCacheSetPtr.find(c->Get()->owner);

	if (it != m_map_pkItemCacheSetPtr.end())
	{
		if (g_log)
			sys_log(0, "ITEM_CACHE: save %u id %u", c->Get()->owner, c->Get()->id);
		else
			sys_log(1, "ITEM_CACHE: save %u id %u", c->Get()->owner, c->Get()->id);
		it->second->insert(c);
	}
	else
	{
		// 현재 소유자가 없으므로 바로 저장해야 다음 접속이 올 때 SQL에 쿼리하여
		// 받을 수 있으므로 바로 저장한다.
		if (g_log)
			sys_log(0, "ITEM_CACHE: direct save %u id %u", c->Get()->owner, c->Get()->id);
		else
			sys_log(1, "ITEM_CACHE: direct save %u id %u", c->Get()->owner, c->Get()->id);

		c->OnFlush();
	}
}

bool CClientManager::DeleteItemCache(uint32_t dwID)
{
	CItemCache * c = GetItemCache(dwID);

	if (!c)
		return false;

	c->Delete();
	return true;
}

// MYSHOP_PRICE_LIST
CItemPriceListTableCache* CClientManager::GetItemPriceListCache(uint32_t dwID)
{
	auto it = m_mapItemPriceListCache.find(dwID);

	if (it == m_mapItemPriceListCache.end())
		return nullptr;

	return it->second;
}

void CClientManager::PutItemPriceListCache(TItemPriceListTable* pItemPriceList)
{
	CItemPriceListTableCache* pCache = GetItemPriceListCache(pItemPriceList->dwOwnerID);

	if (!pCache)
	{
		pCache = new CItemPriceListTableCache;
		m_mapItemPriceListCache.insert(TItemPriceListCacheMap::value_type(pItemPriceList->dwOwnerID, pCache));
	}

	pCache->Put(pItemPriceList, true);
}

void CClientManager::UpdatePlayerCache()
{
	auto it = m_map_playerCache.begin();

	while (it != m_map_playerCache.end())
	{
		CPlayerTableCache * c = (it++)->second;

		if (c->CheckTimeout())
		{
			if (g_log)
				sys_log(0, "UPDATE : UpdatePlayerCache() ==> FlushPlayerCache %d %s ", c->Get(false)->id, c->Get(false)->name);

			c->Flush();

			// Item Cache도 업데이트
			UpdateItemCacheSet(c->Get()->id);
		}
		else if (c->CheckFlushTimeout())
			c->Flush();
	}
}
// END_OF_MYSHOP_PRICE_LIST

void CClientManager::SetCacheFlushCountLimit(int32_t iLimit)
{
	m_iCacheFlushCountLimit = MAX(10, iLimit);
	sys_log(0, "CACHE_FLUSH_LIMIT_PER_SECOND: %d", m_iCacheFlushCountLimit);
}

void CClientManager::UpdateItemCache()
{
	if (m_iCacheFlushCount >= m_iCacheFlushCountLimit)
		return;

	auto it = m_map_itemCache.begin();

	while (it != m_map_itemCache.end())
	{
		CItemCache * c = (it++)->second;

		// 아이템은 Flush만 한다.
		if (c->CheckFlushTimeout())
		{
			if (g_test_server)
				sys_log(0, "UpdateItemCache ==> Flush() vnum %d id owner %d", c->Get()->vnum, c->Get()->id, c->Get()->owner);

			c->Flush();

			if (++m_iCacheFlushCount >= m_iCacheFlushCountLimit)
				break;
		}
	}
}

void CClientManager::UpdateItemPriceListCache()
{
	auto it = m_mapItemPriceListCache.begin();

	while (it != m_mapItemPriceListCache.end())
	{
		CItemPriceListTableCache* pCache = it->second;

		if (pCache->CheckFlushTimeout())
		{
			pCache->Flush();
			m_mapItemPriceListCache.erase(it++);
			delete pCache;
		}
		else
			++it;
	}
}

void CClientManager::QUERY_ITEM_DESTROY(CPeer * pkPeer, const char * c_pData)
{
	uint32_t dwID = *(uint32_t *) c_pData;
	c_pData += sizeof(uint32_t);

	uint32_t dwPID = *(uint32_t *) c_pData;

	if (!DeleteItemCache(dwID))
	{
		char szQuery[64];
		snprintf(szQuery, sizeof(szQuery), "DELETE FROM item WHERE id=%u", dwID);

		if (g_log)
			sys_log(0, "HEADER_GD_ITEM_DESTROY: PID %u ID %u", dwPID, dwID);

		if (dwPID == 0) // 아무도 가진 사람이 없었다면, 비동기 쿼리
			CDBManager::Instance().AsyncQuery(szQuery);
		else
			CDBManager::Instance().ReturnQuery(szQuery, QID_ITEM_DESTROY, pkPeer->GetHandle(), nullptr);
	}
}

void CClientManager::QUERY_FLUSH_CACHE(CPeer *, const char * c_pData)
{
	uint32_t dwPID = *(uint32_t *) c_pData;

	CPlayerTableCache * pkCache = GetPlayerCache(dwPID);

	if (!pkCache)
		return;

	sys_log(0, "FLUSH_CACHE: %u", dwPID);

	pkCache->Flush();
	FlushItemCacheSet(dwPID);

	m_map_playerCache.erase(dwPID);
	delete pkCache;
}

void CClientManager::QUERY_RELOAD_PROTO()
{
	if (!InitializeTables())
	{
		sys_err("QUERY_RELOAD_PROTO: cannot load tables");
		return;
	}

	for (TPeerList::iterator i = m_peerList.begin(); i != m_peerList.end(); ++i)
	{
		CPeer * tmp = *i;

		if (!tmp->GetChannel())
			continue;

		tmp->EncodeHeader(HEADER_DG_RELOAD_PROTO, 0, 
				sizeof(uint16_t) + sizeof(TSkillTable) * m_vec_skillTable.size() +
				sizeof(uint16_t) + sizeof(TBanwordTable) * m_vec_banwordTable.size() +
				sizeof(uint16_t) + sizeof(SItemTable_Server) * m_vec_itemTable.size() +
				sizeof(uint16_t) + sizeof(TMobTable) * m_vec_mobTable.size());

		tmp->EncodeWORD(static_cast<uint16_t>(m_vec_skillTable.size()));
		tmp->Encode(&m_vec_skillTable[0], sizeof(TSkillTable) * m_vec_skillTable.size());

		tmp->EncodeWORD(static_cast<uint16_t>(m_vec_banwordTable.size()));
		tmp->Encode(&m_vec_banwordTable[0], sizeof(TBanwordTable) * m_vec_banwordTable.size());

		tmp->EncodeWORD(static_cast<uint16_t>(m_vec_itemTable.size()));
		tmp->Encode(&m_vec_itemTable[0], sizeof(SItemTable_Server) * m_vec_itemTable.size());

		tmp->EncodeWORD(static_cast<uint16_t>(m_vec_mobTable.size()));
		tmp->Encode(&m_vec_mobTable[0], sizeof(TMobTable) * m_vec_mobTable.size());
	}
}

// ADD_GUILD_PRIV_TIME
/**
 * @version	05/06/08 Bang2ni - 지속시간 추가
 */
void CClientManager::AddGuildPriv(TPacketGiveGuildPriv* p)
{
	CPrivManager::Instance().AddGuildPriv(p->guild_id, p->type, p->value, p->duration_sec);

	__UpdateDefaultPriv("GUILD", p->guild_id, p->type, p->value, p->duration_sec);
}

void CClientManager::AddEmpirePriv(TPacketGiveEmpirePriv* p)
{
	CPrivManager::Instance().AddEmpirePriv(p->empire, p->type, p->value, p->duration_sec);

	__UpdateDefaultPriv("EMPIRE", p->empire, p->type, p->value, p->duration_sec);
}
// END_OF_ADD_GUILD_PRIV_TIME

void CClientManager::AddCharacterPriv(TPacketGiveCharacterPriv* p)
{
	CPrivManager::Instance().AddCharPriv(p->pid, p->type, p->value);

	__UpdateDefaultPriv("PLAYER", p->pid, p->type, p->value, 0);
}

CLoginData * CClientManager::GetLoginData(uint32_t dwKey)
{
	auto it = m_map_pkLoginData.find(dwKey);

	if (it == m_map_pkLoginData.end())
		return nullptr;

	return it->second;
}

CLoginData * CClientManager::GetLoginDataByLogin(const char * c_pszLogin)
{
	char szLogin[LOGIN_MAX_LEN + 1];
	trim_and_lower(c_pszLogin, szLogin, sizeof(szLogin));

	auto it = m_map_pkLoginDataByLogin.find(szLogin);

	if (it == m_map_pkLoginDataByLogin.end())
		return nullptr;

	return it->second;
}

CLoginData * CClientManager::GetLoginDataByAID(uint32_t dwAID)
{
	auto it = m_map_pkLoginDataByAID.find(dwAID);

	if (it == m_map_pkLoginDataByAID.end())
		return nullptr;

	return it->second;
}

void CClientManager::InsertLoginData(CLoginData * pkLD)
{
	char szLogin[LOGIN_MAX_LEN + 1];
	trim_and_lower(pkLD->GetAccountRef().login, szLogin, sizeof(szLogin));

	m_map_pkLoginData.insert(std::make_pair(pkLD->GetKey(), pkLD));
	m_map_pkLoginDataByLogin.insert(std::make_pair(szLogin, pkLD));
	m_map_pkLoginDataByAID.insert(std::make_pair(pkLD->GetAccountRef().id, pkLD));
}

void CClientManager::DeleteLoginData(CLoginData * pkLD)
{
	m_map_pkLoginData.erase(pkLD->GetKey());
	m_map_pkLoginDataByLogin.erase(pkLD->GetAccountRef().login);
	m_map_pkLoginDataByAID.erase(pkLD->GetAccountRef().id);

	if (m_map_kLogonAccount.find(pkLD->GetAccountRef().login) == m_map_kLogonAccount.end())
		delete pkLD;
	else
		pkLD->SetDeleted(true);
}

void CClientManager::QUERY_AUTH_LOGIN(CPeer * pkPeer, uint32_t dwHandle, TPacketGDAuthLogin * p)
{
	if (g_test_server)
		sys_log(0, "QUERY_AUTH_LOGIN %d %d %s", p->dwID, p->dwLoginKey, p->szLogin);
	CLoginData * pkLD = GetLoginDataByLogin(p->szLogin);

	if (pkLD)
	{
		DeleteLoginData(pkLD);
	}

	uint8_t bResult;

	if (GetLoginData(p->dwLoginKey))
	{
		sys_err("LoginData already exist key %u login %s", p->dwLoginKey, p->szLogin);
		bResult = 0;

		pkPeer->EncodeHeader(HEADER_DG_AUTH_LOGIN, dwHandle, sizeof(uint8_t));
		pkPeer->EncodeBYTE(bResult);
	}
	else
	{
		CLoginData * pkLD2 = new CLoginData;

		pkLD2->SetKey(p->dwLoginKey);
		pkLD2->SetClientKey(p->adwClientKey);
		pkLD2->SetPremium(p->iPremiumTimes);

		TAccountTable & r = pkLD2->GetAccountRef();

		r.id = p->dwID;
		trim_and_lower(p->szLogin, r.login, sizeof(r.login));
		strlcpy(r.social_id, p->szSocialID, sizeof(r.social_id));
		strlcpy(r.passwd, "TEMP", sizeof(r.passwd));

		sys_log(0, "AUTH_LOGIN id(%u) login(%s) social_id(%s) login_key(%u), client_key(%u %u %u %u)",
				p->dwID, p->szLogin, p->szSocialID, p->dwLoginKey,
				p->adwClientKey[0], p->adwClientKey[1], p->adwClientKey[2], p->adwClientKey[3]);

		bResult = 1;

		InsertLoginData(pkLD2);

		pkPeer->EncodeHeader(HEADER_DG_AUTH_LOGIN, dwHandle, sizeof(uint8_t));
		pkPeer->EncodeBYTE(bResult);
	}
}

void CClientManager::GuildDepositMoney(TPacketGDGuildMoney* p)
{
	CGuildManager::Instance().DepositMoney(p->dwGuild, p->iGold);
}

void CClientManager::GuildWithdrawMoney(CPeer* peer, TPacketGDGuildMoney* p)
{
	CGuildManager::Instance().WithdrawMoney(peer, p->dwGuild, p->iGold);
}

void CClientManager::GuildWithdrawMoneyGiveReply(TPacketGDGuildMoneyWithdrawGiveReply* p)
{
	CGuildManager::Instance().WithdrawMoneyReply(p->dwGuild, p->bGiveSuccess, p->iChangeGold);
}

void CClientManager::GuildWarBet(TPacketGDGuildWarBet * p)
{
	CGuildManager::Instance().Bet(p->dwWarID, p->szLogin, p->dwGold, p->dwGuild);
}

void CClientManager::CreateObject(TPacketGDCreateObject * p)
{
	using namespace building;

	char szQuery[512];

	snprintf(szQuery, sizeof(szQuery),
			"INSERT INTO object (land_id, vnum, map_index, x, y, x_rot, y_rot, z_rot) VALUES(%u, %u, %d, %d, %d, %f, %f, %f)",
			p->dwLandID, p->dwVnum, p->lMapIndex, p->x, p->y, p->xRot, p->yRot, p->zRot);

	std::unique_ptr<SQLMsg> pmsg(CDBManager::Instance().DirectQuery(szQuery));

	if (pmsg->Get()->uiInsertID == 0)
	{
		sys_err("cannot insert object");
		return;
	}

	auto pkObj = new TObject;

	memset(pkObj, 0, sizeof(TObject));

	pkObj->dwID = static_cast<uint32_t>(pmsg->Get()->uiInsertID);
	pkObj->dwVnum = p->dwVnum;
	pkObj->dwLandID = p->dwLandID;
	pkObj->lMapIndex = p->lMapIndex;
	pkObj->x = p->x;
	pkObj->y = p->y;
	pkObj->xRot = p->xRot;
	pkObj->yRot = p->yRot;
	pkObj->zRot = p->zRot;
	pkObj->lLife = 0;

	ForwardPacket(HEADER_DG_CREATE_OBJECT, pkObj, sizeof(TObject));

	m_map_pkObjectTable.insert(std::make_pair(pkObj->dwID, pkObj));
}

void CClientManager::DeleteObject(uint32_t dwID)
{
	char szQuery[128];

	snprintf(szQuery, sizeof(szQuery), "DELETE FROM object WHERE id=%u", dwID);

	std::unique_ptr<SQLMsg> pmsg(CDBManager::Instance().DirectQuery(szQuery));

	if (pmsg->Get()->uiAffectedRows == 0 || pmsg->Get()->uiAffectedRows == static_cast<uint32_t>(-1))
	{
		sys_err("no object by id %u", dwID);
		return;
	}

	auto it = m_map_pkObjectTable.find(dwID);

	if (it != m_map_pkObjectTable.end())
	{
		delete it->second;
		m_map_pkObjectTable.erase(it);
	}

	ForwardPacket(HEADER_DG_DELETE_OBJECT, &dwID, sizeof(uint32_t));
}

// BLOCK_CHAT
void CClientManager::BlockChat(TPacketBlockChat* p)
{
	char szQuery[256];
	snprintf(szQuery, sizeof(szQuery), "SELECT id FROM player WHERE name = '%s'", p->szName);

	std::unique_ptr<SQLMsg> pmsg(CDBManager::Instance().DirectQuery(szQuery));
	SQLResult * pRes = pmsg->Get();

	if (pRes->uiNumRows)
	{
		MYSQL_ROW row = mysql_fetch_row(pRes->pSQLResult);
		uint32_t pid = strtoul(row[0], nullptr, 10);

		TPacketGDAddAffect pa;
		pa.dwPID = pid;
		pa.elem.dwType = 223;
		pa.elem.bApplyOn = 0;
		pa.elem.lApplyValue = 0;
		pa.elem.dwFlag = 0;
		pa.elem.lDuration = p->lDuration;
		pa.elem.lSPCost = 0;
		QUERY_ADD_AFFECT(nullptr, &pa);
	}
}
// END_OF_BLOCK_CHAT

void CClientManager::MarriageAdd(TPacketMarriageAdd * p)
{
	sys_log(0, "MarriageAdd %u %u %s %s", p->dwPID1, p->dwPID2, p->szName1, p->szName2);
	marriage::CManager::Instance().Add(p->dwPID1, p->dwPID2, p->szName1, p->szName2);
}

void CClientManager::MarriageUpdate(TPacketMarriageUpdate * p)
{
	sys_log(0, "MarriageUpdate PID:%u %u LP:%d ST:%d", p->dwPID1, p->dwPID2, p->iLovePoint, p->byMarried);
	marriage::CManager::Instance().Update(p->dwPID1, p->dwPID2, p->iLovePoint, p->byMarried);
}

void CClientManager::MarriageRemove(TPacketMarriageRemove * p)
{
	sys_log(0, "MarriageRemove %u %u", p->dwPID1, p->dwPID2);
	marriage::CManager::Instance().Remove(p->dwPID1, p->dwPID2);
}

void CClientManager::WeddingRequest(TPacketWeddingRequest * p)
{
	sys_log(0, "WeddingRequest %u %u", p->dwPID1, p->dwPID2);
	ForwardPacket(HEADER_DG_WEDDING_REQUEST, p, sizeof(TPacketWeddingRequest));
	//marriage::CManager::Instance().RegisterWedding(p->dwPID1, p->szName1, p->dwPID2, p->szName2);
}

void CClientManager::WeddingReady(TPacketWeddingReady * p)
{
	sys_log(0, "WeddingReady %u %u", p->dwPID1, p->dwPID2);
	ForwardPacket(HEADER_DG_WEDDING_READY, p, sizeof(TPacketWeddingReady));
	marriage::CManager::Instance().ReadyWedding(p->dwMapIndex, p->dwPID1, p->dwPID2);
}

void CClientManager::WeddingEnd(TPacketWeddingEnd * p)
{
	sys_log(0, "WeddingEnd %u %u", p->dwPID1, p->dwPID2);
	marriage::CManager::Instance().EndWedding(p->dwPID1, p->dwPID2);
}

//
// 캐시에 가격정보가 있으면 캐시를 업데이트 하고 캐시에 가격정보가 없다면
// 우선 기존의 데이터를 로드한 뒤에 기존의 정보로 캐시를 만들고 새로 받은 가격정보를 업데이트 한다.
//
void CClientManager::MyshopPricelistUpdate(const TItemPriceListTable* pPacket)
{
	if (pPacket->byCount > SHOP_PRICELIST_MAX_NUM)
	{
		sys_err("count overflow!");
		return;
	}

	CItemPriceListTableCache* pCache = GetItemPriceListCache(pPacket->dwOwnerID);

	if (pCache)
	{
		TItemPriceListTable table;

		table.dwOwnerID = pPacket->dwOwnerID;
		table.byCount = pPacket->byCount;

		memcpy(table.aPriceInfo, pPacket->aPriceInfo, sizeof(TItemPriceInfo) * pPacket->byCount);

		pCache->UpdateList(&table);
		pCache->OnFlush();
	}
	else
	{
		auto pUpdateTable = new TItemPriceListTable;

		pUpdateTable->dwOwnerID = pPacket->dwOwnerID;
		pUpdateTable->byCount = pPacket->byCount;

		memcpy(pUpdateTable->aPriceInfo, pPacket->aPriceInfo, sizeof(TItemPriceInfo) * pPacket->byCount);

		char szQuery[ASQL_QUERY_MAX_LEN];
		snprintf(szQuery, sizeof(szQuery), "SELECT item_vnum, price FROM myshop_pricelist WHERE owner_id=%u", pUpdateTable->dwOwnerID);
		CDBManager::Instance().ReturnQuery(szQuery, QID_ITEMPRICE_LOAD_FOR_UPDATE, 0, pUpdateTable);
	}
}

// MYSHOP_PRICE_LIST
// 캐시된 가격정보가 있으면 캐시를 읽어 바로 전송하고 캐시에 정보가 없으면 DB 에 쿼리를 한다.
//
void CClientManager::MyshopPricelistRequest(CPeer* peer, uint32_t dwHandle, uint32_t dwPlayerID)
{
	if (CItemPriceListTableCache* pCache = GetItemPriceListCache(dwPlayerID))
	{
		sys_log(0, "Cache MyShopPricelist handle[%d] pid[%d]", dwHandle, dwPlayerID);

		TItemPriceListTable* pTable = pCache->Get(false);

		TPacketMyshopPricelistHeader header =
		{
			pTable->dwOwnerID,
			pTable->byCount
		};

		size_t sizePriceListSize = sizeof(TItemPriceInfo) * pTable->byCount;

		peer->EncodeHeader(HEADER_DG_MYSHOP_PRICELIST_RES, dwHandle, sizeof(header) + sizePriceListSize);
		peer->Encode(&header, sizeof(header));
		peer->Encode(pTable->aPriceInfo, sizePriceListSize);

	}
	else
	{
		sys_log(0, "Query MyShopPricelist handle[%d] pid[%d]", dwHandle, dwPlayerID);

		char szQuery[ASQL_QUERY_MAX_LEN];
		snprintf(szQuery, sizeof(szQuery), "SELECT item_vnum, price FROM myshop_pricelist WHERE owner_id=%u", dwPlayerID);
		CDBManager::Instance().ReturnQuery(szQuery, QID_ITEMPRICE_LOAD, peer->GetHandle(), new TItemPricelistReqInfo(dwHandle, dwPlayerID));
	}
}
// END_OF_MYSHOP_PRICE_LIST

void CPacketInfo::Add(int32_t header)
{
	auto it = m_map_info.find(header);

	if (it == m_map_info.end())
		m_map_info.insert(std::map<int32_t, int32_t>::value_type(header, 1));
	else
		++it->second;
}

void CPacketInfo::Reset()
{
	m_map_info.clear();
}

void CClientManager::ProcessPackets(CPeer * peer)
{
	uint8_t		header;
	uint32_t		dwHandle;
	uint32_t		dwLength;
	const char * data = nullptr;
	int32_t			i = 0;
	int32_t			iCount = 0;

	while (peer->PeekPacket(i, header, dwHandle, dwLength, &data))
	{
		// DISABLE_DB_HEADER_LOG
		// sys_log(0, "header %d %p size %d", header, this, dwLength);
		// END_OF_DISABLE_DB_HEADER_LOG
		m_bLastHeader = header;
		++iCount;

		if (g_test_server)
		{
			if (header != 10)
				sys_log(0, " ProcessPacket Header [%d] Handle[%d] Length[%d] iCount[%d]", header, dwHandle, dwLength, iCount);
		}

		switch (header)
		{
			case HEADER_GD_BOOT:
				QUERY_BOOT(peer, (TPacketGDBoot *) data);
				break;

			case HEADER_GD_LOGIN_BY_KEY:
				QUERY_LOGIN_BY_KEY(peer, dwHandle, (TPacketGDLoginByKey *) data);
				break;

			case HEADER_GD_LOGOUT:
				//sys_log(0, "HEADER_GD_LOGOUT (handle: %d length: %d)", dwHandle, dwLength);
				QUERY_LOGOUT(peer, dwHandle, data);
				break;

			case HEADER_GD_PLAYER_LOAD:
				sys_log(1, "HEADER_GD_PLAYER_LOAD (handle: %d length: %d)", dwHandle, dwLength);
				QUERY_PLAYER_LOAD(peer, dwHandle, (TPlayerLoadPacket *) data);
				break;

			case HEADER_GD_PLAYER_SAVE:
				sys_log(1, "HEADER_GD_PLAYER_SAVE (handle: %d length: %d)", dwHandle, dwLength);
				QUERY_PLAYER_SAVE(peer, dwHandle, (TPlayerTable *) data);
				break;

			case HEADER_GD_PLAYER_CREATE:
				sys_log(0, "HEADER_GD_PLAYER_CREATE (handle: %d length: %d)", dwHandle, dwLength);
				__QUERY_PLAYER_CREATE(peer, dwHandle, (TPlayerCreatePacket *) data);
				sys_log(0, "END PLAYER_CREATE");
				break;

			case HEADER_GD_PLAYER_DELETE:
				sys_log(1, "HEADER_GD_PLAYER_DELETE (handle: %d length: %d)", dwHandle, dwLength);
				__QUERY_PLAYER_DELETE(peer, dwHandle, (TPlayerDeletePacket *) data);
				break;

			case HEADER_GD_QUEST_SAVE:
				sys_log(1, "HEADER_GD_QUEST_SAVE (handle: %d length: %d)", dwHandle, dwLength);
				QUERY_QUEST_SAVE(peer, (TQuestTable *) data, dwLength);
				break;

			case HEADER_GD_SAFEBOX_LOAD:
				QUERY_SAFEBOX_LOAD(peer, dwHandle, (TSafeboxLoadPacket *) data, 0);
				break;

			case HEADER_GD_SAFEBOX_SAVE:
				sys_log(1, "HEADER_GD_SAFEBOX_SAVE (handle: %d length: %d)", dwHandle, dwLength);
				QUERY_SAFEBOX_SAVE(peer, (TSafeboxTable *) data);
				break;

			case HEADER_GD_SAFEBOX_CHANGE_SIZE:
				QUERY_SAFEBOX_CHANGE_SIZE(peer, dwHandle, (TSafeboxChangeSizePacket *) data);
				break;

			case HEADER_GD_SAFEBOX_CHANGE_PASSWORD:
				QUERY_SAFEBOX_CHANGE_PASSWORD(peer, dwHandle, (TSafeboxChangePasswordPacket *) data);
				break;

			case HEADER_GD_MALL_LOAD:
				QUERY_SAFEBOX_LOAD(peer, dwHandle, (TSafeboxLoadPacket *) data, 1);
				break;

			case HEADER_GD_EMPIRE_SELECT:
				QUERY_EMPIRE_SELECT(peer, dwHandle, (TEmpireSelectPacket *) data);
				break;

			case HEADER_GD_SETUP:
				QUERY_SETUP(peer, dwHandle, data);
				break;

			case HEADER_GD_GUILD_CREATE:
				GuildCreate(peer, *(uint32_t *) data);
				break;

			case HEADER_GD_GUILD_SKILL_UPDATE:
				GuildSkillUpdate(peer, (TPacketGuildSkillUpdate *) data);		
				break;

			case HEADER_GD_GUILD_EXP_UPDATE:
				GuildExpUpdate(peer, (TPacketGuildExpUpdate *) data);
				break;

			case HEADER_GD_GUILD_ADD_MEMBER:
				GuildAddMember(peer, (TPacketGDGuildAddMember*) data);
				break;

			case HEADER_GD_GUILD_REMOVE_MEMBER:
				GuildRemoveMember(peer, (TPacketGuild*) data);
				break;

			case HEADER_GD_GUILD_CHANGE_GRADE:
				GuildChangeGrade(peer, (TPacketGuild*) data);
				break;

			case HEADER_GD_GUILD_CHANGE_MEMBER_DATA:
				GuildChangeMemberData(peer, (TPacketGuildChangeMemberData*) data);
				break;

			case HEADER_GD_GUILD_DISBAND:
				GuildDisband(peer, (TPacketGuild*) data);
				break;

			case HEADER_GD_GUILD_WAR:
				GuildWar(peer, (TPacketGuildWar*) data);
				break;

			case HEADER_GD_GUILD_WAR_SCORE:
				GuildWarScore(peer, (TPacketGuildWarScore*) data);
				break;

			case HEADER_GD_GUILD_CHANGE_LADDER_POINT:
				GuildChangeLadderPoint((TPacketGuildLadderPoint*) data);
				break;

			case HEADER_GD_GUILD_USE_SKILL:
				GuildUseSkill((TPacketGuildUseSkill*) data);
				break;

			case HEADER_GD_FLUSH_CACHE:
				QUERY_FLUSH_CACHE(peer, data);
				break;

			case HEADER_GD_ITEM_SAVE:
				QUERY_ITEM_SAVE(peer, data);
				break;

			case HEADER_GD_ITEM_DESTROY:
				QUERY_ITEM_DESTROY(peer, data);
				break;

			case HEADER_GD_ITEM_FLUSH:
				QUERY_ITEM_FLUSH(peer, data);
				break;

			case HEADER_GD_ADD_AFFECT:
				sys_log(1, "HEADER_GD_ADD_AFFECT");
				QUERY_ADD_AFFECT(peer, (TPacketGDAddAffect *) data);
				break;

			case HEADER_GD_REMOVE_AFFECT:
				sys_log(1, "HEADER_GD_REMOVE_AFFECT");
				QUERY_REMOVE_AFFECT(peer, (TPacketGDRemoveAffect *) data);
				break;

			case HEADER_GD_PARTY_CREATE:
				QUERY_PARTY_CREATE(peer, (TPacketPartyCreate*) data);
				break;

			case HEADER_GD_PARTY_DELETE:
				QUERY_PARTY_DELETE(peer, (TPacketPartyDelete*) data);
				break;

			case HEADER_GD_PARTY_ADD:
				QUERY_PARTY_ADD(peer, (TPacketPartyAdd*) data);
				break;

			case HEADER_GD_PARTY_REMOVE:
				QUERY_PARTY_REMOVE(peer, (TPacketPartyRemove*) data);
				break;

			case HEADER_GD_PARTY_STATE_CHANGE:
				QUERY_PARTY_STATE_CHANGE(peer, (TPacketPartyStateChange*) data);
				break;

			case HEADER_GD_PARTY_SET_MEMBER_LEVEL:
				QUERY_PARTY_SET_MEMBER_LEVEL(peer, (TPacketPartySetMemberLevel*) data);
				break;

			case HEADER_GD_RELOAD_PROTO:
				QUERY_RELOAD_PROTO();
				break;

			case HEADER_GD_CHANGE_NAME:
				QUERY_CHANGE_NAME(peer, dwHandle, (TPacketGDChangeName *) data);
				break;

			case HEADER_GD_AUTH_LOGIN:
				QUERY_AUTH_LOGIN(peer, dwHandle, (TPacketGDAuthLogin *) data);
				break;

			case HEADER_GD_REQUEST_GUILD_PRIV:
				AddGuildPriv((TPacketGiveGuildPriv*)data);
				break;

			case HEADER_GD_REQUEST_EMPIRE_PRIV:
				AddEmpirePriv((TPacketGiveEmpirePriv*)data);
				break;

			case HEADER_GD_REQUEST_CHARACTER_PRIV:
				AddCharacterPriv((TPacketGiveCharacterPriv*) data);
				break;

			case HEADER_GD_GUILD_DEPOSIT_MONEY:
				GuildDepositMoney((TPacketGDGuildMoney*)data);
				break;

			case HEADER_GD_GUILD_WITHDRAW_MONEY:
				GuildWithdrawMoney(peer, (TPacketGDGuildMoney*)data);
				break;

			case HEADER_GD_GUILD_WITHDRAW_MONEY_GIVE_REPLY:
				GuildWithdrawMoneyGiveReply((TPacketGDGuildMoneyWithdrawGiveReply*)data);
				break;

			case HEADER_GD_GUILD_WAR_BET:
				GuildWarBet((TPacketGDGuildWarBet *) data);
				break;

			case HEADER_GD_SET_EVENT_FLAG:
				SetEventFlag((TPacketSetEventFlag*) data);
				break;

			case HEADER_GD_CREATE_OBJECT:
				CreateObject((TPacketGDCreateObject *) data);
				break;

			case HEADER_GD_DELETE_OBJECT:
				DeleteObject(*(uint32_t *) data);
				break;

			case HEADER_GD_MARRIAGE_ADD:
				MarriageAdd((TPacketMarriageAdd *) data);
				break;

			case HEADER_GD_MARRIAGE_UPDATE:
				MarriageUpdate((TPacketMarriageUpdate *) data);
				break;

			case HEADER_GD_MARRIAGE_REMOVE:
				MarriageRemove((TPacketMarriageRemove *) data);
				break;

			case HEADER_GD_WEDDING_REQUEST:
				WeddingRequest((TPacketWeddingRequest *) data);
				break;

			case HEADER_GD_WEDDING_READY:
				WeddingReady((TPacketWeddingReady *) data);
				break;

			case HEADER_GD_WEDDING_END:
				WeddingEnd((TPacketWeddingEnd *) data);
				break;

				// BLOCK_CHAT
			case HEADER_GD_BLOCK_CHAT:
				BlockChat((TPacketBlockChat *) data);
				break;
				// END_OF_BLOCK_CHAT

				// MYSHOP_PRICE_LIST
			case HEADER_GD_MYSHOP_PRICELIST_UPDATE:
				MyshopPricelistUpdate((TItemPriceListTable*)data);
				break;

			case HEADER_GD_MYSHOP_PRICELIST_REQ:
				MyshopPricelistRequest(peer, dwHandle, *(uint32_t*)data);
				break;
				// END_OF_MYSHOP_PRICE_LIST
		
				//RELOAD_ADMIN
			case HEADER_GD_RELOAD_ADMIN:
				ReloadAdmin();
				break;
				//END_RELOAD_ADMIN

			case HEADER_GD_BREAK_MARRIAGE:
				BreakMarriage(peer, data);
				break;

			case HEADER_GD_REQ_SPARE_ITEM_ID_RANGE :
				SendSpareItemIDRange(peer);
				break;

			case HEADER_GD_REQ_CHANGE_GUILD_MASTER :
				GuildChangeMaster((TPacketChangeGuildMaster*) data);
				break;

			case HEADER_GD_DC:
				DeleteLoginKey((TPacketDC*) data);
				break;

			case HEADER_GD_VALID_LOGOUT:
				ResetLastPlayerID((TPacketNeedLoginLogInfo*)data);
				break;

			case HEADER_GD_REQUEST_CHARGE_CASH:
				ChargeCash((TRequestChargeCash*)data);
				break;

			//delete gift notify icon

			case HEADER_GD_DELETE_AWARDID:
				DeleteAwardId((TPacketDeleteAwardID*) data);
				break;

			case HEADER_GD_UPDATE_CHANNELSTATUS:
				UpdateChannelStatus((SChannelStatus*) data);
				break;
			case HEADER_GD_REQUEST_CHANNELSTATUS:
				RequestChannelStatus(peer, dwHandle);
				break;

			//Activity
			case HEADER_GD_SAVE_ACTIVITY:
			{
				sys_log(1, "HEADER_GD_SAVE_ACTIVITY (lenght: %d)", dwLength);
				QUERY_SAVE_ACTIVITY(peer, dwHandle, (TActivityTable*)data);
				break;
			}

			default:					
				sys_err("Unknown header (header: %d handle: %d length: %d)", header, dwHandle, dwLength);
				break;
		}
	}

	peer->RecvEnd(i);
}

void CClientManager::AddPeer(socket_t fd)
{
	auto pPeer = new CPeer;

	if (pPeer->Accept(fd))
		m_peerList.push_front(pPeer);
	else
		delete pPeer;
}

void CClientManager::RemovePeer(CPeer * pPeer)
{
	if (m_pkAuthPeer == pPeer)
	{
		m_pkAuthPeer = nullptr;
	}
	else
	{
		TLogonAccountMap::iterator it = m_map_kLogonAccount.begin();

		while (it != m_map_kLogonAccount.end())
		{
			CLoginData * pkLD = it->second;

			if (pkLD->GetConnectedPeerHandle() == pPeer->GetHandle())
			{
				if (pkLD->IsPlay())
				{
					pkLD->SetPlay(false);
				}

				if (pkLD->IsDeleted())
				{
					sys_log(0, "DELETING LoginData");
					delete pkLD;
				}

				m_map_kLogonAccount.erase(it++);
			}
			else
				++it;
		}
	}

	m_peerList.remove(pPeer);
	delete pPeer;
}

CPeer * CClientManager::GetPeer(uint32_t ident)
{
	for (auto tmp : m_peerList)
	{
		if (tmp->GetHandle() == ident)
			return tmp;
	}

	return nullptr;
}

CPeer * CClientManager::GetAnyPeer()
{
	if (m_peerList.empty())
		return nullptr;

	return m_peerList.front();
}

// DB 매니저로 부터 받은 결과를 처리한다.
//
// @version	05/06/10 Bang2ni - 가격정보 관련 쿼리(QID_ITEMPRICE_XXX) 추가
int32_t CClientManager::AnalyzeQueryResult(SQLMsg * msg)
{
	CQueryInfo * qi = static_cast<CQueryInfo *>(msg->pvUserData);
	CPeer * peer = GetPeer(qi->dwIdent);

	switch (qi->iType)
	{
		case QID_ITEM_AWARD_LOAD:
			ItemAwardManager::Instance().Load(msg);
			delete qi;
			return true;

		case QID_GUILD_RANKING:
			CGuildManager::Instance().ResultRanking(msg->Get()->pSQLResult);
			break;

			// MYSHOP_PRICE_LIST
		case QID_ITEMPRICE_LOAD_FOR_UPDATE:
			RESULT_PRICELIST_LOAD_FOR_UPDATE(msg);
			break;
			// END_OF_MYSHOP_PRICE_LIST
	}

	if (!peer)
	{	
		//sys_err("CClientManager::AnalyzeQueryResult: peer not exist anymore. (ident: %d)", qi->dwIdent);
		delete qi;
		return true;
	}

	switch (qi->iType)
	{
		case QID_PLAYER:
		case QID_ITEM:
		case QID_QUEST:
		case QID_AFFECT:
		case QID_ACTIVITY:
			RESULT_COMPOSITE_PLAYER(peer, msg, qi->iType);
			break;

		case QID_LOGIN:
			RESULT_LOGIN(peer, msg);
			break;

		case QID_SAFEBOX_LOAD:
			sys_log(0, "QUERY_RESULT: HEADER_GD_SAFEBOX_LOAD");
			RESULT_SAFEBOX_LOAD(peer, msg);
			break;

		case QID_SAFEBOX_CHANGE_SIZE:
			sys_log(0, "QUERY_RESULT: HEADER_GD_SAFEBOX_CHANGE_SIZE");
			RESULT_SAFEBOX_CHANGE_SIZE(peer, msg);
			break;

		case QID_SAFEBOX_CHANGE_PASSWORD:
			sys_log(0, "QUERY_RESULT: HEADER_GD_SAFEBOX_CHANGE_PASSWORD %p", msg);
			RESULT_SAFEBOX_CHANGE_PASSWORD(peer, msg);
			break;

		case QID_SAFEBOX_CHANGE_PASSWORD_SECOND:
			sys_log(0, "QUERY_RESULT: HEADER_GD_SAFEBOX_CHANGE_PASSWORD %p", msg);
			RESULT_SAFEBOX_CHANGE_PASSWORD_SECOND(peer, msg);
			break;

		case QID_SAFEBOX_SAVE:
		case QID_ITEM_SAVE:
		case QID_ITEM_DESTROY:
		case QID_QUEST_SAVE:
		case QID_PLAYER_SAVE:
		case QID_ITEM_AWARD_TAKEN:
			break;

			// PLAYER_INDEX_CREATE_BUG_FIX	
		case QID_PLAYER_INDEX_CREATE:
			RESULT_PLAYER_INDEX_CREATE(peer, msg);
			break;
			// END_PLAYER_INDEX_CREATE_BUG_FIX	

		case QID_PLAYER_DELETE:
			__RESULT_PLAYER_DELETE(peer, msg);
			break;

		case QID_LOGIN_BY_KEY:
			RESULT_LOGIN_BY_KEY(peer, msg);
			break;

			// MYSHOP_PRICE_LIST
		case QID_ITEMPRICE_LOAD:
			RESULT_PRICELIST_LOAD(peer, msg);
			break;
			// END_OF_MYSHOP_PRICE_LIST

		default:
			sys_log(0, "CClientManager::AnalyzeQueryResult unknown query result type: %d, str: %s", qi->iType, msg->stQuery.c_str());
			break;
	}

	delete qi;
	return true;
}

void UsageLog()
{   
	FILE* fp = nullptr;

	time_t      ct;
	char        *time_s;
	struct tm   lt;

	int32_t         avg = g_dwUsageAvg / 3600; // 60 초 * 60 분

	fp = fopen("usage.txt", "a+");

	if (!fp)
		return;

	ct = time(nullptr);
	lt = *localtime(&ct);
	time_s = asctime(&lt);

	time_s[strlen(time_s) - 1] = '\0';

	fprintf(fp, "| %4d %-15.15s | %5d | %5u |", lt.tm_year + 1900, time_s + 4, avg, g_dwUsageMax);

	fprintf(fp, "\n");
	fclose(fp);

	g_dwUsageMax = g_dwUsageAvg = 0;
}

#define ENABLE_ITEMAWARD_REFRESH
int32_t CClientManager::Process()
{
	int32_t pulses = thecore_idle();

	if (!pulses)
		return 0;

	while (pulses--)
	{
		++thecore_heart->pulse;

		/*
		//30분마다 변경
		if (((thecore_pulse() % (60 * 30 * 10)) == 0))
		{
			g_iPlayerCacheFlushSeconds = MAX(60, rand() % 180);
			g_iItemCacheFlushSeconds = MAX(60, rand() % 180);
			sys_log(0, "[SAVE_TIME]Change saving time item %d player %d", g_iPlayerCacheFlushSeconds, g_iItemCacheFlushSeconds);
		}
		*/

		if (!(thecore_heart->pulse % thecore_heart->passes_per_sec))
		{

			CDBManager::Instance().ResetCounter();

			uint32_t dwCount = CClientManager::Instance().GetUserCount();

			g_dwUsageAvg += dwCount;
			g_dwUsageMax = MAX(g_dwUsageMax, dwCount);

			if (!(thecore_heart->pulse % (thecore_heart->passes_per_sec * 3600)))
				UsageLog();

			m_iCacheFlushCount = 0;


			//플레이어 플러쉬
			UpdatePlayerCache();
			//아이템 플러쉬
			UpdateItemCache();
			//로그아웃시 처리- 캐쉬셋 플러쉬
			UpdateLogoutPlayer();

			// MYSHOP_PRICE_LIST
			UpdateItemPriceListCache();
			// END_OF_MYSHOP_PRICE_LIST

			UpdateActivityCache();

			CGuildManager::Instance().Update();
			CPrivManager::Instance().Update();
			marriage::CManager::Instance().Update();
		}
#ifdef ENABLE_ITEMAWARD_REFRESH
		if (!(thecore_heart->pulse % (thecore_heart->passes_per_sec * 5)))
		{
			ItemAwardManager::Instance().RequestLoad();
		}
#endif

		if (!(thecore_heart->pulse % (thecore_heart->passes_per_sec * 60)))	// 60초에 한번
		{
			// 유니크 아이템을 위한 시간을 보낸다.
			CClientManager::Instance().SendTime();
			
			std::string st;
			CClientManager::Instance().GetPeerP2PHostNames(st);
			sys_log(0, "Current Peer host names...\n%s", st.c_str());
		}
	}

	int32_t num_events = fdwatch(m_fdWatcher, nullptr);
	int32_t idx;
	CPeer * peer;

	for (idx = 0; idx < num_events; ++idx) // 인풋
	{
		peer = (CPeer *) fdwatch_get_client_data(m_fdWatcher, idx);

		if (!peer)
		{
			if (fdwatch_check_event(m_fdWatcher, m_fdAccept, idx) == FDW_READ)
			{
				AddPeer(m_fdAccept);
				fdwatch_clear_event(m_fdWatcher, m_fdAccept, idx);
			}
			else
			{
				sys_err("FDWATCH: peer null in event: ident %d", fdwatch_get_ident(m_fdWatcher, idx));
			}

			continue;
		}

		switch (fdwatch_check_event(m_fdWatcher, peer->GetFd(), idx))
		{
			case FDW_READ:
				if (peer->Recv() < 0)
				{
					sys_err("Recv failed");
					RemovePeer(peer);
				}
				else
				{
					if (peer == m_pkAuthPeer)
						if (g_log)
							sys_log(0, "AUTH_PEER_READ: size %d", peer->GetRecvLength());

					ProcessPackets(peer);
				}
				break;

			case FDW_WRITE:
				if (peer == m_pkAuthPeer)
					if (g_log && peer->GetSendLength() > 0)
						sys_log(0, "AUTH_PEER_WRITE: size %d", peer->GetSendLength());

				if (peer->Send() < 0)
				{
					sys_err("Send failed");
					RemovePeer(peer);
				}

				break;

			case FDW_EOF:
				RemovePeer(peer);
				break;

			default:
				sys_err("fdwatch_check_fd returned unknown result! peer %d, length %d, idx %d", peer->GetFd(), peer->GetSendLength(), idx);
				RemovePeer(peer);
				break;
		}
	}

#ifdef _WIN32
	if (_kbhit())
	{
		int32_t c = _getch();
		switch (c) 
		{
			case VK_ESCAPE: // Esc
				return 0; // shutdown
				break;
			default:
				break;
		}
	}
#endif

	return 1;
}

uint32_t CClientManager::GetUserCount()
{
	// 단순히 로그인 카운트를 센다.. --;
	return m_map_kLogonAccount.size();
}

void CClientManager::SendAllGuildSkillRechargePacket()
{
	ForwardPacket(HEADER_DG_GUILD_SKILL_RECHARGE, nullptr, 0);
}

void CClientManager::SendTime()
{
	time_t now = GetCurrentTime();
	ForwardPacket(HEADER_DG_TIME, &now, sizeof(time_t));
}

void CClientManager::ForwardPacket(uint8_t header, const void* data, int32_t size, uint8_t bChannel, CPeer* except)
{
	for (auto peer : m_peerList)
	{
		if (peer == except)
			continue;

		if (!peer->GetChannel())
			continue;

		if (bChannel && peer->GetChannel() != bChannel)
			continue;

		peer->EncodeHeader(header, 0, size);

		if (size > 0 && data)
			peer->Encode(data, size);
	}
}

void CClientManager::SendNotice(const char * c_pszFormat, ...)
{
	char szBuf[255+1];
	va_list args;

	va_start(args, c_pszFormat);
	int32_t len = vsnprintf(szBuf, sizeof(szBuf), c_pszFormat, args);
	va_end(args);
	szBuf[len] = '\0';

	ForwardPacket(HEADER_DG_NOTICE, szBuf, len + 1);
}

time_t CClientManager::GetCurrentTime()
{
	return time(nullptr);
}

// ITEM_UNIQUE_ID
bool CClientManager::InitializeItemIDRange(uint32_t min, uint32_t max)
{
	sys_log(0, "ItemRange From File %lu ~ %lu ", min, max);
	
	if (CItemIDRangeManager::Instance().BuildRange(min, max, m_itemRange) == false)
	{
		sys_err("Can not build ITEM_ID_RANGE");
		return false;
	}

	sys_log(0, " Init Success Start %lu End %lu Now %lu\n", m_itemRange.dwMin, m_itemRange.dwMax, m_itemRange.dwUsableItemIDMin);
	return true;
}

uint32_t CClientManager::GainItemID()
{
	return m_itemRange.dwUsableItemIDMin++;
}

uint32_t CClientManager::GetItemID()
{
	return m_itemRange.dwUsableItemIDMin;
}
// ITEM_UNIQUE_ID_END

//ADMIN_MANAGER
bool CClientManager::__GetAdminInfo(std::vector<tAdminInfo> & rAdminVec)
{
	char szQuery[512];
	snprintf(szQuery, sizeof(szQuery),
		"SELECT mID,mAccount,mName,mAuthority FROM gmlist");

	std::unique_ptr<SQLMsg> pMsg(CDBManager::Instance().DirectQuery(szQuery, SQL_COMMON));

	if (pMsg->Get()->uiNumRows == 0)
	{
		sys_err("__GetAdminInfo() ==> DirectQuery failed(%s)", szQuery);
		return false;
	}

	MYSQL_ROW row;
	rAdminVec.reserve(pMsg->Get()->uiNumRows);

	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		int32_t idx = 0;
		tAdminInfo Info;

		str_to_number(Info.m_ID, row[idx++]);
		trim_and_lower(row[idx++], Info.m_szAccount, sizeof(Info.m_szAccount));
		strlcpy(Info.m_szName, row[idx++], sizeof(Info.m_szName));
		std::string stAuth = row[idx++];

		if (!stAuth.compare("IMPLEMENTOR"))
			Info.m_Authority = GM_IMPLEMENTOR;
		else if (!stAuth.compare("GOD"))
			Info.m_Authority = GM_GOD; 
		else if (!stAuth.compare("HIGH_WIZARD"))
			Info.m_Authority = GM_HIGH_WIZARD;
		else if (!stAuth.compare("LOW_WIZARD")) 
			Info.m_Authority = GM_LOW_WIZARD;
		else if (!stAuth.compare("WIZARD"))
			Info.m_Authority = GM_WIZARD;
		else 
			continue;

		rAdminVec.push_back(Info);

		sys_log(0, "GM: PID %u Login %s Character %s Authority %d[%s]",
			Info.m_ID, Info.m_szAccount, Info.m_szName, Info.m_Authority, stAuth.c_str());
	}

	return true;
}

bool CClientManager::__GetAdminConfig(uint32_t pAdminConfig[GM_MAX_NUM])
{
	memset(pAdminConfig, 1, sizeof(uint32_t) * GM_MAX_NUM);

	char szQuery[512];
	snprintf(szQuery, sizeof(szQuery),
		"SELECT authority, can_drop_player_item, can_drop_gm_item, can_exchange_player_item_to_gm, can_exchange_player_item_to_player, "
		"can_exchange_gm_item_to_gm, can_exchange_gm_item_to_player, can_exchange_to_gm, can_exchange_to_player, "
		"can_buy_private_item, can_create_private_shop, can_use_safebox, can_create_player, can_delete_player FROM gmconfig");

	std::unique_ptr<SQLMsg> pMsg(CDBManager::Instance().DirectQuery(szQuery, SQL_COMMON));

	if (pMsg->Get()->uiNumRows == 0)
	{
		sys_err("__GetAdminConfig() ==> DirectQuery failed(%s)", szQuery);
		return false;
	}

	MYSQL_ROW row;
	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		int col = 0;

		uint8_t bAuthority = GM_MAX_NUM;

		std::string stAuthority = row[col++];
		if (!stAuthority.compare("IMPLEMENTOR"))
			bAuthority = GM_IMPLEMENTOR;
		else if (!stAuthority.compare("GOD"))
			bAuthority = GM_GOD;
		else if (!stAuthority.compare("HIGH_WIZARD"))
			bAuthority = GM_HIGH_WIZARD;
		else if (!stAuthority.compare("LOW_WIZARD"))
			bAuthority = GM_LOW_WIZARD;
		else if (!stAuthority.compare("WIZARD"))
			bAuthority = GM_WIZARD;
		else if (!stAuthority.compare("PLAYER"))
			bAuthority = GM_PLAYER;
		else
		{
			sys_err("unkown authority %s", stAuthority.c_str());
			continue;
		}

		uint8_t bCanDropPlayerItem = 0;
		str_to_number(bCanDropPlayerItem, row[col++]);

		uint8_t bCanDropGMItem = 0;
		str_to_number(bCanDropGMItem, row[col++]);

		uint8_t bCanExchangePlayerItemToGM = 0;
		str_to_number(bCanExchangePlayerItemToGM, row[col++]);

		uint8_t bCanExchangePlayerItemToPlayer = 0;
		str_to_number(bCanExchangePlayerItemToPlayer, row[col++]);

		uint8_t bCanExchangeGMItemToGM = 0;
		str_to_number(bCanExchangeGMItemToGM, row[col++]);

		uint8_t bCanExchangeGMItemToPlayer = 0;
		str_to_number(bCanExchangeGMItemToPlayer, row[col++]);

		uint8_t bCanExchangeToGM = 0;
		str_to_number(bCanExchangeToGM, row[col++]);

		uint8_t bCanExchangeToPlayer = 0;
		str_to_number(bCanExchangeToPlayer, row[col++]);

		uint8_t bCanBuyPrivateItem = 0;
		str_to_number(bCanBuyPrivateItem, row[col++]);

		uint8_t bCanCreatePrivateShop = 0;
		str_to_number(bCanCreatePrivateShop, row[col++]);

		uint8_t bCanUseSafebox = 0;
		str_to_number(bCanUseSafebox, row[col++]);

		uint8_t bCanCreatePlayer = 0;
		str_to_number(bCanCreatePlayer, row[col++]);

		uint8_t bCanDeletePlayer = 0;
		str_to_number(bCanDeletePlayer, row[col++]);

		uint32_t dwFlag = 0;
		if (bCanDropPlayerItem)
			dwFlag |= GM_ALLOW_DROP_PLAYER_ITEM;
		if (bCanDropGMItem)
			dwFlag |= GM_ALLOW_DROP_GM_ITEM;
		if (bCanExchangePlayerItemToGM)
			dwFlag |= GM_ALLOW_EXCHANGE_PLAYER_ITEM_TO_GM;
		if (bCanExchangePlayerItemToPlayer)
			dwFlag |= GM_ALLOW_EXCHANGE_PLAYER_ITEM_TO_PLAYER;
		if (bCanExchangeGMItemToGM)
			dwFlag |= GM_ALLOW_EXCHANGE_GM_ITEM_TO_GM;
		if (bCanExchangeGMItemToPlayer)
			dwFlag |= GM_ALLOW_EXCHANGE_GM_ITEM_TO_PLAYER;
		if (bCanExchangeToGM)
			dwFlag |= GM_ALLOW_EXCHANGE_TO_GM;
		if (bCanExchangeToPlayer)
			dwFlag |= GM_ALLOW_EXCHANGE_TO_PLAYER;
		if (bCanBuyPrivateItem)
			dwFlag |= GM_ALLOW_BUY_PRIVATE_ITEM;
		if (bCanCreatePrivateShop)
			dwFlag |= GM_ALLOW_CREATE_PRIVATE_SHOP;
		if (bCanUseSafebox)
			dwFlag |= GM_ALLOW_USE_SAFEBOX;
		if (bCanCreatePlayer)
			dwFlag |= GM_ALLOW_CREATE_PLAYER;
		if (bCanDeletePlayer)
			dwFlag |= GM_ALLOW_DELETE_PLAYER;

		pAdminConfig[bAuthority] = dwFlag;
	}

	return true;
}
//END_ADMIN_MANAGER

void CClientManager::ReloadAdmin()
{
	std::vector <tAdminInfo> vAdmin;
	__GetAdminInfo(vAdmin);
	uint32_t adwAdminConfig[GM_MAX_NUM];
	__GetAdminConfig(adwAdminConfig);

	uint32_t dwPacketSize = sizeof(uint16_t) + sizeof(tAdminInfo) * vAdmin.size() + sizeof(uint32_t) * GM_MAX_NUM;

	for (auto it = m_peerList.begin(); it != m_peerList.end(); ++it)
	{
		CPeer * peer = *it;

		if (!peer->GetChannel())
			continue;

		peer->EncodeHeader(HEADER_DG_RELOAD_ADMIN, 0, dwPacketSize);

		peer->EncodeWORD(static_cast<uint16_t>(vAdmin.size()));

		for (size_t n = 0; n < vAdmin.size(); ++n)
			peer->Encode(&vAdmin[n], sizeof(tAdminInfo));

		peer->Encode(&adwAdminConfig[0], sizeof(adwAdminConfig));
	}

	sys_log(0, "ReloadAdmin End");
}

//BREAK_MARRIAGE
void CClientManager::BreakMarriage(CPeer *, const char * data)
{
	uint32_t pid1, pid2;

	pid1 = *(int32_t *) data;
	data += sizeof(int32_t);

	pid2 = *(int32_t *) data;
	data += sizeof(int32_t);

	sys_log(0, "Breaking off a marriage engagement! pid %d and pid %d", pid1, pid2);
	marriage::CManager::Instance().Remove(pid1, pid2);
}
//END_BREAK_MARIIAGE

void CClientManager::UpdateItemCacheSet(uint32_t pid)
{
	auto it = m_map_pkItemCacheSetPtr.find(pid);

	if (it == m_map_pkItemCacheSetPtr.end())
	{
		if (g_test_server)
			sys_log(0, "UPDATE_ITEMCACHESET : UpdateItemCacheSet ==> No ItemCacheSet pid(%d)", pid);
		return;
	}

	auto pSet = it->second;
	auto it_set = pSet->begin();

	while (it_set != pSet->end())
	{
		CItemCache * c = *it_set++;
		c->Flush();
	}

	if (g_log)
		sys_log(0, "UPDATE_ITEMCACHESET : UpdateItemCachsSet pid(%d)", pid);
}


void CClientManager::SendSpareItemIDRange(CPeer* peer)
{
	peer->SendSpareItemIDRange();
}

//
// Login Key만 맵에서 지운다.
// 
void CClientManager::DeleteLoginKey(TPacketDC *data)
{
	char login[LOGIN_MAX_LEN+1] = {0};
	trim_and_lower(data->login, login, sizeof(login));

	CLoginData *pkLD = GetLoginDataByLogin(login);

	if (pkLD)
	{
		auto it = m_map_pkLoginData.find(pkLD->GetKey());

		if (it != m_map_pkLoginData.end())
			m_map_pkLoginData.erase(it);
	}
}

// delete gift notify icon
void CClientManager::DeleteAwardId(TPacketDeleteAwardID *data)
{
	//sys_log(0,"data from game server arrived %d",data->dwID);
	std::map<uint32_t, TItemAward *>::iterator it;
	it = ItemAwardManager::Instance().GetMapAward().find(data->dwID);
	if ( it != ItemAwardManager::Instance().GetMapAward().end() )
	{
		std::set<TItemAward *> & kSet = ItemAwardManager::Instance().GetMapkSetAwardByLogin()[it->second->szLogin];
		if(kSet.erase(it->second))
			sys_log(0,"erase ItemAward id: %d from cache", data->dwID);
		ItemAwardManager::Instance().GetMapAward().erase(data->dwID);
	}
	else
	{
		sys_log(0,"DELETE_AWARDID : could not find the id: %d", data->dwID);
	}

}

void CClientManager::UpdateChannelStatus(TChannelStatus* pData)
{
	auto it = m_mChannelStatus.find(pData->nPort);
	if (it != m_mChannelStatus.end()) {
		it->second = pData->bStatus;
	}
	else {
		m_mChannelStatus.insert(TChannelStatusMap::value_type(pData->nPort, pData->bStatus));
	}
}

void CClientManager::RequestChannelStatus(CPeer* peer, uint32_t dwHandle)
{
	const int32_t nSize = static_cast<int32_t>(m_mChannelStatus.size());

	peer->EncodeHeader(HEADER_DG_RESPOND_CHANNELSTATUS, dwHandle, sizeof(TChannelStatus)*nSize+sizeof(int32_t));
	peer->Encode(&nSize, sizeof(int32_t));
	for (auto & m_mChannelStatu : m_mChannelStatus) {
		peer->Encode(&m_mChannelStatu.first, sizeof(int16_t));
		peer->Encode(&m_mChannelStatu.second, sizeof(uint8_t));
	}
}

void CClientManager::ResetLastPlayerID(const TPacketNeedLoginLogInfo* data)
{
	CLoginData* pkLD = GetLoginDataByAID( data->dwPlayerID );

	if (nullptr != pkLD)
	{
		pkLD->SetLastPlayerID( 0 );
	}
}

void CClientManager::ChargeCash(const TRequestChargeCash* packet)
{
	char szQuery[512];

	if (ERequestCharge_Cash == packet->eChargeType)
		sprintf(szQuery, "update account set `cash` = `cash` + %d where id = %d limit 1", packet->dwAmount, packet->dwAID);
	else if(ERequestCharge_Mileage == packet->eChargeType)
		sprintf(szQuery, "update account set `mileage` = `mileage` + %d where id = %d limit 1", packet->dwAmount, packet->dwAID);
	else
	{
		sys_err ("Invalid request charge type (type : %d, amount : %d, aid : %d)", packet->eChargeType, packet->dwAmount, packet->dwAID);
		return;
	}

	sys_err ("Request Charge (type : %d, amount : %d, aid : %d)", packet->eChargeType, packet->dwAmount, packet->dwAID);

	CDBManager::Instance().AsyncQuery(szQuery, SQL_ACCOUNT);
}
