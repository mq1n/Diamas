#include "stdafx.h"
#include "ClientManager.h"
#include "Main.h"
#include "QID.h"
#include "ItemAwardManager.h"
#include "Cache.h"

extern std::string g_stLocale;
extern bool g_test_server;
extern bool g_log;

//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!! IMPORTANT !!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// Check all SELECT syntax on item table before change this function!!!
//
bool CreateItemTableFromRes(MYSQL_RES * res, std::vector<TPlayerItem> * pVec, uint32_t dwPID)
{
	if (!res)
	{
		pVec->clear();
		return true;
	}

	auto rows = mysql_num_rows(res);
	if (rows <= 0)
	{
		pVec->clear();
		return true;
	}

	pVec->resize(rows);

	for (uint32_t i = 0; i < rows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(res);
		TPlayerItem & item = pVec->at(i);

		int32_t cur = 0;

		// Check all SELECT syntax on item table before change this function!!!
		// Check all SELECT syntax on item table before change this function!!!
		// Check all SELECT syntax on item table before change this function!!!
		str_to_number(item.id, row[cur++]);
		str_to_number(item.window, row[cur++]);
		str_to_number(item.pos, row[cur++]);
		str_to_number(item.count, row[cur++]);
		str_to_number(item.vnum, row[cur++]);
		str_to_number(item.is_gm_owner, row[cur++]);
		str_to_number(item.alSockets[0], row[cur++]);
		str_to_number(item.alSockets[1], row[cur++]);
		str_to_number(item.alSockets[2], row[cur++]);

		for (auto & j : item.aAttr)
		{
			str_to_number(j.bType, row[cur++]);
			str_to_number(j.sValue, row[cur++]);
		}

		item.owner		= dwPID;
	}

	return true;
}

size_t CreatePlayerSaveQuery(char * pszQuery, size_t querySize, TPlayerTable * pkTab)
{
	size_t queryLen;

	queryLen = snprintf(pszQuery, querySize,
			"UPDATE player SET "
			"job = %d, "
			"voice = %d, "
			"dir = %d, "
			"x = %d, "
			"y = %d, "
			"z = %d, "
			"map_index = %d, "
			"exit_x = %d, "
			"exit_y = %d, "
			"exit_map_index = %d, "
			"hp = %d, "
			"mp = %d, "
			"stamina = %d, "
			"random_hp = %d, "
			"random_sp = %d, "
			"playtime = %d, "
			"level = %d, "
			"level_step = %d, "
			"st = %d, "
			"ht = %d, "
			"dx = %d, "
			"iq = %d, "
			"gold = %d, "
			"exp = %u, "
			"stat_point = %d, "
			"skill_point = %d, "
			"sub_skill_point = %d, "
			"stat_reset_count = %d, "
			"ip = '%s', "
			"part_main = %d, "
			"part_hair = %d, "
#ifdef ENABLE_ACCE_SYSTEM
			"part_acce = %d, "
#endif
			"last_play = NOW(), "
			"skill_group = %d, "
			"alignment = %d, "
			"horse_level = %d, "
			"horse_riding = %d, "
			"horse_hp = %d, "
			"horse_hp_droptime = %u, "
			"horse_stamina = %d, "
			"horse_skill_point = %d, "
			"gm_invisible = %d, "
			,
		pkTab->job,
		pkTab->voice,
		pkTab->dir,
		pkTab->x,
		pkTab->y,
		pkTab->z,
		pkTab->lMapIndex,
		pkTab->lExitX,
		pkTab->lExitY,
		pkTab->lExitMapIndex,
		pkTab->hp,
		pkTab->sp,
		pkTab->stamina,
		pkTab->sRandomHP,
		pkTab->sRandomSP,
		pkTab->playtime,
		pkTab->level,
		pkTab->level_step,
		pkTab->st,
		pkTab->ht,
		pkTab->dx,
		pkTab->iq,
		pkTab->gold,
		pkTab->exp,
		pkTab->stat_point,
		pkTab->skill_point,
		pkTab->sub_skill_point,
		pkTab->stat_reset_count,
		pkTab->ip,
		pkTab->parts[PART_MAIN],
		pkTab->parts[PART_HAIR],
#ifdef ENABLE_ACCE_SYSTEM
		pkTab->parts[PART_ACCE],
#endif
		pkTab->skill_group,
		pkTab->lAlignment,
		pkTab->horse.bLevel,
		pkTab->horse.bRiding,
		pkTab->horse.sHealth,
		pkTab->horse.dwHorseHealthDropTime,
		pkTab->horse.sStamina,
		pkTab->horse_skill_point,
		pkTab->is_gm_invisible);

	// Binary 로 바꾸기 위한 임시 공간
	static char text[8192 + 1];

	CDBManager::Instance().EscapeString(text, pkTab->horse.szName, strlen(pkTab->horse.szName));
	queryLen += snprintf(pszQuery + queryLen, querySize - queryLen, "horse_name = '%s', ", text);

	CDBManager::Instance().EscapeString(text, pkTab->skills, sizeof(pkTab->skills));
	queryLen += snprintf(pszQuery + queryLen, querySize - queryLen, "skill_level = '%s', ", text);

	CDBManager::Instance().EscapeString(text, pkTab->quickslot, sizeof(pkTab->quickslot));
	queryLen += snprintf(pszQuery + queryLen, querySize - queryLen, "quickslot = '%s' ", text);

	queryLen += snprintf(pszQuery + queryLen, querySize - queryLen, " WHERE id=%d", pkTab->id);
	return queryLen;
}

CPlayerTableCache * CClientManager::GetPlayerCache(uint32_t id)
{
	auto it = m_map_playerCache.find(id);

	if (it == m_map_playerCache.end())
		return nullptr;

	TPlayerTable* pTable = it->second->Get(false);
	pTable->logoff_interval = GetCurrentTime() - it->second->GetLastUpdateTime();
	return it->second;
}

void CClientManager::PutPlayerCache(TPlayerTable * pNew)
{
	CPlayerTableCache * c;

	c = GetPlayerCache(pNew->id);

	if (!c)
	{
		c = new CPlayerTableCache;
		m_map_playerCache.emplace(pNew->id, c);
	}

	c->Put(pNew);
}


CActivityCache* CClientManager::GetActivityCache(uint32_t id)
{
	auto it = m_map_activityCache.find(id);

	if (it == m_map_activityCache.end())
		return nullptr;

	return it->second;
}

void CClientManager::PutActivityCache(TActivityTable * pNew)
{
	CActivityCache* c = GetActivityCache(pNew->pid);

	if (!c)
	{
		c = new CActivityCache;
		m_map_activityCache.emplace(pNew->pid, c);
	}

	c->Put(pNew);
}

