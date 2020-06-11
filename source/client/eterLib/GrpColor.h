#pragma once

class CGraphicColor 
{
	public:
		CGraphicColor(const CGraphicColor& c_rSrcColor);
		CGraphicColor(float r, float g, float b, float a);
		CGraphicColor(uint32_t color);

		CGraphicColor();
		~CGraphicColor();

		void Clear();

		void Set(float r, float g, float b, float a);
		void Set(const CGraphicColor& c_rSrcColor);
		void Set(uint32_t color);

		void Blend(float p, const CGraphicColor& c_rSrcColor, const CGraphicColor& c_rDstColor);

		uint32_t GetPackValue() const;

	protected:
		float m_r;
		float m_g;
		float m_b;
		float m_a;
};
