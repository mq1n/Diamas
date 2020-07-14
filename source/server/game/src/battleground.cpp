#include "stdafx.h"
#include "battleground.h"
#include "constants.h"
#include "config.h"
#include "quest_manager.h"
#include "start_position.h"
#include "buffer_manager.h"
#include "log.h"
#include "char.h"
#include "char_manager.h"
#include "desc.h"
#include "sectree_manager.h"
#include "battle.h"
#include "item.h"
#include "crc32.h"
#include "dungeon.h"

// -------------------------------------------------------------------------------------

struct FChat
{
	std::string m_stText;
	uint8_t m_nChatType;

	FChat(const std::string& stText, uint8_t nChatType = CHAT_TYPE_INFO) :
		m_stText(stText), m_nChatType(nChatType)
	{
	}

	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER pkChar = (LPCHARACTER) ent;
			if (pkChar->IsPC())
				pkChar->ChatPacket(m_nChatType, "%s", m_stText.c_str());
		}
	}
};

struct FExitAndGoTo
{
	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER pkChar = (LPCHARACTER) ent;
			if (pkChar->IsPC())
				pkChar->WarpSet(EMPIRE_START_X(pkChar->GetEmpire()), EMPIRE_START_Y(pkChar->GetEmpire()));
		}
	}
};

struct FPacket
{
	const void * m_pvData;
	int32_t m_cbSize;
	
	FPacket(const void * p, int32_t size) :
		m_pvData(p), m_cbSize(size)
	{
	}

	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER pkChar = (LPCHARACTER) ent;
			if (pkChar->IsPC() && pkChar->GetDesc())
				pkChar->GetDesc()->Packet(m_pvData, m_cbSize);
		}
	}
};

// -------------------------------------------------------------------------------------

EVENTINFO(minnion_spawn_event_info)
{
	CBattleground * This;

	minnion_spawn_event_info()
	{
		This = nullptr;
	}
};

EVENTFUNC(minnion_spawn_event)
{
	auto info = dynamic_cast<minnion_spawn_event_info*>(event->info);
	if (!info)
	{
		sys_err("minnion_spawn_event> <Factor> Null pointer");
		return 0;
	}

	auto nCurrWave = info->This->GetMinnionWaveIndex();
	auto arrCurrWaveCtx = gs_iBattlegroundMinnionWaves[nCurrWave];

	// sec - 	soldier, archer, commander
//	auto iCurWaveSpawnSec = arrCurrWaveCtx[0];

	if (nCurrWave < BATTLEGROUND_MAX_WAVE_COUNT)
		info->This->SetNextMinnionWaveIndex();

	// ...

	return 0;
}

// -------------------------------------------------------------------------------------

EVENTINFO(bg_managment_event_info)
{
	CBattleground * This;
	bool isStarted;

	bg_managment_event_info()
	{
		This = nullptr;
		isStarted = false;
	}
};

EVENTFUNC(bg_managment_event)
{
	auto info = dynamic_cast<bg_managment_event_info*>(event->info);
	if (!info)
	{
		sys_err("bg_managment_event> <Factor> Null pointer");
		return 0;
	}

	if (info->This)
	{
		if (info->This->IsStarted() == false)
		{
			auto dwPassedSecThenSetup = (get_dword_time() - info->This->GetSetupTime()) / 1000;
			int32_t nStartTimeRemainingSec = BATTLEGROUND_START_TIME_SEC - dwPassedSecThenSetup;

			if (nStartTimeRemainingSec <= 0)
			{
				info->This->Start();
				return PASSES_PER_SEC(1);
			}
			else
			{
				info->This->SendNotice("Battleground will start within %d seconds", nStartTimeRemainingSec);
				return PASSES_PER_SEC(10);
			}
		}
		else
		{
			auto dwPassedSecThenStart = (get_dword_time() - info->This->GetStartTime()) / 1000;

			info->This->SpawnMinnions(dwPassedSecThenStart);

			return PASSES_PER_SEC(5);
		}

		// battleground timerýnda oyuncularýn hepsi oyundamý kontrolü
		// eðer çýkmýþ oyuncu varsa 60sn süre taný ve oyuncuyu cezalandýr
		// eðer takýmlarýn birinde hiç oyuncu kalmamýþsa 60sn süreden sonra oyunu iptal et tüm oyuncularý cezalandýr
	}
	return PASSES_PER_SEC(5);
}

// -------------------------------------------------------------------------------------


CBattleground::CBattleground(int32_t nMapIndex, uint8_t nGameMode, uint8_t nGameType, uint8_t nQueueType) :
	m_dwBattlegroundVID(0), m_nMapIndex(nMapIndex), m_bStarted(false), m_pkMinnionSpawnEvent(nullptr),
	m_nMinnionWaveIndex(0), m_nParticipantLimit(0), m_pkBattlegroundManagmentEvent(nullptr), m_dwRoomID(0),
	m_pkDungeon(nullptr), m_dwStartTime(0)
{
	m_upTeamBlue.reset();
	m_upTeamRed.reset();
	m_vSpecatators.clear();
	m_strLeaderName.clear();
	m_pkLstAttenders.clear();
	
	m_dwSetupTimestamp = get_dword_time();

	m_pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(nMapIndex);

	GPOS pxBase;
	if (SECTREE_MANAGER::Instance().GetMapBasePositionByMapIndex(nMapIndex, pxBase))
		m_pkSectree = m_pkSectreeMap->Find(pxBase.x, pxBase.y);

	m_pkLstSpawnedMobs.clear();
}

