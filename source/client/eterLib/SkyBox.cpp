#include "StdAfx.h"
#include "SkyBox.h"
#include "Camera.h"
#include "StateManager.h"
#include "ResourceManager.h"

#include "../eterBase/Timer.h"

//////////////////////////////////////////////////////////////////////////
// CSkyObjectQuad
//////////////////////////////////////////////////////////////////////////

CSkyObjectQuad::CSkyObjectQuad()
{
	// Index buffer
	m_Indices[0] = 0;
	m_Indices[1] = 2;
	m_Indices[2] = 1;
	m_Indices[3] = 3;

	for (uint8_t uci = 0; uci < 4; ++uci)
	{
		memset(&m_Vertex[uci], 0, sizeof(TPDTVertex));
	}
}

CSkyObjectQuad::~CSkyObjectQuad()
{
}

void CSkyObjectQuad::Clear(const uint8_t & c_rucNumVertex,
						   const float & c_rfRed,
						   const float & c_rfGreen,
						   const float & c_rfBlue,
						   const float & c_rfAlpha)
{
	if (c_rucNumVertex > 3)
		return;
	m_Helper[c_rucNumVertex].Clear(c_rfRed, c_rfGreen, c_rfBlue, c_rfAlpha);
}

void CSkyObjectQuad::SetSrcColor(const uint8_t & c_rucNumVertex,
								 const float & c_rfRed,
								 const float & c_rfGreen,
								 const float & c_rfBlue,
								 const float & c_rfAlpha)
{
	if (c_rucNumVertex > 3)
		return;
	m_Helper[c_rucNumVertex].SetSrcColor(c_rfRed, c_rfGreen, c_rfBlue, c_rfAlpha);
}

void CSkyObjectQuad::SetTransition(const uint8_t & c_rucNumVertex,
								   const float & c_rfRed,
								   const float & c_rfGreen,
								   const float & c_rfBlue,
								   const float & c_rfAlpha,
								   uint32_t dwDuration)
{
	if (c_rucNumVertex > 3)
		return;
	m_Helper[c_rucNumVertex].SetTransition(c_rfRed, c_rfGreen, c_rfBlue, c_rfAlpha, dwDuration);
}

void CSkyObjectQuad::SetVertex(const uint8_t & c_rucNumVertex, const TPDTVertex & c_rPDTVertex)
{
	if (c_rucNumVertex > 3)
		return;
	memcpy (&m_Vertex[m_Indices[c_rucNumVertex]], &c_rPDTVertex, sizeof(TPDTVertex)); 
}

void CSkyObjectQuad::StartTransition()
{
	for (uint8_t uci = 0; uci < 4; ++uci)
	{
		m_Helper[uci].StartTransition();
	}
}

bool CSkyObjectQuad::Update()
{
	bool bResult = false;
	for (uint8_t uci = 0; uci < 4; ++uci)
	{
		bResult = m_Helper[uci].Update() || bResult;
		m_Vertex[m_Indices[uci]].diffuse = m_Helper[uci].GetCurColor();
	}
 	return bResult;
}

void CSkyObjectQuad::Render()
{
	if (CGraphicBase::SetPDTStream(m_Vertex, 4))
		STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	//STATEMANAGER.DrawIndexedPrimitiveUP(D3DPT_TRIANGLESTRIP, 0, 4, 2, m_Indices, D3DFMT_INDEX16, &m_Vertex, sizeof(TPDTVertex));
}

//////////////////////////////////////////////////////////////////////////
// CSkyObject
/////////////////////////////////////////////////////////////////////////
CSkyObject::CSkyObject() :
	m_fCloudScaleX(0), m_fCloudScaleY(0), m_fCloudHeight(0),
	m_fCloudTextureScaleX(0), m_fCloudTextureScaleY(0), m_fCloudScrollSpeedU(0),
	m_fCloudScrollSpeedV(0), m_fCloudPositionU(0.0f), m_fCloudPositionV(0.0f),
	m_v3Position(0.0f, 0.0f, 0.0f), m_fScaleX(1.0f), m_fScaleY(1.0f),
	m_fScaleZ(1.0f), m_ucRenderMode(0), m_bTransitionStarted(false),
	m_bSkyMatrixUpdated(false)
{
	D3DXMatrixIdentity(&m_matWorld);
	D3DXMatrixIdentity(&m_matTranslation);
	D3DXMatrixIdentity(&m_matTextureCloud);

	m_dwlastTime = CTimer::Instance().GetCurrentMillisecond();
}

CSkyObject::~CSkyObject()
{
	Destroy();
}

void CSkyObject::Destroy()
{
}

