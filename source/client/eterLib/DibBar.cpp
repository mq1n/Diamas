#include "StdAfx.h"
#include "DibBar.h"
#include "BlockTexture.h"

void CDibBar::Invalidate()
{
	RECT rect = {0, 0, m_dwWidth, m_dwHeight};

	std::vector<CBlockTexture *>::iterator itor = m_kVec_pkBlockTexture.begin();
	for (; itor != m_kVec_pkBlockTexture.end(); ++itor)
	{
		CBlockTexture * pTexture = *itor;
		pTexture->InvalidateRect(rect);
	}
}

void CDibBar::SetClipRect(const RECT & c_rRect)
{
	std::vector<CBlockTexture *>::iterator itor = m_kVec_pkBlockTexture.begin();
	for (; itor != m_kVec_pkBlockTexture.end(); ++itor)
	{
		CBlockTexture * pTexture = *itor;
		assert(pTexture);
		if (!pTexture)
			continue;
		pTexture->SetClipRect(c_rRect);
	}
}

void CDibBar::ClearBar()
{
	uint32_t  * pdwBuf = (uint32_t *)m_dib.GetPointer();
	memset(pdwBuf, 0, m_dib.GetWidth()*m_dib.GetHeight()*4);
	Invalidate();
}

void CDibBar::Render(int32_t ix, int32_t iy)
{
	std::vector<CBlockTexture *>::iterator itor = m_kVec_pkBlockTexture.begin();
	for (; itor != m_kVec_pkBlockTexture.end(); ++itor)
	{
		CBlockTexture * pTexture = *itor;
		pTexture->Render(ix, iy);
	}
}

uint32_t CDibBar::__NearTextureSize(uint32_t dwSize)
{
	if ((dwSize & (dwSize-1)) == 0)
		return dwSize;

	uint32_t dwRet = 2;
	while (dwRet < dwSize)
	{
		dwRet <<= 1;
	}

	return dwRet;
}

void CDibBar::__DivideTextureSize(uint32_t dwSize, uint32_t dwMax, uint32_t * pdwxStep, uint32_t * pdwxCount, uint32_t * pdwxRest)
{
	if (dwSize<dwMax)
	{
		*pdwxStep = dwMax;
		*pdwxCount = 0;
		*pdwxRest = dwSize%dwMax;
		return;
	}

	*pdwxStep = dwMax;
	*pdwxCount = dwSize/dwMax;
	*pdwxRest = dwSize%dwMax;
}

CBlockTexture * CDibBar::__BuildTextureBlock(uint32_t dwxPos, uint32_t dwyPos, uint32_t dwImageWidth, uint32_t dwImageHeight, uint32_t dwTextureWidth, uint32_t dwTextureHeight)
{
	if (dwTextureWidth == 0 || dwTextureHeight == 0)
		return nullptr;

	RECT posRect = {dwxPos, dwyPos, dwxPos+dwImageWidth, dwyPos+dwImageHeight};

	CBlockTexture * pBlockTexture = new CBlockTexture;
	if (!pBlockTexture->Create(&m_dib, posRect, dwTextureWidth, dwTextureHeight))
	{
		delete pBlockTexture;
		return nullptr;
	}

	return pBlockTexture;
}

void CDibBar::__BuildTextureBlockList(uint32_t dwWidth, uint32_t dwHeight, uint32_t dwMax)
{
	uint32_t dwxStep, dwyStep;
	uint32_t dwxCount, dwyCount;
	uint32_t dwxRest, dwyRest;
	__DivideTextureSize(dwWidth, dwMax, &dwxStep, &dwxCount, &dwxRest);
	__DivideTextureSize(dwHeight, dwMax, &dwyStep, &dwyCount, &dwyRest);
	uint32_t dwxTexRest = __NearTextureSize(dwxRest);
	uint32_t dwyTexRest = __NearTextureSize(dwyRest);

	for (uint32_t y = 0; y < dwyCount; ++y)
	{
		for (uint32_t x = 0; x < dwxCount; ++x)
		{
			CBlockTexture * pTexture = __BuildTextureBlock(x*dwxStep, y*dwyStep,
														   dwxStep, dwyStep,
														   dwMax, dwMax);
			if (pTexture)
				m_kVec_pkBlockTexture.push_back(pTexture);
		}
		
		CBlockTexture * pTexture = __BuildTextureBlock(dwxCount*dwxStep, y*dwyStep,
													   dwxRest, dwyStep,
													   dwxTexRest, dwMax);

		if (pTexture)
			m_kVec_pkBlockTexture.push_back(pTexture);
	}

	for (uint32_t x = 0; x < dwxCount; ++x)
	{
		CBlockTexture * pTexture = __BuildTextureBlock(x*dwxStep, dwyCount*dwyStep,
													   dwxStep, dwyRest,
													   dwMax, dwyTexRest);
		if (pTexture)
			m_kVec_pkBlockTexture.push_back(pTexture);
	}

	if (dwxRest > 0 && dwyRest > 0)
	{
		CBlockTexture * pTexture = __BuildTextureBlock(dwxCount*dwxStep, dwyCount*dwyStep,
													   dwxRest, dwyRest,
													   dwxTexRest, dwyTexRest);
		if (pTexture)
			m_kVec_pkBlockTexture.push_back(pTexture);
	}
}

bool CDibBar::Create(HDC hdc, uint32_t dwWidth, uint32_t dwHeight)
{
	if (!m_dib.Create(hdc, dwWidth, dwHeight))
	{
		Tracef(" Failed to create CDibBar\n");
		return false;
	}

	m_dwWidth = dwWidth;
	m_dwHeight = dwHeight;

	__BuildTextureBlockList(dwWidth, dwHeight);
	OnCreate();

	return true;
}

CDibBar::CDibBar(): m_dwWidth(0), m_dwHeight(0)
{
}
CDibBar::~CDibBar()
{
}
