#ifndef __INC_ETERIMAGELIB_TGAIMAGE_H__
#define __INC_ETERIMAGELIB_TGAIMAGE_H__

#include "Image.h"

class CTGAImage : public CImageC
{
	public:
		enum ETGAImageFlags
		{
			FLAG_RLE_COMPRESS = (1 << 0)
		};
			
		CTGAImage();
		CTGAImage(CImageC &image);
		virtual ~CTGAImage();

		virtual void	Create(int32_t width, int32_t height);
		virtual bool	LoadFromMemory(int32_t iSize, const uint8_t * c_pbMem);
		virtual bool	LoadFromDiskFile(const std::string& stFileName);
		virtual bool	SaveToDiskFile(const char* c_szFileName);

		void			SetCompressed(bool isCompress = true);
		void			SetAlphaChannel(bool isExist = true);

		TGA_HEADER &	GetHeader();
	protected:
		int32_t			GetRawPixelCount(const uint32_t * data);
		int32_t			GetRLEPixelCount(const uint32_t * data);

	protected:
		TGA_HEADER	m_Header;
		uint32_t		m_dwFlag;
		uint32_t *		m_pdwEndPtr;
};

#endif