void CSkyObject::Update()
{
	D3DXVECTOR3 v3Eye = CCameraManager::Instance().GetCurrentCamera()->GetEye();

	if (m_v3Position == v3Eye)
		if (m_bSkyMatrixUpdated == false)
			return;

	m_v3Position = v3Eye;

	m_matWorld._41 = m_v3Position.x;
	m_matWorld._42 = m_v3Position.y;
	m_matWorld._43 = m_v3Position.z;

	m_matWorldCloud._41 = m_v3Position.x;
	m_matWorldCloud._42 = m_v3Position.y;
	m_matWorldCloud._43 = m_v3Position.z + m_fCloudHeight;

	if (m_bSkyMatrixUpdated)
		m_bSkyMatrixUpdated = false;
}

void CSkyObject::Render()
{
}

CGraphicImageInstance * CSkyObject::GenerateTexture(const char * szfilename)
{
	assert(szfilename != nullptr);

	if (strlen(szfilename) <= 0)
		assert(false);

	CResource* pResource = CResourceManager::Instance().GetResourcePointer<CResource>(szfilename);

	if (!pResource->IsType(CGraphicImage::Type()))
	{
		assert(false);
		return nullptr;
	}

	CGraphicImageInstance * pImageInstance = CGraphicImageInstance::New();
	pImageInstance->SetImagePointer(static_cast<CGraphicImage *>(pResource));
	return (pImageInstance);
}

void CSkyObject::DeleteTexture(CGraphicImageInstance * pImageInstance)
{
	if (pImageInstance)
		CGraphicImageInstance::Delete(pImageInstance);
}

void CSkyObject::StartTransition()
{
}

//////////////////////////////////////////////////////////////////////////
// CSkyObject::TSkyObjectFace
//////////////////////////////////////////////////////////////////////////

void CSkyObject::TSkyObjectFace::StartTransition()
{
	for (auto & uci : m_SkyObjectQuadVector)
		uci.StartTransition();
}

bool CSkyObject::TSkyObjectFace::Update()
{
	bool bResult = false;
	for (auto & dwi : m_SkyObjectQuadVector)
		bResult = dwi.Update() || bResult;
 	return bResult;
}

void CSkyObject::TSkyObjectFace::Render()
{
	for (auto & uci : m_SkyObjectQuadVector)
		uci.Render();
}

//////////////////////////////////////////////////////////////////////////
// CSkyBox
//////////////////////////////////////////////////////////////////////////

CSkyBox::CSkyBox()
{
	m_ucVirticalGradientLevelUpper = 0;
	m_ucVirticalGradientLevelLower = 0;
}

CSkyBox::~CSkyBox()
{
	Destroy();
}

void CSkyBox::Destroy()
{
	Unload();
}

void CSkyBox::SetSkyBoxScale(const D3DXVECTOR3 & c_rv3Scale)
{
	m_fScaleX = c_rv3Scale.x;
	m_fScaleY = c_rv3Scale.y;
	m_fScaleZ = c_rv3Scale.z;

	m_bSkyMatrixUpdated = true;
	D3DXMatrixScaling(&m_matWorld, m_fScaleX, m_fScaleY, m_fScaleZ);
}

void CSkyBox::SetGradientLevel(uint8_t byUpper, uint8_t byLower)
{
	m_ucVirticalGradientLevelUpper = byUpper;
	m_ucVirticalGradientLevelLower = byLower;
}

void CSkyBox::SetFaceTexture( const char* c_szFileName, int32_t iFaceIndex )
{
	if( iFaceIndex < 0 || iFaceIndex > 5 ) 
		return;

	auto itor = m_GraphicImageInstanceMap.find(c_szFileName);
	if (m_GraphicImageInstanceMap.end() != itor)
		return;

	m_Faces[iFaceIndex].m_strFaceTextureFileName = c_szFileName;

	CGraphicImageInstance * pGraphicImageInstance = GenerateTexture(c_szFileName);
	m_GraphicImageInstanceMap.emplace(c_szFileName, pGraphicImageInstance);
}


void CSkyBox::SetCloudTexture(const char * c_szFileName)
{
	auto itor = m_GraphicImageInstanceMap.find(c_szFileName);
	if (m_GraphicImageInstanceMap.end() != itor)
		return;

	m_FaceCloud.m_strfacename = c_szFileName;
	CGraphicImageInstance * pGraphicImageInstance = GenerateTexture(c_szFileName);
	m_GraphicImageInstanceMap.insert(TGraphicImageInstanceMap::value_type(m_FaceCloud.m_strfacename, pGraphicImageInstance));

	// 이거 안쓰는거 같은데요? [cronan]
//	CGraphicImage * pImage = (CGraphicImage *) CResourceManager::Instance().GetResourcePointer("D:\\Ymir Work\\special/cloudalpha.tga");
//	m_CloudAlphaImageInstance.SetImagePointer(pImage);
}

void CSkyBox::SetCloudScale(const D3DXVECTOR2 & c_rv2CloudScale)
{
	m_fCloudScaleX = c_rv2CloudScale.x;
	m_fCloudScaleY = c_rv2CloudScale.y;

	D3DXMatrixScaling(&m_matWorldCloud, m_fCloudScaleX, m_fCloudScaleY, 1.0f);
}

