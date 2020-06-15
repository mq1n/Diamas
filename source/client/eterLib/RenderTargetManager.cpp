#include "StdAfx.h"
#include "RenderTargetManager.h"
#include "../eterBase/stl.h"
#include "GrpBase.h"

CRenderTargetManager::CRenderTargetManager(): m_widht(800),
											  m_height(600),
											  m_currentRenderTarget(nullptr)
{
}

CRenderTargetManager::~CRenderTargetManager()
{
	Destroy();
}

void CRenderTargetManager::Destroy()
{
	stl_wipe_second(m_renderTargets);
	m_widht = 800;
	m_height = 600;
	m_currentRenderTarget = nullptr;
}

void CRenderTargetManager::CreateRenderTargetTextures()
{
	for (std::unordered_map<uint32_t, CGraphicRenderTargetTexture*>::iterator it = m_renderTargets.begin(); it != m_renderTargets.end(); it++)
		it->second->CreateTextures();
}

void CRenderTargetManager::ReleaseRenderTargetTextures()
{
	for (std::unordered_map<uint32_t, CGraphicRenderTargetTexture*>::iterator it = m_renderTargets.begin(); it != m_renderTargets.end(); it++)
		it->second->ReleaseTextures();
}

bool CRenderTargetManager::CreateRenderTarget(int32_t width, int32_t height)
{
	m_widht = width;
	m_height = height;
	return CreateGraphicTexture(0, width, height, D3DFMT_X8R8G8B8, D3DFMT_D16);
}

bool CRenderTargetManager::CreateRenderTargetWithIndex(int32_t width, int32_t height, uint32_t index)
{
	m_widht = width;
	m_height = height;
	return CreateGraphicTexture(index, width, height, D3DFMT_X8R8G8B8, D3DFMT_D16);
}

bool CRenderTargetManager::GetRenderTargetRect(uint32_t index, RECT& rect)
{
	CGraphicRenderTargetTexture * pTarget = GetRenderTarget(index);
	if (!pTarget)
		return false;

	rect = *pTarget->GetRenderingRect();
	return true;
}

bool CRenderTargetManager::ChangeRenderTarget(uint32_t index)
{
	m_currentRenderTarget = GetRenderTarget(index);

	if (!m_currentRenderTarget)
		return false;

	m_currentRenderTarget->SetRenderTarget();
	return true;
}

void CRenderTargetManager::ResetRenderTarget()
{
	if (m_currentRenderTarget)
	{
		m_currentRenderTarget->ResetRenderTarget();
		m_currentRenderTarget = nullptr;
	}
}

void CRenderTargetManager::ClearRenderTarget() const
{
	if (m_currentRenderTarget)
		ms_lpd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
}

CGraphicRenderTargetTexture* CRenderTargetManager::GetRenderTarget(uint32_t index)
{
	std::unordered_map<uint32_t, CGraphicRenderTargetTexture*>::iterator it = m_renderTargets.find(index);
	if (it != m_renderTargets.end())
		return it->second;

	return nullptr;
}

bool CRenderTargetManager::CreateGraphicTexture(uint32_t index, uint32_t width, uint32_t height, D3DFORMAT texFormat, D3DFORMAT dephtFormat)
{
	if (GetRenderTarget(index))
		return false;

	CGraphicRenderTargetTexture * pTex = new CGraphicRenderTargetTexture;
	if (!pTex->Create(width, height, texFormat, dephtFormat))
	{
		delete pTex;
		return false;
	}

	m_renderTargets.emplace(index, pTex);
	return true;
}
