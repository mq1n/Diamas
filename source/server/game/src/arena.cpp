#include "stdafx.h"
#include <fstream>
#include "constants.h"
#include "config.h"
#include "packet.h"
#include "desc.h"
#include "buffer_manager.h"
#include "start_position.h"
#include "quest_manager.h"
#include "char.h"
#include "char_manager.h"
#include "arena.h"

CArena::CArena(uint16_t startA_X, uint16_t startA_Y, uint16_t startB_X, uint16_t startB_Y)
{
	m_StartPointA.x = startA_X;
	m_StartPointA.y = startA_Y;
	m_StartPointA.z = 0;

	m_StartPointB.x = startB_X;
	m_StartPointB.y = startB_Y;
	m_StartPointB.z = 0;

	m_ObserverPoint.x = (startA_X + startB_X) / 2;
	m_ObserverPoint.y = (startA_Y + startB_Y) / 2;
	m_ObserverPoint.z = 0;

	m_pEvent = nullptr;
	m_pTimeOutEvent = nullptr;

	Clear();
}

bool CArenaManager::Initialize()
{
	std::ifstream inf((LocaleService_GetBasePath() + "/arena_forbidden_items.txt").c_str());

	if (!inf.is_open())
	{
		sys_err("ArenaManager cannot open 'arena_forbidden_items.txt'!");
		return false;
	}

	std::string ln;
	while (getline(inf, ln))
	{
		if (ln.empty())
			continue;
		
		uint32_t iVnum = atoi(ln.c_str());
		m_listForbidden.push_back(iVnum);
	}

	return true;
}

void CArena::Clear()
{
	m_dwPIDA = 0;
	m_dwPIDB = 0;

	if (m_pEvent != nullptr)
	{
		event_cancel(&m_pEvent);
	}

	if (m_pTimeOutEvent != nullptr)
	{
		event_cancel(&m_pTimeOutEvent);
	}

	m_dwSetCount = 0;
	m_dwSetPointOfA = 0;
	m_dwSetPointOfB = 0;
}

bool CArenaManager::AddArena(uint32_t mapIdx, uint16_t startA_X, uint16_t startA_Y, uint16_t startB_X, uint16_t startB_Y)
{
	CArenaMap *pArenaMap = nullptr;
	auto iter = m_mapArenaMap.find(mapIdx);
	if (iter == m_mapArenaMap.end())
	{
		pArenaMap = M2_NEW CArenaMap;
		m_mapArenaMap.emplace(mapIdx, pArenaMap);
	}
	else
	{
		pArenaMap = iter->second;
	}

	if (pArenaMap->AddArena(mapIdx, startA_X, startA_Y, startB_X, startB_Y) == false)
	{
		sys_log(0, "CArenaManager::AddArena - AddMap Error MapID: %d", mapIdx);
		return false;
	}

	return true;
}

bool CArenaMap::AddArena(uint32_t mapIdx, uint16_t startA_X, uint16_t startA_Y, uint16_t startB_X, uint16_t startB_Y)
{
	for (const auto & arena : m_listArena)
	{
		if (arena->CheckArea(startA_X, startA_Y, startB_X, startB_Y) == false)
		{
			sys_log(0, "CArenaMap::AddArena - Same Start Position set. stA(%d, %d) stB(%d, %d)", startA_X, startA_Y, startB_X, startB_Y);
			return false;
		}
	}

	m_dwMapIndex = mapIdx;

	auto pArena = M2_NEW CArena(startA_X, startA_Y, startB_X, startB_Y);
	m_listArena.push_back(pArena);

	return true;
}

void CArenaManager::Destroy()
{
	for (auto iter = m_mapArenaMap.begin(); iter != m_mapArenaMap.end(); ++iter)
	{
		CArenaMap* pArenaMap = iter->second;
		pArenaMap->Destroy();

		M2_DELETE(pArenaMap);
	}
	m_mapArenaMap.clear();
}

void CArenaMap::Destroy()
{
	sys_log(0, "ARENA: ArenaMap will be destroy. mapIndex(%d)", m_dwMapIndex);

	for (auto iter = m_listArena.begin(); iter != m_listArena.end(); ++iter)
	{
		CArena* pArena = *iter;
		pArena->EndDuel();

		M2_DELETE(pArena);
	}
	m_listArena.clear();
}

bool CArena::CheckArea(uint16_t startA_X, uint16_t startA_Y, uint16_t startB_X, uint16_t startB_Y)
{
	if (m_StartPointA.x == startA_X && m_StartPointA.y == startA_Y && m_StartPointB.x == startB_X && m_StartPointB.y == startB_Y)
		return false;	

	return true;
}

