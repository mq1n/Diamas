#pragma once

#include "GrpTexture.h"
#include "../eterImageLib/DXTCImage.h"

class CGraphicImageTexture : public CGraphicTexture
{
	public:
		CGraphicImageTexture();
		virtual ~CGraphicImageTexture();

		void		Destroy();

		bool		Create(uint32_t width, uint32_t height, D3DFORMAT d3dFmt, uint32_t dwFilter = D3DX_FILTER_POINT);
		bool		CreateDeviceObjects();
		
		void		CreateFromTexturePointer(const CGraphicTexture* c_pSrcTexture);
		bool		CreateFromDiskFile(const char* c_szFileName, D3DFORMAT d3dFmt, uint32_t dwFilter = D3DX_FILTER_POINT);
		bool		CreateFromMemoryFile(uint32_t bufSize, const void* c_pvBuf, D3DFORMAT d3dFmt, uint32_t dwFilter = D3DX_FILTER_POINT);
		bool		CreateDDSTexture(CDXTCImage & image, const uint8_t * c_pbBuf);

		void		SetFileName(const FileSystem::CFileName& filename);

		bool		Lock(int32_t* pRetPitch, void** ppRetPixels, int32_t level=0);
		void		Unlock(int32_t level=0);

	protected:
		void		Initialize();
		bool		ConvertTexture(IDirect3DTexture9* pkTexSrc, IDirect3DTexture9* pkTexDst, int32_t mipMapCount);
		
		D3DFORMAT	m_d3dFmt;
		uint32_t		m_dwFilter;

		FileSystem::CFileName m_stFileName;
};
