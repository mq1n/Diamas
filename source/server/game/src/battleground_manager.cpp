#include "stdafx.h"
#include "battleground.h"
#include "constants.h"
#include "config.h"
#include "quest_manager.h"
#include "start_position.h"
#include "packet.h"
#include "buffer_manager.h"
#include "log.h"
#include "char.h"
#include "char_manager.h"
#include "desc.h"
#include "sectree_manager.h"
#include "battle.h"
#include "item.h"
#include "dungeon.h"

EVENTINFO(bg_event_timer_info)
{
	BYTE phase;
	DWORD time;

	bg_event_timer_info() :
		phase(1), time(0)
	{
	}
};

EVENTFUNC(bg_event_timer_worker)
{
	auto info = dynamic_cast<bg_event_timer_info*>(event->info);
	if (!info)
	{
		sys_err("bg_event_timer_worker> <Factor> Null pointer");
		return 0;
	}

	CBattlegroundManager::Instance().OnEventTimerTick();
	return PASSES_PER_SEC(60);
}

// -------------------------------------------------------------------------------------

EVENTINFO(bg_queue_event_info)
{
	BYTE phase;
	DWORD time;

	bg_queue_event_info() :
		phase(1), time(0)
	{
	}
};

EVENTFUNC(bg_queue_event)
{
	auto info = dynamic_cast<bg_queue_event_info*>(event->info);
	if (!info)
	{
		sys_err("bg_queue_event> <Factor> Null pointer");
		return 0;
	}

	CBattlegroundManager::Instance().OnQueueTick();
	return PASSES_PER_SEC(2);
}

// -------------------------------------------------------------------------------------

EVENTINFO(bg_room_timeout_checker_info)
{
	BYTE phase;
	DWORD time;

	bg_room_timeout_checker_info() :
		phase(1), time(0)
	{
	}
};

EVENTFUNC(bg_room_timeout_checker)
{
	auto info = dynamic_cast<bg_room_timeout_checker_info*>(event->info);
	if (!info)
	{
		sys_err("bg_room_timeout_checker> <Factor> Null pointer");
		return 0;
	}
	CBattlegroundManager::Instance().CheckRoomTimeouts();
	return PASSES_PER_SEC(30);
}

// -------------------------------------------------------------------------------------

CBattlegroundManager::CBattlegroundManager() :
	m_pkBattlegroundQueueWorker(nullptr), m_pkBattlegroundEventTimer(nullptr), m_nBattlegroundJoinType(BATTLEGROUND_DEFAULT_JOIN_METHOD),
	m_dwLastRoomID(0), m_pkBattlegroundRoomTimeoutCheck(nullptr), m_bBlockCreateBattleground(false), m_bStarted(false)
{
	m_pkSetBattlegroundContainer.clear();
	m_upBattlegroundQueue.reset();
	m_pkSetBattlegroundRoomContainer.clear();
	m_arrBattlegroundRequiredItems = { 0, 0, 0 };
}


