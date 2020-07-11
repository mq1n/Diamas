#include <cryptopp/dh.h>
#include <cryptopp/dh2.h>
#include <cryptopp/modes.h>
#include <cryptopp/nbtheory.h>
#include <cryptopp/osrng.h>
#include <utility>

// Algorithms
#include <cryptopp/blowfish.h>
#include <cryptopp/camellia.h>
#include <cryptopp/cast.h>
#include <cryptopp/des.h>
#include <cryptopp/idea.h>
#include <cryptopp/mars.h>
#include <cryptopp/rc5.h>
#include <cryptopp/rc6.h>
#include <cryptopp/seed.h>
#include <cryptopp/serpent.h>
#include <cryptopp/shacal2.h>
#include <cryptopp/tea.h>
#include <cryptopp/twofish.h>

#include "../include/NetEngine.hpp"

#ifdef _DEBUG
	#pragma comment( lib, "cryptopp-static_debug.lib" )
#else
	#pragma comment( lib, "cryptopp-static.lib" )
#endif

namespace net_engine
{
	KeyAgreementCryptation::KeyAgreementCryptation() :
		m_udh(m_dh),
		m_clientAgreedValueLength(0),
		m_serverAgreedValueLength(0),
		m_InvertCiphers(false) 
	{
	}
	KeyAgreementCryptation::~KeyAgreementCryptation()
	{
	}

	std::unique_ptr<CryptoPP::BlockCipher> KeyAgreementCryptation::DecideCipher(uint8_t selection)
	{
		enum : uint8_t {
			CIPHER_RC6 = 1,
			CIPHER_MARS,
			CIPHER_TWOFISH,
			CIPHER_SERPENT,
			CIPHER_CAST256,
			CIPHER_IDEA,
			CIPHER_DESEDE2,
			CIPHER_CAMELLIA,
			CIPHER_SEED,
			CIPHER_RC5,
			CIPHER_BLOWFISH,
			CIPHER_TEA,
			CIPHER_SHACAL2,
			CIPHER_MAX
		};

		selection = selection % CIPHER_MAX;

		switch (selection)
		{
		default:
			break;

		case CIPHER_RC6:
			return std::make_unique<CryptoPP::RC6::Encryption>();

		case CIPHER_MARS:
			return std::make_unique<CryptoPP::MARS::Encryption>();

		case CIPHER_SERPENT:
			return std::make_unique<CryptoPP::Serpent::Encryption>();

		case CIPHER_CAST256:
			return std::make_unique<CryptoPP::CAST256::Encryption>();

		case CIPHER_IDEA:
			return std::make_unique<CryptoPP::IDEA::Encryption>();

		case CIPHER_DESEDE2:
			return std::make_unique<CryptoPP::DES_EDE2::Encryption>();

		case CIPHER_CAMELLIA:
			return std::make_unique<CryptoPP::Camellia::Encryption>();

		case CIPHER_SEED:
			return std::make_unique<CryptoPP::SEED::Encryption>();

		case CIPHER_RC5:
			return std::make_unique<CryptoPP::RC5::Encryption>();

		case CIPHER_BLOWFISH:
			return std::make_unique<CryptoPP::Blowfish::Encryption>();

		case CIPHER_TEA:
			return std::make_unique<CryptoPP::TEA::Encryption>();

		case CIPHER_SHACAL2:
			return std::make_unique<CryptoPP::SHACAL2::Encryption>();
		}

		return std::make_unique<CryptoPP::Twofish::Encryption>();
	}

	bool KeyAgreementCryptation::Initialize()
	{
		// RFC 5114, 1024-bit MODP Group with 160-bit Prime Order Subgroup
		// http://tools.ietf.org/html/rfc5114#section-2.1
		CryptoPP::Integer p(
			"0xB10B8F96A080E01DDE92DE5EAE5D54EC52C99FBCFB06A3C6"
			"9A6A9DCA52D23B616073E28675A23D189838EF1E2EE652C0"
			"13ECB4AEA906112324975C3CD49B83BFACCBDD7D90C4BD70"
			"98488E9C219A73724EFFD6FAE5644738FAA31A4FF55BCCC0"
			"A151AF5F0DC8B4BD45BF37DF365C1A65E68CFDA76D4DA708"
			"DF1FB2BC2E4A4371"
		);

		CryptoPP::Integer g(
			"0xA4D1CBD5C3FD34126765A442EFB99905F8104DD258AC507F"
			"D6406CFF14266D31266FEA1E5C41564B777E690F5504F213"
			"160217B4B01B886A5E91547F9E2749F4D7FBD7D3B9A92EE1"
			"909D0D2263F80A76A6A24C087A091F531DBF0A0169B6A28A"
			"D662A4D18E73AFA32D779D5918D08BC8858F4DCEF97C2A24"
			"855E6EEB22B3B2E5"
		);

		CryptoPP::Integer q("0xF518AA8781A8DF278ABA4E7D64B7CB9D49462353");

		CryptoPP::AutoSeededRandomPool rnd;

		// 1. Initialize and verify p, g, q
		m_dh.AccessGroupParameters().Initialize(p, q, g);

		if (!m_dh.GetGroupParameters().ValidateGroup(rnd, 3))
			return false;

		CryptoPP::Integer v = ModularExponentiation(g, q, p);
		if (v != CryptoPP::Integer::One())
			return false;

		// 2. Elevate to Unified DH
		CryptoPP::DH2 m_udh(m_dh);

		m_shared.New(m_udh.AgreedValueLength());
		m_staticServerPrivKey.New(m_udh.StaticPrivateKeyLength());
		m_staticServerPubKey.New(m_udh.StaticPublicKeyLength());
		m_ephemeralServerPrivKey.New(m_udh.EphemeralPrivateKeyLength());
		m_ephemeralServerPubKey.New(m_udh.EphemeralPublicKeyLength());

		m_udh.GenerateEphemeralKeyPair(rnd, m_ephemeralServerPrivKey, m_ephemeralServerPubKey);
		m_udh.GenerateStaticKeyPair(rnd, m_staticServerPrivKey, m_staticServerPubKey);

		m_serverAgreedValueLength = m_udh.AgreedValueLength();

		m_initialized = true;
		return true;
	}

