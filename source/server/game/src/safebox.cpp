#include "stdafx.h"
#include "../../libgame/include/grid.h"
#include "constants.h"
#include "safebox.h"
#include "char.h"
#include "desc_client.h"
#include "item.h"
#include "item_manager.h"
#include "config.h"

CSafebox::CSafebox(LPCHARACTER pkChrOwner, int32_t iSize) :
	m_pkChrOwner(pkChrOwner), m_iSize(iSize)
{
	assert(m_pkChrOwner != nullptr);
	memset(m_pkItems, 0, sizeof(m_pkItems));

	if (m_iSize)
		m_pkGrid = M2_NEW CGrid(5, m_iSize);
	else
		m_pkGrid = nullptr;

	m_bWindowMode = SAFEBOX;
}

CSafebox::~CSafebox()
{
	__Destroy();
}

void CSafebox::SetWindowMode(uint8_t bMode)
{
	m_bWindowMode = bMode;
}

void CSafebox::__Destroy()
{
	for (int32_t i = 0; i < SAFEBOX_MAX_NUM; ++i)
	{
		if (m_pkItems[i])
		{
			m_pkItems[i]->SetSkipSave(true);
			ITEM_MANAGER::Instance().FlushDelayedSave(m_pkItems[i]);

			M2_DESTROY_ITEM(m_pkItems[i]->RemoveFromCharacter());
			m_pkItems[i] = nullptr;
		}
	}

	if (m_pkGrid)
	{
		M2_DELETE(m_pkGrid);
		m_pkGrid = nullptr;
	}
}

bool CSafebox::Add(uint32_t dwPos, LPITEM pkItem)
{
	if (!IsValidPosition(dwPos))
	{
		sys_err("SAFEBOX: item on wrong position at %d (size of grid = %d)", dwPos, m_pkGrid->GetSize());
		return false;
	}

	pkItem->SetWindow(m_bWindowMode);
	pkItem->SetCell(m_pkChrOwner, dwPos);
	pkItem->Save(); // 강제로 Save를 불러줘야 한다.
	ITEM_MANAGER::Instance().FlushDelayedSave(pkItem);

	m_pkGrid->Put(dwPos, 1, pkItem->GetSize());
	m_pkItems[dwPos] = pkItem;

	if (m_pkChrOwner->GetDesc())
	{
		if (m_bWindowMode == SAFEBOX)
		{
			SPacketGCSafeboxSet pack;
			pack.header = HEADER_GC_SAFEBOX_SET;
			pack.Cell = TItemPos(m_bWindowMode, dwPos);
			pack.vnum = pkItem->GetVnum();
			pack.count = pkItem->GetCount();
			memcpy(pack.alSockets, pkItem->GetSockets(), sizeof(pack.alSockets));
			memcpy(pack.aAttr, pkItem->GetAttributes(), sizeof(pack.aAttr));
			m_pkChrOwner->GetDesc()->Packet(&pack, sizeof(pack));
		}
		else
		{
			SPacketGCMallSet pack;
			pack.header = HEADER_GC_MALL_SET;
			pack.Cell = TItemPos(m_bWindowMode, dwPos);
			pack.vnum = pkItem->GetVnum();
			pack.count = pkItem->GetCount();
			memcpy(pack.alSockets, pkItem->GetSockets(), sizeof(pack.alSockets));
			memcpy(pack.aAttr, pkItem->GetAttributes(), sizeof(pack.aAttr));
			m_pkChrOwner->GetDesc()->Packet(&pack, sizeof(pack));
		}
	}
	sys_log(1, "SAFEBOX: ADD %s %s count %d", m_pkChrOwner->GetName(), pkItem->GetName(), pkItem->GetCount());
	return true;
}

LPITEM CSafebox::Get(uint32_t dwPos)
{
	if (dwPos >= m_pkGrid->GetSize())
		return nullptr;

	return m_pkItems[dwPos];
}

