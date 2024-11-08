#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "desc_client.h"
#include "db.h"
#include "log.h"
#include "skill.h"
#include "text_file_loader.h"
#include "priv_manager.h"
#include "quest_manager.h"
#include "unique_item.h"
#include "safebox.h"
#include "blend_item.h"
#include "dev_log.h"
#include "locale_service.h"
#include "item.h"
#include "item_manager.h"
#include "dragon_soul.h"
#include "cube.h"
#include "../../libgame/include/grid.h"

ITEM_MANAGER::ITEM_MANAGER()
	: m_iTopOfTable(0), m_dwVIDCount(0), m_dwCurrentID(0)
{
	m_ItemIDRange.dwMin = m_ItemIDRange.dwMax = m_ItemIDRange.dwUsableItemIDMin = 0;
	m_ItemIDSpareRange.dwMin = m_ItemIDSpareRange.dwMax = m_ItemIDSpareRange.dwUsableItemIDMin = 0;
}

ITEM_MANAGER::~ITEM_MANAGER()
{
	Destroy();
}

void ITEM_MANAGER::Destroy()
{
	auto it = m_VIDMap.begin();
	for ( ; it != m_VIDMap.end(); ++it) {
#ifdef M2_USE_POOL
		pool_.Destroy(it->second);
#else
		M2_DELETE(it->second);
#endif
	}
	m_VIDMap.clear();
}

void ITEM_MANAGER::GracefulShutdown()
{
	std::unordered_set<LPITEM>::iterator it = m_set_pkItemForDelayedSave.begin();

	while (it != m_set_pkItemForDelayedSave.end())
		SaveSingleItem(*(it++));

	m_set_pkItemForDelayedSave.clear();
}

bool ITEM_MANAGER::Initialize(SItemTable_Server * table, int32_t size)
{
	if (!m_vec_prototype.empty())
		m_vec_prototype.clear();

	m_vec_prototype.resize(size);
	memcpy(&m_vec_prototype[0], table, sizeof(SItemTable_Server) * size);
	for (int32_t i = 0; i < size; i++)
	{
		if (0 != m_vec_prototype[i].dwVnumRange)
		{
			m_vec_item_vnum_range_info.push_back( &m_vec_prototype[i]);
		}
	}

	m_map_ItemRefineFrom.clear();
	for (int32_t i = 0; i < size; ++i)
	{

		if (m_vec_prototype[i].dwRefinedVnum)
			m_map_ItemRefineFrom.insert(std::make_pair(m_vec_prototype[i].dwRefinedVnum, m_vec_prototype[i].dwVnum));

		// NOTE : QUEST_GIVE 플래그는 npc 이벤트로 발생.
		if (m_vec_prototype[i].bType == ITEM_QUEST || IS_SET(m_vec_prototype[i].dwFlags, ITEM_FLAG_QUEST_USE | ITEM_FLAG_QUEST_USE_MULTIPLE)
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
			|| (m_vec_prototype[i].bType == ITEM_COSTUME && m_vec_prototype[i].bSubType == COSTUME_MOUNT)
#endif
		)
			quest::CQuestManager::Instance().RegisterNPCVnum(m_vec_prototype[i].dwVnum);

		m_map_vid.emplace(m_vec_prototype[i].dwVnum, m_vec_prototype[i]); 
		if ( g_bIsTestServer )
			sys_log( 0, "ITEM_INFO %d %s ", m_vec_prototype[i].dwVnum, m_vec_prototype[i].szName );	
	}

	int32_t len = 0, len2;
	char buf[512];

	for (int32_t i = 0; i < size; ++i)
	{
		len2 = snprintf(buf + len, sizeof(buf) - len, "%5u %-16s", m_vec_prototype[i].dwVnum, m_vec_prototype[i].szLocaleName);

		if (len2 < 0 || len2 >= (int32_t) sizeof(buf) - len)
			len += (sizeof(buf) - len) - 1;
		else
			len += len2;

		if (!((i + 1) % 4))
		{
			if ( !g_bIsTestServer )
				sys_log(0, "%s", buf);
			len = 0;
		}
		else
		{
			buf[len++] = '\t';
			buf[len] = '\0';
		}
	}

	ITEM_VID_MAP::iterator it = m_VIDMap.begin();

	sys_log (1, "ITEM_VID_MAP %d", m_VIDMap.size() );

	while (it != m_VIDMap.end())
	{
		LPITEM item = it->second;
		++it;

		const SItemTable_Server* tableInfo = GetTable(item->GetOriginalVnum());

		if (nullptr == tableInfo)
		{
			sys_err("cannot reset item table");
			item->SetProto(nullptr);
		}

		item->SetProto(tableInfo);
	}

	return true;
}

