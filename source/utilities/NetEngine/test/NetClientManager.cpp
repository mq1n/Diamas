#include "../include/NetEngine.hpp"
#include "NetClientManager.hpp"
#include "Packet.hpp"

namespace net_engine
{	
	CNetworkClientManager::CNetworkClientManager(NetServiceBase& service, uint8_t securityLevel, const TPacketCryptKey& cryptKey) :
		NetClientBase(service(), securityLevel, cryptKey, 5000), m_pNetService(service)
	{
		//RegisterPacket(
		//	HEADER_GC_CHAT, EPacketTypes::PacketTypeIncoming, false, std::bind(&CNetworkClientManager::OnRecvChatPacket, this, std::placeholders::_1, std::placeholders::_2)
		//);
	}

	// I/O Service wrappers
	void CNetworkClientManager::Run()
	{
		m_pNetService.Run();
	}
	void CNetworkClientManager::Shutdown()
	{
		m_pNetService.Stop();
	}
	bool CNetworkClientManager::IsShuttingDown() const
	{
		return m_pNetService.HasStopped();
	}
	NetServiceBase& CNetworkClientManager::GetServiceInstance() const
	{
		return m_pNetService;
	}

	void CNetworkClientManager::OnConnect()
	{
		NET_LOG(LL_SYS, "Connected to %s:%u", GetIP().c_str(), GetPort());

		/*
		SNetPacketCGLogin packet;
		strcpy_s(packet.login, "id_Test");
		strcpy_s(packet.password, "pwd_test");
		SendCrypted(packet, true);
		*/
	}
	void CNetworkClientManager::OnDisconnect(const asio::error_code& e)
	{
		NET_LOG(LL_ERR, "Disconnected from: %s Reason: %d", GetIP().c_str(), e.value());
	//	Reconnect(); // aktif edip 3ten fazla ise kapat
	}
	void CNetworkClientManager::OnRead(std::shared_ptr <Packet> packet)
	{
#if 0
		NET_LOG(LL_SYS, "OnRead triggered! %p-%u", data, length);
		if (!data || !length) 
		{
			NET_LOG(LL_SYS, "Null data read!");
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
#endif
	}
	std::size_t CNetworkClientManager::OnWritePre(const void* data, std::size_t length)
	{
		if (!data || !length)
		{
			NET_LOG(LL_CRI, "Null data writed!");
			return length;
		}

		/*
		if (data && IsAssignedFlag(LL_ONWRITE_PRE) && length >= packet_header_min_size)
		{
			SNetPacket packet(reinterpret_cast<const char*>(data));
			NET_LOG(LL_SYS, "Data: %p Size: %u Packet %u-%u-%u-%u", 
				data, length,
				packet.m_header.version, packet.m_header.flags, packet.m_header.opcode, packet.m_header.size
			);
		}
		*/
		
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
	void CNetworkClientManager::OnWritePost(bool bCompleted)
	{
		if (IsAssignedFlag(LL_ONWRITE_POST))
		{
			NET_LOG(LL_SYS, "Write completed! Result: %d", bCompleted ? 1 : 0);
		}
	}
	void CNetworkClientManager::OnError(std::uint32_t error_type, const asio::error_code& er)
	{
		NET_LOG(LL_ERR, "Network error handled! ID: %u System error: %d(%s)", error_type, er.value(), er.message().c_str());
	}

#if 0
	std::size_t CNetworkClientManager::ProcessInput(const void* data, std::size_t maxlength)
	{
		NET_LOG(LL_TRACE, "Data: %p Max length: %u", data, maxlength);

		auto pData = reinterpret_cast<const uint8_t *>(data);

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
				NET_LOG(LL_ERR, "Unknown Packet with id %d (%02x)",
					packetId, packetId
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


	void CNetworkClientManager::SendCrypted(const SNetPacket& packet, bool flush)
	{
		if (IsConnected() == false)
			return;

#if 0
		NET_LOG(LL_SYS, "Original data: %p-%u Packet: %u", pData, ulSize, ulPacketID);

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


		if (m_bRSACompleted || m_bPreCryptCompleted)
		{
			// Crypt packet content
			auto cryptedPacket = NAES256::Encrypt(reinterpret_cast<const uint8_t *>(pData), ulSize, m_bRSACompleted ? m_pRSAKey : m_pPreCryptKey);
			NET_LOG(LL_SYS, "Crpyted data: %p-%u", cryptedPacket.get_data(), cryptedPacket.get_size());

			auto cryptedPacketData = std::make_unique<TCryptedPacket>();
			if (IS_VALID_SMART_PTR(cryptedPacketData))
			{
				memcpy(cryptedPacketData->pContext, cryptedPacket.get_data(), cryptedPacket.get_size());

				cryptedPacketData->uiDecryptedPacketId = ulPacketID;
				cryptedPacketData->ulCryptedSize = cryptedPacket.get_size();
				cryptedPacketData->ulDecryptedSize = ulSize;
				cryptedPacketData->ulCryptedSum = XXH32(cryptedPacket.get_data(), cryptedPacket.get_size(), NET_PACKET_HASH_MAGIC);
				cryptedPacketData->ulDecryptedSum = XXH32(pData, ulSize - (NET_CHECKSUM_LENGTH /* hash */ + NET_CHECKSUM_LENGTH /* sum */), NET_PACKET_HASH_MAGIC);

				NET_LOG(LL_SYS, "Built crpyted packet! Data: %p Size: %u Container size: %u", cryptedPacketData.get(), cryptedPacket.get_size(), cryptedPacketData->size());

				Send(cryptedPacketData.get(), cryptedPacketData->size());
			}
		}
		else
#endif
		{
//			Send(packet.data(), packet.size());
		}
	}


	std::size_t CNetworkClientManager::OnRecvChatPacket(const void* data, std::size_t maxlength)
	{
		auto packet = reinterpret_cast<const SNetPacketGCChat*>(data);

		if (maxlength < packet->size())
			return 0;

//		if (maxlength != SNetPacketGCChat::size())
//		{
//			NET_LOG(LL_CRI, "Packet size mismatch it can be combined packet!");
//			return SNetPacketGCChat::size();
//		}

		NET_LOG(LL_SYS, "Recv chat packet! Msg: %s", packet->msg);

		return packet->size();
	}
#endif
}
