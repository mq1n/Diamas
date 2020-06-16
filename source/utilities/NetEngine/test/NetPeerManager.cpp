#include "../include/NetEngine.hpp"
#include "NetPeerManager.hpp"
#include "NetServerManager.hpp"
#include "Packet.hpp"

namespace net_engine
{
	CNetworkConnectionManager::CNetworkConnectionManager(
		NetServiceBase& netService, std::shared_ptr <CNetworkServerManager> server, uint8_t securityLevel, const TPacketCryptKey& cryptKey) :
		NetPeerBase(server->GetServiceInstance()()), m_server(server), m_netService(netService), m_crypt_key(cryptKey)
	{
		NET_LOG(LL_TRACE, "Creating connection object");
		m_packets = std::make_shared<CPacketContainer>();

#if ENABLE_CRYPT
		_handshake(_server->GetApplication()->GetRandomNumber<uint32_t>(std::numeric_limits<uint32_t>::min(),
			std::numeric_limits<uint32_t>::max())

		if (securityLevel == SECURITY_LEVEL_KEY_AGREEMENT) {
			_cryptation = std::make_unique<KeyAgreementCryptation>();
	}
		else if (securityLevel == SECURITY_LEVEL_XTEA) {
			_cryptation = std::make_unique<XTEACryptation>();
			_cryptation->AddData(XTEA_CRYPTATION_START_KEY, defaultKey, 16);
		}
#endif
	}
	CNetworkConnectionManager::~CNetworkConnectionManager()
	{
		NET_LOG(LL_TRACE, "Destroying connection object");
	}

	void CNetworkConnectionManager::Destroy()
	{
		GetServer()->RemovePeer(GetId());
	}

	// net callbacks
	void CNetworkConnectionManager::OnConnect()
	{
		NET_LOG(LL_SYS, "Client connected %s (%d)", GetIP().c_str(), GetId());

		BeginRead();

#if ENABLE_CRYPT
		if (_securityLevel >= SECURITY_LEVEL_NONE) {
			StartHandshake();
		}
		else {
			_server->CallNewConnectionHandler(shared_from_this());
		}
#endif
	}
	void CNetworkConnectionManager::OnDisconnect(const asio::error_code& er)
	{
		NET_LOG(LL_SYS, "Client disconnected %s (%d) Reason: %d", GetIP().c_str(), GetId(), er.value());

		auto server = GetServer();
		if (server)
			server->RemovePeer(GetId());
	}
	void CNetworkConnectionManager::OnRead(std::shared_ptr <Packet> packet)
	{
	}
	std::size_t CNetworkConnectionManager::OnWritePre(const void* data, std::size_t length)
	{
		return length;
	}
	void CNetworkConnectionManager::OnWritePost(bool bCompleted)
	{
	}
	void CNetworkConnectionManager::OnError(uint32_t error_type, const asio::error_code& er)
	{
		NET_LOG(LL_ERR, "Network error handled! ID: %u System error: %d(%s)", error_type, er.value(), er.message().c_str());
	}


	// Getter setter
	std::shared_ptr <CNetworkServerManager> CNetworkConnectionManager::GetServer() const
	{
		return m_server.lock();
	}
	int32_t CNetworkConnectionManager::GetId() const
	{
		return UniqueId();
	}
	std::string CNetworkConnectionManager::GetData(const std::string& key) const
	{
		const auto it = m_data_container.find(key);
		if (it != m_data_container.end())
			return it->second;
		
		NET_LOG(LL_ERR, "Key not found: %s", key.c_str());
		return "";
	}

	void CNetworkConnectionManager::SetData(const std::string& key, const std::string& value)
	{
		m_data_container.emplace(key, value);
	}


#if 0
	void Connection::SetPhase(uint8_t phase) {
		CORE_LOGGING(trace) << "Set phase to " << boost::format("0x%02x") % (int)phase;

		auto packet =
			_server->GetPacketManager()->CreatePacket(ReservedOutgoingHeaders::HEADER_OUT_PHASE, Direction::Outgoing);
		packet->SetField<uint8_t>("phase", phase);

		_phase = phase;

		Send(packet);
	}

	void Connection::SendAsReply(std::shared_ptr<Packet> request, std::shared_ptr<Packet> reply) {
		auto id = request->GetField<uint64_t>("__REFERENCE_ID");

		reply->SetField<uint64_t>("__REFERENCE_ID", id);
		reply->SetField<uint8_t>("__REFERENCE_TYPE", 1);

		Send(reply);
	}
	std::shared_ptr<Server> Connection::GetServer() {


		bool Connection::HasProperty(const std::string & property) const {
			return _properties.find(property) != _properties.end();
		}

		void Connection::SetProperty(const std::string & property, ConnectionProperty value) {
			_properties[property] = std::move(value);
		}

		ConnectionProperty Connection::GetProperty(const std::string & property) const {
			return _properties.at(property);
		}



		void Connection::HandleHandshake(std::shared_ptr<Packet> packet) {
			auto handshake = packet->GetField<uint32_t>("handshake");
			auto time = packet->GetField<uint32_t>("time");
			auto delta = packet->GetField<uint32_t>("delta");

			if (handshake != _handshake) {
				CORE_LOGGING(warning) << "Closing connection because of handshake mismatch";
				Close();
				return;
			}

			auto currentTime = _server->GetApplication()->GetCoreTime();

			auto diff = currentTime - (time + delta);
			if (diff >= 0 && diff <= 50) {
				CORE_LOGGING(trace) << "Time sync done, handshake done";

				if (_securityLevel >= SECURITY_LEVEL_KEY_AGREEMENT) {
					StartKeyAgreement();
				}
				else {
					_handshaking = false;
					_server->CallNewConnectionHandler(shared_from_this());

					if (_securityLevel == SECURITY_LEVEL_XTEA) {
						CORE_LOGGING(trace) << "(XTEA) Cryptation enabled";
						if (!_cryptation->Finalize()) {
							Close();
						}

						_cryptation->Activate();
					}
				}
				return;
			}

			auto newDelta = (currentTime - time) / 2;
			if (newDelta < 0) {
				CORE_LOGGING(error) << "Failed to sync time";
				Close();
				return;
			}

			// todo max retries?
			SendHandshake(currentTime, newDelta);
		}

		return _server;
	}

	void Connection::StartHandshake() {
		_handshaking = true;

		SetPhase(Phases::PHASE_HANDSHAKE);
		SendHandshake(_server->GetApplication()->GetCoreTime(), 0);
	}

	void Connection::SendHandshake(uint32_t time, uint32_t delta) {
		auto packet = _server->GetPacketManager()->CreatePacket(ReservedOutgoingHeaders::HEADER_OUT_HANDSHAKE,
			Direction::Outgoing);
		packet->SetField<uint32_t>("handshake", _handshake);
		packet->SetField<uint32_t>("time", time);
		packet->SetField<uint32_t>("delta", 0);

		Send(packet);
	}


	void Connection::StartKeyAgreement() {
		CORE_LOGGING(trace) << "(KEYS) Agreement start!";
		const CryptoPP::SecByteBlock* staticKey;
		const CryptoPP::SecByteBlock* ephemeralKey;
		const uint32_t* agreedValue;

		if (!_cryptation->Initialize()) {
			CORE_LOGGING(trace) << "(KEYS) Cannot initialize!";
			Close();
			return;
		}

		staticKey = (const CryptoPP::SecByteBlock*)(_cryptation->GetData(KEY_AGREEMENT_DATA_PUBLIC_STATIC_SERVER_KEY));
		ephemeralKey =
			(const CryptoPP::SecByteBlock*)(_cryptation->GetData(KEY_AGREEMENT_DATA_PUBLIC_EPHEMERAL_SERVER_KEY));

		agreedValue = (const uint32_t*)_cryptation->GetData(KEY_AGREEMENT_DATA_AGREED_VALUE);
		if (*agreedValue < 1 || staticKey->size() < 1 || ephemeralKey->size() < 1 ||
			(ephemeralKey->size() + staticKey->size() > 256)) {
			CORE_LOGGING(trace) << "(KEYS) Invalid values!";
			Close();
			return;
		}

		char keys[256];
		memset(keys, 0, sizeof(keys));
		memcpy(keys, staticKey->data(), staticKey->size());
		memcpy(keys + staticKey->size(), ephemeralKey->data(), ephemeralKey->size());

		auto packet = _server->GetPacketManager()->CreatePacket(ReservedOutgoingHeaders::HEADER_OUT_KEY_AGREEMENT,
			Direction::Outgoing);
		packet->SetField<uint16_t>("valuelen", static_cast<uint16_t>(*agreedValue));
		packet->SetField<uint16_t>("datalen", static_cast<uint16_t>(ephemeralKey->size() + staticKey->size()));
		packet->SetField("data", (uint8_t*)keys, sizeof(keys));

		Send(packet);
	}

	void Connection::HandleKeyAgreement(std::shared_ptr<Packet> packet) {
		PROFILE_FUNCTION();
		if (!_cryptation->IsInitialized()) {
			CORE_LOGGING(trace) << "(KEYS) Connection sent handle keys without "
				"letting the server initialize them";
			Close();
			return;
		}

		uint8_t* data = (uint8_t*)packet->GetField("data");

		{
			PROFILE_SCOPE("Generate keys");
			const CryptoPP::SecByteBlock* staticKey =
				(const CryptoPP::SecByteBlock*)_cryptation->GetData(KEY_AGREEMENT_DATA_PUBLIC_STATIC_SERVER_KEY);
			const CryptoPP::SecByteBlock* ephemeralKey =
				(const CryptoPP::SecByteBlock*)_cryptation->GetData(KEY_AGREEMENT_DATA_PUBLIC_EPHEMERAL_SERVER_KEY);

			uint16_t agreedValue = packet->GetField<uint16_t>("valuelen");
			_cryptation->AddData(KEY_AGREEMENT_DATA_AGREED_VALUE, &agreedValue, sizeof(agreedValue));

			_cryptation->AddData(KEY_AGREEMENT_DATA_PUBLIC_STATIC_CLIENT_KEY, data, staticKey->size());
			_cryptation->AddData(
				KEY_AGREEMENT_DATA_PUBLIC_EPHEMERAL_CLIENT_KEY, data + staticKey->size(), ephemeralKey->size());
		}

		if (!_cryptation->Finalize()) {
			CORE_LOGGING(trace) << "(KEYS) Cannot agree";
			Close();
			return;
		}

		Send(_server->GetPacketManager()->CreatePacket(ReservedOutgoingHeaders::HEADER_OUT_KEY_AGREEMENT_COMPLETED,
			Direction::Outgoing));

		CORE_LOGGING(trace) << "(KEYS) Agreement completed!";
		_handshaking = false;
		_server->CallNewConnectionHandler(shared_from_this());
	}

	void Connection::ChangeXTEAKey(uint32_t* key) { _cryptation->AddData(XTEA_CRYPTATION_LOGIN_DECRYPT_KEY, key, 16); }

#endif

#if 0

	// Packet handler
	std::size_t CNetworkConnectionManager::ProcessInput(const void* data, std::size_t maxlength)
	{
		NET_LOG(LL_SYS, "Peer: %s(%d) Data: %p Max length: %u", GetIP().c_str(), GetId(), data, maxlength);

		auto pData = reinterpret_cast<const uint8_t*>(data);

		std::size_t offset = 0;
		while (offset < maxlength)
		{
			uint32_t packetId = 0;
			while (offset < maxlength && (packetId = pData[offset]) == 0) ++offset;

			/*
			auto handler = m_handlers.find(packetId);
			if (handler == m_handlers.end())
			*/
			const auto packet = CPacketContainer::Instance().GetPacket(packetId);
			if (!packet.get())
			{
				// log + kick? unkwnown packet dc
				NET_LOG(LL_ERR, "Unknown Packet with id %d (%02x) received from PEER %d",
					packetId, packetId, GetId()
				);

				asio::error_code e;
				Disconnect(e);
				return 0;
			}

			std::size_t handlerResult = packet->handler(pData + offset, maxlength - offset);
			if (handlerResult == 0)
				break; // handler returned 0 == too little data
			offset += handlerResult;
		}
		return offset;
	}

	std::size_t CNetworkConnectionManager::OnRead(const void* data, std::size_t length)
	{
		auto pServer = GetServer();
		if (!pServer || !pServer.get())
		{
			NET_LOG(LL_CRI, "Null server ptr!");

			asio::error_code e;
			Disconnect(e);
			return 0;
		}

		if (!data || !length) 
		{
			NET_LOG(LL_CRI, "Null data read!");
			return 0;
		}

		if (IsAssignedFlag(LL_ONREAD) && length >= packet_header_min_size)
		{
			SNetPacket packet(reinterpret_cast<const char*>(data));
			NET_LOG(LL_SYS, "Data: %p Size: %u Packet %u-%u-%u-%u", 
				data, length,
				packet.m_header.version, packet.m_header.flags, packet.m_header.opcode, packet.m_header.size
			);
		}

		return ProcessInput(data, length);
	}
	std::size_t CNetworkConnectionManager::OnWritePre(const void* data, std::size_t length)
	{
		if (!data || !length)
		{
			NET_LOG(LL_CRI, "Null data writed!");
			return length;
		}

		if (data && IsAssignedFlag(LL_ONWRITE_PRE) && length >= packet_header_min_size)
		{
			SNetPacket packet(reinterpret_cast<const char*>(data));
			NET_LOG(LL_SYS, "Data: %p Size: %u Packet %u-%u-%u-%u", 
				data, length,
				packet.m_header.version, packet.m_header.flags, packet.m_header.opcode, packet.m_header.size
			);
		}
		
#if 0
		if (data && ulSize == GetPacketCapacity(pPsuedoPacket->uiPacketID))
		{
			auto pPacketSum = g_nmApp->NetworkMgrInstance()->CreateChecksum(pData, ulSize - (NET_CHECKSUM_LENGTH /* hash */ + NET_CHECKSUM_LENGTH /* sum */));
			(*(uint32_t*)((uint32_t)pData + ulSize - (NET_CHECKSUM_LENGTH /* hash */ + NET_CHECKSUM_LENGTH /* sum */))) = pPacketSum;
			NET_LOG(LL_SYS, "Sum: %p", pPacketSum);

			auto pPacketHash = XXH32(pData, ulSize - NET_CHECKSUM_LENGTH, NET_PACKET_HASH_MAGIC);
			(*(uint32_t*)((uint32_t)pData + ulSize - NET_CHECKSUM_LENGTH)) = pPacketHash;
			NET_LOG(LL_SYS, "Hash: %p", pPacketHash);
		}
#endif
		return length;
	}

	void CNetworkConnectionManager::OnWritePost(bool bCompleted)
	{
		if (IsAssignedFlag(LL_ONWRITE_POST))
		{
			NET_LOG(LL_SYS, "Write completed! Result: %d", bCompleted ? 1 : 0);
		}
	}
	void CNetworkConnectionManager::OnError(uint32_t error_type, const asio::error_code& er)
	{
		NET_LOG(LL_ERR, "Network error handled! ID: %u System error: %d(%s)", error_type, er.value(), er.message().c_str());
	}



	void CNetworkConnectionManager::SendCrypted(const SNetPacket& packet, bool flush)
	{
#if 0
		NET_LOG(LL_SYS, "Original data: %p-%u", pData, ulSize);

		// Process packet security contents
		auto pPacketMagic = (*(uint32_t*)gs_pNetworkMagicValue);
		(*(uint32_t*)((uint32_t)pData)) = pPacketMagic;

		NET_LOG(LL_SYS, "Magic: %p", pPacketMagic);

		auto pPacketSum = g_nmApp->NetworkMgrInstance()->CreateChecksum(pData, ulSize - (NET_CHECKSUM_LENGTH /* hash */ + NET_CHECKSUM_LENGTH /* sum */));
		(*(uint32_t*)((uint32_t)pData + ulSize - (NET_CHECKSUM_LENGTH /* hash */ + NET_CHECKSUM_LENGTH /* sum */))) = pPacketSum;

		NET_LOG(LL_SYS, "Sum: %p", pPacketSum);

		auto pPacketHash = XXH32(pData, ulSize - NET_CHECKSUM_LENGTH, NET_PACKET_HASH_MAGIC);
		(*(uint32_t*)((uint32_t)pData + ulSize - NET_CHECKSUM_LENGTH)) = pPacketHash;

		NET_LOG(LL_SYS, "Hash: %p", pPacketHash);


		if (m_bRSACryptKeyCompleted || m_bPreCryptCompleted)
		{
			// Crypt packet content
			auto cryptedPacket = NAES256::Encrypt(reinterpret_cast<const uint8_t *>(pData), ulSize, m_bRSACryptKeyCompleted ? m_pRSACryptKey : m_pPreCryptKey);
			NET_LOG(LL_SYS, "Crpyted data: %p-%u", cryptedPacket.get_data(), cryptedPacket.get_size());

			auto cryptedPacketData = std::make_unique<TCryptedPacket>();
			if (IS_VALID_SMART_PTR(cryptedPacketData))
			{
				memcpy(cryptedPacketData->pContext, cryptedPacket.get_data(), cryptedPacket.get_size());

				cryptedPacketData->uiDecryptedPacketId	= ulPacketID;
				cryptedPacketData->ulCryptedSize		= cryptedPacket.get_size();
				cryptedPacketData->ulDecryptedSize		= ulSize;
				cryptedPacketData->ulCryptedSum			= XXH32(cryptedPacket.get_data(), cryptedPacket.get_size(), NET_PACKET_HASH_MAGIC);
				cryptedPacketData->ulDecryptedSum		= XXH32(pData, ulSize - (NET_CHECKSUM_LENGTH /* hash */ + NET_CHECKSUM_LENGTH /* sum */), NET_PACKET_HASH_MAGIC);

				NET_LOG(LL_SYS, "Built crpyted packet! Data: %p Size: %u Container size: %u", cryptedPacketData.get(), cryptedPacket.get_size(), cryptedPacketData->size());

				Send(cryptedPacketData.get(), cryptedPacketData->size());
			}
		}
		else
#endif
		{
			Send(packet.data(), packet.size(), flush);
		}
	}

	void CNetworkConnectionManager::SendChatPacket(const std::string& data)
	{
		SNetPacketGCChat retPacket;
		strcpy_s(retPacket.msg, data.c_str());
		SendCrypted(retPacket, true);

#if 0
		auto pDynamicKey = g_nmApp->FunctionsInstance()->GetRandomData(NET_CRYPT_KEY_LENGTH);
	//	EncryptBuffer(pDynamicKey, NET_CRYPT_KEY_LENGTH, NETWORK_BODY_CRYPT_KEY);

		auto preCryptKeyPacket = std::make_shared<TPacketPreCryptKeyInfo>();
		if (IS_VALID_SMART_PTR(preCryptKeyPacket))
		{
			memcpy(preCryptKeyPacket->pMagic, gs_pNetworkMagicValue, sizeof(preCryptKeyPacket->pMagic));
			preCryptKeyPacket->uiPacketID = HEADER_SC_PRE_KEY_INFO;

			// First arg | Dummy
			auto ulRandomDataSize = 36; // g_nmApp->FunctionsInstance()->GetRandomInt(24, 36);
			auto pFirstDummyData = g_nmApp->FunctionsInstance()->GetRandomData(ulRandomDataSize);
			memcpy(preCryptKeyPacket->dummyDataFirst, pFirstDummyData, ulRandomDataSize);

			// Second Arg | Pre crypt key
			memcpy(preCryptKeyPacket->preCryptKey, pDynamicKey, NET_CRYPT_KEY_LENGTH);

			// Third arg | Dummy
			auto ulRandomDataSize2 = g_nmApp->FunctionsInstance()->GetRandomInt(36, 48);
			auto pLastDummyData = g_nmApp->FunctionsInstance()->GetRandomData(ulRandomDataSize2);
			memcpy(preCryptKeyPacket->dummyDataLast, pLastDummyData, ulRandomDataSize2);

			SendCrypted(HEADER_SC_PRE_KEY_INFO, preCryptKeyPacket.get(), preCryptKeyPacket->size());

			// Register key
			this->SetPreCryptKey(pDynamicKey);

			NET_LOG(LL_SYS, "Sent pre crypt key!");
		}
#endif
	}

#endif
}
