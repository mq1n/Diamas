#include "stdafx.h"
#include "Battleground.h"
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

CBattlegroundRoom::CBattlegroundRoom(uint8_t nGameMode, uint8_t nGameType, uint8_t nQueueType) :
	m_nGameMode(nGameMode), m_nGameType(nGameType), m_nQueueType(nQueueType), m_dwRoomID(0), m_dwLeaderPlayerID(0),
	m_pkBattleground(nullptr), m_nBattlegroundMapIndex(0), m_dwBattlegroundVID(0), m_bInGame(false), m_pkDungeon(nullptr),
	m_bIsLocked(false)
{
	m_pkMapAttenders.clear();
	m_pkMapAttenderStates.clear();
	m_pkMapAttenderNames.clear();
	m_strLeaderName.clear();
	m_pkSetKickedPlayers.clear();
	m_dwSetupTime = get_dword_time();
}

bool CBattlegroundRoom::StartBattleground()
{
	m_bIsLocked = true;

	// Game settings
	if (!(m_nGameMode >= BG_GAME_MODE_FUN && m_nGameMode <= BG_GAME_MODE_COMPETITIVE))
	{
		sys_err("Unknown battleground game mode: %u", m_nGameMode);
		m_bIsLocked = false;
		return false;
	}
	if (!(m_nGameType >= BG_QUEUE_TYPE_LOBBY && m_nGameType <= BG_QUEUE_TYPE_GUILD))
	{
		sys_err("Unknown battleground game type: %u", m_nGameType);
		m_bIsLocked = false;
		return false;
	}
	if (!(m_nQueueType >= BG_MATCHMAKING_SOLO && m_nQueueType <= BG_MATCHMAKING_QUINTET))
	{
		sys_err("Unknown battleground queue type: %u", m_nQueueType);
		m_bIsLocked = false;
		return false;
	}

	// Room settings
	if (!m_dwRoomID)
	{
		sys_err("Undefined battleground room id");
		m_bIsLocked = false;
		return false;		
	}

	if (!m_dwLeaderPlayerID)
	{
		sys_err("Undefined battleground leader! room: %u", m_dwRoomID);
		m_bIsLocked = false;
		return false;		
	}
	auto pkLeadChar = CHARACTER_MANAGER::instance().FindByPID(m_dwLeaderPlayerID);
	if (!pkLeadChar)
	{
		sys_err("Unknown battleground leader! pid: %u", m_dwLeaderPlayerID);
		m_bIsLocked = false;
		return false;		
	}

	// Participant controls
	auto nRoomLimit = GetRoomLimit();
	if (!g_bIsTestServer)
	{
		if (m_pkMapAttenders.size() > nRoomLimit)
		{
			sys_err("Battleground participant overflow! Limit: %u Size: %u", nRoomLimit, m_pkMapAttenders.size());
			m_bIsLocked = false;
			return false;
		}
		if (m_pkMapAttenders.size() != nRoomLimit)
		{
			pkLeadChar->ChatPacket(CHAT_TYPE_INFO, "Have not enough player yet for start battleground!");
			m_bIsLocked = false;
			return false;
		}
		
		uint8_t cbBlueTeamAttenderSize = 0;
		uint8_t cbRedTeamAttenderSize = 0;
		for (const auto& [pid, tid] : m_pkMapAttenders)
		{
			switch (tid)
			{
				case BG_TEAM_BLUE:
					++cbBlueTeamAttenderSize;
					break;
				case BG_TEAM_RED:
					++cbRedTeamAttenderSize;
					break;
				default:
					break;
			}

			if (!CHARACTER_MANAGER::instance().FindByPID(pid))
			{
				pkLeadChar->ChatPacket(CHAT_TYPE_INFO, "All attenders must be online!");
				m_bIsLocked = false;
				return false;	
			}
		}
		if (cbBlueTeamAttenderSize != cbRedTeamAttenderSize)
		{
			pkLeadChar->ChatPacket(CHAT_TYPE_INFO, "Both team member count should be same");
			m_bIsLocked = false;
			return false;
		}

		for (const auto& [pid, state] : m_pkMapAttenderStates)
		{
			if (state == false)
			{
				pkLeadChar->ChatPacket(CHAT_TYPE_INFO, "All attenders must be ready!");
				m_bIsLocked = false;
				return false;	
			}
		}
	}

	// Start battleground
	auto pkBattlegroundInfo = CBattlegroundManager::instance().CreateBattleground(m_nGameMode, m_nGameType, m_nQueueType);
	if (!pkBattlegroundInfo)
	{
		sys_err("Battleground dungeon can not created! room id: %u", m_dwRoomID);
		m_bIsLocked = false;
		return false;		
	}
	m_pkBattleground = pkBattlegroundInfo->pkBattleground;
	m_pkDungeon = pkBattlegroundInfo->pkDungeon;
	m_nBattlegroundMapIndex = pkBattlegroundInfo->nMapIndex;
	m_dwBattlegroundVID = pkBattlegroundInfo->dwEventVID;

	delete pkBattlegroundInfo;
	pkBattlegroundInfo = nullptr;

	m_pkBattleground->SetDungeon(m_pkDungeon);
	m_pkBattleground->SetRoomID(GetRoomID());
	m_pkBattleground->SetLeaderName(pkLeadChar->GetName());
	m_pkBattleground->SetParticipantCount(static_cast<uint8_t>(m_pkMapAttenders.size()));
	m_pkBattleground->SetParticipantLimit(nRoomLimit);

	pkLeadChar->ChatPacket(CHAT_TYPE_COMMAND, "bg_game_started");

	// Send notification & warp for all attenders
	for (const auto& [pid, tid] : m_pkMapAttenders)
	{
		auto pkCurrChar = CHARACTER_MANAGER::instance().FindByPID(pid);
		if (pkCurrChar)
		{
			// pkCurrChar->ChatPacket(CHAT_TYPE_INFO, "Battleground created, you will join within 10 seconds!");
			m_pkBattleground->Enter(pkCurrChar, tid);
			CBattlegroundManager::instance().WarpToBattlegroundMap(pkCurrChar, m_dwBattlegroundVID, tid);
		}
	}

	m_bInGame = true;
	return true;
}
void CBattlegroundRoom::DestroyRoom()
{
	if (m_pkMapAttenders.size() > 0)
	{
		for (const auto& [pid, tid] : m_pkMapAttenders)
		{
			auto pkCurrChar = CHARACTER_MANAGER::instance().FindByPID(pid);
			if (pkCurrChar)
				pkCurrChar->ChatPacket(CHAT_TYPE_COMMAND, "bg_room_ntf destroy NONE 0 0 0 0 0");
		}
	}

	CBattlegroundManager::instance().DeleteBattlegroundRoom(m_dwRoomID);
}

