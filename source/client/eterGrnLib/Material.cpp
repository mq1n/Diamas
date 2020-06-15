#include "StdAfx.h"
#include "Material.h"
#include "Mesh.h"
#include "../eterBase/Filename.h"
#include "../eterLib/ResourceManager.h"
#include "../eterLib/StateManager.h"
#include "../eterLib/GrpScreen.h"

CGraphicImageInstance CGrannyMaterial::ms_akSphereMapInstance[SPHEREMAP_NUM];

D3DXVECTOR3	CGrannyMaterial::ms_v3SpecularTrans(0.0f, 0.0f, 0.0f);
D3DXMATRIX	CGrannyMaterial::ms_matSpecular;

D3DXCOLOR g_fSpecularColor = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f);

void CGrannyMaterial::TranslateSpecularMatrix(float fAddX, float fAddY, float fAddZ)
{
	static float SPECULAR_TRANSLATE_MAX = 1000000.0f;

	ms_v3SpecularTrans.x+=fAddX;
	ms_v3SpecularTrans.y+=fAddY;
	ms_v3SpecularTrans.z+=fAddZ;

	if (ms_v3SpecularTrans.x>=SPECULAR_TRANSLATE_MAX)
		ms_v3SpecularTrans.x=0.0f;

	if (ms_v3SpecularTrans.y>=SPECULAR_TRANSLATE_MAX)
		ms_v3SpecularTrans.y=0.0f;

	if (ms_v3SpecularTrans.z>=SPECULAR_TRANSLATE_MAX)
		ms_v3SpecularTrans.z=0.0f;

	D3DXMatrixTranslation(&ms_matSpecular, 
		ms_v3SpecularTrans.x, 
		ms_v3SpecularTrans.y, 
		ms_v3SpecularTrans.z
	);
}

void CGrannyMaterial::ApplyRenderState()
{
	assert(m_pfnApplyRenderState!=nullptr && "CGrannyMaterial::SaveRenderState");
	(this->*m_pfnApplyRenderState)();
}

void CGrannyMaterial::RestoreRenderState()
{
	assert(m_pfnRestoreRenderState!=nullptr && "CGrannyMaterial::RestoreRenderState");
	(this->*m_pfnRestoreRenderState)();
}

CGrannyMaterial::CGrannyMaterial()
{
	m_bTwoSideRender = false;
	m_dwLastCullRenderStateForTwoSideRendering = D3DCULL_CW;

	Initialize();
}

CGrannyMaterial::~CGrannyMaterial()
{
}

CGrannyMaterial::EType CGrannyMaterial::GetType() const
{
	return m_eType;
}

void CGrannyMaterial::SetImagePointer(int32_t iStage, CGraphicImage* pImage)
{	
	assert(iStage<2 && "CGrannyMaterial::SetImagePointer");
	m_roImage[iStage]=pImage;
}

bool CGrannyMaterial::IsIn(const char* c_szImageName, int32_t* piStage)
{
	std::string strImageName = c_szImageName;
	CFileNameHelper::StringPath(strImageName);

	granny_texture * pgrnDiffuseTexture = GrannyGetMaterialTextureByType(m_pgrnMaterial, GrannyDiffuseColorTexture);
	if (pgrnDiffuseTexture)
	{
		std::string strDiffuseFileName = pgrnDiffuseTexture->FromFileName;
		CFileNameHelper::StringPath(strDiffuseFileName);
		if (strDiffuseFileName == strImageName)
		{
			*piStage=0;
			return true;
		}
	}

    granny_texture * pgrnOpacityTexture = GrannyGetMaterialTextureByType(m_pgrnMaterial, GrannyOpacityTexture);
	if (pgrnOpacityTexture)
	{
		std::string strOpacityFileName = pgrnOpacityTexture->FromFileName;
		CFileNameHelper::StringPath(strOpacityFileName);
		if (strOpacityFileName == strImageName)
		{
			*piStage=1;
			return true;
		}
	}

	return false;
}

void CGrannyMaterial::SetSpecularInfo(BOOL bFlag, float fPower, uint8_t uSphereMapIndex)
{
	m_fSpecularPower = fPower;
	m_bSphereMapIndex = uSphereMapIndex;
	m_bSpecularEnable = bFlag;	

	if (bFlag)
	{
		m_pfnApplyRenderState = &CGrannyMaterial::__ApplySpecularRenderState;
		m_pfnRestoreRenderState = &CGrannyMaterial::__RestoreSpecularRenderState;
	}
	else
	{
		m_pfnApplyRenderState = &CGrannyMaterial::__ApplyDiffuseRenderState;
		m_pfnRestoreRenderState = &CGrannyMaterial::__RestoreDiffuseRenderState;
	}
}

bool CGrannyMaterial::operator==(granny_material* pgrnMaterial) const
{
	if (m_pgrnMaterial==pgrnMaterial)
		return true;

	return false;
}


