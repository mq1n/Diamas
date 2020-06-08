#include "../include/NetEngine.h"
#include "NetPeerManager.h"
#include "NetServerManager.h"
#include "Packet.h"

namespace net_engine
{
	CNetworkPeer::CNetworkPeer(std::shared_ptr <CNetworkServerManager> server) :
		NetPeerBase(server->GetServiceInstance()()), m_pServer(server)
	{
	}

	// Getter setter
	std::shared_ptr <CNetworkServerManager> CNetworkPeer::GetServer() const
	{
		return m_pServer.lock();
	}
	int32_t CNetworkPeer::GetId() const
	{
		return UniqueId();
	}

	void CNetworkPeer::SetData(const std::string& key, const std::string& value)
	{
		m_pkMapDataContainer.emplace(key, value);
	}
	std::string CNetworkPeer::GetData(const std::string& key) const
	{
		auto it = m_pkMapDataContainer.find(key);
		if (it != m_pkMapDataContainer.end())
			return it->second;
		
		NET_LOG(LL_ERR, "Key not found: %s", key.c_str());
		return "";
	}

	// net callbacks
	void CNetworkPeer::OnConnect()
	{
		NET_LOG(LL_SYS, "Client connected %s (%d)", GetIP().c_str(), GetId());

		SetData("phase", "conn_req");
		BeginRead();
	}
	void CNetworkPeer::OnDisconnect(const asio::error_code& er)
	{
		NET_LOG(LL_SYS, "Client disconnected %s (%d) Reason: %d", GetIP().c_str(), GetId(), er.value());

		auto server = GetServer();
		if (server)
			server->RemovePeer(GetId());
	}

	std::size_t CNetworkPeer::OnRead(const void* data, std::size_t length)
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

		return pServer->ProcessInput(std::static_pointer_cast<CNetworkPeer>(shared_from_this()), data, length);
	}
	std::size_t CNetworkPeer::OnWritePre(const void* data, std::size_t length)
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

	void CNetworkPeer::OnWritePost(bool bCompleted)
	{
		if (IsAssignedFlag(LL_ONWRITE_POST))
		{
			NET_LOG(LL_SYS, "Write completed! Result: %d", bCompleted ? 1 : 0);
		}
	}
	void CNetworkPeer::OnError(uint32_t error_type, const asio::error_code& er)
	{
		NET_LOG(LL_ERR, "Network error handled! ID: %u System error: %d(%s)", error_type, er.value(), er.message().c_str());
	}

	void CNetworkPeer::SendCrypted(const SNetPacket& packet, bool flush)
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

	void CNetworkPeer::Destroy()
	{
		GetServer()->RemovePeer(GetId());
	}

	void CNetworkPeer::SendChatPacket(const std::string& data)
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
}
