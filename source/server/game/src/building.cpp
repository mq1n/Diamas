#include "stdafx.h"
#include "constants.h"
#include "sectree_manager.h"
#include "item_manager.h"
#include "buffer_manager.h"
#include "config.h"
#include "packet.h"
#include "char.h"
#include "char_manager.h"
#include "guild.h"
#include "guild_manager.h"
#include "desc.h"
#include "desc_manager.h"
#include "desc_client.h"
#include "quest_manager.h"
#include "building.h"

using namespace building;

CObject::CObject(TObject * pData, TObjectProto * pProto)
	: m_pProto(pProto), m_dwVID(0), m_chNPC(nullptr)
{
	CEntity::Initialize(ENTITY_OBJECT);

	memcpy(&m_data, pData, sizeof(TObject));
}

CObject::~CObject()
{
	Destroy();
}

void CObject::Destroy()
{
	if (m_pProto)
	{
		SECTREE_MANAGER::instance().ForAttrRegion(GetMapIndex(),
				GetX() + m_pProto->lRegion[0],
				GetY() + m_pProto->lRegion[1],
				GetX() + m_pProto->lRegion[2],
				GetY() + m_pProto->lRegion[3],
			m_data.xRot, m_data.yRot, m_data.zRot,
				ATTR_OBJECT,
				ATTR_REGION_MODE_REMOVE);
	}

	CEntity::Destroy();

	if (GetSectree())
		GetSectree()->RemoveEntity(this);

	RemoveSpecialEffect();
	// END_OF_BUILDING_NPC
}

// BUILDING_NPC
void CObject::Reconstruct(uint32_t dwVnum)
{
	const TMapRegion * r = SECTREE_MANAGER::instance().GetMapRegion(m_data.lMapIndex);
	if (!r)
		return;

	CLand* pLand = GetLand();
	pLand->RequestDeleteObject(GetID());
	pLand->RequestCreateObject(dwVnum, m_data.lMapIndex, m_data.x - r->sx, m_data.y - r->sy, m_data.xRot, m_data.yRot, m_data.zRot, false);
}
// END_OF_BUILDING_NPC

void CObject::EncodeInsertPacket(LPENTITY entity)
{
	LPDESC d = entity->GetDesc();
	if (!d)
		return;


	sys_log(0, "ObjectInsertPacket vid %u vnum %u rot %f %f %f",
			m_dwVID, m_data.dwVnum, m_data.xRot, m_data.yRot, m_data.zRot);
	bool setGuildID = (m_data.dwVnum == 14200) || (m_pProto && m_pProto->dwNPCVnum != 0);

	TPacketGCCharacterAdd pack;

	memset(&pack, 0, sizeof(TPacketGCCharacterAdd));

	pack.header         = HEADER_GC_CHARACTER_ADD;
	pack.dwVID          = m_dwVID;
	pack.bType          = CHAR_TYPE_BUILDING;
	pack.angle          = m_data.zRot;
	pack.x              = GetX();
	pack.y              = GetY();
	pack.z              = GetZ();
	pack.dwRaceNum      = m_data.dwVnum;
	pack.guildID		= setGuildID ? GetGuildID() : 0;
	pack.level			= 0;
	// ���� ȸ�� ����(���϶��� �� ��ġ)�� ��ȯ
	pack.dwAffectFlag[0] = uint32_t(m_data.xRot);
	pack.dwAffectFlag[1] = uint32_t(m_data.yRot);

	d->Packet(&pack, sizeof(pack));
}

void CObject::EncodeRemovePacket(LPENTITY entity)
{
	LPDESC d = entity->GetDesc();
	if (!d)
		return;

	sys_log(0, "ObjectRemovePacket vid %u", m_dwVID);

	TPacketGCCharacterDelete pack;

	pack.header = HEADER_GC_CHARACTER_DEL;
	pack.id     = m_dwVID;

	d->Packet(&pack, sizeof(TPacketGCCharacterDelete));
}

void CObject::SetVID(uint32_t dwVID)
{
	m_dwVID = dwVID;
}

bool CObject::Show(int32_t lMapIndex, int32_t x, int32_t y)
{
	LPSECTREE tree = SECTREE_MANAGER::instance().Get(lMapIndex, x, y);

	if (!tree)
	{
		sys_err("cannot find sectree by %dx%d mapindex %d id %u vnum %u", x, y, lMapIndex, GetID(), GetVnum());
		return false;
	}

	if (GetSectree())
	{
		GetSectree()->RemoveEntity(this);
		ViewCleanup();
	}

	m_data.lMapIndex = lMapIndex;
	m_data.x = x;
	m_data.y = y;

	Save();

	SetMapIndex(lMapIndex);
	SetXYZ(x, y, 0);

	tree->InsertEntity(this);
	UpdateSectree();

	SECTREE_MANAGER::instance().ForAttrRegion(GetMapIndex(),
			x + m_pProto->lRegion[0],
			y + m_pProto->lRegion[1],
			x + m_pProto->lRegion[2],
			y + m_pProto->lRegion[3],
		m_data.xRot, m_data.yRot, m_data.zRot,
			ATTR_OBJECT,
			ATTR_REGION_MODE_SET);

	return true;
}

void CObject::Save()
{
}