void CArenaManager::SendArenaMapListTo(LPCHARACTER pChar)
{
	for (auto iter = m_mapArenaMap.begin(); iter != m_mapArenaMap.end(); ++iter)
	{
		CArenaMap* pArena = iter->second;
		pArena->SendArenaMapListTo(pChar, (iter->first));
	}
}

void CArenaMap::SendArenaMapListTo(LPCHARACTER pChar, uint32_t mapIdx)
{
	if (pChar == nullptr)
		return;

	for (auto const & arena : m_listArena)
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, "ArenaMapInfo Map: %u stA(%d, %d) stB(%d, %d)", mapIdx, 
			arena->GetStartPointA().x, arena->GetStartPointA().y,
			arena->GetStartPointB().x, arena->GetStartPointB().y);
	}
}

bool CArenaManager::StartDuel(LPCHARACTER pCharFrom, LPCHARACTER pCharTo, int32_t nSetPoint, int32_t nMinute)
{
	if (pCharFrom == nullptr || pCharTo == nullptr) 
		return false;

	for (auto iter = m_mapArenaMap.begin(); iter != m_mapArenaMap.end(); ++iter)
	{
		CArenaMap* pArenaMap = iter->second;
		if (pArenaMap->StartDuel(pCharFrom, pCharTo, nSetPoint, nMinute) == true)
		{
			return true;
		}
	}

	return false;
}

bool CArenaMap::StartDuel(LPCHARACTER pCharFrom, LPCHARACTER pCharTo, int32_t nSetPoint, int32_t nMinute)
{
	for (const auto & arena : m_listArena)
	{
		if (arena->IsEmpty() == true)
		{
			return arena->StartDuel(pCharFrom, pCharTo, nSetPoint, nMinute);
		}
	}

	return false;
}

EVENTINFO(TArenaEventInfo)
{
	CArena *pArena;
	uint8_t state;

	TArenaEventInfo() :
		pArena(nullptr), state(0)
	{
	}
};

