#include "../include/NetEngine.hpp"
#include "NetServerManager.hpp"
#include "NetPeerManager.hpp"
#include "Packet.hpp"

namespace net_engine
{	
	CNetworkServerManager::CNetworkServerManager(NetServiceBase& netService, const std::string& ip_address, uint16_t port, uint8_t securityLevel, const TPacketCryptKey& cryptKey) :
		NetServerBase(netService(), securityLevel, cryptKey), m_netService(netService), m_updateTimer(netService()),
		m_ip_address(ip_address), m_port(port),
		m_stopped(false),
		m_securityLevel(securityLevel), m_crypt_key(cryptKey)
	{
		NET_LOG(LL_TRACE, "Creating server on %s:%u", ip_address.c_str(), port);
	}
	CNetworkServerManager::~CNetworkServerManager()
	{
		NET_LOG(LL_TRACE, "Stopping server...");
		Stop();
	}

#if 0
    void CNetworkServerManager::OnUpdate()
    {
        if (m_stopped)
            return;

        m_updateTimer.expires_from_now(std::chrono::milliseconds(10));
        m_updateTimer.async_wait(std::bind(&CNetworkServerManager::OnUpdate, this));

		BroadcastMessage("tick");
#if 0
        AddNewSockets();

        _sockets.erase(std::remove_if(_sockets.begin(), _sockets.end(), [this](std::shared_ptr<SocketType> sock)
        {
            if (!sock->Update())
            {
                if (sock->IsOpen())
                    sock->CloseSocket();

                this->SocketRemoved(sock);

                --this->_connections;
                return true;
            }

            return false;
        }), _sockets.end());
#endif
	}
#endif

	void CNetworkServerManager::ServerWorker()
	{
		try
		{
			NET_LOG(LL_SYS, "Network engine binding to %s:%u", m_ip_address.c_str(), m_port);
			Bind(m_ip_address, m_port);

			NET_LOG(LL_SYS, "Network engine initializing...");
			// TODO: Config, pre initilization

			/*
			RegisterPacket(
				HEADER_CG_LOGIN, EPacketTypes::PacketTypeOutgoing, false, std::bind(&CNetworkServerManager::OnRecvLoginPacket, this, std::placeholders::_1, std::placeholders::_2)
			);
			RegisterPacket(
				HEADER_CG_CHAT, EPacketTypes::PacketTypeOutgoing, false, std::bind(&CNetworkServerManager::OnRecvChatPacket, this, std::placeholders::_1, std::placeholders::_2)
			);
			*/

			// TODO: DB Connection and other stuffs

			NET_LOG(LL_SYS, "Server initialized");

#if 0
			m_updateTimer.expires_from_now(std::chrono::milliseconds(10));
     	  	m_updateTimer.async_wait(std::bind(&CNetworkServerManager::OnUpdate, this));
#endif

			NET_LOG(LL_SYS, "Network engine started!");
			Run();

			NET_LOG(LL_SYS, "Network engine shutting down...");
			Shutdown();
		}
		catch (const asio::system_error& e)
		{
			NET_LOG(LL_CRI, "Exception handled: %s", e.what());
		}
		catch (std::exception& e)
		{
			NET_LOG(LL_CRI, "Exception handled: %s", e.what());
			abort();
		}
		catch (DWORD dwNumber)
		{
			NET_LOG(LL_CRI, "Exception handled: %p", dwNumber);
			abort();
		}
		catch (...)
		{
			NET_LOG(LL_CRI, "Unhandled exception");
			abort();
		}
	}

	bool CNetworkServerManager::Initialize()
	{
// 		m_serverThread = new std::thread(&CNetworkServerManager::ServerWorker, std::ref(*this));
		ServerWorker();
		return true;
	}
	void CNetworkServerManager::Stop()
    {
        m_stopped = true;
        Shutdown();
    }

	// test only
	void CNetworkServerManager::BroadcastMessage(const std::string& msg)
	{
		/*
		for (const auto& [id, peer] : m_peers)
		{
			if (peer && peer.get())
			{
				peer->SendChatPacket(msg);
			}
		}
		*/
	}

	// I/O Service wrappers
	void CNetworkServerManager::Run()
	{
		m_netService.Run();
	}
	void CNetworkServerManager::Shutdown()
	{
		// _newSockets.clear();
        // _sockets.clear();

		m_netService.Stop();
	}
	bool CNetworkServerManager::IsShuttingDown()
	{
		return m_netService.HasStopped();
	}
	NetServiceBase& CNetworkServerManager::GetServiceInstance()
	{
		return m_netService;
	}

	// Peer stuffs
	std::shared_ptr <CNetworkConnectionManager> CNetworkServerManager::FindPeer(int32_t id) const
	{
		auto iter = m_peers.find(id);
		if (iter != m_peers.end())
			return iter->second;

		return nullptr;
	}

	std::shared_ptr <NetPeerBase> CNetworkServerManager::NewPeer()
	{
		auto peer = std::make_shared<CNetworkConnectionManager>(std::static_pointer_cast<CNetworkServerManager>(shared_from_this()), m_securityLevel, m_crypt_key);

#//ifdef _DEBUG
//		peer->SetLogLevel(LL_ONREAD | LL_ONWRITE_PRE | LL_ONWRITE_POST | LL_ONREGISTERPACKET);
//#endif
//
		m_peers.emplace(peer->GetId(), peer);
		return peer;
	}

	void CNetworkServerManager::RemovePeer(int32_t id)
	{
		auto iter = m_peers.find(id);
		if (iter != m_peers.end())
			m_peers.erase(iter);
	}

#if 0
	std::size_t CNetworkServerManager::OnRecvLoginPacket(const void* data, std::size_t maxlength)
	{
		auto packet = reinterpret_cast<const SNetPacketCGLogin*>(data);

		if (maxlength < packet->size())
			return 0;

//		if (maxlength != SNetPacketGCLogin::size())
//		{
//			NET_LOG(LL_CRI, "Packet size mismatch it can be combined packet!");
//			return SNetPacketGCLogin::size();
//		}

		NET_LOG(LL_SYS, "Recv login packet! ID: %s PWD: %s", packet->login, packet->password);

		SNetPacketGCChat retPacket;
		strcpy_s(retPacket.msg, "done!");
		//peer->SendCrypted(retPacket, true);

		// ...

		return packet->size();
	}

	std::size_t CNetworkServerManager::OnRecvChatPacket(const void* data, std::size_t maxlength)
	{
		auto packet = reinterpret_cast<const SNetPacketGCChat*>(data);

		if (maxlength < packet->size())
			return 0;

//		if (maxlength != SNetPacketGCChat::size())
//		{
//			NET_LOG(LL_CRI, "Packet size mismatch it can be combined packet!");
//			return SNetPacketGCChat::size();
//		}


		// ...

		return packet->size();
	}
#endif
}