void CBattleground::__CreateVID()
{
#if 0
	static int32_t s_crc = 172814;

	char crc_string[128+1];
	snprintf(crc_string, sizeof(crc_string), "BATTLEGROUND%p%d", this, ++s_crc);

	auto dwBattlegroundVID = GetCRC32(crc_string, strlen(crc_string));
	while (CBattlegroundManager::Instance().IsEventVID(dwBattlegroundVID))
	{
		if (dwBattlegroundVID == std::numeric_limits<uint32_t>::max() - 1)
			dwBattlegroundVID = 0x3169;

		++dwBattlegroundVID;
	}
	m_dwBattlegroundVID = dwBattlegroundVID;
#endif
	m_dwBattlegroundVID += 1;

	sys_log(0, "Battleground VID allocated: %u", m_dwBattlegroundVID);
}

bool CBattleground::Initialize()
{
	m_upTeamBlue = std::make_unique<CBattlegroundTeam>(BG_TEAM_BLUE, this, m_pkSectreeMap, m_nMapIndex);
	if (!m_upTeamBlue || !m_upTeamBlue.get())
	{
		sys_err("Battleground blue team object can not allocated!");
		return false;
	}
	m_upTeamRed = std::make_unique<CBattlegroundTeam>(BG_TEAM_RED, this, m_pkSectreeMap, m_nMapIndex);
	if (!m_upTeamRed || !m_upTeamRed.get())
	{
		sys_err("Battleground red team object can not allocated!");
		return false;
	}

	auto info1 = AllocEventInfo<bg_managment_event_info>();
	info1->This = this;
	m_pkBattlegroundManagmentEvent = event_create(bg_managment_event, info1, PASSES_PER_SEC(1));

	__CreateVID();
	return true;
}

void CBattleground::Destroy()
{
	if (IsStarted())
		Close();

	m_upTeamBlue.reset();
	m_upTeamRed.reset();

	if (m_pkBattlegroundManagmentEvent)
	{
		event_cancel(&m_pkBattlegroundManagmentEvent);
		m_pkBattlegroundManagmentEvent = nullptr;
	}	
	if (m_pkMinnionSpawnEvent)
	{
		event_cancel(&m_pkMinnionSpawnEvent);
		m_pkMinnionSpawnEvent = nullptr;
	}

	CDungeonManager::Instance().Destroy(m_pkDungeon->GetId());
}

void CBattleground::__SpawnRandomPos(LPCHARACTER pkChar)
{
	if (pkChar)
	{
		GPOS pos;
		for (;;)
		{
			pos.x = number(pkChar->GetX() - 2000, pkChar->GetX() + 2000);
			pos.y = number(pkChar->GetY() - 1000, pkChar->GetY() + 1000);
			if (SECTREE_MANAGER::Instance().IsMovablePosition(pkChar->GetMapIndex(), pos.x, pos.y))
			{
				pkChar->Show(pkChar->GetMapIndex(), pos.x, pos.y);
				pkChar->Stop();
				break;
			}
		}
	}
}

bool CBattleground::Enter(LPCHARACTER pkChar, uint8_t nTeamID)
{
	// ...
	if (!pkChar)
		return false;

	sys_log(0, "CBattleground::Enter %s(%u)", pkChar->GetName(), pkChar->GetPlayerID());

	pkChar->SetBattleground(this, nTeamID);
	// m_pkDungeon->Join(ch);

	if (nTeamID == BG_TEAM_RED)
		m_upTeamRed->Enter(pkChar->GetPlayerID());
	else
		m_upTeamBlue->Enter(pkChar->GetPlayerID());

	m_pkLstAttenders.emplace_back(pkChar);

#if 0

	if ((GetStatus() == DEATHMATCH_STARTED || GetStatus() == DEATHMATCH_FINISH) && !pkChar->IsGM())
	{
		sys_log(0, "Deathmatch Event info: event started or finished but non-GM char enter in event area %s", pkChar->GetName());
		return false;
	}

	//if (test_server){
		if (!pkChar->IsGM()){
			char szWinNotice[500];
			sprintf(szWinNotice, "Player: %s attended to Deathmatch event", pkChar->GetName());
			SendNotice(szWinNotice);
		}
	//}

	return EnterAttender(pkChar);
#endif

	return true;
}

