#include "StdAfx.h"
#include "PythonChat.h"

#include "AbstractApplication.h"
#include "PythonCharacterManager.h"
#include "../EterBase/Timer.h"

int32_t CPythonChat::TChatSet::ms_iChatModeSize = CHAT_TYPE_MAX_NUM;

const float c_fStartDisappearingTime = 5.0f;
const int32_t c_iMaxLineCount = 5;

///////////////////////////////////////////////////////////////////////////////////////////////////

CDynamicPool<CPythonChat::SChatLine> CPythonChat::SChatLine::ms_kPool;

void CPythonChat::SetChatColor(uint32_t eType, uint32_t r, uint32_t g, uint32_t b)
{
	if (eType>=CHAT_TYPE_MAX_NUM)
		return;

	uint32_t dwColor=(0xff000000)|(r<<16)|(g<<8)|(b);
	m_akD3DXClrChat[eType]=D3DXCOLOR(dwColor);	
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CPythonChat::SChatLine* CPythonChat::SChatLine::New()
{
	return ms_kPool.Alloc();
}

void CPythonChat::SChatLine::Delete(SChatLine * pkChatLine)
{
	pkChatLine->Instance.Destroy();
	ms_kPool.Free(pkChatLine);
}

void CPythonChat::SChatLine::DestroySystem()
{
	ms_kPool.Destroy();	
}

void CPythonChat::SChatLine::SetColor(uint32_t dwID, uint32_t dwColor)
{
	assert(dwID < CHAT_LINE_COLOR_ARRAY_MAX_NUM);

	if (dwID >= CHAT_LINE_COLOR_ARRAY_MAX_NUM)
		return;

	aColor[dwID] = dwColor;
}

void CPythonChat::SChatLine::SetColorAll(uint32_t dwColor)
{
	for (int32_t i = 0; i < CHAT_LINE_COLOR_ARRAY_MAX_NUM; ++i)
		aColor[i] = dwColor;
}

D3DXCOLOR & CPythonChat::SChatLine::GetColorRef(uint32_t dwID)
{
	assert(dwID < CHAT_LINE_COLOR_ARRAY_MAX_NUM);

	if (dwID >= CHAT_LINE_COLOR_ARRAY_MAX_NUM)
	{
		static D3DXCOLOR color(1.0f, 0.0f, 0.0f, 1.0f);
		return color;
	}

	return aColor[dwID];
}

CPythonChat::SChatLine::SChatLine()
{
	for (int32_t i = 0; i < CHAT_LINE_COLOR_ARRAY_MAX_NUM; ++i)
		aColor[i] = 0xff0000ff;
}
CPythonChat::SChatLine::~SChatLine() 
{
	Instance.Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int32_t CPythonChat::CreateChatSet(uint32_t dwID)
{
	m_ChatSetMap.emplace(dwID, TChatSet());
	return dwID;
}

void CPythonChat::UpdateViewMode(uint32_t dwID)
{
	IAbstractApplication& rApp=IAbstractApplication::GetSingleton();

	float fcurTime = rApp.GetGlobalTime();
	//float felapsedTime = rApp.GetGlobalElapsedTime();

	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	TChatLineList * pLineList = &(pChatSet->m_ShowingChatLineList);
	int32_t iLineIndex = pLineList->size();
	int32_t iHeight = -(int32_t(pLineList->size()+1) * pChatSet->m_iStep);

	for (auto itor = pLineList->begin(); itor != pLineList->end();)
	{
		TChatLine * pChatLine = (*itor);

		D3DXCOLOR & rColor = pChatLine->GetColorRef(dwID);

		float fElapsedTime = (fcurTime - pChatLine->fAppendedTime);
		if (fElapsedTime >= c_fStartDisappearingTime || iLineIndex >= c_iMaxLineCount)
		{
			rColor.a -= rColor.a / 10.0f;

			if (rColor.a <= 0.1f)
				itor = pLineList->erase(itor);
			else
				++itor;
		}
		else
			++itor;

		/////

		iHeight += pChatSet->m_iStep;
		--iLineIndex;

		pChatLine->Instance.SetPosition(pChatSet->m_ix, pChatSet->m_iy + iHeight);
		pChatLine->Instance.SetColor(rColor);
		pChatLine->Instance.Update();
	}
}

void CPythonChat::UpdateEditMode(uint32_t dwID)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	const int32_t c_iAlphaLine = std::max(0, GetVisibleLineCount(dwID) - GetEditableLineCount(dwID) + 2);

	int32_t iLineIndex = 0;
	float fAlpha = 0.0f;
	float fAlphaStep = 0.0f;

	if (c_iAlphaLine > 0)
		fAlphaStep = 1.0f / float(c_iAlphaLine);

	TChatLineList * pLineList = &(pChatSet->m_ShowingChatLineList);
	int32_t iHeight = -(int32_t(pLineList->size()+1) * pChatSet->m_iStep);

	for (auto itor = pLineList->begin(); itor != pLineList->end(); ++itor)
	{
		TChatLine * pChatLine = (*itor);

		D3DXCOLOR & rColor = pChatLine->GetColorRef(dwID);

		if (iLineIndex < c_iAlphaLine)
		{
			rColor.a += (fAlpha - rColor.a) / 10.0f;
			fAlpha = fMIN(fAlpha+fAlphaStep, 1.0f);
		}
		else
		{
			rColor.a = fMIN(rColor.a+0.05f, 1.0f);
		}

		iHeight += pChatSet->m_iStep;
		pChatLine->Instance.SetPosition(pChatSet->m_ix, pChatSet->m_iy + iHeight);
		pChatLine->Instance.SetColor(rColor);
		pChatLine->Instance.Update();
	}
}

void CPythonChat::UpdateLogMode(uint32_t dwID)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	TChatLineList * pLineList = &(pChatSet->m_ShowingChatLineList);
	int32_t iHeight = 0;

	for (auto itor = pLineList->rbegin(); itor != pLineList->rend(); ++itor)
	{
		TChatLine * pChatLine = (*itor);

		iHeight -= pChatSet->m_iStep;
		pChatLine->Instance.SetPosition(pChatSet->m_ix, pChatSet->m_iy + iHeight);
		pChatLine->Instance.SetColor(pChatLine->GetColorRef(dwID));
		pChatLine->Instance.Update();
	}
}

