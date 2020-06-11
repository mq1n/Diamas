#pragma once

#include "GrpBase.h"

class CGraphicDib;

class CBlockTexture : public CGraphicBase
{
	public:
		CBlockTexture();
		virtual ~CBlockTexture();

		bool Create(CGraphicDib * pDIB, const RECT & c_rRect, uint32_t dwWidth, uint32_t dwHeight);
		void SetClipRect(const RECT & c_rRect);
		void Render(int32_t ix, int32_t iy);
		void InvalidateRect(const RECT & c_rsrcRect);

	protected:
		CGraphicDib * m_pDIB;
		RECT m_rect;
		RECT m_clipRect;
		BOOL m_bClipEnable;
		uint32_t m_dwWidth;
		uint32_t m_dwHeight;
		LPDIRECT3DTEXTURE9 m_lpd3dTexture;
};