#if 0
bool CBattleground::__EnterAttender(uint32_t dwPlayerID) const
{
	// ...

	if (GetStatus() != DEATHMATCH_OPEN && !pkChar->IsGM())
	{
		sys_err("Deathmatch Event info: event isn't open but non-GM char enter in event area %s", pkChar->GetName());
		return false;
	}

	if (!pkChar->IsGM()){
		DWORD pid = pkChar->GetPlayerID();
		m_map_char.insert(std::make_pair(pid, pid));

		SpawnRandomPos(pkChar);
	}

	return true;
}
#endif

bool CBattleground::EnterSpectator(LPCHARACTER pkChar) const
{
	// ...

#if 0
	if (!pkChar)
		return false;

	if (GetStatus() != DEATHMATCH_OPEN && !pkChar->IsGM())
	{
		sys_err("Deathmatch Event info: event isn't open but non-GM char enter in event area %s", pkChar->GetName());
		return false;
	}

	pkChar->SetQuestFlag("qDeathMatchPanel.is_observer", 1);

	if (pkChar->GetMapIndex() != 200)
		pkChar->WarpSet(95500, 12500, 200);

	//RemoveFromEvent(pkChar);
	DWORD pid = pkChar->GetPlayerID();
	m_map_miss.insert(std::make_pair(pid, pid));

	//if (test_server){
		char szWinNotice[500];
		sprintf(szWinNotice, "Player: %s attended as a spectator to Deathmatch event", pkChar->GetName());
		SendNotice(szWinNotice);
	//}
#endif

	return true;
}

bool CBattleground::__SwitchToSpectator(LPCHARACTER pkChar) const
{
	// ...
#if 0
	//Observer
	if (!x_pkChar)
		return true;

	if (m_map_char.size() <= 0)
		return true;

	if (!x_pkChar->IsPC())
		return true;

	DWORD dwPID = x_pkChar->GetPlayerID();
	m_map_char.erase(dwPID);
	m_map_miss.insert(std::make_pair(dwPID, dwPID));

	x_pkChar->SetObserverMode(true);

	sys_log(0, "%s switched to spectator. Remaining Player: %d", x_pkChar->GetName(), GetAttenderCount());

	if (GetAttenderCount() == 1)
		CheckEventStatus();
#endif

	return true;
}

void CBattleground::Logout(LPCHARACTER pkChar)
{
	// ...

#if 0
	//Observer
	if (!x_pkChar)
		return;

	if (m_map_char.size() <= 0)
		return;

	if (!x_pkChar->IsPC())
		return;

	DWORD dwPID = x_pkChar->GetPlayerID();
	m_map_char.erase(dwPID);

	if (m_map_char.size() == 1 && GetStatus() == DEATHMATCH_STARTED)
		CheckEventStatus();
#endif
}

void CBattleground::__ProcessPlayer(LPCHARACTER pkChar)
{
	if (pkChar->IsHorseRiding() || pkChar->GetMountVnum())
	{
		pkChar->RemoveAffect(AFFECT_MOUNT);
		pkChar->RemoveAffect(AFFECT_MOUNT_BONUS);

		pkChar->StopRiding();
		pkChar->MountVnum(0);
	}

	if (pkChar->GetHorse())
		pkChar->HorseSummon(false);

	if (pkChar->IsPolymorphed()) 
	{
		pkChar->SetPolymorph(0);
		pkChar->RemoveAffect(AFFECT_POLYMORPH);
	}
}