void CObject::ApplySpecialEffect()
{
	if (m_pProto)
	{
		// ADD_SUPPLY_BUILDING
		if (m_pProto->dwVnum == BUILDING_INCREASE_GUILD_MEMBER_COUNT_SMALL ||
				m_pProto->dwVnum == BUILDING_INCREASE_GUILD_MEMBER_COUNT_MEDIUM ||
				m_pProto->dwVnum == BUILDING_INCREASE_GUILD_MEMBER_COUNT_LARGE)
		{
			CLand* pLand = GetLand();
			uint32_t guild_id = 0;
			if (pLand)
				guild_id = pLand->GetOwner();
			CGuild* pGuild = CGuildManager::instance().FindGuild(guild_id);
			if (pGuild)
			{
				switch (m_pProto->dwVnum)
				{
					case BUILDING_INCREASE_GUILD_MEMBER_COUNT_SMALL:
						pGuild->SetMemberCountBonus(6);
						break;
					case BUILDING_INCREASE_GUILD_MEMBER_COUNT_MEDIUM:
						pGuild->SetMemberCountBonus(12);
						break;
					case BUILDING_INCREASE_GUILD_MEMBER_COUNT_LARGE:
						pGuild->SetMemberCountBonus(18);
						break;
				}
				if (pLand && map_allow_find(pLand->GetMapIndex()))
				{
					pGuild->BroadcastMemberCountBonus();
				}
			}
		}
		// END_OF_ADD_SUPPLY_BUILDING
	}
}

void CObject::RemoveSpecialEffect()
{
	if (m_pProto)
	{
		// ADD_SUPPLY_BUILDING
		if (m_pProto->dwVnum == BUILDING_INCREASE_GUILD_MEMBER_COUNT_SMALL ||
				m_pProto->dwVnum == BUILDING_INCREASE_GUILD_MEMBER_COUNT_MEDIUM ||
				m_pProto->dwVnum == BUILDING_INCREASE_GUILD_MEMBER_COUNT_LARGE)
		{
			CLand* pLand = GetLand();
			uint32_t guild_id = 0;
			if (pLand)
				guild_id = pLand->GetOwner();
			CGuild* pGuild = CGuildManager::instance().FindGuild(guild_id);
			if (pGuild)
			{
				pGuild->SetMemberCountBonus(0);
				if (pLand && map_allow_find(pLand->GetMapIndex()))
					pGuild->BroadcastMemberCountBonus();
			}
		}
		// END_OF_ADD_SUPPLY_BUILDING
	}
}

// BUILDING_NPC
void CObject::RegenNPC()
{
	if (!m_pProto)
		return;

	if (!m_pProto->dwNPCVnum)
		return;

	if (!m_pkLand)
		return;

	uint32_t dwGuildID = m_pkLand->GetOwner();
	CGuild* pGuild = CGuildManager::instance().FindGuild(dwGuildID);

	if (!pGuild)
		return;

	int32_t x = m_pProto->lNPCX;
	int32_t y = m_pProto->lNPCY;
	int32_t newX, newY;

	double rot = m_data.zRot * 2.0 * M_PI / 360.0;

	newX = static_cast<int32_t>(( x * cos(rot)) + ( y * sin(rot)));
	newY = static_cast<int32_t>(( y * cos(rot)) - ( x * sin(rot)));

	m_chNPC = CHARACTER_MANAGER::instance().SpawnMob(m_pProto->dwNPCVnum,
			GetMapIndex(),
			GetX() + newX,
			GetY() + newY,
			GetZ(),
			false,
			static_cast<int32_t>(m_data.zRot));


	if (!m_chNPC)
	{
		sys_err("Cannot create guild npc");
		return;
	}

	m_chNPC->SetGuild(pGuild);

	// ���� ������ ��� ��� ������ �渶���� �����س��´�
	if ( m_pProto->dwVnum == 14061 || m_pProto->dwVnum == 14062 || m_pProto->dwVnum == 14063 )
	{
		quest::PC* pPC = quest::CQuestManager::instance().GetPC(pGuild->GetMasterPID());

		if ( pPC != nullptr )
		{
			pPC->SetFlag("alter_of_power.build_level", pGuild->GetLevel());
		}
	}
}
// END_OF_BUILDING_NPC

////////////////////////////////////////////////////////////////////////////////////

CLand::CLand(TLand * pData)
{
	memcpy(&m_data, pData, sizeof(TLand));
}

CLand::~CLand()
{
	Destroy();
}

void CLand::Destroy()
{
	auto it = m_map_pkObject.begin();

	while (it != m_map_pkObject.end())
	{
		LPOBJECT pkObj = (it++)->second;
		CManager::instance().UnregisterObject(pkObj);
		M2_DELETE(pkObj);
	}

	m_map_pkObject.clear();
	m_map_pkObjectByVID.clear();
}

const TLand & CLand::GetData() const
{
	return m_data;
}

