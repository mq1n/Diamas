#pragma once

class CFrameController
{
	public:
		CFrameController();
		virtual ~CFrameController();

		void Clear();

		void Update(float fElapsedTime);

		void SetCurrentFrame(uint32_t dwFrame);
		uint8_t GetCurrentFrame() const;

		void SetMaxFrame(uint32_t dwMaxFrame);
		void SetFrameTime(float fTime);
		void SetStartFrame(uint32_t dwStartFrame);
		void SetLoopFlag(BOOL bFlag);
		void SetLoopCount(int32_t iLoopCount);

		void SetActive(BOOL bFlag);
		BOOL isActive(uint32_t dwMainFrame = 0) const;

	protected:
		// Dynamic
		BOOL m_isActive;
		uint32_t m_dwcurFrame;
		float m_fLastFrameTime;

		int32_t m_iLoopCount;

		// Static
		BOOL m_isLoop;
		uint32_t m_dwMaxFrame;
		float m_fFrameTime;
		uint32_t m_dwStartFrame;
};
