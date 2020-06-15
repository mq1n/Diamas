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

CBattlegroundTeam::CBattlegroundTeam(uint8_t nTeamID, CBattleground* pkBattleground, LPSECTREE_MAP pkSectreeMap, int32_t nMapIndex) : 
	m_nTeamID(nTeamID), m_pkSectreeMap(pkSectreeMap), m_nMapIndex(nMapIndex), m_pkBattleground(pkBattleground)
{
	m_vTeamMembers.clear();
	m_pkMapSpawnedMobs.clear();
}

bool CBattlegroundTeam::IsTeamID(uint8_t nTeamID) const
{
	return m_nTeamID == nTeamID;
}
uint8_t CBattlegroundTeam::GetTeamID() const
{
	return m_nTeamID;
}

void CBattlegroundTeam::Enter(uint32_t dwPlayerID)
{
	m_vTeamMembers.emplace_back(dwPlayerID);
}


bool CBattlegroundTeam::IsAttender(uint32_t dwPlayerID) const
{
	for (const auto& dwMemberPlayerID : m_vTeamMembers)
	{
		if (dwMemberPlayerID == dwPlayerID)
			return true;
	}
	return false;
}
uint32_t CBattlegroundTeam::GetAttenderCount() const
{
	return m_vTeamMembers.size();
}

LPCHARACTER CBattlegroundTeam::GetNexus()
{
	if (m_pkMapSpawnedMobs.size() == 0)
		return nullptr;

	for (const auto& pkMob : m_pkMapSpawnedMobs)
	{
		if (pkMob.second == BG_MOB_NEXUS)
		{
			return pkMob.first;
		}
	}

	return nullptr;	
}
bool CBattlegroundTeam::IsOwnMob(uint32_t dwVID) const
{
	if (m_pkMapSpawnedMobs.size() == 0)
		return false;

	for (const auto& pkMob : m_pkMapSpawnedMobs)
	{
		if (pkMob.first && pkMob.first->GetVID() == dwVID)
		{
			return true;
		}
	}

	return false;	
}
bool CBattlegroundTeam::IsOwnMinnion(uint32_t dwVID) const
{
	if (m_pkMapSpawnedMobs.size() == 0)
		return false;

	for (const auto& pkMob : m_pkMapSpawnedMobs)
	{
		if (pkMob.first && pkMob.second == BG_MOB_MINNION && pkMob.first->GetVID() == dwVID)
		{
			return true;
		}
	}

	return false;
}

LPCHARACTER CBattlegroundTeam::Spawn(uint8_t nMobType, uint8_t nLaneID, uint32_t dwVnum, int32_t iX, int32_t iY, int32_t iDir, bool bSpawnMotion, bool bShow)
{
	if (!m_pkSectreeMap)
		return nullptr;
		
	LPCHARACTER pkMob = CHARACTER_MANAGER::instance().SpawnMob(
		dwVnum, m_nMapIndex,
		m_pkSectreeMap->m_setting.iBaseX + iX * 100,
		m_pkSectreeMap->m_setting.iBaseY + iY * 100,
		0, bSpawnMotion, iDir == 0 ? -1 : (iDir - 1) * 45,
		bShow, true
	);

	if (pkMob)
	{
		sys_log(0, "<CBattleground> Spawn: %s (map index: %d).", pkMob->GetName(), m_nMapIndex);
		m_pkMapSpawnedMobs.emplace(pkMob, nMobType);
	}
	return pkMob;
}
