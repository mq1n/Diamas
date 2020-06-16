
#include "stdafx.h"
#include "ClientManager.h"
#include "Main.h"
#include "QID.h"
#include "Cache.h"
#include "../../common/service.h"

extern std::string g_stLocale;
extern bool CreatePlayerTableFromRes(MYSQL_RES * res, TPlayerTable * pkTab);
extern bool g_test_server;
extern bool g_log;

bool CClientManager::InsertLogonAccount(const char * c_pszLogin, uint32_t dwHandle, const char * c_pszIP)
{
	char szLogin[LOGIN_MAX_LEN + 1];
	trim_and_lower(c_pszLogin, szLogin, sizeof(szLogin));

	auto it = m_map_kLogonAccount.find(szLogin);

	if (m_map_kLogonAccount.end() != it)
		return false;

	CLoginData * pkLD = GetLoginDataByLogin(c_pszLogin);

	if (!pkLD)
		return false;

	pkLD->SetConnectedPeerHandle(dwHandle);
	pkLD->SetIP(c_pszIP);

	m_map_kLogonAccount.insert(TLogonAccountMap::value_type(szLogin, pkLD));
	return true;
}

bool CClientManager::DeleteLogonAccount(const char * c_pszLogin, uint32_t dwHandle)
{
	char szLogin[LOGIN_MAX_LEN + 1];
	trim_and_lower(c_pszLogin, szLogin, sizeof(szLogin));

	auto it = m_map_kLogonAccount.find(szLogin);

	if (it == m_map_kLogonAccount.end())
		return false;

	CLoginData * pkLD = it->second;

	if (pkLD->GetConnectedPeerHandle() != dwHandle)
	{
		sys_err("%s tried to logout in other peer handle %u, current handle %u", szLogin, dwHandle, pkLD->GetConnectedPeerHandle());
		return false;
	}

	if (pkLD->IsPlay())
	{
		pkLD->SetPlay(false);
	}

	if (pkLD->IsDeleted())
	{
		delete pkLD;
	}

	m_map_kLogonAccount.erase(it);
	return true;
}

bool CClientManager::FindLogonAccount(const char * c_pszLogin)
{
	char szLogin[LOGIN_MAX_LEN + 1];
	trim_and_lower(c_pszLogin, szLogin, sizeof(szLogin));

	if (m_map_kLogonAccount.end() == m_map_kLogonAccount.find(szLogin))
		return false;

	return true;
}

void CClientManager::QUERY_LOGIN_BY_KEY(CPeer * pkPeer, uint32_t dwHandle, TPacketGDLoginByKey * p)
{
	CLoginData * pkLoginData = GetLoginData(p->dwLoginKey);
	char szLogin[LOGIN_MAX_LEN + 1];
	trim_and_lower(p->szLogin, szLogin, sizeof(szLogin));

	if (!pkLoginData)
	{
		sys_log(0, "LOGIN_BY_KEY key not exist %s %u", szLogin, p->dwLoginKey);
		pkPeer->EncodeReturn(HEADER_DG_LOGIN_NOT_EXIST, dwHandle);
		return;
	}

	TAccountTable & r = pkLoginData->GetAccountRef();
	if (FindLogonAccount(r.login))
	{
		sys_log(0, "LOGIN_BY_KEY already login %s %u", r.login, p->dwLoginKey);
		TPacketDGLoginAlready ptog;
		strlcpy(ptog.szLogin, szLogin, sizeof(ptog.szLogin));
		pkPeer->EncodeHeader(HEADER_DG_LOGIN_ALREADY, dwHandle, sizeof(TPacketDGLoginAlready));
		pkPeer->Encode(&ptog, sizeof(TPacketDGLoginAlready));
		return;
	}

	if (strcasecmp(r.login, szLogin))
	{
		sys_log(0, "LOGIN_BY_KEY login differ %s %u input %s", r.login, p->dwLoginKey, szLogin);
		pkPeer->EncodeReturn(HEADER_DG_LOGIN_NOT_EXIST, dwHandle);
		return;
	}

	if (memcmp(pkLoginData->GetClientKey(), p->adwClientKey, sizeof(uint32_t) * 4))
	{
		const uint32_t * pdwClientKey = pkLoginData->GetClientKey();

		sys_log(0, "LOGIN_BY_KEY client key differ %s %u %u %u %u, %u %u %u %u",
				r.login,
				p->adwClientKey[0], p->adwClientKey[1], p->adwClientKey[2], p->adwClientKey[3],
				pdwClientKey[0], pdwClientKey[1], pdwClientKey[2], pdwClientKey[3]);

		pkPeer->EncodeReturn(HEADER_DG_LOGIN_NOT_EXIST, dwHandle);
		return;
	}

	auto pkTab = new TAccountTable;
	memset(pkTab, 0, sizeof(TAccountTable));

	pkTab->id = r.id;
	trim_and_lower(r.login, pkTab->login, sizeof(pkTab->login));
	strlcpy(pkTab->passwd, r.passwd, sizeof(pkTab->passwd));
	strlcpy(pkTab->social_id, r.social_id, sizeof(pkTab->social_id));
	strlcpy(pkTab->status, "OK", sizeof(pkTab->status));

	auto info = new ClientHandleInfo(dwHandle);
	info->pAccountTable = pkTab;
	strlcpy(info->ip, p->szIP, sizeof(info->ip));

	sys_log(0, "LOGIN_BY_KEY success %s %u %s", r.login, p->dwLoginKey, info->ip);
	char szQuery[ASQL_QUERY_MAX_LEN];
	snprintf(szQuery, sizeof(szQuery), "SELECT pid1, pid2, pid3, pid4, pid5, empire FROM player_index WHERE id=%u", r.id);
	CDBManager::instance().ReturnQuery(szQuery, QID_LOGIN_BY_KEY, pkPeer->GetHandle(), info);
}

