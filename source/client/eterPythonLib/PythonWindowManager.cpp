#include "StdAfx.h"
#include "PythonWindow.h"
#include "PythonSlotWindow.h"
#include "PythonGridSlotWindow.h"
#include "PythonWindowManager.h"

//#define __WINDOW_LEAK_CHECK__

bool g_bShowOverInWindowName = false;

namespace UI
{	
	static PyObject* gs_poEmptyTuple = nullptr;

	PyObject * BuildEmptyTuple()
	{
		Py_INCREF(gs_poEmptyTuple);
		return gs_poEmptyTuple;
	}

	std::set<CWindow*> gs_kSet_pkWnd;
	
	CWindowManager::CWindowManager()
		: 
		m_lWidth(0), m_lHeight(0),	
		m_iVres(0), m_iHres(0),
		m_lMouseX(0), m_lMouseY(0),
		m_lDragX(0), m_lDragY(0),
		m_lPickedX(0), m_lPickedY(0),
		m_bOnceIgnoreMouseLeftButtonUpEventFlag(false), m_dwIgnoreEndTime(0),
		m_poMouseHandler(nullptr),
		m_bAttachingFlag(false),
		m_dwAttachingType(0),
		m_dwAttachingIndex(0),
		m_dwAttachingSlotNumber(0),
		m_dwAttachingRealSlotNumber(0), m_byAttachingIconWidth(0), m_byAttachingIconHeight(0),
		m_pActiveWindow(nullptr),
		m_pLockWindow(nullptr),
		m_pPointWindow(nullptr),
		m_pLeftCaptureWindow(nullptr),
		m_pRightCaptureWindow(nullptr),
		m_pMiddleCaptureWindow(nullptr)
	{		
		m_pRootWindow = new CWindow(nullptr);
		m_pRootWindow->SetName("root");
		m_pRootWindow->Show();

		const char * layerTbl[] = {"GAME","UI_BOTTOM","UI","TOP_MOST","CURTAIN"};

	for (auto & layer : layerTbl)
	{
		CWindow * pLayer = new CLayer(nullptr);
		pLayer->SetName(layer);
		pLayer->Show();
		m_LayerWindowMap.emplace(layer, pLayer);
		m_pRootWindow->AddChild(pLayer);
		m_LayerWindowList.emplace_back(pLayer);
		}

		m_ActiveWindowList.clear();
		m_LockWindowList.clear();

		gs_poEmptyTuple = Py_BuildValue("()");
	}

	CWindowManager::~CWindowManager()
	{		
		Py_DECREF(gs_poEmptyTuple);

		stl_wipe_second(m_LayerWindowMap);
		m_LayerWindowMap.clear();
		m_LayerWindowList.clear();
		m_KeyCaptureWindowMap.clear();

		delete m_pRootWindow;
		m_pRootWindow = nullptr;
	}

	void CWindowManager::Destroy()
	{
		__ClearReserveDeleteWindowList();
#ifdef __WINDOW_LEAK_CHECK__
		std::set<CWindow*>::iterator i;
		for (i=gs_kSet_pkWnd.begin(); i!=gs_kSet_pkWnd.end(); ++i)
		{
			CWindow* pkWnd=*i;
			Logf(1, "CWindowManager::Destroy LOST WINDOW %s\n", pkWnd->GetName());
		}
#endif		

	}

	void CWindowManager::SetMouseHandler(PyObject * poMouseHandler)
	{
		m_poMouseHandler = poMouseHandler;
	}

	CWindow * CWindowManager::RegisterWindow(PyObject * po, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = new CWindow(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);
#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif
		return (pWin);
	}

