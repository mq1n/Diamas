#include "StdAfx.h"
#include "GrpTextInstance.h"
#include "StateManager.h"
#include "IME.h"
#include "TextTag.h"
#include "../eterBase/Utils.h"
#include "ResourceManager.h"
#include <stdio.h>
#include <cstdint>

const float c_fFontFeather = 0.5f;

CDynamicPool<CGraphicTextInstance>		CGraphicTextInstance::ms_kPool;

static int32_t gs_mx = 0;
static int32_t gs_my = 0;

static std::wstring gs_hyperlinkText;

void CGraphicTextInstance::Hyperlink_UpdateMousePos(int32_t x, int32_t y)
{
	gs_mx = x;
	gs_my = y;
	gs_hyperlinkText.clear();
}

int32_t CGraphicTextInstance::Hyperlink_GetText(char* buf, int32_t len)
{
	if (gs_hyperlinkText.empty())
		return 0;

	int32_t codePage = 1254; CP_UTF8;

	return WideCharToMultiByte(codePage, 0, gs_hyperlinkText.c_str(), gs_hyperlinkText.length(), buf, len, nullptr, nullptr);	
}

int32_t CGraphicTextInstance::__DrawCharacter(CGraphicFontTexture * pFontTexture, wchar_t text, uint32_t dwColor)
{
	CGraphicFontTexture::TCharacterInformation* pInsCharInfo = pFontTexture->GetCharacterInformation(text);

	if (pInsCharInfo)
	{
		m_dwColorInfoVector.emplace_back(dwColor);
		m_pCharInfoVector.emplace_back(pInsCharInfo);

		m_textWidth += pInsCharInfo->advance;
		m_textHeight = std::max<uint16_t>(pInsCharInfo->height, m_textHeight);
		return pInsCharInfo->advance;
	}
	
	return 0;
}

void CGraphicTextInstance::__GetTextPos(uint32_t index, float* x, float* y)
{
	index = std::min<uint32_t>(index, m_pCharInfoVector.size());

	float sx = 0;
	float sy = 0;
	float fFontMaxHeight = 0;

	for(uint32_t i=0; i<index; ++i)
	{
		if (sx+float(m_pCharInfoVector[i]->width) > m_fLimitWidth)
		{
			sx = 0;
			sy += fFontMaxHeight;
		}

		sx += float(m_pCharInfoVector[i]->advance);
		fFontMaxHeight = std::max(float(m_pCharInfoVector[i]->height), fFontMaxHeight);
	}

	*x = sx;
	*y = sy;
}

