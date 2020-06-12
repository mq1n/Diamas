#include "StdAfx.h"
#include "GrpImage.h"

CGraphicImage::CGraphicImage(const FileSystem::CFileName& filename, uint32_t dwFilter) :
CResource(filename),
m_dwFilter(dwFilter)
{
	m_rect.bottom = m_rect.right = m_rect.top = m_rect.left = 0;
}

CGraphicImage::~CGraphicImage()
{
}

bool CGraphicImage::CreateDeviceObjects()
{
	return m_imageTexture.CreateDeviceObjects();
}

void CGraphicImage::DestroyDeviceObjects()
{
	m_imageTexture.DestroyDeviceObjects();
}

CGraphicImage::TType CGraphicImage::Type()
{
	static TType s_type = StringToType("CGraphicImage");
	return s_type;
}

bool CGraphicImage::OnIsType(TType type)
{
	if (CGraphicImage::Type() == type)
		return true;

	return CResource::OnIsType(type);
}

int32_t CGraphicImage::GetWidth() const
{
	return m_rect.right - m_rect.left;
}

int32_t CGraphicImage::GetHeight() const
{
	return m_rect.bottom - m_rect.top;
}

const CGraphicTexture& CGraphicImage::GetTextureReference() const
{
	return m_imageTexture;
}

CGraphicTexture* CGraphicImage::GetTexturePointer()
{
	return &m_imageTexture;
}

const RECT& CGraphicImage::GetRectReference() const
{
	return m_rect;
}

bool CGraphicImage::OnLoad(int32_t iSize, const void * c_pvBuf)
{
	if (!c_pvBuf)
		return false;

	m_imageTexture.SetFileName(GetFilename());

	// Ư�� ��ǻ�Ϳ��� Unknown���� '��'�ϸ� ƨ��� ������ ����-_-; -��
	//@fixme002
	if (!m_imageTexture.CreateFromMemoryFile(iSize, c_pvBuf, D3DFMT_UNKNOWN, m_dwFilter))
	{
		const auto& stRefResourceName = CResource::GetFileNameString();
		TraceError("CGraphicImage::OnLoad: CreateFromMemoryFile: texture not found(%s)", stRefResourceName.c_str());
		return false;
	}

	m_rect.left = 0;
	m_rect.top = 0;
	m_rect.right = m_imageTexture.GetWidth();
	m_rect.bottom = m_imageTexture.GetHeight();
	return true;
}

void CGraphicImage::OnClear()
{
//	Tracef("Image Destroy : %s\n", m_pszFileName);
	m_imageTexture.Destroy();
	memset(&m_rect, 0, sizeof(m_rect));
}

bool CGraphicImage::OnIsEmpty() const
{
	return m_imageTexture.IsEmpty();
}
