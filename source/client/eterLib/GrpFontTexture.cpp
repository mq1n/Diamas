#include "StdAfx.h"
#include "GrpText.h"
#include "../eterBase/Stl.h"

#include "Util.h"

CGraphicFontTexture::CGraphicFontTexture()
{
	Initialize();
}

CGraphicFontTexture::~CGraphicFontTexture()
{
	Destroy();
}

void CGraphicFontTexture::Initialize()
{
	CGraphicTexture::Initialize();
	m_hFontOld = nullptr;
	m_hFont = nullptr;
	m_isDirty = false;
	m_bItalic = false;
}

bool CGraphicFontTexture::IsEmpty() const
{
	return m_fontMap.size() == 0;
}

void CGraphicFontTexture::Destroy()
{
	HDC hDC = m_dib.GetDCHandle();
	if (hDC)
		SelectObject(hDC, m_hFontOld);

	m_dib.Destroy();

	m_lpd3dTexture = nullptr;
	CGraphicTexture::Destroy();
	stl_wipe(m_pFontTextureVector);
	m_charInfoMap.clear();

	if (m_fontMap.size())
	{
		TFontMap::iterator i = m_fontMap.begin();

		while(i != m_fontMap.end())
		{
			DeleteObject((HGDIOBJ)i->second);
			++i;
		}

		m_fontMap.clear();
	}
	
	Initialize();
}

bool CGraphicFontTexture::CreateDeviceObjects()
{
	return true;
}

void CGraphicFontTexture::DestroyDeviceObjects()
{
}

bool CGraphicFontTexture::Create(const char* c_szFontName, int32_t fontSize, bool bItalic)
{
	Destroy();
	
	strncpy(m_fontName, c_szFontName, sizeof(m_fontName)-1);
	m_fontSize	= fontSize;
	m_bItalic	= bItalic;

	m_x = 0;
	m_y = 0;
	m_step = 0;

	uint32_t width = 256,height = 256;
	if (GetMaxTextureWidth() > 512)
		width = 512;
	if (GetMaxTextureHeight() > 512)
		height = 512;
	
	if (!m_dib.Create(ms_hDC, width, height))
		return false;

	HDC hDC = m_dib.GetDCHandle();

	m_hFont = GetFont(GetDefaultCodePage());

	m_hFontOld=(HFONT)SelectObject(hDC, m_hFont);
	SetTextColor(hDC, RGB(255, 255, 255));
	SetBkColor(hDC,	0);

	if (!AppendTexture())
		return false;
	
	return true;
}



HFONT CGraphicFontTexture::GetFont(uint16_t codePage)
{
	HFONT hFont = nullptr;
	TFontMap::iterator i = m_fontMap.find(codePage);

	if(i != m_fontMap.end())
	{
		hFont = i->second;
	}
	else
	{
		LOGFONT logFont;

		memset(&logFont, 0, sizeof(LOGFONT));

		logFont.lfHeight			= m_fontSize;
		logFont.lfEscapement		= 0;
		logFont.lfOrientation		= 0;
		logFont.lfWeight			= FW_NORMAL;
		logFont.lfItalic			= (uint8_t) m_bItalic;
		logFont.lfUnderline			= FALSE;
		logFont.lfStrikeOut			= FALSE;
		logFont.lfCharSet			= GetCharsetFromCodePage(codePage);
		logFont.lfOutPrecision		= OUT_DEFAULT_PRECIS;
		logFont.lfClipPrecision		= CLIP_DEFAULT_PRECIS;
		logFont.lfQuality			= ANTIALIASED_QUALITY;
		logFont.lfPitchAndFamily	= DEFAULT_PITCH;
		//Tracenf("font: %s", GetFontFaceFromCodePage(codePage));
		strcpy(logFont.lfFaceName, m_fontName); //GetFontFaceFromCodePage(codePage));
		//strcpy(logFont.lfFaceName, GetFontFaceFromCodePage(codePage));

		hFont = CreateFontIndirect(&logFont);

		m_fontMap.insert(TFontMap::value_type(codePage, hFont));
	}

	return hFont;
}

bool CGraphicFontTexture::AppendTexture()
{
	CGraphicImageTexture * pNewTexture = new CGraphicImageTexture;

	if (!pNewTexture->Create(m_dib.GetWidth(), m_dib.GetHeight(), D3DFMT_A4R4G4B4))
	{
		delete pNewTexture;
		return false;
	}

	m_pFontTextureVector.push_back(pNewTexture);
	return true;
}