void CSkyBox::SetCloudHeight(float fHeight)
{
	m_fCloudHeight = fHeight;
}

void CSkyBox::SetCloudTextureScale(const D3DXVECTOR2 & c_rv2CloudTextureScale)
{
	m_fCloudTextureScaleX = c_rv2CloudTextureScale.x;
	m_fCloudTextureScaleY = c_rv2CloudTextureScale.y;

	m_matTextureCloud._11 = m_fCloudTextureScaleX;
	m_matTextureCloud._22 = m_fCloudTextureScaleY;
}

void CSkyBox::SetCloudScrollSpeed(const D3DXVECTOR2 & c_rv2CloudScrollSpeed)
{
	m_fCloudScrollSpeedU = c_rv2CloudScrollSpeed.x;
	m_fCloudScrollSpeedV = c_rv2CloudScrollSpeed.y;
}

void CSkyBox::Unload()
{
	auto itor = m_GraphicImageInstanceMap.begin();

	while (itor != m_GraphicImageInstanceMap.end())
	{
		DeleteTexture(itor->second);
		++itor;
	}

	m_GraphicImageInstanceMap.clear();
}

void CSkyBox::SetSkyObjectQuadVertical(TSkyObjectQuadVector * pSkyObjectQuadVector, const D3DXVECTOR2 * c_pv2QuadPoints)
{
	TPDTVertex aPDTVertex;

	uint32_t dwIndex = 0;

	pSkyObjectQuadVector->clear();
	pSkyObjectQuadVector->resize(m_ucVirticalGradientLevelUpper + m_ucVirticalGradientLevelLower);

	uint8_t ucY;
	for (ucY = 0; ucY < m_ucVirticalGradientLevelUpper; ++ucY)
	{
		CSkyObjectQuad & rSkyObjectQuad = pSkyObjectQuadVector->at(dwIndex++);

		aPDTVertex.position.x = c_pv2QuadPoints[0].x;
		aPDTVertex.position.y = c_pv2QuadPoints[0].y;
		aPDTVertex.position.z = 1.0f - (float)(ucY + 1)/ (float)(m_ucVirticalGradientLevelUpper); 
		aPDTVertex.texCoord.x = 0.0f;
		aPDTVertex.texCoord.y = (float)(ucY + 1)/ (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
		rSkyObjectQuad.SetVertex(0 , aPDTVertex);
		aPDTVertex.position.x = c_pv2QuadPoints[0].x;
		aPDTVertex.position.y = c_pv2QuadPoints[0].y;
		aPDTVertex.position.z = 1.0f - (float)(ucY) / (float)(m_ucVirticalGradientLevelUpper); 
		aPDTVertex.texCoord.x = 0.0f;
		aPDTVertex.texCoord.y = (float)(ucY)/ (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
		rSkyObjectQuad.SetVertex(1, aPDTVertex);
		aPDTVertex.position.x = c_pv2QuadPoints[1].x;
		aPDTVertex.position.y = c_pv2QuadPoints[1].y;
		aPDTVertex.position.z = 1.0f - (float)(ucY + 1) / (float)(m_ucVirticalGradientLevelUpper); 
		aPDTVertex.texCoord.x = 1.0f;
		aPDTVertex.texCoord.y = (float)(ucY + 1)/ (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
		rSkyObjectQuad.SetVertex(2, aPDTVertex);
		aPDTVertex.position.x = c_pv2QuadPoints[1].x;
		aPDTVertex.position.y = c_pv2QuadPoints[1].y;
		aPDTVertex.position.z = 1.0f - (float)(ucY) / (float)(m_ucVirticalGradientLevelUpper); 
		aPDTVertex.texCoord.x = 1.0f;
		aPDTVertex.texCoord.y = (float)(ucY)/ (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
		rSkyObjectQuad.SetVertex(3, aPDTVertex);
	}
	for (ucY = 0; ucY < m_ucVirticalGradientLevelLower; ++ucY)
	{
		CSkyObjectQuad & rSkyObjectQuad = pSkyObjectQuadVector->at(dwIndex++);

		aPDTVertex.position.x = c_pv2QuadPoints[0].x;
		aPDTVertex.position.y = c_pv2QuadPoints[0].y;
		aPDTVertex.position.z = -(float)(ucY + 1)/ (float)(m_ucVirticalGradientLevelLower); 
		aPDTVertex.texCoord.x = 0.0f;
		aPDTVertex.texCoord.y = 0.5f + (float)(ucY + 1)/ (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
		rSkyObjectQuad.SetVertex(0, aPDTVertex);
		aPDTVertex.position.x = c_pv2QuadPoints[0].x;
		aPDTVertex.position.y = c_pv2QuadPoints[0].y;
		aPDTVertex.position.z = -(float)(ucY) / (float)(m_ucVirticalGradientLevelLower);
		aPDTVertex.texCoord.x = 0.0f;
		aPDTVertex.texCoord.y = 0.5f + (float)(ucY)/ (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
		rSkyObjectQuad.SetVertex(1, aPDTVertex);
		aPDTVertex.position.x = c_pv2QuadPoints[1].x;
		aPDTVertex.position.y = c_pv2QuadPoints[1].y;
		aPDTVertex.position.z = -(float)(ucY + 1) / (float)(m_ucVirticalGradientLevelLower); 
		aPDTVertex.texCoord.x = 1.0f;
		aPDTVertex.texCoord.y = 0.5f + (float)(ucY + 1)/ (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
		rSkyObjectQuad.SetVertex(2, aPDTVertex);
		aPDTVertex.position.x = c_pv2QuadPoints[1].x;
		aPDTVertex.position.y = c_pv2QuadPoints[1].y;
		aPDTVertex.position.z = -(float)(ucY) / (float)(m_ucVirticalGradientLevelLower); 
		aPDTVertex.texCoord.x = 1.0f;
		aPDTVertex.texCoord.y = 0.5f + (float)(ucY)/ (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
		rSkyObjectQuad.SetVertex(3, aPDTVertex);
	}
}

//void CSkyBox::UpdateSkyFaceQuadTransform(D3DXVECTOR3 * c_pv3QuadPoints)
//{
//	for( int32_t i = 0; i < 4; ++i )
//	{
//		c_pv3QuadPoints[i].x *= m_fScaleX;	
//		c_pv3QuadPoints[i].y *= m_fScaleY;	
//		c_pv3QuadPoints[i].z *= m_fScaleZ;	
//
//		c_pv3QuadPoints[i] += m_v3Position;
//	}
//}

void CSkyBox::SetSkyObjectQuadHorizon(TSkyObjectQuadVector * pSkyObjectQuadVector, const D3DXVECTOR3 * c_pv3QuadPoints)
{
	pSkyObjectQuadVector->clear();
	pSkyObjectQuadVector->resize(1);
	CSkyObjectQuad & rSkyObjectQuad = pSkyObjectQuadVector->at(0);

	TPDTVertex aPDTVertex;
	aPDTVertex.position		= c_pv3QuadPoints[0];
	aPDTVertex.texCoord.x	= 0.0f;
	aPDTVertex.texCoord.y	= 1.0f;
	rSkyObjectQuad.SetVertex(0, aPDTVertex);

	aPDTVertex.position		= c_pv3QuadPoints[1];
	aPDTVertex.texCoord.x	= 0.0f;
	aPDTVertex.texCoord.y	= 0.0f;
	rSkyObjectQuad.SetVertex(1, aPDTVertex);

	aPDTVertex.position		= c_pv3QuadPoints[2];
	aPDTVertex.texCoord.x	= 1.0f;
	aPDTVertex.texCoord.y	= 1.0f;
	rSkyObjectQuad.SetVertex(2, aPDTVertex);

	aPDTVertex.position		= c_pv3QuadPoints[3];
	aPDTVertex.texCoord.x	= 1.0f;
	aPDTVertex.texCoord.y	= 0.0f;
	rSkyObjectQuad.SetVertex(3, aPDTVertex);
}

void CSkyBox::Refresh()
{
	D3DXVECTOR3 v3QuadPoints[4];

	if( m_ucRenderMode == CSkyObject::SKY_RENDER_MODE_DEFAULT ||  m_ucRenderMode == CSkyObject::SKY_RENDER_MODE_DIFFUSE )
	{
		if (m_ucVirticalGradientLevelUpper + m_ucVirticalGradientLevelLower <= 0)
			return;

		D3DXVECTOR2 v2QuadPoints[2];

		//// Face 0: FRONT
		v2QuadPoints[0] = D3DXVECTOR2(1.0f, -1.0f);
		v2QuadPoints[1] = D3DXVECTOR2(-1.0f, -1.0f);
		SetSkyObjectQuadVertical(&m_Faces[0].m_SkyObjectQuadVector, v2QuadPoints);
		m_Faces[0].m_strfacename = "front";

		//// Face 1: BACK
		v2QuadPoints[0] = D3DXVECTOR2(-1.0f, 1.0f);
		v2QuadPoints[1] = D3DXVECTOR2(1.0f, 1.0f);
		SetSkyObjectQuadVertical(&m_Faces[1].m_SkyObjectQuadVector, v2QuadPoints);
		m_Faces[1].m_strfacename = "back";

		//// Face 2: LEFT
		v2QuadPoints[0] = D3DXVECTOR2(-1.0f, -1.0f);
		v2QuadPoints[1] = D3DXVECTOR2(-1.0f, 1.0f);
		SetSkyObjectQuadVertical(&m_Faces[2].m_SkyObjectQuadVector, v2QuadPoints);
		m_Faces[2].m_strfacename = "left";

		//// Face 3: RIGHT
		v2QuadPoints[0] = D3DXVECTOR2(1.0f, 1.0f);
		v2QuadPoints[1] = D3DXVECTOR2(1.0f, -1.0f);
		SetSkyObjectQuadVertical(&m_Faces[3].m_SkyObjectQuadVector, v2QuadPoints);
		m_Faces[3].m_strfacename = "right";

		//// Face 4: TOP
		v3QuadPoints[0] = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
		v3QuadPoints[1] = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);
		v3QuadPoints[2] = D3DXVECTOR3(1.0f, -1.0f, 1.0f);
		v3QuadPoints[3] = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
		SetSkyObjectQuadHorizon(&m_Faces[4].m_SkyObjectQuadVector, v3QuadPoints);
		m_Faces[4].m_strfacename = "top";

		//// Face 5: BOTTOM
		v3QuadPoints[0] = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);
		v3QuadPoints[1] = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
		v3QuadPoints[2] = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
		v3QuadPoints[3] = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
		SetSkyObjectQuadHorizon(&m_Faces[5].m_SkyObjectQuadVector, v3QuadPoints);
		m_Faces[5].m_strfacename = "bottom";

	}
	else if( m_ucRenderMode == CSkyObject::SKY_RENDER_MODE_TEXTURE )
	{
		// Face 0: FRONT
		v3QuadPoints[0] = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
		v3QuadPoints[1] = D3DXVECTOR3(1.0f, -1.0f, 1.0f);
		v3QuadPoints[2] = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
		v3QuadPoints[3] = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);

		//UpdateSkyFaceQuadTransform(v3QuadPoints);

		SetSkyObjectQuadHorizon(&m_Faces[0].m_SkyObjectQuadVector, v3QuadPoints);
		m_Faces[0].m_strfacename = "front";

		//// Face 1: BACK
		v3QuadPoints[0] = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);
		v3QuadPoints[1] = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);
		v3QuadPoints[2] = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
		v3QuadPoints[3] = D3DXVECTOR3(1.0f, 1.0f, 1.0f);

		//UpdateSkyFaceQuadTransform(v3QuadPoints);
		
		SetSkyObjectQuadHorizon(&m_Faces[1].m_SkyObjectQuadVector, v3QuadPoints);
		m_Faces[1].m_strfacename = "back";

		// Face 2: LEFT
		v3QuadPoints[0] = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
		v3QuadPoints[1] = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
		v3QuadPoints[2] = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
		v3QuadPoints[3] = D3DXVECTOR3(1.0f, -1.0f, 1.0f);

		//UpdateSkyFaceQuadTransform(v3QuadPoints);

		SetSkyObjectQuadHorizon(&m_Faces[2].m_SkyObjectQuadVector, v3QuadPoints);
		m_Faces[2].m_strfacename = "left";

		// Face 3: RIGHT
		v3QuadPoints[0] = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
		v3QuadPoints[1] = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
		v3QuadPoints[2] = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);
		v3QuadPoints[3] = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);
		
		//UpdateSkyFaceQuadTransform(v3QuadPoints);
		
		SetSkyObjectQuadHorizon(&m_Faces[3].m_SkyObjectQuadVector, v3QuadPoints);
		m_Faces[3].m_strfacename = "right";

		// Face 4: TOP
		v3QuadPoints[0] = D3DXVECTOR3(1.0f, -1.0f, 1.0f); 
		v3QuadPoints[1] = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
		v3QuadPoints[2] = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
		v3QuadPoints[3] = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);

		//UpdateSkyFaceQuadTransform(v3QuadPoints);

		SetSkyObjectQuadHorizon(&m_Faces[4].m_SkyObjectQuadVector, v3QuadPoints);
		m_Faces[4].m_strfacename = "top";

		////// Face 5: BOTTOM
		// v3QuadPoints[0] = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
		// v3QuadPoints[1] = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
		// v3QuadPoints[2] = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
		// v3QuadPoints[3] = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);
		v3QuadPoints[0] = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
		v3QuadPoints[1] = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
		v3QuadPoints[2] = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
		v3QuadPoints[3] = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);

		//UpdateSkyFaceQuadTransform(v3QuadPoints);
		
		SetSkyObjectQuadHorizon(&m_Faces[5].m_SkyObjectQuadVector, v3QuadPoints);
		m_Faces[5].m_strfacename = "bottom";
	}

	//// Clouds..
	v3QuadPoints[0] = D3DXVECTOR3(1.0f, 1.0f, 0.0f);
	v3QuadPoints[1] = D3DXVECTOR3(-1.0f, 1.0f, 0.0f);
	v3QuadPoints[2] = D3DXVECTOR3(1.0f, -1.0f, 0.0f);
	v3QuadPoints[3] = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
	SetSkyObjectQuadHorizon(&m_FaceCloud.m_SkyObjectQuadVector, v3QuadPoints);
}

