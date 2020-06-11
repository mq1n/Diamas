#pragma once

#include "../eterBase/Utils.h"
#include "../eterXClient/Locale_inc.h"

namespace UI
{
	class CWindow
	{
		public:
			typedef std::list<CWindow *> TWindowContainer;

			static uint32_t Type();
			BOOL IsType(uint32_t dwType);

			enum EHorizontalAlign
			{
				HORIZONTAL_ALIGN_LEFT = 0,
				HORIZONTAL_ALIGN_CENTER = 1,
				HORIZONTAL_ALIGN_RIGHT = 2,					
			};

			enum EVerticalAlign
			{
				VERTICAL_ALIGN_TOP = 0,
				VERTICAL_ALIGN_CENTER = 1,
				VERTICAL_ALIGN_BOTTOM = 2,
			};

			enum EFlags
			{
				FLAG_MOVABLE			= (1 <<  0),	// 움직일 수 있는 창
				FLAG_LIMIT				= (1 <<  1),	// 창이 화면을 벗어나지 않음
				FLAG_SNAP				= (1 <<  2),	// 스냅 될 수 있는 창
				FLAG_DRAGABLE			= (1 <<  3),
				FLAG_ATTACH				= (1 <<  4),	// 완전히 부모에 붙어 있는 창 (For Drag / ex. ScriptWindow)
				FLAG_RESTRICT_X			= (1 <<  5),	// 좌우 이동 제한
				FLAG_RESTRICT_Y			= (1 <<  6),	// 상하 이동 제한
				FLAG_NOT_CAPTURE		= (1 <<  7),
				FLAG_FLOAT				= (1 <<  8),	// 공중에 떠있어서 순서 재배치가 되는 창
				FLAG_NOT_PICK			= (1 <<  9),	// 마우스에 의해 Pick되지 않는 창
				FLAG_IGNORE_SIZE		= (1 << 10),
				FLAG_RTL				= (1 << 11),	// Right-to-left
				FLAG_ALPHA_SENSITIVE	= (1 << 12),
			};
			
			enum WindowTypes // window type flags to recognize expanded_image class
			{
				WINDOW_TYPE_WINDOW,
				WINDOW_TYPE_EX_IMAGE,

				WINDOW_TYPE_MAX_NUM
			};

		public:
			CWindow(PyObject * ppyObject);
			virtual ~CWindow();

			void			AddChild(CWindow * pWin);

			void			Clear();
			void			DestroyHandle();
			void			Update();
			void			Render();

			void			SetName(const char * c_szName);
			const char *	GetName()		{ return m_strName.c_str(); }
			void			SetSize(int32_t width, int32_t height);
			int32_t			GetWidth()		{ return m_lWidth; }
			int32_t			GetHeight()		{ return m_lHeight; }

			void			SetHorizontalAlign(uint32_t dwAlign);
			void			SetVerticalAlign(uint32_t dwAlign);
			void			SetPosition(int32_t x, int32_t y);
			void			GetPosition(int32_t * plx, int32_t * ply);
			int32_t			GetPositionX( void ) const		{ return m_x; }
			int32_t			GetPositionY( void ) const		{ return m_y; }
			RECT &			GetRect()		{ return m_rect; }
			void			GetLocalPosition(int32_t & rlx, int32_t & rly);
			void			GetMouseLocalPosition(int32_t & rlx, int32_t & rly);
			int32_t			UpdateRect();

			RECT &			GetLimitBias()	{ return m_limitBiasRect; }
			void			SetLimitBias(int32_t l, int32_t r, int32_t t, int32_t b) { m_limitBiasRect.left = l, m_limitBiasRect.right = r, m_limitBiasRect.top = t, m_limitBiasRect.bottom = b; }

			void			Show();
			void			Hide();
			bool			IsShow() { return m_bShow; }
			bool			IsRendering();

			bool			HasParent()		{ return m_pParent ? true : false; }
			bool			HasChild()		{ return m_pChildList.empty() ? false : true; }
			int32_t				GetChildCount()	{ return m_pChildList.size(); }
			
			void			IsTransparentOnPixel(int32_t* x, int32_t* y, bool* ret);

			CWindow *		GetRoot();
			CWindow *		GetParent();
			bool			IsChild(CWindow * pWin);
			void			DeleteChild(CWindow * pWin);
			void			SetTop(CWindow * pWin);

