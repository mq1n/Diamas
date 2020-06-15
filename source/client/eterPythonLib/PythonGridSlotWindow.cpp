#include "StdAfx.h"
#include "../eterBase/CRC32.h"
#include "PythonGridSlotWindow.h"

using namespace UI;

void CGridSlotWindow::OnRenderPickingSlot()
{
	if (!CWindowManager::Instance().IsAttaching())
		return;

	uint8_t byWidth, byHeight;
	CWindowManager::Instance().GetAttachingIconSize(&byWidth, &byHeight);

	std::list<TSlot *> SlotList;
	if (GetPickedSlotList(byWidth, byHeight, &SlotList))
	{
		uint32_t dwSlotNumber = CWindowManager::Instance().GetAttachingSlotNumber();
		uint32_t dwItemIndex = CWindowManager::Instance().GetAttachingIndex();

		// UseMode 이고..
		if (m_isUseMode)
		{
			// Pick 된 아이템이 있으면..
			TSlot * pSlot = *SlotList.begin();
			TSlot * pCenterSlot;
			if (GetSlotPointer(pSlot->dwCenterSlotNumber, &pCenterSlot))
			{
				if (pCenterSlot->isItem)
				{
					if (m_isUsableItem)
						CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 0.0f, 0.5f);
					else if (pCenterSlot->dwItemIndex != dwItemIndex)
						CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.0f, 0.0f, 0.5f);
					else
						CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f, 0.5f);

					CPythonGraphic::Instance().RenderBar2d(
						m_rect.left + pCenterSlot->ixPosition, m_rect.top + pCenterSlot->iyPosition,
						m_rect.left + pCenterSlot->ixPosition + pCenterSlot->byxPlacedItemSize * ITEM_WIDTH,
						m_rect.top + pCenterSlot->iyPosition + pCenterSlot->byyPlacedItemSize * ITEM_HEIGHT
					);
					return;
				}
			}
		}

		// 아니면 그냥 옮기기
		if (CheckMoving(dwSlotNumber, dwItemIndex, SlotList))
			CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f, 0.5f);
		else if (CheckSwapping (dwSlotNumber, byHeight, SlotList))
			CPythonGraphic::Instance().SetDiffuseColor (0.22f, 0.67f, 0.65f, 0.5f);
		else
			CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.0f, 0.0f, 0.5f);

		RECT Rect;
		Rect.left = m_rect.right;
		Rect.top = m_rect.bottom;
		Rect.right = 0;
		Rect.bottom = 0;

		for (std::list<TSlot*>::iterator itor = SlotList.begin(); itor != SlotList.end(); ++itor)
		{
			TSlot * pSlot = *itor;
			Rect.left = std::min(Rect.left, m_rect.left + pSlot->ixPosition);
			Rect.top = std::min(Rect.top, m_rect.top + pSlot->iyPosition);
			Rect.right = std::max(Rect.right, m_rect.left + pSlot->ixPosition + pSlot->byxPlacedItemSize*ITEM_WIDTH);
			Rect.bottom = std::max(Rect.bottom, m_rect.top + pSlot->iyPosition + pSlot->byxPlacedItemSize*ITEM_HEIGHT);
		}

		CPythonGraphic::Instance().RenderBar2d(Rect.left, Rect.top, Rect.right, Rect.bottom);
	}
}