void CPythonChat::Update(uint32_t dwID)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	switch (pChatSet->m_iBoardState)
	{
		case BOARD_STATE_VIEW:
			UpdateViewMode(dwID);
			break;
		case BOARD_STATE_EDIT:
			UpdateEditMode(dwID);
			break;
		case BOARD_STATE_LOG:
			UpdateLogMode(dwID);
			break;
	}

	uint32_t dwcurTime = CTimer::Instance().GetCurrentMillisecond();
	for (auto itor = m_WaitChatList.begin(); itor != m_WaitChatList.end();)
	{
		TWaitChat & rWaitChat = *itor;

		if (rWaitChat.dwAppendingTime < dwcurTime)
		{
			AppendChat(rWaitChat.iType, rWaitChat.strChat.c_str());

			itor = m_WaitChatList.erase(itor);
		}
		else
		{
			++itor;
		}
	}
}

void CPythonChat::Render(uint32_t dwID)
{
	TChatLineList * pLineList = GetChatLineListPtr(dwID);
	if (!pLineList)
		return;

	for (auto & itor : *pLineList)
	{
		CGraphicTextInstance & rInstance = itor->Instance;
		rInstance.Render();
	}
}


void CPythonChat::SetBoardState(uint32_t dwID, int32_t iState)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iBoardState = iState;
	ArrangeShowingChat(dwID);
}
void CPythonChat::SetPosition(uint32_t dwID, int32_t ix, int32_t iy)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_ix = ix;
	pChatSet->m_iy = iy;
}
void CPythonChat::SetHeight(uint32_t dwID, int32_t iHeight)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iHeight = iHeight;
}
void CPythonChat::SetStep(uint32_t dwID, int32_t iStep)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iStep = iStep;
}
void CPythonChat::ToggleChatMode(uint32_t dwID, int32_t iMode)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iMode[iMode] = 1 - pChatSet->m_iMode[iMode];
// 	Tracef("ToggleChatMode : %d\n", iMode);
	ArrangeShowingChat(dwID);
}
void CPythonChat::EnableChatMode(uint32_t dwID, int32_t iMode)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iMode[iMode] = TRUE;
// 	Tracef("EnableChatMode : %d\n", iMode);
	ArrangeShowingChat(dwID);
}
void CPythonChat::DisableChatMode(uint32_t dwID, int32_t iMode)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iMode[iMode] = FALSE;
// 	Tracef("DisableChatMode : %d\n", iMode);
	ArrangeShowingChat(dwID);
}
void CPythonChat::SetEndPos(uint32_t dwID, float fPos)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	fPos = std::max(0.0f, fPos);
	fPos = std::min(1.0f, fPos);
	if (pChatSet->m_fEndPos != fPos)
	{
		pChatSet->m_fEndPos = fPos;
		ArrangeShowingChat(dwID);
	}
}


