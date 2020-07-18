#include "StdAfx.h"

#include <stdio.h>
#include <d3d9/d3d9.h>
#include <d3d9/d3d9types.h>
#include <d3d9/d3dx9.h>

#include "../eterBase/Timer.h"
#include "../eterLib/StateManager.h"
#include "../eterLib/Camera.h"

#include "SpeedTreeForestDirectX9.h"
#include "SpeedTreeConfig.h"
#include "VertexShaders.h"
#include <list>
CSpeedTreeForestDirectX9::CSpeedTreeForestDirectX9()  : m_pDx(nullptr)
{
}
CSpeedTreeForestDirectX9::~CSpeedTreeForestDirectX9()
{
}
bool CSpeedTreeForestDirectX9::InitVertexShaders(void)
{
	m_branchVertexShader = LoadBranchShader(m_pDx);
	m_leafVertexShader = LoadLeafShader(m_pDx);

	CSpeedTreeWrapper::SetVertexShaders(m_branchVertexShader, m_leafVertexShader);
	return true;
}

bool CSpeedTreeForestDirectX9::SetRenderingDevice(LPDIRECT3DDEVICE9 lpDevice)
{
	m_pDx = lpDevice;

	if (!InitVertexShaders())
		return false;

	float afLight1[] =
	{
		c_afLightPosition[0], c_afLightPosition[1], c_afLightPosition[2],
		c_afLightDiffuse[0], c_afLightDiffuse[1], c_afLightDiffuse[2],
		c_afLightAmbient[0], c_afLightAmbient[1], c_afLightAmbient[2],
		c_afLightSpecular[0], c_afLightSpecular[1], c_afLightSpecular[2],
		c_afLightPosition[3],
		1.0f, 0.0f, 0.0f
	};

	CSpeedTreeRT::SetNumWindMatrices(c_nNumWindMatrices);

	CSpeedTreeRT::SetLightAttributes(0, afLight1);
	CSpeedTreeRT::SetLightState(0, true);
	return true;
}
void CSpeedTreeForestDirectX9::UploadWindMatrix(uint32_t uiLocation, const float* pMatrix) const
{
	STATEMANAGER.SetVertexShaderConstant(uiLocation, pMatrix, 4);
}

void CSpeedTreeForestDirectX9::UpdateCompundMatrix(const D3DXVECTOR3 & c_rEyeVec, const D3DXMATRIX & c_rmatView, const D3DXMATRIX & c_rmatProj)
{
    
	D3DXMATRIX matBlend;
	D3DXMatrixIdentity(&matBlend);

	D3DXMATRIX matBlendShader;
	D3DXMatrixMultiply(&matBlendShader, &c_rmatView, &c_rmatProj);

	float afDirection[3];
	afDirection[0] = matBlendShader.m[0][2];
	afDirection[1] = matBlendShader.m[1][2];
	afDirection[2] = matBlendShader.m[2][2];
	CSpeedTreeRT::SetCamera(c_rEyeVec, afDirection);

	D3DXMatrixTranspose(&matBlendShader, &matBlendShader);
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_CompoundMatrix, &matBlendShader, 4);
}
void CSpeedTreeForestDirectX9::Render(uint32_t ulRenderBitVector)
{
	UpdateSystem(CTimer::Instance().GetCurrentSecond());

	if (m_pMainTreeMap.empty())
		return;

	if (!(ulRenderBitVector & Forest_RenderToShadow) && !(ulRenderBitVector & Forest_RenderToMiniMap))
		UpdateCompundMatrix(CCameraManager::Instance().GetCurrentCamera()->GetEye(), ms_matView, ms_matProj);

	uint32_t dwLightState = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	uint32_t dwColorVertexState = STATEMANAGER.GetRenderState(D3DRS_COLORVERTEX);
	uint32_t dwFogVertexMode = STATEMANAGER.GetRenderState(D3DRS_FOGVERTEXMODE);

#ifdef WRAPPER_USE_DYNAMIC_LIGHTING
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);
#else
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, TRUE);
#endif

	for (const auto &maintree : m_pMainTreeMap)
	{
		const auto &maintreeInstances = maintree.second->GetInstances();

		for (const auto &it : maintreeInstances)
			it->Advance();
	}

	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Light,	m_afLighting, 3);
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Fog, m_afFog, 1);

	if (ulRenderBitVector & Forest_RenderToShadow)
	{

		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2,	D3DTA_DIFFUSE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);
	}
	else
	{
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2,	D3DTA_DIFFUSE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_MINFILTER,	D3DTEXF_LINEAR);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_MAGFILTER,	D3DTEXF_LINEAR);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_MIPFILTER,	D3DTEXF_LINEAR);

		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	}

	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	if (STATEMANAGER.GetRenderState(D3DRS_FOGENABLE))
	{
#ifdef WRAPPER_USE_GPU_WIND
			STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE); 
#endif
	}

	// choose fixed function pipeline or custom shader for fronds and branches
	STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
	STATEMANAGER.SetVertexShader(m_branchVertexShader);

	// render branches
	if (ulRenderBitVector & Forest_RenderBranches)
	{

		for (const auto &maintree : m_pMainTreeMap)
		{
			const auto &pmaintree = maintree.second;
			const auto &maintreeInstances = pmaintree->GetInstances();

			pmaintree->SetupBranchForTreeType();

			for (const auto &it : maintreeInstances)
			{
				if (it->isShow())
					it->RenderBranches();
			}
		}
	}
	STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	if (ulRenderBitVector & Forest_RenderFronds)
	{
		for (const auto &maintree : m_pMainTreeMap)
		{
			const auto &pmaintree = maintree.second;
			const auto &maintreeInstances = pmaintree->GetInstances();

			pmaintree->SetupFrondForTreeType();

			for (const auto &it : maintreeInstances)
			{
				if (it->isShow())
					it->RenderFronds();
			}
		}
	}
	if (ulRenderBitVector & Forest_RenderLeaves)
	{
		STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_LEAF_VERTEX);
		STATEMANAGER.SetVertexShader(m_leafVertexShader);

		if (STATEMANAGER.GetRenderState(D3DRS_FOGENABLE))
		{
			#if defined WRAPPER_USE_GPU_WIND || defined WRAPPER_USE_GPU_LEAF_PLACEMENT
				STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
#endif
		}

		if (ulRenderBitVector & Forest_RenderToShadow || ulRenderBitVector & Forest_RenderToMiniMap)
		{
			STATEMANAGER.SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);
			STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0x00000000);
		}

		for (const auto &maintree : m_pMainTreeMap)
		{
			const auto &pmaintree = maintree.second;
			const auto &maintreeInstances = pmaintree->GetInstances();

			pmaintree->SetupLeafForTreeType();

			for (const auto &it : maintreeInstances)
			{
				if (it->isShow())
					it->RenderLeaves();
			}
		}

		for (const auto &maintree : m_pMainTreeMap)
		{
			const auto &pmaintree = maintree.second;
			pmaintree->EndLeafForTreeType();
		}

		if (ulRenderBitVector & Forest_RenderToShadow || ulRenderBitVector & Forest_RenderToMiniMap)
		{
			STATEMANAGER.SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
			STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);
		}
	}

	STATEMANAGER.SetVertexShader(nullptr);

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLightState);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, dwColorVertexState);
	STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, dwFogVertexMode);

	if (!(ulRenderBitVector & Forest_RenderToShadow))
	{
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	}

	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
}