void CGraphicTextInstance::Update()
{
	if (m_isUpdate) // 문자열이 바뀌었을 때만 업데이트 한다.
		return;

	if (m_roText.IsNull())
	{
		Tracef("CGraphicTextInstance::Update - Font has not been set\n");
		return;
	}

	if (m_roText->IsEmpty())
		return;

	auto pFontTexture = m_roText->GetTexturePointer();
	if (!pFontTexture)
		return;

	CGraphicFontTexture::TCharacterInformation* pSpaceInfo = pFontTexture->GetCharacterInformation(' ');

	int32_t spaceHeight = pSpaceInfo ? pSpaceInfo->height : 12;
	
	m_pCharInfoVector.clear();
	m_dwColorInfoVector.clear();
	m_hyperlinkVector.clear();

	m_textWidth = 0;
	m_textHeight = spaceHeight;

	/* wstring begin */ 	

	const char* begin = m_stText.c_str();
	const char* end = begin + m_stText.length();

	int32_t wTextMax = (end - begin) * 2;
	wchar_t* wText = (wchar_t*)_alloca(sizeof(wchar_t)*wTextMax);

	uint32_t dwColor = m_dwTextColor;

	auto code_page = 1254; // CP_UTF8;
	int32_t wTextLen = MultiByteToWideChar(code_page, 0,
	                                   begin, end - begin,
	                                   wText, wTextMax);

	if (m_isSecret)
	{
		for(int32_t i=0; i<wTextLen; ++i)
			__DrawCharacter(pFontTexture, '*', dwColor);
	}
	else
	{
		int32_t x = 0;
		int32_t len;
		int32_t hyperlinkStep = 0;
		SHyperlink kHyperlink;
		std::wstring hyperlinkBuffer;

		for (int32_t i = 0; i < wTextLen; )
		{
			int32_t ret = GetTextTag(&wText[i], wTextLen - i, len, hyperlinkBuffer);

			if (ret == TEXT_TAG_PLAIN || ret == TEXT_TAG_TAG)
			{
				if (hyperlinkStep == 1)
					hyperlinkBuffer.append(1, wText[i]);
				else
				{
					int32_t charWidth = __DrawCharacter(pFontTexture, wText[i], dwColor);
					kHyperlink.ex += charWidth;
					x += charWidth;
				}
			}
			else
			{
				if (ret == TEXT_TAG_COLOR)
					dwColor = htoi(hyperlinkBuffer.c_str(), 8);
				else if (ret == TEXT_TAG_RESTORE_COLOR)
					dwColor = m_dwTextColor;
				else if (ret == TEXT_TAG_HYPERLINK_START)
				{
					hyperlinkStep = 1;
					hyperlinkBuffer.clear();
				}
				else if (ret == TEXT_TAG_HYPERLINK_END)
				{
					if (hyperlinkStep == 1)
					{
						++hyperlinkStep;
						kHyperlink.ex = kHyperlink.sx = x; // 실제 텍스트가 시작되는 위치
					}
					else
					{
						kHyperlink.text = hyperlinkBuffer;
								m_hyperlinkVector.emplace_back(kHyperlink);

						hyperlinkStep = 0;
						hyperlinkBuffer.clear();
					}
				}
			}
			i += len;
		}
	}

	pFontTexture->UpdateTexture();

	m_isUpdate = true;
}

