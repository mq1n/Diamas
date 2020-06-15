#include "stdafx.h"
#include "constants.h"
#include "utils.h"
#include "desc.h"
#include "char.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "party.h"
#include "regen.h"
#include "p2p.h"
#include "dungeon.h"
#include "db.h"
#include "config.h"
#include "xmas_event.h"
#include "quest_manager.h"
#include "questlua.h"
#include "locale_service.h"
#include "log.h"
#include "nearby_scanner.h"

#include "../../common/service.h"
CHARACTER_MANAGER::CHARACTER_MANAGER() :
	m_iVIDCount(0),
	m_pkChrSelectedStone(nullptr),
	m_bUsePendingDestroy(false)
{
	RegisterRaceNum(xmas::MOB_XMAS_FIRWORK_SELLER_VNUM);
	RegisterRaceNum(xmas::MOB_SANTA_VNUM);
	RegisterRaceNum(xmas::MOB_XMAS_TREE_VNUM);

	m_iMobItemRate = 100;
	m_iMobDamageRate = 100;
	m_iMobGoldAmountRate = 100;
	m_iMobGoldDropRate = 100;
	m_iMobExpRate = 100;

	m_iMobItemRatePremium = 100;
	m_iMobGoldAmountRatePremium = 100;
	m_iMobGoldDropRatePremium = 100;
	m_iMobExpRatePremium = 100;
	
	m_iUserDamageRate = 100;
	m_iUserDamageRatePremium = 100;
}

CHARACTER_MANAGER::~CHARACTER_MANAGER()
{
	Destroy();
}

void CHARACTER_MANAGER::Destroy()
{
	auto it = m_map_pkChrByVID.begin();
	auto lastIt = it;
	std::vector<uint32_t> deletedVID;

    while (it != m_map_pkChrByVID.end()) 
	{
        LPCHARACTER ch = it->second;
        M2_DESTROY_CHARACTER(ch); // m_map_pkChrByVID is changed here
        it = m_map_pkChrByVID.begin();
		if (it == m_map_pkChrByVID.end())
			break;

		if (lastIt == it)
		{
			sys_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FATAL ERROR !!!!!!!!!!!!!!!!!!!!!!!!!!!!");
			sys_err("Could not process iterator while destroying charMgr. key: %u, expected: %u, pendingDestroy: %b", it->first, (uint32_t)ch->GetVID(), m_bUsePendingDestroy);
			sys_err("Dumping deleted vids list:");
			
			std::string dump = "";
			for (size_t i = 0, leftInLine = 20, size = deletedVID.size(); i < size; ++i, --leftInLine)
			{
				dump += std::to_string(i);
				if (leftInLine <= 0)
				{
					sys_err("%s", dump.c_str());
					dump = "";
					leftInLine = 20;
				}
				else
				{
					dump += ",";
				}
			}

			if (dump != "")
				sys_err("%s", dump.c_str());

			it = m_map_pkChrByVID.erase(it);
		}
		else
		{
			lastIt = it;
			deletedVID.push_back(it->first);
		}
    }
}

void CHARACTER_MANAGER::GracefulShutdown()
{
    auto it = m_map_pkPCChr.begin();

	while (it != m_map_pkPCChr.end())
		(it++)->second->Disconnect("GracefulShutdown");
}

uint32_t CHARACTER_MANAGER::AllocVID()
{
	++m_iVIDCount;
	return m_iVIDCount;
}

LPCHARACTER CHARACTER_MANAGER::CreateCharacter(std::string stName, uint32_t dwPID)
{
	uint32_t dwVID = AllocVID();

#ifdef M2_USE_POOL
	LPCHARACTER ch = pool_.Construct();
#else
	LPCHARACTER ch = M2_NEW CHARACTER;
#endif
    ch->Create(stName, dwVID, dwPID ? true : false);

	m_map_pkChrByVID.insert(std::make_pair(dwVID, ch));

	if (dwPID)
	{
		std::transform(stName.begin(), stName.end(), stName.begin(), ::tolower);

        m_map_pkPCChr.insert(NAME_MAP::value_type(stName.c_str(), ch));
		m_map_pkChrByPID.insert(std::make_pair(dwPID, ch));
	}

	return (ch);
}

