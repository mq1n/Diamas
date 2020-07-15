#include "../include/NetEngine.hpp"
#include "NetClientManager.hpp"
#include "ClientCipher.hpp"
#include "../../../common/packets.h"

namespace net_engine
{	
	CNetworkClientManager::CNetworkClientManager(NetServiceBase& service, uint8_t securityLevel, const TPacketCryptKey& cryptKey) :
		NetClientBase(service(), securityLevel, cryptKey, STAGE_DEV_GAME, EPacketType::PACKET_TYPE_SC, 5000), m_pNetService(service)
	{
		NetPacketManager::Instance().RegisterPackets();
		REGISTER_PACKET_HANDLER(HEADER_GC_KEY_AGREEMENT_COMPLETED, std::bind(&CNetworkClientManager::OnRecvKeyAgreementCompletedPacket, this, std::placeholders::_1));
		REGISTER_PACKET_HANDLER(HEADER_GC_PHASE, std::bind(&CNetworkClientManager::OnRecvPhasePacket, this, std::placeholders::_1));
		REGISTER_PACKET_HANDLER(HEADER_GC_KEY_AGREEMENT, std::bind(&CNetworkClientManager::OnRecvKeyAgreementPacket, this, std::placeholders::_1));
		REGISTER_PACKET_HANDLER(HEADER_GC_HANDSHAKE, std::bind(&CNetworkClientManager::OnRecvHandshakePacket, this, std::placeholders::_1));

		m_kServerTimeSync = {0, 0};
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
	}
	void CNetworkClientManager::OnDisconnect(const asio::error_code& e)
	{
		NET_LOG(LL_ERR, "Disconnected from: %s Reason: %d", GetIP().c_str(), e.value());
	//	Reconnect(); // aktif edip 3ten fazla ise kapat
	}
	void CNetworkClientManager::OnRead(std::shared_ptr <Packet> packet)
	{
		const auto header = packet->GetHeader();
		NET_LOG(LL_SYS, "Packet: %u(0x%x) is ready for process!", header, header);

		const auto handler = m_dispatcher.GetPacketHandler(header);
		if (!handler)
		{
			NET_LOG(LL_ERR, "Unknown header: %u(0x%x) for process", header, header);
			return;
		}
		handler(packet);
	}
	void CNetworkClientManager::OnError(std::uint32_t error_type, const asio::error_code& er)
	{
		NET_LOG(LL_ERR, "Network error handled! ID: %u System error: %d(%s)", error_type, er.value(), er.message().c_str());
	}


	void CNetworkClientManager::OnRecvPhasePacket(std::shared_ptr <Packet> packet)
	{

	}

	void CNetworkClientManager::OnRecvKeyAgreementCompletedPacket(std::shared_ptr <Packet> packet)
	{
		NET_LOG(LL_SYS, "KEY_AGREEMENT_COMPLETED RECV");

		m_cipher.set_activated(true);
	}

	void CNetworkClientManager::OnRecvKeyAgreementPacket(std::shared_ptr <Packet> packet)
	{
		auto valuelen = packet->GetField<uint16_t>("valuelen");
		auto datalen = packet->GetField<uint16_t>("datalen");
		auto data = packet->GetField("data");

		NET_LOG(LL_SYS, "KEY_AGREEMENT RECV %u", datalen);

		auto reply_packet = NetPacketManager::Instance().CreatePacket(BuildPacketID(HEADER_CG_KEY_AGREEMENT, PACKET_TYPE_CS));
		if (!reply_packet)
		{
			NET_LOG(LL_CRI, "Handshake packet could not created!");
			abort();
			return;
		}			
		auto reply_data = reply_packet->GetField("data");

		size_t dataLength = 256;
		size_t agreedLength = m_cipher.Prepare(reply_data, &dataLength);
		if (agreedLength == 0)
		{
			Disconnect2();
			return;
		}

		assert(dataLength <= 256);

		if (m_cipher.Activate(true, valuelen, data, dataLength))
		{
			reply_packet->SetField<uint16_t>("valuelen", static_cast<uint16_t>(agreedLength));
			reply_packet->SetField<uint16_t>("datalen", static_cast<uint16_t>(dataLength));
	
			Send(reply_packet);
			NET_LOG(LL_SYS, "KEY_AGREEMENT SEND %u", dataLength);
		}
		else
		{
			Disconnect2();
			return;
		}
	}

	void CNetworkClientManager::OnRecvHandshakePacket(std::shared_ptr <Packet> packet)
	{
		// auto timestamp = ELTimer_GetMSec();
		auto st = std::time(nullptr);
		auto timestamp = static_cast<uint32_t>(static_cast<std::chrono::seconds>(st).count());


		auto handshake = packet->GetField<uint32_t>("handshake");
		auto time = packet->GetField<uint32_t>("time");
		auto delta = packet->GetField<uint32_t>("delta");

		NET_LOG(LL_SYS, "HANDSHAKE RECV %u %d", time, delta);

		m_kServerTimeSync.m_dwChangeServerTime = time + delta;
		m_kServerTimeSync.m_dwChangeClientTime = timestamp;	

		auto reply_packet = NetPacketManager::Instance().CreatePacket(BuildPacketID(HEADER_CG_HANDSHAKE, PACKET_TYPE_CS));
		if (!reply_packet)
		{
			NET_LOG(LL_CRI, "Handshake packet could not created!");
			abort();
			return;
		}	

		reply_packet->SetField<uint32_t>("handshake", handshake);
		reply_packet->SetField<uint32_t>("time", time + delta + delta);
		reply_packet->SetField<uint32_t>("delta", 0);

		Send(reply_packet);
	}
}