LPITEM ITEM_MANAGER::CreateItem(uint32_t vnum, uint32_t count, uint32_t id, bool bTryMagic, int32_t iRarePct, bool bSkipSave)
{
	if (0 == vnum)
		return nullptr;

	uint32_t dwMaskVnum = 0;
	if (GetMaskVnum(vnum))
	{
		dwMaskVnum = GetMaskVnum(vnum);
	}

	const SItemTable_Server* table = GetTable(vnum);

	if (nullptr == table)
		return nullptr;

	LPITEM item = nullptr;

	//id로 검사해서 존재한다면 -- 리턴! 
	if (m_map_pkItemByID.find(id) != m_map_pkItemByID.end())
	{
		item = m_map_pkItemByID[id];
		LPCHARACTER owner = item->GetOwner();
		sys_err("ITEM_ID_DUP: %u %s owner %p", id, item->GetName(), get_pointer(owner));
		return nullptr;
	}

	//아이템 하나 할당하고
#ifdef M2_USE_POOL
	item = pool_.Construct();
#else
	item = M2_NEW CItem(vnum);
#endif

	bool bIsNewItem = (0 == id);

	//초기화 하고. 테이블 셋하고
	item->Initialize();
	item->SetProto(table);
	item->SetMaskVnum(dwMaskVnum);

	if (item->GetType() == ITEM_ELK) // 돈은 ID가 필요없고 저장도 필요없다.
		item->SetSkipSave(true);

	// Unique ID를 세팅하자
	else if (!bIsNewItem)
	{
		item->SetID(id);
		item->SetSkipSave(true);
	}
	else
	{
		item->SetID(GetNewID());

		if (item->GetType() == ITEM_UNIQUE) // 유니크 아이템은 생성시에 소켓에 남은시간을 기록한다.
		{
			if (item->GetValue(2) == 0)
				item->SetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME, item->GetValue(0)); // 게임 시간 유니크
			else
			{
				//int32_t globalTime = get_unix_time();
				//int32_t lastTime = item->GetValue(0);
				//int32_t endTime = get_unix_time() + item->GetValue(0);
				item->SetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME, get_unix_time() + item->GetValue(0)); // 실시간 유니크
			}
		}
	}


	switch (item->GetVnum())
	{
		case ITEM_AUTO_HP_RECOVERY_S:
		case ITEM_AUTO_HP_RECOVERY_M:
		case ITEM_AUTO_HP_RECOVERY_L:
		case ITEM_AUTO_HP_RECOVERY_X:
		case ITEM_AUTO_SP_RECOVERY_S:
		case ITEM_AUTO_SP_RECOVERY_M:
		case ITEM_AUTO_SP_RECOVERY_L:
		case ITEM_AUTO_SP_RECOVERY_X:
		case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_XS:
		case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_S:
		case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_XS:
		case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_S:
		case FUCKING_BRAZIL_ITEM_AUTO_SP_RECOVERY_S:
		case FUCKING_BRAZIL_ITEM_AUTO_HP_RECOVERY_S:
			if (bIsNewItem)
				item->SetSocket(2, item->GetValue(0), true);
			else
				item->SetSocket(2, item->GetValue(0), false);
			break;
	}

	if (item->GetType() == ITEM_ELK) // 돈은 아무 처리가 필요하지 않음
		;
	else if (item->IsStackable())  // 합칠 수 있는 아이템의 경우
	{
		count = MINMAX(1, count, g_bItemCountLimit);

		if (bTryMagic && count <= 1 && IS_SET(item->GetFlag(), ITEM_FLAG_MAKECOUNT))
			count = item->GetValue(1);
	}
	else
		count = 1;

	item->SetVID(++m_dwVIDCount);

	if (bSkipSave == false)
		m_VIDMap.emplace(item->GetVID(), item);

	if (item->GetID() != 0 && bSkipSave == false)
		m_map_pkItemByID.emplace(item->GetID(), item);

	if (!item->SetCount(count))
		return nullptr;

	item->SetSkipSave(false);

	if (item->GetType() == ITEM_UNIQUE && item->GetValue(2) != 0)
		item->StartUniqueExpireEvent();

	for (int32_t i=0 ; i < ITEM_LIMIT_MAX_NUM ; i++)
	{
		// 아이템 생성 시점부터 사용하지 않아도 시간이 차감되는 방식
		if (LIMIT_REAL_TIME == item->GetLimitType(i))
		{
			if (item->GetLimitValue(i))
			{
				item->SetSocket(0, time(0) + item->GetLimitValue(i)); 
			}
			else
			{
				item->SetSocket(0, time(0) + 60*60*24*7); 
			}

			item->StartRealTimeExpireEvent();
		}

		// 기존 유니크 아이템처럼 착용시에만 사용가능 시간이 차감되는 방식
		else if (LIMIT_TIMER_BASED_ON_WEAR == item->GetLimitType(i))
		{
			// 이미 착용중인 아이템이면 타이머를 시작하고, 새로 만드는 아이템은 사용 가능 시간을 세팅해준다. (
			// 아이템몰로 지급하는 경우에는 이 로직에 들어오기 전에 Socket0 값이 세팅이 되어 있어야 한다.
			if (true == item->IsEquipped())
			{
				item->StartTimerBasedOnWearExpireEvent();
			}
			else if(0 == id)
			{
				int32_t duration = item->GetSocket(0);
				if (0 == duration)
					duration = item->GetLimitValue(i);

				if (0 == duration)
					duration = 60 * 60 * 10;	// 정보가 아무것도 없으면 디폴트로 10시간 세팅

				item->SetSocket(0, duration);
			}
		}
	}

	if (id == 0) // 새로 만드는 아이템일 때만 처리
	{
		// 새로추가되는 약초들일경우 성능을 다르게처리
		if (ITEM_BLEND==item->GetType())
		{
			if (Blend_Item_find(item->GetVnum()))
			{
				Blend_Item_set_value(item);
				return item;
			}
		}

		if (table->sAddonType)
		{
			item->ApplyAddon(table->sAddonType);
		}

		if (bTryMagic)
		{
			if (iRarePct == -1)
				iRarePct = table->bAlterToMagicItemPct;

			if (number(1, 100) <= iRarePct)
				item->AlterToMagicItem();
		}

		if (table->bGainSocketPct)
			item->AlterToSocketItem(table->bGainSocketPct);

		// 50300 == 기술 수련서
		if (vnum == 50300 || vnum == ITEM_SKILLFORGET_VNUM)
		{
			extern uint32_t GetRandomSkillVnum(uint8_t bJob = JOB_MAX_NUM);
			item->SetSocket(0, GetRandomSkillVnum());
		}
		else if (ITEM_SKILLFORGET2_VNUM == vnum)
		{
			uint32_t dwSkillVnum;

			do
			{
				dwSkillVnum = number(112, 119);

				if (nullptr != CSkillManager::Instance().Get(dwSkillVnum))
					break;
			} while (true);

			item->SetSocket(0, dwSkillVnum);
		}
	}
	else
	{
		
		if (100 == table->bAlterToMagicItemPct && 0 == item->GetAttributeCount())
		{
			item->AlterToMagicItem();
		}
	}

	if (item->GetType() == ITEM_QUEST)
	{
		for (auto it = m_map_pkQuestItemGroup.begin(); it != m_map_pkQuestItemGroup.end(); it++)
		{
			if (it->second->m_bType == CSpecialItemGroup::QUEST && it->second->Contains(vnum))
			{
				item->SetSIGVnum(it->first);
			}
		}
	}
	else if (item->GetType() == ITEM_UNIQUE)
	{
		for (auto it = m_map_pkSpecialItemGroup.begin(); it != m_map_pkSpecialItemGroup.end(); it++)
		{
			if (it->second->m_bType == CSpecialItemGroup::SPECIAL && it->second->Contains(vnum))
			{
				item->SetSIGVnum(it->first);
			}
		}
	}

	// 새로 생성되는 용혼석 처리.
	if (item->IsDragonSoul() && 0 == id)
	{
		DSManager::Instance().DragonSoulItemInitialize(item);
	}
	return item;
}

void ITEM_MANAGER::DelayedSave(LPITEM item)
{
	if (item->GetID() != 0)
		m_set_pkItemForDelayedSave.insert(item);
}

void ITEM_MANAGER::FlushDelayedSave(LPITEM item)
{
	std::unordered_set<LPITEM>::iterator it = m_set_pkItemForDelayedSave.find(item);

	if (it == m_set_pkItemForDelayedSave.end())
	{
		return;
	}

	m_set_pkItemForDelayedSave.erase(it);
	SaveSingleItem(item);
}

void ITEM_MANAGER::SaveSingleItem(LPITEM item)
{
	uint8_t window = item->GetWindow();
	if (!item->GetOwner())
	{
		uint32_t dwID = item->GetID();
		uint32_t dwOwnerID = item->GetLastOwnerPID();

		db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_DESTROY, 0, sizeof(uint32_t) + sizeof(uint32_t));
		db_clientdesc->Packet(&dwID, sizeof(uint32_t));
		db_clientdesc->Packet(&dwOwnerID, sizeof(uint32_t));

		sys_log(1, "ITEM_DELETE %s:%u", item->GetName(), dwID);
		return;
	}

	sys_log(1, "ITEM_SAVE %s:%u in %s window %d", item->GetName(), item->GetID(), item->GetOwner()->GetName(), item->GetWindow());

	TPlayerItem t;

	t.id = item->GetID();
	t.window = window;
	switch (t.window)
	{
		case EQUIPMENT:
			t.pos = item->GetCell() - INVENTORY_MAX_NUM;
			break;
		case INVENTORY:
			if (BELT_INVENTORY_SLOT_START <= item->GetCell() && BELT_INVENTORY_SLOT_END > item->GetCell())
			{
				t.window = BELT_INVENTORY;
				t.pos = item->GetCell() - BELT_INVENTORY_SLOT_START;
				break;
			}
		default:
			t.pos = item->GetCell();
			break;
	}
	t.count = item->GetCount();
	t.vnum = item->GetOriginalVnum();
	switch (t.window)
	{
		case SAFEBOX:
		case MALL:
			t.owner = item->GetOwner()->GetAID();
			break;
		default:
			t.owner = item->GetOwner()->GetPlayerID();
			break;
	}
	memcpy(t.alSockets, item->GetSockets(), sizeof(t.alSockets));
	memcpy(t.aAttr, item->GetAttributes(), sizeof(t.aAttr));

	db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_SAVE, 0, sizeof(TPlayerItem));
	db_clientdesc->Packet(&t, sizeof(TPlayerItem));
}