#ifndef DEBUG_ALLOC
void CHARACTER_MANAGER::DestroyCharacter(LPCHARACTER ch)
#else
void CHARACTER_MANAGER::DestroyCharacter(LPCHARACTER ch, const char* file, size_t line)
#endif
{
	if (!ch)
		return;

	// <Factor> Check whether it has been already deleted or not.
	auto it = m_map_pkChrByVID.find(ch->GetVID());
	if (it == m_map_pkChrByVID.end()) {
		sys_err("[CHARACTER_MANAGER::DestroyCharacter] <Factor> %d not found", static_cast<int32_t>(ch->GetVID()));
		return; // prevent duplicated destrunction
	}
	CNearbyScanner::instance().Die(ch->GetVID());

	// ´øAu¿¡ ¼O¼OμE ¸o½ºAI´A ´øAu¿¡¼­μμ ≫eA|CIμμ·I.
	if (ch->IsNPC() && !ch->IsPet() && ch->GetRider() == nullptr)
	{
		if (ch->GetDungeon())
		{
			ch->GetDungeon()->DeadCharacter(ch);
		}
	}

	if (m_bUsePendingDestroy)
	{
		m_set_pkChrPendingDestroy.insert(ch);
		return;
	}

	m_map_pkChrByVID.erase(it);

	if (ch->IsPC())
	{
		std::string stNameLower = ch->GetStringName();
		std::transform(stNameLower.begin(), stNameLower.end(), stNameLower.begin(), ::tolower);

		auto itFndName = m_map_pkPCChr.find(stNameLower);

        if (m_map_pkPCChr.end() != itFndName)
            m_map_pkPCChr.erase(itFndName);
	}

	if (ch->GetPlayerID() && ch->IsPC())
    {
        auto itByPid = m_map_pkChrByPID.find(ch->GetPlayerID());

        if (m_map_pkChrByPID.end() != itByPid)
        {
            m_map_pkChrByPID.erase(itByPid);
        }
    }

	auto itByBG = m_map_pkBGMob.find(ch);
	if (itByBG != m_map_pkBGMob.end())
		m_map_pkBGMob.erase(itByBG);

	UnregisterRaceNumMap(ch);

	RemoveFromStateList(ch);

#ifdef M2_USE_POOL
	pool_.Destroy(ch);
#else
#ifndef DEBUG_ALLOC
	M2_DELETE(ch);
#else
	M2_DELETE_EX(ch, file, line);
#endif
#endif
}

LPCHARACTER CHARACTER_MANAGER::Find(uint32_t dwVID)
{
	auto it = m_map_pkChrByVID.find(dwVID);

	if (m_map_pkChrByVID.end() == it)
		return nullptr;
	
	// <Factor> Added sanity check
	LPCHARACTER found = it->second;
	if (found != nullptr && dwVID != static_cast<uint32_t>(found->GetVID())) {
		sys_err("[CHARACTER_MANAGER::Find] <Factor> %u != %u", dwVID, static_cast<uint32_t>(found->GetVID()));
		return nullptr;
	}
	return found;
}

LPCHARACTER CHARACTER_MANAGER::Find(const VID & vid)
{
	LPCHARACTER tch = Find(static_cast<uint32_t>(vid));

	if (!tch || tch->GetVID() != vid)
		return nullptr;

	return tch;
}

LPCHARACTER CHARACTER_MANAGER::FindByPID(uint32_t dwPID)
{
	auto it = m_map_pkChrByPID.find(dwPID);

	if (m_map_pkChrByPID.end() == it)
		return nullptr;

	// <Factor> Added sanity check
	LPCHARACTER found = it->second;
	if (found != nullptr && dwPID != found->GetPlayerID()) {
		sys_err("[CHARACTER_MANAGER::FindByPID] <Factor> %u != %u", dwPID, found->GetPlayerID());
		return nullptr;
	}
	return found;
}

LPCHARACTER CHARACTER_MANAGER::FindPC(std::string stName)
{
	std::transform(stName.begin(), stName.end(), stName.begin(), ::tolower);

	auto it = m_map_pkPCChr.find(stName);

	if (it == m_map_pkPCChr.end())
		return nullptr;

	// <Factor> Added sanity check
	LPCHARACTER found = it->second;
	if (found != nullptr && strncasecmp(stName.c_str(), found->GetName(), CHARACTER_NAME_MAX_LEN) != 0) {
		sys_err("[CHARACTER_MANAGER::FindPC] <Factor> %s != %s", stName.c_str(), found->GetName());
		return nullptr;
	}
	return found;
}