void CBattlegroundRoom::SetRoomID(uint32_t dwRoomID)
{
	m_dwRoomID = dwRoomID;
}
uint32_t CBattlegroundRoom::GetRoomID() const
{
	return m_dwRoomID;
}

bool CBattlegroundRoom::JoinToRoom(uint32_t dwPlayerID, uint8_t nTeamID)
{
	if (m_bInGame)
	{
		sys_err("Room already is in game");
		return false;	
	}
	if (m_bIsLocked)
	{
		sys_err("Room locked");
		return false;			
	}

	if (!dwPlayerID)
	{
		sys_err("Null attender pid");
		return false;		
	}

	if (!(nTeamID >= BG_TEAM_BLUE && nTeamID <= BG_TEAM_RED))
	{
		sys_err("Unknown battleground team id: %u", nTeamID);
		return false;
	}

	auto it = m_pkMapAttenders.find(dwPlayerID);
	if (it != m_pkMapAttenders.end())
	{
		sys_err("Attender %u already in a room?", dwPlayerID);
		return false;
	}

	auto pkChar = CHARACTER_MANAGER::instance().FindByPID(dwPlayerID);
	if (!pkChar)
	{
		sys_err("Attender %u is not online", dwPlayerID);
		return false;
	}

	if (pkChar->GetLevel() < 15)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, "You must 15+ level for join battleground");
		return false;
	}

	if (pkChar->GetBattleground())
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, "You are already a battleground player");
		return false;
	}

	auto isLeader = (dwPlayerID == m_dwLeaderPlayerID);

	m_pkMapAttenders.emplace(dwPlayerID, nTeamID);
	if (isLeader)
		m_pkMapAttenderStates.emplace(dwPlayerID, true);
	else
		m_pkMapAttenderStates.emplace(dwPlayerID, false);
	m_pkMapAttenderNames.emplace(dwPlayerID, pkChar->GetName());

	for (const auto& [pid, tid] : m_pkMapAttenders)
	{
		auto pkCurrChar = CHARACTER_MANAGER::instance().FindByPID(pid);
		if (pkCurrChar)
		{
			pkCurrChar->ChatPacket(CHAT_TYPE_COMMAND, "bg_room_ntf join %s %u %d %u %u %d",
				pkChar->GetName(), nTeamID, pkChar->GetLevel(), pkChar->GetEmpire(), pkChar->GetRaceNum(), isLeader ? 1 : 0);
		}
	}

	return true;
}
void CBattlegroundRoom::LeaveFromRoom(uint32_t dwPlayerID)
{
	auto it1 = m_pkMapAttenders.find(dwPlayerID);
	if (it1 == m_pkMapAttenders.end())
	{
		sys_err("Attender %u isn't in room?", dwPlayerID);
		return;
	}
	auto nTeamID = it1->second;

	m_pkMapAttenders.erase(it1);

	auto it2 = m_pkMapAttenderStates.find(dwPlayerID);
	if (it2 == m_pkMapAttenderStates.end())
	{
		sys_err("Attender %u isn't in room?", dwPlayerID);
		return;
	}

	m_pkMapAttenderStates.erase(it2);

	auto it3 = m_pkMapAttenderNames.find(dwPlayerID);
	if (it3 == m_pkMapAttenderNames.end())
	{
		sys_err("Attender %u isn't in room?", dwPlayerID);
		return;
	}
	auto stName = it3->second;

	m_pkMapAttenderNames.erase(it3);

	if (m_pkMapAttenders.size() == 0)
	{
		DestroyRoom();
		return;
	}

	for (const auto& [pid, tid] : m_pkMapAttenders)
	{
		auto pkCurrChar = CHARACTER_MANAGER::instance().FindByPID(pid);
		if (pkCurrChar)
		{
			pkCurrChar->ChatPacket(CHAT_TYPE_COMMAND, "bg_room_ntf leave %s %u 0 0 0 0", stName.c_str(), nTeamID);
		}
	}
}
bool CBattlegroundRoom::IsAttender(uint32_t dwPlayerID) const
{
	/* 
	for (const auto& [pid, tid] : m_pkMapAttenders)
	{
		if (pid == dwPlayerID)
			return true;
	}
	return false;
	*/
	auto it = m_pkMapAttenders.find(dwPlayerID);
	return it != m_pkMapAttenders.end();
}
uint32_t CBattlegroundRoom::GetAttenderCount() const
{
	return m_pkMapAttenders.size();
}

