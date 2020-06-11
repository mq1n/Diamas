#include "StdAfx.h"
#include "FrameController.h"

void CFrameController::Update(float fElapsedTime)
{
	m_fLastFrameTime -= fElapsedTime;

	for (int32_t i = 0; i < 20; ++i)
	{
		if (m_fLastFrameTime < 0.0f)
		{
			m_fLastFrameTime += m_fFrameTime;
			++m_dwcurFrame;

			if (m_dwcurFrame >= m_dwMaxFrame)
			{
				if (m_isLoop && --m_iLoopCount!=0)
				{
					if (m_iLoopCount<0)
						m_iLoopCount = 0;
					m_dwcurFrame = 0;
				}
				else
				{
					m_iLoopCount = 1;
					m_dwcurFrame = 0;
					m_isActive = FALSE;
					return;
				}
			}
		}
		else
		{
			break;
		}
	}
}

void CFrameController::SetCurrentFrame(uint32_t dwFrame)
{
	m_dwcurFrame = dwFrame;
}

uint8_t CFrameController::GetCurrentFrame()
{
	return m_dwcurFrame;
}

void CFrameController::SetMaxFrame(uint32_t dwMaxFrame)
{
	m_dwMaxFrame = dwMaxFrame;
}
void CFrameController::SetFrameTime(float fTime)
{
	m_fFrameTime = fTime;
	m_fLastFrameTime = fTime;
}
void CFrameController::SetStartFrame(uint32_t dwStartFrame)
{
	m_dwStartFrame = dwStartFrame;
}

void CFrameController::SetLoopFlag(BOOL bFlag)
{
	m_isLoop = bFlag;
}

void CFrameController::SetLoopCount(int32_t iLoopCount)
{
	m_iLoopCount = iLoopCount;
}

void CFrameController::SetActive(BOOL bFlag)
{
	m_isActive = bFlag;
}

BOOL CFrameController::isActive(uint32_t dwMainFrame)
{
	if (dwMainFrame < m_dwStartFrame)
		return FALSE;

	return m_isActive;
}

void CFrameController::Clear()
{
	m_isActive = TRUE;
	m_dwcurFrame = 0;
	m_fLastFrameTime = 0.0f;
}

CFrameController::CFrameController()
{
	m_isActive = TRUE;
	m_dwcurFrame = 0;
	m_fLastFrameTime = 0.0f;

	m_isLoop = FALSE;
	m_dwMaxFrame = 0;
	m_fFrameTime = 0.0f;
	m_dwStartFrame = 0;
}
CFrameController::~CFrameController()
{
}