void CSkyBox::SetCloudColor(const TGradientColor & c_rColor, const TGradientColor & c_rNextColor, const uint32_t & dwTransitionTime)
{
	TSkyObjectFace & aFaceCloud = m_FaceCloud;
	for (uint32_t dwk = 0; dwk < aFaceCloud.m_SkyObjectQuadVector.size(); ++dwk)
	{
		CSkyObjectQuad & aSkyObjectQuad = aFaceCloud.m_SkyObjectQuadVector[dwk];
		
		aSkyObjectQuad.SetSrcColor(0,
			c_rColor.m_FirstColor.r,
			c_rColor.m_FirstColor.g,
			c_rColor.m_FirstColor.b,
			c_rColor.m_FirstColor.a);
		aSkyObjectQuad.SetTransition(0, 
			c_rNextColor.m_FirstColor.r,
			c_rNextColor.m_FirstColor.g,
			c_rNextColor.m_FirstColor.b,
			c_rNextColor.m_FirstColor.a,
			dwTransitionTime);
		aSkyObjectQuad.SetSrcColor(1,
			c_rColor.m_FirstColor.r,
			c_rColor.m_FirstColor.g,
			c_rColor.m_FirstColor.b,
			c_rColor.m_FirstColor.a);
		aSkyObjectQuad.SetTransition(1,
			c_rNextColor.m_FirstColor.r,
			c_rNextColor.m_FirstColor.g,
			c_rNextColor.m_FirstColor.b,
			c_rNextColor.m_FirstColor.a,
			dwTransitionTime);
		aSkyObjectQuad.SetSrcColor(2,
			c_rColor.m_FirstColor.r,
			c_rColor.m_FirstColor.g,
			c_rColor.m_FirstColor.b,
			c_rColor.m_FirstColor.a);
		aSkyObjectQuad.SetTransition(2,
			c_rNextColor.m_FirstColor.r,
			c_rNextColor.m_FirstColor.g,
			c_rNextColor.m_FirstColor.b,
			c_rNextColor.m_FirstColor.a,
			dwTransitionTime);
		aSkyObjectQuad.SetSrcColor(3,
			c_rColor.m_FirstColor.r,
			c_rColor.m_FirstColor.g,
			c_rColor.m_FirstColor.b,
			c_rColor.m_FirstColor.a);
		aSkyObjectQuad.SetTransition(3,
			c_rNextColor.m_FirstColor.r,
			c_rNextColor.m_FirstColor.g,
			c_rNextColor.m_FirstColor.b,
			c_rNextColor.m_FirstColor.a,
			dwTransitionTime);
	}
}

