#include "StdAfx.h"
#include <FileSystemIncl.hpp>
#include "GrpImageTexture.h"

bool CGraphicImageTexture::Lock(int32_t* pRetPitch, void** ppRetPixels, int32_t level)
{
	D3DLOCKED_RECT lockedRect;
	if (FAILED(m_lpd3dTexture->LockRect(level, &lockedRect, nullptr, 0)))
		return false;

	*pRetPitch = lockedRect.Pitch;
	*ppRetPixels = (void*)lockedRect.pBits;	
	return true;
}

void CGraphicImageTexture::Unlock(int32_t level)
{
	assert(m_lpd3dTexture != nullptr);
	m_lpd3dTexture->UnlockRect(level);
}

void CGraphicImageTexture::Initialize()
{
	CGraphicTexture::Initialize();

	m_stFileName = "";

	m_d3dFmt=D3DFMT_UNKNOWN;
	m_dwFilter=0;
}

void CGraphicImageTexture::Destroy()
{
	CGraphicTexture::Destroy();

	Initialize();
}

bool CGraphicImageTexture::CreateDeviceObjects()
{
	assert(ms_lpd3dDevice != nullptr);
	assert(m_lpd3dTexture == nullptr);

	if (m_stFileName.empty())
	{
		// 폰트 텍스쳐
		if (FAILED(ms_lpd3dDevice->CreateTexture(m_width, m_height, 1, 0, m_d3dFmt, D3DPOOL_MANAGED, &m_lpd3dTexture)))
			return false;
	}
	else
	{
		CFile mappedFile;
		if (!FileSystemManager::Instance().OpenFile(m_stFileName, mappedFile))
			return false;

		//@fixme002
		if (!CreateFromMemoryFile(mappedFile.GetSize(), mappedFile.GetData(), m_d3dFmt, m_dwFilter))
		{
			TraceError("CGraphicImageTexture::CreateDeviceObjects: CreateFromMemoryFile: texture not found(%s)", m_stFileName.c_str());
			return false;
		}
		return true;
		// return CreateFromMemoryFile(mappedFile.GetSize(), mappedFile.GetData(), m_d3dFmt, m_dwFilter);
	}

	m_bEmpty = false;
	return true;
}

bool CGraphicImageTexture::Create(uint32_t width, uint32_t height, D3DFORMAT d3dFmt, uint32_t dwFilter)
{
	assert(ms_lpd3dDevice != nullptr);
	Destroy();

	m_width = width;
	m_height = height;
	m_d3dFmt = d3dFmt;
	m_dwFilter = dwFilter;

	return CreateDeviceObjects();
}

void CGraphicImageTexture::CreateFromTexturePointer(const CGraphicTexture * c_pSrcTexture)
{
	if (m_lpd3dTexture)
		m_lpd3dTexture->Release();
	
	m_width = c_pSrcTexture->GetWidth();
	m_height = c_pSrcTexture->GetHeight();
	m_lpd3dTexture = c_pSrcTexture->GetD3DTexture();
	
	if (m_lpd3dTexture)
		m_lpd3dTexture->AddRef();

	m_bEmpty = false;
}

bool CGraphicImageTexture::CreateDDSTexture(CDXTCImage & image, const uint8_t * /*c_pbBuf*/)
{
	int32_t mipmapCount = image.m_dwMipMapCount == 0 ? 1 : image.m_dwMipMapCount;

	D3DFORMAT format;
	LPDIRECT3DTEXTURE8 lpd3dTexture;
	D3DPOOL pool = ms_bSupportDXT ? D3DPOOL_MANAGED : D3DPOOL_SCRATCH;;

	if(image.m_CompFormat == PF_DXT5)
		format = D3DFMT_DXT5;	
	else if(image.m_CompFormat == PF_DXT3)
		format = D3DFMT_DXT3;	
	else
		format = D3DFMT_DXT1;	

	uint32_t uTexBias=0;
	if (IsLowTextureMemory())
		uTexBias=1;

	uint32_t uMinMipMapIndex=0;
	if (uTexBias>0)
	{
		if (mipmapCount>uTexBias)
		{
			uMinMipMapIndex=uTexBias;
			image.m_nWidth>>=uTexBias;
			image.m_nHeight>>=uTexBias;
			mipmapCount-=uTexBias;
		}
	}

	if (FAILED(D3DXCreateTexture(	ms_lpd3dDevice, image.m_nWidth, image.m_nHeight,
									mipmapCount, 0, format, pool, &lpd3dTexture)))
	{
		TraceError("CreateDDSTexture: Cannot creatre texture");
		return false;
	}

	for (uint32_t i = 0; i < mipmapCount; ++i)
	{
		D3DLOCKED_RECT lockedRect;

		if (FAILED(lpd3dTexture->LockRect(i, &lockedRect, nullptr, 0)))
		{
			TraceError("CreateDDSTexture: Cannot lock texture");
		}
		else
		{
			image.Copy(i+uMinMipMapIndex, (uint8_t*)lockedRect.pBits, lockedRect.Pitch);
			lpd3dTexture->UnlockRect(i);
		}
	}

	if(ms_bSupportDXT)
	{
		m_lpd3dTexture = lpd3dTexture;
	}
	else
	{
		if(image.m_CompFormat == PF_DXT3 || image.m_CompFormat == PF_DXT5)
			format = D3DFMT_A4R4G4B4;
		else
			format = D3DFMT_A1R5G5B5;

		uint32_t imgWidth=image.m_nWidth;
		uint32_t imgHeight=image.m_nHeight;

		extern bool GRAPHICS_CAPS_HALF_SIZE_IMAGE;

		if (GRAPHICS_CAPS_HALF_SIZE_IMAGE && uTexBias>0 && mipmapCount==0)
		{
			imgWidth>>=uTexBias;
			imgHeight>>=uTexBias;		
		}

		if (FAILED(D3DXCreateTexture(	ms_lpd3dDevice, imgWidth, imgHeight, 
										mipmapCount, 0, format, D3DPOOL_MANAGED, &m_lpd3dTexture)))
		{
				TraceError("CreateDDSTexture: Cannot creatre texture");
				return false;
		}

		IDirect3DTexture8* pkTexSrc=lpd3dTexture;
		IDirect3DTexture8* pkTexDst=m_lpd3dTexture;

		for(int32_t i=0; i<mipmapCount; ++i) {

			IDirect3DSurface8* ppsSrc = nullptr;
			IDirect3DSurface8* ppsDst = nullptr;

			if (SUCCEEDED(pkTexSrc->GetSurfaceLevel(i, &ppsSrc)))
			{
				if (SUCCEEDED(pkTexDst->GetSurfaceLevel(i, &ppsDst)))
				{
					D3DXLoadSurfaceFromSurface(ppsDst, nullptr, nullptr, ppsSrc, nullptr, nullptr, D3DX_FILTER_NONE, 0);
					ppsDst->Release();
				}
				ppsSrc->Release();
			}
		}

		lpd3dTexture->Release();
	}

	m_width = image.m_nWidth;
	m_height = image.m_nHeight;
	m_bEmpty = false;

	return true;
}