void CClientManager::RESULT_LOGIN_BY_KEY(CPeer * peer, SQLMsg * msg)
{
	CQueryInfo * qi = static_cast<CQueryInfo *>(msg->pvUserData);
	ClientHandleInfo * info = static_cast<ClientHandleInfo *>(qi->pvData);

	if (msg->uiSQLErrno != 0)
	{
		peer->EncodeReturn(HEADER_DG_LOGIN_NOT_EXIST, info->dwHandle);
		delete info;
		return;
	}

	if (msg->Get()->uiNumRows == 0)
	{
		uint32_t account_id = info->pAccountTable->id;
		char szQuery[ASQL_QUERY_MAX_LEN];
		snprintf(szQuery, sizeof(szQuery), "SELECT pid1, pid2, pid3, pid4, pid5, empire FROM player_index WHERE id=%u", account_id);
		std::unique_ptr<SQLMsg> pMsg(CDBManager::instance().DirectQuery(szQuery, SQL_PLAYER));
		
		sys_log(0, "RESULT_LOGIN_BY_KEY FAIL player_index's nullptr : ID:%d", account_id);

		if (pMsg->Get()->uiNumRows == 0)
		{
			sys_log(0, "RESULT_LOGIN_BY_KEY FAIL player_index's nullptr : ID:%d", account_id);

			//snprintf(szQuery, sizeof(szQuery), "INSERT IGNORE INTO player_index (id) VALUES(%u)", info->pAccountTable->id);
			snprintf(szQuery, sizeof(szQuery), "INSERT INTO player_index (id) VALUES(%u)", info->pAccountTable->id);
			CDBManager::instance().ReturnQuery(szQuery, QID_PLAYER_INDEX_CREATE, peer->GetHandle(), info);
		}
		return;
	}

	MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);

	int32_t col = 0;

	for (; col < PLAYER_PER_ACCOUNT; ++col)
		str_to_number(info->pAccountTable->players[col].dwID, row[col]);

	str_to_number(info->pAccountTable->bEmpire, row[col++]);
	if (info->pAccountTable->bEmpire > 3)
		info->pAccountTable->bEmpire = 3;
		
	info->account_index = 1;

	char szQuery[ASQL_QUERY_MAX_LEN];
	snprintf(szQuery, sizeof(szQuery),
		"SELECT id, name, job, level, playtime, st, ht, dx, iq, part_main, part_hair, "
#ifdef ENABLE_ACCE_SYSTEM
        "part_acce, "
#endif
        "x, y, skill_group, change_name FROM player WHERE account_id=%u",
		info->pAccountTable->id);

	CDBManager::instance().ReturnQuery(szQuery, QID_LOGIN, peer->GetHandle(), info);
}