void CGraphicTextInstance::Render(RECT * pClipRect)
{
	if (!m_isUpdate)
		return;	

	auto pkText=m_roText.GetPointer();
	if (!pkText)
		return;

	auto pFontTexture = pkText->GetTexturePointer();
	if (!pFontTexture)
		return;

	float fStanX = m_v3Position.x;
	float fStanY = m_v3Position.y + 1.0f;

	switch (m_hAlign)
	{
		case HORIZONTAL_ALIGN_RIGHT:
			fStanX -= m_textWidth;
			break;

		case HORIZONTAL_ALIGN_CENTER:
			fStanX -= float(m_textWidth / 2);
			break;
	}

	switch (m_vAlign)
	{
		case VERTICAL_ALIGN_BOTTOM:
			fStanY -= m_textHeight;
			break;

		case VERTICAL_ALIGN_CENTER:
			fStanY -= float(m_textHeight) / 2.0f;
			break;
	}

	//uint16_t FillRectIndices[6] = { 0, 2, 1, 2, 3, 1 };

	STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	uint32_t dwFogEnable = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);
	uint32_t dwLighting = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);

	{
		const float fFontHalfWeight=1.0f;

		float fCurX;
		float fCurY;

		float fFontSx;
		float fFontSy;
		float fFontEx;
		float fFontEy;
		float fFontWidth;
		float fFontHeight;
		float fFontMaxHeight;
		float fFontAdvance;

		SVertex akVertex[4];
		akVertex[0].z=m_v3Position.z;
		akVertex[1].z=m_v3Position.z;
		akVertex[2].z=m_v3Position.z;
		akVertex[3].z=m_v3Position.z;

		CGraphicFontTexture::TCharacterInformation* pCurCharInfo;

		// 테두리
		if (m_isOutline)
		{
			fCurX=fStanX;
			fCurY=fStanY;
			fFontMaxHeight=0.0f;

			CGraphicFontTexture::TPCharacterInformationVector::iterator i;
			for (i=m_pCharInfoVector.begin(); i!=m_pCharInfoVector.end(); ++i)
			{
				pCurCharInfo = *i;

				fFontWidth=float(pCurCharInfo->width);
				fFontHeight=float(pCurCharInfo->height);
				fFontAdvance=float(pCurCharInfo->advance);

				// NOTE : 폰트 출력에 Width 제한을 둡니다. - [levites]
				if ((fCurX+fFontWidth)-m_v3Position.x > m_fLimitWidth)
				{
					if (m_isMultiLine)
					{
						fCurX=fStanX;
						fCurY+=fFontMaxHeight;
					}
					else
					{
						break;
					}
				}

				if (pClipRect)
				{
					if (fCurY <= pClipRect->top)
					{
						fCurX += fFontAdvance;
						continue;
					}
				}

				fFontSx = fCurX - 0.5f + pCurCharInfo->prespacing;
				fFontSy = fCurY - 0.5f;
				fFontEx = fFontSx + fFontWidth;
				fFontEy = fFontSy + fFontHeight;

				pFontTexture->SelectTexture(pCurCharInfo->index);
				STATEMANAGER.SetTexture(0, pFontTexture->GetD3DTexture());

				akVertex[0].u=pCurCharInfo->left;
				akVertex[0].v=pCurCharInfo->top;
				akVertex[1].u=pCurCharInfo->left;
				akVertex[1].v=pCurCharInfo->bottom;
				akVertex[2].u=pCurCharInfo->right;
				akVertex[2].v=pCurCharInfo->top;
				akVertex[3].u=pCurCharInfo->right;
				akVertex[3].v=pCurCharInfo->bottom;

				akVertex[3].color = akVertex[2].color = akVertex[1].color = akVertex[0].color = m_dwOutLineColor;

				
				float feather = 0.0f; // m_fFontFeather
				
				akVertex[0].y=fFontSy-feather;
				akVertex[1].y=fFontEy+feather;
				akVertex[2].y=fFontSy-feather;
				akVertex[3].y=fFontEy+feather;

				// 왼
				akVertex[0].x=fFontSx-fFontHalfWeight-feather;
				akVertex[1].x=fFontSx-fFontHalfWeight-feather;
				akVertex[2].x=fFontEx-fFontHalfWeight+feather;
				akVertex[3].x=fFontEx-fFontHalfWeight+feather;
				
				if (CGraphicBase::SetPDTStream((SPDTVertex*)akVertex, 4))
					STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
				

				// 오른
				akVertex[0].x=fFontSx+fFontHalfWeight-feather;
				akVertex[1].x=fFontSx+fFontHalfWeight-feather;
				akVertex[2].x=fFontEx+fFontHalfWeight+feather;
				akVertex[3].x=fFontEx+fFontHalfWeight+feather;

				if (CGraphicBase::SetPDTStream((SPDTVertex*)akVertex, 4))
					STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
				
				akVertex[0].x=fFontSx-feather;
				akVertex[1].x=fFontSx-feather;
				akVertex[2].x=fFontEx+feather;
				akVertex[3].x=fFontEx+feather;
				
				// 위
				akVertex[0].y=fFontSy-fFontHalfWeight-feather;
				akVertex[1].y=fFontEy-fFontHalfWeight+feather;
				akVertex[2].y=fFontSy-fFontHalfWeight-feather;
				akVertex[3].y=fFontEy-fFontHalfWeight+feather;

				if (CGraphicBase::SetPDTStream((SPDTVertex*)akVertex, 4))
					STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
				
				// 아래
				akVertex[0].y=fFontSy+fFontHalfWeight-feather;
				akVertex[1].y=fFontEy+fFontHalfWeight+feather;
				akVertex[2].y=fFontSy+fFontHalfWeight-feather;
				akVertex[3].y=fFontEy+fFontHalfWeight+feather;

				if (CGraphicBase::SetPDTStream((SPDTVertex*)akVertex, 4))
					STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
				
				fCurX += fFontAdvance;
			}
		}

		// 메인 폰트
		fCurX=fStanX;
		fCurY=fStanY;

		for (uint32_t i = 0; i < m_pCharInfoVector.size(); ++i)
		{
			pCurCharInfo = m_pCharInfoVector[i];

			fFontWidth=float(pCurCharInfo->width);
			fFontHeight=float(pCurCharInfo->height);
			fFontMaxHeight=std::max<float>(fFontHeight, pCurCharInfo->height);
			fFontAdvance=float(pCurCharInfo->advance);

			// NOTE : 폰트 출력에 Width 제한을 둡니다. - [levites]
			if ((fCurX+fFontWidth)-m_v3Position.x > m_fLimitWidth)
			{
				if (m_isMultiLine)
				{
					fCurX=fStanX;
					fCurY+=fFontMaxHeight;
				}
				else
				{
					break;
				}
			}

			if (pClipRect)
			{
				if (fCurY <= pClipRect->top)
				{
					fCurX += fFontAdvance;
					continue;
				}
			}

			fFontSx = fCurX - 0.5f + pCurCharInfo->prespacing;
			fFontSy = fCurY-0.5f;
			fFontEx = fFontSx + fFontWidth;
			fFontEy = fFontSy + fFontHeight;

			pFontTexture->SelectTexture(pCurCharInfo->index);
			STATEMANAGER.SetTexture(0, pFontTexture->GetD3DTexture());

			akVertex[0].x=fFontSx;
			akVertex[0].y=fFontSy;
			akVertex[0].u=pCurCharInfo->left;
			akVertex[0].v=pCurCharInfo->top;

			akVertex[1].x=fFontSx;
			akVertex[1].y=fFontEy;
			akVertex[1].u=pCurCharInfo->left;
			akVertex[1].v=pCurCharInfo->bottom;

			akVertex[2].x=fFontEx;
			akVertex[2].y=fFontSy;
			akVertex[2].u=pCurCharInfo->right;
			akVertex[2].v=pCurCharInfo->top;

			akVertex[3].x=fFontEx;
			akVertex[3].y=fFontEy;
			akVertex[3].u=pCurCharInfo->right;
			akVertex[3].v=pCurCharInfo->bottom;

			//m_dwColorInfoVector[i];
			//m_dwTextColor;
			akVertex[0].color = akVertex[1].color = akVertex[2].color = akVertex[3].color = m_dwColorInfoVector[i];

			if (CGraphicBase::SetPDTStream((SPDTVertex*)akVertex, 4))
				STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
			//STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, akVertex, sizeof(SVertex));

			fCurX += fFontAdvance;
		}
	}

	if (m_isCursor)
	{
		// Draw Cursor
		float sx, sy, ex, ey;
		TDiffuse diffuse;

		int32_t curpos = CIME::GetCurPos();
		int32_t compend = curpos + CIME::GetCompLen();

		__GetTextPos(curpos, &sx, &sy);

		// If Composition
		if(curpos<compend)
		{
			diffuse = 0x7fffffff;
			__GetTextPos(compend, &ex, &sy);
		}
		else
		{
			diffuse = 0xffffffff;
			ex = sx + 2;
		}

		sx += m_v3Position.x;
		sy += m_v3Position.y;
		ex += m_v3Position.x;
		ey = sy + m_textHeight;

		switch (m_vAlign)
		{
			case VERTICAL_ALIGN_BOTTOM:
				sy -= m_textHeight;
				break;

			case VERTICAL_ALIGN_CENTER:
				sy -= float(m_textHeight) / 2.0f;
				break;
		}		
		// 최적화 사항
		// 같은텍스쳐를 사용한다면... STRIP을 구성하고, 텍스쳐가 변경되거나 끝나면 DrawPrimitive를 호출해
		// 최대한 숫자를 줄이도록하자!

		TPDTVertex vertices[4];
		vertices[0].diffuse = diffuse;
		vertices[1].diffuse = diffuse;
		vertices[2].diffuse = diffuse;
		vertices[3].diffuse = diffuse;
		vertices[0].position = TPosition(sx, sy, 0.0f);
		vertices[1].position = TPosition(ex, sy, 0.0f);
		vertices[2].position = TPosition(sx, ey, 0.0f);
		vertices[3].position = TPosition(ex, ey, 0.0f);

		STATEMANAGER.SetTexture(0, nullptr);

		CGraphicBase::SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);
		if (CGraphicBase::SetPDTStream(vertices, 4))
			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);

		int32_t ulbegin = CIME::GetULBegin();
		int32_t ulend = CIME::GetULEnd();

		if(ulbegin < ulend)
		{
			__GetTextPos(curpos+ulbegin, &sx, &sy);
			__GetTextPos(curpos+ulend, &ex, &sy);

			sx += m_v3Position.x;
			sy += m_v3Position.y + m_textHeight;
			ex += m_v3Position.x;
			ey = sy + 2;

			vertices[0].diffuse = 0xFFFF0000;
			vertices[1].diffuse = 0xFFFF0000;
			vertices[2].diffuse = 0xFFFF0000;
			vertices[3].diffuse = 0xFFFF0000;
			vertices[0].position = TPosition(sx, sy, 0.0f);
			vertices[1].position = TPosition(ex, sy, 0.0f);
			vertices[2].position = TPosition(sx, ey, 0.0f);
			vertices[3].position = TPosition(ex, ey, 0.0f);

			STATEMANAGER.DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, c_FillRectIndices, D3DFMT_INDEX16, vertices, sizeof(TPDTVertex));
		}		
	}

	STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);

	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLighting);

	//금강경 링크 띄워주는 부분.
	if (!m_hyperlinkVector.empty())
	{
		int32_t lx = gs_mx - m_v3Position.x;
		int32_t ly = gs_my - m_v3Position.y;

		if (lx >= 0 && ly >= 0 && lx < m_textWidth && ly < m_textHeight)
		{
			auto it = m_hyperlinkVector.begin();

			while (it != m_hyperlinkVector.end())
			{
				SHyperlink & link = *it++;
				if (lx >= link.sx && lx < link.ex)
				{
					gs_hyperlinkText = link.text;
					/*
					OutputDebugStringW(link.text.c_str());
					OutputDebugStringW(L"\n");
					*/
					break;
				}
			}
		}
	}
}