void CClientManager::UpdateActivityCache()
{
	auto it = m_map_activityCache.begin();

	while (it != m_map_activityCache.end())
	{
		CActivityCache* pCache = it->second;

		if (pCache->CheckFlushTimeout()) {
			pCache->Flush();
		}

		if (pCache->CheckTimeout())
		{
			it = m_map_activityCache.erase(it);
			delete pCache;
		}
		else
		{
			++it;
		}
	}
}

/*
 * PLAYER LOAD
 */
void CClientManager::QUERY_PLAYER_LOAD(CPeer * peer, uint32_t dwHandle, TPlayerLoadPacket * packet)
{
	//
	// 한 계정에 속한 모든 캐릭터들 캐쉬처리
	//
	CLoginData * pkLD = GetLoginDataByAID(packet->account_id);

	if (pkLD)
	{
		for (auto & player : pkLD->GetAccountRef().players)
			if (player.dwID != 0)
				DeleteLogoutPlayer(player.dwID);
	}

	//----------------------------------------------------------------
	// 1. 유저정보가 DBCache 에 존재 : DBCache에서 
	// 2. 유저정보가 DBCache 에 없음 : DB에서 
	// ---------------------------------------------------------------
	
	//----------------------------------
	// 1. 유저정보가 DBCache 에 존재 : DBCache에서 
	//----------------------------------

	if (!pkLD || pkLD->IsPlay())
	{
		sys_err("ERROR: LoginData %p IsPlay %d", pkLD, pkLD ? 1 : 0);
		peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_FAILED, dwHandle, 0);
		return;
	}
	pkLD->SetPlay(true);
	//----------------------------------
	// Let's try to load from cache first
	//----------------------------------
	if (CPlayerTableCache * playerCache = GetPlayerCache(packet->player_id))
	{
		TPlayerTable * pTab = playerCache->Get();
		memcpy(pTab->aiPremiumTimes, pkLD->GetPremiumPtr(), sizeof(pTab->aiPremiumTimes));

		peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_SUCCESS, dwHandle, sizeof(TPlayerTable));
		peer->Encode(pTab, sizeof(TPlayerTable));

		if (packet->player_id != pkLD->GetLastPlayerID())
		{
			TPacketNeedLoginLogInfo logInfo;
			logInfo.dwPlayerID = packet->player_id;

			pkLD->SetLastPlayerID( packet->player_id );

			peer->EncodeHeader( HEADER_DG_NEED_LOGIN_LOG, dwHandle, sizeof(TPacketNeedLoginLogInfo) );
			peer->Encode( &logInfo, sizeof(TPacketNeedLoginLogInfo) );
		}

		char szQuery[1024] = { 0, };
		sys_log(0, "[PLAYER_LOAD] ID %s pid %d gold %d ", pTab->name, pTab->id, pTab->gold);

		/////////////////////////////////////////////
		// If there's item cache, send it over, and only load quest, affect & pet.
		/////////////////////////////////////////////
	
		TItemCacheSet * pSet = GetItemCacheSet(pTab->id);

		if (pSet)
		{
			static std::vector<TPlayerItem> s_items;
			s_items.resize(pSet->size());

			uint32_t dwCount = 0;
			auto it = pSet->begin();

			while (it != pSet->end())
			{
				CItemCache * itemCache = *it++;
				TPlayerItem * p = itemCache->Get();

				if (p->vnum) // vnum이 없으면 삭제된 아이템이다.
					memcpy(&s_items[dwCount++], p, sizeof(TPlayerItem));
			}

			if (g_test_server)
				sys_log(0, "ITEM_CACHE: HIT! %s count: %u", pTab->name, dwCount);

			peer->EncodeHeader(HEADER_DG_ITEM_LOAD, dwHandle, sizeof(uint32_t) + sizeof(TPlayerItem) * dwCount);
			peer->EncodeDWORD(dwCount);

			if (dwCount)
				peer->Encode(&s_items[0], sizeof(TPlayerItem) * dwCount);

			// Quest
			snprintf(szQuery, sizeof(szQuery),
					"SELECT dwPID,szName,szState,lValue FROM quest WHERE dwPID=%d AND lValue<>0", pTab->id);

			CDBManager::Instance().ReturnQuery(szQuery, QID_QUEST, peer->GetHandle(), new ClientHandleInfo(dwHandle,0,packet->account_id));

			// Affect
			snprintf(szQuery, sizeof(szQuery),
					"SELECT dwPID,bType,bApplyOn,lApplyValue,dwFlag,lDuration,lSPCost FROM affect WHERE dwPID=%d",
					pTab->id);
			CDBManager::Instance().ReturnQuery(szQuery, QID_AFFECT, peer->GetHandle(), new ClientHandleInfo(dwHandle, pTab->id));
		}
		/////////////////////////////////////////////
		// 2) 아이템이 DBCache 에 없음 : DB 에서 가져옴 
		/////////////////////////////////////////////
		else
		{
			snprintf(szQuery, sizeof(szQuery), 
					"SELECT id,window+0,pos,count,vnum,is_gm_owner,socket0,socket1,socket2,attrtype0,attrvalue0,attrtype1,attrvalue1,attrtype2,attrvalue2,attrtype3,attrvalue3,attrtype4,attrvalue4,attrtype5,attrvalue5,attrtype6,attrvalue6 "
					"FROM item WHERE owner_id=%d AND (window in ('INVENTORY','EQUIPMENT','DRAGON_SOUL_INVENTORY','BELT_INVENTORY'))",
					pTab->id);

			CDBManager::Instance().ReturnQuery(szQuery,
					QID_ITEM,
					peer->GetHandle(),
					new ClientHandleInfo(dwHandle, pTab->id));
			snprintf(szQuery, sizeof(szQuery), 
					"SELECT dwPID, szName, szState, lValue FROM quest WHERE dwPID=%d",
					pTab->id);

			CDBManager::Instance().ReturnQuery(szQuery,
					QID_QUEST,
					peer->GetHandle(),
					new ClientHandleInfo(dwHandle, pTab->id));
			snprintf(szQuery, sizeof(szQuery), 
					"SELECT dwPID, bType, bApplyOn, lApplyValue, dwFlag, lDuration, lSPCost FROM affect WHERE dwPID=%d",
					pTab->id);

			CDBManager::Instance().ReturnQuery(szQuery,
					QID_AFFECT,
					peer->GetHandle(),
					new ClientHandleInfo(dwHandle, pTab->id));
		}
		//ljw
		//return;
	}
	//----------------------------------
	// 2. 유저정보가 DBCache 에 없음 : DB에서 
	//----------------------------------
	else
	{
		sys_log(0, "[PLAYER_LOAD] Load from PlayerDB pid[%d]", packet->player_id);

		char queryStr[ASQL_QUERY_MAX_LEN];

		//--------------------------------------------------------------
		// 캐릭터 정보 얻어오기 : 무조건 DB에서 
		//--------------------------------------------------------------
		snprintf(queryStr, sizeof(queryStr),
				"SELECT "
				"id,name,job,voice,dir,x,y,z,map_index,exit_x,exit_y,exit_map_index,hp,mp,stamina,"
				"random_hp,random_sp,playtime,gold,level,level_step,st,ht,dx,iq,exp,"
				"stat_point,skill_point,sub_skill_point,stat_reset_count,part_base,part_hair,"
#ifdef ENABLE_ACCE_SYSTEM
				"part_acce, "
#endif
				"skill_level,quickslot,skill_group,alignment,"
			"horse_level,horse_name,horse_riding,horse_hp,horse_hp_droptime,horse_stamina,"
			"UNIX_TIMESTAMP(NOW())-UNIX_TIMESTAMP(last_play),horse_skill_point,gm_invisible FROM player WHERE id=%u",
			packet->player_id);

		auto pkInfo = new ClientHandleInfo(dwHandle, packet->player_id);
		pkInfo->account_id = packet->account_id;
		CDBManager::Instance().ReturnQuery(queryStr, QID_PLAYER, peer->GetHandle(), pkInfo);

		//--------------------------------------------------------------
		// 아이템 가져오기 
		//--------------------------------------------------------------
		snprintf(queryStr, sizeof(queryStr),
				"SELECT id,window+0,pos,count,vnum,is_gm_owner,socket0,socket1,socket2,attrtype0,attrvalue0,attrtype1,attrvalue1,attrtype2,attrvalue2,attrtype3,attrvalue3,attrtype4,attrvalue4,attrtype5,attrvalue5,attrtype6,attrvalue6 "
				"FROM item WHERE owner_id=%d AND (window in ('INVENTORY','EQUIPMENT','DRAGON_SOUL_INVENTORY','BELT_INVENTORY'))",
				packet->player_id);
		CDBManager::Instance().ReturnQuery(queryStr, QID_ITEM, peer->GetHandle(), new ClientHandleInfo(dwHandle, packet->player_id));

		//--------------------------------------------------------------
		// QUEST 가져오기
		//--------------------------------------------------------------
		snprintf(queryStr, sizeof(queryStr),
				"SELECT dwPID,szName,szState,lValue FROM quest WHERE dwPID=%d",
				packet->player_id);
		CDBManager::Instance().ReturnQuery(queryStr, QID_QUEST, peer->GetHandle(), new ClientHandleInfo(dwHandle, packet->player_id,packet->account_id));
		//독일 선물 기능에서 item_award테이블에서 login 정보를 얻기위해 account id도 넘겨준다
		//--------------------------------------------------------------
		// AFFECT 가져오기
		//--------------------------------------------------------------
		snprintf(queryStr, sizeof(queryStr),
				"SELECT dwPID,bType,bApplyOn,lApplyValue,dwFlag,lDuration,lSPCost FROM affect WHERE dwPID=%d",
				packet->player_id);
		CDBManager::Instance().ReturnQuery(queryStr, QID_AFFECT, peer->GetHandle(), new ClientHandleInfo(dwHandle, packet->player_id));
	}

	// Activity (Try to load from cache first)
	if (CActivityCache * ac = GetActivityCache(packet->player_id))
	{
		TActivityTable* activityTable = ac->Get();
		LoadActivity(peer, packet->player_id, dwHandle, activityTable);
	}
	else
	{
		char queryStr[ASQL_QUERY_MAX_LEN];
		snprintf(queryStr, sizeof(queryStr),
			"SELECT today_pvp, today_pve, today_other, today_gk, total, last_update FROM activity WHERE pid =%d",
			packet->player_id);
		CDBManager::Instance().ReturnQuery(queryStr, QID_ACTIVITY, peer->GetHandle(), new ClientHandleInfo(dwHandle, packet->player_id));
	}
}
void CClientManager::ItemAward(CPeer *,char* login)
{
	char login_t[LOGIN_MAX_LEN + 1]{};
	strlcpy(login_t,login,LOGIN_MAX_LEN + 1);	
	auto pSet = ItemAwardManager::Instance().GetByLogin(login_t);
	if(pSet == nullptr)
		return;
	auto it = pSet->begin();	//taken_time이 NULL인것들 읽어옴	
	while(it != pSet->end() )
	{				
		TItemAward * pItemAward = *(it++);		
		char* whyStr = pItemAward->szWhy;	//why 콜룸 읽기
		char cmdStr[100] = "";	//why콜룸에서 읽은 값을 임시 문자열에 복사해둠
		strcpy(cmdStr,whyStr);	//명령어 얻는 과정에서 토큰쓰면 원본도 토큰화 되기 때문
		char command[20] = "";
		GetCommand(cmdStr, command);			// command 얻기
		if( !(strcmp(command,"GIFT") ))	// command 가 GIFT이면
		{
			TPacketItemAwardInfromer giftData;
			strcpy(giftData.login, pItemAward->szLogin);	//로그인 아이디 복사
			strcpy(giftData.command, command);					//명령어 복사
			giftData.vnum = pItemAward->dwVnum;				//아이템 vnum도 복사
			ForwardPacket(HEADER_DG_ITEMAWARD_INFORMER,&giftData,sizeof(TPacketItemAwardInfromer));
		}
	}
}
char* CClientManager::GetCommand(char* str, char* command)
{
	char* tok;

	if( str[0] == '[' )
	{
		tok = strtok(str,"]");			
		strcat(command,&tok[1]);		
	}

	return command;
}

