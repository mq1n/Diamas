#include "../include/NetEngine.h"
#include "NetServerManager.h"
#include "NetPeerManager.h"
#include "Packet.h"

namespace net_engine
{	
#define THREE_ARG_HOLDER std::placeholders::_1, std::placeholders::_2, std::placeholders::_3

	CNetworkServerManager::CNetworkServerManager(NetServiceBase& netService, const std::string& ip_address, uint16_t port) :
		NetServerBase(netService()), m_netService(netService), m_updateTimer(netService()),
		m_ip_address(ip_address), m_port(port),
		m_stopped(false)
	{
	}
	CNetworkServerManager::~CNetworkServerManager()
	{
        Stop();
	}

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

	void CNetworkServerManager::ServerWorker()
	{
		try
		{
			NET_LOG(LL_SYS, "Network engine binding to %s:%u", m_ip_address.c_str(), m_port);
			Bind(m_ip_address, m_port);
			m_umPeerMap.clear(); // remove own peer

			NET_LOG(LL_SYS, "Network engine initializing...");
			Init();

     	 	m_updateTimer.expires_from_now(std::chrono::milliseconds(10));
     	  	m_updateTimer.async_wait(std::bind(&CNetworkServerManager::OnUpdate, this));

			NET_LOG(LL_SYS, "Network engine started!");
			Run();

			NET_LOG(LL_SYS, "Network engine shutting down...");
			Shutdown();
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
		for (const auto& [id, peer] : m_umPeerMap)
		{
			if (peer && peer.get())
			{
				peer->SendChatPacket(msg);
			}
		}
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

	// Init & Final funcs
	void CNetworkServerManager::Init()
	{
		// TODO: Config, pre initilization

		SetupPacketDispatchers();

		// TODO: DB Connection and other stuffs

		NET_LOG(LL_SYS, "Server initialized");
	}

	// Peer stuffs
	std::shared_ptr <CNetworkPeer> CNetworkServerManager::FindPeer(int32_t id) const
	{
		auto iter = m_umPeerMap.find(id);
		if (iter != m_umPeerMap.end())
			return iter->second;

		return nullptr;
	}

	std::shared_ptr <NetPeerBase> CNetworkServerManager::NewPeer()
	{
		auto peer = std::make_shared<CNetworkPeer>(std::static_pointer_cast<CNetworkServerManager>(shared_from_this()));

#ifdef _DEBUG
		peer->SetLogLevel(LL_ONREAD | LL_ONWRITE_PRE | LL_ONWRITE_POST | LL_ONREGISTERPACKET);
#endif

		m_umPeerMap.emplace(peer->GetId(), peer);
		return peer;
	}

	void CNetworkServerManager::RemovePeer(int32_t id)
	{
		auto iter = m_umPeerMap.find(id);
		if (iter != m_umPeerMap.end())
			m_umPeerMap.erase(iter);
	}

	// Packet handler
	std::size_t CNetworkServerManager::ProcessInput(std::shared_ptr <CNetworkPeer> peer, const void* data, std::size_t maxlength)
	{
		NET_LOG(LL_SYS, "Peer: %s(%d) Data: %p Max length: %u", peer->GetIP().c_str(), peer->GetId(), data, maxlength);

		auto pData = reinterpret_cast<const uint8_t *>(data);

		std::size_t offset = 0;
		while (offset < maxlength) 
		{
			uint32_t packetId = 0;
			while (offset < maxlength && (packetId = pData[offset]) == 0) ++offset;

			auto handler = m_handlers.find(packetId);
			if (handler == m_handlers.end()) 
			{
				// log + kick? unkwnown packet dc
				NET_LOG(LL_ERR, "Unknown Packet with id %d (%02x) received from PEER %d",
					packetId, packetId, peer->GetId()
				);

				asio::error_code e;
				peer->Disconnect(e);
				return 0;
			}

			std::size_t handlerResult = handler->second.second(peer, pData + offset, maxlength - offset);
			if (handlerResult == 0) 
				break; // handler returned 0 == too little data
			offset += handlerResult;
		}
		return offset;
	}

	void CNetworkServerManager::SetupPacketDispatchers()
	{
		RegisterPacket<SNetPacketGCLogin>(HEADER_GC_LOGIN,
			std::bind(&CNetworkServerManager::OnRecvLoginPacket, this, THREE_ARG_HOLDER)
		);
		RegisterPacket<SNetPacketGCChat>(HEADER_GC_CHAT,
			std::bind(&CNetworkServerManager::OnRecvChatPacket, this, THREE_ARG_HOLDER)
		);
	}

	std::size_t CNetworkServerManager::OnRecvLoginPacket(std::shared_ptr<CNetworkPeer> peer, const void* data, std::size_t maxlength)
	{
		auto packet = reinterpret_cast<const SNetPacketGCLogin*>(data);

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
		peer->SendCrypted(retPacket, true);

		// ...

		return packet->size();
	}

	std::size_t CNetworkServerManager::OnRecvChatPacket(std::shared_ptr<CNetworkPeer> peer, const void* data, std::size_t maxlength)
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
}