void CLand::PutData(const TLand * data)
{
	std::memcpy(&m_data, data, sizeof(m_data));

	if (!m_data.dwGuildID)
		return;

	const TMapRegion * r = SECTREE_MANAGER::instance().GetMapRegion(m_data.lMapIndex);
	if (!r)
		return;

	const auto chars = CHARACTER_MANAGER::instance().GetCharactersByRaceNum(20040);
	for (auto ch : chars)
	{
		if (ch->GetMapIndex() != m_data.lMapIndex)
			continue;

		int32_t x = ch->GetX();
		int32_t y = ch->GetY();

		if (x > m_data.x + m_data.width || x < m_data.x)
			continue;

		if (y > m_data.y + m_data.height || y < m_data.y)
			continue;

		M2_DESTROY_CHARACTER(ch);
	}
}

void CLand::InsertObject(LPOBJECT pkObj)
{
	m_map_pkObject.insert(std::make_pair(pkObj->GetID(), pkObj));
	m_map_pkObjectByVID.insert(std::make_pair(pkObj->GetVID(), pkObj));

	pkObj->SetLand(this);
}

LPOBJECT CLand::FindObject(uint32_t dwID)
{
	auto it = m_map_pkObject.find(dwID);

	if (it == m_map_pkObject.end())
		return nullptr;

	return it->second;
}

LPOBJECT CLand::FindObjectByGroup(uint32_t dwGroupVnum)
{
	std::map<uint32_t, LPOBJECT>::iterator it;
	for (it = m_map_pkObject.begin(); it != m_map_pkObject.end(); ++it)
	{
		LPOBJECT pObj = it->second;
		if (pObj->GetGroup() == dwGroupVnum)
			return pObj;
	}

	return nullptr;
}

LPOBJECT CLand::FindObjectByVnum(uint32_t dwVnum)
{
	std::map<uint32_t, LPOBJECT>::iterator it;
	for (it = m_map_pkObject.begin(); it != m_map_pkObject.end(); ++it)
	{
		LPOBJECT pObj = it->second;
		if (pObj->GetVnum() == dwVnum)
			return pObj;
	}

	return nullptr;
}

// BUILDING_NPC
LPOBJECT CLand::FindObjectByNPC(LPCHARACTER npc)
{
	if (!npc)
		return nullptr;

	std::map<uint32_t, LPOBJECT>::iterator it;
	for (it = m_map_pkObject.begin(); it != m_map_pkObject.end(); ++it)
	{
		LPOBJECT pObj = it->second;
		if (pObj->GetNPC() == npc)
			return pObj;
	}

	return nullptr;
}
// END_OF_BUILDING_NPC

LPOBJECT CLand::FindObjectByVID(uint32_t dwVID)
{
	auto it = m_map_pkObjectByVID.find(dwVID);

	if (it == m_map_pkObjectByVID.end())
		return nullptr;

	return it->second;
}

void CLand::DeleteObject(uint32_t dwID)
{
	LPOBJECT pkObj = FindObject(dwID);

	if (!pkObj)
		return;

	sys_log(0, "Land::DeleteObject %u", dwID);
	CManager::instance().UnregisterObject(pkObj);
	M2_DESTROY_CHARACTER (pkObj->GetNPC());

	m_map_pkObject.erase(dwID);
	m_map_pkObjectByVID.erase(dwID);

	M2_DELETE(pkObj);
}

struct FIsIn
{
	int32_t sx, sy;
	int32_t ex, ey;

	bool bIn;
	FIsIn (	int32_t sx_, int32_t sy_, int32_t ex_, int32_t ey_)
		: sx(sx_), sy(sy_), ex(ex_), ey(ey_), bIn(false)
	{}

	void operator () (LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER) ent;
			if (ch->IsMonster())
			{
				return;
			}
			if (sx <= ch->GetX() && ch->GetX() <= ex
				&& sy <= ch->GetY() && ch->GetY() <= ey)
			{
				bIn = true;
			}
		}
	}
};

