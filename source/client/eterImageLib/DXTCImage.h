#ifndef AFX_IMAGE_DXTC_H__4B89D8D0_7857_11D4_9630_00A0C996DE3D__INCLUDED_
#define AFX_IMAGE_DXTC_H__4B89D8D0_7857_11D4_9630_00A0C996DE3D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

enum EPixFormat
{
	PF_ARGB,
	PF_DXT1,
	PF_DXT2,
	PF_DXT3,
	PF_DXT4,
	PF_DXT5,
	PF_UNKNOWN
};

#define MAX_MIPLEVELS 12

#ifndef DUMMYUNIONNAMEN
#if defined(__cplusplus) || !defined(NONAMELESSUNION)
#define DUMMYUNIONNAMEN(n)
#else
#define DUMMYUNIONNAMEN(n)      u##n
#endif
#endif

typedef struct _XDDPIXELFORMAT
{
	uint32_t       dwSize;                 // size of structure
	uint32_t       dwFlags;                // pixel format flags
	uint32_t       dwFourCC;               // (FOURCC code)

    union
    {
        uint32_t   dwRGBBitCount;          // how many bits per pixel
        uint32_t   dwYUVBitCount;          // how many bits per pixel
        uint32_t   dwZBufferBitDepth;      // how many total bits/pixel in z buffer (including any stencil bits)
        uint32_t   dwAlphaBitDepth;        // how many bits for alpha channels
        uint32_t   dwLuminanceBitCount;    // how many bits per pixel
        uint32_t   dwBumpBitCount;         // how many bits per "buxel", total
        uint32_t   dwPrivateFormatBitCount;// Bits per pixel of private driver formats. Only valid in texture
		// format list and if DDPF_D3DFORMAT is set
    } DUMMYUNIONNAMEN(1);
    union
    {
        uint32_t   dwRBitMask;             // mask for red bit
        uint32_t   dwYBitMask;             // mask for Y bits
        uint32_t   dwStencilBitDepth;      // how many stencil bits (note: dwZBufferBitDepth-dwStencilBitDepth is total Z-only bits)
        uint32_t   dwLuminanceBitMask;     // mask for luminance bits
        uint32_t   dwBumpDuBitMask;        // mask for bump map U delta bits
        uint32_t   dwOperations;           // DDPF_D3DFORMAT Operations
    } DUMMYUNIONNAMEN(2);
    union
    {
        uint32_t   dwGBitMask;             // mask for green bits
        uint32_t   dwUBitMask;             // mask for U bits
        uint32_t   dwZBitMask;             // mask for Z bits
        uint32_t   dwBumpDvBitMask;        // mask for bump map V delta bits
        struct
        {
            uint16_t    wFlipMSTypes;       // Multisample methods supported via flip for this D3DFORMAT
            uint16_t    wBltMSTypes;        // Multisample methods supported via blt for this D3DFORMAT
        } MultiSampleCaps;

    } DUMMYUNIONNAMEN(3);
    union
    {
        uint32_t   dwBBitMask;             // mask for blue bits
        uint32_t   dwVBitMask;             // mask for V bits
        uint32_t   dwStencilBitMask;       // mask for stencil bits
        uint32_t   dwBumpLuminanceBitMask; // mask for luminance in bump map
    } DUMMYUNIONNAMEN(4);
    union
    {
        uint32_t   dwRGBAlphaBitMask;      // mask for alpha channel
        uint32_t   dwYUVAlphaBitMask;      // mask for alpha channel
        uint32_t   dwLuminanceAlphaBitMask;// mask for alpha channel
        uint32_t   dwRGBZBitMask;          // mask for Z channel
        uint32_t   dwYUVZBitMask;          // mask for Z channel
    } DUMMYUNIONNAMEN(5);
} XDDPIXELFORMAT;


class CDXTCImage
{
	public:
		CDXTCImage();
		virtual ~CDXTCImage();

		void	Initialize();
		void	Clear();

	public:
		const uint8_t *		m_pbCompBufferByLevels[MAX_MIPLEVELS];
		std::vector<uint8_t>	m_bCompVector[MAX_MIPLEVELS];

		char				m_strFormat[32];
		EPixFormat			m_CompFormat;

		int32_t				m_lPitch;
		uint32_t				m_dwMipMapCount;
		bool				m_bMipTexture;	// texture has mipmaps?
		uint32_t				m_dwFlags;

		int32_t					m_nWidth;		// in pixels of uncompressed image
		int32_t					m_nHeight;

		XDDPIXELFORMAT		m_xddPixelFormat;

	
		bool LoadHeaderFromMemory(const uint8_t * c_pbMap, uint32_t dwSize);
		bool Copy(int32_t miplevel, uint8_t * pbDest, int32_t lDestPitch);
		VOID DecodePixelFormat(CHAR* strPixelFormat, XDDPIXELFORMAT* pddpf);

};

#endif // #ifndef AFX_IMAGE_DXTC_H__4B89D8D0_7857_11D4_9630_00A0C996DE3D__INCLUDED_
