#include "StdAfx.h"
#include "../eterBase/CRC32.h"
#include "../eterBase/Filename.h"
#include "PythonWindow.h"
#include "PythonSlotWindow.h"

//#define __RENDER_SLOT_AREA__

using namespace UI;

class CSlotWindow::CSlotButton : public CButton
{
	public:
		enum ESlotButtonType
		{
			SLOT_BUTTON_TYPE_PLUS,
			SLOT_BUTTON_TYPE_COVER
		};

	public:
		CSlotButton(ESlotButtonType dwType, uint32_t dwSlotNumber, CSlotWindow * pParent) : CButton(nullptr)
		{
			m_dwSlotButtonType = dwType;
			m_dwSlotNumber = dwSlotNumber;
			m_pParent = pParent;
		}
		virtual ~CSlotButton()
		{
		}

		bool OnMouseLeftButtonDown()
		{
			if (!IsEnable())
				return true;
		if (CWindowManager::Instance().IsAttaching())
				return true;

			m_isPressed = true;
			Down();

			return true;
		}
		bool OnMouseLeftButtonUp()
		{
			if (!IsEnable())
				return true;
			if (!IsPressed())
				return true;

			if (IsIn())
				SetCurrentVisual(&m_overVisual);
			else
				SetCurrentVisual(&m_upVisual);

			m_pParent->OnPressedSlotButton(m_dwSlotButtonType, m_dwSlotNumber);

			return true;
		}

		void OnMouseOverIn()
		{
			if (IsEnable())
				SetCurrentVisual(&m_overVisual);
			m_pParent->OnOverInItem(m_dwSlotNumber);
		}
		void OnMouseOverOut()
		{
			if (IsEnable())
			{
				SetUp();
				SetCurrentVisual(&m_upVisual);
			}
			m_pParent->OnOverOutItem();
		}

	protected:
		ESlotButtonType m_dwSlotButtonType;
		uint32_t m_dwSlotNumber;
		CSlotWindow * m_pParent;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

class CSlotWindow::CCoverButton : public CSlotButton
{
	public:
		CCoverButton(ESlotButtonType dwType, uint32_t dwSlotNumber, CSlotWindow * pParent) : CSlotButton(dwType, dwSlotNumber, pParent)
		{
			m_bLeftButtonEnable = true;
			m_bRightButtonEnable = true;
		}
		virtual ~CCoverButton()
		{
		}

		void SetLeftButtonEnable(bool bEnable)
		{
			m_bLeftButtonEnable = bEnable;
		}
		void SetRightButtonEnable(bool bEnable)
		{
			m_bRightButtonEnable = bEnable;
		}

		void OnRender()
		{
		}

		void RenderButton()
		{
			CButton::OnRender();
		}

		bool OnMouseLeftButtonDown()
		{
			if (!IsEnable())
				return true;
			if (m_bLeftButtonEnable)
			CButton::OnMouseLeftButtonDown();
		else
			m_pParent->OnMouseLeftButtonDown();
			return true;
		}
		bool OnMouseLeftButtonUp()
		{
			if (!IsEnable())
				return true;
			if (m_bLeftButtonEnable)
			{
				CButton::OnMouseLeftButtonUp();
				m_pParent->OnMouseLeftButtonDown();
			}
			else
			{
				m_pParent->OnMouseLeftButtonUp();
			}
			return true;
		}

		bool OnMouseRightButtonDown()
		{
			if (!IsEnable())
				return true;
			if (m_bRightButtonEnable)
				CButton::OnMouseLeftButtonDown();
			return true;
		}
		bool OnMouseRightButtonUp()
		{
			if (!IsEnable())
				return true;
			m_pParent->OnMouseRightButtonDown();
			if (m_bRightButtonEnable)
				CButton::OnMouseLeftButtonUp();
			return true;
		}

	protected:
		bool m_bLeftButtonEnable;
		bool m_bRightButtonEnable;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

class CSlotWindow::CCoolTimeFinishEffect : public CAniImageBox
{
	public:
		CCoolTimeFinishEffect(CSlotWindow * pParent, uint32_t dwSlotIndex) : CAniImageBox(nullptr)
		{
			m_pParent = pParent;
			m_dwSlotIndex = dwSlotIndex;
		}
		virtual ~CCoolTimeFinishEffect()
		{
		}

		void OnEndFrame()
		{
			((CSlotWindow *)m_pParent)->ReserveDestroyCoolTimeFinishEffect(m_dwSlotIndex);
		}

	protected:
		uint32_t m_dwSlotIndex;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// Set & Append

void CSlotWindow::SetWindowType(int32_t iType)
{
	m_iWindowType = iType;
}

void CSlotWindow::SetSlotType(uint32_t dwType)
{
	m_dwSlotType = dwType;
}

void CSlotWindow::SetSlotStyle(uint32_t dwStyle)
{
	m_dwSlotStyle = dwStyle;
}

void CSlotWindow::AppendSlot(uint32_t dwIndex, int32_t ixPosition, int32_t iyPosition, int32_t ixCellSize, int32_t iyCellSize)
{
	TSlot Slot;
	Slot.pInstance = nullptr;
	Slot.pBackgroundInstance = nullptr;
	Slot.pNumberLine = nullptr;
	Slot.pNumberLine2 = NULL;
	Slot.pCoverButton = nullptr;
	Slot.pSlotButton = nullptr;
	Slot.pSignImage = nullptr;
	Slot.pFinishCoolTimeEffect = nullptr;
#ifdef ENABLE_ACCE_SYSTEM
	memset(Slot.pActiveSlotEffect, 0, sizeof(Slot.pActiveSlotEffect));
#endif
	ClearSlot(&Slot);
	Slot.dwSlotNumber = dwIndex;
	Slot.dwCenterSlotNumber = dwIndex;
	Slot.dwRealSlotNumber = dwIndex;
	Slot.dwRealCenterSlotNumber = dwIndex;
	Slot.ixPosition = ixPosition;
	Slot.iyPosition = iyPosition;
	Slot.ixCellSize = ixCellSize;
	Slot.iyCellSize = iyCellSize;
	m_SlotList.emplace_back(Slot);
}

void CSlotWindow::SetCoverButton(uint32_t dwIndex, const char * c_szUpImageName, const char * c_szOverImageName, const char * c_szDownImageName, const char * c_szDisableImageName, bool bLeftButtonEnable, bool bRightButtonEnable)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	CCoverButton *& rpCoverButton = pSlot->pCoverButton;