bool CLand::RequestCreateObject(uint32_t dwVnum, int32_t lMapIndex, int32_t x, int32_t y, float xRot, float yRot, float zRot, bool checkAnother) const
{
	SECTREE_MANAGER& rkSecTreeMgr = SECTREE_MANAGER::instance();
	TObjectProto * pkProto = CManager::instance().GetObjectProto(dwVnum);

	if (!pkProto)
	{
		sys_err("Invalid Object vnum %u", dwVnum);
		return false;
	}
	const TMapRegion * r = rkSecTreeMgr.GetMapRegion(lMapIndex);
	if (!r)
		return false;

	sys_log(0, "RequestCreateObject(vnum=%u, map=%d, pos=(%d,%d), rot=(%.1f,%.1f,%.1f) region(%d,%d ~ %d,%d)",
			dwVnum, lMapIndex, x, y, xRot, yRot, zRot, r->sx, r->sy, r->ex, r->ey);

	x += r->sx;
	y += r->sy;

	int32_t sx = r->sx + m_data.x;
	int32_t ex = sx + m_data.width;
	int32_t sy = r->sy + m_data.y;
	int32_t ey = sy + m_data.height;

	int32_t osx = x + pkProto->lRegion[0];
	int32_t osy = y + pkProto->lRegion[1];
	int32_t oex = x + pkProto->lRegion[2];
	int32_t oey = y + pkProto->lRegion[3];

	double rad = zRot * 2.0 * M_PI / 360.0;

	int32_t tsx = static_cast<int32_t>(pkProto->lRegion[0] * cos(rad) + pkProto->lRegion[1] * sin(rad) + x);
	int32_t tsy = static_cast<int32_t>(pkProto->lRegion[0] * -sin(rad) + pkProto->lRegion[1] * cos(rad) + y);

	int32_t tex = static_cast<int32_t>(pkProto->lRegion[2] * cos(rad) + pkProto->lRegion[3] * sin(rad) + x);
	int32_t tey = static_cast<int32_t>(pkProto->lRegion[2] * -sin(rad) + pkProto->lRegion[3] * cos(rad) + y);

	if (tsx < sx || tex > ex || tsy < sy || tey > ey)
	{
		sys_err("invalid position: object is outside of land region\nLAND: %d %d ~ %d %d\nOBJ: %d %d ~ %d %d", sx, sy, ex, ey, osx, osy, oex, oey);
		return false;
	}

	// ADD_BUILDING_ROTATION
	if ( checkAnother )
	{
		if (rkSecTreeMgr.ForAttrRegion(lMapIndex, osx, osy, oex, oey,
			xRot, yRot, zRot,
			ATTR_OBJECT, ATTR_REGION_MODE_CHECK))
		{
			sys_err("another object already exist");
			return false;
		}
		FIsIn f (osx, osy, oex, oey);
		rkSecTreeMgr.GetMap(lMapIndex)->for_each(f);

		if (f.bIn)
		{
			sys_err("another object already exist");
			return false;
		}
	}
	// END_OF_BUILDING_NPC

	TPacketGDCreateObject p;

	p.dwVnum = dwVnum;
	p.dwLandID = m_data.dwID;
	p.lMapIndex = lMapIndex;
	p.x = x;
	p.y = y;
	p.xRot = xRot;
	p.yRot = yRot;
	p.zRot = zRot;

	db_clientdesc->DBPacket(HEADER_GD_CREATE_OBJECT, 0, &p, sizeof(TPacketGDCreateObject));
	return true;
}

void CLand::RequestDeleteObject(uint32_t dwID)
{
	if (!FindObject(dwID))
	{
		sys_err("no object by id %u", dwID);
		return;
	}

	db_clientdesc->DBPacket(HEADER_GD_DELETE_OBJECT, 0, &dwID, sizeof(uint32_t));
	sys_log(0, "RequestDeleteObject id %u", dwID);
}

void CLand::RequestDeleteObjectByVID(uint32_t dwVID)
{
	LPOBJECT pkObj = FindObjectByVID(dwVID);

	if (!pkObj)
	{
		sys_err("no object by vid %u", dwVID);
		return;
	}

	uint32_t dwID = pkObj->GetID();
	db_clientdesc->DBPacket(HEADER_GD_DELETE_OBJECT, 0, &dwID, sizeof(uint32_t));
	sys_log(0, "RequestDeleteObject vid %u id %u", dwVID, dwID);
}

void CLand::SetOwner(uint32_t dwGuild)
{
	if (m_data.dwGuildID != dwGuild)
	{
		m_data.dwGuildID = dwGuild;
		RequestUpdate(dwGuild);
	}
}

void CLand::RequestUpdate(uint32_t dwGuild) const
{
	uint32_t a[2];

	a[0] = GetID();
	a[1] = dwGuild;

	db_clientdesc->DBPacket(HEADER_GD_UPDATE_LAND, 0, &a[0], sizeof(uint32_t) * 2);
	sys_log(0, "RequestUpdate id %u guild %u", a[0], a[1]);
}

////////////////////////////////////////////////////////////////////////////////////

CManager::CManager()
{
}

CManager::~CManager()
{
	Destroy();
}

void CManager::Destroy()
{
	auto it = m_map_pkLand.begin();
	for ( ; it != m_map_pkLand.end(); ++it) {
		M2_DELETE(it->second);
	}
	m_map_pkLand.clear();
}

bool CManager::LoadObjectProto(const TObjectProto * pProto, int32_t size) // from DB
{
	m_vec_kObjectProto.resize(size);
	memcpy(&m_vec_kObjectProto[0], pProto, sizeof(TObjectProto) * size);

	for (int32_t i = 0; i < size; ++i)
	{
		TObjectProto & r = m_vec_kObjectProto[i];

		// BUILDING_NPC
		sys_log(0, "ObjectProto %u price %u NPC %u",
				r.dwVnum, r.dwPrice, r.dwNPCVnum);
		// END_OF_BUILDING_NPC

		for (auto & kMaterial : r.kMaterials)
		{
			if (!kMaterial.dwItemVnum)
				break;

			if (nullptr == ITEM_MANAGER::instance().GetTable(kMaterial.dwItemVnum))
			{
				sys_err("          mat: ERROR!! no item by vnum %u", kMaterial.dwItemVnum);
				return false;
			}

			sys_log(0, "          mat: %u %u", kMaterial.dwItemVnum, kMaterial.dwCount);
		}

		m_map_pkObjectProto.insert(std::make_pair(r.dwVnum, &m_vec_kObjectProto[i]));
	}

	return true;
}

TObjectProto * CManager::GetObjectProto(uint32_t dwVnum)
{
	auto it = m_map_pkObjectProto.find(dwVnum);

	if (it == m_map_pkObjectProto.end())
		return nullptr;

	return it->second;
}

