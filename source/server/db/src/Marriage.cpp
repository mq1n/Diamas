#include "stdafx.h"
#include "Marriage.h"
#include "Main.h"
#include "DBManager.h"
#include "ClientManager.h"

namespace marriage
{
	const uint32_t WEDDING_LENGTH = 60 * 60; // sec
	bool operator < (const TWedding& lhs, const TWedding& rhs)
	{
		return lhs.dwTime < rhs.dwTime;
	}

	bool operator > (const TWedding& lhs, const TWedding& rhs)
	{
		return lhs.dwTime > rhs.dwTime;
	}

	bool operator > (const TWeddingInfo &lhs, const TWeddingInfo& rhs)
	{
		return lhs.dwTime > rhs.dwTime;
	}

	CManager::CManager()
	{
	}

	CManager::~CManager()
	= default;

	bool CManager::Initialize()
	{
		char szQuery[1024];

		snprintf(szQuery, sizeof(szQuery),
				"SELECT pid1, pid2, love_point, time, is_married, p1.name, p2.name FROM marriage, player as p1, player as p2 WHERE p1.id = pid1 AND p2.id = pid2");

		std::unique_ptr<SQLMsg> pmsg_delete(CDBManager::Instance().DirectQuery("DELETE FROM marriage WHERE is_married = 0"));
		std::unique_ptr<SQLMsg> pmsg(CDBManager::Instance().DirectQuery(szQuery));

		SQLResult * pRes = pmsg->Get();
		sys_log(0, "MarriageList(size=%u)", pRes->uiNumRows);

		if (pRes->uiNumRows > 0)
		{
			for (size_t uiRow = 0; uiRow != pRes->uiNumRows; ++uiRow)
			{
				MYSQL_ROW row = mysql_fetch_row(pRes->pSQLResult);

				uint32_t pid1 = 0; str_to_number(pid1, row[0]);
				uint32_t pid2 = 0; str_to_number(pid2, row[1]);
				int32_t love_point = 0; str_to_number(love_point, row[2]);
				uint32_t time = 0; str_to_number(time, row[3]);
				uint8_t is_married = 0; str_to_number(is_married, row[4]);
				const char* name1 = row[5];
				const char* name2 = row[6];

				auto pMarriage = new TMarriage(pid1, pid2, love_point, time, is_married, name1, name2);
				m_Marriages.insert(pMarriage);
				m_MarriageByPID.insert(std::make_pair(pid1, pMarriage));
				m_MarriageByPID.insert(std::make_pair(pid2, pMarriage));

				sys_log(0, "Marriage %u: LP:%d TM:%u ST:%d %10lu:%16s %10lu:%16s ", uiRow, love_point, time, is_married, pid1, name1, pid2, name2);
			}
		}
		return true;
	}

	TMarriage* CManager::Get(uint32_t dwPlayerID)
	{
		auto it = m_MarriageByPID.find(dwPlayerID);

		if (it != m_MarriageByPID.end())
			return it->second;

		return nullptr;
	}

	void Align(uint32_t& rPID1, uint32_t& rPID2)
	{
		if (rPID1 > rPID2)
			std::swap(rPID1, rPID2);
	}

