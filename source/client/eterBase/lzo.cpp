#include "StdAfx.h"
#include <cstdlib>
#include <lzo/lzodefs.h>
#include "lzo.h"
#include "tea.h"
#include "debug.h"

#define dbg_printf

static class LZOFreeMemoryMgr
{
public:
	enum
	{
		REUSING_CAPACITY = 64*1024,
	};
public:
	~LZOFreeMemoryMgr()
	{
		for(size_t i = 0; i < m_freeVector.size(); ++i)
		{
			delete m_freeVector[i];
		}
		m_freeVector.clear();
	}
	uint8_t* Alloc(uint32_t capacity)
	{
		assert(capacity > 0);
		if (capacity < REUSING_CAPACITY)
		{
			if (!m_freeVector.empty())
			{
				uint8_t* freeMem = m_freeVector.back();
				m_freeVector.pop_back();

				dbg_printf("lzo.reuse_alloc\t%p(%d) free\n", freeMem, capacity);
				return freeMem;
			}
			uint8_t* newMem = new uint8_t[REUSING_CAPACITY];
			dbg_printf("lzo.reuse_alloc\t%p(%d) real\n", newMem, capacity);
			return newMem;
		}
		uint8_t* newMem = new uint8_t[capacity];
		dbg_printf("lzo.real_alloc\t%p(%d)\n", newMem, capacity);
		return newMem;
	}
	void Free(uint8_t* ptr, uint32_t capacity)
	{
		assert(ptr != nullptr);
		assert(capacity > 0);
		if (capacity < REUSING_CAPACITY)
		{
			dbg_printf("lzo.reuse_free\t%p(%d)\n", ptr, capacity);
			m_freeVector.push_back(ptr);
			return;
		}

		dbg_printf("lzo.real_free\t%p(%d)\n", ptr, capacity);
		delete [] ptr;
	}
private:
	std::vector<uint8_t*> m_freeVector;
} gs_freeMemMgr;



uint32_t CLZObject::ms_dwFourCC = MAKEFOURCC('M', 'C', 'O', 'Z');

CLZObject::CLZObject()
{
    Initialize();
}

void CLZObject::Initialize()
{
	m_bInBuffer = false;
    m_pbBuffer = nullptr;
    m_dwBufferSize = 0;

    m_pHeader = nullptr;
    m_pbIn = nullptr;
    m_bCompressed = false;
}

void CLZObject::Clear()
{
    if (m_pbBuffer && !m_bInBuffer)
		gs_freeMemMgr.Free(m_pbBuffer, m_dwBufferSize);
	
	if (m_dwBufferSize > 0)
	{
		dbg_printf("lzo.free %d\n", m_dwBufferSize);
	}

    Initialize();
}

CLZObject::~CLZObject()
{
    Clear();
}

uint32_t CLZObject::GetSize()
{
	assert(m_pHeader);

	if (m_bCompressed)
	{
		if (m_pHeader->dwEncryptSize)
			return sizeof(THeader) + sizeof(uint32_t) + m_pHeader->dwEncryptSize;
		else
			return sizeof(THeader) + sizeof(uint32_t) + m_pHeader->dwCompressedSize;
	}
	else
		return m_pHeader->dwRealSize;
}

void CLZObject::BeginCompress(const void * pvIn, uint32_t uiInLen)
{
    m_pbIn = (const uint8_t *) pvIn;
	
    // sizeof(SHeader) +
    // 암호화를 위한 fourCC 4바이트
    // 압축된 후 만들어질 수 있는 최대 용량 +
    // 암호화를 위한 8 바이트
    m_dwBufferSize = sizeof(THeader) + sizeof(uint32_t) + (uiInLen + uiInLen / 64 + 16 + 3) + 8;
	
    m_pbBuffer = gs_freeMemMgr.Alloc(m_dwBufferSize);
    memset(m_pbBuffer, 0, m_dwBufferSize);
	
    m_pHeader = (THeader *) m_pbBuffer;
    m_pHeader->dwFourCC = ms_dwFourCC;
    m_pHeader->dwEncryptSize = m_pHeader->dwCompressedSize = 0;
    m_pHeader->dwRealSize = uiInLen;
}