void CBattlegroundManager::Initialize()
{
#ifdef ENABLE_BATTLEGROUND_FULL_ACTIVE
	auto info1 = AllocEventInfo<bg_event_timer_info>();
	m_pkBattlegroundEventTimer = event_create(bg_event_timer_worker, info1, PASSES_PER_SEC(1));
#endif

	m_upBattlegroundQueue = std::make_unique<CBattlegroundQueue>();
	m_upBattlegroundQueue->Initialize();
	
	auto info2 = AllocEventInfo<bg_queue_event_info>();
	m_pkBattlegroundQueueWorker = event_create(bg_queue_event, info2, PASSES_PER_SEC(2));

	auto info3 = AllocEventInfo<bg_room_timeout_checker_info>();
	m_pkBattlegroundRoomTimeoutCheck = event_create(bg_room_timeout_checker, info3, PASSES_PER_SEC(3));

	SaveBattlegroundRequiredItems(27001, 27003, 27002);

//	BroadcastNotice("Battleground manager started!");

#ifdef ENABLE_BATTLEGROUND_FULL_ACTIVE
	m_bStarted = true;
#endif
}
void CBattlegroundManager::Destroy()
{
	m_bBlockCreateBattleground = true;

	if (m_pkBattlegroundEventTimer)
	{
		event_cancel(&m_pkBattlegroundEventTimer);
		m_pkBattlegroundEventTimer = nullptr;
	}

	if (m_pkBattlegroundQueueWorker)
	{
		event_cancel(&m_pkBattlegroundQueueWorker);
		m_pkBattlegroundQueueWorker = nullptr;
	}

	if (m_pkBattlegroundRoomTimeoutCheck)
	{
		event_cancel(&m_pkBattlegroundRoomTimeoutCheck);
		m_pkBattlegroundRoomTimeoutCheck = nullptr;		
	}

	if (m_upBattlegroundQueue && m_upBattlegroundQueue.get())
		m_upBattlegroundQueue->Destroy();

	if (m_pkSetBattlegroundContainer.size() > 0)
	{
		for (const auto& bg : m_pkSetBattlegroundContainer)
		{
			if (bg)
			{
				if (bg->IsStarted())
					bg->Close();

				DeleteBattleground(bg->GetVID());
			}
		}
		m_pkSetBattlegroundContainer.clear();
	}

	if (m_pkSetBattlegroundRoomContainer.size() > 0)
	{
		for (auto room : m_pkSetBattlegroundRoomContainer)
		{
			if (room)
			{
				room->DestroyRoom();

				delete room;
				room = nullptr;
			}
		}
		m_pkSetBattlegroundRoomContainer.clear();
	}
}

void CBattlegroundManager::SetStarted(bool bStatus)
{
	m_bStarted = bStatus;
}
bool CBattlegroundManager::IsStarted() const
{
	return m_bStarted;
}

SBattlegroundCtx* CBattlegroundManager::CreateBattleground(uint8_t nGameMode, uint8_t nGameType, uint8_t nQueueType)
{
	if (m_bBlockCreateBattleground)
		return nullptr;
	if (m_bStarted == false)
		return nullptr;

	auto pkDungeon = CDungeonManager::instance().Create(BATTLEGROUND_NORMAL_MAP_INDEX);
	if (!pkDungeon) 
	{
		sys_err("Battleground dungeon can not created!");
		return nullptr;
	}
	sys_log(0, "Battleground created. Normal: %ld Dungeon: %ld", BATTLEGROUND_NORMAL_MAP_INDEX, pkDungeon->GetMapIndex());
	
	auto pkBattleground = new CBattleground(pkDungeon->GetMapIndex(), nGameMode, nGameType, nQueueType);
	if (!pkBattleground)
	{
		sys_err("Battleground object can not allocated!");
		CDungeonManager::instance().Destroy(pkDungeon->GetId());
		return nullptr;
	}
	sys_log(0, "Battleground pointer created: %p", pkBattleground);

	if (!pkBattleground->Initialize())
	{
		sys_err("Battleground object can not initilizated!");

		delete pkBattleground;
		pkBattleground = nullptr;

		CDungeonManager::instance().Destroy(pkDungeon->GetId());
		return nullptr;
	}
	sys_log(0, "Battleground succesfully initialized! VID: %u", pkBattleground->GetVID());
	
	m_pkSetBattlegroundContainer.emplace(pkBattleground);

	auto bg = new SBattlegroundCtx();

	bg->nMapIndex = pkDungeon->GetMapIndex();
	bg->dwEventVID = pkBattleground->GetVID();
	bg->pkDungeon = pkDungeon;
	bg->pkBattleground = pkBattleground;

	return bg;
}

bool CBattlegroundManager::DeleteBattleground(uint32_t dwEventVID)
{
	auto bg = FindBattleground(dwEventVID);
	if (!bg)
	{
		sys_err("Target battleground %u not found", dwEventVID);
		return false;
	}

	m_pkSetBattlegroundContainer.erase(bg);

	bg->ExitAllMembers();
	bg->Destroy();

	delete bg;
	return true;
}

bool CBattlegroundManager::HasBattleground() const
{
	return (m_pkSetBattlegroundContainer.size() > 0);
}