EVENTFUNC(ready_to_start_event)
{
	if (event == nullptr)
		return 0;

	if (event->info == nullptr)
		return 0;

	TArenaEventInfo* info = dynamic_cast<TArenaEventInfo*>(event->info);

	if ( info == nullptr )
	{
		sys_err( "ready_to_start_event> <Factor> Null pointer" );
		return 0;
	}

	CArena* pArena = info->pArena;

	if (pArena == nullptr)
	{
		sys_err("ARENA: Arena start event info is null.");
		return 0;
	}

	LPCHARACTER chA = pArena->GetPlayerA();
	LPCHARACTER chB = pArena->GetPlayerB();

	if (chA == nullptr || chB == nullptr)
	{
		sys_err("ARENA: Player err in event func ready_start_event");

		if (chA != nullptr)
		{
			chA->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("��� ��밡 ����� ����� �����մϴ�."));
			sys_log(0, "ARENA: Oppernent is disappered. MyPID(%u) OppPID(%u)", pArena->GetPlayerAPID(), pArena->GetPlayerBPID());
		}

		if (chB != nullptr)
		{
			chB->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("��� ��밡 ����� ����� �����մϴ�."));
			sys_log(0, "ARENA: Oppernent is disappered. MyPID(%u) OppPID(%u)", pArena->GetPlayerBPID(), pArena->GetPlayerAPID());
		}

		pArena->EndDuel();
		return 0;
	}

	switch (info->state)
	{
		case 0:
			{
				chA->SetArena(pArena);
				chB->SetArena(pArena);

				int32_t count = quest::CQuestManager::Instance().GetEventFlag("arena_potion_limit_count");

				if (count > 10000)
				{
					chA->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ������ �����ϴ�."));
					chB->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ������ �����ϴ�."));
				}
				else
				{
					chA->SetPotionLimit(count);
					chB->SetPotionLimit(count);

					chA->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("������ %d �� ���� ��� �����մϴ�."), chA->GetPotionLimit());
					chB->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("������ %d �� ���� ��� �����մϴ�."), chB->GetPotionLimit());
				}
				chA->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("10�ʵ� ����� ���۵˴ϴ�."));
				chB->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("10�ʵ� ����� ���۵˴ϴ�."));

				info->state++;
				return PASSES_PER_SEC(10);
			}
			break;

		case 1:
			{
				chA->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("����� ���۵Ǿ����ϴ�."));
				chB->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("����� ���۵Ǿ����ϴ�."));

				SPacketGCDuelStart duelStart;
				duelStart.header = HEADER_GC_DUEL_START;
				duelStart.wSize = sizeof(SPacketGCDuelStart) + 4;

				uint32_t dwOppList[8]; // �ִ� ��Ƽ�� 8�� �̹Ƿ�..

				dwOppList[0] = (uint32_t)chB->GetVID();
				TEMP_BUFFER buf;

				buf.write(&duelStart, sizeof(SPacketGCDuelStart));
				buf.write(&dwOppList[0], 4);
				chA->GetDesc()->Packet(buf.read_peek(), buf.size());


				dwOppList[0] = (uint32_t)chA->GetVID();
				TEMP_BUFFER buf2;

				buf2.write(&duelStart, sizeof(SPacketGCDuelStart));
				buf2.write(&dwOppList[0], 4);
				chB->GetDesc()->Packet(buf2.read_peek(), buf2.size());

				return 0;
			}
			break;

		case 2:
			{
				pArena->EndDuel();
				return 0;
			}
			break;

		case 3:
			{
				chA->Show(chA->GetMapIndex(), pArena->GetStartPointA().x * 100, pArena->GetStartPointA().y * 100);
				chB->Show(chB->GetMapIndex(), pArena->GetStartPointB().x * 100, pArena->GetStartPointB().y * 100);

				chA->GetDesc()->SetPhase(PHASE_GAME);
				chA->StartRecoveryEvent();
				chA->SetPosition(POS_STANDING);
				chA->PointChange(POINT_HP, chA->GetMaxHP() - chA->GetHP());
				chA->PointChange(POINT_SP, chA->GetMaxSP() - chA->GetSP());
				chA->ViewReencode();

				chB->GetDesc()->SetPhase(PHASE_GAME);
				chB->StartRecoveryEvent();
				chB->SetPosition(POS_STANDING);
				chB->PointChange(POINT_HP, chB->GetMaxHP() - chB->GetHP());
				chB->PointChange(POINT_SP, chB->GetMaxSP() - chB->GetSP());
				chB->ViewReencode();

				TEMP_BUFFER buf;
				TEMP_BUFFER buf2;
				uint32_t dwOppList[8]; // �ִ� ��Ƽ�� 8�� �̹Ƿ�..
				SPacketGCDuelStart duelStart;
				duelStart.header = HEADER_GC_DUEL_START;
				duelStart.wSize = sizeof(SPacketGCDuelStart) + 4;

				dwOppList[0] = static_cast<uint32_t>(chB->GetVID());
				buf.write(&duelStart, sizeof(SPacketGCDuelStart));
				buf.write(&dwOppList[0], 4);
				chA->GetDesc()->Packet(buf.read_peek(), buf.size());

				dwOppList[0] = static_cast<uint32_t>(chA->GetVID());
				buf2.write(&duelStart, sizeof(SPacketGCDuelStart));
				buf2.write(&dwOppList[0], 4);
				chB->GetDesc()->Packet(buf2.read_peek(), buf2.size());

				chA->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("����� ���۵Ǿ����ϴ�."));
				chB->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("����� ���۵Ǿ����ϴ�."));

				pArena->ClearEvent();

				return 0;
			}
			break;

		default:
			{
				chA->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("����� ������ ���Ͽ� ����� �����մϴ�."));
				chB->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("����� ������ ���Ͽ� ����� �����մϴ�."));

				sys_log(0, "ARENA: Something wrong in event func. info->state(%d)", info->state);

				pArena->EndDuel();

				return 0;
			}
	}
}