void CGraphicTextInstance::CreateSystem(uint32_t uCapacity)
{
	ms_kPool.Create(uCapacity);
}

void CGraphicTextInstance::DestroySystem()
{
	ms_kPool.Destroy();
}

CGraphicTextInstance* CGraphicTextInstance::New()
{
	return ms_kPool.Alloc();
}

void CGraphicTextInstance::Delete(CGraphicTextInstance* pkInst)
{
	pkInst->Destroy();
	ms_kPool.Free(pkInst);
}

void CGraphicTextInstance::ShowCursor()
{
	m_isCursor = true;
}

void CGraphicTextInstance::HideCursor()
{
	m_isCursor = false;
}

void CGraphicTextInstance::ShowOutLine()
{
	m_isOutline = true;
}

void CGraphicTextInstance::HideOutLine()
{
	m_isOutline = false;
}

void CGraphicTextInstance::SetColor(uint32_t color)
{
	if (m_dwTextColor != color)
	{
		for (uint32_t i = 0; i < m_pCharInfoVector.size(); ++i)
			if (m_dwColorInfoVector[i] == m_dwTextColor)
				m_dwColorInfoVector[i] = color;

		m_dwTextColor = color;
	}
}

void CGraphicTextInstance::SetColor(float r, float g, float b, float a)
{
	SetColor(D3DXCOLOR(r, g, b, a));
}

