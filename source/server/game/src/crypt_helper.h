#pragma once
#include <cstdint>
#include <string>

#include "../../libcrypt/include/DataBuffer.h"

#if 0

class CMd5
{
public:
	CMd5() = default;
	~CMd5() = default;

	std::string BuildHash(const void * data, uint32_t size);
};

class CSha256
{
public:
	CSha256() = default;
	~CSha256() = default;

	std::string BuildHash(const void * data, uint32_t size);
};

class CAes256
{
public:
	CAes256() = default;
	~CAes256() = default;

	DataBuffer Encrypt(const uint8_t * data, uint32_t size, const std::string & iv, const uint8_t * key);
	DataBuffer Decrypt(const uint8_t * data, uint32_t size, const std::string & iv, const uint8_t * key);
};
#endif

