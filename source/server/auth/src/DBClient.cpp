#include "stdafx.hpp"
#include "DBClient.hpp"
#include "AuthServer.hpp"
#include "AuthPeer.hpp"
#include "AuthLogHelper.hpp"
using namespace net_engine;

GDBClient::GDBClient(std::shared_ptr <GAuthServer> server) :
	net_engine::NetClientBase(server->GetService()(), 0, {}, 0, EPacketType::PACKET_TYPE_DS, 5000), m_server(server), m_mainPort(0)
{
	auth_log(LL_TRACE, "Creating db client object");

	REGISTER_PACKET_HANDLER(HEADER_DG_AUTH_LOGIN, std::bind(&GDBClient::RecvAuthLogin, this, std::placeholders::_1));
}

void GDBClient::OnConnect()
{
	auth_log(LL_SYS, "Connected to DBCache %s:%u", GetIP().c_str(), GetPort());

	SendSetup(m_mainPort);
}
void GDBClient::OnDisconnect(const asio::error_code& er)
{
	auth_log(LL_SYS, "DB Client disconnected: %d(%s). Reconnecting to: %s:%u", er.value(), er.message().c_str(), GetIP().c_str(), GetPort());

	Reconnect();
}
void GDBClient::OnRead(std::shared_ptr <Packet> packet)
{
	const auto header = packet->GetHeader();
	auth_log(LL_SYS, "Packet: %u(0x%x) is ready for process!", header, header);

	const auto handler = m_dispatcher.GetPacketHandler(header);
	if (!handler)
	{
		auth_log(LL_ERR, "Unknown Packet with id %d (%02x) received", header, header);
		return;
	}
	handler(packet);
}
void GDBClient::OnError(uint32_t error_type, const asio::error_code& er)
{
	auth_log(LL_ERR, "Network error handled! ID: %u System error: %d(%s)", error_type, er.value(), er.message().c_str());
}

void GDBClient::SendSetup(uint16_t mainPort) 
{
	auto packet = NetPacketManager::Instance().CreatePacket(BuildPacketID(HEADER_GD_SETUP, PACKET_TYPE_SD));
	packet->SetField<uint32_t>("packetSize", sizeof(TPacketGDSetup));
	packet->SetField<uint16_t>("mainPort", mainPort);
	packet->SetField<uint8_t>("authServer", 1);

	Send(packet);
}
void GDBClient::SendAuthLogin(
	uint32_t peerId, uint32_t id, const std::string &login, const std::string &socialId, const std::string &lang,
	const std::string &hwid, uint32_t loginKey, const std::vector<int32_t> &aiPremiumTimes, const uint8_t *clientKey)
{
	auto packet = NetPacketManager::Instance().CreatePacket(BuildPacketID(HEADER_GD_AUTH_LOGIN, PACKET_TYPE_SD));
	packet->SetField<uint32_t>("handle", peerId);
	packet->SetField<uint32_t>("packetSize", sizeof(TPacketGDAuthLogin));
	packet->SetField<uint32_t>("id", id);
	packet->SetField<uint32_t>("loginKey", loginKey);
	packet->SetString("login", login);
	packet->SetString("socialId", socialId);
	packet->SetString("lang", lang);
	packet->SetString("hwid", hwid);
	packet->SetField("clientKey", (uint8_t*)clientKey, 16);
	if (!aiPremiumTimes.empty())
		packet->SetField("premiumTimes", (uint8_t*)aiPremiumTimes.data(), aiPremiumTimes.size() * sizeof(int32_t));

	Send(packet);
}

void GDBClient::RecvAuthLogin(std::shared_ptr <Packet> packet)
{
	std::shared_ptr <GAuthServer> server(m_server.lock());
	if (server)
	{
		auto handle = packet->GetField<uint32_t>("handle");
		auto result = packet->GetField<uint8_t>("result");

		auto peer = server->FindPeer(handle);
		if (peer && peer.get())
		{
			auth_log(LL_SYS, "Client %s (%u): Login success %s", peer->GetIP().c_str(), peer->GetId(), peer->GetLogin().c_str());
			peer->SendLoginSuccess(result);
			peer->DelayedDisconnect(5000);
		}
	}
}