void ITEM_MANAGER::Update()
{
	std::unordered_set<LPITEM>::iterator it = m_set_pkItemForDelayedSave.begin();
	std::unordered_set<LPITEM>::iterator this_it;

	while (it != m_set_pkItemForDelayedSave.end())
	{
		this_it = it++;
		LPITEM item = *this_it;

		// SLOW_QUERY 플래그가 있는 것은 종료시에만 저장한다.
		if (item->GetOwner() && IS_SET(item->GetFlag(), ITEM_FLAG_SLOW_QUERY))
			continue;

		SaveSingleItem(item);

		m_set_pkItemForDelayedSave.erase(this_it);
	}
}

void ITEM_MANAGER::RemoveItem(LPITEM item, const char * c_pszReason)
{
	LPCHARACTER o;

	if ((o = item->GetOwner()))
	{
		char szHint[64];
		snprintf(szHint, sizeof(szHint), "%s %u ", item->GetName(), item->GetCount());
		LogManager::Instance().ItemLog(o, item, c_pszReason ? c_pszReason : "REMOVE", szHint);

		// SAFEBOX_TIME_LIMIT_ITEM_BUG_FIX
		if (item->GetWindow() == MALL || item->GetWindow() == SAFEBOX)
		{
			// 20050613.ipkn.시간제 아이템이 상점에 있을 경우 시간만료시 서버가 다운된다.
			CSafebox* pSafebox = item->GetWindow() == MALL ? o->GetMall() : o->GetSafebox();
			if (pSafebox)
			{
				pSafebox->Remove(item->GetCell());
			}
		}
		// END_OF_SAFEBOX_TIME_LIMIT_ITEM_BUG_FIX
		else
		{
			o->SyncQuickslot(QUICKSLOT_TYPE_ITEM, item->GetCell(), 255);
			item->RemoveFromCharacter();
		}
	}

	M2_DESTROY_ITEM(item);
}

#ifndef DEBUG_ALLOC
void ITEM_MANAGER::DestroyItem(LPITEM item)
#else
void ITEM_MANAGER::DestroyItem(LPITEM item, const char* file, size_t line)
#endif
{
	if (!item)
		return;

	if (item->GetSectree())
		item->RemoveFromGround();

	if (item->GetOwner())
	{
		if (CHARACTER_MANAGER::Instance().Find(item->GetOwner()->GetPlayerID()) != nullptr)
		{
			sys_err("DestroyItem (%u): GetOwner %s %s!!", item->GetID(), item->GetName(), item->GetOwner()->GetName());
			item->RemoveFromCharacter();
		}
		else
		{
			sys_err("Item (%u): WTH! Invalid item owner. owner pointer : %p", item->GetID(), item->GetOwner());
		}
	}

	std::unordered_set<LPITEM>::iterator it = m_set_pkItemForDelayedSave.find(item);

	if (it != m_set_pkItemForDelayedSave.end())
		m_set_pkItemForDelayedSave.erase(it);

	uint32_t dwID = item->GetID();
	sys_log(2, "ITEM_DESTROY %s:%u", item->GetName(), dwID);

	if (!item->GetSkipSave() && dwID)
	{
		uint32_t dwOwnerID = item->GetLastOwnerPID();

		db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_DESTROY, 0, sizeof(uint32_t) + sizeof(uint32_t));
		db_clientdesc->Packet(&dwID, sizeof(uint32_t));
		db_clientdesc->Packet(&dwOwnerID, sizeof(uint32_t));
	}
	else
	{
		sys_log(2, "ITEM_DESTROY_SKIP %s:%u (skip=%d)", item->GetName(), dwID, item->GetSkipSave());
	}

	if (dwID)
		m_map_pkItemByID.erase(dwID);

	m_VIDMap.erase(item->GetVID());

#ifdef M2_USE_POOL
	pool_.Destroy(item);
#else
#ifndef DEBUG_ALLOC
	M2_DELETE(item);
#else
	M2_DELETE_EX(item, file, line);
#endif
#endif
}

LPITEM ITEM_MANAGER::Find(uint32_t id)
{
	auto it = m_map_pkItemByID.find(id);
	if (it == m_map_pkItemByID.end())
		return nullptr;
	return it->second;
}

LPITEM ITEM_MANAGER::FindByVID(uint32_t vid)
{
	ITEM_VID_MAP::iterator it = m_VIDMap.find(vid);

	if (it == m_VIDMap.end())
		return nullptr;

	return (it->second);
}

SItemTable_Server * ITEM_MANAGER::GetTable(uint32_t vnum)
{
	int32_t rnum = RealNumber(vnum);

	if (rnum < 0)
	{
		for (size_t i = 0; i < m_vec_item_vnum_range_info.size(); i++)
		{
			SItemTable_Server* p = m_vec_item_vnum_range_info[i];
			if ((p->dwVnum < vnum) &&
				vnum < (p->dwVnum + p->dwVnumRange))
			{
				return p;
			}
		}
			
		return nullptr;
	}

	return &m_vec_prototype[rnum];
}

int32_t ITEM_MANAGER::RealNumber(uint32_t vnum)
{
	int32_t bot, top, mid;

	bot = 0;
	top = m_vec_prototype.size();

	SItemTable_Server * pTable = &m_vec_prototype[0];

	while (1)
	{
		mid = (bot + top) >> 1;

		if ((pTable + mid)->dwVnum == vnum)
			return (mid);

		if (bot >= top)
			return (-1);

		if ((pTable + mid)->dwVnum > vnum)
			top = mid - 1;
		else        
			bot = mid + 1;
	}
}

bool ITEM_MANAGER::GetVnum(const char * c_pszName, uint32_t & r_dwVnum)
{
	int32_t len = strlen(c_pszName);

	SItemTable_Server * pTable = &m_vec_prototype[0];

	for (uint32_t i = 0; i < m_vec_prototype.size(); ++i, ++pTable)
	{
		if (!strncasecmp(c_pszName, pTable->szLocaleName, len))
		{
			r_dwVnum = pTable->dwVnum;
			return true;
		}
	}

	return false;
}

bool ITEM_MANAGER::GetVnumByOriginalName(const char * c_pszName, uint32_t & r_dwVnum)
{
	int32_t len = strlen(c_pszName);

	SItemTable_Server * pTable = &m_vec_prototype[0];

	for (uint32_t i = 0; i < m_vec_prototype.size(); ++i, ++pTable)
	{
		if (!strncasecmp(c_pszName, pTable->szName, len))
		{
			r_dwVnum = pTable->dwVnum;
			return true;
		}
	}

	return false;
}


class CItemDropInfo
{
	public:
		CItemDropInfo(int32_t iLevelStart, int32_t iLevelEnd, int32_t iPercent, uint32_t dwVnum) :
			m_iLevelStart(iLevelStart), m_iLevelEnd(iLevelEnd), m_iPercent(iPercent), m_dwVnum(dwVnum)
			{
			}

		int32_t	m_iLevelStart;
		int32_t	m_iLevelEnd;
		int32_t	m_iPercent; // 1 ~ 1000
		uint32_t	m_dwVnum;

		friend bool operator < (const CItemDropInfo & l, const CItemDropInfo & r)
		{
			return l.m_iLevelEnd < r.m_iLevelEnd;
		}
};

extern std::vector<CItemDropInfo> g_vec_pkCommonDropItem[MOB_RANK_MAX_NUM];

// 20050503.ipkn.
// iMinimum 보다 작으면 iDefault 세팅 (단, iMinimum은 0보다 커야함)
// 1, 0 식으로 ON/OFF 되는 방식을 지원하기 위해 존재
int32_t GetDropPerKillPct(int32_t iMinimum, int32_t iDefault, int32_t iDeltaPercent, const char * c_pszFlag)
{
	int32_t iVal = 0;

	if ((iVal = quest::CQuestManager::Instance().GetEventFlag(c_pszFlag)))
	{
		if (!g_bIsTestServer)
		{
			if (iVal < iMinimum)
				iVal = iDefault;

			if (iVal < 0)
				iVal = iDefault;
		}
	}

	if (iVal == 0)
		return 0;

	// 기본 세팅일때 (iDeltaPercent=100) 
	// 40000 iVal 마리당 하나 느낌을 주기 위한 상수임
	return (40000 * iDeltaPercent / iVal);
}