			bool			IsIn(int32_t x, int32_t y);
			bool			IsIn();
			CWindow *		PickWindow(int32_t x, int32_t y);
			CWindow *		PickTopWindow(int32_t x, int32_t y);	// NOTE : Children으로 내려가지 않고 상위에서만 
															//        체크 하는 특화된 함수

			void			__RemoveReserveChildren();

			void			AddFlag(uint32_t flag)		{ SET_BIT(m_dwFlag, flag);		}
			void			RemoveFlag(uint32_t flag)	{ REMOVE_BIT(m_dwFlag, flag);	}
			bool			IsFlag(uint32_t flag)		{ return (m_dwFlag & flag) ? true : false;	}
			/////////////////////////////////////

			virtual void	OnRender();
			virtual void	OnUpdate();
			virtual void	OnChangePosition(){}

			virtual void	OnSetFocus();
			virtual void	OnKillFocus();

			virtual void	OnMouseDrag(int32_t lx, int32_t ly);
			virtual void	OnMouseOverIn();
			virtual void	OnMouseOverOut();
			virtual void	OnMouseOver();
			virtual void	OnDrop();
			virtual void	OnTop();
			virtual void	OnIMEUpdate();

			virtual void	OnMoveWindow(int32_t x, int32_t y);

			///////////////////////////////////////

			BOOL			RunIMETabEvent();
			BOOL			RunIMEReturnEvent();
			BOOL			RunIMEKeyDownEvent(int32_t ikey);

			CWindow *		RunKeyDownEvent(int32_t ikey);
			BOOL			RunKeyUpEvent(int32_t ikey);
			BOOL			RunPressEscapeKeyEvent();
			BOOL			RunPressExitKeyEvent();

			virtual BOOL	OnIMETabEvent();
			virtual BOOL	OnIMEReturnEvent();
			virtual BOOL	OnIMEKeyDownEvent(int32_t ikey);

			virtual BOOL	OnIMEChangeCodePage();
			virtual BOOL	OnIMEOpenCandidateListEvent();
			virtual BOOL	OnIMECloseCandidateListEvent();
			virtual BOOL	OnIMEOpenReadingWndEvent();
			virtual BOOL	OnIMECloseReadingWndEvent();

			virtual BOOL	OnMouseLeftButtonDown();
			virtual BOOL	OnMouseLeftButtonUp();
			virtual BOOL	OnMouseLeftButtonDoubleClick();
			virtual BOOL	OnMouseRightButtonDown();
			virtual BOOL	OnMouseRightButtonUp();
			virtual BOOL	OnMouseRightButtonDoubleClick();
			virtual BOOL	OnMouseMiddleButtonDown();
			virtual BOOL	OnMouseMiddleButtonUp();
#ifdef ENABLE_MOUSEWHEEL_EVENT
			virtual BOOL	OnMouseWheelScroll(int16_t wDelta);
			virtual void	SetScrollable();
#endif
			virtual BOOL	OnKeyDown(int32_t ikey);
			virtual BOOL	OnKeyUp(int32_t ikey);
			virtual BOOL	OnPressEscapeKey();
			virtual BOOL	OnPressExitKey();
			///////////////////////////////////////

			virtual void	SetColor(uint32_t dwColor){}
			virtual BOOL	OnIsType(uint32_t dwType);
			/////////////////////////////////////

			virtual BOOL	IsWindow() { return TRUE; }
			/////////////////////////////////////

		protected:
			std::string			m_strName;

			EHorizontalAlign	m_HorizontalAlign;
			EVerticalAlign		m_VerticalAlign;
			int32_t				m_x, m_y;				// X,Y 상대좌표
			int32_t				m_lWidth, m_lHeight;	// 크기
			RECT				m_rect;					// Global 좌표
			RECT				m_limitBiasRect;		// limit bias 값

			bool				m_bMovable;
			bool				m_bShow;

			uint32_t				m_dwFlag;			

			PyObject *			m_poHandler;

			CWindow	*			m_pParent;
			TWindowContainer	m_pChildList;

			BOOL				m_isUpdatingChildren;
			TWindowContainer	m_pReserveChildList;
			
			uint8_t				m_windowType; // to recognize window type

#ifdef ENABLE_MOUSEWHEEL_EVENT
			bool				m_bIsScrollable;
#endif

#ifdef _DEBUG
		public:
			uint32_t				DEBUG_dwCounter;
#endif
	};

