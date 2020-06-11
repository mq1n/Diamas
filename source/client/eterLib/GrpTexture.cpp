#include "StdAfx.h"
#include "../eterBase/Stl.h"
#include "GrpTexture.h"
#include "StateManager.h"

void CGraphicTexture::DestroyDeviceObjects()
{
	safe_release(m_lpd3dTexture);
}

void CGraphicTexture::Destroy()
{
	DestroyDeviceObjects();

	Initialize();
}

void CGraphicTexture::Initialize()
{
	m_lpd3dTexture = nullptr;
	m_width = 0;
	m_height = 0;
	m_bEmpty = true;
}

bool CGraphicTexture::IsEmpty() const
{
	return m_bEmpty;
}

void CGraphicTexture::SetTextureStage(int32_t stage) const
{
	assert(ms_lpd3dDevice != nullptr);
	STATEMANAGER.SetTexture(stage, m_lpd3dTexture);	
}

LPDIRECT3DTEXTURE8 CGraphicTexture::GetD3DTexture() const
{
	return m_lpd3dTexture;
}

int32_t CGraphicTexture::GetWidth() const
{
	return m_width;
}

int32_t CGraphicTexture::GetHeight() const
{
	return m_height;
}

CGraphicTexture::CGraphicTexture()
{
	Initialize();
}

CGraphicTexture::~CGraphicTexture()	
{
}
