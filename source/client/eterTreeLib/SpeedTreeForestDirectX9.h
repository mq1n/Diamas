
#pragma once
#define SPEEDTREE_DATA_FORMAT_DIRECTX

#include "SpeedTreeForest.h"
#include "SpeedTreeMaterial.h"
class CSpeedTreeForestDirectX9 : public CSpeedTreeForest, public CGraphicBase, public CSingleton<CSpeedTreeForestDirectX9>
{
	public:
		CSpeedTreeForestDirectX9();
		virtual ~CSpeedTreeForestDirectX9();

		void			UploadWindMatrix(uint32_t uiLocation, const float* pMatrix) const;
		void			UpdateCompundMatrix(const D3DXVECTOR3 & c_rEyeVec, const D3DXMATRIX & c_rmatView, const D3DXMATRIX & c_rmatProj);

		void			Render(uint32_t ulRenderBitVector = Forest_RenderAll);
		bool			SetRenderingDevice(LPDIRECT3DDEVICE9 pDevice);

	private:
		bool			InitVertexShaders();

	private:
		LPDIRECT3DDEVICE9		m_pDx;

		IDirect3DVertexShader9* m_branchVertexShader; // branch/frond vertex shaders
		IDirect3DVertexShader9* m_leafVertexShader; // leaf vertex shader
};