	class CLayer : public CWindow
	{
		public:
			CLayer(PyObject * ppyObject) : CWindow(ppyObject) {}
			virtual ~CLayer() {}

			BOOL IsWindow() { return FALSE; }
	};

	class CBox : public CWindow
	{
		public:
			CBox(PyObject * ppyObject);
			virtual ~CBox();

			void SetColor(uint32_t dwColor);

		protected:
			void OnRender();

		protected:
			uint32_t m_dwColor;
	};

	class CBar : public CWindow
	{
		public:
			CBar(PyObject * ppyObject);
			virtual ~CBar();

			void SetColor(uint32_t dwColor);

		protected:
			void OnRender();

		protected:
			uint32_t m_dwColor;
	};

	class CLine : public CWindow
	{
		public:
			CLine(PyObject * ppyObject);
			virtual ~CLine();

			void SetColor(uint32_t dwColor);

		protected:
			void OnRender();

		protected:
			uint32_t m_dwColor;
	};

	class CBar3D : public CWindow
	{
		public:
			static uint32_t Type();

		public:
			CBar3D(PyObject * ppyObject);
			virtual ~CBar3D();

			void SetColor(uint32_t dwLeft, uint32_t dwRight, uint32_t dwCenter);

		protected:
			void OnRender();

		protected:
			uint32_t m_dwLeftColor;
			uint32_t m_dwRightColor;
			uint32_t m_dwCenterColor;
	};

	// Text
	class CTextLine : public CWindow
	{
		public:
			CTextLine(PyObject * ppyObject);
			virtual ~CTextLine();

			void SetMax(int32_t iMax);
			void SetHorizontalAlign(int32_t iType);
			void SetVerticalAlign(int32_t iType);
			void SetSecret(BOOL bFlag);
			void SetOutline(BOOL bFlag);
			void SetFeather(BOOL bFlag);
			void SetMultiLine(BOOL bFlag);
			void SetFontName(const char * c_szFontName);
			void SetFontColor(uint32_t dwColor);
			void SetLimitWidth(float fWidth);

			void ShowCursor();
			void HideCursor();
			int32_t GetCursorPosition();

			void SetText(const char * c_szText);
			const char * GetText();
			
			void GetTextSize(int32_t* pnWidth, int32_t* pnHeight);

		protected:
			void OnUpdate();
			void OnRender();
			void OnChangePosition();

			virtual void OnSetText(const char * c_szText);

		protected:
			CGraphicTextInstance m_TextInstance;
	};

	class CNumberLine : public CWindow
	{
		public:
			CNumberLine(PyObject * ppyObject);
			CNumberLine(CWindow * pParent);
			virtual ~CNumberLine();

			void SetPath(const char * c_szPath);
			void SetHorizontalAlign(int32_t iType);
			void SetNumber(const char * c_szNumber);

		protected:
			void ClearNumber();
			void OnRender();
			void OnChangePosition();

		protected:
			std::string m_strPath;
			std::string m_strNumber;
			std::vector<CGraphicImageInstance *> m_ImageInstanceVector;

			int32_t m_iHorizontalAlign;
			uint32_t m_dwWidthSummary;
	};

	// Image
	class CImageBox : public CWindow
	{
		public:
			CImageBox(PyObject * ppyObject);
			virtual ~CImageBox();

			BOOL LoadImage(const char * c_szFileName);
			void SetDiffuseColor(float fr, float fg, float fb, float fa);
			void SetScale(float sx, float sy);

			int32_t GetWidth();
			int32_t GetHeight();

		protected:
			virtual void OnCreateInstance();
			virtual void OnDestroyInstance();

			virtual void OnUpdate();
			virtual void OnRender();
			void OnChangePosition();

		protected:
			CGraphicImageInstance * m_pImageInstance;
	};	
	class CMarkBox : public CWindow
	{
		public:
			CMarkBox(PyObject * ppyObject);
			virtual ~CMarkBox();

			void LoadImage(const char * c_szFilename);
			void SetDiffuseColor(float fr, float fg, float fb, float fa);
			void SetIndex(uint32_t uIndex);
			void SetScale(float fScale);

		protected:
			virtual void OnCreateInstance();
			virtual void OnDestroyInstance();

			virtual void OnUpdate();
			virtual void OnRender();
			void OnChangePosition();
		protected:
			CGraphicMarkInstance * m_pMarkInstance;
	};
	class CExpandedImageBox : public CImageBox
	{
		public:
			static uint32_t Type();