	if (!rpCoverButton)
	{
		rpCoverButton = new CCoverButton(CSlotButton::SLOT_BUTTON_TYPE_COVER, pSlot->dwSlotNumber, this);
		CWindowManager::Instance().SetParent(rpCoverButton, this);
	}

	rpCoverButton->SetLeftButtonEnable(bLeftButtonEnable);
	rpCoverButton->SetRightButtonEnable(bRightButtonEnable);
	rpCoverButton->SetUpVisual(c_szUpImageName);
	rpCoverButton->SetOverVisual(c_szOverImageName);
	rpCoverButton->SetDownVisual(c_szDownImageName);
	rpCoverButton->SetDisableVisual(c_szDisableImageName);
	rpCoverButton->Enable();
	rpCoverButton->Show();

	// NOTE : Cover 버튼이 Plus 버튼을 가려버려서 임시 코드를..
	if (pSlot->pSlotButton)
		SetTop(pSlot->pSlotButton);
}

void CSlotWindow::DeleteCoverButton(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	CCoverButton *& rpCoverButton = pSlot->pCoverButton;
	if (!rpCoverButton)
		return;

	rpCoverButton->Hide();
	rpCoverButton->Disable();
	rpCoverButton->DestroyHandle();
}

bool CSlotWindow::HasCoverButton(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return false;

	CCoverButton *& rpCoverButton = pSlot->pCoverButton;
	return rpCoverButton ? true : false;
}

void CSlotWindow::EnableCoverButton(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (!pSlot->pCoverButton)
		return;

	pSlot->pCoverButton->Enable();
}

void CSlotWindow::DisableCoverButton(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (!pSlot->pCoverButton)
		return;

	pSlot->pCoverButton->Disable();
}

void CSlotWindow::SetAlwaysRenderCoverButton(uint32_t dwIndex, bool bAlwaysRender)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (bAlwaysRender)
		SET_BIT(pSlot->dwState, SLOT_STATE_ALWAYS_RENDER_COVER);
	else
		REMOVE_BIT(pSlot->dwState, SLOT_STATE_ALWAYS_RENDER_COVER);
}

void CSlotWindow::ShowSlotBaseImage(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->bRenderBaseSlotImage = true;
}

void CSlotWindow::HideSlotBaseImage(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->bRenderBaseSlotImage = false;
}

bool CSlotWindow::IsDisableCoverButton(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return false;

	if (!pSlot->pCoverButton)
		return false;

	return pSlot->pCoverButton->IsDisable();
}

void CSlotWindow::SetSlotBaseImage(const char * c_szFileName, float fr, float fg, float fb, float fa)
{
	__CreateBaseImage(c_szFileName, fr, fg, fb, fa);
}

void CSlotWindow::SetSlotBaseImageScale(const char * c_szFileName, float fr, float fg, float fb, float fa, float sx, float sy)
{
	__CreateBaseImageScale(c_szFileName, fr, fg, fb, fa, sx, sy);
}

void CSlotWindow::AppendSlotButton(const char * c_szUpImageName, const char * c_szOverImageName, const char * c_szDownImageName)
{
	for (auto & rSlot : m_SlotList)
	{
		CSlotButton *& rpSlotButton = rSlot.pSlotButton;

		if (!rpSlotButton)
		{
			rpSlotButton = new CSlotButton(CSlotButton::SLOT_BUTTON_TYPE_PLUS, rSlot.dwSlotNumber, this);
			rpSlotButton->AddFlag(FLAG_FLOAT);
			CWindowManager::Instance().SetParent(rpSlotButton, this);
		}

		rpSlotButton->SetUpVisual(c_szUpImageName);
		rpSlotButton->SetOverVisual(c_szOverImageName);
		rpSlotButton->SetDownVisual(c_szDownImageName);
		rpSlotButton->SetPosition(rSlot.ixPosition + 1, rSlot.iyPosition + 19);
		rpSlotButton->Hide();
	}
}

void CSlotWindow::AppendRequirementSignImage(const char * c_szImageName)
{
	for (auto & rSlot : m_SlotList)
	{
		CImageBox *& rpSignImage = rSlot.pSignImage;

		if (!rpSignImage)
		{
			rpSignImage = new CImageBox(nullptr);
			CWindowManager::Instance().SetParent(rpSignImage, this);
		}

		rpSignImage->LoadImage(c_szImageName);
		rpSignImage->Hide();
	}
}

bool CSlotWindow::HasSlot(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return false;

	return true;
}

void CSlotWindow::SetSlotRefineLevel(DWORD dwIndex, char* pszRefineLevel)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	delete pSlot->pNumberLine2;
	pSlot->pNumberLine2 = NULL;


	{
		char szCount[20];
		_snprintf(szCount, sizeof(szCount), "+%s", pszRefineLevel);

		if (!pSlot->pNumberLine2)
		{
			CNumberLine * pNumberLine2 = new UI::CNumberLine(this);
			pNumberLine2->SetHorizontalAlign(CNumberLine::HORIZONTAL_ALIGN_RIGHT);
			pNumberLine2->Show();
			pSlot->pNumberLine2 = pNumberLine2;
		}

		pSlot->pNumberLine2->SetNumber(szCount);
	}
}

void CSlotWindow::SetSlot(uint32_t dwIndex, uint32_t dwVirtualNumber, uint8_t byWidth, uint8_t byHeight, CGraphicImage * pImage, D3DXCOLOR& diffuseColor)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (pSlot->isItem)
	{
		if (pSlot->dwItemIndex == dwVirtualNumber)
		{
			pSlot->dwState = 0;
			pSlot->isItem = true;
			if (pImage && pSlot->pInstance)
				pSlot->pInstance->SetImagePointer(pImage);
			return;
		}
	}

	ClearSlot(pSlot);
	pSlot->dwState = 0;
	pSlot->isItem = true;
	pSlot->dwItemIndex = dwVirtualNumber;

	if (pImage)
	{
		assert(nullptr == pSlot->pInstance);
		pSlot->pInstance = CGraphicImageInstance::New();
		pSlot->pInstance->SetDiffuseColor(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
		pSlot->pInstance->SetImagePointer(pImage);
	}

	pSlot->byxPlacedItemSize = byWidth;
	pSlot->byyPlacedItemSize = byHeight;

	if (pSlot->pCoverButton)
		pSlot->pCoverButton->Show();
}

