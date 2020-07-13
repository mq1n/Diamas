#pragma once
#include <Windows.h>
#include <lzo/lzo_asm.h>
#include <lzo/lzo1x.h>
#include <memory>
#include <cstdint>
#include "../../common/singleton.h"

class CLZObject
{
public:
#pragma pack(4)
	typedef struct SHeader
	{
		uint32_t	dwFourCC;
		uint32_t	dwEncryptSize;		// 암호화된 크기
		uint32_t	dwCompressedSize;	// 압축된 데이터 크기
		uint32_t	dwRealSize;		// 실제 데이터 크기
	} THeader;
#pragma pack()
		
	CLZObject();
	~CLZObject();
		
	void Clear();
		
	void BeginCompress(const void * pvIn, uint32_t uiInLen);
	void BeginCompressInBuffer(const void * pvIn, uint32_t uiInLen, void * pvOut);
	bool Compress();

	bool BeginDecompress(const void * pvIn);
	bool Decompress(uint32_t * pdwKey = nullptr);

	bool Encrypt(uint32_t * pdwKey) const;
	bool Decrypt(uint32_t * key, uint8_t * data) const;

	const THeader & GetHeader() const		{ return *m_pHeader; }
	uint8_t *		GetBuffer() const		{ return m_pbBuffer; }
	uint32_t		GetBufferSize() const	{ return m_dwBufferSize; }
	uint32_t		GetSize() const;

	void			AllocBuffer(uint32_t dwSrcSize);

protected:
	void Initialize();

private:
	uint8_t *		m_pbBuffer;
	uint32_t		m_dwBufferSize;

	THeader *		m_pHeader;
	const uint8_t *	m_pbIn;
	bool			m_bCompressed;

	bool			m_bInBuffer;

public:
	static uint32_t ms_dwFourCC;
};

class CLZO : public CSingleton <CLZO>
{
public:
	CLZO();
	virtual ~CLZO();

	bool	CompressMemory(CLZObject & rObj, const void * pIn, uint32_t uiInLen);
	bool	CompressEncryptedMemory(CLZObject & rObj, const void * pIn, uint32_t uiInLen, uint32_t * pdwKey);
	bool	Decompress(CLZObject & rObj, const uint8_t * pbBuf, uint32_t * pdwKey = nullptr);
	uint8_t * GetWorkMemory() const;
		
	size_t	GetMaxCompressedSize(size_t original) {	return (original + (original >> 4) + 64 + 3);	}

private:
	uint8_t * m_pWorkMem;
};
