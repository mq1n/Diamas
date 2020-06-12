#pragma once
#include <unordered_map>
#include "GrpBase.h"
#include "GrpScreen.h"
#include "../eterBase/Singleton.h"
#include "GrpRenderTargetTexture.h"

class CRenderTargetManager : public CGraphicBase, public CSingleton<CRenderTargetManager>
{
	public:
		CRenderTargetManager();
		virtual ~CRenderTargetManager();

		bool CreateRenderTarget(int32_t width, int32_t height);
		bool CreateRenderTargetWithIndex(int32_t width, int32_t height, uint32_t index);

		bool GetRenderTargetRect(uint32_t index, RECT& rect);
		bool ChangeRenderTarget(uint32_t index);

		void CreateRenderTargetTextures();
		void ReleaseRenderTargetTextures();

		CGraphicRenderTargetTexture* GetRenderTarget(uint32_t index);
		void ResetRenderTarget();
		void ClearRenderTarget() const;
		void Destroy();
	
	private:
		bool CreateGraphicTexture(uint32_t index, uint32_t width, uint32_t height, D3DFORMAT texFormat, D3DFORMAT dephtFormat);

		std::unordered_map<uint32_t, CGraphicRenderTargetTexture*> m_renderTargets;
		uint32_t m_widht;
		uint32_t m_height;
		CGraphicRenderTargetTexture* m_currentRenderTarget;
};