void CSlotWindow::SetSlotScale(uint32_t dwIndex, uint32_t dwVirtualNumber, uint8_t byWidth, uint8_t byHeight, CGraphicImage * pImage, float sx, float sy, D3DXCOLOR& diffuseColor)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (pSlot->isItem)
		if (pSlot->dwItemIndex == dwVirtualNumber)
		{
			pSlot->dwState = 0;
			pSlot->isItem = true;
			if (pImage && pSlot->pInstance)
			{
				pSlot->pInstance->SetImagePointer(pImage);
			}
			return;
		}

	ClearSlot(pSlot);
	pSlot->dwState = 0;
	pSlot->isItem = true;
	pSlot->dwItemIndex = dwVirtualNumber;

	if (pImage)
	{
		assert(nullptr == pSlot->pInstance);
		pSlot->pInstance = CGraphicImageInstance::New();
		pSlot->pInstance->SetDiffuseColor(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
		pSlot->pInstance->SetImagePointer(pImage);
		pSlot->pInstance->SetScale(sx, sy);
	}

	pSlot->byxPlacedItemSize = byWidth;
	pSlot->byyPlacedItemSize = byHeight;

	if (pSlot->pCoverButton)
	{
		pSlot->pCoverButton->Show();
	}
}

void CSlotWindow::SetSlotCount(uint32_t dwIndex, uint32_t dwCount)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (dwCount <= 0)
	{
		if (pSlot->pNumberLine2)
		{
			delete pSlot->pNumberLine2;
			pSlot->pNumberLine2 = NULL;
		}

		if (pSlot->pNumberLine)
		{
			delete pSlot->pNumberLine;
			pSlot->pNumberLine = nullptr;
		}
	}
	else
	{
		char szCount[16+1];
		_snprintf_s(szCount, sizeof(szCount), "%u", dwCount);

		if (!pSlot->pNumberLine)
		{
			if (pSlot->pNumberLine2)
			{
				delete pSlot->pNumberLine2;
				pSlot->pNumberLine2 = NULL;
			}

			CNumberLine * pNumberLine = new UI::CNumberLine(this);
			pNumberLine->SetHorizontalAlign(CNumberLine::HORIZONTAL_ALIGN_RIGHT);
			pNumberLine->Show();
			pSlot->pNumberLine = pNumberLine;
		}

		pSlot->pNumberLine->SetNumber(szCount);
	}
}

void CSlotWindow::SetSlotCountNew(uint32_t dwIndex, uint32_t dwGrade, uint32_t dwCount)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (dwCount <= 0)
	{
		if (pSlot->pNumberLine)
		{
			delete pSlot->pNumberLine;
			pSlot->pNumberLine = nullptr;
		}
	}
	else
	{
		char szCount[16+1];

		switch (dwGrade)
		{
			case 0:
				_snprintf_s(szCount, sizeof(szCount), "%u", dwCount);
				break;
			case 1:
				_snprintf_s(szCount, sizeof(szCount), "m%u", dwCount);
				break;
			case 2:
				_snprintf_s(szCount, sizeof(szCount), "g%u", dwCount);
				break;
			case 3:
				_snprintf_s(szCount, sizeof(szCount), "p");
				break;
		}

		if (!pSlot->pNumberLine)
		{
			CNumberLine * pNumberLine = new UI::CNumberLine(this);
			pNumberLine->SetHorizontalAlign(CNumberLine::HORIZONTAL_ALIGN_RIGHT);
			pNumberLine->Show();
			pSlot->pNumberLine = pNumberLine;
		}

		pSlot->pNumberLine->SetNumber(szCount);
	}
}

void CSlotWindow::SetRealSlotNumber(uint32_t dwIndex, uint32_t dwSlotRealNumber)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->dwRealSlotNumber = dwSlotRealNumber;
}

void CSlotWindow::SetSlotCoolTime(uint32_t dwIndex, float fCoolTime, float fElapsedTime)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->fCoolTime = fCoolTime;
	pSlot->fStartCoolTime = CTimer::Instance().GetCurrentSecond() - fElapsedTime;
}

void CSlotWindow::ActivateSlot(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->bActive = true;

	if (!m_pSlotActiveEffect
#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
		|| !m_pSlotActiveEffectSlot2 || !m_pSlotActiveEffectSlot3
#endif
	)
		__CreateSlotEnableEffect();
}

void CSlotWindow::DeactivateSlot(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->bActive = false;
}

#ifdef ENABLE_SLOT_WINDOW_EX
float CSlotWindow::GetSlotCoolTime(uint32_t dwIndex, float * fElapsedTime)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return 0.0;

	*fElapsedTime = CTimer::Instance().GetCurrentSecond() - pSlot->fStartCoolTime;
	return pSlot->fCoolTime;
}

bool CSlotWindow::IsActivatedSlot(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return false;
	return pSlot->bActive;
}
#endif

void CSlotWindow::ClearSlot(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	ClearSlot(pSlot);
}

void CSlotWindow::ClearSlot(TSlot * pSlot)
{
	pSlot->bActive = false;
	pSlot->byxPlacedItemSize = 1;
	pSlot->byyPlacedItemSize = 1;

	pSlot->isItem = false;
	pSlot->dwState = 0;
	pSlot->fCoolTime = 0.0f;
	pSlot->fStartCoolTime = 0.0f;
	pSlot->dwCenterSlotNumber = 0xffffffff;
	pSlot->dwRealCenterSlotNumber = 0xffffffff;

	pSlot->dwItemIndex = 0;
	pSlot->bRenderBaseSlotImage = true;

	if (pSlot->pInstance)
	{
		CGraphicImageInstance::Delete(pSlot->pInstance);
		pSlot->pInstance = nullptr;
	}

	if (pSlot->pBackgroundInstance)
	{
		CGraphicImageInstance::Delete(pSlot->pBackgroundInstance);
		pSlot->pBackgroundInstance = nullptr;
	}

	if (pSlot->pCoverButton)
		pSlot->pCoverButton->Hide();
	if (pSlot->pSlotButton)
		pSlot->pSlotButton->Hide();
	if (pSlot->pSignImage)
 		pSlot->pSignImage->Hide();
	if (pSlot->pFinishCoolTimeEffect)
		pSlot->pFinishCoolTimeEffect->Hide();

#ifdef ENABLE_ACCE_SYSTEM
	for (auto& pEff : pSlot->pActiveSlotEffect)
		if(pEff)
			pEff->Hide();
#endif
}