void CBattlegroundManager::BlockCreateBattleground()
{
	m_bBlockCreateBattleground = true;
}

bool CBattlegroundManager::IsBattlegroundCreateBlocked() const
{
	return m_bBlockCreateBattleground;
}

bool CBattlegroundManager::IsLimitedItem(uint32_t dwVnum) const
{
	switch (dwVnum)
	{
		case 27875:
		case 27877:
		case 27878:
		case 50020:
		case 50021:
		case 50022:
		case 50083:
		case 50801:
		case 50802:
		case 50804:
		case 50813:
		case 50814:
		case 50816:
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
		case 71018:
		case 71044:
		case 71055:
			return true;
	}
	return false;
}

CBattleground* CBattlegroundManager::FindBattleground(uint32_t dwVID) const
{
	for (const auto& bg : m_pkSetBattlegroundContainer)
	{
		if (bg && bg->GetVID() == dwVID)
		{
			return bg;
		}
	}
	return nullptr;
}
CBattleground* CBattlegroundManager::FindBattlegroundByAttender(uint32_t dwPlayerID) const
{
	for (const auto& bg : m_pkSetBattlegroundContainer)
	{
		if (bg && bg->IsAttender(dwPlayerID))
		{
			return bg;
		}
	}
	return nullptr;
}
CBattleground* CBattlegroundManager::FindBattlegroundByMapIndex(int32_t nMapIndex) const
{
	for (const auto& bg : m_pkSetBattlegroundContainer)
	{
		if (bg && bg->GetMapIndex() == nMapIndex)
		{
			return bg;
		}
	}
	return nullptr;
}

bool CBattlegroundManager::StartEvent(uint32_t dwVID) const
{
	sys_log(0, "Start request handled for battleground %u", dwVID);

	auto bg = FindBattleground(dwVID);
	if (!bg)
	{
		sys_err("Target battleground %u not found", dwVID);
		return false;
	}
	return bg->Start();
}

bool CBattlegroundManager::CloseEvent(uint32_t dwVID) const
{
	sys_log(0, "Close request handled for battleground %u", dwVID);

	auto bg = FindBattleground(dwVID);
	if (!bg)
	{
		sys_err("Target battleground %u not found", dwVID);
		return false;
	}
	bg->Close();
	return true;
}


bool CBattlegroundManager::IsEventVID(uint32_t dwVID) const
{
	auto bg = FindBattleground(dwVID);
	if (!bg)
		return false;
	return true;
}

bool CBattlegroundManager::IsEventStarted(uint32_t dwVID) const
{
	auto bg = FindBattleground(dwVID);
	if (!bg)
	{
		sys_err("Target battleground %u not found", dwVID);
		return false;
	}
	return bg->IsStarted();
}

bool CBattlegroundManager::IsEventMap(int32_t nMapIndex) const
{
	if (nMapIndex == BATTLEGROUND_NORMAL_MAP_INDEX)
		return true;
	
	auto lMinIndex = (long)(BATTLEGROUND_NORMAL_MAP_INDEX) * 10000;
	auto lMaxIndex = (long)(BATTLEGROUND_NORMAL_MAP_INDEX) * 10000 + 10000;
	if (nMapIndex >= lMinIndex && nMapIndex <= lMaxIndex)
		return true;

#if 0
	for (const auto& bg : m_pkSetBattlegroundContainer)
	{
		if (bg && bg->GetMapIndex() == dwMapIndex)
		{
			return true;
		}
	}
#endif
	return false;
}


void CBattlegroundManager::OnQueueTick()
{
	m_upBattlegroundQueue->ProcessQueue();
}

void CBattlegroundManager::OnEventTimerTick()
{
#ifndef ENABLE_BATTLEGROUND_FULL_ACTIVE
	auto t = time(nullptr);
	auto datetime = localtime(&t);

	switch (datetime->tm_wday)
	{
		case 0: // Sunday
		case 6: // Saturday
			break;
		default:
			return;
	}

	bool bInDateRange = false;
	switch (datetime->tm_hour)
	{
		case 22:
		case 23:
		case 00:
			bInDateRange = true;
		default:
			break;
	}

	if (bInDateRange == true && IsStarted() == false)
		SetStarted(true);
	else if (bInDateRange == false && IsStarted() == true)
		SetStarted(false);
#endif
}