// PLAYER_INDEX_CREATE_BUG_FIX
void CClientManager::RESULT_PLAYER_INDEX_CREATE(CPeer * pkPeer, SQLMsg * msg)
{
	CQueryInfo * qi = (CQueryInfo *) msg->pvUserData;
	ClientHandleInfo * info = (ClientHandleInfo *) qi->pvData;

	char szQuery[ASQL_QUERY_MAX_LEN];
	snprintf(szQuery, sizeof(szQuery), "SELECT pid1, pid2, pid3, pid4, pid5, empire FROM player_index WHERE id=%u", info->pAccountTable->id);
	CDBManager::instance().ReturnQuery(szQuery, QID_LOGIN_BY_KEY, pkPeer->GetHandle(), info);
}
// END_PLAYER_INDEX_CREATE_BUG_FIX

TAccountTable * CreateAccountTableFromRes(MYSQL_RES * res)
{
	char input_pwd[PASSWD_MAX_LEN + 1];
	MYSQL_ROW row = nullptr;
	uint32_t col;

	row = mysql_fetch_row(res);
	col = 0;

	auto pkTab = new TAccountTable;
	memset(pkTab, 0, sizeof(TAccountTable));

	// 첫번째 컬럼 것만 참고 한다 (JOIN QUERY를 위한 것 임)
	strlcpy(input_pwd, row[col++], sizeof(input_pwd));
	str_to_number(pkTab->id, row[col++]);
	strlcpy(pkTab->login, row[col++], sizeof(pkTab->login));
	strlcpy(pkTab->passwd, row[col++], sizeof(pkTab->passwd));
	strlcpy(pkTab->social_id, row[col++], sizeof(pkTab->social_id));
	str_to_number(pkTab->bEmpire, row[col++]);

	for (int32_t j = 0; j < PLAYER_PER_ACCOUNT; ++j)
		str_to_number(pkTab->players[j].dwID, row[col++]);

	strlcpy(pkTab->status, row[col++], sizeof(pkTab->status));

	if (strcmp(pkTab->passwd, input_pwd))
	{
		delete pkTab;
		return nullptr;
	}

	return pkTab;
}

