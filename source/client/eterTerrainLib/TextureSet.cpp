#include "StdAfx.h"
#include "TextureSet.h"

CTextureSet::CTextureSet()
{
	Initialize();
}

CTextureSet::~CTextureSet()
{
	Clear();
}

void CTextureSet::Initialize()
{
}

void CTextureSet::Create()
{
	CResource * pResource = CResourceManager::Instance().GetResourcePointer("d:/ymir work/special/error.tga");
	m_ErrorTexture.ImageInstance.SetImagePointer(static_cast<CGraphicImage *> (pResource));
	AddEmptyTexture();	// 지우개 텍스춰를 처음에 추가 해야 함
}

bool CTextureSet::Load(const char * c_szTextureSetFileName, float fTerrainTexCoordBase)
{
	Clear();

	CTokenVectorMap stTokenVectorMap;

	if (!LoadMultipleTextData(c_szTextureSetFileName, stTokenVectorMap))
	{
		TraceError("TextureSet::Load : cannot load %s", c_szTextureSetFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("textureset"))
	{
		TraceError("TextureSet::Load : syntax error, TextureSet (filename: %s)", c_szTextureSetFileName);
		return false;
	}
	
	if (stTokenVectorMap.end() == stTokenVectorMap.find("texturecount"))
	{
		TraceError("TextureSet::Load : syntax error, TextureCount (filename: %s)", c_szTextureSetFileName);
		return false;
	}

	Create();

	const std::string & c_rstrCount = stTokenVectorMap["texturecount"][0];

	int32_t lCount = atol(c_rstrCount.c_str());
	char szTextureName[32 + 1];

	m_Textures.resize(lCount + 1);

	for (int32_t i = 0; i < lCount; ++i)
	{
		_snprintf_s(szTextureName, sizeof(szTextureName), "texture%03d", i + 1);

		if (stTokenVectorMap.end() == stTokenVectorMap.find(szTextureName))
			continue;

		const CTokenVector & rVector = stTokenVectorMap[szTextureName];

		const std::string & c_rstrFileName	= rVector[0];
		const std::string & c_rstrUScale	= rVector[1];
		const std::string & c_rstrVScale	= rVector[2];
		const std::string & c_rstrUOffset	= rVector[3];
		const std::string & c_rstrVOffset	= rVector[4];
		const std::string & c_rstrbSplat	= rVector[5];
		const std::string & c_rstrBegin		= rVector[6];
		const std::string & c_rstrEnd		= rVector[7];

		float fuScale, fvScale, fuOffset, fvOffset;
		bool bSplat;
		uint16_t usBegin, usEnd;

		fuScale	= atof(c_rstrUScale.c_str());
		fvScale = atof(c_rstrVScale.c_str());
		fuOffset = atof(c_rstrUOffset.c_str());
		fvOffset = atof(c_rstrVOffset.c_str());
		bSplat = 0 != atoi(c_rstrbSplat.c_str());
		usBegin = static_cast<uint16_t>(atoi(c_rstrBegin.c_str()));
		usEnd = static_cast<uint16_t>(atoi(c_rstrEnd.c_str()));

		if (!SetTexture(i + 1, c_rstrFileName.c_str(), fuScale, fvScale, fuOffset, fvOffset, bSplat, usBegin, usEnd, fTerrainTexCoordBase))
			TraceError("CTextureSet::Load : SetTexture failed : Filename: %s", c_rstrFileName.c_str());
	}

	m_stFileName.assign(c_szTextureSetFileName);
	return true;
}

void CTextureSet::Clear()
{
	m_ErrorTexture.ImageInstance.Destroy();
	m_Textures.clear();
	Initialize();
}

void CTextureSet::AddEmptyTexture()
{
	TTerrainTexture eraser;
	m_Textures.emplace_back(eraser);
}

uint32_t CTextureSet::GetTextureCount()
{
	return m_Textures.size();
}

TTerrainTexture	& CTextureSet::GetTexture(uint32_t ulIndex)
{
	if (GetTextureCount() <= ulIndex)
		return m_ErrorTexture;

	return m_Textures[ulIndex];
}

bool CTextureSet::SetTexture(uint32_t ulIndex,
							 const char * c_szFileName,
							 float fuScale,
							 float fvScale,
							 float fuOffset,
							 float fvOffset,
							 bool bSplat,
							 uint16_t usBegin,
							 uint16_t usEnd,
							 float fTerrainTexCoordBase)
{

	if (ulIndex >= m_Textures.size())
	{
		TraceError("CTextureSet::SetTexture : Index Error : Index(%d) is Larger than TextureSet Size(%d)", ulIndex, m_Textures.size());
		return false;
	}

	CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
	if (!pResource)
		return false;

	if (!pResource->IsType(CGraphicImage::Type()))
	{
		const auto& stRefResourceName = pResource->GetFileNameString();
		TraceError("CTerrainImpl::GenerateTexture : %s is NOT Image File", stRefResourceName.c_str());
		return false;
	}

	TTerrainTexture & tex = m_Textures[ulIndex];

	tex.stFilename = c_szFileName;
	tex.UScale = fuScale;
	tex.VScale = fvScale;
	tex.UOffset = fuOffset;
	tex.VOffset = fvOffset;
	tex.bSplat = bSplat;
	tex.Begin = usBegin;
	tex.End = usEnd;
	tex.ImageInstance.SetImagePointer(static_cast<CGraphicImage *>(pResource));
	tex.pd3dTexture = tex.ImageInstance.GetTexturePointer()->GetD3DTexture();
	
	
	D3DXMatrixScaling(&tex.m_matTransform, fTerrainTexCoordBase * tex.UScale, -fTerrainTexCoordBase * tex.VScale, 0.0f);
	tex.m_matTransform._41 = tex.UOffset;
	tex.m_matTransform._42 = -tex.VOffset;
	return true;
}

void CTextureSet::Reload(float fTerrainTexCoordBase)
{
	for (uint32_t dwIndex = 1; dwIndex < GetTextureCount(); ++dwIndex)
	{
		TTerrainTexture & tex = m_Textures[dwIndex];

		tex.ImageInstance.ReloadImagePointer((CGraphicImage *) CResourceManager::Instance().GetResourcePointer(tex.stFilename.c_str()));
		tex.pd3dTexture = tex.ImageInstance.GetTexturePointer()->GetD3DTexture();

		D3DXMatrixScaling(&tex.m_matTransform, fTerrainTexCoordBase * tex.UScale, -fTerrainTexCoordBase * tex.VScale, 0.0f);
		tex.m_matTransform._41 = tex.UOffset;
		tex.m_matTransform._42 = -tex.VOffset;
	}
}

bool CTextureSet::ReplaceTexture(const char * c_OldszFileName,
							 const char * c_szFileName,
							 float fuScale,
							 float fvScale,
							 float fuOffset,
							 float fvOffset,
							 bool bSplat,
							 uint16_t usBegin,
							 uint16_t usEnd,
							 float fTerrainTexCoordBase)
{
	for (uint32_t i = 1; i < GetTextureCount(); ++i)
	{
		if (0 == m_Textures[i].stFilename.compare(c_OldszFileName))
		{
			SetTexture(i,
			   c_szFileName,
			   fuScale,
			   fvScale,
			   fuOffset,
			   fvOffset,
			   bSplat,
			   usBegin,
			   usEnd,
			   fTerrainTexCoordBase);
			return true;
		}
	}
	return false;
}

bool CTextureSet::AddTexture(const char * c_szFileName,
							 float fuScale,
							 float fvScale,
							 float fuOffset,
							 float fvOffset,
							 bool bSplat,
							 uint16_t usBegin,
							 uint16_t usEnd,
							 float fTerrainTexCoordBase)
{
	if (GetTextureCount() >= 256)
	{
		LogBox("You cannot add more than 255 texture.");
		return false;
	}

	for (uint32_t i = 1; i < GetTextureCount(); ++i)
	{
		if (0 == m_Textures[i].stFilename.compare(c_szFileName))
		{
			LogBox("Texture of the same name already exists.", "Duplicate");
			return false;
		}
	}

	CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
	if (!pResource)
		return false;

	if (!pResource->IsType(CGraphicImage::Type()))
	{
		const auto& stRefResourceName = pResource->GetFileNameString();
		LogBox("CTerrainImpl::GenerateTexture : It's not an image file. %s", stRefResourceName.c_str());
		return false;
	}
	
	m_Textures.reserve(m_Textures.size() + 1);

	// @fixme003
	AddEmptyTexture();
	SetTexture(m_Textures.size() - 1,
			   c_szFileName,
			   fuScale,
			   fvScale,
			   fuOffset,
			   fvOffset,
			   bSplat,
			   usBegin,
			   usEnd,
			   fTerrainTexCoordBase);

	return true;
}

bool CTextureSet::RemoveTexture(uint32_t ulIndex)
{
	if (GetTextureCount() <= ulIndex)
		return false;

	auto itor = m_Textures.begin() + ulIndex;
	m_Textures.erase(itor);
	return true;
}

bool CTextureSet::Save(const char * c_pszFileName)
{
	msl::file_ptr fPtr(c_pszFileName, "w");
	if (!fPtr)
		return false;

	fprintf(fPtr.get(), "TextureSet\n");
	fprintf(fPtr.get(), "\n");

	fprintf(fPtr.get(), "TextureCount %u\n", GetTextureCount() ? (GetTextureCount() - 1) : 0);	// -1 을 하는 이유는 지우개 때문임
	fprintf(fPtr.get(), "\n");

	for (uint32_t i = 1; i < GetTextureCount(); ++i)
	{
		TTerrainTexture & rTex = m_Textures[i];

		fprintf(fPtr.get(), "Start Texture%03u\n", i);
		fprintf(fPtr.get(), "    \"%s\"\n", rTex.stFilename.c_str());
		fprintf(fPtr.get(), "    %f\n", rTex.UScale);
		fprintf(fPtr.get(), "    %f\n", rTex.VScale);
		fprintf(fPtr.get(), "    %f\n", rTex.UOffset);
		fprintf(fPtr.get(), "    %f\n", rTex.VOffset);
		fprintf(fPtr.get(), "    %d\n", rTex.bSplat);
		fprintf(fPtr.get(), "    %hu\n", rTex.Begin);
		fprintf(fPtr.get(), "    %hu\n", rTex.End);
		fprintf(fPtr.get(), "End Texture%03u\n", i);
	}
	return true;
}
