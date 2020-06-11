#include "Stdafx.h"
#include "../Eterbase/Debug.h"
#include "StateManager.h"
#include "GrpDummyFrameBuffer.h"

#define CHECK(condition, sentence, error_handle)\
	if (condition != (hr = sentence)){\
	TraceError(#condition " != " #sentence "(hr = %x)", hr);\
	error_handle;\
	}

CGraphicDummpyFrameBuffer::CGraphicDummpyFrameBuffer()
{
	m_lpd3dTexture = NULL;
	m_lpd3dDummyRenderTarget = NULL;
	m_lpd3dOriginalRenderTarget = NULL;
	Initialize();
}

CGraphicDummpyFrameBuffer::~CGraphicDummpyFrameBuffer()
{
	Destroy();
}

void CGraphicDummpyFrameBuffer::Initialize()
{
	LPDIRECT3DSURFACE9 pBackBuffer = NULL;
	HRESULT hr;
	CHECK(D3D_OK, ms_lpd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer), );

	D3DSURFACE_DESC desc;
	pBackBuffer->GetDesc(&desc);
	m_width = desc.Width;
	m_height = desc.Height;

	desc.Format = D3DFMT_A8R8G8B8;

	CHECK(D3D_OK, ms_lpd3dDevice->CreateTexture(desc.Width, desc.Height, 0, D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &m_lpd3dTexture, nullptr), );
	CHECK(D3D_OK, m_lpd3dTexture->GetSurfaceLevel(0, &m_lpd3dDummyRenderTarget), );
	/*CHECK(D3D_OK, ms_lpd3dDevice->CreateDepthStencilSurface(desc.Width, desc.Height, D3DFMT_D16, D3DMULTISAMPLE_NONE, &m_lpd3dDummyDepthSurface), );*/

	pBackBuffer->Release();
	m_bIsStateRunning = false;
}

void CGraphicDummpyFrameBuffer::Destroy()
{
	if (m_bIsStateRunning)
	{
		End();
	}

	if (m_lpd3dDummyRenderTarget)
	{
		m_lpd3dDummyRenderTarget->Release();
		m_lpd3dDummyRenderTarget = NULL;
	}

	if (m_lpd3dDummyDepthSurface)
	{
		m_lpd3dDummyDepthSurface->Release();
		m_lpd3dDummyDepthSurface = NULL;
	}
}

void CGraphicDummpyFrameBuffer::Begin()
{
	HRESULT hr;
	CHECK(D3D_OK, ms_lpd3dDevice->GetRenderTarget(0, &m_lpd3dOriginalRenderTarget), );
	CHECK(D3D_OK, ms_lpd3dDevice->GetDepthStencilSurface(&m_lpd3dOldDepthBufferSurface), );
	CHECK(D3D_OK, ms_lpd3dDevice->SetRenderTarget(0, m_lpd3dDummyRenderTarget), );
	ms_lpd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0x00808080, 1, 0xFFFFFFFF);
	m_bIsStateRunning = true;
	ms_lpd3dDevice->BeginScene();
}

void CGraphicDummpyFrameBuffer::End()
{
	if (!m_bIsStateRunning)
	{
		return;
	}
	HRESULT hr;
	ms_lpd3dDevice->EndScene();

	//if (::GetAsyncKeyState(VK_9) & 0x8000)
	//{
	//	D3DXSaveTextureToFile("D:/hong.dds", D3DXIFF_DDS, m_lpd3dTexture, NULL);
	//}

	CHECK(D3D_OK, ms_lpd3dDevice->SetRenderTarget(0, m_lpd3dOriginalRenderTarget), );
	m_lpd3dOriginalRenderTarget->Release();
	m_lpd3dOldDepthBufferSurface->Release();
	m_bIsStateRunning = false;
}

void CGraphicDummpyFrameBuffer::RenderRect(RECT& rect)
{
	DWORD diffuseColor = GetColor(1.f, 1.f, 1.f, 1.f);
	float screenWidth = GetWidth();
	float screenHeight = GetHeight();
	SPDTVertexRaw vertices[4] = {
		{rect.left + 0.5f, rect.top + 0.5f, 0.f, diffuseColor, rect.left / screenWidth, rect.top / screenHeight},
		{rect.left + 0.5f, rect.bottom + 0.5f, 0.f, diffuseColor, rect.left / screenWidth, rect.bottom / screenHeight},
		{rect.right + 0.5f, rect.top + 0.5f, 0.f, diffuseColor, rect.right / screenWidth, rect.top / screenHeight},
		{rect.right + 0.5f, rect.bottom + 0.5f, 0.f, diffuseColor, rect.right / screenWidth, rect.bottom / screenHeight},
	};
	ms_lpd3dDevice->BeginScene();
	if (CGraphicBase::SetPDTStream(vertices, 4))
	{
		CGraphicBase::SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);

		STATEMANAGER.SaveTexture(0, m_lpd3dTexture);
		STATEMANAGER.SaveTexture(1, NULL);
		STATEMANAGER.SaveFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1);
		STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
		STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, true);
		STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);	
	}
	ms_lpd3dDevice->EndScene();
	STATEMANAGER.RestoreTexture(0);
	STATEMANAGER.RestoreTexture(1);
	STATEMANAGER.RestoreFVF();
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
}