bool CManager::LoadLand(TLand * pTable) // from DB
{
	// MapAllow�� ���� ���� �������� load�� �ؾ��Ѵ�.
	//	�ǹ�(object)�� ��� ��忡 ���� �ִ��� �˱� ���ؼ��� �ǹ��� ������ ���� ��� ��� �Ҽ����� �˾��Ѵ�.
	//	���� ���� load�� ���� ������ ��� �ǹ��� ��� ��忡 �Ҽӵ� ���� ���� ���ؼ�
	//	��� �ǹ��� ���� ��� ������ ���� ���Ѵ�.
	//if (!map_allow_find(pTable->lMapIndex))
	//	return false;

	auto  pkLand = M2_NEW CLand(pTable);
	m_map_pkLand.insert(std::make_pair(pkLand->GetID(), pkLand));

	sys_log(0, "LAND: %u map %d %dx%d w %u h %u",
			pTable->dwID, pTable->lMapIndex, pTable->x, pTable->y, pTable->width, pTable->height);

	return true;
}

CLand * CManager::FindLand(uint32_t dwID)
{
	auto it = m_map_pkLand.find(dwID);

	if (it == m_map_pkLand.end())
		return nullptr;

	return it->second;
}

CLand * CManager::FindLand(int32_t lMapIndex, int32_t x, int32_t y)
{
	sys_log(0, "BUILDING: FindLand %d %d %d", lMapIndex, x, y);

	const TMapRegion * r = SECTREE_MANAGER::instance().GetMapRegion(lMapIndex);

	if (!r)
		return nullptr;

	x -= r->sx;
	y -= r->sy;

	auto it = m_map_pkLand.begin();

	while (it != m_map_pkLand.end())
	{
		CLand * pkLand = (it++)->second;
		const TLand & s_land = pkLand->GetData();

		if (s_land.lMapIndex != lMapIndex)
			continue;

		if (x < s_land.x || y < s_land.y)
			continue;

		if (x > s_land.x + s_land.width || y > s_land.y + s_land.height)
			continue;

		return pkLand;
	}

	return nullptr;
}

CLand * CManager::FindLandByGuild(uint32_t GID)
{
	auto it = m_map_pkLand.begin();

	while (it != m_map_pkLand.end())
	{
		CLand * pkLand = (it++)->second;

		if (pkLand->GetData().dwGuildID == GID)
			return pkLand;
	}

	return nullptr;
}

bool CManager::LoadObject(TObject * pTable, bool isBoot) // from DB
{
	CLand * pkLand = FindLand(pTable->dwLandID);

	if (!pkLand)
	{
		sys_log(0, "Cannot find land by id %u", pTable->dwLandID);
		return false;
	}

	TObjectProto * pkProto = GetObjectProto(pTable->dwVnum);

	if (!pkProto)
	{
		sys_err("Cannot find object %u in prototype (id %u)", pTable->dwVnum, pTable->dwID);
		return false;
	}

	sys_log(0, "OBJ: id %u vnum %u map %d pos %dx%d", pTable->dwID, pTable->dwVnum, pTable->lMapIndex, pTable->x, pTable->y);

	LPOBJECT pkObj = M2_NEW CObject(pTable, pkProto);

	uint32_t dwVID = CHARACTER_MANAGER::instance().AllocVID();
	pkObj->SetVID(dwVID);
	pkObj->SetGuildID(pkLand->GetData().dwGuildID);

	m_map_pkObjByVID.insert(std::make_pair(dwVID, pkObj));
	m_map_pkObjByID.insert(std::make_pair(pTable->dwID, pkObj));

	pkLand->InsertObject(pkObj);

	if (!isBoot)
		pkObj->Show(pTable->lMapIndex, pTable->x, pTable->y);
	else
	{
		pkObj->SetMapIndex(pTable->lMapIndex);
		pkObj->SetXYZ(pTable->x, pTable->y, 0);
	}

	// BUILDING_NPC
	if (!isBoot)
	{
		if (pkProto->dwNPCVnum)
			pkObj->RegenNPC();

		pkObj->ApplySpecialEffect();
	}
	// END_OF_BUILDING_NPC

	return true;
}

void CManager::FinalizeBoot()
{
	auto it = m_map_pkObjByID.begin();

	while (it != m_map_pkObjByID.end())
	{
		LPOBJECT pkObj = (it++)->second;

		pkObj->Show(pkObj->GetMapIndex(), pkObj->GetX(), pkObj->GetY());
		// BUILDING_NPC
		pkObj->RegenNPC();
		pkObj->ApplySpecialEffect();
		// END_OF_BUILDING_NPC
	}

	// BUILDING_NPC
	sys_log(0, "FinalizeBoot");
	// END_OF_BUILDING_NPC

	auto it2 = m_map_pkLand.begin();

	while (it2 != m_map_pkLand.end())
	{
		CLand * pkLand = (it2++)->second;

		const TLand & r = pkLand->GetData();

		// LAND_MASTER_LOG
		sys_log(0, "LandMaster map_index=%d pos=(%d, %d)", r.lMapIndex, r.x, r.y);
		// END_OF_LAND_MASTER_LOG

		if (r.dwGuildID != 0)
			continue;

		if (!map_allow_find(r.lMapIndex))
			continue;

		const TMapRegion * region = SECTREE_MANAGER::instance().GetMapRegion(r.lMapIndex);
		if (!region)
			continue;

		CHARACTER_MANAGER::instance().SpawnMob(20040, r.lMapIndex, region->sx + r.x + (r.width / 2), region->sy + r.y + (r.height / 2), 0);
	}
}

