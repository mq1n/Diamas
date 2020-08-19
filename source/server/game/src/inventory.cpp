#include "stdafx.h"
#include "config.h"
#include "char.h"
#include "item.h"
#include "quest_manager.h"
#include "item_manager.h"
#include "log.h"
#include "inventory.h"

LPITEM CInventory::FindSameItem(LPITEM item, int32_t fromPos, int32_t untilPos) const
{
	for (int32_t i = fromPos; i < untilPos; ++i)
	{
		LPITEM cItem = GetItemAtPos(i);
		if (!cItem)
			continue;

		if (cItem->GetVnum() == item->GetVnum())
		{
			//Vnum matches, verify sockets
			bool matched = true;
			for (int32_t j = 0; j < ITEM_SOCKET_MAX_NUM && matched; ++j)
				matched = cItem->GetSocket(j) == item->GetSocket(j);

			if (matched)
				return cItem;
		}
	}

	return nullptr; // (We got here, nothing found)
}

void CInventory::TryStack(int32_t fromInventoryPos, int32_t untilInventoryPos, LPITEM item, LPITEM &match, uint8_t &bCount)
{
	if (!item || !bCount) {
		sys_err("Invalid parameters passed.");
		match = nullptr;
		bCount = 0;
		return;
	}

	//Keep trying to stack until we run out of things to stack or places to stack onto
	while (bCount > 0 && ((match = FindSameItem(item, fromInventoryPos, untilInventoryPos))))
	{
		fromInventoryPos = match->GetCell() + 1;
		uint8_t mCount = (uint8_t)match->GetCount();
		uint8_t newCount = MIN(g_bItemCountLimit, mCount + bCount);

		if (newCount > mCount)
		{
			bCount -= newCount - mCount;
			match->SetCount(newCount);
		}
	}
}

CInventory::EStatus CInventory::Add(LPITEM &item)
{
	if (!item)
		return EStatus::ERROR_UNDEFINED;

	// If the item is stackable, try to stack it
	if (item->IsStackable())
	{
		uint8_t bCount = (uint8_t)item->GetCount();
		LPITEM match = nullptr;

		//Try to stack on belt and on inventory afterwards
		TryStack(BELT_INVENTORY_SLOT_START, BELT_INVENTORY_SLOT_END, item, match, bCount);
		if (bCount > 0)
#ifdef __ENABLE_EXTEND_INVEN_SYSTEM__
			TryStack(0, m_owner->GetExtendInvenMax(), item, match, bCount);
#else
//			TryStack(0, INVENTORY_MAX_COUNT, item, match, bCount);
#endif

		if (bCount == 0)
		{
			// We're left with nothing more, we stacked the item in its
			// entirety. Delete the old one.
			M2_DESTROY_ITEM(item);
			item = nullptr;
			OUT item = match; // For all purposes, the item is now the last matched one
			return EStatus::SUCCESS;
		}
		// Otherwise, we need to update the old item count
		// (maybe we managed to stack a part of it)
		item->SetCount(bCount);
	}

	// Either we couldn't stack it, or we only stacked a portion.
	// We need to place the item in the inventory!
	int32_t iEmptyCell;
	if (item->IsDragonSoul())
	{
		if ((iEmptyCell = m_owner->GetEmptyDragonSoulInventory(item)) == -1)
		{
			sys_log(0, "No empty ds inventory p#%u size %d i#%u", m_owner->GetPlayerID(), item->GetSize(), item->GetID());
			return EStatus::ERROR_NO_SPACE;
		}
	}
	else
	{
		if ((iEmptyCell = m_owner->GetEmptyInventory(item->GetSize())) == -1)
		{
			sys_log(0, "No empty inventory p#%u size %d i#%u", m_owner->GetPlayerID(), item->GetSize(), item->GetID());
			return EStatus::ERROR_NO_SPACE;
		}
	}

	// If the item is on the ground, we will remove it. Else, this won't do anything
	item->RemoveFromGround();

	// Bind the item to the character to the cell
	if (item->IsDragonSoul())
		item->AddToCharacter(m_owner, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell));
	else
		item->AddToCharacter(m_owner, TItemPos(INVENTORY, iEmptyCell));

	return EStatus::SUCCESS;
};

