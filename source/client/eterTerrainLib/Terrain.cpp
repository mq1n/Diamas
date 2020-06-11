#include "Stdafx.h"
#include <FileSystemIncl.hpp>

#include "terrain.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////
// Texture Set
//////////////////////////////////////////////////////////////////////////
CTextureSet * CTerrainImpl::ms_pTextureSet = nullptr;

void CTerrainImpl::SetTextureSet(CTextureSet * pTextureSet)
{
	static CTextureSet s_EmptyTextureSet;

	if (!pTextureSet)
		ms_pTextureSet = &s_EmptyTextureSet;
	else
		ms_pTextureSet = pTextureSet;
}

CTextureSet * CTerrainImpl::GetTextureSet()
{
	if (!ms_pTextureSet)
		SetTextureSet(nullptr);

	return ms_pTextureSet;
}

CTerrainImpl::CTerrainImpl()
{
	Initialize();
}

CTerrainImpl::~CTerrainImpl()
{
	Clear();
}

void CTerrainImpl::Initialize()
{
	memset(m_lWaterHeight, -1, sizeof(m_lWaterHeight));

	m_byNumWater = 0;
	memset(&m_HeightMapHeader, 0, sizeof(TGA_HEADER));
	memset(&m_awShadowMap, 0xFFFF, sizeof(m_awShadowMap));
	memset(&m_lpAlphaTexture, 0, sizeof(m_lpAlphaTexture));
	
	m_lViewRadius = 0;

	m_wTileMapVersion = 8976;

	m_fHeightScale = 0.0f;
	
	m_lpShadowTexture = nullptr;

	m_lSplatTilesX = 0;
	m_lSplatTilesY = 0;	
}

void CTerrainImpl::Clear()
{
	for (uint32_t i = 0; i < GetTextureSet()->GetTextureCount(); ++i)
	{
		if (m_lpAlphaTexture[i])
		{
			m_lpAlphaTexture[i]->Release();
			m_lpAlphaTexture[i] = nullptr;
		}
	}
	
	Initialize();
}

bool CTerrainImpl::LoadHeightMap(const char*c_szFileName)
{
	Tracef("LoadRawHeightMapFile %s ", c_szFileName);
	
	CFile kMappedFile;
	if (!FileSystemManager::Instance().OpenFile(c_szFileName, kMappedFile, c_szFileName))
	{
		Tracen("Error");
		TraceError("CTerrainImpl::LoadHeightMap - %s OPEN ERROR", c_szFileName);
		return false;
	}
	
	memcpy(m_awRawHeightMap, kMappedFile.GetData(), sizeof(uint16_t) * HEIGHTMAP_RAW_XSIZE * HEIGHTMAP_RAW_YSIZE);
	
	return true;
}

