#pragma once

#include "DibBar.h"

class CTextBar : public CDibBar
{
	public:
		CTextBar(int32_t fontSize, bool isBold);
		virtual ~CTextBar();
		
		void TextOut(int32_t ix, int32_t iy, const char * c_szText);
		void SetTextColor(int32_t r, int32_t g, int32_t b);
		void GetTextExtent(const char * c_szText, SIZE* p_size);

	protected:
		void __SetFont(int32_t fontSize, bool isBold);

		void OnCreate();

	protected:
		HFONT m_hFont;
		HFONT m_hOldFont;
		
		int32_t		m_fontSize;
		bool	m_isBold;
};
