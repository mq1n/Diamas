#include "../include/NetEngine.hpp"
#include "NetServerManager.hpp"
#include "NetPeerManager.hpp"

namespace net_engine
{	
	CNetworkServerManager::CNetworkServerManager(NetServiceBase& netService, const std::string& ip_address, uint16_t port, uint8_t securityLevel, const TPacketCryptKey& cryptKey) :
		NetServerBase(netService(), securityLevel, cryptKey), m_netService(netService),
		m_ip_address(ip_address), m_port(port),
		m_securityLevel(securityLevel), m_crypt_key(cryptKey)
	{
		NET_LOG(LL_TRACE, "Creating server on %s:%u", ip_address.c_str(), port);
		NetPacketManager::Instance().RegisterPackets(true);
	}
	CNetworkServerManager::~CNetworkServerManager()
	{
		NET_LOG(LL_TRACE, "Stopping server...");
		Stop();
	}

	bool CNetworkServerManager::ParseCommandLine()
	{
		NET_LOG(LL_SYS, "ParseCommandLine triggered!");

		return true;
	}

	bool CNetworkServerManager::ParseConfigFile()
	{
		NET_LOG(LL_SYS, "ParseConfigFile triggered!");

		return true;
	}

	bool CNetworkServerManager::CreateDBConnection()
	{
		NET_LOG(LL_SYS, "CreateDBConnection triggered!");

		return true;
	}

	void CNetworkServerManager::ServerWorker()
	{
		//try
		{
			NET_LOG(LL_SYS, "Network engine initializing...");

			if (!ParseCommandLine())
			{
				NET_LOG(LL_ERR, "ParseCommandLine has been failed!");
				return;
			}

			if (!ParseConfigFile())
			{
				NET_LOG(LL_ERR, "ParseConfigFile has been failed!");
				return;
			}

			if (!CreateDBConnection())
			{
				NET_LOG(LL_ERR, "CreateDBConnection has been failed!");
				return;
			}

			NET_LOG(LL_SYS, "Network engine binding to %s:%u", m_ip_address.c_str(), m_port);
			Bind(m_ip_address, m_port);

			NET_LOG(LL_SYS, "Server initialized");

			NET_LOG(LL_SYS, "Network engine starting...");
			Run();

			NET_LOG(LL_SYS, "Network engine shutting down...");
			Shutdown();
		}
		/*
		catch (const asio::system_error& e)
		{
			NET_LOG(LL_CRI, "Exception handled: %s", e.what());
			abort();
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
		*/
	}

	bool CNetworkServerManager::Initialize()
	{
// 		m_serverThread = new std::thread(&CNetworkServerManager::ServerWorker, std::ref(*this));
		ServerWorker();
		return true;
	}
	void CNetworkServerManager::Stop()
    {
        Shutdown();
    }

	void CNetworkServerManager::BroadcastPacket(std::shared_ptr <Packet> packet)
	{
		for (const auto& [id, peer] : m_peers)
		{
			if (peer && peer.get())
			{
				peer->Send(packet);
			}
		}
	}
	void CNetworkServerManager::SendTo(std::shared_ptr <Packet> packet, std::function<bool(CNetworkConnectionManager*)> filter)
	{
		for (const auto& [id, peer] : m_peers)
		{
			if (peer && peer.get())
			{
				if (filter && filter(peer.get()))
				{
					peer->Send(packet);
				}
			}
		}		
	}
    void CNetworkServerManager::SendToPhase(std::shared_ptr <Packet> packet, uint8_t phase)
	{
        SendTo(std::move(packet), [phase](CNetworkConnectionManager* connection) {
            return connection->GetPhase() == phase;
        });
    }

	// I/O Service wrappers
	void CNetworkServerManager::Run()
	{
		m_netService.Run();
	}
	void CNetworkServerManager::Shutdown()
	{
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
	std::shared_ptr <CNetworkConnectionManager> CNetworkServerManager::FindPeer(uint32_t id) const
	{
		auto iter = m_peers.find(id);
		if (iter != m_peers.end())
			return iter->second;

		return nullptr;
	}

	std::shared_ptr <NetPeerBase> CNetworkServerManager::NewPeer()
	{
		auto peer = std::make_shared<CNetworkConnectionManager>(std::static_pointer_cast<CNetworkServerManager>(shared_from_this()), m_securityLevel, m_crypt_key);
		m_peers.emplace(peer->GetId(), peer);

		return peer;
	}

	void CNetworkServerManager::RemovePeer(uint32_t id)
	{
		auto iter = m_peers.find(id);
		if (iter != m_peers.end())
			m_peers.erase(iter);
	}
}
