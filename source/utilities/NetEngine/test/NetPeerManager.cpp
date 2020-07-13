#include "../include/NetEngine.hpp"
#include "NetPeerManager.hpp"
#include "NetServerManager.hpp"

namespace net_engine
{
	CNetworkConnectionManager::CNetworkConnectionManager(
		std::shared_ptr <CNetworkServerManager> server, uint8_t securityLevel, const TPacketCryptKey& cryptKey) :
		NetPeerBase(server->GetServiceInstance()(), securityLevel, cryptKey, true), m_server(server), m_deadline_timer(server->GetServiceInstance()())
	{
		NET_LOG(LL_TRACE, "Creating connection object");

		RegisterPacket(
			"GC_KEY_AGREEMENT_COMPLETED", ReservedGCHeaders::HEADER_GC_KEY_AGREEMENT_COMPLETED, nullptr, false, true, nullptr
		);
		RegisterPacket(
			"GC_PHASE", ReservedGCHeaders::HEADER_GC_PHASE, nullptr, false, true,
			[](std::shared_ptr <PacketDefinition> packet_def) -> void {
				packet_def->AddField<uint8_t>("phase");
			}
		);
		RegisterPacket(
			"GC_KEY_AGREEMENT", ReservedGCHeaders::HEADER_GC_KEY_AGREEMENT, nullptr, false, true,
			[](std::shared_ptr <PacketDefinition> packet_def) -> void {
				packet_def->AddField<uint16_t>("valuelen");
				packet_def->AddField<uint16_t>("datalen");
				packet_def->AddField<char[256]>("data");
			}
		);
		RegisterPacket(
			"GC_HANDSHAKE", ReservedGCHeaders::HEADER_GC_HANDSHAKE, nullptr, false, true,
			[](std::shared_ptr <PacketDefinition> packet_def) -> void {
				packet_def->AddField<uint32_t>("handshake");
				packet_def->AddField<uint32_t>("time");
				packet_def->AddField<uint32_t>("delta");
			}
		);

		// ---

		RegisterPacket(
			"CG_KEY_AGREEMENT", ReservedCGHeaders::HEADER_CG_KEY_AGREEMENT, std::bind(&CNetworkConnectionManager::OnRecvKeyAgreementPacket, this, std::placeholders::_1), true, false,
			[](std::shared_ptr <PacketDefinition> packet_def) -> void {
				packet_def->AddField<uint16_t>("valuelen");
				packet_def->AddField<uint16_t>("datalen");
				packet_def->AddField<char[256]>("data");
			}
		);
		RegisterPacket(
			"CG_HANDSHAKE", ReservedCGHeaders::HEADER_CG_HANDSHAKE, std::bind(&CNetworkConnectionManager::OnRecvHandshakePacket, this, std::placeholders::_1), true, false,
			[](std::shared_ptr <PacketDefinition> packet_def) -> void {
				packet_def->AddField<uint32_t>("handshake");
				packet_def->AddField<uint32_t>("time");
				packet_def->AddField<uint32_t>("delta");
			}
		);

		m_deadline_timer.expires_from_now(std::chrono::seconds(30));
     	m_deadline_timer.async_wait(std::bind(&CNetworkConnectionManager::CheckDeadlineStatus, this));
	}
	CNetworkConnectionManager::~CNetworkConnectionManager()
	{
		NET_LOG(LL_TRACE, "Destroying connection object");
	}

	void CNetworkConnectionManager::CheckDeadlineStatus()
    {
		auto phase = GetPhase();
		switch (phase)
		{
			case PHASE_HANDSHAKE:
			case PHASE_LOGIN:
			case PHASE_AUTH:
			// TODO: drop connection
				break;
			
			default:
				break;
		}
	}

	void CNetworkConnectionManager::Destroy()
	{
		auto server = GetServer();
		if (server)
			server->RemovePeer(GetId());
	}

	// net callbacks
	void CNetworkConnectionManager::OnConnect()
	{
		NET_LOG(LL_SYS, "Client connected %s (%d)", GetIP().c_str(), GetId());
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
		const auto header = packet->GetHeader();
		NET_LOG(LL_SYS, "Peer: %s(%d) Packet: %u(0x%x) is ready for process!", GetIP().c_str(), GetId(), header, header);

		const auto packet_map = m_handlers.find(header);
		if (packet_map == m_handlers.end())
		{
			NET_LOG(LL_ERR, "Unknown Packet with id %d (%02x) received from PEER %d",
				header, header, GetId()
			);
			return;
		}
		packet_map->second(packet);
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


	bool CNetworkConnectionManager::HasProperty(const std::string& property) const
	{
		return m_properties.find(property) != m_properties.end();
	}

	void CNetworkConnectionManager::SetProperty(const std::string& property, ConnectionProperty value)
	{
		m_properties[property] = std::move(value);
	}

	ConnectionProperty CNetworkConnectionManager::GetProperty(const std::string& property) const
	{
		return m_properties.at(property);
	}


	void CNetworkConnectionManager::SendAsReply(std::shared_ptr<Packet> request, std::shared_ptr<Packet> reply)
	{
		auto id = request->GetField<uint64_t>("__REFERENCE_ID");

		reply->SetField<uint64_t>("__REFERENCE_ID", id);
		reply->SetField<uint8_t>("__REFERENCE_TYPE", 1);

		Send(reply);
	}


	void CNetworkConnectionManager::OnRecvKeyAgreementPacket(std::shared_ptr <Packet> packet)
	{
		HandleKeyAgreementGC(packet);
	}

	void CNetworkConnectionManager::OnRecvHandshakePacket(std::shared_ptr <Packet> packet)
	{
		HandleHandshakeGC(packet);
	}
}
