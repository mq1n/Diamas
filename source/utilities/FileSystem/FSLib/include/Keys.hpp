#pragma once
#include "Constants.hpp"
#include "FileNameHelper.hpp"
#include <tuple>
#include <vector>
#include <xorstr.hpp>
// TODO

namespace FileSystem
{
    enum EKeyContainerIndexes : uint8_t
    {
        KEY_CONTAINER_VERSION,
        KEY_CONTAINER_FILENAME,
        KEY_CONTAINER_KEY
    };

	using TKeyContainer = std::tuple <uint32_t /* version */, CFileName /* filename*/, TArchiveKey /* key_array */>;
	static const auto gs_vecFileAndKeys = std::vector <TKeyContainer>
	{
        /* 1 */ {
            1,
            xorstr("test_name").crypt_get(),
            {
                // Key: 0 - 32
                0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2,

                // IV: 32 - 64
                0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4
            }
        }
        // ...
	};
};


#if 0
#include <cstdint>
#include <array>
#include <map>
#include <string>

namespace BasicCrypt
{
	static void DecryptBuffer(uint8_t * lpBuf, std::size_t dwSize, uint8_t pKey)
	{
		for (std::size_t i = 0; i < dwSize; i++)
		{
			lpBuf[i] ^= pKey;
			lpBuf[i] += (uint8_t)i;
			lpBuf[i] ^= (uint8_t)i + 8;
		}
	}

	static void EncryptBuffer(uint8_t * lpBuf, std::size_t dwSize, uint8_t pKey)
	{
		for (std::size_t i = 0; i < dwSize; i++)
		{
			lpBuf[i] ^= (uint8_t)i + 8;
			lpBuf[i] -= (uint8_t)i;
			lpBuf[i] ^= pKey;
		}
	}
}

namespace FileSystem
{
	using TArchiveKey = std::array <uint8_t, 64>;
//	using TKeyContainer = std::map <std::string /* stFilename*/, TArchiveKey /* arrKey */>;
	static const auto mapFileAndKeys = std::map <std::string /* stFilename*/, TArchiveKey /* arrKey */>
	{
        /* 1 */{
            "test_name",
            {
                    // Key: 0 - 32
                    0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
                    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2,

                    // IV: 32 - 64
                    0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
                    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4
            }
        }
	};
};

int main()
{
    for (auto& x : FileSystem::mapFileAndKeys)
    {
        printf("%s\n", x.first.c_str());
        auto key = const_cast<uint8_t*>(x.second.data());
        for (int i = 0; i < 64; ++i)
        {
            printf("0x%X, ", key[i]);
        }
        printf("\n");

        BasicCrypt::EncryptBuffer(key, 64, 69);

        for (int i = 0; i < 64; ++i)
        {
            printf("0x%X, ", key[i]);
        }
    }
}
#endif
