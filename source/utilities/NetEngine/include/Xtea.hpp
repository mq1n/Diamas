#pragma once
#include <cstddef>
#include <cstdint>

namespace net_engine::utils
{
    class XTEA
    {
    public:
        static void Encrypt(const uint8_t* input, uint8_t* output, std::size_t size, const uint32_t* key, uint32_t numRounds);
        static uint32_t Decrypt(const uint8_t* input, uint8_t* output, std::size_t size, const uint32_t* key, uint32_t numRounds);

    private:
        static void EncryptStep(uint32_t numRounds, uint32_t* v, const uint32_t* key);
        static void DecryptStep(uint32_t numRounds, uint32_t* v, const uint32_t* key);
    };
};