int32_t CPythonChat::GetVisibleLineCount(uint32_t dwID)
{
	TChatLineList * pLineList = GetChatLineListPtr(dwID);
	if (!pLineList)
		return 0;

	return pLineList->size();
}

int32_t CPythonChat::GetEditableLineCount(uint32_t dwID)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return 0;

	return pChatSet->m_iHeight / pChatSet->m_iStep + 1;
}

int32_t CPythonChat::GetLineCount(uint32_t dwID)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return 0;

	int32_t iCount = 0;
	for (auto & i : m_ChatLineDeque)
	{
		if (!pChatSet->CheckMode(i->iType))
			continue;

		++iCount;
	}

	return iCount;
}

int32_t CPythonChat::GetLineStep(uint32_t dwID)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return 0;

	return pChatSet->m_iStep;
}

CPythonChat::TChatLineList * CPythonChat::GetChatLineListPtr(uint32_t dwID)
{
	auto itor = m_ChatSetMap.find(dwID);
	if (m_ChatSetMap.end() == itor)
		return nullptr;

	TChatSet & rChatSet = itor->second;
	return &(rChatSet.m_ShowingChatLineList);
}

CPythonChat::TChatSet * CPythonChat::GetChatSetPtr(uint32_t dwID)
{
	auto itor = m_ChatSetMap.find(dwID);
	if (m_ChatSetMap.end() == itor)
		return nullptr;

	TChatSet & rChatSet = itor->second;
	return &rChatSet;
}

void CPythonChat::ArrangeShowingChat(uint32_t dwID)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_ShowingChatLineList.clear();

	TChatLineDeque TempChatLineDeque;
	for (auto itor = m_ChatLineDeque.begin(); itor != m_ChatLineDeque.end(); ++itor)
	{
		TChatLine * pChatLine = *itor;
		if (pChatSet->CheckMode(pChatLine->iType))
			TempChatLineDeque.emplace_back(pChatLine);
	}

	int32_t icurLineCount = TempChatLineDeque.size();
	int32_t iVisibleLineCount = std::min(icurLineCount, (pChatSet->m_iHeight + pChatSet->m_iStep) / pChatSet->m_iStep);
	int32_t iEndLine = iVisibleLineCount + int32_t(float(icurLineCount - iVisibleLineCount - 1) * pChatSet->m_fEndPos);

	/////

	int32_t iHeight = 12;
	for (int32_t i = std::min(icurLineCount-1, iEndLine); i >= 0; --i)
	{
		if (!pChatSet->CheckMode(TempChatLineDeque[i]->iType))
			continue;

		if (pChatSet->m_iHeight + pChatSet->m_iStep <= iHeight)
			break;

		pChatSet->m_ShowingChatLineList.push_front(TempChatLineDeque[i]);

		iHeight += pChatSet->m_iStep;
	}
}