void CBattleground::SpawnFirstWaveMobs()
{
	LPCHARACTER pkMobProtege = nullptr;

	// Towers
	Spawn(BG_TEAM_BLUE, BG_MOB_TOWER, BG_LANE_LEFT, TOWER, gs_iBattlegroundSpawnPositions[20][3], gs_iBattlegroundSpawnPositions[20][4], 0, true, nullptr);
	Spawn(BG_TEAM_BLUE, BG_MOB_TOWER, BG_LANE_LEFT, TOWER, gs_iBattlegroundSpawnPositions[21][3], gs_iBattlegroundSpawnPositions[21][4], 0, true, nullptr);
	Spawn(BG_TEAM_BLUE, BG_MOB_TOWER, BG_LANE_MID, TOWER, gs_iBattlegroundSpawnPositions[22][3], gs_iBattlegroundSpawnPositions[22][4], 0, true, nullptr);
	Spawn(BG_TEAM_BLUE, BG_MOB_TOWER, BG_LANE_MID, TOWER, gs_iBattlegroundSpawnPositions[23][3], gs_iBattlegroundSpawnPositions[23][4], 0, true, nullptr);
	Spawn(BG_TEAM_BLUE, BG_MOB_TOWER, BG_LANE_RIGHT, TOWER, gs_iBattlegroundSpawnPositions[24][3], gs_iBattlegroundSpawnPositions[24][4], 0, true, nullptr);
	Spawn(BG_TEAM_BLUE, BG_MOB_TOWER, BG_LANE_RIGHT, TOWER, gs_iBattlegroundSpawnPositions[25][3], gs_iBattlegroundSpawnPositions[25][4], 0, true, nullptr);

	Spawn(BG_TEAM_RED, BG_MOB_TOWER, BG_LANE_LEFT, TOWER, gs_iBattlegroundSpawnPositions[26][3], gs_iBattlegroundSpawnPositions[26][4], 0, true, nullptr);
	Spawn(BG_TEAM_RED, BG_MOB_TOWER, BG_LANE_LEFT, TOWER, gs_iBattlegroundSpawnPositions[27][3], gs_iBattlegroundSpawnPositions[27][4], 0, true, nullptr);
	Spawn(BG_TEAM_RED, BG_MOB_TOWER, BG_LANE_MID, TOWER, gs_iBattlegroundSpawnPositions[28][3], gs_iBattlegroundSpawnPositions[28][4], 0, true, nullptr);
	Spawn(BG_TEAM_RED, BG_MOB_TOWER, BG_LANE_MID, TOWER, gs_iBattlegroundSpawnPositions[29][3], gs_iBattlegroundSpawnPositions[29][4], 0, true, nullptr);
	Spawn(BG_TEAM_RED, BG_MOB_TOWER, BG_LANE_RIGHT, TOWER, gs_iBattlegroundSpawnPositions[30][3], gs_iBattlegroundSpawnPositions[30][4], 0, true, nullptr);
	Spawn(BG_TEAM_RED, BG_MOB_TOWER, BG_LANE_RIGHT, TOWER, gs_iBattlegroundSpawnPositions[31][3], gs_iBattlegroundSpawnPositions[31][4], 0, true, nullptr);

	// Inhibitor & tower guards
	pkMobProtege = Spawn(BG_TEAM_BLUE, BG_MOB_INHIBITOR, BG_LANE_LEFT, INHIBITOR, gs_iBattlegroundSpawnPositions[33][3], gs_iBattlegroundSpawnPositions[33][4], 0, true, nullptr);
	Spawn(BG_TEAM_BLUE, BG_MOB_TOWER, BG_LANE_LEFT, TOWER, gs_iBattlegroundSpawnPositions[32][3], gs_iBattlegroundSpawnPositions[32][4], 0, true, pkMobProtege);

	pkMobProtege = Spawn(BG_TEAM_BLUE, BG_MOB_INHIBITOR, BG_LANE_RIGHT, INHIBITOR, gs_iBattlegroundSpawnPositions[35][3], gs_iBattlegroundSpawnPositions[35][4], 0, true, nullptr);
	Spawn(BG_TEAM_BLUE, BG_MOB_TOWER, BG_LANE_RIGHT, TOWER, gs_iBattlegroundSpawnPositions[34][3], gs_iBattlegroundSpawnPositions[34][4], 0, true, pkMobProtege);

	pkMobProtege = Spawn(BG_TEAM_RED, BG_MOB_INHIBITOR, BG_LANE_LEFT, INHIBITOR, gs_iBattlegroundSpawnPositions[37][3], gs_iBattlegroundSpawnPositions[37][4], 0, true, nullptr);
	Spawn(BG_TEAM_RED, BG_MOB_TOWER, BG_LANE_LEFT, TOWER, gs_iBattlegroundSpawnPositions[36][3], gs_iBattlegroundSpawnPositions[36][4], 0, true, pkMobProtege);

	pkMobProtege = Spawn(BG_TEAM_RED, BG_MOB_INHIBITOR, BG_LANE_RIGHT, INHIBITOR, gs_iBattlegroundSpawnPositions[39][3], gs_iBattlegroundSpawnPositions[39][4], 0, true, nullptr);
	Spawn(BG_TEAM_RED, BG_MOB_TOWER, BG_LANE_RIGHT, TOWER, gs_iBattlegroundSpawnPositions[38][3], gs_iBattlegroundSpawnPositions[38][4], 0, true, pkMobProtege);

	// Nexus & tower guards
	pkMobProtege = Spawn(BG_TEAM_BLUE, BG_MOB_NEXUS, BG_LANE_NONE, NEXUS_FINAL_BOSS, gs_iBattlegroundSpawnPositions[43][3], gs_iBattlegroundSpawnPositions[43][4], 0, true, nullptr);
	Spawn(BG_TEAM_BLUE, BG_MOB_TOWER, BG_LANE_NONE, TOWER, gs_iBattlegroundSpawnPositions[40][3], gs_iBattlegroundSpawnPositions[40][4], 0, true, pkMobProtege);
	Spawn(BG_TEAM_BLUE, BG_MOB_TOWER, BG_LANE_NONE, TOWER, gs_iBattlegroundSpawnPositions[41][3], gs_iBattlegroundSpawnPositions[41][4], 0, true, pkMobProtege);
	Spawn(BG_TEAM_BLUE, BG_MOB_TOWER, BG_LANE_NONE, TOWER, gs_iBattlegroundSpawnPositions[42][3], gs_iBattlegroundSpawnPositions[42][4], 0, true, pkMobProtege);

	pkMobProtege = Spawn(BG_TEAM_RED, BG_MOB_NEXUS, BG_LANE_NONE, NEXUS_FINAL_BOSS, gs_iBattlegroundSpawnPositions[47][3], gs_iBattlegroundSpawnPositions[47][4], 0, true, nullptr);
	Spawn(BG_TEAM_RED, BG_MOB_TOWER, BG_LANE_NONE, TOWER, gs_iBattlegroundSpawnPositions[44][3], gs_iBattlegroundSpawnPositions[44][4], 0, true, pkMobProtege);
	Spawn(BG_TEAM_RED, BG_MOB_TOWER, BG_LANE_NONE, TOWER, gs_iBattlegroundSpawnPositions[45][3], gs_iBattlegroundSpawnPositions[45][4], 0, true, pkMobProtege);
	Spawn(BG_TEAM_RED, BG_MOB_TOWER, BG_LANE_NONE, TOWER, gs_iBattlegroundSpawnPositions[46][3], gs_iBattlegroundSpawnPositions[46][4], 0, true, pkMobProtege);
}