LPCHARACTER CHARACTER_MANAGER::SpawnMobRandomPosition(uint32_t dwVnum, int32_t lMapIndex)
{
	// CØAA¸| ½ºÆuCOAo ¸≫Ao¸| °aA¤CO ¼o AO°O CO
	if (dwVnum == 5002 && !quest::CQuestManager::instance().GetEventFlag("newyear_mob"))
	{
		sys_log(1, "HAETAE (new-year-mob) [5002] regen disabled.");
		return nullptr;
	}


	if (dwVnum == 5004 && !quest::CQuestManager::instance().GetEventFlag("independence_day"))
	{
		sys_log(1, "INDEPENDECE DAY [5004] regen disabled.");
		return nullptr;
	}
	

	const CMob * pkMob = CMobManager::instance().Get(dwVnum);

	if (!pkMob)
	{
		sys_err("no mob data for vnum %u on map index %d", dwVnum, lMapIndex);
		return nullptr;
	}

	if (!map_allow_find(lMapIndex))
	{
		sys_err("not allowed map %u", lMapIndex);
		return nullptr;
	}

	LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::instance().GetMap(lMapIndex);
	if (pkSectreeMap == nullptr) {
		return nullptr;
	}

	int32_t i;
    int32_t x = 0, y = 0;
	for (i=0; i<2000; i++)
	{
		x = number(1, (pkSectreeMap->m_setting.iWidth / 100)  - 1) * 100 + pkSectreeMap->m_setting.iBaseX;
		y = number(1, (pkSectreeMap->m_setting.iHeight / 100) - 1) * 100 + pkSectreeMap->m_setting.iBaseY;
		//LPSECTREE tree = SECTREE_MANAGER::instance().Get(lMapIndex, x, y);
		LPSECTREE tree = pkSectreeMap->Find(x, y);

		if (!tree)
			continue;

		uint32_t dwAttr = tree->GetAttribute(x, y);

		if (IS_SET(dwAttr, ATTR_BLOCK | ATTR_OBJECT))
			continue;

        if (IS_SET(dwAttr, ATTR_BANPK) && !mining::IsVeinOfOre(dwVnum))
			continue;

		break;
	}

	if (i == 2000)
	{
		sys_err("cannot find valid location");
		return nullptr;
	}

	LPSECTREE sectree = SECTREE_MANAGER::instance().Get(lMapIndex, x, y);

	if (!sectree)
	{
		sys_log(0, "SpawnMobRandomPosition: cannot create monster at non-exist sectree %d x %d (map %d)", x, y, lMapIndex);
		return nullptr;
	}

	LPCHARACTER ch = CHARACTER_MANAGER::instance().CreateCharacter(pkMob->m_table.szLocaleName);

	if (!ch)
	{
		sys_log(0, "SpawnMobRandomPosition: cannot create new character");
		return nullptr;
	}

	ch->SetProto(pkMob);

	// if mob is npc with no empire assigned, assign to empire of map
	if (pkMob->m_table.bType == CHAR_TYPE_NPC)
		if (ch->GetEmpire() == 0)
			ch->SetEmpire(SECTREE_MANAGER::instance().GetEmpireFromMapIndex(lMapIndex));

	ch->SetRotation(number(0, 360));

	if (!ch->Show(lMapIndex, x, y, 0, false))
	{
		M2_DESTROY_CHARACTER(ch);
		sys_err(0, "SpawnMobRandomPosition: cannot show monster");
		return nullptr;
	}

	char buf[512+1];
	int32_t local_x = x - pkSectreeMap->m_setting.iBaseX;
	int32_t local_y = y - pkSectreeMap->m_setting.iBaseY;
	snprintf(buf, sizeof(buf), "spawn %s[%d] random position at %d %d %d %d (time: %d)", ch->GetName(), dwVnum, x, y, local_x, local_y, static_cast<int32_t>(get_global_time()));
	
	if (g_bIsTestServer)
		SendNotice(buf);

	sys_log(0, buf);
	return (ch);
}