	void CManager::Add(uint32_t dwPID1, uint32_t dwPID2, const char* szName1, const char* szName2)
	{
		uint32_t now = CClientManager::Instance().GetCurrentTime();
		if (IsMarried(dwPID1) || IsMarried(dwPID2))
		{
			sys_err("cannot marry already married character. %d - %d", dwPID1, dwPID2);
			return;
		}

		Align(dwPID1, dwPID2);

		char szQuery[512];
		snprintf(szQuery, sizeof(szQuery), "INSERT INTO marriage(pid1, pid2, love_point, time) VALUES (%u, %u, 0, %u)", dwPID1, dwPID2, now);

		std::unique_ptr<SQLMsg> pmsg(CDBManager::Instance().DirectQuery(szQuery));

		SQLResult* res = pmsg->Get();
		if (res->uiAffectedRows == 0 || res->uiAffectedRows == (uint32_t)-1)
		{
			sys_err("cannot insert marriage");
			return;
		}

		sys_log(0, "MARRIAGE ADD %u %u", dwPID1, dwPID2);

		auto pMarriage = new TMarriage(dwPID1, dwPID2, 0, now, 0, szName1, szName2);
		m_Marriages.insert(pMarriage);
		m_MarriageByPID.insert(std::make_pair(dwPID1, pMarriage));
		m_MarriageByPID.insert(std::make_pair(dwPID2, pMarriage));

		TPacketMarriageAdd p;
		p.dwPID1 = dwPID1;
		p.dwPID2 = dwPID2;
		p.tMarryTime = now;
		strlcpy(p.szName1, szName1, sizeof(p.szName1));
		strlcpy(p.szName2, szName2, sizeof(p.szName2));
		CClientManager::Instance().ForwardPacket(HEADER_DG_MARRIAGE_ADD, &p, sizeof(p));
	}

	void CManager::Update(uint32_t dwPID1, uint32_t dwPID2, INT iLovePoint, uint8_t byMarried)
	{
		TMarriage* pMarriage = Get(dwPID1);
		if (!pMarriage || pMarriage->GetOther(dwPID1) != dwPID2)
		{
			sys_err("not under marriage : %u %u", dwPID1, dwPID2);
			return;
		}

		if (pMarriage->love_point == iLovePoint && pMarriage->is_married == byMarried)
			return;
		if (iLovePoint < 0)
			return;
		Align(dwPID1, dwPID2);

		char szQuery[512];
		snprintf(szQuery, sizeof(szQuery), "UPDATE marriage SET love_point = %d, is_married = %d WHERE pid1 = %u AND pid2 = %u", 
				iLovePoint, byMarried, pMarriage->pid1, pMarriage->pid2);

		std::unique_ptr<SQLMsg> pmsg(CDBManager::Instance().DirectQuery(szQuery));

		SQLResult* res = pmsg->Get();
		if (res == nullptr)
		{
			sys_err("pmsg->Get() == nullptr! PID1: %d PID2: %d", dwPID1, dwPID2);
			return;
		}
		
		if (res->uiAffectedRows == 0 || res->uiAffectedRows == (uint32_t)-1)
		{
			sys_err("cannot update marriage : PID:%u %u", dwPID1, dwPID2);
			return;
		}

		sys_log(0, "MARRIAGE UPDATE PID:%u %u LP:%u ST:%d", dwPID1, dwPID2, iLovePoint, byMarried);
		pMarriage->love_point = iLovePoint;
		pMarriage->is_married = byMarried;

		TPacketMarriageUpdate p;
		p.dwPID1 = dwPID1;
		p.dwPID2 = dwPID2;
		p.iLovePoint = pMarriage->love_point;
		p.byMarried = pMarriage->is_married;
		CClientManager::Instance().ForwardPacket(HEADER_DG_MARRIAGE_UPDATE, &p, sizeof(p));
	}