EVENTFUNC(duel_time_out)
{
	if (event == nullptr)
		return 0;

	if (event->info == nullptr)
		return 0;

	TArenaEventInfo* info = dynamic_cast<TArenaEventInfo*>(event->info);

	if ( info == nullptr )
	{
		sys_err( "duel_time_out> <Factor> Null pointer" );
		return 0;
	}

	CArena* pArena = info->pArena;

	if (pArena == nullptr)
	{
		sys_err("ARENA: Time out event error");
		return 0;
	}

	LPCHARACTER chA = pArena->GetPlayerA();
	LPCHARACTER chB = pArena->GetPlayerB();

	if (chA == nullptr || chB == nullptr)
	{
		if (chA != nullptr)
		{
			chA->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("��� ��밡 ����� ����� �����մϴ�."));
			sys_log(0, "ARENA: Oppernent is disappered. MyPID(%u) OppPID(%u)", pArena->GetPlayerAPID(), pArena->GetPlayerBPID());
		}

		if (chB != nullptr)
		{
			chB->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("��� ��밡 ����� ����� �����մϴ�."));
			sys_log(0, "ARENA: Oppernent is disappered. MyPID(%u) OppPID(%u)", pArena->GetPlayerBPID(), pArena->GetPlayerAPID());
		}

		pArena->EndDuel();
		return 0;
	}
	else
	{
		switch (info->state)
		{
			case 0:
			{
				chA->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("��� �ð� �ʰ��� ����� �ߴ��մϴ�."));
				chA->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("10�ʵ� ������ �̵��մϴ�."));

				chB->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("��� �ð� �ʰ��� ����� �ߴ��մϴ�."));
				chB->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("10�ʵ� ������ �̵��մϴ�."));

				SPacketGCDuelStart duelStart;
				duelStart.header = HEADER_GC_DUEL_START;
				duelStart.wSize = sizeof(SPacketGCDuelStart);
				if (chA->GetDesc())
					chA->GetDesc()->Packet(&duelStart, sizeof(SPacketGCDuelStart));
				if (chB->GetDesc())
					chB->GetDesc()->Packet(&duelStart, sizeof(SPacketGCDuelStart));

				info->state++;

				sys_log(0, "ARENA: Because of time over, duel is end. PIDA(%u) vs PIDB(%u)", pArena->GetPlayerAPID(), pArena->GetPlayerBPID());

				return PASSES_PER_SEC(10);
			} break;

			case 1:
				pArena->EndDuel();
				break;
		}
	}

	return 0; 
}

bool CArena::StartDuel(LPCHARACTER pCharFrom, LPCHARACTER pCharTo, int32_t nSetPoint, int32_t nMinute)
{
	this->m_dwPIDA = pCharFrom->GetPlayerID();
	this->m_dwPIDB = pCharTo->GetPlayerID();
	this->m_dwSetCount = nSetPoint;

	pCharFrom->WarpSet(GetStartPointA().x * 100, GetStartPointA().y * 100);
	pCharTo->WarpSet(GetStartPointB().x * 100, GetStartPointB().y * 100);

	if (m_pEvent != nullptr) {
		event_cancel(&m_pEvent);
	}

	TArenaEventInfo* info = AllocEventInfo<TArenaEventInfo>();

	info->pArena = this;
	info->state = 0;

	m_pEvent = event_create(ready_to_start_event, info, PASSES_PER_SEC(10));

	if (m_pTimeOutEvent != nullptr) {
		event_cancel(&m_pTimeOutEvent);
	}

	info = AllocEventInfo<TArenaEventInfo>();

	info->pArena = this;
	info->state = 0;

	m_pTimeOutEvent = event_create(duel_time_out, info, PASSES_PER_SEC(nMinute*60));

	pCharFrom->PointChange(POINT_HP, pCharFrom->GetMaxHP() - pCharFrom->GetHP());
	pCharFrom->PointChange(POINT_SP, pCharFrom->GetMaxSP() - pCharFrom->GetSP());

	pCharTo->PointChange(POINT_HP, pCharTo->GetMaxHP() - pCharTo->GetHP());
	pCharTo->PointChange(POINT_SP, pCharTo->GetMaxSP() - pCharTo->GetSP());

	sys_log(0, "ARENA: Start Duel with PID_A(%u) vs PID_B(%u)", GetPlayerAPID(), GetPlayerBPID());
	return true;
}

void CArenaManager::EndAllDuel()
{
	for (auto iter = m_mapArenaMap.begin(); iter != m_mapArenaMap.end(); ++iter)
	{
		CArenaMap *pArenaMap = iter->second;
		if (pArenaMap != nullptr)
			pArenaMap->EndAllDuel();
	}

	return;
}

void CArenaMap::EndAllDuel()
{
	for (auto iter = m_listArena.begin(); iter != m_listArena.end(); ++iter)
	{
		CArena *pArena = *iter;
		if (pArena != nullptr)
			pArena->EndDuel();
	}
}

