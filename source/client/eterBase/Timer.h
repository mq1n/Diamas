#pragma once
#include <Windows.h>
#include "../../common/singleton.h"

class CTimer : public CSingleton<CTimer>
{
	public:
		CTimer();
		virtual ~CTimer();

		void	Advance();
		void	Adjust(int32_t iTimeGap);
		void	SetBaseTime();

	float GetCurrentSecond() const;
	uint32_t GetCurrentMillisecond() const;

		float	GetElapsedSecond();
	uint32_t GetElapsedMilliecond() const;

		void	UseCustomTime();

	protected:
		bool	m_bUseRealTime;
		uint32_t	m_dwBaseTime;
		uint32_t	m_dwCurrentTime;
		float	m_fCurrentTime;
		uint32_t	m_dwElapsedTime;
		int32_t		m_index;
};

BOOL	ELTimer_Init();

uint32_t	ELTimer_GetMSec();

VOID	ELTimer_SetServerMSec(uint32_t dwServerTime);
uint32_t	ELTimer_GetServerMSec();

VOID	ELTimer_SetFrameMSec();
uint32_t	ELTimer_GetFrameMSec();