void CBattlegroundManager::SearchBattleground(LPCHARACTER ch, uint8_t nGameMode, uint8_t nGameType, uint8_t nQueueType)
{
	if (!ch)
		return;
	
	if (m_pkSetBattlegroundRoomContainer.empty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Battleground odasi bulunamadi");
		return;
	}

	bool bSentAnyRoom = false;
	for (const auto& pkCurrRoom : m_pkSetBattlegroundRoomContainer)
	{
		if (pkCurrRoom)
		{
			if (pkCurrRoom->GetGameMode() == nGameMode && pkCurrRoom->GetGameType() == nGameType &&
				pkCurrRoom->GetQueueType() == nQueueType && pkCurrRoom->IsInGame() == false)
			{
				if (ch->GetDesc())
				{
					bSentAnyRoom = true;

					ch->ChatPacket(CHAT_TYPE_COMMAND, "bg_search_ret %u %u %u %u %s %u %u",
						pkCurrRoom->GetRoomID(), pkCurrRoom->GetGameMode(), pkCurrRoom->GetGameType(), pkCurrRoom->GetQueueType(),
						pkCurrRoom->GetLeaderName().c_str(), pkCurrRoom->GetAttenderCount(), pkCurrRoom->GetRoomLimit()
					);
				}
			}
		}
	}

	if (bSentAnyRoom == false)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Aradiginiz ozelliklere uygun battleground odasi bulunamadi");
	}
}

void CBattlegroundManager::SetBattlegroundJoinType(uint8_t nJoinType)
{
	m_nBattlegroundJoinType = nJoinType;
}
uint8_t CBattlegroundManager::GetBattlegroundJoinType() const
{
	return m_nBattlegroundJoinType;
}

CBattlegroundRoom* CBattlegroundManager::FindBattlegroundRoom(uint32_t dwRoomID) const
{
	for (const auto& room : m_pkSetBattlegroundRoomContainer)
	{
		if (room->GetRoomID() == dwRoomID)
		{
			return room;
		}
	}
	return nullptr;
}
CBattlegroundRoom* CBattlegroundManager::FindBattlegroundRoomByLeader(uint32_t dwLeaderPID) const
{
	for (const auto& room : m_pkSetBattlegroundRoomContainer)
	{
		if (room->GetLeaderPlayerID() == dwLeaderPID)
		{
			return room;
		}
	}
	return nullptr;
}
CBattlegroundRoom* CBattlegroundManager::FindBattlegroundRoomByAttender(uint32_t dwPlayerID) const
{
	for (const auto& room : m_pkSetBattlegroundRoomContainer)
	{
		if (room->IsAttender(dwPlayerID))
		{
			return room;
		}
	}
	return nullptr;
}

void CBattlegroundManager::SaveBattlegroundRequiredItems(uint32_t dwReqItem1Vnum, uint32_t dwReqItem2Vnum, uint32_t dwReqItem3Vnum)
{
	m_arrBattlegroundRequiredItems = { dwReqItem1Vnum, dwReqItem2Vnum, dwReqItem3Vnum };
}
std::array <uint32_t, 3> CBattlegroundManager::GetBattlegroundRequiredItems() const
{
	return m_arrBattlegroundRequiredItems;
}