bool CreatePlayerTableFromRes(MYSQL_RES * res, TPlayerTable * pkTab)
{
	if (mysql_num_rows(res) == 0)	// 데이터 없음
		return false;

	memset(pkTab, 0, sizeof(TPlayerTable));

	MYSQL_ROW row = mysql_fetch_row(res);

	int32_t	col = 0;

	// "id,name,job,voice,dir,x,y,z,map_index,exit_x,exit_y,exit_map_index,hp,mp,stamina,random_hp,random_sp,playtime,"
	// "gold,level,level_step,st,ht,dx,iq,exp,"
	// "stat_point,skill_point,sub_skill_point,stat_reset_count,part_base,part_hair,"
	// "skill_level,quickslot,skill_group,alignment,horse_level,horse_riding,horse_hp,horse_stamina FROM player%s WHERE id=%d",
	str_to_number(pkTab->id, row[col++]);
	strlcpy(pkTab->name, row[col++], sizeof(pkTab->name));
	str_to_number(pkTab->job, row[col++]);
	str_to_number(pkTab->voice, row[col++]);
	str_to_number(pkTab->dir, row[col++]);
	str_to_number(pkTab->x, row[col++]);
	str_to_number(pkTab->y, row[col++]);
	str_to_number(pkTab->z, row[col++]);
	str_to_number(pkTab->lMapIndex, row[col++]);
	str_to_number(pkTab->lExitX, row[col++]);
	str_to_number(pkTab->lExitY, row[col++]);
	str_to_number(pkTab->lExitMapIndex,  row[col++]);
	str_to_number(pkTab->hp, row[col++]);
	str_to_number(pkTab->sp, row[col++]);
	str_to_number(pkTab->stamina, row[col++]);
	str_to_number(pkTab->sRandomHP, row[col++]);
	str_to_number(pkTab->sRandomSP, row[col++]);
	str_to_number(pkTab->playtime, row[col++]);
	str_to_number(pkTab->gold, row[col++]);
	str_to_number(pkTab->level, row[col++]);
	str_to_number(pkTab->level_step, row[col++]);
	str_to_number(pkTab->st, row[col++]);
	str_to_number(pkTab->ht, row[col++]);
	str_to_number(pkTab->dx, row[col++]);
	str_to_number(pkTab->iq, row[col++]);
	str_to_number(pkTab->exp, row[col++]);
	str_to_number(pkTab->stat_point, row[col++]);
	str_to_number(pkTab->skill_point, row[col++]);
	str_to_number(pkTab->sub_skill_point, row[col++]);
	str_to_number(pkTab->stat_reset_count, row[col++]);
	str_to_number(pkTab->part_base, row[col++]);
	str_to_number(pkTab->parts[PART_HAIR], row[col++]);
#ifdef ENABLE_ACCE_SYSTEM
	str_to_number(pkTab->parts[PART_ACCE], row[col++]);
#endif

	if (row[col])
		memcpy(pkTab->skills, row[col], sizeof(pkTab->skills));
	else
		memset(&pkTab->skills, 0, sizeof(pkTab->skills));

	col++;

	if (row[col])
		memcpy(pkTab->quickslot, row[col], sizeof(pkTab->quickslot));
	else
		memset(pkTab->quickslot, 0, sizeof(pkTab->quickslot));

	col++;

	str_to_number(pkTab->skill_group, row[col++]);
	str_to_number(pkTab->lAlignment, row[col++]);

	str_to_number(pkTab->horse.bLevel, row[col++]);
	strlcpy(pkTab->horse.szName, row[col++], sizeof(pkTab->horse.szName));
	str_to_bool(pkTab->horse.bRiding, row[col++]);
	str_to_number(pkTab->horse.sHealth, row[col++]);
	str_to_number(pkTab->horse.dwHorseHealthDropTime, row[col++]);
	str_to_number(pkTab->horse.sStamina, row[col++]);
	str_to_number(pkTab->logoff_interval, row[col++]);
	str_to_number(pkTab->horse_skill_point, row[col++]);
	str_to_number(pkTab->is_gm_invisible, row[col++]);

	// reset sub_skill_point
	{
		pkTab->skills[123].bLevel = 0; // SKILL_CREATE

		if (pkTab->level > 9)
		{
			int32_t max_point = pkTab->level - 9;

			int32_t skill_point = 
				MIN(20, pkTab->skills[121].bLevel) +	// SKILL_LEADERSHIP			통솔력
				MIN(20, pkTab->skills[124].bLevel) +	// SKILL_MINING				채광
				MIN(10, pkTab->skills[131].bLevel) +	// SKILL_HORSE_SUMMON		말소환
				MIN(20, pkTab->skills[141].bLevel) +	// SKILL_ADD_HP				HP보강
				MIN(20, pkTab->skills[142].bLevel);		// SKILL_RESIST_PENETRATE	관통저항

			pkTab->sub_skill_point = max_point - skill_point;
		}
		else
			pkTab->sub_skill_point = 0;
	}

	return true;
}

