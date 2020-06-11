#pragma once

namespace UI 
{
	class CWindow;

	class CWindowManager : public CSingleton<CWindowManager>
	{
		public:
			typedef std::map<std::string, CWindow *> TLayerContainer;
			typedef std::list<CWindow *> TWindowContainer;
			typedef std::map<int32_t, CWindow *> TKeyCaptureWindowMap;

		public:
			CWindowManager();
			virtual ~CWindowManager();

			void		Destroy();

			float		GetAspect();
			void		SetScreenSize(int32_t lWidth, int32_t lHeight);
			void		SetResolution(int32_t hres, int32_t vres);

			void		GetResolution(int32_t & rx, int32_t & ry)
			{
				rx=m_iHres;
				ry=m_iVres;
			}

			void		SetMouseHandler(PyObject * poMouseHandler);
			int32_t		GetScreenWidth()		{ return m_lWidth; }
			int32_t		GetScreenHeight()		{ return m_lHeight; }
			void		GetMousePosition(int32_t & rx, int32_t & ry);
			BOOL		IsDragging();

			CWindow *	GetLockWindow()		{ return m_pLockWindow; }
			CWindow *	GetPointWindow()	{ return m_pPointWindow; }
			bool		IsFocus()			{ return (m_pActiveWindow || m_pLockWindow); }
			bool		IsFocusWindow(CWindow * pWindow)	{ return pWindow == m_pActiveWindow; }

			void		SetParent(CWindow * pWindow, CWindow * pParentWindow);
			void		SetPickAlways(CWindow * pWindow);

			enum
			{		
				WT_NORMAL,
				WT_SLOT,
				WT_GRIDSLOT,
				WT_TEXTLINE,
				WT_MARKBOX,
				WT_IMAGEBOX,
				WT_EXP_IMAGEBOX,
				WT_ANI_IMAGEBOX,
				WT_BUTTON,
				WT_RATIOBUTTON,
				WT_TOGGLEBUTTON,
				WT_DRAGBUTTON,
				WT_BOX,
				WT_BAR,
				WT_LINE,
				WT_BAR3D,
				WT_NUMLINE,				
			};

			CWindow *	RegisterWindow(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterTypeWindow(PyObject * po, uint32_t dwWndType, const char * c_szLayer);

			CWindow *	RegisterSlotWindow(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterGridSlotWindow(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterTextLine(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterMarkBox(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterImageBox(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterExpandedImageBox(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterAniImageBox(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterButton(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterRadioButton(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterToggleButton(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterDragButton(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterBox(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterBar(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterLine(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterBar3D(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterNumberLine(PyObject * po, const char * c_szLayer);

			void		DestroyWindow(CWindow * pWin);
			void		NotifyDestroyWindow(CWindow * pWindow);

			// Attaching Icon
			BOOL		IsAttaching();
			uint32_t		GetAttachingType();
			uint32_t		GetAttachingIndex();
			uint32_t		GetAttachingSlotNumber();
			uint32_t		GetAttachingRealSlotNumber();
			void		GetAttachingIconSize(uint8_t * pbyWidth, uint8_t * pbyHeight);
			void		AttachIcon(uint32_t dwType, uint32_t dwIndex, uint32_t dwSlotNumber, uint8_t byWidth, uint8_t byHeight);
			void		DeattachIcon();
			void		SetAttachingFlag(BOOL bFlag);
			void		SetAttachingRealSlotNumber(uint32_t dwRealslotNumber);
			// Attaching Icon

			void		OnceIgnoreMouseLeftButtonUpEvent();
			void		LockWindow(CWindow * pWin);
			void		UnlockWindow();

			void		ActivateWindow(CWindow * pWin);
			void		DeactivateWindow();
			CWindow *	GetActivateWindow();
			void		SetTop(CWindow * pWin);
			void		SetTopUIWindow();
			void		ResetCapture();

			void		Update();
			void		Render();

			void		RunMouseMove(int32_t x, int32_t y);
			void		RunMouseLeftButtonDown(int32_t x, int32_t y);
			void		RunMouseLeftButtonUp(int32_t x, int32_t y);
			void		RunMouseLeftButtonDoubleClick(int32_t x, int32_t y);
			void		RunMouseRightButtonDown(int32_t x, int32_t y);
			void		RunMouseRightButtonUp(int32_t x, int32_t y);
			void		RunMouseRightButtonDoubleClick(int32_t x, int32_t y);
			void		RunMouseMiddleButtonDown(int32_t x, int32_t y);
			void		RunMouseMiddleButtonUp(int32_t x, int32_t y);
#ifdef ENABLE_MOUSEWHEEL_EVENT
			bool		RunMouseWheelScroll(int32_t x, int32_t y , int16_t wDelta);
#endif

			void		RunIMEUpdate();
			void		RunIMETabEvent();
			void		RunIMEReturnEvent();
			void		RunIMEKeyDown(int32_t vkey);
			void		RunChangeCodePage();
			void		RunOpenCandidate();
			void		RunCloseCandidate();
			void		RunOpenReading();
			void		RunCloseReading();

			void		RunKeyDown(int32_t vkey);
			void		RunKeyUp(int32_t vkey);
			void		RunPressEscapeKey();
			void		RunPressExitKey();

		private:
			void		SetMousePosition(int32_t x, int32_t y);
			CWindow *	__PickWindow(int32_t x, int32_t y);
			
			CWindow *	__NewWindow(PyObject * po, uint32_t dwWndType);
			void		__ClearReserveDeleteWindowList();

		private:
			int32_t					m_lWidth;
			int32_t					m_lHeight;

			int32_t						m_iVres;
			int32_t						m_iHres;

			int32_t					m_lMouseX, m_lMouseY;
			int32_t					m_lDragX, m_lDragY;
			int32_t					m_lPickedX, m_lPickedY;

			BOOL					m_bOnceIgnoreMouseLeftButtonUpEventFlag;
			int32_t						m_iIgnoreEndTime;

			// Attaching Icon
			PyObject *				m_poMouseHandler;
			BOOL					m_bAttachingFlag;
			uint32_t					m_dwAttachingType;
			uint32_t					m_dwAttachingIndex;
			uint32_t					m_dwAttachingSlotNumber;
			uint32_t					m_dwAttachingRealSlotNumber;
			uint8_t					m_byAttachingIconWidth;
			uint8_t					m_byAttachingIconHeight;
			// Attaching Icon

			CWindow	*				m_pActiveWindow;
			TWindowContainer		m_ActiveWindowList;
			CWindow *				m_pLockWindow;
			TWindowContainer		m_LockWindowList;
			CWindow	*				m_pPointWindow;
			CWindow	*				m_pLeftCaptureWindow;
			CWindow	*				m_pRightCaptureWindow;
			CWindow *				m_pMiddleCaptureWindow;
			TKeyCaptureWindowMap	m_KeyCaptureWindowMap;
			TWindowContainer		m_ReserveDeleteWindowList;
			TWindowContainer		m_PickAlwaysWindowList;

			CWindow *				m_pRootWindow;
			TWindowContainer		m_LayerWindowList;
			TLayerContainer			m_LayerWindowMap;
	};

	PyObject * BuildEmptyTuple();
};
