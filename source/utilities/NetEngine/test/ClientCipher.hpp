#ifndef __CIPHER_H__
#define __CIPHER_H__

#include <cassert>
#include <cstdint>
#include <cryptopp/cryptlib.h>

#pragma warning(push)
#pragma warning(disable: 4100 4127 4189 4231 4512 4706)
#pragma warning(pop)

// Forward declaration
class KeyAgreement;

// Communication channel encryption handler.
class Cipher {
 public:
  explicit Cipher();
  ~Cipher();

  void CleanUp();

  // Returns agreed value length in bytes, or zero on failure.
  size_t Prepare(void* buffer, size_t* length);
  // Try to activate cipher algorithm with agreement data received from peer.
  bool Activate(bool polarity, size_t agreed_length,
                const void* buffer, size_t length);

  // Encrypts the given block of data. (no padding required)
  void Encrypt(void* buffer, size_t length) {
    assert(activated_);
    if (!activated_) {
      return;
    }
    encoder_->ProcessData((uint8_t*)buffer, (const uint8_t*)buffer, length);
  }
  // Decrypts the given block of data. (no padding required)
  void Decrypt(void* buffer, size_t length) {
    assert(activated_);
    if (!activated_) {
      return;
    }
    decoder_->ProcessData((uint8_t*)buffer, (const uint8_t*)buffer, length);
  }

  bool activated() const { return activated_; }
  
  void set_activated(bool value) { activated_ = value; }

 private:
  bool SetUp(bool polarity);

  bool activated_;

  CryptoPP::SymmetricCipher* encoder_;
  CryptoPP::SymmetricCipher* decoder_;

  KeyAgreement* key_agreement_;
};

#endif // _IMPROVED_PACKET_ENCRYPTION_