void CBattleground::SpawnMinnions(uint32_t dwPassedSec)
{
	auto nCurrWave = GetMinnionWaveIndex();
	auto arrCurrWaveCtx = gs_iBattlegroundMinnionWaves[nCurrWave];

	// sec - 	soldier, archer, commander
	if (dwPassedSec < (uint32_t)arrCurrWaveCtx[0])
		return;

	// todo: ext loop: team, lane, mobtype
	
	/* 
	Spawn(BG_TEAM_RED, BG_MOB_MINNION, BG_LANE_LEFT, MINNION_SOLDIER, gs_iBattlegroundSpawnPositions[3][3], gs_iBattlegroundSpawnPositions[3][4], 0, true, nullptr);
	Spawn(BG_TEAM_RED, BG_MOB_MINNION, BG_LANE_RIGHT, MINNION_SOLDIER, gs_iBattlegroundSpawnPositions[5][3], gs_iBattlegroundSpawnPositions[5][4], 0, true, nullptr);

	Spawn(BG_TEAM_BLUE, BG_MOB_MINNION, BG_LANE_LEFT, MINNION_SOLDIER, gs_iBattlegroundSpawnPositions[0][3], gs_iBattlegroundSpawnPositions[0][4], 0, true, nullptr);
	Spawn(BG_TEAM_BLUE, BG_MOB_MINNION, BG_LANE_RIGHT, MINNION_SOLDIER, gs_iBattlegroundSpawnPositions[2][3], gs_iBattlegroundSpawnPositions[2][4], 0, true, nullptr);
	*/

//#if 0
	for (auto i = 0; i < arrCurrWaveCtx[1]; ++i)
	{
		Spawn(BG_TEAM_BLUE, BG_MOB_MINNION, BG_LANE_LEFT, MINNION_SOLDIER, gs_iBattlegroundSpawnPositions[0][3], gs_iBattlegroundSpawnPositions[0][4], 0, true, nullptr);
		Spawn(BG_TEAM_BLUE, BG_MOB_MINNION, BG_LANE_MID, MINNION_SOLDIER, gs_iBattlegroundSpawnPositions[1][3], gs_iBattlegroundSpawnPositions[1][4], 0, true, nullptr);
		Spawn(BG_TEAM_BLUE, BG_MOB_MINNION, BG_LANE_RIGHT, MINNION_SOLDIER, gs_iBattlegroundSpawnPositions[2][3], gs_iBattlegroundSpawnPositions[2][4], 0, true, nullptr);

		Spawn(BG_TEAM_RED, BG_MOB_MINNION, BG_LANE_LEFT, MINNION_SOLDIER, gs_iBattlegroundSpawnPositions[3][3], gs_iBattlegroundSpawnPositions[3][4], 0, true, nullptr);
		Spawn(BG_TEAM_RED, BG_MOB_MINNION, BG_LANE_MID, MINNION_SOLDIER, gs_iBattlegroundSpawnPositions[4][3], gs_iBattlegroundSpawnPositions[4][4], 0, true, nullptr);
		Spawn(BG_TEAM_RED, BG_MOB_MINNION, BG_LANE_RIGHT, MINNION_SOLDIER, gs_iBattlegroundSpawnPositions[5][3], gs_iBattlegroundSpawnPositions[5][4], 0, true, nullptr);
	}
	for (auto i = 0; i < arrCurrWaveCtx[2]; ++i)
	{
		Spawn(BG_TEAM_BLUE, BG_MOB_MINNION, BG_LANE_LEFT, MINNION_ARCHER, gs_iBattlegroundSpawnPositions[0][3], gs_iBattlegroundSpawnPositions[0][4], 0, true, nullptr);
		Spawn(BG_TEAM_BLUE, BG_MOB_MINNION, BG_LANE_MID, MINNION_ARCHER, gs_iBattlegroundSpawnPositions[1][3], gs_iBattlegroundSpawnPositions[1][4], 0, true, nullptr);
		Spawn(BG_TEAM_BLUE, BG_MOB_MINNION, BG_LANE_RIGHT, MINNION_ARCHER, gs_iBattlegroundSpawnPositions[2][3], gs_iBattlegroundSpawnPositions[2][4], 0, true, nullptr);

		Spawn(BG_TEAM_RED, BG_MOB_MINNION, BG_LANE_LEFT, MINNION_ARCHER, gs_iBattlegroundSpawnPositions[3][3], gs_iBattlegroundSpawnPositions[3][4], 0, true, nullptr);
		Spawn(BG_TEAM_RED, BG_MOB_MINNION, BG_LANE_MID, MINNION_ARCHER, gs_iBattlegroundSpawnPositions[4][3], gs_iBattlegroundSpawnPositions[4][4], 0, true, nullptr);
		Spawn(BG_TEAM_RED, BG_MOB_MINNION, BG_LANE_RIGHT, MINNION_ARCHER, gs_iBattlegroundSpawnPositions[5][3], gs_iBattlegroundSpawnPositions[5][4], 0, true, nullptr);
	}
	for (auto i = 0; i < arrCurrWaveCtx[3]; ++i)
	{
		Spawn(BG_TEAM_BLUE, BG_MOB_MINNION, BG_LANE_LEFT, MINNION_COMMANDER, gs_iBattlegroundSpawnPositions[0][3], gs_iBattlegroundSpawnPositions[0][4], 0, true, nullptr);
		Spawn(BG_TEAM_BLUE, BG_MOB_MINNION, BG_LANE_MID, MINNION_COMMANDER, gs_iBattlegroundSpawnPositions[1][3], gs_iBattlegroundSpawnPositions[1][4], 0, true, nullptr);
		Spawn(BG_TEAM_BLUE, BG_MOB_MINNION, BG_LANE_RIGHT, MINNION_COMMANDER, gs_iBattlegroundSpawnPositions[2][3], gs_iBattlegroundSpawnPositions[2][4], 0, true, nullptr);

		Spawn(BG_TEAM_RED, BG_MOB_MINNION, BG_LANE_LEFT, MINNION_COMMANDER, gs_iBattlegroundSpawnPositions[3][3], gs_iBattlegroundSpawnPositions[3][4], 0, true, nullptr);
		Spawn(BG_TEAM_RED, BG_MOB_MINNION, BG_LANE_MID, MINNION_COMMANDER, gs_iBattlegroundSpawnPositions[4][3], gs_iBattlegroundSpawnPositions[4][4], 0, true, nullptr);
		Spawn(BG_TEAM_RED, BG_MOB_MINNION, BG_LANE_RIGHT, MINNION_COMMANDER, gs_iBattlegroundSpawnPositions[5][3], gs_iBattlegroundSpawnPositions[5][4], 0, true, nullptr);
	}
//#endif

//	for (const auto& pkChar : m_pkLstAttenders) // moved to: void CHARACTER::StateMove() (863)
//		pkChar->UpdateSectree();

	if (nCurrWave < BATTLEGROUND_MAX_WAVE_COUNT)
		SetNextMinnionWaveIndex();

	// ...
}

