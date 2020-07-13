#pragma once
#include "../include/NetEngine.hpp"
#include "NetPeerManager.hpp"
#include "nameof.hpp"

namespace net_engine
{	
	class CNetworkServerManager : public NetServerBase
	{
	public:
		CNetworkServerManager(NetServiceBase& netService, const std::string& ip_address, uint16_t port, uint8_t securityLevel, const TPacketCryptKey& cryptKey);
		virtual ~CNetworkServerManager();	

		NetServiceBase & GetServiceInstance();

		bool Initialize();
		void Stop();
		void ServerWorker();
	
		virtual void Run();
		virtual void Shutdown();
		virtual bool IsShuttingDown();
	
		virtual std::shared_ptr <CNetworkConnectionManager> FindPeer(uint32_t id) const;
		virtual void RemovePeer(uint32_t id);

		void BroadcastPacket(std::shared_ptr <Packet> packet);
		void SendTo(std::shared_ptr <Packet> packet, std::function<bool(CNetworkConnectionManager*)> filter);
		void SendToPhase(std::shared_ptr <Packet> packet, uint8_t phase);
	
	protected:
		virtual std::shared_ptr <NetPeerBase> NewPeer();
		
		bool ParseCommandLine();
		bool ParseConfigFile();
		bool CreateDBConnection();

	private:
		std::string m_ip_address;
		uint16_t m_port;

		NetServiceBase&	m_netService;

		TPacketCryptKey m_crypt_key;
		uint8_t m_securityLevel;

		std::unordered_map <uint32_t, std::shared_ptr <CNetworkConnectionManager> >	m_peers;
	};
};
