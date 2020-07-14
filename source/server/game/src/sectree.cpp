#include "stdafx.h"
#include "sectree_manager.h"
#include "../../libgame/include/attribute.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "desc_manager.h"

SECTREE::SECTREE()
{
	Initialize();
}

SECTREE::~SECTREE()
{
	Destroy();
}

void SECTREE::Initialize()
{
	m_id.package = 0;
	m_pkAttribute = nullptr;
	m_iPCCount = 0;
	isClone = false;
}

void SECTREE::Destroy()
{
	if (!m_set_entity.empty())
	{
		sys_err("Sectree: entity set not empty!!");

		for (auto it = m_set_entity.begin(); it != m_set_entity.end(); ++it)
		{
			CEntity* ent = *it;
			if (ent)
			{
				if (ent->IsType(ENTITY_CHARACTER))
				{
					auto ch = static_cast<LPCHARACTER>(ent);
					if (ch)
					{
						sys_err("Sectree: destroying character: %s is_pc %d", ch->GetName(), ch->IsPC() ? 1 : 0);

						if (ch->GetDesc())
							DESC_MANAGER::Instance().DestroyDesc(ch->GetDesc());
						else
							M2_DESTROY_CHARACTER(ch);
					}
				}
				else if (ent->IsType(ENTITY_ITEM))
				{
					auto item = static_cast<LPITEM>(ent);
					if (item)
					{
						sys_err("Sectree: destroying Item: %s", item->GetName());
						
						M2_DESTROY_ITEM(item);
					}
				}
				else
				{
					sys_err("Sectree: unknown type: %d", ent->GetType());
				}
			}
		}
	}
	m_set_entity.clear();

	if (!isClone && m_pkAttribute)
	{
		M2_DELETE(m_pkAttribute);
		m_pkAttribute = nullptr;
	}
}

SECTREEID SECTREE::GetID() const
{
	return m_id;
}

void SECTREE::IncreasePC()
{
	auto it_tree = m_neighbor_list.begin();

	while (it_tree != m_neighbor_list.end())
	{
		++(*it_tree)->m_iPCCount;
		++it_tree;
	}
}

void SECTREE::DecreasePC()
{
	auto it_tree = m_neighbor_list.begin();

	while (it_tree != m_neighbor_list.end())
	{
		LPSECTREE tree = *it_tree++;

		if (--tree->m_iPCCount <= 0)
		{
			if (tree->m_iPCCount < 0)
			{
				sys_err("tree pc count lower than zero (value %d coord %d %d)", tree->m_iPCCount, tree->m_id.coord.x, tree->m_id.coord.y);
				tree->m_iPCCount = 0;
			}

			auto it_entity = tree->m_set_entity.begin();

			while (it_entity != tree->m_set_entity.end())
			{
				auto pkEnt = *(it_entity++);

				if (pkEnt->IsType(ENTITY_CHARACTER))
				{
					auto ch = static_cast<LPCHARACTER>(pkEnt);
					if (ch)
						ch->StopStateMachine();
				}
			}
		}
	}
}

bool SECTREE::InsertEntity(LPENTITY pkEnt)
{
	auto pkCurTree = pkEnt->GetSectree();
	if (pkCurTree == this)
		return false;

	if (m_set_entity.find(pkEnt) != m_set_entity.end()) 
	{
		sys_err("Foreign entity %p already in this sectree!", pkEnt);
		return false;
	}

	if (pkCurTree)
		pkCurTree->m_set_entity.erase(pkEnt);

	pkEnt->SetSectree(this);
//	pkEnt->UpdateSectree();

	m_set_entity.insert(pkEnt);

	if (pkEnt->IsType(ENTITY_CHARACTER))
	{
		auto pkChr = static_cast<LPCHARACTER>(pkEnt);

		if (pkChr->IsPC())
		{
			IncreasePC();

			if (pkCurTree)
				pkCurTree->DecreasePC();
		}
		else if (m_iPCCount > 0 && !pkChr->IsWarp() && !pkChr->IsGoto()) // PC가 아니고 이 곳에 PC가 있다면 Idle event를 시작 시킨다.
		{
			pkChr->StartStateMachine();
		}
	}

	return true;
}

void SECTREE::RemoveEntity(LPENTITY pkEnt)
{
	if (!pkEnt)
		return;
	
	auto it = m_set_entity.find(pkEnt);

	if (it == m_set_entity.end())
		return;
	
	m_set_entity.erase(it);

	pkEnt->SetSectree(nullptr);

	if (pkEnt->IsType(ENTITY_CHARACTER))
	{
		if (static_cast<LPCHARACTER>(pkEnt)->IsPC())
			DecreasePC();
	}
}

void SECTREE::BindAttribute(CAttribute * pkAttribute)
{
	m_pkAttribute = pkAttribute;
}

void SECTREE::CloneAttribute(LPSECTREE tree)
{
	m_pkAttribute = tree->m_pkAttribute;
	isClone = true;
}

void SECTREE::SetAttribute(uint32_t x, uint32_t y, uint32_t dwAttr)
{
	assert(m_pkAttribute != nullptr);
	x = (x % SECTREE_SIZE) / CELL_SIZE;
	y = (y % SECTREE_SIZE) / CELL_SIZE;
	m_pkAttribute->Set(x, y, dwAttr);
}

void SECTREE::RemoveAttribute(uint32_t x, uint32_t y, uint32_t dwAttr)
{
	assert(m_pkAttribute != nullptr);
	x = (x % SECTREE_SIZE) / CELL_SIZE;
	y = (y % SECTREE_SIZE) / CELL_SIZE;
	m_pkAttribute->Remove(x, y, dwAttr);
}

uint32_t SECTREE::GetAttribute(int32_t x, int32_t y)
{
	assert(m_pkAttribute != nullptr);
	x = (x % SECTREE_SIZE) / CELL_SIZE;
	y = (y % SECTREE_SIZE) / CELL_SIZE;
	return m_pkAttribute->Get(x, y);
}

bool SECTREE::IsAttr(int32_t x, int32_t y, uint32_t dwFlag)
{
	if (IS_SET(GetAttribute(x, y), dwFlag))
		return true;

	return false;
}

int32_t SECTREE::GetEventAttribute(int32_t x, int32_t y)
{
	return GetAttribute(x, y) >> 8;
}