bool CBattleground::Start()
{
	// TODO
	// 90 sec wait
	// broadcast timer to clients
	// itemshop stuff
	// after than 90 sec start minnion spawn event

//	auto info1 = AllocEventInfo<minnion_spawn_event_info>();
//	info1->This = this;
//	m_pkMinnionSpawnEvent = event_create(minnion_spawn_event, info1, PASSES_PER_SEC(1));
	
	m_bStarted = true;
	m_dwStartTime = get_dword_time();

	SendNotice("Battleground started!");

	SpawnFirstWaveMobs();

//	for (const auto& pkChar : m_pkLstAttenders)
//		pkChar->UpdateSectree();

	return true;
}

void CBattleground::Close()
{
	
#if 0
	// Players
	itertype(m_map_char) iter = m_map_char.begin();

	LPCHARACTER pkChar = NULL;
	for (; iter != m_map_char.end(); ++iter)
	{
		LPCHARACTER pkChar = CHARACTER_MANAGER::Instance().FindByPID(iter->second);

		if (pkChar != NULL){
			if (pkChar->IsAffectFlag(AFF_STUN))
				pkChar->RemoveAffect(AFFECT_STUN);

			ResetScore(pkChar);
			pkChar->WarpSet(EMPIRE_START_X(pkChar->GetEmpire()), EMPIRE_START_Y(pkChar->GetEmpire()));
		}
	}

	m_map_char.clear();

	// Spectators
	itertype(m_map_miss) iter2 = m_map_miss.begin();

	for (; iter2 != m_map_miss.end(); ++iter2)
	{
		LPCHARACTER pkChar = CHARACTER_MANAGER::Instance().FindByPID(iter2->second);

		if (pkChar != NULL){
			if (pkChar->IsAffectFlag(AFF_STUN))
				pkChar->RemoveAffect(AFFECT_STUN);

			pkChar->SetObserverMode(false);
			ResetScore(pkChar);
			pkChar->WarpSet(EMPIRE_START_X(pkChar->GetEmpire()), EMPIRE_START_Y(pkChar->GetEmpire()));
		}
	}

	m_map_miss.clear();
	dwWinnerPID = 0;

	return true;
#endif

	m_bStarted = false;
}

bool CBattleground::IsStarted() const
{
	return m_bStarted;
}