LPCHARACTER CHARACTER_MANAGER::SpawnMob(uint32_t dwVnum, int32_t lMapIndex, int32_t x, int32_t y, int32_t z, bool bSpawnMotion, int32_t iRot, bool bShow, bool bBattleground)
{
	const CMob * pkMob = CMobManager::instance().Get(dwVnum);
	if (!pkMob)
	{
		sys_err("SpawnMob: no mob data for vnum %u (map %d)", dwVnum, lMapIndex);
		return nullptr;
	}

	if (!(pkMob->m_table.bType == CHAR_TYPE_NPC || pkMob->m_table.bType == CHAR_TYPE_WARP || pkMob->m_table.bType == CHAR_TYPE_GOTO) || mining::IsVeinOfOre (dwVnum))
	{
		LPSECTREE tree = SECTREE_MANAGER::instance().Get(lMapIndex, x, y);

		if (!tree)
		{
			sys_log(0, "no sectree for spawn at %d %d mobvnum %d mapindex %d", x, y, dwVnum, lMapIndex);
			return nullptr;
		}

		uint32_t dwAttr = tree->GetAttribute(x, y);

		bool is_set = false;

		if ( mining::IsVeinOfOre (dwVnum) ) is_set = IS_SET(dwAttr, ATTR_BLOCK);
		else is_set = IS_SET(dwAttr, ATTR_BLOCK | ATTR_OBJECT);

		if ( is_set )
		{
			// SPAWN_BLOCK_LOG
			static bool s_isLog=quest::CQuestManager::instance().GetEventFlag("spawn_block_log");
			static uint32_t s_nextTime=get_global_time()+10000;

			uint32_t curTime=get_global_time();

			if (curTime>s_nextTime)
			{
				s_nextTime=curTime;
				s_isLog=quest::CQuestManager::instance().GetEventFlag("spawn_block_log");

			}

			if (s_isLog)
				sys_log(0, "SpawnMob: BLOCKED position for spawn %s %u at %d %d (attr %u)", pkMob->m_table.szName, dwVnum, x, y, dwAttr);
			// END_OF_SPAWN_BLOCK_LOG
			return nullptr;
		}

        if (IS_SET(dwAttr, ATTR_BANPK) && !mining::IsVeinOfOre(dwVnum))
		{
			sys_log(0, "SpawnMob: BAN_PK position for mob spawn %s %u at %d %d %d", pkMob->m_table.szName, dwVnum, x, y, lMapIndex);
			return nullptr;
		}
	}

	LPSECTREE sectree = SECTREE_MANAGER::instance().Get(lMapIndex, x, y);

	if (!sectree)
	{
		sys_log(0, "SpawnMob: cannot create monster at non-exist sectree %d x %d (map %d)", x, y, lMapIndex);
		return nullptr;
	}

	LPCHARACTER ch = CHARACTER_MANAGER::instance().CreateCharacter(pkMob->m_table.szLocaleName);

	if (!ch)
	{
		sys_log(0, "SpawnMob: cannot create new character");
		return nullptr;
	}

	if (iRot == -1)
		iRot = number(0, 360);

	ch->SetProto(pkMob);

	// if mob is npc with no empire assigned, assign to empire of map
	if (pkMob->m_table.bType == CHAR_TYPE_NPC)
		if (ch->GetEmpire() == 0)
			ch->SetEmpire(SECTREE_MANAGER::instance().GetEmpireFromMapIndex(lMapIndex));

	ch->SetRotation(static_cast<float>(iRot));

	if (bShow && !ch->Show(lMapIndex, x, y, z, bSpawnMotion))
	{
		M2_DESTROY_CHARACTER(ch);
		sys_log(0, "SpawnMob: cannot show monster on map %d x %d y %d", lMapIndex, x, y);
		return nullptr;
	}

	if (bBattleground)
		m_map_pkBGMob.emplace(ch);

	return ch;
}

LPCHARACTER CHARACTER_MANAGER::SpawnMobRange(uint32_t dwVnum, int32_t lMapIndex, int32_t sx, int32_t sy, int32_t ex, int32_t ey, bool bIsException, bool bSpawnMotion, bool bAggressive )
{
	const CMob * pkMob = CMobManager::instance().Get(dwVnum);

	if (!pkMob)
		return nullptr;

	if (pkMob->m_table.bType == CHAR_TYPE_STONE)	// 돌은 무조건 SPAWN 모션이 있다.
		bSpawnMotion = true;

	int32_t i = 16;

	while (i--)
	{
		int32_t x = number(sx, ex);
		int32_t y = number(sy, ey);
		/*
		   if (bIsException)
		   if (is_regen_exception(x, y))
		   continue;
		 */
		LPCHARACTER ch = SpawnMob(dwVnum, lMapIndex, x, y, 0, bSpawnMotion);

		if (ch)
		{
			sys_log(1, "MOB_SPAWN: %s(%d) %dx%d", ch->GetName(), (uint32_t) ch->GetVID(), ch->GetX(), ch->GetY());
			if ( bAggressive )
				ch->SetAggressive();
			return (ch);
		}
	}

	return nullptr;
}

void CHARACTER_MANAGER::SelectStone(LPCHARACTER pkChr)
{
	m_pkChrSelectedStone = pkChr;
}

