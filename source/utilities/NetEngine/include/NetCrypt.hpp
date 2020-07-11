#pragma once

#include <cryptopp/dh.h>
#include <cryptopp/dh2.h>
#include <cryptopp/modes.h>

namespace net_engine
{
	class CryptationBase
	{
	public:
		CryptationBase() :
			m_initialized(false), m_activated(false)
		{
		}
		virtual ~CryptationBase()
		{
		}

		bool IsReady() { return m_activated; }

		bool IsInitialized() { return m_initialized; }

		virtual bool Finalize() { return false; }
		virtual bool Initialize() { return false; }

		void Activate() { m_activated = true; }

		void Reset() 
		{
			m_initialized = false;
			m_activated = false;
		}

		virtual void Encrypt(uint8_t* dest, const uint8_t* src, size_t len) {}
		virtual void Decrypt(uint8_t* dest, const uint8_t* src, size_t len) {}

		void Encrypt(std::vector <uint8_t>& dest, std::vector <uint8_t> src)
		{
			Encrypt(dest.data(), src.data(), src.size());
		}

		void Decrypt(std::vector <uint8_t>& dest, std::vector <uint8_t> src)
		{
			Decrypt(dest.data(), src.data(), src.size());
		}

		virtual void AddData(size_t id, void* data, size_t length) {}

		virtual const void* GetData(size_t id) { return nullptr; }

	protected:
		bool m_initialized, m_activated;
	};

	enum KeyAgreementDataID : size_t {
		KEY_AGREEMENT_DATA_AGREED_VALUE,
		KEY_AGREEMENT_DATA_PUBLIC_STATIC_SERVER_KEY,
		KEY_AGREEMENT_DATA_PUBLIC_EPHEMERAL_SERVER_KEY,
		KEY_AGREEMENT_DATA_PUBLIC_STATIC_CLIENT_KEY,
		KEY_AGREEMENT_DATA_PUBLIC_EPHEMERAL_CLIENT_KEY,
		KEY_AGREEMENT_DATA_INVERT_CIPHER,
	};

	class KeyAgreementCryptation : public CryptationBase
	{
	public:
		KeyAgreementCryptation();
		virtual ~KeyAgreementCryptation();

		bool Finalize();
		bool Initialize();

		void Encrypt(uint8_t* dest, const uint8_t* src, size_t len);
		void Decrypt(uint8_t* dest, const uint8_t* src, size_t len);

		void AddData(size_t id, void* data, size_t length);
		const void* GetData(size_t id);

	protected:
		std::unique_ptr <CryptoPP::BlockCipher> DecideCipher(uint8_t selection);

	private:
		CryptoPP::DH2 m_udh;
		CryptoPP::DH m_dh;
		CryptoPP::SecByteBlock m_shared, m_staticServerPrivKey, m_staticServerPubKey, m_ephemeralServerPrivKey,
							   m_ephemeralServerPubKey, m_staticClientPubKey, m_ephemeralClientPubKey;

		uint32_t m_clientAgreedValueLength, m_serverAgreedValueLength;

		CryptoPP::CTR_Mode_ExternalCipher::Encryption m_encrypt;
		CryptoPP::CTR_Mode_ExternalCipher::Decryption m_decrypt;

		std::unique_ptr <CryptoPP::BlockCipher> m_cipher1, m_cipher2;

		bool m_InvertCiphers;
	};

	enum XTEACryptationDataID : size_t
	{
		XTEA_CRYPTATION_START_KEY,
		XTEA_CRYPTATION_LOGIN_DECRYPT_KEY,
	};

	class XTEACryptation : public CryptationBase
	{
	public:
		XTEACryptation();
		virtual ~XTEACryptation();

		bool Finalize() { return true; }
		bool Initialize() { return true; }

		void Encrypt(uint8_t* dest, const uint8_t* src, size_t len);
		void Decrypt(uint8_t* dest, const uint8_t* src, size_t len);

		void AddData(size_t id, void* data, size_t length);

	protected:
		void __GenerateSpecialKey(std::vector<uint32_t>& key);
		void __ComputeCryptationKey();

	private:
		uint32_t m_cryptKey[4];
		uint32_t m_decryptKey[4];
	};
};