void CSlotWindow::ClearAllSlot()
{
	Destroy();
}

void CSlotWindow::RefreshSlot()
{
	OnRefreshSlot();

	// NOTE : Refresh 될때 ToolTip 도 갱신 합니다 - [levites]
	if (IsRendering())
	{
		TSlot * pSlot;
		if (GetPickedSlotPointer(&pSlot))
		{
			OnOverOutItem();
			OnOverInItem(pSlot->dwSlotNumber);
		}
	}
}

void CSlotWindow::OnRefreshSlot()
{
}

uint32_t CSlotWindow::GetSlotCount()
{
	return m_SlotList.size();
}

void CSlotWindow::LockSlot(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->dwState |= SLOT_STATE_LOCK;
}
void CSlotWindow::UnlockSlot(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->dwState ^= SLOT_STATE_LOCK;
}
void CSlotWindow::SetCantUseSlot(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->dwState |= SLOT_STATE_CANT_USE;
}
void CSlotWindow::SetUseSlot(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->dwState ^= SLOT_STATE_CANT_USE;
}
void CSlotWindow::EnableSlot(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	REMOVE_BIT(pSlot->dwState, SLOT_STATE_DISABLE);
	//pSlot->dwState |= SLOT_STATE_DISABLE;
}
void CSlotWindow::DisableSlot(uint32_t dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;
	SET_BIT(pSlot->dwState, SLOT_STATE_DISABLE);
	//pSlot->dwState ^= SLOT_STATE_DISABLE;
}

// Select

void CSlotWindow::SelectSlot(uint32_t dwSelectingIndex)
{
	auto itor = m_dwSelectedSlotIndexList.begin();
	for (; itor != m_dwSelectedSlotIndexList.end();)
	{
		if (dwSelectingIndex == *itor)
		{
			m_dwSelectedSlotIndexList.erase(itor);
			return;
		}
		++itor;
	}

	TSlot * pSlot;
	if (GetSlotPointer(dwSelectingIndex, &pSlot))
	{
		if (!pSlot->isItem)
			return;

		m_dwSelectedSlotIndexList.emplace_back(dwSelectingIndex);
	}
}

bool CSlotWindow::isSelectedSlot(uint32_t dwIndex)
{
	auto itor = m_dwSelectedSlotIndexList.begin();
	for (; itor != m_dwSelectedSlotIndexList.end(); ++itor)
	{
		if (dwIndex == *itor)
			return true;
	}

	return false;
}

void CSlotWindow::ClearSelected()
{
	m_dwSelectedSlotIndexList.clear();
}

uint32_t CSlotWindow::GetSelectedSlotCount()
{
	return m_dwSelectedSlotIndexList.size();
}

uint32_t CSlotWindow::GetSelectedSlotNumber(uint32_t dwIndex)
{
	if (dwIndex >= m_dwSelectedSlotIndexList.size())
		return uint32_t(-1);

	uint32_t dwCount = 0;
	auto itor = m_dwSelectedSlotIndexList.begin();
	for (; itor != m_dwSelectedSlotIndexList.end(); ++itor)
	{
		if (dwIndex == dwCount)
			break;

		++dwCount;
	}

	return *itor;
}

void CSlotWindow::ShowSlotButton(uint32_t dwSlotNumber)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return;

	if (pSlot->pSlotButton)
		pSlot->pSlotButton->Show();
}

void CSlotWindow::HideAllSlotButton()
{
	for (auto & rSlot : m_SlotList)
	{
		if (rSlot.pSlotButton)
			rSlot.pSlotButton->Hide();
	}
}

void CSlotWindow::ShowRequirementSign(uint32_t dwSlotNumber)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return;

	if (!pSlot->pSignImage)
		return;

	pSlot->pSignImage->Show();
}

void CSlotWindow::HideRequirementSign(uint32_t dwSlotNumber)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return;

	if (!pSlot->pSignImage)
		return;

	pSlot->pSignImage->Hide();
}

// Event

bool CSlotWindow::OnMouseLeftButtonDown()
{
	TSlot * pSlot;
	if (!GetPickedSlotPointer(&pSlot))
	{
		CWindowManager::Instance().DeattachIcon();
		return true;
	}

	if (pSlot->isItem && !(pSlot->dwState & SLOT_STATE_LOCK))
		OnSelectItemSlot(pSlot->dwSlotNumber);
	else
		OnSelectEmptySlot(pSlot->dwSlotNumber);

	return true;
}

bool CSlotWindow::OnMouseLeftButtonUp()
{
	if (UI::CWindowManager::Instance().IsAttaching())
	if (UI::CWindowManager::Instance().IsDragging())
	if (IsIn())
	{
		UI::CWindow * pWin = UI::CWindowManager::Instance().GetPointWindow();

		if (pWin)
		if (pWin->IsType(UI::CSlotWindow::Type()))
		{
			UI::CSlotWindow * pSlotWin = (UI::CSlotWindow *)pWin;

			TSlot * pSlot;
			if (!pSlotWin->GetPickedSlotPointer(&pSlot))
			{
				UI::CWindowManager::Instance().DeattachIcon();
				return true;
			}

			if (pSlot->isItem)
				pSlotWin->OnSelectItemSlot(pSlot->dwSlotNumber);
			else
				pSlotWin->OnSelectEmptySlot(pSlot->dwSlotNumber);

			return true;
		}

		return false;
	}

	return false;
}

void CSlotWindow::SetSlotID(uint32_t dwIndex, uint32_t dwID)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (dwID <= 0)
	{
		return;
	}
	else
	{
		
		pSlot->dwItemID=dwID;
	}
}

bool CSlotWindow::OnMouseRightButtonDown()
{
	TSlot * pSlot;
	if (!GetPickedSlotPointer(&pSlot))
		return true;

	if (pSlot->isItem)
		OnUnselectItemSlot(pSlot->dwSlotNumber);
	else
		OnUnselectEmptySlot(pSlot->dwSlotNumber);

	return true;
}

bool CSlotWindow::OnMouseLeftButtonDoubleClick()
{
	TSlot * pSlot;
	
	if (GetPickedSlotPointer(&pSlot))
		OnUseSlot();

	return true;
}

