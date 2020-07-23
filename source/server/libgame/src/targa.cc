#include "../../libthecore/include/stdafx.h"
#include <stdio.h>
#include <string.h>
#include "targa.h"

CTargaImage::CTargaImage() :
    m_pbuf(nullptr), m_x(0), m_y(0)
{
	memset(&m_header, 0, sizeof(m_header));
}

CTargaImage::~CTargaImage()
{
	delete [] m_pbuf;
}

char * CTargaImage::GetBasePointer(int32_t line)
{
    return m_pbuf + (m_x * line) * sizeof(uint32_t);
}

void CTargaImage::Create(int32_t x, int32_t y)
{
    memset(&m_header, 0, sizeof(m_header));

    m_header.imgType	= 2;
    m_header.width	= (uint16_t)x;
    m_header.height	= (uint16_t)y;
    m_header.colorBits	= 32;
    m_header.desc	= 0x20;

    m_pbuf = new char[x * y * sizeof(uint32_t)];
    memset(m_pbuf, 0, x * y * sizeof(uint32_t));

    m_x = x;
    m_y = y;
}

bool CTargaImage::Save(const char * filename)
{
    auto fp = msl::file_ptr(filename, "wb");
    if (!fp)
		return false;

    fp.write(&m_header, sizeof(TGA_HEADER));
    fp.write(m_pbuf, m_x * m_y * sizeof(uint32_t));

    return true;
}