bool ITEM_MANAGER::GetDropPct(LPCHARACTER pkChr, LPCHARACTER pkKiller, OUT int32_t& iDeltaPercent, OUT int32_t& iRandRange)
{
 	if (nullptr == pkChr || nullptr == pkKiller)
		return false;

	int32_t iLevel = pkKiller->GetLevel();
	iDeltaPercent = 100;

	if (!pkChr->IsStone() && pkChr->GetMobRank() >= MOB_RANK_BOSS)
		iDeltaPercent = PERCENT_LVDELTA_BOSS(pkKiller->GetLevel(), pkChr->GetLevel());
	else
		iDeltaPercent = PERCENT_LVDELTA(pkKiller->GetLevel(), pkChr->GetLevel());

	uint8_t bRank = pkChr->GetMobRank();

	if (1 == number(1, 50000))
		iDeltaPercent += 1000;
	else if (1 == number(1, 10000))
		iDeltaPercent += 500;

	sys_log(3, "CreateDropItem for level: %d rank: %u pct: %d", iLevel, bRank, iDeltaPercent);
	iDeltaPercent = iDeltaPercent * CHARACTER_MANAGER::Instance().GetMobItemRate(pkKiller) / 100;

	// ADD_APPLY
	if (pkKiller->GetPoint(POINT_ITEM_DROP_BONUS) > 0)
	{
		iDeltaPercent += pkKiller->GetPoint(POINT_ITEM_DROP_BONUS);
		if (g_bIsTestServer)
			pkKiller->ChatPacket(CHAT_TYPE_INFO, "item drop bonus %d new perc %d", pkKiller->GetPoint(POINT_ITEM_DROP_BONUS), iDeltaPercent);
	}
	// END_OF_ADD_APPLY

	//if (pkKiller->GetPoint(POINT_MALL_ITEMBONUS) > 0)
	//iDeltaPercent += iDeltaPercent * pkKiller->GetPoint(POINT_MALL_ITEMBONUS) / 100;
	// ADD_PREMIUM
	if (pkKiller->GetPremiumRemainSeconds(PREMIUM_ITEM) > 0 ||
			pkKiller->IsEquipUniqueGroup(UNIQUE_GROUP_DOUBLE_ITEM))
		iDeltaPercent += iDeltaPercent;
	// END_OF_ADD_PREMIUM

	iRandRange = 4000000;
	iRandRange = iRandRange * 100 / 
		(100 + 
		 CPrivManager::Instance().GetPriv(pkKiller, PRIV_ITEM_DROP) + 
		 (pkKiller->IsEquipUniqueItem(UNIQUE_ITEM_DOUBLE_ITEM)?100:0));

	return true;
}

bool ITEM_MANAGER::CreateDropItem(LPCHARACTER pkChr, LPCHARACTER pkKiller, std::vector<LPITEM> & vec_item)
{
	int32_t iLevel = pkKiller->GetLevel();

	int32_t iDeltaPercent, iRandRange;
	if (!GetDropPct(pkChr, pkKiller, iDeltaPercent, iRandRange))
		return false;

	uint8_t bRank = pkChr->GetMobRank();
	LPITEM item = nullptr;

	// Common Drop Items
	std::vector<CItemDropInfo>::iterator it = g_vec_pkCommonDropItem[bRank].begin();

	while (it != g_vec_pkCommonDropItem[bRank].end())
	{
		const CItemDropInfo & c_rInfo = *(it++);

		if (iLevel < c_rInfo.m_iLevelStart || iLevel > c_rInfo.m_iLevelEnd)
			continue;

		int32_t iPercent = (c_rInfo.m_iPercent * iDeltaPercent) / 100;
		sys_log(3, "CreateDropItem %d ~ %d %d(%d)", c_rInfo.m_iLevelStart, c_rInfo.m_iLevelEnd, c_rInfo.m_dwVnum, iPercent, c_rInfo.m_iPercent);

		if (iPercent >= number(1, iRandRange))
		{
			SItemTable_Server * table = GetTable(c_rInfo.m_dwVnum);

			if (!table)
				continue;

			item = nullptr;

			if (table->bType == ITEM_POLYMORPH)
			{
				if (c_rInfo.m_dwVnum == pkChr->GetPolymorphItemVnum())
				{
					item = CreateItem(c_rInfo.m_dwVnum, 1, 0, true);

					if (item)
						item->SetSocket(0, pkChr->GetRaceNum());
				}
			}
			else
				item = CreateItem(c_rInfo.m_dwVnum, 1, 0, true);

			if (item) vec_item.push_back(item);
		}
	}

	// Drop Item Group
	{
		auto it2 = m_map_pkDropItemGroup.find(pkChr->GetRaceNum());
		if (it2 != m_map_pkDropItemGroup.end())
		{
			auto v = it2->second->GetVector();

			for (uint32_t i = 0; i < v.size(); ++i)
			{
				int32_t iPercent = (v[i].dwPct * iDeltaPercent) / 100;

				if (iPercent >= number(1, iRandRange))
				{
					item = CreateItem(v[i].dwVnum, v[i].iCount, 0, true);

					if (item)
					{
						if (item->GetType() == ITEM_POLYMORPH)
						{
							if (item->GetVnum() == pkChr->GetPolymorphItemVnum())
							{
								item->SetSocket(0, pkChr->GetRaceNum());
							}
						}

						vec_item.push_back(item);
					}
				}
			}
		}
	}

	// MobDropItem Group
	{
		auto it2 = m_map_pkMobItemGroup.find(pkChr->GetRaceNum());
		if ( it2 != m_map_pkMobItemGroup.end() )
		{
			CMobItemGroup* pGroup = it2->second;

			// MOB_DROP_ITEM_BUG_FIX
			// 에 아이템이 없을 경우 CMobItemGroup::GetOne() 접근시 문제 발생 수정
			if (pGroup && !pGroup->IsEmpty())
			{
				int32_t iPercent = 40000 * iDeltaPercent / pGroup->GetKillPerDrop();
				if (iPercent >= number(1, iRandRange))
				{
					const CMobItemGroup::SMobItemGroupInfo& info = pGroup->GetOne();

					item = CreateItem(info.dwItemVnum, info.iCount, 0, true, info.iRarePct);
					if (item)
						vec_item.push_back(item);
				}
			}
			// END_OF_MOB_DROP_ITEM_BUG_FIX
		}
	}

	// Level Item Group
	{
		auto it2 = m_map_pkLevelItemGroup.find(pkChr->GetRaceNum());

		if ( it2 != m_map_pkLevelItemGroup.end() )
		{
			if ( it2->second->GetLevelLimit() <= (uint32_t)iLevel )
			{
				auto v = it2->second->GetVector();

				for ( uint32_t i=0; i < v.size(); i++ )
				{
					if ( v[i].dwPct >= (uint32_t)number(1, 1000000/*iRandRange*/) )
					{
						uint32_t dwVnum = v[i].dwVNum;

						item = CreateItem(dwVnum, v[i].iCount, 0, true);
						if ( item )
							vec_item.push_back(item);
					}
				}
			}
		}
	}
	
	// BuyerTheitGloves Item Group
	{
		if (pkKiller->GetPremiumRemainSeconds(PREMIUM_ITEM) > 0 ||
				pkKiller->IsEquipUniqueGroup(UNIQUE_GROUP_DOUBLE_ITEM))
		{
			auto it2 = m_map_pkGloveItemGroup.find(pkChr->GetRaceNum());

			if (it2 != m_map_pkGloveItemGroup.end())
			{
				auto v = it2->second->GetVector();

				for (uint32_t i = 0; i < v.size(); ++i)
				{
					int32_t iPercent = (v[i].dwPct * iDeltaPercent) / 100;

					if (iPercent >= number(1, iRandRange))
					{
						uint32_t dwVnum = v[i].dwVnum;

						item = CreateItem(dwVnum, v[i].iCount, 0, true);
						if (item)
							vec_item.push_back(item);
					}
				}
			}
		}
	}
	
	// 잡템
	if (pkChr->GetMobDropItemVnum())
	{
		auto it2 = m_map_dwEtcItemDropProb.find(pkChr->GetMobDropItemVnum());

		if (it2 != m_map_dwEtcItemDropProb.end())
		{
			int32_t iPercent = (it2->second * iDeltaPercent) / 100;

			if (iPercent >= number(1, iRandRange))
			{
				item = CreateItem(pkChr->GetMobDropItemVnum(), 1, 0, true);
				if (item)
					vec_item.push_back(item);
			}
		}
	}

	if (pkChr->IsStone())
	{
		if (pkChr->GetDropMetinStoneVnum())
		{
			int32_t iPercent = (pkChr->GetDropMetinStonePct() * iDeltaPercent) * 400;

			if (iPercent >= number(1, iRandRange))
			{
				item = CreateItem(pkChr->GetDropMetinStoneVnum(), 1, 0, true);
				if (item) vec_item.push_back(item);
			}
		}
	}

	if (pkKiller->IsHorseRiding() && 
			GetDropPerKillPct(1000, 1000000, iDeltaPercent, "horse_skill_book_drop") >= number(1, iRandRange))
	{
		sys_log(0, "EVENT HORSE_SKILL_BOOK_DROP");

		if ((item = CreateItem(ITEM_HORSE_SKILL_TRAIN_BOOK, 1, 0, true)))
			vec_item.push_back(item);
	}


	//
	// 스페셜 드롭 아이템
	// 
	CreateQuestDropItem(pkChr, pkKiller, vec_item, iDeltaPercent, iRandRange);

	for (auto it2 = vec_item.begin(); it2 != vec_item.end(); ++it2)
	{
		LPITEM item2 = *it2;
		LogManager::Instance().MoneyLog(MONEY_LOG_DROP, item2->GetVnum(), item2->GetCount());
	}

	return vec_item.size();
}