void CSlotWindow::OnMouseOverOut()
{
	OnOverOutItem();
}

void CSlotWindow::OnMouseOver()
{
	// FIXME : 윈도우를 드래깅 하는 도중에 SetTop이 되어버리면 Capture가 풀어져 버린다. 그것의 방지 코드.
	//         좀 더 근본적인 해결책을 찾아야 할 듯 - [levites]
//	if (UI::CWindowManager::Instance().IsCapture())
//	if (!UI::CWindowManager::Instance().IsAttaching())
//		return;

	CWindow * pPointWindow = CWindowManager::Instance().GetPointWindow();
	if (this == pPointWindow)
	{
		TSlot * pSlot;
		if (GetPickedSlotPointer(&pSlot))
		{
			if (OnOverInItem(pSlot->dwSlotNumber))
				return;
		}
	}

	OnOverOutItem();
}

void CSlotWindow::OnSelectEmptySlot(int32_t iSlotNumber)
{
	PyCallClassMemberFunc(m_poHandler, "OnSelectEmptySlot", Py_BuildValue("(i)", iSlotNumber));
}
void CSlotWindow::OnSelectItemSlot(int32_t iSlotNumber)
{
//	OnOverOutItem();
	PyCallClassMemberFunc(m_poHandler, "OnSelectItemSlot", Py_BuildValue("(i)", iSlotNumber));

	if (CWindowManager::Instance().IsAttaching())
		OnOverOutItem();
}
void CSlotWindow::OnUnselectEmptySlot(int32_t iSlotNumber)
{
	PyCallClassMemberFunc(m_poHandler, "OnUnselectEmptySlot", Py_BuildValue("(i)", iSlotNumber));
}
void CSlotWindow::OnUnselectItemSlot(int32_t iSlotNumber)
{
	PyCallClassMemberFunc(m_poHandler, "OnUnselectItemSlot", Py_BuildValue("(i)", iSlotNumber));
}
void CSlotWindow::OnUseSlot()
{
	TSlot * pSlot;
	if (GetPickedSlotPointer(&pSlot))
	{
		if (pSlot->isItem)
			PyCallClassMemberFunc(m_poHandler, "OnUseSlot", Py_BuildValue("(i)", pSlot->dwSlotNumber));
	}
}

bool CSlotWindow::OnOverInItem(uint32_t dwSlotNumber)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return false;

	if (!pSlot->isItem)
		return false;

	if (pSlot->dwSlotNumber == m_dwToolTipSlotNumber)
		return true;

	m_dwToolTipSlotNumber = dwSlotNumber;
	PyCallClassMemberFunc(m_poHandler, "OnOverInItem", Py_BuildValue("(i)", dwSlotNumber));

	return true;
}

void CSlotWindow::OnOverOutItem()
{
	if (SLOT_NUMBER_NONE == m_dwToolTipSlotNumber)
		return;

	m_dwToolTipSlotNumber = SLOT_NUMBER_NONE;
	PyCallClassMemberFunc(m_poHandler, "OnOverOutItem", Py_BuildValue("()"));
}

void CSlotWindow::OnPressedSlotButton(uint32_t dwType, uint32_t dwSlotNumber, bool isLeft)
{
	if (CSlotButton::SLOT_BUTTON_TYPE_PLUS == dwType)
		PyCallClassMemberFunc(m_poHandler, "OnPressedSlotButton", Py_BuildValue("(i)", dwSlotNumber));
	else if (CSlotButton::SLOT_BUTTON_TYPE_COVER == dwType)
	{
		if (isLeft)
			OnMouseLeftButtonDown();
	}
}

void CSlotWindow::OnUpdate()
{
	for (auto & dwSlotIndex : m_ReserveDestroyEffectDeque)
	{
		TSlot * pSlot;
		if (!GetSlotPointer(dwSlotIndex, &pSlot))
			continue;

		__DestroyFinishCoolTimeEffect(pSlot);

#ifdef ENABLE_ACCE_SYSTEM
		for (auto& pEff : pSlot->pActiveSlotEffect)
		{
			if (pEff)
			{
				delete pEff;
				pEff = nullptr;
			}
		}
#endif
	}
	m_ReserveDestroyEffectDeque.clear();

	if (m_pSlotActiveEffect)
		m_pSlotActiveEffect->Update();
#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
	if (m_pSlotActiveEffectSlot2)
		m_pSlotActiveEffectSlot2->Update();
	if (m_pSlotActiveEffectSlot3)
		m_pSlotActiveEffectSlot3->Update();
#endif
}

void CSlotWindow::OnRender()
{
	RenderSlotBaseImage();

	switch (m_dwSlotStyle)
	{
		case SLOT_STYLE_PICK_UP:
			OnRenderPickingSlot();
			break;
		case SLOT_STYLE_SELECT:
			OnRenderSelectedSlot();
			break;
	}

	for (const auto & window : m_pChildList)
		window->OnRender();

	//
	// 모든 슬롯 상자 그리기
	//////////////////////////////////////////////////////////////////////////
#ifdef __RENDER_SLOT_AREA__
	CPythonGraphic::Instance().SetDiffuseColor(0.5f, 0.5f, 0.5f);
	for (itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot & rSlot = *itor;
		CPythonGraphic::Instance().RenderBox2d(m_rect.left + rSlot.ixPosition,
											   m_rect.top + rSlot.iyPosition,
											   m_rect.left + rSlot.ixPosition + rSlot.ixCellSize,
											   m_rect.top + rSlot.iyPosition + rSlot.iyCellSize);
	}
	CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.0f, 0.0f, 1.0f);
	CPythonGraphic::Instance().RenderBox2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
