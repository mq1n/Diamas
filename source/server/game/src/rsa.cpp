#include "stdafx.h"
#include "RSA.h"

#ifdef RSA_NET_PACKET_ENCRYPTION

#include "../../libcrypt/RSA.h"

void Crypt_Client::create_keypair()
{
	RSA::create_keypair(m_Random, m_private_exponent, m_public_exponent, m_modulus);
}

void Crypt_Client::set_crypt_key(const DataBuffer &public_key)
{
	m_WrappedCryptKey = public_key;
	m_CryptKey = RSA::decrypt(m_private_exponent, m_modulus, public_key);
}

DataBuffer Crypt_Client::get_modulus()
{
	return m_modulus;
}

DataBuffer Crypt_Client::get_public_exponent()
{
	return m_public_exponent;
}

Secret Crypt_Client::get_crypt_key()
{
	return m_CryptKey;
}

/////////////////////////////////

void Crypt_Server::create_keypair()
{
	RSA::create_keypair(m_Random, m_private_exponent, m_public_exponent, m_modulus);
}

void Crypt_Server::create_crypt_key()
{
	m_CryptKey = Secret(NET_CRYPT_KEY_LENGTH);
	m_Random.get_random_bytes(m_CryptKey.get_data(), m_CryptKey.get_size());
	m_WrappedCryptKey = RSA::encrypt(2, m_Random, m_public_exponent, m_modulus, m_CryptKey);
}

const char* Crypt_Server::export_wrap_crypt_key(const uint8_t* pPublicExponent, uint32_t ulPublicExponentSize, const uint8_t* pModulus, uint32_t ulModulusSize, uint32_t * pKeySize)
{
	auto _pPublicExponent = DataBuffer(pPublicExponent, ulPublicExponentSize);
	auto _pModulus = DataBuffer(pModulus, ulModulusSize);

	auto ret = RSA::encrypt(RSA_KEY_ROUND, m_Random, _pPublicExponent, _pModulus, m_CryptKey);
	if (ret.is_null())
		return nullptr;

	if (pKeySize) *pKeySize = ret.get_size();
	return ret.get_data();
}

Secret Crypt_Server::get_crypt_key()
{
	return m_CryptKey;
}


#endif