void CClientManager::RESULT_COMPOSITE_PLAYER(CPeer * peer, SQLMsg * pMsg, uint32_t dwQID)
{
	CQueryInfo * qi = (CQueryInfo *) pMsg->pvUserData;
	std::unique_ptr<ClientHandleInfo> info((ClientHandleInfo *) qi->pvData);
	
	MYSQL_RES * pSQLResult = pMsg->Get()->pSQLResult;
	if (!pSQLResult)
	{
		sys_err("null MYSQL_RES QID %u", dwQID);
		return;
	}

	switch (dwQID)
	{
		case QID_PLAYER:
			sys_log(0, "QID_PLAYER %u %u", info->dwHandle, info->player_id);
			RESULT_PLAYER_LOAD(peer, pSQLResult, info.get());

			break;

		case QID_ITEM:
			sys_log(0, "QID_ITEM %u", info->dwHandle);
			RESULT_ITEM_LOAD(peer, pSQLResult, info->dwHandle, info->player_id);
			break;

		case QID_QUEST:
			{
				sys_log(0, "QID_QUEST %u", info->dwHandle);
				RESULT_QUEST_LOAD(peer, pSQLResult, info->dwHandle, info->player_id);
				//aid얻기
				ClientHandleInfo*  temp1 = info.get();
				if (temp1 == nullptr)
					break;
				
				CLoginData* pLoginData1 = GetLoginDataByAID(temp1->account_id);	//				
				//독일 선물 기능
				if( pLoginData1 == nullptr )
					break;
				if (strncmp(pLoginData1->GetAccountRef().login, "", sizeof(pLoginData1->GetAccountRef().login)))
					break;

				sys_log(0,"info of pLoginData1 before call ItemAwardfunction %d",pLoginData1);
				ItemAward(peer,pLoginData1->GetAccountRef().login);
			}
			break;

		case QID_AFFECT:
			sys_log(0, "QID_AFFECT %u", info->dwHandle);
			RESULT_AFFECT_LOAD(peer, pSQLResult, info->dwHandle, info->player_id);
			break;

		case QID_ACTIVITY:
			sys_log(0, "QID_ACTIVITY %u", info->dwHandle);
			RESULT_ACTIVITY_LOAD(peer, pSQLResult, info->dwHandle, info->player_id);
			break;
	}
	
}

