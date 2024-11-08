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

#pragma once

#include "sha.h"
#include <string>

class DataBuffer;

class SHA256_Impl : private SHA
{
public:
	SHA256_Impl(cl_sha_type new_sha_type);

	std::string get_hash(bool uppercase = false) const;
	void get_hash(uint8_t *out_hash) const;

	void reset();
	void set_hmac(const void *key_data, int32_t key_size);
	void add(const void *data, int32_t size);
	void calculate();

private:
	inline uint32_t sigma_rr2_rr13_rr22(uint32_t value) const
	{
		return (rightrotate_uint32(value, 2) ^ rightrotate_uint32(value, 13) ^ rightrotate_uint32(value, 22));
	}

	inline uint32_t sigma_rr6_rr11_rr25(uint32_t value) const
	{
		return (rightrotate_uint32(value, 6) ^ rightrotate_uint32(value, 11) ^ rightrotate_uint32(value, 25));
	}

	inline uint32_t sigma_rr7_rr18_sr3(uint32_t value) const
	{
		return (rightrotate_uint32(value, 7) ^ rightrotate_uint32(value, 18) ^ (value >> 3));
	}

	inline uint32_t sigma_rr17_rr19_sr10(uint32_t value) const
	{
		return (rightrotate_uint32(value, 17) ^ rightrotate_uint32(value, 19) ^ (value >> 10));
	}

	inline uint32_t sha_ch(uint32_t x, uint32_t y, uint32_t z) const
	{
		return (((x)& ((y) ^ (z))) ^ (z));
	}

	inline uint32_t sha_maj(uint32_t x, uint32_t y, uint32_t z) const
	{
		return  (((x)& ((y) | (z))) | ((y)& (z)));
	}

	void process_chunk();

	uint32_t h0, h1, h2, h3, h4, h5, h6, h7;
	const static int32_t block_size = 64;
	uint8_t chunk[block_size];
	int32_t chunk_filled;
	uint64_t length_message;
	bool calculated;

	cl_sha_type sha_type;
	bool hmac_enabled;
	uint8_t hmac_key_chunk[block_size];
};

class SHA256
{
public:
	/// \brief Constructs a SHA-256 hash generator.
	SHA256();

	static const int32_t hash_size = 32;

	/// \brief Returns the calculated hash.
	std::string get_hash(bool uppercase = false) const;

	/// \brief Get hash
	///
	/// \param out_hash = where to write to
	void get_hash(uint8_t out_hash[hash_size]) const;

	/// \brief Resets the hash generator.
	void reset();

	/// \brief Enable a HMAC based calculation
	///
	/// Call this function before the initial add(). It is reset by reset()
	///
	/// \param key_data = The HMAC key
	/// \param key_size = The size of the key_data
	void set_hmac(const void *key_data, int32_t key_size);

	/// \brief Adds data to be hashed.
	void add(const void *data, int32_t size);

	/// \brief Add
	///
	/// \param data = Data Buffer
	void add(const DataBuffer &data);

	/// \brief Finalize hash calculation.
	void calculate();

private:
	std::shared_ptr<SHA256_Impl> impl;
};