	void CManager::Remove(uint32_t dwPID1, uint32_t dwPID2)
	{
		TMarriage* pMarriage = Get(dwPID1);

		if (pMarriage)
		{
			sys_log(0, "Break Marriage pid1 %d pid2 %d Other %d", dwPID1, dwPID2, pMarriage->GetOther(dwPID1));
		}
		if (!pMarriage || pMarriage->GetOther(dwPID1) != dwPID2)
		{
			auto it = m_MarriageByPID.begin();

			for (; it != m_MarriageByPID.end(); ++it)
			{
				sys_log(0, "Marriage List pid1 %d pid2 %d", it->second->pid1, it->second->pid2);
			}
			sys_err("not under marriage : PID:%u %u", dwPID1, dwPID2);
			return;
		}

		Align(dwPID1, dwPID2);

		char szQuery[512];
		snprintf(szQuery, sizeof(szQuery), "DELETE FROM marriage WHERE pid1 = %u AND pid2 = %u", dwPID1, dwPID2);

		std::unique_ptr<SQLMsg> pmsg(CDBManager::Instance().DirectQuery(szQuery));

		SQLResult* res = pmsg->Get();
		if (res->uiAffectedRows == 0 || res->uiAffectedRows == (uint32_t)-1)
		{
			sys_err("cannot delete marriage : PID:%u %u", dwPID1, dwPID2);
			return;
		}

		sys_log(0, "MARRIAGE REMOVE PID:%u %u", dwPID1, dwPID2);

		m_Marriages.erase(pMarriage);
		m_MarriageByPID.erase(dwPID1);
		m_MarriageByPID.erase(dwPID2);

		TPacketMarriageRemove p;
		p.dwPID1 = dwPID1;
		p.dwPID2 = dwPID2;
		CClientManager::Instance().ForwardPacket(HEADER_DG_MARRIAGE_REMOVE, &p, sizeof(p));

		delete pMarriage;
	}

	void CManager::EngageToMarriage(uint32_t dwPID1, uint32_t dwPID2)
	{
		TMarriage* pMarriage = Get(dwPID1);
		if (!pMarriage || pMarriage->GetOther(dwPID1) != dwPID2)
		{
			sys_err("not under marriage : PID:%u %u", dwPID1, dwPID2);
			return;
		}

		if (pMarriage->is_married)
		{
			sys_err("already married, cannot change engage to marry : PID:%u %u", dwPID1, dwPID2);
			return;
		}

		Align(dwPID1, dwPID2);

		char szQuery[512];
		snprintf(szQuery, sizeof(szQuery), "UPDATE marriage SET is_married = 1 WHERE pid1 = %u AND pid2 = %u", 
				pMarriage->pid1, pMarriage->pid2);

		std::unique_ptr<SQLMsg> pmsg(CDBManager::Instance().DirectQuery(szQuery));

		SQLResult* res = pmsg->Get();
		if (res->uiAffectedRows == 0 || res->uiAffectedRows == (uint32_t)-1)
		{
			sys_err("cannot change engage to marriage : PID:%u %u affectedRows = (%u)", dwPID1, dwPID2, res->uiAffectedRows);
			return;
		}

		sys_log(0, "MARRIAGE ENGAGE->MARRIAGE PID:%u %u", dwPID1, dwPID2);
		pMarriage->is_married = 1;

		TPacketMarriageUpdate p;
		p.dwPID1 = dwPID1;
		p.dwPID2 = dwPID2;
		p.iLovePoint = pMarriage->love_point;
		p.byMarried = pMarriage->is_married;
		CClientManager::Instance().ForwardPacket(HEADER_DG_MARRIAGE_UPDATE, &p, sizeof(p));
	}

	void CManager::OnSetup(CPeer* peer)
	{
		// 결혼한 사람들 보내기
		for (auto pMarriage : m_Marriages)
		{
			{
				TPacketMarriageAdd p;
				p.dwPID1 = pMarriage->pid1;
				p.dwPID2 = pMarriage->pid2;
				p.tMarryTime = pMarriage->time;
				strlcpy(p.szName1, pMarriage->name1.c_str(), sizeof(p.szName1));
				strlcpy(p.szName2, pMarriage->name2.c_str(), sizeof(p.szName2));
				peer->EncodeHeader(HEADER_DG_MARRIAGE_ADD, 0, sizeof(p));
				peer->Encode(&p, sizeof(p));
			}

			{
				TPacketMarriageUpdate p;
				p.dwPID1 = pMarriage->pid1;
				p.dwPID2 = pMarriage->pid2;
				p.iLovePoint = pMarriage->love_point;
				p.byMarried	= pMarriage->is_married;
				peer->EncodeHeader(HEADER_DG_MARRIAGE_UPDATE, 0, sizeof(p));
				peer->Encode(&p, sizeof(p));
			}
		}

		// 결혼식 보내기
		for (auto & it : m_mapRunningWedding)
		{
			const TWedding& t = it.second;

			TPacketWeddingReady p;
			p.dwPID1 = t.dwPID1;
			p.dwPID2 = t.dwPID2;
			p.dwMapIndex = t.dwMapIndex;

			peer->EncodeHeader(HEADER_DG_WEDDING_READY, 0, sizeof(p));
			peer->Encode(&p, sizeof(p));

			TPacketWeddingStart p2;
			p2.dwPID1 = t.dwPID1;
			p2.dwPID2 = t.dwPID2;

			peer->EncodeHeader(HEADER_DG_WEDDING_START, 0, sizeof(p2));
			peer->Encode(&p2, sizeof(p2));
		}
	}

