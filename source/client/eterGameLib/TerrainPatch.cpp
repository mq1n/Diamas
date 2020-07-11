// TerrainPatch.cpp: implementation of the CTerrainPatch class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TerrainPatch.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CTerrainPatch::Clear()
{
	m_kHT.m_kVB.Destroy();
	
	m_WaterVertexBuffer.Destroy();
	ClearID();
	SetUse(false);

	m_bWaterExist = false;
	m_bNeedUpdate = true;

	m_dwWaterPriCount = 0;
	m_byType = PATCH_TYPE_PLAIN;
	
	m_fMinX = m_fMaxX = m_fMinY = m_fMaxY = m_fMinZ = m_fMaxZ = 0.0f;

	m_dwVersion=0;
}

void CTerrainPatch::BuildWaterVertexBuffer(SWaterVertex* akSrcVertex, uint32_t uWaterVertexCount)
{
	CGraphicVertexBuffer& rkVB=m_WaterVertexBuffer;

	if (!rkVB.Create(uWaterVertexCount, D3DFVF_XYZ | D3DFVF_DIFFUSE, D3DUSAGE_WRITEONLY, D3DPOOL_MANAGED)) 
		return;
	
	SWaterVertex* akDstWaterVertex;
	if (rkVB.Lock((void **) &akDstWaterVertex))
	{
		uint32_t uVBSize=sizeof(SWaterVertex)*uWaterVertexCount;
		memcpy(akDstWaterVertex, akSrcVertex, uVBSize);
		m_dwWaterPriCount=uWaterVertexCount/3;

		rkVB.Unlock();		
	}	
}
		
void CTerrainPatch::BuildTerrainVertexBuffer(HardwareTransformPatch_SSourceVertex* akSrcVertex)
{
	__BuildHardwareTerrainVertexBuffer(akSrcVertex);
}

void CTerrainPatch::__BuildHardwareTerrainVertexBuffer(HardwareTransformPatch_SSourceVertex* akSrcVertex)
{
	
	CGraphicVertexBuffer& rkVB=m_kHT.m_kVB;
	if (!rkVB.Create(TERRAIN_VERTEX_COUNT, D3DFVF_XYZ | D3DFVF_NORMAL, D3DUSAGE_WRITEONLY, D3DPOOL_MANAGED)) 
		return;
	
	HardwareTransformPatch_SSourceVertex* akDstVertex;
	if (rkVB.Lock((void **) &akDstVertex))
	{
		uint32_t uVBSize=sizeof(HardwareTransformPatch_SSourceVertex)*TERRAIN_VERTEX_COUNT;
	
		memcpy(akDstVertex, akSrcVertex, uVBSize);
		rkVB.Unlock();		
	}
}

uint32_t CTerrainPatch::GetWaterFaceCount() const
{
	return m_dwWaterPriCount;
}

CTerrainPatchProxy::CTerrainPatchProxy()
{
	Clear();
}

CTerrainPatchProxy::~CTerrainPatchProxy()
{
	Clear();
}

void CTerrainPatchProxy::SetCenterPosition(const D3DXVECTOR3& c_rv3Center)
{
	m_v3Center=c_rv3Center;
}

bool CTerrainPatchProxy::IsIn(const D3DXVECTOR3& c_rv3Target, float fRadius)
{
	float dx=m_v3Center.x-c_rv3Target.x;
	float dy=m_v3Center.y-c_rv3Target.y;
	float fDist=dx*dx+dy*dy;
	float fCheck=fRadius*fRadius;

	if (fDist<fCheck)
		return true;

	return false;
}

CGraphicVertexBuffer* CTerrainPatchProxy::HardwareTransformPatch_GetVertexBufferPtr() const
{
	if (m_pTerrainPatch)
		return m_pTerrainPatch->HardwareTransformPatch_GetVertexBufferPtr();

	return nullptr;
}

uint32_t CTerrainPatchProxy::GetWaterFaceCount()
{
	if (m_pTerrainPatch)
		return m_pTerrainPatch->GetWaterFaceCount();
	
	return 0;
}

void CTerrainPatchProxy::Clear()
{
	m_bUsed = false;
	m_sPatchNum = 0;
	m_byTerrainNum = 0xFF;

	m_pTerrainPatch = nullptr;
}