void CSkyBox::SetSkyColor(const TVectorGradientColor & c_rColorVector, const TVectorGradientColor & c_rNextColorVector, int32_t lTransitionTime)
{
	uint32_t ulVectorGradientColornum = 0;
	uint32_t uck;
	for (uint8_t ucj = 0; ucj < 4; ++ucj)
	{
		TSkyObjectFace & aFace = m_Faces[ucj];
		ulVectorGradientColornum = 0;
		for (uck = 0; uck < aFace.m_SkyObjectQuadVector.size(); ++uck)
		{
			CSkyObjectQuad & aSkyObjectQuad = aFace.m_SkyObjectQuadVector[uck];

			TGradientColor colorVector = c_rColorVector[ulVectorGradientColornum];
			TGradientColor nextColorVector = c_rNextColorVector[ulVectorGradientColornum];

			aSkyObjectQuad.SetSrcColor(0,
				colorVector.m_SecondColor.r,
				colorVector.m_SecondColor.g,
				colorVector.m_SecondColor.b,
				colorVector.m_SecondColor.a);
			aSkyObjectQuad.SetTransition(0, 
				nextColorVector.m_SecondColor.r,
				nextColorVector.m_SecondColor.g,
				nextColorVector.m_SecondColor.b,
				nextColorVector.m_SecondColor.a,
				lTransitionTime);
			aSkyObjectQuad.SetSrcColor(1,
				colorVector.m_FirstColor.r,
				colorVector.m_FirstColor.g,
				colorVector.m_FirstColor.b,
				colorVector.m_FirstColor.a);
			aSkyObjectQuad.SetTransition(1,
				nextColorVector.m_FirstColor.r,
				nextColorVector.m_FirstColor.g,
				nextColorVector.m_FirstColor.b,
				nextColorVector.m_FirstColor.a,
				lTransitionTime);
			aSkyObjectQuad.SetSrcColor(2,
				colorVector.m_SecondColor.r,
				colorVector.m_SecondColor.g,
				colorVector.m_SecondColor.b,
				colorVector.m_SecondColor.a);
			aSkyObjectQuad.SetTransition(2,
				nextColorVector.m_SecondColor.r,
				nextColorVector.m_SecondColor.g,
				nextColorVector.m_SecondColor.b,
				nextColorVector.m_SecondColor.a,
				lTransitionTime);
			aSkyObjectQuad.SetSrcColor(3,
				colorVector.m_FirstColor.r,
				colorVector.m_FirstColor.g,
				colorVector.m_FirstColor.b,
				colorVector.m_FirstColor.a);
			aSkyObjectQuad.SetTransition(3,
				nextColorVector.m_FirstColor.r,
				nextColorVector.m_FirstColor.g,
				nextColorVector.m_FirstColor.b,
				nextColorVector.m_FirstColor.a,
				lTransitionTime);

			ulVectorGradientColornum++;
		}
	}

	/////

	TSkyObjectFace & aFaceTop = m_Faces[4];

	//instead of ulVectorGradientColornum = 0 we use [0] here
	TGradientColor zeroColorVector = c_rColorVector[0]; 
	TGradientColor zeroNextColorVector = c_rNextColorVector[0];

	for (uck = 0; uck < aFaceTop.m_SkyObjectQuadVector.size(); ++uck)
	{
		CSkyObjectQuad & aSkyObjectQuad = aFaceTop.m_SkyObjectQuadVector[uck];

		aSkyObjectQuad.SetSrcColor(0,
			zeroColorVector.m_FirstColor.r,
			zeroColorVector.m_FirstColor.g,
			zeroColorVector.m_FirstColor.b,
			zeroColorVector.m_FirstColor.a);
		aSkyObjectQuad.SetTransition(0, 
			zeroNextColorVector.m_FirstColor.r,
			zeroNextColorVector.m_FirstColor.g,
			zeroNextColorVector.m_FirstColor.b,
			zeroNextColorVector.m_FirstColor.a,
			lTransitionTime);
		aSkyObjectQuad.SetSrcColor(1,
			zeroColorVector.m_FirstColor.r,
			zeroColorVector.m_FirstColor.g,
			zeroColorVector.m_FirstColor.b,
			zeroColorVector.m_FirstColor.a);
		aSkyObjectQuad.SetTransition(1,
			zeroNextColorVector.m_FirstColor.r,
			zeroNextColorVector.m_FirstColor.g,
			zeroNextColorVector.m_FirstColor.b,
			zeroNextColorVector.m_FirstColor.a,
			lTransitionTime);
		aSkyObjectQuad.SetSrcColor(2,
			zeroColorVector.m_FirstColor.r,
			zeroColorVector.m_FirstColor.g,
			zeroColorVector.m_FirstColor.b,
			zeroColorVector.m_FirstColor.a);
		aSkyObjectQuad.SetTransition(2,
			zeroNextColorVector.m_FirstColor.r,
			zeroNextColorVector.m_FirstColor.g,
			zeroNextColorVector.m_FirstColor.b,
			zeroNextColorVector.m_FirstColor.a,
			lTransitionTime);
		aSkyObjectQuad.SetSrcColor(3,
			zeroColorVector.m_FirstColor.r,
			zeroColorVector.m_FirstColor.g,
			zeroColorVector.m_FirstColor.b,
			zeroColorVector.m_FirstColor.a);
		aSkyObjectQuad.SetTransition(3,
			zeroNextColorVector.m_FirstColor.r,
			zeroNextColorVector.m_FirstColor.g,
			zeroNextColorVector.m_FirstColor.b,
			zeroNextColorVector.m_FirstColor.a,
			lTransitionTime);
	}

	TSkyObjectFace & aFaceBottom = m_Faces[5];
	ulVectorGradientColornum = c_rColorVector.size() - 1;

	TGradientColor faceColorVector = c_rColorVector[ulVectorGradientColornum];
	TGradientColor faceNextColorVector = c_rNextColorVector[ulVectorGradientColornum];
	for (uck = 0; uck < aFaceBottom.m_SkyObjectQuadVector.size(); ++uck)
	{
		CSkyObjectQuad & aSkyObjectQuad = aFaceBottom.m_SkyObjectQuadVector[uck];
		
		aSkyObjectQuad.SetSrcColor(0,
			faceColorVector.m_SecondColor.r,
			faceColorVector.m_SecondColor.g,
			faceColorVector.m_SecondColor.b,
			faceColorVector.m_SecondColor.a);
		aSkyObjectQuad.SetTransition(0, 
			faceNextColorVector.m_SecondColor.r,
			faceNextColorVector.m_SecondColor.g,
			faceNextColorVector.m_SecondColor.b,
			faceNextColorVector.m_SecondColor.a,
			lTransitionTime);
		aSkyObjectQuad.SetSrcColor(1,
			faceColorVector.m_SecondColor.r,
			faceColorVector.m_SecondColor.g,
			faceColorVector.m_SecondColor.b,
			faceColorVector.m_SecondColor.a);
		aSkyObjectQuad.SetTransition(1,
			faceNextColorVector.m_SecondColor.r,
			faceNextColorVector.m_SecondColor.g,
			faceNextColorVector.m_SecondColor.b,
			faceNextColorVector.m_SecondColor.a,
			lTransitionTime);
		aSkyObjectQuad.SetSrcColor(2,
			faceColorVector.m_SecondColor.r,
			faceColorVector.m_SecondColor.g,
			faceColorVector.m_SecondColor.b,
			faceColorVector.m_SecondColor.a);
		aSkyObjectQuad.SetTransition(2,
			faceNextColorVector.m_SecondColor.r,
			faceNextColorVector.m_SecondColor.g,
			faceNextColorVector.m_SecondColor.b,
			faceNextColorVector.m_SecondColor.a,
			lTransitionTime);
		aSkyObjectQuad.SetSrcColor(3,
			faceColorVector.m_SecondColor.r,
			faceColorVector.m_SecondColor.g,
			faceColorVector.m_SecondColor.b,
			faceColorVector.m_SecondColor.a);
		aSkyObjectQuad.SetTransition(3,
			faceNextColorVector.m_SecondColor.r,
			faceNextColorVector.m_SecondColor.g,
			faceNextColorVector.m_SecondColor.b,
			faceNextColorVector.m_SecondColor.a,
			lTransitionTime);
	}
}