LPDIRECT3DTEXTURE9 CGrannyMaterial::GetD3DTexture(int32_t iStage) const
{
	const CGraphicImage::TRef & ratImage = m_roImage[iStage];

	if (ratImage.IsNull())
		return nullptr;

	CGraphicImage * pImage = ratImage.GetPointer();
	const CGraphicTexture * pTexture = pImage->GetTexturePointer();
	return pTexture->GetD3DTexture();
}

CGraphicImage * CGrannyMaterial::GetImagePointer(int32_t iStage) const
{
	const CGraphicImage::TRef & ratImage = m_roImage[iStage];

	if (ratImage.IsNull())
		return nullptr;

	CGraphicImage * pImage = ratImage.GetPointer();
	return pImage;
}

const CGraphicTexture* CGrannyMaterial::GetDiffuseTexture() const
{
	if (m_roImage[0].IsNull())
		return nullptr;

	return m_roImage[0].GetPointer()->GetTexturePointer();
}

const CGraphicTexture* CGrannyMaterial::GetOpacityTexture() const
{
	if (m_roImage[1].IsNull())
		return nullptr;

	return m_roImage[1].GetPointer()->GetTexturePointer();
}

BOOL CGrannyMaterial::__IsSpecularEnable() const
{
	return m_bSpecularEnable;
}

float CGrannyMaterial::__GetSpecularPower() const
{
	return m_fSpecularPower;
}

extern const std::string& GetModelLocalPath();

CGraphicImage* CGrannyMaterial::__GetImagePointer(const char* fileName)
{
	assert(*fileName != '\0');

	CResourceManager& rkResMgr = CResourceManager::Instance();

	// SUPPORT_LOCAL_TEXTURE
	int32_t fileName_len = strlen(fileName);
	if (fileName_len > 2 && fileName[1] != ':')
	{
		char localFileName[256];		
		const std::string& modelLocalPath = GetModelLocalPath();

		int32_t localFileName_len = modelLocalPath.length() + 1 + fileName_len;
		if (localFileName_len < sizeof(localFileName) - 1)
		{
			_snprintf_s(localFileName, sizeof(localFileName), "%s%s", GetModelLocalPath().c_str(), fileName);
			CResource* pResource = rkResMgr.GetResourcePointer(localFileName);
			return static_cast<CGraphicImage*>(pResource);
		}		
	}
	// END_OF_SUPPORT_LOCAL_TEXTURE
	

	CResource* pResource = rkResMgr.GetResourcePointer(fileName);
	return static_cast<CGraphicImage*>(pResource);
}

bool CGrannyMaterial::CreateFromGrannyMaterialPointer(granny_material * pgrnMaterial)
{
	m_pgrnMaterial = pgrnMaterial;

	granny_texture* pgrnDiffuseTexture = nullptr;
	granny_texture* pgrnOpacityTexture = nullptr;

	if (pgrnMaterial)
	{
		if (pgrnMaterial->MapCount > 1 && !_strnicmp(pgrnMaterial->Name, "Blend", 5))
		{
			pgrnDiffuseTexture = GrannyGetMaterialTextureByType(pgrnMaterial->Maps[0].Material, GrannyDiffuseColorTexture);
			pgrnOpacityTexture = GrannyGetMaterialTextureByType(pgrnMaterial->Maps[1].Material, GrannyDiffuseColorTexture);
		}
		else
		{
			pgrnDiffuseTexture = GrannyGetMaterialTextureByType(m_pgrnMaterial, GrannyDiffuseColorTexture);
			pgrnOpacityTexture = GrannyGetMaterialTextureByType(m_pgrnMaterial, GrannyOpacityTexture);
		}

		// Two-Side 렌더링이 필요한 지 검사
		{			
			granny_int32 twoSided = 0;
			granny_data_type_definition TwoSidedFieldType[] =
			{
				{GrannyInt32Member, "Two-sided"},
				{GrannyEndMember},
			};

			granny_variant twoSideResult;


			if (GrannyFindMatchingMember(pgrnMaterial->ExtendedData.Type, pgrnMaterial->ExtendedData.Object, "Two-sided", &twoSideResult)
				&& nullptr != twoSideResult.Type)
				GrannyConvertSingleObject(twoSideResult.Type, twoSideResult.Object, TwoSidedFieldType, &twoSided, nullptr);

			m_bTwoSideRender = 1 == twoSided;
		}
	}

	if (pgrnDiffuseTexture)
		m_roImage[0].SetPointer(__GetImagePointer(pgrnDiffuseTexture->FromFileName));

	if (pgrnOpacityTexture)
		m_roImage[1].SetPointer(__GetImagePointer(pgrnOpacityTexture->FromFileName));

	// 오퍼시티가 있으면 블렌딩 메쉬
	if (!m_roImage[1].IsNull())
		m_eType = TYPE_BLEND_PNT;
	else
		m_eType = TYPE_DIFFUSE_PNT;

	return true;
}

void CGrannyMaterial::Initialize()
{
	m_roImage[0] = nullptr;
	m_roImage[1] = nullptr;

	SetSpecularInfo(FALSE, 0.0f, 0);
}