bool CHARACTER_MANAGER::SpawnMoveGroup(uint32_t dwVnum, int32_t lMapIndex, int32_t sx, int32_t sy, int32_t ex, int32_t ey, int32_t tx, int32_t ty, LPREGEN pkRegen, bool bAggressive_)
{
	CMobGroup * pkGroup = CMobManager::Instance().GetGroup(dwVnum);

	if (!pkGroup)
	{
		sys_err("NOT_EXIST_GROUP_VNUM(%u) Map(%u) ", dwVnum, lMapIndex);
		return false;
	}

	LPCHARACTER pkChrMaster = nullptr;
	LPPARTY pkParty = nullptr;

	const std::vector<uint32_t> & c_rdwMembers = pkGroup->GetMemberVector();

	bool bSpawnedByStone = false;
	bool bAggressive = bAggressive_;

	if (m_pkChrSelectedStone)
	{
		bSpawnedByStone = true;
		if (m_pkChrSelectedStone->GetDungeon())
			bAggressive = true;
	}

	for (uint32_t i = 0; i < c_rdwMembers.size(); ++i)
	{
		LPCHARACTER tch = SpawnMobRange(c_rdwMembers[i], lMapIndex, sx, sy, ex, ey, true, bSpawnedByStone);

		if (!tch)
		{
			if (i == 0)	// 못만든 몬스터가 대장일 경우에는 그냥 실패
				return false;

			continue;
		}

		sx = tch->GetX() - number(300, 500);
		sy = tch->GetY() - number(300, 500);
		ex = tch->GetX() + number(300, 500);
		ey = tch->GetY() + number(300, 500);

		if (m_pkChrSelectedStone)
			tch->SetStone(m_pkChrSelectedStone);
		else if (pkParty)
		{
			pkParty->Join(tch->GetVID());
			pkParty->Link(tch);
		}
		else if (!pkChrMaster)
		{
			pkChrMaster = tch;
			pkChrMaster->SetRegen(pkRegen);

			pkParty = CPartyManager::instance().CreateParty(pkChrMaster);
		}
		if (bAggressive)
			tch->SetAggressive();

		if (tch->Goto(tx, ty))
			tch->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0); // checkme
	}

	return true;
}

bool CHARACTER_MANAGER::SpawnGroupGroup(uint32_t dwVnum, int32_t lMapIndex, int32_t sx, int32_t sy, int32_t ex, int32_t ey, LPREGEN pkRegen, bool bAggressive_, LPDUNGEON pDungeon)
{
	const uint32_t dwGroupID = CMobManager::Instance().GetGroupFromGroupGroup(dwVnum);

	if( dwGroupID != 0 )
	{
		return SpawnGroup(dwGroupID, lMapIndex, sx, sy, ex, ey, pkRegen, bAggressive_, pDungeon);
	}
	else
	{
		sys_err( "NOT_EXIST_GROUP_GROUP_VNUM(%u) MAP(%d)", dwVnum, lMapIndex );
		return false;
	}
}

