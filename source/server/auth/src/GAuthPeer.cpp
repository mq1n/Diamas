#include "stdafx.h"

#include "GAuthPeer.h"
#include "GAuthServer.h"
#include "Packets.h"
#include "Utils.h"

GAuthPeer::GAuthPeer(std::shared_ptr<GAuthServer> server) : net_engine::NetPeerBase(server->GetService()()), m_server(server), m_loginKey(0) {
	m_id = UniqueId();
}
GAuthPeer::~GAuthPeer() {
	Destroy();
}

std::shared_ptr<GAuthServer> GAuthPeer::GetServer() const { return m_server.lock(); }
int32_t GAuthPeer::GetId() const { return m_id; }

void GAuthPeer::OnConnect()
{
	AUTH_LOG(info, "Client connected {0} ({1})", GetIP(), GetId());
	Phase(PHASE_AUTH); // auth phase
	BeginRead();
}
void GAuthPeer::OnDisconnect()
{
	AUTH_LOG(info, "Client disconnected {0} ({1})", GetIP(), GetId());
	std::shared_ptr<GAuthServer> server = GetServer();
	if(server)
	{
		if(!m_name.empty())
			server->GetAccountManager().DisconnectAccount(m_name);
		server->RemovePeer(GetId());
	}
}

std::size_t GAuthPeer::OnRead(const void* data, std::size_t length)
{
	std::size_t result = 0;
	std::shared_ptr<GAuthServer> server = GetServer();
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
}
std::size_t GAuthPeer::OnWrite(const void* data, std::size_t length)
{
	return length;
}

void GAuthPeer::Destroy() { GetServer()->RemovePeer(GetId()); }

void GAuthPeer::Phase(uint8_t phaseId)
{
	TPacketGCPhase packet;
	packet.phase = phaseId;
	Send(&packet, packet.size());
	sys_log(LL_SYS, "Sent phase packet %s (%d)", GetIP().c_str(), GetId());

}
void GAuthPeer::LoginFailure(const std::string& status)
{
	TPacketGCLoginFailure packet;
	CopyStringSafe(packet.status, status);
	Send(&packet, packet.size());
}
void GAuthPeer::LoginSuccess(uint8_t result)
{
	TPacketGCAuthSuccess packet;
	packet.loginKey = GetLoginKey();
	packet.result = result;
	Send(&packet, packet.size());
}


void GAuthPeer::SetClientKey(const uint8_t* key) { memcpy(m_clientKey, key, 16); }
const uint8_t* GAuthPeer::GetClientKey() const { return m_clientKey; }

void GAuthPeer::SetLoginKey(uint32_t loginKey) { m_loginKey = loginKey; }
uint32_t GAuthPeer::GetLoginKey() const { return m_loginKey; }

#if defined(AUTH_USE_HWID)
void GAuthPeer::SetHwid(const std::string &hwid) { m_hwid = hwid; }
const std::string& GAuthPeer::GetHwid() const { return m_hwid; }
#endif