bool CGraphicFontTexture::UpdateTexture()
{
	if(!m_isDirty)
		return true;

	m_isDirty = false;

	CGraphicImageTexture * pFontTexture = m_pFontTextureVector.back();

	if (!pFontTexture)
		return false;

	uint16_t* pwDst;
	int32_t pitch;

	if (!pFontTexture->Lock(&pitch, (void**)&pwDst))
		return false;

	pitch /= 2;

	int32_t width = m_dib.GetWidth();
	int32_t height = m_dib.GetHeight();

	uint32_t * pdwSrc = (uint32_t*)m_dib.GetPointer();

	for (int32_t y = 0; y < height; ++y, pwDst += pitch, pdwSrc += width)
		for (int32_t x = 0; x < width; ++x)
			pwDst[x]=pdwSrc[x];
	
	pFontTexture->Unlock();
	return true;
}

CGraphicFontTexture::TCharacterInfomation* CGraphicFontTexture::GetCharacterInfomation(uint16_t codePage, wchar_t keyValue)
{
	TCharacterKey code(codePage, keyValue);

	TCharacterInfomationMap::iterator f = m_charInfoMap.find(code);

	if (m_charInfoMap.end() == f)
	{
		return UpdateCharacterInfomation(code);
	}
	else
	{
		return &f->second;	
	}
}

CGraphicFontTexture::TCharacterInfomation* CGraphicFontTexture::UpdateCharacterInfomation(TCharacterKey code)
{
	HDC hDC = m_dib.GetDCHandle();
	SelectObject(hDC, GetFont(code.first));

	wchar_t keyValue = code.second;

	if (keyValue == 0x08)
		keyValue = L' '; // ÅÇÀº °ø¹éÀ¸·Î ¹Ù²Û´Ù (¾Æ¶ø Ãâ·Â½Ã ÅÇ »ç¿ë: NAME:\tTEXT -> TEXT\t:NAME ·Î ÀüÈ¯µÊ )

	ABCFLOAT	stABC;
	SIZE		size;

	if (!GetTextExtentPoint32W(hDC, &keyValue, 1, &size) || !GetCharABCWidthsFloatW(hDC, keyValue, keyValue, &stABC))
		return nullptr;

	size.cx = stABC.abcfB;
	if( stABC.abcfA > 0.0f )
		size.cx += ceilf(stABC.abcfA);
	if( stABC.abcfC > 0.0f )
		size.cx += ceilf(stABC.abcfC);
	size.cx++;

	int32_t lAdvance = ceilf( stABC.abcfA + stABC.abcfB + stABC.abcfC );

	int32_t width = m_dib.GetWidth();
	int32_t height = m_dib.GetHeight();

	if (m_x + size.cx >= (width - 1))
	{
		m_y += (m_step + 1);
		m_step = 0;
		m_x = 0;

		if (m_y + size.cy >= (height - 1))
		{
			if (!UpdateTexture())
			{
				return nullptr;
			}

			if (!AppendTexture())
				return nullptr;

			m_y = 0;
		}
	}

	TextOutW(hDC, m_x, m_y, &keyValue, 1);
		
	int32_t nChrX;
	int32_t nChrY;
	int32_t nChrWidth = size.cx;
	int32_t nChrHeight = size.cy;
	int32_t nDIBWidth = m_dib.GetWidth();

	
	uint32_t*pdwDIBData=(uint32_t*)m_dib.GetPointer();		
	uint32_t*pdwDIBBase=pdwDIBData+nDIBWidth*m_y+m_x;
	uint32_t*pdwDIBRow;
	
	pdwDIBRow=pdwDIBBase;
	for (nChrY=0; nChrY<nChrHeight; ++nChrY, pdwDIBRow+=nDIBWidth)
	{			
		for (nChrX=0; nChrX<nChrWidth; ++nChrX)
		{
			pdwDIBRow[nChrX]=(pdwDIBRow[nChrX]&0xff) ? 0xffff : 0;
		}
	}

	float rhwidth = 1.0f / float(width);
	float rhheight = 1.0f / float(height);

	TCharacterInfomation& rNewCharInfo = m_charInfoMap[code];

	rNewCharInfo.index = m_pFontTextureVector.size() - 1;
	rNewCharInfo.width = size.cx;
	rNewCharInfo.height = size.cy;
	rNewCharInfo.left = float(m_x) * rhwidth;
	rNewCharInfo.top = float(m_y) * rhheight;
	rNewCharInfo.right = float(m_x+size.cx) * rhwidth;
	rNewCharInfo.bottom = float(m_y+size.cy) * rhheight;
	rNewCharInfo.advance = (float) lAdvance;

	m_x += size.cx;

	if (m_step < size.cy)
		m_step = size.cy;	

	m_isDirty = true;

	return &rNewCharInfo;
}

bool CGraphicFontTexture::CheckTextureIndex(uint32_t dwTexture)
{
	if (dwTexture >= m_pFontTextureVector.size())
		return false;

	return true;
}

void CGraphicFontTexture::SelectTexture(uint32_t dwTexture)
{
	assert(CheckTextureIndex(dwTexture));
	m_lpd3dTexture = m_pFontTextureVector[dwTexture]->GetD3DTexture();
}