// ADD_GRANDMASTER_SKILL
int32_t GetThreeSkillLevelAdjust(int32_t level)
{
	if (level < 40)
		return 32;
	if (level < 45)
		return 16;
	if (level < 50)
		return 8;
	if (level < 55)
		return 4;
	if (level < 60)
		return 2;
	return 1;
}
// END_OF_ADD_GRANDMASTER_SKILL

bool ITEM_MANAGER::GetPossibleItemsToDrop(LPCHARACTER pkChr, std::vector<uint32_t> & s_vec_itemsDrop)
{
	// Drop Item Group
	{
		auto it = m_map_pkDropItemGroup.find(pkChr->GetRaceNum());

		if (it != m_map_pkDropItemGroup.end())
		{
			auto v = it->second->GetVector();

			for (uint32_t i = 0; i < v.size(); ++i)
			{
				s_vec_itemsDrop.push_back(v[i].dwVnum);
			}
		}
	}

	// MobDropItem Group
	{
		auto it = m_map_pkMobItemGroup.find(pkChr->GetRaceNum());

		if (it != m_map_pkMobItemGroup.end())
		{
			CMobItemGroup* pGroup = it->second;

			if (pGroup && !pGroup->IsEmpty())
			{
				const CMobItemGroup::SMobItemGroupInfo& info = pGroup->GetOne();
				s_vec_itemsDrop.push_back(info.dwItemVnum);
			}
			// END_OF_MOB_DROP_ITEM_BUG_FIX
		}
	}

	// Level Item Group
	{
		auto it = m_map_pkLevelItemGroup.find(pkChr->GetRaceNum());

		if (it != m_map_pkLevelItemGroup.end())
		{
			auto v = it->second->GetVector();
			for (uint32_t i = 0; i < v.size(); i++)
			{
				uint32_t dwVnum = v[i].dwVNum;
				s_vec_itemsDrop.push_back(dwVnum);
			}
		}
	}

	// BuyerTheitGloves Item Group
	{
		auto it = m_map_pkGloveItemGroup.find(pkChr->GetRaceNum());

		if (it != m_map_pkGloveItemGroup.end())
		{
			auto v = it->second->GetVector();

			for (uint32_t i = 0; i < v.size(); ++i)
			{
				uint32_t dwVnum = v[i].dwVnum;
				s_vec_itemsDrop.push_back(dwVnum);
			}
		}
	}

	// 잡템
	if (pkChr->GetMobDropItemVnum())
	{
		auto it = m_map_dwEtcItemDropProb.find(pkChr->GetMobDropItemVnum());

		if (it != m_map_dwEtcItemDropProb.end())
		{
			s_vec_itemsDrop.push_back(pkChr->GetMobDropItemVnum());
		}
	}

	return s_vec_itemsDrop.size();
}

void ITEM_MANAGER::GetChestItemList(uint32_t dwChestVnum, std::vector<TChestDropInfoTable>& vec_item)
{
	TChestDropInfoTable kTempTab;

	const uint8_t dwChestDropPageCount = 5;
	CGrid* pGrids[dwChestDropPageCount];

	for (uint8_t i = 0; i < dwChestDropPageCount; ++i)
	{
		pGrids[i] = new CGrid(15, 5);
		pGrids[i]->Clear();
	}

	const CSpecialItemGroup* pGroup = GetSpecialItemGroup(dwChestVnum);

	if (pGroup)
	{
		for (int32_t i = 0; i < pGroup->GetGroupSize(); i++)
		{
			const SItemTable_Server* itemTable = GetTable(pGroup->GetVnum(i));

			if (itemTable)
			{
				for (uint8_t iPage = 0; iPage < dwChestDropPageCount; ++iPage)
				{
					int32_t iPos = pGrids[iPage]->FindBlank(1, itemTable->bSize);

					if (iPos >= 0)
					{
						pGrids[iPage]->Put(iPos, 1, itemTable->bSize);

						kTempTab.bPageIndex = iPage + 1;
						kTempTab.bSlotIndex = iPos;

						kTempTab.bItemCount = pGroup->GetCount(i);
						kTempTab.dwItemVnum = pGroup->GetVnum(i);

						vec_item.push_back(kTempTab);

						break;
					}
				}
			}
		}
	}

	for (uint8_t i = 0; i < dwChestDropPageCount; ++i)
	{
		if (pGrids[i])
			M2_DELETE(pGrids[i]);
	}
}

// DROPEVENT_CHARSTONE
// drop_char_stone 1
// drop_char_stone.percent_lv01_10 5
// drop_char_stone.percent_lv11_30 10
// drop_char_stone.percent_lv31_MX 15
// drop_char_stone.level_range	   10
static struct DropEvent_CharStone
{
	int32_t percent_lv01_10;
	int32_t percent_lv11_30;
	int32_t percent_lv31_MX;
	int32_t level_range;
	bool alive;

	DropEvent_CharStone()
	{
		percent_lv01_10 =  100;
		percent_lv11_30 =  200;
		percent_lv31_MX =  300;
		level_range = 10;
		alive = false;
	}
} gs_dropEvent_charStone;

