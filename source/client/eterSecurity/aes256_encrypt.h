#pragma once

/*
**  ClanLib SDK
**  Copyright (c) 1997-2016 The ClanLib Team
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**  Note: Some of the libraries ClanLib may link to may have additional
**  requirements or restrictions.
**
**  File Author(s):
**
**    Mark Page
*/

#include "aes.h"
#include "DataBuffer.hpp"

#include <memory>

class AES256_Encrypt_Impl : public AES_Impl
{
public:
	AES256_Encrypt_Impl();

	/// \brief Get encrypted data
	///
	/// This is the databuffer used internally to store the encrypted data.
	/// You may call "set_size()" to clear the buffer, inbetween calls to "add()"
	/// You may call "set_capacity()" to optimise storage requirements before the add() call
	DataBuffer get_data() const;

	/// \brief Resets the encryption
	void reset();

	/// \brief Purge the databuffer
	///
	/// See get_data()\n
	/// reset() calls this function
	void purge_data();

	/// \brief Sets the initialisation vector
	///
	/// This must be called before the initial add()
	void set_iv(const uint8_t iv[16]);

	/// \brief Sets the cipher key
	///
	/// This must be called before the initial add()
	void set_key(const uint8_t key[32]);

	void set_padding(bool value, bool use_pkcs7, uint32_t num_additional_padded_blocks);

	/// \brief Adds data to be encrypted
	void add(const void *data, int32_t size);

	/// \brief Add data to be encrypted
	///
	/// \param data = Data Buffer
	void add(const DataBuffer &data);

	/// \brief Finalize decryption
	void calculate();

private:
	void process_chunk();

	uint32_t key_expanded[aes256_nb_mult_nr_plus1];

	uint8_t chunk[aes256_block_size_bytes];
	uint32_t initialisation_vector_1;
	uint32_t initialisation_vector_2;
	uint32_t initialisation_vector_3;
	uint32_t initialisation_vector_4;

	int32_t chunk_filled;

	bool initialisation_vector_set;
	bool cipher_key_set;
	bool calculated;
	bool padding_enabled;
	bool padding_pkcs7;
	uint32_t padding_num_additional_padded_blocks;

	DataBuffer databuffer;
};


class AES256_Encrypt
{
public:
	/// \brief Constructs a AES-256 generator (running in Cipher Block Chaining mode)
	AES256_Encrypt();

	/// \brief Get encrypted data
	///
	/// This is the databuffer used internally to store the encrypted data.
	/// You may call "set_size()" to clear the buffer, inbetween calls to "add()"
	/// You may call "set_capacity()" to optimise storage requirements before the add() call
	DataBuffer get_data() const;

	static const int32_t iv_size = 16;
	static const int32_t key_size = 32;
	static const int32_t block_size = 16;

	/// \brief Resets the encryption
	void reset();

	/// \brief Sets the initialisation vector
	///
	/// This should be a random number\n
	/// This must be called before the initial add()
	void set_iv(const uint8_t iv[iv_size]);

	/// \brief Sets the cipher key
	///
	/// This must be called before the initial add()
	void set_key(const uint8_t key[key_size]);

	/// \brief Enable AES Padding
	///
	/// Example (use_pkcs7==true) : ... 0x03 0x03 0x03 (3 octets of padding)
	/// Example (use_pkcs7==false) : ... 0x02 0x02 0x02 (3 octets of padding, the last octet is the length)
	///
	/// \param value = true = Enable padding (default)
	/// \param use_pkcs7 = true = This uses the PKCS#7/RFC3369 method (Enabled by default). false = use the TLS method (rfc2246)
	/// \param num_additional_padded_blocks = (Only valid when use_pkcs7==false). Set to "frustrate attacks on a protocol based on analysis of the lengths of exchanged messages". (Range 0 to 15)
	void set_padding(bool value = true, bool use_pkcs7 = true, uint32_t num_additional_padded_blocks = 0);

	/// \brief Adds data to be encrypted
	void add(const void *data, int32_t size);

	/// \brief Add data to be encrypted
	///
	/// \param data = Data Buffer
	void add(const DataBuffer &data);

	/// \brief Finalize encryption
	void calculate();

private:
	std::shared_ptr<AES256_Encrypt_Impl> impl;
};