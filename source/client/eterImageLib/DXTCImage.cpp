// need the d3d.h for things in format of .dds file
#include "StdAfx.h"
#include <ddraw.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <algorithm>
#include "../eterBase/Debug.h"
#include "DXTCImage.h"

CDXTCImage::CDXTCImage()
{
	Initialize();
}

CDXTCImage::~CDXTCImage()
{
}

void CDXTCImage::Initialize()
{
	m_nWidth = 0;
	m_nHeight = 0;

	for (int32_t i = 0; i < MAX_MIPLEVELS; ++i)
		m_pbCompBufferByLevels[i] = nullptr;
}

void CDXTCImage::Clear()
{
	for (int32_t i = 0; i < MAX_MIPLEVELS; ++i)
		m_bCompVector[i].clear();

	Initialize();
}

bool CDXTCImage::LoadHeaderFromMemory(const uint8_t * c_pbMap, uint32_t dwSize)
{
	//////////////////////////////////////
	// start reading the file
	// from Microsoft's mssdk D3DIM example "Compress"

	//File is smaller than uint32_t unlikely we might have a corrupt file.
	if (dwSize <= sizeof(uint32_t))
		return false;

	uint32_t dwMagic;

	// Read magic number*/
	dwMagic = *(uint32_t *)c_pbMap;
	
	if (dwMagic != MAKEFOURCC('D','D','S',' '))
		return false;

	c_pbMap += sizeof(uint32_t);

	// File is smaller than the DDS header cant be a dds file.
	if (dwSize - sizeof(uint32_t) <= sizeof(DDSURFACEDESC2))
		return false;

	DDSURFACEDESC2 ddsd; // read from dds file

						 // Read the surface description
	memcpy(&ddsd, c_pbMap, sizeof(DDSURFACEDESC2));
	c_pbMap += sizeof(DDSURFACEDESC2);

	// Does texture have mipmaps?
	m_bMipTexture = (ddsd.dwMipMapCount > 0) ? TRUE : FALSE;

	// Clear unwanted flags
	// Can't do this!!!  surface not re-created here
	//    ddsd.dwFlags &= (~DDSD_PITCH);
	//    ddsd.dwFlags &= (~DDSD_LINEARSIZE);

	// Is it DXTC ?
	// I sure hope pixelformat is valid!
	m_xddPixelFormat.dwFlags = ddsd.ddpfPixelFormat.dwFlags;
	m_xddPixelFormat.dwFourCC = ddsd.ddpfPixelFormat.dwFourCC;
	m_xddPixelFormat.dwSize = ddsd.ddpfPixelFormat.dwSize;
	m_xddPixelFormat.dwRGBBitCount = ddsd.ddpfPixelFormat.dwRGBBitCount;
	m_xddPixelFormat.dwRGBAlphaBitMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
	m_xddPixelFormat.dwRBitMask = ddsd.ddpfPixelFormat.dwRBitMask;
	m_xddPixelFormat.dwGBitMask = ddsd.ddpfPixelFormat.dwGBitMask;
	m_xddPixelFormat.dwBBitMask = ddsd.ddpfPixelFormat.dwBBitMask;

	DecodePixelFormat(m_strFormat, &m_xddPixelFormat);

	if (m_CompFormat != PF_DXT1 &&
		m_CompFormat != PF_DXT3 &&
		m_CompFormat != PF_DXT5)
	{
		return false;
	}

	if (ddsd.dwMipMapCount > MAX_MIPLEVELS)
		ddsd.dwMipMapCount = MAX_MIPLEVELS;

	m_nWidth = ddsd.dwWidth;
	m_nHeight = ddsd.dwHeight;
	//!@#
	m_dwMipMapCount = std::max<uint32_t>(1, ddsd.dwMipMapCount);
	m_dwFlags = ddsd.dwFlags;

	if (ddsd.dwFlags & DDSD_PITCH)
	{
		m_lPitch = ddsd.lPitch;
		m_pbCompBufferByLevels[0] = c_pbMap;
	}
	else
	{
		m_lPitch = ddsd.dwLinearSize;

		if (ddsd.dwFlags & DDSD_MIPMAPCOUNT)
		{
			for (uint32_t dwLinearSize = ddsd.dwLinearSize, i = 0; i < m_dwMipMapCount; ++i, dwLinearSize >>= 2)
			{
				m_pbCompBufferByLevels[i] = c_pbMap;
				c_pbMap += dwLinearSize;
			}
		}
		else
		{
			m_pbCompBufferByLevels[0] = c_pbMap;
		}
	}

	return true;
}

// Name: GetNumberOfBits()
// Desc: Returns the number of bits set in a uint32_t mask
//	from microsoft mssdk d3dim sample "Compress"
//-----------------------------------------------------------------------------
static uint16_t GetNumberOfBits(uint32_t dwMask)
{
	uint16_t wBits;
	for (wBits = 0; dwMask; wBits++)
		dwMask = (dwMask & (dwMask - 1));

	return wBits;
}

//-----------------------------------------------------------------------------
// Name: PixelFormatToString()
// Desc: Creates a string describing a pixel format.
//	adapted from microsoft mssdk D3DIM Compress example
//  PixelFormatToString()
//-----------------------------------------------------------------------------
VOID CDXTCImage::DecodePixelFormat(CHAR* strPixelFormat, XDDPIXELFORMAT* pxddpf)
{
	switch (pxddpf->dwFourCC)
	{
	case 0:
	{
		// This dds texture isn't compressed so write out ARGB format
		uint16_t a = GetNumberOfBits(pxddpf->dwRGBAlphaBitMask);
		uint16_t r = GetNumberOfBits(pxddpf->dwRBitMask);
		uint16_t g = GetNumberOfBits(pxddpf->dwGBitMask);
		uint16_t b = GetNumberOfBits(pxddpf->dwBBitMask);

		_snprintf_s(strPixelFormat, 31, _TRUNCATE, "ARGB-%d%d%d%d%s", a, r, g, b,
			pxddpf->dwBBitMask & DDPF_ALPHAPREMULT ? "-premul" : "");
		m_CompFormat = PF_ARGB;
	}
	break;

	case MAKEFOURCC('D', 'X', 'T', '1'):
		strncpy_s(strPixelFormat, 31, "DXT1", 31);
		m_CompFormat = PF_DXT1;
		break;

	case MAKEFOURCC('D', 'X', 'T', '2'):
		strncpy_s(strPixelFormat, 31, "DXT2", 31);
		m_CompFormat = PF_DXT2;
		break;

	case MAKEFOURCC('D', 'X', 'T', '3'):
		strncpy_s(strPixelFormat, 31, "DXT3", 31);
		m_CompFormat = PF_DXT3;
		break;

	case MAKEFOURCC('D', 'X', 'T', '4'):
		strncpy_s(strPixelFormat, 31, "DXT4", 31);
		m_CompFormat = PF_DXT4;
		break;

	case MAKEFOURCC('D', 'X', 'T', '5'):
		strncpy_s(strPixelFormat, 31, "DXT5", 31);
		m_CompFormat = PF_DXT5;
		break;

	default:
		strcpy_s(strPixelFormat, 31, "Format Unknown");
		m_CompFormat = PF_UNKNOWN;
		break;
	}
}


bool CDXTCImage::Copy(int32_t miplevel, uint8_t * pbDest, int32_t lDestPitch)
{
	if (!(m_dwFlags & DDSD_MIPMAPCOUNT))
		if (miplevel)
			return false;

	memcpy(pbDest, m_pbCompBufferByLevels[miplevel], m_lPitch >> (miplevel * 2));
	pbDest += lDestPitch;
	return true;
}