bool CBattleground::IsAttender(uint32_t dwPlayerID) const
{
	if (m_upTeamBlue->IsAttender(dwPlayerID))
		return true;
	else if (m_upTeamRed->IsAttender(dwPlayerID))
		return true;
	return false;
}


uint32_t CBattleground::GetAttenderCount(uint8_t nTeamID) const
{
	switch (nTeamID)
	{
		case BG_TEAM_BLUE:
			return m_upTeamBlue->GetAttenderCount();
		case BG_TEAM_RED:
			return m_upTeamRed->GetAttenderCount();
		default:
			sys_err("Unknown team id %u", nTeamID);
			return 0;
	}
}

uint8_t CBattleground::GetAttenderTeamID(uint32_t dwPlayerID) const
{
	if (m_upTeamBlue->IsAttender(dwPlayerID))
		return BG_TEAM_BLUE;
	else if (m_upTeamRed->IsAttender(dwPlayerID))
		return BG_TEAM_RED;
	return BG_TEAM_NONE;
}

uint32_t CBattleground::GetSpectatorCount() const
{
	return m_vSpecatators.size();
}

LPCHARACTER	CBattleground::GetOurNexus(uint8_t nTeamID) const
{
	switch (nTeamID)
	{
		case BG_TEAM_RED:
			return m_upTeamRed->GetNexus();
		case BG_TEAM_BLUE:
			return m_upTeamBlue->GetNexus();
		default:
			return nullptr;
	}
}
LPCHARACTER	CBattleground::GetEnemyNexus(uint8_t nTeamID) const
{
	switch (nTeamID)
	{
		case BG_TEAM_RED:
			return m_upTeamBlue->GetNexus();
		case BG_TEAM_BLUE:
			return m_upTeamRed->GetNexus();
		default:
			return nullptr;
	}
}

uint8_t CBattleground::GetBattlegroundMobTeamID(uint32_t dwVID) const
{
	if (m_upTeamBlue->IsOwnMinnion(dwVID))
		return BG_TEAM_BLUE;

	if (m_upTeamRed->IsOwnMinnion(dwVID))
		return BG_TEAM_RED;

	return BG_TEAM_NONE;
}
uint8_t CBattleground::GetBattlegroundAttenderTeamID(uint32_t dwPlayerID) const
{
	if (m_upTeamBlue->IsAttender(dwPlayerID))
		return BG_TEAM_BLUE;

	if (m_upTeamRed->IsAttender(dwPlayerID))
		return BG_TEAM_RED;

	return BG_TEAM_NONE;
}


void CBattleground::SendNotice(const char* c_szFormat, ...)
{
	char szChatBuf[CHAT_MAX_LEN + 1];
	va_list vaArgs;

	va_start(vaArgs, c_szFormat);
	vsnprintf(szChatBuf, sizeof(szChatBuf), c_szFormat, vaArgs);
	va_end(vaArgs);

	FChat f(szChatBuf, CHAT_TYPE_NOTICE);
	GetMapSectree()->for_each(f);
}
void CBattleground::ExitAllMembers()
{
	FExitAndGoTo f;
	GetMapSectree()->for_each(f);
}
void CBattleground::SendPacket(const void* data, int32_t size)
{
	FPacket f(data, size);
	GetMapSectree()->for_each(f);
}
void CBattleground::GoToHome(LPCHARACTER pkChar)
{
	if (pkChar)
		pkChar->WarpSet(EMPIRE_START_X(pkChar->GetEmpire()), EMPIRE_START_Y(pkChar->GetEmpire()));
}
LPCHARACTER CBattleground::Spawn(uint8_t nTeamID, uint8_t nMobType, uint8_t nLaneID, uint32_t dwVnum, int32_t iX, int32_t iY, int32_t iDir, bool bSpawnMotion, LPCHARACTER pkProtegeMob)
{
	LPCHARACTER pkMob = nullptr;

	switch (nTeamID)
	{
		case BG_TEAM_BLUE:
			pkMob = m_upTeamBlue->Spawn(nMobType, nLaneID, dwVnum, iX, iY, iDir, bSpawnMotion, true);
			break;
		case BG_TEAM_RED:
			pkMob = m_upTeamRed->Spawn(nMobType, nLaneID, dwVnum, iX, iY, iDir, bSpawnMotion, true);
			break;			
		default:
			sys_err("Unknown team id: %u", nTeamID);
			return nullptr;
	}

	if (g_bIsTestServer)
		sys_log(0, "Team: %u Lane: %u Mob: %u(%u) spawned -> %d.%d", nTeamID, nLaneID, dwVnum, nMobType, iX, iY);
	
	if (pkMob)
	{
		pkMob->SetBattleground(this, nTeamID);
		if (pkProtegeMob)
			pkMob->SetBattlegroundProtege(pkProtegeMob);

		pkMob->StartStateMachine();

		if (nMobType == BG_MOB_MINNION)
		{
			if (nLaneID == BG_LANE_LEFT)
			{
				if (nTeamID == BG_TEAM_RED)
				{
					pkMob->RegisterBattlegroundMovingWay(1, 180, 920);
					pkMob->RegisterBattlegroundMovingWay(2, 90, 830);
					pkMob->RegisterBattlegroundMovingWay(3, 90, 140);
				}
				else
				{		
					pkMob->RegisterBattlegroundMovingWay(1, 870, 90);
					pkMob->RegisterBattlegroundMovingWay(2, 920, 180);
					pkMob->RegisterBattlegroundMovingWay(3, 920, 870);
				}
			}
			else if (nLaneID == BG_LANE_RIGHT)
			{
				if (nTeamID == BG_TEAM_RED)
				{
					pkMob->RegisterBattlegroundMovingWay(1, 920, 180);
					pkMob->RegisterBattlegroundMovingWay(2, 850, 90);
					pkMob->RegisterBattlegroundMovingWay(3, 140, 90);
				}
				else
				{
					pkMob->RegisterBattlegroundMovingWay(1, 90, 850);
					pkMob->RegisterBattlegroundMovingWay(2, 210, 920);
					pkMob->RegisterBattlegroundMovingWay(3, 870, 920);				
				}
			}
			else // mid lane
			{
				if (nTeamID == BG_TEAM_RED)
				{
					pkMob->RegisterBattlegroundMovingWay(1, 130, 130);
				}
				else
				{
					pkMob->RegisterBattlegroundMovingWay(1, 880, 880);
				}
			}

			//pkMob->scale

			LPCHARACTER pkNexus = nullptr;
			if (nTeamID == BG_TEAM_RED)
				pkNexus = m_upTeamBlue->GetNexus();
			else
				pkNexus = m_upTeamRed->GetNexus();

			if (pkNexus)
			{
				pkMob->SetEnemyNexus(pkNexus);
				pkMob->SetVictim(pkNexus);
			}
			else
			{
				sys_err("Nexus can NOT target'd for: %u", (uint32_t)pkMob->GetVID());
			}

			pkMob->MoveBattlegroundMinnion();
		}

		if (nMobType == BG_MOB_TOWER || nMobType == BG_MOB_INHIBITOR || nMobType == BG_MOB_NEXUS)
		{
			pkMob->SetNoMove();
			pkMob->SetAggressive();
		}

		if (m_pkSectree)
		{
			m_pkSectree->InsertEntity(pkMob);
		}
		
		m_pkLstSpawnedMobs.emplace_back(pkMob);
	}
	return pkMob;
}