uint32_t CBattlegroundManager::CreateBattlegroundRoom(LPCHARACTER pkLeaderChar, uint8_t nGameMode, uint8_t nGameType, uint8_t nQueueType)
{
	if (!pkLeaderChar)
		return 0;

	if (FindBattlegroundByAttender(pkLeaderChar->GetPlayerID()))
		return 0;
	if (FindBattlegroundRoomByAttender(pkLeaderChar->GetPlayerID()))
		return 0;

	auto pRoom = new CBattlegroundRoom(nGameMode, nGameType, nQueueType);
	if (!pRoom)
	{
		sys_err("Battleground room object can not allocated!");
		return 0;
	}
	pRoom->SetRoomID(++m_dwLastRoomID);
	pRoom->SetLeaderPlayerID(pkLeaderChar->GetPlayerID());
	pRoom->JoinToRoom(pkLeaderChar->GetPlayerID(), number(BG_TEAM_BLUE, BG_TEAM_RED));

	m_pkSetBattlegroundRoomContainer.emplace(pRoom);

	return pRoom->GetRoomID();
}
void CBattlegroundManager::DeleteBattlegroundRoom(uint32_t dwRoomID)
{
	for (auto pkRoom : m_pkSetBattlegroundRoomContainer)
	{
		if (pkRoom->GetRoomID() == dwRoomID)
		{
			m_pkSetBattlegroundRoomContainer.erase(pkRoom);
		}
	}
}
void CBattlegroundManager::CheckRoomTimeouts()
{
	auto dwNowTimestamp = get_dword_time();
	
	for (const auto & room : m_pkSetBattlegroundRoomContainer)
	{
		if (dwNowTimestamp - room->GetSetupTime() > BATTLEGROUND_ROOM_TIMEOUT_MSEC)
		{
			room->DestroyRoom();
		}
	}
}

bool CBattlegroundManager::JoinBattlegroundQueue(uint32_t dwPlayerID, uint8_t nGameMode, uint8_t nGameType, uint8_t nQueueType) const
{
	// todo
	return true;
}
void CBattlegroundManager::LeaveFromBattlegroundQueue(uint32_t dwPlayerID)
{
	// todo
}


void CBattlegroundManager::WarpToBattlegroundMap(LPCHARACTER pkChar, uint32_t dwVID, uint8_t nTeamID)
{
	auto bg = FindBattleground(dwVID);
	if (!bg)
	{
		sys_err("Target battleground %u not found", dwVID);
		return;
	}

	int32_t x = 0, y = 0;

	switch (nTeamID)
	{
		case BG_TEAM_BLUE:
		{
			x = 60;
			y = 60;
		} break;
		case BG_TEAM_RED:
		{
			x = 950;
			y = 950;
		} break;
		default:
			sys_err("Unknown team id: %u for: %u ", nTeamID, pkChar->GetPlayerID());
			return;
	}

	auto pos = GPOS(x, y);
	auto newPos = bg->NormalizePosition(pos);

	if (g_bIsTestServer)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d, %d )", newPos.x, newPos.y, bg->GetMapIndex());
		sys_log(0, "Warp target: %d, %d, %d", newPos.x, newPos.y, bg->GetMapIndex());
	}

	pkChar->WarpSet(newPos.x, newPos.y, bg->GetMapIndex());
	pkChar->Stop();
}

uint16_t CBattlegroundManager::GetMobAttackRange(uint16_t wRaceNum)
{
	return 0; // TODO
}