void CArena::EndDuel()
{
	if (m_pEvent != nullptr) {
		event_cancel(&m_pEvent);
	}
	if (m_pTimeOutEvent != nullptr) {
		event_cancel(&m_pTimeOutEvent);
	}

	LPCHARACTER playerA = GetPlayerA();
	LPCHARACTER playerB = GetPlayerB();

	if (playerA != nullptr)
	{
		playerA->SetPKMode(PK_MODE_PEACE);
		playerA->StartRecoveryEvent();
		playerA->SetPosition(POS_STANDING);
		playerA->PointChange(POINT_HP, playerA->GetMaxHP() - playerA->GetHP());
		playerA->PointChange(POINT_SP, playerA->GetMaxSP() - playerA->GetSP());

		playerA->SetArena(nullptr);

		playerA->WarpSet(ARENA_RETURN_POINT_X(playerA->GetEmpire()), ARENA_RETURN_POINT_Y(playerA->GetEmpire()));
	}

	if (playerB != nullptr)
	{
		playerB->SetPKMode(PK_MODE_PEACE);
		playerB->StartRecoveryEvent();
		playerB->SetPosition(POS_STANDING);
		playerB->PointChange(POINT_HP, playerB->GetMaxHP() - playerB->GetHP());
		playerB->PointChange(POINT_SP, playerB->GetMaxSP() - playerB->GetSP());

		playerB->SetArena(nullptr);

		playerB->WarpSet(ARENA_RETURN_POINT_X(playerB->GetEmpire()), ARENA_RETURN_POINT_Y(playerB->GetEmpire()));
	}

	for (auto iter = m_mapObserver.begin(); iter != m_mapObserver.end(); ++iter)
	{
		LPCHARACTER pChar = CHARACTER_MANAGER::Instance().FindByPID(iter->first);
		if (pChar != nullptr)
		{
			pChar->WarpSet(ARENA_RETURN_POINT_X(pChar->GetEmpire()), ARENA_RETURN_POINT_Y(pChar->GetEmpire()));
		}
	}

	m_mapObserver.clear();

	sys_log(0, "ARENA: End Duel PID_A(%u) vs PID_B(%u)", GetPlayerAPID(), GetPlayerBPID());

	Clear();
}

void CArenaManager::GetDuelList(lua_State* L)
{
	int32_t index = 1;

	lua_newtable(L);

	for (auto iter = m_mapArenaMap.begin(); iter != m_mapArenaMap.end(); ++iter)
	{
		CArenaMap* pArenaMap = iter->second;
		if (pArenaMap != nullptr)
			index = pArenaMap->GetDuelList(L, index);
	}
}

int32_t CArenaMap::GetDuelList(lua_State* L, int32_t index)
{
	for (auto iter = m_listArena.begin(); iter != m_listArena.end(); ++iter)
	{
		CArena* pArena = *iter;

		if (pArena == nullptr) continue;

		if (pArena->IsEmpty() == false)
		{
			LPCHARACTER chA = pArena->GetPlayerA();
			LPCHARACTER chB = pArena->GetPlayerB();

			if (chA != nullptr && chB != nullptr)
			{
				lua_newtable(L);

				lua_pushstring(L, chA->GetName());
				lua_rawseti(L, -2, 1);

				lua_pushstring(L, chB->GetName());
				lua_rawseti(L, -2, 2);

				lua_pushnumber(L, m_dwMapIndex);
				lua_rawseti(L, -2, 3);

				lua_pushnumber(L, pArena->GetObserverPoint().x);
				lua_rawseti(L, -2, 4);

				lua_pushnumber(L, pArena->GetObserverPoint().y);
				lua_rawseti(L, -2, 5);

				lua_rawseti(L, -2, index++);
			}
		}
	}

	return index;
}

bool CArenaManager::CanAttack(LPCHARACTER pCharAttacker, LPCHARACTER pCharVictim)
{
	if (pCharAttacker == nullptr || pCharVictim == nullptr) 
		return false;

	if (pCharAttacker == pCharVictim) 
		return false;

	int32_t mapIndex = pCharAttacker->GetMapIndex();
	if (mapIndex != pCharVictim->GetMapIndex())
		return false;

	auto iter = m_mapArenaMap.find(mapIndex);

	if (iter == m_mapArenaMap.end())
		return false;

	CArenaMap* pArenaMap = static_cast<CArenaMap*>(iter->second);
	return pArenaMap->CanAttack(pCharAttacker, pCharVictim);
}

bool CArenaMap::CanAttack(LPCHARACTER pCharAttacker, LPCHARACTER pCharVictim)
{
	if (pCharAttacker == nullptr || pCharVictim == nullptr) 
		return false;

	uint32_t dwPIDA = pCharAttacker->GetPlayerID();
	uint32_t dwPIDB = pCharVictim->GetPlayerID();

	for (auto iter = m_listArena.begin(); iter != m_listArena.end(); ++iter)
	{
		CArena* pArena = *iter;
		if (pArena->CanAttack(dwPIDA, dwPIDB) == true)
		{
			return true;
		}
	}
	return false;
}

bool CArena::CanAttack(uint32_t dwPIDA, uint32_t dwPIDB)
{
	// 1:1 ���� �ٴ�� �� ��� ���� �ʿ�
	if (m_dwPIDA == dwPIDA && m_dwPIDB == dwPIDB)
		return true;

	if (m_dwPIDA == dwPIDB && m_dwPIDB == dwPIDA)
		return true;

	return false;
}