LPITEM CSafebox::Remove(uint32_t dwPos)
{
	LPITEM pkItem = Get(dwPos);

	if (!pkItem)
		return nullptr;

	if (!m_pkGrid)
		sys_err("Safebox::Remove : nil grid");
	else
		m_pkGrid->Get(dwPos, 1, pkItem->GetSize());

	pkItem->RemoveFromCharacter();

	m_pkItems[dwPos] = nullptr;

	if (m_pkChrOwner->GetDesc())
	{
		if (m_bWindowMode == SAFEBOX)
		{
			SPacketGCSafeboxDel pack;

			pack.header = HEADER_GC_SAFEBOX_DEL;
			pack.pos = dwPos;
			m_pkChrOwner->GetDesc()->Packet(&pack, sizeof(pack));
		}
		else
		{
			SPacketGCMallDel pack;

			pack.header = HEADER_GC_MALL_DEL;
			pack.pos = dwPos;
			m_pkChrOwner->GetDesc()->Packet(&pack, sizeof(pack));
		}
	}

	sys_log(1, "SAFEBOX: REMOVE %s %s count %d", m_pkChrOwner->GetName(), pkItem->GetName(), pkItem->GetCount());
	return pkItem;
}

void CSafebox::Save()
{
	TSafeboxTable t;

	memset(&t, 0, sizeof(TSafeboxTable));

	t.dwID = m_pkChrOwner->GetDesc()->GetAccountTable().id;
	t.dwGold = 0;

	db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_SAVE, 0, &t, sizeof(TSafeboxTable));
	sys_log(1, "SAFEBOX: SAVE %s", m_pkChrOwner->GetName());
}

bool CSafebox::IsEmpty(uint32_t dwPos, uint8_t bSize)
{
	if (!m_pkGrid)
		return false;

	return m_pkGrid->IsEmpty(dwPos, 1, bSize);
}

void CSafebox::ChangeSize(int32_t iSize)
{
	// 현재 사이즈가 인자보다 크면 사이즈를 가만 둔다.
	if (m_iSize >= iSize)
		return;

	m_iSize = iSize;

	CGrid * pkOldGrid = m_pkGrid;

	if (pkOldGrid)
	{
		m_pkGrid = M2_NEW CGrid(pkOldGrid, 5, m_iSize);
		delete pkOldGrid;
	}
	else
		m_pkGrid = M2_NEW CGrid(5, m_iSize);
}

LPITEM CSafebox::GetItem(uint8_t bCell)
{
	if (bCell >= 5 * m_iSize)
	{
		sys_err("CHARACTER::GetItem: invalid item cell %d", bCell);
		return nullptr;
	}

	return m_pkItems[bCell];
}

bool CSafebox::MoveItem(uint8_t bCell, uint8_t bDestCell, uint8_t count)
{

	int32_t max_position = 5 * m_iSize;

	if (bCell >= max_position || bDestCell >= max_position)
		return false;

	LPITEM item = GetItem(bCell);
	if (!item)
		return false;

	if (item->IsExchanging())
		return false;

	if (item->GetCount() < count)
		return false;

	{
		LPITEM item2 = GetItem(bDestCell);

		if (item2 && item != item2 && item2->IsStackable() &&
				!IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_STACK) &&
				item2->GetVnum() == item->GetVnum()) // 합칠 수 있는 아이템의 경우
		{
			for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
				if (item2->GetSocket(i) != item->GetSocket(i))
					return false;

			if (count == 0)
				count = item->GetCount();

			count = MIN(g_bItemCountLimit - item2->GetCount(), count);

			if (item->GetCount() >= count)
				Remove(bCell);

			item->SetCount(item->GetCount() - count);
			item2->SetCount(item2->GetCount() + count);

			sys_log(1, "SAFEBOX: STACK %s %d -> %d %s count %d", m_pkChrOwner->GetName(), bCell, bDestCell, item2->GetName(), item2->GetCount());
			return true;
		}

		if (!IsEmpty(bDestCell, item->GetSize()))
			return false;

		m_pkGrid->Get(bCell, 1, item->GetSize());

		if (!m_pkGrid->Put(bDestCell, 1, item->GetSize()))
		{
			m_pkGrid->Put(bCell, 1, item->GetSize());
			return false;
		}
		else
		{
			m_pkGrid->Get(bDestCell, 1, item->GetSize());
			m_pkGrid->Put(bCell, 1, item->GetSize());
		}

		sys_log(1, "SAFEBOX: MOVE %s %d -> %d %s count %d", m_pkChrOwner->GetName(), bCell, bDestCell, item->GetName(), item->GetCount());

		Remove(bCell);
		Add(bDestCell, item);
	}

	return true;
}

bool CSafebox::IsValidPosition(uint32_t dwPos)
{
	if (!m_pkGrid)
		return false;

	if (dwPos >= m_pkGrid->GetSize())
		return false;

	return true;
}

