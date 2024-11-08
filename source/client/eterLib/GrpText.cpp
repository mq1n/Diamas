#include "StdAfx.h"
#include "../eterBase/Utils.h"
#include "GrpText.h"

CGraphicText::CGraphicText(const FileSystem::CFileName& filename) : CResource(filename)
{
}

CGraphicText::~CGraphicText()
{
}

bool CGraphicText::CreateDeviceObjects()
{
	return m_fontTexture.CreateDeviceObjects();
}

void CGraphicText::DestroyDeviceObjects()
{
	m_fontTexture.DestroyDeviceObjects();
}

CGraphicFontTexture* CGraphicText::GetTexturePointer()
{
	return &m_fontTexture;
}

CGraphicText::TType CGraphicText::Type()
{
	static TType s_type = StringToType("CGraphicText");
	return s_type;
}

bool CGraphicText::OnLoad(int32_t /*iSize*/, const void* /*c_pvBuf*/)
{
	static char strName[32];
	int32_t size;
	bool bItalic = false;

	// format
	// ����.fnt		"����" ��Ʈ �⺻ ������ 12 �� �ε�
	// ����:18.fnt  "����" ��Ʈ ������ 18 �� �ε�
	// ����:14i.fnt "����" ��Ʈ ������ 14 & ���Ÿ����� �ε�
	const auto& stRefResourceName = GetFileNameString();
	const char * p = strrchr(stRefResourceName.c_str(), ':');

	if (p)
	{
		strncpy_s(strName, stRefResourceName.c_str(), MIN(31, p - stRefResourceName.c_str()));
		++p;

		static char num[8];

		int32_t i = 0;
		while (*p && isdigit(*p))
			num[i++] = *(p++);

		num[i] = '\0';
		if(*p == 'i')
			bItalic = true;
		size = atoi(num);
	}
	else
	{
		p = strrchr(stRefResourceName.c_str(), '.');

		if (!p)
		{
			assert(!"CGraphicText::OnLoadFromFile there is no extension (ie: .fnt)");
			strName[0] = '\0';
		}
		else
			strncpy_s(strName, stRefResourceName.c_str(), MIN(31, p - stRefResourceName.c_str()));
		
		size = 12;
	}

	if (!m_fontTexture.Create(strName, size, bItalic))
		return false;

	return true;
}

void CGraphicText::OnClear()
{
	m_fontTexture.Destroy();
}

bool CGraphicText::OnIsEmpty() const
{
	return m_fontTexture.IsEmpty();
}

bool CGraphicText::OnIsType(TType type)
{
	if (CGraphicText::Type() == type)
		return true;
	
	return CResource::OnIsType(type);
}