void CGraphicTextInstance::SetOutLineColor(uint32_t color)
{
	m_dwOutLineColor=color;
}

void CGraphicTextInstance::SetOutLineColor(float r, float g, float b, float a)
{
	m_dwOutLineColor=D3DXCOLOR(r, g, b, a);
}

void CGraphicTextInstance::SetSecret(bool Value)
{
	m_isSecret = Value;
}

void CGraphicTextInstance::SetOutline(bool Value)
{
	m_isOutline = Value;
}

void CGraphicTextInstance::SetFeather(bool Value)
{
	if (Value)
		m_fFontFeather = c_fFontFeather;
	else
		m_fFontFeather = 0.0f;
}

void CGraphicTextInstance::SetMultiLine(bool Value)
{
	m_isMultiLine = Value;
}

void CGraphicTextInstance::SetHorizonalAlign(int32_t hAlign)
{
	m_hAlign = hAlign;
}

void CGraphicTextInstance::SetVerticalAlign(int32_t vAlign)
{
	m_vAlign = vAlign;
}

void CGraphicTextInstance::SetMax(int32_t iMax)
{
	m_iMax = iMax;
}

void CGraphicTextInstance::SetLimitWidth(float fWidth)
{
	m_fLimitWidth = fWidth;
}

void CGraphicTextInstance::SetValueString(const std::string& c_stValue)
{
	if (c_stValue == m_stText)
		return;

	m_stText = c_stValue;
	m_isUpdate = false;
}