static int32_t __DropEvent_CharStone_GetDropPercent(int32_t killer_level)
{
	int32_t killer_levelStep = (killer_level-1)/10;

	switch (killer_levelStep)
	{
		case 0:
			return gs_dropEvent_charStone.percent_lv01_10;

		case 1:
		case 2:
			return gs_dropEvent_charStone.percent_lv11_30;
	}

	return gs_dropEvent_charStone.percent_lv31_MX;
}

static void __DropEvent_CharStone_DropItem(CHARACTER & killer, CHARACTER & victim, ITEM_MANAGER& itemMgr, std::vector<LPITEM>& vec_item)
{
	if (!gs_dropEvent_charStone.alive)
		return;

	int32_t killer_level = killer.GetLevel();
	int32_t dropPercent = __DropEvent_CharStone_GetDropPercent(killer_level);

	int32_t MaxRange = 10000;

	if (number(1, MaxRange) <= dropPercent)
	{
		int32_t log_level = (g_bIsTestServer || killer.GetGMLevel() >= GM_LOW_WIZARD) ? 0 : 1;
		int32_t victim_level = victim.GetLevel();
		int32_t level_diff = victim_level - killer_level;

		if (level_diff >= +gs_dropEvent_charStone.level_range || level_diff <= -gs_dropEvent_charStone.level_range)
		{
			sys_log(log_level, 
					"dropevent.drop_char_stone.level_range_over: killer(%s: lv%d), victim(%s: lv:%d), level_diff(%d)",
					killer.GetName(), killer.GetLevel(), victim.GetName(), victim.GetLevel(), level_diff);	
			return;
		}

		static const int32_t Stones[] = { 30210, 30211, 30212, 30213, 30214, 30215, 30216, 30217, 30218, 30219, 30258, 30259, 30260, 30261, 30262, 30263 };
		int32_t item_vnum = Stones[number(0, _countof(Stones))];

		LPITEM p_item = nullptr;

		if ((p_item = itemMgr.CreateItem(item_vnum, 1, 0, true)))
		{
			vec_item.push_back(p_item);

			sys_log(log_level, 
					"dropevent.drop_char_stone.item_drop: killer(%s: lv%d), victim(%s: lv:%d), item_name(%s)",
					killer.GetName(), killer.GetLevel(), victim.GetName(), victim.GetLevel(), p_item->GetName());	
		}
	}
}

bool DropEvent_CharStone_SetValue(const std::string& name, int32_t value)
{
	if (name == "drop_char_stone")
	{
		gs_dropEvent_charStone.alive = value;

		if (value)
			sys_log(0, "dropevent.drop_char_stone = on");
		else
			sys_log(0, "dropevent.drop_char_stone = off");

	}
	else if (name == "drop_char_stone.percent_lv01_10")
		gs_dropEvent_charStone.percent_lv01_10 = value;
	else if (name == "drop_char_stone.percent_lv11_30")
		gs_dropEvent_charStone.percent_lv11_30 = value;
	else if (name == "drop_char_stone.percent_lv31_MX")
		gs_dropEvent_charStone.percent_lv31_MX = value;
	else if (name == "drop_char_stone.level_range")
		gs_dropEvent_charStone.level_range = value;
	else
		return false;

	sys_log(0, "dropevent.drop_char_stone: %d", gs_dropEvent_charStone.alive ? true : false);
	sys_log(0, "dropevent.drop_char_stone.percent_lv01_10: %f", gs_dropEvent_charStone.percent_lv01_10/100.0f);
	sys_log(0, "dropevent.drop_char_stone.percent_lv11_30: %f", gs_dropEvent_charStone.percent_lv11_30/100.0f);
	sys_log(0, "dropevent.drop_char_stone.percent_lv31_MX: %f", gs_dropEvent_charStone.percent_lv31_MX/100.0f);
	sys_log(0, "dropevent.drop_char_stone.level_range: %d", gs_dropEvent_charStone.level_range);

	return true;
}

// END_OF_DROPEVENT_CHARSTONE

// fixme
// 위의 것과 함께 quest로 뺄것 빼보자. 
// 이거 너무 더럽잖아...
// 뷁.. 하드코딩 싫다 ㅜㅠ
// 계량 아이템 보상 시작.
// by rtsummit 고치자 진짜
static struct DropEvent_RefineBox
{
	int32_t percent_low;
	int32_t low;
	int32_t percent_mid;
	int32_t mid;
	int32_t percent_high;
	//int32_t level_range;
	bool alive;

	DropEvent_RefineBox()
	{
		percent_low =  100;
		low = 20;
		percent_mid =  100;
		mid = 45;
		percent_high =  100;
		//level_range = 10;
		alive = false;
	}
} gs_dropEvent_refineBox;

static LPITEM __DropEvent_RefineBox_GetDropItem(CHARACTER & killer, CHARACTER & victim, ITEM_MANAGER& itemMgr)
{
	static const int32_t lowerBox[] = { 50197, 50198, 50199 };
	static const int32_t lowerBox_range = 3;
	static const int32_t midderBox[] = { 50203, 50204, 50205, 50206 };
	static const int32_t midderBox_range = 4;
	static const int32_t higherBox[] = { 50207, 50208, 50209, 50210, 50211 };
	static const int32_t higherBox_range = 5;

	if (victim.GetMobRank() < MOB_RANK_KNIGHT)
		return nullptr;

	int32_t killer_level = killer.GetLevel();
	//int32_t level_diff = victim_level - killer_level;

	//if (level_diff >= +gs_dropEvent_refineBox.level_range || level_diff <= -gs_dropEvent_refineBox.level_range)
	//{
	//	sys_log(log_level, 
	//		"dropevent.drop_refine_box.level_range_over: killer(%s: lv%d), victim(%s: lv:%d), level_diff(%d)",
	//		killer.GetName(), killer.GetLevel(), victim.GetName(), victim.GetLevel(), level_diff);	
	//	return nullptr;
	//}

	if (killer_level <= gs_dropEvent_refineBox.low)
	{
		if (number (1, gs_dropEvent_refineBox.percent_low) == 1)
		{
			return itemMgr.CreateItem(lowerBox [number (1,lowerBox_range) - 1], 1, 0, true);
		}
	}
	else if (killer_level <= gs_dropEvent_refineBox.mid)
	{
		if (number (1, gs_dropEvent_refineBox.percent_mid) == 1)
		{
			return itemMgr.CreateItem(midderBox [number (1,midderBox_range) - 1], 1, 0, true);
		}
	}
	else
	{
		if (number (1, gs_dropEvent_refineBox.percent_high) == 1)
		{
			return itemMgr.CreateItem(higherBox [number (1,higherBox_range) - 1], 1, 0, true);
		}
	}
	return nullptr;
}

static void __DropEvent_RefineBox_DropItem(CHARACTER & killer, CHARACTER & victim, ITEM_MANAGER& itemMgr, std::vector<LPITEM>& vec_item)
{
	if (!gs_dropEvent_refineBox.alive)
		return;

	int32_t log_level = (g_bIsTestServer || killer.GetGMLevel() >= GM_LOW_WIZARD) ? 0 : 1;

	LPITEM p_item = __DropEvent_RefineBox_GetDropItem(killer, victim, itemMgr);

	if (p_item)
	{
		vec_item.push_back(p_item);

		sys_log(log_level, 
			"dropevent.drop_refine_box.item_drop: killer(%s: lv%d), victim(%s: lv:%d), item_name(%s)",
			killer.GetName(), killer.GetLevel(), victim.GetName(), victim.GetLevel(), p_item->GetName());	
	}
}

