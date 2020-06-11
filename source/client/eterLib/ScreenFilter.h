#pragma once

#include "GrpScreen.h"

class CScreenFilter : public CScreen
{
	public:
		CScreenFilter();
		virtual ~CScreenFilter();

		void SetEnable(BOOL bFlag);
		void SetBlendType(uint8_t bySrcType, uint8_t byDestType);
		void SetColor(const D3DXCOLOR & c_rColor);

		void Render();

	protected:
		BOOL m_bEnable;
		uint8_t m_bySrcType;
		uint8_t m_byDestType;
		D3DXCOLOR m_Color;
};