void CBattlegroundManager::OnRevive(LPCHARACTER pkChar, int32_t nDeadTime)
{
		if (nDeadTime == 180)
		{
			pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ Àç½ÃÀÛ ÇÒ ¼ö ¾ø½À´Ï´Ù. (%dÃÊ ³²À½)"), nDeadTime - 179);
			return;
		}
		
		if (nDeadTime - 160 > 0)
		{
			pkChar->ChatPacket(CHAT_TYPE_INFO, "%d saniye sonra dirilebilirsin", nDeadTime - 160);
			return;
		}

		pkChar->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
		pkChar->GetDesc()->SetPhase(PHASE_GAME);
		pkChar->SetPosition(POS_STANDING);
		pkChar->StartRecoveryEvent();

		//CDeathMatchManager::instance().SpawnRandomPos(pkChar);
		pkChar->RestartAtSamePos();
		
//		pkChar->PointChange(POINT_HP, pkChar->GetMaxHP() - pkChar->GetHP());
//		pkChar->PointChange(POINT_SP, pkChar->GetMaxSP() - pkChar->GetSP());
		pkChar->PointChange(POINT_HP, 50 - pkChar->GetHP());
		
		pkChar->DeathPenalty(0);
		pkChar->ReviveInvisible(3);
}
void CBattlegroundManager::OnLogin(LPCHARACTER pkChar)
{
	if (pkChar && pkChar->IsPC() && pkChar->GetDesc())
	{
		if (!pkChar->GetBattleground())
		{
			auto pkBattleground = FindBattlegroundByAttender(pkChar->GetPlayerID());
			if (!pkBattleground)
			{
				sys_err("Unknown user detected in battleground! PID: %u", pkChar->GetPlayerID());
				return;
			}
			pkChar->SetBattleground(pkBattleground, pkBattleground->GetAttenderTeamID(pkChar->GetPlayerID()));
		}
	}


#if 0
	else if (ch->GetMapIndex() == DEATMATCH_MAP_INDEX)
	{
		// If disconnected from event send to empire
		if (ch->GetQuestFlag("qDeathMatchPanel.is_disconnected")){
			CDeathMatchManager::instance().ResetScore(ch);
			ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
		}

		// If player is spectator set observer flag
		if (ch->GetQuestFlag("qDeathMatchPanel.is_observer"))
			ch->SetObserverMode(true);

		// If player login without event is active send to empire
		if (!ch->IsObserverMode()){
			if (CDeathMatchManager::instance().Enter(ch) == false)
			{
				if (ch->GetGMLevel() == GM_PLAYER){
					ch->SetQuestFlag("qDeathMatchPanel.IsInLoginned", 0);
					ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
				}
			}
		}
	}


	if (ch->GetMapIndex() == DEATMATCH_MAP_INDEX){
		if (ch->GetParty()){
			LPPARTY pParty = ch->GetParty();

			if (pParty->GetMemberCount() == 2) {
				CPartyManager::instance().DeleteParty(pParty);
			}
			else {
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼¿¡¼­ ³ª°¡¼Ì½À´Ï´Ù."));
				pParty->Quit(ch->GetPlayerID());
			}
		}

		if (ch->GetMountVnum()){
			ch->RemoveAffect(AFFECT_MOUNT);
			ch->RemoveAffect(AFFECT_MOUNT_BONUS);
			if (ch->IsHorseRiding())
				ch->StopRiding();
		}

		if(!ch->IsObserverMode())
			CDeathMatchManager::instance().ResetScore(ch);
	}

	if (ch->GetMapIndex() != DEATMATCH_MAP_INDEX){
		if (ch->GetQuestFlag("qDeathMatchPanel.is_disconnected")){
			ch->SetQuestFlag("qDeathMatchPanel.is_disconnected", 0);
		}
	}	
#endif
}
void CBattlegroundManager::OnLogout(LPCHARACTER pkChar)
{
	// TODO geçici listeye al LPEVENT üzerinden listeyi kontrol et ekleme zamanýndan 60sn geçmiþse eventten atýp cezalandýr
	//oyundan çýkarsa karaktere cooldown cezasý
	//oyun modu competitive ise -rank puaný cezasý

	// varsa roomdan çýk
}
void CBattlegroundManager::OnAffectLoad(LPCHARACTER pkChar)
{
	// todo ilk x saniye yürümeyi engelle
#if 0
	if (GetMapIndex() == DEATMATCH_MAP_INDEX){
		if (!IsAffectFlag(AFF_STUN) && !IsObserverMode() && !IsGM() && CDeathMatchManager::instance().GetStatus() != DEATHMATCH_STARTED)
			AddAffect(AFFECT_STUN, 0, 0, AFF_STUN, INFINITE_AFFECT_DURATION, 0, 0, 0);
	}
	else{
		if (IsAffectFlag(AFF_STUN)){
			CAffect* pStunAffect = FindAffect(AFFECT_STUN);
			if (pStunAffect){
				if (pStunAffect->lDuration > 60 * 365 * 24){
					RemoveAffect(AFFECT_STUN);
				}
			}
		}
	}
#endif
}
void CBattlegroundManager::OnKill(LPCHARACTER pkKiller, LPCHARACTER pkTarget)
{
	#if 0
					// Old info
				int iDMKillerOldScore = CDeathMatchManager::instance().GetScore(pkKiller->GetPlayerID()); // Get killer now score
				int iDMKillerKillAverage = pkKiller->GetQuestFlag("qDeathMatchPanel.kill_average"); // Get killer kill average

				// Set new score to system
				/*                            If killer score in below zero set score in average */
				if (iDMKillerKillAverage > 0)
					pkKiller->SetQuestFlag("qDeathMatchPanel.kill_average", iDMKillerKillAverage - 1);
				else
					CDeathMatchManager::instance().SetScore(pkKiller->GetPlayerID(), iDMKillerOldScore + 1);

				// New Info
				int iDMKillerNewScore = CDeathMatchManager::instance().GetScore(pkKiller->GetPlayerID()); // Get killer new score

				// Send info to killer
				// CDeathMatchManager::instance().SendScoreInfo(pkKiller->GetPlayerID(), iDMKillerNewScore);
				// Send info to system
				sys_log(0, "<DeathMatch> Kill Info: Killer: %s K_Score: %d Victim: %s V_Score: %d", pkKiller->GetName(), iDMKillerNewScore, GetName(), CDeathMatchManager::instance().GetScore(GetPlayerID()));

				// Notice
				if (iDMKillerNewScore == 50 || iDMKillerNewScore == 50 * 2 || iDMKillerNewScore == 50 * 3 || iDMKillerNewScore == 50 * 4 || iDMKillerNewScore == 50 * 5 ||
					iDMKillerNewScore == 50 * 6 || iDMKillerNewScore == 50 * 7 || iDMKillerNewScore == 50 * 8 || iDMKillerNewScore == 50 * 9 || iDMKillerNewScore == 50 * 10)
					CDeathMatchManager::instance().NoticeScore(pkKiller->GetPlayerID(), iDMKillerNewScore);

				// Info to killer
				if (pkKiller->GetQuestFlag("qDeathMatchPanel.kill_average") != 0)
					pkKiller->ChatPacket(CHAT_TYPE_INFO, "%s i öldürdün ve 1 skor kazandýn. Skorun: -%d", GetName(), iDMKillerKillAverage - 1);
				else
					pkKiller->ChatPacket(CHAT_TYPE_INFO, "%s i öldürdün ve 1 skor kazandýn. Skorun: %d", GetName(), iDMKillerNewScore);

				// Victim Stuffs
				{
					int iDMVictimScore = CDeathMatchManager::instance().GetScore(GetPlayerID()); // Get Victim now score
					int iDMVictimKillCount = GetQuestFlag("qDeathMatchPanel.kill_count"); // Get Victim kill count
					int iDMVictimLastKiller = GetQuestFlag("qDeathMatchPanel.last_killer"); // Get Victim last killer

					// Check victim kill average
					int iDMVictimKillAverage = GetQuestFlag("qDeathMatchPanel.kill_average");
					if (iDMVictimScore <= 0){
						if (iDMVictimKillAverage >= 4){
							ChatPacket(CHAT_TYPE_INFO, "Skorun -5'e indigi icin eventten cikarildin");

							if (IsAffectFlag(AFF_STUN))
								RemoveAffect(AFFECT_STUN);

							CDeathMatchManager::instance().ResetScore(this);
							CDeathMatchManager::instance().SwitchToSpectator(this);
						}

						SetQuestFlag("qDeathMatchPanel.kill_average", iDMVictimKillAverage + 1);
						iDMVictimKillAverage += 1;
					}

					// Check victim dead count
					if (iDMVictimKillCount >= quest::CQuestManager::instance().GetEventFlag("deathmatch_kill_limit")){
						ChatPacket(CHAT_TYPE_INFO, "%d defa oldugun icin eventten cikarildin", iDMVictimKillCount + 1);

						if (IsAffectFlag(AFF_STUN))
							RemoveAffect(AFFECT_STUN);

						CDeathMatchManager::instance().ResetScore(this);
						CDeathMatchManager::instance().SwitchToSpectator(this);
					}

					// Set new scores to system
					SetQuestFlag("qDeathMatchPanel.kill_count", iDMVictimKillCount + 1);
					CDeathMatchManager::instance().SetScore(GetPlayerID(), iDMVictimScore - 1);

					// Last killer
					if (iDMVictimLastKiller && iDMVictimLastKiller == pkKiller->GetPlayerID()){
						if (GetQuestFlag("qDeathMatchPanel.same_killer") >= 4){
							ChatPacket(CHAT_TYPE_INFO, "%s isimli oyuncuya 5 defa ustuste oldugun icin eventten cikarildin", pkKiller->GetName());

							if (IsAffectFlag(AFF_STUN))
								RemoveAffect(AFFECT_STUN);

							CDeathMatchManager::instance().ResetScore(this);
							CDeathMatchManager::instance().SwitchToSpectator(this);
						}

						SetQuestFlag("qDeathMatchPanel.same_killer", GetQuestFlag("qDeathMatchPanel.same_killer") + 1);
					}
					else
						SetQuestFlag("qDeathMatchPanel.same_killer", 0);

					// Set as last killer
					SetQuestFlag("qDeathMatchPanel.last_killer", pkKiller->GetPlayerID());

					// Info
					if (iDMVictimKillAverage != 0)
						ChatPacket(CHAT_TYPE_INFO, "%s seni olduruldu ve 1 skor kaybettin. Skorun: -%d Olme miktarin: %d", pkKiller->GetName(), iDMVictimKillAverage, iDMVictimKillCount + 1);
					else
						ChatPacket(CHAT_TYPE_INFO, "%s seni olduruldu ve 1 skor kaybettin. Skorun: %d Olme miktarin: %d", pkKiller->GetName(), iDMVictimScore, iDMVictimKillCount + 1);


					// Extra bonus
					int iExtraBonus = 0;

					if (iDMVictimScore >= iDMKillerNewScore * 4)
						iExtraBonus += 5;
					else if (iDMVictimScore >= iDMKillerNewScore * 3)
						iExtraBonus += 3;
					else if (iDMVictimScore >= iDMKillerNewScore * 2)
						iExtraBonus += 2;

					if (iExtraBonus > 0 && iDMVictimScore > 5 && iDMKillerNewScore > 5){
						CDeathMatchManager::instance().SetScore(pkKiller->GetPlayerID(), iDMKillerNewScore + iExtraBonus);
						pkKiller->ChatPacket(CHAT_TYPE_INFO, "Senden guclu bir oyuncuyu oldurdun ve %d ekstra bonus kazandin.", iExtraBonus);
					}
				}

				// Check killer score with highscore
				CDeathMatchManager::instance().CheckHighScore(pkKiller->GetPlayerID());
	#endif
}
int32_t CBattlegroundManager::OnDamage(LPCHARACTER pkAttacker, LPCHARACTER pkVictim, int32_t nOldDamage)
{
	#if 0
			if (GetMapIndex() == DEATMATCH_MAP_INDEX){
				if (pAttacker->IsPC() && IsPC()){
					// Check event stat
					if (CDeathMatchManager::instance().GetStatus() != DEATHMATCH_STARTED /* && !pAttacker->IsGM() */){
						pAttacker->ChatPacket(CHAT_TYPE_INFO, "Deathmarch event do without an active you can not attack on this map");
						SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
						return false;
					}

					// Bonus damage stuffs
					int iDMKillerScore = CDeathMatchManager::instance().GetScore(pAttacker->GetPlayerID());
					int iDMVictimScore = CDeathMatchManager::instance().GetScore(GetPlayerID());

					if (iDMVictimScore >= iDMKillerScore * 4)
						dam += dam * 20 / 100;
					else if (iDMVictimScore >= iDMKillerScore * 3)
						dam += dam * 10 / 100;
					else if (iDMVictimScore >= iDMKillerScore * 2)
						dam += dam * 5 / 100;
				}
			}
	#endif

	return nOldDamage;
}
void CBattlegroundManager::OnSwitchIdle(LPCHARACTER pkChar)
{
	if (!pkChar)
		return;

	if (pkChar->HasBattlegroundMoveTarget())
	{
		sys_err("OnSwitchIdle");
		pkChar->MoveBattlegroundMinnion();
	}
}