bool CGridSlotWindow::GetPickedSlotPointer(TSlot ** ppSlot)
{
	if (!CWindowManager::Instance().IsAttaching())
		return CSlotWindow::GetPickedSlotPointer(ppSlot);

	uint8_t byWidth, byHeight;
	CWindowManager::Instance().GetAttachingIconSize(&byWidth, &byHeight);

	std::list<TSlot*> SlotList;
	if (!GetPickedSlotList(byWidth, byHeight, &SlotList))
		return false;

	TSlot * pMinSlot = nullptr;
	//uint32_t dwSlotNumber = UI::CWindowManager::Instance().GetAttachingSlotNumber();
	//uint32_t dwAttachingItemIndex = UI::CWindowManager::Instance().GetAttachingIndex();

	for (auto & pSlot : SlotList)
	{
		// NOTE : 한 슬롯 이상 사이즈의 아이템의 경우 가장 왼쪽 위의 슬롯 포인터를 리턴한다.
		//        명시적이지 못한 코드.. 더 좋은 방법은 없는가? - [levites]
		if (!pMinSlot)
			pMinSlot = pSlot;
		else
		{
			if (pSlot->dwSlotNumber < pMinSlot->dwSlotNumber)
				pMinSlot = pSlot;
			else
			{
				if (m_iWindowType != UI::SLOT_WND_INVENTORY && !pMinSlot->isItem && pSlot->isItem)
					pMinSlot = pSlot;
			}
		}
	}

	if (!pMinSlot)
		return false;
	TSlot * pCenterSlot;
	if (!GetSlotPointer(pMinSlot->dwCenterSlotNumber, &pCenterSlot))
		return false;

	*ppSlot = pCenterSlot;

	// 현재 아이템을 들고 있는 중이고..
	if (CWindowManager::Instance().IsAttaching())
	{
		uint32_t dwSlotNumber = CWindowManager::Instance().GetAttachingSlotNumber();

		if (dwSlotNumber == pCenterSlot->dwSlotNumber)
			*ppSlot = pMinSlot;
	}

	return true;
}

bool CGridSlotWindow::GetPickedSlotList(int32_t iWidth, int32_t iHeight, std::list<TSlot*> * pSlotPointerList)
{
	int32_t lx, ly;
	GetMouseLocalPosition(lx, ly);

	if (lx < 0)
		return false;
	if (ly < 0)
		return false;
	if (lx >= GetWidth())
		return false;
	if (ly >= GetHeight())
		return false;

	pSlotPointerList->clear();

	int32_t ix, iy;
	if (GetPickedGridSlotPosition(lx, ly, &ix, &iy))
	{
		int32_t ixHalfStep = (iWidth / 2);
		//int32_t iyHalfStep = (iHeight / 2);

		int32_t ixStart = int32_t(ix) - int32_t(ixHalfStep - (ixHalfStep % 2));
		int32_t ixEnd = int32_t(ix) + int32_t(ixHalfStep);

		// FIXME : 제대로 된 계산 공식을 찾자 - [levites]
		int32_t iyStart = 0, iyEnd = 0;

		if (1 == iHeight)
		{
			iyStart = iy;
			iyEnd = iy;
		}
		else if (2 == iHeight)
		{
			iyStart = iy;
			iyEnd = iy + 1;
		}
		else if (3 == iHeight)
		{
			iyStart = iy - 1;
			iyEnd = iy + 1;
		}

		if (ixStart < 0)
		{
			ixEnd += -ixStart;
			ixStart = 0;
		}

		if (iyStart < 0)
		{
			iyEnd += -iyStart;
			iyStart = 0;
		}

		if (uint32_t(ixEnd) >= m_dwxCount)
		{
			int32_t ixTemporary = uint32_t(ixEnd) - m_dwxCount + 1;
			ixStart -= ixTemporary;
			ixEnd -= ixTemporary;
		}

		if (uint32_t(iyEnd) >= m_dwyCount)
		{
			int32_t iyTemporary = uint32_t(iyEnd) - m_dwyCount + 1;
			iyStart -= iyTemporary;
			iyEnd -= iyTemporary;
		}

		for (int32_t i = ixStart; i <= ixEnd; ++i)
		{
			for (int32_t j = iyStart; j <= iyEnd; ++j)
			{
				TSlot * pSlot;
				if (GetGridSlotPointer(uint32_t(i), uint32_t(j), &pSlot))
					pSlotPointerList->emplace_back(pSlot);
			}
		}

		// Refine Scroll 등을 위한 예외 처리
		if (m_isUseMode && 1 == pSlotPointerList->size())
		{
			TSlot * pMainSlot = *pSlotPointerList->begin();

			auto itor = m_SlotVector.begin();
			for (; itor != m_SlotVector.end(); ++itor)
			{
				TSlot * pSlot = *itor;
				if (pSlot->dwCenterSlotNumber == pMainSlot->dwCenterSlotNumber)
				{
					if (pSlotPointerList->end() == std::find(pSlotPointerList->begin(), pSlotPointerList->end(), pSlot))
						pSlotPointerList->emplace_back(pSlot);
				}
			}
		}

		if (!pSlotPointerList->empty())
			return true;
	}

	return false;
}

