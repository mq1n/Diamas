#include "StdAfx.h"
#include "PythonApplication.h"
#include "../eterlib/Camera.h"
#include "../eterWebBrowser/CWebBrowser.h"

#include <winuser.h>

static int32_t gs_nMouseCaptureRef = 0;

void CPythonApplication::SafeSetCapture()
{
	SetCapture(m_hWnd);
	gs_nMouseCaptureRef++;
}

void CPythonApplication::SafeReleaseCapture()
{
	gs_nMouseCaptureRef--;
	if (gs_nMouseCaptureRef==0)
		ReleaseCapture();
}

void CPythonApplication::__SetFullScreenWindow(HWND hWnd, uint32_t dwWidth, uint32_t dwHeight, uint32_t dwBPP)
{
	DEVMODE DevMode;
	DevMode.dmSize = sizeof(DevMode);
	DevMode.dmBitsPerPel = dwBPP;
	DevMode.dmPelsWidth = dwWidth;
	DevMode.dmPelsHeight = dwHeight;
	DevMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	int32_t Error = ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN);
	if(Error == DISP_CHANGE_RESTART)
	{
		ChangeDisplaySettings(0,0);
	}
}

void CPythonApplication::__MinimizeFullScreenWindow(HWND hWnd, uint32_t dwWidth, uint32_t dwHeight)
{
	ChangeDisplaySettings(0, 0);
	SetWindowPos(hWnd, 0, 0, 0,
				 dwWidth,
				 dwHeight,
				 SWP_SHOWWINDOW);
	ShowWindow(hWnd, SW_MINIMIZE);
}

void CPythonApplication::__ResetCameraWhenMinimize()
{
	CCameraManager& rkCmrMgr=CCameraManager::Instance();
	CCamera* pkCmrCur=rkCmrMgr.GetCurrentCamera();
	if (pkCmrCur) 
	{
		pkCmrCur->EndDrag();
	}
	
	SetCursorNum(NORMAL);
	if ( CURSOR_MODE_HARDWARE == GetCursorMode())
		SetCursorVisible(TRUE);
}