void CGraphicTextInstance::SetValue(const char* c_szText, size_t len)
{
	if (0 == m_stText.compare(c_szText))
		return;

	m_stText = c_szText;
	m_isUpdate = false;
}

void CGraphicTextInstance::SetPosition(float fx, float fy, float fz)
{
	m_v3Position.x = fx;
	m_v3Position.y = fy;
	m_v3Position.z = fz;
}

void CGraphicTextInstance::SetTextPointer(CGraphicText* pText)
{
	m_roText = pText;
}

const std::string & CGraphicTextInstance::GetValueStringReference()
{
	return m_stText;
}

uint16_t CGraphicTextInstance::GetTextLineCount()
{
	CGraphicFontTexture::TCharacterInformation* pCurCharInfo;
	CGraphicFontTexture::TPCharacterInformationVector::iterator itor;

	float fx = 0.0f;
	uint16_t wLineCount = 1;
	for (itor=m_pCharInfoVector.begin(); itor!=m_pCharInfoVector.end(); ++itor)
	{
		pCurCharInfo = *itor;

		float fFontWidth=float(pCurCharInfo->width);
		float fFontAdvance=float(pCurCharInfo->advance);
		//float fFontHeight=float(pCurCharInfo->height);

		// NOTE : 폰트 출력에 Width 제한을 둡니다. - [levites]
		if (fx+fFontWidth > m_fLimitWidth)
		{
			fx = 0.0f;
			++wLineCount;
		}

		fx += fFontAdvance;
	}

	return wLineCount;
}

void CGraphicTextInstance::GetTextSize(int32_t* pRetWidth, int32_t* pRetHeight)
{
	*pRetWidth = m_textWidth;
	*pRetHeight = m_textHeight;
}

int32_t CGraphicTextInstance::PixelPositionToCharacterPosition(int32_t iPixelPosition)
{
	int32_t icurPosition = 0;
	for (int32_t i = 0; i < (int32_t)m_pCharInfoVector.size(); ++i)
	{
		CGraphicFontTexture::TCharacterInformation* pCurCharInfo = m_pCharInfoVector[i];
		icurPosition += pCurCharInfo->width;

		if (iPixelPosition < icurPosition)
			return i;
	}

	return -1;
}

int32_t CGraphicTextInstance::GetHorizontalAlign()
{
	return m_hAlign;
}

void CGraphicTextInstance::__Initialize()
{
	m_roText = nullptr;

	m_hAlign = HORIZONTAL_ALIGN_LEFT;
	m_vAlign = VERTICAL_ALIGN_TOP;

	m_iMax = 0;
	m_fLimitWidth = 1600.0f; // NOTE : 해상도의 최대치. 이보다 길게 쓸 일이 있을까? - [levites]

	m_isCursor = false;
	m_isSecret = false;
	m_isMultiLine = false;

	m_isOutline = false;
	m_fFontFeather = c_fFontFeather;

	m_isUpdate = false;

	m_textWidth = 0;
	m_textHeight = 0;

	m_v3Position.x = m_v3Position.y = m_v3Position.z = 0.0f;

	m_dwOutLineColor=0xff000000;
}

void CGraphicTextInstance::Destroy()
{
	m_stText.clear();
	m_pCharInfoVector.clear();
	m_dwColorInfoVector.clear();
	m_hyperlinkVector.clear();

	__Initialize();
}

uint16_t CGraphicTextInstance::GetLineHeight()
{
	return m_textHeight;
}

D3DXVECTOR3& CGraphicTextInstance::GetPosition()
{
	return m_v3Position;
}

CGraphicTextInstance::CGraphicTextInstance()
{
	__Initialize();
}

CGraphicTextInstance::~CGraphicTextInstance()
{
	Destroy();
}
