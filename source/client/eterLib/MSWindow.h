#pragma once

#include "../eterBase/Stl.h"

class CMSWindow
{
	public:
		CMSWindow();
		
		virtual ~CMSWindow();
		
		void Destroy();
		bool Create(const char* c_szName, int32_t brush=BLACK_BRUSH, uint32_t cs=0, uint32_t ws=WS_OVERLAPPEDWINDOW, HICON hIcon=nullptr, int32_t iCursorResource=32512);
		
		void Show();
		void Hide();

		void SetVisibleMode(bool isVisible);

		void SetPosition(int32_t x, int32_t y);
		void SetCenterPosition();

		void SetText(const char* c_szText);

		void AdjustSize(int32_t width, int32_t height);
		void SetSize(int32_t width, int32_t height);

		bool IsVisible();
		bool IsActive();

		void GetMousePosition(POINT* ppt);
		void GetClientRect(RECT* prc);
		void GetWindowRect(RECT* prc);

		int32_t	GetScreenWidth();
		int32_t	GetScreenHeight();

		HWND GetWindowHandle();
		HINSTANCE GetInstance();

		virtual LRESULT	WindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
		virtual void	OnSize(WPARAM wParam, LPARAM lParam);
		
		LPSTR GetMyClassName();
	protected:
		const char* RegisterWindowClass(uint32_t style, int32_t brush, WNDPROC pfnWndProc, HICON hIcon=nullptr, int32_t iCursorResource=32512);

	protected:
		typedef std::set<char*, stl_sz_less> TWindowClassSet;
		
	protected:
		HWND m_hWnd;
		RECT m_rect;
		bool m_isActive;
		bool m_isVisible;
		
	protected:
		static TWindowClassSet ms_stWCSet;
		static HINSTANCE ms_hInstance;
};