LPCHARACTER CHARACTER_MANAGER::SpawnGroup(uint32_t dwVnum, int32_t lMapIndex, int32_t sx, int32_t sy, int32_t ex, int32_t ey, LPREGEN pkRegen, bool bAggressive_, LPDUNGEON pDungeon, bool bRandom)
{
	CMobGroup * pkGroup = CMobManager::Instance().GetGroup(dwVnum);

	if (!pkGroup)
	{
		sys_err("NOT_EXIST_GROUP_VNUM(%u) Map(%u) ", dwVnum, lMapIndex);
		return nullptr;
	}

	LPCHARACTER pkChrMaster = nullptr;
	LPPARTY pkParty = nullptr;

	const std::vector<uint32_t> & c_rdwMembers = pkGroup->GetMemberVector();

	bool bSpawnedByStone = false;
	bool bAggressive = bAggressive_;

	if (m_pkChrSelectedStone)
	{
		bSpawnedByStone = true;

		if (m_pkChrSelectedStone->GetDungeon())
			bAggressive = true;
	}

	LPCHARACTER chLeader = nullptr;

	for (uint32_t i = 0; i < c_rdwMembers.size(); ++i)
	{
		LPCHARACTER tch;
		if (bRandom && i == 0)
			tch = SpawnMobRandomPosition(c_rdwMembers[i], lMapIndex);
		else
			tch = SpawnMobRange(c_rdwMembers[i], lMapIndex, sx, sy, ex, ey, true, bSpawnedByStone);

		if (!tch)
		{
			if (i == 0)	// 못만든 몬스터가 대장일 경우에는 그냥 실패
				return nullptr;

			continue;
		}

		if (i == 0)
			chLeader = tch;

		tch->SetDungeon(pDungeon);

		sx = tch->GetX() - number(300, 500);
		sy = tch->GetY() - number(300, 500);
		ex = tch->GetX() + number(300, 500);
		ey = tch->GetY() + number(300, 500);

		if (m_pkChrSelectedStone)
			tch->SetStone(m_pkChrSelectedStone);
		else if (pkParty)
		{
			pkParty->Join(tch->GetVID());
			pkParty->Link(tch);
		}
		else if (!pkChrMaster)
		{
			pkChrMaster = tch;
			pkChrMaster->SetRegen(pkRegen);

			pkParty = CPartyManager::instance().CreateParty(pkChrMaster);
		}

		if (bAggressive)
			tch->SetAggressive();
	}

	return chLeader;
}
/*
struct FuncUpdateAndResetChatCounter
{
	void operator () (LPCHARACTER ch)
	{
		ch->ResetChatCounter();
		ch->CFSM::Update();
	}
};
*/
void CHARACTER_MANAGER::Update(int32_t iPulse)
{
	BeginPendingDestroy();

	auto resetChatCounter = !(iPulse % PASSES_PER_SEC(5));

	// Update PC character
	std::for_each(m_map_pkPCChr.begin(), m_map_pkPCChr.end(),
		[&resetChatCounter, &iPulse](const NAME_MAP::value_type& v)
		{
			auto ch = v.second;

			if (resetChatCounter)
			{
				auto ac = ch->GetAbuseController();
				if (ac)
					ac->Analyze();

				ch->ResetChatCounter();
				ch->CFSM::Update();
			}

			ch->UpdateCharacter(iPulse);
		}
	);

	// Update Monster
	std::for_each(m_set_pkChrState.begin(), m_set_pkChrState.end(),
		[iPulse](LPCHARACTER ch)
		{
			ch->UpdateStateMachine(iPulse);
		}
	);

	// Update to Santa
	// /*
	{
		auto snapshot = CHARACTER_MANAGER::instance().GetCharactersByRaceNum(xmas::MOB_SANTA_VNUM);
		if (!snapshot.empty())
		{
			std::for_each(snapshot.begin(), snapshot.end(), [iPulse](LPCHARACTER ch)
				{
					ch->UpdateStateMachine(iPulse);
				}
			);
		}
	}
	// */

	// Record mob hunting counts once every hour
	if (0 == (iPulse % PASSES_PER_SEC(3600)))
	{
		for (const auto& it : m_map_dwMobKillCount)
			LogManager::instance().MoneyLog(MONEY_LOG_MONSTER_KILL, it.first, it.second);

		m_map_dwMobKillCount.clear();
	}

	// The test server counts the number of characters every 60 seconds
	if (g_bIsTestServer && 0 == (iPulse % PASSES_PER_SEC(60)))
		sys_log(0, "CHARACTER COUNT vid %zu pid %zu", m_map_pkChrByVID.size(), m_map_pkChrByPID.size());

	// Delayed DestroyCharacter
	FlushPendingDestroy();
}


void CHARACTER_MANAGER::ProcessDelayedSave()
{
	for (auto & ch : m_set_pkChrForDelayedSave)
		ch->SaveReal();


	m_set_pkChrForDelayedSave.clear();
}

bool CHARACTER_MANAGER::AddToStateList(LPCHARACTER ch)
{
	assert(ch != nullptr);

	auto it = m_set_pkChrState.find(ch);

	if (it == m_set_pkChrState.end())
	{
		m_set_pkChrState.insert(ch);
		return true;
	}

	return false;
}

void CHARACTER_MANAGER::RemoveFromStateList(LPCHARACTER ch)
{
    auto it = m_set_pkChrState.find(ch);

	if (it != m_set_pkChrState.end())
	{
		//sys_log(0, "RemoveFromStateList %p", ch);
		m_set_pkChrState.erase(it);
	}
}

void CHARACTER_MANAGER::DelayedSave(LPCHARACTER ch)
{
	m_set_pkChrForDelayedSave.insert(ch);
}

bool CHARACTER_MANAGER::FlushDelayedSave(LPCHARACTER ch)
{
    auto it = m_set_pkChrForDelayedSave.find(ch);

	if (it == m_set_pkChrForDelayedSave.end())
		return false;

	m_set_pkChrForDelayedSave.erase(it);
	ch->SaveReal();
	return true;
}

void CHARACTER_MANAGER::KillLog(uint32_t dwVnum)
{
	const uint32_t SEND_LIMIT = 10000;

	auto it = m_map_dwMobKillCount.find(dwVnum);

	if (it == m_map_dwMobKillCount.end())
		m_map_dwMobKillCount.insert(std::make_pair(dwVnum, 1));
	else
	{
		++it->second;

		if (it->second > SEND_LIMIT)
		{
			LogManager::instance().MoneyLog(MONEY_LOG_MONSTER_KILL, it->first, it->second);
			m_map_dwMobKillCount.erase(it);
		}
	}
}

