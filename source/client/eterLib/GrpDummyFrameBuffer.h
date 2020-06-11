#ifndef __GRP_DUMMY_FRAME_BUFFER_H__
#define __GRP_DUMMY_FRAME_BUFFER_H__
#include "GrpTexture.h"

/*!
 * \brief
 * Framebuffer와 같은 설정의 texture
 * 
 * Begin()과 End() 사이의 �?든 렌더링은 이 texture에 그려진다.
 * 
 * \remarks
 * UI에서 특정 윈도우의 clip을 위한 class. 
 * 
 */
class CGraphicDummpyFrameBuffer : public CGraphicTexture
{
private:
	typedef CGraphicTexture super;
public:
	CGraphicDummpyFrameBuffer();
	virtual ~CGraphicDummpyFrameBuffer();

	void Begin();
	void End();
	void RenderRect(RECT& rect);

protected:
	virtual void Initialize();
	virtual void Destroy();

	LPDIRECT3DSURFACE9 m_lpd3dDummyRenderTarget;
	LPDIRECT3DSURFACE9 m_lpd3dDummyDepthSurface;

	LPDIRECT3DSURFACE9 m_lpd3dOriginalRenderTarget;
	LPDIRECT3DSURFACE9 m_lpd3dOldDepthBufferSurface;
	bool m_bIsStateRunning;
};

#endif