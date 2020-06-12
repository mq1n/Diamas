#pragma once

#include "MSWindow.h"

class CMSApplication : public CMSWindow
{
	public:
		CMSApplication();		
		virtual ~CMSApplication();

		void Initialize(HINSTANCE hInstance);

		void MessageLoop();

		bool IsMessage();
		bool MessageProcess();

	protected:
		LRESULT WindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
};