void CHARACTER_MANAGER::RegisterRaceNum(uint32_t dwVnum)
{
	m_set_dwRegisteredRaceNum.insert(dwVnum);
}

void CHARACTER_MANAGER::RegisterRaceNumMap(LPCHARACTER ch)
{
	uint32_t dwVnum = ch->GetRaceNum();

	if (m_set_dwRegisteredRaceNum.find(dwVnum) != m_set_dwRegisteredRaceNum.end()) // 등록된 번호 이면
	{
		sys_log(0, "RegisterRaceNumMap %s %u", ch->GetName(), dwVnum);
		m_map_pkChrByRaceNum[dwVnum].insert(ch);
	}
}

void CHARACTER_MANAGER::UnregisterRaceNumMap(LPCHARACTER ch)
{
	uint32_t dwVnum = ch->GetRaceNum();

	auto it = m_map_pkChrByRaceNum.find(dwVnum);

	if (it != m_map_pkChrByRaceNum.end())
		it->second.erase(ch);
}

CharacterSetSnapshot CHARACTER_MANAGER::GetCharactersByRaceNum(uint32_t dwRaceNum)
{
	const auto it = m_map_pkChrByRaceNum.find(dwRaceNum);
	if (it == m_map_pkChrByRaceNum.end())
		return CharacterSetSnapshot();

	return CharacterSetSnapshot(it->second);
}

int32_t CHARACTER_MANAGER::CountCharactersByRaceNum(uint32_t dwRaceNum)
{
	auto it = m_map_pkChrByRaceNum.find(dwRaceNum);

	if (it == m_map_pkChrByRaceNum.end())
		return 0;
	
	return it->second.size();
}

#define FIND_JOB_WARRIOR_0	(1 << 3)
#define FIND_JOB_WARRIOR_1	(1 << 4)
#define FIND_JOB_WARRIOR_2	(1 << 5)
#define FIND_JOB_WARRIOR	(FIND_JOB_WARRIOR_0 | FIND_JOB_WARRIOR_1 | FIND_JOB_WARRIOR_2)
#define FIND_JOB_ASSASSIN_0	(1 << 6)
#define FIND_JOB_ASSASSIN_1	(1 << 7)
#define FIND_JOB_ASSASSIN_2	(1 << 8)
#define FIND_JOB_ASSASSIN	(FIND_JOB_ASSASSIN_0 | FIND_JOB_ASSASSIN_1 | FIND_JOB_ASSASSIN_2)
#define FIND_JOB_SURA_0		(1 << 9)
#define FIND_JOB_SURA_1		(1 << 10)
#define FIND_JOB_SURA_2		(1 << 11)
#define FIND_JOB_SURA		(FIND_JOB_SURA_0 | FIND_JOB_SURA_1 | FIND_JOB_SURA_2)
#define FIND_JOB_SHAMAN_0	(1 << 12)
#define FIND_JOB_SHAMAN_1	(1 << 13)
#define FIND_JOB_SHAMAN_2	(1 << 14)
#define FIND_JOB_SHAMAN		(FIND_JOB_SHAMAN_0 | FIND_JOB_SHAMAN_1 | FIND_JOB_SHAMAN_2)
#ifdef ENABLE_WOLFMAN_CHARACTER
#define FIND_JOB_WOLFMAN_0	(1 << 15)
#define FIND_JOB_WOLFMAN_1	(1 << 16)
#define FIND_JOB_WOLFMAN_2	(1 << 17)
#define FIND_JOB_WOLFMAN		(FIND_JOB_WOLFMAN_0 | FIND_JOB_WOLFMAN_1 | FIND_JOB_WOLFMAN_2)
#endif

//
// (job+1)*3+(skill_group)
//
LPCHARACTER CHARACTER_MANAGER::FindSpecifyPC(uint32_t uiJobFlag, int32_t lMapIndex, LPCHARACTER except, int32_t iMinLevel, int32_t iMaxLevel)
{
	LPCHARACTER chFind = nullptr;
	int32_t n = 0;

	for(auto & it : m_map_pkChrByPID)
	{
		LPCHARACTER ch = it.second;

		if (ch == except)
			continue;

		if (ch->GetLevel() < iMinLevel)
			continue;

		if (ch->GetLevel() > iMaxLevel)
			continue;

		if (ch->GetMapIndex() != lMapIndex)
			continue;

		if (uiJobFlag)
		{
			uint32_t uiChrJob = (1 << ((ch->GetJob() + 1) * 3 + ch->GetSkillGroup()));

			if (!IS_SET(uiJobFlag, uiChrJob))
				continue;
		}

		if (!chFind || number(1, ++n) == 1)
			chFind = ch;
	}

	return chFind;
}