void CLZObject::BeginCompressInBuffer(const void * pvIn, uint32_t uiInLen, void * /*pvOut*/)
{
    m_pbIn = (const uint8_t *) pvIn;
	
    // sizeof(SHeader) +
    // 암호화를 위한 fourCC 4바이트
    // 압축된 후 만들어질 수 있는 최대 용량 +
    // 암호화를 위한 8 바이트
    m_dwBufferSize = sizeof(THeader) + sizeof(uint32_t) + (uiInLen + uiInLen / 64 + 16 + 3) + 8;
	
    m_pbBuffer = gs_freeMemMgr.Alloc(m_dwBufferSize);
    memset(m_pbBuffer, 0, m_dwBufferSize);
	
    m_pHeader = (THeader *) m_pbBuffer;
    m_pHeader->dwFourCC = ms_dwFourCC;
    m_pHeader->dwEncryptSize = m_pHeader->dwCompressedSize = 0;
    m_pHeader->dwRealSize = uiInLen;
	m_bInBuffer = true;
}

bool CLZObject::Compress()
{
    uint32_t	iOutLen;
    uint8_t *	pbBuffer;
	
    pbBuffer = m_pbBuffer + sizeof(THeader);
    *(uint32_t *) pbBuffer = ms_dwFourCC;
    pbBuffer += sizeof(uint32_t);

	int32_t r = lzo1x_999_compress((uint8_t *) m_pbIn, m_pHeader->dwRealSize, pbBuffer, (lzo_uint*) &iOutLen, CLZO::Instance().GetWorkMemory());

	
    if (LZO_E_OK != r)
    {
		TraceError("LZO: lzo1x_999_compress failed");
		return false;
    }
	
    m_pHeader->dwCompressedSize = iOutLen;
    m_bCompressed = true;
    return true;
}

bool CLZObject::BeginDecompress(const void * pvIn)
{
	if (pvIn == nullptr)
	{
		TraceError("LZObject: pvIn = nullptr! Corrupted data");
		return false;
	}

    THeader * pHeader = (THeader *) pvIn;

    if (pHeader->dwFourCC != ms_dwFourCC)
    {
		TraceError("LZObject: not a valid data");
		return false;
    }
	
    m_pHeader	= pHeader;
    m_pbIn	= (const uint8_t *) pvIn + (sizeof(THeader) + sizeof(uint32_t));

	/*
	static uint32_t sum = 0;
	static uint32_t count = 0;
	sum += pHeader->dwRealSize;
	count++;
	printf("decompress cur: %d, ave: %d\n", pHeader->dwRealSize, sum/count);
	*/
	m_dwBufferSize = pHeader->dwRealSize;
	m_pbBuffer = gs_freeMemMgr.Alloc(m_dwBufferSize);
    memset(m_pbBuffer, 0, pHeader->dwRealSize);
    return true;
}

class DecryptBuffer
{
public:
	enum
	{
		LOCAL_BUF_SIZE = 8 * 1024,
	};
public:
	DecryptBuffer(uint32_t size)
	{
		static uint32_t count = 0;
		static uint32_t sum = 0;
		static uint32_t maxSize = 0;

		sum += size;
		count++;

		maxSize = std::max(size, maxSize);
		if (size >= LOCAL_BUF_SIZE)
		{
			m_buf = new char[size];
			dbg_printf("DecryptBuffer - AllocHeap %d max(%d) ave(%d)\n", size, maxSize/1024, sum/count);
		}
		else
		{
			dbg_printf("DecryptBuffer - AllocStack %d max(%d) ave(%d)\n", size, maxSize/1024, sum/count);
			m_buf = m_local_buf;
		}
	}
	~DecryptBuffer()
	{
		if (m_local_buf != m_buf)
		{
			dbg_printf("DecryptBuffer - FreeHeap\n");
			delete [] m_buf;
		}
		else
		{
			dbg_printf("DecryptBuffer - FreeStack\n");
		}
	}
	void* GetBufferPtr()
	{
		return m_buf;
	}

private:
	char*	m_buf;
	char	m_local_buf[LOCAL_BUF_SIZE];
};