#endif
	//////////////////////////////////////////////////////////////////////////

	for (auto & rSlot : m_SlotList)
	{
		if (rSlot.pSlotButton)
			rSlot.pSlotButton->SetPosition(rSlot.ixPosition + 1, rSlot.iyPosition + 19);
		if (rSlot.pSignImage)
			rSlot.pSignImage->SetPosition(rSlot.ixPosition - 7, rSlot.iyPosition + 10);

		if (rSlot.pInstance)
		{
			rSlot.pInstance->SetPosition(m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition);
			rSlot.pInstance->Render();
		}

		if (!rSlot.isItem)
		{
			if (IS_SET(rSlot.dwState, SLOT_STATE_ALWAYS_RENDER_COVER))
			{
				rSlot.pCoverButton->Show();
				rSlot.pCoverButton->SetPosition(rSlot.ixPosition, rSlot.iyPosition);
				rSlot.pCoverButton->RenderButton();
			}

			continue;
		}

		if (IS_SET(rSlot.dwState, SLOT_STATE_DISABLE))
		{
			CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.0f, 0.0f, 0.3f);
			CPythonGraphic::Instance().RenderBar2d(m_rect.left + rSlot.ixPosition,
				m_rect.top + rSlot.iyPosition,
				m_rect.left + rSlot.ixPosition + rSlot.ixCellSize,
				m_rect.top + rSlot.iyPosition + rSlot.iyCellSize);
		}

		if (rSlot.fCoolTime != 0.0f)
		{
			float fcurTime = CTimer::Instance().GetCurrentSecond();
			float fPercentage = (fcurTime - rSlot.fStartCoolTime) / rSlot.fCoolTime;
			CPythonGraphic::Instance().RenderCoolTimeBox(m_rect.left + rSlot.ixPosition + 16.0f, m_rect.top + rSlot.iyPosition + 16.0f, 16.0f, fPercentage);

			if (fcurTime - rSlot.fStartCoolTime >= rSlot.fCoolTime)
			{
				// 쿨타임이 끝난지 1초 이내라면..
				if ((fcurTime - rSlot.fStartCoolTime) - rSlot.fCoolTime < 1.0f)
					__CreateFinishCoolTimeEffect(&rSlot);

				rSlot.fCoolTime = 0.0f;
				rSlot.fStartCoolTime = 0.0f;
			}
		}

		if (rSlot.pCoverButton)
		{
			rSlot.pCoverButton->SetPosition(rSlot.ixPosition, rSlot.iyPosition);
			rSlot.pCoverButton->RenderButton();
		}

		if (rSlot.pNumberLine)
		{
			int32_t ix = rSlot.byxPlacedItemSize*ITEM_WIDTH + rSlot.ixPosition - 4;
			int32_t iy = rSlot.iyPosition + rSlot.byyPlacedItemSize*ITEM_HEIGHT - 12 + 2;
			rSlot.pNumberLine->SetPosition(ix, iy);
			rSlot.pNumberLine->Update();
			rSlot.pNumberLine->Render();
		}

		if (rSlot.pNumberLine2)
		{
			int ix = rSlot.byxPlacedItemSize*ITEM_WIDTH + rSlot.ixPosition - 4;
			int iy = rSlot.iyPosition + rSlot.byyPlacedItemSize*ITEM_HEIGHT - 12 + 2;
			rSlot.pNumberLine2->SetPosition(ix, iy);
			rSlot.pNumberLine2->Update();
			rSlot.pNumberLine2->Render();
		}

		if (rSlot.pFinishCoolTimeEffect)
		{
			rSlot.pFinishCoolTimeEffect->SetPosition(rSlot.ixPosition, rSlot.iyPosition);
			rSlot.pFinishCoolTimeEffect->Update();
			rSlot.pFinishCoolTimeEffect->Render();
		}

#ifdef ENABLE_ACCE_SYSTEM
		{
			int32_t iX = m_rect.left + rSlot.ixPosition, iY = m_rect.top + rSlot.iyPosition, iItemYSize = rSlot.byyPlacedItemSize;
			for (int32_t i = 0; i < 3; ++i)
			{
				auto& pEff = rSlot.pActiveSlotEffect[i];
				if ((pEff) && (iItemYSize == i + 1))
				{
					pEff->Show();
					pEff->SetPosition(iX, iY);
					pEff->Update();
					pEff->Render();
				}
			}
		}
#endif

		if (rSlot.bActive)
		{
			if (m_pSlotActiveEffect && rSlot.byyPlacedItemSize==1)
			{
				int32_t ix = m_rect.left + rSlot.ixPosition;
				int32_t iy = m_rect.top + rSlot.iyPosition;
				m_pSlotActiveEffect->SetPosition(ix, iy);
				m_pSlotActiveEffect->Render();
			}
#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
			else if (m_pSlotActiveEffectSlot2 && rSlot.byyPlacedItemSize==2)
			{
				int32_t ix = m_rect.left + rSlot.ixPosition;
				int32_t iy = m_rect.top + rSlot.iyPosition;
				m_pSlotActiveEffectSlot2->SetPosition(ix, iy);
				m_pSlotActiveEffectSlot2->Render();
			}
			else if (m_pSlotActiveEffectSlot3 && rSlot.byyPlacedItemSize==3)
			{
				int32_t ix = m_rect.left + rSlot.ixPosition;
				int32_t iy = m_rect.top + rSlot.iyPosition;
				m_pSlotActiveEffectSlot3->SetPosition(ix, iy);
				m_pSlotActiveEffectSlot3->Render();
			}
#endif
		}
	}

	RenderLockedSlot();
}

void CSlotWindow::RenderSlotBaseImage()
{
	if (!m_pBaseImageInstance)
		return;

	for (auto & rSlot : m_SlotList)
	{
		if (!rSlot.bRenderBaseSlotImage)
			continue;

		CGraphicImageInstance* bgImageInstance = rSlot.pBackgroundInstance ? rSlot.pBackgroundInstance : m_pBaseImageInstance;
		
		bgImageInstance->SetPosition(m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition);
		bgImageInstance->Render();
	}
}

void CSlotWindow::OnRenderPickingSlot()
{
	if (!CWindowManager::Instance().IsAttaching())
		return;

	TSlot * pSlot;
	if (!GetSelectedSlotPointer(&pSlot))
		return;

	CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f, 0.5f);
	CPythonGraphic::Instance().RenderBar2d(m_rect.left + pSlot->ixPosition,
											m_rect.top + pSlot->iyPosition,
											m_rect.left + pSlot->ixPosition + pSlot->ixCellSize,
											m_rect.top + pSlot->iyPosition + pSlot->iyCellSize);
}

void CSlotWindow::OnRenderSelectedSlot()
{
	auto itor = m_dwSelectedSlotIndexList.begin();
	for (; itor != m_dwSelectedSlotIndexList.end(); ++itor)
	{
		TSlot * pSlot;
		if (!GetSlotPointer(*itor, &pSlot))
			continue;

		CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f, 0.5f);
		CPythonGraphic::Instance().RenderBar2d(m_rect.left + pSlot->ixPosition,
												m_rect.top + pSlot->iyPosition,
												m_rect.left + pSlot->ixPosition + pSlot->ixCellSize,
												m_rect.top + pSlot->iyPosition + pSlot->iyCellSize);
	}
}