LPITEM CInventory::GetItemAtPos(const uint16_t& wCell) const
{
#ifdef __ENABLE_EXTEND_INVEN_SYSTEM__
	if (wCell < m_owner->GetExtendInvenMax() || (wCell >= BELT_INVENTORY_SLOT_START && wCell < BELT_INVENTORY_SLOT_END))
		return m_owner->GetItem(TItemPos(INVENTORY, wCell));
#else
//	if (wCell < INVENTORY_MAX_COUNT || (wCell >= BELT_INVENTORY_SLOT_START && wCell < BELT_INVENTORY_SLOT_END))
//		return m_owner->GetItem(TItemPos(INVENTORY, wCell));
#endif
	if (wCell < DRAGON_SOUL_INVENTORY_MAX_NUM)
		return m_owner->GetItem(TItemPos(DRAGON_SOUL_INVENTORY, wCell));

	return nullptr;
}

#ifdef __ENABLE_EXTEND_INVEN_SYSTEM__

//TODO BETTER!
int CInventory::CheckExtendAvailableSlots3(LPCHARACTER pc)
{
	if (pc->GetExtendInvenStage() == 1)
		return 1;
	else if (pc->GetExtendInvenStage() == 2)
		return 2;
	else if (pc->GetExtendInvenStage() == 3)
		return 3;
	else if (pc->GetExtendInvenStage() == 4)
		return 4;
	else if (pc->GetExtendInvenStage() == 5)
		return 5;
	else if (pc->GetExtendInvenStage() == 6)
		return 6;
	else if (pc->GetExtendInvenStage() == 7)
		return 7;
	else if (pc->GetExtendInvenStage() == 8)
		return 8;
	else if (pc->GetExtendInvenStage() >= 9)
		return 9;

	return 0;
}

int CInventory::CheckExtendAvailableSlots4(LPCHARACTER pc)
{
	if (pc->GetExtendInvenStage() == 10)
		return 1;
	else if (pc->GetExtendInvenStage() == 11)
		return 2;
	else if (pc->GetExtendInvenStage() == 12)
		return 3;
	else if (pc->GetExtendInvenStage() == 13)
		return 4;
	else if (pc->GetExtendInvenStage() == 14)
		return 5;
	else if (pc->GetExtendInvenStage() == 15)
		return 6;
	else if (pc->GetExtendInvenStage() == 16)
		return 7;
	else if (pc->GetExtendInvenStage() == 17)
		return 8;
	else if (pc->GetExtendInvenStage() >= 18)
		return 9;

	return 0;
}

int CInventory::CheckExtendAvailableSlots5(LPCHARACTER pc)
{
	if (pc->GetExtendInvenStage() == 19)
		return 1;
	else if (pc->GetExtendInvenStage() == 20)
		return 2;
	else if (pc->GetExtendInvenStage() == 21)
		return 3;
	else if (pc->GetExtendInvenStage() == 22)
		return 4;
	else if (pc->GetExtendInvenStage() == 23)
		return 5;
	else if (pc->GetExtendInvenStage() == 24)
		return 6;
	else if (pc->GetExtendInvenStage() == 25)
		return 7;
	else if (pc->GetExtendInvenStage() == 26)
		return 8;
	else if (pc->GetExtendInvenStage() >= 27)
		return 9;

	return 0;
}

bool CInventory::IsAvailableCell(uint16_t cell, short ExtendSlots)
{
	return cell <= ExtendSlots;
}

//Used for packet 
int CInventory::GetExtendItemNeedCount(LPCHARACTER pc)
{
	if (pc->GetExtendInvenStage() < 3)
		return 2;
	else if (pc->GetExtendInvenStage() < 6)
		return 3;
	else if (pc->GetExtendInvenStage() < 9)
		return 4;
	else if (pc->GetExtendInvenStage() < 12)
		return 5;
	else if (pc->GetExtendInvenStage() < 15)
		return 6;
	else if (pc->GetExtendInvenStage() < 18)
		return 7;
	else if (pc->GetExtendInvenStage() < 21)
		return 8;
	else if (pc->GetExtendInvenStage() < 24)
		return 9;
	else if (pc->GetExtendInvenStage() < 27)
		return 10;

	return 0;
}

#endif