bool CGraphicImageTexture::CreateFromMemoryFile(uint32_t bufSize, const void * c_pvBuf, D3DFORMAT d3dFmt, uint32_t dwFilter)
{
	assert(ms_lpd3dDevice != nullptr);
	assert(m_lpd3dTexture == nullptr);

	static CDXTCImage image;

	if (image.LoadHeaderFromMemory((const uint8_t *) c_pvBuf, bufSize))	// DDS인가 확인
	{
		return (CreateDDSTexture(image, (const uint8_t *) c_pvBuf));
	}
	else
	{
		D3DXIMAGE_INFO imageInfo;
		if (FAILED(D3DXCreateTextureFromFileInMemoryEx(
					ms_lpd3dDevice,
					c_pvBuf,
					bufSize,
					D3DX_DEFAULT,
					D3DX_DEFAULT,
					D3DX_DEFAULT,
					0,
					d3dFmt,
					D3DPOOL_MANAGED,
					dwFilter,
					dwFilter,
					0xffff00ff,
					&imageInfo,
					nullptr,
					&m_lpd3dTexture)))
		{
			TraceError("CreateFromMemoryFile: Cannot create texture");
			return false;
		}

		m_width = imageInfo.Width;
		m_height = imageInfo.Height;

		D3DFORMAT format=imageInfo.Format;
		switch(imageInfo.Format) {
			case D3DFMT_A8R8G8B8:
				format = D3DFMT_A4R4G4B4;
				break;

			case D3DFMT_X8R8G8B8:
			case D3DFMT_R8G8B8:
				format = D3DFMT_A1R5G5B5;
				break;
		}

		uint32_t uTexBias=0;

		extern bool GRAPHICS_CAPS_HALF_SIZE_IMAGE;
		if (GRAPHICS_CAPS_HALF_SIZE_IMAGE)
			uTexBias=1;

		if (IsLowTextureMemory())
		if (uTexBias || format!=imageInfo.Format)
		{
			IDirect3DTexture8* pkTexSrc=m_lpd3dTexture;
			IDirect3DTexture8* pkTexDst;
			
			
			if (SUCCEEDED(D3DXCreateTexture(	
				ms_lpd3dDevice, 
				imageInfo.Width>>uTexBias, 
				imageInfo.Height>>uTexBias, 
				imageInfo.MipLevels, 
				0, 
				format, 
				D3DPOOL_MANAGED, 
				&pkTexDst)))
			{
				m_lpd3dTexture=pkTexDst;
				
				for(int32_t i=0; i<imageInfo.MipLevels; ++i) {

					IDirect3DSurface8* ppsSrc = nullptr;
					IDirect3DSurface8* ppsDst = nullptr;

					if (SUCCEEDED(pkTexSrc->GetSurfaceLevel(i, &ppsSrc)))
					{
						if (SUCCEEDED(pkTexDst->GetSurfaceLevel(i, &ppsDst)))
						{
							D3DXLoadSurfaceFromSurface(ppsDst, nullptr, nullptr, ppsSrc, nullptr, nullptr, D3DX_FILTER_LINEAR, 0);
							ppsDst->Release();
						}
						ppsSrc->Release();
					}
				}

				pkTexSrc->Release();
			}
		}
	}

	m_bEmpty = false;
	return true;
}

void CGraphicImageTexture::SetFileName(const char * c_szFileName)
{
	m_stFileName=c_szFileName;
}

bool CGraphicImageTexture::CreateFromDiskFile(const char * c_szFileName, D3DFORMAT d3dFmt, uint32_t dwFilter)
{
	Destroy();

	SetFileName(c_szFileName);

	m_d3dFmt = d3dFmt;
	m_dwFilter = dwFilter;
	return CreateDeviceObjects();
}

CGraphicImageTexture::CGraphicImageTexture()
{
	Initialize();
}

CGraphicImageTexture::~CGraphicImageTexture()
{
	Destroy();
}