LRESULT CPythonApplication::WindowProcedure(HWND hWnd, uint32_t uiMsg, WPARAM wParam, LPARAM lParam)
{
	const int32_t c_DoubleClickTime = 300;
	const int32_t c_DoubleClickBox = 5;
	static int32_t s_xDownPosition = 0;
	static int32_t s_yDownPosition = 0;	

	switch (uiMsg)
	{
		case WM_ACTIVATEAPP:
			{
				m_isActivateWnd = (wParam == WA_ACTIVE) || (wParam == WA_CLICKACTIVE);

				if (m_isActivateWnd)
				{
					m_SoundManager.RestoreVolume();

					//////////////////

					if (m_isWindowFullScreenEnable)
					{
						__MinimizeFullScreenWindow(hWnd, m_dwWidth, m_dwHeight);
						__ResetCameraWhenMinimize();
					}
					else
					{
						__ResetCameraWhenMinimize();
					}
				}
				else
				{
					m_SoundManager.SaveVolume();

					//////////////////

					if (m_isWindowFullScreenEnable)
					{
						__MinimizeFullScreenWindow(hWnd, m_dwWidth, m_dwHeight);
					}
				}
			}
			break;

		case WM_INPUTLANGCHANGE:
			return CPythonIME::Instance().WMInputLanguage(hWnd, uiMsg, wParam, lParam);
			break;

		case WM_IME_STARTCOMPOSITION:
			return CPythonIME::Instance().WMStartComposition(hWnd, uiMsg, wParam, lParam);
			break;

		case WM_IME_COMPOSITION:
			return CPythonIME::Instance().WMComposition(hWnd, uiMsg, wParam, lParam);
			break;

		case WM_IME_ENDCOMPOSITION:
			return CPythonIME::Instance().WMEndComposition(hWnd, uiMsg, wParam, lParam);
			break;

		case WM_IME_NOTIFY:
			return CPythonIME::Instance().WMNotify(hWnd, uiMsg, wParam, lParam);
			break;

		case WM_IME_SETCONTEXT:
			lParam &= ~(ISC_SHOWUICOMPOSITIONWINDOW | ISC_SHOWUIALLCANDIDATEWINDOW);
			break;

		case WM_CHAR:
			return CPythonIME::Instance().WMChar(hWnd, uiMsg, wParam, lParam);
			break;

		case WM_KEYDOWN:
			OnIMEKeyDown(LOWORD(wParam));
			break;

		case WM_LBUTTONDOWN:
			SafeSetCapture();

			if (ELTimer_GetMSec() - m_dwLButtonDownTime < c_DoubleClickTime &&
				abs(LOWORD(lParam) - s_xDownPosition) < c_DoubleClickBox &&
				abs(HIWORD(lParam) - s_yDownPosition) < c_DoubleClickBox)
			{
				m_dwLButtonDownTime = 0;

				OnMouseLeftButtonDoubleClick(int16_t(LOWORD(lParam)), int16_t(HIWORD(lParam)));
			}
			else
			{
				m_dwLButtonDownTime = ELTimer_GetMSec();

				OnMouseLeftButtonDown(int16_t(LOWORD(lParam)), int16_t(HIWORD(lParam)));
			}

			s_xDownPosition = LOWORD(lParam);
			s_yDownPosition = HIWORD(lParam);
			return 0;

		case WM_LBUTTONUP:
			m_dwLButtonUpTime = ELTimer_GetMSec();

			if (hWnd == GetCapture())
			{
				SafeReleaseCapture();
				OnMouseLeftButtonUp(int16_t(LOWORD(lParam)), int16_t(HIWORD(lParam)));
			}
			return 0;

		case WM_MBUTTONDOWN:
			SafeSetCapture();

			UI::CWindowManager::Instance().RunMouseMiddleButtonDown(int16_t(LOWORD(lParam)), int16_t(HIWORD(lParam)));
//			OnMouseMiddleButtonDown(int16_t(LOWORD(lParam)), int16_t(HIWORD(lParam)));
			break;

		case WM_MBUTTONUP:
			if (GetCapture() == hWnd)
			{
				SafeReleaseCapture();

				UI::CWindowManager::Instance().RunMouseMiddleButtonUp(int16_t(LOWORD(lParam)), int16_t(HIWORD(lParam)));
//				OnMouseMiddleButtonUp(int16_t(LOWORD(lParam)), int16_t(HIWORD(lParam)));
			}
			break;

		case WM_RBUTTONDOWN:
			SafeSetCapture();
			OnMouseRightButtonDown(int16_t(LOWORD(lParam)), int16_t(HIWORD(lParam)));
			return 0;

		case WM_RBUTTONUP:
			if (hWnd == GetCapture()) 
			{
				SafeReleaseCapture();

				OnMouseRightButtonUp(int16_t(LOWORD(lParam)), int16_t(HIWORD(lParam)));
			}
			return 0;



		case WM_MOUSEWHEEL:
			if (WebBrowser_IsVisible())
			{
				// 웹브라우저 상태일때는 휠 작동 안되도록 처리
			}
			else
			{
#ifdef ENABLE_MOUSEWHEEL_EVENT
				
				int32_t  xPos		= GET_X_LPARAM(lParam);
				int32_t  yPos		= GET_Y_LPARAM(lParam);
				int16_t zDelta	= GET_WHEEL_DELTA_WPARAM(wParam);

				if(OnMouseWheelScroll(xPos, yPos, zDelta))
					break;
				
#endif

				OnMouseWheel(int16_t(HIWORD(wParam)));
			}
			break;

		case WM_SIZE:
			switch (wParam)
			{
				case SIZE_RESTORED:
				case SIZE_MAXIMIZED:
					{
						RECT rcWnd; 
						GetClientRect(&rcWnd); 
				
						uint32_t uWidth=rcWnd.right-rcWnd.left; 
						uint32_t uHeight=rcWnd.bottom-rcWnd.left; 
						m_grpDevice.ResizeBackBuffer(uWidth, uHeight);				
					}
					break;
			}

			if (wParam==SIZE_MINIMIZED)
				m_isMinimizedWnd=true;
			else
				m_isMinimizedWnd=false;

			OnSizeChange(int16_t(LOWORD(lParam)), int16_t(HIWORD(lParam)));

			break;

		case WM_EXITSIZEMOVE:    
			{
				RECT rcWnd; 
				GetClientRect(&rcWnd); 
				
				uint32_t uWidth=rcWnd.right-rcWnd.left; 
				uint32_t uHeight=rcWnd.bottom-rcWnd.left; 
				m_grpDevice.ResizeBackBuffer(uWidth, uHeight);				
				OnSizeChange(int16_t(LOWORD(lParam)), int16_t(HIWORD(lParam)));
			}
			break; 

		case WM_SYSKEYDOWN:
			switch (LOWORD(wParam))
			{
				case VK_F10:
					break;
			}
			break;

		case WM_SYSKEYUP:
			switch(LOWORD(wParam))
			{
				case 18:
					return FALSE;
					break;
				case VK_F10:
					break;
			}
			break;

		case WM_SETCURSOR:
			if (IsActive())
			{
				if (m_bCursorVisible && CURSOR_MODE_HARDWARE == m_iCursorMode)
				{
					SetCursor((HCURSOR) m_hCurrentCursor);
					return 0;
				}
				else
				{
					SetCursor(nullptr);
					return 0;
				}
			}
			break;

		case WM_CLOSE:
#ifdef _DEBUG
			PostQuitMessage(0);
#else	
			RunPressExitKey();
#endif
			return 0;

		case WM_DESTROY:
			return 0;
		default:
			//Tracenf("%x msg %x", timeGetTime(), uiMsg);
			break;
	}	

	return CMSApplication::WindowProcedure(hWnd, uiMsg, wParam, lParam);
}