void CClientManager::RESULT_PLAYER_LOAD(CPeer * peer, MYSQL_RES * pRes, ClientHandleInfo * pkInfo)
{
	TPlayerTable tab;

	if (!CreatePlayerTableFromRes(pRes, &tab))
	{
		peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_FAILED, pkInfo->dwHandle, 0); 
		return;
	}

	CLoginData * pkLD = GetLoginDataByAID(pkInfo->account_id);
	
	if (!pkLD || !pkLD->IsPlay())
	{
		sys_err("ERROR: LoginData %p IsPlay %d", pkLD, pkLD ? 0 : 1);
		peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_FAILED, pkInfo->dwHandle, 0);
		return;
	}

	memcpy(tab.aiPremiumTimes, pkLD->GetPremiumPtr(), sizeof(tab.aiPremiumTimes));

	peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_SUCCESS, pkInfo->dwHandle, sizeof(TPlayerTable));
	peer->Encode(&tab, sizeof(TPlayerTable));

	if (tab.id != pkLD->GetLastPlayerID())
	{
		TPacketNeedLoginLogInfo logInfo;
		logInfo.dwPlayerID = tab.id;

		pkLD->SetLastPlayerID( tab.id );

		peer->EncodeHeader( HEADER_DG_NEED_LOGIN_LOG, pkInfo->dwHandle, sizeof(TPacketNeedLoginLogInfo) );
		peer->Encode( &logInfo, sizeof(TPacketNeedLoginLogInfo) );
	}
}

void CClientManager::RESULT_ITEM_LOAD(CPeer * peer, MYSQL_RES * pRes, uint32_t dwHandle, uint32_t dwPID)
{
	static std::vector<TPlayerItem> s_items;
	//DB에서 아이템 정보를 읽어온다.
	CreateItemTableFromRes(pRes, &s_items, dwPID);
	uint32_t dwCount = s_items.size();

	peer->EncodeHeader(HEADER_DG_ITEM_LOAD, dwHandle, sizeof(uint32_t) + sizeof(TPlayerItem) * dwCount);
	peer->EncodeDWORD(dwCount);

	//CacheSet을 만든다  
	CreateItemCacheSet(dwPID);

	// ITEM_LOAD_LOG_ATTACH_PID
	sys_log(0, "ITEM_LOAD: count %u pid %u", dwCount, dwPID);
	// END_OF_ITEM_LOAD_LOG_ATTACH_PID

	if (dwCount)
	{
		peer->Encode(&s_items[0], sizeof(TPlayerItem) * dwCount);

		for (uint32_t i = 0; i < dwCount; ++i)
			PutItemCache(&s_items[i], true); // 로드한 것은 따로 저장할 필요 없으므로, 인자 bSkipQuery에 true를 넣는다.
	}
}

void CClientManager::RESULT_AFFECT_LOAD(CPeer * peer, MYSQL_RES * pRes, uint32_t dwHandle, uint32_t dwRealPID)
{
	uint32_t dwNumRows = static_cast<uint32_t>(mysql_num_rows(pRes));

	if (dwNumRows == 0) // 데이터 없음
	{
		static uint32_t dwPID;
		static uint32_t dwCount = 0; //1;
		static TPacketAffectElement paeTable = {0};

		dwPID = dwRealPID;
		sys_log(0, "AFFECT_LOAD: count %u PID %u RealPID %u", dwCount, dwPID, dwRealPID);

		peer->EncodeHeader(HEADER_DG_AFFECT_LOAD, dwHandle, sizeof(uint32_t) + sizeof(uint32_t) + sizeof(TPacketAffectElement) * dwCount);
		peer->Encode(&dwPID, sizeof(uint32_t));
		peer->Encode(&dwCount, sizeof(uint32_t));
		peer->Encode(&paeTable, sizeof(TPacketAffectElement) * dwCount);
		return;
	}

	static std::vector<TPacketAffectElement> s_elements;
	s_elements.resize(dwNumRows);

	uint32_t dwPID = 0;

	MYSQL_ROW row;

	for (uint32_t i = 0; i < dwNumRows; ++i)
	{
		TPacketAffectElement & r = s_elements[i];
		row = mysql_fetch_row(pRes);

		if (dwPID == 0)
			str_to_number(dwPID, row[0]);

		str_to_number(r.dwType, row[1]);
		str_to_number(r.bApplyOn, row[2]);
		str_to_number(r.lApplyValue, row[3]);
		str_to_number(r.dwFlag, row[4]);
		str_to_number(r.lDuration, row[5]);
		str_to_number(r.lSPCost, row[6]);
	}

	sys_log(0, "AFFECT_LOAD: count %d PID %u", s_elements.size(), dwPID);

	uint32_t dwCount = s_elements.size();

	peer->EncodeHeader(HEADER_DG_AFFECT_LOAD, dwHandle, sizeof(uint32_t) + sizeof(uint32_t) + sizeof(TPacketAffectElement) * dwCount);
	peer->Encode(&dwPID, sizeof(uint32_t));
	peer->Encode(&dwCount, sizeof(uint32_t));
	peer->Encode(&s_elements[0], sizeof(TPacketAffectElement) * dwCount);
}

void CClientManager::RESULT_QUEST_LOAD(CPeer * peer, MYSQL_RES * pRes, uint32_t dwHandle, uint32_t)
{
	uint32_t iNumRows = static_cast<uint32_t>(mysql_num_rows(pRes));

	if (iNumRows == 0)
	{
		uint32_t dwCount = 0; 
		peer->EncodeHeader(HEADER_DG_QUEST_LOAD, dwHandle, sizeof(uint32_t));
		peer->Encode(&dwCount, sizeof(uint32_t));
		return;
	}

	static std::vector<TQuestTable> s_table;
	s_table.resize(iNumRows);

	MYSQL_ROW row;

	for (uint32_t i = 0; i < iNumRows; ++i)
	{
		TQuestTable & r = s_table[i];

		row = mysql_fetch_row(pRes);

		str_to_number(r.dwPID, row[0]);
		strlcpy(r.szName, row[1], sizeof(r.szName));
		strlcpy(r.szState, row[2], sizeof(r.szState));
		str_to_number(r.lValue, row[3]);
	}

	sys_log(0, "QUEST_LOAD: count %d PID %u", s_table.size(), s_table[0].dwPID);

	uint32_t dwCount = s_table.size();

	peer->EncodeHeader(HEADER_DG_QUEST_LOAD, dwHandle, sizeof(uint32_t) + sizeof(TQuestTable) * dwCount);
	peer->Encode(&dwCount, sizeof(uint32_t));
	peer->Encode(&s_table[0], sizeof(TQuestTable) * dwCount);
}

void CClientManager::RESULT_ACTIVITY_LOAD(CPeer* peer, MYSQL_RES* pRes, uint32_t dwHandle, uint32_t dwPID)
{
	uint32_t iNumRows = (uint32_t)mysql_num_rows(pRes);

	if (iNumRows == 0)
		return;

	if (iNumRows > 1)
		sys_err("More than 1 row when loading activity (%u). Loading first and ignoring the rest.", dwPID);

	//today_pvp, today_pve, today_other, today_gk, total, last_update
	MYSQL_ROW row = mysql_fetch_row(pRes);

	TActivityTable ai;
	ai.pid = dwPID;
	str_to_number(ai.today.pvp, row[0]);
	str_to_number(ai.today.pve, row[1]);
	str_to_number(ai.today.other, row[2]);
	str_to_number(ai.today.gk, row[3]);
	str_to_number(ai.total, row[4]);
	str_to_number(ai.lastUpdate, row[5]);

	LoadActivity(peer, dwPID, dwHandle, &ai);
}

