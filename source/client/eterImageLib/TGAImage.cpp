#include "StdAfx.h"

#include <assert.h>

#include <FileSystemIncl.hpp>
#include "TGAImage.h"

CTGAImage::CTGAImage() : m_dwFlag(0), m_pdwEndPtr(nullptr)
{
}

CTGAImage::~CTGAImage()
{
}

CTGAImage::CTGAImage(CImageC &image) : m_dwFlag(0)
{
	int32_t w = image.GetWidth();
	int32_t h = image.GetHeight();

	Create(w, h);

	uint32_t * pdwDest = GetBasePointer();
	memcpy(pdwDest, image.GetBasePointer(), w * h * sizeof(uint32_t));
	FlipTopToBottom();
}

void CTGAImage::Create(int32_t width, int32_t height)
{
	memset(&m_Header, 0, sizeof(m_Header));

	m_Header.imgType	= 2;
	m_Header.width		= (int16_t) width;
	m_Header.height		= (int16_t) height;
	m_Header.colorBits	= 32;
	m_Header.desc		= 0x08;	// alpha channel 있음

	CImageC::Create(width, height);
}

bool CTGAImage::LoadFromMemory(int32_t iSize, const uint8_t * c_pbMem)
{
	memcpy(&m_Header, c_pbMem, 18);
	c_pbMem += 18;
	iSize -= 18;

	CImageC::Create(m_Header.width, m_Header.height);

	uint32_t hxw = m_Header.width * m_Header.height;
	uint8_t r, g, b, a;
	uint32_t i;

	uint32_t * pdwDest = GetBasePointer();

	switch (m_Header.imgType)
	{
		case 3:	// 알파만 있는 것 (1bytes per pixel, 거의 안쓰임)
			{
				for (i = 0; i < hxw; ++i)
				{
					a = (char) *(c_pbMem++);
					pdwDest[i] = (a << 24) | (a << 16) | (a << 8) | a;
				}
			}
			break;

		case 2:	// 압축 안된 TGA
			{
				if (m_Header.colorBits == 16)	// 16bit
				{
					for (i = 0; i < hxw; ++i)
					{
						uint16_t w;

						memcpy(&w, c_pbMem, sizeof(uint16_t));
						c_pbMem += sizeof(uint16_t);
						iSize -= sizeof(uint16_t);
						
						b = (uint8_t) (w & 0x1F);
						g = (uint8_t) ((w >> 5) & 0x1F);
						r = (uint8_t) ((w >> 10) & 0x1F);
						
						b <<= 3;
						g <<= 3;
						r <<= 3;
						a = 0xff;
						
						pdwDest[i] = (a << 24) | (r << 16) | (g << 8) | b;
					}
				}
				else if (m_Header.colorBits == 24)	// 24bit
				{
					for (i = 0; i < hxw; ++i)
					{
						r = (uint8_t) * (c_pbMem++);
						--iSize;
						g = (uint8_t) * (c_pbMem++);
						--iSize;
						b = (uint8_t) * (c_pbMem++);
						--iSize;
						a = 0xff;
						
						pdwDest[i] = (a << 24) | (r << 16) | (g << 8) | b;
					}
				}
				else if (m_Header.colorBits == 32)	// 32bit
				{
					int32_t size = GetWidth();
					size *= GetHeight() * 4;

					memcpy(pdwDest, c_pbMem, size);
					c_pbMem += size;
					iSize -= size;
				}
			}
			break;

		case 10: // 압축 된 TGA (RLE)
			{
				uint8_t rle;

				if (m_Header.colorBits == 24)
				{
					i = 0;
					while (i < hxw)
					{
						rle = (uint8_t) * (c_pbMem++);
						--iSize;

						if (rle < 0x80)	// 압축 안된 곳
						{
							rle++;

							while (rle)
							{
								b = (uint8_t) * (c_pbMem++);
								--iSize;
								g = (uint8_t) * (c_pbMem++);
								--iSize;
								r = (uint8_t) * (c_pbMem++);
								--iSize;
								a = 0xff;
								pdwDest[i++] = (a << 24) | (r << 16) | (g << 8) | b;

								if (i > hxw)
								{
									assert(!"RLE overflow");
#ifdef _DEBUG
									printf("RLE overflow");
#endif
									return false;
								}
								--rle;
							}
						}
						else
						{
							// 압축 된 곳
							rle -= 127;

							b = (uint8_t) * (c_pbMem++);
							--iSize;
							g = (uint8_t) * (c_pbMem++);
							--iSize;
							r = (uint8_t) * (c_pbMem++);
							--iSize;
							a = 0xff;

							while (rle)
							{
								pdwDest[i++] = (a << 24) | (r << 16) | (g << 8) | b;

								if (i > hxw)
								{
									assert(!"RLE overflow");
#ifdef _DEBUG
									printf("RLE overflow");
#endif
									return false;
								}
								--rle;
							}
						}
					}
				}
				else if (m_Header.colorBits == 32)
				{
					i = 0;
					while (i < hxw)
					{
						rle = (uint8_t) * (c_pbMem++);
						--iSize;
						
						if (rle < 0x80)
						{
							rle++;
							
							while (rle)
							{
								b = (uint8_t) * (c_pbMem++);
								--iSize;
								g = (uint8_t) * (c_pbMem++);
								--iSize;
								r = (uint8_t) * (c_pbMem++);
								--iSize;
								a = (uint8_t) * (c_pbMem++);
								--iSize;
								pdwDest[i++] = (a << 24) | (r << 16) | (g << 8) | b;
								
								if (i > hxw)
								{
									assert(!"RLE overflow");
#ifdef _DEBUG
									printf("RLE overflow");
#endif
									return false;
								}
								--rle;
							}
						}
						else
						{
							rle -= 127;
							
							b = (uint8_t) * (c_pbMem++);
							--iSize;
							g = (uint8_t) * (c_pbMem++);
							--iSize;
							r = (uint8_t) * (c_pbMem++);
							--iSize;
							a = (uint8_t) * (c_pbMem++);
							--iSize;
							
							while (rle)
							{
								pdwDest[i++] = (a << 24) | (r << 16) | (g << 8) | b;
								
								if (i > hxw)
								{
									assert(!"RLE overflow");
#ifdef _DEBUG
									printf("RLE overflow");
#endif
									return false;
								}

								--rle;
							}
						}
					}
				}
			}
			break;
	}
	
	if (!(m_Header.desc & 0x20))
		FlipTopToBottom();

	return true;
}

