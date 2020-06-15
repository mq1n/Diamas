#include "stdafx.h"
#include "utils.h"
#include "char.h"
#include "sectree_manager.h"
#include "config.h"
#include "Battleground.h"

void CEntity::ViewCleanup()
{
	for (const auto& p : m_mapView)
		p.first->ViewRemove(this, false);

	m_mapView.clear();
}

void CEntity::ViewReencode()
{
	if (m_isObserver)
		return;

	EncodeRemovePacket(this);
	EncodeInsertPacket(this);

	for (const auto& p : m_mapView) 
	{
		auto entity = p.first;

		if (!m_isObserver) 
		{
			EncodeRemovePacket(entity);
			EncodeInsertPacket(entity);
		}

		if (!entity->m_isObserver) 
		{
			entity->EncodeRemovePacket(this);
			entity->EncodeInsertPacket(this);
		}
	}
}

void CEntity::ViewInsert(LPENTITY entity, bool recursive)
{
	if (this == entity)
		return;

	auto r = m_mapView.emplace(entity, m_viewAge);
	if (!r.second) 
	{
		r.first->second = m_viewAge;
		return;
	}

	if (!entity->m_isObserver)
		entity->EncodeInsertPacket(this);

	if (recursive)
		entity->ViewInsert(this, false);
}

void CEntity::ViewRemove(LPENTITY entity, bool recursive)
{
	auto it = m_mapView.find(entity);
	if (it == m_mapView.end())
		return;

	m_mapView.erase(it);

	if (!entity->m_isObserver)
		entity->EncodeRemovePacket(this);

	if (recursive)
		entity->ViewRemove(this, false);
}

void CEntity::UpdateSectree()
{
	if (!GetSectree()) 
	{
		if (IsType(ENTITY_CHARACTER)) 
		{
			auto tch = static_cast<CHARACTER*>(this);
			sys_err("null sectree name: %s %d %d", tch->GetName(), GetX(), GetY());
		}

		return;
	}

	++m_viewAge;

	auto f = [&](LPENTITY ent)
	{
		// Objects that are not objects are calculated by distance.
		if (!ent->IsType(ENTITY_OBJECT) /* && !CBattlegroundManager::instance().IsEventMap(m_mapIndex) */)
		{
			if (DISTANCE_APPROX(ent->GetX() - this->GetX(), ent->GetY() - this->GetY()) > VIEW_RANGE + VIEW_BONUS_RANGE)
				return;
		}

		// Add me to target
		this->ViewInsert(ent);

		// If both entities are characters
		if (ent->IsType(ENTITY_CHARACTER) && this->IsType(ENTITY_CHARACTER)) 
		{
			auto chMe = static_cast<CHARACTER*>(this);
			auto chEnt = static_cast<CHARACTER*>(ent);

			// If the target is an NPC, the StateMachine is started.
			if (chMe->IsPC() && !chEnt->IsPC() && !chEnt->IsWarp() && !chEnt->IsGoto())
				chEnt->StartStateMachine();
		}
	};

	GetSectree()->ForEachAround(f); // PC ONLY
//	GetSectree()->ForEachEntity(f);

	auto it = m_mapView.begin();
	auto end = m_mapView.end();

	while (it != end) 
	{
		if (it->second < m_viewAge) 
		{
			auto entity = it->first;
			if (entity && entity->IsType(ENTITY_CHARACTER) && (static_cast<CHARACTER*>(entity)->IsDead() || !entity->IsBattlegroundEntity()))
			{
				if (!entity->m_isObserver)
					entity->EncodeRemovePacket(this);

				entity->ViewRemove(this, false);

				it = m_mapView.erase(it);
			}
		} 
		else 
		{
			++it;
		}
	}
}