	CWindow *	CWindowManager::__NewWindow(PyObject * po, uint32_t dwWndType)
	{
		switch(dwWndType)
		{
			case WT_SLOT:
				return new CSlotWindow(po);
			case WT_GRIDSLOT:
				return new CGridSlotWindow(po);
			case WT_TEXTLINE:
				return new CTextLine(po);
			case WT_MARKBOX:
				return new CMarkBox(po);
			case WT_IMAGEBOX:
				return new CImageBox(po);
			case WT_EXP_IMAGEBOX:
				return new CExpandedImageBox(po);
			case WT_ANI_IMAGEBOX:
				return new CAniImageBox(po);
			case WT_BUTTON:
				return new CButton(po);
			case WT_RATIOBUTTON:
				return new CRadioButton(po);
			case WT_TOGGLEBUTTON:
				return new CToggleButton(po);
			case WT_DRAGBUTTON:
				return new CDragButton(po);
			case WT_BOX:
				return new CBox(po);
			case WT_BAR:
				return new CBar(po);
			case WT_LINE:
				return new CLine(po);
			case WT_BAR3D:
				return new CBar3D(po);
			case WT_NUMLINE:
				return new CNumberLine(po);
			default:
				assert(!"CWindowManager::__NewWindow");
				break;
		}	
		return new CWindow(po);				
	}

	CWindow *	CWindowManager::RegisterTypeWindow(PyObject * po, uint32_t dwWndType, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = __NewWindow(po, dwWndType);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif		
		return pWin;
	}