bool CTerrainImpl::LoadAttrMap(const char *c_szFileName)
{
	uint32_t dwStart = ELTimer_GetMSec();
	Tracef("LoadAttrMapFile %s ", c_szFileName);

	CFile kMappedFile;
	if (!FileSystemManager::Instance().OpenFile(c_szFileName, kMappedFile, c_szFileName))
	{
		TraceError("CTerrainImpl::LoadAttrMap - %s OPEN ERROR", c_szFileName);
		return false;
	}

	uint32_t dwFileSize = kMappedFile.GetSize();
	uint8_t* abFileData = (uint8_t*)kMappedFile.GetData();

	// LoadAttrMap
	{
#pragma pack(push)
#pragma pack(1)
		struct SAttrMapHeader
		{
			uint16_t m_wMagic;
			uint16_t m_wWidth;
			uint16_t m_wHeight;
		};
#pragma pack(pop)
		
		if (dwFileSize < sizeof(SAttrMapHeader))
		{
			TraceError(" CTerrainImpl::LoadAttrMap - %s FILE SIZE ERROR", c_szFileName);
			return false;
		}

		SAttrMapHeader kAttrMapHeader;
		memcpy(&kAttrMapHeader, abFileData, sizeof(kAttrMapHeader));

		const uint16_t c_wAttrMapMagic = 2634;
		if (c_wAttrMapMagic != kAttrMapHeader.m_wMagic)
		{
			TraceError("CTerrainImpl::LoadAttrMap - %s MAGIC NUMBER(%d!=MAGIC[%d]) ERROR", c_szFileName, kAttrMapHeader.m_wMagic, kAttrMapHeader.m_wMagic);
			return false;
		}

		if (ATTRMAP_XSIZE != kAttrMapHeader.m_wWidth)
		{
			TraceError("CTerrainImpl::LoadAttrMap - kAttrMapHeader(%s).m_width(%d)!=ATTRMAP_XSIZE(%d)", c_szFileName, kAttrMapHeader.m_wWidth, ATTRMAP_XSIZE);
			return false;
		}

		if (ATTRMAP_YSIZE != kAttrMapHeader.m_wHeight)
		{
			TraceError("CTerrainImpl::LoadAttrMap - kAttrMapHeader(%s).m_height(%d)!=ATTRMAP_YSIZE(%d)", c_szFileName, kAttrMapHeader.m_wHeight, ATTRMAP_YSIZE);
			return false;
		}

		uint32_t dwFileRestSize=dwFileSize-sizeof(kAttrMapHeader);
		uint32_t dwFileNeedSize=sizeof(m_abyAttrMap);
		if (dwFileRestSize != dwFileNeedSize)
		{
			TraceError("CTerrainImpl::LoadAttrMap - %s FILE DATA SIZE(rest %d != need %d) ERROR", c_szFileName, dwFileRestSize, dwFileNeedSize);
			return false;
		}

		uint8_t* abSrcAttrData= abFileData+sizeof(kAttrMapHeader);
		memcpy(m_abyAttrMap, abSrcAttrData, sizeof(m_abyAttrMap));		
	}

	Tracef("%d\n", ELTimer_GetMSec() - dwStart);
	return true;
}

bool CTerrainImpl::RAW_LoadTileMap(const char * c_szFileName)
{
	Tracef("LoadSplatFile %s ", c_szFileName);
	
	CFile kMappedFile;
	if (!FileSystemManager::Instance().OpenFile(c_szFileName, kMappedFile, c_szFileName))
	{
		Tracen("Error");
		TraceError("CTerrainImpl::RAW_LoadTileMap - %s OPEN ERROR", c_szFileName);
		return false;
	}
	
	memcpy(m_abyTileMap, kMappedFile.GetData(), sizeof(uint8_t) * (TILEMAP_RAW_XSIZE) * (TILEMAP_RAW_YSIZE));
	
	return true;

}

bool CTerrainImpl::LoadWaterMap(const char * c_szFileName)
{	
	uint32_t dwStart = ELTimer_GetMSec();

	if (!LoadWaterMapFile(c_szFileName))
	{
		memset(m_abyWaterMap, 0xFF, sizeof(m_abyWaterMap));

		m_byNumWater = 0;
		memset(m_lWaterHeight, -1, sizeof(m_lWaterHeight));
		
		TraceError("CMapOutdoor::LoadWaterMap LoadWaterMapFile(%s) Failed", c_szFileName);

		return false;
	}

	Tracef("LoadWaterMapFile %s %d\n", c_szFileName, ELTimer_GetMSec() - dwStart);
	return true;
}

