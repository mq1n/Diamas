#include "stdafx.h"
#include "constants.h"
#include "char.h"
#include "desc.h"
#include "desc_manager.h"
#include "item.h"

/////////////////////////////////////////////////////////////////////////////
// QUICKSLOT HANDLING
/////////////////////////////////////////////////////////////////////////////
void CHARACTER::SyncQuickslot(uint8_t bType, uint8_t bOldPos, uint8_t bNewPos) // bNewPos == 255 ¸é DELETE
{
	if (bOldPos == bNewPos)
		return;

	for (int32_t i = 0; i < QUICKSLOT_MAX_NUM; ++i)
	{
		if (m_quickslot[i].Type == bType && m_quickslot[i].Position == bOldPos)
		{
			if (bNewPos == 255)
				DelQuickslot(i);
			else
			{
				TQuickSlot slot;

				slot.Type = bType;
				slot.Position = bNewPos;

				SetQuickslot(i, slot);
			}
		}
	}
}

bool CHARACTER::GetQuickslot(uint8_t pos, TQuickSlot** ppSlot)
{
	if (pos >= QUICKSLOT_MAX_NUM)
		return false;

	*ppSlot = &m_quickslot[pos];
	return true;
}

bool CHARACTER::SetQuickslot(uint8_t pos, TQuickSlot& rSlot)
{
	SPacketGCQuickSlotAdd pack_quickslot_add;

	if (pos >= QUICKSLOT_MAX_NUM)
		return false;

	if (rSlot.Type >= QUICKSLOT_TYPE_MAX_NUM)
		return false;

	for (int32_t i = 0; i < QUICKSLOT_MAX_NUM; ++i)
	{
		if (rSlot.Type == 0)
			continue;
		else if (m_quickslot[i].Type == rSlot.Type && m_quickslot[i].Position == rSlot.Position)
			DelQuickslot(i);
	}

	TItemPos srcCell(INVENTORY, rSlot.Position);

	switch (rSlot.Type)
	{
		case QUICKSLOT_TYPE_ITEM:
			if (false == srcCell.IsDefaultInventoryPosition() && false == srcCell.IsBeltInventoryPosition())
				return false;

			break;

		case QUICKSLOT_TYPE_SKILL:
			if ((int32_t) rSlot.Position >= SKILL_MAX_NUM)
				return false;

			break;

		case QUICKSLOT_TYPE_COMMAND:
			break;

		default:
			return false;
	}

	m_quickslot[pos] = rSlot;

	if (GetDesc())
	{
		pack_quickslot_add.header	= HEADER_GC_QUICKSLOT_ADD;
		pack_quickslot_add.pos		= pos;
		pack_quickslot_add.slot		= m_quickslot[pos];

		GetDesc()->Packet(&pack_quickslot_add, sizeof(pack_quickslot_add));
	}

	return true;
}

bool CHARACTER::DelQuickslot(uint8_t pos)
{
	SPacketGCQuickSlotDel pack_quickslot_del;

	if (pos >= QUICKSLOT_MAX_NUM)
		return false;

	memset(&m_quickslot[pos], 0, sizeof(TQuickSlot));

	pack_quickslot_del.header	= HEADER_GC_QUICKSLOT_DEL;
	pack_quickslot_del.pos	= pos;

	GetDesc()->Packet(&pack_quickslot_del, sizeof(pack_quickslot_del));
	return true;
}

bool CHARACTER::SwapQuickslot(uint8_t a, uint8_t b)
{
	SPacketGCQuickSlotSwap pack_quickslot_swap;
	TQuickSlot quickslot;

	if (a >= QUICKSLOT_MAX_NUM || b >= QUICKSLOT_MAX_NUM)
		return false;

	// Äü ½½·Ô ÀÚ¸®¸¦ ¼­·Î ¹Ù²Û´Ù.
	quickslot = m_quickslot[a];

	m_quickslot[a] = m_quickslot[b];
	m_quickslot[b] = quickslot;

	pack_quickslot_swap.header	= HEADER_GC_QUICKSLOT_SWAP;
	pack_quickslot_swap.pos	= a;
	pack_quickslot_swap.change_pos = b;

	GetDesc()->Packet(&pack_quickslot_swap, sizeof(pack_quickslot_swap));
	return true;
}

void CHARACTER::ChainQuickslotItem(LPITEM pItem, uint8_t bType, uint8_t bOldPos)
{
	if (pItem->IsDragonSoul())
		return;
	for ( int32_t i=0; i < QUICKSLOT_MAX_NUM; ++i )
	{
		if ( m_quickslot[i].Type == bType && m_quickslot[i].Position == bOldPos )
		{
			TQuickSlot slot;
			slot.Type = bType;
			slot.Position = pItem->GetCell();

			SetQuickslot(i, slot);

			break;
		}
	}
}