void CClientManager::LoadActivity(CPeer * peer, uint32_t pid, uint32_t dwHandle, TActivityTable * activity)
{
	sys_log(0, "ACTIVITY_LOAD: %d for PID %u", activity->total, pid);

	peer->EncodeHeader(HEADER_DG_ACTIVITY_LOAD, dwHandle, sizeof(uint32_t) + sizeof(TActivityTable));
	peer->Encode(&pid, sizeof(uint32_t));
	peer->Encode(activity, sizeof(TActivityTable));
}

/*
 * PLAYER SAVE
 */
void CClientManager::QUERY_PLAYER_SAVE(CPeer *, uint32_t, TPlayerTable * pkTab)
{
	if (g_test_server)
		sys_log(0, "PLAYER_SAVE: %s", pkTab->name);

	PutPlayerCache(pkTab);
}

void CClientManager::QUERY_SAVE_ACTIVITY(CPeer*, uint32_t, TActivityTable* pkActivity)
{
	if (g_test_server)
		sys_log(0, "ACTIVITY_SAVE: %u", pkActivity->pid);

	PutActivityCache(pkActivity);
}

typedef std::map<uint32_t, time_t> time_by_id_map_t;
static time_by_id_map_t s_createTimeByAccountID;

/*
 * PLAYER CREATE
 */
void CClientManager::__QUERY_PLAYER_CREATE(CPeer *peer, uint32_t dwHandle, TPlayerCreatePacket* packet)
{
	char	queryStr[ASQL_QUERY_MAX_LEN];
	int32_t		queryLen;
	uint32_t	player_id;

	// 한 계정에 X초 내로 캐릭터 생성을 할 수 없다.
	auto it = s_createTimeByAccountID.find(packet->account_id);

	if (it != s_createTimeByAccountID.end())
	{
		time_t curtime = time(nullptr);

		if (curtime - it->second < 30)
		{
			peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
			return;
		}
	}

	queryLen = snprintf(queryStr, sizeof(queryStr), 
			"SELECT pid%u FROM player_index WHERE id=%d", packet->account_index + 1, packet->account_id);

	std::unique_ptr<SQLMsg> pMsg0(CDBManager::Instance().DirectQuery(queryStr));

	if (pMsg0->Get()->uiNumRows != 0)
	{
		if (!pMsg0->Get()->pSQLResult)
		{
			peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
			return;
		}

		MYSQL_ROW row = mysql_fetch_row(pMsg0->Get()->pSQLResult);

		uint32_t dwPID = 0; str_to_number(dwPID, row[0]);
		if (row[0] && dwPID > 0)
		{
			peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_ALREADY, dwHandle, 0);
			sys_log(0, "ALREADY EXIST AccountChrIdx %d ID %d", packet->account_index, dwPID);
			return;
		}
	}
	else
	{
		peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
		return;
	}

	snprintf(queryStr, sizeof(queryStr), 
			"SELECT COUNT(*) as count FROM player WHERE name='%s'", packet->player_table.name);

	std::unique_ptr<SQLMsg> pMsg1(CDBManager::Instance().DirectQuery(queryStr));

	if (pMsg1->Get()->uiNumRows)
	{
		if (!pMsg1->Get()->pSQLResult)
		{
			peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
			return;
		}

		MYSQL_ROW row = mysql_fetch_row(pMsg1->Get()->pSQLResult);

		if (*row[0] != '0')
		{
			sys_log(0, "ALREADY EXIST name %s, row[0] %s query %s", packet->player_table.name, row[0], queryStr);
			peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_ALREADY, dwHandle, 0);
			return;
		}
	}
	else
	{
		peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
		return;
	}

	queryLen = snprintf(queryStr, sizeof(queryStr), 
			"INSERT INTO player "
			"(id, account_id, name, level, st, ht, dx, iq, "
			"job, voice, dir, x, y, z, "
			"hp, mp, random_hp, random_sp, stat_point, stamina, part_base, part_main, part_hair, "
#ifdef ENABLE_ACCE_SYSTEM
			"part_acce, "
#endif
			"gold, playtime, "
			"skill_level, quickslot) "
			"VALUES(0, %u, '%s', %d, %d, %d, %d, %d, "
			"%d, %d, %d, %d, %d, %d, %d, "
			"%d, %d, %d, %d, %d, %d, %d, 0, "
			
#ifdef ENABLE_ACCE_SYSTEM
			"0, "
#endif
			"%d, 0, ",
			packet->account_id, packet->player_table.name, packet->player_table.level, packet->player_table.st, packet->player_table.ht, packet->player_table.dx, packet->player_table.iq,
			packet->player_table.job, packet->player_table.voice, packet->player_table.dir, packet->player_table.x, packet->player_table.y, packet->player_table.z,
			packet->player_table.hp, packet->player_table.sp, packet->player_table.sRandomHP, packet->player_table.sRandomSP, packet->player_table.stat_point, packet->player_table.stamina,
			packet->player_table.part_base, packet->player_table.part_base, packet->player_table.gold);

	sys_log(0, "PlayerCreate accountid %d name %s level %d gold %d, st %d ht %d job %d",
			packet->account_id, 
			packet->player_table.name, 
			packet->player_table.level, 
			packet->player_table.gold, 
			packet->player_table.st, 
			packet->player_table.ht, 
			packet->player_table.job);

	static char text[4096 + 1];

	CDBManager::Instance().EscapeString(text, packet->player_table.skills, sizeof(packet->player_table.skills));
	queryLen += snprintf(queryStr + queryLen, sizeof(queryStr) - queryLen, "'%s', ", text);
	if (g_test_server)
		sys_log(0, "Create_Player queryLen[%d] TEXT[%s]", queryLen, text);

	CDBManager::Instance().EscapeString(text, packet->player_table.quickslot, sizeof(packet->player_table.quickslot));
	queryLen += snprintf(queryStr + queryLen, sizeof(queryStr) - queryLen, "'%s')", text);

	std::unique_ptr<SQLMsg> pMsg2(CDBManager::Instance().DirectQuery(queryStr));
	if (g_test_server)
		sys_log(0, "Create_Player queryLen[%d] TEXT[%s]", queryLen, text);

	if (pMsg2->Get()->uiAffectedRows <= 0)
	{
		peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_ALREADY, dwHandle, 0);
		sys_log(0, "ALREADY EXIST3 query: %s AffectedRows %u", queryStr, pMsg2->Get()->uiAffectedRows);
		return;
	}

	player_id = (uint32_t)pMsg2->Get()->uiInsertID;

	snprintf(queryStr, sizeof(queryStr), "UPDATE player_index SET pid%d=%u WHERE id=%u", 
			packet->account_index + 1, player_id, packet->account_id);
	std::unique_ptr<SQLMsg> pMsg3(CDBManager::Instance().DirectQuery(queryStr));

	if (pMsg3->Get()->uiAffectedRows <= 0)
	{
		sys_err("QUERY_ERROR: %s", queryStr);

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM player WHERE id=%d", player_id);
		CDBManager::Instance().DirectQuery(queryStr);

		peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
		return;
	}

	TPacketDGCreateSuccess pack;
	memset(&pack, 0, sizeof(pack));

	pack.bAccountCharacterIndex = packet->account_index;

	pack.player.dwID			= player_id;
	strlcpy(pack.player.szName, packet->player_table.name, sizeof(pack.player.szName));
	pack.player.byJob			= packet->player_table.job;
	pack.player.byLevel			= 1;
	pack.player.dwPlayMinutes	= 0;
	pack.player.byST			= packet->player_table.st;
	pack.player.byHT			= packet->player_table.ht;
	pack.player.byDX 			= packet->player_table.dx;
	pack.player.byIQ			= packet->player_table.iq;
	pack.player.wMainPart		= packet->player_table.part_base;
	pack.player.x			= packet->player_table.x;
	pack.player.y			= packet->player_table.y;

	peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_SUCCESS, dwHandle, sizeof(TPacketDGCreateSuccess));
	peer->Encode(&pack, sizeof(TPacketDGCreateSuccess));

	sys_log(0, "7 name %s job %d", pack.player.szName, pack.player.byJob);

	s_createTimeByAccountID[packet->account_id] = time(nullptr);
}