void CPythonChat::AppendChat(int32_t iType, const char * c_szChat)
{
	// DEFAULT_FONT
	//static CResource * s_pResource = CResourceManager::Instance().GetResourcePointer(g_strDefaultFontName.c_str());

	CGraphicText* pkDefaultFont = static_cast<CGraphicText*>(DefaultFont_GetResource());
	if (!pkDefaultFont)
	{
		TraceError("CPythonChat::AppendChat - CANNOT_FIND_DEFAULT_FONT");
		return;
	}
	// END_OF_DEFAULT_FONT

	IAbstractApplication& rApp=IAbstractApplication::GetSingleton();
	SChatLine * pChatLine = SChatLine::New();
	pChatLine->iType = iType;
	pChatLine->Instance.SetValue(c_szChat);

	// DEFAULT_FONT
	pChatLine->Instance.SetTextPointer(pkDefaultFont);
	// END_OF_DEFAULT_FONT

	pChatLine->fAppendedTime = rApp.GetGlobalTime();
	pChatLine->SetColorAll(GetChatColor(iType));

	m_ChatLineDeque.emplace_back(pChatLine);
	if (m_ChatLineDeque.size() > CHAT_LINE_MAX_NUM)
	{
		SChatLine * pChatLine = m_ChatLineDeque.front();
		SChatLine::Delete(pChatLine);
		m_ChatLineDeque.pop_front();
	}

	for (auto & itor : m_ChatSetMap)
	{
		TChatSet * pChatSet = &(itor.second);
		//pChatLine->SetColor(itor->first, GetChatColor(iType));

		// Edit Mode 를 억지로 끼워 맞추기 위해 추가
		if (BOARD_STATE_EDIT == pChatSet->m_iBoardState)
			ArrangeShowingChat(itor.first);
		else // if (BOARD_STATE_VIEW == pChatSet->m_iBoardState)
		{
			pChatSet->m_ShowingChatLineList.emplace_back(pChatLine);
			if (pChatSet->m_ShowingChatLineList.size() > CHAT_LINE_MAX_NUM)
				pChatSet->m_ShowingChatLineList.pop_front();
		}
	}
}

void CPythonChat::AppendChatWithDelay(int32_t iType, const char * c_szChat, int32_t iDelay)
{
	TWaitChat WaitChat;
	WaitChat.iType = iType;
	WaitChat.strChat = c_szChat;
	WaitChat.dwAppendingTime = CTimer::Instance().GetCurrentMillisecond() + iDelay;
	m_WaitChatList.emplace_back(WaitChat);
}

uint32_t CPythonChat::GetChatColor(int32_t iType)
{
	if (iType<CHAT_TYPE_MAX_NUM)
		return m_akD3DXClrChat[iType];

	return D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
}

void CPythonChat::IgnoreCharacter(const char * c_szName)
{
	auto itor = m_IgnoreCharacterSet.find(c_szName);
	// NOTE : 이미 차단 중이라면..
	if (m_IgnoreCharacterSet.end() != itor)
	{
		m_IgnoreCharacterSet.erase(itor);
		// NOTE : 차단이 되지 않은 캐릭터라면..
	}
	else
		m_IgnoreCharacterSet.emplace(c_szName);
}

BOOL CPythonChat::IsIgnoreCharacter(const char * c_szName)
{
	auto itor = m_IgnoreCharacterSet.find(c_szName);

	if (m_IgnoreCharacterSet.end() == itor)
		return FALSE;

	return TRUE;
}

CWhisper * CPythonChat::CreateWhisper(const char * c_szName)
{
	CWhisper * pWhisper = CWhisper::New();
	m_WhisperMap.emplace(c_szName, pWhisper);
	return pWhisper;
}

void CPythonChat::AppendWhisper(int32_t iType, const char * c_szName, const char * c_szChat)
{
	CWhisper * pWhisper{};
	auto itor = m_WhisperMap.find(c_szName); 

	if (itor != m_WhisperMap.end())
		pWhisper = itor->second;
	else
		pWhisper = CreateWhisper(c_szName);

	pWhisper->AppendChat(iType, c_szChat);
}

void CPythonChat::ClearWhisper(const char * c_szName)
{
	auto itor = m_WhisperMap.find(c_szName);
	if (itor != m_WhisperMap.end())
	{
		CWhisper * pWhisper = itor->second;
		CWhisper::Delete(pWhisper);
		
		m_WhisperMap.erase(itor);
	}
}

