#include "StdAfx.h"
#include <FileSystemIncl.hpp>
#include "GrpImageTexture.h"
#include <d3d9/DxErr.h>

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

	m_stFileName = FileSystem::CFileName();

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

	const auto& stRefFilename = m_stFileName.GetPathA();
	if (stRefFilename.empty())
	{
		// 폰트 텍스쳐
		if (FAILED(ms_lpd3dDevice->CreateTexture(m_width, m_height, 1, 0, m_d3dFmt, D3DPOOL_MANAGED, &m_lpd3dTexture, nullptr)))
			return false;
	}
	else
	{
		CFile mappedFile;
		if (!FileSystemManager::Instance().OpenFile(m_stFileName, mappedFile))
			return false;

		if (!CreateFromMemoryFile(mappedFile.GetSize(), mappedFile.GetData(), m_d3dFmt, m_dwFilter))
		{
			const auto& stRefFilename = m_stFileName.GetPathA();
			TraceError("CGraphicImageTexture::CreateDeviceObjects: CreateFromMemoryFile: texture not found(%s)", stRefFilename.c_str());
			return false;
		}
		return true;
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

bool CGraphicImageTexture::ConvertTexture(IDirect3DTexture9* pkTexSrc, IDirect3DTexture9* pkTexDst, int32_t mipMapCount)
{
	HRESULT hr;
	bool failed = false;
	IDirect3DSurface9* ppsSrc = nullptr;
	IDirect3DSurface9* ppsDst = nullptr;

	const auto& stRefFilename = m_stFileName.GetPathA();
	for (int32_t i = 0; i < mipMapCount; ++i) {
		hr = pkTexSrc->GetSurfaceLevel(i, &ppsSrc);
		if (FAILED(hr)) {
			TraceError("CreateDDSTexture: ms_bSupportDXT = false GetSurfaceLevel(ppsSrc) failed (file: %s, mipLevel: %d) Error: %s", stRefFilename.c_str(), i, DXGetErrorString(hr));
			failed = true;
			break;
		}

		hr = pkTexDst->GetSurfaceLevel(i, &ppsDst);
		if (FAILED(hr)) {
			TraceError("CreateDDSTexture: ms_bSupportDXT = false GetSurfaceLevel(ppsDst) failed (file: %s, mipLevel: %d) Error: %s", stRefFilename.c_str(), i, DXGetErrorString(hr));
			failed = true;
			break;
		}

		hr = D3DXLoadSurfaceFromSurface(ppsDst, nullptr, nullptr, ppsSrc, nullptr, nullptr, D3DX_FILTER_NONE, 0);
		if (FAILED(hr)) {
			TraceError("CreateDDSTexture: ms_bSupportDXT = false D3DXLoadSurfaceFromSurface failed (file: %s, mipLevel: %d) Error: %s", stRefFilename.c_str(), i, DXGetErrorString(hr));
			failed = true;
			break;
		}

		ppsDst->Release();
		ppsSrc->Release();
	}

	if (failed) {
		if (ppsSrc)
			ppsSrc->Release();

		if (ppsDst)
			ppsDst->Release();

		return false;
	}

	return true;
}

bool CGraphicImageTexture::CreateDDSTexture(CDXTCImage & image, const uint8_t * /*c_pbBuf*/)
{
	int32_t mipmapCount = image.m_dwMipMapCount == 0 ? 1 : image.m_dwMipMapCount;

	const auto& stRefFilename = m_stFileName.GetPathA();

	LPDIRECT3DTEXTURE9 lpd3dTexture;
	HRESULT hr;
	D3DPOOL pool = ms_bSupportDXT ? D3DPOOL_MANAGED : D3DPOOL_SCRATCH;;

	hr = D3DXCreateTexture(ms_lpd3dDevice, image.m_nWidth, image.m_nHeight, mipmapCount, 0, (D3DFORMAT)image.m_xddPixelFormat.dwFourCC, pool, &lpd3dTexture);

	if (FAILED(hr)) {
		TraceError("CreateDDSTexture: Cannot create texture (file: %s, pool: %d, ms_bSupportDXT: %d) Error: %s", stRefFilename.c_str(), pool, ms_bSupportDXT, DXGetErrorString(hr));
		return false;
	}

	for (int32_t i = 0; i < mipmapCount; ++i)
	{
		D3DLOCKED_RECT lockedRect;

		hr = lpd3dTexture->LockRect(i, &lockedRect, nullptr, 0);

		if (FAILED(hr)) {
			TraceError("CreateDDSTexture: Cannot lock texture (file: %s, mipmapCount: %d) Error: %s", stRefFilename.c_str(), mipmapCount, DXGetErrorString(hr));
			continue;
		}

		image.Copy(i, (uint8_t*)lockedRect.pBits, lockedRect.Pitch);
		lpd3dTexture->UnlockRect(i);
	}

	if (ms_bSupportDXT)
		m_lpd3dTexture = lpd3dTexture;
	else
	{
		D3DFORMAT compatFormat;

		if (image.m_CompFormat == PF_DXT1) // Everything except DXT1 uses a non 1 bit alpha channel
			compatFormat = D3DFMT_A1R5G5B5;
		else if (IsLowTextureMemory())
			compatFormat = D3DFMT_A4R4G4B4; // Glitches are visible but it uses less memory.
		else 
			compatFormat = D3DFMT_A8R8G8B8;

		uint32_t imgWidth=image.m_nWidth;
		uint32_t imgHeight=image.m_nHeight;

		hr = D3DXCreateTexture(ms_lpd3dDevice, imgWidth, imgHeight, mipmapCount, 0, compatFormat , D3DPOOL_MANAGED, &m_lpd3dTexture);

		if (FAILED(hr)) {
			TraceError("CreateDDSTexture: ms_bSupportDXT = false Cannot create texture (file: %s, compatFormat: %d) Error: %s", stRefFilename.c_str(), compatFormat, DXGetErrorString(hr));
			return false;
		}

		IDirect3DTexture9* pkTexSrc=lpd3dTexture;
		IDirect3DTexture9* pkTexDst=m_lpd3dTexture;

		if (!ConvertTexture(pkTexSrc, pkTexDst, mipmapCount)) {
			lpd3dTexture->Release();
			return false;
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
	
	const auto& stRefFilename = m_stFileName.GetPathA();

	static CDXTCImage image;

	if (image.LoadHeaderFromMemory((const uint8_t *) c_pvBuf, bufSize))	// DDS Texture found
		return (CreateDDSTexture(image, (const uint8_t *) c_pvBuf));

	/************************************************************************/
	/* We got a non .DDS DXT file                                           */
	/************************************************************************/

	uint32_t width, height;
	height = width = D3DX_DEFAULT;

	// This is only valid because we don not load an DXT1 - DXT5 compressed texture if partial Support is given the call would fail 
	// for said formats.
	int32_t pow2TextureSupportLevel = GetNonPow2TextureSupportLevel();
	if (pow2TextureSupportLevel == NONPOW2_SUPPORT_LEVEL::FULL_SUPPORT || pow2TextureSupportLevel == NONPOW2_SUPPORT_LEVEL::PARTIAL_SUPPORT) {
		width = D3DX_DEFAULT_NONPOW2;
		height = D3DX_DEFAULT_NONPOW2;
	}

	HRESULT hr;
	D3DXIMAGE_INFO imageInfo;

	hr = D3DXCreateTextureFromFileInMemoryEx(ms_lpd3dDevice, c_pvBuf, bufSize, width, height, D3DX_DEFAULT, 0, d3dFmt, D3DPOOL_MANAGED, dwFilter, D3DX_DEFAULT, 0xffff00ff, &imageInfo, nullptr, &m_lpd3dTexture);

	if (FAILED(hr)) {
		TraceError("CreateFromMemoryFile: Non DDS file cannot create texture (file: %s) Error: %s", stRefFilename.c_str(), DXGetErrorString(hr));
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

	if (IsLowTextureMemory() && (format != imageInfo.Format))
	{
		IDirect3DTexture9* pkTexSrc = m_lpd3dTexture;
		IDirect3DTexture9* pkTexDst;

		hr = D3DXCreateTexture(ms_lpd3dDevice, imageInfo.Width, imageInfo.Height, imageInfo.MipLevels, 0, format, D3DPOOL_MANAGED, &pkTexDst);

		if (FAILED(hr))	{
			TraceError("CreateFromMemoryFile: Non DDS file IsLowTextureMemory cannot create texture (file: %s) Error: %s", stRefFilename.c_str(), DXGetErrorString(hr));
			return  false;
		}

		m_lpd3dTexture = pkTexDst;

		if (!ConvertTexture(pkTexSrc, pkTexDst, imageInfo.MipLevels)) {
			pkTexSrc->Release();
			return false;
		}

		pkTexSrc->Release();
	}

	m_bEmpty = false;
	return true;
}

void CGraphicImageTexture::SetFileName(const FileSystem::CFileName& filename)
{
	m_stFileName=filename;
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