bool DropEvent_RefineBox_SetValue(const std::string& name, int32_t value)
{
	if (name == "refine_box_drop")
	{
		gs_dropEvent_refineBox.alive = value;

		if (value)
			sys_log(0, "refine_box_drop = on");
		else
			sys_log(0, "refine_box_drop = off");

	}
	else if (name == "refine_box_low")
		gs_dropEvent_refineBox.percent_low = value < 100 ? 100 : value;
	else if (name == "refine_box_mid")
		gs_dropEvent_refineBox.percent_mid = value < 100 ? 100 : value;
	else if (name == "refine_box_high")
		gs_dropEvent_refineBox.percent_high = value < 100 ? 100 : value;
	//else if (name == "refine_box_level_range")
	//	gs_dropEvent_refineBox.level_range = value;
	else
		return false;

	sys_log(0, "refine_box_drop: %d", gs_dropEvent_refineBox.alive ? true : false);
	sys_log(0, "refine_box_low: %d", gs_dropEvent_refineBox.percent_low);
	sys_log(0, "refine_box_mid: %d", gs_dropEvent_refineBox.percent_mid);
	sys_log(0, "refine_box_high: %d", gs_dropEvent_refineBox.percent_high);
	//sys_log(0, "refine_box_low_level_range: %d", gs_dropEvent_refineBox.level_range);

	return true;
}
// 개량 아이템 보상 끝.


void ITEM_MANAGER::CreateQuestDropItem(LPCHARACTER pkChr, LPCHARACTER pkKiller, std::vector<LPITEM> & vec_item, int32_t iDeltaPercent, int32_t iRandRange)
{
	LPITEM item = nullptr;

	if (!pkChr)
		return;

	if (!pkKiller)
		return;

	sys_log(1, "CreateQuestDropItem victim(%s), killer(%s)", pkChr->GetName(), pkKiller->GetName() );

	// DROPEVENT_CHARSTONE
	__DropEvent_CharStone_DropItem(*pkKiller, *pkChr, *this, vec_item);
	// END_OF_DROPEVENT_CHARSTONE
	__DropEvent_RefineBox_DropItem(*pkKiller, *pkChr, *this, vec_item);

	// corap drop
	if (quest::CQuestManager::Instance().GetEventFlag("xmas_sock"))
	{
		const uint32_t SOCK_ITEM_VNUM = 50010;

		int32_t iDropPerKill[MOB_RANK_MAX_NUM] =
		{
			2000,
			1000,
			300,
			50,
			0,
			0,
		};

		if ( iDropPerKill[pkChr->GetMobRank()] != 0 )
		{
			int32_t iPercent = 40000 * iDeltaPercent / iDropPerKill[pkChr->GetMobRank()];

			sys_log(0, "CORAP DROP %d %d", iPercent, iRandRange);
			if (iPercent >= number(1, iRandRange))
			{
				if ((item = CreateItem(SOCK_ITEM_VNUM, 1, 0, true)))
					vec_item.push_back(item);
			}
		}
	}

	// ay isigi define sandigi
	if (quest::CQuestManager::Instance().GetEventFlag("drop_moon"))
	{
		const uint32_t ITEM_VNUM = 50011;

		int32_t iDropPerKill[MOB_RANK_MAX_NUM] =
		{
			2000,
			1000,
			300,
			50,
			0,
			0,
		};

		if (iDropPerKill[pkChr->GetMobRank()])
		{
			int32_t iPercent = 40000 * iDeltaPercent / iDropPerKill[pkChr->GetMobRank()];

			if (iPercent >= number(1, iRandRange))
			{
				if ((item = CreateItem(ITEM_VNUM, 1, 0, true)))
					vec_item.push_back(item);
			}
		}
	}

	// oyma tas
	if (pkKiller->GetLevel() >= 15 && abs(pkKiller->GetLevel() - pkChr->GetLevel()) <= 5)
	{
		int32_t pct = quest::CQuestManager::Instance().GetEventFlag("hc_drop");

		if (pct > 0)
		{
			const uint32_t ITEM_VNUM = 30178;

			if (number(1,100) <= pct)
			{
				if ((item = CreateItem(ITEM_VNUM, 1, 0, true)))
					vec_item.push_back(item);
			}
		}
	}

	// altigen hediye paketi
	if (GetDropPerKillPct(100, 2000, iDeltaPercent, "2006_drop") >= number(1, iRandRange))
	{
		sys_log(0, "ALTIGEN DROP EVENT ");

		const static uint32_t dwVnum = 50037;

		if ((item = CreateItem(dwVnum, 1, 0, true)))
			vec_item.push_back(item);

	}

	//mevcut degil
	if (GetDropPerKillPct(100, 2000, iDeltaPercent, "2007_drop") >= number(1, iRandRange))
	{
		sys_log(0, "HATALI ITEM DROP EVENT ");

		const static uint32_t dwVnum = 50043;

		if ((item = CreateItem(dwVnum, 1, 0, true)))
			vec_item.push_back(item);
	}

	// havai fisek
	if (GetDropPerKillPct(/* minimum */ 100, /* default */ 1000, iDeltaPercent, "newyear_fire") >= number(1, iRandRange))
	{
		const uint32_t havai_fisek[3] = { 50106, 50107, 50108, };
		uint32_t dwVnum = havai_fisek[number(0, 2)];

		if ((item = CreateItem(dwVnum, 1, 0, true)))
			vec_item.push_back(item);
	}

	// fasulyeli hamur vs
	if (GetDropPerKillPct(100, 500, iDeltaPercent, "newyear_moon") >= number(1, iRandRange))
	{
		sys_log(0, "EVENT NEWYEAR_MOON DROP");

		const static uint32_t wonso_items[6] = { 50016, 50017, 50018, 50019, 50019, 50019, };
		uint32_t dwVnum = wonso_items[number(0,5)];

		if ((item = CreateItem(dwVnum, 1, 0, true)))
			vec_item.push_back(item);
	}

	// gul cikolata - sevgililer gunu icin
	if (GetDropPerKillPct(1, 2000, iDeltaPercent, "valentine_drop") >= number(1, iRandRange))
	{
		sys_log(0, "EVENT VALENTINE_DROP");

		const static uint32_t valentine_items[2] = { 50024, 50025 };
		uint32_t dwVnum = valentine_items[number(0, 1)];

		if ((item = CreateItem(dwVnum, 1, 0, true)))
			vec_item.push_back(item);
	}

	// dondurma -  mevcut degil
	if (GetDropPerKillPct(100, 2000, iDeltaPercent, "icecream_drop") >= number(1, iRandRange))
	{
		const static uint32_t icecream = 50123;

		if ((item = CreateItem(icecream, 1, 0, true)))
			vec_item.push_back(item);
	}

	// new 크리스마스 이벤트
	// 53002 : 아기 순록 소환권
	if ((pkKiller->CountSpecifyItem(53002) > 0) && (GetDropPerKillPct(50, 100, iDeltaPercent, "new_xmas_event") >= number(1, iRandRange)))
	{
		const static uint32_t xmas_sock = 50010;
		pkKiller->AutoGiveItem (xmas_sock, 1);
	}

	if ((pkKiller->CountSpecifyItem(53007) > 0) && (GetDropPerKillPct(50, 100, iDeltaPercent, "new_xmas_event") >= number(1, iRandRange)))
	{
		const static uint32_t xmas_sock = 50010;
		pkKiller->AutoGiveItem (xmas_sock, 1);
	}

	// simya
	if (pkChr->GetLevel() >= 30 && (GetDropPerKillPct(50, 100, iDeltaPercent, "ds_drop") >= number(1, iRandRange)))
	{
		const static uint32_t dragon_soul_gemstone = 30270;
		if ((item = CreateItem(dragon_soul_gemstone, 1, 0, true)))
			vec_item.push_back(item);
	}

	if ( GetDropPerKillPct(100, 2000, iDeltaPercent, "halloween_drop") >= number(1, iRandRange) )
	{
		const static uint32_t halloween_item = 30321;

		if ( (item=CreateItem(halloween_item, 1, 0, true)) )
			vec_item.push_back(item);
	}
	
	if ( GetDropPerKillPct(100, 2000, iDeltaPercent, "ramadan_drop") >= number(1, iRandRange) )
	{
		const static uint32_t ramadan_item = 30315;

		if ( (item=CreateItem(ramadan_item, 1, 0, true)) )
			vec_item.push_back(item);
	}

	if ( GetDropPerKillPct(100, 2000, iDeltaPercent, "easter_drop") >= number(1, iRandRange) )
	{
		const static uint32_t easter_item_base = 50160;

		if ( (item=CreateItem(easter_item_base+number(0,19), 1, 0, true)) )
			vec_item.push_back(item);
	}

	// 월드컵 이벤트
	if ( GetDropPerKillPct(100, 2000, iDeltaPercent, "football_drop") >= number(1, iRandRange) )
	{
		const static uint32_t football_item = 50096;

		if ( (item=CreateItem(football_item, 1, 0, true)) )
			vec_item.push_back(item);
	}

	// 화이트 데이 이벤트
	if (GetDropPerKillPct(100, 2000, iDeltaPercent, "whiteday_drop") >= number(1, iRandRange))
	{
		sys_log(0, "EVENT WHITEDAY_DROP");
		const static uint32_t whiteday_items[2] = { ITEM_WHITEDAY_ROSE, ITEM_WHITEDAY_CANDY };
		uint32_t dwVnum = whiteday_items[number(0,1)];

		if ((item = CreateItem(dwVnum, 1, 0, true)))
			vec_item.push_back(item);
	}

	// 어린이날 수수께끼 상자 이벤트
	if (pkKiller->GetLevel()>=50)
	{
		if (GetDropPerKillPct(100, 1000, iDeltaPercent, "kids_day_drop_high") >= number(1, iRandRange))
		{
			uint32_t ITEM_QUIZ_BOX = 50034;

			if ((item = CreateItem(ITEM_QUIZ_BOX, 1, 0, true)))
				vec_item.push_back(item);
		}
	}
	else
	{
		if (GetDropPerKillPct(100, 1000, iDeltaPercent, "kids_day_drop") >= number(1, iRandRange))
		{
			uint32_t ITEM_QUIZ_BOX = 50034;

			if ((item = CreateItem(ITEM_QUIZ_BOX, 1, 0, true)))
				vec_item.push_back(item);
		}
	}

	// 올림픽 드롭 이벤트
	if (pkChr->GetLevel() >= 30 && GetDropPerKillPct(50, 100, iDeltaPercent, "medal_part_drop") >= number(1, iRandRange))
	{
		const static uint32_t drop_items[] = { 30265, 30266, 30267, 30268, 30269 };
		int32_t i = number (0, 4);
		item = CreateItem(drop_items[i]);
		if (item != nullptr)
			vec_item.push_back(item);
	}

	// ADD_GRANDMASTER_SKILL
	// 혼석 아이템 드롭
	if (pkChr->GetLevel() >= 40 && pkChr->GetMobRank() >= MOB_RANK_BOSS && GetDropPerKillPct(/* minimum */ 1, /* default */ 1000, iDeltaPercent, "three_skill_item") / GetThreeSkillLevelAdjust(pkChr->GetLevel()) >= number(1, iRandRange))
	{
		const uint32_t ITEM_VNUM = 50513;

		if ((item = CreateItem(ITEM_VNUM, 1, 0, true)))
			vec_item.push_back(item);
	}
	// END_OF_ADD_GRANDMASTER_SKILL

	//
	// 종자 아이템 drop
	//
	if (GetDropPerKillPct(100, 1000, iDeltaPercent, "dragon_boat_festival_drop") >= number(1, iRandRange))
	{
		const uint32_t ITEM_SEED = 50085;

		if ((item = CreateItem(ITEM_SEED, 1, 0, true)))
			vec_item.push_back(item);
	}

	// 무신의 축복서용 만년한철 drop
	if (pkKiller->GetLevel() >= 15 && quest::CQuestManager::Instance().GetEventFlag("mars_drop"))
	{
		const uint32_t ITEM_HANIRON = 70035;
		int32_t iDropMultiply[MOB_RANK_MAX_NUM] =
		{
			50,
			30,
			5,
			1,
			0,
			0,
		};

		if (iDropMultiply[pkChr->GetMobRank()] &&
				GetDropPerKillPct(1000, 1500, iDeltaPercent, "mars_drop") >= number(1, iRandRange) * iDropMultiply[pkChr->GetMobRank()])
		{
			if ((item = CreateItem(ITEM_HANIRON, 1, 0, true)))
				vec_item.push_back(item);
		}
	}
}