bool CBattlegroundRoom::KickPlayer(const std::string& strName)
{
	if (strName.empty())
		return false;

	m_pkSetKickedPlayers.emplace(strName);

	auto pkChar = CHARACTER_MANAGER::instance().FindPC(strName.c_str());
	if (pkChar)
	{
		LeaveFromRoom(pkChar->GetPlayerID());
		return true;		
	}
	return false;
}

void CBattlegroundRoom::SetAttenderState(uint32_t dwPlayerID, bool bNewState)
{
	if (m_bIsLocked)
		return;

	auto it = m_pkMapAttenderStates.find(dwPlayerID);
	if (it == m_pkMapAttenderStates.end())
	{
		sys_err("Attender %u isn't in room?", dwPlayerID);
		return;
	}

	if (it->second == bNewState) 
	{
		sys_err("State change request blocked. same state.");
		return;
	}
	it->second = bNewState;

	auto pkChar = CHARACTER_MANAGER::instance().FindByPID(dwPlayerID);
	if (pkChar)
	{
		for (const auto& [pid, state] : m_pkMapAttenderStates)
		{
			auto pkCurrChar = CHARACTER_MANAGER::instance().FindByPID(pid);
			if (pkCurrChar)
				pkCurrChar->ChatPacket(CHAT_TYPE_COMMAND, "bg_room_ntf state %s %d 0 0 0 0", pkChar->GetName(), bNewState ? 1 : 0);
		}
	}
}
bool CBattlegroundRoom::GetAttenderState(uint32_t dwPlayerID) const
{
	auto it = m_pkMapAttenderStates.find(dwPlayerID);
	if (it == m_pkMapAttenderStates.end())
	{
		sys_err("Attender %u isn't in room?", dwPlayerID);
		return false;
	}

	return it->second;
}

