// Decal.h: interface for the CDecal class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DECAL_H__E3D27DFC_30CB_4995_B9B9_396B5E8A5F02__INCLUDED_)
#define AFX_DECAL_H__E3D27DFC_30CB_4995_B9B9_396B5E8A5F02__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GrpBase.h"

class CDecal  
{
public:

	enum
	{
		MAX_DECAL_VERTICES	= 256,
	};

	CDecal();
	virtual ~CDecal();

	void Clear();

	virtual void Make(D3DXVECTOR3 v3Center, D3DXVECTOR3 v3Normal, D3DXVECTOR3 v3Tangent, float fWidth, float fHeight, float fDepth) = 0;
// 	virtual void Update();
	virtual void Render();

protected:
	//
	D3DXVECTOR3		m_v3Center;
	D3DXVECTOR3		m_v3Normal;
	
	// Clip Plane
	D3DXPLANE		m_v4LeftPlane;
	D3DXPLANE		m_v4RightPlane;
	D3DXPLANE		m_v4BottomPlane;
	D3DXPLANE		m_v4TopPlane;
	D3DXPLANE		m_v4FrontPlane;
	D3DXPLANE		m_v4BackPlane;

	// ����
	uint32_t			m_dwVertexCount;
	uint32_t			m_dwPrimitiveCount;

	// ���ý� ���ۿ� �δ콺 ����
//	CGraphicVertexBuffer	m_GraphicVertexBuffer;
//	CGraphicIndexBuffer		m_GraphicIndexBuffer;

	// ���ý� ���ۿ� �δ콺 ���� ��ſ� �迭 ����� DrawIndexedPrimitiveUP�� �׸���.
	typedef struct 
	{
		uint16_t			m_wMinIndex;
		uint32_t			m_dwVertexCount;
		uint32_t			m_dwPrimitiveCount;
		uint32_t			m_dwVBOffset;
	} TTRIANGLEFANSTRUCT;
	
	std::vector<TTRIANGLEFANSTRUCT> m_TriangleFanStructVector;

	TPDTVertex		m_Vertices[MAX_DECAL_VERTICES];
	uint16_t			m_Indices[MAX_DECAL_VERTICES];
	
	const float m_cfDecalEpsilon;

protected:
	bool AddPolygon(uint32_t dwAddCount, const D3DXVECTOR3 *c_pv3Vertex, const D3DXVECTOR3 *c_pv3Normal);
	void ClipMesh(uint32_t dwPrimitiveCount, const D3DXVECTOR3 *c_pv3Vertex, const D3DXVECTOR3 *c_pv3Normal);
	uint32_t ClipPolygon(uint32_t dwVertexCount, 
		const D3DXVECTOR3 *c_pv3Vertex, 
		const D3DXVECTOR3 *c_pv3Normal, 
		D3DXVECTOR3 *c_pv3NewVertex, 
		D3DXVECTOR3 *c_pv3NewNormal) const;
	static uint32_t ClipPolygonAgainstPlane(const D3DXPLANE& v4Plane, 
		uint32_t dwVertexCount,
		const D3DXVECTOR3 *c_pv3Vertex, 
		const D3DXVECTOR3 *c_pv3Normal, 
		D3DXVECTOR3 *c_pv3NewVertex, 
		D3DXVECTOR3 *c_pv3NewNormal);
};
/*

class CDecalManager : public CSingleton<CDecalManager>
{
public:
	CDecalManager();
	~CDecalManager();

	void Add(CDecal * pDecal);
	void Remove(CDecal * pDecal);
	void Update();
	void Render();
	
private:
	std::vector<CDecal *>	m_DecalPtrVector;
	
};

*/
#endif // !defined(AFX_DECAL_H__E3D27DFC_30CB_4995_B9B9_396B5E8A5F02__INCLUDED_)