bool CTGAImage::LoadFromDiskFile(const std::string& stFileName)
{
	CFile file;
	if (!file.Create(stFileName, FILEMODE_READ, true))
		return false;

	return LoadFromMemory(file.GetSize(), file.GetData());
}

int32_t CTGAImage::GetRLEPixelCount(const uint32_t * data)
{
	int32_t r = 1;

	if (data >= m_pdwEndPtr)
		return 0;

	uint32_t pixel = *data;

    while ((r < 127) && (data < m_pdwEndPtr))
    {
		if (pixel != *(++data))
			return r;
		
        r++;
    }
	
	return r;
}

int32_t CTGAImage::GetRawPixelCount(const uint32_t * data)
{
    int32_t i = 0;
    
    if (data >= m_pdwEndPtr)
        return 0;
	
    while ((data < m_pdwEndPtr) && (i < 127))
    {
		int32_t rle = GetRLEPixelCount(data);
		
		if (rle >= 4)
			break;

        data++;
        i++;
    }
	
    return i;
}

void CTGAImage::SetCompressed(bool isCompress)
{
	if (isCompress)
		m_Header.imgType = 10;
	else
		m_Header.imgType = 2;
}

void CTGAImage::SetAlphaChannel(bool isExist)
{
	if (isExist)
		m_Header.desc |= 0x08;
	else
		m_Header.desc &= ~0x08;
}

bool CTGAImage::SaveToDiskFile(const char* c_szFileName)
{
	FILE * fp;
	fopen_s(&fp, c_szFileName, "wb");
	
	if (!fp)
		return false;

	fwrite(&m_Header, 18, 1, fp);
	
	if (m_Header.imgType == 10)	// RLE 압축으로 저장
	{
		uint32_t * data = GetBasePointer();
		
		while (data < m_pdwEndPtr)
		{
			int32_t rle = GetRLEPixelCount(data);
			
			if (rle < 4)
			{
				int32_t raw = GetRawPixelCount(data);
				
				if (raw == 0)
					break;
				
				fputc(raw - 1, fp);
				
				while (raw)
				{
					fwrite(data, sizeof(uint32_t), 1, fp);
					data++;
					raw--;
				}
			}
			else
			{
				fputc((rle - 1) | 0x80, fp);
				fwrite(data, sizeof(uint32_t), 1, fp);
				data += rle;
			}
		}
	}
	else
	{
		int32_t size = GetWidth();
		size *= GetHeight() * 4;
		fwrite(GetBasePointer(), size, 1, fp);
	}

	fclose(fp);
	return true;
}

TGA_HEADER & CTGAImage::GetHeader()
{
	return m_Header;
}