	void CManager::ReadyWedding(uint32_t dwMapIndex, uint32_t dwPID1, uint32_t dwPID2)
	{
		uint32_t dwStartTime = CClientManager::Instance().GetCurrentTime();
		m_pqWeddingStart.push(TWedding(dwStartTime + 5, dwMapIndex, dwPID1, dwPID2));
	}

	void CManager::EndWedding(uint32_t dwPID1, uint32_t dwPID2)
	{
		auto it = m_mapRunningWedding.find(std::make_pair(dwPID1, dwPID2));
		if (it == m_mapRunningWedding.end())
		{
			sys_err("try to end wedding %u %u", dwPID1, dwPID2);
			return;
		}

		TWedding& w = it->second;

		TPacketWeddingEnd p;
		p.dwPID1 = w.dwPID1;
		p.dwPID2 = w.dwPID2;
		CClientManager::Instance().ForwardPacket(HEADER_DG_WEDDING_END, &p, sizeof(p));
		m_mapRunningWedding.erase(it);
	}

	void CManager::Update()
	{
		uint32_t now = CClientManager::Instance().GetCurrentTime();

		if (!m_pqWeddingEnd.empty())
		{
			while (!m_pqWeddingEnd.empty() && m_pqWeddingEnd.top().dwTime <= now)
			{
				TWeddingInfo wi = m_pqWeddingEnd.top();
				m_pqWeddingEnd.pop();

				auto it = m_mapRunningWedding.find(std::make_pair(wi.dwPID1, wi.dwPID2));
				if (it == m_mapRunningWedding.end())
					continue;

				TWedding& w = it->second;

				TPacketWeddingEnd p;
				p.dwPID1 = w.dwPID1;
				p.dwPID2 = w.dwPID2;
				CClientManager::Instance().ForwardPacket(HEADER_DG_WEDDING_END, &p, sizeof(p));
				m_mapRunningWedding.erase(it);

				auto it_marriage = m_MarriageByPID.find(w.dwPID1);

				if (it_marriage != m_MarriageByPID.end())
				{
					TMarriage* pMarriage = it_marriage->second;
					if (!pMarriage->is_married)
					{
						Remove(pMarriage->pid1, pMarriage->pid2);
					}
				}
			}
		}
		if (!m_pqWeddingStart.empty())
		{
			while (!m_pqWeddingStart.empty() && m_pqWeddingStart.top().dwTime <= now)
			{
				TWedding w = m_pqWeddingStart.top();
				m_pqWeddingStart.pop();

				TPacketWeddingStart p;
				p.dwPID1 = w.dwPID1;
				p.dwPID2 = w.dwPID2;
				CClientManager::Instance().ForwardPacket(HEADER_DG_WEDDING_START, &p, sizeof(p));

				w.dwTime += WEDDING_LENGTH;
				m_pqWeddingEnd.push(TWeddingInfo(w.dwTime, w.dwPID1, w.dwPID2));
				m_mapRunningWedding.insert(std::make_pair(std::make_pair(w.dwPID1, w.dwPID2), w));
			}
		}
	}
}