bool CArenaManager::OnDead(LPCHARACTER pCharKiller, LPCHARACTER pCharVictim)
{
	if (pCharKiller == nullptr || pCharVictim == nullptr) 
		return false;

	int32_t mapIndex = pCharKiller->GetMapIndex();
	if (mapIndex != pCharVictim->GetMapIndex()) 
		return false;

	auto iter = m_mapArenaMap.find(mapIndex);
	if (iter == m_mapArenaMap.end()) 
		return false;

	CArenaMap* pArenaMap = iter->second;
	return pArenaMap->OnDead(pCharKiller,  pCharVictim);
}

bool CArenaMap::OnDead(LPCHARACTER pCharKiller, LPCHARACTER pCharVictim)
{
	uint32_t dwPIDA = pCharKiller->GetPlayerID();
	uint32_t dwPIDB = pCharVictim->GetPlayerID();

	for (auto iter = m_listArena.begin(); iter != m_listArena.end(); ++iter)
	{
		CArena* pArena = *iter;

		if (pArena->IsMember(dwPIDA) == true && pArena->IsMember(dwPIDB) == true)
		{
			pArena->OnDead(dwPIDA, dwPIDB);
			return true;
		}
	}
	return false;
}

bool CArena::OnDead(uint32_t dwPIDA, uint32_t dwPIDB)
{
	bool restart = false;

	LPCHARACTER pCharA = GetPlayerA();
	LPCHARACTER pCharB = GetPlayerB();

	if (pCharA == nullptr && pCharB == nullptr)
	{
		// �Ѵ� ������ �������� ?!
		restart = false;
	}
	else if (pCharA == nullptr && pCharB != nullptr)
	{
		pCharB->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("���� ĳ������ ������ ���Ͽ� ����� �����մϴ�."));
		restart = false;
	}
	else if (pCharA != nullptr && pCharB == nullptr)
	{
		pCharA->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("���� ĳ������ ������ ���Ͽ� ����� �����մϴ�."));
		restart = false;
	}
	else if (pCharA != nullptr && pCharB != nullptr)
	{
		if (m_dwPIDA == dwPIDA)
		{
			m_dwSetPointOfA++;

			if (m_dwSetPointOfA >= m_dwSetCount)
			{
				pCharA->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("%s ���� ��ÿ��� �¸��Ͽ����ϴ�."), pCharA->GetName());
				pCharB->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("%s ���� ��ÿ��� �¸��Ͽ����ϴ�."), pCharA->GetName());

				sys_log(0, "ARENA: Duel is end. Winner %s(%u) Loser %s(%u)",
						pCharA->GetName(), GetPlayerAPID(), pCharB->GetName(), GetPlayerBPID());
			}
			else
			{
				restart = true;
				pCharA->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ���� �¸��Ͽ����ϴ�."), pCharA->GetName());
				pCharA->ChatPacket(CHAT_TYPE_NOTICE, "%s %u : %u %s", pCharA->GetName(), m_dwSetPointOfA, m_dwSetPointOfB, pCharB->GetName());

				pCharB->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ���� �¸��Ͽ����ϴ�."), pCharA->GetName());
				pCharB->ChatPacket(CHAT_TYPE_NOTICE, "%s %u : %u %s", pCharA->GetName(), m_dwSetPointOfA, m_dwSetPointOfB, pCharB->GetName());

				sys_log(0, "ARENA: %s(%u) won a round vs %s(%u)",
						pCharA->GetName(), GetPlayerAPID(), pCharB->GetName(), GetPlayerBPID());
			}
		}
		else if (m_dwPIDB == dwPIDA)
		{
			m_dwSetPointOfB++;
			if (m_dwSetPointOfB >= m_dwSetCount)
			{
				pCharA->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("%s ���� ��ÿ��� �¸��Ͽ����ϴ�."), pCharB->GetName());
				pCharB->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("%s ���� ��ÿ��� �¸��Ͽ����ϴ�."), pCharB->GetName());

				sys_log(0, "ARENA: Duel is end. Winner(%d) Loser(%d)", GetPlayerBPID(), GetPlayerAPID());
			}
			else
			{
				restart = true;
				pCharA->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ���� �¸��Ͽ����ϴ�."), pCharB->GetName());
				pCharA->ChatPacket(CHAT_TYPE_NOTICE, "%s %u : %u %s", pCharA->GetName(), m_dwSetPointOfA, m_dwSetPointOfB, pCharB->GetName());

				pCharB->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ���� �¸��Ͽ����ϴ�."), pCharB->GetName());
				pCharB->ChatPacket(CHAT_TYPE_NOTICE, "%s %u : %u %s", pCharA->GetName(), m_dwSetPointOfA, m_dwSetPointOfB, pCharB->GetName());

				sys_log(0, "ARENA : PID(%u) won a round. Opp(%u)", GetPlayerBPID(), GetPlayerAPID());
			}
		}
		else
		{
			// wtf
			sys_log(0, "ARENA : OnDead Error (%u, %u) (%u, %u)", m_dwPIDA, m_dwPIDB, dwPIDA, dwPIDB);
		}

		int32_t potion = quest::CQuestManager::Instance().GetEventFlag("arena_potion_limit_count");
		pCharA->SetPotionLimit(potion);
		pCharB->SetPotionLimit(potion);
	}

	if (restart == false)
	{
		if (pCharA != nullptr)
			pCharA->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("10�ʵ� ������ �ǵ��ư��ϴ�."));

		if (pCharB != nullptr)
			pCharB->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("10�ʵ� ������ �ǵ��ư��ϴ�."));

		if (m_pEvent != nullptr) {
			event_cancel(&m_pEvent);
		}

		TArenaEventInfo* info = AllocEventInfo<TArenaEventInfo>();

		info->pArena = this;
		info->state = 2;

		m_pEvent = event_create(ready_to_start_event, info, PASSES_PER_SEC(10));
	}
	else
	{
		if (pCharA != nullptr)
			pCharA->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("10�ʵ� ���� ���� �����մϴ�."));

		if (pCharB != nullptr)
			pCharB->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("10�ʵ� ���� ���� �����մϴ�."));

		if (m_pEvent != nullptr) {
			event_cancel(&m_pEvent);
		}

		TArenaEventInfo* info = AllocEventInfo<TArenaEventInfo>();

		info->pArena = this;
		info->state = 3;

		m_pEvent = event_create(ready_to_start_event, info, PASSES_PER_SEC(10));
	}

	return true;
}