BOOL CPythonChat::GetWhisper(const char * c_szName, CWhisper ** ppWhisper)
{
	auto itor = m_WhisperMap.find(c_szName);
	if (itor != m_WhisperMap.end())
	{
		*ppWhisper = itor->second;
		return TRUE;
	}
	return FALSE;
}

void CPythonChat::InitWhisper(PyObject * ppyObject)
{
	for (auto itor = m_WhisperMap.begin(); itor != m_WhisperMap.end(); ++itor)
	{
		std::string strName = itor->first;
		PyCallClassMemberFunc(ppyObject, "MakeWhisperButton", Py_BuildValue("(s)", strName.c_str()));
	}
}

void CPythonChat::__DestroyWhisperMap()
{
	for (auto itor = m_WhisperMap.begin(); itor != m_WhisperMap.end(); ++itor)
		CWhisper::Delete(itor->second);
	m_WhisperMap.clear();
}

void CPythonChat::Close()
{
	for (auto itor = m_ChatSetMap.begin(); itor != m_ChatSetMap.end(); ++itor)
	{
		TChatSet & rChatSet = itor->second;
		TChatLineList * pLineList = &(rChatSet.m_ShowingChatLineList);
		for (auto & pChatLine : *pLineList)
			pChatLine->fAppendedTime = 0.0f;
	}
}

void CPythonChat::Destroy()
{
	__DestroyWhisperMap();	

	m_ShowingChatLineList.clear();
	m_ChatSetMap.clear();
	m_ChatLineDeque.clear();

	SChatLine::DestroySystem();
	CWhisper::DestroySystem();

	__Initialize();
}

void CPythonChat::__Initialize()
{
	m_akD3DXClrChat[CHAT_TYPE_TALKING]		= D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_INFO]			= D3DXCOLOR(1.0f, 0.785f, 0.785f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_NOTICE]		= D3DXCOLOR(1.0f, 0.902f, 0.730f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_PARTY]		= D3DXCOLOR(0.542f, 1.0f, 0.949f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_GUILD]		= D3DXCOLOR(0.906f, 0.847f, 1.0f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_COMMAND]		= D3DXCOLOR(0.658f, 1.0f, 0.835f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_SHOUT]		= D3DXCOLOR(0.658f, 1.0f, 0.835f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_WHISPER]		= D3DXCOLOR(0xff4AE14A);
	m_akD3DXClrChat[CHAT_TYPE_BIG_NOTICE]	= D3DXCOLOR(1.0f, 0.902f, 0.730f, 1.0f);
#ifdef ENABLE_DICE_SYSTEM
	m_akD3DXClrChat[CHAT_TYPE_DICE_INFO]	= D3DXCOLOR(0xFFcc00cc);
#endif
}

CPythonChat::CPythonChat()
{
	__Initialize();
}

