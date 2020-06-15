#pragma once

#ifdef RSA_NET_PACKET_ENCRYPTION

#define NET_CRYPT_KEY_LENGTH 32
#define RSA_KEY_ROUND 2

#include "../../libcrypt/DataBuffer.h"
#include "../../libcrypt/Secret.h"
#include "../../libcrypt/Random.h"

class Crypt_Client
{
public:
	Crypt_Client() = default;

	void create_keypair();
	void set_crypt_key(const DataBuffer &public_key);

	DataBuffer get_modulus();
	DataBuffer get_public_exponent();
	Secret get_crypt_key();

private:
	Random m_Random;

	DataBuffer m_WrappedCryptKey;
	Secret m_CryptKey;

	Secret m_private_exponent;
	DataBuffer m_public_exponent;
	DataBuffer m_modulus;
};

class Crypt_Server
{
public:
	Crypt_Server() = default;

	void create_keypair();
	void create_crypt_key();

	const char* export_wrap_crypt_key(const uint8_t* pPublicExponent, uint32_t ulPublicExponentSize, const uint8_t* pModulus, uint32_t ulModulusSize, uint32_t * pKeySize);

	Secret get_crypt_key();

private:
	Random m_Random;
	Secret m_CryptKey;
	DataBuffer m_WrappedCryptKey;

	Secret m_private_exponent;
	DataBuffer m_public_exponent;
	DataBuffer m_modulus;
};

#endif