/*
 * PLAYER DELETE
 */
void CClientManager::__QUERY_PLAYER_DELETE(CPeer* peer, uint32_t dwHandle, TPlayerDeletePacket* packet)
{
	if (!packet->login[0] || !packet->player_id || packet->account_index >= PLAYER_PER_ACCOUNT)
		return;

	CLoginData * ld = GetLoginDataByLogin(packet->login);

	if (!ld)
	{
		peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, 1);
		peer->EncodeBYTE(packet->account_index);
		return;
	}

	TAccountTable & r = ld->GetAccountRef();
	if (strlen(r.social_id) < 7 || strncmp(packet->private_code, r.social_id + strlen(r.social_id) - 7, 7))
	{
		sys_log(0, "PLAYER_DELETE FAILED len(%d)", strlen(r.social_id));
		peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, 1);
		peer->EncodeBYTE(packet->account_index);
		return;
	}
	CPlayerTableCache * pkPlayerCache = GetPlayerCache(packet->player_id);
	if (pkPlayerCache)
	{
		TPlayerTable * pTab = pkPlayerCache->Get();
		if (pTab->level >= m_iPlayerDeleteLevelLimit)
		{
			sys_log(0, "PLAYER_DELETE FAILED LEVEL %u >= DELETE LIMIT %d", pTab->level, m_iPlayerDeleteLevelLimit);
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, 1);
			peer->EncodeBYTE(packet->account_index);
			return;
		}
		if (pTab->level < m_iPlayerDeleteLevelLimitLower)
		{
			sys_log(0, "PLAYER_DELETE FAILED LEVEL %u < DELETE LIMIT %d", pTab->level, m_iPlayerDeleteLevelLimitLower);
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, 1);
			peer->EncodeBYTE(packet->account_index);
			return;
		}
	}
	char szQuery[128];
	snprintf(szQuery, sizeof(szQuery), "SELECT p.id, p.level, p.name FROM player_index AS i, player AS p WHERE pid%u=%u AND pid%u=p.id",
			packet->account_index + 1, packet->player_id, packet->account_index + 1);

	auto pi = new ClientHandleInfo(dwHandle, packet->player_id);
	pi->account_index = packet->account_index;

	sys_log(0, "PLAYER_DELETE TRY: %s %d pid%d", packet->login, packet->player_id, packet->account_index + 1);
	CDBManager::Instance().ReturnQuery(szQuery, QID_PLAYER_DELETE, peer->GetHandle(), pi);
}

