#include "stdafx.h"
#include "quest_manager.h"
#include "char.h"
#include "item.h"
#include "item_manager.h"
#include "log.h"
#include "refine.h"
#include "sectree_manager.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	//
	// "item" Lua functions
	//

	int32_t item_get_cell(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();

		if (q.GetCurrentItem())
			lua_pushnumber(L, q.GetCurrentItem()->GetCell());
		else
			lua_pushnumber(L, 0);
		return 1;
	}

	int32_t item_select_cell(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushboolean(L, false);
			return 1;
		}
		uint16_t cell = (uint16_t)lua_tonumber(L, 1);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		CItem * item = ch ? ch->GetInventoryItem(cell) : nullptr;
		if (!item)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		CQuestManager::Instance().SetCurrentItem(item);
		CQuestManager::Instance().UpdateStateItem(CQuestManager::Instance().GetCurrentCharacterPtr(), item);

		lua_pushboolean(L, true);
		return 1;
	}

	int32_t item_select(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushboolean(L, false);
			return 1;
		}
		uint32_t id = (uint32_t) lua_tonumber(L, 1);

		CItem * item = ITEM_MANAGER::Instance().Find(id);
		if (!item)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		CQuestManager::Instance().SetCurrentItem(item);
		CQuestManager::Instance().UpdateStateItem(CQuestManager::Instance().GetCurrentCharacterPtr(), item);

		lua_pushboolean(L, true);
		return 1;
	}

	int32_t item_get_id(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();

		if (q.GetCurrentItem())
			lua_pushnumber(L, q.GetCurrentItem()->GetID());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t item_remove(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		if (!q.GetCurrentCharacterPtr())
			return 0;

		CItem * item = q.GetCurrentItem();
		if (item && item->GetOwner())
		{
			if (q.GetCurrentCharacterPtr() == item->GetOwner())
				ITEM_MANAGER::Instance().RemoveItem(item);
			else
				sys_err("%u Tried to remove invalid item %p", item->GetOwner()->GetPlayerID(), get_pointer(item));
			
			q.ClearCurrentItem();
		}

		return 0;
	}

	int32_t item_get_socket(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		if (q.GetCurrentItem() && lua_isnumber(L, 1))
		{
			int32_t idx = (int32_t) lua_tonumber(L, 1);
			if (idx < 0 || idx >= ITEM_SOCKET_MAX_NUM)
				lua_pushnumber(L,0);
			else
				lua_pushnumber(L, q.GetCurrentItem()->GetSocket(idx));
		}
		else
		{
			lua_pushnumber(L,0);
		}
		return 1;
	}

	int32_t item_set_socket(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		if (q.GetCurrentItem() && lua_isnumber(L,1) && lua_isnumber(L,2))
		{
			int32_t idx = (int32_t) lua_tonumber(L, 1);
			int32_t value = (int32_t) lua_tonumber(L, 2);
			if (idx >=0 && idx < ITEM_SOCKET_MAX_NUM)
				q.GetCurrentItem()->SetSocket(idx, value);
		}
		return 0;
	}

	int32_t item_get_vnum(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPITEM item = q.GetCurrentItem();

		if (item) 
			lua_pushnumber(L, item->GetVnum());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t item_has_flag(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("flag is not a number.");
			lua_pushboolean(L, false);
			return 1;
		}
		int32_t lCheckFlag = (int32_t)lua_tonumber(L, 1);	

		CQuestManager& q = CQuestManager::Instance();
		CItem * item = q.GetCurrentItem();
		if (!item)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		lua_pushboolean(L, IS_SET(item->GetFlag(), lCheckFlag));
		return 1;
	}

	int32_t item_get_value(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("index is not a number");
			lua_pushnumber(L, 0);
			return 1;
		}
		int32_t index = (int32_t)lua_tonumber(L, 1);

		CQuestManager& q = CQuestManager::Instance();
		CItem * item = q.GetCurrentItem();
		if (!item)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if (index < 0 || index >= ITEM_VALUES_MAX_NUM)
		{
			sys_err("index(%d) is out of range (0..%d)", index, ITEM_VALUES_MAX_NUM);
			lua_pushnumber(L, 0);
		}
		else
			lua_pushnumber(L, item->GetValue(index));

		return 1;
	}

	int32_t item_set_value(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("index is not a number");
			lua_pushnumber(L, 0);
			return 1;
		}

		CQuestManager& q = CQuestManager::Instance();
		CItem * item = q.GetCurrentItem();
		if (!item)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		item->SetForceAttribute((int32_t)lua_tonumber(L, 1), (uint8_t)lua_tonumber(L, 2), (int16_t)lua_tonumber(L, 3));

		return 0;
	}

	int32_t item_get_name(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushstring(L, item->GetName());
		else
			lua_pushstring(L, "");

		return 1;
	}

	int32_t item_get_size(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushnumber(L, item->GetSize());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t item_get_count(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushnumber(L, item->GetCount());
		else
			lua_pushnumber(L, 0);

		return 1;
	}


	int32_t item_set_count(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushboolean(L, false);
			return 1;
		}
		uint32_t count = (uint32_t)lua_tonumber(L, 1);

		CQuestManager& q = CQuestManager::Instance();
		CItem * item = q.GetCurrentItem();

		if (item)
			lua_pushboolean(L, item->SetCount(count));
		else
			lua_pushboolean(L, false);

		return 1;
	}

	int32_t item_get_type(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushnumber(L, item->GetType());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t item_get_sub_type(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushnumber(L, item->GetSubType());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t item_get_refine_vnum(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushnumber(L, item->GetRefinedVnum());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t item_next_refine_vnum(lua_State* L)
	{
		uint32_t vnum = 0;
		if (lua_isnumber(L, 1))
			vnum = (uint32_t) lua_tonumber(L, 1);

		TItemTable* pTable = ITEM_MANAGER::Instance().GetTable(vnum);
		if (pTable)
		{
			lua_pushnumber(L, pTable->dwRefinedVnum);
		}
		else
		{
			sys_err("Cannot find item table of vnum %u", vnum);
			lua_pushnumber(L, 0);
		}
		return 1;
	}

	int32_t item_get_level(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushnumber(L, item->GetRefineLevel());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t item_get_level_limit (lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();

		if (q.GetCurrentItem())
		{
			if (q.GetCurrentItem()->GetType() != ITEM_WEAPON && q.GetCurrentItem()->GetType() != ITEM_ARMOR)
			{
				return 0;
			}
			lua_pushnumber(L, q.GetCurrentItem() -> GetLevelLimit());
			return 1;
		}
		return 0;
	}

	int32_t item_start_realtime_expire(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		CItem * pItem = q.GetCurrentItem();

		if (pItem)
		{
			pItem->StartRealTimeExpireEvent();
			return 0;
		}

		return 0;
	}
	
	int32_t item_copy_and_give_before_remove(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushboolean(L, false);
			return 1;
		}
		uint32_t vnum = (uint32_t)lua_tonumber(L, 1);

		CQuestManager& q = CQuestManager::Instance();
		CItem * pItem = q.GetCurrentItem();
		if (!pItem)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		LPCHARACTER pChar = q.GetCurrentCharacterPtr();
		if (!pChar)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		CItem * pkNewItem = ITEM_MANAGER::Instance().CreateItem(vnum, 1, 0, false);
		if (pkNewItem)
		{
			ITEM_MANAGER::CopyAllAttrTo(pItem, pkNewItem);
			LogManager::Instance().ItemLog(pChar, pkNewItem, "COPY SUCCESS", pkNewItem->GetName());

			uint16_t wCell = pItem->GetCell();

			ITEM_MANAGER::Instance().RemoveItem(pItem, "REMOVE (COPY SUCCESS)");

			pkNewItem->AddToCharacter(pChar, TItemPos(INVENTORY, wCell));
			ITEM_MANAGER::Instance().FlushDelayedSave(pkNewItem);

			lua_pushboolean(L, true);			
		}

		return 1;
	}

	int32_t item_get_immuneflag(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		CItem* item = q.GetCurrentItem();

		if (item)
			lua_pushnumber(L, item->GetImmuneFlag());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t item_has_immuneflag(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("immuneflag is not a number.");
			lua_pushboolean(L, false);
			return 1;
		}

		CQuestManager& q = CQuestManager::Instance();
		CItem* item = q.GetCurrentItem();
		if (item)
			lua_pushboolean(L, IS_SET(item->GetImmuneFlag(), (int32_t)lua_tonumber(L, 1)));
		else
			lua_pushboolean(L, false);

		return 1;
	}

	int32_t item_is_available(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		CItem* item = q.GetCurrentItem();

		lua_pushboolean(L, item != nullptr);
		return 1;
	}

	int32_t item_to_character(lua_State* L)
	{
		LPCHARACTER pc = quest::CQuestManager::Instance().GetCurrentCharacterPtr();
		CItem * item = quest::CQuestManager::Instance().GetCurrentItem();
		if (!item || !pc) 
			return 0;

		bool bCell = false;
		if (lua_gettop(L) > 0)
		{
			if (lua_isnumber(L, 1))
				bCell = true;
		}

		int32_t cell = 0;

		if (!bCell)
			cell = pc->GetEmptyInventory(item->GetSize());
		else 
		{
			cell = (int32_t)lua_tonumber(L, 1);

			if (pc->GetItem(TItemPos(cell, INVENTORY)) || !pc->IsEmptyItemGrid(TItemPos(cell, INVENTORY), item->GetSize(), (int32_t)cell))
			{
				lua_pushboolean(L, false);
				return 1;
			}
		}

		item->AddToCharacter(pc, TItemPos(INVENTORY, cell));

		lua_pushboolean(L, true);
		return 1;
	}

	int32_t item_to_equipment(lua_State* L)
	{
		LPCHARACTER pc = quest::CQuestManager::Instance().GetCurrentCharacterPtr();
		CItem * item = quest::CQuestManager::Instance().GetCurrentItem();

		if (!lua_isnumber(L, 1) || !item || !pc) 
		{
			lua_pushboolean(L, false);
			return 1;
		}
		int32_t cell = (int32_t)lua_tonumber(L, 1);

		CItem * wear = pc->GetWear(cell);
		if (wear) 
		{
			lua_pushboolean(L, false);
			return 1;
		}

		item->EquipTo(pc, cell);
		lua_pushboolean(L, true);
		return 1;
	}

	int32_t item_to_ground(lua_State* L)
	{
		LPCHARACTER pc = quest::CQuestManager::Instance().GetCurrentCharacterPtr();
		CItem * item = quest::CQuestManager::Instance().GetCurrentItem();

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5) || !item || !pc)
		{
			lua_pushboolean(L, false);
			return 1;
		}
		int32_t mapIndex = (int32_t)lua_tonumber(L, 1);
		int32_t ownershipTime = (int32_t)lua_tonumber(L, 4);
		int32_t destroyTime = (int32_t)lua_tonumber(L, 5);

		GPOS pos;
		pos.x = (int32_t)lua_tonumber(L, 2);
		pos.y = (int32_t)lua_tonumber(L, 3);

		LPSECTREE sectree = SECTREE_MANAGER::Instance().Get(mapIndex, pos.x, pos.y);

		if (sectree)
		{
			item->AddToGround(pc->GetMapIndex(), pos);

			if (destroyTime != 0)
			{
				item->StartDestroyEvent(destroyTime);
			}
			if (ownershipTime != 0)
			{
				item->SetOwnership(pc, ownershipTime);
			}

			lua_pushboolean(L, true);
		}
		else 
		{
			lua_pushboolean(L, false);
		}

		return 1;
	}

	int32_t item_get_wearflag(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();

		lua_pushnumber(L, item ? item->GetWearFlag() : 0);
		return 1;
	}

	int32_t item_is_wearflag(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		CItem* item = CQuestManager::Instance().GetCurrentItem();
		if (item)
		{
			lua_pushboolean(L, (item->GetProto()->dwWearFlags & (uint32_t)lua_tonumber(L, 1)));
			return 1;
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	int32_t item_get_attr(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();
		if (!lua_isnumber(L, 1) || !item)
		{
			return 0;
		}

		int32_t attr_index = (int32_t)lua_tonumber(L, 1);
		if (attr_index < 0 || attr_index > 6)
		{
			return 0;
		}

		TPlayerItemAttribute attr = item->GetAttribute(attr_index);
		lua_pushnumber(L, attr.bType);
		lua_pushnumber(L, attr.sValue);
		return 2;
	}

	int32_t item_set_attr(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !item)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		int32_t attr_index = (int32_t)lua_tonumber(L, 1);
		if (attr_index < 0 || attr_index > 6)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		item->SetForceAttribute(attr_index, (uint8_t)lua_tonumber(L, 2), (int16_t)lua_tonumber(L, 3));
		lua_pushboolean(L, true);
		return 1;
	}

	int32_t item_equip(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1) || !item || !ch)
		{
			lua_pushboolean(L, false);
			return 1;
		}
		int32_t equipcell = (int32_t)lua_tonumber(L, 1);

		CItem* wear = ch->GetWear(equipcell);
		if (wear)
		{
			ch->UnequipItem(wear);
		}

		item->EquipTo(ch, equipcell);

		lua_pushboolean(L, true);
		return 1;
	}

	int32_t item_get_flag(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();

		lua_pushnumber(L, item ? item->GetFlag() : 0);
		return 1;
	}


	int32_t item_get_antiflag(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();

		lua_pushnumber(L, item ? item->GetAntiFlag() : 0);
		return 1;
	}

	int32_t item_has_antiflag(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("Invalid argument.");
			return 0;
		}
		int32_t lAntiCheck = (int32_t)lua_tonumber(L, 1);

		CItem* item = CQuestManager::Instance().GetCurrentItem();
		if (!item) 
			return 0;

		lua_pushboolean(L, IS_SET(item->GetAntiFlag(), lAntiCheck));
		return 1;
	}

	int32_t item_get_refine_set(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();

		lua_pushnumber(L, item ? item->GetRefineSet() : 0);
		return 1;
	}

	int32_t item_get_limit(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();
		if (!item)
			return 0;

		if (!lua_isnumber(L, 1))
		{
			sys_err("Wrong argument, need a number from range(0..%d)!", ITEM_LIMIT_MAX_NUM);
			lua_pushnumber(L, 0);
			return 1;
		}
		int32_t byLimitIndex = (int32_t)lua_tonumber(L, 1);

		if (byLimitIndex < 0 || byLimitIndex >= ITEM_LIMIT_MAX_NUM)
		{
			sys_err("Invalid limit type(%d). Out of range(0..%d)", byLimitIndex, ITEM_LIMIT_MAX_NUM);
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_newtable(L);
		{
			lua_pushnumber(L, item->GetLimitType(byLimitIndex));
			lua_rawseti(L, -2, 1);

			lua_pushnumber(L, item->GetLimitValue(byLimitIndex));
			lua_rawseti(L, -2, 2);
		}
		return 1;
	}

	int32_t item_get_apply(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();
		if (!item)
		{
			sys_err("No current item selected!");
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("Wrong argument, need a number from range(0..%d)!", ITEM_APPLY_MAX_NUM);
			lua_pushnumber(L, 0);
			return 1;
		}
		int32_t bApplyIndex = (int32_t)lua_tonumber(L, 1);

		if (bApplyIndex < 0 || bApplyIndex >= ITEM_APPLY_MAX_NUM)
		{
			sys_err("Invalid apply index(%d). Out of range(0..%d)", bApplyIndex, ITEM_APPLY_MAX_NUM);
			lua_pushnumber(L, 0);
			return 1;
		}

		const TItemTable* itemTable = item->GetProto();
		if (!itemTable)
		{
			sys_err("itemTable null ptr");
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_newtable(L);
		{
			lua_pushnumber(L, itemTable->aLimits[bApplyIndex].bType);
			lua_rawseti(L, -2, 1);

			lua_pushnumber(L, itemTable->aLimits[bApplyIndex].lValue);
			lua_rawseti(L, -2, 2);
		}
		return 1;
	}

	int32_t item_get_applies(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();
		if (!item)
		{
			sys_err("No current item selected!");
			return 0;
		}

		const TItemTable* itemTable = item->GetProto();
		if (!itemTable)
		{
			sys_err("itemTable null ptr");
			return 0;
		}

		lua_newtable(L);
		{
			for (uint8_t i = 0; i < ITEM_APPLY_MAX_NUM; i++)
			{
				lua_newtable(L);
				lua_pushnumber(L, itemTable->aLimits[i].bType);
				lua_rawseti(L, -2, 1);

				lua_pushnumber(L, itemTable->aLimits[i].lValue);
				lua_rawseti(L, -2, 2);
				lua_rawseti(L, -2, i);
			}
		}

		return 1;
	}

	int32_t item_get_refine_materials(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();
		if (!item)
		{
			sys_err("No current item selected!");
			return 0;
		}

		const TRefineTable * prt = CRefineManager::Instance().GetRefineRecipe(item->GetRefineSet());
		if (!prt)
		{
			sys_err("Failed to get refine materials!");
			return 0;
		}

		if (prt->cost == 0 && prt->material_count == 0)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_newtable(L);
		{
			lua_pushstring(L, "cost");
			lua_pushnumber(L, prt->cost);
			lua_rawset(L, -3);

			lua_pushstring(L, "material_count");
			lua_pushnumber(L, prt->material_count);
			lua_rawset(L, -3);

			lua_pushstring(L, "materials");
			lua_newtable(L);
			{
				for (uint8_t i = 0; i < prt->material_count; i++)
				{
					lua_newtable(L);
					lua_pushnumber(L, prt->materials[i].vnum);
					lua_rawseti(L, -2, 1);

					lua_pushnumber(L, prt->materials[i].count);
					lua_rawseti(L, -2, 2);
					lua_rawseti(L, -2, i + 1);
				}
			}
			lua_rawset(L, -3);
		}

		return 1;
	}

	int32_t item_dec(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();
		if (!item)
			return 0;

		int32_t dec = lua_isnumber(L, 1) ? (int32_t)lua_tonumber(L, 1) : 1;
		if (dec < 1 || dec > 250)
			dec = 1;

		if ((item->GetCount() - dec) < 0)
			return 0;

		if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK) || !item->IsStackable())
			return 0;

		item->SetCount(item->GetCount() - dec);
		return 0;
	}

	int32_t item_inc(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();
		if (!item)
			return 0;

		int32_t inc = lua_isnumber(L, 1) ? (int32_t)lua_tonumber(L, 1) : 1;
		if (inc < 1 || inc > 250)
			inc = 1;

		if ((item->GetCount() + inc) > 250)
			return 0;

		if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK) || !item->IsStackable())
			return 0;

		item->SetCount(item->GetCount() + inc);
		return 0;
	}

	int32_t item_add_attribute(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();
		if (!item)
			return 0;

		if (item->GetType() == ITEM_COSTUME)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		bool bRet = true;
		if (item->GetAttributeCount() < 5)
			item->AddAttribute();
		else
			bRet = false;

		lua_pushboolean(L, bRet);
		return 1;
	}

	int32_t item_get_attribute(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();
		if (!item)
			return 0;

		if (!lua_isnumber(L, 1))
		{
			sys_err("Wrong argument, need a number from range(0..%d)!", ITEM_ATTRIBUTE_MAX_NUM - 2);

			lua_pushnumber(L, 0);
			return 1;
		}

		int32_t iAttrIndex = (int32_t)lua_tonumber(L, 1);
		if (iAttrIndex < 0 || iAttrIndex >= ITEM_ATTRIBUTE_MAX_NUM - 2)
		{
			sys_err("Invalid index %d. Index out of range(0..%d)", iAttrIndex, ITEM_ATTRIBUTE_MAX_NUM - 2);

			lua_pushnumber(L, 0);
			return 1;
		}

		const TPlayerItemAttribute& AttrItem = item->GetAttribute(iAttrIndex);

		lua_newtable(L);

		lua_pushnumber(L, AttrItem.bType);
		lua_rawseti(L, -2, 1);

		lua_pushnumber(L, AttrItem.sValue);
		lua_rawseti(L, -2, 2);

		return 1;
	}

	int32_t item_set_attribute(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();
		if (!item) 
			return 0;

		if (item->GetType() == ITEM_COSTUME)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("Wrong argument[AttrIdx] #1.");

			lua_pushboolean(L, false);
			return 1;
		}
		else if (!lua_isnumber(L, 2))
		{
			sys_err("Wrong argument[AttrType] #2.");

			lua_pushboolean(L, false);
			return 1;
		}
		else if (!lua_isnumber(L, 3))
		{
			sys_err("Wrong argument[AttrValue] #3.");

			lua_pushboolean(L, false);
			return 1;
		}

		int32_t bAttrIndex = (int32_t)lua_tonumber(L, 1);
		if (bAttrIndex < 0 || bAttrIndex >= ITEM_ATTRIBUTE_MAX_NUM - 2)
		{
			sys_err("Invalid AttrIndex %d. AttrIndex out of range(0..4)", bAttrIndex);

			lua_pushboolean(L, false);
			return 1;
		}

		int32_t bAttrType = (int32_t)lua_tonumber(L, 2);
		if (bAttrType < 1 || bAttrType >= MAX_APPLY_NUM)
		{
			sys_err("Invalid AttrType %d. AttrType out of range(1..%d)", MAX_APPLY_NUM);

			lua_pushboolean(L, false);
			return 1;
		}

		if (item->HasAttr(bAttrType) && (item->GetAttribute(bAttrIndex).bType != bAttrType))
		{
			sys_err("AttrType[%d] multiplicated.", bAttrType);

			lua_pushboolean(L, false);
			return 1;
		}

		int32_t bAttrValue = (int32_t)lua_tonumber(L, 3);
		if (bAttrValue < 1 || bAttrValue > 32767)
		{
			sys_err("Invalid AttrValue %d. AttrValue should be between 1 and 32767!", bAttrValue);

			lua_pushboolean(L, false);
			return 1;
		}

		bool bRet = true;
		int32_t bAttrCount = item->GetAttributeCount();
		if (bAttrCount <= 4 && bAttrCount >= 0)
		{
			if (bAttrCount < bAttrIndex)
				bAttrIndex = bAttrCount;

			item->SetForceAttribute(bAttrIndex, bAttrType, bAttrValue);
		}
		else
			bRet = false;

		lua_pushboolean(L, bRet);
		return 1;
	}

	int32_t item_change_attribute(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();
		if (!item) 
			return 0;

		if (item->GetType() == ITEM_COSTUME)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		bool bRet = true;
		if (item->GetAttributeCount() > 0)
			item->ChangeAttribute();
		else
			bRet = false;

		lua_pushboolean(L, bRet);
		return 1;
	}

	int32_t item_add_rare_attribute(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();
		if (!item) 
			return 0;

		if (item->GetType() == ITEM_COSTUME)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		lua_pushboolean(L, item->AddRareAttribute() ? true : false);
		return 1;
	}

	int32_t item_get_rare_attribute(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();
		if (!item) 
			return 0;

		if (item->GetType() == ITEM_COSTUME)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("Wrong argument, need a number from range(0..1)!");

			lua_pushnumber(L, 0);
			return 1;
		}

		int32_t iRareAttrIndex = (int32_t)lua_tonumber(L, 1);
		if (iRareAttrIndex < 0 || iRareAttrIndex > 1)
		{
			sys_err("Invalid index %d. Index out of range(0..1)", iRareAttrIndex);

			lua_pushboolean(L, false);
			return 1;
		}

		const TPlayerItemAttribute& RareAttrItem = item->GetAttribute(iRareAttrIndex + 5);

		lua_newtable(L);

		lua_pushnumber(L, RareAttrItem.bType);
		lua_rawseti(L, -2, 1);

		lua_pushnumber(L, RareAttrItem.sValue);
		lua_rawseti(L, -2, 2);
		return 1;
	}

	int32_t item_set_rare_attribute(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();
		if (!item) 
			return 0;

		if (item->GetType() == ITEM_COSTUME)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("Wrong argument[AttrIdx], not number!");

			lua_pushboolean(L, false);
			return 1;
		}
		else if (!lua_isnumber(L, 2))
		{
			sys_err("Wrong argument[AttrType], not number!");

			lua_pushboolean(L, false);
			return 1;
		}
		else if (!lua_isnumber(L, 3))
		{
			sys_err("Wrong argument[AttrValue], not number!");

			lua_pushboolean(L, false);
			return 1;
		}

		int32_t iRareAttrIndex = (int32_t)lua_tonumber(L, 1);
		if (iRareAttrIndex < 0 || iRareAttrIndex > 1)
		{
			sys_err("Invalid index %d. Index out of range(0..1)", iRareAttrIndex);

			lua_pushboolean(L, false);
			return 1;
		}

		int32_t iRareAttrType = (int32_t)lua_tonumber(L, 2);
		if (iRareAttrType < 1 || iRareAttrType >= MAX_APPLY_NUM)
		{
			sys_err("Invalid apply %d. Apply out of range(1..%d)", MAX_APPLY_NUM);

			lua_pushboolean(L, false);
			return 1;
		}

		if (item->HasAttr(iRareAttrType) && (item->GetAttribute(iRareAttrIndex).bType != iRareAttrType))
		{
			sys_err("Apply %d muliplicated.", iRareAttrType);

			lua_pushboolean(L, false);
			return 1;
		}

		int32_t iRareAttrValue = (int32_t)lua_tonumber(L, 3);
		if (iRareAttrValue < 1 || iRareAttrValue > 32767)
		{
			sys_err("Invalid value %d. The value should be between 1 and 32767!", iRareAttrValue);

			lua_pushboolean(L, false);
			return 1;
		}

		bool bRet = true;
		int32_t iRareAttrCount = item->GetRareAttrCount();
		if (iRareAttrCount <= 1 && iRareAttrCount >= 0)
		{
			if (iRareAttrCount < iRareAttrIndex)
				iRareAttrIndex = iRareAttrCount;

			item->SetForceAttribute(iRareAttrIndex + 5, iRareAttrType, iRareAttrValue);
		}
		else
			bRet = false;

		lua_pushboolean(L, bRet);
		return 1;
	}

	int32_t item_change_rare_attribute(lua_State* L)
	{
		CItem* item = CQuestManager::Instance().GetCurrentItem();
		if (!item) 
			return 0;

		if (item->GetType() == ITEM_COSTUME)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		lua_pushboolean(L, item->ChangeRareAttribute());
		return 1;
	}

	int32_t item_equip_selected(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("Argument error.");

			lua_pushboolean(L, false);
			return 1;
		}

		int32_t bCell = (int32_t)lua_tonumber(L, 1);
		if (bCell < 0 || bCell >= WEAR_MAX_NUM)
		{
			sys_err("Invalid wear position %d. Index out of range(0..%d)", bCell, WEAR_MAX_NUM);

			lua_pushboolean(L, false);
			return 1;
		}

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");

			lua_pushboolean(L, false);
			return 1;
		}

		CItem* item = CQuestManager::Instance().GetCurrentItem();
		CItem* equipped = ch->GetWear(bCell);
		if (!item || !equipped)
		{
			sys_err("null item ptr");

			lua_pushboolean(L, false);
			return 1;
		}

		//remove the equipped item
		if (equipped->GetVnum() || item->IsEquipped())
			ch->UnequipItem(equipped);

		//equipping the item to the given slot
		item->EquipTo(ch, bCell);

		lua_pushboolean(L, true);
		return 1;
	}


	void RegisterITEMFunctionTable()
	{

		luaL_reg item_functions[] =
		{
			{ "get_id",		item_get_id		},
			{ "get_cell",		item_get_cell		},
			{ "select",		item_select		},
			{ "select_cell",	item_select_cell	},
			{ "remove",		item_remove		},
			{ "get_socket",		item_get_socket		},
			{ "set_socket",		item_set_socket		},
			{ "get_vnum",		item_get_vnum		},
			{ "has_flag",		item_has_flag		},
			{ "get_value",		item_get_value		},
			{ "set_value",		item_set_value		},
			{ "get_name",		item_get_name		},
			{ "get_size",		item_get_size		},
			{ "get_count",		item_get_count		},
			{ "get_type",		item_get_type		},
			{ "get_sub_type",	item_get_sub_type	},
			{ "get_refine_vnum",	item_get_refine_vnum	},
			{ "get_level",		item_get_level		},
			{ "next_refine_vnum",	item_next_refine_vnum	},

			{ "get_attr",						item_get_attr						},
			{ "set_attr",						item_set_attr						},
			{ "is_wearflag",					item_is_wearflag					},
			{ "equip",							item_equip							},
			{ "get_count",						item_get_count						},
			{ "get_vnum",						item_get_vnum						},
			{ "get_immuneflag",					item_get_immuneflag					},
			{ "has_immuneflag",					item_has_immuneflag					},
			{ "get_flag",						item_get_flag                       },
			{ "get_wearflag",					item_get_wearflag                   },
			{ "get_antiflag",					item_get_antiflag                   },
			{ "has_antiflag",					item_has_antiflag                   },
			{ "get_refine_set",					item_get_refine_set                 },
			{ "get_limit",						item_get_limit                      },
			{ "get_apply",						item_get_apply                      },
			{ "get_applies",					item_get_applies,                   },
			{ "get_refine_materials",			item_get_refine_materials           },
			{ "dec",							item_dec                            },
			{ "inc",							item_inc                            },
			{ "add_attribute",					item_add_attribute                  },
			{ "get_attribute",					item_get_attribute                  },
			{ "set_attribute",					item_set_attribute                  },
			{ "change_attribute",               item_change_attribute               },
			{ "add_rare_attribute",             item_add_rare_attribute             },
			{ "get_rare_attribute",             item_get_rare_attribute             },
			{ "set_rare_attribute",             item_set_rare_attribute             },
			{ "change_rare_attribute",          item_change_rare_attribute          },
			{ "set_count",						item_set_count                      },
			{ "is_available",					item_is_available					},
			{ "to_equipment",					item_to_equipment					},
			{ "to_character",					item_to_character					},
			{ "to_ground",						item_to_ground						},
			{ "get_level_limit", 				item_get_level_limit },
			{ "start_realtime_expire", 			item_start_realtime_expire },
			{ "copy_and_give_before_remove",	item_copy_and_give_before_remove},

			{ nullptr,			nullptr			}
		};
		CQuestManager::Instance().AddLuaFunctionTable("item", item_functions);
	}
}
