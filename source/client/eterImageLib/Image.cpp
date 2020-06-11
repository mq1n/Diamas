#include "StdAfx.h"

#include <cassert>
#include "Image.h"

CImageC::CImageC(CImageC & image)
{
	Initialize();

	int32_t w = image.GetWidth();
	int32_t h = image.GetHeight();

	Create(w, h);

	uint32_t * pdwDest = GetBasePointer();
	uint32_t * pdwSrc = image.GetBasePointer();

	memcpy(pdwDest, pdwSrc, w * h * sizeof(uint32_t));
}

void CImageC::SetFileName(const char* c_szFileName)
{
	m_stFileName = c_szFileName;
}

const std::string& CImageC::GetFileNameString()
{
	return m_stFileName;
}

void CImageC::PutImage(int32_t x, int32_t y, CImageC* pImage)
{
	assert(x >= 0 && x + pImage->GetWidth() <= GetWidth());
	assert(y >= 0 && y + pImage->GetHeight() <= GetHeight());

	int32_t len = pImage->GetWidth() * sizeof(uint32_t);

	for (int32_t j = 0; j < pImage->GetHeight(); ++j)
	{
		uint32_t * pdwDest = GetLinePointer(y + j) + x;
		memcpy(pdwDest, pImage->GetLinePointer(j), len);
	}
}

uint32_t* CImageC::GetBasePointer()
{
	assert(m_pdwColors != nullptr);
	return m_pdwColors;
}

uint32_t* CImageC::GetLinePointer(int32_t line)
{
	assert(m_pdwColors != nullptr);
	return m_pdwColors + line * m_width;
}

int32_t CImageC::GetWidth() const
{
	assert(m_pdwColors != nullptr);
	return m_width;
}

int32_t CImageC::GetHeight() const
{
	assert(m_pdwColors != nullptr);
	return m_height;
}

void CImageC::Clear(uint32_t color)
{
	assert(m_pdwColors != nullptr);

	for (int32_t y = 0; y < m_height; ++y)
	{
		uint32_t * colorLine = &m_pdwColors[y * m_width];

		for (int32_t x = 0; x < m_width; ++x)
			colorLine[x] = color;
	}
}

void CImageC::Create(int32_t width, int32_t height)
{
	Destroy();

	m_width = width;
	m_height = height;
	m_pdwColors = new uint32_t[m_width*m_height];
}

void CImageC::Destroy()
{
	if (m_pdwColors)
	{
		delete [] m_pdwColors;
		m_pdwColors = nullptr;
	}
}

void CImageC::Initialize()
{
	m_pdwColors = nullptr;
	m_width = 0;
	m_height = 0;
}

bool CImageC::IsEmpty() const
{
	return (m_pdwColors == nullptr) ? true : false;
}

void CImageC::FlipTopToBottom()
{
	uint32_t * swap = new uint32_t[m_width * m_height];

	int32_t row;
	uint32_t width = GetWidth();
	uint32_t height = GetHeight();
	uint32_t * end_row;
	uint32_t * start_row;

	for (row = 0; row < GetHeight() / 2; row++)
	{
		end_row		= &(m_pdwColors[width * (height - row - 1)]);
		start_row	= &(m_pdwColors[width * row]);

		memcpy(swap, end_row, width * sizeof(uint32_t));
		memcpy(end_row, start_row, width * sizeof(uint32_t));
		memcpy(start_row, swap, width * sizeof(uint32_t));
	}

	delete [] swap;
}

CImageC::CImageC()
{
	Initialize();
}

CImageC::~CImageC()
{
	Destroy();
}
