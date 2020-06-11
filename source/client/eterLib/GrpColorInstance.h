#pragma once

#include "GrpColor.h"
#include "Pool.h"

class CGraphicColorInstance
{
	public:
		CGraphicColorInstance();
		virtual ~CGraphicColorInstance();

		void Clear();

		void SetColorReference(const CGraphicColor& c_rSrcColor);
		void BlendColorReference(uint32_t blendTime, const CGraphicColor& c_rDstColor);

		void Update();

		const CGraphicColor& GetCurrentColorReference() const;

	protected:
		uint32_t GetCurrentTime();

	protected:
		CGraphicColor m_srcColor;
		CGraphicColor m_dstColor;
		CGraphicColor m_curColor;

		uint32_t m_baseTime;
		uint32_t m_blendTime;
};

typedef CDynamicPool<CGraphicColorInstance> TGraphicColorInstancePool;