void CManager::DeleteObject(uint32_t dwID) // from DB
{
	sys_log(0, "OBJ_DEL: %u", dwID);

	auto it = m_map_pkObjByID.find(dwID);

	if (it == m_map_pkObjByID.end())
		return;

	it->second->GetLand()->DeleteObject(dwID);
}

LPOBJECT CManager::FindObjectByVID(uint32_t dwVID)
{
	auto it = m_map_pkObjByVID.find(dwVID);

	if (it == m_map_pkObjByVID.end())
		return nullptr;

	return it->second;
}

void CManager::UnregisterObject(LPOBJECT pkObj)
{
	m_map_pkObjByID.erase(pkObj->GetID());
	m_map_pkObjByVID.erase(pkObj->GetVID());
}

void CManager::UpdateLand(TLand * pTable)
{
	CLand * pkLand = FindLand(pTable->dwID);
	if (!pkLand)
	{
		sys_err("cannot find land by id %u", pTable->dwID);
		return;
	}
	pkLand->PutData(pTable);

	TPacketGCGuildLandUpdate p;
	p.header = HEADER_GC_UPDATE_LAND;
	p.landID = pTable->dwID;
	p.guildID = pTable->dwGuildID;

	const DESC_MANAGER::DESC_SET & cont = DESC_MANAGER::instance().GetClientSet();
	for (auto *desc : cont) {
		if (desc->GetCharacter() && desc->GetCharacter()->GetMapIndex() == pTable->lMapIndex)
			desc->Packet(&p, sizeof(TPacketGCGuildLandUpdate));
	}
}
void CManager::SendLandList(LPDESC d, int32_t lMapIndex)
{
	TLandPacketElement e;

	TEMP_BUFFER buf;

	uint16_t wCount = 0;

	auto it = m_map_pkLand.begin();

	while (it != m_map_pkLand.end())
	{
		CLand * pkLand = (it++)->second;
		const TLand & r = pkLand->GetData();

		if (r.lMapIndex != lMapIndex)
			continue;

		//
		LPCHARACTER ch  = d->GetCharacter();
		if (ch)
		{
			CGuild *guild = CGuildManager::instance().FindGuild(r.dwGuildID);
			ch->SendGuildName(guild);
		}
		//

		e.dwID = r.dwID;
		e.x = r.x;
		e.y = r.y;
		e.width = r.width;
		e.height = r.height;
		e.dwGuildID = r.dwGuildID;

		buf.write(&e, sizeof(TLandPacketElement));
		++wCount;
	}

	sys_log(0, "SendLandList map %d count %u elem_size: %d", lMapIndex, wCount, buf.size());

	if (wCount != 0)
	{
		TPacketGCLandList p;

		p.header = HEADER_GC_LAND_LIST;
		p.size = sizeof(TPacketGCLandList) + buf.size();

		d->BufferedPacket(&p, sizeof(TPacketGCLandList));
		d->Packet(buf.read_peek(), buf.size());
	}
}

// LAND_CLEAR
void CManager::ClearLand(uint32_t dwLandID)
{
	CLand* pLand = FindLand(dwLandID);

	if ( pLand == nullptr )
	{
		sys_log(0, "LAND_CLEAR: there is no LAND id like %d", dwLandID);
		return;
	}

	pLand->ClearLand();

	sys_log(0, "LAND_CLEAR: request Land Clear. LandID: %d", pLand->GetID());
}

void CManager::ClearLandByGuildID(uint32_t dwGuildID)
{
	CLand* pLand = FindLandByGuild(dwGuildID);

	if ( pLand == nullptr )
	{
		sys_log(0, "LAND_CLEAR: there is no GUILD id like %d", dwGuildID);
		return;
	}

	pLand->ClearLand();

	sys_log(0, "LAND_CLEAR: request Land Clear. LandID: %d", pLand->GetID());
}

void CLand::ClearLand()
{
	auto iter = m_map_pkObject.begin();

	while ( iter != m_map_pkObject.end() )
	{
		RequestDeleteObject(iter->second->GetID());
		++iter;
	}

	SetOwner(0);

	const TLand & r = GetData();
	const TMapRegion * region = SECTREE_MANAGER::instance().GetMapRegion(r.lMapIndex);

	CHARACTER_MANAGER::instance().SpawnMob(20040, r.lMapIndex, region->sx + r.x + (r.width / 2), region->sy + r.y + (r.height / 2), 0);
}
// END_LAND_CLEAR

