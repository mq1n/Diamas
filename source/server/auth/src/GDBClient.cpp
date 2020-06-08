#include "stdafx.h"

#include "GDBClient.h"
#include "GAuthServer.h"
#include "Packets.h"
#include "GAuthPeer.h"
#include "Utils.h"

GDBClient::GDBClient(std::shared_ptr <GAuthServer> server) : 
	net_engine::NetClientBase(server->GetService()(), 5000), m_server(server), m_mainPort(0)
{
	// register packets
	RegisterPacket<TPacketDGAuthLogin>(HEADER_DG_AUTH_LOGIN, std::bind(&GDBClient::RecvAuthLogin, this, std::placeholders::_1, std::placeholders::_2));
}

void GDBClient::SetMainPort(uint16_t port)
{
	m_mainPort = port; 
}
uint16_t GDBClient::GetMainPort() const
{ 
	return m_mainPort;
}

void GDBClient::OnConnect()
{
	sys_log(LL_SYS, "Connected to DBCache %s:%u", GetIP().c_str(), GetPort());
	SendSetup(m_mainPort);
	BeginRead();
}
void GDBClient::OnDisconnect() 
{
	sys_log(LL_SYS, "Reconnecting to DBCache %s:%u", GetIP().c_str(), GetPort());
	Reconnect();
}

std::size_t GDBClient::OnRead(const void *data, std::size_t length) 
{
	auto pData = reinterpret_cast<const uint8_t *>(data);

	std::size_t offset = 0;
	while (offset < length) 
	{
		uint8_t packetId = 0;
		while (offset < length && (packetId = pData[offset]) == 0) ++offset;

		auto handler = m_handlers.find(packetId);
		if (handler == m_handlers.end()) 
		{
			// unkwnown packet dc
			sys_log(LL_ERR, "Unknown Packet with id %d (%02x) received from DBCache", packetId, packetId);

			asio::error_code er;
			Disconnect(er);
			return 0;
		}

		std::size_t handlerResult = handler->second.second(pData + offset, length - offset);
		if (handlerResult == 0) 
			break; // handler returned 0 == zu wenig daten
		offset += handlerResult;
	}
	return offset;
}
std::size_t GDBClient::OnWrite(const void *data, std::size_t length)
{
	return length;
}

void GDBClient::SendSetup(uint16_t mainPort) 
{
	TPacketGDSetup packet;
	packet.packetSize = packet.size() - 9;
	packet.mainPort = mainPort;
	packet.authServer = true;
	Send(&packet, packet.size());
}

void GDBClient::SendAuthLogin(int32_t peerId, int64_t id, const std::string &login, const std::string &socialId,
							  const std::string &lang,
#if defined(AUTH_USE_HWID)
							  const std::string &hwid,
#endif
							  uint32_t loginKey, const std::vector<int32_t> &aiPremiumTimes, const uint8_t *clientKey)
{
	TPacketGDAuthLogin packet;
	packet.handle = peerId;
	packet.packetSize = packet.size() - sizeof(packet.handle) - sizeof(packet.packetSize) - sizeof(packet.header);
	packet.id = id;
	packet.loginKey = loginKey;
	CopyStringSafe(packet.login, login);
	CopyStringSafe(packet.socialId, socialId);
	CopyStringSafe(packet.lang, lang);
	std::memcpy(packet.clientKey, clientKey, std::min<std::size_t>(sizeof(packet.clientKey), 16));

	if (!aiPremiumTimes.empty()) 
		memcpy(packet.premiumTimes, aiPremiumTimes.data(), std::min<std::size_t>(sizeof(packet.premiumTimes), aiPremiumTimes.size() * sizeof(int32_t)));

#if defined(AUTH_USE_HWID)
	CopyStringSafe(packet.hwid, hwid);
#endif
	Send(&packet, packet.size());
}

std::size_t GDBClient::RecvAuthLogin(const void *data, std::size_t maxlength)
{
	std::shared_ptr<GAuthServer> server(m_server.lock());

	if (maxlength < TPacketDGAuthLogin::size())
		return 0;

	auto packet = reinterpret_cast<const TPacketDGAuthLogin *>(data);

	auto peer = server->FindPeer(packet->handle);
	if (peer && peer.get()) 
	{
		AUTH_LOG(info, "Client {0} ({1}): Login success {2}", peer->GetIP(), peer->GetId(), peer->GetLogin());
		peer->LoginSuccess(packet->result);
		peer->DelayedDisconnect(5000);
	}
	return packet->size();
}
