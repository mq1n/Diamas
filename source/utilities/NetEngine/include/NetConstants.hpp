#pragma once
#include <cstdint>
#include <unordered_map>
#include <list>
#include <array>
#include <memory>

#define NET_CREATEMAGIC(b0, b1, b2, b3) \
	(uint32_t(uint8_t(b0)) | (uint32_t(uint8_t(b1)) << 8) | \
	(uint32_t(uint8_t(b2)) << 16) | (uint32_t(uint8_t(b3)) << 24))

// #define ENABLE_CRYPT

namespace net_engine
{
	static const auto PACKET_MAGIC 				= NET_CREATEMAGIC('S', 'N', 'E', 'T');
	static const auto PACKET_VERSION			= 1;
	static const auto PACKET_CRYPT_KEY_LENGTH	= 32;

	using TPacketCryptKey = std::array <uint8_t, PACKET_CRYPT_KEY_LENGTH>;
	static const auto DEFAULT_CRYPT_KEY = TPacketCryptKey // TODO: XOR
	{
		// Key: 0 - 32
		0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
		0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2
	};

#if 0
	enum EPacketFlags
	{
		PACKET_FLAG_RAW  	= 1 << 0,
		PACKET_FLAG_LZ4  	= 1 << 1,
		PACKET_FLAG_LZO  	= 1 << 2,
		PACKET_FLAG_ZLIB	= 1 << 3,
		PACKET_FLAG_AES256	= 1 << 4,
		PACKET_FLAG_TWOFISH	= 1 << 5,
		PACKET_FLAG_RC5 	= 1 << 6,
		PACKET_FLAG_MAX		= PACKET_FLAG_RC5 | PACKET_FLAG_ZLIB
	};
#endif

	enum ESecurityLevel : uint8_t
	{
		SECURITY_LEVEL_NONE = 0,  // No handshake
		SECURITY_LEVEL_BASIC = 1, // Basic handshake (No keys or Diffie-Hellman)
		SECURITY_LEVEL_XTEA = 2,  // Key/Pong keys
		SECURITY_LEVEL_KEY_AGREEMENT = 3,  // Diffie-Hellman Key agreement
	};
}
