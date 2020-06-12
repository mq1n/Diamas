#include "StdAfx.h"
#include "../eterBase/Stl.h"
#include "GrpRenderTargetTexture.h"
#include "StateManager.h"
#include "../EterBase/vk.h"

void CGraphicRenderTargetTexture::ReleaseTextures()
{
	safe_release (m_lpd3dRenderTexture);
	safe_release (m_lpd3dRenderTargetSurface);
	safe_release (m_lpd3dDepthSurface);
	safe_release (m_lpd3dDepthSurface);
	safe_release (m_lpd3dOriginalRenderTarget);
	safe_release (m_lpd3dOldDepthBufferSurface);
	m_renderRect.left = 0;
	m_renderRect.top = 0;
	m_renderRect.right = 0;
	m_renderRect.bottom = 0;
}

void CGraphicRenderTargetTexture::Initialize()
{
	CGraphicTexture::Initialize();
	m_lpd3dRenderTexture = nullptr;
	m_lpd3dRenderTargetSurface = nullptr;
	m_lpd3dDepthSurface = nullptr;
	m_lpd3dOriginalRenderTarget = nullptr;
	m_lpd3dOldDepthBufferSurface = nullptr;
	m_d3dFormat = D3DFMT_UNKNOWN;
	m_depthStencilFormat = D3DFMT_UNKNOWN;
	m_renderRect.left = 0;
	m_renderRect.top = 0;
	m_renderRect.right = 0;
	m_renderRect.bottom = 0;
}

bool CGraphicRenderTargetTexture::Create (int32_t width, int32_t height, D3DFORMAT texFormat, D3DFORMAT dephtFormat)
{
	__Initialize();

	m_height = height;
	m_width = width;

	if (!CreateRenderTexture (width, height, texFormat))
		return false;
	
	if (!CreateRenderDepthStencil (width, height, dephtFormat))
		return false;

	return true;
}

void CGraphicRenderTargetTexture::CreateTextures()
{
	if (CreateRenderTexture (m_width, m_height, m_d3dFormat))
	{
		CreateRenderDepthStencil (m_width, m_height, m_depthStencilFormat);
	}
}

bool CGraphicRenderTargetTexture::CreateRenderTexture (int32_t width, int32_t height, D3DFORMAT format)
{
	m_d3dFormat = format;

	if (FAILED (ms_lpd3dDevice->CreateTexture (width, height, 1, D3DUSAGE_RENDERTARGET, m_d3dFormat, D3DPOOL_DEFAULT, &m_lpd3dRenderTexture, nullptr)))
		return false;

	if (FAILED(m_lpd3dRenderTexture->GetSurfaceLevel(0, &m_lpd3dRenderTargetSurface)))
		return false;
	
	return true;
}

bool CGraphicRenderTargetTexture::CreateRenderDepthStencil (int32_t width, int32_t height, D3DFORMAT format)
{
	m_depthStencilFormat = format;

	return (ms_lpd3dDevice->CreateDepthStencilSurface (width, height, m_depthStencilFormat, D3DMULTISAMPLE_NONE, 0, FALSE, &m_lpd3dDepthSurface, nullptr)) == D3D_OK;
}

void CGraphicRenderTargetTexture::SetRenderTarget()
{
	ms_lpd3dDevice->GetRenderTarget(0, &m_lpd3dOriginalRenderTarget);
	ms_lpd3dDevice->GetDepthStencilSurface(&m_lpd3dOldDepthBufferSurface);
	ms_lpd3dDevice->SetRenderTarget(0, m_lpd3dRenderTargetSurface);
	ms_lpd3dDevice->SetDepthStencilSurface(m_lpd3dDepthSurface);
}

void CGraphicRenderTargetTexture::ResetRenderTarget()
{
	ms_lpd3dDevice->SetRenderTarget(0, m_lpd3dOriginalRenderTarget);
	ms_lpd3dDevice->SetDepthStencilSurface(m_lpd3dOldDepthBufferSurface);

	safe_release (m_lpd3dOriginalRenderTarget);
	safe_release (m_lpd3dOldDepthBufferSurface);
}

void CGraphicRenderTargetTexture::SetRenderingRect (RECT* rect)
{
	m_renderRect = *rect;
}

RECT* CGraphicRenderTargetTexture::GetRenderingRect()
{
	return &m_renderRect;
}

LPDIRECT3DTEXTURE9 CGraphicRenderTargetTexture::GetRenderTargetTexture() const
{
	return m_lpd3dRenderTexture;
}

void CGraphicRenderTargetTexture::Render() const
{
	D3DSURFACE_DESC desc;
	m_lpd3dRenderTargetSurface->GetDesc(&desc);

	float su = m_renderRect.left * 1.0f / desc.Width;
	float sv = m_renderRect.top * 1.0f / desc.Height;
	float eu = 1.0f / desc.Width * m_renderRect.right;
	float ev = 1.0f / desc.Height * m_renderRect.bottom;

	TPDTVertex pVertices[4];

	pVertices[0].position = TPosition (m_renderRect.left - 0.5f,
									   m_renderRect.top - 0.5f,
									   0.0f);
	pVertices[0].texCoord = TTextureCoordinate (su, sv);
	pVertices[0].diffuse = 0xffffffff;


	pVertices[1].position = TPosition (m_renderRect.left + m_renderRect.right - m_renderRect.left - 0.5f,
									   m_renderRect.top - 0.5f,
									   0.0f);
	pVertices[1].texCoord = TTextureCoordinate (eu, sv);
	pVertices[1].diffuse = 0xffffffff;


	pVertices[2].position = TPosition (m_renderRect.left - 0.5f,
									   m_renderRect.top + m_renderRect.bottom - m_renderRect.top - 0.5f,
									   0.0f);
	pVertices[2].texCoord = TTextureCoordinate (su, ev);
	pVertices[2].diffuse = 0xffffffff;

	pVertices[3].position = TPosition (m_renderRect.left + m_renderRect.right - m_renderRect.left - 0.5f,
									   m_renderRect.top + m_renderRect.bottom - m_renderRect.top - 0.5f,
									   0.0f);
	pVertices[3].texCoord = TTextureCoordinate (eu, ev);
	pVertices[3].diffuse = 0xffffffff;

	if (CGraphicBase::SetPDTStream (pVertices, 4))
	{
		CGraphicBase::SetDefaultIndexBuffer (CGraphicBase::DEFAULT_IB_FILL_RECT);

		STATEMANAGER.SetTexture (0, GetRenderTargetTexture());
		STATEMANAGER.SetTexture (1, nullptr);
		STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE);
		STATEMANAGER.DrawIndexedPrimitive (D3DPT_TRIANGLELIST, 0, 4, 0, 2);
	}
}

void CGraphicRenderTargetTexture::__Initialize()
{
	CGraphicTexture::Destroy();
	ReleaseTextures();

	m_d3dFormat = D3DFMT_UNKNOWN;
	m_depthStencilFormat = D3DFMT_UNKNOWN;
}

CGraphicRenderTargetTexture::CGraphicRenderTargetTexture()
{
	CGraphicRenderTargetTexture::Initialize();
}

CGraphicRenderTargetTexture::~CGraphicRenderTargetTexture()
{
	__Initialize();
}
