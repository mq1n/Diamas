#pragma once

#include "GrpTexture.h"
#include "GrpImageTexture.h"
#include "GrpDIB.h"

#include <vector>
#include <map>

class CGraphicFontTexture : public CGraphicTexture
{
public:
	typedef struct SCharacterInformation
	{
		int16_t index;
		int16_t width;
		int16_t height;
		float left;
		float top;
		float right;
		float bottom;
		float prespacing;
		float advance;
	} TCharacterInformation;

	typedef std::vector<TCharacterInformation*>		TPCharacterInformationVector;

public:
	CGraphicFontTexture();
	virtual ~CGraphicFontTexture();

	void Destroy();
	bool Create(const std::string& fontName, int32_t fontSize, bool bItalic);

	bool CreateDeviceObjects();
	void DestroyDeviceObjects();

	bool CheckTextureIndex(uint32_t dwTexture);
	void SelectTexture(uint32_t dwTexture);

	bool UpdateTexture();

	TCharacterInformation* GetCharacterInformation(wchar_t keyValue);
	TCharacterInformation* UpdateCharacterInformation(wchar_t keyValue);

	bool IsEmpty() const;

protected:
	void Initialize();

	bool AppendTexture();

	HFONT GetFont();

protected:		
	typedef std::vector<CGraphicImageTexture*>				TGraphicImageTexturePointerVector;
	typedef std::map<wchar_t, TCharacterInformation>	TCharacterInformationMap;

protected:		
	CGraphicDib	m_dib;

	HFONT	m_hFontOld;
	HFONT	m_hFont;

	TGraphicImageTexturePointerVector m_pFontTextureVector;

	TCharacterInformationMap m_charInfoMap;

	int32_t m_x;
	int32_t m_y;
	int32_t m_step;		
	bool m_isDirty;

	std::string m_fontName;
	LONG	m_fontSize;
	bool	m_bItalic;
};
