#include "stdafx.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "db.h"
#include "guild_manager.h"
#include "marriage.h"
#include "desc.h"

/*
   Return Value
		0 : 알 수 없는 에러 or 쿼리 에러
		1 : 동일한 제국으로 바꾸려고함
		2 : 길드 가입한 캐릭터가 있음
		3 : 결혼한 캐릭터가 있음

		999 : 제국 이동 성공
*/
int32_t CHARACTER::ChangeEmpire(uint8_t empire)
{
	if (GetEmpire() == empire)
		return 1;

	char szQuery[1024+1];
	uint32_t dwAID;
	uint32_t dwPID[4];
	memset(dwPID, 0, sizeof(dwPID));

	{
		// 1. 내 계정의 모든 pid를 얻어 온다
		snprintf(szQuery, sizeof(szQuery),
				"SELECT id, pid1, pid2, pid3, pid4, pid5 FROM player_index WHERE pid1=%u OR pid2=%u OR pid3=%u OR pid4=%u OR pid5=%u AND empire=%u",
				GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetEmpire());
		std::unique_ptr<SQLMsg> msg(DBManager::Instance().DirectQuery(szQuery));

		if (msg->Get()->uiNumRows == 0)
		{
			return 0;
		}

		MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);

		str_to_number(dwAID, row[0]);
		str_to_number(dwPID[0], row[1]);
		str_to_number(dwPID[1], row[2]);
		str_to_number(dwPID[2], row[3]);
		str_to_number(dwPID[3], row[4]);
	}

	const int32_t loop = 4;

	{
		// 2. 각 캐릭터의 길드 정보를 얻어온다.
		//   한 캐릭터라도 길드에 가입 되어 있다면, 제국 이동을 할 수 없다.
		uint32_t dwGuildID[4];
		CGuild * pGuild[4];
		SQLMsg * pMsg = nullptr;
		
		for (int32_t i = 0; i < loop; ++i)
		{
			snprintf(szQuery, sizeof(szQuery), "SELECT guild_id FROM guild_member WHERE pid=%u", dwPID[i]);

			pMsg = DBManager::Instance().DirectQuery(szQuery);

			if (pMsg != nullptr)
			{
				if (pMsg->Get()->uiNumRows > 0)
				{
					MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

					str_to_number(dwGuildID[i], row[0]);

					pGuild[i] = CGuildManager::Instance().FindGuild(dwGuildID[i]);

					if (pGuild[i] != nullptr)
					{
						M2_DELETE(pMsg);
						return 2;
					}
				}
				else
				{
					dwGuildID[i] = 0;
					pGuild[i] = nullptr;
				}

				M2_DELETE(pMsg);
			}
		}
	}

	{
		// 3. 각 캐릭터의 결혼 정보를 얻어온다.
		//   한 캐릭터라도 결혼 상태라면 제국 이동을 할 수 없다.
		for (int32_t i = 0; i < loop; ++i)
		{
			if (marriage::CManager::Instance().IsEngagedOrMarried(dwPID[i]) == true)
				return 3;
		}
	}
	
	{
		// 4. db의 제국 정보를 업데이트 한다.
		snprintf(szQuery, sizeof(szQuery), "UPDATE player_index SET empire=%u WHERE pid1=%u OR pid2=%u OR pid3=%u OR pid4=%u OR pid5=%u AND empire=%u",
				empire, GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetEmpire());
		std::unique_ptr<SQLMsg> msg(DBManager::Instance().DirectQuery(szQuery));

		if (msg->Get()->uiAffectedRows > 0)
		{
			// 5. 제국 변경 이력을 추가한다.
			SetChangeEmpireCount();
			SetEmpire(empire);
			UpdatePacket();
			return 999;
		}
	}

	return 0;
}

int32_t CHARACTER::GetChangeEmpireCount() const
{
	char szQuery[1024+1];
	uint32_t dwAID = GetAID();

	if (dwAID == 0)
		return 0;

	snprintf(szQuery, sizeof(szQuery), "SELECT change_count FROM change_empire WHERE account_id = %u", dwAID);

	SQLMsg * pMsg = DBManager::Instance().DirectQuery(szQuery);

	if (pMsg != nullptr)
	{
		if (pMsg->Get()->uiNumRows == 0)
		{
			M2_DELETE(pMsg);
			return 0;
		}

		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

		uint32_t count = 0;
		str_to_number(count, row[0]);

		M2_DELETE(pMsg);

		return count;
	}

	return 0;
}

void CHARACTER::SetChangeEmpireCount()
{
	char szQuery[1024+1];

	uint32_t dwAID = GetAID();

	if (dwAID == 0) return;

	int32_t count = GetChangeEmpireCount();

	if (count == 0)
	{
		count++;
		snprintf(szQuery, sizeof(szQuery), "INSERT INTO change_empire VALUES(%u, %d, NOW())", dwAID, count);
	}
	else
	{
		count++;
		snprintf(szQuery, sizeof(szQuery), "UPDATE change_empire SET change_count=%d WHERE account_id=%u", count, dwAID);
	}

	std::unique_ptr<SQLMsg> pmsg(DBManager::Instance().DirectQuery(szQuery));
}

uint32_t CHARACTER::GetAID() const
{
	if (GetDesc()) 
	{
		const TAccountTable& rkTab = GetDesc()->GetAccountTable();
		if (rkTab.id)
			return rkTab.id;
	}

	char szQuery[1024+1];
	uint32_t dwAID = 0;
	snprintf(szQuery, sizeof(szQuery), "SELECT id FROM player_index WHERE pid1=%u OR pid2=%u OR pid3=%u OR pid4=%u OR pid5=%u AND empire=%u",
			GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetEmpire());
	SQLMsg* pMsg = DBManager::Instance().DirectQuery(szQuery);

	if (pMsg != nullptr)
	{
		if (pMsg->Get()->uiNumRows == 0)
		{
			M2_DELETE(pMsg);
			return 0;
		}

		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

		str_to_number(dwAID, row[0]);

		M2_DELETE(pMsg);

		return dwAID;
	}
	else
	{
		return 0;
	}
}

