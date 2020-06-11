#pragma once

#include "GrpDib.h"

class CBlockTexture;

class CDibBar
{
	public:
		CDibBar();
		virtual ~CDibBar();

		bool Create(HDC hdc, uint32_t dwWidth, uint32_t dwHeight);
		void Invalidate();
		void SetClipRect(const RECT & c_rRect);
		void ClearBar();
		void Render(int32_t ix, int32_t iy);

	protected:
		uint32_t __NearTextureSize(uint32_t dwSize);
		void __DivideTextureSize(uint32_t dwSize, uint32_t dwMax, uint32_t * pdwxStep, uint32_t * pdwxCount, uint32_t * pdwxRest);
		CBlockTexture * __BuildTextureBlock(uint32_t dwxPos, uint32_t dwyPos, uint32_t dwImageWidth, uint32_t dwImageHeight, uint32_t dwTextureWidth, uint32_t dwTextureHeight);
		void __BuildTextureBlockList(uint32_t dwWidth, uint32_t dwHeight, uint32_t dwMax=256);

		virtual void OnCreate(){}

	protected:
		CGraphicDib m_dib;
		std::vector<CBlockTexture *> m_kVec_pkBlockTexture;

		uint32_t m_dwWidth;
		uint32_t m_dwHeight;
};