void CSlotWindow::RenderLockedSlot()
{
	CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.5f);
	for (auto & rSlot : m_SlotList)
	{
		if (!rSlot.isItem)
			continue;

		if (rSlot.dwState & SLOT_STATE_LOCK)
		{
			CPythonGraphic::Instance().RenderBar2d(m_rect.left + rSlot.ixPosition,
												   m_rect.top  + rSlot.iyPosition,
												   m_rect.left + rSlot.ixPosition + rSlot.ixCellSize,
												   m_rect.top  + rSlot.iyPosition + rSlot.iyCellSize);
		}
	}
}

// Check Slot

bool CSlotWindow::GetSlotPointer(uint32_t dwIndex, TSlot ** ppSlot)
{
	for (auto & rSlot : m_SlotList)
	{
		if (dwIndex == rSlot.dwSlotNumber)
		{
			*ppSlot = &rSlot;
			return true;
		}
	}

	return false;
}

bool CSlotWindow::GetSelectedSlotPointer(TSlot ** ppSlot)
{
	int32_t lx, ly;
	GetMouseLocalPosition(lx, ly);

	for (auto & rSlot : m_SlotList)
	{
		if (lx >= rSlot.ixPosition)
			if (ly >= rSlot.iyPosition)
			{
				if (lx <= rSlot.ixPosition + rSlot.ixCellSize)
				{
					if (ly <= rSlot.iyPosition + rSlot.iyCellSize)
					{
						*ppSlot = &rSlot;
						return true;
					}
				}
			}
	}

	return false;
}

bool CSlotWindow::GetPickedSlotPointer(TSlot ** ppSlot)
{
	int32_t lx, ly;
	CWindowManager::Instance().GetMousePosition(lx, ly);

	int32_t ixLocal = lx - m_rect.left;
	int32_t iyLocal = ly - m_rect.top;

	// NOTE : 왼쪽 맨위 상단 한곳이 기준 이라는 점을 이용해 왼쪽 위에서부터 오른쪽 아래로
	//        차례로 검색해 감으로써 덮혀 있는 Slot은 자동 무시 된다는 특성을 이용한다. - [levites]
	for (auto & rSlot : m_SlotList)
	{
		int32_t ixCellSize = rSlot.ixCellSize;
		int32_t iyCellSize = rSlot.iyCellSize;

		// NOTE : Item이 Hide 되어있을 경우를 위한..
		if (rSlot.isItem)
		{
			ixCellSize = std::max(rSlot.ixCellSize, int32_t(rSlot.byxPlacedItemSize * ITEM_WIDTH));
			iyCellSize = std::max(rSlot.iyCellSize, int32_t(rSlot.byyPlacedItemSize * ITEM_HEIGHT));
		}

		if (ixLocal >= rSlot.ixPosition)
		{
			if (iyLocal >= rSlot.iyPosition)
			{
				if (ixLocal <= rSlot.ixPosition + ixCellSize)
					if (iyLocal <= rSlot.iyPosition + iyCellSize)
					{
						*ppSlot = &rSlot;
						return true;
					}
			}
		}
	}

	return false;
}

void CSlotWindow::SetUseMode(bool bFlag)
{
	m_isUseMode = bFlag;
}

void CSlotWindow::SetUsableItem(bool bFlag)
{
	m_isUsableItem = bFlag;
}

void CSlotWindow::ReserveDestroyCoolTimeFinishEffect(uint32_t dwSlotIndex)
{
	m_ReserveDestroyEffectDeque.emplace_back(dwSlotIndex);
}

uint32_t CSlotWindow::Type()
{
	static int32_t s_Type = GetCRC32("CSlotWindow", strlen("CSlotWindow"));
	return s_Type;
}

#ifdef ENABLE_ACCE_SYSTEM
void CSlotWindow::ActivateEffect(uint32_t dwSlotIndex, float r, float g, float b, float a)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwSlotIndex, &pSlot))
		return;

	for (int32_t i = 0; i < 3; ++i)
	{
		auto& pActiveEff = pSlot->pActiveSlotEffect[i];

		if (pActiveEff)
		{
			delete pActiveEff;
			pActiveEff = nullptr;
		}

		const int32_t ciImageCount = 12;

		pActiveEff = new CAniImageBox(nullptr);
		for (int32_t j = 0; j <= ciImageCount ; ++j)
		{
			char cBuf[72];
			snprintf(cBuf, sizeof(cBuf), "d:/ymir work/ui/public/slotactiveeffect/slot%d/%02d.sub", (i + 1), j);
			pActiveEff->AppendImage(cBuf, r, g, b, a);
		}

		pActiveEff->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	}
}

void CSlotWindow::DeactivateEffect(uint32_t dwSlotIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwSlotIndex, &pSlot))
		return;

	for (auto& pActiveEff : pSlot->pActiveSlotEffect)
	{
		if (pActiveEff)
		{
			delete pActiveEff;
			pActiveEff = nullptr;
		}
	}
}
#endif

bool CSlotWindow::OnIsType(uint32_t dwType)
{
	if (Type() == dwType)
		return true;

	return CWindow::OnIsType(dwType);
}

void CSlotWindow::SetSlotBackground(uint32_t dwIndex, const char* c_szFileName)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	//Destroy old one first
	if (pSlot->pBackgroundInstance) 
	{
		CGraphicImageInstance::Delete(pSlot->pBackgroundInstance);
		pSlot->pBackgroundInstance = nullptr;
	}

	CGraphicImage* pImage = CResourceManager::Instance().GetResourcePointer<CGraphicImage>(c_szFileName);
	pSlot->pBackgroundInstance = CGraphicImageInstance::New();
	pSlot->pBackgroundInstance->SetImagePointer(pImage);
}

void CSlotWindow::__CreateToggleSlotImage()
{
	__DestroyToggleSlotImage();

	m_pToggleSlotImage = new CImageBox(nullptr);
	m_pToggleSlotImage->LoadImage("d:/ymir work/ui/public/slot_toggle.sub");
	m_pToggleSlotImage->Show();
}