//
// @version	05/06/10 Bang2ni - 플레이어 삭제시 가격정보 리스트 삭제 추가.
//
void CClientManager::__RESULT_PLAYER_DELETE(CPeer *peer, SQLMsg* msg)
{
	CQueryInfo * qi = (CQueryInfo *) msg->pvUserData;
	ClientHandleInfo * pi = (ClientHandleInfo *) qi->pvData;

	if (msg->Get() && msg->Get()->uiNumRows)
	{
		MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);

		uint32_t dwPID = 0;
		str_to_number(dwPID, row[0]);

		int32_t deletedLevelLimit = 0;
		str_to_number(deletedLevelLimit, row[1]);

		char szName[64];
		strlcpy(szName, row[2], sizeof(szName));

		if (deletedLevelLimit >= m_iPlayerDeleteLevelLimit)
		{
			sys_log(0, "PLAYER_DELETE FAILED LEVEL %u >= DELETE LIMIT %d", deletedLevelLimit, m_iPlayerDeleteLevelLimit);
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, 1);
			peer->EncodeBYTE(pi->account_index);
			return;
		}

		if (deletedLevelLimit < m_iPlayerDeleteLevelLimitLower)
		{
			sys_log(0, "PLAYER_DELETE FAILED LEVEL %u < DELETE LIMIT %d", deletedLevelLimit, m_iPlayerDeleteLevelLimitLower);
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, 1);
			peer->EncodeBYTE(pi->account_index);
			return;
		}

		char queryStr[ASQL_QUERY_MAX_LEN];

		snprintf(queryStr, sizeof(queryStr), "INSERT INTO player_deleted SELECT * FROM player WHERE id=%d", pi->player_id);
		std::unique_ptr<SQLMsg> pIns(CDBManager::Instance().DirectQuery(queryStr));

		if (pIns->Get()->uiAffectedRows == 0 || pIns->Get()->uiAffectedRows == static_cast<uint32_t>(-1))
		{
			sys_log(0, "PLAYER_DELETE FAILED %u CANNOT INSERT TO player_deleted", dwPID);

			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, 1);
			peer->EncodeBYTE(pi->account_index);
			return;
		}

		// 삭제 성공
		sys_log(0, "PLAYER_DELETE SUCCESS %u", dwPID);

		char account_index_string[16];

		snprintf(account_index_string, sizeof(account_index_string), "player_id%d", m_iPlayerIDStart + pi->account_index);

		// 플레이어 테이블을 캐쉬에서 삭제한다.
		CPlayerTableCache * pkPlayerCache = GetPlayerCache(pi->player_id);

		if (pkPlayerCache)
		{
			m_map_playerCache.erase(pi->player_id);
			delete pkPlayerCache;
		}

		// 아이템들을 캐쉬에서 삭제한다.
		TItemCacheSet * pSet = GetItemCacheSet(pi->player_id);

		if (pSet)
		{
			auto it = pSet->begin();

			while (it != pSet->end())
			{
				CItemCache * pkItemCache = *it++;
				DeleteItemCache(pkItemCache->Get()->id);
			}

			pSet->clear();
			delete pSet;

			m_map_pkItemCacheSetPtr.erase(pi->player_id);
		}

		snprintf(queryStr, sizeof(queryStr), "UPDATE player_index SET pid%u=0 WHERE pid%u=%u", 
				pi->account_index + 1, 
				pi->account_index + 1, 
				pi->player_id);

		std::unique_ptr<SQLMsg> pMsg(CDBManager::Instance().DirectQuery(queryStr));

		if (pMsg->Get()->uiAffectedRows == 0 || pMsg->Get()->uiAffectedRows == static_cast<uint32_t>(-1))
		{
			sys_log(0, "PLAYER_DELETE FAIL WHEN UPDATE account table");
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, 1);
			peer->EncodeBYTE(pi->account_index);
			return;
		}

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM player WHERE id=%u", pi->player_id);
		delete CDBManager::Instance().DirectQuery(queryStr);

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM item WHERE owner_id=%d AND (window in ('INVENTORY','EQUIPMENT','DRAGON_SOUL_INVENTORY','BELT_INVENTORY'))", pi->player_id);
		delete CDBManager::Instance().DirectQuery(queryStr);

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM quest WHERE dwPID=%u", pi->player_id);
		CDBManager::Instance().AsyncQuery(queryStr);

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM affect WHERE dwPID=%u", pi->player_id);
		CDBManager::Instance().AsyncQuery(queryStr);

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM guild_member WHERE pid=%u", pi->player_id);
		CDBManager::Instance().AsyncQuery(queryStr);

		// MYSHOP_PRICE_LIST
		snprintf(queryStr, sizeof(queryStr), "DELETE FROM myshop_pricelist WHERE owner_id=%u", pi->player_id);
		CDBManager::Instance().AsyncQuery(queryStr);
		// END_OF_MYSHOP_PRICE_LIST

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM messenger_list WHERE account='%s' OR companion='%s'", szName, szName);
		CDBManager::Instance().AsyncQuery(queryStr);

		peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_SUCCESS, pi->dwHandle, 1);
		peer->EncodeBYTE(pi->account_index);
	}
	else
	{
		// 삭제 실패
		sys_log(0, "PLAYER_DELETE FAIL NO ROW");
		peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, 1);
		peer->EncodeBYTE(pi->account_index);
	}
}

void CClientManager::QUERY_ADD_AFFECT(CPeer *, TPacketGDAddAffect * p)
{
	char queryStr[ASQL_QUERY_MAX_LEN];
	/*
	   snprintf(queryStr, sizeof(queryStr),
	   "INSERT INTO affect (dwPID, bType, bApplyOn, lApplyValue, dwFlag, lDuration, lSPCost) "
	   "VALUES(%u, %u, %u, %d, %u, %d, %d) "
	   "ON DUPLICATE KEY UPDATE lApplyValue=%d, dwFlag=%u, lDuration=%d, lSPCost=%d",
	   p->dwPID,
	   p->elem.dwType,
	   p->elem.bApplyOn,
	   p->elem.lApplyValue,
	   p->elem.dwFlag,
	   p->elem.lDuration,
	   p->elem.lSPCost,
	   p->elem.lApplyValue,
	   p->elem.dwFlag,
	   p->elem.lDuration,
	   p->elem.lSPCost);
	   */
	snprintf(queryStr, sizeof(queryStr),
			"REPLACE INTO affect (dwPID, bType, bApplyOn, lApplyValue, dwFlag, lDuration, lSPCost) "
			"VALUES(%u, %u, %u, %d, %u, %d, %d)",
			p->dwPID,
			p->elem.dwType,
			p->elem.bApplyOn,
			p->elem.lApplyValue,
			p->elem.dwFlag,
			p->elem.lDuration,
			p->elem.lSPCost);

	CDBManager::Instance().AsyncQuery(queryStr);
}

void CClientManager::QUERY_REMOVE_AFFECT(CPeer *, TPacketGDRemoveAffect * p)
{
	char queryStr[ASQL_QUERY_MAX_LEN];

	snprintf(queryStr, sizeof(queryStr),
			"DELETE FROM affect WHERE dwPID=%u AND bType=%u AND bApplyOn=%u", p->dwPID, p->dwType, p->bApplyOn);

	CDBManager::Instance().AsyncQuery(queryStr);
}

void CClientManager::InsertLogoutPlayer(uint32_t pid)
{
	auto it = m_map_logout.find(pid);

	// 존재하지 않을경우 추가
	if (it != m_map_logout.end())
	{
		// 존재할경우 시간만 갱신
		if (g_log)
			sys_log(0, "LOGOUT: Update player time pid(%d)", pid);

		it->second->time = time(nullptr);
		return;
	}
		
	auto pLogout = new TLogoutPlayer;
	pLogout->pid = pid;
	pLogout->time = time(nullptr);
	m_map_logout.insert(std::make_pair(pid, pLogout));

	if (g_log)
		sys_log(0, "LOGOUT: Insert player pid(%d)", pid);
}

void CClientManager::DeleteLogoutPlayer(uint32_t pid)
{
	auto it = m_map_logout.find(pid);

	if (it != m_map_logout.end())
	{
		delete it->second;
		m_map_logout.erase(it);
	}
}

extern int32_t g_iLogoutSeconds;

void CClientManager::UpdateLogoutPlayer()
{
	time_t now = time(nullptr);

	auto it = m_map_logout.begin();

	while (it != m_map_logout.end())
	{
		TLogoutPlayer* pLogout = it->second;

		if (now - g_iLogoutSeconds > pLogout->time)
		{
			FlushItemCacheSet(pLogout->pid);
			FlushPlayerCacheSet(pLogout->pid);

			delete pLogout;
			m_map_logout.erase(it++);
		}
		else
			++it;
	}
}

void CClientManager::FlushPlayerCacheSet(uint32_t pid)
{
	auto it = m_map_playerCache.find(pid);

	if (it != m_map_playerCache.end())
	{
		CPlayerTableCache * c = it->second;
		m_map_playerCache.erase(it);

		c->Flush();
		delete c; 
	}
}