bool CTerrainImpl::LoadWaterMapFile(const char * c_szFileName)
{	
	CFile kMappedFile;
	if (!FileSystemManager::Instance().OpenFile(c_szFileName, kMappedFile, c_szFileName))
	{
		Tracen("Error");
		TraceError("CTerrainImpl::LoadWaterMap - %s OPEN ERROR", c_szFileName);
		return false;
	}	

	uint32_t dwFileSize = kMappedFile.GetSize();
	uint8_t* abFileData = (uint8_t*)kMappedFile.GetData();

	{
#pragma pack(push)
#pragma pack(1)
		struct SWaterMapHeader
		{
			uint16_t m_wMagic;
			uint16_t m_wWidth;
			uint16_t m_wHeight;
			uint8_t m_byLayerCount;
		};
#pragma pack(pop)
		
		if (dwFileSize < sizeof(SWaterMapHeader))
		{
			TraceError("CTerrainImpl::LoadWaterMap - %s FILE SIZE ERROR", c_szFileName);
			return false;
		}

		SWaterMapHeader kWaterMapHeader;
		memcpy(&kWaterMapHeader, abFileData, sizeof(kWaterMapHeader));		

		const uint16_t c_wWaterMapMagic = 5426;
		
		if (c_wWaterMapMagic != kWaterMapHeader.m_wMagic)
		{
			TraceError("CTerrainImpl::LoadWaterMap - %s MAGIC NUMBER(%d!=MAGIC[%d]) ERROR", c_szFileName, kWaterMapHeader.m_wMagic, c_wWaterMapMagic);
			return false;
		}	
		
		if (WATERMAP_XSIZE != kWaterMapHeader.m_wWidth)
		{
			TraceError("CTerrainImpl::LoadWaterMap - kWaterMapHeader(%s).m_width(%d)!=WATERMAP_XSIZE(%d)", c_szFileName, kWaterMapHeader.m_wWidth, WATERMAP_XSIZE);
			return false;
		}

		if (WATERMAP_YSIZE != kWaterMapHeader.m_wHeight)
		{
			TraceError("CTerrainImpl::LoadWaterMap - kWaterMapHeader(%s).m_height(%d)!=WATERMAP_YSIZE(%d)", c_szFileName, kWaterMapHeader.m_wHeight, WATERMAP_YSIZE);
			return false;
		}

		m_byNumWater = kWaterMapHeader.m_byLayerCount;

		uint32_t dwFileRestSize = dwFileSize - sizeof(kWaterMapHeader);
		uint32_t dwFileNeedSize = sizeof(m_abyWaterMap) + sizeof(int32_t) * m_byNumWater;
		uint32_t dwFileNeedSize2 = sizeof(m_abyWaterMap) + sizeof(uint16_t) * m_byNumWater;
		if (dwFileRestSize == dwFileNeedSize2)
		{
			uint16_t wWaterHeight[MAX_WATER_NUM + 1];
			
			uint8_t * abSrcWaterData = abFileData + sizeof(kWaterMapHeader);
			memcpy(m_abyWaterMap, abSrcWaterData, sizeof(m_abyWaterMap));	

			uint8_t * abSrcWaterHeight = abSrcWaterData + sizeof(m_abyWaterMap);

			m_byNumWater = MIN(MAX_WATER_NUM, m_byNumWater);
			if (m_byNumWater)
			{
				memcpy(wWaterHeight, abSrcWaterHeight, sizeof(uint16_t) * m_byNumWater);

				for (int32_t i = 0; i < m_byNumWater; ++i)
					m_lWaterHeight[i] = wWaterHeight[i];
			}
		}
		else if (dwFileRestSize != dwFileNeedSize)
		{
			TraceError("CTerrainImpl::LoadWaterMap - %s FILE DATA SIZE(rest %d != need %d) ERROR", c_szFileName, dwFileRestSize, dwFileNeedSize);
			return false;
		}
	
		uint8_t * abSrcWaterData = abFileData + sizeof(kWaterMapHeader);
		memcpy(m_abyWaterMap, abSrcWaterData, sizeof(m_abyWaterMap));	

		uint8_t * abSrcWaterHeight = abSrcWaterData + sizeof(m_abyWaterMap);

		if (m_byNumWater)
			memcpy(m_lWaterHeight, abSrcWaterHeight, sizeof(int32_t) * m_byNumWater);
	}

	return true;
}

uint32_t CTerrainImpl::GetShadowMapColor(float fx, float fy)
{
	float fMapSize = (float)(TERRAIN_XSIZE);
	float fooMapSize = 1.0f / fMapSize;
	if (fx < 0 || fy < 0 || fx >= fMapSize || fy >= fMapSize)
		return 0xFFFFFFFF;

	fx = fx * fooMapSize * (float)(SHADOWMAP_XSIZE - 1);
	fy = fy * fooMapSize * (float)(SHADOWMAP_YSIZE - 1);
	int32_t ix, iy;
	PR_FLOAT_TO_INT(fx, ix);
	PR_FLOAT_TO_INT(fy, iy);

	uint16_t w = *(m_awShadowMap + (iy * SHADOWMAP_XSIZE) + ix);

	int32_t b = w & 0x1f; w >>= 5; b <<= 3;
	int32_t g = w & 0x1f; w >>= 5; g <<= 3;
	int32_t r = w & 0x1f;		   r <<= 3;

	return (uint32_t) (0xff << 24) | (g << 16) | (g << 8) | r;
}