	CWindow * CWindowManager::RegisterSlotWindow(PyObject * po, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = new CSlotWindow(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif		
		return pWin;
	}

	CWindow * CWindowManager::RegisterGridSlotWindow(PyObject * po, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = new CGridSlotWindow(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif
		return pWin;
	}

	CWindow * CWindowManager::RegisterTextLine(PyObject * po, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = new CTextLine(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif
		return pWin;
	}

	CWindow * CWindowManager::RegisterImageBox(PyObject * po, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = new CImageBox(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif
		return pWin;
	}

	CWindow * CWindowManager::RegisterMarkBox(PyObject * po, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = new CMarkBox(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif
		return pWin;
	}

	CWindow * CWindowManager::RegisterExpandedImageBox(PyObject * po, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = new CExpandedImageBox(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif
		return pWin;
	}

	CWindow * CWindowManager::RegisterAniImageBox(PyObject * po, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = new CAniImageBox(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif
		return pWin;
	}

	CWindow * CWindowManager::RegisterButton(PyObject * po, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = new CButton(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif
		return pWin;
	}

	CWindow * CWindowManager::RegisterRadioButton(PyObject * po, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = new CRadioButton(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif
		return pWin;
	}

	CWindow * CWindowManager::RegisterToggleButton(PyObject * po, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = new CToggleButton(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif
		return pWin;
	}

	CWindow * CWindowManager::RegisterDragButton(PyObject * po, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = new CDragButton(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif
		return pWin;
	}

	CWindow * CWindowManager::RegisterBox(PyObject * po, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = new CBox(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif
		return pWin;
	}

	CWindow * CWindowManager::RegisterBar(PyObject * po, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = new CBar(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif
		return pWin;
	}

	CWindow * CWindowManager::RegisterLine(PyObject * po, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = new CLine(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif
		return pWin;
	}

	CWindow * CWindowManager::RegisterBar3D(PyObject * po, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = new CBar3D(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif
		return pWin;
	}

	CWindow * CWindowManager::RegisterNumberLine(PyObject * po, const char * c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow * pWin = new CNumberLine(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif
		return pWin;
	}

	CWindow* CWindowManager::RegisterRenderTarget(PyObject* po, const char* c_szLayer)
	{
		assert(m_LayerWindowMap.end() != m_LayerWindowMap.find(c_szLayer));

		CWindow* pWin = new CRenderTarget(po);
		m_LayerWindowMap[c_szLayer]->AddChild(pWin);

#ifdef __WINDOW_LEAK_CHECK__
		gs_kSet_pkWnd.insert(pWin);
#endif
		return pWin;
	}

	void CWindowManager::NotifyDestroyWindow(CWindow * pWindow)
	{
		if (pWindow == m_pActiveWindow)
			m_pActiveWindow = nullptr;

		if (pWindow == m_pPointWindow)
			m_pPointWindow = nullptr;

		if (pWindow == m_pLeftCaptureWindow)
			m_pLeftCaptureWindow = nullptr;

		if (pWindow == m_pMiddleCaptureWindow)
			m_pMiddleCaptureWindow = nullptr;

		if (pWindow == m_pRightCaptureWindow)
			m_pRightCaptureWindow = nullptr;

		if (pWindow == m_pLockWindow)
			m_pLockWindow = nullptr;

		m_LockWindowList.remove(pWindow);
		m_ActiveWindowList.remove(pWindow);
		m_PickAlwaysWindowList.remove(pWindow);

		auto itor = m_KeyCaptureWindowMap.begin();
		for (; itor != m_KeyCaptureWindowMap.end();)
		{
			if (pWindow == itor->second)
				itor = m_KeyCaptureWindowMap.erase(itor);
			else
				++itor;
		}
	}

	void CWindowManager::DestroyWindow(CWindow * pWin)
	{
		NotifyDestroyWindow(pWin);
		if (pWin->HasParent())
		{
			CWindow * pParentWin = pWin->GetParent();
			pParentWin->DeleteChild(pWin);
		}
		pWin->Clear();
		m_ReserveDeleteWindowList.emplace_back(pWin);
	}

	bool CWindowManager::IsDragging()
	{
		int32_t ldx = abs(m_lMouseX - m_lPickedX);
		int32_t ldy = abs(m_lMouseY - m_lPickedY);
		if (ldx+ldy < 10)
			return false;

		return true;
	}

	bool CWindowManager::IsAttaching()
	{
		return m_bAttachingFlag;
	}

	uint32_t CWindowManager::GetAttachingType()
	{
		return m_dwAttachingType;
	}

	uint32_t CWindowManager::GetAttachingIndex()
	{
		return m_dwAttachingIndex;
	}

	uint32_t CWindowManager::GetAttachingSlotNumber()
	{
		return m_dwAttachingSlotNumber;
	}
	
	uint32_t CWindowManager::GetAttachingRealSlotNumber()
	{
		return m_dwAttachingRealSlotNumber;
	}

	void CWindowManager::GetAttachingIconSize(uint8_t * pbyWidth, uint8_t * pbyHeight)
	{
		*pbyWidth = m_byAttachingIconWidth;
		*pbyHeight = m_byAttachingIconHeight;
	}

	void CWindowManager::AttachIcon(uint32_t dwType, uint32_t dwIndex, uint32_t dwSlotNumber, uint8_t byWidth, uint8_t byHeight)
	{
		m_bAttachingFlag = true;
		m_dwAttachingType = dwType;
		m_dwAttachingIndex = dwIndex;
		m_dwAttachingSlotNumber = dwSlotNumber;

		m_byAttachingIconWidth = byWidth;
		m_byAttachingIconHeight = byHeight;
	}

	void CWindowManager::SetAttachingFlag(bool bFlag)
	{
		m_bAttachingFlag = bFlag;
	}
	
	void CWindowManager::SetAttachingRealSlotNumber(uint32_t dwRealSlotNumber)
	{
		m_dwAttachingRealSlotNumber = dwRealSlotNumber;
	}

	void CWindowManager::DeattachIcon()
	{
		if (IsDisableDeattach())
			return;
		SetAttachingFlag(false);
		if (m_poMouseHandler)
			PyCallClassMemberFunc(m_poMouseHandler, "DeattachObject", BuildEmptyTuple());
	}

	void CWindowManager::SetParent(CWindow * pWindow, CWindow * pParentWindow)
	{
		if (!pWindow)
		{
			assert(!"CWindowManager::SetParent - There is no self window!");
			return;
		}
		if (!pParentWindow)
		{
			assert(!"There is no parent window");
			return;
		}

		if (pWindow->HasParent())
		{
			CWindow * pOldParentWindow = pWindow->GetParent();

			if (pParentWindow == pOldParentWindow)
				return;

			pOldParentWindow->DeleteChild(pWindow);
		}

		pParentWindow->AddChild(pWindow);
		pWindow->UpdateRect();
	}

	void CWindowManager::SetPickAlways(CWindow * pWindow)
	{
		m_PickAlwaysWindowList.emplace_back(pWindow);
	}

	void CWindowManager::OnceIgnoreMouseLeftButtonUpEvent()
	{
		m_bOnceIgnoreMouseLeftButtonUpEventFlag = true;
		m_dwIgnoreEndTime = timeGetTime() + 500;
	}

	void CWindowManager::LockWindow(CWindow * pWin)
	{
		if (m_pActiveWindow)
			m_pActiveWindow->OnKillFocus();

		// 이미 락된 윈도우리스트안에 있다면 제거한다..
		m_LockWindowList.remove(pWin);

		if (m_pLockWindow)
		{
			if (m_pLockWindow==pWin)
				return;

			m_LockWindowList.emplace_back(m_pLockWindow);
		}

		m_pLockWindow = pWin;
	}

	void CWindowManager::UnlockWindow()
	{
		if (m_pLockWindow)
		{
			if (m_LockWindowList.empty())
			{
				m_pLockWindow = nullptr;
				if (m_pActiveWindow)
					m_pActiveWindow->OnSetFocus();
			}
			else
			{
				m_pLockWindow = m_LockWindowList.back();
				m_LockWindowList.pop_back();
			}			
		}
	}

	void CWindowManager::ActivateWindow(CWindow * pWin)
	{
		m_ActiveWindowList.remove(pWin);

		if (pWin == m_pActiveWindow)
			return;

		if (m_pActiveWindow)
		{
			// NOTE : 누적된 Window가 많아지면 Clear를 해줘야 할까?
			//        일단은 중복 누적이 안되며 포커스 되는 갯수 자체가 5개 미만이니 굳이 필요하지는 않을 듯.. - [levites]
			m_ActiveWindowList.emplace_back(m_pActiveWindow);
			m_pActiveWindow->OnKillFocus();
		}

		m_pActiveWindow = pWin;

		if (m_pActiveWindow)
			m_pActiveWindow->OnSetFocus();
	}

	void CWindowManager::DeactivateWindow()
	{
		if (m_pActiveWindow)
		{
			if (m_ActiveWindowList.empty())
			{
				m_pActiveWindow->OnKillFocus();
				m_pActiveWindow = nullptr;
			}
			else
			{
				m_pActiveWindow->OnKillFocus();

				m_pActiveWindow = m_ActiveWindowList.back();
				m_ActiveWindowList.pop_back();

				m_pActiveWindow->OnSetFocus();
			}
		}
	}

	void CWindowManager::SetTop(CWindow * pWin)
	{
		if (!pWin->HasParent())
			return;

		CWindow * pParentWindow = pWin->GetParent();
		pParentWindow->SetTop(pWin);

		// NOTE : Capture가 리셋된다..? - [levites]
		// NOTE : 인벤토리에서 아이템을 드래그 해서 밖에다 놓을때 캡춰가 남아서 창의 버튼을 두번 눌러야 하는 버그를 위해 추가
//		ResetCapture();
	}

	void CWindowManager::SetTopUIWindow()
	{
		if (m_pLockWindow)
			return;

		// GameLayer에 속해 있는 윈도우가 피킹 됐다면 무조건 SetTop을 해준다.
		auto itor = m_LayerWindowMap.find("UI");
		if (itor == m_LayerWindowMap.end())
			return;
		CWindow * pGameLayer = itor->second;
		CWindow * pTopWindow = pGameLayer->PickTopWindow(m_lMouseX, m_lMouseY);
		if (pTopWindow)
			SetTop(pTopWindow);
	}

	CWindow * CWindowManager::GetActivateWindow()
	{
		return m_pActiveWindow;
	}

	void CWindowManager::ResetCapture()
	{
		m_pLeftCaptureWindow = nullptr;
		m_pMiddleCaptureWindow = nullptr;
		m_pRightCaptureWindow = nullptr;
	}

	void CWindowManager::SetResolution(int32_t hres, int32_t vres)
	{
		if (hres<=0 || vres<=0)
			return;

		m_iHres = hres;
		m_iVres = vres;
	}

	float CWindowManager::GetAspect()
	{
		return (m_iHres)/float(m_iVres);
	}

	void CWindowManager::SetScreenSize(int32_t lWidth, int32_t lHeight)
	{
		m_lWidth	= lWidth;
		m_lHeight	= lHeight;

		for (auto& itor : m_LayerWindowMap)
			itor.second->SetSize(lWidth, lHeight);
	}

	void CWindowManager::__ClearReserveDeleteWindowList()
	{
		for (TWindowContainer::iterator itor = m_ReserveDeleteWindowList.begin(); itor != m_ReserveDeleteWindowList.end(); ++itor)
		{
			CWindow * pWin = *itor;
#ifdef __WINDOW_LEAK_CHECK__
			gs_kSet_pkWnd.erase(pWin);
#endif
			delete pWin;
		}
		m_ReserveDeleteWindowList.clear();

	}	

	void CWindowManager::Update()
	{
		__ClearReserveDeleteWindowList();
		
		m_pRootWindow->Update();
	}

	void CWindowManager::Render()
	{
		m_pRootWindow->Render();
	}

	CWindow * CWindowManager::__PickWindow(int32_t x, int32_t y)
	{
		if (m_pLockWindow)
			return m_pLockWindow->PickWindow(x, y);

		for (auto pWindow : m_PickAlwaysWindowList)
		{
			if (pWindow->IsRendering())
			{
				if (pWindow->IsIn(x, y))
					return pWindow;
			}
		}

		for (auto ritor = m_LayerWindowList.rbegin(); ritor != m_LayerWindowList.rend(); ++ritor)
		{
			CWindow * pLayer = *ritor;
			CWindow * pPickedWindow = pLayer->PickWindow(x, y);

			if (pPickedWindow != pLayer)
				return pPickedWindow;
		}

		return nullptr;
	}

	void CWindowManager::SetMousePosition(int32_t x, int32_t y)
	{
		if (m_iHres==0)
			return;

		if (m_iVres==0)
			return;

		m_lMouseX = m_lWidth * x / m_iHres;
		m_lMouseY = m_lHeight * y / m_iVres;
	}

	void CWindowManager::GetMousePosition(int32_t & rx, int32_t & ry)
	{
		rx = m_lMouseX;
		ry = m_lMouseY;
	}

	void CWindowManager::RunMouseMove(int32_t x, int32_t y)
	{
		if (IsAttaching())
		{
			if (x > m_lWidth)
				x = m_lWidth;
			if (y > m_lHeight)
				y = m_lHeight;
		}

		SetMousePosition(x, y);
		CWindow * pPointWindow = __PickWindow(m_lMouseX, m_lMouseY);

		if (g_bShowOverInWindowName)
		{
			if (pPointWindow)
			{
			static std::string strPickWindowName;
			if (pPointWindow->GetName() != strPickWindowName)
				{
					Tracef(" OverInWindowName [%s]\n", pPointWindow->GetName());
					strPickWindowName = pPointWindow->GetName();
				}
			}
		}

		if (m_pLeftCaptureWindow)
		{
			CWindow * pWin = m_pLeftCaptureWindow;

			if (pWin->IsFlag(CWindow::FLAG_MOVABLE))
			{
			int32_t localX = m_lMouseX - m_lDragX;
			int32_t localY = m_lMouseY - m_lDragY;
			if (pWin->HasParent())
			{
				localX -= pWin->GetParent()->GetRect().left;
				localY -= pWin->GetParent()->GetRect().top;
			}

			int32_t lx, ly;
			pWin->GetPosition(&lx, &ly);
			if (pWin->IsFlag(CWindow::FLAG_RESTRICT_X))
				localX = lx;
			if (pWin->IsFlag(CWindow::FLAG_RESTRICT_Y))
				localY = ly;

			if (pWin->IsFlag(CWindow::FLAG_LIMIT))
			{
				RECT limitRect = pWin->GetLimitBias();

				limitRect.right = m_lWidth - limitRect.right;
				limitRect.bottom = m_lHeight - limitRect.bottom;

				if (localX < limitRect.left)
					localX = limitRect.left;
				else if (localX + pWin->GetWidth() >= limitRect.right)
					localX = limitRect.right - pWin->GetWidth();

				if (localY < limitRect.top)
					localY = limitRect.top;
				else if (localY + pWin->GetHeight() >= limitRect.bottom)
					localY = limitRect.bottom - pWin->GetHeight();
			}

			pWin->SetPosition(localX, localY);
			pWin->OnMoveWindow(localX, localY);
		}
		else if (m_pLeftCaptureWindow->IsFlag(CWindow::FLAG_DRAGABLE))
		{
			const int32_t localX = m_lMouseX - m_lDragX;
			const int32_t localY = m_lMouseY - m_lDragY;
			m_pLeftCaptureWindow->OnMouseDrag(localX, localY);
			}
		}

		if (m_pPointWindow != pPointWindow)
		{
#ifdef _DEBUG
			if (pPointWindow && pPointWindow->GetName())
			{
				if (!strcmp(pPointWindow->GetName(), "!!debug"))
				{
					_asm nop;
				}
				Tracenf("PointWindow: %s", pPointWindow->GetName());
			}
#endif
			if (m_pPointWindow)
				m_pPointWindow->OnMouseOverOut();

			m_pPointWindow = pPointWindow;

			if (m_pPointWindow)
				m_pPointWindow->OnMouseOverIn();
		}

		if (m_pPointWindow)
			m_pPointWindow->OnMouseOver();
	}

	void CWindowManager::RunMouseLeftButtonDown(int32_t x, int32_t y)
	{
		SetTopUIWindow();

		/////

		SetMousePosition(x, y);
		CWindow * pWin = GetPointWindow();

		if (!pWin)
			return;

		// Attach
		if (pWin->IsFlag(CWindow::FLAG_ATTACH))
			pWin = pWin->GetRoot();

		// Drag
		if (!pWin->IsFlag(CWindow::FLAG_NOT_CAPTURE))
			m_pLeftCaptureWindow = pWin;

		m_lDragX = m_lMouseX - pWin->GetRect().left;
		m_lDragY = m_lMouseY - pWin->GetRect().top;
		m_lPickedX = m_lMouseX;
		m_lPickedY = m_lMouseY;

		// Event
		pWin->OnMouseLeftButtonDown();
	}

	void CWindowManager::RunMouseLeftButtonUp(int32_t x, int32_t y)
	{
		if (m_bOnceIgnoreMouseLeftButtonUpEventFlag)
		{
			m_bOnceIgnoreMouseLeftButtonUpEventFlag = false;

			if (timeGetTime() < m_dwIgnoreEndTime)
				return;
		}

		SetMousePosition(x, y);

		if (m_pLeftCaptureWindow)
		{
			if (m_pLeftCaptureWindow->OnMouseLeftButtonUp())
			{
				// NOTE : 여기서 m_pLeftCaptureWindow가 nullptr 일 수 있습니다!! - [levites]
				m_pLeftCaptureWindow = nullptr;
				return;
			}
		}

		CWindow * pWin = GetPointWindow();
		if (pWin)
			pWin->OnMouseLeftButtonUp();

		m_pLeftCaptureWindow = nullptr;
	}

	void CWindowManager::RunMouseLeftButtonDoubleClick(int32_t x, int32_t y)
	{
		SetMousePosition(x, y);

		CWindow * pWin = GetPointWindow();
		if (!pWin)
			return;

		pWin->OnMouseLeftButtonDoubleClick();
	}

	void CWindowManager::RunMouseRightButtonDown(int32_t x, int32_t y)
	{
		SetTopUIWindow();

		///////////////////////////////////////////////////////////////////////////////////////////

		SetMousePosition(x, y);
		CWindow * pWin = GetPointWindow();
		if (!pWin)
			return;

		// Attach
		if (pWin->IsFlag(CWindow::FLAG_ATTACH))
			pWin = pWin->GetRoot();

		// Drag
		if (!pWin->IsFlag(CWindow::FLAG_NOT_CAPTURE))
			m_pRightCaptureWindow = pWin;

		pWin->OnMouseRightButtonDown();
	}

	void CWindowManager::RunMouseRightButtonUp(int32_t x, int32_t y)
	{
		if (m_pRightCaptureWindow)
		{
			if (m_pRightCaptureWindow->OnMouseRightButtonUp())
			{
				m_pRightCaptureWindow = nullptr;
				return;
			}
		}

		CWindow * pWin = GetPointWindow();
		if (pWin)
			pWin->OnMouseRightButtonUp();

		m_pRightCaptureWindow = nullptr;
		DeattachIcon();
	}

	void CWindowManager::RunMouseRightButtonDoubleClick(int32_t x, int32_t y)
	{
		SetMousePosition(x, y);

		CWindow * pWin = GetPointWindow();
		if (pWin)
			pWin->OnMouseRightButtonDoubleClick();
	}

	void CWindowManager::RunMouseMiddleButtonDown(int32_t x, int32_t y)
	{
		SetMousePosition(x, y);

		CWindow * pWin = GetPointWindow();
		if (!pWin)
			return;

		m_pMiddleCaptureWindow = pWin;

		pWin->OnMouseMiddleButtonDown();
	}

	void CWindowManager::RunMouseMiddleButtonUp(int32_t x, int32_t y)
	{
		SetMousePosition(x, y);

		if (m_pMiddleCaptureWindow)
		{
			if (m_pMiddleCaptureWindow->OnMouseMiddleButtonUp())
			{
				m_pMiddleCaptureWindow = nullptr;
				return;
			}
		}

		CWindow * pWin = GetPointWindow();
		if (!pWin)
			return;

		pWin->OnMouseMiddleButtonUp();
		m_pMiddleCaptureWindow = nullptr;
	}


#ifdef ENABLE_MOUSEWHEEL_EVENT
	bool CWindowManager::RunMouseWheelScroll(int32_t x, int32_t y, int16_t wDelta)
	{
		SetMousePosition(x, y);
		CWindow * pWin = GetPointWindow();
		if (!pWin)
			return false;

		return pWin->OnMouseWheelScroll(wDelta) == true;
	}
#endif

	// IME
	void CWindowManager::RunIMEUpdate()
	{
		if (m_pLockWindow)
		{
			m_pLockWindow->OnIMEUpdate();
			return;
		}

		if (!m_pActiveWindow)
			return;
		if (!m_pActiveWindow->IsRendering())
			return;

		m_pActiveWindow->OnIMEUpdate();
	}

	void CWindowManager::RunIMETabEvent()
	{
		if (m_pLockWindow)
		{
			m_pLockWindow->OnIMETabEvent();
			return;
		}
		if (m_pActiveWindow)
		{
			if (m_pActiveWindow->IsRendering())
			{
				if (m_pActiveWindow->OnIMETabEvent())
					return;
			}
		}

		if (!m_pRootWindow->RunIMETabEvent())
		{
			if (!m_ActiveWindowList.empty())
			{
				CWindow * pWindow = *(m_ActiveWindowList.begin());
				ActivateWindow(pWindow);

				/////////////////////////////////////////////

				CWindow * pParentWindow = pWindow;
				CWindow * pCurrentWindow = pWindow->GetParent();

				uint32_t dwMaxLoopCount = 20;
				for (uint32_t i = 0; i < dwMaxLoopCount; ++i)
				{
					if (!pParentWindow)
						break;

					if (pParentWindow == m_LayerWindowMap["GAME"])
					{
						SetTop(pCurrentWindow);
						break;
					}

					pCurrentWindow = pParentWindow;
					pParentWindow = pCurrentWindow->GetParent();
				}
			}
		}
	}

	void CWindowManager::RunIMEReturnEvent()
	{
		if (m_pLockWindow)
		{
			m_pLockWindow->OnIMEReturnEvent();
			return;
		}
		if (m_pActiveWindow)
		{
			if (m_pActiveWindow->IsRendering())
			{
				if (m_pActiveWindow->OnIMEReturnEvent())
					return;
			}
		}

		m_pRootWindow->RunIMEReturnEvent();
	}

	void CWindowManager::RunIMEKeyDown(int32_t vkey)
	{
		if (m_pLockWindow)
		{
			m_pLockWindow->OnKeyDown(vkey);
			return;
		}
		if (m_pActiveWindow)
		{
			if (m_pActiveWindow->IsRendering())
			{
				if (m_pActiveWindow->OnIMEKeyDownEvent(vkey))
					return;
			}
		}

		// NOTE : 전체로 돌리지 않고 Activate되어있는 EditLine에만 보내는 이벤트
	}

	void CWindowManager::RunChangeCodePage()
	{
		if (m_pActiveWindow)
		{
			if (m_pActiveWindow->IsRendering())
			{
				if (m_pActiveWindow->OnIMEChangeCodePage())
					return;
			}
		}
	}
	void CWindowManager::RunOpenCandidate()
	{
		if (m_pLockWindow)
		{
			m_pLockWindow->OnIMEOpenCandidateListEvent();
			return;
		}
		if (m_pActiveWindow)
		{
			if (m_pActiveWindow->IsRendering())
			{
				if (m_pActiveWindow->OnIMEOpenCandidateListEvent())
					return;
			}
		}
	}

	void CWindowManager::RunCloseCandidate()
	{
		if (m_pLockWindow)
		{
			m_pLockWindow->OnIMECloseCandidateListEvent();
			return;
		}
		if (m_pActiveWindow)
		{
			if (m_pActiveWindow->IsRendering())
			{
				if (m_pActiveWindow->OnIMECloseCandidateListEvent())
					return;
			}
		}
	}

	void CWindowManager::RunOpenReading()
	{
		if (m_pLockWindow)
		{
			m_pLockWindow->OnIMEOpenReadingWndEvent();
			return;
		}
		if (m_pActiveWindow)
		{
			if (m_pActiveWindow->IsRendering())
			{
				if (m_pActiveWindow->OnIMEOpenReadingWndEvent())
					return;
			}
		}
	}

	void CWindowManager::RunCloseReading()
	{
		if (m_pLockWindow)
		{
			m_pLockWindow->OnIMECloseReadingWndEvent();
			return;
		}
		if (m_pActiveWindow)
		{
			if (m_pActiveWindow->IsRendering())
			{
				if (m_pActiveWindow->OnIMECloseReadingWndEvent())
					return;
			}
		}
	}
	// IME

	void CWindowManager::RunKeyDown(int32_t vkey)
	{
		if (m_pLockWindow)
		{
			m_pLockWindow->OnKeyDown(vkey);
			m_KeyCaptureWindowMap.emplace(vkey, m_pLockWindow);
			return;
		}
		if (m_pActiveWindow)
		{
			if (m_pActiveWindow->IsShow())
			{
				if (m_pActiveWindow->OnKeyDown(vkey))
				{
					m_KeyCaptureWindowMap.emplace(vkey, m_pActiveWindow);
					return;
				}
			}
		}

		CWindow * pKeyCaptureWindow = m_pRootWindow->RunKeyDownEvent(vkey);
		if (pKeyCaptureWindow)
		{
			if (m_ReserveDeleteWindowList.end() ==
				std::find(m_ReserveDeleteWindowList.begin(), m_ReserveDeleteWindowList.end(), pKeyCaptureWindow))
				m_KeyCaptureWindowMap.emplace(vkey, pKeyCaptureWindow);
		}
	}

	void CWindowManager::RunKeyUp(int32_t vkey)
	{
		auto itor = m_KeyCaptureWindowMap.find(vkey);
		if (m_KeyCaptureWindowMap.end() != itor)
		{
			CWindow * pKeyCaptureWindow = itor->second;
			if (pKeyCaptureWindow)
				pKeyCaptureWindow->OnKeyUp(vkey);

			m_KeyCaptureWindowMap.erase(itor);
			return;
		}

		if (m_pLockWindow)
		{
			m_pLockWindow->OnKeyUp(vkey);
			return;
		}

		if (m_pActiveWindow)
		{
			if (m_pActiveWindow->OnKeyUp(vkey))
				return;
		}

		m_pRootWindow->RunKeyUpEvent(vkey);
	}

	void CWindowManager::RunPressEscapeKey()
	{
		if (m_pLockWindow)
		{
			m_pLockWindow->OnPressEscapeKey();
			return;
		}

		m_pRootWindow->RunPressEscapeKeyEvent();
	}

	void CWindowManager::RunPressExitKey()
	{
		if (m_pLockWindow)
		{
			m_pLockWindow->OnPressExitKey();
			return;
		}
		if (m_pActiveWindow)
		{
			if (m_pActiveWindow->OnPressExitKey())
				return;
		}

		m_pRootWindow->RunPressExitKeyEvent();
	}
}