void CSkyBox::StartTransition()
{
	m_bTransitionStarted = true;
	for (uint8_t ucj = 0; ucj < 6; ++ucj)
		m_Faces[ucj].StartTransition();
	m_FaceCloud.StartTransition();
}

void CSkyBox::Update()
{
	CSkyObject::Update();

	if (!m_bTransitionStarted)
		return;
	
	bool bResult = false;
	for (uint8_t uci = 0; uci < 6; ++uci)
 		bResult = m_Faces[uci].Update() || bResult;
 	bResult = m_FaceCloud.Update() || bResult;

	m_bTransitionStarted = bResult;
}

void CSkyBox::Render()
{
	// 2004.01.25 myevan 처리를 렌더링 후반으로 옮기고, DepthTest 처리
	STATEMANAGER.SaveRenderState(D3DRS_ZENABLE,	TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	STATEMANAGER.SetTexture(1, nullptr);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorld);

	//Render Face
	if( m_ucRenderMode == CSkyObject::SKY_RENDER_MODE_TEXTURE )
	{
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSU,	D3DTADDRESS_CLAMP);
		STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSV,	D3DTADDRESS_CLAMP);

		for (uint32_t i = 0; i < 6; ++i)
		{
			CGraphicImageInstance * pFaceImageInstance = m_GraphicImageInstanceMap[m_Faces[i].m_strFaceTextureFileName];
			if (!pFaceImageInstance)
				break;

			STATEMANAGER.SetTexture( 0, pFaceImageInstance->GetTextureReference().GetD3DTexture() );

			m_Faces[i].Render();
		}

		//STATEMANAGER.SetTexture( 0, nullptr );

		STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSU);
		STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSV);
	}
	else
	{
		for (uint32_t i = 0; i < 6; ++i)
		{
			m_Faces[i].Render();
		}
	}

	STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
	STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
}