bool CArenaManager::AddObserver(LPCHARACTER pChar, uint32_t mapIdx, uint16_t ObserverX, uint16_t ObserverY)
{
	auto iter = m_mapArenaMap.find(mapIdx);
	if (iter == m_mapArenaMap.end())
		return false;

	CArenaMap* pArenaMap = iter->second;
	return pArenaMap->AddObserver(pChar, ObserverX, ObserverY);
}

bool CArenaMap::AddObserver(LPCHARACTER pChar, uint16_t ObserverX, uint16_t ObserverY)
{
	for (auto iter = m_listArena.begin(); iter != m_listArena.end(); ++iter)
	{
		CArena* pArena = *iter;

		if (pArena->IsMyObserver(ObserverX, ObserverY))
		{
			pChar->SetArena(pArena);
			return pArena->AddObserver(pChar);
		}
	}

	return false;
}

bool CArena::IsMyObserver(uint16_t ObserverX, uint16_t ObserverY)
{
	return ((ObserverX == m_ObserverPoint.x) && (ObserverY == m_ObserverPoint.y));
}

bool CArena::AddObserver(LPCHARACTER pChar)
{
	uint32_t pid = pChar->GetPlayerID();

	m_mapObserver.insert(std::make_pair(pid, (LPCHARACTER)nullptr));

	pChar->SaveExitLocation();
	pChar->WarpSet(m_ObserverPoint.x * 100, m_ObserverPoint.y * 100);

	return true;
}

bool CArenaManager::IsArenaMap(uint32_t dwMapIndex)
{
	return m_mapArenaMap.find(dwMapIndex) != m_mapArenaMap.end();
}

MEMBER_IDENTITY CArenaManager::IsMember(uint32_t dwMapIndex, uint32_t PID)
{
	auto iter = m_mapArenaMap.find(dwMapIndex);

	if (iter != m_mapArenaMap.end())
	{
		CArenaMap* pArenaMap = iter->second;
		return pArenaMap->IsMember(PID);
	}

	return MEMBER_NO;
}

MEMBER_IDENTITY CArenaMap::IsMember(uint32_t PID)
{
	for (auto iter = m_listArena.begin(); iter != m_listArena.end(); ++iter)
	{
		CArena* pArena = *iter;

		if (pArena->IsObserver(PID)) 
			return MEMBER_OBSERVER;

		if (pArena->IsMember(PID)) 
			return MEMBER_DUELIST;
	}
	return MEMBER_NO;
}

bool CArena::IsObserver(uint32_t PID)
{
	auto iter = m_mapObserver.find(PID);

	return iter != m_mapObserver.end();
}

