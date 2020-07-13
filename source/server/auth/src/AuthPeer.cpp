#include "stdafx.hpp"
#include "AuthPeer.hpp"
#include "AuthServer.hpp"
#include "Packets.hpp"
using namespace net_engine;
/*
CNetworkConnectionManager::CNetworkConnectionManager(
	std::shared_ptr <CNetworkServerManager> server, uint8_t securityLevel, const TPacketCryptKey& cryptKey) :
	NetPeerBase(server->GetServiceInstance()(), securityLevel, cryptKey, true), m_server(server), m_deadline_timer(server->GetServiceInstance()())
	*/
GAuthPeer::GAuthPeer(std::shared_ptr <GAuthServer> server, uint8_t securityLevel, const TPacketCryptKey& cryptKey) :
	NetPeerBase(server->GetService()(), securityLevel, cryptKey, true), m_server(server), m_loginKey(0)
{
//	m_id = UniqueId();
}
GAuthPeer::~GAuthPeer()
{
	Destroy();
}

void GAuthPeer::OnConnect()
{
	AUTH_LOG(info, "Client connected {0} ({1})", GetIP(), GetId());
	Phase(PHASE_AUTH); // auth phase
	BeginRead();
}
void GAuthPeer::OnDisconnect(const asio::error_code& er)
{
	AUTH_LOG(info, "Client disconnected {0} ({1}). Reason {2} ({3})",
		GetIP(), GetId(), er.value(), er.message().c_str());

	std::shared_ptr <GAuthServer> server = GetServer();
	if(server)
	{
		if(!m_name.empty())
			server->GetAccountManager().DisconnectAccount(m_name);
		server->RemovePeer(GetId());
	}
}

void GAuthPeer::OnRead(std::shared_ptr <Packet> packet)
{
	/*
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
	*/

	/*
	std::size_t result = 0;
	std::shared_ptr <GAuthServer> server = GetServer();
	if(server)
	{
		result = server->ProcessInput(std::static_pointer_cast<GAuthPeer>(shared_from_this()), data, length);
		BeginRead();
	}
	else
	{
		asio::error_code er;
		Disconnect(er);
	}
	return result;
	*/
}
void GAuthPeer::OnError(uint32_t error_type, const asio::error_code& er)
{
	sys_log(LL_ERR, "Network error handled! ID: %u System error: %d(%s)", error_type, er.value(), er.message().c_str());
}

void GAuthPeer::Destroy()
{
	GetServer()->RemovePeer(GetId());
}

void GAuthPeer::Phase(uint8_t phaseId)
{
	/*
	TPacketGCPhase packet;
	packet.phase = phaseId;
	Send(&packet, packet.size());
	sys_log(LL_SYS, "Sent phase packet %s (%d)", GetIP().c_str(), GetId());
	*/
}
void GAuthPeer::LoginFailure(const std::string& status)
{
	/*
	TPacketGCLoginFailure packet;
	CopyStringSafe(packet.status, status);
	Send(&packet, packet.size());
	*/
}
void GAuthPeer::LoginSuccess(uint8_t result)
{
	/*
	TPacketGCAuthSuccess packet;
	packet.loginKey = GetLoginKey();
	packet.result = result;
	Send(&packet, packet.size());
	*/
}

void GAuthPeer::SetClientKey(const uint8_t* key)
{
	memcpy(m_clientKey, key, sizeof(m_clientKey));
}
const uint8_t* GAuthPeer::GetClientKey() const
{
	return m_clientKey;
}

void GAuthPeer::SetLoginKey(uint32_t loginKey)
{
	m_loginKey = loginKey;
}
uint32_t GAuthPeer::GetLoginKey() const
{
	return m_loginKey;
}

void GAuthPeer::SetHwid(const std::string &hwid)
{
	m_hwid = hwid;
}
const std::string& GAuthPeer::GetHwid() const
{
	return m_hwid;
}