void CBattlegroundRoom::SetLeaderPlayerID(uint32_t dwLeaderPlayerID)
{
	m_dwLeaderPlayerID = dwLeaderPlayerID;

	auto pkLeaderChar = CHARACTER_MANAGER::instance().FindByPID(dwLeaderPlayerID);
	if (pkLeaderChar)
		m_strLeaderName = pkLeaderChar->GetName();
}
uint32_t CBattlegroundRoom::GetLeaderPlayerID() const
{
	return m_dwLeaderPlayerID;
}
std::string CBattlegroundRoom::GetLeaderName() const
{
	return m_strLeaderName;
}

uint8_t CBattlegroundRoom::GetGameMode() const
{
	return m_nGameMode;
}
uint8_t CBattlegroundRoom::GetGameType() const
{
	return m_nGameType;
}
uint8_t CBattlegroundRoom::GetQueueType() const
{
	return m_nQueueType;
}
uint8_t CBattlegroundRoom::GetRoomLimit() const
{
	switch (m_nQueueType)
	{
		case BG_MATCHMAKING_SOLO:
			return 2;
		case BG_MATCHMAKING_DUO:
			return 4;
		case BG_MATCHMAKING_TRIO:
			return 6;
		case BG_MATCHMAKING_QUARTET:
			return 8;
		case BG_MATCHMAKING_QUINTET:
			return 10;
		default:
			sys_err("Unknown battleground queue type! room id: %u", m_dwRoomID);
			return 0;
	}
}
uint32_t CBattlegroundRoom::GetSetupTime() const
{
	return m_dwSetupTime;
}
bool CBattlegroundRoom::IsInGame() const
{
	return m_bInGame;
}

void CBattlegroundRoom::SendRoomDetails(LPCHARACTER pkChar)
{
	if (!pkChar || !pkChar->GetDesc())
		return;

	pkChar->ChatPacket(CHAT_TYPE_COMMAND, "bg_room_info %u %u %u %u %s %u %d",
		m_dwRoomID, m_nGameMode, m_nGameType, m_nQueueType, m_strLeaderName.c_str(), m_dwSetupTime, m_dwLeaderPlayerID == pkChar->GetPlayerID());

	for (const auto& [pid, tid] : m_pkMapAttenders)
	{
		auto pkCurrChar = CHARACTER_MANAGER::instance().FindByPID(pid);
		if (pkCurrChar)
		{
			pkChar->ChatPacket(CHAT_TYPE_COMMAND, "bg_room_player_info %u %s %d %u %u %u",
				tid, pkCurrChar->GetName(), pkCurrChar->GetLevel(), pkCurrChar->GetEmpire(), pkCurrChar->GetRaceNum(), GetAttenderState(pkCurrChar->GetPlayerID())
			);
		}
	}
}

void CBattlegroundRoom::SetTeamID(uint32_t dwPlayerID, uint8_t nTeamID)
{
	if (m_bIsLocked)
		return;
		
	if (!(nTeamID >= BG_TEAM_BLUE && nTeamID <= BG_TEAM_RED))
	{
		sys_err("Unknown battleground team id: %u", nTeamID);
		return;
	}
	
	auto it = m_pkMapAttenders.find(dwPlayerID);
	if (it == m_pkMapAttenders.end())
	{
		sys_err("Attender %u isn't in room?", dwPlayerID);
		return;
	}

	if (it->second == nTeamID) 
	{
		sys_err("Team change request blocked. same team.");
		return;
	}

	it->second = nTeamID;

	auto pkChar = CHARACTER_MANAGER::instance().FindByPID(dwPlayerID);
	if (pkChar)
	{
		for (const auto& [pid, tid] : m_pkMapAttenders)
		{
			auto pkCurrChar = CHARACTER_MANAGER::instance().FindByPID(pid);
			if (pkCurrChar)
				pkCurrChar->ChatPacket(CHAT_TYPE_COMMAND, "bg_room_ntf team_id %s %u 0 0 0 0", pkChar->GetName(), nTeamID);
		}
	}
}
uint8_t CBattlegroundRoom::GetTeamID(uint32_t dwPlayerID) const
{
	auto it = m_pkMapAttenders.find(dwPlayerID);
	if (it == m_pkMapAttenders.end())
	{
		sys_err("Attender %u isn't in room?", dwPlayerID);
		return BG_TEAM_NONE;
	}

	return it->second;
}