void CSkyBox::RenderCloud()
{
	CGraphicImageInstance * pCloudGraphicImageInstance = m_GraphicImageInstanceMap[m_FaceCloud.m_strfacename];
	if (!pCloudGraphicImageInstance)
		return;

	// 2004.01.25 myevan 처리를 렌더링 후반으로 옮기고, DepthTest 처리
	STATEMANAGER.SaveRenderState(D3DRS_ZENABLE,	TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, FALSE);	
	STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);

	STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

	m_matTextureCloud._31 = m_fCloudPositionU;
	m_matTextureCloud._32 = m_fCloudPositionV;
	
	uint32_t dwCurTime = CTimer::Instance().GetCurrentMillisecond();
	
	m_fCloudPositionU += m_fCloudScrollSpeedU * (float)( dwCurTime - m_dwlastTime ) * 0.001f;
	if (m_fCloudPositionU >= 1.0f)
		m_fCloudPositionU = 0.0f;
	
	m_fCloudPositionV += m_fCloudScrollSpeedV * (float)( dwCurTime - m_dwlastTime ) * 0.001f;
	if (m_fCloudPositionV >= 1.0f)
		m_fCloudPositionV = 0.0f;
	
	m_dwlastTime = dwCurTime;
	
	STATEMANAGER.SaveTransform(D3DTS_TEXTURE0, &m_matTextureCloud);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATEINVALPHA_ADDCOLOR);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
 	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	
	D3DXMATRIX matProjCloud;
	D3DXMatrixPerspectiveFovRH(&matProjCloud, D3DX_PI * 0.25f, 1.33333f, 50.0f, 999999.0f);
	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldCloud);
	STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &matProjCloud);
	STATEMANAGER.SetTexture(0, pCloudGraphicImageInstance->GetTexturePointer()->GetD3DTexture());
	m_FaceCloud.Render();
	STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);
	
	STATEMANAGER.RestoreTransform(D3DTS_TEXTURE0);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS);

	STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
	STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
}
