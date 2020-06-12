#pragma once
#include "GrpBase.h"
#include "GrpTexture.h"

/*!
 * \brief
 * Interface of texture.
 */
class CGraphicRenderTargetTexture : public CGraphicTexture
{
	public:
		CGraphicRenderTargetTexture();
		virtual	~CGraphicRenderTargetTexture();
	public:
		bool Create (int32_t width, int32_t height, D3DFORMAT texFormat, D3DFORMAT dephtFormat);
		void CreateTextures();
		bool CreateRenderTexture (int32_t width, int32_t height, D3DFORMAT format);
		bool CreateRenderDepthStencil (int32_t width, int32_t height, D3DFORMAT format);
		void SetRenderTarget();
		void ResetRenderTarget();
		void SetRenderingRect (RECT* rect);
		RECT* GetRenderingRect();
		LPDIRECT3DTEXTURE9 GetRenderTargetTexture() const;
		void ReleaseTextures();

		void Render() const;

	protected:
		void __Initialize();


		virtual void Initialize();

	protected:
		LPDIRECT3DTEXTURE9 m_lpd3dRenderTexture;
		LPDIRECT3DSURFACE9 m_lpd3dRenderTargetSurface;
		LPDIRECT3DSURFACE9 m_lpd3dDepthSurface;

		LPDIRECT3DSURFACE9 m_lpd3dOriginalRenderTarget;
		LPDIRECT3DSURFACE9 m_lpd3dOldDepthBufferSurface;

		D3DFORMAT m_d3dFormat;
		D3DFORMAT m_depthStencilFormat;
		RECT m_renderRect;
};