		public:
			CExpandedImageBox(PyObject * ppyObject);
			virtual ~CExpandedImageBox();

			void SetScale(float fx, float fy);
			void SetOrigin(float fx, float fy);
			void SetRotation(float fRotation);
			void SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
			void SetRenderingMode(int32_t iMode);
			D3DXCOLOR GetPixelColor(int32_t x, int32_t y) { if (m_pImageInstance) return m_pImageInstance->GetPixelColor(x, y); else return D3DXCOLOR(0, 0, 0, 0); }

		protected:
			void OnCreateInstance();
			void OnDestroyInstance();

			virtual void OnUpdate();
			virtual void OnRender();

			BOOL OnIsType(uint32_t dwType);
	};
	class CAniImageBox : public CWindow
	{
		public:
			static uint32_t Type();

		public:
			CAniImageBox(PyObject * ppyObject);
			virtual ~CAniImageBox();

			void SetDelay(int32_t iDelay);
			void AppendImageScale(const char * c_szFileName, float scale_x, float scale_y);
#ifdef ENABLE_ACCE_SYSTEM
			void	AppendImage(const char * c_szFileName, float r = 1.0, float g = 1.0, float b = 1.0, float a = 1.0);
#else
			void	AppendImage(const char * c_szFileName);
#endif
			void SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
			void SetRenderingMode(int32_t iMode);

			void ResetFrame();

		protected:
			void OnUpdate();
			void OnRender();
			void OnChangePosition();
			virtual void OnEndFrame();

			BOOL OnIsType(uint32_t dwType);

		protected:
			uint8_t m_bycurDelay;
			uint8_t m_byDelay;
			uint8_t m_bycurIndex;
			std::vector<CGraphicExpandedImageInstance*> m_ImageVector;
	};

	// Button
	class CButton : public CWindow
	{
		public:
			CButton(PyObject * ppyObject);
			virtual ~CButton();

			BOOL SetUpVisual(const char * c_szFileName);
			BOOL SetOverVisual(const char * c_szFileName);
			BOOL SetDownVisual(const char * c_szFileName);
			BOOL SetDisableVisual(const char * c_szFileName);

			const char * GetUpVisualFileName();
			const char * GetOverVisualFileName();
			const char * GetDownVisualFileName();

			void Flash();
			void FlashEx();
			void Enable();
			void Disable();

			void SetUp();
			void Up();
			void Over();
			void Down();

			BOOL IsDisable();
			BOOL IsPressed();

		protected:
			void OnUpdate();
			void OnRender();
			void OnChangePosition();

			BOOL OnMouseLeftButtonDown();
			BOOL OnMouseLeftButtonDoubleClick();
			BOOL OnMouseLeftButtonUp();
			void OnMouseOverIn();
			void OnMouseOverOut();

			BOOL IsEnable();

			void SetCurrentVisual(CGraphicImageInstance * pVisual);

		protected:
			BOOL m_bEnable;
			BOOL m_isPressed;
			BOOL m_isFlash;
			BOOL m_isFlashEx;
			CGraphicImageInstance * m_pcurVisual;
			CGraphicImageInstance m_upVisual;
			CGraphicImageInstance m_overVisual;
			CGraphicImageInstance m_downVisual;
			CGraphicImageInstance m_disableVisual;
	};
	class CRadioButton : public CButton
	{
		public:
			CRadioButton(PyObject * ppyObject);
			virtual ~CRadioButton();

		protected:
			BOOL OnMouseLeftButtonDown();
			BOOL OnMouseLeftButtonUp();
			void OnMouseOverIn();
			void OnMouseOverOut();
	};
	class CToggleButton : public CButton
	{
		public:
			CToggleButton(PyObject * ppyObject);
			virtual ~CToggleButton();

		protected:
			BOOL OnMouseLeftButtonDown();
			BOOL OnMouseLeftButtonUp();
			void OnMouseOverIn();
			void OnMouseOverOut();
	};
	class CDragButton : public CButton
	{
		public:
			CDragButton(PyObject * ppyObject);
			virtual ~CDragButton();

			void SetRestrictMovementArea(int32_t ix, int32_t iy, int32_t iwidth, int32_t iheight);

		protected:
			void OnChangePosition();
			void OnMouseOverIn();
			void OnMouseOverOut();

		protected:
			RECT m_restrictArea;
	};
};

extern BOOL g_bOutlineBoxEnable;