void CGrannyMaterial::__ApplyDiffuseRenderState()
{
	STATEMANAGER.SetTexture(0, GetD3DTexture(0));

	if (m_bTwoSideRender)
	{
		// -_-렌더링 프로세스가 좀 구려서... Save & Restore 하면 순서때문에 좀 꼬인다. 귀찮으니 Save & Restore 대신 따로 저장해 둠.
		m_dwLastCullRenderStateForTwoSideRendering = STATEMANAGER.GetRenderState(D3DRS_CULLMODE);
		STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	}
}

void CGrannyMaterial::__RestoreDiffuseRenderState()
{
	if (m_bTwoSideRender)
	{
		STATEMANAGER.SetRenderState(D3DRS_CULLMODE, m_dwLastCullRenderStateForTwoSideRendering);
	}
}

void CGrannyMaterial::__ApplySpecularRenderState()
{
	if (TRUE == STATEMANAGER.GetRenderState(D3DRS_ALPHABLENDENABLE))
	{
		__ApplyDiffuseRenderState();
		return;
	}

	CGraphicTexture* pkTexture=ms_akSphereMapInstance[m_bSphereMapIndex].GetTexturePointer();

	STATEMANAGER.SetTexture(0, GetD3DTexture(0));

	if (pkTexture)
		STATEMANAGER.SetTexture(1, pkTexture->GetD3DTexture());
	else
		STATEMANAGER.SetTexture(1, nullptr);

	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, D3DXCOLOR(g_fSpecularColor.r, g_fSpecularColor.g, g_fSpecularColor.b, __GetSpecularPower()));
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG2,	D3DTA_TFACTOR);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1,	D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,	D3DTOP_MODULATEALPHA_ADDCOLOR);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1,	D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,	D3DTOP_SELECTARG1);

	STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &ms_matSpecular);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
	STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
}

void CGrannyMaterial::__RestoreSpecularRenderState()
{
	if (TRUE == STATEMANAGER.GetRenderState(D3DRS_ALPHABLENDENABLE))
	{
		__RestoreDiffuseRenderState();
		return;
	}

	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS);
	STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSU);
	STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSV);

	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXCOORDINDEX);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
}

void CGrannyMaterial::CreateSphereMap(uint32_t uMapIndex, const char* c_szSphereMapImageFileName)
{
	CResourceManager& rkResMgr = CResourceManager::Instance();
	CGraphicImage * pImage = (CGraphicImage *)rkResMgr.GetResourcePointer(c_szSphereMapImageFileName);
	ms_akSphereMapInstance[uMapIndex].SetImagePointer(pImage);
}

void CGrannyMaterial::DestroySphereMap()
{
	for (uint32_t uMapIndex=0; uMapIndex<SPHEREMAP_NUM; ++uMapIndex)
		ms_akSphereMapInstance[uMapIndex].Destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CGrannyMaterialPalette::SetMaterialImage(const char* materialName,
                                              CGraphicImage* image)
{
	for (auto& material : m_materials) {
		int32_t stage;
		if (!material.IsIn(materialName, &stage))
			continue;

		material.SetImagePointer(stage, image);
		break;
	}
}

void CGrannyMaterialPalette::SetMaterialData(const char* materialName,
                                             const SMaterialData& data)
{
	if (materialName) {
		for (auto& material : m_materials) {
			int32_t stage;
			if (!material.IsIn(materialName, &stage))
				continue;

			material.SetImagePointer(stage, data.pImage);
			material.SetSpecularInfo(data.isSpecularEnable,
			                         data.fSpecularPower,
			                         data.bSphereMapIndex);
			break;
		}
	} else {
		for (auto& material : m_materials) {
			material.SetSpecularInfo(data.isSpecularEnable,
			                         data.fSpecularPower,
			                         data.bSphereMapIndex);
			break;
		}
	}
}

void CGrannyMaterialPalette::SetSpecularInfo(const char* materialName, bool enable, float power)
{
	if (materialName) {
		for (auto& material : m_materials) {
			int32_t stage;
			if (!material.IsIn(materialName, &stage))
				continue;

			material.SetSpecularInfo(enable, power, 0);
			break;
		}
	} else {
		for (auto& material : m_materials) {
			material.SetSpecularInfo(enable, power, 0);
			break;
		}
	}
}

std::size_t CGrannyMaterialPalette::RegisterMaterial(granny_material* material)
{
	std::size_t size = m_materials.size();
	for (std::size_t i = 0; i != size; ++i) {
		if (m_materials[i] == material)
			return i;
	}

	m_materials.emplace_back();
	m_materials.back().CreateFromGrannyMaterialPointer(material);
	return size;
}

CGrannyMaterial& CGrannyMaterialPalette::GetMaterialRef(std::size_t index)
{
	assert(index < m_materials.size());
	return m_materials[index];
}

std::size_t CGrannyMaterialPalette::GetMaterialCount() const
{
	return m_materials.size();
}