void CreateAccountPlayerDataFromRes(MYSQL_RES * pRes, TAccountTable * pkTab)
{
	if (!pRes)
		return;

	for (uint32_t i = 0; i < mysql_num_rows(pRes); ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(pRes);
		int32_t col = 0;

		uint32_t player_id = 0;
		!row[col++] ? 0 : str_to_number(player_id, row[col - 1]);

		if (!player_id)
			continue;

		int32_t j;

		for (j = 0; j < PLAYER_PER_ACCOUNT; ++j)
		{
			if (pkTab->players[j].dwID == player_id)
			{
				CPlayerTableCache * pc = CClientManager::instance().GetPlayerCache(player_id);
				TPlayerTable * pt = pc ? pc->Get(false) : nullptr;

				if (pt)
				{
					strlcpy(pkTab->players[j].szName, pt->name, sizeof(pkTab->players[j].szName));

					pkTab->players[j].byJob			= pt->job;
					pkTab->players[j].byLevel			= pt->level;
					pkTab->players[j].dwPlayMinutes		= pt->playtime;
					pkTab->players[j].byST			= pt->st;
					pkTab->players[j].byHT			= pt->ht;
					pkTab->players[j].byDX			= pt->dx;
					pkTab->players[j].byIQ			= pt->iq;
					pkTab->players[j].wMainPart			= pt->parts[PART_MAIN];
					pkTab->players[j].wHairPart			= pt->parts[PART_HAIR];
#ifdef ENABLE_ACCE_SYSTEM
					pkTab->players[j].dwAccePart			= pt->parts[PART_ACCE];
#endif
					pkTab->players[j].x				= pt->x;
					pkTab->players[j].y				= pt->y;
					pkTab->players[j].skill_group		= pt->skill_group;
					pkTab->players[j].bChangeName		= 0;
				}
				else
				{
					if (!row[col++])
						*pkTab->players[j].szName = '\0';
					else
						strlcpy(pkTab->players[j].szName, row[col - 1], sizeof(pkTab->players[j].szName));

					pkTab->players[j].byJob			= 0;
					pkTab->players[j].byLevel		= 0;
					pkTab->players[j].dwPlayMinutes	= 0;
					pkTab->players[j].byST			= 0;
					pkTab->players[j].byHT			= 0;
					pkTab->players[j].byDX			= 0;
					pkTab->players[j].byIQ			= 0;
					pkTab->players[j].wMainPart		= 0;
					pkTab->players[j].wHairPart		= 0;
#ifdef ENABLE_ACCE_SYSTEM
					pkTab->players[j].dwAccePart		= 0;
#endif
					pkTab->players[j].x				= 0;
					pkTab->players[j].y				= 0;
					pkTab->players[j].skill_group	= 0;
					pkTab->players[j].bChangeName	= 0;

					str_to_number(pkTab->players[j].byJob, row[col++]);
					str_to_number(pkTab->players[j].byLevel, row[col++]);
					str_to_number(pkTab->players[j].dwPlayMinutes, row[col++]);
					str_to_number(pkTab->players[j].byST, row[col++]);
					str_to_number(pkTab->players[j].byHT, row[col++]);
					str_to_number(pkTab->players[j].byDX, row[col++]);
					str_to_number(pkTab->players[j].byIQ, row[col++]);
					str_to_number(pkTab->players[j].wMainPart, row[col++]);
					str_to_number(pkTab->players[j].wHairPart, row[col++]);
#ifdef ENABLE_ACCE_SYSTEM
					str_to_number(pkTab->players[j].dwAccePart, row[col++]);
#endif
					str_to_number(pkTab->players[j].x, row[col++]);
					str_to_number(pkTab->players[j].y, row[col++]);
					str_to_number(pkTab->players[j].skill_group, row[col++]);
					str_to_number(pkTab->players[j].bChangeName, row[col++]);
				}

				sys_log(0, "%s %u %u hair %u",
						pkTab->players[j].szName, pkTab->players[j].x, pkTab->players[j].y, pkTab->players[j].wHairPart);
				break;
			}
		}
		/*
		   if (j == PLAYER_PER_ACCOUNT)
		   sys_err("cannot find player_id on this account (login: %s id %u account %u %u %u)", 
		   pkTab->login, player_id,
		   pkTab->players[0].dwID,
		   pkTab->players[1].dwID,
		   pkTab->players[2].dwID);
		   */
	}
}

void CClientManager::RESULT_LOGIN(CPeer * peer, SQLMsg * msg)
{
	CQueryInfo * qi = (CQueryInfo *) msg->pvUserData;
	ClientHandleInfo * info = (ClientHandleInfo *) qi->pvData;

	if (info->account_index == 0)
	{
		// 계정이 없네?
		if (msg->Get()->uiNumRows == 0)
		{
			sys_log(0, "RESULT_LOGIN: no account");
			peer->EncodeHeader(HEADER_DG_LOGIN_NOT_EXIST, info->dwHandle, 0);
			delete info;
			return;
		}

		info->pAccountTable = CreateAccountTableFromRes(msg->Get()->pSQLResult);

		if (!info->pAccountTable)
		{
			sys_log(0, "RESULT_LOGIN: no account : WRONG_PASSWD");
			peer->EncodeReturn(HEADER_DG_LOGIN_WRONG_PASSWD, info->dwHandle);
			delete info;
		}
		else
		{
			++info->account_index;

			char queryStr[512];
			snprintf(queryStr, sizeof(queryStr),
				"SELECT id, name, job, level, playtime, st, ht, dx, iq, part_main, part_hair, "
#ifdef ENABLE_ACCE_SYSTEM
				"part_acce, "
#endif
                "x, y, skill_group, change_name FROM player WHERE account_id=%u",
				info->pAccountTable->id);

			CDBManager::instance().ReturnQuery(queryStr, QID_LOGIN, peer->GetHandle(), info);
		}
		return;
	}
	else
	{
		if (!info->pAccountTable) // 이럴리는 없겠지만;;
		{
			peer->EncodeReturn(HEADER_DG_LOGIN_WRONG_PASSWD, info->dwHandle);
			delete info;
			return;
		}

		// 다른 컨넥션이 이미 로그인 해버렸다면.. 이미 접속했다고 보내야 한다.
		if (!InsertLogonAccount(info->pAccountTable->login, peer->GetHandle(), info->ip))
		{
			sys_log(0, "RESULT_LOGIN: already logon %s", info->pAccountTable->login);

			TPacketDGLoginAlready p;
			strlcpy(p.szLogin, info->pAccountTable->login, sizeof(p.szLogin));

			peer->EncodeHeader(HEADER_DG_LOGIN_ALREADY, info->dwHandle, sizeof(TPacketDGLoginAlready));
			peer->Encode(&p, sizeof(p));
		}
		else
		{
			sys_log(0, "RESULT_LOGIN: login success %s rows: %u", info->pAccountTable->login, msg->Get()->uiNumRows);

			if (msg->Get()->uiNumRows > 0)
				CreateAccountPlayerDataFromRes(msg->Get()->pSQLResult, info->pAccountTable);

			//PREVENT_COPY_ITEM
			CLoginData * p = GetLoginDataByLogin(info->pAccountTable->login);
			memcpy(&p->GetAccountRef(), info->pAccountTable, sizeof(TAccountTable));

			//END_PREVENT_COPY_ITEM
			peer->EncodeHeader(HEADER_DG_LOGIN_SUCCESS, info->dwHandle, sizeof(TAccountTable));
			peer->Encode(info->pAccountTable, sizeof(TAccountTable));

		}

		delete info->pAccountTable;
		info->pAccountTable = nullptr;
		delete info;
	}
}

