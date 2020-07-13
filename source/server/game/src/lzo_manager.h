#pragma once
#include "minilzo.h"

class LZOManager : public CSingleton<LZOManager>
{
	public:
		LZOManager();
		virtual ~LZOManager();

		bool	Compress(const uint8_t* src, size_t srcsize, uint8_t* dest, lzo_uint * puiDestSize);
		bool	Decompress(const uint8_t* src, size_t srcsize, uint8_t* dest, lzo_uint * puiDestSize);
		size_t	GetMaxCompressedSize(size_t original);

		uint8_t *	GetWorkMemory() { return m_workmem; }

	private:
		uint8_t *	m_workmem;
};
