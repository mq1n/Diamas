#include "../include/NetEngine.hpp"
#include "NetClientManager.hpp"
#include "ClientCipher.hpp"

namespace net_engine
{	
	CNetworkClientManager::CNetworkClientManager(NetServiceBase& service, uint8_t securityLevel, const TPacketCryptKey& cryptKey) :
		NetClientBase(service(), securityLevel, cryptKey, 5000), m_pNetService(service)
	{
		// TODO: SERVER-CLIENT TEK DOSYADA BÝRLEÞTÝR
		
		RegisterPacket(
			"GC_KEY_AGREEMENT_COMPLETED", ReservedGCHeaders::HEADER_GC_KEY_AGREEMENT_COMPLETED, std::bind(&CNetworkClientManager::OnRecvKeyAgreementCompletedPacket, this, std::placeholders::_1), true, false, nullptr
		);
		RegisterPacket(
			"GC_PHASE", ReservedGCHeaders::HEADER_GC_PHASE, std::bind(&CNetworkClientManager::OnRecvPhasePacket, this, std::placeholders::_1), true, false,
			[](std::shared_ptr <PacketDefinition> packet_def) -> void {
				packet_def->AddField<uint8_t>("phase");
			}
		);
		RegisterPacket(
			"GC_KEY_AGREEMENT", ReservedGCHeaders::HEADER_GC_KEY_AGREEMENT, std::bind(&CNetworkClientManager::OnRecvKeyAgreementPacket, this, std::placeholders::_1), true, false,
			[](std::shared_ptr <PacketDefinition> packet_def) -> void {
				packet_def->AddField<uint16_t>("valuelen");
				packet_def->AddField<uint16_t>("datalen");
				packet_def->AddField<char[256]>("data");
			}
		);
		RegisterPacket(
			"GC_HANDSHAKE", ReservedGCHeaders::HEADER_GC_HANDSHAKE, std::bind(&CNetworkClientManager::OnRecvHandshakePacket, this, std::placeholders::_1), true, false,
			[](std::shared_ptr <PacketDefinition> packet_def) -> void {
				packet_def->AddField<uint32_t>("handshake");
				packet_def->AddField<uint32_t>("time");
				packet_def->AddField<uint32_t>("delta");
			}
		);

		// ---

		RegisterPacket(
			"CG_KEY_AGREEMENT", ReservedCGHeaders::HEADER_CG_KEY_AGREEMENT, nullptr, false, true,
			[](std::shared_ptr <PacketDefinition> packet_def) -> void {
				packet_def->AddField<uint16_t>("valuelen");
				packet_def->AddField<uint16_t>("datalen");
				packet_def->AddField<char[256]>("data");
			}
		);
		RegisterPacket(
			"CG_HANDSHAKE", ReservedCGHeaders::HEADER_CG_HANDSHAKE, nullptr, false, true,
			[](std::shared_ptr <PacketDefinition> packet_def) -> void {
				packet_def->AddField<uint32_t>("handshake");
				packet_def->AddField<uint32_t>("time");
				packet_def->AddField<uint32_t>("delta");
			}
		);

		// ---

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

		const auto packet_map = m_handlers.find(header);
		if (packet_map == m_handlers.end())
		{
			NET_LOG(LL_ERR, "Unknown header: %u(0x%x) for process", header, header);
			return;
		}
		packet_map->second(packet);
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

		auto reply_packet = PacketManager::Instance().CreatePacket(ReservedCGHeaders::HEADER_CG_KEY_AGREEMENT, EPacketDirection::Outgoing);
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

		auto reply_packet = PacketManager::Instance().CreatePacket(ReservedCGHeaders::HEADER_CG_HANDSHAKE, EPacketDirection::Outgoing);
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
