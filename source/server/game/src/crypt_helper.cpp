#include "stdafx.h"
#include "crypt_helper.h"
#include "../../common/service.h"
#include "../../libcrypt/include/DataBuffer.h"
#include "../../libcrypt/include/sha256.h"
#include "../../libcrypt/include/md5.h"
#include "../../libcrypt/include/aes256_decrypt.h"
#include "../../libcrypt/include/aes256_encrypt.h"
#include "../../libcrypt/include/Exception.h"

#if 0
static void convert_ascii(const char *src, std::vector<unsigned char> &dest)
{
	std::size_t len = strlen(src) / 2;
	dest.resize(len);

	for (std::size_t cnt = 0; cnt < len; cnt++)
	{
		unsigned int let1 = toupper(*(src++));
		unsigned int let2 = toupper(*(src++));
		let1 >= 'A' ? let1 = let1 - 'A' + 10 : let1 = let1 - '0';
		let2 >= 'A' ? let2 = let2 - 'A' + 10 : let2 = let2 - '0';
		dest[cnt] = let1 << 4 | let2;
	}
}


DataBuffer CAes256::Encrypt(const uint8_t * data, uint32_t size, const std::string & iv, const uint8_t * key)
{
	DataBuffer pBuffer;

	try
	{
		std::vector <unsigned char> _iv;
		convert_ascii(iv.c_str(), _iv);

		AES256_Encrypt aes256_encrypt;
		aes256_encrypt.set_padding(true);

		aes256_encrypt.set_iv(&_iv[0]);
		aes256_encrypt.set_key(key);
		aes256_encrypt.add(data, size);
		aes256_encrypt.calculate();

		pBuffer = aes256_encrypt.get_data();
	}
	catch (Exception & e)
	{
		sys_err("CAes256|Encrypt exception! Data: %p Size: %u Reason: %s", data, size, e.what());
	}
	catch (...)
	{
		sys_err("CAes256|Encrypt unhandled exception!");
	}

	return pBuffer;
}

DataBuffer CAes256::Decrypt(const uint8_t * data, uint32_t size, const std::string & iv, const uint8_t * key)
{
	DataBuffer pBuffer;

	try
	{
		std::vector<unsigned char> _iv;
		convert_ascii(iv.c_str(), _iv);

		AES256_Decrypt aes256_decrypt;
		aes256_decrypt.set_padding(true);

		aes256_decrypt.set_iv(&_iv[0]);
		aes256_decrypt.set_key(key);
		aes256_decrypt.add(data, size);

		bool result = aes256_decrypt.calculate();
		if (result)
			pBuffer = aes256_decrypt.get_data();
	}
	catch (Exception & e)
	{
		sys_err("CAes256|Decrypt exception! Data: %p Size: %u Reason: %s", data, size, e.what());
	}
	catch (...)
	{
		sys_err("CAes256|Decrypt unhandled exception!");
	}

	return pBuffer;
}


std::string CSha256::BuildHash(const void * data, uint32_t size)
{
	auto strHash = std::string("");

	try
	{
		SHA256 sha256;
		sha256.add(data, size);
		sha256.calculate();

		strHash = sha256.get_hash();
	}
	catch (Exception & e)
	{
		sys_err("CSha256|BuildHash exception! Data: %p Size: %u Reason: %s", data, size, e.what());
	}
	catch (...)
	{
		sys_err("CSha256|BuildHash unhandled exception!");
	}

	return strHash;
}


std::string CMd5::BuildHash(const void * data, uint32_t size)
{
	auto strHash = std::string("");

	try
	{
		MD5 md5buf;
		md5buf.add(data, size);
		md5buf.calculate();

		strHash = md5buf.get_hash();
	}
	catch (Exception & e)
	{
		sys_err("CMd5|BuildHash exception! Data: %p Size: %u Reason: %s", data, size, e.what());
	}
	catch (...)
	{
		sys_err("CMd5|BuildHash unhandled exception!");
	}

	return strHash;
}

#endif
