#include "StdAfx.h"
#include "MarkImage.h"
#include "../eterBase/lzo.h"

CGuildMarkImage::CGuildMarkImage()
{
	m_uImg = INVALID_HANDLE;
}

CGuildMarkImage::~CGuildMarkImage()
{
	Destroy();
}

void CGuildMarkImage::Destroy()
{
	if (INVALID_HANDLE == m_uImg)
		return;

	ilDeleteImages(1, &m_uImg);
	m_uImg = INVALID_HANDLE;
}

void CGuildMarkImage::Create()
{
	if (INVALID_HANDLE != m_uImg)
		return;

	ilGenImages(1, &m_uImg);
}

bool CGuildMarkImage::Save(const char * c_szFileName) const
{
	ilEnable(IL_FILE_OVERWRITE);
	ilBindImage(m_uImg);

	if (!ilSave(IL_TGA, c_szFileName))
		return false;

	return true;
}

bool CGuildMarkImage::Build(const char * c_szFileName)
{
	Destroy();
	Create();

	ilBindImage(m_uImg);
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

	std::vector<uint8_t> data(sizeof(Pixel) * WIDTH * HEIGHT);

	if (!ilTexImage(WIDTH, HEIGHT, 1, 4, IL_BGRA, IL_UNSIGNED_BYTE, data.data()))
	{
		TraceError("CGuildMarkImage: cannot initialize image");
		return false;
	}

	ilEnable(IL_FILE_OVERWRITE);

	if (!ilSave(IL_TGA, c_szFileName))
		return false;

	return true;
}

bool CGuildMarkImage::Load(const char * c_szFileName) 
{
	Destroy();
	Create();	

	ilBindImage(m_uImg);
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

	if (!ilLoad(IL_TYPE_UNKNOWN, c_szFileName))
	{
		Build(c_szFileName);
		
		if (!Load(c_szFileName))
		{
			TraceError("CGuildMarkImage: cannot open file for writing %s", c_szFileName);
			return false;
		}
	}

	if (ilGetInteger(IL_IMAGE_WIDTH) != WIDTH)	
	{
		TraceError("CGuildMarkImage: %s width must be %u", c_szFileName, WIDTH);
		return false;
	}

	if (ilGetInteger(IL_IMAGE_HEIGHT) != HEIGHT)
	{
		TraceError("CGuildMarkImage: %s height must be %u", c_szFileName, HEIGHT);
		return false;
	}

	ilConvertImage(IL_BGRA, IL_UNSIGNED_BYTE);

	BuildAllBlocks();
	return true;
}

void CGuildMarkImage::PutData(uint32_t x, uint32_t y, uint32_t width, uint32_t height, void * data) const
{
	ilBindImage(m_uImg);
	ilSetPixels(x, y, 0, width, height, 1, IL_BGRA, IL_UNSIGNED_BYTE, data);
}

void CGuildMarkImage::GetData(uint32_t x, uint32_t y, uint32_t width, uint32_t height, void * data) const
{
	ilBindImage(m_uImg);
	ilCopyPixels(x, y, 0, width, height, 1, IL_BGRA, IL_UNSIGNED_BYTE, data);	
}

// 이미지 = 512x512
//   블럭 = 마크 4 x 4
//   마크 = 16 x 12
// 한 이미지의 블럭 = 8 x 10

// SERVER
bool CGuildMarkImage::SaveMark(uint32_t posMark, uint8_t * pbImage)
{
	if (posMark >= MARK_TOTAL_COUNT)
	{
		TraceError("CGuildMarkImage::CopyMarkFromData: Invalid mark position %u", posMark);
		return false;
	}

	// 마크를 전체 이미지에 그린다.
	uint32_t colMark = posMark % MARK_COL_COUNT;
	uint32_t rowMark = posMark / MARK_COL_COUNT;

#ifdef _DEBUG
	printf("PutMark pos %u %ux%u\n", posMark, colMark * SGuildMark::WIDTH, rowMark * SGuildMark::HEIGHT);
#endif
	PutData(colMark * SGuildMark::WIDTH, rowMark * SGuildMark::HEIGHT, SGuildMark::WIDTH, SGuildMark::HEIGHT, pbImage);
	uint32_t rowBlock = rowMark / SGuildMarkBlock::MARK_PER_BLOCK_HEIGHT;
	uint32_t colBlock = colMark / SGuildMarkBlock::MARK_PER_BLOCK_WIDTH;

	Pixel apxBuf[SGuildMarkBlock::SIZE];
	GetData(colBlock * SGuildMarkBlock::WIDTH, rowBlock * SGuildMarkBlock::HEIGHT, SGuildMarkBlock::WIDTH, SGuildMarkBlock::HEIGHT, apxBuf);
	m_aakBlock[rowBlock][colBlock].Compress(apxBuf);
	return true;
}

bool CGuildMarkImage::DeleteMark(uint32_t posMark)
{
	Pixel image[SGuildMark::SIZE];
	memset(&image, 0, sizeof(image));
	return SaveMark(posMark, reinterpret_cast<uint8_t *>(&image));
}