void CArena::OnDisconnect(uint32_t pid)
{
	if (m_dwPIDA == pid)
	{
		if (GetPlayerB() != nullptr)
			GetPlayerB()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ĳ���Ͱ� ������ �����Ͽ� ����� �����մϴ�."));

		sys_log(0, "ARENA : Duel is end because of Opp(%u) is disconnect. MyPID(%u)", GetPlayerAPID(), GetPlayerBPID());
		EndDuel();
	}
	else if (m_dwPIDB == pid)
	{
		if (GetPlayerA() != nullptr)
			GetPlayerA()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ĳ���Ͱ� ������ �����Ͽ� ����� �����մϴ�."));

		sys_log(0, "ARENA : Duel is end because of Opp(%u) is disconnect. MyPID(%u)", GetPlayerBPID(), GetPlayerAPID());
		EndDuel();
	}
}

void CArena::RemoveObserver(uint32_t pid)
{
	auto iter = m_mapObserver.find(pid);
	if (iter != m_mapObserver.end())
	{
		m_mapObserver.erase(iter);
	}
}

bool CArenaManager::EndDuel(uint32_t pid)
{
	for (auto iter = m_mapArenaMap.begin(); iter != m_mapArenaMap.end(); ++iter)
	{
		CArenaMap* pArenaMap = iter->second;
		if (pArenaMap->EndDuel(pid) == true)
			return true;
	}
	return false;
}

bool CArenaMap::EndDuel(uint32_t pid)
{
	for (auto iter = m_listArena.begin(); iter != m_listArena.end(); ++iter)
	{
		CArena* pArena = *iter;
		if (pArena->IsMember(pid) == true)
		{
			pArena->EndDuel();
			return true;
		}
	}
	return false;
}

bool CArenaManager::RegisterObserverPtr(LPCHARACTER pChar, uint32_t mapIdx, uint16_t ObserverX, uint16_t ObserverY)
{
	if (pChar == nullptr) 
		return false;

	auto iter = m_mapArenaMap.find(mapIdx);
	if (iter == m_mapArenaMap.end())
	{
		sys_log(0, "ARENA : Cannot find ArenaMap. %u %d %d", mapIdx, ObserverX, ObserverY);
		return false;
	}

	CArenaMap* pArenaMap = iter->second;
	return pArenaMap->RegisterObserverPtr(pChar, mapIdx, ObserverX, ObserverY);
}

bool CArenaMap::RegisterObserverPtr(LPCHARACTER pChar, uint32_t mapIdx, uint16_t ObserverX, uint16_t ObserverY)
{
	for (auto iter = m_listArena.begin(); iter != m_listArena.end(); ++iter)
	{
		CArena* pArena = *iter;

		if (pArena->IsMyObserver(ObserverX, ObserverY) == true)
		{
			return pArena->RegisterObserverPtr(pChar);
		}
	}

	return false;
}

bool CArena::RegisterObserverPtr(LPCHARACTER pChar)
{
	uint32_t pid = pChar->GetPlayerID();
	auto iter = m_mapObserver.find(pid);

	if (iter == m_mapObserver.end())
	{
		sys_log(0, "ARENA : not in ob list");
		return false;
	}

	m_mapObserver[pid] = pChar;
	return true;
}

// #ifdef ENABLE_NEWSTUFF
bool IsAllowedPotionOnPVP(uint32_t dwVnum)
{
	switch (dwVnum)
	{
		// blue potions
		case 27004:
		case 27005:
		case 27006:
		// auto blue potions
		case 39040:
		case 39041:
		case 39042:
		case 72727:
		case 72728:
		case 72729:
		case 72730:
			return true;
	}
	return false;
}

bool IsLimitedPotionOnPVP(uint32_t dwVnum)
{
	return IsLimitedPotion(dwVnum) && !IsAllowedPotionOnPVP(dwVnum);
}

bool IsLimitedPotion(uint32_t dwVnum)
{
	if (dwVnum >= 50801 && dwVnum <= 50826)
		return true;

	switch (dwVnum)
	{
		case 50020:
		case 50021:
		case 50022:
		case 50801:
		case 50802:
		case 50813:
		case 50814:
		case 50817:
		case 50818:
		case 50819:
		case 50820:
		case 50821:
		case 50822:
		case 50823:
		case 50824:
		case 50825:
		case 50826:
		case 71044:
		case 71055:
			return true;
	}
	return false;
}
// #endif

bool CArenaManager::IsLimitedItem(int32_t lMapIndex, uint32_t dwVnum)
{
	if (IsArenaMap(lMapIndex) && IsLimitedPotion(dwVnum))
		return true;

	return false;
}