// BUILD_WALL
void CLand::DrawWall(uint32_t dwVnum, int32_t nMapIndex, int32_t& x, int32_t& y, char length, float zRot) const
{
	int32_t rot = static_cast<int32_t>(zRot);
	rot = ((rot%360) / 90) * 90;

	int32_t dx=0, dy=0;

	switch ( rot )
	{
		case 0 :
			dx = -500;
			dy = 0;
			break;

		case 90 :
			dx = 0;
			dy = 500;
			break;

		case 180 :
			dx = 500;
			dy = 0;
			break;

		case 270 :
			dx = 0;
			dy = -500;
			break;
	}

	for ( int32_t i=0; i < length; i++ )
	{
		RequestCreateObject(dwVnum, nMapIndex, x, y, 0.0f, 0.0f, static_cast<float>(rot), false);
		x += dx;
		y += dy;
	}
}


bool CLand::RequestCreateWall(int32_t nMapIndex, float rot)
{
	const bool 	WALL_ANOTHER_CHECKING_ENABLE = false;

	const TLand& land = GetData();

	int32_t center_x = land.x + land.width  / 2;
	int32_t center_y = land.y + land.height / 2;

	int32_t wall_x = center_x;
	int32_t wall_y = center_y;
	int32_t wall_half_w = 1000;
	int32_t wall_half_h = 1362;

	if (rot == 0.0f) 		// ���� ��
	{
		int32_t door_rot_x = wall_x;
		int32_t door_rot_y = wall_y + wall_half_h;
		RequestCreateObject(WALL_DOOR_VNUM, nMapIndex, wall_x, wall_y + wall_half_h, static_cast<float>(door_rot_x), static_cast<float>(door_rot_y), 0.0f, WALL_ANOTHER_CHECKING_ENABLE);
		RequestCreateObject(WALL_BACK_VNUM, nMapIndex, wall_x, wall_y - wall_half_h, static_cast<float>(door_rot_x), static_cast<float>(door_rot_y), 0.0f, WALL_ANOTHER_CHECKING_ENABLE);
		RequestCreateObject(WALL_LEFT_VNUM, nMapIndex, wall_x - wall_half_w, wall_y, static_cast<float>(door_rot_x), static_cast<float>(door_rot_y), 0.0f, WALL_ANOTHER_CHECKING_ENABLE);
		RequestCreateObject(WALL_RIGHT_VNUM, nMapIndex, wall_x + wall_half_w, wall_y, static_cast<float>(door_rot_x), static_cast<float>(door_rot_y), 0.0f, WALL_ANOTHER_CHECKING_ENABLE);
	}
	else if (rot == 180.0f)		// ���� ��
	{
		int32_t door_rot_x = wall_x;
		int32_t door_rot_y = wall_y - wall_half_h;
		RequestCreateObject(WALL_DOOR_VNUM, nMapIndex, wall_x, wall_y - wall_half_h, static_cast<float>(door_rot_x), static_cast<float>(door_rot_y), 180.0f, WALL_ANOTHER_CHECKING_ENABLE);
		RequestCreateObject(WALL_BACK_VNUM, nMapIndex, wall_x, wall_y + wall_half_h, static_cast<float>(door_rot_x), static_cast<float>(door_rot_y), 0.0f, WALL_ANOTHER_CHECKING_ENABLE);
		RequestCreateObject(WALL_LEFT_VNUM, nMapIndex, wall_x - wall_half_w, wall_y, static_cast<float>(door_rot_x), static_cast<float>(door_rot_y), 0.0f, WALL_ANOTHER_CHECKING_ENABLE);
		RequestCreateObject(WALL_RIGHT_VNUM, nMapIndex, wall_x + wall_half_w, wall_y, static_cast<float>(door_rot_x), static_cast<float>(door_rot_y), 0.0f, WALL_ANOTHER_CHECKING_ENABLE);
	}
	else if (rot == 90.0f)		// ���� �� 
	{
		int32_t door_rot_x = wall_x + wall_half_h;
		int32_t door_rot_y = wall_y;
		RequestCreateObject(WALL_DOOR_VNUM, nMapIndex, wall_x + wall_half_h, wall_y, static_cast<float>(door_rot_x), static_cast<float>(door_rot_y), 90.0f, WALL_ANOTHER_CHECKING_ENABLE);
		RequestCreateObject(WALL_BACK_VNUM, nMapIndex, wall_x - wall_half_h, wall_y, static_cast<float>(door_rot_x), static_cast<float>(door_rot_y), 90.0f, WALL_ANOTHER_CHECKING_ENABLE);
		RequestCreateObject(WALL_LEFT_VNUM, nMapIndex, wall_x, wall_y - wall_half_w, static_cast<float>(door_rot_x), static_cast<float>(door_rot_y), 90.0f, WALL_ANOTHER_CHECKING_ENABLE);
		RequestCreateObject(WALL_RIGHT_VNUM, nMapIndex, wall_x, wall_y + wall_half_w, static_cast<float>(door_rot_x), static_cast<float>(door_rot_y), 90.0f, WALL_ANOTHER_CHECKING_ENABLE);
	}
	else if (rot == 270.0f)		// ���� �� 
	{
		int32_t door_rot_x = wall_x - wall_half_h;
		int32_t door_rot_y = wall_y;
		RequestCreateObject(WALL_DOOR_VNUM, nMapIndex, wall_x - wall_half_h, wall_y, static_cast<float>(door_rot_x), static_cast<float>(door_rot_y), 90.0f, WALL_ANOTHER_CHECKING_ENABLE);
		RequestCreateObject(WALL_BACK_VNUM, nMapIndex, wall_x + wall_half_h, wall_y, static_cast<float>(door_rot_x), static_cast<float>(door_rot_y), 90.0f, WALL_ANOTHER_CHECKING_ENABLE);
		RequestCreateObject(WALL_LEFT_VNUM, nMapIndex, wall_x, wall_y - wall_half_w, static_cast<float>(door_rot_x), static_cast<float>(door_rot_y), 90.0f, WALL_ANOTHER_CHECKING_ENABLE);
		RequestCreateObject(WALL_RIGHT_VNUM, nMapIndex, wall_x, wall_y + wall_half_w, static_cast<float>(door_rot_x), static_cast<float>(door_rot_y), 90.0f, WALL_ANOTHER_CHECKING_ENABLE);
	}

	if (g_bIsTestServer)
	{
		RequestCreateObject(FLAG_VNUM, nMapIndex, land.x + 50, 			land.y + 50, 0, 0, 0.0, WALL_ANOTHER_CHECKING_ENABLE);
		RequestCreateObject(FLAG_VNUM, nMapIndex, land.x + land.width - 50,	land.y + 50, 0, 0, 90.0, WALL_ANOTHER_CHECKING_ENABLE);
		RequestCreateObject(FLAG_VNUM, nMapIndex, land.x + land.width - 50,	land.y + land.height - 50, 0, 0, 180.0, WALL_ANOTHER_CHECKING_ENABLE);
		RequestCreateObject(FLAG_VNUM, nMapIndex, land.x + 50, 			land.y + land.height - 50, 0, 0, 270.0, WALL_ANOTHER_CHECKING_ENABLE);
	}
	return true;
}