void CSlotWindow::__CreateSlotEnableEffect()
{
	__DestroySlotEnableEffect();

	m_pSlotActiveEffect = new CAniImageBox(nullptr);
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/00.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/01.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/02.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/03.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/04.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/05.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/06.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/07.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/08.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/09.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/10.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/11.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/12.sub");
	m_pSlotActiveEffect->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	m_pSlotActiveEffect->Show();

#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
	m_pSlotActiveEffectSlot2 = new CAniImageBox(nullptr);
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/00.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/01.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/02.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/03.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/04.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/05.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/06.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/07.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/08.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/09.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/10.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/11.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/12.sub");
	m_pSlotActiveEffectSlot2->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	m_pSlotActiveEffectSlot2->Show();

	m_pSlotActiveEffectSlot3 = new CAniImageBox(nullptr);
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/00.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/01.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/02.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/03.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/04.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/05.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/06.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/07.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/08.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/09.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/10.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/11.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/12.sub");
	m_pSlotActiveEffectSlot3->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	m_pSlotActiveEffectSlot3->Show();
#endif
}

void CSlotWindow::__CreateFinishCoolTimeEffect(TSlot * pSlot)
{
	__DestroyFinishCoolTimeEffect(pSlot);

	CAniImageBox * pFinishCoolTimeEffect = new CCoolTimeFinishEffect(this, pSlot->dwSlotNumber);
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/00.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/01.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/02.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/03.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/04.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/05.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/06.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/07.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/08.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/09.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/10.sub");
	pFinishCoolTimeEffect->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	pFinishCoolTimeEffect->ResetFrame();
	pFinishCoolTimeEffect->SetDelay(2);
	pFinishCoolTimeEffect->Show();

	pSlot->pFinishCoolTimeEffect = pFinishCoolTimeEffect;
}

void CSlotWindow::__CreateBaseImage(const char * c_szFileName, float fr, float fg, float fb, float fa)
{
	__DestroyBaseImage();

	CGraphicImage* pImage = CResourceManager::Instance().GetResourcePointer<CGraphicImage>(c_szFileName);
	m_pBaseImageInstance = CGraphicImageInstance::New();
	m_pBaseImageInstance->SetImagePointer(pImage);
	m_pBaseImageInstance->SetDiffuseColor(fr, fg, fb, fa);
}

void CSlotWindow::__CreateBaseImageScale(const char * c_szFileName, float fr, float fg, float fb, float fa, float sx, float sy)
{
	__DestroyBaseImage();

	CGraphicImage* pImage = CResourceManager::Instance().GetResourcePointer<CGraphicImage>(c_szFileName);
	m_pBaseImageInstance = CGraphicImageInstance::New();
	m_pBaseImageInstance->SetImagePointer(pImage);
	m_pBaseImageInstance->SetDiffuseColor(fr, fg, fb, fa);
	m_pBaseImageInstance->SetScale(sx, sy);
}

void CSlotWindow::__DestroyToggleSlotImage()
{
	if (m_pToggleSlotImage)
	{
		delete m_pToggleSlotImage;
		m_pToggleSlotImage = nullptr;
	}
}

void CSlotWindow::__DestroySlotEnableEffect()
{
	if (m_pSlotActiveEffect)
	{
		delete m_pSlotActiveEffect;
		m_pSlotActiveEffect = nullptr;
	}
#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
	if (m_pSlotActiveEffectSlot2)
	{
		delete m_pSlotActiveEffectSlot2;
		m_pSlotActiveEffectSlot2 = nullptr;
	}
	if (m_pSlotActiveEffectSlot3)
	{
		delete m_pSlotActiveEffectSlot3;
		m_pSlotActiveEffectSlot3 = nullptr;
	}
#endif
}

void CSlotWindow::__DestroyFinishCoolTimeEffect(TSlot * pSlot)
{
	if (pSlot->pFinishCoolTimeEffect)
	{
		delete pSlot->pFinishCoolTimeEffect;
		pSlot->pFinishCoolTimeEffect = nullptr;
	}
}

void CSlotWindow::__DestroyBaseImage()
{
	if (m_pBaseImageInstance)
	{
		CGraphicImageInstance::Delete(m_pBaseImageInstance);
		m_pBaseImageInstance = nullptr;
	}
}

void CSlotWindow::__Initialize()
{
	m_dwSlotType = 0;
	m_iWindowType = SLOT_WND_DEFAULT;
	m_dwSlotStyle = SLOT_STYLE_PICK_UP;
	m_dwToolTipSlotNumber = SLOT_NUMBER_NONE;

	m_isUseMode = false;
	m_isUsableItem = false;

	m_pToggleSlotImage = nullptr;
	m_pSlotActiveEffect = nullptr;
#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
	m_pSlotActiveEffectSlot2 = nullptr;
	m_pSlotActiveEffectSlot3 = nullptr;
#endif
	m_pBaseImageInstance = nullptr;
}

void CSlotWindow::Destroy()
{
	for (auto & rSlot : m_SlotList)
	{
		ClearSlot(&rSlot);

		if (rSlot.pNumberLine)
		{
			delete rSlot.pNumberLine;
			rSlot.pNumberLine = nullptr;
		}
		if (rSlot.pNumberLine2)
		{
			delete rSlot.pNumberLine2;
			rSlot.pNumberLine2 = nullptr;
		}
		if (rSlot.pCoverButton)
			CWindowManager::Instance().DestroyWindow(rSlot.pCoverButton);
		if (rSlot.pSlotButton)
			CWindowManager::Instance().DestroyWindow(rSlot.pSlotButton);
		if (rSlot.pSignImage)
			CWindowManager::Instance().DestroyWindow(rSlot.pSignImage);
		if (rSlot.pFinishCoolTimeEffect)
			CWindowManager::Instance().DestroyWindow(rSlot.pFinishCoolTimeEffect);

#ifdef ENABLE_ACCE_SYSTEM
		for (auto& pEff: rSlot.pActiveSlotEffect )
			if (pEff)
				CWindowManager::Instance().DestroyWindow(pEff);
#endif
	}

	m_SlotList.clear();

	__DestroyToggleSlotImage();
	__DestroySlotEnableEffect();
	__DestroyBaseImage();

	__Initialize();
}

CSlotWindow::CSlotWindow(PyObject * ppyObject) : CWindow(ppyObject)
{
	__Initialize();
}

CSlotWindow::~CSlotWindow()
{
	Destroy();
}
