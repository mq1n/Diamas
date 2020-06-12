#include "StdAfx.h"
#include "FrameController.h"

void CFrameController::Update(float fElapsedTime)
{
	if (m_fFrameTime == 0.0f || m_dwMaxFrame == 0) return;
	m_fLastFrameTime += fElapsedTime;

	const auto count = static_cast<uint32_t>(m_fLastFrameTime / m_fFrameTime);
	m_dwcurFrame += count;

	const uint32_t loopCount = m_dwcurFrame / m_dwMaxFrame;
	if (0 != loopCount) {
		// A |m_iLoopCount| of 0 means infinite iterations
		if (!m_isLoop || (0 != m_iLoopCount && loopCount >= static_cast<uint32_t>(m_iLoopCount))) {
			m_fLastFrameTime = 0.0f;
			m_dwcurFrame = 0;
			m_iLoopCount = 1;
			m_isActive = false;
			return;
		}

		m_dwcurFrame %= m_dwMaxFrame;
		m_iLoopCount -= loopCount;
	}

	// Store leftover time for the next call to Update()
	m_fLastFrameTime -= m_fFrameTime * count;
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
	m_fLastFrameTime = 0.0f;
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
	m_iLoopCount = 0;
}

CFrameController::CFrameController()
{
	m_isActive = TRUE;
	m_dwcurFrame = 0;
	m_fLastFrameTime = 0.0f;
	m_iLoopCount = 0;
	m_isLoop = FALSE;
	m_dwMaxFrame = 0;
	m_fFrameTime = 0.0f;
	m_dwStartFrame = 0;
}
CFrameController::~CFrameController()
{
}
