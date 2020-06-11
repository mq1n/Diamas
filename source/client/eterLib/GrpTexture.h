#pragma once

#include "GrpBase.h"

class CGraphicTexture : public CGraphicBase
{
	public:
		virtual bool IsEmpty() const;

		int32_t GetWidth() const;
		int32_t GetHeight() const;

		void SetTextureStage(int32_t stage) const;
		LPDIRECT3DTEXTURE9 GetD3DTexture() const;

		void DestroyDeviceObjects();
		
	protected:
		CGraphicTexture();
		virtual	~CGraphicTexture();

		void Destroy();
		void Initialize();

	protected:
		bool m_bEmpty;

		int32_t m_width;
		int32_t m_height;

		LPDIRECT3DTEXTURE9 m_lpd3dTexture;
};