bool CGridSlotWindow::GetGridSlotPointer(int32_t ix, int32_t iy, TSlot ** ppSlot)
{
	uint32_t dwSlotIndex = ix + iy*m_dwxCount;
	if (dwSlotIndex >= m_SlotVector.size())
		return false;

	*ppSlot = m_SlotVector[dwSlotIndex];

	return true;
}

bool CGridSlotWindow::GetSlotPointerByNumber (uint32_t dwSlotNumber, TSlot** ppSlot)
{
	if (dwSlotNumber >= m_SlotVector.size())
		return false;

	*ppSlot = m_SlotVector[dwSlotNumber];

	return true;
}

bool CGridSlotWindow::GetPickedGridSlotPosition(int32_t ixLocal, int32_t iyLocal, int32_t * pix, int32_t * piy)
{
	for (uint32_t x = 0; x < m_dwxCount; ++x)
	{
		for (uint32_t y = 0; y < m_dwyCount; ++y)
		{
			TSlot * pSlot;
			if (!GetGridSlotPointer(x, y, &pSlot))
				continue;

			if (ixLocal >= pSlot->ixPosition)
			{
				if (iyLocal >= pSlot->iyPosition)
				{
					if (ixLocal <= pSlot->ixPosition + pSlot->ixCellSize)
					{
						if (iyLocal <= pSlot->iyPosition + pSlot->iyCellSize)
						{
							*pix = x;
							*piy = y;
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

void CGridSlotWindow::ArrangeGridSlot(uint32_t dwStartIndex, uint32_t dwxCount, uint32_t dwyCount, int32_t ixSlotSize, int32_t iySlotSize, int32_t ixTemporarySize, int32_t iyTemporarySize)
{
	Destroy();

	m_dwxCount = dwxCount;
	m_dwyCount = dwyCount;

	m_SlotVector.clear();
	m_SlotVector.resize(dwxCount*dwyCount);

	for (uint32_t x = 0; x < dwxCount; ++x)
	{
		for (uint32_t y = 0; y < dwyCount; ++y)
		{
			uint32_t dwIndex = dwStartIndex + x + y * dwxCount;
			int32_t ixPosition = x * (ixSlotSize + ixTemporarySize);
			int32_t iyPosition = y * (iySlotSize + iyTemporarySize);

			AppendSlot(dwIndex, ixPosition, iyPosition, ixSlotSize, iySlotSize);

			m_SlotVector[x + y * dwxCount] = &(*m_SlotList.rbegin());
		}
	}

	int32_t iWidth = dwxCount * (ixSlotSize + ixTemporarySize);
	int32_t iHeight = dwyCount * (iySlotSize + iyTemporarySize);
	SetSize(iWidth, iHeight);
}

void CGridSlotWindow::OnRefreshSlot()
{
	uint32_t x, y;

	for (x = 0; x < m_dwxCount; ++x)
	{
		for (y = 0; y < m_dwyCount; ++y)
		{
			TSlot * pSlot;
			if (!GetGridSlotPointer(x, y, &pSlot))
				continue;

			pSlot->dwCenterSlotNumber = pSlot->dwSlotNumber;
			pSlot->dwRealCenterSlotNumber = pSlot->dwRealSlotNumber;
		}
	}

	for (x = 0; x < m_dwxCount; ++x)
	{
		for (y = 0; y < m_dwyCount; ++y)
		{
			TSlot * pSlot;
			if (!GetGridSlotPointer(x, y, &pSlot))
				continue;

			if (pSlot->isItem)
			{
				for (uint32_t xSub = 0; xSub < pSlot->byxPlacedItemSize; ++xSub)
				{
					for (uint32_t ySub = 0; ySub < pSlot->byyPlacedItemSize; ++ySub)
					{
						TSlot * pSubSlot;
						if (!GetGridSlotPointer(x + xSub, y + ySub, &pSubSlot))
							continue;

						pSubSlot->dwCenterSlotNumber = pSlot->dwSlotNumber;
						pSubSlot->dwRealCenterSlotNumber = pSlot->dwRealSlotNumber;
						pSubSlot->dwItemIndex = pSlot->dwItemIndex;
					}
				}
			}
			else
				pSlot->dwItemIndex = 0;
		}
	}
}

bool CGridSlotWindow::CheckMoving(uint32_t dwSlotNumber, uint32_t dwItemIndex, const std::list<TSlot*> & c_rSlotList)
{
	if (m_dwSlotStyle != SLOT_STYLE_PICK_UP)
		return true;

	uint16_t wCellMaxPerPage = m_SlotVector.size();
	while (dwSlotNumber >= wCellMaxPerPage)
		dwSlotNumber -= wCellMaxPerPage;

	for (std::list<TSlot*>::const_iterator itor = c_rSlotList.begin(); itor != c_rSlotList.end(); ++itor)
	{
		TSlot* pSlot = *itor;
		if (dwSlotNumber == pSlot->dwSlotNumber && itor == c_rSlotList.begin())
			return true;

		if (dwSlotNumber != pSlot->dwCenterSlotNumber)
		{
			if (c_rSlotList.size() == 2)
			{
				std::list<TSlot*>::const_iterator it = c_rSlotList.begin();
				std::advance(it, 1);
				if (0 != pSlot->dwItemIndex && 0 != (*it)->dwItemIndex)
					return false;
			}
			if (c_rSlotList.size() == 3)
			{
				std::list<TSlot*>::const_iterator it = c_rSlotList.begin();
				std::advance(it, 1);
				if (0 != pSlot->dwItemIndex && 0 != (*it)->dwItemIndex)
					return false;
				else
				{
					std::advance(it, 1);
					if (0 != pSlot->dwItemIndex && 0 != (*it)->dwItemIndex)
						return false;
				}
			}

			if (0 != pSlot->dwItemIndex || pSlot->dwCenterSlotNumber != pSlot->dwSlotNumber)
				if (dwItemIndex != pSlot->dwItemIndex)
					return false;
		}
	}

	return true;
}

bool CGridSlotWindow::CheckSwapping(uint32_t dwRealSlotNumber, uint32_t dwItemIndex, const std::list<TSlot*> & c_rSlotList)
{
	if (m_dwSlotStyle != SLOT_STYLE_PICK_UP)
		return true;

	uint8_t byWidth, byHeight;
	UI::CWindowManager::Instance().GetAttachingIconSize (&byWidth, &byHeight);

	int32_t iyBound = byHeight;
	int32_t iyBasePosition = 0;

	for (std::list<TSlot*>::const_iterator itor = c_rSlotList.begin(); itor != c_rSlotList.end(); ++itor)
	{
		TSlot* pSlot = *itor;

		if (dwRealSlotNumber == pSlot->dwRealCenterSlotNumber) // I can't swap with myself
			return false;

		if (itor == c_rSlotList.begin()) { //First one, mark
			iyBasePosition = pSlot->iyPosition;
		}

		if (pSlot->dwSlotNumber == pSlot->dwCenterSlotNumber)
			iyBound -= pSlot->byyPlacedItemSize;

		if (!pSlot->dwItemIndex) {
			TSlot * centerItem;
			if (!GetSlotPointerByNumber(pSlot->dwCenterSlotNumber, &centerItem)) //Some sort of error
				continue;

			if (!centerItem || !centerItem->dwCenterSlotNumber) {
				continue; // I can always swap with empty slots, but this may not be the only overlayed slot, so lets continue
			}

			if (centerItem->iyPosition < iyBasePosition)
				return false; //Out of bounds, upper side
		}
		
		if (pSlot->iyPosition < iyBasePosition) //Out of bounds, upper side
			return false;

		if (iyBound < 0) //An item will go out of bounds on the lower side
			return false;
	}

	if (iyBound > 0) //Space was not perfectly filled
		return false;

	return true;
}

void CGridSlotWindow::Destroy()
{
	CSlotWindow::Destroy();

	m_SlotVector.clear();

	__Initialize();
}

void CGridSlotWindow::__Initialize()
{
	m_dwxCount = 0;
	m_dwyCount = 0;
}

uint32_t CGridSlotWindow::Type()
{
	static int32_t s_Type = GetCRC32("CGridSlotWindow", strlen("CGridSlotWindow"));
	return s_Type;
}

bool CGridSlotWindow::OnIsType(uint32_t dwType)
{
	if (Type() == dwType)
		return true;

	return CSlotWindow::OnIsType(dwType);
}

CGridSlotWindow::CGridSlotWindow(PyObject * ppyObject) : CSlotWindow(ppyObject), m_dwxCount(0), m_dwyCount(0)
{
}

CGridSlotWindow::~CGridSlotWindow()
{
}