// CLIENT
bool CGuildMarkImage::SaveBlockFromCompressedData(uint32_t posBlock, const uint8_t * pbComp, uint32_t dwCompSize)
{
	if (posBlock >= BLOCK_TOTAL_COUNT)
		return false;

	Pixel apxBuf[SGuildMarkBlock::SIZE];
	size_t sizeBuf = sizeof(apxBuf);

	auto lzoDecompressResult = lzo1x_decompress_safe(pbComp, dwCompSize, reinterpret_cast<uint8_t *>(apxBuf), reinterpret_cast<lzo_uint *>(&sizeBuf),
		CLZO::Instance().GetWorkMemory());
	if (LZO_E_OK != lzoDecompressResult)
	{
		TraceError("CGuildMarkImage::CopyBlockFromCompressedData: cannot decompress, compressed size = %u [errorID: %d]", dwCompSize, lzoDecompressResult);
		return false;
	}

	if (sizeBuf != sizeof(apxBuf))
	{
		TraceError("CGuildMarkImage::CopyBlockFromCompressedData: image corrupted, decompressed size = %u", sizeBuf);
		return false;
	}

	uint32_t rowBlock = posBlock / BLOCK_COL_COUNT;
	uint32_t colBlock = posBlock % BLOCK_COL_COUNT;

	PutData(colBlock * SGuildMarkBlock::WIDTH, rowBlock * SGuildMarkBlock::HEIGHT, SGuildMarkBlock::WIDTH, SGuildMarkBlock::HEIGHT, apxBuf);

	m_aakBlock[rowBlock][colBlock].CopyFrom(pbComp, dwCompSize,
											GetCRC32(reinterpret_cast<const char *>(apxBuf), sizeof(Pixel) * SGuildMarkBlock::SIZE));
	return true;
}

void CGuildMarkImage::BuildAllBlocks() 
{
	Pixel apxBuf[SGuildMarkBlock::SIZE];
	Tracenf("CGuildMarkImage::BuildAllBlocks");

	for (uint32_t row = 0; row < BLOCK_ROW_COUNT; ++row)
	{
		for (uint32_t col = 0; col < BLOCK_COL_COUNT; ++col)
		{
			GetData(col * SGuildMarkBlock::WIDTH, row * SGuildMarkBlock::HEIGHT, SGuildMarkBlock::WIDTH, SGuildMarkBlock::HEIGHT, apxBuf);
			m_aakBlock[row][col].Compress(apxBuf);
		}
		}
}

uint32_t CGuildMarkImage::GetEmptyPosition()
{
	SGuildMark kMark;

	for (uint32_t row = 0; row < MARK_ROW_COUNT; ++row)
	{
		for (uint32_t col = 0; col < MARK_COL_COUNT; ++col)
		{
			GetData(col * SGuildMark::WIDTH, row * SGuildMark::HEIGHT, SGuildMark::WIDTH, SGuildMark::HEIGHT, kMark.m_apxBuf);

			if (kMark.IsEmpty())
				return (row * MARK_COL_COUNT + col);
		}
	}

	return INVALID_MARK_POSITION;
}

void CGuildMarkImage::GetDiffBlocks(const uint32_t * crcList, std::map<uint8_t, const SGuildMarkBlock *> & mapDiffBlocks)
{
	uint8_t posBlock = 0;

	for (auto & row : m_aakBlock)
	{
		for (auto & col : row)
		{
			if (col.m_crc != *crcList)
				mapDiffBlocks.emplace(posBlock, &col);
			++crcList;
			++posBlock;
		}
	}
}

void CGuildMarkImage::GetBlockCRCList(uint32_t * crcList)
{
	for (auto & row : m_aakBlock)
	{
		for (const auto & col : row)
			*(crcList++) = col.GetCRC();
	}
}
void SGuildMark::Clear()
{
	for (auto & iPixel : m_apxBuf)
		iPixel = 0xff000000;
}

bool SGuildMark::IsEmpty()
{
	for (auto & iPixel : m_apxBuf)
	{
		if (iPixel != 0x00000000)
			return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
uint32_t SGuildMarkBlock::GetCRC() const
{
	return m_crc;
}

void SGuildMarkBlock::CopyFrom(const uint8_t * pbCompBuf, uint32_t dwCompSize, uint32_t crc)
{
	if (dwCompSize > MAX_COMP_SIZE)
		return;

	m_sizeCompBuf = dwCompSize;
	memcpy(m_abCompBuf, pbCompBuf, dwCompSize);
	m_crc = crc;
}

void SGuildMarkBlock::Compress(const Pixel * pxBuf)
{
	m_sizeCompBuf = MAX_COMP_SIZE;

	if (LZO_E_OK !=
		lzo1x_999_compress(reinterpret_cast<const uint8_t *>(pxBuf), sizeof(Pixel) * SIZE, m_abCompBuf,
						   reinterpret_cast<lzo_uint *>(&m_sizeCompBuf), CLZO::Instance().GetWorkMemory()))
	{
		TraceError("SGuildMarkBlock::Compress: Error! %u > %u", sizeof(Pixel) * SIZE, m_sizeCompBuf);
		return;
	}

	m_crc = GetCRC32(reinterpret_cast<const char *>(pxBuf), sizeof(Pixel) * SIZE);
}