	bool KeyAgreementCryptation::Finalize()
	{
		if (!m_staticClientPubKey.size() || !m_ephemeralClientPubKey.size() || !m_initialized)
			return false;

		if (m_clientAgreedValueLength != m_serverAgreedValueLength) 
			return false;

		if (!m_udh.Agree(m_shared, m_staticServerPrivKey, m_ephemeralServerPrivKey, m_staticClientPubKey, m_ephemeralClientPubKey))
			return false;

		if (m_shared.size() < 2)
			return false;

		// 1: Encrypt, 2 : Decrypt [Server], 1: Decrypt 2: Encrypt [Client]
		uint8_t cipherSelection1 = m_shared.data()[0] % m_shared.size();
		uint8_t cipherSelection2 = m_shared.data()[1] % m_shared.size();

		m_cipher1 = DecideCipher(m_shared.data()[cipherSelection1]);
		m_cipher2 = DecideCipher(m_shared.data()[cipherSelection2]);

		size_t KeySize1 = m_cipher1->DefaultKeyLength(),
			KeySize2 = m_cipher2->DefaultKeyLength(),
			IVSize1 = m_cipher1->BlockSize(),
			IVSize2 = m_cipher2->BlockSize();

		if (m_shared.size() < KeySize1 + KeySize2 + IVSize2 + IVSize1)
			return false;

		CryptoPP::SecByteBlock Key1(KeySize1), Key2(KeySize2), IV1(IVSize1), IV2(IVSize2);

		Key1.Assign(m_shared.data(), KeySize1);
		Key2.Assign(m_shared.data() + KeySize1, KeySize2);

		size_t IVPos = m_shared.size() - IVSize1;
		IV1.Assign(m_shared.data() + IVPos, IVSize1);

		if (IVPos < IVSize2)
			IVPos = 0;
		else 
			IVPos -= IVSize2;

		IV2.Assign(m_shared.data() + IVPos, IVSize2);

		m_cipher1->SetKey(Key1, KeySize1);
		m_cipher2->SetKey(Key2, KeySize2);

		if (m_InvertCiphers)
		{
			m_decrypt.SetCipherWithIV(*m_cipher1, IV1);
			m_encrypt.SetCipherWithIV(*m_cipher2, IV2);
		}
		else
		{
			m_decrypt.SetCipherWithIV(*m_cipher2, IV2);
			m_encrypt.SetCipherWithIV(*m_cipher1, IV1);
		}

		return true;
	}

	void KeyAgreementCryptation::Encrypt(uint8_t* dest, const uint8_t* src, size_t len) 
	{
		if (!m_activated)
			return;

		m_encrypt.ProcessData(dest, src, len);
	}

	void KeyAgreementCryptation::Decrypt(uint8_t* dest, const uint8_t* src, size_t len)
	{
		if (!m_activated)
			return;

		m_decrypt.ProcessData(dest, src, len);
	}

	void KeyAgreementCryptation::AddData(size_t id, void* data, size_t length)
	{
		if (id == KEY_AGREEMENT_DATA_AGREED_VALUE)
		{
			if (length != 2)
			{
				NET_LOG(LL_ERR, "Key agreement length is not 2!");
				return;
			}

			m_clientAgreedValueLength = *reinterpret_cast<uint16_t*>(data);
		}
		else if (id == KEY_AGREEMENT_DATA_PUBLIC_STATIC_CLIENT_KEY)
		{
			m_staticClientPubKey.New(length);
			m_staticClientPubKey.Assign(reinterpret_cast<uint8_t*>(data), length);
		}
		else if (id == KEY_AGREEMENT_DATA_PUBLIC_EPHEMERAL_CLIENT_KEY)
		{
			m_ephemeralClientPubKey.New(length);
			m_ephemeralClientPubKey.Assign(reinterpret_cast<uint8_t*>(data),  length);
		}
		else if (id == KEY_AGREEMENT_DATA_INVERT_CIPHER)
		{
			if (length != 1)
			{
				NET_LOG(LL_ERR, "Invert shared cipher must be a bool type!");
				return;
			}

			m_InvertCiphers = *reinterpret_cast<bool*>(data);
		}
	}

	const void* KeyAgreementCryptation::GetData(size_t id)
	{
		if (!m_initialized)
			return nullptr;

		if (id == KEY_AGREEMENT_DATA_AGREED_VALUE)
			return &m_serverAgreedValueLength;
		
		else if (id == KEY_AGREEMENT_DATA_PUBLIC_STATIC_SERVER_KEY)
			return &m_staticServerPubKey;
		
		else if (id == KEY_AGREEMENT_DATA_PUBLIC_EPHEMERAL_SERVER_KEY)
			return &m_ephemeralServerPubKey;
		
		return nullptr;
	}
}
