#include "stdafx.h"
#include "char.h"
#include "desc.h"
#include "sectree_manager.h"
#include "desc_manager.h"

CEntity::CEntity()
{
	Initialize();
}

CEntity::~CEntity()
{
	if (!m_isDestroyed)
		assert(!"You must call CEntity::Destroy() method in your derived class destructor");
}

void CEntity::Initialize(int32_t type)
{
	m_isDestroyed = false;

	m_objectType = type;
	m_viewAge = 0;
	m_position.x = m_position.y = m_position.z = 0;
	m_mapView.clear();

	m_sectree = nullptr;
	m_desc = nullptr;
	m_mapIndex = 0;
	m_isObserver = false;

	m_isBattlegroundEntity = false;
}

void CEntity::Destroy()
{
	if (m_isDestroyed)
		return;

	ViewCleanup();
	m_isDestroyed = true;
}

void CEntity::SetType(int32_t type)
{
	m_objectType = type;
}

int32_t CEntity::GetType() const
{
	return m_objectType;
}

bool CEntity::IsType(int32_t type) const
{
	return (m_objectType == type ? true : false);
}

struct FuncPacketAround
{
	const void *        m_data;
	int32_t             m_bytes;
	LPENTITY            m_except;

	FuncPacketAround(const void * data, int32_t bytes, LPENTITY except = nullptr) :m_data(data), m_bytes(bytes), m_except(except)
	{
	}

	void operator () (LPENTITY ent) const
	{
		if (ent == m_except)
			return;

		if (ent->GetDesc())
			ent->GetDesc()->Packet(m_data, m_bytes);
	}
};

struct FuncPacketView : public FuncPacketAround
{
	FuncPacketView(const void * data, int32_t bytes, LPENTITY except = nullptr) :
		FuncPacketAround(data, bytes, except)
	{
	}

	void operator() (const CEntity::MAP_VIEW::value_type& v) const
	{
		FuncPacketAround::operator() (v.first);
	}
};

void CEntity::PacketAround(const void * data, int32_t bytes, LPENTITY except)
{
	PacketView(data, bytes, except);
}

void CEntity::PacketView(const void * data, int32_t bytes, LPENTITY except)
{
	if (!GetSectree())
		return;

	FuncPacketView f(data, bytes, except);

	// In the observer state, only my packets are received by me.
	if (!m_isObserver)
		std::for_each(m_mapView.begin(), m_mapView.end(), f);

	f(std::make_pair(this, 0));
}

void CEntity::PacketMap(int32_t nMapIndex, const void * data, int32_t bytes)
{
	const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::Instance().GetClientSet();

	for (const auto& pkDesc : c_ref_set)
	{
		if (!pkDesc->GetCharacter())
			continue;

		if (pkDesc->GetCharacter()->GetMapIndex() != nMapIndex)
			continue;

		pkDesc->Packet(data, bytes);
	}
}

void CEntity::SetObserverMode(bool bFlag)
{
	if (m_isObserver == bFlag)
		return;

	m_isObserver = bFlag;
	
	if (m_isObserver) 
	{
		auto f = [this](const MAP_VIEW::value_type& p)
		{
			EncodeRemovePacket(p.first); 
		};

		std::for_each(m_mapView.begin(), m_mapView.end(), f);
	}
	else 
	{
		auto f = [this](const MAP_VIEW::value_type& p)
		{
			EncodeInsertPacket(p.first); 
		};

		std::for_each(m_mapView.begin(), m_mapView.end(), f);
	}

	if (IsType(ENTITY_CHARACTER))
	{
		auto ch = static_cast<LPCHARACTER>(this);
		ch->ChatPacket(CHAT_TYPE_COMMAND, "ObserverMode %d", m_isObserver ? 1 : 0);
	}
}

