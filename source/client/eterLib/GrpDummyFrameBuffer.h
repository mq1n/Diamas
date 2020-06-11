#ifndef __GRP_DUMMY_FRAME_BUFFER_H__
#define __GRP_DUMMY_FRAME_BUFFER_H__
#include "GrpTexture.h"

/*!
 * \brief
 * Framebuffer�� ���� ������ texture
 * 
 * Begin()�� End() ������ �?�� �������� �� texture�� �׷�����.
 * 
 * \remarks
 * UI���� Ư�� �������� clip�� ���� class. 
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