void CBattleground::SetNextMinnionWaveIndex()
{
	if (std::numeric_limits<uint8_t>::max() != m_nMinnionWaveIndex)
		++m_nMinnionWaveIndex;
}

uint8_t CBattleground::GetMinnionWaveIndex() const
{
	return m_nMinnionWaveIndex;
}

uint8_t CBattleground::GetGameMode() const
{
	return m_nGameMode;
}
uint8_t CBattleground::GetGameType() const
{
	return m_nGameType;
}
uint8_t	CBattleground::GetQueueType() const
{
	return m_nQueueType;
}
uint32_t CBattleground::GetSetupTime() const
{
	return m_dwSetupTimestamp;
}
uint32_t CBattleground::GetStartTime() const
{
	return m_dwStartTime;
}
uint32_t CBattleground::GetVID() const
{
	return m_dwBattlegroundVID;
}
int32_t CBattleground::GetMapIndex() const
{
	return m_nMapIndex;
}

void CBattleground::SetLeaderName(const std::string& strName)
{
	m_strLeaderName = strName;
}
std::string CBattleground::GetLeaderName() const
{
	return m_strLeaderName;
}

void CBattleground::SetParticipantCount(uint8_t nParticipantCount)
{
	m_nParticipantCount = nParticipantCount;
}
uint8_t CBattleground::GetParticipantCount() const
{
	return m_nParticipantCount;
}

void CBattleground::SetParticipantLimit(uint8_t nParticipantLimit)
{
	m_nParticipantLimit = nParticipantLimit;
}
uint8_t CBattleground::GetParticipantLimit() const
{
	return m_nParticipantLimit;
}

void CBattleground::SetRoomID(uint32_t dwRoomID)
{
	m_dwRoomID = dwRoomID;
}
uint32_t CBattleground::GetRoomID() const
{
	return m_dwRoomID;
}

void CBattleground::SetDungeon(LPDUNGEON pkDungeon)
{
	m_pkDungeon = pkDungeon;
}
LPDUNGEON CBattleground::GetDungeon() const
{
	return m_pkDungeon;
}

LPSECTREE_MAP &	CBattleground::GetMapSectree()
{
	return m_pkSectreeMap;	
}
GPOS CBattleground::NormalizePosition(GPOS oldPos)
{
	auto newPos = GPOS(oldPos.x, oldPos.y);
	if (m_pkSectreeMap)
	{
		newPos = GPOS(m_pkSectreeMap->m_setting.iBaseX + oldPos.x * 100, m_pkSectreeMap->m_setting.iBaseY + oldPos.y * 100);
		return newPos;
	}	
	return newPos;
}