CPythonChat::~CPythonChat()
{
	assert(m_ChatLineDeque.empty());
	assert(m_ShowingChatLineList.empty());
	assert(m_ChatSetMap.empty());
	assert(m_WhisperMap.empty());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CDynamicPool<CWhisper> CWhisper::ms_kPool;

CWhisper* CWhisper::New()
{
	return ms_kPool.Alloc();
}

void CWhisper::Delete(CWhisper* pkWhisper)
{
	pkWhisper->Destroy();
	ms_kPool.Free(pkWhisper);
}

void CWhisper::DestroySystem()
{
	ms_kPool.Destroy();	

	SChatLine::DestroySystem();	
}

void CWhisper::SetPosition(float fPosition)
{
	m_fcurPosition = fPosition;
	__ArrangeChat();
}

const char * CWhisper::GetLastWhisper()
{
	return m_lastWhisper;
}

void CWhisper::SetBoxSize(float fWidth, float fHeight)
{
	m_fWidth = fWidth;
	m_fHeight = fHeight;

	for (auto itor = m_ChatLineDeque.begin(); itor != m_ChatLineDeque.end(); ++itor)
	{
		TChatLine * pChatLine = *itor;
		pChatLine->Instance.SetLimitWidth(fWidth);
	}
}

void CWhisper::AppendChat(int32_t iType, const char * c_szChat)
{
	// DEFAULT_FONT
	//static CResource * s_pResource = CResourceManager::Instance().GetResourcePointer(g_strDefaultFontName.c_str());

	CGraphicText* pkDefaultFont = (iType == CPythonChat::WHISPER_TYPE_GM) ? static_cast<CGraphicText*>(DefaultItalicFont_GetResource()) : static_cast<CGraphicText*>(DefaultFont_GetResource());
	if (!pkDefaultFont)
	{
		TraceError("CWhisper::AppendChat - CANNOT_FIND_DEFAULT_FONT");
		return;
	}
	// END_OF_DEFAULT_FONT

	SChatLine * pChatLine = SChatLine::New();
	pChatLine->Instance.SetValue(c_szChat);

	m_lastWhisper = c_szChat;

	// DEFAULT_FONT
	pChatLine->Instance.SetTextPointer(pkDefaultFont);
	// END_OF_DEFAULT_FONT

	pChatLine->Instance.SetLimitWidth(m_fWidth);
	pChatLine->Instance.SetMultiLine(TRUE);

	switch(iType)
	{
		case CPythonChat::WHISPER_TYPE_SYSTEM:
			pChatLine->Instance.SetColor(D3DXCOLOR(1.0f, 0.785f, 0.785f, 1.0f));
			break;
		case CPythonChat::WHISPER_TYPE_GM:
			pChatLine->Instance.SetColor(D3DXCOLOR(1.0f, 0.632f, 0.0f, 1.0f));
			break;
		case CPythonChat::WHISPER_TYPE_CHAT:
		default:
			pChatLine->Instance.SetColor(0xffffffff);
			break;
	}

	m_ChatLineDeque.emplace_back(pChatLine);

	__ArrangeChat();
}

void CWhisper::__ArrangeChat()
{
	for (auto itor = m_ChatLineDeque.begin(); itor != m_ChatLineDeque.end(); ++itor)
	{
		TChatLine * pChatLine = *itor;
		pChatLine->Instance.Update();
	}
}

void CWhisper::Render(float fx, float fy)
{
	float fHeight = fy + m_fHeight;

	int32_t iViewCount = int32_t(m_fHeight / m_fLineStep) - 1;
	int32_t iLineCount = int32_t(m_ChatLineDeque.size());
	int32_t iStartLine = -1;
	if (iLineCount > iViewCount)
		iStartLine = int32_t(float(iLineCount - iViewCount) * m_fcurPosition) + iViewCount - 1;
	else if (!m_ChatLineDeque.empty())
		iStartLine = iLineCount - 1;

	RECT Rect = { fx, fy, fx+m_fWidth, fy+m_fHeight };

	for (int32_t i = iStartLine; i >= 0; --i)
	{
		assert(i >= 0 && i < int32_t(m_ChatLineDeque.size()));
		TChatLine * pChatLine = m_ChatLineDeque[i];

		uint16_t wLineCount = pChatLine->Instance.GetTextLineCount();
		fHeight -= wLineCount * m_fLineStep;

		pChatLine->Instance.SetPosition(fx, fHeight);
		pChatLine->Instance.Render(&Rect);

		if (fHeight < fy)
			break;
	}
}

void CWhisper::__Initialize()
{
	m_fLineStep = 15.0f;
	m_fWidth = 300.0f;
	m_fHeight = 120.0f;
	m_fcurPosition = 1.0f;
	m_lastWhisper = "";
}

void CWhisper::Destroy()
{
	std::for_each(m_ChatLineDeque.begin(), m_ChatLineDeque.end(), SChatLine::Delete);
	m_ChatLineDeque.clear();
	m_ShowingChatLineList.clear();
}

CWhisper::CWhisper()
{
	__Initialize();
}

CWhisper::~CWhisper()
{
	Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CDynamicPool<CWhisper::SChatLine> CWhisper::SChatLine::ms_kPool;

CWhisper::SChatLine* CWhisper::SChatLine::New()
{
	return ms_kPool.Alloc();
}

void CWhisper::SChatLine::Delete(SChatLine * pkChatLine)
{
	pkChatLine->Instance.Destroy();
	ms_kPool.Free(pkChatLine);
}

void CWhisper::SChatLine::DestroySystem()
{
	ms_kPool.Destroy();	
}