bool CLZObject::Decompress(uint32_t * pdwKey)
{
    uint32_t uiSize;
    int32_t r;
	
    if (m_pHeader->dwEncryptSize)
    {
		DecryptBuffer buf(m_pHeader->dwEncryptSize);

		uint8_t* pbDecryptedBuffer = (uint8_t*)buf.GetBufferPtr();
			
		__Decrypt(pdwKey, pbDecryptedBuffer);
		
		if (*(uint32_t *) pbDecryptedBuffer != ms_dwFourCC)
		{
			TraceError("LZObject: key incorrect");
			return false;
		}
		
		if (LZO_E_OK != (r = lzo1x_decompress(pbDecryptedBuffer + sizeof(uint32_t), m_pHeader->dwCompressedSize, m_pbBuffer, (lzo_uint*) &uiSize, nullptr)))
		{
			TraceError("LZObject: Decompress failed(decrypt) ret %d\n", r);
			return false;
		}
    }
    else
    {
		uiSize = m_pHeader->dwRealSize;
		
		//if (LZO_E_OK != (r = lzo1x_decompress_safe(m_pbIn, m_pHeader->dwCompressedSize, m_pbBuffer, (lzo_uint*) &uiSize, nullptr)))
		if (LZO_E_OK != (r = lzo1x_decompress(m_pbIn, m_pHeader->dwCompressedSize, m_pbBuffer, (lzo_uint*) &uiSize, nullptr)))
		{
			TraceError("LZObject: Decompress failed : ret %d, CompressedSize %d\n", r, m_pHeader->dwCompressedSize);
			return false;
		}
    }
	
    if (uiSize != m_pHeader->dwRealSize)
    {
		TraceError("LZObject: Size differs");
		return false;
    }
	
    return true;
}

bool CLZObject::Encrypt(uint32_t * pdwKey)
{
    if (!m_bCompressed)
    {
		assert(!"not compressed yet");
		return false;
    }
	
    uint8_t * pbBuffer = m_pbBuffer + sizeof(THeader);
    m_pHeader->dwEncryptSize = tea_encrypt((uint32_t *) pbBuffer, (const uint32_t *) pbBuffer, pdwKey, m_pHeader->dwCompressedSize + 19);
    return true;
}

bool CLZObject::__Decrypt(uint32_t * key, uint8_t* data)
{
    assert(m_pbBuffer);
		
    tea_decrypt((uint32_t *) data, (const uint32_t *) (m_pbIn - sizeof(uint32_t)), key, m_pHeader->dwEncryptSize);
    return true;
}

void CLZObject::AllocBuffer(uint32_t dwSrcSize)
{
    if (m_pbBuffer && !m_bInBuffer)
		gs_freeMemMgr.Free(m_pbBuffer, m_dwBufferSize);
    
    m_pbBuffer = gs_freeMemMgr.Alloc(dwSrcSize);
	m_dwBufferSize = dwSrcSize;
}
/*
void CLZObject::CopyBuffer(const char* pbSrc, uint32_t dwSrcSize)
{
	AllocBuffer(dwSrcSize);
	memcpy(m_pbBuffer, pbSrc, dwSrcSize);
}
*/

CLZO::CLZO() : m_pWorkMem(nullptr)
{
    if (lzo_init() != LZO_E_OK)
    {
		TraceError("LZO: cannot initialize");
		return;
    }

#if defined( LZO1X_999_MEM_COMPRESS )
    m_pWorkMem = (uint8_t *) malloc(LZO1X_999_MEM_COMPRESS);
#else
    m_pWorkMem = (uint8_t *) malloc(LZO1X_1_MEM_COMPRESS);
#endif

    if (nullptr == m_pWorkMem)
    {
		TraceError("LZO: cannot alloc memory");
		return;
    }
}

CLZO::~CLZO()
{
    if (m_pWorkMem)
    {
		free(m_pWorkMem);
		m_pWorkMem = nullptr;
    }
}

bool CLZO::CompressMemory(CLZObject & rObj, const void * pIn, uint32_t uiInLen)
{
    rObj.BeginCompress(pIn, uiInLen);
    return rObj.Compress();
}

bool CLZO::CompressEncryptedMemory(CLZObject & rObj, const void * pIn, uint32_t uiInLen, uint32_t * pdwKey)
{
    rObj.BeginCompress(pIn, uiInLen);
	
    if (rObj.Compress())
    {
		if (rObj.Encrypt(pdwKey))
			return true;
		
		return false;
    }   
	
    return false;
}   

bool CLZO::Decompress(CLZObject & rObj, const uint8_t * pbBuf, uint32_t * pdwKey)
{
    if (!rObj.BeginDecompress(pbBuf))
		return false;
	
    if (!rObj.Decompress(pdwKey))
		return false;
	
    return true;
}


uint8_t * CLZO::GetWorkMemory()
{   
    return m_pWorkMem;
}