void CLand::RequestDeleteWall()
{
	auto iter = m_map_pkObject.begin();

	while (iter != m_map_pkObject.end())
	{
		uint32_t id   = iter->second->GetID();
		uint32_t vnum = iter->second->GetVnum();

		switch (vnum)
		{
			case WALL_DOOR_VNUM:
			case WALL_BACK_VNUM:
			case WALL_LEFT_VNUM:
			case WALL_RIGHT_VNUM:
				RequestDeleteObject(id);
				break;
		}


		if (g_bIsTestServer)
		{
			if (FLAG_VNUM == vnum)
				RequestDeleteObject(id);

		}

		++iter;
	}
}

bool CLand::RequestCreateWallBlocks(uint32_t dwVnum, int32_t nMapIndex, char wallSize, bool doorEast, bool doorWest, bool doorSouth, bool doorNorth)
{
	const TLand & r = GetData();

	int32_t startX = r.x + (r.width  / 2) - (1300 + wallSize*500);
	int32_t startY = r.y + (r.height / 2) + (1300 + wallSize*500);

	uint32_t corner = dwVnum - 4;
	uint32_t wall   = dwVnum - 3;
	uint32_t door   = dwVnum - 1;

	bool checkAnother = false;
	int32_t* ptr = nullptr;
	int32_t delta = 1;
	int32_t rot = 270;

	bool doorOpen[4];
	doorOpen[0] = doorWest;
	doorOpen[1] = doorNorth;
	doorOpen[2] = doorEast;
	doorOpen[3] = doorSouth;

	if ( wallSize > 3 ) wallSize = 3;
	else if ( wallSize < 0 ) wallSize = 0;

	for ( int32_t i=0; i < 4; i++, rot -= 90 )
	{
		switch ( i )
		{
			case 0 :
				delta = -1;
				ptr = &startY;
				break;
			case 1 :
				delta = 1;
				ptr = &startX;
				break;
			case 2 :
				ptr = &startY;
				delta = 1;
				break;
			case 3 :
				ptr = &startX;
				delta = -1;
				break;
		}

		RequestCreateObject(corner, nMapIndex, startX, startY, 0.0f, 0.0f, static_cast<float>(rot), checkAnother);

		*ptr = *ptr + ( 700 * delta );

		if ( doorOpen[i] )
		{
			DrawWall(wall, nMapIndex, startX, startY, wallSize, static_cast<float>(rot));

			*ptr = *ptr + ( 700 * delta );

			RequestCreateObject(door, nMapIndex, startX, startY, 0.0f, 0.0f, static_cast<float>(rot), checkAnother);

			*ptr = *ptr + ( 1300 * delta );

			DrawWall(wall, nMapIndex, startX, startY, wallSize, static_cast<float>(rot));
		}
		else
		{
			DrawWall(wall, nMapIndex, startX, startY, wallSize * 2 + 4, static_cast<float>(rot));
		}

		*ptr = *ptr + ( 100 * delta );
	}

	return true;
}

void CLand::RequestDeleteWallBlocks(uint32_t dwID)
{
	auto iter = m_map_pkObject.begin();

	uint32_t corner = dwID - 4;
	uint32_t wall = dwID - 3;
	uint32_t door = dwID - 1;
	uint32_t dwVnum = 0;

	while ( iter != m_map_pkObject.end() )
	{
		dwVnum = iter->second->GetVnum();

		if ( dwVnum == corner || dwVnum == wall || dwVnum == door )
		{
			RequestDeleteObject(iter->second->GetID());
		}
		++iter;
	}
}
// END_BUILD_WALL