void CClientManager::QUERY_LOGOUT(CPeer * peer, uint32_t, const char * data)
{
	TLogoutPacket* packet = (TLogoutPacket*)data;

	if (!*packet->login)
		return;

	CLoginData * pLoginData = GetLoginDataByLogin(packet->login);

	if (pLoginData == nullptr)
		return;

	int32_t pid[PLAYER_PER_ACCOUNT];

	for (int32_t n = 0; n < PLAYER_PER_ACCOUNT; ++n)
	{
		if (pLoginData->GetAccountRef().players[n].dwID == 0)
		{
			if (g_test_server)
				sys_log(0, "LOGOUT %s %d", packet->login, pLoginData->GetAccountRef().players[n].dwID);
			continue;
		}
		
		pid[n] = pLoginData->GetAccountRef().players[n].dwID;

		if (g_log)
			sys_log(0, "LOGOUT InsertLogoutPlayer %s %d", packet->login, pid[n]);

		InsertLogoutPlayer(pid[n]);
	}
	
	if (DeleteLogonAccount(packet->login, peer->GetHandle()))
	{
		if (g_log)
			sys_log(0, "LOGOUT %s ", packet->login);
	}
}

void CClientManager::QUERY_CHANGE_NAME(CPeer * peer, uint32_t dwHandle, TPacketGDChangeName * p)
{
	char queryStr[ASQL_QUERY_MAX_LEN];

	snprintf(queryStr, sizeof(queryStr),
		"SELECT COUNT(*) as count FROM player WHERE name='%s' AND id <> %u", p->name, p->pid);

	std::unique_ptr<SQLMsg> pMsg(CDBManager::instance().DirectQuery(queryStr, SQL_PLAYER));

	if (pMsg->Get()->uiNumRows)
	{
		if (!pMsg->Get()->pSQLResult)
		{
			peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
			return;
		}

		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

		if (*row[0] != '0')
		{
			peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_ALREADY, dwHandle, 0);
			return;
		}
	}   
	else
	{
		peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
		return;
	}

	snprintf(queryStr, sizeof(queryStr),
			"UPDATE player SET name='%s',change_name=0 WHERE id=%u", p->name, p->pid);

	std::unique_ptr<SQLMsg> pMsg0(CDBManager::instance().DirectQuery(queryStr, SQL_PLAYER));

	TPacketDGChangeName pdg;
	peer->EncodeHeader(HEADER_DG_CHANGE_NAME, dwHandle, sizeof(TPacketDGChangeName));
	pdg.pid = p->pid;
	strlcpy(pdg.name, p->name, sizeof(pdg.name));
	peer->Encode(&pdg, sizeof(TPacketDGChangeName));
}

