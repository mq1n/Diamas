#ifndef __INC_ETERLIB_GRPTEXTINSTANCE_H__
#define __INC_ETERLIB_GRPTEXTINSTANCE_H__

#include "Pool.h"
#include "GrpText.h"
#include "GrpImageInstance.h"

class CGraphicTextInstance
{
	public:
		typedef CDynamicPool<CGraphicTextInstance> TPool;

	public:
		enum EHorizontalAlign
		{
			HORIZONTAL_ALIGN_LEFT		= 0x01,
			HORIZONTAL_ALIGN_CENTER		= 0x02,
			HORIZONTAL_ALIGN_RIGHT		= 0x03,
		};
		enum EVerticalAlign
		{
			VERTICAL_ALIGN_TOP		= 0x10,
			VERTICAL_ALIGN_CENTER	= 0x20,
			VERTICAL_ALIGN_BOTTOM	= 0x30
		};

	public:
		static void Hyperlink_UpdateMousePos(int32_t x, int32_t y);
		static int32_t  Hyperlink_GetText(char* buf, int32_t len);

	public:
		CGraphicTextInstance();
		virtual ~CGraphicTextInstance();

		void Destroy();
		
		void Update();
		void Render(RECT * pClipRect = nullptr);

		void ShowCursor();
		void HideCursor();

		void ShowOutLine();
		void HideOutLine();

		void SetColor(uint32_t color);
		void SetColor(float r, float g, float b, float a = 1.0f);

		void SetOutLineColor(uint32_t color);
		void SetOutLineColor(float r, float g, float b, float a = 1.0f);

		void SetHorizonalAlign(int32_t hAlign);
		void SetVerticalAlign(int32_t vAlign);
		void SetMax(int32_t iMax);
		void SetTextPointer(CGraphicText* pText);
		void SetValueString(const std::string& c_stValue);
		void SetValue(const char* c_szValue, size_t len = -1);
		void SetPosition(float fx, float fy, float fz = 0.0f);
		void SetSecret(bool Value);
		void SetOutline(bool Value);
		void SetFeather(bool Value);
		void SetMultiLine(bool Value);
		void SetLimitWidth(float fWidth);

		void GetTextSize(int32_t* pRetWidth, int32_t* pRetHeight);
		const std::string& GetValueStringReference();
		uint16_t GetTextLineCount();

		int32_t PixelPositionToCharacterPosition(int32_t iPixelPosition);
		int32_t GetHorizontalAlign();
		
	protected:
		void __Initialize();
		int32_t  __DrawCharacter(CGraphicFontTexture * pFontTexture, uint16_t codePage, wchar_t text, uint32_t dwColor);
		void __GetTextPos(uint32_t index, float* x, float* y);
		int32_t __GetTextTag(const wchar_t * src, int32_t maxLen, int32_t & tagLen, std::wstring & extraInfo);

	protected:
		struct SHyperlink
		{
			int16_t sx;
			int16_t ex;
			std::wstring text;

			SHyperlink() : sx(0), ex(0) { }
		};

	protected:
		uint32_t m_dwTextColor;
		uint32_t m_dwOutLineColor;

		uint16_t m_textWidth;
		uint16_t m_textHeight;

		uint8_t m_hAlign;
		uint8_t m_vAlign;

		uint16_t m_iMax;
		float m_fLimitWidth;

		bool m_isCursor;
		bool m_isSecret;
		bool m_isMultiLine;

		bool m_isOutline;
		float m_fFontFeather;

		/////

		std::string m_stText;
		D3DXVECTOR3 m_v3Position;

	private:
		bool m_isUpdate;
		bool m_isUpdateFontTexture;
		
		CGraphicText::TRef m_roText;
		CGraphicFontTexture::TPCharacterInfomationVector m_pCharInfoVector;
		std::vector<uint32_t> m_dwColorInfoVector;
		std::vector<SHyperlink> m_hyperlinkVector;

	public:
		static void CreateSystem(uint32_t uCapacity);
		static void DestroySystem();

		static CGraphicTextInstance* New();
		static void Delete(CGraphicTextInstance* pkInst);

		static CDynamicPool<CGraphicTextInstance>		ms_kPool;
};

extern const char* FindToken(const char* begin, const char* end);
extern int32_t ReadToken(const char* token);

#endif