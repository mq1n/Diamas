#pragma once

class CGraphicDib 
{
	public:
		CGraphicDib();
		virtual ~CGraphicDib();

		void Destroy();	
		bool Create(HDC hDC, int32_t width, int32_t height);

		void SetBkMode(int32_t iBkMode);
		void TextOut(int32_t ix, int32_t iy, const char * c_szText);
		void Put(HDC hDC, int32_t x, int32_t y);

		int32_t GetWidth();
		int32_t GetHeight();

		void* GetPointer();

		HDC GetDCHandle();
		
	protected:
		void Initialize();

	protected:		
		HDC			m_hDC;
		HBITMAP		m_hBmp;
		BITMAPINFO	m_bmi;

		int32_t			m_width;
		int32_t			m_height;

		void *		m_pvBuf;
};
