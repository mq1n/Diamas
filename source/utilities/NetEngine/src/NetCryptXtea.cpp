#include "../include/NetEngine.hpp"
#include "../include/Xtea.hpp"

namespace net_engine
{
	XTEACryptation::XTEACryptation()
	{
		memset(m_cryptKey, 0, sizeof(m_cryptKey));
		memset(m_decryptKey, 0, sizeof(m_decryptKey));
	}
	XTEACryptation::~XTEACryptation()
	{
	}

	void XTEACryptation::Encrypt(uint8_t *dst, const uint8_t *src, size_t len)
	{
		net_engine::utils::XTEA::Encrypt(src, dst, len, m_cryptKey, 32);
	}

	void XTEACryptation::Decrypt(uint8_t *dst, const uint8_t *src, size_t len)
	{
		net_engine::utils::XTEA::Decrypt(src, dst, len, m_decryptKey, 32);
	}

	void XTEACryptation::AddData(size_t id, void *data, size_t length)
	{
		if (id == XTEA_CRYPTATION_START_KEY)
		{
			if (length != sizeof(m_cryptKey))
			{
				NET_LOG(LL_ERR, "Invalid XTEA start key");
				return;
			}

			memcpy(m_cryptKey, data, length);
			memcpy(m_decryptKey, data, length);
		}
		else if (id == XTEA_CRYPTATION_LOGIN_DECRYPT_KEY)
		{
			if (length != sizeof(m_decryptKey))
			{
				NET_LOG(LL_ERR, "Invalid login decryptation key size");
				return;
			}

			memcpy(m_decryptKey, data, length);
			__ComputeCryptationKey();
		}
	}

	void XTEACryptation::__ComputeCryptationKey()
	{
		std::vector<uint32_t> randomKey;
		randomKey.resize(175);
		randomKey.reserve(175);

		__GenerateSpecialKey(randomKey);

		return net_engine::utils::XTEA::Encrypt(
			(const uint8_t *)m_decryptKey,
			(uint8_t *)m_cryptKey,
			16,
			(const uint32_t *)(((char *)randomKey.data()) + 37),
			32
		);
	}

	void XTEACryptation::__GenerateSpecialKey(std::vector<uint32_t> &key)
	{
		unsigned int i = 0, n = 1491971513;

		do {
			n = (n ^ 0x80164043) - 702581315;
			key.push_back(n);
			i++;
		} while (i < (uint8_t)n);
	}
}