uint32_t ITEM_MANAGER::GetRefineFromVnum(uint32_t dwVnum)
{
	auto it = m_map_ItemRefineFrom.find(dwVnum);
	if (it != m_map_ItemRefineFrom.end())
		return it->second;
	return 0;
}

const CSpecialItemGroup* ITEM_MANAGER::GetSpecialItemGroup(uint32_t dwVnum)
{
	auto it = m_map_pkSpecialItemGroup.find(dwVnum);
	if (it != m_map_pkSpecialItemGroup.end())
	{
		return it->second;
	}
	return nullptr;
}

const CSpecialAttrGroup* ITEM_MANAGER::GetSpecialAttrGroup(uint32_t dwVnum)
{
	auto it = m_map_pkSpecialAttrGroup.find(dwVnum);
	if (it != m_map_pkSpecialAttrGroup.end())
	{
		return it->second;
	}
	return nullptr;
}

uint32_t ITEM_MANAGER::GetMaskVnum(uint32_t dwVnum)
{
	TMapDW2DW::iterator it = m_map_new_to_ori.find (dwVnum);
	if (it != m_map_new_to_ori.end())
	{
		return it->second;
	}
	else
		return 0;
}

// pkNewItem으로 모든 속성과 소켓 값들을 목사하는 함수.
// 기존에 char_item.cpp 파일에 있던 로컬함수인 TransformRefineItem 그대로 복사함
void ITEM_MANAGER::CopyAllAttrTo(LPITEM pkOldItem, LPITEM pkNewItem)
{
	// ACCESSORY_REFINE
	if (pkOldItem->IsAccessoryForSocket())
	{
		for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		{
			pkNewItem->SetSocket(i, pkOldItem->GetSocket(i));
		}
		//pkNewItem->StartAccessorySocketExpireEvent();
	}
	// END_OF_ACCESSORY_REFINE
	else
	{
		// 여기서 깨진석이 자동적으로 청소 됨
		for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		{
			if (!pkOldItem->GetSocket(i))
				break;
			else
				pkNewItem->SetSocket(i, 1);
		}

		// 소켓 설정
		int32_t slot = 0;

		for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		{
			int32_t socket = pkOldItem->GetSocket(i);
			const int32_t ITEM_BROKEN_METIN_VNUM = 28960; // 이건 뭐 똑같은 상수가 3군데나 있냐... 하나로 해놓지ㅠㅠㅠ 나는 패스 홍이 할꺼임
			if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
				pkNewItem->SetSocket(slot++, socket);
		}

	}

	// 매직 아이템 설정
	pkOldItem->CopyAttributeTo(pkNewItem);
}

void ITEM_MANAGER::DestroyMobDropItem()
{
	if (!m_map_pkMobItemGroup.empty())
		m_map_pkMobItemGroup.clear();

	if (!m_map_pkDropItemGroup.empty())
		m_map_pkDropItemGroup.clear();

	if (!m_map_pkLevelItemGroup.empty())
		m_map_pkLevelItemGroup.clear();

	if (!m_map_pkGloveItemGroup.empty())
		m_map_pkGloveItemGroup.clear();
}