int32_t CHARACTER_MANAGER::GetMobItemRate(LPCHARACTER ch) const
{ 
	if (ch && ch->GetPremiumRemainSeconds(PREMIUM_ITEM) > 0)
		return m_iMobItemRatePremium;
	return m_iMobItemRate; 
}

int32_t CHARACTER_MANAGER::GetMobDamageRate(LPCHARACTER ch) const
{ 
	return m_iMobDamageRate; 
}

int32_t CHARACTER_MANAGER::GetMobGoldAmountRate(LPCHARACTER ch) const
{ 
	if ( !ch )
		return m_iMobGoldAmountRate;

	if (ch && ch->GetPremiumRemainSeconds(PREMIUM_GOLD) > 0)
		return m_iMobGoldAmountRatePremium;
	return m_iMobGoldAmountRate; 
}

int32_t CHARACTER_MANAGER::GetMobGoldDropRate(LPCHARACTER ch) const
{
	if ( !ch )
		return m_iMobGoldDropRate;
	if (ch && ch->GetPremiumRemainSeconds(PREMIUM_GOLD) > 0)
		return m_iMobGoldDropRatePremium;
	return m_iMobGoldDropRate;
}

int32_t CHARACTER_MANAGER::GetMobExpRate(LPCHARACTER ch) const
{ 
	if ( !ch )
		return m_iMobExpRate;

	if (ch && ch->GetPremiumRemainSeconds(PREMIUM_EXP) > 0)
		return m_iMobExpRatePremium;
	return m_iMobExpRate; 
}

int32_t	CHARACTER_MANAGER::GetUserDamageRate(LPCHARACTER ch) const
{
	if (!ch)
		return m_iUserDamageRate;

	if (ch && ch->GetPremiumRemainSeconds(PREMIUM_EXP) > 0)
		return m_iUserDamageRatePremium;

	return m_iUserDamageRate;
}

void CHARACTER_MANAGER::SendScriptToMap(int32_t lMapIndex, const std::string & s)
{
	LPSECTREE_MAP pSecMap = SECTREE_MANAGER::instance().GetMap(lMapIndex);

	if (nullptr == pSecMap)
		return;

	struct packet_script p;

	p.header = HEADER_GC_SCRIPT;
	p.skin = 1;
	p.src_size = static_cast<uint16_t>(s.size());

	quest::FSendPacket f;
	p.size = p.src_size + sizeof(struct packet_script);
	f.buf.write(&p, sizeof(struct packet_script));
	f.buf.write(&s[0], s.size());

	pSecMap->for_each(f);
}

bool CHARACTER_MANAGER::BeginPendingDestroy()
{
	// Begin 이 후에 Begin을 또 하는 경우에 Flush 하지 않는 기능 지원을 위해
	// 이미 시작되어있으면 false 리턴 처리
	if (m_bUsePendingDestroy)
		return false;

	m_bUsePendingDestroy = true;
	return true;
}

void CHARACTER_MANAGER::FlushPendingDestroy()
{
	using namespace std;

	m_bUsePendingDestroy = false; // 플래그를 먼저 설정해야 실제 Destroy 처리가 됨

	if (!m_set_pkChrPendingDestroy.empty())
	{
		sys_log(0, "FlushPendingDestroy size %d", m_set_pkChrPendingDestroy.size());
		
        auto it = m_set_pkChrPendingDestroy.begin(),
			end = m_set_pkChrPendingDestroy.end();
		for ( ; it != end; ++it) {
			M2_DESTROY_CHARACTER(*it);
		}

		m_set_pkChrPendingDestroy.clear();
	}
}

CharacterSnapshotGuard::CharacterSnapshotGuard()
	: m_hasPendingOwnership(CHARACTER_MANAGER::instance().BeginPendingDestroy())
{
	// ctor
}

CharacterSnapshotGuard::~CharacterSnapshotGuard()
{
	if (m_hasPendingOwnership)
		CHARACTER_MANAGER::instance().FlushPendingDestroy();
}

CharacterSetSnapshot::CharacterSetSnapshot()
	: m_chars(nullptr)
{
	// ctor
}

CharacterSetSnapshot::CharacterSetSnapshot(const std::unordered_set<CHARACTER*>& chars)
	: m_chars(&chars)
{
	// ctor
}

std::unordered_set<CHARACTER*>::const_iterator CharacterSetSnapshot::begin() const
{
	if (m_chars)
		return m_chars->begin();

	return{};
}

std::unordered_set<CHARACTER*>::const_iterator CharacterSetSnapshot::end() const
{
	if (m_chars)
		return m_chars->end();

	return{};
}
